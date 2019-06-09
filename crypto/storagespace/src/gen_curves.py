#!/usr/bin/env sage
from sage.all import (
    divisors,
    EllipticCurve,
    factor,
    GF,
    random_prime,
)
import threading
import socket
import random
import base64
import queue
import time
import json
import sys
import os


MAX_BITS_FOR_FACTOR = 40
MIN_GENERATOR_DIVISOR = 10000


def gen_generator(E):
    """generates a random generator for given curve E"""
    N = E.order()
    choices = [d for d in divisors(N) if d > MIN_GENERATOR_DIVISOR]
    if len(choices) == 0:
        raise ValueError('bad curve, only small divisors :(')
    n = random.choice(choices)
    h = N // n

    while True:
        P = E.random_point()
        G = P * h
        if G != E(0, 1, 0):
            return G


def gen_random_curve(bits):
    """generates a random curve with prime of given bit length"""
    p = random_prime(2**bits, proof=True, lbound=2**(bits-1))
    while True:
        a = random.randint(1, p-1)
        b = random.randint(1, p-1)
        if ((4*(a**3)) + (27*(b**2))) % p != 0:
            F = GF(p)
            E = EllipticCurve(F, [a, b])
            return (E, F)


def gen_unsafe_curve(bits, attempts=-1):
    """generates a random curve that has only small factors"""
    while attempts != 0:
        E, F = gen_random_curve(bits)
        factors = [ x for x,y in factor(E.order()) ]
        bit_lengths = [ int(f).bit_length() for f in factors ]
        if all(l <= MAX_BITS_FOR_FACTOR for l in bit_lengths):
            a = E.ainvs()[3]
            b = E.ainvs()[4]
            p = F.cardinality()
            P = gen_generator(E)
            return a, b, p, P.xy()
        if attempts > 0:
            attempts -= 1


def format_curve(a, b, p, P):
    curve = repr((a, b, p, P))
    return str(curve)


def cmd_gen(args):
    filename = args.outfile

    def writeline(o, m):
        if not args.quiet:
            print(m)
        o.write(m + '\n')

    if os.path.exists(filename) and not args.no_append:
        outfile = open(filename, 'r+')
        if not args.quiet:
            print('appending to existing file')
    else:
        outfile = open(filename, 'w')

    while True:
        try:
            formatted = format_curve(*gen_unsafe_curve(128))
            writeline(outfile, formatted)
        except KeyboardInterrupt:
            break
        except Exception as e:
            print('error: {}'.format(e))
            break


def cmd_order(args):
    """append curve order to the file, oops"""
    with open(args.file) as f:
        lines = f.readlines()

    i = 0
    with open(args.file + '.orders', 'w') as f:
        for line in lines:
            i += 1
            print(f'{i:04d}', end='\r')
            line = line.strip()
            a, b, p, P = eval(line)
            E = EllipticCurve(GF(p), (a, b))
            Po = E(P).order()
            o = E.order()
            f.write(repr((a, b, p, o, P, Po)) + '\n')


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(title='subcommands', description='valid subcommands', dest='subcommand')

    # gen command
    cmdparser = subparsers.add_parser('gen')
    cmdparser.add_argument(
        'outfile',
        type=str,
        nargs='?',
        default='curves.txt',
        help='file to store new curves',
    )
    cmdparser.add_argument(
        '--no-append',
        action='store_true',
        help='don\'t attempt to preserve existing file',
    )
    cmdparser.add_argument(
        '--quiet',
        action='store_true',
        help='silently write to file',
    )
    cmdparser.set_defaults(func=cmd_gen)

    # gen orders command
    cmdparser = subparsers.add_parser('order')
    cmdparser.add_argument(
        'file',
        type=str,
        help='file to convert',
    )
    cmdparser.set_defaults(func=cmd_order)

    args = parser.parse_args()
    if args.subcommand is not None:
        res = args.func(args)
    else:
        parser.parse_args(['-h'])
