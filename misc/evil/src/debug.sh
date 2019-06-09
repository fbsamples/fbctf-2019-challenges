#! /bin/sh
NIC=$(ip -o -4 route show to default | awk '{print $5}')
tshark -i $NIC -f "dst port 8000" -Tfields -e ip.addr -e ip.flags.rb
