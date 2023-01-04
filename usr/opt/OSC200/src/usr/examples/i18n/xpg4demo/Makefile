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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/25 22:02:17 $
#

DATAFILES		= Mkf.native \
			  README \
			  commands.c \
			  commands.h \
			  database.c \
			  message.c \
			  screen.c \
			  screen.h \
			  util.c \
			  xpg4demo.c \
			  xpg4demo.h \
			  xpg4demo.msg \
			  xpg4demo_en_US.msg \
			  xpg4demo_ja_JP.msg \
			  xpg4demo_ja_JP.SJIS.msg

INSTDIRS		= en_US ja_JP ja_JP.SJIS
en_US_IMODE		= 755
ja_JP_IMODE		= 755
ja_JP.SJIS_IMODE	= 755

ILIST			= ${DATAFILES}
IDIR			= /usr/examples/i18n/xpg4demo
IMODE			= 644

SYMLINKS		= Mkf.native
Mkf.native_SYMLINK	= Makefile
SYMLINKDIR1		= .
SYMLINKDIR2		= ${IDIR}

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
