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
# (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# @(#)$RCSfile: Makefile,v $ $Revision: 4.2.8.3 $ (DEC) $Date: 1993/10/14 18:33:17 $
#
#
# HISTORY
#
# OSF/1 Release 1.0
#
PROGRAMS		= Mail fmt
DATAFILES		= Mail.rc
MSGHDRS			= Mail_msg.h
CATFILES		= Mail.cat

ILIST			= ${PROGRAMS} ${DATAFILES}
IDIR			= /usr/bin/
Mail_IGROUP		= mail
Mail_IMODE		= 775
Mail_ILINKS		= ${IDIR}mailx
Mail.rc_IDIR		= /usr/share/lib/

CFLAGS			= -D_BSD -DLOCKF -DASIAN_I18N -DLIBICONV
HFILES			= configdefs.h def.h glob.h \
			  local.h rcv.h sysexits.h ${MSGHDRS}
Mail_OFILES		= aux.o cmd2.o cmd3.o cmd4.o cmdtab.o collect.o \
			  config.o edit.o fio.o getname.o head.o lex.o list.o \
			  main.o names.o optim.o popen.o quit.o send.o \
			  strings.o temp.o tty.o v7.local.o vars.o cmd1.o \
			  version.o conv.o
fmt_OFILES		= fmt.o head.o
LIBS			= -lbsd -liconv
ANSIFLAG		= -std0

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/datafiles.mk
