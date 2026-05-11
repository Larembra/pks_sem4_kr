#include "../include/weapons/revolver.h"
#include "../include/API/telegram.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <random>

using namespace std;




void revolver::new_magazine() {
    srand(time(0));
    random_device rd;
    mt19937 g(rd());

    // Определяем количество пуль (1-5)
    uniform_int_distribution<> dist(1, 5);
    bullets = dist(g);

    // Создаем магазин из 6 ячеек, заполненных нулями
    magazine = vector<int>(6, 0);

    // Заполняем случайные ячейки пулями
    for (int i = 0; i < bullets; ++i) {
        magazine[i] = 1;
    }

    // Перемешиваем пули в магазине
    shuffle(magazine.begin(), magazine.end(), g);

    // Начинаем с первой ячейки
    currentCell = 0;
}

bool revolver::cell() {
    return magazine[currentCell] == 1;
}

int revolver::num_bullets() {
    return bullets;
}

int revolver::num_chambers() {
    return 6 - currentCell;
}


bool revolver::shoot(const int64_t chat_id) {
    if (cell()) {
        sendTelegramMessage(chat_id, "💥 Bullet! 💥");
        cout << "Bullet!" << endl;
        bullets--;
        currentCell = (currentCell + 1) % 6;
        if (bullets == 0) {
            new_magazine();
            sendTelegramMessage(chat_id, R"(❗️ No more bullets ❗️
Revolver randomly reloaded with ❗️)" + to_string(num_bullets()) + "❗️ more bullets");
            //std::cout << "No more bullets! Reloading...\n";
            //bullets = std::rand() % 5 + 1;
            //new_magazine();
        }
        return true;
    }
    sendTelegramMessage(chat_id, "❌ No bullet! ❌");
    //cout << "No bullet!" << endl;
    currentCell = (currentCell + 1) % 6;
    return false;
}

void revolver::swap_bullet() {
    if (cell()) {
        magazine[currentCell] = 0;
    }
    else {
        magazine[currentCell] = 1;
    }
}


void revolver::printmagazine() {
    cout << "Magazine [";
    for(int i = 0; i < 6; ++i) {
        if(i == currentCell) {
            std::cout << "(" << magazine[i] << ")";
        } else {
            std::cout << " " << magazine[i] << " ";
        }
        if(i < 5) std::cout << ",";
    }
    cout << "] Bullets: " << bullets << "\n";
}

