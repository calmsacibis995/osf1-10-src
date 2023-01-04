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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/10/14 18:33:23 $
#

PROGRAMS		= gawk

DATAFILES		= COPYING Makefile Makefile.generic README \
			  array.c awk.h awk.y awktab.c builtin.c \
			  config.h dfa.c dfa.h eval.c field.c \
			  gawk.1 getopt.c getopt.h io.c iop.c \
			  main.c msg.c node.c patchlevel.h protos.h \
			  re.c regex.c regex.h version.c

ILIST			= ${PROGRAMS} ${DATAFILES}
IDIR			= /usr/bin/

SOURCE_IDIR		= /usr/lib/gawk/
COPYING_IDIR		= ${SOURCE_IDIR}
Makefile_IDIR		= ${SOURCE_IDIR}
Makefile.generic_IDIR	= ${SOURCE_IDIR}
README_IDIR		= ${SOURCE_IDIR}
array.c_IDIR		= ${SOURCE_IDIR}
awk.h_IDIR		= ${SOURCE_IDIR}
awk.y_IDIR		= ${SOURCE_IDIR}
awktab.c_IDIR		= ${SOURCE_IDIR}
builtin.c_IDIR		= ${SOURCE_IDIR}
config.h_IDIR		= ${SOURCE_IDIR}
dfa.c_IDIR		= ${SOURCE_IDIR}
dfa.h_IDIR		= ${SOURCE_IDIR}
eval.c_IDIR		= ${SOURCE_IDIR}
field.c_IDIR		= ${SOURCE_IDIR}
gawk.1_IDIR		= ${SOURCE_IDIR}
getopt.c_IDIR		= ${SOURCE_IDIR}
getopt.h_IDIR		= ${SOURCE_IDIR}
io.c_IDIR		= ${SOURCE_IDIR}
iop.c_IDIR		= ${SOURCE_IDIR}
main.c_IDIR		= ${SOURCE_IDIR}
msg.c_IDIR		= ${SOURCE_IDIR}
node.c_IDIR		= ${SOURCE_IDIR}
patchlevel.h_IDIR	= ${SOURCE_IDIR}
protos.h_IDIR		= ${SOURCE_IDIR}
re.c_IDIR		= ${SOURCE_IDIR}
regex.c_IDIR		= ${SOURCE_IDIR}
regex.h_IDIR		= ${SOURCE_IDIR}
version.c_IDIR		= ${SOURCE_IDIR}

LIBS			= -lm
LINTFLAGS		= -hbax
INCFLAGS		= -I.
STD_FLAG		= -std0

HFILES			= awk.h config.h dfa.h getopt.h patchlevel.h \
			  protos.h regex.h
gawk_OFILES		= main.o eval.o builtin.o msg.o iop.o io.o field.o \
			  array.o node.o version.o re.o getopt.o \
			  awktab.o \
			  regex.o dfa.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/datafiles.mk
