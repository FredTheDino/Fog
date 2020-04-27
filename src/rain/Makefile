RAIN = ../../out/rain

CC = g++
FLAGS = -ggdb -std=c++17
LIBS_FOLDER = ../../out


LIBS = ""
ifeq ($(shell uname -s | cut -c -5),MINGW)
    LIBS = -L/mingw64/lib -lfog -lSDL2 -LSDL2main
else
    LIBS = -lfog -lSDL2 -lSDL2main -ldl -lpthread -lc -lm
endif

INCLUDES = -I$(LIBS_FOLDER)
SOURCE_FILES = $(shell find *.cpp)

.PHONY: default rain
default: rain

rain: $(RAIN)
$(RAIN): $(SOURCE_FILES)
	$(CC) $(FLAGS) -o $@ $(SOURCE_FILES) -L$(LIBS_FOLDER) $(LIBS) $(INCLUDES)

clean:
	rm -f $(RAIN)

