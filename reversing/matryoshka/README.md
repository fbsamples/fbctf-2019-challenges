# matryoshka

Name: `matryoshka`
Description:
```
There was a downloader found on a Mac desktop. It's your job to have layers of fun getting the flag.
```
Flag: `fb{Y0_daWg_1_h34rd_u_1ik3_fl4gs}`
1) ROT13, key = 4MoreLevels2GoKeepGoing
2) VIGENERE CIPHER, key = r00t
3) Streaming XOR, key = LJcbuOQJ
4) RC4, key = YrQmwT96

## what does it do?
1) downloads image of pickachu.png.

2) Load image into memory with mmap RWX of size 0x4096.

3) Find the offset of PNG header. Shellcode is at offset 0x60000 from the PNG header.

4) Jump into the shellcode.

5) User is prompted for a password for ROT13, Key produces the next offset of the 2nd Shellcode.

6) Load 2nd Shellcode with mmap RWX.

7) Jump into the shellcode.

8) Decrypt VIGENERE CIPHER, Streaming XOR, and then RC4

7) RC4 returns the key FLAG_INT in order to find the final flag:

8) Algorithm is thus:

```
 0x5F675761645F3059 /*Y0_daWg_*/  ^ 0x5F64723433685F31 /*1_h34rd_*/
 0x5F64723433685F31 /*1_h34rd_*/ ^ 0x665F336B69315F75 /*u_1ik3_f*/
 0x665F336B69315F75 /*u_1ik3_f*/ ^ 0x7367346C /*l4gs*/
 FLAG_INT = 0x7367346C /*l4gs*/ ^ SEED
 ```

9) User will need to calculate the final flag and input


## How to Build

1) run python script
```
$python generate.py --build
```

2) place png image in hosted server, must be http, and get request should be like the following
```
http://157.230.132.171/pickachu_wut.png

GET REQUEST:
GET /pickachu_wut.png HTTP/1.1
Host: 157.230.132.171
```

## How to Configure

Only modify `CONFIGURATION OPTIONS` section of generate.py.

These are the modifable options:
```
* FLAG                 = b"Y0_daWg_1_h34rd_u_1ik3_fl4gs" # MUST BE 28 BYTES
* SEED                 = random.randint(1, 0x1fcd1d2076ca6573)
* RC4_KEY              = "YrQmwT96" # KEY MUST BE 8 BYTES
* XOR_KEY              = "LJcbuOQJ" # KEY MUST BE 8 BYTES
* CIPHER_KEY           = "r00t" # KEY MUST BE 4 BYTES
* ROT13_KEY            = '4MoreLevels2GoKeepGoing'

* RC4_ASSEMBLY_FILE    = "gen_rc4.asm"
* XOR_ASSEMBLY_FILE    = "gen_xor.asm"
* CIPHER_ASSEMBLY_FILE = 'gen_cipher.asm'
* ROT13_ASSEMBLY_FILE  = 'gen_rot13.asm'
* DOWNLOADER_FILE      = 'downloader'

* RC4_PRINT            = 'Almost there, this is the home stretch!\n'
* XOR_PRINT            = "Keep going!"
* CIPHER_PRINT         = "Keep going!"
* ROT13_PRINT          = "I believe in you ..."

* CODE_PNG_OFFSET      = 0x60000 # MUST BE SMALLER THAN IMAGE
* CODE_PNG_OFFSET_2    = 0x7878 # 0x60000 + 0x7878
* HOSTNAME             = "157.230.132.171"
* PNG_FILE             = "pickachu.png"
* PNG_NAME             = "pickachu_wut.png"
* FAIL_WHALE           = []
```

## Requirements

* python macholib https://pypi.org/project/macholib/

* must be build on MacOsx


## Example input
```
malwareunic-mbp:FBCTF_2019 malwareunicorn$ ./dist/downloader.macho
Please input a key value:
4MoreLevels2GoKeepGoing
I believe in you ...
Please input a key value:
r00t
Keep going!
Please input a key value:
LJcbuOQJ
Keep going!
Please input a key value:
YrQmwT96
Almost there, this is the home stretch!
Enter the actual flag:
Y0_daWg_1_h34rd_u_1ik3_fl4gs

fb{Y0_daWg_1_h34rd_u_1ik3_fl4gs}

Congratulations!!
Created by @malwareunicorn
```
