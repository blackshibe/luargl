CFLAGS = -std=c++17 -ggdb -O0
LDFLAGS = -llua -lrgl

main:
	gcc -fPIC -shared -Wall src/lualibs/test.c -o test.so ${LDFLAGS}
	gcc -fPIC -shared -Wall src/lualibs/luargl.c -o luargl.so ${LDFLAGS}

	g++ $(CFLAGS) -o app src/*.cpp $(LDFLAGS)