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

#	Copyright 1988 Massachusetts Institute of Technology.
#
#	For copying and distribution information, see the file
#	"mit-copyright.h".
#
#

PROGRAMS	= compile_et

ILIST		= compile_et
IDIR		= /tools/${target_machine}_${target_os_type}/bin/

LIBS		= -ll

OPT_LEVEL	= -g
CFLAGS		= 
HFILES		= compiler.h
INCFLAGS	= -I../lib
OFILES		= error_table.o compile_et.o

error_table.o:	et_lex.lex.c error_table.y

et_lex.lex.c:	et_lex.lex.l

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

