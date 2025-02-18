#include "BotCommand.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <sstream>

#include "../include/Channel.hpp"
#include "../include/Client.hpp"
#include "../include/Server.hpp"

/**
 * @brief Sends a message to the specified client, appending CRLF at the end.
 * @param fd File descriptor of the target client.
 * @param msg The message to send (without CRLF).
 */
static void sendToClient(int fd, const std::string& msg)
{
    std::string withCrLf = msg + "\r\n";
    send(fd, withCrLf.c_str(), withCrLf.size(), 0);
}

static std::string getRandom8BallAnswer()
{
    static bool seeded = false;
    if (!seeded) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        seeded = true;
    }

    static const char* answers[] = {
        "Yes!",
        "Think again!",
        "Maybe...",
        "Certainly yes",
        "Ask again later",
        "Don't ask if you don't want to know the answer",
        "Chances are low",
        "Check your code, not me"
    };

    int size = static_cast<int>(sizeof(answers) / sizeof(answers[0]));
    int randomIndex = std::rand() % size;
    return answers[randomIndex];
}

static std::string getRandomJoke()
{
    static bool seeded = false;
    if (!seeded) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        seeded = true;
    }

    static const char* jokes[] = {
        "There are 10 types of people in the world: those who understand binary and those who don't.",
        "Debugging: Being the detective in a crime movie where you are also the murderer.",
        "To understand recursion, you must first understand recursion.",
        "In a world without fences and walls, who needs Gates and Windows?",
        "Programming is like writing a book, but if you miss a single comma, the whole story falls apart.",
        "A SQL query walks into a bar, approaches two tables and asks: 'Can I join you?'",
        "How many programmers does it take to change a light bulb? None. It's a hardware problem!",
        "Why do programmers prefer dark mode? Because light attracts bugs!"
    };

    int size = static_cast<int>(sizeof(jokes) / sizeof(jokes[0]));
    int randomIndex = std::rand() % size;
    return jokes[randomIndex];
}

static std::string getRandomFact()
{
    static bool seeded = false;
    if (!seeded) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        seeded = true;
    }

    static const char* facts[] = {
        "C++ was developed by Bjarne Stroustrup starting in 1979.",
        "IRC was created by Jarkko Oikarinen in 1988.",
        "The first computer programmer was Ada Lovelace in the 19th century.",
        "The number 42 is a reference from 'The Hitchhiker's Guide to the Galaxy'.",
        "The term 'bug' in programming originated from a real moth found in a computer.",
        "Linux was created by Linus Torvalds in 1991.",
        "Python was named after 'Monty Python', not the snake.",
        "Java was originally called Oak.",
        "The first website went live on August 6, 1991.",
        "The first version of C++ was released in 1985."
    };

    int size = static_cast<int>(sizeof(facts) / sizeof(facts[0]));
    int randomIndex = std::rand() % size;
    return facts[randomIndex];
}

static std::string getServerTime()
{
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime);
    return std::string(buffer);
}

/**
 * @brief Returns a help message listing all supported bot commands.
 */
static std::string getHelpMessage()
{
    return "Available BOT commands:\n"
           "  BOT ROLL [NdM]               - Roll N dice with M sides (default "
           "1d6)\n"
           "  BOT 8BALL <question>         - Magic 8-Ball answers\n"
           "  BOT JOKE                     - Receive a random joke\n"
           "  BOT FACT                     - Receive a random fact\n"
           "  BOT TIME                     - Get server local time\n"
           "  BOT HELP                     - Show this help\n";
}

/**
 * @brief Parses a dice expression in the form NdM (e.g., 2d20).
 * @param s The string to parse.
 * @param N Number of dice (output).
 * @param M Number of sides per die (output).
 * @return True if parsing is successful, otherwise false.
 */
static bool parseDice(const std::string& s, int& N, int& M)
{
    size_t pos = s.find('d');
    if (pos == std::string::npos)
        return false;

    std::string left = s.substr(0, pos);
    std::string right = s.substr(pos + 1);

    for (size_t i = 0; i < left.size(); ++i) {
        if (!std::isdigit(left[i]))
            return false;
    }
    for (size_t i = 0; i < right.size(); ++i) {
        if (!std::isdigit(right[i]))
            return false;
    }

    int tmpN = std::atoi(left.c_str());
    int tmpM = std::atoi(right.c_str());
    if (tmpN <= 0 || tmpM <= 0)
        return false;

    N = tmpN;
    M = tmpM;
    return true;
}

/**
 * @brief Rolls N dice each with M sides, then returns a descriptive string.
 * @param N Number of dice.
 * @param M Number of sides per die.
 * @return A string describing each roll and the total sum.
 */
static std::string rollDice(int N, int M)
{
    static bool seeded = false;
    if (!seeded) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        seeded = true;
    }

    std::ostringstream out;
    out << "You rolled " << N << "d" << M << ": [";

    int sum = 0;
    for (int i = 0; i < N; ++i) {
        int roll = (std::rand() % M) + 1;
        sum += roll;
        out << roll;
        if (i < N - 1)
            out << ", ";
    }
    out << "] (sum = " << sum << ")";
    return out.str();
}

/**
 * @brief Handles the BOT command, parsing the subcommand and dispatching.
 * @param server Pointer to the Server instance.
 * @param fd File descriptor of the client.
 * @param tokens Tokenized command parts.
 * @param fullCommand Original command string (unused).
 */
void handleBotCommand(Server* server, int fd,
    const std::vector<std::string>& tokens,
    const std::string& /*fullCommand*/)
{
    (void)server;
    if (tokens.size() < 2) {
        sendToClient(fd, "461 BOT :Not enough parameters");
        return;
    }

    std::string subCommand = tokens[1];
    std::transform(subCommand.begin(), subCommand.end(), subCommand.begin(),
        ::toupper);

    if (subCommand == "HELP") {
        sendToClient(fd, getHelpMessage());
    } else if (subCommand == "JOKE") {
        sendToClient(fd, getRandomJoke());
    } else if (subCommand == "FACT") {
        sendToClient(fd, getRandomFact());
    } else if (subCommand == "TIME") {
        std::string t = getServerTime();
        sendToClient(fd, "Server local time: " + t);
    } else if (subCommand == "8BALL") {
        if (tokens.size() < 3) {
            sendToClient(
                fd, "461 BOT 8BALL :Not enough parameters (ask a question!)");
            return;
        }
        std::string question;
        for (size_t i = 2; i < tokens.size(); ++i) {
            if (i > 2)
                question += " ";
            question += tokens[i];
        }
        std::string answer = getRandom8BallAnswer();
        sendToClient(fd, "Magic 8-Ball says: " + answer);
    } else if (subCommand == "ROLL") {
        int N = 1, M = 6;
        if (tokens.size() >= 3) {
            if (!parseDice(tokens[2], N, M)) {
                sendToClient(fd, "Usage: BOT ROLL [NdM], e.g. BOT ROLL 2d20");
                return;
            }
        }
        std::string result = rollDice(N, M);
        sendToClient(fd, result);
    } else {
        sendToClient(fd, "421 BOT " + subCommand + " :Unknown BOT subcommand");
    }
}
