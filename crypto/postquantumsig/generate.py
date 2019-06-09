import hashlib
import secrets
import csv
import random
import itertools
import binascii


def s256(msg):
    # string hex sha256 of a string or hex string message
    try:
        return hashlib.sha256(binascii.unhexlify(msg)).hexdigest()
    except:
        return hashlib.sha256(msg.encode('utf-8')).hexdigest()


def gen_key():
    # generate a random 256 bit (i.e. 32 byte) secret
    return secrets.token_hex(32)


def make_leaf():
    # leaf nodes have a special structure with only 1 child
    base = gen_key()
    return (s256(base), (base,))


def join_pair(a, b):
    # given two binary trees, return a new binary tree with a new parent
    # the two trees supplied with be the children
    return (s256(a[0] + b[0]), (a, b))


def definite_chain(a):
    # helper to unspool iterable/generators
    return [i for i in itertools.chain(*a)]


def make_tree(depth):
    # recursively build a tree of desired depth.
    if depth <= 1:
        return make_leaf()
    else:
        return join_pair(make_tree(depth - 1), make_tree(depth - 1))


def kth_bit(n, k):
    # flag of the kth bit
    # e.g. k(8,1) == 0; k(8,3) == 1
    return 1 if n & (1 << (k)) else 0


def opp(i):
    # helper function to force 0/1 behavior
    return 0 if i else 1


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


def sign_stream_from_tree(tree):
    # given a binary private key tree, give a stream of pairs of special leaves
    # this is helpful because of how lamport signatures pick signatures
    # bit-wise
    stream = []

    def gen_sign_stream(tree, stream):
        # i wish I could get this to work with yield
        if len(tree[1]) != 2:
            stream.append(tree)
        else:
            for subtree in tree[1]:
                gen_sign_stream(subtree, stream)
    gen_sign_stream(tree, stream)
    return group_by_n(stream)


def sign_with_streams(bit_stream, sign_stream):
    # given a message bit stream, and a stream of signatures, make a signature
    return_stream = []
    for bit, sign in zip(bit_stream, sign_stream):
        if bit:
            return_stream.append(sign[0][0])
            return_stream.append(sign[1][1][0])
        else:
            return_stream.append(sign[0][1][0])
            return_stream.append(sign[1][0])
    return return_stream


def get_tree_height(tree):
    # helper function to test how tall a tree is
    if len(tree[1]) != 2:
        return 1
    return 1 + get_tree_height(tree[1][0])


def pick_tree_segment(tree, segment, segment_height):
    # creates a numbering convention for 'segments' of trees
    # by using a constant segment height and each segment only once
    # it prevents private key resuse
    depth = get_tree_height(tree) - segment_height
    if segment >= depth**2:
        raise ValueError('segment out of range: {segment}')
    subtree = tree
    other_hashes = []
    for i in range(depth):
        k = kth_bit(segment, i)
        other_hashes.append((k, subtree[1][opp(k)][0]))
        subtree = subtree[1][k]
    return (subtree, other_hashes[::-1])


def make_signed_message(msg, tree, sig_no):
    # put it all together so that a single top level identity can sign many
    # messages without ever resuing a specific one time use leaf
    s_tree, others = pick_tree_segment(tree, sig_no, 10)
    h_msg = s256(msg)
    sign_stream = sign_stream_from_tree(s_tree)
    bit_stream = [i for i in bit_stream_from_msg(h_msg)]
    signature = sign_with_streams(bit_stream, sign_stream)
    return [tree[0], msg] + signature + definite_chain(others)


def make_a_msg(sender, recipient):
    # make the stylized transaction message
    amount = random.random() + random.randint(16, 123)
    return f'{sender} sent {amount} zuccoins to {recipient}'


def write_records(players, filename):
    # given a list of players, write records
    player_names = set([player[0] for player in players])
    rows = []
    for player in players:
        for i in range(random.randint(4, 14)):
            sender = player[0]
            msg = make_a_msg(sender, secrets.choice(
                list(player_names - set((sender,)))))
            if sender == players[0][0]:  # this is the security flaw
                rows.append(make_signed_message(msg, player, 0))
            else:
                rows.append(make_signed_message(msg, player, i))
    random.shuffle(rows)  # to make it a little less obvious
    with open(filename, 'w') as outfile:
        outcsv = csv.writer(outfile)
        for row in rows:
            outcsv.writerow(row)


def main():
    players = [make_tree(random.randint(14, 16)) for _i in range(8)]
    filename = 'dist/signatures.csv'
    write_records(players, filename)

if __name__ == '__main__':
    main()
