MAKEFLAGS+=-Rr
SHELL:=/bin/bash
.ONESHELL:
.SECONDEXPANSION:

srcdir:=..
vpath %.c $(srcdir)
vpath %.h $(srcdir)

target:=trie.t.tsk
obj:=trie.o trie.t.o
dfiles:=$(obj:.o=.d)
.SECONDARY: $(obj)
.NOTINTERMEDIATE: $(dfiles) %.h

asan_flags:=-fsanitize=address -fsanitize=pointer-compare -fsanitize=leak -fsanitize=undefined
all_ldflags:=-Wl,--hash-style=gnu $(asan_flags) $(LDFLAGS)
all: $(target)
$(target): $(obj)
	gcc -o $@ $(all_ldflags) $^

all_cppflags:=-I$(srcdir) $(CPPFLAGS)
all_cflags:=-Wall -Wextra -Werror -ggdb -O0 -m64 $(asan_flags) $(CFLAGS)

%.o: %.c %.d $$(file <%.d)
	gcc $(all_cppflags) $(all_cflags) -MMD -MF $*.td -o $@ -c $<
	read obj src headers <$*.td; echo "$$headers" >$*.d
	touch -c $@

$(dfiles):;
%.h:;

clean:
	rm -f $(target) $(obj) $(dfiles) $(obj:.o=.td)

.PHONY: all clean
makefile::;
