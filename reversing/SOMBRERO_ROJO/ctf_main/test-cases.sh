#!/bin/bash

declare -a arr=("" "my_sUp3r_s3cret_p@\$\$w0rd1" "AA BB")

rm /tmp/key.bin

## now loop through the above array
for i in "${arr[@]}"
do
    echo "Sedning: $i"
    ./reallybad $i
done
echo "Writing key..."
echo -ne "\xfb\x04\x95\x17\x90\xf4\x0a" > /tmp/key.bin
b64_data=$(./reallybad)
echo $b64_data 
echo $b64_data | base64 -d
echo "\n"