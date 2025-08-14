#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace FileUtils {

    std::string readFile(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open file: " + filePath);
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    void writeFile(const std::string& filePath, const std::string& content) {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open file for writing: " + filePath);
        }
        file << content;
    }

} // namespace FileUtils