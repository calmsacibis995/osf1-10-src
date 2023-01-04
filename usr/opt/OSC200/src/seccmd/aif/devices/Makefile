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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:55:46 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

#
# Copyright (c) 1990 SecureWare, Inc.  All rights reserved.
#


SEC_LIBRARIES   = libdevices.a
LIBRARIES       = ${SEC_LIBRARIES}

OFILES		= de_scrns.o decp.o desl.o dedevs.o \
		  desd.o desdh.o desdp.o desdr.o desdt.o \
		  deadp.o dedsp.o dermp.o demdp.o \
		  deadt.o dedst.o dermt.o demdt.o \
		  deadr.o dedsr.o dermr.o demdr.o \
		  delck.o deunlck.o deaus.o



INCFLAGS        = -I../screen -I../../common

CFLAGS		= ${SEC_DEFS} -DALIGN -DNO_SIGS


include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk

