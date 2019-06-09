import binascii
import os
import socket

from collections import defaultdict

import crypto


class RemoteServer(object):
    def __init__(self, host, port):
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._sock.connect((host, port))

    def _get_line_str(self):
        out = b''
        while True:
            out += self._sock.recv(1)
            if out[-1] == ord("\n"):
                return out[0:-1]

    def _get_lines_until_done(self):
        lines = []
        while True:
            line = self._get_line_str().strip()
            if line == b"done":
                return lines
            lines.append(line)

    def _send_str(self, cmd, arg):
        self._sock.sendall(b"%s %s\n" % (cmd, arg))

    def register(self, pubkey):
        self._sock.sendall(b"%s\n" % pubkey)
        return self._get_line_str()

    def put_msg(self, msg):
        self._send_str(b"put", msg)
        return self._get_line_str()

    def get_msg(self, mid):
        self._send_str(b"get", mid)
        return self._get_line_str()

    def get_user(self, user):
        self._send_str(b"key", user)
        return self._get_line_str()

    def list_users(self):
        self._send_str(b"list", b"")
        return self._get_lines_until_done()

    def send(self, _, user, msg):
        self._send_str(b"send %s" % user, msg)
        return self._get_line_str()

    def recv(self, _):
        self._send_str(b"recv", b"")
        return self._get_lines_until_done()

    def report(self, _, who, ts, ctxt, fbtag, msg):
        self._sock.sendall(b"report %s %s %s %s %s\n" % (
            who,
            ts,
            ctxt,
            fbtag,
            msg,
        ))
        return self._get_line_str()


class Client(object):
    def __init__(self, server):
        self.server = server
        self._priv_key = crypto.generate_rsa_key()
        self.uid = self.server.register(
            binascii.hexlify(
                crypto.get_pubkey_bytes(
                    self._priv_key.public_key()
                )
            )
        )
        self._all_messages = defaultdict(list)

    def list(self):
        return self.server.list_users()

    def send(self, msg, *users):
        km, cm = crypto.encrypt_inner(msg)
        mid = self.server.put_msg(binascii.hexlify(cm))
        hcm = crypto.hash(cm)

        for user in users:
            self._send_ctxt(mid, km, hcm, user)

    def _send_ctxt(self, mid, km, hcm, user):
        out_msg = mid + km + hcm
        pubkey = binascii.unhexlify(self.server.get_user(user))
        ctxt, com = crypto.encrypt_outer(out_msg, pubkey)
        out = self.server.send(self.uid, user, binascii.hexlify(ctxt + com))
        assert out == b"sent", out


    def recv(self):
        lines = self.server.recv(self.uid)
        msgs = []
        for line in lines:
            who, ts, msg, fbtag = line.split(b" ")
            msgs.append(
                (who, int(ts), binascii.unhexlify(msg), binascii.unhexlify(fbtag))
            )
        out = []
        for (who, ts, ctxt, fbtag) in msgs:
            msg = crypto.decrypt_outer(ctxt, self._priv_key)

            (mid, km, hcm, _) = crypto.split_outer_message(msg)
            cm = binascii.unhexlify(self.server.get_msg(mid))
            assert crypto.hash(cm) == hcm, "bad message hash"
            m = crypto.decrypt_inner(km, cm)

            self._all_messages[who].append((mid, ts, ctxt, msg, fbtag))
            out.append((who, mid, m))

        return out

    def report(self, who, mid):
        (_, ts, ctxt, msg, fbtag) = [
            x for x in self._all_messages[who] if x[0] == mid
        ][0]
        return self.server.report(
            self.uid,
            who,
            str(ts).encode('utf-8'),
            binascii.hexlify(ctxt),
            binascii.hexlify(fbtag),
            binascii.hexlify(msg),
        )
