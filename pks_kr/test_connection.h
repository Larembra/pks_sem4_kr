// test_connection.h
#ifndef TEST_CONNECTION_H
#define TEST_CONNECTION_H

// Функция для тестирования соединения с Telegram API
// Возвращает true если соединение работает, false если нет
bool testTelegramAPI();

// Функция для проверки и настройки прокси если нужно
bool checkAndSetupProxy();

#endif // TEST_CONNECTION_H