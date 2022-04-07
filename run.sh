# i don't know how to use makefiles

echo "compiling"
export LD_LIBRARY_PATH=/usr/local/lib 
make

echo "running"
./app

echo "cleanup"
rm ./app
rm ./test.so
rm ./luargl.so

