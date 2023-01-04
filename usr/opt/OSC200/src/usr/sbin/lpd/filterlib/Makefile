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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/15 18:44:34 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

LIBRARIES	= libfilter.a
HFILES		= filetype.h
OFILES		= guesser.o fdtype_precomp.o file_magic.o fregex.o

#magic.o_OFILES	= file.o
#file.o_CFLAGS	= -DHAVE_MAGIC

#MTABINIT_OBJS =	file.o mtabinit.o fregex.o filetype.o fdtype_nocomp.o
#MAGIC_OBJS =	magic.o mtabinit.o fregex.o filetype.o \
#		file_magic.o fdtype_precomp.o 


#fdtype_precomp.o_OFILES	= fdtype.o
fdtype.o_CFLAGS	= -DHAVE_MAGIC

#fdtype_precomp.o_CFLAGS	= -DHAVE_MAGIC
#fdtype_nocomp.o_CFLAGS  = 

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/datafiles.mk


fdtype_precomp.o: fdtype.o
	${CP} fdtype.o fdtype_precomp.o


#
# test program for new file guesser
#guesser: guesser.c fdtype_precomp.o fregex.o file_magic.o
#       $(LD) $(CFLAGS) -g -DTESTING guesser.c fdtype_precomp.o fregex.o \
#       file_magic.o
#
#magic:	${MAGIC_OBJS}
#	${_CC_} ${_LDFLAGS_} -o magic ${MAGIC_OBJS} ${_LIBS_} 
#
#mtabinit: ${MTABINIT_OBJS}
#	${_CC_} ${_LDFLAGS_} -o mtabinit ${MTABINIT_OBJS} ${_LIBS_}
#
#file_magic.c: mtabinit magic.print
#	mtabinit -im magic.print > $@ || rm $@
#
#magic.o: file.c
#	@rm -f file.o
#	${CC} -DHAVE_MAGIC ../file.c
#	mv file.o $@
