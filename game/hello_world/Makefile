GAME_NAME = hello
SOURCE = src/example.c

CC = gcc
FLAGS = -ggdb -std=c11 -fPIC
LIBS_FOLDER = lib

FOG_LIB :=
ifeq ($(shell uname -s),Darwin)
	FOG_LIB = $(LIBS_FOLDER)/libfog.dylib
endif
ifeq ($(shell uname -s),Linux)
	FOG_LIB = $(LIBS_FOLDER)/libfog.a
endif
# Would be nice to remove some of these...
LIBS = -lfog -lSDL2 -lSDL2main -ldl -lpthread -lc -lm -lc++
INCUDES = -Iinc

ASSET_BUILDER = ../../out/mist
ASSET_FILE = data.fog
ASSETS = $(shell find res/ -type f -name "*.*")
SOURCE_FILES = $(shell find src/ -type f -name "*.*")

.PHONY: default run game

default: game
game: $(ASSET_FILE) $(GAME_NAME)

run: $(GAME_NAME)
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
	rm -f lib/*
	rm -f $(GAME_NAME)
	rm -f $(ASSET_FILE)

