# ctf_elf

Challenge 1: This binary has code that gets executed before main. The code before main checks for a debugger, if a debugger is found the program will jump to the fake main and eventually exit. Else the will decrypt some strings, read a key from /tmp/key.bin, then decrypt the flag. 

Challenge 2: This contains an encrypted executable that was encrypted with modified version of RC4 encryption. There is a Key Encryption Key(KEK) that is derived from the previous challenge flags (currently statically compiled). The two previous flags are sha256 hashed to derive the KEK. The KEK is used to decrypt padding to check for the correct decryption and the real encryption key. 

This challenge attempts to decrypt append payload but fails because the decryption is one byte off from the key position. In DEBUG mode it will create two files one with the correctly decrypted payload "decrypted_payload.bin" and one that is not decrypted correctly "next_challenge.bin"

The binary has been statically compiled and stripped to remove all libc function symbols. The header has also been modified were the endianess flag was flipped and may cause some older dissasemblers crash.
## Build
In order to build the modules make "build" directory at the root of repo and cd into build and execute "cmake .." this will build all the modules and test apps.

## Solution

Challenge 1:

The file ```/tmp/key.bin``` must contain ```\xfb\x04\x95\x17\x90\xf4\0a```.  This will output ```RkxBR3s3aDQ3X1c0c243X1MwX0g0UmR9```, which can be b64 decoded to ```fb{7h47_W4sn7_S0_H4Rd}\n```. Solution for challenge 2 is to decrypt the embedded executable.

Challenge 2:
The object of this challenge is to correctly decrypt the ELF file by RE'ing the app and identiyfing the correct decryption key and correct offset were decryption should begin.

## main (decoy)

Nothing important happens in main, it is all a decoy.

If no args are provide the following error will be thrown:

```c
printf("Give me some args!\n");
```

If one arg is provided the binary will run through a "fake" decryption loop. If ```my_sUp3r_s3cret_p@$$w0rd1``` is given as the argument the binary will decrpyt data and print ```Nope{Lolz_this_isnt_the_flag...Try again...}```


# Testing

To compile the challenge in release mode use the ./buildit.sh script with the -release flag. The final binary will be in the ./build/ctf_main/ called reallybad.

You can also run the ./buildit.sh script in -debug mode which will print out a lot debug statements and create an executable that will drop both "decrypted_payload.bin" and "next_challenge.bin" in the ./build/test directory.