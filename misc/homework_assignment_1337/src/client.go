package main

import (
	"context"
	"crypto/tls"
	"fmt"

	"github.com/apache/thrift/lib/go/thrift"
	"github.com/theopolis/thrift-challenge/gen-go/ping"
)

var defaultCtx = context.Background()

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

func handleClientSetup(client *ping.PingBotClient, key string) (err error) {
	pings := ping.NewPing()
	pings.Proto = ping.Proto_TCP
	pings.Host = "localhost:9090"
	pings.Data = key

	if msg, err := client.Ping(defaultCtx, pings); err != nil {
		fmt.Printf("ping() error %s\n", err.Error())
	} else {
		fmt.Printf("ping(): %s\n", msg)
	}

	return nil
}

func runClient(transportFactory thrift.TTransportFactory, protocolFactory thrift.TProtocolFactory, addr string, secure, test bool, key string) error {
	var transport thrift.TTransport
	var err error
	if secure {
		cfg := new(tls.Config)
		cfg.InsecureSkipVerify = true
		transport, err = thrift.NewTSSLSocket(addr, cfg)
	} else {
		transport, err = thrift.NewTSocket(addr)
	}
	if err != nil {
		fmt.Println("Error opening socket:", err)
		return err
	}
	if transport == nil {
		return fmt.Errorf("Error opening socket, got nil transport. Is server available?")
	}

	transport, err = transportFactory.GetTransport(transport)
	if transport == nil {
		return fmt.Errorf("Error from transportFactory.GetTransport(), got nil transport. Is server available?")
	}

	err = transport.Open()
	if err != nil {
		return err
	}
	defer transport.Close()

	if test {
		return handleClientSolution(ping.NewPingBotClientFactory(transport, protocolFactory))
	}

	return handleClientSetup(ping.NewPingBotClientFactory(transport, protocolFactory), key)
}
