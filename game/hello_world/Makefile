GAME_NAME = hello
SOURCE = src/example.cpp

CC = g++
FLAGS = -ggdb
LIBS_FOLDER = lib

FOG_LIB :=
ifeq ($(shell uname),Darwin)
	FOG_LIB = $(LIBS_FOLDER)/libfog.dylib
endif
ifeq ($(shell uname),Linux)
	FOG_LIB = $(LIBS_FOLDER)/libfog.a
endif
# Would be nice to remove some of these...
LIBS = -lfog -lSDL2 -lSDL2main -ldl -lpthread -lc -lm
INCUDES = -Iinc

ASSET_BUILDER = ../../out/mist
ASSET_FILE = data.fog
ASSETS = $(shell find res/ -type f -name "*.*")
SOURCE_FILES = $(shell find src/ -type f -name "*.*")

.PHONY: default run

default: $(ASSET_FILE) $(GAME_NAME)

run: default
	./$(GAME_NAME)

$(ASSET_FILE): $(ASSETS) $(ASSET_BUILDER)
	$(ASSET_BUILDER) -o $@ $(ASSETS)

$(GAME_NAME): $(SOURCE_FILES) $(FOG_LIB)
	$(CC) $(FLAGS) -o $@ $(SOURCE) -L$(LIBS_FOLDER) $(LIBS) $(INCUDES)


$(ASSET_BUILDER): $(FOG_LIB)

$(FOG_LIB):
	make -C ../..
	cp ../../out/libfog.* lib/
	mkdir -p inc
	cp ../../out/fog.h inc/

clean:
	make -C ../.. clean
	rm lib/*
	rm $(GAME_NAME)
	rm $(ASSET_FILE)

