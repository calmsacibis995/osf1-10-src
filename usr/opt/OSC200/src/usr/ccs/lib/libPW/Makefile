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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:41:51 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

LIBRARIES		= libPW.a
MSGHDRS			= pw_msg.h
CATFILES		= pw.cat

EXPLIB_TARGETS		= export_libPW.a

ILIST			= libPW.a
IDIR			= /usr/ccs/lib/
IMODE			= 644

OFILES = \
	abspath.o \
	any.o \
	anystr.o \
	bal.o \
	cat.o \
	clean.o \
	curdir.o \
	data.o \
	dname.o \
	fatal.o \
	fdfopen.o \
	giveup.o \
	imatch.o \
	lockit.o \
	logname.o \
	move.o \
	patoi.o \
	patol.o \
	regcmp.o \
	regex.o \
	repeat.o \
	repl.o \
	satoi.o \
	setsig.o \
	sname.o \
	strend.o \
	substr.o \
	trnslat.o \
	userdir.o \
	userexit.o \
	username.o \
	verify.o \
	xalloc.o \
	xcreat.o \
	xlink.o \
	xmsg.o \
	xopen.o \
	xpipe.o \
	xunlink.o \
	xwrite.o \
	zero.o \
	zeropad.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk

fatal.o regex.o xcreat.o xlink.o xmsg.o xopen.o: pw_msg.h
