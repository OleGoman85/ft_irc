# Comments added in English

NAME = ircserv
CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++17
SRC_DIR = src
CMD_DIR = commands
OBJ_DIR = objects
INC_DIR = include

# Get all .cpp files and generate the list of object files
SRCS = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(CMD_DIR)/*.cpp)
OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(notdir $(SRCS)))

# Colors for output text
BGreen = \033[1;32m
BRed = \033[1;31m
BYellow = \033[1;33m
BPurple = \033[0;35m

# Main target
all: tag $(NAME)

# Build the executable
$(NAME): $(OBJS)
	@printf "$(BGreen)\nCompiling FT_IRC..."
	@$(CXX) $(CXXFLAGS) -I $(INC_DIR) -o $(NAME) $(OBJS)
	@printf "$(BGreen) DONE 🎉\033[0m\n"

# Compile .cpp files into object files from SRC_DIR
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@$(CXX) $(CXXFLAGS) -I $(INC_DIR) -c $< -o $@
	@/bin/echo -n "..."

# Compile .cpp files into object files from CMD_DIR
$(OBJ_DIR)/%.o: $(CMD_DIR)/%.cpp | $(OBJ_DIR)
	@$(CXX) $(CXXFLAGS) -I $(INC_DIR) -c $< -o $@
	@/bin/echo -n "..."

# Create the directory for object files
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

# Clean up object files and the executable
clean:
	@echo "$(BRed)🤖 Cleaning up object files..."
	@rm -rf $(OBJ_DIR)
	@echo "$(BGreen)Cleanup complete!✨"

fclean: clean
	@echo "$(BRed)Removing executable $(NAME)..."
	@rm -f $(NAME)
	@echo "$(BGreen)FT_IRC environment is spotless! 🌟"



# ASCII art for a cool tag header
tag:
	@if [ ! -e $(OBJ_DIR)/.tag ]; then \
		mkdir -p $(OBJ_DIR); \
		printf "$(BPurple)\n\n\n"; \
		printf "███████╗████████╗     ██╗██████╗  ██████╗ \n"; \
		printf "██╔════╝╚══██╔══╝     ██║██╔══██╗██╔════╝ \n"; \
		printf "█████╗     ██║OG && AA██║██████╔╝██║      \n"; \
		printf "██╔══╝     ██║        ██║██╔══██╗██║      \n"; \
		printf "██║        ██║███████╗██║██║  ██║╚██████╗ \n"; \
		printf "╚═╝        ╚═╝╚══════╝╚═╝╚═╝  ╚═╝ ╚═════╝ \n"; \
		printf "$(BPurple)\n"; \
		touch $(OBJ_DIR)/.tag; \
	fi

# Declare pseudo-targets to avoid conflicts with files named all, clean, etc.
.PHONY: all clean fclean re tag

# Run tests
test:
	@echo "$(BYellow)[🔍] Running tests..."
	@python3 tests/test_irc.py || echo "$(BRed)[❌] Tests failed!"

# Rebuild everything and run tests
re: fclean all test

