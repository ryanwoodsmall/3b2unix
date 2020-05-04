/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcc2:m32/local.c	10.5"
/*	local.c - machine dependent stuff for front end
 *
 *		WE 32000
 *
 */

#include <signal.h>
#include "paths.h"		/* to get TMPDIR, directory for temp files */
#include "mfile1.h"

#ifdef REGSET
/* register numbers in the compiler and their external numbers:
**
**	comp	name
**	0-2	r0-r2
**	3	f0
**	4-9	r3-8
**	10-11	f1-2
*/

/* bit vectors for register variables */

#define	REGREGS	(RS_BIT(4)|RS_BIT(5)|RS_BIT(6)|RS_BIT(7)|RS_BIT(8)|RS_BIT(9))
#define	MAUREGS	(RS_BIT(10)|RS_BIT(11))

/* bit vector of all register variables used in a function */
RST maxregused = RS_NONE; /* maximum registers used */
#endif

int dsflag = 0;			/* symbol info initially enabled */
static int dlflag = 0;		/* line number info initially enabled */

#ifdef IMPREGAL
extern int sizeopt;		/* weight optimization info for size if set */
#endif
 
extern char *rnames[];		/* register names	*/

extern int wloop_level;		/* for control of placement of loop tests */
extern int floop_level;	

	/* location counters for PROG, DATA, ADATA, ISTRNG, STRNG */
char *locnames[] =
{
	"	.text\n",
	"	.data\n",
	"	.data\n",
	"	.data	1\n",
	"	.data	1\n",
};

#ifndef REGSET
int minrvar = TOTREGS;
#endif

static char *tmpfn;
static FILE *tmpfp;

#ifdef IN_LINE		/*  asm inline expansion */
FILE *inlfp;		/* file to store asm 'function' bodies */
static char *inlfn;	/* name for file */
#define INNMBUF_LTH  150
static char innmbuf[INNMBUF_LTH];
char *strcpy();
#endif

myexit(n)
{
	unlink(tmpfn);
#ifdef IN_LINE
	unlink(inlfn);
#endif
	if (n == 1)
		n = 51;
	exit(n);
}

catch_fpe()
{
    uerror("floating point constant folding causes exception");
    myexit( 1 );
}

getout()
{
	myexit(55);
}

FILE *fopen();
static int proflag;

extern int singflag;	/* flag for turning on single precision floating arith */

main(argc, argv)
int argc;
char **argv;
{
	int j, m, n;
	char buf[BUFSIZ], *gotnm = NULL, *findnm;

	/* catch signals if they're not now being ignored */

	if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
	    signal(SIGHUP, getout);
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
	    signal(SIGINT, getout);
	if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
	    signal(SIGQUIT, getout);
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
	    signal(SIGTERM, getout);

	/* catch floating error */

	if (signal(SIGFPE, SIG_IGN) != SIG_IGN)
	    signal(SIGFPE, catch_fpe);

#ifdef IN_LINE
	if ((inlfp = fopen(inlfn =tempnam( TMPDIR , "inl"),"w+")) == NULL)
	{
		cerror("can't open asm macro temp file");
		/*NOTREACHED*/
	}
	/* if user specifies a very long temporary directory via TMPDIR */
	if (strlen(inlfn) > INNMBUF_LTH)
		cerror("temporary directory name too long\n");
	inlfn = strcpy(innmbuf, inlfn);
#endif

	for (m = 1; m < argc && argv[m][0] == '-'; ++m)
	{
		switch( argv[m][1] )
		{
		case 'i':	/* input file name */
			if (!freopen(argv[m+1], "r", stdin))
			{
				fprintf(stderr, "Can't open %s\n", argv[m+1]);
				myexit(2);
			}
			n = 2;
			break;

		case 'o':	/* output file name */
			if (!freopen(argv[m+1], "w", stdout))
			{
				fprintf(stderr, "Can't open %s\n", argv[m+1]);
				myexit(2);
			}
			n = 2;
			break;

		case 'f':	/* .c source file name */
			for (gotnm = findnm = argv[m+1]; *findnm; ++findnm)
				if (*findnm == '/')
					gotnm = findnm + 1;
			n = 2;
			break;

		case 'd':	/* prevent symbolic debug output */
			if (argv[m][2] == 'l' || argv[m][3] == 'l')
				dlflag = 1;
			if (argv[m][2] == 's' || argv[m][3] == 's')
				dsflag = 1;
			n = 1;
			break;

		case '1':	/* debug flags to front */
			argv[m][1] = 'X';
			continue;

		case '2':	/* debug flags to back */
			argv[m]++;
			argv[m][0] = '-';
			continue;

		case 'V':	/* print version info */
#ifdef REGSET
			fputs("WE 32106 rcc compiler: C-FP+ Issue 0.1\n",
#else
                        fputs("WE32000 qcc compiler: SVR3.0\n",
#endif
			    stderr);
			n = 1;
			break;

		case 'p':
			++proflag;
			n = 1;
			break;

		case 'F':
			++singflag;
			n = 1;
			break;

#ifdef IMPREGAL
		case 'K':
			if (argv[m+1][0] == 's' && argv[m+1][1] == 'z')
				/* weight optimizer info for size optims. */
				++sizeopt;
			n = 2;
			break;
#endif

		default:
			continue;

		}

		argc -= n;
		for( j = m; j < argc; ++j )
			argv[j] = argv[j+n];
		m--;
	}

	if (gotnm != NULL)
	{
		strcpy(ftitle, "\"");
		strcat(ftitle, gotnm);
		strcat(ftitle, "\"");
	}

#ifdef	M32B
	wloop_level = LL_BOT;       /* place "while" test at loop end */
	floop_level = LL_BOT;       /* place "for" test at loop end (only) */
#endif

	n = mainp1(argc, argv);
#ifdef IN_LINE
	fclose(inlfp);
#endif
	fclose(tmpfp);
	if (!(tmpfp = fopen(tmpfn, "r")))
		cerror("string file disappeared???");
	while (m = fread(buf, 1, BUFSIZ, tmpfp))
		fwrite(buf, 1, m, stdout);
	myexit(n);
}

beg_file()
{
	/* called as the very first thing by the parser to do machine
	 * dependent stuff
	 */
	register char *p, *s;
	char *tempnam();

			/* note: double quotes already in ftitle... */
	p = ftitle + strlen( ftitle ) - 2;
	s = p - 14;	/* max name length */
	while ( p > s && *p != '"' && *p != '/' )
		--p;
	printf( "\t.file\t\"%.15s\n", p + 1 );
        /* COMPVERS supplied by makefile, usually */
        printf( "\t.version\t\"%s\"\n", COMPVERS );
	if(!(tmpfp= fopen(tmpfn = tempnam( TMPDIR , "pcc2S"), "w")))
		cerror("can't create string file\n");
}

locctr(l)		/* output the location counter */
{
	static int lastloc = PROG, lastpd = PROG;
	int retval = lastloc;		/* value to return at end */

	curloc = l;
	switch (l)
	{
	case PROG:
		if (lastloc == PROG)
			break;
		if (lastpd == DATA)
			puts("	.text");
		lastpd = PROG;
		outfile = stdout;
		break;

	case DATA:
	case ADATA:
		if (lastloc == DATA || lastloc == ADATA)
			break;
		if (lastpd == PROG)
			puts("	.data");
		lastpd = DATA;
		outfile = stdout;
		break;

	case STRNG:
	case ISTRNG:
		/* output string initializers to a temporary file for now
		 * don't update lastloc
		 */
		
		outfile = tmpfp;
		break;

	default:
		cerror( "illegal location counter" );
	}

	lastloc = l;
	return( retval );		/* declare previous loc. ctr. */
}

NODE *
clocal(p)			/* manipulate the parse tree for local xforms */
register NODE *p;
{
	register NODE *l, *r;

#if defined(M32B) && defined(IMPREGAL)
	if ((p->in.op == VAUTO || p->in.op == VPARAM) && p->in.pad[0] == '@')
	{
		raname(p);
		p->in.pad[0] = '~';	/*has been raname()'d*/
	}
#endif
	if (!asgbinop(p->in.op) && p->in.op != ASSIGN)
		return (p);
	r = p->in.right;
	if (optype(r->in.op) == LTYPE)
		return (p);
	l = r->in.left;
	if (r->in.op == QUEST ||
		(r->in.op == CONV && l->in.op == QUEST) ||
		(r->in.op == CONV && l->in.op == CONV &&
		l->in.left->in.op == QUEST))
				/* distribute assigns over colons */
	{
		register NODE *pwork;
		register TWORD tp;
		NODE *pcpy = treecpy(p), *pnew;
#ifndef NODBG
		extern int xdebug, eprint();

		if (xdebug)
		{
			puts("Entering [op]=?: distribution");
			eprint(p);
		}
#endif
		tp = p->in.type;		/* save type of assign op */
		pnew = pcpy->in.right;
		while (pnew->in.op != QUEST)
			pnew = pnew->in.left;
		/*
		* pnew is top of new tree
		*/
		if ((pwork = p)->in.right->in.op == QUEST)
		{
			tfree(pwork->in.right);
			pwork->in.right = pnew->in.right->in.left;
			pnew->in.right->in.left = pwork;
			/* at this point, 1/2 distributed. Tree looks like:
			*		ASSIGN|ASGOP
			*	LVAL			QUEST
			*		EXPR1		COLON
			*			ASSIGN|ASGOP	EXPR3
			*		LVAL		EXPR2
			* pnew "holds" new tree from QUEST node
			*/
		}
		else
		{
			NODE *pholdtop = pwork;

			pwork = pwork->in.right;
			while (pwork->in.left->in.op != QUEST)
				pwork = pwork->in.left;
			tfree(pwork->in.left);
			pwork->in.left = pnew->in.right->in.left;
			pnew->in.right->in.left = pholdtop;
			/* at this point, 1/2 distributed. Tree looks like:
			*		ASSIGN|ASGOP
			*	LVAL			ANY # OF CONVs
			*			QUEST
			*		EXPR1		COLON
			*			ASSIGN|ASGOP	EXPR3
			*		LVAL		ANY # OF CONVs
			*			EXPR2
			* pnew "holds" new tree from QUEST node
			*/
		}
		if ((pwork = pcpy)->in.right->in.op == QUEST)
		{
			pwork->in.right = pnew->in.right->in.right;
			pnew->in.right->in.right = pwork;
			/*
			* done with the easy case
			*/
		}
		else
		{
			NODE *pholdtop = pwork;

			pwork = pwork->in.right;
			while (pwork->in.left->in.op != QUEST)
				pwork = pwork->in.left;
			pwork->in.left = pnew->in.right->in.right;
			pnew->in.right->in.right = pholdtop;
			/*
			* done with the CONVs case
			*/
		}
		p = pnew;
		p->in.type = tp;	/* type of Qnode is type of assignment */
#ifndef NODBG
		if (xdebug)
		{
			puts("Leaving [op]=?: distribution");
			eprint(p);
		}
#endif
	}
	return(p);
}


cisreg(t)			/* can auto of type t go in reg */
TWORD t;
{
#ifdef REGSET
	RST avail = RS_NONE;		/* available registers */
	RST curbit;			/* bit for current register */
	int curreg;			/* current register number */
#else
        int nregs;
#endif

	/*
	* Only allow register basic types or pointers.
	* Someday, maybe allow register small-struct
	*/
	switch (t)
	{

#ifdef REGSET
        default:
            if (! ISPTR(t)) break;      /* can't deal with others */
#endif
	case CHAR:	case SHORT:	case INT:	case LONG:
	case UCHAR:	case USHORT:	case UNSIGNED:	case ULONG:
#ifdef REGSET
		avail = ~regvar & REGREGS;
		break;
	case FLOAT:	case DOUBLE:
		avail = ~regvar & MAUREGS;
		break;
	}
	if (avail == 0)			/* none available */
	    return( 0 );

	/* find first available register */
	for (curreg = TOTREGS-1, curbit = RS_BIT(curreg); (avail & curbit) == 0;) {
	    --curreg;
	    curbit >>= 1;
	}
	
	nextrvar = curreg;		/* pass register number out */
	regvar |= curbit;		/* remember bits in use now */
	maxregused |= curbit;		/* remember what's been used */
	return( 1 );			/* success */
#else
        case FLOAT:     case DOUBLE:
                break;
        default:
                if (ISPTR(t))
                        break;
                return (0);
        }
        nregs = (t == DOUBLE ? 2 : 1);
        if (regvar + nregs <= TOTREGS-NRGS)     /* check for too many */
        {
                regvar += nregs;        /* number now in use */
                nextrvar = TOTREGS - regvar;
                if (nextrvar < minrvar)
                        minrvar = nextrvar;
                if( nregs == 2 ) radbl();   /* signal optimizer double reg used */
                return (1);
        }
        else
                return (0);
#endif
}

/* simple initialization code for integer types >= SZINT.  On the 3B2,3B5,3B20,
** this can only be an INT.  Do the obvious thing.
** We're handed an INIT node.
*/

sincode(p, sz)
NODE * p;
int sz;
{
    if (sz != SZINT || p->in.op != INIT || (p=p->in.left)->in.op != ICON)
        cerror("sincode:  got funny node\n");

    p2tree( p );                        /* perform magic to convert to pass2 tree */
    printf("	.word	");
    acon(p);                            /* output constant appropriately */
    putchar('\n');
    inoff += sz;
    return;
}



incode( p, sz )
register NODE *p;
{
	/* generate initialization code for assigning a constant c
	 * 	to a field of width sz
	 * we assume that the proper alignment has been obtained and sz < SZINT
	 * inoff is updated to have the proper final value
	 */
	if (inoff % SZINT == 0 )
		printf("	.word	%d:%d", sz, p->tn.lval);
	else
		printf(",%d:%d", sz, p->tn.lval);
	inoff += sz;
	if (inoff % SZINT == 0)
		putchar('\n');
}

vfdzero(n)		/* define n bits of zeros in a vfd */
int n;
{
	if( n <= 0 )
		return;
	if (inoff % SZINT == 0 )
		printf("	.word	%d:0", n);
	else
		printf(",%d:0", n);
	inoff += n;
	if (inoff % SZINT == 0)
		putchar('\n');
}

fincode(d, sz)		/* floating initialization */
FP_DOUBLE d;
int sz;
{
#if defined(u3b) || defined(u3b15) || defined(u3b5) || defined(u3b2) || defined(uts)
	union { FP_DOUBLE d; FP_FLOAT f; int i[2]; } cheat;

	if (sz == SZDOUBLE)
	{
		cheat.d = d;
		printf("	.word	0x%x,0x%x\n", cheat.i[0], cheat.i[1]);
	}
	else
	{
		cheat.f = FP_DTOF(d);
		printf("	.word	0x%x\n", cheat.i[0]);
	}
#else
        printf("\t.%s\t%.15e\n", sz == SZDOUBLE ? "double" : "float", d);
#endif
	inoff += sz;
}


char *
exname(p)			/* a name using external naming conventions */
char *p;
{
    return( p );
}


commdec(id)			/* generate a .comm from stab index id */
int id;
{
	register struct symtab *psym;
	OFFSZ n;

	psym = &stab[id];
	n = tsize(psym->stype, psym->dimoff, psym->sizoff) / SZCHAR;
# ifndef NOSYMB
	if ( psym->slevel == 0)
		    prdef(psym, dsflag);	/* output debug info */
# endif
	switch( psym->sclass ) {
	case STATIC:
		printf("	.bss	%s,%ld,%ld\n", exname(psym->sname),
					n, nalign(psym->stype));
		break;
	case EXTERN:
		printf("	.comm	%s,%ld\n", exname(psym->sname), n);
		break;
	case EXTDEF:
		werror("redeclaration of %s", exname(psym->sname));
		break;
	default:
		cerror("Illegal class in commdec");
	}
	return;
}


static
nalign(t)		/* figure alignment for type t */
TWORD t;
{
	int aln;

	if (ISPTR(t))
		aln = ALPOINT;
	else switch (BTYPE(t))
	{
	case CHAR:
	case UCHAR:
		aln = ALCHAR;
		break;
	case SHORT:
	case USHORT:
		aln = ALSHORT;
		break;
	case INT:
	case UNSIGNED:
	case ENUMTY:
		aln = ALINT;
		break;
	case LONG:
	case ULONG:
		aln = ALLONG;
		break;
	case FLOAT:
		aln = ALFLOAT;
		break;
	case DOUBLE:
		aln = ALDOUBLE;
		break;
	case STRTY:
	case UNIONTY:
		aln = ALSTRUCT;
		break;
	default:
		cerror("Confused type in nalign");
		/*NOTREACHED*/
	}
	return (aln / SZCHAR);
}


branch(n)			/* branch to label n or return */
int n;
{
	if (!reached)			/* return <expr>; } comes here 2x */
		return;
	else
		jmplab(n);
}


defalign(n)			/* align to multiple of n */
int n;
{
	if ((n /= SZCHAR) > 1)
		printf("	.align	%d\n", n);
}

jmplab(n)			/* produce jump to label n */
int n;
{
    printf("	jmp	.L%d\n", n);
}


deflab(n)			/* label n */
int n;
{
	fprintf(outfile, ".L%d:\n", n);
}

#ifdef REGSET
static int mauregs;		/* number of MAU register variables */
static int maxusrsize = 0;	/* stack size needed for MAU regs */
#endif

efcode()			/* wind up a function */
{
	extern int maxboff, maxtemp, maxarg;
	extern int strftn;	/* non-zero if function is structure function,
				** contains label number of local static value
				*/
	register int t,n;	/* number of registers used to hold return value */
        TWORD indtype();
#ifdef REGSET
	int i;			/* loop index */
	int regregs;		/* number of regular register variables */
	RST tempreg;		/* temporary for register bits */
#endif

	/* if there's a value returned
	/* figure out how many registers the return value will take 
	*/
	if( retstat & RETVAL )
	{
		switch( indtype( DECREF( stab[curftn].stype ) )) {
		case VOID:	n = 0; break;
		case DOUBLE:	n = 2; break;
		default:	n = 1; break;
		}
	}
	else n=0;		/* no value returned */
#ifdef REGSET
	/* figure out number of register variables used */
	regregs = 0;
	for (tempreg = maxregused; tempreg != 0; ) {
	    RST regbit = RS_CHOOSE(tempreg);	/* examine next register bit */

	    if (regbit & REGREGS)		/* regular register? */
		++regregs;			/* yes */
	    tempreg -= regbit;			/* peel off a bit */
	}
#endif
	deflab(retlab);


	efdebug( dlflag, dsflag );	/* end of function debug info */

	if (strftn)			/* return addr in 1st auto */
		printf("	movw	0(%%fp),%%r0\n", ftnno);


	printf("\t.set\t.F%d,%d\n\t.set\t.R%d,%d\n\tret\t&.R%d#%d\n", ftnno,
#ifdef REGSET
		(maxtemp + maxboff + maxusrsize) / SZCHAR, ftnno, regregs, ftnno, n);

	maxregused = RS_NONE;		/* reset bitmask */
	maxusrsize = 0;			/* reset stack size needed */
#else
                (maxtemp + maxboff) / SZCHAR, ftnno, TOTREGS - minrvar, ftnno, n);
        minrvar = TOTREGS;
#endif
}


bfcode(a, n)			/* begin function code. a is array of n stab */
int a[], n;			/* indices corresponding to incoming args */
{
	int temp;

	printf("	save	&.R%d\n", ftnno);
	if (proflag)
	{
		puts("	.data");
		temp = getlab();
		puts("	.align	4");
		deflab(temp);
		puts("	.word	0\n	.text");
		printf("	movw	&.L%d,%%r0\n",temp);
		puts("	jsb	_mcount");
	}
	printf("	addw2	&.F%d,%%sp\n", ftnno);
	retlab = getlab();
	bfdebug( a, n, dsflag );	/* do debug info at fcn begin */
}

#ifdef REGSET
savmau()		/* save active MAU user register variables */
{
	RST tempreg;	/* temporary for register bits */

	mauregs = 0;
	for ( tempreg = regvar; tempreg != 0; ) {
		RST regbit = RS_CHOOSE(tempreg);

		if ( regbit & MAUREGS)
			++mauregs;
		tempreg -= regbit;
	}
	switch(mauregs)
	{
	case 2:
		printf("	mmovdd	%%f1,.F%d-%d(%%fp)\n",
			ftnno, 2*SZDOUBLE/SZCHAR );
		maxusrsize += SZDOUBLE;
	case 1:
		printf("	mmovdd 	%%f2,.F%d-%d(%%fp)\n",
			ftnno, SZDOUBLE/SZCHAR );
		maxusrsize += SZDOUBLE;
	}
}


resmau()		/* restore MAU user register variables */
{
	switch( mauregs )
	{
	case 2:
		printf("	mmovdd	.F%d-%d(%%fp),%%f1\n"
			,ftnno, 2*SZDOUBLE/SZCHAR );
	case 1:
		printf("	mmovdd	.F%d-%d(%%fp),%%f2\n"
			,ftnno, SZDOUBLE/SZCHAR );
	}
}
#endif

defnam(psym)			/* current location is name from psym */
register struct symtab *psym;
{
	/* define the current location as the name psym->sname */

	if (psym->sclass == EXTDEF)
		printf("	.globl	%s\n", exname(psym->sname));
	printf("%s:\n", exname(psym->sname));
}

/* produce debug information for external definitions */

defnamdbg(psym)
struct symtab *psym;
{
    prextdef( psym, dsflag );		/* call common routine, pass flag */
    return;
}


bycode(ch, loc)			/* byte ch into string location loc */
int ch, loc;
{
	if (ch < 0)		/* eos */
	{
		if (loc)
			putc('\n', outfile);
	}
	else
	{
		if ((loc % 10) == 0)
			fputs("\n	.byte	", outfile);
		else
			putc(',', outfile);
		fprintf(outfile, "%d", ch);
	}
}

zecode(n)			/* n words of 0 */
register int n;
{
	if (n <= 0)		/* this is possible, folks */
		return;
	printf("	.zero	%d\n", (SZINT / SZCHAR) * n);
	inoff += n * SZINT;
}

struct sw heapsw[SWITSZ];	/* heap for switches */

genswitch(p, n)
register struct sw *p;
int n;
{
	/* p points to an array of structures, each consisting	*/
	/* of a constant value and a label. 			*/
	/* The first is >=0 if there is a default label; its	*/
	/* value is the label number. The entries p[1] to p[n]	*/
	/* are the nontrivial cases				*/

	register int i;
	register CONSZ j;
	unsigned long range;		/* room for MAXINT - (-MAXINT) */
	register int dlab, swlab;

	range = p[n].sval-p[1].sval;
	if (range > 0 && range <= (3 * n) && n >= 4)
	{				/* implement a direct switch */
		dlab = (p->slab >= 0) ? p->slab : getlab();
		if (p[1].sval)
#if defined(M32B) && defined(IMPSWREG)
			{
			printf("\tsubw3\t&%ld,%s,%%r0\n\tjneg\t.L%d\n",
				p[1].sval, rnames[swregno] , dlab);
			swregno = 0;
			}
#else
			printf("\tsubw2\t&%ld,%%r0\n\tjneg\t.L%d\n",
				p[1].sval, dlab);
#endif
		swlab = getlab();
#if defined(M32B) && defined(IMPSWREG)
		printf("\tcmpw\t%s,&%d\n\tjg\t.L%d\n\tALSW3\t&2,%s,%%r0\n"
			, rnames[swregno], range, dlab, rnames[swregno]);
		printf("\tcmpw\t%s,&0\n\tjneg\t.L%d\n",
			rnames[swregno], dlab);
#else
		printf("\tcmpw\t%%r0,&%d\n\tjg\t.L%d\n\tALSW3\t&2,%%r0,%%r0\n",
			range, dlab);
		printf("\tcmpw\t%%r0,&0 \n\tjneg\t.L%d\n", dlab);
#endif
		printf("\tjmp\t*.L%d(%%r0)\n", swlab);
		locctr(ADATA);			/* output table */
		defalign(ALPOINT);
		puts("#SWBEG");
		deflab(swlab);
		for (i = 1, j = p[1].sval; i <= n; ++j)
			printf("	.word	.L%d\n",
			    (j == p[i].sval) ? p[i++].slab : dlab );
		puts("#SWEND");
		locctr(PROG);
		if (p->slab < 0)
			deflab(dlab);
	}
	else if ( n > 8 )
	{
		heapsw[0].slab = dlab = p->slab >= 0 ? p->slab : getlab();
		makeheap( p, n, 1 );	/* build heap */
		walkheap( 1, n );	/* produce code */
		if( p->slab >= 0 )
			branch( dlab );
		else
			deflab( dlab );
	}
	else					/* simple switch code */
	{
		for (i = 1; i <= n; ++i)
#if defined(M32B) && defined(IMPSWREG)
			printf("\tcmpw\t&%ld,%s\n\tje\t.L%d\n",
				p[i].sval, rnames[swregno], p[i].slab);
#else
			printf("\tcmpw\t&%ld,%%r0\n\tje\t.L%d\n",
				p[i].sval, p[i].slab);
#endif
		if (p->slab >= 0)
			branch(p->slab);
	}
}

makeheap( p, m, n )
	register struct sw *p;
{
	register int q;

	if( n >= SWITSZ )
		cerror( "heap switch size exceeded" );
	q = select( m );
	heapsw[n] = p[q];
	if( q > 1 )
		makeheap( p, q-1, 2*n );
	if( q < m )
		makeheap( p+q, m-q, 2*n+1 );
}

select( m )
{
	register int l, i, k;

	for( i=1; ; i*=2 )
		if( (i-1) > m ) break;
	l = ((k = i/2 - 1) + 1)/2;
	return( l + (m-k < l ? m-k : l) );
}

walkheap( start, limit )
{
	int label;

	if( start > limit )
		return;
#if defined(M32B) && defined(IMPSWREG)
	fprintf( outfile, "\tcmpw\t%s,&%ld\n\tje\t.L%d\n",
		rnames[swregno], heapsw[start].sval, heapsw[start].slab );
#else
	fprintf( outfile, "\tcmpw\t%%r0,&%ld\n\tje\t.L%d\n",
		heapsw[start].sval, heapsw[start].slab );
#endif
	if( (2*start) > limit )
	{
		fprintf( outfile, "	jmp	.L%d\n", heapsw[0].slab );
		return;
	}
	if( (2*start+1) <= limit )
	{
		label = getlab();
		fprintf( outfile, "	jg	.L%d\n", label );
	}
	else
		fprintf( outfile, "	jg	.L%d\n", heapsw[0].slab );
	walkheap( 2*start, limit );
	if( (2*start+1) <= limit )
	{
		fprintf( outfile, ".L%d:\n", label );
		walkheap( 2*start+1, limit );
	}
}

#ifdef M32B
NODE *
setswreg( p )
	NODE *p;
{
	NODE *q;

	swregno = 0;
#ifdef IMPSWREG
	if (p->in.op == ASSIGN && p->in.left->in.op == SNODE)
	{
		q = p->in.right;
		if( q->in.op == REG )
		{
			swregno = q->tn.rval;
			p->in.op = FREE;
			p->in.left->in.op = FREE;
			p = q;
		}
	}
#endif
	return( p );
}

NODE *
myand(p)
	NODE *p;
{
#ifdef IMPREGAL
	raua(p);
#endif
	return(p);
}
#endif



/* interface routines to debug output */

fixdef(p)
struct symtab * p;
{
    fix1def(p,dsflag);			/* pass along flag */
    return;
}

strend(dimst)
int dimst;
{
    str1end( dimst, dsflag );		/* pass along flag */
    return;
}

sdbline()
{
    sdbln1( dlflag );			/* pass along flag */
    return;
}

aoend()
{
    aoend1( dsflag );			/* pass along flag */
    return;
}
