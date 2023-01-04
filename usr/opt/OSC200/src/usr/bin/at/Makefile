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
# (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# @(#)$RCSfile: Makefile,v $ $Revision: 4.2.7.4 $ (DEC) $Date: 1993/06/10 14:29:38 $
#
#
# HISTORY
#
# OSF/1 Release 1.0

#
# We get cron.h, permit.c, funcs.c, cronsub.c from here
#
VPATH			= ${MAKETOP}/usr/sbin/cron

SEC_PROGRAMS		= at atq atrm
PROGRAMS		= at atq atrm 
SCRIPTS			= batch
DATAFILES		= at.deny
MSGHDRS 		= cron_msg.h
CATFILES 		= cron.cat

SEC_ILIST		= at atq atrm
ILIST			= at atq atrm batch at.deny
IDIR			= /usr/bin/
ADM_IDIR		= /var/adm/cron/
at_IMODE		= 4755
at_IOWNER		= root
at.deny_IDIR		= ${ADM_IDIR}

INCFLAGS		= -I${MAKETOP}/usr/sbin/cron
at.o_CFLAGS		= ${SEC_DEFS}
at_secure.o_CFLAGS	= ${SEC_DEFS}
atq.o_CFLAGS		= ${SEC_DEFS}
atrm.o_CFLAGS		= ${SEC_DEFS}
cronsub.o_CFLAGS	= ${SEC_DEFS} -DBUILDING_AT
YFLAGS			= -d
HFILES			= cron_msg.h
at_OFILES 		= at.o att1.o permit.o funcs.o cronsub.o \
			  ${SEC_DEFS?at_secure.o:}
atq_OFILES 		= atq.o cronsub.o funcs.o ${SEC_DEFS?at_secure.o:}
atrm_OFILES 		= atrm.o cronsub.o funcs.o ${SEC_DEFS?at_secure.o:}
LIBS			= ${SEC_LIBS}

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/datafiles.mk
include ${MAKEFILEPATH}/scripts.mk
include ${MAKEFILEPATH}/objects.mk

att1.h: att1.c
	-if [ -f y.tab.h ]; then ${MV} -f y.tab.h att1.h; fi

att1.o: att1.h
at.o atq.o:	cron.h
