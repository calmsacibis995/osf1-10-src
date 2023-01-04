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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.10.3 $ (DEC) $Date: 1993/08/05 22:18:00 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

VPATH			= ${target_machine}

LIBRARIES		= libaio.a

EXPLIB_TARGETS		= export_libaio.a

ILIST			= libaio.a
IDIR			= /usr/ccs/lib/
IMODE			= 644

# HFILES			= 

# OTHERS			= libaio.a
# MSGHDRS			= libaio_msg.h
# CATFILES		= libaio.cat
# OBJECTS			=

# USE_STATIC_LIBRARIES	=

# GLINE			=
INCFLAGS		= -I${target_machine}
CFLAGS			= -DCOMPAT_43 -DLANGUAGE_C -DLIBAIO_SCCS \
                          -D_POSIX_4SOURCE -D_OSF_SOURCE -D_REENTRANT\
			  ${${target_machine}CFLAGS} \
			  -DAIO_THREADS_SWITCH
OFILES			= ${STDOBJS} \
			  ${${TARGET_MACHINE}SOBJS} \
			  ${${TARGET_MACHINE}COBJS}
GARBAGE			= *.S *.as tags

STDOBJS= aio.o aio_d10.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk
include ${target_machine}/machdep.mk
