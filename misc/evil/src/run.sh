#! /bin/sh
echo "Resetting iptables rules"
./reset_iptables.sh

echo "Setting up evil filter"
# Explicitly block incoming traffic to 8001
iptables -t mangle -A PREROUTING -p tcp --dport 8001 -j MARK --set-mark 1
iptables -A INPUT -p tcp -m mark --mark 1 -j REJECT --reject-with tcp-reset
# Filter by evil
iptables -t nat -m u32 --u32 "3&0x80>>7=1" -p tcp --dport 8000 -A PREROUTING -j ACCEPT
# Redirect non-evil to 8001
iptables -t nat -A PREROUTING -p tcp --dport 8000 -j REDIRECT --to-port 8001
iptables -t nat -A OUTPUT -o lo -p tcp --dport 8000 -j REDIRECT --to-port 8001
# Send evil traffic back
iptables -A OUTPUT -p tcp --match multiport --sport 8000,8001 -j NFQUEUE --queue-num 1


trap "exit" INT TERM
trap "kill 0" EXIT
echo "Starting netfilterqueue"
./evilify.py &
echo "Starting up socat servers"
socat -T60 TCP-LISTEN:8001,reuseaddr,fork EXEC:"./server.py bad" &
socat -T60 -t5 TCP-LISTEN:8000,reuseaddr,fork EXEC:"./server.py"
