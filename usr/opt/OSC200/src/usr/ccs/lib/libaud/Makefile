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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.5.5 $ (DEC) $Date: 1993/08/02 19:58:26 $
#

LIBRARIES	= libaud.a

EXPLIB_TARGETS	= export_libaud.a

ILIST		= libaud.a
IDIR		= /usr/ccs/lib/
IMODE		= 644
syscalls.o_CFLAGS = -DCOMPAT_43 -DKERNEL
CFLAGS		= -DCOMPAT_43

OFILES		= aud_event.o aud_mask.o audgenl.o trustedevents.o syscalls.o
GARBAGE		= syscalls.c init_sysent.c
VPATH		= ${MAKETOP}/kernel/conf

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk

syscalls.c : makesyscalls.ksh syscalls.master
	@${RM} -f ${GARBAGE}
	ksh makesyscalls.ksh -k . syscalls.master
