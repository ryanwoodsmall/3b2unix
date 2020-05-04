/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/prf.c	10.3.1.1"
/*
 *	UNIX Operating System Profiler
 *
 *	Sorted Kernel text addresses are written into driver.  At each
 *	clock interrupt a binary search locates the counter for the
 *	interval containing the captured PC and increments it.
 *	The last counter is used to hold the User mode counts.
 */

#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/immu.h"
#include "sys/fs/s5dir.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/nvram.h"
#include "sys/user.h"
#include "sys/buf.h"

# define PRF_ON    1		/* profiler collecting samples */
# define PRF_VAL   2		/* profiler contains valid text symbols */
# define BPW	   4		/* bytes per word */
# define L2BPW	   2		/* log2(BPW) */

extern int maxprf;
extern unsigned prfctr[];	/* counters for symbols; last used for User */
extern unsigned prfsym[];	/* text symbols */
extern unsigned  prfstat;	/* state of profiler */
unsigned  prfmax;		/* number of loaded text symbols */

prfread()
{
	unsigned  min();

	if ((prfstat & PRF_VAL) == 0) {
		u.u_error = ENXIO;
		return;
	}
	iomove((caddr_t) prfsym, min(u.u_count, prfmax * BPW), B_READ);
	iomove((caddr_t) prfctr, min(u.u_count, (prfmax + 1) * BPW), B_READ);
}

prfwrite()
{
	register  unsigned  *ip;

	if (u.u_count > (maxprf * sizeof (int)))
		u.u_error = ENOSPC;
	else if (u.u_count & (BPW - 1) || u.u_count < 3 * BPW)
		u.u_error = E2BIG;
	else if (prfstat & PRF_ON)
		u.u_error = EBUSY;
	if (u.u_error)
		return;
	for (ip = prfctr; ip != &prfctr[maxprf + 1];)
		*ip++ = 0;
	prfmax = u.u_count >> L2BPW;
	iomove((caddr_t) prfsym, u.u_count, B_WRITE);
	for (ip = &prfsym[1]; ip != &prfsym[prfmax]; ip++)
		if (*ip < ip[-1]) {
			u.u_error = EINVAL;
			break;
		}
	if (u.u_error)
		prfstat = 0;
	else
		prfstat = PRF_VAL;
}

prfioctl(dev, cmd, arg, mode)
{
	switch (cmd) {
	case 1:
		u.u_r.r_reg.r_val1 = prfstat;
		break;
	case 2:
		u.u_r.r_reg.r_val1 = prfmax;
		break;
	case 3:
		if (prfstat & PRF_VAL) {
			prfstat = PRF_VAL | arg & PRF_ON;
			break;
		}
	default:
		u.u_error = EINVAL;
	}
}

prfintr(pc, ps)
	register  unsigned  pc;
	psw_t  ps;
{
	register  int  h, l, m;

	if (USERMODE(ps))
		prfctr[prfmax]++;
	else {
		l = 0;
		h = prfmax;
		while ((m = (l + h) / 2) != l)
			if (pc >= prfsym[m])
				l = m;
			else
				h = m;
		prfctr[m]++;
	}
}
