#include "../include/Utils.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

/**
 * @brief Splits a string using a specified delimiter.
 *
 * This function takes an input string and splits it into tokens
 * using the provided delimiter character. Each token is appended to a vector of strings,
 * which is then returned.
 *
 * @param str The input string to be split.
 * @param delimiter The character used to separate tokens in the string.
 * @return std::vector<std::string> A vector containing the tokens extracted from the input string.
 */

std::vector<std::string> Utils::split(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens; // Stores parts of the input string.
    std::stringstream ss(str);
    std::string token;
    // Extract tokens separated by the delimiter and add them to the tokens vector.
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string Utils::getTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << "["
       << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X")
       << "] ";
    return ss.str();
}
