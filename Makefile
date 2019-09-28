CXX = g++
DEBUG_FLAGS = -ggdb -O0 -DDEBUG  -DFOG_VERBOSE
WARNINGS = -Wall -Wno-unknown-warning-option -Wno-unused-but-set-variable -Wno-unused-function -Wno-missing-braces -Wno-error
FLAGS = $(WARNINGS) -std=c++17 -Iinc $(DEBUG_FLAGS)
LIB_PATH = ./lib/linux 
LIBS = -lSDL2 -lSDL2main -ldl -lpthread
BIN_DIR = bin
ENGINE_PROGRAM_NAME = $(BIN_DIR)/fog
ENGINE_SOURCE_FILE = src/linux_main.cpp
ASSET_BUILDER_PROGRAM_NAME = $(BIN_DIR)/mist
ASSET_BUILDER_SOURCE_FILE = src/linux_assets.cpp
TERMINAL = $(echo $TERM)
SOURCE_FILES = $(shell find src/ -name '*.cpp')
HEADERS = $(shell find src/ -name '*.h')

.PHONY: default run clean debug valgrind

default: $(ENGINE_PROGRAM_NAME) $(ASSET_BUILDER_PROGRAM_NAME)

$(ENGINE_PROGRAM_NAME): $(SOURCE_FILES) $(HEADERS) $(ASSET_BUILDER_PROGRAM_NAME)
	mkdir -p bin
	$(CXX) $(FLAGS) $(ENGINE_SOURCE_FILE) -o $(ENGINE_PROGRAM_NAME) -L $(LIB_PATH) $(LIBS)

$(ASSET_BUILDER_PROGRAM_NAME): $(SOURCE_FILES) $(HEADERS)
	mkdir -p bin
	$(CXX) $(FLAGS) $(ASSET_BUILDER_SOURCE_FILE) -o $(ASSET_BUILDER_PROGRAM_NAME) -L $(LIB_PATH) $(LIBS)

clean:
	rm $(BIN_DIR)/*

asset: $(ASSET_BUILDER_PROGRAM_NAME)
	./$(ASSET_BUILDER_PROGRAM_NAME)

run: $(ENGINE_PROGRAM_NAME)
	./$(ENGINE_PROGRAM_NAME)

debug: $(ENGINE_PROGRAM_NAME)
	gdb ./$(ENGINE_PROGRAM_NAME)

valgrind: $(ENGINE_PROGRAM_NAME)
	$(TERMINAL) gdb $(ENGINE_PROGRAM_NAME) &
	valgrind --vgdb-error=0 --suppressions=useable.supp $(ENGINE_PROGRAM_NAME)
