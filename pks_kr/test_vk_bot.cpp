#include "test_vk_bot.h"
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <cctype>
#include <filesystem>
#include "json.hpp"

using json = nlohmann::json;

// --- CURL helpers ---
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// --- VKBot construction/destruction ---
VKBot::VKBot(const std::string& token, const std::string& gid, const std::string& version)
    : access_token(token), group_id(gid), api_version(version), running(false), longpoll_thread(nullptr) {}

VKBot::~VKBot() {
    stop();
}

// --- Universal VK API Request ---
std::string VKBot::makeApiRequest(const std::string& method, const std::string& params) {
    CURL* curl = curl_easy_init();
    std::string response;

    if (!curl) {
        std::cerr << "VK API: Failed to initialize CURL" << std::endl;
        return "";
    }

    std::string url = "https://api.vk.com/method/" + method;
    std::string post_fields = params + "&access_token=" + access_token + "&v=" + api_version;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "VKBot/1.0");

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::cerr << "VK API: CURL error: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        return "";
    }

    curl_easy_cleanup(curl);
    return response;
}

bool VKBot::enableLongPoll() {
    std::cout << "Enabling Long Poll API for private group..." << std::endl;

    std::string params = "group_id=" + group_id + "&enabled=1";
    params += "&message_new=1&message_reply=1&message_edit=0&message_allow=1";
    params += "&message_deny=0&message_typing_state=0";
    params += "&photo_new=0&audio_new=0&video_new=0";
    params += "&wall_reply_new=0&wall_reply_edit=0&wall_reply_delete=0&wall_reply_restore=0";
    params += "&wall_post_new=0&board_post_new=0&board_post_edit=0";
    params += "&board_post_restore=0&board_post_delete=0";
    params += "&group_join=1&group_leave=1";
    params += "&group_change_settings=0&group_change_photo=0&group_officers_edit=0";
    params += "&market_order_new=0&market_order_edit=0";

    std::string response = makeApiRequest("groups.setLongPollSettings", params);

    if (response.empty()) {
        std::cerr << "Failed to enable Long Poll API" << std::endl;
        return false;
    }

    try {
        auto resp_json = json::parse(response);
        if (resp_json.contains("response") && resp_json["response"] == 1) {
            std::cout << "Long Poll API enabled successfully!" << std::endl;
            return true;
        } else if (resp_json.contains("error")) {
            std::cerr << "Error enabling Long Poll: " << resp_json["error"]["error_msg"] << std::endl;
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing enable Long Poll response: " << e.what() << std::endl;
    }

    return false;
}

std::string VKBot::getLongPollSettings() {
    return makeApiRequest("groups.getLongPollSettings", "group_id=" + group_id);
}

std::string VKBot::getGroupInfo() {
    return makeApiRequest("groups.getById", "group_id=" + group_id);
}

// --- Keyboard builder (global free func, not member) ---
json createKeyboard(const std::vector<std::vector<std::pair<std::string, std::string>>>& buttons, bool one_time) {
    json keyboard;
    keyboard["one_time"] = one_time;
    keyboard["buttons"] = json::array();

    for (const auto& row : buttons) {
        json row_json = json::array();
        for (const auto& btn : row) {
            json button;
            button["action"]["type"] = "text";
            button["action"]["label"] = btn.first;
            button["action"]["payload"] = btn.second;

            if (btn.first.find("🎮") != std::string::npos ||
                btn.first.find("💚") != std::string::npos) {
                button["color"] = "positive";
            } else if (btn.first.find("🔫") != std::string::npos ||
                       btn.first.find("🎯") != std::string::npos) {
                button["color"] = "negative";
            } else if (btn.first.find("✨") != std::string::npos ||
                       btn.first.find("📜") != std::string::npos ||
                       btn.first.find("🤖") != std::string::npos ||
                       btn.first.find("💫") != std::string::npos) {
                button["color"] = "primary";
            } else {
                button["color"] = "secondary";
            }

            row_json.push_back(button);
        }
        keyboard["buttons"].push_back(row_json);
    }

    return keyboard;
}

// --- Core VK Message Loop & Command Processing ---
// (Остаётся весь старый код, только чуть дополнен для новой команды и кнопки)

void VKBot::processMessage(const json& update) {
    try {
        std::cout << "VK Bot: Processing update..." << std::endl;

        if (!update.contains("type")) {
            std::cerr << "VK Bot: Update doesn't contain 'type' field" << std::endl;
            return;
        }

        std::string event_type = update["type"];

        if (event_type == "message_new") {
            json message_data;

            if (update.contains("object") && update["object"].contains("message")) {
                message_data = update["object"]["message"];
            } else if (update.contains("object")) {
                message_data = update["object"];
            } else {
                std::cerr << "VK Bot: Cannot find message data in update" << std::endl;
                return;
            }

            int64_t user_id = 0;
            if (message_data.contains("from_id")) {
                if (message_data["from_id"].is_number()) {
                    user_id = message_data["from_id"].get<int64_t>();
                } else if (message_data["from_id"].is_string()) {
                    user_id = std::stoll(message_data["from_id"].get<std::string>());
                }
            } else if (message_data.contains("user_id")) {
                if (message_data["user_id"].is_number()) {
                    user_id = message_data["user_id"].get<int64_t>();
                } else if (message_data["user_id"].is_string()) {
                    user_id = std::stoll(message_data["user_id"].get<std::string>());
                }
            }

            std::string text;
            if (message_data.contains("text")) {
                text = message_data["text"].get<std::string>();
            }

            // Проверяем payload от клавиатуры
            std::string payload;
            if (message_data.contains("payload")) {
                payload = message_data["payload"].get<std::string>();
                std::cout << "VK Bot: Received payload: " << payload << std::endl;
            }

            if (user_id == 0) {
                std::cerr << "VK Bot: Invalid message data - user_id=0" << std::endl;
                return;
            }

            std::cout << "VK Bot: Received message from " << user_id << ": " << text << std::endl;

            // Определяем команду из payload или текста
            std::string command;

            if (!payload.empty()) {
                try {
                    auto payload_json = json::parse(payload);
                    if (payload_json.contains("command")) {
                        command = payload_json["command"].get<std::string>();
                    } else if (payload_json.contains("enemy")) {
                        command = payload_json["enemy"].get<std::string>();
                    } else if (payload_json.contains("action")) {
                        command = payload_json["action"].get<std::string>();
                    } else if (payload_json.contains("magic")) {
                        command = payload_json["magic"].get<std::string>();
                    }
                } catch (...) {
                    // Если не удалось распарсить payload, используем текст дальше
                }
            }

            // Если команда не определена из payload, определяем из текста
            if (command.empty()) {
                std::string lower_text = text;
                for (char& c : lower_text) {
                    c = std::tolower(c);
                }

                if (lower_text.find("/start") != std::string::npos || lower_text.find("начать") != std::string::npos) {
                    command = "start";
                } else if (lower_text.find("/rules") != std::string::npos || lower_text.find("правила") != std::string::npos) {
                    command = "rules";
                } else if (lower_text.find("/play") != std::string::npos || lower_text.find("играть") != std::string::npos) {
                    command = "play";
                } else if (lower_text.find("deepseek") != std::string::npos) {
                    command = "deepseek";
                } else if (lower_text.find("chatgpt") != std::string::npos) {
                    command = "chatgpt";
                } else if (lower_text.find("друг") != std::string::npos) {
                    command = "friend";
                } else if (lower_text.find("выстрелить в себя") != std::string::npos) {
                    command = "shoot_self";
                } else if (lower_text.find("выстрелить в врага") != std::string::npos) {
                    command = "shoot_enemy";
                } else if (lower_text.find("магия") != std::string::npos) {
                    command = "magic";
                } else if (lower_text.find("восстановление") != std::string::npos) {
                    command = "heal";
                } else if (lower_text.find("оглушение") != std::string::npos) {
                    command = "stun";
                } else if (lower_text.find("назад") != std::string::npos) {
                    command = "back";
                } else if (lower_text.find("меню") != std::string::npos) {
                    command = "menu";
                } else if (lower_text.find("power_up") != std::string::npos || lower_text.find("/power_up") != std::string::npos) {
                    command = "power_up";
                }
            }

            std::cout << "VK Bot: Executing command: " << command << std::endl;

            // --- PROCESS COMMANDS ---

            if (command == "start") {
                std::vector<std::vector<std::pair<std::string, std::string>>> buttons = {
                    {{"🎮 Играть", "{\"command\":\"play\"}"}},
                    {{"📜 Правила", "{\"command\":\"rules\"}"}},
                    {{"⚡ Power Up!", "{\"command\":\"power_up\"}"}}
                };
                json keyboard = createKeyboard(buttons, false);

                std::string welcome = "🎮 Привет! Я бот для игры Magic Revolver Duel!\n\n"
                                    "🔫 Правила игры:\n"
                                    "• У вас и противника по 3 жизни\n"
                                    "• В револьвере 6 патронов, но заряжен только 1\n"
                                    "• Вы можете выстрелить в себя или в противника\n"
                                    "• Если попадёте в себя - потеряете жизнь\n"
                                    "• Если попадёте в противника - он потеряет жизнь\n"
                                    "• Используйте магию, чтобы восстановить здоровье или оглушить врага!\n\n"
                                    "Используйте кнопки для навигации, или попробуйте команду /power_up:";
                sendMessageWithKeyboard(user_id, welcome, keyboard);

            } else if (command == "rules") {
                std::string rules = "📜 Правила игры Magic Revolver Duel:\n\n"
                                  "🎯 Цель: победить противника\n\n"
                                  "🔄 Ходы:\n"
                                  "1. Выстрелить в себя (риск)\n"
                                  "2. Выстрелить в противника\n"
                                  "3. Использовать магию\n\n"
                                  "✨ Магия:\n"
                                  "• Восстановление здоровья (+1 HP)\n"
                                  "• Оглушение противника (пропуск хода)\n\n"
                                  "💀 В револьвере всегда 1 пуля из 6\n"
                                  "После каждого выстрела барабан прокручивается";

                std::vector<std::vector<std::pair<std::string, std::string>>> buttons = {
                    {{"🎮 Играть", "{\"command\":\"play\"}"}},
                    {{"◀ Назад", "{\"command\":\"back\"}"}},
                    {{"⚡ Power Up!", "{\"command\":\"power_up\"}"}}
                };
                json keyboard = createKeyboard(buttons, false);

                sendMessageWithKeyboard(user_id, rules, keyboard);

            } else if (command == "play") {
                std::vector<std::vector<std::pair<std::string, std::string>>> buttons = {
                    {{"🤖 DeepSeek", "{\"enemy\":\"deepseek\"}"}},
                    {{"🤖 ChatGPT", "{\"enemy\":\"chatgpt\"}"}},
                    {{"👤 Друг (скоро)", "{\"enemy\":\"friend\"}"}},
                    {{"◀ Назад", "{\"command\":\"back\"}"}},
                    {{"⚡ Power Up!", "{\"command\":\"power_up\"}"}}
                };
                json keyboard = createKeyboard(buttons, true);

                std::string play_msg = "🎮 Выберите противника:";
                sendMessageWithKeyboard(user_id, play_msg, keyboard);

            } else if (command == "deepseek") {
                std::vector<std::vector<std::pair<std::string, std::string>>> buttons = {
                    {{"🔫 Выстрелить в себя", "{\"action\":\"shoot_self\"}"}},
                    {{"🎯 Выстрелить в врага", "{\"action\":\"shoot_enemy\"}"}},
                    {{"✨ Магия", "{\"action\":\"magic\"}"}},
                    {{"◀ В меню", "{\"command\":\"menu\"}"}},
                    {{"⚡ Power Up!", "{\"command\":\"power_up\"}"}}
                };
                json keyboard = createKeyboard(buttons, false);

                std::string msg = "🤖 Вы выбрали противника: DeepSeek\n"
                                "Игра начинается! Удачи!\n\n"
                                "Ваше здоровье: ❤️❤️❤️ (3)\n"
                                "Здоровье врага: 💚💚💚 (3)\n"
                                "Ваша мана: 💙💙💙 (3)\n\n"
                                "Ваш ход! Выберите действие:";
                sendMessageWithKeyboard(user_id, msg, keyboard);

            } else if (command == "chatgpt") {
                std::vector<std::vector<std::pair<std::string, std::string>>> buttons = {
                    {{"🔫 Выстрелить в себя", "{\"action\":\"shoot_self\"}"}},
                    {{"🎯 Выстрелить в врага", "{\"action\":\"shoot_enemy\"}"}},
                    {{"✨ Магия", "{\"action\":\"magic\"}"}},
                    {{"◀ В меню", "{\"command\":\"menu\"}"}},
                    {{"⚡ Power Up!", "{\"command\":\"power_up\"}"}}
                };
                json keyboard = createKeyboard(buttons, false);

                std::string msg = "🤖 Вы выбрали противника: ChatGPT\n"
                                "Игра начинается! Удачи!\n\n"
                                "Ваше здоровье: ❤️❤️❤️ (3)\n"
                                "Здоровье врага: 💚💚💚 (3)\n"
                                "Ваша мана: 💙💙💙 (3)\n\n"
                                "Ваш ход! Выберите действие:";
                sendMessageWithKeyboard(user_id, msg, keyboard);

            } else if (command == "friend") {
                std::vector<std::vector<std::pair<std::string, std::string>>> buttons = {
                    {{"◀ Назад", "{\"command\":\"back\"}"}},
                    {{"⚡ Power Up!", "{\"command\":\"power_up\"}"}}
                };
                json keyboard = createKeyboard(buttons, false);

                std::string msg = "👤 Режим игры с другом в разработке.\n"
                                "Попробуйте сыграть с ботом!";
                sendMessageWithKeyboard(user_id, msg, keyboard);

            } else if (command == "back" || command == "menu") {
                std::vector<std::vector<std::pair<std::string, std::string>>> buttons = {
                    {{"🎮 Играть", "{\"command\":\"play\"}"}},
                    {{"📜 Правила", "{\"command\":\"rules\"}"}},
                    {{"⚡ Power Up!", "{\"command\":\"power_up\"}"}}
                };
                json keyboard = createKeyboard(buttons, false);

                std::string msg = "Главное меню. Выберите действие:";
                sendMessageWithKeyboard(user_id, msg, keyboard);

            } else if (command == "shoot_self") {
                std::string msg = "🔫 Вы выстрелили в себя...\n"
                                "БАХ!\n"
                                "Промах! Вам повезло!\n\n"
                                "Ход переходит к противнику...";
                sendMessage(user_id, msg);

                // Возвращаем игровую клавиатуру
                std::vector<std::vector<std::pair<std::string, std::string>>> buttons = {
                    {{"🔫 Выстрелить в себя", "{\"action\":\"shoot_self\"}"}},
                    {{"🎯 Выстрелить в врага", "{\"action\":\"shoot_enemy\"}"}},
                    {{"✨ Магия", "{\"action\":\"magic\"}"}},
                    {{"◀ В меню", "{\"command\":\"menu\"}"}},
                    {{"⚡ Power Up!", "{\"command\":\"power_up\"}"}}
                };
                json keyboard = createKeyboard(buttons, false);
                std::string next_msg = "Ваш ход! Выберите действие:";
                sendMessageWithKeyboard(user_id, next_msg, keyboard);

            } else if (command == "shoot_enemy") {
                std::string msg = "🎯 Вы выстрелили в противника...\n"
                                "БАХ!\n"
                                "Попадание! Противник теряет 1 жизнь!\n\n"
                                "Ход переходит к противнику...";
                sendMessage(user_id, msg);

                // Возвращаем игровую клавиатуру
                std::vector<std::vector<std::pair<std::string, std::string>>> buttons = {
                    {{"🔫 Выстрелить в себя", "{\"action\":\"shoot_self\"}"}},
                    {{"🎯 Выстрелить в врага", "{\"action\":\"shoot_enemy\"}"}},
                    {{"✨ Магия", "{\"action\":\"magic\"}"}},
                    {{"◀ В меню", "{\"command\":\"menu\"}"}},
                    {{"⚡ Power Up!", "{\"command\":\"power_up\"}"}}
                };
                json keyboard = createKeyboard(buttons, false);
                std::string next_msg = "Ваш ход! Выберите действие:";
                sendMessageWithKeyboard(user_id, next_msg, keyboard);

            } else if (command == "magic") {
                std::vector<std::vector<std::pair<std::string, std::string>>> buttons = {
                    {{"💚 Восстановление", "{\"magic\":\"heal\"}"}},
                    {{"💫 Оглушение", "{\"magic\":\"stun\"}"}},
                    {{"◀ Назад", "{\"command\":\"back_game\"}"}},
                    {{"⚡ Power Up!", "{\"command\":\"power_up\"}"}}
                };
                json keyboard = createKeyboard(buttons, true);

                std::string msg = "✨ Выберите заклинание:\n"
                                "💚 Восстановление - +1 здоровье (стоимость: 2 маны)\n"
                                "💫 Оглушение - противник пропускает ход (стоимость: 1 мана)";
                sendMessageWithKeyboard(user_id, msg, keyboard);

            } else if (command == "heal") {
                std::string msg = "💚 Вы использовали восстановление!\n"
                                "Здоровье +1\n"
                                "Мана -2\n\n"
                                "Ход переходит к противнику...";
                sendMessage(user_id, msg);

                // Возвращаем игровую клавиатуру
                std::vector<std::vector<std::pair<std::string, std::string>>> buttons = {
                    {{"🔫 Выстрелить в себя", "{\"action\":\"shoot_self\"}"}},
                    {{"🎯 Выстрелить в врага", "{\"action\":\"shoot_enemy\"}"}},
                    {{"✨ Магия", "{\"action\":\"magic\"}"}},
                    {{"◀ В меню", "{\"command\":\"menu\"}"}},
                    {{"⚡ Power Up!", "{\"command\":\"power_up\"}"}}
                };
                json keyboard = createKeyboard(buttons, false);
                std::string next_msg = "Ваш ход! Выберите действие:";
                sendMessageWithKeyboard(user_id, next_msg, keyboard);

            } else if (command == "stun") {
                std::string msg = "💫 Вы использовали оглушение!\n"
                                "Противник оглушён и пропускает ход!\n"
                                "Мана -1\n\n"
                                "Вы можете сделать ещё один ход!";
                sendMessage(user_id, msg);

                // Возвращаем игровую клавиатуру
                std::vector<std::vector<std::pair<std::string, std::string>>> buttons = {
                    {{"🔫 Выстрелить в себя", "{\"action\":\"shoot_self\"}"}},
                    {{"🎯 Выстрелить в врага", "{\"action\":\"shoot_enemy\"}"}},
                    {{"✨ Магия", "{\"action\":\"magic\"}"}},
                    {{"◀ В меню", "{\"command\":\"menu\"}"}},
                    {{"⚡ Power Up!", "{\"command\":\"power_up\"}"}}
                };
                json keyboard = createKeyboard(buttons, false);
                std::string next_msg = "Ваш ход! Выберите действие:";
                sendMessageWithKeyboard(user_id, next_msg, keyboard);

            } else if (command == "back_game") {
                // Возврат к игре из меню магии
                std::vector<std::vector<std::pair<std::string, std::string>>> buttons = {
                    {{"🔫 Выстрелить в себя", "{\"action\":\"shoot_self\"}"}},
                    {{"🎯 Выстрелить в врага", "{\"action\":\"shoot_enemy\"}"}},
                    {{"✨ Магия", "{\"action\":\"magic\"}"}},
                    {{"◀ В меню", "{\"command\":\"menu\"}"}},
                    {{"⚡ Power Up!", "{\"command\":\"power_up\"}"}}
                };
                json keyboard = createKeyboard(buttons, false);

                std::string msg = "Выберите действие:";
                sendMessageWithKeyboard(user_id, msg, keyboard);

            } else if (command == "power_up") {
                // Реакция на кнопку Power Up и команду /power_up
                std::string gif_path = "gifs/power_up.mp4";
                if (!sendGif(user_id, gif_path, "⚡ Power Up!")) {
                    sendMessage(user_id, "Ошибка отправки гифки power_up!");
                }
            } else {
                std::vector<std::vector<std::pair<std::string, std::string>>> buttons = {
                    {{"🎮 Играть", "{\"command\":\"play\"}"}},
                    {{"📜 Правила", "{\"command\":\"rules\"}"}},
                    {{"⚡ Power Up!", "{\"command\":\"power_up\"}"}}
                };
                json keyboard = createKeyboard(buttons, false);

                std::string echo = "Неизвестная команда. Используйте кнопки для навигации:";
                sendMessageWithKeyboard(user_id, echo, keyboard);
            }

            // Пользовательский обработчик
            if (on_message_callback) {
                on_message_callback(user_id, text);
            }

        } else {
            std::cout << "VK Bot: Ignoring event type: " << event_type << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "VK Bot: Error processing message: " << e.what() << std::endl;
    }
}

// --- Long Poll Loop ---
void VKBot::longPollLoop() {
    std::string lp_response = makeApiRequest("groups.getLongPollServer", "group_id=" + group_id);
    if (lp_response.empty()) {
        std::cerr << "VK Bot: Failed to get Long Poll server" << std::endl;
        return;
    }

    try {
        auto lp_data = json::parse(lp_response);

        if (lp_data.contains("error")) {
            std::cerr << "VK Bot: API Error: " << lp_data["error"]["error_msg"] << std::endl;
            return;
        }

        if (!lp_data.contains("response")) {
            std::cerr << "VK Bot: Invalid Long Poll server response: " << lp_response << std::endl;
            return;
        }

        std::string server = lp_data["response"]["server"];
        std::string key = lp_data["response"]["key"];
        std::string ts;

        if (lp_data["response"]["ts"].is_number()) {
            ts = std::to_string(lp_data["response"]["ts"].get<int64_t>());
        } else if (lp_data["response"]["ts"].is_string()) {
            ts = lp_data["response"]["ts"].get<std::string>();
        } else {
            std::cerr << "VK Bot: Invalid TS format" << std::endl;
            return;
        }

        std::cout << "VK Bot: Long Poll server obtained, starting to listen..." << std::endl;

        while (running) {
            CURL* curl = curl_easy_init();
            if (!curl) {
                std::cerr << "VK Bot: Failed to initialize CURL for Long Poll" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(5));
                continue;
            }

            std::string lp_url = server + "?act=a_check&key=" + key + "&ts=" + ts + "&wait=25";
            std::string response;

            curl_easy_setopt(curl, CURLOPT_URL, lp_url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "VKBot/1.0");

            CURLcode res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            if (res != CURLE_OK) {
                std::cerr << "VK Bot: Long Poll error: " << curl_easy_strerror(res) << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(5));
                continue;
            }

            if (response.empty()) {
                continue;
            }

            try {
                auto poll_result = json::parse(response);

                if (poll_result.contains("failed")) {
                    int fail_code = 0;
                    if (poll_result["failed"].is_number()) {
                        fail_code = poll_result["failed"].get<int>();
                    }
                    std::cout << "VK Bot: Long Poll failed with code " << fail_code << ", refreshing..." << std::endl;

                    lp_response = makeApiRequest("groups.getLongPollServer", "group_id=" + group_id);
                    if (lp_response.empty()) {
                        std::this_thread::sleep_for(std::chrono::seconds(5));
                        continue;
                    }

                    lp_data = json::parse(lp_response);
                    if (!lp_data.contains("response")) {
                        std::this_thread::sleep_for(std::chrono::seconds(5));
                        continue;
                    }

                    server = lp_data["response"]["server"];
                    key = lp_data["response"]["key"];

                    if (lp_data["response"]["ts"].is_number()) {
                        ts = std::to_string(lp_data["response"]["ts"].get<int64_t>());
                    } else {
                        ts = lp_data["response"]["ts"].get<std::string>();
                    }

                    continue;
                }

                if (poll_result.contains("ts")) {
                    if (poll_result["ts"].is_number()) {
                        ts = std::to_string(poll_result["ts"].get<int64_t>());
                    } else if (poll_result["ts"].is_string()) {
                        ts = poll_result["ts"].get<std::string>();
                    }
                }

                if (poll_result.contains("updates") && poll_result["updates"].is_array()) {
                    for (const auto& update : poll_result["updates"]) {
                        if (update.is_array() && update.size() > 0) {
                            json event_obj;
                            event_obj["type"] = update[0];
                            if (update[0] == "message_new" && update.size() > 1) {
                                event_obj["object"] = update[1];
                            } else {
                                event_obj["object"] = update;
                            }
                            processMessage(event_obj);
                        } else if (update.is_object()) {
                            processMessage(update);
                        }
                    }
                }

            } catch (const std::exception& e) {
                std::cerr << "VK Bot: Error parsing Long Poll response: " << e.what() << std::endl;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "VK Bot: Long Poll setup error: " << e.what() << std::endl;
    }
}

void VKBot::start() {
    if (running) {
        std::cout << "VK Bot: Already running" << std::endl;
        return;
    }

    running = true;
    longpoll_thread = new std::thread(&VKBot::longPollLoop, this);
    std::cout << "VK Bot: Started" << std::endl;
}

void VKBot::stop() {
    if (!running) {
        return;
    }

    running = false;
    if (longpoll_thread && longpoll_thread->joinable()) {
        longpoll_thread->join();
        delete longpoll_thread;
        longpoll_thread = nullptr;
    }
    std::cout << "VK Bot: Stopped" << std::endl;
}

void VKBot::setMessageHandler(std::function<void(int64_t, const std::string&)> handler) {
    on_message_callback = handler;
}

bool VKBot::sendMessage(int64_t user_id, const std::string& message) {
    char* escaped_message = curl_easy_escape(nullptr, message.c_str(), message.length());
    std::string params = "user_id=" + std::to_string(user_id) +
                        "&message=" + std::string(escaped_message) +
                        "&random_id=" + std::to_string(
                            std::chrono::system_clock::now().time_since_epoch().count());
    curl_free(escaped_message);

    std::string response = makeApiRequest("messages.send", params);

    if (response.empty()) {
        return false;
    }

    try {
        auto resp_json = json::parse(response);
        if (resp_json.contains("error")) {
            std::cerr << "VK Bot: Error sending message: " << resp_json["error"]["error_msg"] << std::endl;
            return false;
        }
        return resp_json.contains("response");
    } catch (...) {
        return false;
    }
}

bool VKBot::sendMessageWithKeyboard(int64_t user_id, const std::string& message, const json& keyboard) {
    char* escaped_message = curl_easy_escape(nullptr, message.c_str(), message.length());

    std::string keyboard_str = keyboard.dump();
    char* escaped_keyboard = curl_easy_escape(nullptr, keyboard_str.c_str(), keyboard_str.length());

    std::string params = "user_id=" + std::to_string(user_id) +
                        "&message=" + std::string(escaped_message) +
                        "&keyboard=" + std::string(escaped_keyboard) +
                        "&random_id=" + std::to_string(
                            std::chrono::system_clock::now().time_since_epoch().count());

    curl_free(escaped_message);
    curl_free(escaped_keyboard);

    std::string response = makeApiRequest("messages.send", params);

    std::cout << "VK Bot: Send message response: " << response << std::endl;

    if (response.empty()) {
        return false;
    }

    try {
        auto resp_json = json::parse(response);
        if (resp_json.contains("error")) {
            std::cerr << "VK Bot: Error sending message: " << resp_json["error"]["error_msg"] << std::endl;
            return false;
        }
        return resp_json.contains("response");
    } catch (...) {
        return false;
    }
}

// --- GIF/MP4 Sending: VK ---

// Получение upload_url для doc/video_message
std::string VKBot::getUploadUrl(const std::string& type, int64_t peer_id) {
    std::string method, params;
    if (type == "doc") {
        method = "docs.getMessagesUploadServer";
        params = "type=doc&peer_id=" + std::to_string(peer_id);
    } else if (type == "video_message") {
        method = "video.getVideoUploadServer";
        params = "peer_id=" + std::to_string(peer_id);
    } else {
        return "";
    }
    std::string resp = makeApiRequest(method, params);
    try {
        auto json_resp = json::parse(resp);
        if (json_resp.contains("response") && json_resp["response"].contains("upload_url")) {
            return json_resp["response"]["upload_url"];
        }
    } catch (...) {}
    return "";
}

// Используем CURL для загрузки файла (multipart/form-data); универсально для doc и т.д.
std::string VKBot::uploadFile(const std::string& upload_url, const std::string& file_path, const std::string& field_name) {
    CURL* curl = curl_easy_init();
    std::string response;
    if (!curl) {
        std::cerr << "uploadFile: Failed to initialize curl" << std::endl;
        return "";
    }

    curl_mime *mime;
    curl_mimepart *part;
    mime = curl_mime_init(curl);

    part = curl_mime_addpart(mime);
    curl_mime_name(part, field_name.c_str());
    curl_mime_filedata(part, file_path.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, upload_url.c_str());
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    CURLcode res = curl_easy_perform(curl);

    curl_mime_free(mime);
    curl_easy_cleanup(curl);

    if(res != CURLE_OK) {
        std::cerr << "uploadFile: curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        return "";
    }

    return response;
}

json VKBot::saveUploadedFile(const std::string& type, const std::string& upload_result, int64_t peer_id) {
    try {
        std::cout << "--- upload_result: " << upload_result << std::endl; // добавь отладку!
        auto j = json::parse(upload_result);
        if (type == "doc" && j.contains("file")) {
            std::string file = j["file"];
            std::string params = "file=" + file;
            std::cout << "--- docs.save params: " << params << std::endl;
            std::string resp = makeApiRequest("docs.save", params);
            std::cout << "--- docs.save response: " << resp << std::endl;
            return json::parse(resp);
        }
    } catch (...) {
        std::cerr << "Exception in saveUploadedFile!" << std::endl;
        return {};
    }
    return {};
}

bool VKBot::sendGif(int64_t user_id, const std::string& file_path, const std::string& caption) {
    // 1. Получаем upload_url
    std::string upload_url = getUploadUrl("doc", user_id);
    if (upload_url.empty()) {
        std::cerr << "sendGif: Could not get upload_url" << std::endl;
        return false;
    }
    // 2. Заливаем файл
    std::string upload_resp = uploadFile(upload_url, file_path, "file");
    if (upload_resp.empty()) {
        std::cerr << "sendGif: Could not upload file" << std::endl;
        return false;
    }
    // 3. Сохраняем на стороне VK
    json doc_info = saveUploadedFile("doc", upload_resp, user_id);
    if (doc_info.empty() || !doc_info.contains("response") || !doc_info["response"].is_array()) {
        std::cerr << "sendGif: Could not save doc" << std::endl;
        return false;
    }
    auto doc = doc_info["response"][0];
    std::string attachment = "doc" + std::to_string(doc["owner_id"].get<int>()) + "_" + std::to_string(doc["id"].get<int>());
    if (doc.contains("access_key")) {
        attachment += "_" + doc["access_key"].get<std::string>();
    }
    std::string params = "user_id=" + std::to_string(user_id) +
                         "&attachment=" + attachment +
                         "&random_id=" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    if (!caption.empty()) {
        char* escaped_caption = curl_easy_escape(nullptr, caption.c_str(), caption.length());
        params += "&message=" + std::string(escaped_caption);
        curl_free(escaped_caption);
    }
    std::string resp = makeApiRequest("messages.send", params);
    std::cout << "sendGif: response: " << resp << std::endl;
    try {
        auto check = json::parse(resp);
        if (check.contains("response")) return true;
    } catch (...) {}
    return false;
}

// Документы (файлы любых типов) - не используется для power_up, но реализовано для расширения
bool VKBot::sendDocument(int64_t user_id, const std::string& file_path, const std::string& caption) {
    return sendGif(user_id, file_path, caption); // для гиф/видео аналогично gif
}

// --- TESTER ---
int test_vk_bot() {
    std::cout << "=== VK Bot Test for Private Group ===" << std::endl;
    std::string access_token = "vk1.a.OcICgTI1FWXikQLIefZ0ldSKcjLc5OrqWls0QUVhLk1z6ZosdCXL5n20tRpxoIn-BqW_27WToFGSLu1Kpe4irIY7cZiIQGZk0YsjSAd7sbYreLEAJIwq8hNbh2VJ9G-dDdByVXKBlWsHwwJNhEaYUUwzVL956ZPXAati2BYS2LQQrMw2N7WAUyndBvuaBL_iiNG2AIZgN7eRg6YLuuT3FA";
    std::string group_id = "237882937";

    if (access_token == "YOUR_VK_GROUP_ACCESS_TOKEN_HERE" || group_id == "YOUR_GROUP_ID_HERE") {
        std::cout << "Please set your VK access_token and group_id in the code!" << std::endl;
        return 1;
    }

    VKBot bot(access_token, group_id);

    std::cout << "\n1. Getting group info..." << std::endl;
    bot.getGroupInfo();
    std::cout << "Group info received" << std::endl;

    std::cout << "\n2. Enabling Long Poll for private group..." << std::endl;
    bot.enableLongPoll();

    std::cout << "\n3. Checking Long Poll settings..." << std::endl;
    bot.getLongPollSettings();
    std::cout << "Long Poll is configured" << std::endl;

    bot.setMessageHandler([](int64_t user_id, const std::string& message) {
        std::cout << "Custom handler: User " << user_id << " sent: " << message << std::endl;
    });

    std::cout << "\n4. Starting bot..." << std::endl;
    bot.start();

    std::cout << "\n✅ VK Bot is running with KEYBOARD & GIF support!" << std::endl;
    std::cout << "Send 'начать', /power_up или нажмите на Power Up! в меню." << std::endl;
    std::cout << "Press Enter to stop the bot..." << std::endl;
    std::cin.get();

    bot.stop();
    std::cout << "Bot stopped. Goodbye!" << std::endl;

    return 0;
}