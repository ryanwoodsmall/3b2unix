#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)graf:src/dev.d/uplib/Makefile	1.7"
###
### Standard Constants
###

ROOT     =
LROOT    = 
BIN      = $(ROOT)/$(LROOT)/usr/bin/graf
LIB      = uplib.a

INC	 = $(ROOT)/usr/include
LINC	 = ../include
LINC2    = ../../../include

CC       = cc
CFLAGS   = -c -O -I$(INC) -I$(LINC) -I$(LINC2) $(FFLAG)


#top#
# Generated by makefile 1.44    Mon Mar 11 13:35:30 EST 1985

MAKEFILE = Makefile


LIBRARY = uplib.a

OBJECTS =  arcfill.o clipseg.o compwind.o dispfile.o dispprim.o inarea.o \
	linefill.o softarc.o softtext.o utodcoord.o

SOURCES =  arcfill.c clipseg.c compwind.c dispfile.c dispprim.c inarea.c \
	linefill.c softarc.c softtext.c utodcoord.c

ALL:		$(LIBRARY)

$(LIBRARY):	$(LIBRARY)(utodcoord.o)	$(LIBRARY)(softtext.o) \
		$(LIBRARY)(softarc.o) $(LIBRARY)(linefill.o) \
		$(LIBRARY)(inarea.o) $(LIBRARY)(dispprim.o) \
		$(LIBRARY)(dispfile.o) $(LIBRARY)(compwind.o) \
		$(LIBRARY)(clipseg.o) $(LIBRARY)(arcfill.o) 


$(LIBRARY)(arcfill.o):	 $(INC)/stdio.h $(LINC)/dev.h \
		 $(LINC2)/debug.h $(LINC2)/errpr.h \
		 $(LINC2)/util.h $(INC)/ctype.h \
		 $(LINC2)/gsl.h	$(LINC2)/gpl.h \
		 $(LINC)/fill.h 

$(LIBRARY)(clipseg.o):	 $(INC)/stdio.h $(LINC)/dev.h \
		 $(LINC2)/debug.h $(LINC2)/errpr.h \
		 $(LINC2)/util.h $(INC)/ctype.h \
		 $(LINC2)/gsl.h	$(LINC2)/gpl.h 

$(LIBRARY)(compwind.o):	 $(INC)/stdio.h $(LINC)/dev.h \
		 $(LINC2)/debug.h $(LINC2)/errpr.h \
		 $(LINC2)/util.h $(INC)/ctype.h \
		 $(LINC2)/gsl.h	$(LINC2)/gpl.h 

$(LIBRARY)(dispfile.o):	 $(INC)/stdio.h $(LINC)/dev.h \
		 $(LINC2)/debug.h $(LINC2)/errpr.h \
		 $(LINC2)/util.h $(INC)/ctype.h \
		 $(LINC2)/gsl.h	$(LINC2)/gpl.h 

$(LIBRARY)(dispprim.o):	 $(INC)/stdio.h $(LINC)/dev.h \
		 $(LINC2)/debug.h $(LINC2)/errpr.h \
		 $(LINC2)/util.h $(INC)/ctype.h \
		 $(LINC2)/gsl.h	$(LINC2)/gpl.h 

$(LIBRARY)(inarea.o):	 $(INC)/stdio.h $(LINC)/dev.h \
		 $(LINC2)/debug.h $(LINC2)/errpr.h \
		 $(LINC2)/util.h $(INC)/ctype.h \
		 $(LINC2)/gsl.h	$(LINC2)/gpl.h 

$(LIBRARY)(linefill.o):	 $(INC)/stdio.h $(LINC)/dev.h \
		 $(LINC2)/debug.h $(LINC2)/errpr.h \
		 $(LINC2)/util.h $(INC)/ctype.h \
		 $(LINC2)/gsl.h	$(LINC2)/gpl.h \
		 $(LINC)/fill.h 

$(LIBRARY)(softarc.o):	 $(INC)/stdio.h $(LINC)/dev.h \
		 $(LINC2)/debug.h $(LINC2)/errpr.h \
		 $(LINC2)/util.h $(INC)/ctype.h \
		 $(LINC2)/gsl.h	$(LINC2)/gpl.h 

$(LIBRARY)(softtext.o):	 $(INC)/stdio.h $(LINC)/dev.h \
		 $(LINC2)/debug.h $(LINC2)/errpr.h \
		 $(LINC2)/util.h $(INC)/ctype.h \
		 $(LINC2)/gsl.h	$(LINC2)/gpl.h 

$(LIBRARY)(utodcoord.o):	 $(INC)/stdio.h \
		 $(LINC)/dev.h $(LINC2)/debug.h \
		 $(LINC2)/errpr.h $(LINC2)/util.h \
		 $(INC)/ctype.h $(LINC2)/gsl.h \
		 $(LINC2)/gpl.h	

GLOBALINCS = $(LINC2)/debug.h $(LINC2)/errpr.h \
	$(LINC2)/gpl.h $(LINC2)/gsl.h \
	$(LINC2)/util.h $(LINC)/dev.h $(LINC)/fill.h \
	$(INC)/ctype.h $(INC)/stdio.h 



clean:
	rm -f $(OBJECTS)
	
clobber:	
	rm -f $(OBJECTS) $(LIBRARY)

newmakefile:
	makefile -m -f $(MAKEFILE) -L $(LIBRARY)  -s LINC2 $(LINC2) -s INC $(INC) -s LINC $(LINC)
#bottom#
install      : $(LIBRARY)
	cpset $(LIBRARY) $(LIB)
	
###########
	
listings     : $(SOURCES)
	pr -h "     uplib.d     " $(SOURCES) | lp
