/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libF77:z_exp.c	1.3"
#include "complex"

z_exp(r, z)
dcomplex *r, *z;
{
double expx;
double exp(), cos(), sin();

expx = exp(z->dreal);
r->dreal = expx * cos(z->dimag);
r->dimag = expx * sin(z->dimag);
}
