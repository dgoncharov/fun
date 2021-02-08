SHELL:=/bin/bash
.ONESHELL:
.SUFFIXES:
MAKEFLAGS+=--no-builtin-rules
MAKEFLAGS+=--no-builtin-variables
.SECONDEXPANSION: %.o
.NOTINTERMEDIATE: %.d %.h
.PHONY: all clean

all: hello.tsk

srcdir:=..
CPPFLAGS:=$(CPPFLAGS) -I$(srcdir)
CFLAGS:=$(CFLAGS) -g -O0 -Wall -Wextra
depflags=-MD -MF $(@:.o=.Td)

obj:=main.o hello.o hello_util.o
dfiles:=$(obj:.o=.d)
vpath %.c $(srcdir)
vpath %.h $(srcdir)

hello.tsk: $(obj)
	gcc $(LDFLAGS) -o $@ $^

%.o: %.c %.d $$(file <%.d)
	gcc $(CPPFLAGS) $(CFLAGS) $(depflags) -o $@ -c $< || exit 1
	$(srcdir)/filter_headers $(@:.o=.Td) >$(@:.o=.d) || exit 1
	touch -c $@


%.d: ;

# Replaces force-target-per-header generarated by -MP.
%.h: ;

clean:
	-rm -f -- $(obj) $(dfiles) hello.tsk
