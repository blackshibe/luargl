#!/bin/sh

if [ -d "rgl" ]
then
	cd rgl
	git pull
	cd ..
else
	git clone https://github.com/rxtthin/rgl
fi
