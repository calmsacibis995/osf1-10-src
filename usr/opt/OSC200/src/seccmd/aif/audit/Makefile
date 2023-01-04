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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:54:46 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

#
# Copyright (c) 1990 SecureWare, Inc.  All rights reserved.
#


SEC_LIBRARIES   = libaudit.a
LIBRARIES       = ${SEC_LIBRARIES}

SFILES		= au_scrns.c aucpe.c aucpm.c ausl.c aumb.c aumr.c aumm.c \
		  aumd.c aurc.c aurs.c aurd.c aurr.c austats.c slabel.c 

OFILES		= au_scrns.o aucpe.o aucpm.o ausl.o aumb.o aumr.o aumm.o \
		  aumd.o aurc.o aurs.o aurd.o aurr.o austats.o slabel.o


INCFLAGS        = -I../screen -I../../common

CFLAGS		= ${SEC_DEFS} -DALIGN -DLOGGING

LIBS		= ${SEC_LIBS} 

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/scripts.mk
include ${MAKEFILEPATH}/objects.mk

