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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:48:05 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

VPATH			= ..:../lvmcmdhdr

LIBRARIES		= lvmcmdlib.a
MSGHDRS			= lvm_msg.h

USE_STATIC_LIBRARIES	=

INCFLAGS		= -I../lvmcmdhdr
CFLAGS			= -UDEBUG
HFILES			= debug.h dump.h logvol.h lvmcmds.h lvmdefmsg.h \
			  lvmtab.h maps.h options.h user_interf.h utilities.h \
			  ${MSGHDRS}
OFILES			= options.o user_interf.o utilities.o debug.o\
			  lvmtab.o logvol.o dump.o maps.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk
