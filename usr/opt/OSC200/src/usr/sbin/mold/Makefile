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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/08/05 22:32:23 $
#
# /usr/sbin/mold/Makefile
# Makefile for producing MOLD and i18n catalogue file

PROGRAMS		= mold

ILIST			= mold
IDIR			= /usr/sbin/

CFLAGS          	= -std1 -DXOPEN_SOURCE
INCFLAGS        	= -I. -I${MAKETOP}../../export/${target_machine}/usr/include/eca
LIBS			= -lmir -lmoss -lpthreads -lmach -lc_r -lbsd -lmld 

HFILES			= ${MSGHDRS} mold_msg_text.h  mold_private.h
OFILES			= mold.o mold_agent.o mold_mo.o mold_msg_text.o sck_mold_sstb.o

include ${MAKEFILEPATH}/rules.mk
