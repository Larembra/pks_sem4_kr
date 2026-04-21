

#ifndef TELEGRAM_H
#define TELEGRAM_H

#include <string>
#include <vector>
#include <cstdint>

struct TelegramMessage {
    int64_t chat_id;
    std::string text;
};

int64_t getChatId();

// Отправка обычного сообщения
void sendTelegramMessage(const int64_t chat_id, const std::string& text);

// Отправка сообщения с клавиатурой
void sendTelegramMessageWithKeyboard(const std::int64_t chat_id, const std::string& text, const std::vector<std::vector<std::string>>& button_rows);

// void sendTelegramMessageWithInlineKeyboard(
//     const std::string& token,
//     int64_t chat_id,
//     const std::string& text,
//     const std::vector<std::pair<std::string, std::string>>& buttons // {текст кнопки, callback_data}
// );
//void sendTelegramGif(const std::string& gif_path);
void sendTelegramMp4Animation(const int64_t chat_id, const std::string& mp4_path);
// Ожидание и получение нового сообщения пользователя
std::string waitForTelegramInput(const int64_t chat_id);

// Вспомогательная функция для CURL (вы можете сделать её static в cpp, если не планируете использовать снаружи)
size_t writecallback(void* contents, size_t size, size_t nmemb, void* userp);

int64_t getLastUpdateId();

void setBotCommands();

#endif //TELEGRAM_H
