CXX = g++
DEBUG_FLAGS = -ggdb -O0 -DDEBUG  # -DFOG_VERBOSE
WARNINGS = -Wall -Wno-unknown-warning-option -Wno-unused-but-set-variable -Wno-unused-function -Wno-missing-braces -Wno-error
FLAGS = $(WARNINGS) -std=c++17 -Iinc $(DEBUG_FLAGS)
LIB_PATH = ./lib/linux 
LIBS = -lSDL2 -lSDL2main -ldl -lpthread
BIN_DIR = bin
ENGINE_PROGRAM_NAME = fog
ENGINE_PROGRAM_PATH = $(BIN_DIR)/$(ENGINE_PROGRAM_NAME)
ENGINE_SOURCE_FILE = src/engine/linux_main.cpp
ASSET_BUILDER_PROGRAM_NAME = $(BIN_DIR)/mist
ASSET_BUILDER_SOURCE_FILE = src/engine/linux_assets.cpp
ASSET_OUTPUT = $(BIN_DIR)/data.fog
ASSET_FILES = $(shell find res/ -type f -name "*.*")
ASSET_SOURCE_FILES = $(shell find src/engine/asset/ -type f -name "*.*")
ASSET_SOURCE_FILES += src/engine/linux_assets.cpp
SOURCE_FILES = $(shell find src/ -type f -name "*.*")
DOCUMENTATION_GENERATOR = $(shell python3 doc/doc-builder.py)
DOCUMENTATION = doc/doc.html

TERMINAL = $(echo $TERM)

.PHONY: default run asset clean debug valgrind doc

default: $(ENGINE_PROGRAM_PATH) $(ASSET_OUTPUT) $(DOCUMENTATION)

doc: $(DOCUMENTATION)

$(DOCUMENTATION) : $(SOURCE_FILES) $(DOCUMENTATION_GENERATOR)
	$(DOCUMENTATION_GENERATOR)

$(ENGINE_PROGRAM_PATH): $(SOURCE_FILES) $(ASSET_OUTPUT) $(ASSET_FILES) $(ASSET_BUILDER_PROGRAM_NAME)
	mkdir -p bin
	rm -f $(BIN_DIR)/res
	ln -s $(shell pwd)/res $(shell pwd)/$(BIN_DIR)/res
	$(CXX) $(FLAGS) $(ENGINE_SOURCE_FILE) -o $(ENGINE_PROGRAM_PATH) -L $(LIB_PATH) $(LIBS)

$(ASSET_BUILDER_PROGRAM_NAME): $(ASSET_SOURCE_FILES) $(ASSET_BUILDER_SOURCE_FILE)
	mkdir -p $(BIN_DIR)
	$(CXX) $(FLAGS) $(ASSET_BUILDER_SOURCE_FILE) -o $(ASSET_BUILDER_PROGRAM_NAME) -L $(LIB_PATH) $(LIBS)

$(ASSET_OUTPUT): $(ASSET_BUILDER_PROGRAM_NAME) $(ASSET_FILES)
	echo $(ASSET_FILES)
	./$(ASSET_BUILDER_PROGRAM_NAME) -o $(ASSET_OUTPUT) $(ASSET_FILES)

asset: $(ASSET_OUTPUT)

clean:
	rm -f $(BIN_DIR)/*
	rm -f src/fog_assets.cpp
	rm -f doc/doc.html

run: $(ENGINE_PROGRAM_PATH) 
	cd $(BIN_DIR); ./$(ENGINE_PROGRAM_NAME)

debug: $(ENGINE_PROGRAM_PATH)
	cd $(BIN_DIR); gdb ./$(ENGINE_PROGRAM_NAME)

valgrind: $(ENGINE_PROGRAM_PATH)
	cd $(BIN_DIR); $(TERMINAL) gdb $(ENGINE_PROGRAM_NAME) &
	cd $(BIN_DIR); valgrind --vgdb-error=0 --suppressions=useable.supp $(ENGINE_PROGRAM_NAME)
