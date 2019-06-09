
Theme text for challenge:

## Welcome to our Image Protection Vault!

We have had some issues with profile photo theft as of late, so I built a proof of concept vault to store your pictures, it's so secure, even I don't know how to get the photo back out!

-----

## Imageprot README

This is a Rust binary reversing challenge. The problem launches and does a series of simple checks for the state of the machine, attempting to emulate a simple malware sample performing anti-analysis checks. Once these checks complete, detailed below, the sample proceeds to fetch a key from a remote resource. It then uses this key to decrypt a blob which is some python code. This code is execed assuming a python instance is found, and the code is immediately deleted from disk, so as to keep the contents in volatile memory. The Python code b64 decodes an image, which it holds in memory in a variable. This image contains a text string of the flag.

## Solution

TL;DR The flag is fb{Prot3ct_Th3_Gr4ph!1}, additional help is in the `SOLUTION.md` file

The binary performs a couple of simple checks:
1.) The binary checks that gdb, virtualbox, vmware, and vagrant are **not** being used by enumerating the processes on the system and checking for process names from decrypted strings.
2.) The binary attempts to connect to a very specific URI `http://challenges.fbctf.com/vault_is_intern` which is supposed to emulate an internal endpoint. The trick is that this endpoint doesn't exist. The reverser should spin up some sort of web server, or patch the sample, to fake access to this website.
3.) The image is decrypted in volatile memory using the same decryption routine with a key pulled from the content of `https://httpbin.org/status/418`

The image md5 is checked and the program exits. The easiest way to accomplish this task is to extract the image content from strings or IDA and manually decrypt the image, see `SOLUTION.md`.
