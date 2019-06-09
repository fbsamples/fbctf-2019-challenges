#!/bin/bash

cd ../; make clean; cmake ..; make; cd -
echo "Encrypt flag"
./encrypt_file ../flag/flag ../flag/flag.enc -e 
echo "Appending file to main"

cat ../ctf_main/reallybad ../flag/flag.enc  > reallybad

chmod +x reallybad

cp reallybad ../ctf_main/reallybad