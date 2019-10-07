CXX = g++
DEBUG_FLAGS = -ggdb -O0 -DDEBUG  # -DFOG_VERBOSE
WARNINGS = -Wall -Wno-unknown-warning-option -Wno-unused-but-set-variable -Wno-unused-function -Wno-missing-braces -Wno-error
FLAGS = $(WARNINGS) -std=c++17 -Iinc $(DEBUG_FLAGS)
LIB_PATH = ./lib/linux 
LIBS = -lSDL2 -lSDL2main -ldl -lpthread
BIN_DIR = bin
ENGINE_PROGRAM_NAME = $(BIN_DIR)/fog
ENGINE_SOURCE_FILE = src/linux_main.cpp
ASSET_BUILDER_PROGRAM_NAME = $(BIN_DIR)/mist
ASSET_BUILDER_SOURCE_FILE = src/linux_assets.cpp
ASSET_OUTPUT = data.fog
ASSET_FILES = $(shell find res/)
TERMINAL = $(echo $TERM)

.PHONY: default run asset clean debug valgrind

default: $(ENGINE_PROGRAM_NAME) $(ASSET_OUTPUT)

$(ENGINE_PROGRAM_NAME): $(ASSET_OUTPUT) $(ASSET_FILES) $(ASSET_BUILDER_PROGRAM_NAME)
	mkdir -p bin
	$(CXX) $(FLAGS) $(ENGINE_SOURCE_FILE) -o $(ENGINE_PROGRAM_NAME) -L $(LIB_PATH) $(LIBS)

$(ASSET_BUILDER_PROGRAM_NAME): $(ASSET_BUILDER_SOURCE_FILE) $(HEADERS)
	mkdir -p bin
	$(CXX) $(FLAGS) $(ASSET_BUILDER_SOURCE_FILE) -o $(ASSET_BUILDER_PROGRAM_NAME) -L $(LIB_PATH) $(LIBS)

$(ASSET_OUTPUT): $(ASSET_BUILDER_PROGRAM_NAME) $(ASSET_FILES)
	echo $(ASSET_FILES)
	./$(ASSET_BUILDER_PROGRAM_NAME) -o $(ASSET_OUTPUT) $(ASSET_FILES)

asset: $(ASSET_OUTPUT)

clean:
	rm -f $(BIN_DIR)/*
	rm -f data.fog
	rm -f src/__fog_assets.cpp

run: $(ENGINE_PROGRAM_NAME) 
	./$(ENGINE_PROGRAM_NAME)

debug: $(ENGINE_PROGRAM_NAME)
	gdb ./$(ENGINE_PROGRAM_NAME)

valgrind: $(ENGINE_PROGRAM_NAME)
	$(TERMINAL) gdb $(ENGINE_PROGRAM_NAME) &
	valgrind --vgdb-error=0 --suppressions=useable.supp $(ENGINE_PROGRAM_NAME)
