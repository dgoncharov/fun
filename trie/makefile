MAKEFLAGS+=-Rr
SHELL:=/bin/bash
.ONESHELL:
.SECONDEXPANSION:
.NOTINTERMEDIATE: %.d %.h
.SECONDARY: trie.o trie.t.o

srcdir:=..
vpath %.c $(srcdir)
vpath %.h $(srcdir)

all_cppflags:=-I$(srcdir) $(CPPFLAGS)
all_cflags:=-Wall -Wextra -Werror -ggdb -O0 -m64 $(CFLAGS)

obj:=trie.o trie.t.o
dfiles:=$(obj:.o=.d)

all: trie.t.tsk
trie.t.tsk: $(obj)
	gcc -o $@ $^

%.o: %.c %.d $$(file <%.d)
	gcc $(all_cppflags) $(all_cflags) -MMD -MF $*.td -o $@ -c $<
	read obj src headers <$*.td; echo "$$headers" >$*.d
	touch -c $@

$(dfiles): ;
%.h:;

clean:
	rm -f trie.t.tsk $(obj) $(dfiles) $(obj:.o=.td)

makefile::;
