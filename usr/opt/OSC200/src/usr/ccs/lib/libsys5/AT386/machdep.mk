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
#	@(#)$RCSfile: machdep.mk,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:18:57 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

AT386CFLAGS =

AT386COBJS =

AT386SOBJS = setjmp.o

INCFLAGS = -I../libc/AT386

setjmp.o_CCTYPE = traditional

${AT386SOBJS:.o=.S}: $${@:.S=.s}
	${RM} -f $@
	${CP} ${@:.S=.s} $@

${AT386SOBJS}: $${@:.o=.S}
	${_CC_} ${_CCFLAGS_} -c $*.S
#	${LD} -x -r $*.o
#	${MV} -f a.out $*.o
