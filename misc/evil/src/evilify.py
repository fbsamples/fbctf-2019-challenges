#! /usr/bin/env python3
# requirements: apt-get install build-essential python-dev libnetfilter-queue-dev
# requirements: pip install NetfilterQueue
from netfilterqueue import *
from scapy.all import *

def evilify(packet):
    pkt = IP(packet.get_payload())
    pkt[IP].flags = pkt[IP].flags | 4
    del pkt[IP].chksum
    packet.set_payload(bytes(pkt))
    packet.accept()

if __name__ == '__main__':
    nfqueue = NetfilterQueue()
    nfqueue.bind(1, evilify)
    try:
        nfqueue.run(block=True)
    except KeyboardInterrupt:
        self.nfqueue.unbind()
