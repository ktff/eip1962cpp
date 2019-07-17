CXX		  := g++-7
CXX_FLAGS := -Wall -Wextra -std=c++1z -ggdb

BIN		:= bin
SRC		:= src
INCLUDE	:= include 
LIB		:= lib

LIBRARIES	:= 
EXECUTABLE	:= tests


all: $(BIN)/$(EXECUTABLE)

run: clean all
	clear
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/*.cpp
	$(CXX) $(CXX_FLAGS) -isystem$(INCLUDE) -L$(LIB) $^ -o $@ $(LIBRARIES)

clean:
	-rm $(BIN)/*
