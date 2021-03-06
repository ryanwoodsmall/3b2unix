/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:m32/gendefs.h	1.10"
/*
 */

#define  LESS    -1
#define  EQUAL    0
#define  GREATER  1

#define  NO       0
#define  YES      1

#define NCPS	8	/* number of characters per symbol */
#define BITSPBY	8
#define BITSPOW	8
#define BITSPW	32
#define OUTWTYPE	char
/* if the previous line changes the following line should be change to look like
 * #define	OUT(a,b)	fwrite((char *)(&a), sizeof(OUTWTYPE), 1, b)
*/
#define	OUT(a,b)	putc((a), b)
#define	MAX(a,b)	(a > b) ? a : b

#define NBPW	32

#define SCTALIGN 4L /* byte alignment for sections */
#define TXTFILL	0x70L
#define FILL 0L
#define NULLVAL 0L
#define NULLSYM ((symbol *)NULL)

/* constants used in testing and flag parsing */

#define ARGERR	"Illegal argument - ignored\n"
#define TESTVAL	-2
#define NSECTIONS 9
#define NFILES	NSECTIONS + 6	/* number of section files + other files */

/* index of action routines in modes array */

#define	NOACTION	0
#define	DEFINE	1
#define	SETVAL	2
#define	SETSCL	3
#define	SETTYP	4
#define	SETTAG	5
#define	SETLNO	6
#define	SETSIZ	7
#define	SETDIM1	8
#define	SETDIM2	9
#define	ENDEF	10
#define	LLINENO	11
#define	LLINENUM	12
#define	LLINEVAL	13
#define	NEWSTMT	14
#define	SETFILE	15
#define	RELDIR32	16
#define	RESABS	17
#define	RELPC8	18
#define	RELPC16	19
#define	BRNOPT	20
#define	BSBNOPT	21
#define FNDBRLEN	22
#define	SHIFTVAL	23
#define	RELDAT32	24
#define	SAVTRANS	25
#define SETEXPAND	26
#define	DOTZERO		27
#ifdef	CALLPCREL
#define	CALLNOPT	28
#define	NACTION		28
#else
#define	NACTION		27
#endif
