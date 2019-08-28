CXX = clang++
DEBUG_FLAGS = -ggdb -O0 -DDEBUG
WARNINGS = -Wall -Wno-unknown-warning-option -Wno-missing-braces -Wno-error
FLAGS = $(WARNINGS) -std=c++17 -Iinc $(DEBUG_FLAGS)
LIB_PATH = ./lib/linux 
LIBS = -lSDL2 -lSDL2main -ldl -lpthread
PROGRAM_NAME = bin/fog
SOURCE_FILE = src/linux_main.cpp
TERMINAL = $(echo $TERM)
SOURCE_FILES = $(shell find src/ -name '*.cpp')
HEADERS = $(shell find src/ -name '*.h')

.PHONY: default run clean debug valgrind

default: $(PROGRAM_NAME)

$(PROGRAM_NAME): $(SOURCE_FILES) $(HEADERS)
	mkdir -p bin
	$(CXX) $(FLAGS) $(SOURCE_FILE) -o $(PROGRAM_NAME) -L $(LIB_PATH) $(LIBS)

clean:
	rm $(PROGRAM_NAME)

run: $(PROGRAM_NAME)
	./$(PROGRAM_NAME)

debug: $(PROGRAM_NAME)
	gdb ./$(PROGRAM_NAME)

valgrind: $(PROGRAM_NAME)
	$(TERMINAL) gdb $(PROGRAM_NAME) &
	valgrind --vgdb-error=0 --suppressions=useable.supp $(PROGRAM_NAME)
