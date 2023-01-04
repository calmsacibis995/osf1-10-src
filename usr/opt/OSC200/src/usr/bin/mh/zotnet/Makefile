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
# @(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:40:30 $  
#
Options = -DSYS5DIR -D_BSD -DISI -DMHRC -DWHATNOW -DMHE -DBSD42 -DBERK \
          -DBSD43 -DLOCKF -DNETWORK -DBIND -DRPATHS -DDPOP -DRPOP -DUK \
          -DFOLDPROT='"0700"' -DMSGPROT='"0600"' -DTYPESIG='void' \
          -DPOPSERVICE='"pop"' -DSPRINTF='int' -DSENDMTS -DSMTP -DPOP -DX400
CFLAGS = ${CFLAGS} ${Options}
LIBRARIES = libzot.a
OFILES = mts.o lock.o client.o mf.o \
	 getbbent.o dtime.o dtimep.o lexstring.o
DATAFILES = mtstailor

IDIR = /usr/lib/mh/
IMODE = 644
ILIST = ${DATAFILES}

LEX = lex -nt
# PROGRAMS = dtimep dtime date
GARBAGE	= dtimep.c

lexstring.o_CFLAGS = ${CFLAGS} ${OPTIONS} -DONECASE
dtimep_OFILES = dtimep.o lexstring.o
dtime_OFILES = dtime.o
date_OFILES = date.o dtime.o

dtimep.c:	dtimep.lex lexedit.sed
		$(LEX) dtimep.lex | $(SED) -f lexedit.sed > dtimep.c

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/datafiles.mk
