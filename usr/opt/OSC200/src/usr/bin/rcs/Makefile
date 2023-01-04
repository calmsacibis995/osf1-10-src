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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/05/27 14:58:16 $
#

PROGRAMS		= ci co ident merge rcs rcsclean rcsdiff \
			  rcsmerge rlog rcsit sccstorcs

SCRIPTS			= rcsfreeze

SUBDIRS			= diff

DATAFILES		= COPYING maketime.c rcsedit.c rcsmap.c \
			  Makefile merge.c rcsfcmp.c rcsmerge.c \
			  Makefile.generic merger.c rcsfnms.c rcsrev.c \
			  README partime.c rcsfreeze.sh rcssyn.c \
			  ci.c rcs.c rcsgen.c rcstest.sh \
			  co.c rcsbase.h rcsit.c rcsutil.c \
			  conf.h rcsclean.c rcskeep.c rlog.c \
			  conf.sh rcsdiff.c rcskeys.c sccstorcs.c \
			  ident.c rcslex.c

ILIST			= ${PROGRAMS} ${SCRIPTS} ${DATAFILES}
IDIR			= /usr/bin/

SOURCE_IDIR		= /usr/lib/rcs/
COPYING_IDIR		= ${SOURCE_IDIR}
Makefile_IDIR		= ${SOURCE_IDIR}
Makefile.generic_IDIR	= ${SOURCE_IDIR}
README_IDIR		= ${SOURCE_IDIR}
ci.c_IDIR		= ${SOURCE_IDIR}
co.c_IDIR		= ${SOURCE_IDIR}
conf.h_IDIR		= ${SOURCE_IDIR}
conf.sh_IDIR		= ${SOURCE_IDIR}
ident.c_IDIR		= ${SOURCE_IDIR}
maketime.c_IDIR		= ${SOURCE_IDIR}
merge.c_IDIR		= ${SOURCE_IDIR}
merger.c_IDIR		= ${SOURCE_IDIR}
partime.c_IDIR		= ${SOURCE_IDIR}
rcs.c_IDIR		= ${SOURCE_IDIR}
rcsbase.h_IDIR		= ${SOURCE_IDIR}
rcsclean.c_IDIR		= ${SOURCE_IDIR}
rcsdiff.c_IDIR		= ${SOURCE_IDIR}
rcsedit.c_IDIR		= ${SOURCE_IDIR}
rcsfcmp.c_IDIR		= ${SOURCE_IDIR}
rcsfnms.c_IDIR		= ${SOURCE_IDIR}
rcsfreeze.sh_IDIR	= ${SOURCE_IDIR}
rcsgen.c_IDIR		= ${SOURCE_IDIR}
rcsit.c_IDIR		= ${SOURCE_IDIR}
rcskeep.c_IDIR		= ${SOURCE_IDIR}
rcskeys.c_IDIR		= ${SOURCE_IDIR}
rcslex.c_IDIR		= ${SOURCE_IDIR}
rcsmap.c_IDIR		= ${SOURCE_IDIR}
rcsmerge.c_IDIR		= ${SOURCE_IDIR}
rcsrev.c_IDIR		= ${SOURCE_IDIR}
rcssyn.c_IDIR		= ${SOURCE_IDIR}
rcstest.sh_IDIR		= ${SOURCE_IDIR}
rcsutil.c_IDIR		= ${SOURCE_IDIR}
rlog.c_IDIR		= ${SOURCE_IDIR}
sccstorcs.c_IDIR	= ${SOURCE_IDIR}

HFILES			= conf.h rcsbase.h
LINTFLAGS		= -hbax
INCFLAGS		= -I.
CFLAGS			= 
STD_FLAG		= -std1
COMMON_OFILES		= rcsfnms.o rcskeep.o rcskeys.o rcslex.o rcsmap.o \
			  rcsrev.o rcssyn.o rcsutil.o

ci_OFILES		= ci.o rcsgen.o rcsedit.o partime.o maketime.o \
			  rcsfcmp.o ${COMMON_OFILES}
co_OFILES		= co.o rcsgen.o rcsedit.o partime.o maketime.o \
			  ${COMMON_OFILES}
ident_OFILES		= ident.o rcsmap.o
merge_OFILES		= merge.o merger.o ${COMMON_OFILES}
rcs_OFILES		= rcs.o rcsgen.o rcsedit.o ${COMMON_OFILES}
rcsclean_OFILES		= rcsclean.o rcsedit.o rcsfcmp.o rcsgen.o \
			  ${COMMON_OFILES}
rcsdiff_OFILES		= rcsdiff.o maketime.o partime.o ${COMMON_OFILES}
rcsmerge_OFILES		= rcsmerge.o merger.o ${COMMON_OFILES}
rlog_OFILES		= rlog.o partime.o maketime.o ${COMMON_OFILES}
sccstorcs_OFILES	= sccstorcs.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/scripts.mk
include ${MAKEFILEPATH}/datafiles.mk
include ${MAKEFILEPATH}/subdirs.mk
