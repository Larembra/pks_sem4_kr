#include "../include/contestants/nn.h"
#include "../include/weapons/revolver.h"
#include <iostream>

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include "curl/curl.h"
#include "../include/json/json.hpp"
#include "../include/API/telegram.h"
#include "../include/secret/dotenv.h"


using json = nlohmann::json;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total_size = size * nmemb;
    output->append((char*)contents, total_size);
    return total_size;
}

bool nn::move(const int64_t chat_id, contestant& enemy, revolver& gun, std::string neuro) {
    if (mp < max_mp) {
        mp++;
    }
    std::string msg = format_status(enemy,gun);
    sendTelegramMessage(chat_id, msg);

    // std::cout << "your hp: " << enemy.get_hp() << std::endl;
    // std::cout << "your mp: " << enemy.get_mp() << std::endl;
    // std::cout << std::endl;
    // std::cout << "enemy hp: " << hp << std::endl;
    // std::cout << "enemy mp: " << mp << std::endl;
    // std::cout << std::endl;
    // std::cout << "bullets: " << gun.num_bullets() << std::endl;
    // std::cout << "chambers: " << gun.num_chambers() << std::endl;
    std::vector <std::string> acts = get_response(enemy, gun, neuro);
    for (auto a : acts) {
        std::cout << a << std::endl;
    }
    std::cout << std::endl;
    int damage = 1;
    // std::string act = "";
    // std::cin >> act;


    // acts = {"heal"};//!!!!!!!!!!!!!!!!!!!!!!!!
    // hp = 2;

    for (auto act : acts) {
        if (act == "Power up") {
            if (mp >= 3) {
                mp -= 3;
                damage = 2;
                sendTelegramMessage(chat_id, "💀 Enemy powered up 💪");
                //sendTelegramMp4Animation(chat_id, "power_up.mp4");
                //sendTelegramMessage(chat_id, "Enemy mp: "+std::to_string(mp));
                // std::cout << "enemy used Power up" << std::endl;
                // std::cout << "enemy mp: " << mp << std::endl;
            }
            else {
                // std::cout << "not enough mana" << std::endl;
                // std::cout << "your mp: " << mp << ", need 3" << std::endl;
            }
        }
        else if (act == "Stun") {
            if (mp >= 5) {
                mp -= 5;
                enemy.stun();
                sendTelegramMessage(chat_id, "💀 Enemy stunned you for 1 turn 💖💫");
                //sendTelegramMp4Animation(chat_id, "stun.mp4");
                //sendTelegramMessage(chat_id, "enemy mp: "+std::to_string(mp));
                // std::cout << "enemy used stun" << std::endl;
                // std::cout << "enemy mp: " << mp << std::endl;
            }
            else {
                // std::cout << "not enough mana" << std::endl;
                // std::cout << "your mp: " << mp << ", need 5" << std::endl;
            }
        }
        else if (act == "Heal") {
            if (mp >= 3 and hp < max_hp) {
                mp -= 3;
                hp++;
                sendTelegramMessage(chat_id, "💀 Enemy recovered 1 HP ❤️‍🩹");
                //sendTelegramMp4Animation(chat_id, "heal.mp4");
                //sendTelegramMessage(chat_id, "enemy mp: "+std::to_string(mp));
                //sendTelegramMessage(chat_id, "enemy hp: "+std::to_string(hp));
                // std::cout << "enemy used heal" << std::endl;
                // std::cout << "enemy mp: " << mp << std::endl;
                // std::cout << "enemy hp: " << hp << std::endl;
            }
            else if (hp == max_hp) {
                // std::cout << "you already have max health" << std::endl;
            }
            else {
                // std::cout << "not enough mana" << std::endl;
                // std::cout << "your mp: " << mp << ", need 3" << std::endl;
            }
        }
        else if (act == "Swap bullet") {
            if (mp >= 3) {
                mp -= 3;
                gun.swap_bullet();
                sendTelegramMessage(chat_id, "💀 Enemy swapped bullet 🔄");
                //sendTelegramMp4Animation(chat_id, "swap_bullet.mp4");
                //sendTelegramMessage(chat_id, "enemy mp: "+std::to_string(mp));
                // std::cout << "enemy used swap bullet" << std::endl;
                // std::cout << "enemy mp: " << mp << std::endl;
            }
            else {
                // std::cout << "not enough mana" << std::endl;
                // std::cout << "your mp: " << mp << ", need 3" << std::endl;
            }
        }
        else if (act == "Fireball") {
            if (mp >= 5) {
                mp -= 5;
                enemy.set_hp(enemy.get_hp() - 1);
                if (enemy.get_hp() == 0) {
                    return 1;
                }
                sendTelegramMessage(chat_id, "💀 Enemy casted fireball 💖💥");
                //sendTelegramMp4Animation(chat_id, "fireball.mp4");
                //sendTelegramMessage(chat_id, "enemy mp: "+std::to_string(mp));
                //sendTelegramMessage(chat_id, "your hp: "+std::to_string(enemy.get_hp()));
                // std::cout << "enemy used fireball" << std::endl;
                // std::cout << "enemy mp: " << mp << std::endl;
                // std::cout << "your hp: " << enemy.get_hp() << std::endl;
            }
            else {
                // std::cout << "not enough mana" << std::endl;
                // std::cout << "your mp: " << mp << ", need 5" << std::endl;
            }
        }
        else if (act == "Blood ritual") {
            if (hp > 1) {
                hp--;
                if (mp+2 > max_mp) {
                    mp = max_mp;
                }
                else {
                    mp+=2;
                }
                sendTelegramMessage(chat_id, "💀 Enemy performed a blood ritual 🩸➡️🔮");
                //sendTelegramMp4Animation(chat_id, "blood_ritual.mp4");
                //sendTelegramMessage(chat_id, "enemy mp: "+std::to_string(mp));
                //sendTelegramMessage(chat_id, "enemy hp: "+std::to_string(hp));
                // std::cout << "enemy used bloodritual" << std::endl;
                // std::cout << "enemy mp: " << mp << std::endl;
                // std::cout << "enemy hp: " << hp << std::endl;
            }
            else {
                // std::cout << "not enough health" << std::endl;
                // std::cout << "your hp: " << hp << std::endl;
            }
        }
        else if (act == "Shoot enemy") {
            sendTelegramMessage(chat_id, "💀 Enemy shot you!");
            //std::cout << "enemy shot you!" << std::endl;
            if (gun.shoot(chat_id)) {
                //sendTelegramMp4Animation(chat_id, "shot_bullet.mp4");
                enemy.set_hp(enemy.get_hp() - damage);
            }
            else {
                //sendTelegramMp4Animation(chat_id, "shot_no_bullet.mp4");
            }
            sendTelegramMessage(chat_id, "💖 Your HP: "+std::to_string(enemy.get_hp()));

            //std::cout << "your hp: " << enemy.get_hp() << std::endl;
            return 0;
        }
        else if (act == "Shoot myself") {
            //sendTelegramMp4Animation(chat_id, "shoot_myself.mp4");
            sendTelegramMessage(chat_id, "💀 Enemy shot himself!");
            //std::cout << "enemy shot himself!" << std::endl;
            if (!gun.shoot(chat_id)) {
                return 1;
            }
            hp-=damage;
            sendTelegramMessage(chat_id, "💀 Enemy hp: "+std::to_string(hp));
            //std::cout << "enemy hp: " << hp << std::endl;
            return 0;
        }
        else {
            sendTelegramMessage(chat_id, "❓Unknown action!❓");
            //std::cout << "unknown action" << std::endl;
        }
        //std::cin >> act;
    }
    sendTelegramMessage(chat_id, "⚠️Error, check your internet connection and try again.⚠️");
    //std::cout << "ERROR!!!!!!!!"<< std::endl;
    return 0;

}

const std::string nn_PROMPT_FMT = R"(You are participating in Magic Revolver Duel - a variation of russian roulette with magical rules.
Here are the conditions: you and your opponent have 3 HP each (maximum 3) and 3 MP each (maximum 10, recovers 1 MP per turn).
A 6-chamber revolver is used with a random number of bullets (from 1 to 5).
Available spells: 'Power up' (3 MP, next shot deals double damage - 2 HP), 'Stun' (5 MP, opponent skips a turn), 'Heal' (3 MP, +1 HP to your health, but not above maximum),
'Swap bullet' (3 MP, if there was a bullet in chamber, bullet disappears, if wasn't, bullet appears), 'Fireball' (5 MP, deals 1 HP damage to the enemy), 'Blood ritual' (0 MP, you lose 1 HP,
but instantly gain 2 MP that can be used this turn).
All your response MUST BE STRICTLY FORMATTED: first list the used spells, EACH ON A NEW LINE (if you have enough mana, you can use one or more unique or identical spells, as well as no spells per turn),
DO NOT use any SPACES or TABS after spell name before changing line, then ON A NEW LINE always write 'Shoot myself' (gives an extra turn if there wasn't a bullet in the chamber) or 'Shoot enemy' (deals 1 HP damage if bullet).
You cannot use spells you don't have enough mana for.

Example of your response:
Fireball
Heal
Shoot enemy

Current status: Your HP: {MY_HP} | MP: {MY_MP}
Opponent HP: {ENEMY_HP} | MP: {ENEMY_MP}
Remaining chambers: {CHAMBERS}
Remaining bullets: {BULLETS}
FOLLOW THE FORMAT, DON'T ADD EXPLANATIONS. DON'T REASON LONG, YOU HAVE TO GIVE ANSWER ALMOST INSTANTLY. If HP drops to 0 - game over. Let's begin!)";

std::string nn::format_prompt(contestant& enemy, revolver& gun) {
    std::ostringstream oss;
    std::string prompt = nn_PROMPT_FMT;

    // Заменяем плейсхолдеры на фактические значения
    size_t pos;
    while ((pos = prompt.find("{MY_HP}")) != std::string::npos)
        prompt.replace(pos, 7, std::to_string(hp));
    while ((pos = prompt.find("{MY_MP}")) != std::string::npos)
        prompt.replace(pos, 7, std::to_string(mp));
    while ((pos = prompt.find("{ENEMY_HP}")) != std::string::npos)
        prompt.replace(pos, 10, std::to_string(enemy.get_hp()));
    while ((pos = prompt.find("{ENEMY_MP}")) != std::string::npos)
        prompt.replace(pos, 10, std::to_string(enemy.get_mp()));
    while ((pos = prompt.find("{CHAMBERS}")) != std::string::npos)
        prompt.replace(pos, 10, std::to_string(gun.num_chambers()));
    while ((pos = prompt.find("{BULLETS}")) != std::string::npos)
        prompt.replace(pos, 9, std::to_string(gun.num_bullets()));

    return prompt;
}

std::string nn::format_status(contestant& enemy, revolver& gun) {
    //std::ostringstream oss;
    std::string msg = R"(❗️❗️❗️STATUS❗️❗️❗️
💖 Your HP: {MY_HP} | 🔮 MP: {MY_MP}
💀 Opponent HP: {ENEMY_HP} | 🔮 MP: {ENEMY_MP}
❗️❗️❗️STATUS❗️❗️❗️)";

    // Заменяем плейсхолдеры на фактические значения
    size_t pos;
    while ((pos = msg.find("{MY_HP}")) != std::string::npos)
        msg.replace(pos, 7, std::to_string(enemy.get_hp()));
    while ((pos = msg.find("{MY_MP}")) != std::string::npos)
        msg.replace(pos, 7, std::to_string(enemy.get_mp()));
    while ((pos = msg.find("{ENEMY_HP}")) != std::string::npos)
        msg.replace(pos, 10, std::to_string(hp));
    while ((pos = msg.find("{ENEMY_MP}")) != std::string::npos)
        msg.replace(pos, 10, std::to_string(mp));
    // while ((pos = msg.find("{CHAMBERS}")) != std::string::npos)
    //     msg.replace(pos, 10, std::to_string(gun.num_chambers()));
    // while ((pos = msg.find("{BULLETS}")) != std::string::npos)
    //     msg.replace(pos, 9, std::to_string(gun.num_bullets()));

    return msg;
}

// std::vector<std::string> nn::get_response(contestant& enemy, revolver& gun, std::string neuro) {
//     std::string githubToken, model;
//     if (neuro == "DeepSeek-V3-0324") {
//         githubToken = "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";
//         model = "deepseek/DeepSeek-V3-0324";
//     }
//     if (neuro == "gpt-4.1-mini") {
//         githubToken = "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";
//         model = "openai/gpt-4.1-mini";
//     }
//     if (neuro == "Ministral-3B") {
//         githubToken = "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";
//         model = "mistral-ai/Ministral-3B";
//     }
//     // std::string githubToken = "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";
//     // std::string model = "deepseek/DeepSeek-V3-0324";
//     std::string userMessage = format_prompt(enemy, gun);
//
//     std::vector<std::string> actions;
//
//     curl_global_init(CURL_GLOBAL_ALL);
//     CURL* curl = curl_easy_init();
//     if (!curl) {
//         std::cerr << "Failed to initialize curl!" << std::endl;
//         return actions;
//     }
//
//     std::string url = "https://models.github.ai/inference/chat/completions";
//
//     // Create a JSON object directly to handle escaping properly
//     json jsonPayload;
//     jsonPayload["model"] = model;
//     jsonPayload["max_tokens"] = 2048*4;
//
//     json messageObj;
//     messageObj["role"] = "user";
//     messageObj["content"] = userMessage;
//
//     json messagesArray = json::array();
//     messagesArray.push_back(messageObj);
//
//     jsonPayload["messages"] = messagesArray;
//
//     std::string payloadStr = jsonPayload.dump();
//
//     std::string response;
//
//     struct curl_slist* headers = nullptr;
//     headers = curl_slist_append(headers, ("Authorization: Bearer " + githubToken).c_str());
//     headers = curl_slist_append(headers, "Content-Type: application/json");
//
//     curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
//     curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
//     curl_easy_setopt(curl, CURLOPT_POST, 1L);
//     curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
//     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
//     curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
//
//     CURLcode res = curl_easy_perform(curl);
//     if (res != CURLE_OK) {
//         std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
//     } else {
//         try {
//             json jsonResponse = json::parse(response);
//             std::string content = jsonResponse["choices"][0]["message"]["content"];
//
//             //std::cout << "Full nn R1 Response: \n" << content << std::endl;
//
//             // Look for the </think> tag
//             std::string thinkTag = "</think>";
//             size_t thinkTagPos = content.find(thinkTag);
//
//             if (thinkTagPos != std::string::npos) {
//                 // Start processing content after the </think> tag
//                 std::string filteredContent = content.substr(thinkTagPos + thinkTag.length());
//                 //std::cout << "\nFiltered content (after </think>): \n" << filteredContent << std::endl;
//
//                 // Split filtered content into lines and add to actions
//                 size_t start = 0, end = 0;
//                 while ((end = filteredContent.find('\n', start)) != std::string::npos) {
//                     std::string line = filteredContent.substr(start, end - start);
//                     if (!line.empty()) actions.push_back(line);
//                     start = end + 1;
//                 }
//                 // Add the last line if not empty
//                 if (start < filteredContent.size()) {
//                     std::string line = filteredContent.substr(start);
//                     if (!line.empty()) actions.push_back(line);
//                 }
//             } else {
//                 std::cout << "Warning: </think> tag not found in response. Processing full response." << std::endl;
//
//                 // If </think> tag is not found, process the entire content
//                 size_t start = 0, end = 0;
//                 while ((end = content.find('\n', start)) != std::string::npos) {
//                     std::string line = content.substr(start, end - start);
//                     if (!line.empty()) actions.push_back(line);
//                     start = end + 1;
//                 }
//                 // Add the last line if not empty
//                 if (start < content.size()) {
//                     std::string line = content.substr(start);
//                     if (!line.empty()) actions.push_back(line);
//                 }
//             }
//         } catch (const std::exception& e) {
//             std::cerr << "Error parsing JSON: " << e.what() << std::endl;
//             std::cerr << "Raw response: " << response << std::endl;
//         }
//     }
//
//     curl_slist_free_all(headers);
//     curl_easy_cleanup(curl);
//     curl_global_cleanup();
//
//     return actions;
// }

std::vector<std::string> nn::get_response(contestant& enemy, revolver& gun, std::string neuro) {
    std::string githubToken = getDotenvValue(loadDotenv(".env"), "GITHUB_MODELS_TOKEN"), model;
    if (neuro == "DeepSeek-V3-0324") {

        model = "deepseek/DeepSeek-V3-0324";
    }
    if (neuro == "GPT-4.1-mini") {

        model = "openai/gpt-4.1-mini";
    }
    if (neuro == "Ministral-3B") {

        model = "mistral-ai/Ministral-3B";
    }

    std::string userMessage = format_prompt(enemy, gun);
    std::vector<std::string> actions;

    curl_global_init(CURL_GLOBAL_ALL);
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize curl!" << std::endl;
        return actions;
    }

    std::string url = "https://models.github.ai/inference/chat/completions";

    // Create a JSON object directly to handle escaping properly
    json jsonPayload;
    jsonPayload["model"] = model;
    jsonPayload["max_tokens"] = 2048*4;

    json messageObj;
    messageObj["role"] = "user";
    messageObj["content"] = userMessage;

    json messagesArray = json::array();
    messagesArray.push_back(messageObj);

    jsonPayload["messages"] = messagesArray;

    std::string payloadStr = jsonPayload.dump();
    std::string response;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + githubToken).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
    } else {
        try {
            // Check if the response is not empty and starts with {
            if (!response.empty() && response[0] == '{') {
                json jsonResponse = json::parse(response);
                std::string content;

                // Handle different response formats based on the model
                if (neuro == "gpt-4.1-mini") {
                    // Special handling for ChatGPT response format
                    // Check different possible JSON paths for the content
                    if (jsonResponse.contains("choices") &&
                        !jsonResponse["choices"].empty() &&
                        jsonResponse["choices"][0].contains("message") &&
                        jsonResponse["choices"][0]["message"].contains("content")) {
                        content = jsonResponse["choices"][0]["message"]["content"];
                    } else if (jsonResponse.contains("response")) {
                        // Alternative path that might be used by OpenAI models
                        content = jsonResponse["response"].get<std::string>();
                    } else {
                        // Last resort - dump the entire response for debugging
                        std::cerr << "Unexpected JSON structure from ChatGPT: " << response << std::endl;
                        content = response; // Just use the raw response
                    }
                } else {
                    // Default path for DeepSeek and Ministral
                    content = jsonResponse["choices"][0]["message"]["content"];
                }

                // Common processing for all models
                std::string thinkTag = "</think>";
                size_t thinkTagPos = content.find(thinkTag);

                if (thinkTagPos != std::string::npos) {
                    // Start processing content after the </think> tag
                    std::string filteredContent = content.substr(thinkTagPos + thinkTag.length());

                    // Split filtered content into lines and add to actions
                    size_t start = 0, end = 0;
                    while ((end = filteredContent.find('\n', start)) != std::string::npos) {
                        std::string line = filteredContent.substr(start, end - start);
                        if (!line.empty()) actions.push_back(line);
                        start = end + 1;
                    }
                    // Add the last line if not empty
                    if (start < filteredContent.size()) {
                        std::string line = filteredContent.substr(start);
                        if (!line.empty()) actions.push_back(line);
                    }
                } else {
                    std::cout << "Warning: </think> tag not found in response. Processing full response." << std::endl;

                    // If </think> tag is not found, process the entire content
                    size_t start = 0, end = 0;
                    while ((end = content.find('\n', start)) != std::string::npos) {
                        std::string line = content.substr(start, end - start);
                        if (!line.empty()) actions.push_back(line);
                        start = end + 1;
                    }
                    // Add the last line if not empty
                    if (start < content.size()) {
                        std::string line = content.substr(start);
                        if (!line.empty()) actions.push_back(line);
                    }
                }
            } else {
                // Handle non-JSON responses (like "Unauthorized")
                std::cerr << "Non-JSON response received: " << response << std::endl;

                // Check for specific error messages and handle accordingly
                if (response.find("Unauthorized") != std::string::npos) {
                    std::cerr << "Authentication failed with token for model: " << model << std::endl;
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            std::cerr << "Raw response: " << response << std::endl;
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return actions;
}