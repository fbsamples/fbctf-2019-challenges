#!/bin/bash

if [ "$1" = "-debug" ]; then
	cp CMakeLists_debug.txt CMakeLists.txt;
	rm -rf build; mkdir build; cd build; cmake ..; cp ../test/retest.sh ./test;
	cd ./test;
	./retest.sh;
fi

if [ "$1" == "-release" ]; then
	cp CMakeLists_rel.txt CMakeLists.txt;
	#rm -rf dist; mkdir dist
	rm -rf build; mkdir build; cd build; cmake ..; cp ../test/release.sh ./test;
	cd ./test; ./release.sh;
	cp reallybad ../ctf_main/reallybad;
	cp ../ctf_main/reallybad ../../dist/sombrero_rojo;
	cd ../../dist;
	echo "New file -->" $PWD; ls ;
	printf '\x02' | dd conv=notrunc of=./sombrero_rojo bs=1 seek=5

fi
