#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file "pcrt1.s"
	.ident	"@(#)libc-m32:csu/pcrt1.s	1.3"
# C runtime startoff
# modified for System V - release 2 function-call floating-point

# exit() is system call #1, _exit == (call #)*(sizeof(gatevector))
	.set	_exit,1*8

# global entities defined in this file
	.globl	exit
	.globl	_start
	.globl	_istart
	.globl	_mcount
	.globl	environ
	.globl	CAproc

# global entities defined elsewhere, but used here
	.globl	__fpstart
	.globl	setchrclass
	.globl	main
	.globl	_cleanup
	.globl	_CAdump		

# special exit for profiling
# exit(code)
# code is return in r0 to system

exit:
	CALL	0(%sp),_CAdump
	CALL	0(%sp),_cleanup
	MOVW	&4,%r0
	MOVW	&_exit,%r1
	GATE

#
#	C language startup routine
#

_start:
	PUSHW	0(%ap)		# argc

	MOVAW	4(%ap),%r0
	PUSHW	%r0		# argv
	MOVW	(%r0),CAproc
.L1:
	TSTW	0(%r0)		# null args term ?
	je	.L2
	ADDW2	&4,%r0
	jmp	.L1
.L2:
#	MOVW	*8(%r0),CAproc
	MOVAW	4(%r0),%r0
	MOVW	%r0,environ	# indir is 0 if no env ; not 0 if env

	PUSHW	%r0		# envp

	CALL	0(%sp),_istart	# execute initialization code

	CALL	0(%sp),__fpstart	# set up floating-point state

	PUSHW	&0
	CALL	-4(%sp), setchrclass	#initialize _ctype array
	CALL	-3*4(%sp),main

	PUSHW	%r0
	CALL	-1*4(%sp),exit

	MOVW	&4,%r0
	MOVW	&_exit,%r1
	GATE
#
_mcount:			# dummy version for the case when
	rsb			# files have been compiled with -p but
				# not loaded with load module
	.data
	.align	4
CAproc:
	.word	0
environ:
	.word	0

	.section	.init,"x"
_istart:
	SAVE	%fp
