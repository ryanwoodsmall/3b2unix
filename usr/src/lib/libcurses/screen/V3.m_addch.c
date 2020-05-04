/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/V3.m_addch.c	1.1"

#include	"curses_inc.h"
extern	int	_outchar();

#ifdef	_VR3_COMPAT_CODE
m_addch(c)
_ochtype	c;
{
    return (waddch(stdscr, c));
}
#endif	/* _VR3_COMPAT_CODE */