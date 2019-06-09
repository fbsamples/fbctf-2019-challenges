# EVIL

## Overview
Basically [RFC3514](https://tools.ietf.org/html/rfc3514).
The server is configured to redirect traffic without the evil bit to another port with a failure message.

If you manage to send traffic with the evil bit, use the pcap file to figure out the "password" format - "Every Villain Is {}" where {} is a word we send back. Just your basic "Connect to and interact with socket" challenge at this point.

The difficulty of this challenge can vary significantly depending on your Google-fu. I would categorize this as a medium level networking challenge.

## Challenge Text
I'm trying to break into the EVIL club and I've figured out their password. I still can't get in though because they now have a new secret evil handshake. I've attached what I captured from the old handshake, maybe it will help.

`nc HOST 8000`

## Setup
`sudo ./install.sh`
`sudo ./run.sh`

## Flag
`fb{th4t5_th3_3vi1est_th1ng_!_c4n_im4g1ne}`

## Solution
`sudo ./solved.py <HOST> <PORT>`

Aside from figuring out that this challenge is related to the evil bit (again, should be easily searchable), the hard part of this is actually sending traffic with the evil bit set. I've looked into several ways for doing this:

1. scapy - You can construct packets manually and set the evil bit. There are some github projects online that make doing this easier (some used in conjunction with netfilterqueue). I wrote my own netfilterqueue script for the solution, inspired by [ScapRoxy](https://github.com/Ag0s/ScapRoxy).

2. Patch the kernel - Patch the kernel with [this patch](https://blog.benjojo.co.uk/asset/bxwi3gFqKd). Compile and install. I have this working. The patch is easy to find online if you search for anything evil bit related though. The downside of this solution is that it takes a while to compile the kernel.

3. Kernel module - Use a kernel module that essentially does the same thing as the patched kernel above. This [module](https://github.com/alwilson/evil) should do the trick. This is easier and quicker than compiling your own kernel, so it's probably preferred.

4. eBPF/bcc - This lets us hook into arbitrary kernel functions, so in theory we can write something that would set the bit. I haven't actually tried this, but it should be possible.

## Optional Hints / Support
- "Try thinking a _bit_ more evil"
- Just drop the RFC
- Suggest recording the traffic from the server and inspecting the packets
- If there are issues with people using VMs, tell them to use bridged mode

## Debugging
If things don't work, we want to check that the packets have the bit set when sent from the client and when received on the server.

Client: wireshark

Server: `sudo ./debug.sh`

## Notes
- After a lot of debugging, it seems that virtual NICs somehow discard the evil bit. If using a VM as the origin of the packets, it has to be in Bridged mode (i.e. sharing the actual physical NIC on hardware). NAT and Host Only modes don't work. Tested on VMWare and VirtualBox.

- Command to run on the server to manually see traffic coming in and if it has the bit set: `sudo tshark -i eth0 -f "dst port 8000" -Tfields -e ip.addr -e ip.flags.rb` (This is provided as `debug.sh`)

- There is a [bug in Docker](https://github.com/moby/moby/issues/38682) that makes setting this up in a container hard/impossible. Basically, if we forward a port using `-p` the container will accept even if the port isn't listening. This makes setting up the iptables rules a bit trickier since docker's container external rules will catch the requests before my internal rules can and accept instead of forward.

- The code words are sourced from `/usr/share/dict/american-english`

- The password format is a reference to Spongebob Squarepants [S3E52b](http://en.spongepedia.org/index.php?title=Mermaid_Man_and_Barnacle_Boy_V).

- The flag is a reference to Powerpuff Girls [S6E2](https://knowyourmeme.com/memes/that-s-the-evilest-thing-i-can-imagine).

## References
- https://tools.ietf.org/html/rfc3514
- https://blog.benjojo.co.uk/post/evil-bit-RFC3514-real-world-usage - If you find this blog post, you find the patch and the chal is done for you. This is also where I learned how to filter by evil.
- http://www.stearns.org/doc/iptables-u32.current.html - tutorial on using iptables u32 module.
- https://github.com/Ag0s/ScapRoxy/ - one of the scapy tools that didn't work since it's buggy, but points the way to the scapy + netfilterqueue solution.
- https://github.com/alwilson/evil - kernel module to send the evil bit.
