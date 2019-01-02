# Source, Executable, Includes, Library Defines

SRC    = simanneal
EXE=   $(SRC).out

# Compiler, Linker Defines
CC = g++

#LIBPATH = -L.
LDFLAGS = -o $(EXE)

default: all 

all:	
	$(CC) $(SRC).cpp $(LDFLAGS)
