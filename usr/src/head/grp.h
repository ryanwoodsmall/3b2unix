/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:grp.h	1.3"
/*	3.0 SID #	1.1	*/
struct	group {	/* see getgrent(3) */
	char	*gr_name;
	char	*gr_passwd;
	int	gr_gid;
	char	**gr_mem;
};
