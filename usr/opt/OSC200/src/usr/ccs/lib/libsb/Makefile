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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.3 $ (DEC) $Date: 1991/11/26 11:43:37 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

LIBRARIES		= libsb.a

EXPLIB_TARGETS		= export_libsb.a
EXPDIR			= /usr/ccs/lib/

CFLAGS			= -D_BSD
run.o_CFLAGS		= ${CFLAGS} -D${${TARGET_MACHINE}_VA_ARGV}
run_p.o_CFLAGS		= ${CFLAGS} -D${${TARGET_MACHINE}_VA_ARGV}
OFILES			= mklinks.o par_cmd_line.o par_rc_file.o \
			  print_usage.o rc_file_ext.o runcmd.o uquit.o \
			  abspath.o concat.o editor.o errmsg.o fdate.o \
			  ffilecopy.o filecopy.o getbool.o getstab.o \
			  getstr.o makepath.o nxtarg.o path.o quit.o \
			  run.o salloc.o searchp.o sindex.o stablk.o \
			  fold.o skipto.o stlmatch.o prstab.o \
			  ${NEED_FOR_PORTING}

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk
