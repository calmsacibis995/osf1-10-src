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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:48:41 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

VPATH			= lvmcmdhdr:${MAKETOP}/usr/sbin/newfs

SUBDIRS			= lvmcmdlib
PROGRAMS 		= lvchange lvcreate lvdisplay lvextend lvreduce \
			  lvremove lvsync pvchange pvcreate pvdisplay \
			  pvmove vgchange vgcreate vgdisplay vgextend \
			  vgreduce vgremove vgsync
MSGHDRS			= lvm_msg.h
CATFILES		= lvm.cat

ILIST			= ${PROGRAMS}
IDIR			= /usr/sbin/

USE_STATIC_LIBRARIES	=

INCFLAGS		= -Ilvmcmdhdr -I${MAKETOP}/usr/sbin/newfs
CFLAGS			=
HFILES			= debug.h dump.h logvol.h lvmcmds.h lvmdefmsg.h \
			  lvmtab.h maps.h options.h user_interf.h utilities.h \
			  ${MSGHDRS}
LIBS			= lvmcmdlib/lvmcmdlib.a

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
