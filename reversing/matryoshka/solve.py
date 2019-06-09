# solve.py
import ctypes
from ctypes import *
import codecs
import itertools
import string
import struct
import time

cipher_encrypted = [199,120,185,89]
xor_encrypted = [25,2,234,135]
rc4_encrypted = [
    236,67,21,191,187,74,71,214,189,144,233,104,208,105,55,41,248,18,244,229,208,251,243,126,114,97,121,25,237,68,18,82,245,249,170,20,54,13,31,178,82,107,242,106,218,157,236,60]

def solve_rot13(key):
    key = codecs.encode(key, 'rot_13')
    print "ROT13 key:           %s" % (key)
    # get offset of shellcode 2
    pkey = create_string_buffer(key)
    pkeyint = cast(pkey, POINTER(c_ushort))
    pkeyint = pkeyint[0] ^ 0x354C
    return pkeyint

def solve_cipher(src):
    a = ((src[0] - 0x55 + 256) % 256)
    b = ((src[1] - 0x48 + 256) % 256)
    c = ((src[2] - 0x89 + 256) % 256)
    d = ((src[3] - 0xE5 + 256) % 256)

    print "VINEGERE CIPHER key: %c%c%c%c" % (a,b,c,d)

def solve_xor(src):
    a = src[0] ^ 0x55
    b = src[1] ^ 0x48
    c = src[2] ^ 0x89
    d = src[3] ^ 0xE5
    s = struct.pack("<I", 0x4a514f75)
    print "STEAMING XOR key:    %c%c%c%c%s" % (a,b,c,d,s)

def rc4_key_schedule(key):
    # create the rc4 key schedule
    keylength = len(key)
    S = range(256)
    j = 0
    for i in range(256):
        k = ord(key[i % keylength])
        j = (j + S[i] + k) % 256
        S[i], S[j] = S[j], S[i]  # swap
    return S, j
def rc4_encrypt(dst, src, key):
    # rc4 encrypt the src
    src_len = len(src)
    S, j = rc4_key_schedule(key)
    j = 0
    i = 0
    m = 0
    while (m < src_len):
        char = src[m]
        i = (i + 1) % 256
        j = (j + S[i]) % 256
        # swap
        S[i], S[j] = S[j], S[i]
        k = S[(S[i] + S[j]) % 256]
        dst[m] = char ^ k
        m+=1

def xor_encrypt(src, key):
    # stream xor encrypt
    i = 0
    for c in src:
        src[i]=ord(c)^ord(key[i % 8])
        i+=1

def isprintable(s):
    check = 0
    for c in s:
      if c in string.printable:
          check = check + 1
      else:
        return False
    return True

# they will need to brute force
def solve_rc4(src):
    chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
    decrypted = [0] * 48
    attempts = 0
    s = struct.pack("<I", 0x36395477)
    for guess in itertools.product(chars, repeat=4):
        attempts += 1
        key = "%c%c%c%c%s" % (guess[0],guess[1],guess[2], guess[3],s)
        rc4_encrypt(decrypted, rc4_encrypted, key)
        list_of_ints = decrypted[8:]
        if isprintable(''.join(chr(x) for x in list_of_ints)):
            #print ''.join(chr(x) for x in list_of_ints)
            print "RC4 key:             %s" % (key)
            return key

def solve_flag(flag_int, seed):
    chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_"
    attempts = 0
    for guess in itertools.product(chars, repeat=4):
        attempts += 1
        D = int("0x%x%x%x%x" % (
            ord(guess[0]),
            ord(guess[1]),
            ord(guess[2]),
            ord(guess[3])), 16)
        if flag_int == (seed ^ D):
            C = 0x665f336b1a566b19 ^ D
            B = 0x393b415f5a590044 ^ C
            A = 0x3255557376f68 ^ B
            a =  struct.pack("<L", D)
            b =  struct.pack("<Q", C)
            c =  struct.pack("<Q", B)
            d =  struct.pack("<Q", A)
            print "FLAG:                %s%s%s%s" % (d,c,b,a)
            return

def main():
    # ROT13
    offset_shellcode2 = solve_rot13("4ZberYriryf2TbXrrcTbvat")
    #print "offset %X" % (0x60000 + offset_shellcode2)

    # VINEGERE CIPHER
    solve_cipher(cipher_encrypted)

    # STREAM XOR ENCRYPTION
    solve_xor(xor_encrypted)

    # RC4 ENCRYPTION
    start_time = time.time()
    key = solve_rc4(rc4_encrypted)
    print("--- solve_rc4 took %s seconds ---" % (time.time() - start_time))
    decrypted = [0] * 48
    rc4_encrypt(decrypted, rc4_encrypted, key)
    #rc4_encrypt(decrypted, rc4_encrypted, "YrQmwT96")
    string = "0x%s" % (''.join("%X" % x for x in decrypted[:8]))
    flag_int = int(string, 16)

    # SOLVE FLAG
    start_time = time.time()
    solve_flag(flag_int, 0xb334f7f3b9ca653)
    print("--- solve_flag took %s seconds ---" % (time.time() - start_time))

if __name__ == '__main__':
    main()
