#!/usr/bin/env python3

import sys

from client import Client, RemoteServer


c = Client(RemoteServer(sys.argv[1], int(sys.argv[2])))
users = c.list()
assert len(users) == 3, users
c.send(b"sup?", *users)
msgs = c.recv()
assert len(msgs) == 3, msgs

assert [user for (user, _, _) in msgs] == users, msgs
assert [msg for (_, _, msg) in msgs] == [
    b"sup?",
    b"Cool story bro",
    b"This message is abuse"
], msgs

out = []
for m in msgs:
    out.append(c.report(m[0], m[1]))

assert out[0] == b"This message does not contain 'abuse'.", out[0]
assert out[1] == b"This message does not contain 'abuse'.", out[1]
assert out[2] == b"Thank you for reporting 'abuse'!", out[2]

try:
    c.send(b"abuse", *users)
except AssertionError as e:
    assert str(e) == 'b"You are banned for sending \'abuse\'"', e

