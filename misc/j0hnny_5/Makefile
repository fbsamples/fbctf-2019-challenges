
client = client.go
server = server.go

client:
	go build ${client} 

server:
	go build ${server}

static:
	CGO_ENABLED=0 GOOS=linux go build -a -installsuffix cgo -o server ${server}

all:
	go build ${client} 
	go build ${server}

.PHONY: server
