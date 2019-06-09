# storagespace
### easy crypto
In order to fit in with all the other CTFs out there, I've written a *secure* flag storage system!

It accepts commands in the form of json. For example: `help(command="flag")` will display help info for the flag command, and the request would look like:

```
{"command": "help", "params": {"command": "flag"}}

        flag(name: Optional[str])
        Retrieve flag by name.

{"command": "flag", "params": {"name": "myflag"}}

        flag{this_is_not_a_real_flag}

```

You can access it at nc localhost 2302

P.S. some commands require a signed request. The sign command will take care of that for you, but no way you'll convince me to sign the flag command xD

## description
This challenge is a simple key-value flag storage server. It accepts json object requests like
```
{
  "command": "save",
  "params": {
    "name": "myflag",
    "flag": fb{asdf}"
  }
}
```

However, only two commands 'help' and 'sign' can be done
this way, the rest of the commands ('info', 'spec', 'flag',
'save', 'list') have to be signed with a EC signature (schnorr).
The 'sign' command will sign your requests, but will not sign a
request using the 'flag' command, which is the only way to
withdraw flags from the service.

There's a good chance this challenge has alternative solutions,
because I have no idea what else is wrong with these poorly
generator curves and how they interact with signing. But this
is intended to be an easy challenge so it's okay.

## Vulnerability
The server will generate a random curve and key pair to do
the signing. This curve is not actually random, but guaranteed
to be breakable for a few reasons: the curves are generally
small and could be brute forced. Otherwise, the curve orders
will always be factorable into many small prime factors, in
which case you can use the pohlig-hellman algorithm to solve
the discrete log. This is the approach used in solve.py

Given the curve, generator, and public key, you can solve for
the private key and then sign your own requests. To avoid
annoying bugs, the 'spec' command will print out the source
code for signing and verifying requests. This should not
give any hints towards breaking the curve though, as the
intended bug is not in the signing algorithm.

## distribution
Distribute the server ip/port

## testing
1. interact with the server with `./challenge.py`
2. test functionality with `./test.py`
2. get the flag with `./solve.py`
