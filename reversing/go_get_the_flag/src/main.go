package main

import (
	"crypto/aes"
	"crypto/cipher"
	"crypto/md5"
	"encoding/hex"
	"fmt"
	"os"
)

func check(err error) {
	if err != nil {
		panic(err)
	}
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println("Please give me a password...")
		os.Exit(1)
	}
	if len(os.Args) >= 3 {
		fmt.Println("Woah... you gave me too many arguments!")
		os.Exit(1)
	}
	password := os.Args[1]
	checkpassword(password)
}

func checkpassword(password string) {
	if len(password) != 18 {
		wrongpass()
	}
	if password == "s0_M4NY_Func710n2!" {
		keystr := fmt.Sprintf("%x%x", md5.Sum([]byte(password)), md5.Sum([]byte(password)))
		decryptflag(keystr)
	} else {
		wrongpass()
	}
}

func decryptflag(keystr string) {
	key, err := hex.DecodeString(keystr)
	check(err)
	ciphertext, err := hex.DecodeString("c55dc63af806636fbc3bc81051c279c92ec9a6607aedaa877018cf4dbcc53cb2d23ac2fe04b8ae5d24")
	check(err)
	nonce, err := hex.DecodeString("e738989a5bcb3c93a1101010")
	block, err := aes.NewCipher(key)
	check(err)
	aesgcm, err := cipher.NewGCM(block)
	check(err)
	plaintext, err := aesgcm.Open(nil, nonce, ciphertext, nil)
	check(err)
	fmt.Printf("%s\n", plaintext)
}

func wrongpass() {
	fmt.Println("Wrong password!")
	os.Exit(0)
}
