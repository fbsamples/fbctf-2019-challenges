/* Note: I did this project in Go to learn it some, it could use a lot of
* cleanup and optimizations. I also wanted it to be fairly "clear" code to
* read, in case anyone needed to debug it, make their own challenge off of it, etc.
* -dyn
*
* This client application IS NOT part of the challenge, it's simply intended to
* be used for testing the server if you just want to "skip ahead" in the
* challenge and run it all locally.
*
* You can also use socat with the server with:
*      socat readline ABSTRACT-CONNECT:$(echo -ne "\xf0\x9f\xa4\x96"),cr
*/
package main

import (
	"bufio"
	"bytes"
	"fmt"
	"io"
	"log"
	"net"
	"os"
)

const (
	//DELIMITER byte = '\xff'
	DELIMITER byte = '\r'
	QUIT_CMD      = "quit"
)

func Read(conn net.Conn, delim byte) (string, error) {
	reader := bufio.NewReader(conn)
	var buffer bytes.Buffer
	for {
		//ba, isPrefix, err := reader.ReadLine()
		ba, err := reader.ReadBytes(DELIMITER)
		if err != nil {
			if err == io.EOF {
				break
			}
			return "", err
		}
		buffer.Write(ba)
		break
	}
	return buffer.String(), nil
}

func Write(conn net.Conn, content string) (int, error) {
	writer := bufio.NewWriter(conn)
	number, err := writer.WriteString(content)
	if err == nil {
		err = writer.Flush()
	}
	return number, err
}

func main() {
	conn, err := net.Dial("unix", "@\xf0\x9f\xa4\x96")
	if err != nil {
		log.Printf("Sender: Dial Error: %s\n", err)
		os.Exit(1)
	}
	log.Println("Sender: Dial OK.")
	//fmt.Printf("reading...")
	resp, err := Read(conn, DELIMITER)
	if err != nil {
		log.Printf("Sender: Read error: %s", err)
	}
	fmt.Printf("%s\n> ", resp)
	input := bufio.NewScanner(os.Stdin)
	for input.Scan() {
		cmd := input.Text()
		switch cmd {
			case "\n":
				break
			case "exit":
				break
			default:
				send_cmd := fmt.Sprintf("%s%c", cmd, DELIMITER)
				//log.Printf("Sender: the cmd content: %s\n", send_cmd)
				_, err := Write(conn, send_cmd)
				if err != nil {
					log.Printf("Sender: Write Error: %s\n", err)
					break
				}
				//log.Printf("Sender: Wrote %d byte(s)\n", num)
				respContent, err := Read(conn, DELIMITER)
				if err != nil {
					log.Printf("Sender: Read error: %s", err)
					break
				}
				//log.Printf("Sender: Received content: %s\n", respContent)
				fmt.Printf("%s\n> ", respContent)
		}
	}
}
