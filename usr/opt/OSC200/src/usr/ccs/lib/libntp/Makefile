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
# 	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:39:25 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

LIBRARIES       = libntp.a

EXPLIB_TARGETS	= export_libntp.a

IDIR		= /usr/ccs/lib/

CFLAGS 		= -D_BSD -DLITTLE_ENDIAN

OFILES		= atoint.o atolfp.o atouint.o auth12crypt.o authdecrypt.o \
		  authdes.o authencrypt.o authkeys.o authparity.o \
		  authreadkeys.o authusekey.o buftvtots.o caljulian.o \
		  calleapwhen.o caltontp.o calyearstart.o clocktime.o \
		  dofptoa.o dolfptoa.o emalloc.o fptoa.o fptoms.o getopt.o \
		  gettstamp.o hextoint.o hextolfp.o humandate.o inttoa.o \
		  lfptoa.o lfptoms.o lib_strbuf.o mfptoa.o mfptoms.o \
		  modetoa.o mstolfp.o msutotsf.o ntoa.o ntohost.o numtoa.o \
		  numtohost.o octtoint.o prettydate.o ranny.o tsftomsu.o \
		  tstotv.o tvtoa.o tvtots.o ufptoa.o ufptoms.o uglydate.o \
		  uinttoa.o ulfptoa.o ulfptoms.o umfptoa.o umfptoms.o \
		  utvtoa.o


include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk
