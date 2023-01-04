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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/03/15 20:49:14 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

PROGRAMS		= window
#DATAFILES		= windowrc
MSGHDRS			= window_msg.h
CATFILES		= window.cat
MCC_STD_FLAG		= -std0

ILIST			= ${PROGRAMS} ${DATAFILES}
IDIR			= /usr/bin/
#LIB_IDIR		= /usr/local/lib
#windowrc_IDIR		= ${LIB_IDIR}

CFLAGS			= -D_BSD
OFILES			= char.o cmd.o cmd1.o cmd2.o cmd3.o cmd4.o \
			  cmd5.o cmd6.o cmd7.o context.o error.o lcmd.o \
			  lcmd1.o lcmd2.o main.o mloop.o parser1.o \
			  parser2.o parser3.o parser4.o parser5.o \
			  scanner.o startup.o string.o ttf100.o \
			  ttgeneric.o tth19.o tth29.o ttinit.o \
			  ttoutput.o tttermcap.o tttvi925.o ttwyse60.o \
			  ttwyse75.o ttzentec.o var.o win.o wwadd.o \
			  wwalloc.o wwbox.o wwchild.o wwclose.o \
			  wwclreol.o wwclreos.o wwcursor.o wwdata.o \
			  wwdelchar.o wwdelete.o wwdelline.o wwdump.o \
			  wwend.o wwenviron.o wwerror.o wwflush.o \
			  wwframe.o wwgets.o wwinit.o wwinschar.o \
			  wwinsline.o wwiomux.o wwlabel.o wwmisc.o \
			  wwmove.o wwopen.o wwprintf.o wwpty.o wwputc.o \
			  wwputs.o wwredraw.o wwredrawwin.o wwrint.o \
			  wwscroll.o wwsize.o wwspawn.o wwsuspend.o \
			  wwtty.o wwunframe.o wwupdate.o wwwrite.o 
LIBS			= -lbsd -ltermlib 

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
#include ${MAKEFILEPATH}/datafiles.mk

char.o: char.h
cmd.o: defs.h ww.h window_msg.h
cmd1.o: defs.h ww.h window_msg.h
cmd2.o: defs.h ww.h window_msg.h
cmd3.o: defs.h ww.h
cmd4.o: defs.h ww.h
cmd5.o: defs.h ww.h window_msg.h
cmd6.o: defs.h ww.h string.h char.h window_msg.h
cmd7.o: defs.h ww.h window_msg.h
context.o: value.h string.h context.h
error.o: defs.h ww.h value.h context.h char.h window_msg.h
lcmd.o: defs.h ww.h value.h lcmd.h
lcmd1.o: defs.h ww.h string.h value.h lcmd.h var.h window_msg.h
lcmd2.o: defs.h ww.h string.h value.h var.h lcmd.h alias.h window_msg.h
main.o: defs.h ww.h string.h char.h local.h window_msg.h
mloop.o: defs.h ww.h window_msg.h
parser1.o: parser.h value.h context.h token.h string.h window_msg.h
parser2.o: parser.h value.h context.h token.h string.h var.h lcmd.h alias.h window_msg.h
parser3.o: parser.h value.h context.h token.h string.h window_msg.h
parser4.o: parser.h value.h context.h token.h string.h window_msg.h
parser5.o: parser.h value.h context.h token.h string.h var.h window_msg.h
scanner.o: value.h token.h context.h string.h
startup.o: defs.h ww.h value.h var.h char.h local.h window_msg.h
string.o: string.h
ttf100.o: ww.h tt.h
ttgeneric.o: ww.h tt.h
tth19.o: ww.h tt.h
tth29.o: ww.h tt.h
ttinit.o: ww.h tt.h
ttoutput.o: ww.h tt.h
tttermcap.o: tt.h
tttvi925.o: ww.h tt.h
var.o: value.h var.h string.h
win.o: defs.h ww.h window_msg.h
wwadd.o: ww.h
wwalloc.o: ww.h
wwbox.o: ww.h tt.h
wwchild.o: ww.h
wwclose.o: ww.h
wwclreol.o: ww.h tt.h
wwclreos.o: ww.h
wwcursor.o: ww.h
wwdata.o: ww.h
wwdelchar.o: ww.h tt.h
wwdelete.o: ww.h
wwdelline.o: ww.h
wwdump.o: ww.h tt.h
wwend.o: ww.h tt.h
wwenviron.o: ww.h
wwerror.o: ww.h window_msg.h
wwflush.o: ww.h tt.h
wwframe.o: ww.h tt.h
wwgets.o: ww.h char.h
wwinit.o: ww.h tt.h char.h
wwinschar.o: ww.h tt.h
wwinsline.o: ww.h
wwiomux.o: ww.h
wwlabel.o: ww.h char.h
wwmisc.o: ww.h tt.h char.h
wwmove.o: ww.h
wwopen.o: ww.h
wwprintf.o: ww.h
wwpty.o: ww.h
wwputc.o: ww.h
wwputs.o: ww.h
wwredraw.o: ww.h tt.h
wwredrawwin.o: ww.h
wwrint.o: ww.h
wwscroll.o: ww.h tt.h
wwsize.o: ww.h
wwspawn.o: ww.h
wwsuspend.o: ww.h tt.h
wwtty.o: ww.h
wwunframe.o: ww.h
wwupdate.o: ww.h tt.h
wwwrite.o: ww.h tt.h char.h
