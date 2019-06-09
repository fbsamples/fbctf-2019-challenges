# Whysapp

We're working on a brand new sort of encrypted version of Whatsapp, we call it Whysapp. We took the base technology behind Whatsapp and upgraded it to use a super shiny new language.

# Notes

Provide users with the beam file and nothing else. The beam file is the client logic for the server. The client connects to the server and both send base64 encoded, AES encrypted messages. 

The client implements the necessary connection logic but doesn't loop or connect to the right server. The expected solution is that players will need to reverse the BEAM bytecode to get the AES key or write the necessary code to loop the connection code. 

Run with `docker run -p 5242:5242 -it whysapp`. The client only connects locally. 

Run `make` to rebuild the `Elixir.Whysapp.beam` module file. 

Run `make server` to create the Docker server. 

Run `make test` to run the testing client. 


# Description

This is an Erlang/Elixir/BEAM reversing challenge. `server.py` contains a chat "server" which Whysapp.ex communicates with. The user is to be provided the compiled .beam file which has the main logic of the chat client without any looping and missing some commands. 

The goal for the player is to reverse out the commands, build out a fake client that responds correctly to the messages sent to it, and eventually spoof some of the values sent.

The client is aware of some server commands like math, ping, and cats. These commands when sent to the client must respond with the appropriate response. After enough correct commands have been sent, the server will eventually restrict messages to only those with lower user IDs. The user will then need to demonstrate that they understand the protocol enough to modify the messages sent to have a lower user ID. 

Finally the user will be told to send the flag command and that only zuck can send the flag command. At this point, the player will need to spoof their "user id" to 4 and issue the flag command to get the flag. 

# Flag

fb{whys_app_when_you_can_whats_app}
