#include "../include/secret/dotenv.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>

static inline void ltrim(std::string& s) {
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    s.erase(0, i);
}

static inline void rtrim(std::string& s) {
    size_t i = s.size();
    while (i > 0 && std::isspace(static_cast<unsigned char>(s[i - 1]))) --i;
    s.erase(i);
}

static inline void trim(std::string& s) {
    ltrim(s);
    rtrim(s);
}

static inline std::string unquote(std::string s) {
    if (s.size() >= 2) {
        if ((s.front() == '"' && s.back() == '"') || (s.front() == '\'' && s.back() == '\'')) {
            s = s.substr(1, s.size() - 2);
        }
    }
    return s;
}

std::unordered_map<std::string, std::string> loadDotenv(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        throw std::runtime_error("Cannot open .env file: " + path);
    }

    std::unordered_map<std::string, std::string> out;
    std::string line;

    while (std::getline(f, line)) {
        trim(line);
        if (line.empty()) continue;
        if (line[0] == '#') continue;

        auto eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);

        trim(key);
        trim(val);

        val = unquote(val);

        if (!key.empty()) out[key] = val;
    }

    return out;
}

std::string getDotenvValue(
    const std::unordered_map<std::string, std::string>& env,
    const std::string& key
) {
    auto it = env.find(key);
    if (it == env.end()) return {};
    return it->second;
}