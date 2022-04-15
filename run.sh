# i don't know how to use makefiles

echo "[run.sh] compiling"
export LD_LIBRARY_PATH=/usr/local/lib 
make

echo "[run.sh] running"
cd lua
../app
cd ../

echo "[run.sh] cleanup"
rm -f app
rm -f ./*.so
rm -f ./*.o
rm -f lua/luargl.so