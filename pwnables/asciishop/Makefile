all:
	clang asciishop.c -fsanitize=safe-stack -o asciishop -ggdb

prod:
	clang asciishop.c -Wl,-z,relro,-z,now -fsanitize=safe-stack -pie -fPIC -o asciishop -w
	strip --strip-all asciishop

final:
	clang asciishop_dirty.c -Wl,-z,relro,-z,now -fsanitize=safe-stack -pie -fPIC -o asciishop -w
	strip --strip-all asciishop
	cp asciishop dist
