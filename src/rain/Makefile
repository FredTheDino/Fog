TARGET = ../../out/rain

CC = g++
FLAGS = -ggdb -std=c++17
LIBS_FOLDER = ../../out

LIBS = -lfog -lSDL2 -lSDL2main -ldl -lpthread -lc -lm# -lc++
INCLUDES = -I$(LIBS_FOLDER)
SOURCE_FILES = $(shell find *.{cpp,h})

.PHONY: default rain
default: rain

rain: $(TARGET)
$(TARGET): $(SOURCE_FILES)
	$(CC) $(FLAGS) -o $@ $(SOURCE_FILES) -L$(LIBS_FOLDER) $(LIBS) $(INCLUDES)

clean:
	rm -f $(TARGET)

