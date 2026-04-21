

#ifndef NN_H
#define NN_H

#include "contestant.h"
#include "revolver.h"
#include <string>
#include <cstdint>

class nn : public contestant {
public:
    bool move(const int64_t chat_id, contestant& enemy, revolver& gun, std::string model);
    std::vector <std::string> get_response(contestant& enemy, revolver& gun, std::string model);
    std::string format_prompt(contestant& enemy, revolver& gun);
    std::string format_status(contestant& enemy, revolver& gun);
};

#endif //DEEPSEEK_H
