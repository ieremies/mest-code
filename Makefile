HOMEDIR  = .

SYSTEM 	:= $(shell uname -s)
CC       = g++
CC_WARN  = #-Wall -Wextra -Wpedantic -Wshadow -Weffc++
CC_ARGS  = -march=native -std=c++17
CC_LIB   = -lm -lpthread -ldl

# All is the debug version
all: CC_ARGS += -g -Werror -Og
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

#================= CLIQUER =====================================================
CLIQUER_DIR = $(HOMEDIR)/lib/cliquer-1.21
CLIQUER_SR = cliquer.c reorder.c graph_cliquer.c
CLIQUER_SRC = $(patsubst %,$(CLIQUER_DIR)/%,$(CLIQUER_SR))
CLIQUER_OBJ = $(CLIQUER_SR:.c=.o)


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
INC = $(GUROBI_INC) $(LOGURU_INC) -I$(HOMEDIR_INC) -I$(CLIQUER_DIR)
LIB = $(CC_LIB) $(GUROBI_LIB) -L$(HOMEDIR_LIB) -L$(CLIQUER_DIR)

_EX = main.cpp
_SR = pricing.cpp solver.cpp utils.cpp branch.cpp graph.cpp dsatur.cpp # wave.cpp
# _SR = pricing_cliquer.cpp solver.cpp utils.cpp branch.cpp graph.cpp dsatur.cpp # wave.cpp
_OB = $(_SR:.cpp=.o) loguru.o $(CLIQUER_OBJ) # all object files
_BN = $(_EX:.cpp=.e) # all executables

# Complete paths
_SRC = $(patsubst %,$(HOMEDIR_SRC)/%,$(_EX))
_OBJ = $(patsubst %,$(HOMEDIR_OBJ)/%,$(_OB))
_BIN = $(patsubst %,$(HOMEDIR_BIN)/%,$(_BN))

executable: $(_OBJ) $(_SRC) $(_BIN)

$(HOMEDIR_OBJ)/cliquer.o: $(CLIQUER_DIR)/cliquer.c
	gcc -c -fomit-frame-pointer -funroll-loops $^ -o $@ $(INC)

$(HOMEDIR_OBJ)/reorder.o: $(CLIQUER_DIR)/reorder.c
	gcc -c -fomit-frame-pointer -funroll-loops $^ -o $@ $(INC)

$(HOMEDIR_OBJ)/graph_cliquer.o: $(CLIQUER_DIR)/graph_cliquer.c
	gcc -c -fomit-frame-pointer -funroll-loops $^ -o $@ $(INC)

$(HOMEDIR_OBJ)/loguru.o: $(LOGURU_DIR)/loguru.cpp
	$(CC) $(CC_ARGS) -c $^ -o $@ $(INC)

$(HOMEDIR_BIN)/%.e: $(HOMEDIR_OBJ)/%.o $(_OBJ)
	$(CC) $(CC_ARGS) $(CC_WARN) $^ -o $@ $(LIB) $(INC)

$(HOMEDIR_OBJ)/%.o: $(HOMEDIR_SRC)/%.cpp
	$(CC) $(CC_ARGS) $(CC_WARN) -c $^ -o $@ $(INC)

clean:
	rm -f $(HOMEDIR)/*~ $(HOMEDIR_BIN)/*.e $(HOMEDIR_OBJ)/*.o $(HOMEDIR_SRC)/*~
