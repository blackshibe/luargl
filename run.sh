# i don't know how to use makefiles

echo "compiling"
export LD_LIBRARY_PATH=/usr/local/lib 
make

echo "building"

./update_rgl.sh

cd rgl
./build_rgl.sh
cd ..


echo "running"
./app

# rm -f ./app
