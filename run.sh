# i don't know how to use makefiles

echo "[run.sh] compiling"
export LD_LIBRARY_PATH=/usr/local/lib 
make
rm -f ./*.so
rm -f ./*.o

echo "[run.sh] running"
cd lua
../app $1
cd ../

echo "[run.sh] cleanup"
rm -f app
rm -f lua/luargl.so