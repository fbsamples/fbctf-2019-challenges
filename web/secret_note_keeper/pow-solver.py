#!/usr/bin/env python
import sys
import string
import random
import hashlib
import re

alpha = string.ascii_lowercase + string.ascii_uppercase + string.digits

def rand_string(length):
    return ''.join([random.choice(alpha) for _ in range(length)])

def solve(prefix):
    while True:
        attempt = rand_string(5)
        digest = hashlib.md5(attempt.encode("utf-8")).hexdigest()
        if prefix == digest[:len(prefix)]:
            print('Solution: {}'.format(attempt))
            return

if __name__ == '__main__':
    solve(sys.argv[1])
