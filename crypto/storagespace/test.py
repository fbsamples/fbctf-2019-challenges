#!/usr/bin/env sage
from typing import Any, Callable, Dict, Optional, Tuple
import base64
import socket
import json

from solve import Connection

"""
Tests functionality of the server and a few sanity checks
"""

JSON = Dict[str, Any]
TestFunction = Callable[[Connection], None]


def do_request(
    store: Connection,
    request: JSON,
) -> str:
    store.sendline(json.dumps(request, sort_keys=True))
    r = store.recvuntil('\n> ')
    return r[:-3]


def sign_request(
    store: Connection,
    request: JSON,
) -> JSON:
    sign_request = {
        'command': 'sign',
        'params': {
            'command': request['command'],  
            'params': request['params'],
        }
    }
    result = do_request(store, sign_request)
    signed_request = json.loads(result)
    return signed_request


def do_signed(
    store: Connection,
    request: JSON,
) -> str:
    signed_request = sign_request(store, request)
    return do_request(store, signed_request)


def request(
    __command: str,
    **params,
) -> JSON:
    return {
        'command': __command,
        'params': params,
    }


def test_help(store: Connection) -> None:
    r = do_request(store, request('help'))
    assert 'help(command' in r, 'no help'
    assert 'list of commands' in r, 'no list'
    assert '- flag' in r, 'no flag'


def test_sign(store: Connection) -> None:
    r = do_request(store, request('sign', command='help'))
    try:
        json.loads(r)
    except json.decoder.JSONDecodeError:
        assert False, 'json decode fail'


def test_flag(store: Connection) -> None:
    req = request('flag')
    r = do_request(store, req)
    assert r == 'signature required', 'sig required'
    r = do_signed(store, req)
    assert 'bad command' in r


def test_info(store: Connection) -> None:
    req = request('info')
    r = do_request(store, req)
    assert r == 'signature required', 'sig required'
    r = do_signed(store, req)
    assert 'curve' in r, 'no curve'
    assert 'generator' in r, 'no generator'


def test_spec(store: Connection) -> None:
    req = request('spec', mode='sign')
    r = do_request(store, req)
    assert r == 'signature required', 'sig required'
    r = do_signed(store, req)
    assert 'def sign' in r, 'mode=sign'

    req = request('spec', mode='verify')
    r = do_signed(store, req)
    assert 'def verify' in r, 'mode=verify'

    req = request('spec', mode='request')
    r = do_signed(store, req)
    assert 'base64' in r, 'mode=request'

    req = request('spec', mode='all')
    all_r = do_signed(store, req)
    req = request('spec')
    r = do_signed(store, req)
    assert all_r == r, 'default behavior'

    req = request('spec', mode='asdf')
    r = do_signed(store, req)
    assert 'please use' in r


def test_save_and_list(store: Connection) -> None:
    list_req = request('list')
    r = do_request(store, list_req)
    assert r == 'signature required', 'sig required (list)'
    r = do_signed(store, list_req)
    assert r == 'fbctf', 'fbctf only'

    save_req = request('save')
    r = do_request(store, save_req)
    assert r == 'signature required', 'sig required (save)'
    r = do_signed(store, save_req)
    assert 'missing' in r, 'missing both'
    save_req = request('save', name='myflag')
    r = do_signed(store, save_req)
    assert 'missing' in r, 'missing flag'
    save_req = request('save', flag='asdf')
    r = do_signed(store, save_req)
    assert 'missing' in r, 'missing name'

    save_req = request('save', name='myflag', flag='asdf')
    r = do_signed(store, save_req)
    assert 'stored' in r, 'stored: '
    r = do_signed(store, save_req)
    assert 'overwritten' in r, 'overwrite'

    r = do_signed(store, list_req)
    assert r == 'fbctf\nmyflag' or r == 'myflag\nfbctf', 'list both'


def test_bad(store: Connection) -> None:
    req = request('bad')
    r = do_request(store, req)
    assert r == 'bad command!', 'bad'


def test_caps(store: Connection) -> None:
    req = request('FLAG')
    r = do_request(store, req)
    assert 'invalid' in r, 'FLAG (nosig): ' + r
    r = do_signed(store, req)
    assert 'bad' in r, 'FLAG: ' + r
    req = request('FlAg')
    r = do_signed(store, req)
    assert 'bad' in r, 'FlAg: ' + r
    req = request('iNfO')
    r = do_signed(store, req)
    assert 'bad' in r, 'iNfO: ' + r


def test_badsig(store: Connection) -> None:
    req = request('info')
    signed = sign_request(store, req)
    sig = base64.b64decode(signed['sig']).decode('utf-8').split('|')
    r, s = list(map(int, sig))[:2]

    modified_request = request('info')
    modified_request['sig'] = base64.b64encode(bytes(str(r) + '|' + str(s), 'utf-8')).decode('utf-8')
    res = do_request(store, modified_request)
    assert 'curve' in res, 'normal failed'

    modified_request = request('info')
    modified_request['sig'] = base64.b64encode(bytes(str(r) + '|' + str(s+1), 'utf-8')).decode('utf-8')
    res = do_request(store, modified_request)
    assert 'invalid sig' in res, 'modified s'

    modified_request = request('info')
    modified_request['sig'] = base64.b64encode(bytes(str(r) + '|' + str(0), 'utf-8')).decode('utf-8')
    res = do_request(store, modified_request)
    assert 'invalid sig' in res, 's = 0'

    modified_request = request('info')
    modified_request['sig'] = base64.b64encode(bytes(str(r+1) + '|' + str(s), 'utf-8')).decode('utf-8')
    res = do_request(store, modified_request)
    assert 'invalid sig' in res, 'modified r'

    modified_request = request('info')
    modified_request['sig'] = base64.b64encode(bytes(str(0) + '|' + str(s), 'utf-8')).decode('utf-8')
    res = do_request(store, modified_request)
    assert 'invalid sig' in res, 'r = 0'

    modified_request = request('info')
    modified_request['sig'] = base64.b64encode(bytes(str(0) + '|' + str(0), 'utf-8')).decode('utf-8')
    res = do_request(store, modified_request)
    assert 'invalid sig' in res, 'r = s = 0'

    modified_request = request('info')
    modified_request['sig'] = base64.b64encode(b'asdf').decode('utf-8')
    res = do_request(store, modified_request)
    assert 'invalid sig' in res, 'sig=asdf'


def do_test(
    test: TestFunction,
    store: Connection,
) -> bool:
    try:
        test(store)
        return True
    except AssertionError as e:
        print(e)
        return False


def do_all_tests() -> Tuple[int, int]:
    store = Connection()
    prompt = store.recvuntil('\n> ')
    if 'please provide S' in prompt:
        # Need to do proof of work
        from collections import namedtuple
        from proof import cmd_solve
        lines = prompt.split('\n')
        prefix = lines[1].split('"')[1]
        challenge = lines[4].split('"')[1]
        if len(lines) > 5 and 'len(S)' in lines[5]:
            length = int(lines[5].split(' == ')[-1])
        else:
            length = 20 # rando default
        print(f'doing proof of work, {prefix} -> {challenge} (len {length})')
        Args = namedtuple('Args', ['prefix', 'challenge', 'length'])
        proof = cmd_solve(Args(prefix, challenge, length))
        print(f'solved : {proof}')
        store.sendline(proof)
        check = store.recvline()
        if 'invalid' in check or 'timeout' in check:
            print('proof of work failed!')
            exit(1)
        prompt = check + store.recvuntil('\n> ')
    success = 0
    failure = 0

    test_names = [g for g in globals().keys() if g.startswith('test_')]
    for name in test_names:
        func = globals()[name]
        if callable(func):
            if do_test(func, store): # type: ignore
                print(f'{name}: pass')
                success += 1
            else:
                print(f'{name}: fail')
                failure += 1
    return (success, failure)


if __name__ == '__main__':
    success, failure = do_all_tests()
    total = success + failure
    print(f'{success} / {total} tests pass')
