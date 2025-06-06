#include "Hasher.h"

#include <string>
#include <fstream>
#include <sstream>
using namespace std;

/* Hash a string with standard cpp hash function */
size_t Hasher::HashString(const std::string& str) {
    std::hash<std::string> hasher;
    return hasher(str);
}

std::string Hasher::GetFile(const std::string& pathWithFileName) {
    ifstream hashingRead{};
    hashingRead.open(pathWithFileName.c_str(), std::ios_base::in);
    if (!hashingRead.is_open()) {
        // file not found
        return nullptr;
    }

    std::string line;
    std::ostringstream ss;
    while(std::getline(hashingRead, line)) {
        ss << line << '\n';
    }
    const std::string fileContents = ss.str();
    hashingRead.close();

    return fileContents;

}