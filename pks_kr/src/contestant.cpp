#include "contestant.h"

void contestant::set_hp(int Hp) {
    hp = Hp;

}
void contestant::set_max_hp(int max_Hp) {
    hp = max_Hp;
    max_hp = max_Hp;
}

void contestant::set_mp(int Mp) {
    mp = Mp;
}
void contestant::set_max_mp(int max_Mp) {
    max_mp = max_Mp;
}
int contestant::get_hp() {
    return hp;
}

int contestant::get_mp() {
    return mp;
}
void contestant::stun() {
    stunned = true;
}
void contestant::unstun() {
    stunned = false;
}
bool contestant::is_stunned() {
    return stunned;
}