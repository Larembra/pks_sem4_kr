#ifndef TEST_VK_BOT_H
#define TEST_VK_BOT_H

#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include "json.hpp"

using json = nlohmann::json;

class VKBot {
private:
    std::string access_token;
    std::string group_id;
    std::string api_version;
    std::atomic<bool> running;
    std::thread* longpoll_thread;

    // Callback функции
    std::function<void(int64_t, const std::string&)> on_message_callback;

    // Внутренние методы
    void longPollLoop();
    void processMessage(const json& update);

    // Дополнительно для загрузки/отправки файлов
    std::string getUploadUrl(const std::string& type, int64_t peer_id = 0);
    std::string uploadFile(const std::string& upload_url, const std::string& file_path, const std::string& field_name);
    json saveUploadedFile(const std::string& type, const std::string& upload_result, int64_t peer_id = 0);

public:
    VKBot(const std::string& token, const std::string& gid, const std::string& version = "5.131");
    ~VKBot();

    // Запуск и остановка бота
    void start();
    void stop();

    // Установка обработчика сообщений
    void setMessageHandler(std::function<void(int64_t, const std::string&)> handler);

    // Отправка обычного сообщения
    bool sendMessage(int64_t user_id, const std::string& message);

    // Отправка сообщения с клавиатурой
    bool sendMessageWithKeyboard(int64_t user_id, const std::string& message, const json& keyboard);

    // Отправка гифки (mp4)
    bool sendGif(int64_t user_id, const std::string& file_path, const std::string& caption = "");

    // Отправка документа (не используется сейчас)
    bool sendDocument(int64_t user_id, const std::string& file_path, const std::string& caption = "");

    // Получение информации о группе
    std::string getGroupInfo();

    // Сделать API запрос публичным для тестирования
    std::string makeApiRequest(const std::string& method, const std::string& params);

    // Включить Long Poll для частной группы
    bool enableLongPoll();

    // Проверить статус Long Poll
    std::string getLongPollSettings();
};

// Вспомогательная функция для создания правильной клавиатуры
json createKeyboard(const std::vector<std::vector<std::pair<std::string, std::string>>>& buttons, bool one_time = false);

// Функция для тестирования VK бота
int test_vk_bot();

#endif // TEST_VK_BOT_H