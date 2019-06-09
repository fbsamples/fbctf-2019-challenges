# asciishop

Ascii is in. Canaries are out.

```
Linux 0d2dfceac1f7 4.15.0-1032-aws #34-Ubuntu SMP Thu Jan 17 15:18:09 UTC 2019 x86_64 x86_64 x86_64 GNU/Linux
```
# Vulnerabliiteis

Compiled with SafeStack (https://clang.llvm.org/docs/SafeStack.html)

```
>>> checksec asciishop
[*] '/home/n/fbctf-2019/pwnables/asciishop/asciishop'
    Arch:     amd64-64-little
    RELRO:    Full RELRO
    Stack:    No canary found
    NX:       NX enabled
    PIE:      PIE enabled
```


The validation check is not sufficient. if -2147483648 is passed then header, width, and/or offset will pass the
length check at the end because -1 * -2147483648 == -2147483648 (two's compliment)

```
int validate_ascii_header(struct image_header_t *image_header) {
  if (memcmp(image_header->magic, "ASCI", 4) != 0) {
    return -EFAIL;
  }

  if (image_header->width < 0)
    image_header->width = -image_header->width;

  if (image_header->height < 0)
    image_header->height = -image_header->height;

  if (image_header->offset < 0)
    image_header->offset = -image_header->offset;

  if (image_header->width > MAX_W) return -EFAIL;
  if (image_header->height > MAX_H) return -EFAIL;
  if (image_header->offset > MAX_LENGTH) return -EFAIL;

  return 1;
}

```

This -2147483648 passes all of the integer overflow checks in `as_touchup_change_byte`. Which gives us an arbitrary number
of 1 byte writes within 65536 bytes bytes of an image.

```
  ascii[position & 0xffff] = as_coordinates.c;
}
```

Images are placed in mmapped regions which means they are allocated in the mmapped region of the process

```
0x00007fe4321aa000 0x00007fe4321ae000 0x0000000000000000 rw-
0x00007fe4321ae000 0x00007fe4321d5000 0x0000000000000000 r-x /lib/x86_64-linux-gnu/ld-2.27.so
0x00007fe4323c5000 0x00007fe4323ca000 0x0000000000000000 rw- /dev/zero (deleted) <----- IMAGE MMAP
0x00007fe4323ca000 0x00007fe4323ce000 0x0000000000000000 rw-
0x00007fe4323d0000 0x00007fe4323d5000 0x0000000000000000 rw- /dev/zero (deleted) <----- IMAGE MMAP
0x00007fe4323d5000 0x00007fe4323d6000 0x0000000000027000 r-- /lib/x86_64-linux-gnu/ld-2.27.so
0x00007fe4323d6000 0x00007fe4323d7000 0x0000000000028000 rw- /lib/x86_64-linux-gnu/ld-2.27.so
0x00007fe4323d7000 0x00007fe4323d8000 0x0000000000000000 rw-
```

With our one byte out of bounds write, we can edit the width and height of images in adjacent chunks, these images have
already passed validation and there are no more checks after upload.

```
struct image_header_t {
  char magic[4];
  int width; <----
  int height; <-----
  int offset;
};

struct image_t {
  struct image_header_t header;
  uint8_t ascii[MAX_W][MAX_H];
};
```

## Infoleak Primitive

After overwriting the width and height, download the image

```
void as_handle_download_ascii(void) {
  struct active_chunk_t *active_chunk;
  struct image_header_t *header;
  uint8_t *ascii;

  active_chunk = as_prompt_user_for_image();

  if (active_chunk == NULL) {
    return;
  }

  ascii  = as_get_chunk_ascii(active_chunk);
  header = as_get_chunk_header(active_chunk);

  as_writen(ascii, header->width * header->height); <----- INFO LEAK
  as_writen("<<<EOF\n", 7);
}
```

When infoleaking we are looking for the Thread Local Storage section. This section holds pointers to libc, ld-so, and most
importantly the safe and unsafe SafeStack pointer.

## Code Execution
After finding the Thread Local Storage, getting a safe and unsafe stack infoleak, put another image with arbitrary write
capabilities next to the TLS.

```
0x00007fe4323c5000 0x00007fe4323ca000 0x0000000000000000 rw- /dev/zero (deleted)
0x00007fe4323ca000 0x00007fe4323ce000 0x0000000000000000 rw-
```

Overwrite the unsafe stack pointer with the safe stack pointer. Now the next time the unsafe stack pointer is read using
the segment register, it will read the pointer we placed in the Thread Local Storage.

No buffer overflows here, but there is a buffer operation, so the unsafe SafeStack buffer is pulled from the TLS
```c
void as_handle_asciishop_filter(void) {
  int retval = 0;
  int i = 0;
  struct active_chunk_t *active_chunk = 0;
  uint8_t *ascii;
  char ascii_filter[1024];

  active_chunk = as_prompt_user_for_image();

  if (active_chunk == NULL) {
    return;
  }

  puts("Upload filter");

  retval = as_readn(ascii_filter, 1024); <--- Read 1024 bytes into the unsafe stack
```

```asm
as_handle_asciishop_filter
  ...
  mov rcx, qword [fs:rax] <----- UnsafeStack pointer read from the register because there is a buffer operation in this frame
```

Now we have written 1024 bytes onto the safe SafeStack. The safe SafeStack holds return addresses, pointers, etc. So from here
we can do a normal rop. I use the one_gadget rop.

_Note_ Alignment here is a bit tricky as we don't know where the safe SafeStack is. This requires ~256 brute force
to get the return address right
