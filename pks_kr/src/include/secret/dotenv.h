
#ifndef PKS_KR_DOTENV_H
#define PKS_KR_DOTENV_H

#pragma once

#include <string>
#include <unordered_map>

std::unordered_map<std::string, std::string> loadDotenv(const std::string& path = ".env");

std::string getDotenvValue(
    const std::unordered_map<std::string, std::string>& env,
    const std::string& key
);

#endif //PKS_KR_DOTENV_H
