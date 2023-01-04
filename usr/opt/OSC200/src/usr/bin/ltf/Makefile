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
# @(#)$RCSfile: Makefile,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1993/08/03 10:26:22 $
#
#
# HISTORY
#
# OSF/1 Release 1.0



PROGRAMS		= ltf
MSGHDRS                 = ltf_msg.h

ILIST			= ltf
IDIR			= /usr/bin/

# Never define conditions ASIAN_I18N and KANJI at the same time
#   Define ASIAN_I18N is to build i18n-ltf
#   Define KANJI is to build j-ltf

CFLAGS			= -D_BSD -DNULL=0 
scantape.o_CFLAGS	= ${CFLAGS} -Olimit 700
HFILES			= ${MSGHDRS} filetypes.h ltfdefs.h ltferrs.h

OFILES			= \
			  filenames.o	\
			  filetype.o	\
			  initvol.o	\
			  ltf.o		\
			  ltfvars.o	\
			  mstrcmp.o	\
			  odm.o		\
			  putdir.o	\
			  putfile.o	\
			  scantape.o	\
			  statchk.o	\
			  xtractf.o	\
			  defmsgs.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
