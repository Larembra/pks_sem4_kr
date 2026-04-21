#include "player.h"
#include "revolver.h"
#include <iostream>
#include "telegram.h"

using namespace std;

const std::string TELEGRAM_TOKEN = "7928899642:AAGmfMYq3LmjwgySM9lx0heY51Ov6sbscko";
int64_t CHAT_ID;
const std::vector<std::vector<std::string>> acts = {{"Power up(3MP)", "Stun(5MP)", "Heal(3MP)"}, {"Swap bullet(3MP)", "Fireball(5MP)", "Blood ritual(0MP)"}, {"Shoot myself", "Shoot enemy"}};


std::string player::format_status(contestant& enemy, revolver& gun) {
    //std::ostringstream oss;
    string msg = R"(❗️❗️❗️STATUS❗️❗️❗️
💖 Your HP: {MY_HP} | 🔮 MP: {MY_MP}
💀 Opponent HP: {ENEMY_HP} | 🔮 MP: {ENEMY_MP}
❗️❗️❗️STATUS❗️❗️❗️)";

    // Заменяем плейсхолдеры на фактические значения
    size_t pos;
    while ((pos = msg.find("{MY_HP}")) != std::string::npos)
        msg.replace(pos, 7, std::to_string(hp));
    while ((pos = msg.find("{MY_MP}")) != std::string::npos)
        msg.replace(pos, 7, std::to_string(mp));
    while ((pos = msg.find("{ENEMY_HP}")) != std::string::npos)
        msg.replace(pos, 10, std::to_string(enemy.get_hp()));
    while ((pos = msg.find("{ENEMY_MP}")) != std::string::npos)
        msg.replace(pos, 10, std::to_string(enemy.get_mp()));
    // while ((pos = msg.find("{CHAMBERS}")) != std::string::npos)
    //     msg.replace(pos, 10, std::to_string(gun.num_chambers()));
    // while ((pos = msg.find("{BULLETS}")) != std::string::npos)
    //     msg.replace(pos, 9, std::to_string(gun.num_bullets()));

    return msg;
}

bool player::move(const int64_t chat_id, contestant& enemy, revolver& gun) {
    if (mp < max_mp) {
        mp++;
    }
    string msg = format_status(enemy,gun);




    // cout << "your hp: " << hp << endl;
    // cout << "your mp: " << mp << endl;
    // cout << endl;
    // cout << "enemy hp: " << enemy.get_hp() << endl;
    // cout << "enemy mp: " << enemy.get_mp() << endl;
    // cout << endl;
    // std::cout << "bullets: " << gun.num_bullets() << std::endl;
    // std::cout << "chambers: " << gun.num_chambers() << std::endl;
    int damage = 1;
    string act = "";
    //cin >> act;
    //int64_t last_update_id = getLastUpdateId();
    sendTelegramMessageWithKeyboard(chat_id, msg, acts);
    act = waitForTelegramInput(chat_id);
    while (act != "Shoot enemy" and act != "Shoot myself") {
        if (act == "Power up(3MP)") {
            if (mp >= 3) {
                mp -= 3;
                damage = 2;
                sendTelegramMessage(chat_id, "💖 You powered up! 💪");
                sendTelegramMp4Animation(chat_id, "power_up.mp4");
                //sendTelegramMessage(chat_id, "your mp: "+std::to_string(mp));
                // cout << "Spell was casted" << endl;
                // cout << "your mp: " << mp << endl;
            }
            else {
                sendTelegramMessage(chat_id, "❌ Not enough mana");
                sendTelegramMessage(chat_id, "🔮 MP: "+std::to_string(mp)+", need 3");
                // cout << "not enough mana" << endl;
                // cout << "your mp: " << mp << ", need 3" << endl;
            }
        }
        else if (act == "Stun(5MP)") {
            if (mp >= 5) {
                mp -= 5;
                enemy.stun();
                sendTelegramMessage(chat_id, "💖 You stunned the enemy for 1 turn 💀💫");
                sendTelegramMp4Animation(chat_id, "stun.mp4");
                //sendTelegramMessage(chat_id, "your mp: "+std::to_string(mp));
                // cout << "Spell was casted" << endl;
                //cout << "your mp: " << mp << endl;
            }
            else {
                sendTelegramMessage(chat_id, "❌ Not enough mana");
                sendTelegramMessage(chat_id, "🔮 MP: "+std::to_string(mp)+", need 5");
                // cout << "not enough mana" << endl;
                // cout << "your mp: " << mp << ", need 5" << endl;
            }
        }
        else if (act == "Heal(3MP)") {
            if (mp >= 3 and hp < max_hp) {
                mp -= 3;
                hp++;
                sendTelegramMessage(chat_id, "💖 You recovered 1 HP ❤️‍🩹");
                sendTelegramMp4Animation(chat_id, "heal.mp4");
                //sendTelegramMessage(chat_id, "your mp: "+std::to_string(mp));
                //sendTelegramMessage(chat_id, "your hp: "+std::to_string(hp));

                // cout << "Spell was casted" << endl;
                // cout << "your mp: " << mp << endl;
                // cout << "your hp: " << hp << endl;
            }
            else if (hp == max_hp) {
                sendTelegramMessage(chat_id, "❌ You already have max health");
                //cout << "you already have max health" << endl;
            }
            else {
                sendTelegramMessage(chat_id, "❌ Not enough mana");
                sendTelegramMessage(chat_id, "🔮 MP: "+std::to_string(mp)+", need 3");
                // cout << "not enough mana" << endl;
                // cout << "your mp: " << mp << ", need 3" << endl;
            }
        }
        else if (act == "Swap bullet(3MP)") {
            if (mp >= 3) {
                mp -= 3;
                gun.swap_bullet();
                sendTelegramMessage(chat_id, "💖 You swapped bullet 🔄");
                sendTelegramMp4Animation(chat_id, "swap_bullet.mp4");
                //sendTelegramMessage(chat_id, "your mp: "+std::to_string(mp));
                // cout << "Spell was casted" << endl;
                // cout << "your mp: " << mp << endl;
            }
            else {
                sendTelegramMessage(chat_id, "❌ Not enough mana");
                sendTelegramMessage(chat_id, "🔮 MP: "+std::to_string(mp)+", need 3");
                // cout << "not enough mana" << endl;
                // cout << "your mp: " << mp << ", need 3" << endl;
            }
        }
        else if (act == "Fireball(5MP)") {
            if (mp >= 5) {
                mp -= 5;
                enemy.set_hp(enemy.get_hp() - 1);
                if (enemy.get_hp() == 0) {
                    return 0;
                }
                sendTelegramMessage(chat_id, "💖 You casted fireball 💀💥");
                sendTelegramMp4Animation(chat_id, "fireball.mp4");
                //sendTelegramMessage(chat_id, "your mp: "+std::to_string(mp));
                //sendTelegramMessage(chat_id, "enemy hp: "+std::to_string(enemy.get_hp()));
                // cout << "Spell was casted" << endl;
                // cout << "your mp: " << mp << endl;
                // cout << "enemy hp: " << enemy.get_hp() << endl;
            }
            else {
                sendTelegramMessage(chat_id, "❌ Not enough mana");
                sendTelegramMessage(chat_id, "🔮 MP: "+std::to_string(mp)+", need 5");
                // cout << "not enough mana" << endl;
                // cout << "your mp: " << mp << ", need 5" << endl;
            }
        }
        else if (act == "Blood ritual(0MP)") {
            if (hp > 1) {
                hp--;
                if (mp+2 > max_mp) {
                    mp = max_mp;
                }
                else {
                    mp+=2;
                }
                sendTelegramMessage(chat_id, "💖 You performed a blood ritual 🩸➡️🔮");
                sendTelegramMp4Animation(chat_id, "blood_ritual.mp4");
                //sendTelegramMessage(chat_id, "your mp: "+std::to_string(mp));
                //sendTelegramMessage(chat_id, "your hp: "+std::to_string(hp));
                // cout << "Spell was casted" << endl;
                // cout << "your mp: " << mp << endl;
                // cout << "your hp: " << hp << endl;
            }
            else {
                sendTelegramMessage(chat_id, "❌ Not enough health\n 💖 HP: 1");

                // cout << "not enough health" << endl;
                // cout << "your hp: 1" << endl;
            }
        }
        else {
            sendTelegramMessage(chat_id, "❓ Unknown action ❓");
            //cout << "unknown action" << endl;
        }
        //last_update_id = getLastUpdateId();
        msg = format_status(enemy,gun);
        sendTelegramMessageWithKeyboard(chat_id, msg, acts);
        act = waitForTelegramInput(chat_id);
        //cin >> act;
    }
    if (act == "Shoot enemy") {
        if (gun.shoot(chat_id)) {
            sendTelegramMp4Animation(chat_id, "shot_bullet.mp4");
            enemy.set_hp(enemy.get_hp() - damage);
        }
        else {
            sendTelegramMp4Animation(chat_id, "shot_no_bullet.mp4");
        }
        sendTelegramMessage(chat_id, "💀 Enemy HP: "+std::to_string(enemy.get_hp()));
        //cout << "enemy hp: " << enemy.get_hp() << endl;
        return 1;
    }
    else if (act == "Shoot myself") {
        sendTelegramMp4Animation(chat_id, "shoot_myself.mp4");
        if (!gun.shoot(chat_id)) {
            enemy.set_mp(enemy.get_mp()+1);
            return 0;
        }
        hp-=damage;

        sendTelegramMessage(chat_id, "💖 Your HP: "+std::to_string(hp));
        //cout << "your hp: " << hp << endl;
        return 1;
    }
}