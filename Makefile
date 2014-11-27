
# Ooooh, look--a Makefile!

#	$Id: Makefile,v 1.9 1993/08/07 00:42:27 jaw Exp jaw $

#	tricky make file, you may need gmake if yours won't handle it

# Target would best be set to one of 96 or 48
# so as to create an assembler for that device
# if you don't want an assembler, leave this blank
# and generate error messges which may enjoyed for
# hours and hours!

#TARGET = 96
 TARGET = 48 96

# have you got a C++ compiler (I hope so!)
# tell me its name

# CCC = CC
CCC = g++

# you prefer something else? go ahead, change it...
CPPFLAGS = -I. $(PROC) 
CCFLAGS = -g -O

# make install likes to put things in nice cozy places
# where is a good place?

BINDIR = /usr/local/bin
MANDIR = /usr/local/man
MANEXT = 1
MANPAGE = asx


# you ought not be changing anything below here

# for older makes
SUFFIXES += .cc
.SUFFIXES: $(SUFFIXES)
.cc.o:
	$(CCC) $(CCFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c $<

OBJS48 = codegen.o error.o expr.o input.o object.o stack.o symtab.o parse.o\
	lexer.o parse48.o main.o

OBJS96 = codegen.o error.o expr.o input.o object.o stack.o symtab.o parse.o\
	lexer.o parse96.o main.o

all: directories
	for t in $(TARGET) ; do\
		cd src$$t ;\
		$(MAKE) PROC=-DMCS$$t ../as$$t ;\
		cd .. ;\
	done
directories: src/Makefile
	for t in $(TARGET) ; do\
		if test ! -d src$$t ; then\
			mkdir src$$t ;\
			cd src ;\
			for f in * ; do\
				ln -s ../src/$$f ../src$$t/$$f ;\
			done ;\
			cd .. ;\
		fi ;\
	done
src/Makefile:
	ln -s ../Makefile src/Makefile
as96: $(OBJS96)
	$(CCC) $(CPPFLAGS) $(CCFLAGS) $(LDFLAGS) -o as96 $(OBJS96)
../as96: as96
	cp as96 ..
as48: $(OBJS48)
	$(CCC) $(CPPFLAGS) $(CCFLAGS) $(LDFLAGS) -o as48 $(OBJS48)
../as48: as48
	cp as48 ..
clean:
	for t in $(TARGET) ; do cd src$$t ; $(MAKE) _clean ; cd .. ;done
_clean:
	rm -f $(OBJS96) $(OBJS48) as48 as96 core shar* *tar *% *~ #*#
realclean: _clean
	rm  -rf as48 as96 src48 src96
_rcs:
	cd src; \
	for x in *cc *h ; do \
		echo .|ci -l $$x ; \
	done
rcs: _rcs
	for x in Makefile License Describble README Reference manpage ; do\
		echo .|ci -l $$x ; \
	done
love:
	@echo not war
war:
	@echo not love\?
install: all
	for t in $(TARGET) ; do install -c -m 755 as$$t $(BINDIR) ; done
	install -c -m 444 manpage $(MANDIR)/man$(MANEXT)/$(MANPAGE).$(MANEXT)
print:
	vgrind -t -lc++ *.h *.cc|pscat|psnup -4|lpr
_depends:
	@echo making depends
	@cd src ;\
	$(CCC) -M $(CPPFLAGS) -DMCS96 *.cc >../depends 2>/dev/null ;\
	cd ..
_Makefile: _depends
	@echo making Makefile
	@cp Makefile Makefile.bak
	@sed -n '1,/#END_[oO]F_MAKE/p' Makefile >Make.new
	@cat Make.new depends >Makefile
	@rm -f Make.new
#	@echo restarting make...
#	$(MAKE)

# DO NOT remove the following line
#END_OF_MAKE
