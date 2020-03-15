CXX = g++
# NOTE: The verbose flag doesn't allow compilation on cirtain mac systems.
WARNINGS = -Wall -Wno-unused-function -Wno-missing-braces
FLAGS = $(WARNINGS) -std=c++17 -Iinc
DEBUG_FLAGS = $(FLAGS) -ggdb -O0 -DDEBUG  # -DFOG_VERBOSE
RELEASE_FLAGS = $(FLAGS) -O3  #TODO(gu)
LIB_PATH = ./lib/linux
LIBS = # -lSDL2 -lSDL2main -ldl -lpthread -lc -lm
BIN_DIR = out

LIB_FLAG =
ENGINE_PROGRAM_NAME =
ifeq ($(shell uname -s),Darwin)
	LIB_FLAG += -dynamiclib
	ENGINE_PROGRAM_NAME = libfog.dylib
endif
ifeq ($(shell uname -s),Linux)
	LIB_FLAG += -static
	ENGINE_PROGRAM_NAME = libfog.a
endif

ENGINE_PROGRAM_PATH = $(BIN_DIR)/$(ENGINE_PROGRAM_NAME)
ENGINE_SOURCE_FILE = src/engine/unix_main.cpp
ASSET_BUILDER_PROGRAM_NAME = $(BIN_DIR)/mist
ASSET_BUILDER_SOURCE_FILE = src/engine/unix_assets.cpp
ASSET_FILES = $(shell find res/ -type f -name "*.*")
ASSET_SOURCE_FILES = $(shell find src/engine/asset/ -type f -name "*.*")
ASSET_SOURCE_FILES += src/engine/unix_assets.cpp
EDITOR_PROGRAM_NAME = rain
EDITOR_PROGRAM_PATH = $(BIN_DIR)/$(EDITOR_PROGRAM_NAME)
EDITOR_SOURCE_FILE = src/engine/unix_main.cpp
SOURCE_FILES = $(shell find src/ -type f -name "*.*")
DOCUMENTATION_GENERATOR = $(shell python3 tools/doc-builder.py)
DOCUMENTATION = tools/doc.html
BINDING_GENERATOR = $(shell python3 tools/bind-gen.py)
BINDINGS = $(BIN_DIR)/fog.h 
BINDINGS += src/fog_bindings.cpp 
RAIN = $(BIN_DIR)/rain

TERMINAL = $(echo $TERM)

.PHONY: default engine doc bindings clean rain # run edit asset debug valgrind

default: engine
engine: $(ENGINE_PROGRAM_PATH) $(DOCUMENTATION) $(ASSET_BUILDER_PROGRAM_NAME) $(RAIN)

$(RAIN):
	make -C src/rain

release: $(SOURCE_FILES) $(ASSET_BUILDER_PROGRAM_NAME) | $(BIN_DIR) $(BINDINGS)
	$(CXX) $(RELEASE_FLAGS) -c -fPIC $(LIB_FLAG) $(ENGINE_SOURCE_FILE) -o $(ENGINE_PROGRAM_PATH) -L $(LIB_PATH) $(LIBS)

doc: $(DOCUMENTATION)

bindings: $(BINDINGS)

$(BINDINGS):
	$(BINDING_GENERATOR)

$(DOCUMENTATION): $(ENGINE_PROGRAM_PATH)
	$(DOCUMENTATION_GENERATOR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(ENGINE_PROGRAM_PATH): $(SOURCE_FILES) | $(BIN_DIR) $(BINDINGS)
	$(CXX) $(DEBUG_FLAGS) -c -fPIC $(LIB_FLAG) $(ENGINE_SOURCE_FILE) -o $@ -L $(LIB_PATH) $(LIBS)

#$(EDITOR_PROGRAM_PATH): $(SOURCE_FILES) | $(BIN_DIR)
#	$(CXX) $(DEBUG_FLAGS) -DFOG_EDITOR $(EDITOR_SOURCE_FILE) -o $@ -L $(LIB_PATH) $(LIBS)

$(ASSET_BUILDER_PROGRAM_NAME): $(ASSET_SOURCE_FILES) $(ASSET_BUILDER_SOURCE_FILE) | $(BIN_DIR)
	$(CXX) $(DEBUG_FLAGS) $(ASSET_BUILDER_SOURCE_FILE) -o $(ASSET_BUILDER_PROGRAM_NAME) -L $(LIB_PATH) $(LIBS)

clean:
	rm -rf $(BIN_DIR)
	rm -f src/bindings.cpp
	rm -f tools/doc.html

edit: $(EDITOR_PROGRAM_PATH)

