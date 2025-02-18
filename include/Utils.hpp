#ifndef UTILS_HPP
#define UTILS_HPP
#include <string>  
#include <vector>

/**
 * @brief Utility functions for string manipulation and general helper methods.
 *
 * The `Utils` namespace provides common helper functions that are used
 * throughout the project. These include string processing utilities and 
 * timestamp retrieval.
 */
namespace Utils
{
    /**
     * @brief Splits a string into a vector of substrings using a specified delimiter.
     *
     * This function iterates over the input string and extracts substrings
     * separated by the given delimiter. The substrings are stored in a 
     * vector and returned.
     *
     * @param str The input string to split.
     * @param delimiter The character used as the delimiter.
     * @return std::vector<std::string> A vector containing the resulting substrings.
     */
    std::vector<std::string> split(const std::string& str, char delimiter);

    /**
     * @brief Retrieves the current timestamp as a formatted string.
     *
     * This function returns the current system timestamp in a human-readable 
     * format (e.g., "YYYY-MM-DD HH:MM:SS"). It can be used for logging and 
     * message timestamps.
     *
     * @return std::string The formatted current timestamp.
     */
    std::string getTimestamp();

}  // namespace Utils

#endif  // UTILS_HPP
