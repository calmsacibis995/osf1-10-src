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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:10:56 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
# OSF/1 Release 1.0
#
# Copyright (c) 1990 SecureWare, Inc.  All rights reserved.



SEC_PROGRAMS	= XIsso
PROGRAMS	= ${SEC_PROGRAMS}

SEC_SUBDIRS	= accounts audit devices
SUBDIRS		= ${SEC_SUBDIRS}

SEC_ILIST	= ${SEC_PROGRAMS} ${SEC_DATAFILES}
ILIST		= ${PROGRAMS}
IDIR		= /tcb/bin/
isso.msg_IDIR   = /usr/share/lib/sechelp/Xisso/
isso.msg_ILINKS = /usr/share/lib/sechelp/aif/isso.msg

CFLAGS		= ${SEC_DEFS}

INCFLAGS	= -I../common

LIBFLAGS	= -L./audit -L./devices -L./accounts -L../common \
		  -L${MAKETOP}/usr/ccs/lib/libsecurity \
		  -L${MAKETOP}/usr/ccs/lib/libXm \
		  -L${MAKETOP}/usr/ccs/lib/libXt \
		  -L${MAKETOP}/usr/ccs/lib/libX11

LIBS		= -laudit -ldevices -laccounts -lcommon -lsecurity \
		  -lXm -lXt -lX11

SEC_DATAFILES 	= isso.msg
DATAFILES	= ${SEC_DATAFILES}


include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

isso.msg.sr:	../common/com.msg.sr main.msg.sr \
		../isso/accounts/accts.msg.sr \
		../isso/audit/audit.msg.sr \
		../isso/devices/dev.msg.sr
	cat ../common/com.msg.sr >isso.msg.sr
	cat main.msg.sr 	>>isso.msg.sr
	cat ../isso/accounts/accts.msg.sr >>isso.msg.sr
	cat ../isso/audit/audit.msg.sr >>isso.msg.sr
	cat ../isso/devices/dev.msg.sr >>isso.msg.sr

isso.msg:	isso.msg.sr
	../tools/msgprep isso.msg.sr isso.msg > /dev/null
