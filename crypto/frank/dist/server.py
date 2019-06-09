import binascii
import os
import secrets
import time
import uuid

from collections import defaultdict

import crypto


class Server(object):

    def __init__(self):
        self._message_store = {}
        self._key_store = {}
        self._queues = defaultdict(list)
        self._fbtag_key = secrets.token_bytes(crypto.HASH_SIZE)
        self._client = None
        self._should_exit = False

    def should_exit(self):
        return self._should_exit

    def _set_client(self, client):
        if self._client is None:
            self._client = client

    @staticmethod
    def _gen_uuid():
        return str(uuid.uuid4()).encode('utf-8')

    def put_msg(self, msg):
        mid = self._gen_uuid()
        self._message_store[mid] = msg
        return mid

    def get_msg(self, mid):
        return self._message_store[mid]

    def _add_user(self, user, pubkey):
        if not user in self._key_store:
            self._key_store[user] = pubkey
        else:
            raise ValueError(f"{user} already registered")

    def register(self, pubkey):
        uid = self._gen_uuid()
        self._add_user(uid, pubkey)
        return uid

    def get_user(self, user):
        return self._key_store[user]

    def list_users(self):
        return list(self._key_store.keys())

    def _gen_fbtag(self, ctxt, from_user, to_user, timestamp):
        com = ctxt[-crypto.HASH_SIZE : ]
        fbtag = crypto.commit(
            self._fbtag_key,
            com + from_user + to_user + timestamp.to_bytes(4, byteorder='big'),
        )
        return fbtag

    def send(self, from_user, to_user, ctxt):
        assert from_user in self._key_store
        assert to_user in self._key_store
        timestamp = int(time.time())
        fbtag = self._gen_fbtag(ctxt, from_user, to_user, timestamp)
        fbtag = binascii.hexlify(fbtag)
        self._queues[to_user].append(
            (from_user, timestamp, ctxt, fbtag)
        )
        return b"sent"

    def recv(self, user):
        queue = self._queues[user]
        self._queues[user] = []

        return [
            b"%s %d %s %s" % (who, ts, ctxt, fbtag)
            for (who, ts, ctxt, fbtag) in queue
        ]

    def report(self, reporting_user, send_user, ts, ctxt, fbtag, msg):
        fbtag = binascii.unhexlify(fbtag)
        assert fbtag == self._gen_fbtag(ctxt, send_user, reporting_user, int(ts)), "fake report"

        ctxt = binascii.unhexlify(ctxt)
        msg = binascii.unhexlify(msg)
        crypto.validate_outer_commitment(ctxt, msg)

        (mid, km, hcm, _) = crypto.split_outer_message(msg)
        cm = binascii.unhexlify(self.get_msg(mid))
        assert crypto.hash(cm) == hcm, "bad message hash"
        message = crypto.decrypt_inner(km, cm)

        # The actual abuse check:
        if b"abuse" in message.lower():
            if send_user == self._client:
                self._should_exit = True
                return b"You are banned for sending 'abuse'"

            return b"Thank you for reporting 'abuse'!"
        else:
            if reporting_user != self._client and send_user == self._client:
                with open('flag', 'rb') as f:
                    self._should_exit = True
                    return (
                        b"You were erroneously reported for 'abuse', "
                        b"here's a consolation prize: %s"
                        % f.read()
                    )
            return b"This message does not contain 'abuse'."
