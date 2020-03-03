CXX = g++
# NOTE: The verbose flag doesn't allow compilation on cirtain mac systems.
DEBUG_FLAGS = -ggdb -O0 -DDEBUG  # -DFOG_VERBOSE
WARNINGS = -Wall -Wno-unused-function -Wno-missing-braces
FLAGS = $(WARNINGS) -std=c++17 -Iinc $(DEBUG_FLAGS)
LIB_PATH = ./lib/linux
LIBS = -lSDL2 -lSDL2main -ldl -lpthread
BIN_DIR = out

LIB_FLAG :=
ENGINE_PROGRAM_NAME :=
ifeq ($(shell uname),Darwin)
	LIB_FLAG += -dynamiclib
	ENGINE_PROGRAM_NAME += libfog.dylib
endif
ifeq ($(shell uname),Linux)
	LIB_FLAG += -shared
	ENGINE_PROGRAM_NAME += libfog.so
endif

ENGINE_PROGRAM_PATH = $(BIN_DIR)/$(ENGINE_PROGRAM_NAME)
ENGINE_SOURCE_FILE = src/engine/unix_main.cpp
ASSET_BUILDER_PROGRAM_NAME = $(BIN_DIR)/mist
ASSET_BUILDER_SOURCE_FILE = src/engine/unix_assets.cpp
ASSET_OUTPUT = $(BIN_DIR)/data.fog
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

TERMINAL = $(echo $TERM)

.PHONY: default run edit asset clean debug valgrind doc bindings

default: $(ENGINE_PROGRAM_PATH) $(DOCUMENTATION) $(ASSET_BUILDER_PROGRAM_NAME)

doc: $(DOCUMENTATION)

bindings: $(BINDINGS)

$(BINDINGS):
	echo $(BINDING_GENERATOR)

$(DOCUMENTATION): $(ENGINE_PROGRAM_PATH)
	$(DOCUMENTATION_GENERATOR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(ENGINE_PROGRAM_PATH): $(SOURCE_FILES) | $(BIN_DIR) $(BINDINGS)
	rm -f $(BIN_DIR)/res
	$(CXX) $(FLAGS) -c -fPIC $(LIB_FLAG) $(ENGINE_SOURCE_FILE) -o $@ # -L $(LIB_PATH) $(LIBS)

$(EDITOR_PROGRAM_PATH): $(SOURCE_FILES) | $(BIN_DIR)
	$(CXX) $(FLAGS) -DFOG_EDITOR $(EDITOR_SOURCE_FILE) -o $@ -L $(LIB_PATH) $(LIBS)


$(ASSET_BUILDER_PROGRAM_NAME): $(ASSET_SOURCE_FILES) $(ASSET_BUILDER_SOURCE_FILE) | $(BIN_DIR)
	$(CXX) $(FLAGS) $(ASSET_BUILDER_SOURCE_FILE) -o $(ASSET_BUILDER_PROGRAM_NAME) -L $(LIB_PATH) $(LIBS)

$(ASSET_OUTPUT): $(ASSET_BUILDER_PROGRAM_NAME) $(ASSET_FILES)
	./$(ASSET_BUILDER_PROGRAM_NAME) -o $(ASSET_OUTPUT) $(ASSET_FILES)

asset: $(ASSET_OUTPUT)

clean:
	rm -rf $(BIN_DIR)
	rm -f src/fog_assets.cpp
	rm -f src/bindings.cpp
	rm -f tools/doc.html

edit: $(EDITOR_PROGRAM_PATH)
	

run: $(ENGINE_PROGRAM_PATH)
	cd $(BIN_DIR); ./$(ENGINE_PROGRAM_NAME)

debug: $(ENGINE_PROGRAM_PATH)
	cd $(BIN_DIR); gdb -ex "b _fog_assert_failed()" -ex "b _fog_illegal_allocation()" ./$(ENGINE_PROGRAM_NAME)

valgrind: $(ENGINE_PROGRAM_PATH)
	cd $(BIN_DIR); $(TERMINAL) gdb $(ENGINE_PROGRAM_NAME) &
	cd $(BIN_DIR); valgrind --vgdb-error=0 --suppressions=useable.supp $(ENGINE_PROGRAM_NAME)
