#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

.ident	"@(#)libc-m32:sys/netunboot.s	1.2"

	.set	_netunboot,71*8

	.globl	netunboot
	.globl	_cerror

netunboot:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_netunboot,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	RET
