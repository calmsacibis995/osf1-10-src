#
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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.9.2 $ (DEC) $Date: 1993/07/29 15:26:13 $
#
#
PROGRAMS		= kdbx
SUBDIRS			= extensions
LDFLAGS			= -L.
OFILES			= krash.o array.o lex.yy.o callback_io.o string_io.o \
			  null_io.o file_io.o packet_io.o cleanup.o 
LIBS			= -ll -lmld
HFILES			= krash.h array.h util.h vdComResPkt.h \
			  krash_arrays.h krash_p.h
CFLAGS			= -DDEBUGGER -D_BSD
INCFLAGS		= -I../../../kernel \
			  -I../../../../obj/alpha/kernel/BINARY

ILIST			= kdbx 
IDIR			= /usr/bin/
OPT_LEVEL		= -g

GEN_ARRAYS		= ${MAKETOP}../../src/usr/bin/krash/gen_arrays

EXPORT_DIR		= /osf/usr/krash
EXPORT_EXT_DIR		= ${EXPORT_DIR}/extensions
EXPORT_BIN_DIR		= ${EXPORT_DIR}/bin
EXPORT_DOC_DIR		= ${EXPORT_DIR}/doc

cmd :
	${CMD}

lex.yy.c : kr.l
	lex $?

_myexport_ACTION_=myexport

myexport_all : __DO_myexport__PASS_BASIC__SUBDIR_extensions__
	cp ${MAKETOP}../../src/usr/bin/krash/krash.h ${EXPORT_EXT_DIR}
	cp ${MAKETOP}../../src/usr/bin/krash/vdComResPkt.h ${EXPORT_EXT_DIR}
	cp ${MAKETOP}../../src/usr/bin/krash/util.h ${EXPORT_EXT_DIR}
	cp ${MAKETOP}../../src/usr/bin/krash/krash.8 ${EXPORT_DOC_DIR}
	cp ${MAKETOP}../../src/usr/bin/krash/README ${EXPORT_DIR}
	ar cr libkrash.a array.o krashlib.o
	cp libkrash.a ${EXPORT_EXT_DIR}
	ranlib ${EXPORT_EXT_DIR}/libkrash.a
	cp kdbx ${EXPORT_BIN_DIR}
	cd ${EXPORT_DIR} ;\
	tar cvf krash.tar ./bin ./kdbx ./doc ./extensions ./README

#krash_arrays.h : krash.arrays
#	sh ${GEN_ARRAYS} < krash.arrays > krash_arrays.h

#krashlib_arrays.h : krashlib.arrays
#	sh ${GEN_ARRAYS} < krashlib.arrays > krashlib_arrays.h


include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/subdirs.mk
