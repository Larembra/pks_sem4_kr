#include <iostream>
#include "curl/curl.h"
#include <string>

std::string apiKey = "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";
std::string siteUrl = "<YOUR_SITE_URL>";
std::string siteName = "<YOUR_SITE_NAME>";
std::string model = "deepseek/deepseek-r1-distill-llama-70b:free";
std::string content = "What is the meaning of life?";
std::string responseStr;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int main() {
    CURL* curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl) {
        std::string url = "https://openrouter.ai/api/v1/chat/completions";
        std::string jsonPayload = "{\"model\": \"" + model + "\", \"messages\": [{\"role\": \"user\", \"content\": \"" + content + "\"}]}";

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + apiKey).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, ("HTTP-Referer: " + siteUrl).c_str());
        headers = curl_slist_append(headers, ("X-Title: " + siteName).c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStr);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            std::cout << "Response: " << responseStr << std::endl;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return 0;
}
