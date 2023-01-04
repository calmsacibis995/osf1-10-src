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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/14 18:33:09 $
#

VPATH		= ${MAKETOP}/usr/bin/rcs/diff

PROGRAMS	= diff diff3
OTHERS		= upd_diff upd_diff3

DATAFILES	= COPYING README ChangeLog Makefile Makefile.generic \
		  alloca.c analyze.c context.c diff.c diff.h diff3.c \
		  dir.c ed.c getopt.c getopt.h getopt1.c ifdef.c io.c \
		  normal.c regex.c regex.h util.c version.c

IDIR		= /usr/lbin/
ILIST		= ${OTHERS}

SOURCE_IDIR		= /usr/lib/rcs/diff/
COPYING_IDIR		= ${SOURCE_IDIR}
ChangeLog		= ${SOURCE_IDIR}
Makefile_IDIR		= ${SOURCE_IDIR}
Makefile.generic_IDIR	= ${SOURCE_IDIR}
README_IDIR		= ${SOURCE_IDIR}
alloca.c_IDIR		= $(SOURCE_IDIR)
analyze.c_IDIR		= $(SOURCE_IDIR)
context.c_IDIR		= $(SOURCE_IDIR)
diff.c_IDIR		= $(SOURCE_IDIR)
diff.h_IDIR		= $(SOURCE_IDIR)
diff3.c_IDIR		= $(SOURCE_IDIR)
dir.c_IDIR		= $(SOURCE_IDIR)
ed.c_IDIR		= $(SOURCE_IDIR)
getopt.c_IDIR		= $(SOURCE_IDIR)
getopt.h_IDIR		= $(SOURCE_IDIR)
getopt1.c_IDIR		= $(SOURCE_IDIR)
ifdef.c_IDIR		= $(SOURCE_IDIR)
io.c_IDIR		= $(SOURCE_IDIR)
normal.c_IDIR		= $(SOURCE_IDIR)
regex.c_IDIR		= $(SOURCE_IDIR)
regex.h_IDIR		= $(SOURCE_IDIR)
tags_IDIR		= $(SOURCE_IDIR)
util.c_IDIR		= $(SOURCE_IDIR)
version.c_IDIR		= $(SOURCE_IDIR)

USE_STATIC_LIBRARIES	=

INCFLAGS		= -I. -I./../../../usr/bin/rcs/diff
CENV			= -DUSG
STD_FLAG		= -std1

diff3.o_CFLAGS		= -DDIFF_PROGRAM=\"/usr/lbin/upd_diff\"

diff_OFILES		= diff.o analyze.o io.o context.o ed.o normal.o \
			  util.o dir.o regex.o ifdef.o version.o getopt.o \
			  getopt1.o

${diff_OFILES}: diff.h

context.o diff.o: regex.h

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/datafiles.mk
include ${MAKEFILEPATH}/others.mk

upd_%: %
	cp % $@
