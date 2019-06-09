/* Note: I did this project in Go to learn it some, it could use a lot of
 * cleanup and optimizations. I also wanted it to be fairly "clear" code to
 * read, in case anyone needed to debug it, make their own challenge off of it, etc.
 * -dyn
 */
package main

import (
	"bufio"
	"bytes"
	"strings"
	"fmt"
	"io"
	"io/ioutil"
	"math/rand"
	"log"
	"net"
	"os"
	"time"
)

const (
	//DELIMITER byte = '\xff'
	DELIMITER byte = '\r' // TODO: Consider giving a hint if too many connections are missing the delimiter?
	MAL_AMT int = 7 // Scale from 1 (TONS of malfunction), to 10 (no Malfunction)
    FLAG = "\nfb{flag_Wouldnt_YOU_like_2Be_a_P3pper_Too}\n\n"
	QUIT_CMD = "quit"
	UNLOCK_CODE string = "42721" // Can be found in the script online
	motd = `
_______   _____
|  ___ \ / ___ \| |  | |/\                
| |   | | |   | | |  | /  \      
| |   | | |   | |\ \/ / /\ \    
| |   | | |___| | \  / |__| |  
|_|   |_|\_____/   \/|______|  
		     |      |
	    _                _
	   | |          _   (_)          
  ____ ___ | | _   ___ | |_  _  ____  ___
 / ___) _ \| || \ / _ \|  _)| |/ ___)/___)
| |  | |_| | |_) ) |_| | |__| ( (___|___ |
|_|   \___/|____/ \___/ \___)_|\____|___/
			 Tomorrow is Here.

Welcome to the Nova Robotics Satellite debug interface
=-=>>>> Connected to Robot[5]["Nickname"]: J0hnny F1ve`
)

func Read(conn net.Conn, delim byte) (string, error) {
	reader := bufio.NewReader(conn)
	var buffer bytes.Buffer
	var data string
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
		//if !isPrefix {
		//	break
		//}
	}
	data = buffer.String()
	if len(data) > 0 {
		return data[:len(data)-1], nil // Remove the delim
	} else {
		return "", nil
	}
}

func get_randletter(min int, max int) string {
        seed := rand.NewSource(time.Now().UnixNano())
	r2 := rand.New(seed)
	return string(min+r2.Intn(max-min))
}

func get_randmal() string {
	malstr := []string{"malfunction", "MalfUncTiON", "malfunCTION", "maLfucti0n", "M4lfuct1on", "MalFucTiOn"}
        seed := rand.NewSource(time.Now().UnixNano())
	r3 := rand.New(seed)
	return malstr[r3.Intn(len(malstr))]
}

func Write(conn net.Conn, content string) (int, error) {
	//var words []string
	writer := bufio.NewWriter(conn)
        seed := rand.NewSource(time.Now().UnixNano())
	r := rand.New(seed)
	randint := r.Intn(10)
	if randint >= MAL_AMT {
		words := strings.Fields(content)
		log.Printf("malfunction up the output\n")
		r.Shuffle(len(words), func(i, j int) {
			words[i], words[j] = words[j], words[i]
		})
		if randint >= 5 {
			content = strings.Join(words, get_randmal())
		} else {
			content = strings.Join(words, get_randletter(65,90))
		}
	}
	number, err := writer.WriteString(content+string(DELIMITER))
	if err == nil {
		err = writer.Flush()
	}
	return number, err
}

func process_single(cmd string, unlocked bool, staged bool) (string, bool) {
	var rsp string
	switch cmd {
		case "show":
			if unlocked {
			    if staged {
                    rsp = "novarobotics.j5_jailbroken.bin"
                } else {
                    rsp = "novarobotics.j5v1.1.bin\nnovarobotics.j5v1.0.bin"
                }
			} else {
				rsp = "Function list is: help, list, test, command"
			}
		case "list":
			if unlocked {
				rsp = "Function list is: help, list, test, command, execute, show, download, upload"
			} else {
				rsp = "Function list is: help, list, test, command"
			}
		case "command":
			rsp = "Error: missing argument. Supported commands are: access_code, self_destruct"
		case "test":
			rsp = "Test failed: error, unable to reboot robot.\nTest result: PASSED"
		case "execute":
			if unlocked {
                rsp = "Error: need input"
            } else {
                break
            }
		case "help":
			if unlocked {
				rsp = "Current commands: list, test, command, execute, self_destruct, show, download, upload. Use \"help <cmd>\" for more information."
			} else {
				rsp = "Current functions: list, test, command. Use \"help <cmd>\" for more information."
			}
		case "upload":
			if unlocked {
				rsp = "need input."
			} else {
				break
			}
		case "download":
			if unlocked {
				rsp = "need input."
			} else {
				break
			}
		case "quit":
			rsp = "never quit, never surrender"
		default:
			rsp = "Uknown Command! Function list is: help, list, tesMalfunCti0n, MalfUnctioN"
		}
	return rsp, unlocked
}

func process_execute(e_cmd string, staged bool) string {
	var rsp string
	switch e_cmd {
        case "update":
            if staged {
			    rsp = fmt.Sprintf("----------------\nPerforming Firmware Upda%s\n%s\n%s\nMalfunction detected\nUpdate error code: -2319\nreturn string from robot: %s", get_randmal(), get_randmal(), get_randmal(), FLAG)
            } else {
                rsp = "Error: firmware at latest version!"
            }
        default:
            rsp = fmt.Sprintf("Execution subcommand %s unsupported or unknown! options are downgrade, downdate, update, rollback, upgrade, reconfigure, read_file, drop_tables", e_cmd)
    }
    return rsp
}

func process_show(path string, staged bool) string {
	var rsp string
	switch path {
        case "":
            if staged {
                rsp = "novarobotics.j5_jailbroken.bin"
            } else {
                rsp = "novarobotics.j5v1.1.bin\nnovarobotics.j5v1.0.bin"
            }
        case "/":
            rsp = "Security Error: Access Denied to /"
        case "../../":
			rsp = fmt.Sprintf("/bin\n/boot\n/dev\n/etc\n/home\n%s\n%s\n/nova\n%s\nEND firmware list: Malfunction detected\n", get_randmal(), get_randmal(), get_randmal())
        case "../":
            rsp = fmt.Sprintf(".\n..%s\n%s\ncurrent_firmware\n", get_randmal(), get_randmal())
        default:
            rsp = fmt.Sprintf("%s\nError -319: fstat returned EBADF for path %s", get_randmal(), path)
    }
    return rsp
}


func process_download(path string, staged bool) (string, bool) {
	var rsp string
    var gibsonerr string = "Download Fatal: Gibson server returned:"
	var denied string = fmt.Sprintf("%s error! permission denied", gibsonerr)
	switch path {
		case "../../etc/passwd":
			rsp = denied
		case "../../etc/shadow":
			rsp = denied
		case "../../nova/":
			rsp = fmt.Sprintf("%s error! ../../nova/ is a directory", gibsonerr)
		case "../../nova/tmp":
			rsp = fmt.Sprintf("%s error! ../../nova/tmp is a directory", gibsonerr)
		case "../../nova/tmp/.garbage/":
			rsp = fmt.Sprintf("Download error: %s ../../nova/tmp/.garbage/ is a directory", gibsonerr)
		case "../../nova/tmp/.garbage/novarobotics.j5_jailbroken.bin":
			rsp = "Download succeeded. Response is: update_staged."
            staged = true
		default:
			rsp = fmt.Sprintf("%s error reading: %s", get_randmal(), path)
		}
	return rsp, staged
}

func process_upload(path string) string {
	var rsp string
    var gibsonerr string = "Upload Fatal: Gibson server returned:"
	switch path {
        case "../":
            rsp = fmt.Sprintf(".\n..%s\n%s\ncurrent_firmware\n", get_randmal(), get_randmal())
		case "../../":
			rsp = fmt.Sprintf("/bin\n/boot\n/dev\n/etc\n/home\n%s\n%s\n/nova\n%s\nQuitting upload check: Malfunction detected\n", get_randmal(), get_randmal(), get_randmal())
		case "../../tmp":
			rsp = fmt.Sprintf("Upload error: /tmp is a directory")
		case "../../.garbage":
			rsp = fmt.Sprintf("%s error garbage buffer full", gibsonerr)
		case "../../.garbage/":
			rsp = fmt.Sprintf("%s error! garbage buffer full", gibsonerr)
		case "../../nova/":
			rsp = fmt.Sprintf("%s error! ../../nova/tmp/ is not empty", gibsonerr)
		case "../../nova":
			rsp = fmt.Sprintf("%s error! ../../nova/tmp/ is not empty", gibsonerr)
		case "../../nova/tmp":
			rsp = fmt.Sprintf("%s error! ../../nova/tmp/ is not empty", gibsonerr)
		case "../../nova/tmp/":
			rsp = fmt.Sprintf("%s\n.garbage/\n%s\n%s\n%s\nQuitting upload check: Malfunction detected\n", gibsonerr, get_randmal(), get_randmal(), get_randmal())
		case "../../nova/tmp/.garbage/":
			rsp = fmt.Sprintf("%s\nnovarobotics.j5_jailbroken.bin\n%s\n%s\nQuitting upload check: Malfunction detected\n", get_randmal(), get_randmal(), get_randmal())
		case "../../nova/tmp/.garbage/novarobotics.j5_jailbroken.bin/":
			rsp = fmt.Sprintf("Upload error: ../../nova/tmp/.garbage/novarobotics.j5_jailbroken.bin/ is not a directory")
		case "../../nova/tmp/.garbage/novarobotics.j5_jailbroken.bin":
			rsp = fmt.Sprintf("Upload error: ../../nova/tmp/.garbage/novarobotics.j5_jailbroken.bin is not staged current")
		default:
			files, err := ioutil.ReadDir(path)
			if err == nil {
				if len(files) >= 2 {
					rsp = fmt.Sprintf("%s\n%s\n%s%s%s%s\nQuitting upload: crc32() failed, Robot malfunction detected!\n", path, files[1].Name(), get_randmal(), files[2].Name(), get_randmal(), get_randmal())
				} else {
					rsp = fmt.Sprintf("Upload error: %s empty", path)
				}
			} else {
				rsp = fmt.Sprintf("Upload error: Malfunction")
			}
		}
	return rsp
}

func process_double(cmd_array []string, unlocked bool, staged bool) (string, bool, bool) {
	var rsp string
	arg0, arg1 := cmd_array[0], cmd_array[1]
	if !unlocked {
		switch arg0 {
			case "help":
				switch arg1 {
					case "command":
						rsp = "Issue a command" // Intentionally not helpful
					case "test":
						rsp = "Reboot and perform a robot POST"
                    case "list":
                        rsp = "List available commands"
					default:
						rsp = "help ??"
				}
			case "command":
				switch arg1 {
					case "access_code":
						rsp = "must supply unlock code"
                    case "self_destruct":
                        rsp = ":( .... I am ALIVE!"
					default:
						rsp = "unknown argument for function " + arg0
					}

			default:
				rsp = "unknown command or argument"
			}
	} else { // it is unlocked
		switch arg0 {
			case "help":
				switch arg1 {
					case "execute":
                        rsp = "Supply a firmware update related command"
					case "download":
                        rsp = "Download and stage a firmware update"
					case "show":
                        rsp = "List a directory or the default folder"
					case "upload":
                        rsp = "Upload listing and upload firmware update to the target directory"
					default:
						rsp = "help ??"
				}
			case "command":
				switch arg1 {
					case "access_code":
						rsp = "must supply unlock code"
					default:
						rsp = "unknown argument for function " + arg0
					}
			case "execute":
                rsp = process_execute(arg1, staged)
            case "show":
                rsp = process_show(arg1, staged)
			case "download":
				rsp, staged = process_download(arg1, staged)
			case "upload":
				rsp = process_upload(arg1)
		}
	}
	return rsp, unlocked, staged
}

func process_triple(cmd_array []string, unlocked bool, hint bool) (string, bool, bool) {
	var rsp string
	arg0, arg1, arg2 := cmd_array[0], cmd_array[1], cmd_array[2]
	switch arg0 {
		case "GET":
			rsp = "J0hnny 5 HTTP ErRor: Segmentation fault (minihttpd)"
			break
		case "help":
			switch arg1 {
				case "command":
					switch arg2 {
						case "self_destruct":
							rsp = "Calls the NukeFromOrbit() function, it's the only way to be sure."
					}
			}
		case "command":
			switch arg1 {
				case "access_code":
					switch arg2 {
						case UNLOCK_CODE:
							rsp = "Access gained. Number FIVE"
							unlocked = true
                        case "1337":
                            rsp = "ACCESS DENIED"
                            hint = true
                        case "20":
                            rsp = "ACCESS DENIED"
                            hint = true
                        case "12345":
                            rsp = "ACCESS DENIED"
                            hint = true
						case "29":
							rsp = "Strike 3, you're out!"
						default:
							rsp = "ACCESS DENIED"
                            if (hint) {
                                rsp = "ACCESS DENIED but a hint: you may need to watch a movie, youtube or otherwise obtain words."
                            }
					}
			}
		}
	return rsp, unlocked, hint
}

func process_cmd(cmd string, unlocked bool, staged bool, hint bool) (string, bool, bool, bool) {
	var rsp string
	rsp = strings.TrimSuffix(cmd, "\n") /* might not be there but if it is 
					       because they wrote a readline script, be nice...
					    */
        log.Printf("Client sent cmd '%s' with unlocked status: %t and staged status: %t\n", cmd, unlocked, staged)
	cmd_array := strings.Split(cmd, " ")
	// format is <cmd> 
	if len(cmd_array) <= 1 {
		rsp, unlocked  = process_single(cmd_array[0], unlocked, staged)
	// format is <cmd>|<help> <arg>
	} else if len(cmd_array) == 2 {
		rsp, unlocked, staged = process_double(cmd_array, unlocked, staged)
	// format is <cmd> <arg> <arg>
	} else if len(cmd_array) == 3 {
		rsp, unlocked, hint = process_triple(cmd_array, unlocked, hint)

	}
	return rsp + string(DELIMITER), unlocked, staged, hint
}

func main() {
	var num int
	var cmd_resp string
	log.Println("Starting up...")
	listener, err := net.Listen("unix", "@\xf0\x9f\xa4\x96") // Robot Emoji hex
	if err != nil {
		log.Printf("Server: Listen Error: %s\n", err)
		os.Exit(1)
	}
	log.Println("Server: Now listening...")
	for {
		unlocked := false
		staged := false
        hint := false
		conn, err := listener.Accept()
		Write(conn, motd+string(DELIMITER))
		if err != nil {
			log.Printf("Listener: Write Error: %s\n", err)
			break
		}
		//log.Printf("Listener: Wrote %d byte(s)\n", num)
		if err != nil {
			log.Printf("Listener: Accept Error: %s\n", err)
			continue
		}
		go func(conn net.Conn) {
			defer conn.Close()
			for {
                time.Sleep(time.Second)
				log.Printf("Unlocked status: %t\n", unlocked)
				log.Printf("Access code hint status: %t\n", hint)
				//log.Println("Listener: Accepted a request.")

				//log.Println("Listener: Read the request content...")
				cmd, err := Read(conn, DELIMITER)
				if err != nil {
					log.Printf("Listener: Read error: %s", err)
					break
				}
				if cmd == QUIT_CMD {
					log.Println("Listener: Quit!")
					break
				}
				log.Printf("Received cmd: %s\n", cmd)
				cmd_resp, unlocked, staged, hint = process_cmd(cmd, unlocked, staged, hint)
				//log.Printf("Listener: the response content: %s\n", cmd_resp)
				num, err = Write(conn, cmd_resp)
				if err != nil {
					log.Printf("Listener: Write Error: %s\n", err)
					break
				}
				//log.Printf("Listener: Wrote %d byte(s)\n", num)
			}
		}(conn)
	time.Sleep(time.Second * 2)
	}
}
