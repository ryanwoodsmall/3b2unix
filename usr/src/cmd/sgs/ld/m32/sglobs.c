/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:m32/sglobs.c	1.5"

#include <stdio.h>

#include "system.h"
#include "structs.h"
#include "tv.h"
#include "ldtv.h"
#include "sgs.h"

char	Xflag = 1;      /* generate optional header with "old"
/*eject*/
/*
 * Structure of information needed about the transfer vector (and the
 * .tv section).  Part of this structure is added to outsclst, so that
 * the list must not be freed before the last use of tvspec.
 */

TVINFO	tvspec = {
	NULL,		/* tvosptr */
	"",		/* tvfnfill: fill name for tv slots	*/
	-1L,		/* tvfill: fill value for tv slots	*/
	NULL,		/* tvinflnm: file containing tv specs	*/
	0,		/* tvinlnno: line nbr of tv directive	*/
	0,		/* tvlength: tv area length		*/
	-1L,		/* tvbndadr: tv area bond address	*/
	0, 0		/* tvrange				*/
	};

unsigned short magic = (unsigned short) MAGIC;
