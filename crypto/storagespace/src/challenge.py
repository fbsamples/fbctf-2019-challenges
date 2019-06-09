#!/usr/bin/env python2
from datetime import datetime
from sympy.ntheory import factorint
from fastecdsa.curve import Curve
import hashlib
import socket
import random
import base64
import json
import time
import sys
import re


FLAG_FILE = 'flag'
PRECOMPUTED_CURVES = []

# RATE_LIMIT = 5 # seconds
# RATE_EXTEND_LIMIT = 3

#
# These 3 strings are accessible through the challenge
# server so that it's easy for players to match my
# request signatures.
#

# attaching signature to request
REQUEST_SPEC = '''\
request = {
    'command': 'help',
    'params': {
        'command': 'info'
    }
}
text = json.dumps(request, sort_keys=True)
r, s = sign(text, privkey)
sig = base64.b64encode((str(r) + '|' + str(s)).encode())
request['sig'] = sig.decode('utf-8')\
'''


# signing a message
SIGN_SPEC = '''\
def sign(message, key):
    # n: curve order
    # G: generator
    while True:
        k = random.randint(1, n)
        Q = G * k

        hash_message = message + str(int(Q.x))
        mhash = hashlib.sha256(hash_message)
        r = int(mhash.hexdigest(), 16)
        if r % n == 0:
            continue

        s = (k - (r * key)) % n
        if s != 0:
            return (r, s)\
'''


# verifying a signature
VERIFY_SPEC = '''\
def verify(message, signature):
    # n: curve order
    # G: generator
    # H: public key
    r, s = signature
    if r < 0 or s < 1 or s > n - 1:
        return False

    Q = (G * s) + (H * r)
    if Q == 0:
        return False

    hash_message = message + str(int(Q.x))
    mhash = hashlib.sha256(hash_message)
    v = int(mhash.hexdigest(), 16)
    return v == r\
'''


class Challenge:
    """class for the actual ecc challenge

    Generates a random unsafe curve and key pair, then allows
    signing and verifying signatures for any message string.

    Attributes:
        field (GF): finite field of prime p
        curve (EllipticCurve): the curve
        generator (Point): generator point on curve
        priv (int): private key
        pub (Point): public key
    """

    def __init__(self):
        a, b, p, o, G, Go = random.choice(PRECOMPUTED_CURVES)
        self._a = a
        self._b = b
        self._p = p

        self.curve = Curve(
            'generic curve',
            p,
            a,
            b,
            Go,
            G[0],
            G[1]
        )
        self.curve_order = o
        self.generator_order = Go
        self.generator = self.curve.G
        self.priv = self.gen_private_key()
        self.pub = self.generator * self.priv

    def gen_private_key(self):
        key_bound = 1
        largest_bits = 0
        for f, e in factorint(self.curve_order).items():
            bits = int(f).bit_length()
            largest_bits = max(largest_bits, bits)
            key_bound = max(key_bound, f)
        l = random.randint(1, min(key_bound, self.curve_order))
        return l

    def sign(
        self,
        message,
        priv = None,
    ):
        """signs a message, optionaly provide a privkey"""
        priv = priv or self.priv
        n = self.curve_order
        while True:
            k = random.randint(1, n)
            Q = self.generator * k

            hash_message = message + str(int(Q.x))
            mhash = hashlib.sha256(hash_message)
            r = int(mhash.hexdigest(), 16)
            if r % n == 0:
                continue

            s = (k - (r * priv)) % n
            if s != 0:
                return (r, s)

    def verify(
        self,
        message,
        signature,
    ):
        """verify a message signature"""
        r, s = signature
        n = self.curve_order
        if r < 0 or s < 1 or s > n - 1:
            return False

        Q = (self.generator * s) + (self.pub * r)
        if Q == self.generator.IDENTITY_ELEMENT:
            return False

        hash_message = message + str(int(Q.x))
        mhash = hashlib.sha256(hash_message)
        v = int(mhash.hexdigest(), 16)
        return v == r


class InvalidRequestError(Exception):
    pass


class FlagStorage:
    """flag storage service wrapper around Challenge

    Attributes:
        challenge (Challenge): the ecc challenge
        unrestricted (List[str]): list of unrestricted commands
        flags (Dict[str, str]): flag store
    """

    def __init__(self):
        self.challenge = Challenge()
        with open(FLAG_FILE) as f:
            self.flag = f.read()
        self.unrestricted = [
            'command_help',
            'command_sign',
            'command_bad',
        ]
        self.flags = {
            'fbctf': self.flag,
        }

    def handle_request(
        self,
        request_text,
    ):
        """handles a json request"""
        try:
            try:
                request = json.loads(request_text)
            except ValueError:
                raise InvalidRequestError()
            command = request.get('command') or 'help'
            params = request.get('params') or {}

            # do unrestricted (no sig) commands
            handler_name = 'command_{}'.format(command)
            if not hasattr(self, handler_name):
                return 'invalid request'

            handler = getattr(self, handler_name)
            if handler_name in self.unrestricted:
                return handler(params)

            # otherwise, verify the signature
            encoded = request.get('sig')
            if encoded is None:
                return 'signature required'

            decoded = base64.b64decode(encoded).decode('utf-8')
            try:
                signature = tuple(map(int, decoded.split('|')))
                assert len(signature) == 2
            except ValueError:
                return 'invalid signature'

            del request['sig']
            sig_message = json.dumps(request, sort_keys=True)
            valid = self.challenge.verify(sig_message, signature[:2]) # type: ignore
            if not valid:
                return 'invalid signature'
            if handler == self.command_flag:
                print('accessing flag command: ' + request_text)
            return handler(params)
        except InvalidRequestError:
            return 'invalid request'
        except Exception as e:
            # print(e)
            return 'invalid request'

    #
    # Unrestricted Commands
    #

    def command_help(
        self,
        params,
    ):
        '''
        help(command: Optional[str])
        Displays help information for supported commands.
        Please provide the optional 'command' parameter
        to specify which command you would like to know
        more about. Without this parameters, you get this
        this nice list of commands:
            - help
            - sign
            - info
            - spec
            - flag
            - save
            - list
        '''
        command = params.get('command')
        if command is not None:
            method_name = 'command_{}'.format(command)
            if hasattr(self, method_name):
                method = getattr(self, method_name)
                if hasattr(method, '__doc__'):
                    return method.__doc__
        return self.command_help.__doc__ or ''

    def command_sign(
        self,
        params,
    ):
        '''
        sign(command: str, params: Optional[Dict])
        Generate a signed request for the given command.
        Many commands can only be executing through a
        signed request, but we will not sign flag(). If
        you need access to the flag, please sign the
        request manually. Authorized users will know
        the signing key.
        '''
        command = params.get('command')
        new_params = params.get('params') or {}
        if command is not None and isinstance(new_params, dict):
            handler_name = 'command_{}'.format(command)
            if command == 'flag' or not hasattr(self, handler_name):
                command = 'bad'
            request = {
                'command': command,
                'params': new_params,
            }
            text = json.dumps(request, sort_keys=True)
            r, s = self.challenge.sign(text)
            sig = base64.b64encode((str(r) + '|' + str(s)).encode())
            request['sig'] = sig.decode('utf-8')
            return json.dumps(request, sort_keys=True)
        raise InvalidRequestError()

    #
    # Restricted Commands
    #

    def command_flag(
        self,
        params,
    ):
        '''
        flag(name: Optional[str])
        Retrieve flag by name.
        '''
        flag = params.get('name') or 'fbctf'
        if flag in self.flags:
            return self.flags[flag]
        return 'flag does not exist'

    def command_info(
        self,
        params,
    ):
        '''
        info()
        Get information about the current encryption setup.
        For maximum security, this info will change with
        every  session!
        '''
        a = self.challenge._a
        b = self.challenge._b
        p = self.challenge._p
        generator = self.challenge.generator
        pubkey = self.challenge.pub
        return '\n'.join([
            'curve: y**2 = x**3 + {}*x + {} (mod {})'.format(a, b, p),
            'generator: ({}, {})'.format(generator.x, generator.y),
            'public key: ({}, {})'.format(pubkey.x, pubkey.y),
        ])

    def command_spec(
        self,
        params,
    ):
        '''
        spec(mode: Optional[str])
        Returns a specification for the request
        signature scheme. Helpful for developers
        who need to integrate with our service.

        provide parameter 'mode' as one of:
            sign, verify, request, all
        '''
        mode = params.get('mode') or 'all'
        if mode == 'sign':
            return SIGN_SPEC
        elif mode == 'verify':
            return VERIFY_SPEC
        elif mode == 'request':
            return REQUEST_SPEC
        elif mode == 'all':
            return '\n'.join([
                SIGN_SPEC,
                '---',
                VERIFY_SPEC,
                '---',
                REQUEST_SPEC,
            ])
        return 'please use mode = sign|verify|request|all'

    def command_save(
        self,
        params,
    ):
        '''
        save(name: str, flag: str)
        Store flag.
        '''
        name = params.get('name')
        flag = params.get('flag')
        if isinstance(name, basestring) and isinstance(flag, basestring):
            override = name in self.flags
            self.flags[name] = flag
            if override:
                return 'flag overwritten'
            return 'flag stored'
        return 'missing params name/flag'

    def command_list(
        self,
        params,
    ):
        '''
        list()
        List contents of flag store
        '''
        return '\n'.join(self.flags.keys())

    def command_bad(
        self,
        params,
    ):
        '''
        bad()
        a NOP command
        '''
        return 'bad command!'


# Forking Server
import SocketServer
import signal


TIME_LIMIT = 2 * 60 # seconds


class ChallengeServer(SocketServer.BaseRequestHandler, object):

    def send(self, m):
        self.request.setblocking(0)
        return self.request.sendall(m)

    def sendline(self, m):
        return self.send(m + '\n')

    def recv(self, n):
        self.request.setblocking(1)
        self.request.settimeout(TIME_LIMIT)
        data = self.request.recv(n)
        return data.decode('utf-8')

    def handle(self):
        random.seed() # random state persists over fork
        end_time = time.time() + TIME_LIMIT
        try:
            self.challenge(end_time)
        except socket.timeout:
            self.timeout_message()
        except Exception as e:
            print('other error: {}'.format(e))
        finally:
            self.request.close()

    def timeout_message(self):
        self.sendline('\nFor security, all sessions are')
        self.sendline('limited to 2 minutes. Goodbye!')
        print('challenge timeout')

    def challenge(self, end_time):
        raise NotImplementedError()


class HandleChallenge(ChallengeServer):

    def challenge(self, end_time):
        self.sendline('Another Boring Flag Storage Service')
        self.sendline('-----------------------------------')
        self.sendline('please wait while we setup a secure')
        self.sendline('signature scheme. This could take a')
        self.sendline('few seconds...')
        store = FlagStorage()
        self.sendline('Done! Thank you for your patience')

        self.request.settimeout(TIME_LIMIT)
        while time.time() < end_time:
            self.send('> ')
            request = self.recv(1024).strip()
            if request == '':
                return
            response = store.handle_request(request)
            self.sendline(response)
        self.timeout_message()


class TCPServer(SocketServer.ForkingMixIn, SocketServer.TCPServer, object):
    max_children = 250

    def __init__(self, *args, **kwargs):
        super(TCPServer, self).__init__(*args, **kwargs)
        self._start_time = time.time()
        self._client_ratelimit = {}

    def process_request(self, request, client_address):
        children = len(self.active_children or [])
        print('Accepting Connection, {}/{} ({})'.format(
            children+1,
            self.max_children,
            client_address,
        ))
        return super(TCPServer, self).process_request(request, client_address)


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('port', nargs='?', type=int, default=2302)
    args = parser.parse_args()

    # with open('/srv/curves') as f:
    with open('curves.txt') as f:
        for line in f.readlines():
            line = line.strip()
            if line != '':
                PRECOMPUTED_CURVES.append(eval(line))

    server = TCPServer(('0.0.0.0', args.port), HandleChallenge)
    server.allow_reuse_address = True
    print('server starting on {}'.format(args.port))
    while True:
        server.handle_request()

