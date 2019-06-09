Image prot is a simple binary that decrypts an embedded image, using a remote endpoint response as the decryption key. The decryption is a simple XOR against the key

## solution for imageprot

First, you'll need to get the content from IDA using IDAPython. To do this, I looked for strings that were super big, say > 1000, and noticed that one was a base64 encoded value. Other ways you might've found this would've been to traipse your way through IDA looking for where the user code begins, and you would've spotted the base64 decode routines. At this point you should've also found the request to httpbin.org hitting the status code 418 page, better read "I'm a little teapot", the response of which we use for the decryption of our image. The image itself is never written to disk, so one needs to either extract the image using the IDAPython snippet below, or better yet scan the volatile memory of our program and carve out the JPEG as it's in the clear.

```python
bytes = ''
ea = 0x02BE31A
cnt = 0
while cnt < 0x159c5:
  bytes += chr(Byte(ea + cnt))
  cnt += 1

with open("bytes_out", 'wb') as fout:
  fout.write(bytes)
```

## Python script to decrypt the image

```python
#!/usr/bin/env python3
import base64
import requests

b64 = ''
with open("bytes_out", 'rb') as fin:
  b64 = fin.read()

enc = base64.b64decode(b64)

resp = requests.get("https://httpbin.org/status/418")
key = resp.content

clear = b''
for i in range(len(enc)):
    clear += bytes([enc[i] ^ key[i % len(key)]])

with open("decrypted.jpg", 'wb') as fout:
    fout.write(clear)
```
