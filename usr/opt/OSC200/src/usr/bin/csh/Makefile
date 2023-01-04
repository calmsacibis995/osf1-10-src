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
# (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# @(#)$RCSfile: Makefile,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/11/10 23:12:03 $
#
#
# HISTORY
#

PROGRAMS		= csh
MSGHDRS 		= csh_msg.h
CATFILES 		= csh.cat

ILIST			= csh
IDIR			= /usr/bin/

#  for CMDEDIT - command line editing 
#  Not supported, but provided as a service for those who would
#  want to turn it on.
LIBS			= -lcurses -lretired_malloc

CFLAGS			= -D_BSD -DCMDEDIT
HFILES			= csh_msg.h sh.h local.h
OFILES 			= dir.o dol.o doprnt.o err.o exec.o \
			  exp.o file.o func.o glob.o hist.o init.o \
			  lex.o misc.o parse.o print.o proc.o sem.o \
			  set.o sh.o time.o tenex.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

dir.o proc.o: dir.h
func.o: pathnames.h
proc.o: proc.h
