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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/15 12:19:18 $
#
VPATH		= ${MAKETOP}/kernel/

PROGRAMS		= screend ckscreentab screentest screenmode \
			  screenstat screenmini

INCFLAGS		= -I${VPATH}

YFLAGS			= -vd

ILIST			= screend screenmode screenstat 
#
# These are test programs only, they do not ship
# ILIST                   = ckscreentab screentest screenmini

IDIR			= /usr/sbin/

CFLAGS			= -D_BSD
LIBS                    = -lc -ll
NO_SHARED_LIBRARIES	= 1
USE_STATIC_LIBRARIES	= 1
HFILES			= screentab.h

PARSEOBJS		= screentabl.o screentab.o nametoval.o \
			  buildtab.o printsubs.o acttab.o
SCREENOBJS 		= unpack.o screenit.o frag.o log.o
TESTOBJS 		= screentest.o printsubs.o nametoval.o

screend_OFILES		= ${PARSEOBJS} ${SCREENOBJS} screend.o 
ckscreentab_OFILES	= ${PARSEOBJS} ckscreentab.o
screentest_OFILES	= screentest.o printsubs.o nametoval.o
screenmode_OFILES	= screenmode.o
screenstat_OFILES       = screenstat.o
screenmini_OFILES       = screenmini.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

screentabl.o:	screentab.o
