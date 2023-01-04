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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.3 $ (DEC) $Date: 1991/09/21 17:11:50 $

PROGRAMS 		= extract strextract strmerge trans

MSGHDRS			= extract_msg.h trans_msg.h
CATFILES		= extract.cat   trans.cat

ILIST			= ${PROGRAMS}
IDIR			= /usr/bin/
CCTYPE			= traditional
HFILES			= ${MSGHDRS} defs.h

extract_LIBS		= -lcurses
extract_OFILES		= \
			  extract.o \
			  re.o \
			  gen.o \
			  duplicate.o

strextract_OFILES	= \
			  strextract.o \
			  re.o \
			  gen.o \
			  duplicate.o

strmerge_OFILES		= \
			  strmerge.o \
			  re.o \
			  gen.o \
			  duplicate.o

trans_LIBS		= -lcurses
trans_OFILES		= \
			  trans.o \
			  getline.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

extract.o:		defs.h extract_msg.h
strextract.o:		defs.h extract_msg.h
strmerge.o:		defs.h extract_msg.h
re.o:			defs.h extract_msg.h
gen.o:			defs.h
duplicate.o:		defs.h

trans.o:		trans_msg.h
getline.o:		trans_msg.h
