.phony: all
HOMEDIR  = .

SYSTEM 	:= $(shell uname -s)
CC       = g++
CC_WARN  = #-Wall -Wextra -Wpedantic -Wshadow -Weffc++
CC_ARGS  = -march=native -std=c++17
CC_LIB   = -lm -lpthread -ldl

# All is the debug version
all: CC_ARGS += -g -Werror # -Og
all: executable

release: CC_ARGS += -O3 -DNDEBUG
release: executable

#================= GUROBI =====================================================
GUROBI_DIR = /opt/gurobi1003/linux64
ifeq ($(SYSTEM),Darwin)
	GUROBI_DIR = /Library/gurobi1002/macos_universal2
endif
FLAGVERSION := 100

GUROBI_INC = -isystem$(GUROBI_DIR)/include # used isystem to ignore warnings
GUROBI_LIB = -L$(GUROBI_DIR)/lib -lgurobi_c++ -lgurobi$(FLAGVERSION)

#================= LOGURU =======================================================
LOGURU_DIR = $(HOMEDIR)/lib
LOGURU_INC = -I$(LOGURU_DIR)

#===============================================================================
HOMEDIR_INC = $(HOMEDIR)/incl
HOMEDIR_SRC = $(HOMEDIR)/src
HOMEDIR_OBJ = $(HOMEDIR)/obj
HOMEDIR_BIN = $(HOMEDIR)/bin
HOMEDIR_LIB = $(HOMEDIR)/lib

# Create the directories if they don't exist
$(shell mkdir -p $(HOMEDIR_OBJ) $(HOMEDIR_BIN) $(HOMEDIR_LIB))

#---------------------------------------------
# define includes and libraries
INC = $(GUROBI_INC) $(LOGURU_INC) -I$(HOMEDIR_INC)
LIB = $(CC_LIB) $(GUROBI_LIB) -L$(HOMEDIR_LIB)

_EX = main.cpp
_SR = $(notdir $(wildcard $(HOMEDIR_SRC)/*.cpp))
_OB = $(_SR:.cpp=.o) loguru.o
_BN = $(_EX:.cpp=.e)

# Complete paths
_SRC = $(patsubst %,$(HOMEDIR_SRC)/%,$(_EX))
_OBJ = $(patsubst %,$(HOMEDIR_OBJ)/%,$(_OB))
_BIN = $(patsubst %,$(HOMEDIR_BIN)/%,$(_BN))

executable: primal.e # dual.e

primal.e: $(HOMEDIR_OBJ)/solver_primal.o $(_OBJ)
	$(CC) $(CC_ARGS) $(CC_WARN) $^ -o $(HOMEDIR_BIN)/$@ $(LIB) $(INC)

dual.e: $(HOMEDIR_OBJ)/solver_dual.o $(_OBJ)
	$(CC) $(CC_ARGS) $(CC_WARN) $^ -o $(HOMEDIR_BIN)/$@ $(LIB) $(INC)

$(HOMEDIR_OBJ)/loguru.o: $(LOGURU_DIR)/loguru.cpp
	$(CC) $(CC_ARGS)  -c $^ -o $@ $(INC)

$(HOMEDIR_OBJ)/%.o: $(HOMEDIR_SRC)/%.cpp
	$(CC) $(CC_ARGS) $(CC_WARN) -c $^ -o $@ $(INC)

clean:
	rm -f $(HOMEDIR)/*~ $(HOMEDIR_BIN)/*.e $(HOMEDIR_OBJ)/*.o $(HOMEDIR_SRC)/*~
