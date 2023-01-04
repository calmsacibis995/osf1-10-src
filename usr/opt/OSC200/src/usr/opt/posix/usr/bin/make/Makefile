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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/29 22:01:08 $
#
# 

VPATH			= lst.lib

PROGRAMS		= make
MSGHDRS			= pmake_msg.h
CATFILES		= pmake.cat
#USE_STATIC_LIBRARIES    =
#CFLAGS                  = -g
ILIST			= make
IDIR			= /usr/opt/posix/usr/bin

INCFLAGS		= -I. -Ilst.lib

HFILES			= lst.h alphaosf.h ${MSGHDRS}

IMPL_OBJS		= arch.o compat.o dir.o job.o main.o make.o\
			  parse.o str.o suff.o targ.o var.o

UTIL_OBJS		= buf.o hash.o

LIST_OBJS		= lstAppend.o lstAtEnd.o lstAtFront.o\
			  lstClose.o lstConcat.o lstDatum.o\
			  lstDeQueue.o lstDestroy.o\
			  lstFind.o lstFindFrom.o\
			  lstFirst.o lstForEach.o lstForEachFrom.o\
			  lstInit.o lstInsert.o \
			  lstIsEmpty.o lstLast.o lstMember.o lstNext.o\
			  lstOpen.o lstRemove.o lstReplace.o lstSucc.o

OFILES			= ${IMPL_OBJS} ${UTIL_OBJS} ${LIST_OBJS}

include ${MAKEFILEPATH}/rules.mk

${IMPL_OBJS}: make.h nonints.h

${LIST_OBJS}: lstInt.h

arch.o dir.o hash.o targ.o: hash.h

buf.o parse.o var.o: buf.h

job.o: job.h

main.o: pathnames.h

suff.o: bit.h
