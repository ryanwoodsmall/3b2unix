/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/_insch.c	1.1"

#define		NOMACROS
#include	"curses_inc.h"

insch(c)
chtype	c;
{
    return (winsch(stdscr, c));
}
