/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcc2:common/manifest.h	10.14"

/*To allow multiple includes:*/
#ifndef MANIFEST_H
#define	MANIFEST_H

#ifndef EOF
# include <stdio.h>
#endif

#ifndef PUTCHAR
#define PUTCHAR(c) ((void)(putc((c),outfile)))
#endif
#ifndef	PUTS
#define	PUTS(s) ((void) fputs((s), outfile))
#endif

	/* macro definitions for common cases of type collapsing */
# ifdef NOSHORT
# define SZSHORT SZINT
# define ALSHORT ALINT
# endif

# ifdef NOLONG
# define SZLONG SZINT
# define ALLONG ALINT
# endif

# ifdef NOFLOAT
# define SZFLOAT SZLONG
# define SZDOUBLE SZLONG
# define ALFLOAT ALLONG
# define ALDOUBLE ALLONG
# endif

# ifdef ONEFLOAT
# define SZFLOAT SZDOUBLE
# define ALFLOAT ALDOUBLE
# endif

/* define default assembly language comment starter */

# ifndef COMMENTSTR
# define COMMENTSTR	"#"
#endif

/*	manifest constant file for the lex/yacc interface */

# define ERROR 1
# define NAME 2
# define STRING 3
# define ICON 4
# define FCON 5
# define PLUS 6
# define MINUS 8
# define MUL 11
# define STAR (UNARY MUL)
# define AND 14
# define OR 17
# define ER 19
# define QUEST 21
# define COLON 22
# define ANDAND 23
# define OROR 24

/*	special interfaces for yacc alone */
/*	These serve as abbreviations of 2 or more ops:
	ASOP	=, = ops
	RELOP	LE,LT,GE,GT
	EQUOP	EQ,NE
	DIVOP	DIV,MOD
	SHIFTOP	LS,RS
	ICOP	INCR,DECR
	UNOP	NOT,COMPL
	STROP	DOT,STREF

	*/
# define ASOP 25
# define RELOP 26
# define EQUOP 27
# define DIVOP 28
# define SHIFTOP 29
# define INCOP 30
# define UNOP 31
# define STROP 32

/*	reserved words, etc */
# define TYPE 33
# define CLASS 34
# define STRUCT 35
# define RETURN 36
# define GOTO 37
# define IF 38
# define ELSE 39
# define SWITCH 40
# define BREAK 41
# define CONTINUE 42
# define WHILE 43
# define DO 44
# define FOR 45
# define DEFAULT 46
# define CASE 47
# define SIZEOF 48
# define ENUM 49

/*	little symbols, etc. */
/*	namely,

	LP	(
	RP	)

	LC	{
	RC	}

	LB	[
	RB	]

	CM	,
	SM	;

	*/

# define LP 50
# define RP 51
# define LC 52
# define RC 53
# define LB 54
# define RB 55
# define CM 56
# define SM 57
# define ASSIGN 58
	/* ASM returned only by yylex, and totally eaten by yyparse */
# define ASM 59

/*	END OF YACC */

/*	left over tree building operators */
# define COMOP 59
# define DIV 60
# define MOD 62
# define LS 64
# define RS 66
# define DOT 68
# define STREF 69
# define CALL 70
# define FORTCALL 73
# define NOT 76
# define COMPL 77
# define INCR 78
# define DECR 79
# define EQ 80
# define NE 81
# define LE 82
# define LT 83
# define GE 84
# define GT 85
# define ULE 86
# define ULT 87
# define UGE 88
# define UGT 89
# define SETBIT 90
# define TESTBIT 91
# define RESETBIT 92
# define ARS 93
# define REG 94
# define TEMP 95
# define CCODES 96
# define FREE 97
# define STASG 98
# define STARG 99
# define STCALL 100

/*	some conversion operators */
# define FLD 103
# define CONV 104
# define PMUL 105
# define PDIV 106

/*	special node operators, used for special contexts */
/* # define FORCE 107 */
# define GENLAB 108
# define CBRANCH 109
# define GENBR 110
# define CMP 111
# define CMPE 112	/* if IEEE standard, used for exception raising fp cmps */
# define GENUBR 113
# define INIT 114
# define CAST 115
# define FUNARG 116
# define VAUTO 117
# define VPARAM 118
# define RNODE 119
# define SNODE 120
# define QNODE 121
/*	a whole bunch of ops, done with unary; I don't need to tackle prec */
# define UOP0  122
# define UOP1  123
# define UOP2  124
# define UOP3  125
# define UOP4  126
# define UOP5  127
# define UOP6  128
# define UOP7  129
# define UOP8  130
# define UOP9  131
#ifdef	IN_LINE
# define INCALL 132			/* beware of UNARY INCALL == 134! */
#endif
# define MANY  133
/*	UNARY INCALL 134 */

/* ops used for NAIL*/
#ifdef CG
# define SEMI	135
# define ENTRY	136
# define PROLOG	137
# define ENDF	138
# define LOCCTR	139
# define LXINFO	140
# define SWBEG	141
# define SWCASE	142
# define SWEND	143
# define ALLOC	144
# define DEFNAM	145
# define UNINIT	146
# define CURFRAME 147
# define FCHAIN	148
# define BMOVE	150
# define BMOVEO	151
# define EXSETV	152
# define EXGETV	153
# define EXCLEAR 154
# define EXRAISE 155
# define JUMP	156
# define SINIT	157
# define LET	158
# define CSE	159
# define CURCAP	160
# define ALIGN	162
# define NPRETURN 163
# define VLRETURN 164
# define FSELECT 165
# define FCONV	166
# define BCMP	167
# define EXTEST	168
# define COPYASM 169
# define NOP	170
# define VFLD	171
# define COPY	172
# define BEGF	173
# define CAPCALL 174
# define RSAVE	175
# define RREST	176
# define CAPRET 177
#endif	/* def CG */

	/* DSIZE is the size of the dope array:  highest OP # + 1 */
#ifdef	CG
#define DSIZE  CAPRET+1
#else
# define DSIZE MANY+1+1			/* room for UNARY INCALL */
#endif

/*	node types */
# define LTYPE 02
# define UTYPE 04
# define BITYPE 010


/*	type names, used in symbol table building */
# define TNULL 0
# define FARG 1
# define CHAR 2
# define SHORT 3
# define INT 4
# define LONG 5
# define FLOAT 6
# define DOUBLE 7
# define STRTY 8
# define UNIONTY 9
# define ENUMTY 10
# define MOETY 11
# define UCHAR 12
# define USHORT 13
# define UNSIGNED 14
# define ULONG 15
# define VOID 16
# define UNDEF 17

# define ASG 1+
# define UNARY 2+
# define NOASG (-1)+
# define NOUNARY (-2)+

/*	various flags */
# define NOLAB (-1)

/* type modifiers */

# define PTR  040
# define FTN  0100
# define ARY  0140

/* type packing constants */

# define MTMASK 03
# define BTMASK 037
# define BTSHIFT 5 
# define TSHIFT 2
# define TMASK (MTMASK<<BTSHIFT)
# define TMASK1 (MTMASK<<(BTSHIFT+TSHIFT))
# define TMASK2  (TMASK||MTMASK)

/*	macros	*/

# ifndef BITMASK
	/* beware 1's complement */
# define BITMASK(n) (((n)==SZLONG)?-1L:((1L<<(n))-1))
# endif
# define ONEBIT(n) (1L<<(n))
# define MODTYPE(x,y) x = (x&(~BTMASK))|y  /* set basic type of x to y */
# define BTYPE(x)  (x&BTMASK)   /* basic type of x */
# define ISUNSIGNED(x) ((x)<=ULONG&&(x)>=UCHAR)
# define UNSIGNABLE(x) ((x)<=LONG&&(x)>=CHAR)
# define ENUNSIGN(x) ((x)+(UNSIGNED-INT))
# define DEUNSIGN(x) ((x)+(INT-UNSIGNED))
# define ISPTR(x) ((x&TMASK)==PTR)
# define ISFTN(x)  ((x&TMASK)==FTN)  /* is x a function type */
# define ISARY(x)   ((x&TMASK)==ARY)   /* is x an array type */
# define INCREF(x) (((x&~BTMASK)<<TSHIFT)|PTR|(x&BTMASK))
# define DECREF(x) (((x>>TSHIFT)&~BTMASK)|(x&BTMASK))
# define SETOFF(x,y)   if( (x)%(y) != 0 ) x = ( ((x)/(y) + 1) * (y))
		/* advance x to a multiple of y */
# define NOFIT(x,y,z)   ( ((x)%(z) + (y)) > (z) )
	/* can y bits be added to x without overflowing z */
	/* pack and unpack field descriptors (size and offset) */
# define PKFIELD(s,o) (((o)<<6)|(s))
# define UPKFSZ(v)  ((v)&077)
# define UPKFOFF(v) ((v)>>6)

/* Definitions of hooks for floating-point support in
** cross environments.
*/

extern int errno;
#ifndef	FP_DOUBLE
#define	FP_DOUBLE	double	/* type containing doubles */
#endif
#ifndef	FP_FLOAT
#define	FP_FLOAT	float	/* type containing floats */
#endif
#ifndef	FP_DTOF
#define	FP_DTOF(d) ((float) d)	/* convert double to float */
#endif
#ifndef	FP_DTOFP		/* truncate double to float precision */
#define	FP_DTOFP(d) ((double) FP_DTOF(d))
#endif
#ifndef	FP_DTOL			/* convert double to long */
#define	FP_DTOL(d) ((long) (d))
#endif
#ifndef	FP_LTOD			/* convert long to double */
#define	FP_LTOD(l) ((double) (l))
#endif
#ifndef	FP_ULTOD		/* convert unsigned long to double */
#define	FP_ULTOD(ul) ((double) (ul))
#endif
#ifndef	FP_NEG			/* negate double */
#define	FP_NEG(d) (-(d))
#endif
#ifndef	FP_PLUS			/* add double */
#define	FP_PLUS(d1,d2) ((d1)+(d2))
#endif
#ifndef	FP_MINUS		/* subtract double */
#define	FP_MINUS(d1,d2) ((d1)-(d2))
#endif
#ifndef	FP_TIMES		/* multiply double */
#define	FP_TIMES(d1,d2) ((d1)*(d2))
#endif
#ifndef	FP_DIVIDE		/* divide double */
#define	FP_DIVIDE(d1,d2) ((d1)/(d2))
#endif
#ifndef	FP_ISZERO		/* is double value zero? */
#define	FP_ISZERO(d) (!(d))
#endif
#ifndef	FP_CMPD			/* compare two doubles */
#define	FP_CMPD(x,y) ((x)==(y)?0:((x)>(y)?1:-1))
#endif
#ifndef	FP_ATOF				/* convert string to FP_DOUBLE */
#ifndef	FLOATCVT			/* backward compatibility */
#define	FP_ATOF(s) atof(s)		/* use UNIX atof() */
			/*Note: for CG, we also need a C++ declaration
			  of atof(); */
#  ifdef c_plusplus
extern double atof(char *);
#  else
extern double atof();
#  endif
#else
#define	FP_ATOF(s) FLOATCVT(s)
#endif
#endif

/*	operator information */

# define TYFLG 016
# define ASGFLG 01
# define LOGFLG 020

# define SIMPFLG 040
# define COMMFLG 0100
# define DIVFLG 0200
# define FLOFLG 0400
# define LTYFLG 01000
# define CALLFLG 02000
# define MULFLG 04000
# define SHFFLG 010000
# define ASGOPFLG 020000

# define SPFLG 040000
# define STRFLG 0100000		/* for structure ops */

#ifdef	REGSET
#	define RS_BIT(n) (((RST) 1) << (n)) /* bit corresponding to reg n */
#	define RS_PAIR(n) ((RST)(n) << 1) /* reg n's pair companion */
/* Choose a register from a register set bit vector.  Used to pick a
** scratch register.  Always choose right-most bit.
*/
#	define RS_CHOOSE(vec) ((vec) & ~((vec)-1))
/* Choose left-most bit.  This one's a function. */
extern RST RS_RCHOOSE();

#	define RS_NONE	((RST) 0)	/* no register bits */
/* these definitions are slight perversions of the use of RS_BIT */
#	define RS_NRGS	(RS_BIT(NRGS)-1) /* bits for all scratch registers */
#	define RS_TOT	(RS_BIT(TOTREGS)-1) /* bits for all registers */
#endif

#define optype(o) (dope[o]&TYFLG)
#define asgop(o) (dope[o]&ASGFLG)
#define asgbinop(o) (dope[o]&ASGOPFLG)
#define logop(o) (dope[o]&LOGFLG)
#define callop(o) (dope[o]&CALLFLG)
#define structop(o) (dope[o]&STRFLG)

#ifdef	CG
/*	for CG: two basic exceptions*/
#define NUMERIC 1
#define STACKOV 2
/* 	numeric exception types*/
#define EXLSHIFT	01	/*dropped bit on ls*/
#define EXCAST		02	/*dropped bit on convert*/
#define EXINT		04	/*integer overflow*/
#define EXFLOAT		010	/*floating overflow*/
#define EXDBY0		020	/*div by zero*/
#endif

/*	table sizes	*/

/* The following undef-initions will help flag places where
** older machine-dependent code depends on fixed size tables.
*/
#undef BCSZ
#undef MAXNEST
#undef SYMTSZ
#undef DIMTABSZ
#undef PARAMSZ
#undef ARGSZ
#undef TREESZ
/* keep this one for non-users of MAKEHEAP */
# ifndef SWITSZ
# define SWITSZ 250 /* size of switch table */
# endif
# ifndef YYMAXDEPTH
# define YYMAXDEPTH 300 /* yacc parse stack */
# endif

/* The following define initial sizes for dynamic tables. */

#ifndef INI_NINS		/* inst[]:  table of generated instructions */
#define INI_NINS 50
#endif

#ifndef	INI_SWITSZ		/* swtab[]:  switch case table */
#define	INI_SWITSZ 100
#endif

#ifndef	INI_HSWITSZ		/* heapsw[]:  heap-sorted switch table */
#define	INI_HSWITSZ 25
#endif

#ifndef INI_BBFSZ		/* bb_flags[]:  .bb debug flag table */
#define INI_BBFSZ 10
#endif

#ifndef INI_FAKENM		/* mystrtab[]:  unnamed struct fake names */
#define INI_FAKENM 15
#endif

#ifndef	INI_N_MAC_ARGS		/* macarg_tab[]:  enhanced asm arg. names */
#define INI_N_MAC_ARGS 5
#endif

#ifndef	INI_SZINLARGS		/* inlargs[]:  buffer for enhanced asm. formals */
#define	INI_SZINLARGS 50
#endif

#ifndef INI_ASMBUF		/* asmbuf[]:  buffer for asm() lines */
#define INI_ASMBUF 20
#endif

#ifndef	INI_SYMTSZ		/* stab[]:  symbol table */
#define	INI_SYMTSZ 500
#endif

#ifndef INI_RNGSZ		/*case_ranges[]: ranges for big cases (CG) */
#define INI_RNGSZ INI_SWITSZ
#endif

/* dimtab must be at least 16:  see mainp1() */
#if !defined(INI_DIMTABSZ) || INI_DIMTABSZ < 20
#undef INI_DIMTABSZ		/* dimtab[]:  dimension (and misc.) table */
#define INI_DIMTABSZ 400
#endif

#ifndef	INI_PARAMSZ		/* paramstk[]:  struct parameter stack */
#define INI_PARAMSZ 100
#endif

#ifndef	INI_BCSZ		/* asavbc[]:  block information */
#define INI_BCSZ 50
#endif

/* scopestack should be at least 3 (indices 0-2) to handle simple fct. */
#if !defined(INI_MAXNEST) || INI_MAXNEST < 3
#undef INI_MAXNEST		/* scopestack[]:  sym. tab. scope stack */
#define INI_MAXNEST 20
#endif

#ifndef INI_ARGSZ		/* argstk[], argsoff[], argty[]:  incoming
				** arg information
				*/
#define INI_ARGSZ 15
#endif

#ifndef INI_INSTK		/* instack[]:  initialization stack */
#define INI_INSTK 10
#endif

#ifndef	INI_TREESZ		/* number of nodes per cluster */
#define	INI_TREESZ 100
#endif

#ifndef	INI_MAXHASH		/* number of segmented scanner hash tables */
#define	INI_MAXHASH 20
#endif


#ifdef	CG
/*	turn on NONEST if can't nest calls or allocs */
#  if defined(NOFNEST) || defined(NOANEST)
#    define NONEST
#  endif
#endif	/* def CG */

	char		*hash();
	char		*savestr();
	char		*tstr();
	extern int	tstrused;
	extern char	*tstrbuf[];
	extern char	**curtstr;
#define freestr()	( curtstr = tstrbuf, tstrused = 0 )
	extern FILE *	outfile;

#	define NCHNAM 8  /* number of characters in a truncated name */

/*	common defined variables */

extern int nerrors;  /* number of errors seen so far */

typedef unsigned int TWORD;
typedef long CONSZ;  /* size in which constants are converted */

	/* default is byte addressing */
	/* BITOOR(x) converts bit width x into address offset */
# ifndef BITOOR
# define BITOOR(x) ((x)/SZCHAR)
# endif

/* Many architectures can't handle literal double/float constants.
** For those that do, define LITDCON to return 1 in situations where
** a literal is permitted.  p is the FCON node.  Otherwise return 0,
** and a named constant will be defined.
*/

#ifndef	LITDCON
#define	LITDCON(p)	(0)
#endif

# ifdef SDB
# define STABS
# endif

#if defined(TMPSRET) && !defined(AUXREG)
#define AUXREG (NRGS - 1)
#endif

# define NIL (NODE *)0

extern int dope[];  /* a vector containing operator information */
extern char *opst[];  /* a vector containing names for ops */

# define NCOSTS (NRGS+4)

typedef union ndu NODE;

/* This #define gives those items that must be in both first
** and second pass nodes.
*/

#define ND12 \
    int op;		/* opcode */		\
    TWORD type;		/* type encoding */	\
    NODE * left;	/* left operand */	\
    NODE * right;	/* right operand */	\
    CONSZ lval;		/* constant value */	\
    int rval;		/* usually stab entry */ \
    FP_DOUBLE dval	/* double constant */


/* Pass 1 node */
typedef struct {
    ND12;		/* the above, and ... */
    int flags;		/* special pass 1 flags */
    int csiz;		/* sizoff */
    int cdim;		/* dimoff */
} ND1;

/* Pass 2 node */
typedef struct {
    ND12;		/* the basics, plus ... */

    int goal;		/* code generation goal */
    int label;		/* branch target label # */
    int lop;		/* GENBR branch condition */
    char * name;	/* pointer to name string */
    int stsize;		/* structure size */
    short stalign;	/* structure alignment */
    short argsize;	/* size of CALL argument list */
#ifndef STINCC
    int cst[NCOSTS];	/* PCC2 costs */
#endif
#if defined(REGSET) || defined(CG)
    int scratch;	/* index to scratch register vectors */
#endif
#ifdef CG
    int strat;		/* code generation strategy */
    int id;		/* name of CSE destination */
#endif
} ND2;

union ndu {
    ND1 fn;	/* front end node */
    ND1 fpn;	/* floating point node */

    ND2 in;	/* interior (binary) node */
    ND2 tn;	/* terminal (leaf) node */
    ND2 bn;	/* branch node */
    ND2 stn;	/* structure node */
#ifdef CG
    ND2 csn;	/* CSE node */
#endif
};


typedef struct nndu {
    NODE node;
    int _node_no;
} nNODE;			/* numbered node */

#define node_no(p) /* NODE * p */ (((nNODE *) (p))->_node_no)


#ifdef	CG
/* For CG, file pointers for output and debugging*/
extern FILE *textfile;		/* user-requested output file.
				** Normally same as outfile.
				*/
extern FILE *debugfile; 	/*output file for debugging*/
extern char 	costing;	/*1 if just costing*/
extern int strftn, proflag;
extern int str_spot;		/*place to hold struct return*/
NODE	*firstl(), *lastl();	/*CG functions*/
#endif	/* def CG */

/* Location counters.  Formerly in mfile1.h, moved here for nail's
** benefit.
*/
# define PROG 0
# define ADATA 1
# define DATA 2
# define ISTRNG 3
# define STRNG 4
#ifdef	CG
# define CDATA 5	/* constants */
# define CURRENT 6	/* Return current locctr*/
# define UNK 7		/* Unknown (inital) state */
#endif

/* Define table descriptor structure for dynamically
** managed tables.
*/

struct td {
    int td_allo;		/* array elements allocated */
    int td_used;		/* array elements in use */
    int td_size;		/* sizeof(array element) */
    int td_flags;		/* flags word */
    char * td_start;		/* (really void *) pointer to array start */
    char * td_name;		/* table name for error */
#ifdef STATSOUT
    int td_max;			/* maximum value reached */
#endif
};

/* flags for td_flags */
#define TD_MALLOC 	1	/* array was malloc'ed */
#define TD_ZERO		2	/* zero expansion area */

int td_enlarge();		/* enlarges a table so described, returns
				** old size
				*/

#define TD_INIT(tab, allo, size, flags, start, name) \
struct td tab = { \
	allo,	/* allocation */ \
	0,	/* used always 0 */ \
	size,	/* entry size */ \
	flags,	/* flags */ \
	(char *)start,	/* pointer */ \
	name	/* table name */ \
}


#ifdef	IN_LINE
/* The table for inline formal names gets used by both Pass 1 and
** Pass 2 code.
*/
extern struct td td_inlargs;
#define	inlargs ((char *) td_inlargs.td_start)
#define	sz_inlargs (td_inlargs.td_used)
#define	SZINLARGS (td_inlargs.td_allo)
#endif

#endif	/* def MANIFEST_H:  from top of file */
