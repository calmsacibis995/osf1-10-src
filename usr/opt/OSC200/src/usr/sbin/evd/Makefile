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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:30:19 $
#
# /usr/sbin/evd/Makefile
# Makefile for producing and installing Common Agent EVD and catalogue file
# Issue:	i18n

PROGRAMS 	= evd
ILIST	 	= evd
IDIR	 	= /usr/sbin/

USE_STATIC_LIBRARIES=
CFLAGS		= -std1 -DXOPEN_SOURCE
INCFLAGS 	= -I. -I${MAKETOP}../../export/${target_machine}/usr/include/eca
LIBS		= -lmoss -lpthreads -lc_r -lc  -lbsd -lpthreads -lmld -lmach

CLEANFILES 	= *.o *.cat *.a 
HFILES		= ev.h evd.h 
OFILES		= evd_init.o  evd_main.o  evd_recv.o  evd_text.o sck_evd_sstb.o

include ${MAKEFILEPATH}/rules.mk
