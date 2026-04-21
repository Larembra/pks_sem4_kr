#include "telegram.h"
#include <curl/curl.h>
#include "json.hpp"
#include <thread>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

using json = nlohmann::json;

const std::string token = "7928899642:AAH6uR3pM54MJL9oZMAwzsjKqUrFF0VZtpA";
int64_t chat_id;

size_t writecallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int64_t getChatId() {
    int64_t last_update_id = 0;
    while (true) {
        CURL* curl = curl_easy_init();
        std::string readBuffer;
        if (curl) {
            std::string url = "https://api.telegram.org/bot" + token + "/getUpdates?timeout=10";
            if (last_update_id != 0) {
                url += "&offset=" + std::to_string(last_update_id + 1);
            }
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writecallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            CURLcode res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            if (res != CURLE_OK) {
                std::cerr << "getChatId: curl error: " << curl_easy_strerror(res) << std::endl;
            } else {
                if (readBuffer.empty()) {
                    std::cerr << "getChatId: empty response from Telegram" << std::endl;
                } else {
                    try {
                        auto j = json::parse(readBuffer);
                        if (j.contains("result") && j["result"].is_array()) {
                            for (const auto& update : j["result"]) {
                                if (update.contains("update_id")) {
                                    last_update_id = update["update_id"];
                                }
                                if (update.contains("message") &&
                                    update["message"].contains("text") &&
                                    update["message"]["text"] == "/start" &&
                                    update["message"].contains("chat") &&
                                    update["message"]["chat"].contains("id")) {
                                    return update["message"]["chat"]["id"].get<int64_t>();
                                }
                            }
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "getChatId: JSON parse error: " << e.what()
                                  << "\nResponse was: " << readBuffer << std::endl;
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void sendTelegramMessage(const int64_t chat_id, const std::string& text) {
    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = "https://api.telegram.org/bot" + token + "/sendMessage";

        std::string postFields = "chat_id=" + std::to_string(chat_id)
            + "&text=" + curl_easy_escape(curl, text.c_str(), 0)
            + "&parse_mode=HTML";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());

        std::string readBuffer;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writecallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "sendTelegramMessage: curl error: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }
}

void sendTelegramMessageWithKeyboard(
    const int64_t chat_id,
    const std::string& text,
    const std::vector<std::vector<std::string>>& button_rows
) {
    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = "https://api.telegram.org/bot" + token + "/sendMessage";

        json keyboard = {
            {"keyboard", button_rows},
            {"one_time_keyboard", true},
            {"resize_keyboard", true}
        };
        std::string keyboardStr = keyboard.dump();

        std::string postFields = "chat_id=" + std::to_string(chat_id) +
            "&text=" + curl_easy_escape(curl, text.c_str(), 0) +
            "&parse_mode=HTML" +
            "&reply_markup=" + curl_easy_escape(curl, keyboardStr.c_str(), 0);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());

        std::string readBuffer;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writecallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "sendTelegramMessageWithKeyboard: curl error: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }
}

void sendTelegramMp4Animation(const int64_t chat_id, const std::string& mp4_path) {
    CURL* curl = curl_easy_init();
    if (curl) {
        struct curl_httppost* form = nullptr;
        struct curl_httppost* last = nullptr;

        std::ifstream file(mp4_path, std::ios::binary);
        if (!file.is_open()) {
            sendTelegramMessage(chat_id, "Error: file mp4 not found!");
            curl_easy_cleanup(curl);
            return;
        }
        if (mp4_path.size() < 4 || mp4_path.substr(mp4_path.size() - 4) != ".mp4") {
            sendTelegramMessage(chat_id, "Error: file isn't mp4!");
            curl_easy_cleanup(curl);
            return;
        }

        curl_formadd(&form, &last,
            CURLFORM_COPYNAME, "chat_id",
            CURLFORM_COPYCONTENTS, std::to_string(chat_id).c_str(),
            CURLFORM_END);

        curl_formadd(&form, &last,
            CURLFORM_COPYNAME, "animation",
            CURLFORM_FILE, mp4_path.c_str(),
            CURLFORM_END);

        std::string url = "https://api.telegram.org/bot" + token + "/sendAnimation";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, form);

        std::string readBuffer;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writecallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "sendTelegramMp4Animation: curl error: " << curl_easy_strerror(res) << std::endl;
        }

        curl_formfree(form);
        curl_easy_cleanup(curl);
    }
}

int64_t getLastUpdateId() {
    CURL* curl = curl_easy_init();
    std::string readBuffer;
    int64_t last_update_id = 0;

    if (curl) {
        std::string url = "https://api.telegram.org/bot" + token + "/getUpdates?timeout=1";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writecallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "getLastUpdateId: curl error: " << curl_easy_strerror(res) << std::endl;
            return 0;
        }

        if (readBuffer.empty()) {
            std::cerr << "getLastUpdateId: empty response from Telegram" << std::endl;
            return 0;
        }

        try {
            auto j = json::parse(readBuffer);
            if (j.contains("result") && j["result"].is_array()) {
                for (const auto& update : j["result"]) {
                    if (update.contains("update_id") &&
                        update["update_id"].get<int64_t>() > last_update_id) {
                        last_update_id = update["update_id"].get<int64_t>();
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "getLastUpdateId: JSON parse error: " << e.what()
                      << "\nResponse was: " << readBuffer << std::endl;
        }
    }

    return last_update_id;
}

std::string waitForTelegramInput(const int64_t chat_id) {
    int64_t last_update_id = getLastUpdateId();
    while (true) {
        CURL* curl = curl_easy_init();
        std::string readBuffer;
        if (curl) {
            std::string url = "https://api.telegram.org/bot" + token + "/getUpdates?timeout=10";
            if (last_update_id != 0) {
                url += "&offset=" + std::to_string(last_update_id + 1);
            }
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writecallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            CURLcode res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            if (res != CURLE_OK) {
                std::cerr << "waitForTelegramInput: curl error: " << curl_easy_strerror(res) << std::endl;
            } else {
                if (readBuffer.empty()) {
                    std::cerr << "waitForTelegramInput: empty response from Telegram" << std::endl;
                } else {
                    try {
                        auto j = json::parse(readBuffer);
                        if (j.contains("result") && j["result"].is_array()) {
                            for (const auto& update : j["result"]) {
                                if (update.contains("update_id")) {
                                    last_update_id = update["update_id"];
                                }
                                if (update.contains("message") &&
                                    update["message"].contains("chat") &&
                                    update["message"]["chat"]["id"] == chat_id &&
                                    update["message"].contains("text")) {
                                    return update["message"]["text"].get<std::string>();
                                }
                            }
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "waitForTelegramInput: JSON parse error: " << e.what()
                                  << "\nResponse was: " << readBuffer << std::endl;
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void setBotCommands() {
    CURL* curl = curl_easy_init();
    if (curl) {
        json commands_json = json::array({
            {
                {"command", "start"},
                {"description", "Start a new game"}
            }
        });

        std::string commandsStr = commands_json.dump();

        std::string url = "https://api.telegram.org/bot" + token + "/setMyCommands";

        char* escaped_commands = curl_easy_escape(curl, commandsStr.c_str(), 0);
        std::string postFields = "commands=";
        postFields += escaped_commands;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());

        std::string readBuffer;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writecallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);

        curl_free(escaped_commands);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "Failed to set bot commands: " << curl_easy_strerror(res) << std::endl;
        } else {
            if (readBuffer.empty()) {
                std::cerr << "setBotCommands: empty response from Telegram" << std::endl;
            } else {
                std::cerr << "setBotCommands response: " << readBuffer << std::endl;
            }
        }
    }
}