# Theme 

Short Circut!

``` 
            _//)_//)
           / /)=/ /)
          ((O)=(O))
             \||/
     ________====____[o]_ 
    |___._==      ==_.___|
(( \ || '-.________.-' || / ))
 \=/ ||     ..''..     || \=/
  \\_//    / [||] \    \\_//
   \V/    / ...... \    \V/
          \::::::::/
    _____.---'  '---._____
   |_-_-_|__------__|_-_-_|
   |_-_-_|=        =|_-_-_|
   |_-_-_|          |_-_-_|LGB

```

# Challenge 

## External Text

*Come and Follow Me to ip.ad.dr.ess:9090, look around and see if lightning
strikes. A socat tty works better than netcat.*

## Internal Challenge Overview

This is intended to be a easy-medium challenge involving abstract sockets,
network communication, and some logic of a "weird machine". Two minimal docker
containers are used, right now it's setup for a single server container, however
we could easily create a unique container pair per client.

```
Internet ---> socat listener 
                    |
                    |
            [ Client container 1 ] <--ABS SOCK--> [ Server container ]
            [ Client container 2 ] <--ABS SOCK---------|      |
            [ Client container 3 ] <--ABS SOCK----------------|
```

## Connecting

Either using socat, which gives you a clean happy tty:
    - `socat tcp:example.com:9090 file:`tty`,raw,echo=0`

Or using netcat:
    - `nc a.b.c.d 9090`
    - However, to then cleanup the nc shell, do this:
        1) Background your nc command with ctrl-z after connecting
        2) type `stty raw -echo` 
        3) forground your nc command again with: `fg`
        4) Now you should have a nice normal terminal.
        5) Once you disconnect, run `reset` to fix your normal terminal.

- Once connected, the player is expected to write their own abstract socket
  netcat util, using x0d as the EOL delimeter. 

# Infra

## Docker

### Client

- The client is heavily resource restricted and intentionally made too small to
  make it difficult to get anything like socat on it for an easy solve...(or
  crypto miners, etc).

#### Dockerfile
- Note, this requires 'clue.txt' to exist in the Docker build directory.
- clue.txt contents are: *Sentience is an abstract concept...*
    - get it "abstract" ;)

```
FROM python:3.7-alpine
RUN adduser -S console
COPY clue.txt /home/console/
RUN chmod 444 /home/console/clue.txt
USER console
WORKDIR /home/console
```
- Now build the client: `sudo docker build -t ctf-client -f client.docker .`

### Server

- I intentionally didn't use `FROM scratch` to build the smallest container
  because I wanted the players to be able to look around the filesystem vs
  having everything purely "synthetic"... the upload, download and show commands
  provide this.  However, given that almost everything is "synthetic", we
  probably should switch to this model...

#### Dockerfile

- Note, this requires the server Go binary to be built statically, and sitting
  in the Docker build dir... This is also included in the Makefile:
- `CGO_ENABLED=0 GOOS=linux go build -a -installsuffix cgo -o server server.go`

```
FROM alpine
RUN adduser -S howard
COPY server /sbin/saintcom
USER howard
WORKDIR /home/howard
ENTRYPOINT ["/sbin/saintcom"]
```
- Now build the server: `sudo docker build -t ctf-server -f server.docker .`

# Running it

## TODO 

- Limit the server and client mem/CPU resources, below command should work, but need to test it...: 
- `sudo docker run --memory="256m" --cpus=".5" --cpu-shares="256" ctf-server`

## Remote Connections

- The "client" docker image is launched dynamically using socat:
    - Using the following script:
    ```
	#!/bin/bash
	export SERVER=$(sudo docker ps | grep ctf-server | awk '{print $NF}')
	export CMAX=500
	export CNAME="ctf-client"
	export LOGFILE="launch_client_container.socat.log"
	export COUNT=$(sudo docker ps | grep $CNAME | wc -l)

	if (( $COUNT > $CMAX )); then
			echo "Too many connections, try again later"
			echo "ERROR! Hit too many containers ($COUNT) on `date`" >> $LOGFILE
			exit 1
	fi

	echo "Launching Docker container number $COUNT" >> $LOGFILE
	# TODO maybe use -icc false to prevent cross-container comms, otherwise people could fuck with eachother...?
    # Make almost the whole think read-only to make the challenge more difficult and to prevent abuse
	sudo docker run --mount type=tmpfs,destination=/xf0x9fxa4x94,tmpfs-size=4096 --network="container:$SERVER" -ti --read-only $CNAME sh
	exit 0
    ```

    Clients should clear up, and they're very light weight (only a few megs of
    memory each), so, we should be able to support a lot...

    - Listening with the following script:
	```
	#!/bin/bash
	PORT=9090
	echo "Starting CTF listener"
	# Start the server only if it's not running
	sudo docker ps | grep ctf-server > /dev/null || sudo docker run -d ctf-server && echo "Started CTF Server"
	echo "Starting socat listener on $PORT"
	while true; do sleep 5 ; socat -d -d tcp-l:$PORT,reuseaddr,fork exec:./launch.sh,pty,setsid,sane,stderr,sigint 2>&1 | tee -a launch_client_container.$PORT.socat.log; done
	```

# Solving it

## Overview

Solving this challenge requires: Finding the writable tmpfs directory, noticing
an abstract socket via netstat to connect and get a command interface, noticing
the delimer byte is \r in the MOTD, googling the movie script or watching
youtube clips for a code, using a fairly easy to find directory traversal
vulnerability in the upload functionality to discover a directory, and using
another easy vulnerability in the download functionality to get a firmware file
staged (which contains the flag), and a final command that reads it by
attempting to apply an "update", which "errors and fails, but lists the flag
out". The whole time, random "malfunctions" errors are occurring which
"corrupt" the output, so persistence and re-running commands are key. Ideally,
there's a small clue leading the player for each step.

### Details 

In the beginning, everything is read-only, so they have to find the tmpfs
directory mounted to a hex address which is THINKING emoji. Players then must
notice (hint provided in the clue) there's an abstract socket is listening on
the ROBOT emoji hex chars. Connecting to this will spit back an MOTD, delineated
by 0x0d, which they'll need to modify their netcat-ish clients to connect to it
and end "commands" with that character (I tried to make this easy enough to try
or discover via HTTP requests even if they don't notice it). The title, clue and
MOTD should give them hints they're dealing with a "Short Circuit" theme, but
sometimes it throws a bunch of "MalFuction" errors back as the very first thing.
Upon issuing various requests, the player will encounter the servers output
being "corrupted" with "MalFunctI0n" errors. This will occur randomly and
requires retrying commands, sometimes 2-3 times, and can be thought of as a
random injected frustration or difficulty (borrowed from the movie).

Second, assuming they have a stable interface to send and receive commands,
they'll be presented with a simple command and response interface, but it's
"locked". By issuing various commands, they'll discover that the functionality
is limited and needs to be unlocked. Finding the `command access_code` command
is the first step to unlocking it, the second is knowing the right unlock
numerical code. A clue is provided at access code 29, 12345, and a few other
numbers which is mentioned in a free YouTube clip of the movie, googling the
script, or possibly via brute-force, or watching the whole movie, the player
will come across unlock code 42721. This "unlocks" the interface and more
commands are now available to be explored.

Now in **unlocked mode**, they'll need to discover that using the `show` and
`upload` command they can partially read directories, but only the first few
files. After discovering a directory traversal, and when reading the root
directory, they'll discover ../../nova (the name of the Robotics company in the
movie). They'll then be able to work their way to discover the directory that
contains the firmware with the flag "in" it:
`../../nova/tmp/.garbage/novarobotics.j5_jailbroken.bin`.  However, the `upload`
command won't let them read it. For that they'll need to discover/use the
`download` command. (The .garbage directory being a Hackers movie reference of
course.)

Fourth, still in **unlocked mode**, the download command can be issued on their
discovered directory, when issued on `../../nova/tmp/.garbage/flag`, they'll see a
response *"download succeeded. Response is: updated_staged"*, this also sets the
**staged mode** flag, and the user can proceed to the final part of the
challenge and get the flag.

Fifth, in **unlocked and staged mode**, the player can issue the `execute
update` command, which will act as if it's performing an update, but then error
with another malfunction but print the flag contents from the "jailbroken"
firmware...You cannot stage it without unlocking (commands don't exist), and you
cannot execute it without staging it (as you shouldn't know the path). The show
command also will not list the staged firmware until you have "downloaded" it.

## Exact Commands:

- Note: You can test this locally using the client and server applications in
  this repo. 
- Using socat, you can setup a trivial "client" application:
    -  `socat readline ABSTRACT-CONNECT:$(echo -ne "\xf0\x9f\xa4\x96"),cr`
- Then enter unlocked mode with: 
    - `command access_code 42721`
- Then find it (eventually, we could make it harder to find?): 
    - `upload ../../nova/tmp/.garbage/` 
    - This lists "novarobotics.j5_jailbroken.bin" as a file.
- Then "stage" it (by trying to download it): 
    - `download ../../nova/tmp/.garbage/novarobotics.j5_jailbroken.bin`
- Now the `show` command will list it as the staged firmware if you check it.
- Then "execute" it: 
    `execute update` - which will print the flag as the returned "error message".

