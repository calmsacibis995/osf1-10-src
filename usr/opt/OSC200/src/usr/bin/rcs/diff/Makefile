#
# *********************************************************************
# *                                                                   *
# *       Modified by Digital Equipment Corporation, 1991, 1994       *
# *                                                                   *
# *       This file no longer matches the original Free Software      *
# *       Foundation file.                                            *
# *                                                                   *
# *********************************************************************
#
#
# HISTORY
#
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/10/14 18:33:31 $
#

PROGRAMS	= diff diff3

DATAFILES	= COPYING README ChangeLog Makefile Makefile.generic \
		  alloca.c analyze.c context.c diff.c diff.h diff3.c \
		  dir.c ed.c getopt.c getopt.h getopt1.c ifdef.c io.c \
		  normal.c regex.c regex.h util.c version.c

IDIR		= /usr/lbin/
ILIST		= ${PROGRAMS} ${DATAFILES}

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

INCFLAGS	= -I.
CENV		= -DUSG
STD_FLAG	= -std1

diff3.o_CFLAGS	= -DDIFF_PROGRAM=\"$(IDIR)diff\"

diff_OFILES	= diff.o analyze.o io.o context.o ed.o normal.o util.o dir.o \
	          regex.o ifdef.o version.o getopt.o getopt1.o

${diff_OFILES}: diff.h

context.o diff.o: regex.h

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/datafiles.mk
