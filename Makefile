HOMEDIR  = .

# TODO Maybe use this to set flags? https://wiki.gentoo.org/wiki/Safe_CFLAGS
# TODO ignore warnings in include
PLATFORM = linux64
CC       = g++
CC_WARN  = -Wall -Wextra -pedantic -Wshadow # -Weffc++
CC_ARGS  = -g -march=native -O3 -std=c++17 $(CC_WARN)
CC_LIB   = -lm -lpthread

#================ LEMON =======================================================
LEMON_DIR = /usr/local
LEMON_INC = -I$(LEMON_DIR)/include/lemon
LEMON_LIB = -L$(LEMON_DIR)/lib -lemon

#================= GUROBI =====================================================
GUROBI_DIR = /opt/gurobi1000/linux64
#FLAGVERSION := $(shell gurobi_cl --version | cut -c 26,28 | head -n 1)
FLAGVERSION := 100

GUROBI_INC = -I$(GUROBI_DIR)/include
GUROBI_LIB = -L$(GUROBI_DIR)/lib -lgurobi_c++ -lgurobi$(FLAGVERSION)

#===============================================================================
HOMEDIR_INC = $(HOMEDIR)/incl
HOMEDIR_SRC = $(HOMEDIR)/src
HOMEDIR_OBJ = $(HOMEDIR)/obj
HOMEDIR_BIN = $(HOMEDIR)/bin

#---------------------------------------------
# define includes and libraries
INC = $(GUROBI_INC) $(LEMON_INC) -I$(HOMEDIR_INC)
LIB = $(CC_LIB) $(GUROBI_LIB) $(LEMON_LIB)

_EX = main.cpp # main executable
_SR = pricing.cpp solver.cpp utils.cpp
_OB = $(_SR:.cpp=.o) # all object files
_BN = $(_EX:.cpp=.e) # all executables

# Complete paths
_SRC = $(patsubst %,$(HOMEDIR_SRC)/%,$(_EX))
_OBJ = $(patsubst %,$(HOMEDIR_OBJ)/%,$(_OB))
_BIN = $(patsubst %,$(HOMEDIR_BIN)/%,$(_BN))

all: $(_OBJ) $(_SRC) $(_BIN)

$(HOMEDIR_BIN)/%.e: $(HOMEDIR_OBJ)/%.o $(_OBJ)
	$(CC) $(CC_ARGS) $^ -o $@ $(LIB) $(INC)

$(HOMEDIR_OBJ)/%.o: $(HOMEDIR_SRC)/%.cpp
	$(CC) $(CC_ARGS) -c $^ -o $@ $(INC)

clean:
	rm -f $(HOMEDIR)/*~ $(HOMEDIR_BIN)/*.e $(HOMEDIR_OBJ)/*.o $(HOMEDIR_SRC)/*~
