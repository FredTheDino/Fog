CXX = g++

BIN_DIR = out
LIB_DIR = lib

WARNINGS = -Wall -Wno-unused-function -Wno-missing-braces
FLAGS = $(WARNINGS) -std=c++17 -Iinc
DEBUG_FLAGS = $(FLAGS) -ggdb -O0 -DDEBUG
LIBS = -LSDL2 -LSDL2main -lc -ldl

ifeq ($(shell uname -s | cut -c -5),MINGW)
    LIBS = -L/mingw64/lib -LSDL2 -LSDL2main 
endif

ENGINE_SOURCE = src/engine/unix_main.cpp
ENGINE_LIBRARY_NAME = libfog.a
ENGINE_LIBRARY = $(BIN_DIR)/$(ENGINE_LIBRARY_NAME)
SOURCES := $(shell find src/ -type f -name "*.*")

ASSET_BUILDER = $(BIN_DIR)/mist
ASSET_BUILDER_SOURCE = src/engine/unix_assets.cpp
ASSET_FILES := $(shell find res/ -type f -name "*.*")
ASSET_SOURCE_FILES := $(shell find src/engine/asset/ -type f -name "*.*")
ASSET_SOURCE_FILES += $(ASSET_BUILDER_SOURCE)

EDITOR_NAME = rain
EDITOR = $(BIN_DIR)/$(EDITOR_PROGRAM_NAME)
EDITOR_SOURCE_FILE = src/engine/unix_main.cpp

DOCUMENTATION_GENERATOR = tools/doc-builder.py
DOCUMENTATION = tools/doc.html

BINDING_GENERATOR = tools/bind-gen.py
BINDINGS = $(BIN_DIR)/fog.h
BINDINGS += src/fog_bindings.cpp

default: all

all: engine docs mist rain

.NOTPARALLEL: $(ENGINE_LIBRARY)
engine: $(ENGINE_LIBRARY)
$(ENGINE_LIBRARY): $(ENGINE_SOURCE) $(SOURCES) | $(BIN_DIR) $(BINDINGS)
	$(CXX) $(DEBUG_FLAGS) -c -fpic -static $< -o $@ $(LIBS)

rain: $(ENGINE_LIBRARY)
	make -C src/rain

mist: $(ASSET_BUILDER)
$(ASSET_BUILDER): $(ASSET_BUILDER_SOURCE) $(ASSET_SOURCE_FILES) | $(BIN_DIR)
	$(CXX) $(DEBUG_FLAGS) $< -o $@ $(LIBS)

$(BINDINGS):
	python3 $(BINDING_GENERATOR)

.PHONY: docs
docs:
	python3 $(DOCUMENTATION_GENERATOR)

.PHONY: clean
clean:
	rm -rf $(BIN_DIR)
	rm -f src/fog_bindings.cpp
	rm -f tools/doc.html

$(BIN_DIR):
	mkdir -p $(BIN_DIR)
