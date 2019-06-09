#!/usr/bin/env python3

import binascii
import sys
import os

base = os.path.dirname(__file__)
sys.path.append(os.path.join(base, '../src'))

import collide  # type: ignore
import crypto  # type: ignore
import client  # type: ignore


assert len(sys.argv) == 3, f"Usage: {sys.argv[0]} host port"

c = client.Client(client.RemoteServer(sys.argv[1], int(sys.argv[2])))
target = c.list()[1]

msg = b"abuse"
pad = b"a" * (collide.BLOCK_SIZE - len(msg))

k1, k2, iv, n, ctxt, t = collide.collide_encrypt(pad + msg)
cm = iv + ctxt + t

mid = c.server.put_msg(binascii.hexlify(cm))
hcm = crypto.hash(cm)  # type: ignore

c._send_ctxt(mid, k2, hcm, target)
print('sent first message')

flag = None
try:
    c._send_ctxt(mid, k1, hcm, target)
except AssertionError as e:
    flag = str(e)

print(flag)
