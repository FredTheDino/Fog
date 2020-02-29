#!/usr/bin/bash
g++ example.cpp -ggdb -L./out -l:libfog.so -Iout -Iinc -lSDL2 -lSDL2main -ldl -lpthread
