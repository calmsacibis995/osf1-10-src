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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/04/20 14:47:04 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

PROGRAMS		= dprog style1 style2 style3
DATAFILES		= dict.d explain.d
MSGHDRS			= dprog_msg.h nwords_msg.h
CATFILES		= dprog.cat nwords.cat

ILIST			= ${PROGRAMS} ${DATAFILES}
IDIR			= /usr/lbin/

dict.d_IDIR		= /usr/share/lib/
explain.d_IDIR		= /usr/share/lib/

dprog.o_CFLAGS		= -DDICT=\"/usr/share/lib/dict.d\"
style1_OFILES		= nwords.o
style1_LIBS		= -lln
style2_OFILES		= end.o
style2_LIBS		= -lln
style3_OFILES		= part.o pscan.o outp.o
style3_LIBS		= -lln
GARBAGE			= end.c nwords.c part.c

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/datafiles.mk

dprog.o:	dprog_msg.h
end.o:		end.c names.h ehash.c edict.c
nwords.o:	nwords.c names.h nhash.c dict.c ydict.c abbrev.c nwords_msg.h
outp.o:		style.h names.h conp.h
part.o:		part.c style.h names.h conp.h
pscan.o:	names.h conp.h
