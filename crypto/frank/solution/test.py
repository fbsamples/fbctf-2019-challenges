#!/usr/bin/env python

from collide import *


def test_gf_mult_distributive():
    x = rand_block()
    y = rand_block()
    z = rand_block()

    # x * ( y + z )
    o1 = gf_mult(x, xor_block(y, z))
    # x*y + x*z
    o2 = xor_block(gf_mult(x,y), gf_mult(x,z))

    assert o1 == o2, (x, y, z)

def test_gf_mult_commutative():
    x = rand_block()
    y = rand_block()
    o1 = gf_mult(x, y)
    o2 = gf_mult(y, x)
    assert o1 == o2, (x, y)

def test_gf_mult_associative():
    x = rand_block()
    y = rand_block()
    z = rand_block()

    # (x * y) * z
    o1 = gf_mult(gf_mult(x, y), z)

    # x * (y * z)
    o2 = gf_mult(x, gf_mult(y, z))

    assert o1 == o2, (x, y ,z)

def test_gf_inv():
    x = rand_block()
    xinv = gf_inv(x)
    o1 = gf_mult(x, xinv)
    o2 = gf_mult(xinv, x)

    one = GF_MULT_IDENTITY

    assert o1 == one and o2 == one, (x, xinv, o1, o2)

def test_gf_mult_identity():
    x = rand_block()
    one = GF_MULT_IDENTITY

    o1 = gf_mult(x, one)
    o2 = gf_mult(one, x)
    assert o1 == x and o2 == x, (x, o1, o2)

def test_shift_right_block():
    b = os.urandom(4)
    x = int.from_bytes(b, byteorder='big', signed=False)

    o1 = int.from_bytes(shift_right_block(b), byteorder='big', signed=False)
    o2 = x >> 1
    assert o1 == o2, (b, x)

def test_shift_left_block():
    b = os.urandom(4)
    x = int.from_bytes(b, byteorder='big', signed=False)

    o1 = int.from_bytes(shift_left_block(b), byteorder='big', signed=False)
    o2 = (x <<  1) % 0x100000000
    assert o1 == o2, (b, x)

def test_encrypt_gcm():
    msg = rand_block()
    k = rand_block()
    iv = os.urandom(96 // 8)
    (n, c, t) = encrypt_gcm(k, iv, msg)
    (rn, rc, rt) = reference_enc_gcm(k, iv, msg)

    assert rc == c, (k, iv, msg)



if __name__ == "__main__":
    for _ in range(100):
        test_gf_mult_distributive()
        test_gf_mult_commutative()
        test_gf_mult_associative()
        test_shift_right_block()
        test_shift_left_block()
        test_gf_mult_identity()

    for _ in range(10):
        test_encrypt_gcm()
        test_gf_inv()

    print('All tests passed')
