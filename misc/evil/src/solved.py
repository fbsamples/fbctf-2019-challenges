#! /usr/bin/env python3
# requirements: apt-get install build-essential python-dev libnetfilter-queue-dev
# requirements: pip install NetfilterQueue
from multiprocessing import Process
from netfilterqueue import *
from scapy.all import *
from evilify import evilify

import os
import socket
import sys

class Solver:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.setup()

    def setup(self):
        print("Setting up iptables rules")
        print("Filtering port {}".format(port))
        os.system("iptables -A OUTPUT -p tcp --dport {} -j NFQUEUE --queue-num 1".format(self.port))
        self.nfqueue = NetfilterQueue()
        self.nfqueue.bind(1, evilify)

    def teardown(self):
        self.nfqueue.unbind()
        print("Removing iptables rules")
        os.system("iptables -D OUTPUT -p tcp --dport {} -j NFQUEUE --queue-num 1".format(self.port))

    def run(self):
        try:
            self.nfqueue.run(block=True)
        except KeyboardInterrupt:
            self.teardown()

    def interaction(self):
        s = socket.socket()
        print(self.host)
        print(self.port)
        s.connect((self.host, self.port))
        blah = s.recv(1024)
        print(blah)
        word = blah.split(b" ")[-1]
        word = word.strip().strip(b".").capitalize()
        print(word)
        s.send("Every Villain Is {}\n".format(word.decode()).encode("utf-8"))
        print(s.recv(1024))

if __name__ == '__main__':
    host = sys.argv[1]
    port = int(sys.argv[2])
    s = Solver(host, port)
    try:
        p = Process(target=s.run)
        p.start()
        s.interaction()
        p.terminate()
        p.join()
        s.teardown()
    except KeyboardInterrupt:
        s.teardown()
