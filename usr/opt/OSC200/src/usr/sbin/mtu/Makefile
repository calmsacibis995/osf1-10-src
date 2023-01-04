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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/10/20 23:16:29 $
#
# /usr/sbin/mtu
# Makefile for producing the MIB Translator Utility and I18n message catalogue
#
PROGRAMS	= mtu
OTHERS		= ntv_mtu
ILIST		= mtu
IDIR		= /usr/sbin/

CFLAGS		= -std1 -D_XOPEN_SOURCE 
INCFLAGS        = -I. -I${MAKETOP}../../export/${target_machine}/usr/include/eca
LIBS		= -lmoss
YFLAGS		= -vd

GARBAGE		= y.output
CLEANFILES	= *.o mtu_lex.c mtu_yacc.c mtu_yacc.h y.output

HFILES		= ${MSGHDRS} mtu.h mtu_msg.h

OFILES		= mtu.o mtu_entitytree.o mtu_getopt.o mtu_lex.o mtu_mibtree.o mtu_msfile.o mtu_msg.o mtu_yacc.o

mtu_yacc.c mtu_yacc.h : mtu_yacc.y
	$(YACC) $(YFLAGS) mtu_yacc.y; mv y.tab.h mtu_yacc.h; mv y.tab.c  mtu_yacc.c

mtu_lex.c : mtu_lex.l mtu_yacc.h
	lex mtu_lex.l; mv lex.yy.c mtu_lex.c

ntv_mtu :	$(OFILES)
	$(CC) -o ntv_mtu -non_shared ${OFILES} -L -L/usr/lib -L/usr/ccs/lib -L${MAKETOP}../../export/${target_machine}/usr/ccs/lib ${LIBS}

include ${MAKEFILEPATH}/rules.mk
