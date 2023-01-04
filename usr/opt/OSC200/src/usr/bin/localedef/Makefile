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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/11/19 23:27:09 $
#
PROGRAMS		= localedef
MSGHDRS			= localedef_msg.h
CATFILES		= localedef.cat

ILIST			= localedef
IDIR			= /usr/bin/

SEMFILES		= sem_chr.o sem_ctype.o sem_xlat.o sem_method.o  \
			  semstack.o sem_coll.o
OFILES			= err.o gen.o scan.o gram.o init.o localedef.o \
			  method.o symtab.o check.o ${SEMFILES} copy.o

HFILES			= ${MSGHDRS}
CFLAGS			= -D_LOCALEDEF -UADD_UNDERSCORE -D_NO_GCC_HACK

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

.y.c:
	${YACC} -vd $*.y
	/bin/mv -f y.tab.c $*.c
	/bin/mv -f y.tab.h $*.h

gram.h:		gram.c
localedef.o:	pathnames.h
scan.o:		gram.h symtab.h gram.o
gen.o:		semstack.h symtab.h  locdef.h
gram.o:		symtab.h semstack.h locdef.h
init.o:		symtab.h locdef.h err.h
check.o:	semstack.h symtab.h err.h
${SEMFILES}:	semstack.h symtab.h err.h
symtab.o:	symtab.h
copy.o: 	semstack.h err.h locdef.h
