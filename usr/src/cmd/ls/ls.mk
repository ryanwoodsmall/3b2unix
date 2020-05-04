#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ls:ls.mk	1.12"

#	Makefile for ls

ROOT =

DIR = $(ROOT)/bin

INC = $(ROOT)/usr/include

LDFLAGS = -s -lcurses $(LDLIBS)

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

#top#

MAKEFILE = ls.mk

MAINS = ls

OBJECTS =  ls.o

SOURCES =  ls.c

ALL:		$(MAINS)

ls:	ls.o
	$(CC) $(CFLAGS) -o ls ls.o $(LDFLAGS)

ls.o:		 $(INC)/sys/param.h \
		 $(INC)/sys/types.h \
		 $(INC)/sys/sysmacros.h \
		 $(INC)/sys/stat.h \
		 $(INC)/stdio.h \
		 $(INC)/dirent.h \
		 $(INC)/sys/dirent.h \
		 $(INC)/curses.h	\
		 $(INC)/stdio.h \
		 $(INC)/unctrl.h	\
		 $(INC)/termio.h	\
		 $(INC)/sys/termio.h \
		 $(INC)/term.h 

GLOBALINCS = $(INC)/curses.h \
	$(INC)/stdio.h \
	$(INC)/dirent.h \
	$(INC)/sys/dirent.h \
	$(INC)/sys/param.h \
	$(INC)/sys/stat.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/termio.h \
	$(INC)/sys/types.h \
	$(INC)/term.h \
	$(INC)/termio.h \
	$(INC)/unctrl.h 


clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

newmakefile:
	makefile -m -f $(MAKEFILE)  -s INC $(INC)
#bottom#

all : ALL

install: ALL
	cpset $(MAINS) $(DIR)

size: ALL
	$(SIZE) $(MAINS)

strip: ALL
	$(STRIP) $(MAINS)

#	These targets are useful but optional

partslist:
	@echo $(MAKEFILE) $(SOURCES) $(LOCALINCS)  |  tr ' ' '\012'  |  sort

productdir:
	@echo $(DIR) | tr ' ' '\012' | sort

product:
	@echo $(MAINS)  |  tr ' ' '\012'  | \
	sed 's;^;$(DIR)/;'

srcaudit:
	@fileaudit $(MAKEFILE) $(LOCALINCS) $(SOURCES) -o $(OBJECTS) $(MAINS)
