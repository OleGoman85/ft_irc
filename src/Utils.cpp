/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ogoman <ogoman@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/08 12:49:17 by ogoman            #+#    #+#             */
/*   Updated: 2025/02/17 10:23:54 by ogoman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Utils.hpp"
#include <sstream> // for std::stringstream and std::getline
#include <chrono>
#include <iomanip>
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


// std::string Utils::getTimestamp() 
// {
//     auto now = std::chrono::system_clock::now();
//     auto in_time_t = std::chrono::system_clock::to_time_t(now);
//     std::stringstream ss;
//     ss << "\033[1;32m[" 
//        << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X")
//        << "]\033[0m ";
//     return ss.str();
// }


std::string Utils::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << "["
       << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X")
       << "] ";
    return ss.str();
}


//! std::vector<std::string>.
/*
Это динамический массив строк (std::string), в котором будут храниться части строки str, разделенные символом delimiter.
*/

//! const std::string& str:
/*
Входная строка, которую нужно разделить на части.
Передается по ссылке (&), чтобы избежать копирования.
const: Указывает, что функция не изменяет строку str.
*/

//! char delimiter:
/*
Символ-разделитель, по которому выполняется разделение строки.
*/

//!Utils
/*
Указывает, что функция split принадлежит пространству имен Utils.
*/

//!tokens
/*
Создается пустой вектор tokens, который будет хранить части строки str, разделенные символом delimiter.
*/

//!std::stringstream
/*
Создает объект std::stringstream, инициализированный строкой str.
Этот поток позволяет работать со строкой, как с потоком ввода, из которого можно читать данные.
Почему используется std::stringstream?

Он удобен для обработки строк, так как позволяет читать их построчно или по определенному разделителю, используя методы std::getline.
*/

//!Utils
/*
Функция Utils::split — удобный инструмент для разделения строки на части по указанному символу. Она эффективно использует встроенные функции и классы C++, такие как std::stringstream и std::getline, для упрощения обработки строк.
*/


//!
/*
Задача:

вспомогательные функции, разделение строк.
*/