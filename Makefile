NAME        = ircserv

# Compilation
CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98
RM          = rm -f

# Directories
SRC_DIR     = src
OBJ_DIR     = obj

# Source Files
SRC         = $(SRC_DIR)/main.cpp \
              $(SRC_DIR)/Channel/Channel.cpp \
              $(SRC_DIR)/Client/Client.cpp \
              $(SRC_DIR)/Server/Server.cpp \
              $(SRC_DIR)/Server/commandHandler.cpp \
              $(SRC_DIR)/Server/commands/cap.cpp \
              $(SRC_DIR)/Server/commands/pass.cpp \
              $(SRC_DIR)/Server/commands/nick.cpp \
              $(SRC_DIR)/Server/commands/user.cpp \
              $(SRC_DIR)/Server/commands/privmsg.cpp \
              $(SRC_DIR)/Server/commands/mode.cpp \
              $(SRC_DIR)/Server/commands/join.cpp \
              $(SRC_DIR)/Server/commands/invite.cpp \
              $(SRC_DIR)/Server/commands/topic.cpp \
              $(SRC_DIR)/Server/commands/kick.cpp \
              $(SRC_DIR)/Server/commands/quit.cpp

# Object Files
OBJ         = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))

# Colors
GREEN       = \033[0;32m
YELLOW      = \033[0;33m
RESET       = \033[0m

all: $(NAME)

# Compiling Mandatory
$(NAME): $(OBJ)
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
	@printf "$(GREEN)$(NAME) compiled successfully$(RESET)\n"

# Pattern Rule for Objects
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@$(RM) -r $(OBJ_DIR)
	@printf "$(YELLOW)Objects cleaned$(RESET)\n"

fclean: clean
	@$(RM) $(NAME)
	@printf "$(YELLOW)Executable $(NAME) cleaned$(RESET)\n"

re: fclean all

.PHONY: all clean fclean re
