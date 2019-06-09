.PHONY: clean run

CC=gcc
CFLAGS=--std=c99 -Wextra -Wall -pedantic -fno-stack-protector

overfloat: src/overfloat.c
	$(CC) $(CFLAGS) -o dist/$@ $^

docker: Dockerfile
	docker build -t 'fbctf:overfloat' .

run:
	docker run -p 2303:2302 fbctf:overfloat

clean:
	rm -f dist/overfloat
