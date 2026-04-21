#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>
#include <curl/curl.h>
#include "json.hpp"

// Helper to read the response into a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::string* s = (std::string*)userp;
    size_t newLength = size * nmemb;
    s->append((char*)contents, newLength);
    return newLength;
}

// Split string by newline
std::vector<std::string> split_by_newline(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream iss(text);
    std::string line;
    while (std::getline(iss, line)) {
        lines.push_back(line);
    }
    return lines;
}

int main() {
    // Get your GitHub PAT token from environment variable
    const char* token = std::getenv("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");
    if (!token) {
        std::cerr << "GITHUB_TOKEN environment variable not set." << std::endl;
        return 1;
    }

    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = "https://models.github.ai/inference/chat/completions";
        std::string readBuffer;
        std::stringstream authHeader;
        authHeader << "Authorization: Bearer " << token;
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, authHeader.str().c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Accept: application/json");

        // Prepare the JSON body
        std::string jsonBody = R"(
{
  "messages": [
    { "role": "user", "content": "Can you explain the basics of machine learning?" }
  ],
  "model": "deepseek/DeepSeek-R1",
  "max_tokens": 2048
}
)";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBody.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: "
                      << curl_easy_strerror(res) << std::endl;
        } else {
            // Parse JSON and extract answer
            try {
                auto json = nlohmann::json::parse(readBuffer);
                std::string ai_answer = json["choices"][0]["message"]["content"];
                // Split answer by lines
                std::vector<std::string> lines = split_by_newline(ai_answer);
                // Output as array
                std::cout << "AI answer" << std::endl;
                std::cout << "[";
                for (size_t i = 0; i < lines.size(); ++i) {
                    std::cout << "\"" << lines[i] << "\"";
                    if (i + 1 < lines.size()) std::cout << ", ";
                }
                std::cout << "]" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "JSON parse error: " << e.what() << std::endl;
                std::cerr << "Raw response: " << readBuffer << std::endl;
            }
        }
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    return 0;
}