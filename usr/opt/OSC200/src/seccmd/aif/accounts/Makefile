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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:52:12 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

#
# Copyright (c) 1990 SecureWare, Inc.  All rights reserved.
#


SEC_LIBRARIES   = libaccounts.a
LIBRARIES       = ${SEC_LIBRARIES}

OFILES		= ac_scrns.o acag.o acau.o accue.o \
		acauca.o acaulc.o acaupp.o \
		acduca.o acdulc.o acdupp.o \
		acmupp.o \
		acdca.o acdcl.o acdka.o acucl.o acdlc.o acdpp.o \
		acdg.o acdu.o aclu.o \
		acmu.o acmg.o acru.o acsg.o acsge.o \
		acsu.o acsue.o acsutils.o acutils.o acuu.o \
		agutils.o acauae.o acauka.o

INCFLAGS        = -I../../common -I../screen

CFLAGS		= ${SEC_DEFS} -DALIGN -DLOGGING -DNO_SIGS

LIBS		= ${SEC_LIBS} 

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk

