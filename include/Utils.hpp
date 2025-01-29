/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ogoman <ogoman@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/08 12:48:47 by ogoman            #+#    #+#             */
/*   Updated: 2025/01/16 09:51:11 by ogoman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string> // for std::getline
#include <vector>

/**
 * @brief Utility functions for string manipulation.
 *
 * The Utils namespace contains helper functions used throughout the project.
 */
namespace Utils {
    /**
     * @brief Splits a string using the specified delimiter.
     *
     * This function divides the input string into a vector of substrings based
     * on the provided delimiter character. It uses std::getline to iterate over the string.
     *
     * @param str The input string to split.
     * @param delimiter The character used as the delimiter.
     * @return std::vector<std::string> A vector containing the resulting substrings.
     */
    std::vector<std::string> split(const std::string& str, char delimiter);
}

#endif // UTILS_HPP

