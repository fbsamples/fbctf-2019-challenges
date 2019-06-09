# Frank

This problem is a version of the
["Invisible Salamanders"](https://eprint.iacr.org/2019/016.pdf) problem. It
implements a simple E2EE messaging service, with one real user and the rest as
bots, and the goal of the problem is to send an 'abuse' message that's
unreportable.

## Details

Run it via xinetd for the competition, but for testing:

```bash
$ ncat -lp 8080 -3 src/server.py
```

Mess around with the client by doing:

```bash
$ ipython src/client.py -i
```

and, eg:

```python
> c = Client(RemoteServer('localhost', 8080))
> users = c.list()
> c.send(b'sup?', *users)
> msgs = c.recv()
> c.report(msgs[0][0], msgs[0][1])
```

Or run the client test in `src/test.py`, which sends some messages and tests the
reporting flow.

The solution lives in `solution/solution.py`, and can be invoked like:

```bash
$ solution/solution.py localhost 8080
```


## Background

E2EE messaging in Facebook Messenger (aka Secret Conversations) uses a
commitment scheme called "message franking" whereby a user commits to the
plaintext of the message they send, Facebook commits to that commitment, and a
receiver of an abusive message can send the plaintext to Facebook along with
these commitments, and Facebook can be sure that the user indeed sent that
abusive message and take moderation action.
[More details in this paper](https://eprint.iacr.org/2017/664.pdf).

Our implementation of message franking for attachments, however, contained a
flaw by which an attacker can generate two keys for the same ciphertext, and one
will decrypt to an abusive message, and the other to an innocuous one. By
sending the innocuous message first and then the abusive one, a bug in the
reporting logic would only allow the first key for the ciphertext to be
collected, and only the innocuous message would be seen by Facebook.
[More details in this paper](https://eprint.iacr.org/2019/016.pdf).

## The Problem

### The Crypto Protocol

The E2EE messaging server in this problem works similarly to the way
attachments are franked in Secret Conversations. First the message is encrypted with
AES-GCM with a key of the sender's choice. They send that ciphertext to the
message server, which stores the ciphertext and gives them back a message id.
The sender then takes the message id, the key for that inner message, and a hash
of the inner ciphertext, and concatenates them to form the plaintext of the
outer message. The sender then generates an HMAC commitment key, appends it to
the plaintext, and computes a commitment over the plaintext. The outer plaintext
(now including the commitment key) is now encrypted using RSA to the public key
of the reciever to produce the outer ciphertext. The commitment is appended to
the ciphertext and then sent to the server along with the user id of the
receiver. In Psuedocode:

```
Enc(m, recv_pubkey):
  km <-$ gen_aes_key()
  iv <-$ secure_rand()
  cm <- aes_gcm(km, iv, m)

  mid <- server_put(cm)

  kf <-$ gen_hmac_key()
  om <- mid | km | SHA256(cm) | kf

  com <- HMAC(kf, om)
  ctxt <- RSA_Enc(recv_pubkey, om)

  return (ctxt | com)
```

Decryption works in the reverse, decrypting with the RSA private key, confirming
the commitment HMAC, and fetching the inner ciphertext from the server using the
message id.

### The Server

The server has two bots, both reply to messages sent with them. ReportBot always
reports messages containing the string "abuse", and AbuseBot always replies to
messages by sending a message containing the string "abuse". The server is wired
such that if a bot reports one of your messages as abusive, but it doesn't
contain the string "abuse" it will give you the flag.

The server communicates via newline delimited simple messages, and all binary
data is hexlified before sending. Commands include

- `list`: shows all user ids on the server (in this case, just the client and
  the bots). Returns multiple lines and then "done" to indicate it's done.
- `put  <inner_ctxt:binary_hex>`: stores a ciphertext, give back a message id string
- `get <message_id>`: retrieves a ciphertext from the message id
- `key <user_id>`: returns the pubkey for a given user
- `send <user_id> <outer_ctxt:binary_hex>`: sends the message to the given user
- `recv`: returns all the unread messages in your inbox. returns message sender
  id, message ctxt, timestamp, and fbtag (server's commitment of the sender,
  timestamp, and sender's commitment). Returns multiple lines and then "done" to
  indicate it's done.
- `report <user_id> <timestamp> <outer_ctxt:binary_hex> <fbtag:binary_hex> <msg:binary_hex>`:
  reports the given user for the message they sent. Server checks if the message
  is valid via the commitment and the fbtag, then either tells the user the
  message isn't abusive or thanks them for their report.

## The Solution

Following the steps in the
["Invisible Salamanders" paper](https://eprint.iacr.org/2019/016.pdf), we can
generate two keys, k1 and k2 for the given inner ciphertext. We then send two
outer messages that point to the same message id for that inner ciphertext, the
first containing innocuous k2, and the second containing abusive k1. Because of
the bug in the client that only reports the first key for a given inner
ciphertext, ReportBot will report the first message instead of the second when
it attempts to report the second, and the server will reward the client with the
flag.
