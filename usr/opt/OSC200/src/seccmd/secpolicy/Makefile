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
# @(#)$RCSfile: Makefile,v $ $Revision: 4.2.11.2 $ (DEC) $Date: 1993/04/01 20:17:20 $
# 
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#
# Copyright (c) 1989 SecureWare, Inc.  All rights reserved.
#



SEC_ILIST	= ${ILIST}
ILIST		= ${PROGRAMS}
IDIR		= /var/tcb/bin/
SYMLINKS	= ${PROGRAMS}
SYMLINKDIR1	= ../../var/tcb/bin
SYMLINKDIR2	= /tcb/bin
SEC_IMODE	= 100
SEC_IGROUP	= sec

spdinit_IDIR	= /tcb/bin/
spdinit_SYMLINKDIR1=/tcb/bin
spdinit_SYMLINKDIR2=/var/tcb/bin

pacld_IDIR	=/tcb/bin/
pacld_SYMLINKDIR1=/tcb/bin
pacld_SYMLINKDIR2=/var/tcb/bin

USE_STATIC_LIBRARIES	=

PROGRAMS		= ${PROGRAMS_${SEC_LEVEL}}
SEC_PROGRAMS		= ${PROGRAMS}
PROGRAMS_BASE   	= 
PROGRAMS_ARCH		= spdinit mkdb dbck 
PROGRAMS_ACL		= dbck_acl
PROGRAMS_ACL_SWARE	= acld
PROGRAMS_ACL_POSIX	= pacld
PROGRAMS_MAC		= macd dbck_mac
PROGRAMS_ILB		= macilbd 

MSGHDRS		= secpolicy_msg.h
CATFILES	= secpolicy.cat
CFLAGS		= ${SEC_DEFS} -DDEBUG

macilbd_CFLAGS	= -DSEC_ILB ${CFLAGS}
HFILES		= ${MSGHDRS}
dbck_OFILES	= dbck.o spdbm.o
dbck_IMODE	= 2110
dbck_acl_IMODE	= 2110
acld_OFILES	= acld.o spdbm.o
pacld_OFILES	= pacld.o spdbm.o
macd_OFILES	= macilbd.o spdbm.o
macilbd_OFILES	= macilbd.o spdbm.o
LIBS		= -lsecurity

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

macd.o:	macilbd.c
	rm -f ./macd.c
	cp macilbd.c ./macd.c
	${_CC_} -c ${_CCFLAGS_} ./macd.c
