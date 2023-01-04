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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:57:27 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

PROGRAMS		= telnet
MSGHDRS			= telnet_msg.h
CATFILES		= telnet.cat

ILIST			= telnet
IDIR			= /usr/bin/

CCTYPE			= writable_strings
CFLAGS			= -D_BSD  -DSRCRT -DKLUDGELINEMODE

HFILES			= telnet_msg.h defines.h externs.h general.h ring.h
OFILES			= commands.o main.o network.o ring.o sys_bsd.o \
			  telnet.o terminal.o tn3270.o utilities.o
LIBS			= -ltermlib -lbsd

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

commands.o sys_bsd.o telnet.o terminal.o: types.h
network.o sys_bsd.o tn3270.o: fdset.h
