#include <iostream>
#include <curl/curl.h>
#include <string>
#include <vector>

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void testTelegramAPI() {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return;
    }

    std::cout << "=== Testing API Connectivity ===\n" << std::endl;

    // Тест 1: Простой HTTP запрос к google.com для проверки интернета
    std::cout << "1. Testing general internet connectivity (google.com)..." << std::endl;
    {
        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, "https://www.google.com");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            std::cout << "   ✓ Internet is working (Google accessible)" << std::endl;
        } else {
            std::cout << "   ✗ Internet connectivity issue: " << curl_easy_strerror(res) << std::endl;
        }
    }

    // Тест 2: Прямой доступ к api.telegram.org без прокси
    std::cout << "\n2. Testing direct Telegram API access (without proxy)..." << std::endl;
    {
        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.telegram.org");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_PROXY, "");

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            std::cout << "   ✓ api.telegram.org is accessible directly" << std::endl;
            std::cout << "   Response length: " << response.length() << " bytes" << std::endl;
        } else {
            std::cout << "   ✗ Cannot access api.telegram.org: " << curl_easy_strerror(res) << std::endl;
        }
    }

    // Тест 3: Доступ через публичные прокси (для обхода блокировки)
    std::cout << "\n3. Testing Telegram API through public proxies..." << std::endl;
    std::vector<std::string> proxies = {
        "http://51.89.14.70:80",
        "http://165.225.208.41:10605",
        "http://20.206.106.192:80",
        "http://20.210.113.32:80"
    };

    for (const auto& proxy : proxies) {
        std::cout << "   Trying proxy: " << proxy << "..." << std::endl;

        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.telegram.org/bot7928899642:AAH6uR3pM54MJL9oZMAwzsjKqUrFF0VZtpA/getMe");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            std::cout << "      ✓ Success! Response: " << response << std::endl;
            break;
        } else {
            std::cout << "      ✗ Failed: " << curl_easy_strerror(res) << std::endl;
        }
    }

    // Тест 4: Проверка DNS разрешения
    std::cout << "\n4. Testing DNS resolution..." << std::endl;
    {
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.telegram.org");
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1L);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            std::cout << "   ✓ DNS resolution successful" << std::endl;
        } else {
            std::cout << "   ✗ DNS resolution failed: " << curl_easy_strerror(res) << std::endl;
        }
    }

    // Тест 5: Проверка с разными настройками SSL
    std::cout << "\n5. Testing with different SSL settings..." << std::endl;
    {
        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.telegram.org/bot7928899642:AAGmfMYq3LmjwgySM9lx0heY51Ov6sbscko/getMe");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_ALLOW_BEAST);
        curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            std::cout << "   ✓ Success with adjusted SSL settings!" << std::endl;
            std::cout << "   Bot info: " << response << std::endl;
        } else {
            std::cout << "   ✗ Failed with adjusted SSL: " << curl_easy_strerror(res) << std::endl;
        }
    }

    // Тест 6: Проверка через альтернативный эндпоинт
    std::cout << "\n6. Testing alternative endpoint (api.telegram.org:443)..." << std::endl;
    {
        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.telegram.org:443");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            std::cout << "   ✓ Port 443 is accessible" << std::endl;
        } else {
            std::cout << "   ✗ Port 443 issue: " << curl_easy_strerror(res) << std::endl;
        }
    }

    // Тест 7: Проверка VK API
    std::cout << "\n7. Testing VK API access..." << std::endl;
    {
        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.vk.com/method/users.get?v=5.131&user_ids=1");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            std::cout << "   ✓ VK API is accessible!" << std::endl;
            std::cout << "   Response: " << response << std::endl;
        } else {
            std::cout << "   ✗ Cannot access VK API: " << curl_easy_strerror(res) << std::endl;
        }
    }

    // Тест 8: Проверка VK API через прокси (если прямой доступ не работает)
    std::cout << "\n8. Testing VK API through proxies..." << std::endl;
    {
        bool vk_success = false;
        for (const auto& proxy : proxies) {
            std::cout << "   Trying proxy: " << proxy << " for VK API..." << std::endl;

            std::string response;
            curl_easy_setopt(curl, CURLOPT_URL, "https://api.vk.com/method/users.get?v=5.131&user_ids=1");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
            curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());

            CURLcode res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                std::cout << "      ✓ VK API accessible via proxy!" << std::endl;
                std::cout << "      Response: " << response << std::endl;
                vk_success = true;
                break;
            } else {
                std::cout << "      ✗ Failed: " << curl_easy_strerror(res) << std::endl;
            }
        }
        if (!vk_success) {
            std::cout << "   ✗ VK API not accessible through tested proxies" << std::endl;
        }
    }

    // Тест 9: Проверка VK OAuth endpoint
    std::cout << "\n9. Testing VK OAuth endpoint..." << std::endl;
    {
        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, "https://oauth.vk.com/authorize?client_id=1&redirect_uri=https://oauth.vk.com/blank.html&response_type=token");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            std::cout << "   ✓ VK OAuth endpoint is accessible!" << std::endl;
            std::cout << "   Response length: " << response.length() << " bytes" << std::endl;
        } else {
            std::cout << "   ✗ Cannot access VK OAuth: " << curl_easy_strerror(res) << std::endl;
        }
    }

    curl_easy_cleanup(curl);

    std::cout << "\n=== Recommendations ===" << std::endl;
    std::cout << "If Telegram API tests fail, it's likely blocked in your region." << std::endl;
    std::cout << "If VK API tests fail, check your internet connection or VK availability." << std::endl;
    std::cout << "Possible solutions:" << std::endl;
    std::cout << "1. Use a VPN or proxy service" << std::endl;
    std::cout << "2. Use a SOCKS5 proxy with your bot (add to CURL options)" << std::endl;
    std::cout << "3. Deploy your bot on a VPS outside restricted regions" << std::endl;
    std::cout << "4. For Telegram: Use a Bot API proxy like https://github.com/tdlib/telegram-bot-api" << std::endl;
    std::cout << "5. For VK: Ensure you have valid access token and proper API version" << std::endl;
}

// Пример функции для добавления прокси к вашим CURL запросам
void demonstrateProxyUsage() {
    std::cout << "\n=== How to add proxy support to your bot ===\n" << std::endl;
    std::cout << "Add this to your CURL setup in all functions:\n" << std::endl;
    std::cout << R"(
    // For HTTP proxy:
    curl_easy_setopt(curl, CURLOPT_PROXY, "http://proxy-ip:port");

    // For SOCKS5 proxy:
    curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
    curl_easy_setopt(curl, CURLOPT_PROXY, "socks5://proxy-ip:port");

    // With authentication:
    curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, "username:password");
    )" << std::endl;

    std::cout << "\n=== VK Bot Quick Start Guide ===" << std::endl;
    std::cout << "To create a VK bot:" << std::endl;
    std::cout << "1. Create a VK community (group)" << std::endl;
    std::cout << "2. Go to Manage → API usage → Create token" << std::endl;
    std::cout << "3. Use Long Poll API or Callback API for bot" << std::endl;
    std::cout << "4. API endpoint: https://api.vk.com/method/" << std::endl;
    std::cout << "5. Required parameters: access_token, v (API version), group_id" << std::endl;
}

int test_connection() {
    curl_global_init(CURL_GLOBAL_ALL);
    
    testTelegramAPI();
    demonstrateProxyUsage();
    
    curl_global_cleanup();
    
    std::cout << "\nPress Enter to exit...";
    std::cin.get();
    
    return 0;
}