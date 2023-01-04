#
# *****************************************************************
# *                                                               *
# *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
# *                                                               *
# *   All Rights Reserved.  Unpublished rights  reserved  under   *
# *   the copyright laws of the United States.                    *
# *                                                               *
# *   The software contained on this media  is  proprietary  to   *
# *   and  embodies  the  confidential  technology  of  Digital   *
# *   Equipment Corporation.  Possession, use,  duplication  or   *
# *   dissemination of the software and media is authorized only  *
# *   pursuant to a valid written license from Digital Equipment  *
# *   Corporation.                                                *
# *                                                               *
# *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
# *   by the U.S. Government is subject to restrictions  as  set  *
# *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
# *   or  in  FAR 52.227-19, as applicable.                       *
# *                                                               *
# *****************************************************************
#
#
# HISTORY
#
#
#	@(#)$RCSfile: machdep.mk,v $ $Revision: 1.1.10.7 $ (DEC) $Date: 1993/08/24 19:49:10 $	
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

${target_machine}UNENC_CFLAGS	=
${target_machine}UNENC_ASFLAGS	= -D__LANGUAGE_ASSEMBLY__

${target_machine}UNENC_COBJS	=

${target_machine}UNENC_SOBJS	= signal.o s5unlink.o s5open.o \
			sigaction.o waitpid.o libsys5init.o jmp.o \
			longjmp.o

#
# Create dependencies on libc/alpha include files.
#
jmp.s		: ../../libc/${target_machine}/_setjmp_incl.s
longjmp.s	: ../../libc/${target_machine}/_longjmp_incl.s \
		  ../../libc/${target_machine}/ljresume_incl.s

#${alphaSOBJS:.o=.S}: $${@:.S=.s}
#	${RM} -f $@
#	${CP} ${@:.S=.s} $@

${${target_machine}UNENC_SOBJS}: $${@:.o=.s}
	${_CC_} ${_CCFLAGS_} -g0 -O -c $*.s
#	${LD} -x -r $*.o
#	${MV} -f a.out $*.o
