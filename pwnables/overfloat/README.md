# overfloat
### baby pwnable

## vulnerability
Just a simple buffer overflow except that the buffer can
only contain floats. Reads an unlimited number of floats
into a stack buffer sized for 10. nx, partial relro, aslr,
no canaries.

Input is read using gets(), but the string is memset after.
A buffer overflow would lead to null pointer error, not
exploitation. gets is still in the got and could be used as
second stage in exploits.

## solution
Overflow the float buffer with a rop chain to leak printf
from the got, then return to main for a second phase.
Calculate the location of execve and rop to
execve("/bin/sh", [], []).

This needs to be encoded as floats, which was basically
just an issue of formatting input. My solution script is a
bit more complicated than necessary, so that it mostly works
across recompiling and making changes. A normal solution
should be shorter.

## setup
`make dist`

## distribution
distribute the binaries 'overfloat' and 'libc-2.27.so'
along with server ip/port

## testing
1. setup server
   - `make dist    # build binary`
   - `make docker  # build docker image`
   - `make run     # run docker and tail logs`
2. fix solution -- most constants are found dynamically in solved.py, but two need manual changes when the binary changes:
   - MAIN = the address of main
   - DATA\_NULL = the address of a null pointer u64(0), section.data usually works
3. copy or link libc to ./libc so that the script can auto find symbols and rop gadgets
   - `docker cp <conatiner_id>:/lib/x86\_64-linux-gnu/libc-2.27.so libc`
4. run the solution script
   - `./solved.py localhost:2302` to run against docker
