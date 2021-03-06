#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rfpasswd:rfpasswd.mk	1.8"
ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/bin
INC = $(ROOT)/usr/include
INS = :
CFLAGS = -O -s
LDFLAGS = -lns -lcrypt $(LDLIBS)
FRC =

all: rfpasswd

rfpasswd: rfpasswd.c 
	$(CC) -I$(INC) $(CFLAGS) -o $(TESTDIR)/rfpasswd rfpasswd.c $(LDFLAGS)

install: all
	install -n $(INSDIR) $(TESTDIR)/rfpasswd

clean:
	rm -f rfpasswd.o

clobber: clean
	rm -f $(TESTDIR)/rfpasswd
FRC:
