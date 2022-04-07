CFLAGS = -std=c++17 -ggdb -O0
LDFLAGS = -llua -lrgl -lfreetype

main:
	gcc -fPIC -shared -Wall src/lualibs/test.c -o test.so ${LDFLAGS}
	gcc -Iinclude -fPIC -shared -Wall src/lualibs/luargl.c -o luargl.so ${LDFLAGS}

	g++ $(CFLAGS) -o app src/*.cpp $(LDFLAGS)