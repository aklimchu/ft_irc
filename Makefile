#Colors
DEF_COLOR = \033[0;39m
RED = \033[0;91m
GREEN = \033[0;92m

# Program name
NAME		= ircserv

# Compiler
C++ 		= c++
CFLAGS		= -Wall -Werror -Wextra -std=c++20
RM			= rm -f
RMDIR		= rm -rf

# Source / OBJ files / Includes
SRC 		=	srcs/main.cpp
OBJ 		= $(SRC:%.cpp=$(OBJ_DIR)/%.o)
OBJ_DIR		= obj
INCLUDE		= -I "./inc"


# Rules
all:	$(NAME)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	@$(C++) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(NAME):	$(OBJ_DIR) $(OBJ)
	@$(C++) $(CFLAGS) $(OBJ) $(INCLUDE) -o $(NAME)
	@echo "$(GREEN)SUCCESS, IRCSERV IS READY $(DEF_COLOR)"

clean:
	@echo "$(RED)Deleting object files... $(DEF_COLOR)"
	@$(RMDIR) $(OBJ_DIR) 

fclean:		clean
	@echo "$(GREEN)Deleting ircserv... $(DEF_COLOR)"
	@$(RMDIR) $(NAME)
	@echo "$(GREEN)CLEAR $(DEF_COLOR)"

re: 		fclean all

.PHONY: 	all clean fclean re