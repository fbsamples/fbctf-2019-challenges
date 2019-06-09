import os
import secrets
import sys

from cryptography.hazmat.backends import default_backend  # type: ignore
from cryptography.hazmat.primitives import hashes, hmac, serialization  # type: ignore
from cryptography.hazmat.primitives.asymmetric import rsa, padding  # type: ignore
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes  # type: ignore


BLOCK_SIZE = 128 // 8
IV_SIZE = 96 // 8
HASH_SIZE = 256 // 8


def generate_rsa_key():
    return rsa.generate_private_key(
        public_exponent=65537,
        key_size=4096,
        backend=default_backend(),
    )

def get_pubkey_bytes(pubkey):
    return pubkey.public_bytes(
        encoding=serialization.Encoding.DER,
        format=serialization.PublicFormat.SubjectPublicKeyInfo,
    )


def generate_aes_key():
    return secrets.token_bytes(BLOCK_SIZE)

def decrypt_inner(key, ctxt):
    iv = ctxt[0 : IV_SIZE]
    c = ctxt[IV_SIZE : -BLOCK_SIZE]
    t = ctxt[-BLOCK_SIZE : ]

    dec = Cipher(
        algorithms.AES(key),
        modes.GCM(iv, t),
        backend=default_backend(),
    ).decryptor()

    return dec.update(c) + dec.finalize()

def encrypt_inner(msg):
    """Encrypt the (potentially large) message using a fast symmetric cipher"""
    key = generate_aes_key()
    iv = secrets.token_bytes(96 // 8)
    enc = Cipher(
        algorithms.AES(key),
        modes.GCM(iv),
        backend=default_backend(),
    ).encryptor()

    return (key, iv + enc.update(msg) + enc.finalize() + enc.tag)

def encrypt_outer(msg, pubkey):
    kf = secrets.token_bytes(HASH_SIZE)
    com = commit(kf, msg)

    send_key = serialization.load_der_public_key(
        pubkey,
        backend=default_backend(),
    )
    ctxt = send_key.encrypt(
        msg + kf,
        padding.OAEP(
            mgf=padding.MGF1(algorithm=hashes.SHA256()),
            algorithm=hashes.SHA256(),
            label=None,
        )
    )
    return (ctxt, com)

def decrypt_outer(ctxt, priv_key):
    c = ctxt[0 : -HASH_SIZE]
    com = ctxt[-HASH_SIZE : ]
    msg = priv_key.decrypt(
        c,
        padding.OAEP(
            mgf=padding.MGF1(algorithm=hashes.SHA256()),
            algorithm=hashes.SHA256(),
            label=None,
        )
    )

    validate_outer_commitment(ctxt, msg)

    return msg

def validate_outer_commitment(ctxt, msg):
    com = ctxt[-HASH_SIZE : ]
    m = msg[0 : -HASH_SIZE]
    kf = msg[-HASH_SIZE : ]
    assert com == commit(kf, m), "bad commitment"

def split_outer_message(msg):
    mid = msg[0 : -(BLOCK_SIZE + HASH_SIZE + HASH_SIZE)]
    km = msg[-(BLOCK_SIZE + HASH_SIZE + HASH_SIZE) : -(HASH_SIZE + HASH_SIZE) ]
    hcm = msg[-(HASH_SIZE + HASH_SIZE) : -HASH_SIZE]
    kf = msg[-HASH_SIZE : ]
    return (mid, km, hcm, kf)

def hash(msg):
    h = hashes.Hash(hashes.SHA256(), backend=default_backend())
    h.update(msg)
    return h.finalize()

def commit(kf, msg):
    h = hmac.HMAC(kf, hashes.SHA256(), backend=default_backend())
    h.update(msg)
    h.update(kf)
    com = h.finalize()
    return com
