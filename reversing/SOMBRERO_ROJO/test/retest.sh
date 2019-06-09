#!/bin/bash

rm ./reallybad
rm ./muymalo.bin
rm ./muy_muy_malo.bin

cd ../; make clean; cmake ..; make; cd -
echo "Encrypt flag"
./encrypt_file ../flag/flag ../flag/flag.enc -e 

echo "Decrypt flag"
./encrypt_file ../flag/flag.enc ../flag/flag.dec -d

echo "Appending file to main"

cat ../ctf_main/reallybad ../flag/flag.enc  > reallybad


chmod +x reallybad

cp reallybad ../ctf_main/reallybad

echo "Running ReallyBad Test"
echo "Writing key..."
echo -ne "\xfb\x04\x95\x17\x90\xf4\x0a" > /tmp/key.bin
./reallybad
