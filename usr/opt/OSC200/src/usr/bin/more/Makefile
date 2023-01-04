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
# @(#)$RCSfile: Makefile,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 15:46:12 $
#
# 
# HISTORY
# 
# HISTORY
# $OSF_Log:	Makefile,v $
# Revision 1.1.1.4  93/01/07  08:44:00  devrcs
#  *** OSF1_1_2B07 version ***
# 
# Revision 1.10.2.2  1992/08/24  18:15:39  tom
# 	New more for POSIX.2/XPG4.
# 	[1992/08/24  17:29:57  tom]
#
# $OSF_EndLog$
#

PROGRAMS		= more
MSGHDRS			= more_msg.h
CATFILES		= more.cat
#DATAFILES		= more.help

ILIST			= ${PROGRAMS} 
IDIR			= /usr/bin/
LIB_IDIR		= /usr/share/lib/
more_ILINKS		= ${IDIR}page
#more.help_IDIR		= ${LIB_IDIR}

#help.o_CFLAGS		= -D_PATH_HELPFILE="\"${LIB_IDIR}more.help\""
HFILES			=  more_msg.h less.h

OFILES = ch.o command.o decode.o help.o input.o line.o linenum.o main.o \
	option.o os.o output.o position.o prim.o screen.o signal.o tags.o \
	ttyin.o
LIBS=	-lcurses

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
