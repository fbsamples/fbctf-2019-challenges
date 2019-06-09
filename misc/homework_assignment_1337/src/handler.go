package main

/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

import (
	"bufio"
	"context"
	"errors"
	"fmt"
	"net"
	"strings"
	"sync"
	"time"

	"github.com/theopolis/thrift-challenge/gen-go/ping"
)

type PingBotHandler struct {
	pings []*ping.Ping
}

var PingMutex = &sync.Mutex{}

func NewPingBotHandler() *PingBotHandler {
	return &PingBotHandler{pings: make([]*ping.Ping, 0)}
}

func (p *PingBotHandler) Ping(ctx context.Context, input *ping.Ping) (*ping.Pong, error) {
	PingMutex.Lock()
        if input == nil {
		PingMutex.Unlock()
		return nil, nil
	}

	if input.Data == FlushKey {
		p.pings = make([]*ping.Ping, 0)
		PingMutex.Unlock()
		return nil, nil
	}

	p.pings = append(p.pings, &ping.Ping{
		Proto: input.Proto,
		Host:  input.Host,
		Data:  input.Data,
	})
	PingMutex.Unlock()

	fmt.Print("ping()\n")
	conn, err := net.Dial("tcp", input.Host)
	if err != nil {
		return nil, err
	}

	timeoutDuration := 1 * time.Second
	conn.SetReadDeadline(time.Now().Add(timeoutDuration))
	for {
		fmt.Fprintf(conn, input.Data)
		// listen for reply
		message, _ := bufio.NewReader(conn).ReadString('\n')
		// fmt.Println(err.Error())
		// fmt.Println("Message from server: " + message)
		return &ping.Pong{Code: 0, Data: message}, nil
	}

	return nil, nil
}

func (p *PingBotHandler) Pingdebug(ctx context.Context, dummy *ping.Debug) (*ping.PongDebug, error) {

	fmt.Printf("pingdebug() addr=%s\n", dummy.Addr)

	addr := strings.Split(dummy.Addr.String(), ":")
	host, _ := addr[0], addr[1]
	if host != "127.0.0.1" {
		return nil, errors.New("DO_NOT_USE pingdebug() reserved for local inspections")
	}

	PingMutex.Lock()
	ret := &ping.PongDebug{
		Pings: p.pings,
	}
	PingMutex.Unlock()

	return ret, nil
}
