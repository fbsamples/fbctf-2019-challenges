#!/usr/bin/env python3
# TODO: Add encrypted flag generation script here
from ctypes import c_int, c_uint
import zipfile
import time
import argparse
import os
try:
    from Crypto.Cipher import AES
    from Crypto.Util import Counter
except ImportError:
    import sys
    print('pycryptodome module not installed.')
    sys.exit()

FLAG_FILENAME = 'temp.bin'
ENCRYPTED_OUTPUT_FILE = 'recovered_file.zip'


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


def encrypt_flag(data, creation_time, charset='0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'):
    srand(creation_time)
    rand_str = []
    for _ in range(32):
        rand_str.append(charset[rand() % len(charset)])
    key = ''.join(rand_str)
    #counter = Counter.new(32, prefix=b'\x00\x00\x00\x00\x0B\xAD\xC0\xDE\xDE\xC0\xAD\x0B')
    counter = Counter.new(128, initial_value=0xe4b8c75accb877dab0f5a6f0a7aa0e67)
    cipher = AES.new(key.encode('ascii'), AES.MODE_CTR, counter=counter)
    return cipher.encrypt(data)


def main():
    global FLAG_FILENAME
    parser = argparse.ArgumentParser(description='Generate encrypted flag file')
    parser.add_argument('flag_file', help='Unencrypted flag file to encrypt', type=argparse.FileType('rb'))
    parser.add_argument('--create-date', '-d', dest='creation_date', type=int)
    args = parser.parse_args()
    # Use provided creation_date or use current timestamp
    creation_time = int(time.time()) if not args.creation_date else args.creation_date
    print('Using creation time as:', creation_time)
    # Encrypt file using provided creation_time as seed for PRNG
    encrypted_data = encrypt_flag(args.flag_file.read(), creation_time)
    # Write encrypted flag back to file
    temp_filename = ENCRYPTED_OUTPUT_FILE + '.tmp'
    with open(temp_filename, 'wb') as f:
        f.write(encrypted_data)
    # Set file creation/modification/access time (platform-specific) as this is
    # what the user will need for bruteforcing the key
    os.utime(temp_filename, (creation_time, creation_time))
    # Write the file into a ZIP so that timestamps are preserved during distribution
    with zipfile.ZipFile(ENCRYPTED_OUTPUT_FILE, mode='w', compression=zipfile.ZIP_DEFLATED) as zf:
        zf.write(temp_filename, FLAG_FILENAME, compress_type=zipfile.ZIP_DEFLATED)
    # Remove temp file
    os.remove(temp_filename)
    # Show output message and quit
    print('Encrypted file has been written to:', ENCRYPTED_OUTPUT_FILE)


if __name__ == '__main__':
    main()
