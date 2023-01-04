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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:57:12 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

#
# Copyright (c) 1990 SecureWare, Inc.  All rights reserved.
#


SEC_PROGRAMS	= aif
PROGRAMS	= ${SEC_PROGRAMS}

SEC_ILIST	= ${SEC_PROGRAMS}
ILIST		= ${SEC_ILIST}
IDIR		= /tcb/bin/
aif_ILINKS	= ${IDIR}sysadminif ${IDIR}issoif

OFILES		= aif_main.o aif_scrns.o bldstr.o curs_supp.o \
		curscr.o fillscr.o getscr.o headers.o \
		helpscr.o initscreen.o initst.o key_map.o \
		menuscr.o mprscr.o msgscr.o prscr.o \
		putscr.o scrnfuncs.o scrnsubs.o scrscr.o \
		traverse.o vars.o prntdsply.o



INCFLAGS        = -I../../common

CFLAGS		= ${SEC_DEFS} -DALIGN -DLOGGING 

LIBFLAGS        = -L../accounts -L../audit -L../devices -L../../common  

LIBS		= -laccounts -laudit -ldevices -lcommon  \
                  -lsecurity -lcurses -ltermlib

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

