#ifndef GAME_H
#define GAME_H

#include <thread>
#include <string>
#include <vector>
#include "player.h"
#include "revolver.h"
#include "nn.h"

enum class GameStage {
    Intro,
    Rules,
    ChooseEnemy,
    Playing,
    GameOver
};

struct GameState {
    GameStage stage = GameStage::Intro;
    std::string chosen_enemy;
    player p1;
    nn d;
    revolver gun;
    bool turn_player = true; // true = player, false = enemy
    bool game_over = false;
};

void game(int64_t chat_id);

#endif //GAME_H
