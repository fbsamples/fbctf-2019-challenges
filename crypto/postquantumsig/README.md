# Post-Quantum Transaction Signatures

## Challenge and Vulnerability

Quantum computers could seriously disrupt many types of encryption by making certain hard problems easier. One hard problem there is no proposed speedup from quantum computing, is finding collisions for a hash. Therefore a signature scheme that relies only on the difficulty of finding hash collisions is just as resistant to quantum computers as it is to regular computers.

This problem utilizes both a Lamport Signature to create a one-time signature that is resistant to quantum computing, and a Merkle Tree to allow a single signature to be used “multiple times”. In reality, the Merkle tree doesn’t do that, it just creates one top level key with a tree below it, the leaf nodes are each one time keys. All it is is an efficient method of bundling many one time keys together into a single verifiable root public key at the top.

Lamport signatures are actually conceptually simple in their construction. If you want to be able to sign a 256-bit message, you generate twice that many random numbers, in this case 512. You then hash each one and publish the exact order as your public signature. The order matters, the first number corresponds to 0 for the first bit, the end number 1 for the first bit, the third number 0 for the second bit, and so on. When you want to sign a message, you publish the unhashed random numbers that correspond to the bits that are in the message. Since you already publicly declared the hashed versions, anyone can hash each one and check that it corresponds to the right spot in the public key for the given 0 or 1 bit in the message that was signed. Since hashes are one way, only someone that knew the base numbers used to generate the hash could have come up with all of the right numbers that would hash to the value sin the public key. The reason that it is a one time key, is that if you ever revealed any of the other underlying random numbers, anyone else could sign any message with positional bits that existed in either of the signed messages.

In order to sign long messages without needing to generate a obscene number of keys, we’ll instead sign a hash, so that all messages will have the same signing space. As a consequence, if two messages were signed with the same public key there would be about 50% overlap (assuming the second signature wasn’t a chose plaintext chosen to have less overlap. This means that for about 50% of the hash output bits, you could choose either value, meaning the hash only protects with about half of the number of bits it’s supposed to.

As mentioned before, a Merkle tree is just a mechanism for trading off public key length for signature size and length. It means that 2^n public keys can be nested into a single public key. It’s a binary tree, the leaf nodes contain the base public key values. Each node contains the hash of the the child nodes concatenated. Since the hashes are all one way, you can easily traverse towards the root from any combination of nodes you know the value for. To sign a message, you reveal the public key for that exact message, the signature (the underlying random numbers), and any intermediate nodes necessary to calculate the root public key (in general, about n nodes, minus any that can be trivially calculated from the exposed leafs).

This CTF operates by “accidentally” signing multiple times with the same public key leaves, leaving the public key open to signing almost arbitrary message. It’s “almost” arbitrary because the the hash will need to collide on known signed bits, but if for each repetition, about half of the remaining private key is revealed, so after only 4 signatures, it’s only protecting with 32-bit hash security, only about 4 billion guesses required to crack. With 5 signatures, it’s only 16 bit security, only 65k guesses to collide. In the dist directory as prepared, the message has been signed up to 14 times, compromising all 256 bits so no brute force is required.

Our example will be of a transaction ledger as might exist in a cryptocurrency block chain. We’re not going to replicate any other parts of the block chain, just signatures against a ledger. The objective of the CTF will be to sign any transaction from a legitimate address (that exists in the provided transaction ledger), signed by that address, depositing currency into any unclaimed address.

## Solution Description

In order to complete the hack successfully, a participant would need to:

1. identify which address has used multiple signatures
2. compile a list of which bits they are able to sign
3. brute force a transaction that hashes to bits they can forge if they don't have perfect coverage
4. construct the forged signature by pieceing together fragments of other signatures for the bits

Signature verification takes place in two phases, traversing the portion of the tree where the private keys were used to sign the actual message, and then traversing the "top" of the tree with the parent node hashes that will be used to sign other messages. In the first phase the order is deterministic based on the message bits, but the later phase the concatination order needs to be specified. If the same leaf nodes are used, it will be obvious that the same top part of the tree is being used. The easiest way to test for this is to look for identically repeated patterns in the last part of the signature, called "others" in the parser included in the verifier.

Once the solver has identified which player has reused keys, they need to compile a list of which bits of the message hash they are able to forge to which values. They may only be able to forge a 0 or 1 for certain bits if that is the only bit they have observed in a signature, but for many bits with will have observed both. They should then construct a transaction message from the weak player that deposits coins to a previously unseen address, and randomly vary the exact transaction amount until they get a collision with the bits they are able to hash. In the current example, all 256 bits are compromised, so the solver should be able to forge a signature of ANY message, without any brute force.

## Setup Instructions

The docker container should contain everythign necessary to host the special verification that makes sure it's a net new transaction etc.

To generate new sample ledgers use the generate.py script. Since the number of signatures is random, and how many bits collide per hash is random, you should check how many weak bits there are using the solver.py. If there are fewer than 256, there will be a brute force component (current solver doesn't do this, but it's as easy as repeatedly trying different messages until you get one that hashes to a value you can sign).

## Files to Be Distributed

Send everything in the dist folder, and make sure the objective of signing a transaction from a known address to a new address is understood. The inclusion of a transaction verifier is all but essential, it helps them understand the file structure and the kinda funky convention of writing to text strings and then encoding to ascii between repeated hashes. This convention is inefficient but harmless, and makes it a bit easier for pythonistas to follow what is going on since the hashes are always human readible.

## Difficulty

On the easy side of moderate, only because Merkle trees and Lamport signatures are slightly exotic techniques. Once you understand those topics, the solve is very straightforward.

## Additional Notes
To make the challenge a little bit harder, don't provide 256 compromised bits. This will force the solver to brute force different messages until they find ne that hashes to bits they are able to forge. To make it _considerably_ harder, instead of reusing the exact same private keys in the same order multiple times with the same identity, use multiple partially overlapping public keys. Since order will vary there will be different top level signatures. This is harder to construct and _much_ harder for the solver to even identify that keys have been reused and how to reconstruct the "top" part of the hash tree. The solution as is the solver only needs to identify that a top portion has been reused and then doesn't need to actually interact with it.

