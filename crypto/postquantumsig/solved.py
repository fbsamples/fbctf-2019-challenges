import hashlib
import csv
import random
import itertools
import collections
import sys
import binascii


def s256(msg):
    # string hex sha256 of a string or hex string message
    try:
        return hashlib.sha256(binascii.unhexlify(msg)).hexdigest()
    except:
        return hashlib.sha256(msg.encode('utf-8')).hexdigest()


def join_pair(a, b):
    # given two binary trees, return a new binary tree with a new parent
    # the two trees supplied with be the children
    return (s256(a[0] + b[0]), (a, b))


def kth_bit(n, k):
    # flag of the kth bit
    # e.g. k(8,1) == 0; k(8,3) == 1
    return 1 if n & (1 << (k)) else 0


def group_by_n(s, n=2):
    # takes a list or tuple and chunks it up into pairs or other n
    # e.g. (1,2,3,4,5,6) -> ((1,2),(3,4),(5,6))
    return [s[i:i + n] for i in range(0, len(s), n)]


def bit_stream_from_msg(msg):
    # given a hex string msg, generate a stream of 1 and 0
    # e.g. 'e8' -> (0,0,0,1,0,1,1,1)
    newmsg = int(msg, 16)
    for i in range(4 * len(msg))[::-1]:
        yield kth_bit(newmsg, i)


def definite_chain(a):
    # helper to unspool iterable/generators
    return [i for i in itertools.chain(*a)]


def parse_signed_message(msg_w_signature):
    # separate a formatted message and signature into components
    top_identity = msg_w_signature[0]
    msg = msg_w_signature[1]
    h_msg = s256(msg)
    signature = msg_w_signature[2:514]
    others = group_by_n(msg_w_signature[514:])
    return (top_identity, h_msg, signature, others)


def make_a_msg(sender, recipient):
    # make the stylized transaction message
    amount = random.random() + random.randint(16, 123)
    return f'{sender} sent {amount} zuccoins to {recipient}'


def find_weak_player(filename):
    # figure out which player has reused their key
    weak_others = collections.defaultdict(int)
    other_to_players = {}
    with open(filename, 'r') as infile:
        incsv = csv.reader(infile)
        for row in incsv:
            identity, msg, sign, others = parse_signed_message(row)
            weak_others[f'{others}'] += 1
            other_to_players[f'{others}'] = identity
        max_other = ''
        max_cnt = 0
        for other, cnt in weak_others.items():
            if cnt > max_cnt:
                max_other = other
                max_cnt = cnt
        return(other_to_players[max_other])


def collect_weak_bits(filename, weak_player):
    # collet how many bits have been reused
    weak_bits = collections.defaultdict(dict)
    weak_other = []
    with open(filename, 'r') as infile:
        incsv = csv.reader(infile)
        for row in incsv:
            identity, msg, sign, others = parse_signed_message(row)
            if identity == weak_player:
                weak_other = others
                bit_stream = bit_stream_from_msg(msg)
                sign_stream = group_by_n(sign, 2)
                for i, bit, sign in zip(range(256), bit_stream, sign_stream):
                    weak_bits[i][bit] = sign
        bit = [bit for bit, bitvals in weak_bits.items() if len(bitvals) > 1]
        print(f'# of weak bits: {len(bit)}', file=sys.stderr)
        return (weak_bits, weak_other)


def sign_with_weak_bits(sender, msg, bits, others):
    # create the signature with the weak bits
    h_msg = s256(msg)
    bit_stream = [i for i in bit_stream_from_msg(h_msg)]
    signature = []
    for i in range(256):
        g = bits[i][bit_stream[i]]
        signature.append(g[0])
        signature.append(g[1])
    return [sender, msg] + signature + definite_chain(others)


def sign_bad_message(filename):
    # find the weak player, collect the weak bits, and then use them to sign
    weak_player = find_weak_player(filename)
    weak_bits, weak_others = collect_weak_bits(filename, weak_player)
    recipient = ''.join('f' for _i in range(64))
    amount = 123 + random.random()
    msg = f'{weak_player} sent {amount} zuccoins to {recipient}'
    row = sign_with_weak_bits(weak_player, msg, weak_bits, weak_others)
    return(row)


def main():
    filename = 'dist/signatures.csv'
    row = sign_bad_message(filename)
    print(','.join(row))

if __name__ == '__main__':
    main()
