#!/usr/bin/python3

import binascii
import csv
import hashlib
import sys

SIGNATURE_PATH = '/home/postquantumsig/signatures.csv'

def s256(msg):
    # string hex sha256 of a string or hex string message
    try:
        return hashlib.sha256(binascii.unhexlify(msg)).hexdigest()
    except:
        return hashlib.sha256(msg.encode('utf-8')).hexdigest()


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


def make_top_hash_from_leaves(tree):
    # combine an ordered list key pairs to a top level public key.
    if len(tree) < 2:
        return tree[0]
    else:
        return make_top_hash_from_leaves(
            [s256(a + b) for a, b in group_by_n(tree)]
        )


def make_top_hash_from_others(initial, others):
    # traverse asymmetic segments of public key signature to get public key
    top_hash = initial
    for rank, other in others:
        if int(rank):
            top_hash = s256(other + top_hash)
        else:
            top_hash = s256(top_hash + other)
    return top_hash


def msg_to_hashes(msg, signature):
    # turn a message with signature into an ordered list of key pairs
    bit_stream = bit_stream_from_msg(msg)
    sign_stream = group_by_n(signature, 2)
    return_stream = []
    for bit, sign in zip(bit_stream, sign_stream):
        if bit:
            return_stream.append(sign[0])
            return_stream.append(s256(sign[1]))
        else:
            return_stream.append(s256(sign[0]))
            return_stream.append(sign[1])
    return return_stream


def parse_signed_message(msg_w_signature):
    # separate a formatted message and signature into components
    top_identity = msg_w_signature[0]
    msg = msg_w_signature[1]
    h_msg = s256(msg)
    signature = msg_w_signature[2:514]
    others = group_by_n(msg_w_signature[514:])
    return (top_identity, h_msg, signature, others)


def msg_internal_validity(msg, identity):
    # test zuccoin transaction message validity
    msg_list = msg.split(' ')
    return msg_list[0] == identity and \
        msg_list[1] == 'sent' and \
        float(msg_list[2]) < 500 and \
        msg_list[3] == 'zuccoins' and \
        msg_list[4] == 'to' and \
        len(msg_list[5]) == 64 and \
        len(msg_list) == 6


def verify_signed_message(msg_w_signature):
    # return the public key used to sign a message
    identity, h_msg, signature, others = parse_signed_message(msg_w_signature)
    initial = make_top_hash_from_leaves(msg_to_hashes(h_msg, signature))
    top = make_top_hash_from_others(initial, others)
    return (identity, top)


def get_player_names(filename):
    # read a file to find all of the valid addresses
    player_names = set()
    with open(filename, 'r') as infile:
        incsv = csv.reader(infile)
        for row in incsv:
            player_names.add(row[0])
    return player_names


def verify_problem_solution(row, player_names):
    # verifies the row and reveals the flag if valid
    # otherwise tries to give a helpful error message
    # could be reworked to instead return the string and have try blocks
    msg = row[1]
    id, pubkey = verify_signed_message(row)
    msg_valid = msg_internal_validity(msg, id)
    recipient = msg.split(' ')[5]
    sender_ok = True if id in player_names else False
    recipient_ok = False if recipient in player_names else True
    if (id != pubkey):
        return f'id does not match pubkey: {id} {pubkey}'
    elif not msg_valid:
        return f'bad transaction message format: {id} {msg}'
    elif not sender_ok:
        return f'transaction not from known sender: {id}'
    elif not recipient_ok:
        return f'transaction is to known recipient: {recipient}'
    else:
        return 'fb{reduce_reuse_recycle_is_bad_advice_for_ots}'


def test():
    # test some example cases
    # the final version should accept rows from the submission mechanism
    # and probably needs some try blocks since this will fail if the row
    # isn't in the correct format
    player_names = get_player_names(SIGNATURE_PATH)
    with open(SIGNATURE_PATH, 'r') as goodfile:
        goodcsv = csv.reader(goodfile)
        for row in goodcsv:
            verify_problem_solution(row, player_names)  # should succeed
    with open(SIGNATURE_PATH, 'r') as badfile:
        badcsv = csv.reader(badfile)
        for row in badcsv:
            verify_problem_solution(row, player_names)  # should fail
    with open(SIGNATURE_PATH, 'r') as basefile:
        basecsv = csv.reader(basefile)
        for row in basecsv:
            verify_problem_solution(row, player_names)  # should all fail


def main():
    player_names = get_player_names(SIGNATURE_PATH)
    try:
        raw_row = input('Enter signed transaction row: ')
        reader = csv.reader((raw_row,))
        row = next(reader)
        print('\n')
        print(verify_problem_solution(row, player_names))
    except:
        print('Failed to parse message. Make sure you have the right number of columns and structure. Also, try piping in directly, pasting may truncate.')
        try:
            print(f'Input message received: {raw_row}')
        except:
            pass
    quit()

if __name__ == '__main__':
    main()
