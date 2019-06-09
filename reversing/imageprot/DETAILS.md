# imageprot - A relatively simple Rust binary reverse-me

## Problem flow

This is a Rust binary reversing challenge. The problem launches and does a series of simple checks for the state of the machine, attempting to emulate a simple malware sample performing anti-analysis checks. Once these checks complete, detailed below, the sample proceeds to fetch a key from a remote resource. It then uses this key to decrypt a blob which is some python code. This code is execed assuming a python instance is found, and the code is immediately deleted from disk, so as to keep the contents in volatile memory. The Python code b64 decodes an image, which it holds in memory in a variable. This image contains a text string of the flag.

## Solution

TL;DR The flag is fb{Prot3ct_Th3_Gr4ph!1}

The binary performs a couple of simple checks:
1.) Binary must be connected to the internet, it downloads a remote resource and checks the hash of the resource
2.) A process with the name of `windbg.exe` must be running. Note it just needs that name

If both parts are solid, the embedded image resource is decrypted in volatile memory, the digest of the image is periodically verified. The image contains a string embedded in it which is the flag.