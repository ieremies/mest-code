HOMEDIR  = .

# TODO Maybe use this to set flags? https://wiki.gentoo.org/wiki/Safe_CFLAGS
PLATFORM = linux64
CC       = g++
CC_WARN  = -Wall -Wextra -Wpedantic -Wshadow -Weffc++
CC_ARGS  = -march=native -std=c++17
CC_LIB   = -lm -lpthread -ldl

# All is the debug version
all: CC_ARGS += -g -Werror -Og
all: executable

release: CC_ARGS += -O3 -DNDEBUG
release: executable

#================ LEMON =======================================================
LEMON_DIR = /usr/local
LEMON_INC = -isystem$(LEMON_DIR)/include/lemon
LEMON_LIB = -L$(LEMON_DIR)/lib -lemon

#================= GUROBI =====================================================
GUROBI_DIR = /opt/gurobi1000/linux64
#FLAGVERSION := $(shell gurobi_cl --version | cut -c 26,28 | head -n 1)
FLAGVERSION := 100

GUROBI_INC = -isystem$(GUROBI_DIR)/include # used isystem to ignore warnings
GUROBI_LIB = -L$(GUROBI_DIR)/lib -lgurobi_c++ -lgurobi$(FLAGVERSION)

#================= LOGURU =======================================================
LOGURU_DIR = $(HOMEDIR)/lib/
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
INC = $(GUROBI_INC) $(LEMON_INC) -I$(HOMEDIR_INC) $(LOGURU_INC)
LIB = $(CC_LIB) $(GUROBI_LIB) $(LEMON_LIB) -L$(HOMEDIR_LIB)

_EX = main.cpp
_SR = pricing.cpp solver.cpp utils.cpp branch2.cpp graph.cpp heuristic.cpp
_OB = $(_SR:.cpp=.o) loguro.o # all object files
_BN = $(_EX:.cpp=.e) # all executables

# Complete paths
_SRC = $(patsubst %,$(HOMEDIR_SRC)/%,$(_EX))
_OBJ = $(patsubst %,$(HOMEDIR_OBJ)/%,$(_OB))
_BIN = $(patsubst %,$(HOMEDIR_BIN)/%,$(_BN))

executable: $(_OBJ) $(_SRC) $(_BIN)

$(HOMEDIR_OBJ)/loguro.o: $(LOGURU_DIR)/loguru.cpp
	$(CC) $(CC_ARGS) -c $^ -o $@ $(INC)

$(HOMEDIR_BIN)/%.e: $(HOMEDIR_OBJ)/%.o $(_OBJ)
	$(CC) $(CC_ARGS) $(CC_WARN) $^ -o $@ $(LIB) $(INC)

$(HOMEDIR_OBJ)/%.o: $(HOMEDIR_SRC)/%.cpp
	$(CC) $(CC_ARGS) $(CC_WARN) -c $^ -o $@ $(INC)

# Remove all objects and executables
# except loguru.o
clean:
	rm -f $(HOMEDIR)/*~ $(HOMEDIR_BIN)/*.e $(HOMEDIR_OBJ)/*.o $(HOMEDIR_SRC)/*~
