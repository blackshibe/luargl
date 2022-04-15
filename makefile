CFLAGS = -std=c++17 -ggdb -O0
LDFLAGS = -llua -lrgl -lfreetype 

SRC = -c src/luargl/*.c
OBJ = *.o

all: compile link main

compile:
	gcc -Iinclude -fPIC -Wall $(SRC) 

link:
	gcc $(OBJ) -shared -o lua/luargl.so $(LDFLAGS)

main:
	g++ $(CFLAGS) -o app src/main.cpp $(LDFLAGS)
