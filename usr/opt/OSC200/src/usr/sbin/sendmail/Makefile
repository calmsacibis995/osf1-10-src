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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/12/14 23:03:17 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

PROGRAMS		= sendmail mailstats
DATAFILES		= sendmail.cf sendmail.hf aliases sendmail.st
MSGHDRS			= sendmail_msg.h mailstats_msg.h
CATFILES		= sendmail.cat mailstats.cat

ILIST			= ${PROGRAMS} ${DATAFILES}
IDIR			= /usr/sbin/
ADM_IDIR		= /var/adm/sendmail/
sendmail.cf_IDIR	= ${ADM_IDIR}
sendmail.hf_IDIR	= /usr/share/lib/
aliases_IDIR		= ${ADM_IDIR}
sendmail.st_IDIR	= ${ADM_IDIR}
sendmail_IOWNER		= root
sendmail_IMODE		= 4755
sendmail_ILINKS		= ${IDIR}mailq ${IDIR}newaliases ${IDIR}smtpd
sendmail.cf_IOWNER	= root
sendmail.cf_IMODE	= 644
aliases_IOWNER		= root
aliases_IMODE		= 644
sendmail.st_IOWNER	= root
sendmail.st_IMODE	= 644
SYMLINKS		= sendmail
SYMLINKDIR1		= ../sbin
SYMLINKDIR2		= /usr/lib

CCTYPE			= writable_strings
USE_STATIC_LIBRARIES	=
INCFLAGS		= -I.
CFLAGS			= -D_BSD -DNOCATMACRO -DVMUNIX

HFILES			= conf.h mailstats.h pathnames.h sendmail.h \
			  useful.h ${MSGHDRS}
sendmail_OFILES		= alias.o arpadate.o clock.o collect.o conf.o \
			  convtime.o daemon.o datecompiled.o \
			  deliver.o domain.o envelope.o \
			  err.o headers.o macro.o main.o parseaddr.o \
			  queue.o readcf.o recipient.o savemail.o \
			  srvrsmtp.o stab.o stats.o sysexits.o trace.o \
			  usersmtp.o util.o version.o

sendmail_LIBS		= -lbsd -ldbm -lutil

datecompiled.c:
		echo 'char datecompiled[] = "' `date` '";' > datecompiled.c

sendmail.st:
		cp /dev/null sendmail.st

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/datafiles.mk
