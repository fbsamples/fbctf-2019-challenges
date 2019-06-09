# Netscream

Based on previous work in ["A Systematic Analysis of the Juniper Dual EC
Incident"](https://hovav.net/ucsd/dist/juniper.pdf), this problem uses Dual EC
to generate keys and an IV for a simple AES-CBC encryption and therefore can be
decrypted by an adversary who knows the discrete log of the Q-value. In this
case, we provide the d-value, because it's otherwise impossible to compute.

## The Files

- `src`: the source code that created the encryption binary. Includes helpers
  that implement the basic encryption functions in `crypto.{c,h}` and Dual EC in
  `dualec.{c,h}`. The main binary logic is in `screamos.c`

- `flag` and `key`: `flag` is, of course, the flag, and `key` is the key that
  was used to encrypt the flag for debugging purposes. The encrypted flag is
  located in the `dist` folder.

- `dist`: the part of the problem that should be zipped up and distributed as
  the problem. Includes the binary generated from `src`, named `bin`, with
  symbols stripped out. Also includes the `d` value that was used to generate
  the Q value used in our Dual EC implementation, and `enc`, which is the result
  of encrypting the flag using this binary, eg: `./bin ../flag && mv key ..`

- `solution`: contains a solution in c that solves the problem given the input
  files d and enc. Details explained below.

## The Problem

As detailed in the paper above, Dual EC has the property that if an adversary
knows the value of `d = log_Q(P)`, then from a single output of the generator they
can recover the internal state and predict all future numbers generated. In this
case, since the adversary is able to choose the Q-value used to generate random
numbers, they can select a d-value such that `d*Q = P`, where P is the standard
generator of the P-256 curve.\*

For this problem, we've selected our own Q value such that we know the value of
d, and we've included the value of d in the problem.

The screamos binary then generates, using Dual EC\*\*, an IV and then an
encryption key, and encrypts its input using AES-IGE\*\*\*.

---

\* The attacker does so by picking an arbitrary value e, generating `Q = e*P`, and
then finding `d = e^(-1) mod n`, where n is the curve order.

\*\* There is the additional complication, as in the paper above, that the
binary appears to only be using Dual EC to reseed a different, safe PRNG, but
in fact a subtle error via the use of global state that causes the Dual EC
output to be exposed and used for the IV and key material.

\*\*\* The use of IGE is a bit of a red-herring, but honestly I don't remember
why I picked it when I originally wrote the problem.

## The Solution

Because the IV was generated before the key, the attacker can use the IV and the
d-value to recover the state of the Dual EC generator, then crank the generator
forward to determine the value used for the key. Once they have the key, they
can simply decrypt the file using AES-IGE as it was encrypted and recover the
flag.

## The Details

### Dual EC

First you need to understand how Dual EC generates random numbers. Given a
starting state s0 (which is an integer), Dual EC generates a number by doing
`r0 = x(s0*Q)`, where `x(Y)` is a function that takes the x value of a point Y.
It then truncates the 32-byte value of r0 to 30-bytes, and returns that as the
output. To generate its next state, it calculates `s1 = x(s0*P)`.

An attacker with knowledge of the d-value such that `d*Q = P` can recover the
state s1 from an output r0_truncated by guessing the missing 2 bytes to recover
r0, finding a point on the curve that corresponds to that x value (here denoted
as the function `y(z)`), and then multiplying that point by the d-value and
taking the x-coordinate, ie: `x(d*y(r0)) = x(d*s0*Q) = x(s0*P) = s1`.


### screamos

The screamos binary generates a 32-byte IV or key by generating two successive
Dual EC 30-byte outputs, and concatenating the first 30-byte output with the
first two bytes of the second output to form a 32-byte value.

As such, an attacker with knowledge of the d-value can take the first 30 bytes
of a given nonce, guess at the two Dual EC truncated bytes, recover a potential
state, and then check if that guess was correct by generating another value and
comparing it to the remaining two bytes of the nonce.

eg: Dual EC has state `s0`, generates `r0` and `s1`, and exports
`t0 = msb30(r0)` as the first output, where msbX(y) takes the most significant X
bytes of value y. Dual EC uses `s1` to generate `r1`, and
exports `t1 = msb30(r1)` as the second output. screamos uses
`iv = t0 || msb2(t1)` as a 32-byte nonce. screamos then generates another two
outputs, t2 and t3, and concatenates them similarly to produce the key.

An attacker then takes `msb30(iv) = t0` and `lsb2(iv) = msb2(t1)`, guesses at the last
two bytes of r0 to form `r0? = t0 || guess`. They can then use the d-value to
recover a potential `s1?` as described above, and churn the generator to
generate a potential `r1?` value. They can then check their guess by comparing
`msb2(r1?) == lsb2(iv) = msb2(t1)`. If it matches, then the guess was correct
and then can use `s1?` to generate `s2`, `s3`, and therefore `t2` and `t3`, and
therefore the key.

![Visual explanation of the solution](solution.jpg)
