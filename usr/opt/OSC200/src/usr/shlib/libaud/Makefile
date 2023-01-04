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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.6.4 $ (DEC) $Date: 1993/06/11 15:04:59 $
# 

SHARED_LIBRARIES	= libaud.so
OTHERS			= libaud.so

USE_SHARED_LIBRARIES	=
EXPSHLIB_TARGETS	= export_libaud.so

LIBS			= -lc
ILIST			= libaud.so
IDIR			= /usr/shlib/
IMODE			= 644
CFLAGS			= -DCOMPAT_43
OFILES			= aud_event.o audgenl.o aud_mask.o trustedevents.o syscalls.o
GARBAGE			= syscalls.c init_sysent.c
VPATH			= ${MAKETOP}/kernel/conf:${MAKETOP}/usr/ccs/lib/libaud

include ${MAKEFILEPATH}/rules.mk

syscalls.c : makesyscalls.ksh syscalls.master
	@${RM} -f ${GARBAGE}
	ksh makesyscalls.ksh -k . syscalls.master
