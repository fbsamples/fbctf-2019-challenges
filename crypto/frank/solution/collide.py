#!/usr/bin/env python

import binascii
import os
import re

from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes  # type: ignore
from cryptography.hazmat.backends import default_backend  # type: ignore


BLOCK_SIZE = 128 // 8
IV_SIZE = 96 // 8


def printable(buf):
    out = ''
    for i in range(0, len(buf)):
        c = chr(buf[i])
        if re.match(r'[a-zA-Z0-9]', c):
            out += c
        else:
            out += '.'
    return out

def print_buf(name, buf):
    if len(name) == 1:
        name = ' ' + name
    print(f"{name}: {binascii.hexlify(buf)} {printable(buf)}")
    return buf

def encode_int(n, buf_size=BLOCK_SIZE):
    bytelen = (n.bit_length() + 7) // 8
    assert bytelen <= buf_size

    out = b''
    for i in range(0, bytelen):
        out = bytes([n % 256]) + out
        n >>= 8

    zerolen = buf_size - len(out)
    out = (b'\0' * zerolen) + out
    return out

def copy_buf(buf):
    out = b''
    for i in range(0, len(buf)):
        out += bytes([buf[i]])
    assert buf == out
    return out

def get_bit(buf, index):
    byte = index // 8
    offset = index % 8
    mask = 1 << (7 - offset)

    return int(buf[byte] & mask != 0)

def get_block(buf, index):
    return buf[index * BLOCK_SIZE : (index + 1) * BLOCK_SIZE]

def inc_block(b):
    out = b''
    carry = 1
    for i in range(0, len(b)):
        byte = b[len(b) - 1 - i]
        csum = byte + carry
        out = bytes([(csum % 256)]) + out
        carry = csum // 256
    return out

def shift_right_block(b):
    out = b''
    carry_bit = 0

    for i in range(0, len(b)):
        shift = b[i] >> 1
        mask = carry_bit << 7

        out += bytes([shift | mask])

        carry_bit = b[i] & 1

    return out

def shift_left_block(b):
    out = b''
    carry_bit = 0

    for i in range(0, len(b)):
        byte = b[len(b) - i - 1]
        shift = (byte << 1) % 0x100

        out = bytes([shift | carry_bit]) + out

        carry_bit = int((byte & (1 << 7)) != 0)

    return out

def xor_block(x, y):
    assert len(x) == len(y), "%d vs %d" % (len(x), len(y))

    out = b''
    for i in range(len(x)):
        out += bytes([x[i] ^ y[i]])

    return out

GF_MULT_R = bytes([int('11100001', 2)]) + (b'\0' * (BLOCK_SIZE - 1))

# http://luca-giuzzi.unibs.it/corsi/Support/papers-cryptography/gcm-spec.pdf
def gf_mult(x, y):
    assert len(x) == BLOCK_SIZE
    assert len(y) == BLOCK_SIZE

    z = encode_int(0, BLOCK_SIZE)
    v = copy_buf(x)

    for i in range(0, 128):
        if get_bit(y, i):
            z = xor_block(z, v)

        if not get_bit(v, 127):
            v = shift_right_block(v)
        else:
            v = xor_block(shift_right_block(v), GF_MULT_R)

    return z

def gf_pow(x, y):
    return gf_fast_pow(x, y)

GF_MULT_IDENTITY = encode_int(1 << 127)

def gf_fast_pow(x, y):
    out = GF_MULT_IDENTITY
    acc = copy_buf(x)

    for i in range(0, y.bit_length()):
        if (y >> i) & 1 != 0:  # the i-th bit of y
            out = gf_mult(out, acc)
        acc = gf_mult(acc, acc)

    return out

# https://en.wikipedia.org/wiki/Finite_field_arithmetic#Multiplicative_inverse
def gf_inv(x):
    r = pow(2, 128) - 2
    return gf_fast_pow(x, r)

# https://en.wikipedia.org/wiki/Galois/Counter_Mode
# assumes cyphertext only, zero-length associated data
def raw_ghash(h, buf):
    acc = encode_int(0, BLOCK_SIZE)
    buflen = len(buf) // BLOCK_SIZE
    for i in range(0, buflen):
        acc = gf_mult(xor_block(acc, get_block(buf, i)), h)
    return acc

def ghash(h, buf):
    return raw_ghash(h, buf+encode_int(len(buf)*8))

def aes_enc_block(k, m):
    assert len(m) == BLOCK_SIZE
    assert len(k) == BLOCK_SIZE

    return Cipher(
        algorithms.AES(k),
        modes.ECB(),
        backend=default_backend(),
    ).encryptor().update(m)

def aes_dec_block(k, c):
    assert len(c) == BLOCK_SIZE
    assert len(k) == BLOCK_SIZE

    return Cipher(
        algorithms.AES(k),
        modes.ECB(),
        backend=default_backend(),
    ).decryptor().update(c)

def collide_gcm(k1, k2, n, c):
    zero = encode_int(0, BLOCK_SIZE)

    h1 = aes_enc_block(k1, zero)
    h2 = aes_enc_block(k2, zero)

    p1 = aes_enc_block(k1, inc_block(n))
    p2 = aes_enc_block(k2, inc_block(n))

    mlen = (len(c) // BLOCK_SIZE) + 1

    # length in bits encoded to block size
    len_in_bits = mlen * BLOCK_SIZE * 8
    lens = encode_int(len_in_bits, BLOCK_SIZE)

    # acc = lens * (h1 ^ h2) ^ p1 ^ p2
    acc = gf_mult(lens, xor_block(h1, h2))
    acc = xor_block(acc, p1)
    acc = xor_block(acc, p2)

    for i in range(0, mlen - 1):
        p = mlen + 1 - i
        h = xor_block(gf_pow(h1, p), gf_pow(h2, p))
        cb = get_block(c, i)
        acc = xor_block(acc, gf_mult(cb, h))

    inv = gf_inv(xor_block(gf_pow(h1, 2), gf_pow(h2, 2)))

    clast = gf_mult(acc, inv)

    cout = c + clast

    t = xor_block(ghash(h1, cout), p1)

    return (cout, t)

def partial_enc_gcm(k, n, m):

    enc = Cipher(
        algorithms.AES(k),
        modes.CTR(inc_block(inc_block(n))),
        backend=default_backend(),
    ).encryptor()

    return enc.update(m) + enc.finalize()

def rand_block():
    return os.urandom(BLOCK_SIZE)

def encode_iv(iv):
    assert len(iv) == IV_SIZE
    return iv + encode_int(0, BLOCK_SIZE - IV_SIZE)

def collide_encrypt(m):
    k1 = b'\x03'*BLOCK_SIZE
    k2 = b'\x01'*BLOCK_SIZE
    iv = b'\x02'*(96 // 8)
    n = encode_iv(iv)

    c = partial_enc_gcm(k1, n, m)

    (cout, t) = collide_gcm(k1, k2, n, c)

    return (k1, k2, iv, n, cout, t)

def encrypt_gcm(k, iv, m):
    assert len(m) % BLOCK_SIZE == 0, 'm must be multiple of block size'
    n = encode_iv(iv)

    h = aes_enc_block(k, encode_int(0, BLOCK_SIZE))
    p = aes_enc_block(k, inc_block(n))

    mlen = len(m) // BLOCK_SIZE

    c = b''
    ctr = inc_block(n)
    for i in range(0, mlen):
        ctr = inc_block(ctr)
        c += xor_block(get_block(m, i), aes_enc_block(k, ctr))

    t = xor_block(ghash(h, c), p)

    return (n, c, t)

def reference_enc_gcm(k, n, m):
    enc = Cipher(
        algorithms.AES(k),
        modes.GCM(n),
        backend=default_backend(),
    ).encryptor()

    return (n, enc.update(m) + enc.finalize(), enc.tag)

def decrypt_gcm(k, iv, c, t):
    decryptor = Cipher(
        algorithms.AES(k),
        modes.GCM(iv, t),
        backend=default_backend(),
    ).decryptor()

    return decryptor.update(c) + decryptor.finalize()


if __name__ == "__main__":
    msg = b'humpty dumpty yo'

    (k1, k2, iv, n, c, t) = collide_encrypt(msg)
    print_buf('k1', k1)
    print_buf('k2', k2)
    print_buf('iv', iv)
    print_buf('n', n)
    print_buf('c', c)
    print_buf('t', t)
    print()

    print_buf('m1', decrypt_gcm(k1, iv, c, t))
    print_buf('m2', decrypt_gcm(k2, iv, c, t))
