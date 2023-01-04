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
# @(#)$RCSfile: Makefile,v $ $Revision: 4.3.3.4 $ (DEC) $Date: 1992/10/14 14:26:36 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
USE_STATIC_LIBRARIES 	=
PROGRAMS		= lprsetup
DATAFILES		= lprsetup.dat
MSGHDRS			= lprsetup_msg.h
CATFILES		= lprsetup.cat


ILIST			= lprsetup lprsetup.dat
IDIR			= /usr/sbin/

lprsetup.dat_IDIR	= /etc/
lprsetup.dat_IOWNER	= root
lprsetup.dat_IMODE	= 444

INCFLAGS                = -I.
HFILES                  = ${MSGHDRS} lprsetup.h globals.h argstrings.h
OFILES			= lprsetup.o misc.o printlib.o safesyscalls.o\
			  argstrings.o
lprsetup.o_CFLAGS   	= -DLAT_MAJORDEV=5 ${CFLAGS}


include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/datafiles.mk
include ${MAKEFILEPATH}/objects.mk
