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
# @(#)$RCSfile: Makefile,v $ $Revision: 4.2.4.4 $ (DEC) $Date: 1993/09/02 17:02:39 $
#
#
# HISTORY
#
# OSF/1 Release 1.0

SEC_PROGRAMS 		= crontab cron
PROGRAMS 		= crontab cron
MSGHDRS 		= cron_msg.h
CATFILES 		= cron.cat
DATAFILES		= .proto cron.deny log queuedefs adm root sys

SEC_ILIST		= ${SEC_PROGRAMS}
ILIST			= ${PROGRAMS} ${DATAFILES}
IDIR			= /usr/sbin/
CRONTAB_IDIR		= /usr/bin/
crontab_IDIR		= ${CRONTAB_IDIR}
crontab_IOWNER		= root
crontab_IMODE		= 4755

SPOOL_IDIR		= /var/spool/cron/crontabs/
ADM_IDIR		= /var/adm/cron/
SYMLINKS		= cron
SYMLINKDIR1		= ../../var/adm
SYMLINKDIR2		= /usr/lib

.proto_IDIR		= ${ADM_IDIR}
cron.deny_IDIR		= ${ADM_IDIR}
log_IDIR		= ${ADM_IDIR}
queuedefs_IDIR		= ${ADM_IDIR}
adm_IDIR		= ${SPOOL_IDIR}
root_IDIR		= ${SPOOL_IDIR}
sys_IDIR		= ${SPOOL_IDIR}

cron.o_CFLAGS 		= ${SEC_DEFS}
cron_sec.o_CFLAGS	= ${SEC_DEFS}
cronsub.o_CFLAGS 	= ${SEC_DEFS}
crontab.o_CFLAGS 	= ${SEC_DEFS}
crontab_sec.o_CFLAGS	= ${SEC_DEFS}
permit.o_CFLAGS		= ${SEC_DEFS}
HFILES			= cron_msg.h cron.h
crontab_OFILES          = crontab.o permit.o funcs.o cronsub.o \
                          ${SEC_DEFS?crontab_sec.o:}
cron_OFILES             = cron.o funcs.o elm.o ${SEC_DEFS?cron_sec.o:}
LIBS                    = ${SEC_LIBS} -lutil

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/datafiles.mk
