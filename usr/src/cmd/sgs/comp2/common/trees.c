/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcc2:common/trees.c	1.48"

# include "mfile1.h"

/*	some special actions, used in finding the type of nodes */
# define NCVT 01
# define PUN 02
# define TYPL 04
# define TYPR 010
# define TYMATCH 040
# define LVAL 0100
# define CVTO 0200
# define CVTL 0400
# define CVTR 01000
# define PTMATCH 02000
# define OTHER 04000
# define NCVTR 010000

/* node conventions:
**
** NAME:	rval>0 is stab index for external
** rval<0 is -inlabel number
** lval is offset in address units
** (NAME really means "STATIC VARIABLE")
** ICON:	lval has the value
** rval has the STAB index, or - label number,
** if a name whose address is in the constant
** rval = NONAME means no name
** VAUTO:	automatic name: lval has offset in address units
** VPARAM:	parameter: lval has offset in address units
** REG:	rval is reg. number
**
*/

extern int watchdog;			/* watchdog for too many nodes alloc. */
extern int maxarg;

/* Flag to allow single precision floating point arithmetic.
/* Default is double precision.  Single Precision turn on with -b option.
*/
int singflag = 0;	

TWORD
indtype( t )
register TWORD t;
{
	/* return the type of an intermediate expression of type t */
	switch( t )
	{

	case CHAR:
	case SHORT:
		return( INT );

	case UCHAR:
	case USHORT:
		return( UNSIGNED );

	case FLOAT:
	/* intermediate results promoted only if called for */
		if( !singflag ) return( DOUBLE );
		break;

	}
	return( t );
}

int bdebug = 0;
extern ddebug;

NODE *
buildtree( o, l, r )
register NODE *l, *r;
register o;
{
	register NODE *p, *q;
	register actions;
	register opty;
	register struct symtab *sp;
	register NODE *lr, *ll;
	int	i;

	watchdog = 0;			/* no table overrun when building tree */

# ifndef NODBG
	if( bdebug )
		printf( "buildtree( %s, %d, %d )\n",
				opst[o], node_no(l), node_no(r) );
# endif

	/* special case to recognize subscripting explicitly */

	if( o == LB ) return( buildtree( STAR, buildtree( PLUS, l, r ), NIL ) );
	opty = optype(o);

	/* check for constants */

	if( o == NOT && l->in.op == ICON && hflag )
	{
		werror( "constant argument to NOT" );
	}

	else if( o==UNARY MINUS && l->in.op==FCON )
	{
		errno = 0;
		l->fpn.dval = FP_NEG(l->fpn.dval);
		if (errno)
		    werror("float/double negation causes range error");
		return(l);
	}

	else if( o==QUEST && l->in.op==ICON && l->tn.rval==NONAME )
	{
		l->in.op = FREE;
		r->in.op = FREE;
		if( l->tn.lval )
		{
			tfree( r->in.right );
			return( r->in.left );
		}
		else
		{
			tfree( r->in.left );
			return( r->in.right );
		}
	}

	else if( (o==ANDAND || o==OROR) && (l->in.op==ICON||r->in.op==ICON) )
		goto ccwarn;

	else if( opty == BITYPE && l->in.op == ICON && r->in.op == ICON )
	{

		switch( o )
		{

		case ULT:
		case UGT:
		case ULE:
		case UGE:
		case LT:
		case GT:
		case LE:
		case GE:
		case EQ:
		case NE:
		case ANDAND:
		case OROR:
		case CBRANCH:

ccwarn:
			if( hflag ) werror( "constant in conditional context" );
		}
	}

	/* we make a real node, and look for shrinking later */

	p = block( o, l, r, INT, 0, INT );

	/* patch up enum casts here instead of mucking through cgram */
	if ( o==CAST && BTYPE( p->in.left->in.type) == ENUMTY ) 
		econvert(p->in.left);

	/* check for VOID type in unfortunate places */

	switch( o ) {
	case CM:			/* handled better by OTHER case */
	case COMOP:
	case QUEST:
	case COLON:
	case CAST:
	    break;			/* let parent handle these */

	default:
	    if (   (optype(o) != LTYPE && l->in.type == VOID)
		|| (optype(o) == BITYPE && r->in.type == VOID)
		)
		uerror("void type illegal in expression, op %s", opst[o]);
	}

	actions = opact(p);
# ifdef MYOPACT
	actions = MYOPACT(p,actions);
# endif

	if( actions&LVAL )
	{
		 /* check left descendent */
		if( notlval(p->in.left, 0) )	/* struct ref not allowed */
		{
		    /* produce a reasonable error message */
		    switch( p->in.op ) {
		    case CAST:
			uerror( "illegal cast" );
			break;
		    case INCR:
		    case DECR:
			uerror( "illegal lhs of ++ or --" );
			break;
		    default:
			uerror( "illegal lhs of assignment operator" );
		    }
		}
	}

	if( actions & NCVTR )
	{
		p->in.left = pconvert( p->in.left );
	}
	else if( !(actions & NCVT ) )
	{
		switch( opty )
		{

		case BITYPE:
			p->in.right = pconvert( p->in.right );
		case UTYPE:
			if( !(actions&LVAL) )
				p->in.left = pconvert( p->in.left );

		}
	}

	if( (actions&PUN) && (o!=CAST||cflag) )
	{
		chkpun(p);
	}

	if( actions & (TYPL|TYPR) )
	{

		q = (actions&TYPL) ? p->in.left : p->in.right;

		p->in.type = q->in.type;
		p->fn.cdim = q->fn.cdim;
		p->fn.csiz = q->fn.csiz;
	}

	if( actions & CVTL ) p = convert( p, CVTL );
	if( actions & CVTR ) p = convert( p, CVTR );

	if( actions & TYMATCH ) p = tymatch(p);
	if( actions & PTMATCH ) p = ptmatch(p);

	if( actions & SPFLG ) p = clocal(p);

	if( actions & OTHER )
	{
		l = p->in.left;
		r = p->in.right;

		switch(o)
		{

		case NAME:
			sp = &stab[idname];
			if( sp->stype == UNDEF )
			{
				uerror( "%s undefined", sp->sname );
				/* make p look reasonable */
				p->in.type = p->fn.cdim = p->fn.csiz = INT;
				p->tn.rval = idname;
				p->tn.lval = 0;
				defid( p, SNULL );
				break;
			}
			p->in.type = sp->stype;
			p->fn.cdim = sp->dimoff;
			p->fn.csiz = sp->sizoff;
			/* special case: MOETY is really an ICON... */
			if( p->in.type == MOETY )
			{
				p->tn.rval = NONAME;
				p->tn.lval = sp->offset;
				p->fn.cdim = 0;
				p->in.type = ENUMTY;
				p->in.op = ICON;
				/* change enum constant to an integer type */
				econvert(p);	
			}
			else
			{
				/* change enum variables to integer types */
				if( BTYPE(p->in.type) == ENUMTY ) econvert(p);

				switch( sp->sclass )
				{

				case AUTO:
					p->in.op = VAUTO;
					p->tn.rval = NONAME;
					p->tn.lval = BITOOR(sp->offset);
					break;

				case PARAM:
					p->in.op = VPARAM;
					p->tn.rval = NONAME;
					p->tn.lval = BITOOR(sp->offset);
					break;

				case REGISTER:
					p->in.op = REG;
					p->tn.lval = 0;
					p->tn.rval = sp->offset;
					break;

				case ULABEL:
				case LABEL:
				case STATIC:
					if( sp->slevel != 0 )
					{
						p->tn.lval = 0;
						p->tn.rval = -sp->offset;
						break;
					}
					/* FALLTHRU */

				default:
					p->tn.lval = 0;
					p->tn.rval = idname;
				}
			}
			break;

		case ICON:
			p->in.type = INT;
			p->fn.cdim = 0;
			p->fn.csiz = INT;
			break;

		case STRING:
			p->in.op = NAME;
			p->in.type = CHAR+ARY;
			p->tn.lval = 0;
			p->tn.rval = NOLAB;
			p->fn.cdim = curdim;
			p->fn.csiz = CHAR;
			break;

		case FCON:
			p->tn.lval = 0;
			p->tn.rval = 0;
			p->in.type = DOUBLE;
			p->fn.cdim = 0;
			p->fn.csiz = DOUBLE;
			break;

		case STREF:
			/* p->x turned into *(p+offset) */
			/* rhs must be a name; check correctness */

			i = r->tn.rval;
			if(i<0 || i>=SYMTSZ || ((sp= &stab[i])->sclass != MOS &&
			    sp->sclass != MOU && !(sp->sclass&FIELD)) )
			{
				uerror( "member of structure or union required" );
				break;
			}
			else
				/* if this name is non-unique, find right one */
				if( stab[i].sflags&SNONUNIQ && (l->in.type==PTR+STRTY ||
				    l->in.type == PTR+UNIONTY) &&
				    (l->fn.csiz+1) >= 0 )
			{
				/* nonunique name && structure defined */
				char * memnam, * tabnam;
				register j;
				int memi;
				j=dimtab[l->fn.csiz+1];
				for( ; (memi=dimtab[j]) >= 0; ++j )
				{
					tabnam = stab[memi].sname;
					memnam = stab[i].sname;
# ifndef NODBG
					if( ddebug>1 )
					{
						printf("member %s==%s?\n",
							memnam, tabnam);
					}
# endif
					if( stab[memi].sflags & SNONUNIQ )
					{
						if ( memnam != tabnam )
							continue;
						r->tn.rval = i = memi;
						break;
					}
					continue;
				}
				if( memi < 0 )
					uerror("illegal member use: %s",
					stab[i].sname);
			}
			else
			{
				register j;

				if( l->in.type != PTR+STRTY &&
				    l->in.type != PTR+UNIONTY )
				{
					if( stab[i].sflags & SNONUNIQ )
					{
						uerror(
						"nonunique name demands struct/union or struct/union pointer" );
					}
					else {
					    register int align =
						talign(BTYPE(l->in.type), l->fn.csiz);
					    /* must be aligned on ALSTRUCT multiple */
					    if ((align % ALSTRUCT) == 0)
						werror("struct/union or struct/union pointer required" );
					    else
						uerror("bad structure offset");
					}
				}
				else if( (j=l->fn.csiz+1)<0 )
					cerror( "undefined structure or union");
				else if( !chkstr( i, dimtab[j],
					DECREF(l->in.type) ) )
				{
					werror( "illegal member use: %s",
					stab[i].sname );
				}
			}
			p = stref( p );
			break;

		case STAR:
			if( l->in.op == UNARY AND )
			{
				p->in.op = l->in.op = FREE;
				p = l->in.left;
			}
			if( !ISPTR(l->in.type) ) uerror("illegal indirection");
			p->in.type = DECREF(l->in.type);
			p->fn.cdim = l->fn.cdim;
			p->fn.csiz = l->fn.csiz;
			break;

		case UNARY AND:
			switch( l->in.op )
			{

			case STAR:
				p->in.op = l->in.op = FREE;
				p = l->in.left;
			case NAME:
			case VAUTO:
			case VPARAM:
			case TEMP:
refinc:
				p->in.type = INCREF( l->in.type );
				p->fn.cdim = l->fn.cdim;
				p->fn.csiz = l->fn.csiz;
				break;

			case RNODE:
#ifdef TMPSRET
				/* don't bother, let optim clean up */
				if (simpstr(l->fn.cdim, l->fn.csiz) != STRTY)
					goto refinc;

				p->in.op = STAR;
				p->in.type = STRTY;
				p->fn.csiz = l->fn.csiz;
				p->fn.cdim = l->fn.cdim;
				l->in.op = FREE;
				idname = lookup(hash(".stfake"), 0);
				p->in.left = buildtree(NAME, NIL, NIL);
				p->in.left->fn.csiz = p->fn.csiz;
				p->in.left->fn.cdim = p->fn.cdim;
				break;
#else
				goto refinc;
#endif
			case COMOP:
				lr = l->in.right;
				/* worry about array reference on right of ,OP;
				** ARY int becomes PTR int
				*/
				if (ISARY( lr->in.type )) {
				    lr->in.type = DECREF( lr->in.type );
				}
				lr = buildtree( UNARY AND, lr, NIL );
				p->in.op = l->in.op = FREE;
				p = buildtree( COMOP, l->in.left, lr );
				break;

			case QUEST:
				lr = buildtree( UNARY AND,
				l->in.right->in.right, NIL );
				ll = buildtree( UNARY AND,
				l->in.right->in.left, NIL );
				p->in.op = l->in.op = l->in.right->in.op = FREE;
				p = buildtree( QUEST, l->in.left,
				buildtree( COLON, ll, lr ) );
				break;

			default:
				uerror( "unacceptable operand of &" );
				break;
			}
			break;

		case RETURN:
		case ASSIGN:
		case CAST:
			/* structure assignment */
			/* take the addresses of the two sides; then make an
			** operator using STASG and
			** the addresses of left and right
			*/

			{
				register TWORD t;
				register d, s, sz;

				if( l->fn.csiz != r->fn.csiz )
					uerror(
					"assignment of different structures" );

				r = buildtree( UNARY AND, r, NIL );
				l = buildtree( UNARY AND, l, NIL );
				t = r->in.type;
				d = r->fn.cdim;
				s = r->fn.csiz;
#ifdef ENDSTRUCT
				sz = tsize( STRTY, d, s );
				l = aadjust( l, sz );
				r = aadjust( r, sz );
#endif

				l = block( STASG, l, r, t, d, s );

				if( o == RETURN )
				{
					p->in.op = FREE;
					p = l;
					break;
				}

				p->in.op = STAR;
				p->in.left = clocal(l);
				p->in.right = NIL;
				break;
			}
		case COLON:
			/* structure or void colon */

			if( l->in.type == VOID)	/* both sides are void */
			    p->in.type = VOID;	/* force type of COLON */
			else if( l->fn.csiz != r->fn.csiz )
				uerror( "type clash in conditional" );
			break;

		case CALL:
		    {
			NODE * temp;
			i = 0;
			p->in.right = r = strargs( p->in.right, &i );
			/* check first argument for VOID-ness; others tested by , */
			temp = r;
			if (temp->in.op == CM)
			    temp = temp->in.left;
			if (temp->in.type == VOID || temp->in.type == UNDEF)
			    uerror("void type illegal as argument");
			if( i > maxarg ) maxarg = i;
		    }
		case UNARY CALL:
			if( !ISPTR(l->in.type)) uerror("illegal function");
			p->in.type = DECREF(l->in.type);
			if( !ISFTN(p->in.type)) uerror("illegal function");
			p->in.type = DECREF( p->in.type );
			p->fn.cdim = l->fn.cdim;
			p->fn.csiz = l->fn.csiz;
			if( l->in.op == UNARY AND &&
			    l->in.left->in.op == NAME &&
			    l->in.left->tn.rval >= 0 &&
			    l->in.left->tn.rval != NONAME &&
			    ((i=stab[l->in.left->tn.rval].sclass)==FORTRAN
			    || i == UFORTRAN ) )
			{
				p->in.op += (FORTCALL-CALL);
			}
#ifdef IN_LINE
			if( l->in.op == UNARY AND &&
			    l->in.left->in.op == NAME &&
			    l->in.left->tn.rval >= 0 &&
			    l->in.left->tn.rval != NONAME &&
			    stab[l->in.left->tn.rval].sclass == INLINE )
			{
				p->in.op += (INCALL-CALL);
			}
#endif
			if( p->in.type == STRTY || p->in.type == UNIONTY )
			{
				/* function returning structure */
				/*  function really returns ptr with * */

# ifdef ARGSRET
				/* set aside argument space for return val */
				i = tsize( p->in.type, p->fn.cdim, p->fn.csiz );
				/* alignment??? */
				if( i > maxarg ) maxarg = i;
# endif
				p->in.op += STCALL-CALL;
				p->in.type = INCREF( p->in.type );
				p = buildtree( STAR, p, NIL );
				break;
			}

			/* fix up type of return value from call */
			/* float functions always return double, even if */
			/* single precision option set */
			{
				register TWORD t;
				t = indtype( p->tn.type );
				if ( singflag && t == FLOAT ) t = DOUBLE;
				if( t != p->tn.type )
				{
					p->fn.csiz = p->tn.type = t;
				}
			}
# ifdef ARGALLRET
			/* set aside argument space for return val */
			i = tsize( p->in.type, p->fn.cdim, p->fn.csiz );
			/* alignment??? */
			if( i > maxarg ) maxarg = i;
# endif
			break;

		case ANDAND :
		case OROR :
			/* change array or function operand to pointer constant */
			if ( ISARY( r->in.type) )
			{
				r->in.type = DECREF( r->in.type );
				++r->fn.cdim;
				p->in.right =  buildtree( UNARY AND, r, NIL );
			}
			else if ( ISFTN( r->in.type) )
				p->in.right = buildtree( UNARY AND, r, NIL );
		case CBRANCH :
		case NOT :
			/* change array or function operand to pointer constant */
			if ( ISARY( l->in.type) )
			{
				l->in.type = DECREF( l->in.type );
				++l->fn.cdim;
				p->in.left =  buildtree( UNARY AND, l, NIL );
			}
			else if ( ISFTN( l->in.type) )
				p->in.left = buildtree( UNARY AND, l, NIL );
			else if (! isscalar( l ))
				uerror("scalar required in conditional context");

			break;

		case INCR:
		case DECR:
			/* The only thing we care about here is fixing up the
			** types on INCR and DECR of type float.  Normally they
			** get built as (++[f] (CONV[d] obj[f]) FCON[d]).
			** Make the type of the ++/-- into [d] (since its operands
			** are) and put conversion above.
			*/
			if (p->in.type == FLOAT) {
			    p->in.type = DOUBLE;
			    p->fn.csiz = DOUBLE;
			    p->fn.cdim = 0;
			    p = makety(p, FLOAT, 0, FLOAT);
			}
		case ASG PLUS:
		case ASG MINUS:
			break;
		case CM:			/* must have a value */
			if (   l->in.type == VOID || l->in.type == UNDEF
			    || r->in.type == VOID || r->in.type == UNDEF )
				uerror("void type illegal as argument");
			break;

		case QUEST:			/* must have scalar on left */
			if (! isscalar( l ))
				uerror("scalar required in conditional context");
			break;

		default:
			cerror( "other code %d", o );
		}

	}

	if( actions & CVTO ) p = oconvert(p);
	p = clocal(conval(p));

# ifndef NODBG
	if( bdebug ) eprint(p);
# endif
	return(p);

}

NODE *
strargs( p, off )
register NODE *p;
register *off;
{
	 /* rewrite arguments */
	/* allocate the arguments at *off, and update *off */
	register TWORD t;
	int workoff;

	t = p->fn.type;
	if( p->in.op == CM )
	{
#if defined(BACKARGS) && defined(BACKPARAM) || \
	!defined(BACKARGS) && !defined(BACKPARAM)

		p->in.left = strargs( p->in.left, off );
		p->in.right = strargs( p->in.right, off );

#else

		p->in.right = strargs(p->in.right, off);
		p->in.left = strargs(p->in.left, off);

#endif
		return( p );
	}

	p = block( STARG, p, NIL, t, p->fn.cdim, p->fn.csiz );
	if( t == STRTY || t == UNIONTY )
	{
		p->in.left = buildtree( UNARY AND, p->in.left, NIL );
	}
	else
	{
		p->in.op = FUNARG;
	}

	SETOFF( *off, talign( t, p->fn.csiz ) );
	workoff = tsize( t, p->fn.cdim, p->fn.csiz );
	/* force minimum size to be size of INT for struct/union */
	if (t == STRTY || t == UNIONTY) {
	    SETOFF( workoff, ALINT );
	}

#ifdef BACKARGS
	*off += workoff;
	p->tn.rval = -*off;
#else
	p->tn.rval = *off;
	*off += workoff;
#endif
	return( clocal(p) );
}

chkstr( i, j, type )
register TWORD type;
register i,j;
{
	/* is the MOS or MOU at stab[i] OK for strict reference by a ptr */
	/* i has been checked to contain a MOS or MOU */
	/* j is the index in dimtab of the members... */
	register k, kk;

	extern int ddebug;

# ifndef NODBG
	if( ddebug > 1 ) printf( "chkstr( %s(%d), %d )\n", stab[i].sname, i, j );
# endif
	if( (k = j) < 0 ) uerror( "undefined structure or union" );
	else
	{
		for( ; (kk = dimtab[k] ) >= 0; ++k )
		{
			if( kk >= SYMTSZ )
			{
				cerror( "gummy structure" );
				return(1);
			}
			if( kk == i ) return( 1 );
			switch( stab[kk].stype )
			{

			case STRTY:
			case UNIONTY:
				if( type == STRTY ) continue;  /* no recursive looking for strs */
				if( hflag && chkstr( i, dimtab[stab[kk].sizoff+1], stab[kk].stype ) )
				{
					if( stab[kk].sname[0] == '$' ) return(0);  /* $FAKE */
					werror(
					"illegal member use: perhaps %s.%s?",
						stab[kk].sname, stab[i].sname);
					return(1);
				}
			}
		}
	}
	return( 0 );
}

NODE *
conval( p )
register NODE *p;
{
	register NODE *l, *r;
	int o, u;

	u = ISUNSIGNED(p->tn.type);

	switch( optype(o = p->tn.op) )
	{
	case BITYPE:
		l = p->in.left;
		r = p->in.right;
		break;

	case UTYPE:
		r = l = p->in.left;
		break;

	case LTYPE:
		return( p );
	}

	/* float/double cases */
	if ( l->tn.op == FCON && r->tn.op == FCON ) {

	    double lval = l->fpn.dval;
	    double rval = r->fpn.dval;

	    errno = 0;			/* check float/double range error */

	    switch( o ) {
	    case PLUS:	l->fpn.dval = FP_PLUS(lval, rval); break;
	    case MINUS:	l->fpn.dval = FP_MINUS(lval, rval); break;
	    case MUL:	l->fpn.dval = FP_TIMES(lval, rval); break;
	    case DIV:
		    if( FP_ISZERO(rval) )
			uerror( "division by 0" );
		    else
			l->fpn.dval = FP_DIVIDE(lval, rval);
		    break;

	    case UNARY MINUS:
			l->fpn.dval = FP_NEG(lval); break;
	    case NOT:	l->tn.lval = FP_ISZERO(lval); break;

	    case LT:	l->tn.lval = FP_CMPD(lval, rval) < 0; break;
	    case LE:	l->tn.lval = FP_CMPD(lval, rval) <= 0; break;
	    case GT:	l->tn.lval = FP_CMPD(lval, rval) > 0; break;
	    case GE:	l->tn.lval = FP_CMPD(lval, rval) >= 0; break;
	    case EQ:	l->tn.lval = FP_CMPD(lval, rval) == 0; break;
	    case NE:	l->tn.lval = FP_CMPD(lval, rval) != 0; break;
	    case ANDAND:l->tn.lval = !FP_ISZERO(lval) && !FP_ISZERO(rval); break;
	    case OROR:	l->tn.lval = !FP_ISZERO(lval) || !FP_ISZERO(rval); break;
	    default:
		    return(p);
	    }
	    if (logop(o)) {
		l->tn.op = ICON;	/* left side now INT constant */
		l->tn.type = INT;
		l->tn.rval = NONAME;	/* clean out FCON debris */
	    }
	    else if (p->in.type == FLOAT)
		l->fpn.dval = FP_DTOFP(l->tn.lval); /* truncate to FLOAT result */

	    /* FP_* routines may set errno on range error */
	    if (errno)
		werror("float/double constant folding out of range");

	} /* end float/double case */
	else if ( l->tn.op == ICON && r->tn.op == ICON ) {
	    CONSZ rval = r->tn.lval;

	    /* weed out unprofitable integer cases */
	    if( l->tn.rval != NONAME && r->tn.rval != NONAME ) return(p);
	    if( r->tn.rval != NONAME && o!=PLUS ) return(p);
	    if( l->tn.rval != NONAME && o!=PLUS && o!=MINUS ) return(p);


	    switch( o )
	    {
	    case PLUS:
			l->tn.lval += rval;
			if( l->tn.rval == NONAME )
			{
				l->tn.rval = r->tn.rval;
				l->in.type = r->in.type;
			}
			break;

	    case MINUS:	l->tn.lval -= rval; break;
	    case MUL:	l->tn.lval *= rval; break;
	    case DIV:
			if( rval == 0 )
			    uerror( "division by 0" );
			else if( u )
			    l->tn.lval = (unsigned long)l->tn.lval / rval;
			else
			    l->tn.lval /= rval;
			break;

	    case MOD:
			if( rval == 0 )
			    uerror( "division by 0" );
			else if( u )
			    l->tn.lval = (unsigned long)l->tn.lval % rval;
			else
			    l->tn.lval %= rval;
			break;

	    case AND:	l->tn.lval &= rval; break;
	    case OR:	l->tn.lval |= rval; break;
	    case ER:	l->tn.lval ^=  rval; break;
	    case LS:	l->tn.lval <<= (int) rval; break;
	    case RS:	l->tn.lval >>= (int) rval; break;
	    case UNARY MINUS:
			l->tn.lval = - l->tn.lval; break;
	    case COMPL: l->tn.lval = ~l->tn.lval; break;
	    case NOT:	l->tn.lval = !l->tn.lval; break;
	    case LT:	l->tn.lval = l->tn.lval < rval; break;
	    case LE:	l->tn.lval = l->tn.lval <= rval; break;
	    case GT:	l->tn.lval = l->tn.lval > rval; break;
	    case GE:	l->tn.lval = l->tn.lval >= rval; break;
	    case ULT:	l->tn.lval = (l->tn.lval-rval)<0; break;
	    case ULE:	l->tn.lval = (l->tn.lval-rval)<=0; break;
	    case UGE:	l->tn.lval = (l->tn.lval-rval)>=0; break;
	    case UGT:	l->tn.lval = (l->tn.lval-rval)>0; break;
	    case EQ:	l->tn.lval = l->tn.lval == rval; break;
	    case NE:	l->tn.lval = l->tn.lval != rval; break;
	    case ANDAND:l->tn.lval = l->tn.lval && rval; break;
	    case OROR:	l->tn.lval = l->tn.lval || rval; break;

	    default:
		    return(p);
	    }
	} /* end of int case */
	else
	    return( p );		/* can't change others */

	if( l != r ) r->tn.op = FREE; /* don't clobber unary answer */
	l = makety( l, p->tn.type, p->fn.cdim, p->fn.csiz );
	p->tn.op = FREE;
	return( l );
}

chkpun(p)
register NODE *p;
{
	/* checks p for the existance of a pun */
	/* this is called when the op of p is ASSIGN, RETURN, CAST, COLON,
	** or relational
	*/

	/* in the other case, one operand is a pointer, the other integer */
	/* we check that this integer is in fact a constant zero... */
	/* if it is, we redo the type of the 0 to match the other side */

	/* with ASSIGN, assignment of pointer to integer is illegal */
	/* this falls out, because the LHS is never 0 */

	register NODE *q1, *q2;
	register t1, t2;
	register d1, d2;

	q1 = p->in.left;
	q2 = p->in.right;
	t1 = q1->in.type;
	t2 = q2->in.type;


	if( ISPTR(t1) || ISARY(t1) )
	{	/* switch roles: q1 should be non-pointer */
		q1 = q2;
		q2 = p->in.left;
	}

	if( !ISPTR(q1->in.type) && !ISARY(q1->in.type) )
	{
		if( q1->in.op == ICON && q1->tn.lval == 0 && q1->tn.rval == NONAME )
		{	/* make the 0 have the type of the other side */
			q1->fn.type = q2->fn.type;
			q1->fn.cdim = q2->fn.cdim;
			q1->fn.csiz = q2->fn.type;
		}
		else
		{
			combo( "pointer/integer", p );

			/* since only giving a warning, paint over type 
			/* to avoid producing incorrect code
			*/
			q1->fn.type = q2->fn.type;
			q1->fn.cdim = q2->fn.cdim;
			q1->fn.csiz = q2->fn.type;

		}
		return;
	}
	else
	{	/* q1 and q2 still mean left and right */
		d1 = q1->fn.cdim;
		d2 = q2->fn.cdim;
		for( ;; )
		{
			if( t1 == t2 )
			{
				;
				if(q1->fn.csiz!=q2->fn.csiz)
				{
					combo( "structure pointer", p );
				}
				return;
			}
/****** LINT Change: Turn on only if LINT is to be used with this pcc2 ******/
/*			/* complain about pointer casts if cflag is set
/*			/* (this implies that pflag is set)
/*			*/
/*			if (p->in.op == CAST)
/*			{
/*				if (ISPTR(tl) && ISPTR(t2))
/*				{
/*					werror(
/*					"pointer casts may be troublesome");
/*					return;
/*				}
/*			}
/****************************************************************************/
			if( ISARY(t1) || ISPTR(t1) )
			{
				if( !ISARY(t2) && !ISPTR(t2) ) break;
				if( ISARY(t1) && ISARY(t2) &&
				    dimtab[d1] != dimtab[d2] )
				{
					combo( "array size", p );
					return;
				}
				if( ISARY(t1) ) ++d1;
				if( ISARY(t2) ) ++d2;
			}
			else break;
			t1 = DECREF(t1);
			t2 = DECREF(t2);
		}
		combo( "pointer", p );
	}
}

combo( s, p )
char *s;
NODE *p;
{
	char buf[100];
	sprintf( buf, "illegal %s combination, op %s", s, opst[p->tn.op] );
	werror( buf );
}

NODE *
stref( p )
register NODE *p;
{
	register TWORD t;
	register d, s, dsc, align;
	register OFFSZ off;
	register struct symtab *q;
	register NODE *qq;	/* for holding symtab info if conversion from */
				/* ENUMTY needed */
	int newtype = 0;	/* for changing field references */

	/* make p->x */
	/* this is also used to reference automatic variables */

	q = &stab[p->in.right->tn.rval];
	p->in.right->in.op = FREE;
	qq = p;		/* keep a scratch node for ENUMTY conversions */
	p = pconvert( p->in.left );

	/* make p look like ptr to x */

	if( !ISPTR(p->in.type))
	{
		p->in.type = PTR+UNIONTY;
	}

	d = qq->fn.cdim = q->dimoff;
	s = qq->fn.csiz = q->sizoff;
	qq->in.type = q->stype;

	if( BTYPE(qq->in.type) == ENUMTY ) 	/* change enums to ints */
		econvert(qq);

	t = INCREF( qq->in.type );


	/* compute the offset to be added */

	off = q->offset;
	dsc = q->sclass;

	/* see if we can change a bitfield reference to a direct CHAR, SHORT,
	** INT, or LONG reference.
	*/

	if ( dsc & FIELD ) {
	    int size = dsc&FLDSIZ;	/* size of bitfield */

	    if (size == SZCHAR && off % ALCHAR == 0)
		newtype = UCHAR;
#ifndef	NOSHORT
	    else if (size == SZSHORT && off % ALSHORT == 0)
		newtype = USHORT;
#endif
	    else if (size == SZINT && off % ALINT == 0)
		newtype = UNSIGNED;
#ifndef	NOLONG
	    else if (size == SZLONG && off % ALLONG == 0)
		newtype = ULONG;
#endif

	    if (newtype != 0) {	/* we can make a change */
#ifndef	NODBG
		if (bdebug) {
		    printf("changing bitfield reference:\n");
		    eprint(p);
		}
#endif
#ifdef	SIGNEDFIELDS
		/* make type signed if user's was */
		if (   qq->in.type == CHAR || qq->in.type == SHORT
		    || qq->in.type == INT || qq->in.type == LONG)
		    newtype = DEUNSIGN(newtype);
#endif
		dsc &= ~FIELD;		/* turn off FIELD-ness */
		t = INCREF(newtype);	/* use this as the "real" type */
#ifndef	NODBG
		if (bdebug)
		    printf("to:  type %d, offset %d\n", t, off);
#endif
	    }
	}

	p = makety( p, t, d, s );

	if( dsc & FIELD )
	{
		  /* normalize offset */
		switch(qq->in.type)
		{

		case CHAR:
		case UCHAR:
			align = ALCHAR;
			s = CHAR;
			break;

		case SHORT:
		case USHORT:
			align = ALSHORT;
			s = SHORT;
			break;

		case ENUMTY:
			cerror("enum bitfield not converted");
			break;
		case INT:
		case UNSIGNED:
			align = ALINT;
			s = INT;
			break;

# ifdef LONGFIELDS
		case LONG:
		case ULONG:
			align = ALLONG;
			s = LONG;
			break;
# endif

		default:
			cerror( "undefined bit field type" );
		}
/* If fields MUST be aligned, force alignment */
#ifdef	ALFIELD
		align = ALFIELD;
#endif
		off = (off/align)*align;
	}
	if( off != 0 )
		p = clocal( block( PLUS, p, offcon( off, t, d, s ), t, d, s ) );

	p = buildtree( STAR, p, NIL );
	if (newtype)
	    p->fn.flags |= FF_ISFLD;	/* note this node started as a field */

	/* if field, build field info */

	if( dsc & FIELD )
	{
		p = block( FLD, p, NIL, qq->in.type, 0, q->sizoff );
		p->tn.rval = PKFIELD( dsc&FLDSIZ, q->offset%align );
	}

	qq->in.op = FREE;	/* throw-out scratch node */

	return( clocal(p) );
}

notlval(p, structok)
register NODE *p;
int structok;				/* non-zero if struct reference is
					** okay as lvalue
					*/
{

	/* return 0 if p an lvalue, 1 otherwise */

again:
	switch( p->in.op )
	{

	case FLD:
		p = p->in.left;
		goto again;

	case STAR:
		if (!structok) {
		    int o = p->in.left->in.op;
		    TWORD ty = BTYPE(p->in.left->in.type);

		    /* fix the &(a=b) bug, given that a and b are structures */
		    /* also, fix stcall() = *p bug */
		    if(   (o == STASG || o == STCALL || o == UNARY STCALL)
		       && (ty == STRTY || ty == UNIONTY)
		       )  return( 1 );
		}
	case NAME:
	case VAUTO:
	case VPARAM:
	case RNODE:
	case QNODE:
	case SNODE:
		if( ISARY(p->in.type) || ISFTN(p->in.type) ) return(1);
	case REG:
		return(0);

	case COMOP:
		if (structok) {
		    p = p->in.right;	/* is lval if right side is */
		    goto again;
		}
		else
		    return(1);
	
	case QUEST:
		if (structok) {
		    p = p->in.right;	/* is lval if both sides of : are */
		    return( notlval(p->in.left, 1) || notlval(p->in.right,1) );
		}
		else
		    return(1);

	default:
		return(1);

	}

}

NODE *
bcon( i )
{
	 /* make a constant node with value i */
	register NODE *p;

	p = block( ICON, NIL, NIL, INT, 0, INT );
	p->tn.lval = i;
	p->tn.rval = NONAME;
	return( clocal(p) );
}

NODE *
bpsize(p)
register NODE *p;
{
	return( offcon( psize(p), p->in.type, p->fn.cdim, p->fn.csiz ) );
}

OFFSZ
psize( p )
register NODE *p;
{
	/* p is a node of type pointer; psize returns the
	** size of the thing pointed to
	*/

	if( !ISPTR(p->in.type) )
	{
		uerror( "pointer required");
		return( SZINT );
	}
	/* note: no pointers to fields */
	return( tsize( DECREF(p->in.type), p->fn.cdim, p->fn.csiz ) );
}

NODE *
unconvert( p, q )
register NODE *p, *q;
{
	/* p is divided by the size of q */
	/* q had better have type pointer */

# ifdef MYPDIV
	return( clocal( block( PDIV, p, bpsize(q), INT, 0, INT ) ) );
# else
	p = makety( p, PTRTYPE, 0, PTRTYPE );
	p = clocal( buildtree( DIV, p, bpsize(q) ) );

	return( makety( p, PTRTYPE, 0, PTRTYPE ) );
# endif
}

NODE *
convert( p, f )
register NODE *p;
register f;
{
	/*  convert an operand of p
	** f is either CVTL or CVTR
	** operand has type int, and is converted by the size of the other side
	*/

	register NODE *q, *r;

	if( f == CVTL )
	{
		q = p->in.left;
		r = bpsize( p->in.right );
	}
	else
	{
		q = p->in.right;
		r = bpsize( p->in.left );
	}
# ifdef MYPMUL
	r = clocal( block( PMUL, q, r, PTRTYPE, 0, PTRTYPE ) );
# else
	/* if PTRTYPE is defined, make q this size; then, the MUL will be */
	q = makety( q, PTRTYPE, 0, PTRTYPE );
	r = clocal( buildtree( MUL, q, r ) );
# endif
	if( f == CVTL )
		p->in.left = r;
	else
		p->in.right = r;
	return(p);

}

econvert( p )
register NODE *p;
{

	/* change enums to ints, or appropriate types */

	register TWORD ty;

	if( (ty=BTYPE(p->in.type)) == ENUMTY || ty == MOETY )
	{
		if( dimtab[ p->fn.csiz ] == SZCHAR ) ty = CHAR;
		else if( dimtab[ p->fn.csiz ] == SZINT ) ty = INT;
		else if( dimtab[ p->fn.csiz ] == SZSHORT ) ty = SHORT;
		else ty = LONG;
		ty = ctype( ty );
		p->fn.csiz = ty;
		MODTYPE(p->in.type,ty);
		if( p->in.op == ICON && ty != LONG )
			p->in.type = p->fn.csiz = INT;
	}
}

NODE *
pconvert( p )
register NODE *p;
{
	register TWORD t;

	/* if p should be changed into a pointer, do so */
	/* also, widen p so it can be used as an operand */

	if( ISARY( p->in.type) )
	{
		p->in.type = DECREF( p->in.type );
		++p->fn.cdim;
		return( buildtree( UNARY AND, p, NIL ) );
	}
	if( ISFTN( p->in.type) )
		return( buildtree( UNARY AND, p, NIL ) );

	t = indtype( p->tn.type );
	if( t == p->tn.type ) return( p );
	if( p->tn.op == CONV && cbigger( p ) )
	{
		p->tn.type = p->fn.csiz = t;
	}
	else p = makety( p, t, 0, (int) t );
	return( p );
}

bigsize( t )
TWORD	t; 
{
	/* return a type size for t */
	switch( t ) 
	{

	case CHAR:
	case UCHAR:
		return( SZCHAR );

	case SHORT:
	case USHORT:
		return( SZSHORT );

	case VOID:
	case INT:
	case UNSIGNED:
		return( SZINT );

	case LONG:
	case ULONG:
		return( SZLONG );

	case FLOAT:
		return( SZLONG+1 );  /* must appear > long */
	case DOUBLE:
		return( SZLONG+2 );

	case STRTY:
	case UNIONTY:		/* structures and union can't be converted */
		uerror("conversion of structure or union not allowed");
		return( SZLONG );

	}

	if( ISPTR(t) || ISARY(t) ) return( SZPOINT );
	cerror( "bad bigsize: 0%o", t );
/*NOTREACHED*/
}

cbigger( p )
register NODE *p; 
{
	/* returns 1 if the conversion op p makes things bigger */

	if( p->tn.op != CONV ) cerror( "cbigger" );
	return( bigsize( p->tn.type ) > bigsize( p->in.left->tn.type ) );
}

NODE *
oconvert(p)
register NODE *p;

{
	/* convert the result itself: used for pointer and unsigned */
	register TWORD t;

	switch(p->in.op)
	{

	case LE:
	case LT:
	case GE:
	case GT:
		t = p->in.left->in.type;
		if( ISUNSIGNED(t) || ISPTR(t) || ISARY(t) )
		{
			p->in.op += (ULE-LE);
		}
		else
		{
			t = p->in.right->in.type;
			if( ISUNSIGNED(t) || ISPTR(t) || ISARY(t) )
			{
				p->in.op += (ULE-LE);
			}
		}
	case EQ:
	case NE:
	case ULE:
	case ULT:
	case UGE:
	case UGT:
		return( p );

	case MINUS:
		return( unconvert( p, p->in.left ) );
	}

	cerror( "illegal oconvert: %d", p->in.op );

	return(p);
}

NODE *
ptmatch(p)
register NODE *p;
{

	/* makes the operands of p agree; they are
	** either pointers or integers, by this time
	*/
	/* with MINUS, the sizes must be the same */
	/* with COLON, the types must be the same */

	register TWORD t1, t2, t;
	register o, d2, d, s2, s;
	NODE * r;

	o = p->in.op;
	t = t1 = p->in.left->in.type;
	t2 = p->in.right->in.type;
	d = p->in.left->fn.cdim;
	d2 = p->in.right->fn.cdim;
	s = p->in.left->fn.csiz;
	s2 = p->in.right->fn.csiz;

	switch( o )
	{

	case ASSIGN:
	case CAST:
		r = p->in.right;
		if (   (BTYPE(r->in.type) == STRTY || BTYPE(r->in.type) == UNIONTY)
		    && (r->in.op == STCALL || r->in.op == UNARY STCALL)
		    )
		    uerror("can't take address of return value");
	case RETURN:			/* same check when return changed to
					** assign to RNODE
					*/
		break;

	case MINUS:
		if( psize(p->in.left) != psize(p->in.right) )
		{
			uerror( "illegal pointer subtraction");
		}
		p->tn.type = p->fn.csiz = PTRTYPE;
		p->fn.cdim = 0;
		return( clocal(p) );

	case COLON:
		if( t1 != t2 ) uerror( "illegal types in :");
		break;

	default:
		if( !ISPTR(t1) )
		{
			t = t2;
			d = d2;
			s = s2;
			break;
		}
		if( !ISPTR(t2) )
		{
			break;
		}

		/* both are pointers */
		if( talign(t2,s2) < talign(t,s) )
		{
			t = t2;
			s = s2;
		}
		break;
	}

	p->in.left = makety( p->in.left, t, d, s );
	p->in.right = makety( p->in.right, t, d, s );
	if( !logop(o) )
	{
		p->in.type = t;
		p->fn.cdim = d;
		p->fn.csiz = s;
	}

	return(clocal(p));
}

int tdebug = 0;

NODE *
tymatch(p)
register NODE *p;
{

	/* satisfy the types of various arithmetic binary ops */

	/* rules are:
	** if any float or doubles, make double if singflag == 0
	** else if singflag == 1, allow single precision arithmetic:
	**	if any doubles, make double
	**	if a float and an integer constant, make double to preserve precision
	**	if any floats, make float
	** if any longs, make long
	** if any ints, make int
	** if any shorts, make short
	** otherwise, make char
	** if either operand is unsigned, the result is...
	** if a simple assignment, type = type of lhs
	** if an asg. op, type = type of original lhs
	** 	(under CONV, if any)
	*/

	register o;
	register TWORD t1, t2;
	TWORD t, tu;
	register NODE *l, *r;
	int u;
	int o1, o2;

	o = p->in.op;

	t1 = (l=p->in.left)->in.type;
	t2 = (r=p->in.right)->in.type;

	/* constants have a kind of "flexible" type */

	if( (o2 = r->tn.op) == ICON && r->tn.rval == NONAME &&
	    (t1==CHAR || t1==UCHAR || t1==SHORT || t1==USHORT) )
	{
		/* if the constant retains its value when cast to the
		** type of the lhs, assume it has the lhs type
		*/
		if( r->tn.lval == ccast( r->tn.lval, t1 ) )
		{
			r->in.type = t2 = t1;
			r->fn.cdim = l->fn.cdim;
			r->fn.csiz = l->fn.csiz;
		}
	}

	/* this is the opposite case from the above (it is too early
		to assume that constants are on the right) */

	if( (o1 = l->tn.op) == ICON && l->tn.rval == NONAME &&
	    (t2==CHAR || t2==UCHAR || t2==SHORT || t2==USHORT) )
	{
		/* if the constant retains its value when cast to the
		** type of the rhs, assume it has the rhs type
		*/
		if( l->tn.lval == ccast( l->tn.lval, t2 ) )
		{
			l->in.type = t1 = t2;
			l->fn.cdim = r->fn.cdim;
			l->fn.csiz = r->fn.csiz;
		}
	}

	u = 0;
	if( ISUNSIGNED(t1) )
	{
		u = 1;
		t1 = DEUNSIGN(t1);
	}
	if( ISUNSIGNED(t2) )
	{
		u = 1;
		t2 = DEUNSIGN(t2);
	}

	/* float functions always return doubles, even if single precision */
	/* option is set */
	if ( singflag && o1 == RNODE &&  t1 == FLOAT )  l->in.type = t1 = DOUBLE;

	if( (!singflag) && (t1==DOUBLE || t1==FLOAT || t2==DOUBLE || t2==FLOAT) ) 
		t = DOUBLE;
	else if( t1==DOUBLE || t2==DOUBLE ) t=DOUBLE;
	else if( (t1==FLOAT && o2==ICON) || (t2==FLOAT && o1==ICON) ) t=DOUBLE;
	else if( t1==FLOAT || t2==FLOAT ) t=FLOAT;
	else if( t1==LONG || t2==LONG ) t = LONG;
	else if( t1==INT || t2==INT ) t = INT;
	else if( t1==SHORT || t2==SHORT ) t = SHORT;
	else t = CHAR;

	if( o == ASSIGN || o == CAST || o == RETURN )
	{
		t = tu = t1;	/* set up to avoid lhs conversion */
		u = ISUNSIGNED(l->tn.type);
		t2 = r->tn.type;  /* back to reality... */
	}
	tu = (u && UNSIGNABLE(t))?ENUNSIGN(t):t;

	if( tu != t1 ) p->in.left = makety( l, tu, 0, (int)tu );
	/* for shift operators, the right side MUST be an INT.  Since
	** widening has (we hope) already been done, force to INT if
	** appropriate.
	*/
	if ( o == LS || o == RS || o == ASG LS || o == ASG RS ) {
	    if (t2 != INT)
		p->in.right = makety( r, INT, 0, INT );
	}
	else if( tu != t2 ) p->in.right = makety( r, tu, 0, (int)tu );

	if( !logop(o) )	/* the type of the node = type of the operation */
	{
		p->in.type = tu;
		p->fn.cdim = 0;
		p->fn.csiz = t;
	}

	if( asgop(o) && p->in.left->tn.op == CONV  )
	{
		/* original, unconverted type */
		/* this will get rewritten to A = A op B, in most cases */
		/* if the op is an unsigned op, change the lhs to unsigned */
		/* and add a coversion so the result is the type of original lhs */
		l = p->in.left->in.left;
		t1 = tu = l->in.type;
		if( u && (UNSIGNABLE(tu)) ) tu = ENUNSIGN(tu);
		r = 0;
		if( tu != t1 )
		{
		    r = talloc();		/* r doesn't mean right, here. */
		    r->in.op = CONV;		/* make a conversion node so that */
		    r->in.type = t1;		/* result is type of lhs */
		    r->in.left = p;
		    r->fn.csiz = l->fn.csiz;	/* be sure to propagate size info */
		    r->fn.cdim = l->fn.cdim;
		}
		p->in.type = l->in.type = tu;
		p->fn.cdim = l->fn.cdim;
		p->fn.csiz = l->fn.csiz;
		if( r ) p = r;
	}

# ifndef NODBG
	if( tdebug )
	{
		printf( "tymatch(%o): %o '%s' %o => %o\n",p,t1,opst[o],t2,tu );
		eprint(p);
	}
# endif

	return(p);
}

NODE *
makety( p, t, d, s )
register NODE *p;
register TWORD t;
register d,s;
{
	/* make p into type t by inserting a conversion */
	register TWORD pt;

	pt = p->in.type;


 	if( ISARY(pt) || ISFTN(pt) )
 	{
 		p = pconvert( p );
 		pt = p->in.type;
 	}
 
	if( t == pt )
	{
rew:
		p->fn.type = t;
		p->fn.cdim = d;
		p->fn.csiz = s;
		return( p );
	}

	if( t & TMASK )
	{
		/* non-simple type */
		if( ISPTR(pt)
# ifdef TWOPTRS
		    && TWOPTRS(t) == TWOPTRS(pt)
# endif
		    )
		{
			/* don't generate CONV: just rewrite type */
			goto rew;
		}

		return( block( CONV, p, NIL, t, d, s ) );
	}

	if( p->in.op == ICON && p->tn.rval==NONAME )
	{
		if( t==DOUBLE||t==FLOAT )
		{
			p->in.op = FCON;
			if( ISUNSIGNED(pt) )
			    p->fpn.dval = FP_ULTOD((unsigned long) p->tn.lval);
			else
			    p->fpn.dval = FP_LTOD(p->tn.lval);

			goto rew;
		}
		p->tn.lval = ccast( p->tn.lval, t );
		goto rew;
	}
	return( block( CONV, p, NIL, t, d, s ) );
}

NODE *
block( o, l, r, t, d, s )
register NODE *l, *r;
register TWORD t;
register o,d,s;
{

	register NODE *p;

	p = talloc();
	p->in.op = o;
	p->in.left = l;
	p->in.right = r;
	p->in.type = t;
	p->fn.cdim = d;
	p->fn.csiz = s;
	p->fn.flags = 0;

/*	for really heavy debugging
	printf( "block( %s, %d, %d, 0%o, %d, %d ) yields %d\n",
		opst[o], node_no(l), node_no(r), t, d, s, node_no(p) );
*/

	return(p);
}

icons(p)
register NODE *p;
{
	/* if p is an integer constant, return its value */
	register val;

	if( p->in.op != ICON )
	{
		uerror( "constant expected");
		val = 1;
	}
	else
	{
		val = p->tn.lval;
		if( val != p->tn.lval )
			uerror( "constant too big for cross-compiler" );
	}
	tfree( p );
	return(val);
}

/* 	the intent of this table is to examine the
** operators, and to check them for
** correctness.
**
** The table is searched for the op and the
** modified type (where this is one of the
** types INT (includes char and short), LONG,
** DOUBLE (includes FLOAT), and POINTER
**
** The default action is to make the node type integer
**
** The actions taken include:
** PUN	  check for puns
** CVTL	  convert the left operand
** CVTR	  convert the right operand
** TYPL	  the type is determined by the left operand
** TYPR	  the type is determined by the right operand
** TYMATCH	  force type of left and right to match, by inserting conversions
** PTMATCH	  like TYMATCH, but for pointers
** LVAL	  left operand must be lval
** CVTO	  convert the op
** NCVT	  do not convert the operands
** OTHER	  handled by code
** NCVTR	  convert the left operand, not the right...
**
*/

# define MINT 01  /* integer */
# define MDBI 02   /* integer or double */
# define MSTR 04  /* structure */
# define MPTR 010  /* pointer */
# define MPTI 020  /* pointer or integer */
# define MENU 040 /* enumeration variable or member */

opact( p )
register NODE *p;
{
	register mt12, mt1, mt2, o;

	mt12 = 0;

	switch( optype(o=p->in.op) )
	{

	case BITYPE:
		mt12=mt2 = moditype( p->in.right->in.type );
	case UTYPE:
		mt12 &= (mt1 = moditype( p->in.left->in.type ));

	}

	switch( o )
	{

	case NAME :
	case STRING :
	case ICON :
	case FCON :
	case CALL :
	case UNARY CALL:
	case CM:
	case STAR:
		{
			  return( OTHER );
		}
	case UNARY MINUS:
		if( mt1 & MDBI ) return( TYPL );
		break;

	case COMPL:
		if( mt1 & MINT ) return( TYPL );
		break;

	case UNARY AND:
		{
			  return( NCVT+OTHER );
		}

	case INIT:
		return( 0 );
	case NOT:
	case CBRANCH:
	case ANDAND:
	case OROR:
		return( NCVT+OTHER );

	case MUL:
	case DIV:
		if( mt12 & MDBI ) return( TYMATCH );
		break;

	case MOD:
	case AND:
	case OR:
	case ER:
	case LS:
	case RS:
		if( mt12 & MINT ) return( TYMATCH );
		break;

	case EQ:
	case NE:
	case LT:
	case LE:
	case GT:
	case GE:
		if( mt12 & MDBI ) return( TYMATCH+CVTO );
		else if( mt12 & MPTR ) return( PTMATCH+PUN+CVTO );
		else if( mt12 & MPTI ) return( PTMATCH+PUN+CVTO );
		else break;

	case QUEST:
		return( TYPR+NCVTR+OTHER );

	case COMOP:
		return( TYPR+NCVTR );

	case STREF:
		return( NCVTR+OTHER );

	case COLON:
		if( mt12 & MDBI ) return( TYMATCH );
		else if( mt12 & MPTR ) return( TYPL+PTMATCH+PUN );
		else if( (mt1&MINT) && (mt2&MPTR) ) return( TYPR+PUN );
		else if( (mt1&MPTR) && (mt2&MINT) ) return( TYPL+PUN );
		else if( mt12 & MSTR ) return( NCVT+TYPL+OTHER );
		/* both sides void */
		else if( !mt1 && !mt2 ) return( OTHER );
		break;

	case ASSIGN:
	case RETURN:
		if( mt12 & MSTR ) return( LVAL+NCVT+TYPL+OTHER );
	case CAST:
		if(o==CAST && mt1==0)return(TYPL+TYMATCH);
		if( mt12 & MDBI ) return( TYPL+LVAL+TYMATCH );
		else if( mt12 == 0 ) break;
		else if( mt1 & MPTR ) return( LVAL+PTMATCH+PUN );
		else if( mt12 & MPTI ) return( TYPL+LVAL+TYMATCH+PUN );
		break;

	case ASG MUL:
	case ASG DIV:
		if( mt12 & MDBI ) return( LVAL+TYMATCH );
		break;

	case ASG MOD:
	case ASG AND:
	case ASG OR:
	case ASG ER:
	case ASG LS:
	case ASG RS:
		if( mt12 & MINT ) return( LVAL+TYMATCH );
		break;

	case ASG PLUS:
	case ASG MINUS:
	case INCR:
	case DECR:
		if( mt12 & MDBI ) return( TYMATCH+LVAL+OTHER );
		else if( (mt1&MPTR) && (mt2&MINT) ) return( TYPL+LVAL+CVTR );
		break;

	case MINUS:
		if( mt12 & MPTR ) return( CVTO+PTMATCH+PUN );
		if( mt2 & MPTR ) break;
	case PLUS:
		if( mt12 & MDBI ) return( TYMATCH );
		else if( (mt1&MPTR) && (mt2&MINT) ) return( TYPL+CVTR );
		else if( (mt1&MINT) && (mt2&MPTR) ) return( TYPR+CVTL );
		break;

		/* special operators */
	case UOP0:
	case UOP1:
	case UOP2:
	case UOP3:
	case UOP4:
	case UOP5:
	case UOP6:
	case UOP7:
	case UOP8:
	case UOP9:
		return( TYPL );
	}

	if ( (mt1&MENU) || (mt2&MENU) ) cerror("enum operands in opact()!");
	if ( o == CAST )		/* special message, since users don't
					** know CAST has operands
					*/
	    uerror( "illegal cast" );
	else
	    uerror( "operands of %s have incompatible types", opst[o] );
	return( NCVT );
}

moditype( ty )
register TWORD ty;
{

	switch( ty )
	{

	case UNDEF:
	case VOID:
		return(0); /* type is void */
	case ENUMTY:
	case MOETY:
		return( MENU );

	case STRTY:
	case UNIONTY:
		return( MSTR );

	case CHAR:
	case SHORT:
	case UCHAR:
	case USHORT:
		return( MINT|MPTI|MDBI );

	case UNSIGNED:
	case ULONG:
	case INT:
	case LONG:
		return( MINT|MDBI|MPTI );

	case FLOAT:
	case DOUBLE:
		return( MDBI );

	default:
		return( MPTR|MPTI );
	}
}

# ifndef MYCCAST
CONSZ
ccast( v, t )
register CONSZ v;
register TWORD t;
{
	/* cast value v into simple type t */
	/* must be done as it would be on the target machine */

	switch( t )
	{

	case CHAR:
# ifdef CHSIGN
		if( v&ONEBIT(SZCHAR-1) )
		{
			return( v | ~BITMASK(SZCHAR) );
		}
# endif
	case UCHAR:
		return( v & BITMASK(SZCHAR) );

	case SHORT:
		if( v&ONEBIT(SZSHORT-1) )
		{
			return( v | ~BITMASK(SZSHORT) );
		}
	case USHORT:
		return( v & BITMASK(SZSHORT) );

	case INT:
		if( v&ONEBIT(SZINT-1) )
		{
			return( v | ~BITMASK(SZINT) );
		}
	case UNSIGNED:
		return( v & BITMASK(SZINT) );

	default:
		return( v );
	}
}
# endif


NODE *
doszof( p )
register NODE *p;
{
	/* do sizeof p */
	register i;

	/* whatever is the meaning of this if it is a bitfield? */
	i = tsize( p->in.type, p->fn.cdim, p->fn.csiz )/SZCHAR;

	tfree(p);
	if( i <= 0 ) werror( "sizeof returns 0" );
	p = bcon(i);
	p->tn.type = UNSIGNED;  /* damn dmr anyhow! */
	return( p );
}

# ifndef NODBG
eprint(p)
NODE *p;
{
	printf("\n++++++++\n");
	e1print(p,"T");
	printf("---------\n");
}
e1print( p ,s)
register NODE *p;
char *s;
{
	register ty, d;
	static down = 0;

	ty = optype( p->tn.op );

	if( ty == BITYPE )
	{
		++down;
		e1print( p->in.right ,"R");
		--down;
	}

	for( d=down; d>1; d -= 2 ) printf( "\t" );
	if( d ) printf( "    " );

	printf("%s=%d) %s, ", s, (int) node_no(p), opst[p->in.op] );
	if( ty == LTYPE )
	{
		printf( "lval=%ld", p->tn.lval );
		printf( ", rval=%d, ", p->tn.rval );
	}
	printf("type=");
	tprint( p->in.type );
	if (p->in.op == FLD)
	    printf(", foff=%d, fsiz=%d", UPKFOFF(p->tn.rval), UPKFSZ(p->tn.rval));
	printf( ", dim=%d, siz=%d\n", p->fn.cdim, p->fn.csiz );
	if( ty != LTYPE )
	{
		++down;
		e1print( p->in.left ,"L");
		--down;
	}
}

tprint( t )
register TWORD t; 
{
	 /* output a nice description of the type of t */
	static char * tnames[] = 
	{
		"undef",
		"farg",
		"char",
		"short",
		"int",
		"long",
		"float",
		"double",
		"strty",
		"unionty",
		"enumty",
		"moety",
		"uchar",
		"ushort",
		"unsigned",
		"ulong",
		"?", "?"
	};

	for(;; t = DECREF(t) )
	{
		if( ISPTR(t) ) printf( "PTR " );
		else if( ISFTN(t) ) printf( "FTN " );
		else if( ISARY(t) ) printf( "ARY " );
		else 
		{
			printf( "%s", tnames[t] );
			return;
		}
	}
}
# endif

# ifndef MYLOCCTR
locctr(l)
register l;
{
	register temp, lt;
	/* look in locnames; print out the location counter name */
	/* null means use the next; all null, don't print */
	for( lt=l; lt<=STRNG && !locnames[lt]; ++lt )
	{
		 /* EMPTY */
	}
	if( lt == curloc ) return( lt );
	temp = curloc;
	if( lt > STRNG ) lt = l;
	else printf( locnames[lt] );
	curloc = lt;
	return( temp );
}
# endif

# ifndef NOFLOAT

prtdcon( p )
register NODE *p;
{
	register i;
	register TWORD t;

	if( p->in.op == FCON && ! LITDCON(p) )
	{
		locctr( DATA );
		t = p->tn.type;
		defalign( t==DOUBLE?ALDOUBLE:ALFLOAT );
		deflab( i = getlab() );
		fincode( p->fpn.dval, t==DOUBLE?SZDOUBLE:SZFLOAT );
		p->tn.lval = 0;
		p->tn.rval = -i;
		p->in.op = NAME;
	}
	if( (i = optype(p->in.op)) == BITYPE ) prtdcon( p->in.right );
	if( i != LTYPE ) prtdcon( p->in.left );
}
# endif

# ifndef MYLABELS
getlab()
{
	static crslab = 10;
	return( ++crslab );
}
# endif

int edebug = 0;
ecomp( p )
register NODE *p;
{
# ifndef NODBG
	if( edebug ) eprint(p);
# endif
	if( !reached )
	{
		werror( "statement not reached" );
		reached = 1;
	}
# ifdef CLOCAL
	p = clocal(p);
# endif
	p = optim(p);
# ifndef NOFLOAT
	prtdcon(p);
# endif
	locctr( PROG );
	ecode(p);
	tfree(p);
}

# ifndef MYECODE
ecode( p )
register NODE *p;
{
	/* standard version of writing the tree nodes */
	if( nerrors ) return;
# ifdef SDB
	sdbline();
# endif
	watchdog = 0;			/* start panic countdown */

	p2tree( p );
	p2compile( p );
}
# endif

# ifndef MYPRTREE

# ifndef RNODNAME
# define RNODNAME LABFMT
# endif

p2tree(p)
register NODE *p;
{
	register ty;
	register o;
	char temp[32];			/* place to dump label stuff */
	TWORD oldbtype = BTYPE(p->in.type); /* old basic type of node */

# ifdef MYP2TREE
	MYP2TREE(p);  /* local action can be taken here; then return... */
# endif

	/* this routine sits painfully between pass1 and pass2 */
	ty = optype(o=p->in.op);
	p->tn.goal = 0;  /* an illegal goal, just to clear it out */
	p->tn.type = ttype( p->tn.type );  /* type gets second-pass (bits) */

	switch( o )
	{

	case TEMP:
	case NAME:
	case ICON:
	case VAUTO:
	case VPARAM:
		if( p->tn.rval == NONAME )
			p->in.name = (char *) 0;
		else if( p->tn.rval >= 0 )
		{
			 /* copy name from exname */
			register char *cp;
			cp = exname( stab[p->tn.rval].sname );
			p->in.name = tstr( cp );
		}
		else if( p->tn.rval == - strftn )
		{
			sprintf( temp, RNODNAME, -p->tn.rval );
			p->in.name = tstr( temp );
		}
		else
		{
			sprintf( temp, LABFMT, -p->tn.rval );
			p->in.name = tstr( temp );
		}
		break;

	case STARG:
	case STASG:
	case STCALL:
	case UNARY STCALL:
		/* set up size parameters */
		{
		    /* Get information from node that is still struct/union
		    ** type.  In some cases an STASG or (U)STCALL node gets
		    ** a different type when dereferenced, trashing the
		    ** struct/union size.
		    */
		    NODE * stinfo = p;

		    if (oldbtype != STRTY && oldbtype != UNIONTY)
			stinfo = p->in.left;
		    p->stn.stsize = tsize(STRTY,stinfo->fn.cdim,stinfo->fn.csiz);
		    p->stn.stalign = talign(STRTY,stinfo->fn.csiz);
		}
		break;

	/* leave FCON alone; clearing p->in.name zeroes the floating constant! */
	case FCON:
		break;

		/* this should do something only if temporary regs are
		/* built into the tree by machine-dependent actions */
	case REG:
		rbusy( p->tn.rval, p->in.type );
	default:
		p->in.name = (char *) 0;
	}

	if( ty != LTYPE ) p2tree( p->in.left );
	if( ty == BITYPE ) p2tree( p->in.right );
}

# endif


NODE *
treecpy(p)		/* first pass version of tcopy() */
	register NODE *p; 
{
	/* make a fresh copy of p */
	register NODE *q;

	q = talloc();
	*q = *p;
	switch (optype(q->in.op))
	{
	case BITYPE:
		q->in.right = treecpy(p->in.right);
	case UTYPE:
		q->in.left = treecpy(p->in.left);
	}
	return (q);
}

int					/* return 1 if type of node is a scalar */
isscalar( p )
NODE * p;
{
    switch( p->in.type) {
    case CHAR:	case UCHAR:
    case SHORT:	case USHORT:
    case INT:	case UNSIGNED:
    case LONG:	case ULONG:
    case FLOAT:	case DOUBLE:
    case ENUMTY:
	return( 1 );
    }
    return( ISPTR( p->in.type ));
}
