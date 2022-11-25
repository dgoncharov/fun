MAKEFLAGS=-Rr
.SUFFIXES:
CFLAGS:=-Wall -Wextra -ggdb -O0 -m64 -fsanitize=address -fsanitize=pointer-compare -fsanitize=undefined -fsanitize=leak 
all:: palindrome
palindrome: palindrome.c; gcc $(CFLAGS) -o $@ $<
makefile::;
