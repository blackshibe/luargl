# i don't know how to use makefiles

echo "compiling"
export LD_LIBRARY_PATH=/usr/local/lib 
make

echo "building"
make rgl/linux.mk -f install

echo "running"
./app

# rm -f ./app