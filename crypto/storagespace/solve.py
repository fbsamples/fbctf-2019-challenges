#!/usr/bin/env sage
from typing import Any, Dict, Optional, Tuple
from sage.all import (
    bsgs,
    CRT_list,
    EllipticCurve,
    factor,
    GF,
)
import warnings
import logging
import hashlib
import random
import base64
import json
import sys


# ignore bsgs deprecation
warnings.filterwarnings("ignore", category=DeprecationWarning)


def pohlig(E, H, P) -> int:
    # pohlig-hellman algorithm, factor the order of the curve
    # and use bsgs to solve for each factor, then use CRT to
    # get a solution.
    bases = []
    resids = []

    for i, j in factor(E.order()):  
        e = i ** j
        logging.info(f' pohlig: {e}')
        t = E.order() // e
        tH = H * t
        tP = P * t
        dlog = bsgs(tP, tH, (0, e), '+')
        bases.append(dlog)
        resids.append(e)
    return CRT_list(bases, resids)


import socket


class Connection:
    def __init__(self) -> None:
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.connect(('localhost', 2302))
        self.buffer = b''

    def sendline(
        self,
        s: str,
    ) -> None:
        self.socket.sendall(bytes(f'{s}\n', 'utf-8'))

    def recvline(
        self,
        die_on: Optional[str] = None,
    ) -> str:
        return self.recvuntil('\n', die_on=die_on)

    def recvuntil(
        self,
        s: str,
        die_on: Optional[str] = None,
    ) -> str:
        end = bytes(s, 'utf-8')
        die_bytes = bytes(die_on, 'utf-8') if die_on else None
        die = die_bytes is not None and die_bytes in self.buffer
        while end not in self.buffer and not die:
            self.buffer += self.socket.recv(512)
            die = die_bytes is not None and die_bytes in self.buffer
        if end not in self.buffer:
            raise ValueError('did not find {} in recv'.format(s))
        result, self.buffer = self.buffer.split(end, 1)
        return (result + end).decode('utf-8')


# The following methods are just convenience
# wrappers for interacting with the challenge
# and signing requests


def do_request(
    # store: FlagStorage,
    r: Connection,
    request: Dict,
) -> str:
    # return store.handle_request(json.dumps(request, sort_keys=True))
    r.sendline(json.dumps(request, sort_keys=True))
    response = r.recvuntil('\n> ', die_on='For security,')
    return response[:-3]


def make_request(
    command: str,
    **params,
) -> Dict:
    request: Dict[str, Any] = {}
    sig = params.get('sig')
    if sig in params:
        del params['sig']
    request['command'] = command
    request['params'] = params
    if sig is not None:
        request['sig'] = sig
    return request


def make_signed(
    # store: FlagStorage,
    r: Connection,
    command: str,
    **params,
) -> Dict:
    request = {
        'command': 'sign',
        'params': {
            'command': command,
            'params': params,
        }
    }
    result = do_request(r, request)
    signed_request = json.loads(result)
    return signed_request


# The sign method comes from the challege itself
# by using the 'spec' command
def sign(
    message: str,
    priv: int,
    n: int,
    G: Any,
) -> Tuple[int, int]:
    # Curve: EllipticCurve(GF(p), [a, b])
    # with generator G
    while True:
        k = random.randint(1, n)
        Q = G * k

        hash_message = message + str(int(Q.xy()[0]))
        mhash = hashlib.sha256(bytes(hash_message, 'utf-8'))
        r = int(mhash.hexdigest(), 16)
        if r % n == 0:
            continue

        s = (k - (r * priv)) % n
        if s != 0:
            return (r, s)\


def do_offline() -> None:
    print('{"command":"sign", "params":{"command":"info"}}')
    p = int(input('p: '))
    a = int(input('a: '))
    b = int(input('b: '))
    Hxy = eval(input('H: '))
    Gxy = eval(input('G: '))

    E = EllipticCurve(GF(p), [a, b])
    H = E(Hxy)
    G = E(Gxy)
    key = pohlig(E, H, G)
    print(f'solved: {key}')

    # now we forge a signed request with the 'flag' command
    print('forging request...')
    request = make_request('flag', name='fbctf')
    message = json.dumps(request, sort_keys=True)
    R, S = sign(message, key, E.order(), G)
    sig = base64.b64encode(bytes(str(R) + '|' + str(S), 'utf-'))
    request['sig'] = sig.decode('utf-8')
    print(json.dumps(request, sort_keys=True))



if __name__ == '__main__':
    offline_mode = len(sys.argv) > 1 and sys.argv[1] == 'offline'
    if offline_mode:
        do_offline()
        exit(0)
    store = Connection()
    prompt = store.recvuntil('\n> ')
    print(prompt)

    # test that everything works by
    # adding a custom flag
    print('adding my flag', end='')
    r = do_request(store, make_signed(store, 'save', name='myflag', flag='fb{asdf}'))
    print(r)

    # and check that the flag is there
    r = do_request(store, make_signed(store, 'list'))
    print('existing flags: {}'.format(r.split('\n')))

    # this step should fail because we can't sign
    # the 'flag' command
    print('attempting to get flag')
    r = do_request(store, make_signed(store, 'flag', name='fbctf'))
    print(r)

    # so now we have to break the keypair
    print('breaking keypair...')

    # reference output for parsing the curve parameters
    # curve: y**2 = x**3 + 58282675424460415227418654738035536337*x + 144288397705739543714728904359617219152 (mod 222456561233155634483440409711692209113)
    # generator: (85586763974426059451579478976587356396, 216499470610537587062727084816152398296)
    # public key: (33061912354674209948015789486030611955, 19203293643222063800036245064339809019)
    info = do_request(store, make_signed(store, 'info'))
    print(info)
    curve_s, gen_s, pub_s = info.split('\n')
    _, ax, bmp = curve_s.split('+')
    a = int(ax.split('*')[0])
    b = int(bmp.split('(')[0])
    p = int(bmp.split('mod')[1][:-1])
    Gxy = eval(gen_s.split(': ')[1])
    assert isinstance(Gxy, tuple) and len(Gxy) == 2
    Hxy = eval(pub_s.split(': ')[1])
    assert isinstance(Hxy, tuple) and len(Hxy) == 2
    print(f'a: {a}\nb: {b}\np: {p}\ngen: {Gxy}\npub: {Hxy}')

    # done parsing curve, now use pohlig to get a priv key
    print(f'doing pohlig...')
    E = EllipticCurve(GF(p), [a, b])
    H = E(Hxy)
    G = E(Gxy)
    key = pohlig(E, H, G)
    print(f'solved: {key}')

    # now we forge a signed request with the 'flag' command
    print('forging request...')
    request = make_request('flag', name='fbctf')
    message = json.dumps(request, sort_keys=True)
    R, S = sign(message, key, E.order(), G)
    sig = base64.b64encode(bytes(str(R) + '|' + str(S), 'utf-'))
    request['sig'] = sig.decode('utf-8')
    r = do_request(store, request)
    print(r) # FLAG!
