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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:10:54 $
# 
# @OSF_COPYRIGHT@
# 
# HISTORY
# $OSF_Log:	Makefile,v $
# Revision 1.1.1.2  92/06/23  01:44:00  devrcs
#  *** OSF1_1B30 version ***
# 
# Revision 1.1.2.4  1992/04/21  13:32:47  marquard
# 	Use ../tools/msgprep now, since we know it will be built before we need it.
# 	[1992/04/20  20:49:10  marquard]
#
# Revision 1.1.2.3  1992/04/05  18:19:10  marquard
# 	paclif POSIX ACL interface program.
# 	[1992/04/05  12:00:39  marquard]
# 
# $OSF_EndLog$
#
# (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
# OSF/1 Release 1.0.1
#
# Copyright (c) 1990 SecureWare, Inc.  All rights reserved.
#
# @(#)Makefile	1.1 11:15:21 11/8/91 SecureWare
#
# Based on OSF Version: 
# 	@(#)Makefile	1.11 16:30:38 5/17/91 SecureWare

SEC_PROGRAMS	= paclif
PROGRAMS	= ${SEC_PROGRAMS}

SEC_ILIST	= ${SEC_PROGRAMS} paclif.msg
ILIST		= ${SEC_ILIST}
IDIR		= /tcb/bin/
paclif.msg_IDIR = /usr/share/lib/sechelp/paclif/

OFILES		= pacl_main.o pacl_scrns.o bldstr.o curs_supp.o \
		curscr.o fillscr.o getscr.o headers.o \
		helpscr.o initscreen.o initst.o key_map.o \
		menuscr.o msgscr.o prscr.o \
		putscr.o scrnfuncs.o scrnsubs.o scrscr.o \
		traverse.o vars.o prntdsply.o signals.o slabel.o Errors.o \
		pacl_callbacks.o utils.o acc_callbacks.o acc_scrns.o

INCFLAGS        = -I../../common

CFLAGS		= ${SEC_DEFS} 

LIBFLAGS        = -L../../common  -L../../../lib/libsecurity

LIBS		=  -lcommon -lsecurity -lcurses -ltermlib

SEC_DATAFILES 	= paclif.msg

DATAFILES	= ${SEC_DATAFILES}

include ${MAKEFILEPATH}/rules.mk

paclif:		${OFILES} ../../common/libcommon.a

paclif.msg:	paclif.msg.sr
		../tools/msgprep $(.ALLSRC) paclif.msg > /dev/null

