#include "../include/game/game.h"
#include "../include/weapons/revolver.h"
#include "../include/contestants/player.h"
#include "../include/contestants/nn.h"
#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include "../include/json/json.hpp"
#include <unistd.h>
#include <thread>
#include <chrono>
#include "../include/API/telegram.h"

std::string rules =
    "<b>Magic Revolver Duel</b> - variation of russian roulette with magical rules.\n"
    "You and your opponent have 3 HP each (maximum 3) and 3 MP each (maximum 10, recovers 1 MP per turn).\n"
    "A <b>6-chamber</b> revolver is used with a random number of bullets (from 1 to 5).\n"
    "<b>Available spells:</b>\n"
    "<b>Power up</b> (3 MP, next shot deals double damage - 2 HP),\n"
    "<b>Stun</b> (5 MP, opponent skips a turn),\n"
    "<b>Heal</b> (3 MP, +1 HP to your health, but not above maximum),\n"
    "<b>Swap bullet</b> (3 MP, if there was a bullet in chamber, bullet disappears, if wasn't, bullet appears),\n"
    "<b>Fireball</b> (5 MP, deals 1 HP damage to the enemy),\n"
    "<b>Blood ritual</b> (0 MP, you lose 1 HP, but instantly gain 2 MP that can be used this turn).\n\n"
    "Your and the enemy's turn <b>ends</b> with the use of one of <b>this actions:</b>\n"
    "<b>Shoot enemy</b> (enemy will take 1 HP damage if there was a bullet in the chamber, and the turn is passed to the enemy),\n"
    "<b>Shoot myself</b> (<b>bullet in chamber:</b> you will take 1 HP damage and the turn will be passed to the enemy; <b>no bullet:</b> you will be given an additional turn).\n"
    "If HP drops to 0 - game over. Let's begin!";

void game(int64_t chat_id) {
    std::string chosen;
    //sendTelegramMessageWithInlineKeyboard(TELEGRAM_TOKEN, CHAT_ID, "Welcome to Magic Revolver Duel!", {{"Play!", "play"}, {"Rules", "rules"}, {"Telegram", "telegram"}, {"Telegram", "telegram"}, {"Telegram", "telegram"}, {"Telegram", "telegram"}, {"Telegram", "telegram"}});
    sendTelegramMessageWithKeyboard(chat_id, "🎩Welcome to Magic Revolver Duel!🎩", {{"Play!", "Rules"}});
    chosen = waitForTelegramInput(chat_id);
    while (true) {
        if (chosen == "Rules") {
            sendTelegramMessageWithKeyboard(chat_id, rules, {{"Play!"}});
            chosen = waitForTelegramInput(chat_id);
        }
        if (chosen == "Play!") {
            std::vector <std::string> enemies = {"DeepSeek-V3-0324", "GPT-4.1-mini", "Ministral-3B"};
            // Отправляем клавиатуру с кнопкой "DeepSeek"
            sendTelegramMessageWithKeyboard(chat_id, "⚔️Choose your enemy:⚔️", {enemies});

            // Ждём нажатия кнопки (пользователь пришлёт сообщение "DeepSeek")
            chosen = waitForTelegramInput(chat_id);

            revolver gun;
            gun.new_magazine();
            sendTelegramMessage(chat_id, "6-chamber revolver randomly loaded with ❗️"+ std::to_string(gun.num_bullets())+"❗️ bullets!");
            sendTelegramMessage(chat_id, "🍀 Good luck! 🍀");

            player p1;
            p1.set_max_hp(3);
            p1.set_mp(3);
            p1.set_max_mp(10);
            nn enemy;
            enemy.set_max_hp(3);
            enemy.set_mp(3);
            enemy.set_max_mp(10);

            bool result = 0;
            while (p1.get_hp() > 0 && enemy.get_hp() > 0) {
                if (result == 0) {
                    if (!p1.is_stunned()) {
                        enemy.unstun();
                        sendTelegramMessage(chat_id, "💖 Your turn! 💖");
                        // Ждём хода игрока через Telegram:
                        //std::string command = waitForTelegramInput(TELEGRAM_TOKEN, CHAT_ID, last_update_id);
                        // Передайте команду в move, если move умеет принимать строку команды:
                        result = p1.move(chat_id, enemy, gun);
                    }
                    else {
                        p1.unstun();
                        sendTelegramMessage(chat_id, "💀 Enemy turn! 💀");
                        result = enemy.move(chat_id, p1, gun, chosen);
                    }
                }
                else if (result == 1) {
                    if (!enemy.is_stunned()) {
                        p1.unstun();
                        sendTelegramMessage(chat_id, "💀 Enemy turn! 💀");
                        result = enemy.move(chat_id, p1, gun, chosen);
                    }
                    else {
                        enemy.unstun();
                        sendTelegramMessage(chat_id, "💖 Your turn! 💖");
                        //std::string command = waitForTelegramInput(TELEGRAM_TOKEN, CHAT_ID, last_update_id);
                        result = p1.move(chat_id, enemy, gun);
                    }
                }
            }
            if (p1.get_hp() <= 0) {
                sendTelegramMessage(chat_id, "💀 You lose! 💀");
            }
            else {
                sendTelegramMessage(chat_id, "🎉 You win! 🎉");
            }
            sendTelegramMessageWithKeyboard(chat_id, "Game over!", {{"Play!", "Rules"}});
            chosen = waitForTelegramInput(chat_id);
        }
        if (chosen == "/start") {
            sendTelegramMessageWithKeyboard(chat_id, "🎩Welcome to Magic Revolver Duel!🎩", {{"Play!", "Rules"}});
            chosen = waitForTelegramInput(chat_id);
        }
    }
}