/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/comobj/newstats.c	6.3"
# include	"../../hdr/defines.h"

newstats(pkt,strp,ch)
register struct packet *pkt;
register char *strp;
register char *ch;
{
	char fivech[6];
	register char *r;
	int i;

	r = fivech;
	for (i=0; i < 5; i++)
		*r++ = *ch;
	*r = '\0';
	sprintf(strp,"%c%c %s/%s/%s\n",CTLCHAR,STATS,fivech,fivech,fivech);
	putline(pkt,strp);
}
