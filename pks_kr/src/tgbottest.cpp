#include <iostream>
// #include <string>
// #include <thread>
// #include <chrono>
// #include <set>
#include <curl/curl.h>
// #include "json.hpp"
//#include <stdio.h>
//#include <tgbot/tgbot.h>

//using json = nlohmann::json;

// Обработка получения данных curl
//size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
//    ((std::string*)userp)->append((char*)contents, size * nmemb);
//    return size * nmemb;
//}

// Функция отправки сообщения пользователю
//void sendMessage(const std::string& token, int64_t chat_id, const std::string& text) {
//    CURL* curl = curl_easy_init();
//    if (curl) {
//        std::string url = "https://api.telegram.org/bot" + token + "/sendMessage";
//        std::string postFields = "chat_id=" + std::to_string(chat_id) + "&text=" + curl_easy_escape(curl, text.c_str(), 0);
//
//        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
//        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
//
//        curl_easy_perform(curl);
//        curl_easy_cleanup(curl);
//    }
//}

int main() {
    //TgBot::Bot bot("7928899642:AAGmfMYq3LmjwgySM9lx0heY51Ov6sbscko");
    std::cout << "Hello World!\n";
    return 0;
}