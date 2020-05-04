#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)passwd:passwd.mk	1.5.1.1"

#	Makefile for passwd

ROOT =

DIR = $(ROOT)/bin

INC = $(ROOT)/usr/include

LDFLAGS = -s -lsec -lcrypt $(LDLIBS)

CFLAGS =  -O -I$(INC)

STRIP = strip

SIZE = size

#top#
# Generated by makefile 1.47

MAKEFILE = passwd.mk


MAINS = passwd

OBJECTS =  passwd.o

SOURCES =  passwd.c

ALL:		$(MAINS)

passwd:		passwd.o 
	$(CC) $(CFLAGS)  -o passwd  passwd.o   $(LDFLAGS)


passwd.o:	 $(INC)/stdio.h $(INC)/signal.h \
		 $(INC)/sys/signal.h $(INC)/pwd.h \
		 $(INC)/sys/types.h $(INC)/ctype.h 

GLOBALINCS = $(INC)/ctype.h $(INC)/pwd.h $(INC)/shadow.h $(INC)/signal.h \
	$(INC)/stdio.h $(INC)/sys/signal.h \
	$(INC)/sys/types.h 


clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

newmakefile:
	makefile -m -f $(MAKEFILE)  -s INC $(INC)
#bottom#

all : ALL

install: ALL
	cpset $(MAINS) $(DIR) 6755 root sys

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
