The modern technique of building dependencies automatically uses include
directive.
While this technique is superior to the manual maintenance of dep files there
is still room for improvement.
The main issue is that the include directive is not a part of the dag. include
is unconditional.


E.g consider the following makefile


obj:=$(wildcard *.c)
dfiles:=$(obj:.o=.d)
%.o: %.c %.d
	gcc -I. -MMD -o $@ -c $<
%.d: ;
include $(dfiles)

When the user runs
$ make hello.o

make includes all the dfiles in the current directory, even though only hello.d
is needed.


This becomes especially problematic when a project has one test driver per
source code file. E.g. a lib can have implementation files api.c, util.c, and engine.c
and test programs api.t.c, util.t.c and engine.t.c. Each test program contains
main and is supposed to be compiled and linked with the module that it tests.
e.g.

%.t.tsk: %.o %.t.o
    $(CC) -o $@ $^

When the user runs
$ make api.t.tsk

only api.d and api.t.d are required to be included.

Variations of this style are commonly used. E.g. glibc uses it (except the tests are called
tst-<module.c).

This becomes especially unpleasant when it is desired to avoid hardcoding
the names of the object files in a makefile.
E.g. a makefile contains a set of implicit rules and is reused for multiple
projects.



Another issue with unconditional include is phony targets, such as clean, gzip,
etc.

The usual workaround is to to figure out the specified target and have a set of
ifeq statements to avoid including dfiles. This piece of ifeq code
is difficult, especially when there are a lot of phony targets and the user
can specify multiple targets on the command line. E.g.
$ make api.t.tsk gzip


All of these issue are caused by the same root of the evil - unconditional
include.

It is possible to achieve the same automatic generation of dfiles, but
without unconditional include.
Consider

SHELL:=/bin/bash
.ONESHELL:
.SECONDEXPANSION: %.o

%.o: %.c %.d $$(file <%.d)
	gcc $(CPPFLAGS) $(CFLAGS) -MD -MF $*.td -o $@ -c $<
	read obj src headers <$*.td
	echo "$$headers" >$*.d
	touch -c $@

%.d: ;
%.h: ;


Here, second expantion is used to append the contents of %.d to the list of
prereqs.

-MD is used to generate a regular dep file.

	read obj src headers <$*.td
	echo "$$headers" >$*.d
is used to extract header files from the generated .td file and store this list of
headers files to a .d file.


Note, we are not using gcc's option -MP. The content is of the form

api.o: api.c api.h <other header files>...\
    <more header files>...

This postprocessing handle the gcc format of dep files. To handle aix and sun
format read has to be in a loop.

There is only one missing piece in this makefile. make considers generated dep
files and all headers files to be intermediate.
The usual workaround is to list all intermediate files explicitly as targets.

Listing all dep files explicitly hinders reuse of this makefile between project
and causes an additional maintenance burden

There is nothing explicitly specified in this makefile. There are 3 implicit
rules which figure out what to remake.
What is missing is ability to specify that %.d and %.h are not to be treated as
intermediate.


So, we need to introduce another special target .NOTINTERMEDIATE and our
makefile becomes


SHELL:=/bin/bash
.ONESHELL:
.SECONDEXPANSION: %.o
.NOTINTERMEDIATE: %.d %.h

%.o: %.c %.d $$(file <%.d)
	gcc $(CPPFLAGS) $(CFLAGS) -MD -MF $*.td -o $@ -c $<
	read obj src headers <$*.td
	echo "$$headers" >$*.d
	touch -c $@

%.d: ;
%.h: ;


This makefile solves all the above specified issues of unconditional
include.
An additional bonus is that $(file) is faster than include. When the file
exists include parses the file and evals its contents and when the file is missing include
searches a list of directories. $(file) does none of that.

With the fix from sv (thanks for applying) it is possible to make dep file and
header files explicit in certain cases w/o .NOTINTERMEDIATE, but with

%.o: %.c $$*.d $$(file <$$*.d)

However,
%.o: %.c %.d $$(file <%.d)
.NOTINTERMEDIATE: %.d %.d

should be preferred. It is more specific patterns are preferred over less specific
and directories are added to the stem.
Also, .NOTINTERMEDIATE could be used with built-in rules, if needed.

gcc has option -MP to generate an explicit target for each header file.
There are compilers (e.g. sun cc and ibm xlc) which do not have such an option.

.NOTINTERMEDIATE %.h
can be used to make header files not intermediate with those compilers.
