#pragma once

#include <string>
#include <fstream>

class Hasher {
public:
    static size_t HashString(const std::string& str);
    static std::string GetFile(const std::string& pathWithFileName);
};