#!/usr/bin/env python3
from ctypes import c_int, c_uint
import argparse
import os
try:
    from Crypto.Cipher import AES
    from Crypto.Util import Counter
except ImportError:
    import sys
    print('pycryptodome module not installed.')
    sys.exit()

TARGET_FIRST_LINE = "// Copyright 2019 - QwarkSoft"
SECONDS_TO_BRUTEFORCE = 10


# GLIBC rand implementation stolen from: https://gist.github.com/integeruser/4cca768836c68751904fe215c94e914c
def srand(seed):
    srand.r = [0 for _ in range(34)]
    srand.r[0] = c_int(seed).value
    for i in range(1, 31):
        srand.r[i] = (16807 * srand.r[i - 1]) % 2147483647
    for i in range(31, 34):
        srand.r[i] = srand.r[i - 31]
    srand.k = 0
    for _ in range(34, 344):
        rand()


def rand():
    srand.r[srand.k] = srand.r[(srand.k - 31) % 34] + srand.r[(srand.k - 3) % 34]
    r = c_uint(srand.r[srand.k]).value >> 1
    srand.k = (srand.k + 1) % 34
    return r


def decrypt_flag(data, creation_time, charset='0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'):
    srand(creation_time)
    rand_str = []
    for _ in range(32):
        rand_str.append(charset[rand() % len(charset)])
    key = ''.join(rand_str)
    counter = Counter.new(128, initial_value=0xe4b8c75accb877dab0f5a6f0a7aa0e67)
    cipher = AES.new(key.encode('ascii'), AES.MODE_CTR, counter=counter)
    return cipher.decrypt(data)


def main():
    global TARGET_FIRST_LINE, SECONDS_TO_BRUTEFORCE
    parser = argparse.ArgumentParser(description='Decrypt encrypted flag file')
    parser.add_argument('flag_file', help='Encrypted flag file to encrypt')
    args = parser.parse_args()
    # Read last modified timestamp to decide start of brute force
    lastmodified_timestamp = int(os.path.getmtime(args.flag_file))
    print('Flag was last modified at:', lastmodified_timestamp)
    with open(args.flag_file, 'rb') as f:
        encrypted_data = f.read()
    # Assume encryption took at most SECONDS_TO_BRUTEFORCE seconds
    # Start brute forcing from SECONDS_TO_BRUTEFORCE seconds before file was written to disk
    lastmodified_timestamp -= SECONDS_TO_BRUTEFORCE
    decrypt_successful = False
    # Looking ahead for 2 seconds so that file generated
    # by python script (encryption takes less than a second) can be decrypted
    for _ in range(2 + SECONDS_TO_BRUTEFORCE):
        print('Bruteforcing w/ value:', lastmodified_timestamp)
        decrypted_data = decrypt_flag(encrypted_data, lastmodified_timestamp)
        try:
            if decrypted_data.decode('ascii').startswith(TARGET_FIRST_LINE):
                decrypt_successful = True
                break
        except UnicodeDecodeError:
            # Decryption did not succeed
            pass
        lastmodified_timestamp += 1
    if decrypt_successful:
        print('Decrypted File:')
        print(decrypted_data.decode('ascii'))
    else:
        print('WTF - Could not decrypt file')


if __name__ == '__main__':
    main()
