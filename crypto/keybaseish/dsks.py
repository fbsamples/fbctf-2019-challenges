#!/usr/bin/env python3
from Crypto.Util.number import isPrime, getPrime
from functools import reduce
import random
import math
import sys


# Most of this code comes from https://github.com/GrosQuildu/CryptoAttacks
# MIT Licensed


def product(numbers):
    if len(numbers) == 0:
        return 0
    if type(numbers) == list:
        return reduce(lambda x, y: x * y, numbers)
    elif type(numbers) == dict:
        return reduce(lambda x, y: x * (y**numbers[y]), numbers, 1)
    return False


def crt(a, n):
    """Solve chinese remainder theorem
    from: http://rosettacode.org/wiki/Chinese_remainder_theorem#Python
    The solution will be modulo product of modules
    Args:
        a(list): remainders
        n(list): modules
    Returns:
        int: solution to crt
    """
    if len(a) != len(n):
        print("Different number of remainders({}) and modules({})".format(len(a), len(n)))

    prod = product(n)
    sum_crt = 0

    for n_i, a_i in zip(n, a):
        p = prod // n_i
        sum_crt += a_i * invmod(p, n_i) * p
    return int(sum_crt % prod)


def egcd(*args):
    """Extended Euclidean algorithm"""
    if len(args) < 2:
        print("Give at least two values")

    if len(args) == 2:
        a, b = args
        s0, t0, s1, t1 = 1, 0, 0, 1
        while b:
            q, a, b = a//b, b, a%b
            s0, s1 = s1, s0 - q*s1
            t0, t1 = t1, t0 - q*t1
        return a, s0, t0
    else:
        d, s, t = egcd(args[0], args[1])
        coefficients = [s, t]
        for i in range(2, len(args)):
            d, s, t = egcd(d, args[i])
            for j in range(len(coefficients)):
                coefficients[j] *= s
            coefficients.append(t)
        coefficients.insert(0, d)
        return coefficients


def invmod(a, n):
    """Modular inverse. a*invmod(a) == 1 (mod n)"""
    d, s, t = egcd(a, n)
    if d != 1:
        raise ValueError("Modular inverse doesn't exists ({}**(-1) % {})".format(a, n))
    return s % n


def generate_smooth_prime(bit_size, primitive_roots=[], smooth_bit_size=50, exclude=[]):
    """Generate smooth prime n
    Args:
        bit_size(int): size of generated prime in bits
        primitive_roots(list(int)): list of numbers that will be primitive roots modulo n
        smooth_bit_size(int): most factors of n-1 will be of this bit size   
        exclude(list(int)): n-1 won't have any factor from that list
    Returns:
        int: n
    """
    while True:
        n = 2
        factors = {2:1}

        # get random primes of correct size
        print('smooth prime - loop of size about {}'.format((bit_size - 2*smooth_bit_size)//smooth_bit_size))
        while n.bit_length() < bit_size - 2*smooth_bit_size:
            q = getPrime(smooth_bit_size)
            if q in exclude:
                continue
            n *= q
            if q in factors:
                factors[q] += 1
            else:
                factors[q] = 1

        # find last prime so that n+1 is prime and the size is correct
        smooth_bit_size_padded = bit_size - n.bit_length()
        print('smooth prime - smooth_bit_size_padded = {}'.format(smooth_bit_size_padded))
        while True:
            q = getPrime(smooth_bit_size_padded)
            if q in exclude:
                continue
            if isPrime((n*q)+1):
                n = (n*q)+1
                if q in factors:
                    factors[q] += 1
                else:
                    factors[q] = 1
                break
        
        # check if given numbers are primitive roots
        print('smooth prime - checking primitive roots')
        are_primitive_roots = True
        if len(primitive_roots) > 0: 
            for factor, factor_power in factors.items():
                for primitive_root in primitive_roots:
                    if pow(primitive_root, (n-1)//(factor**factor_power), n) == 1:
                        are_primitive_roots = False
                        break

        if are_primitive_roots:
            print('smooth prime - done')
            return n, factors
        else:
            print('primitive roots criterion not met')



def babystep_giantstep(g, h, p, upper_bound):
    m = int(math.ceil(math.sqrt(upper_bound)))
    print('babystep-giantstep with loops of size {}'.format(m))
    g_j = {}
    g_j_tmp = 1
    for j in range(m):
        g_j[g_j_tmp] = j
        g_j_tmp = (g_j_tmp*g) % p

    g_m = invmod(pow(g, m, p), p)
    y = h
    for i in range(m):
        if y in g_j:
            return (i*m + g_j[y]) % p
        y = (y*g_m) % p


def pohlig_hellman(g, h, n, n_order_factors):
    """ Pohlig-Hellman discrete logarithm method (with babystep-giantstep)

            g^x == h % n

        Args:
            g, h, n(int)
            n_order_factors(dict): factors of n's order (euler_phi(n))

        Returns:
            int: x
    """
    no = product(n_order_factors)
    xi = []
    ci, loop_size = 1, len(n_order_factors.keys())
    for pi, ei in n_order_factors.items():
        print('Pohlig-Hellman iteration {}/{}'.format(ci, loop_size))
        ci += 1

        gi = pow(g, no//(pi**ei), n)  # gi have order pi**ei
        hi = pow(h, no//(pi**ei), n)  # hi is in <gi>

        xk = 0
        gamma = pow(gi, pi**(ei-1), n)  # gamma has order pi
        for k in range(ei):
            hk = invmod(pow(gi, xk, n), n)
            hk = pow(hk * hi, pi**(ei-1-k), n)  # hk is in <gamma>
            dk = babystep_giantstep(gamma, hk, n, pi)
            if dk is None:
                return None
            xk = (xk + pow(pi, k, pi**ei)*dk) % (pi**ei)
        xi.append(xk)
        
    return crt(xi, [pi**ei for pi, ei in n_order_factors.items()])


def dsks(message, signature, n, smooth_bit_size=30, hash_function=None):
    """ Duplicate-Signature Key Selection on RSA Create key pair verifies given signature
            signature^e == hash_function(message) % n
        So if we have someone's public key (with n) and signature s of some message signed
        with corresponding private key we can create new key pair that will verify the signature,
        BUT new e will be large Can also be used stuff like generating key pair that will
        decrypt given message to choosen plaintext

        Args:
            message(int/arg for hash_function)
            signature(int)
            n(int)
            smooth_bit_size(int): to tweak, most factors of p-1 and q-1 will be of this bit size 
            hash_function(NoneType/callable): converting message to int

        Returns:
            tuple(int): n', factors of p'-1, factors of q'-1, e', d'
    """
    m = message
    s = signature

    key_size = n.bit_length() + 1
    while True:
        p, p_order_factors = generate_smooth_prime(
                key_size//2,
                primitive_roots=[m, s],
                smooth_bit_size=smooth_bit_size,
        )

        q, q_order_factors = generate_smooth_prime(
                key_size - p.bit_length() + 1,
                primitive_roots=[m, s],
                smooth_bit_size=smooth_bit_size,
                exclude=p_order_factors,
        )

        n_p = p*q
        if n_p > n:
            print("n generated")
            print("n' = {}".format(n_p, n_p.bit_length()))
            print("p' = {}".format(p, p_order_factors))
            print("q' = {}".format(q, q_order_factors))

            ep = pohlig_hellman(s, m, p, p_order_factors)
            eq = pohlig_hellman(s, m, q, q_order_factors)
            print("ep' = {}".format(ep))
            print("eq' = {}".format(eq))

            e = crt([ep, eq], [p-1, (q-1)//2])
            print("e' = {}".format(e))

            d = invmod(e, (p-1)*(q-1))
            print("d' = {}".format(d))
            return n_p, p_order_factors, q_order_factors, e, d
        else:
            print('nope', float(n_p) / float(n))


if __name__ == '__main__':
    if len(sys.argv) < 2:
        # do whole challenge
        from solution import *
        import requests
        import re

        t = get_recover_page()
        pin = parse_challenge(t)
        print('pin: {}'.format(pin))
        csrf_token = parse_csrf_token(t)
    else:
        # otherwise, just solve the crypto
        from solution import EXISTING_SIG
        pin = int(sys.argv[1])

    n_p, p_f, q_f, e, d = dsks(pin, int(EXISTING_SIG), random.randint(2**1023, 2**2024))
    pubkey = '{}:{}'.format(e, n_p)

    if len(sys.argv) < 2:
        temppass = get_password(pubkey, csrf_token)
        t = get_login_page()
        csrf_token = parse_csrf_token(t)
        flag = get_flag(temppass, csrf_token)
        print('flag: {}'.format(flag))
    else:
        print(pubkey)

