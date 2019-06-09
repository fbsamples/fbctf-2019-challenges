all: solve

solve: solve.c
	clang -g -o $@ -std=c99 -lcrypto $^

clean:
	rm -f *.o solve
