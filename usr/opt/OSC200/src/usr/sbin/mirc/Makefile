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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/10/20 23:16:23 $
#
# /usr/sbin/mirc/Makefile
# Makefile for producing the mirc program and catalogue file

PROGRAMS	= mirc
OTHERS		= ntv_mirc

ILIST		= mirc
IDIR		= /usr/sbin/

CFLAGS		= -std1 -D_XOPEN_SOURCE -Olimit 1700
INCFLAGS        = -I. -I/usr/include -I${MAKETOP}../../export/${target_machine}/usr/include/eca
LIBS		= -lmir 
YFLAGS		= -vd

CLEANFILES	= *.o builtin_types.dat mir_yacc.h mir_yacc.c
GARBAGE		= y.output

HFILES		= ${MSGHDRS} mcc_vea_def.h

OFILES		= mir_backend.o mir_external.o mir_frontend.o mir_intermediate.o mir_internal.o \
		  mir_lex.o mir_remove.o mir_yacc.o

mir_yacc.c mir_yacc.h : mir_yacc.y
	$(YACC) $(YFLAGS) mir_yacc.y; mv y.tab.h mir_yacc.h; mv y.tab.c  mir_yacc.c

mir_lex.c : mir_lex.l mir_yacc.h
	lex mir_lex.l; mv lex.yy.c mir_lex.c        

ntv_mirc:	${OFILES}
	cc -o ntv_mirc -non_shared ${OFILES} -L -L/usr/lib -L/usr/ccs/lib -L${MAKETOP}../../export/${target_machine}/usr/ccs/lib ${LIBS}
include ${MAKEFILEPATH}/rules.mk
