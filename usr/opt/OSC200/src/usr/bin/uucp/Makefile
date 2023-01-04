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
#
#       @(#)$RCSfile: Makefile,v $ $Revision: 4.2.20.10 $ (DEC) $Date: 1994/01/11 17:44:53 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0


SYMLINKS	= uucp cronuucp Devices Dialcodes Dialers Poll Maxuuscheds \
		Maxuuxqts Permissions Systems
cronuucp_SYMLINK	= uucp
cronuucp_SYMLINKDIR1	= .
cronuucp_SYMLINKDIR2	= /var/spool/cron/crontabs
Devices_SYMLINK		= Devices
Devices_SYMLINKDIR1	= ${ETCLDIR}
Devices_SYMLINKDIR2	= ${LIBIDIR}
Dialcodes_SYMLINK	= Dialcodes
Dialcodes_SYMLINKDIR1	= ${ETCLDIR}
Dialcodes_SYMLINKDIR2	= ${LIBIDIR}
Dialers_SYMLINK		= Dialers
Dialers_SYMLINKDIR1	= ${ETCLDIR}
Dialers_SYMLINKDIR2	= ${LIBIDIR}
Maxuuscheds_SYMLINK	= Maxuuscheds
Maxuuscheds_SYMLINKDIR1	= ${ETCLDIR}
Maxuuscheds_SYMLINKDIR2	= ${LIBIDIR}
Maxuuxqts_SYMLINK	= Maxuuxqts
Maxuuxqts_SYMLINKDIR1	= ${ETCLDIR}
Maxuuxqts_SYMLINKDIR2	= ${LIBIDIR}
Permissions_SYMLINK	= Permissions
Permissions_SYMLINKDIR1	= ${ETCLDIR}
Permissions_SYMLINKDIR2	= ${LIBIDIR}
Poll_SYMLINK		= Poll
Poll_SYMLINKDIR1	= ${ETCLDIR}
Poll_SYMLINKDIR2	= ${LIBIDIR}
Systems_SYMLINK		= Systems
Systems_SYMLINKDIR1	= ${ETCLDIR}
Systems_SYMLINKDIR2	= ${LIBIDIR}

SYMLINKDIR1	= ../lib
SYMLINKDIR2	= /usr/lbin

PROGRAMS		= uucheck uucico uucleanup uucp uucpd \
			  uuencode uudecode uugetty uuq uupoll uumonitor\
			  uuname uusend uustat uux uuxqt uusched 
MSGHDRS			= uucp_msg.h uuencode_msg.h uudecode_msg.h \
			  uusend_msg.h
CATFILES		= uucp.cat uuencode.cat uudecode.cat uusend.cat

USE_STATIC_LIBRARIES=

ILIST			= uucheck uucico uucleanup uucp uucpd \
			  uuencode uudecode uugetty uumonitor\
			  uuname uusend uustat uux uuxqt uusched \
		 	  Devices Dialcodes Dialers Maxuuscheds\
			  Maxuuxqts Permissions Poll Systems\
			  remote.unknown uudemon.admin uudemon.cleanu\
			  uudemon.hour uudemon.poll addinsys addoutsys\
			  addmodems uucpsetup uulog uupick uuto uutry\
			  uugettydefs cronuucp configpoll
DATAFILES		= Devices Dialcodes Dialers Maxuuscheds\
			  Maxuuxqts Permissions Poll Systems\
			  remote.unknown uudemon.admin uudemon.cleanu\
			  uudemon.hour uudemon.poll addinsys addoutsys\
			  addmodems uucpsetup uulog uupick uuto uutry\
			  uugettydefs cronuucp configpoll 
IDIR			= /usr/bin
SBINIDIR		= /usr/sbin
LIBIDIR			= /usr/lib/uucp
ETCIDIR			= /etc/uucp
ETCLDIR			= ../../../etc/uucp
SPOOLIDIR		= /var/spool
CRONDIR		        = /var/spool/cron/crontabs
cronuucp_IDIR		= ${CRONDIR}
IOWNER			= uucp
IGROUP			= uucp
HFILES			= uucp_msg.h uuencode_msg.h uudecode_msg.h \
			  uusend_msg.h
CCTYPE                  = writable_strings
uucpd_IMODE		= 500
uucpd_CFLAGS		= -D_BSD
uucpd.o_CFLAGS		= -D_BSD
LINTFLAGS		= -chbap
uucp_OFILES		= chremdir.o cpmv.o expfile.o gename.o getargs.o \
			  getpwinfo.o gnamef.o gtcfile.o gwd.o logent.o  \
			  permission.o systat.o ulockf.o utility.o uucp.o \
			  uucpdefs.o uucpname.o versys.o xqt.o
uucp_IMODE		= 6111
uux_OFILES		= chremdir.o  cpmv.o expfile.o gename.o getargs.o \
			  getprm.o getpwinfo.o gnamef.o gtcfile.o gwd.o \
			  logent.o permission.o systat.o  ulockf.o utility.o \
			  uucpdefs.o uucpname.o uux.o versys.o xqt.o
uux_IMODE		= 6111
uucpd_IDIR		= ${SBINIDIR}
uucpd_LIBS		= -lbsd
uuxqt_IDIR		= ${LIBIDIR}
uuxqt_IMODE		= 6111
uuxqt_OFILES		= chremdir.o cpmv.o expfile.o gename.o \
			  getargs.o getprm.o getpwinfo.o gnamef.o gtcfile.o \
			  logent.o mailst.o permission.o shio.o systat.o \
			  ulockf.o utility.o uucpdefs.o uucpname.o uuxqt.o \
			  versys.o xqt.o
uucico_IDIR		= ${LIBIDIR}
uucico_IMODE		= 6111
uucico_OFILES		= anlwrk.o callers.o chremdir.o cico.o fio.o cntrl.o \
			  conn.o cpmv.o eio.o expfile.o gename.o getargs.o \
			  getpwinfo.o gio.o gnamef.o gnxseq.o gtcfile.o \
			  imsg.o line.o logent.o mailst.o pk0.o pk1.o \
			  pkdefs.o permission.o sys5uux.o systat.o tio.o \
			  ttylock.o ulockf.o utility.o uucpdefs.o uucpname.o \
			  versys.o xio.o xqt.o
uuname_OFILES		= getpwinfo.o uucpdefs.o uucpname.o  uuname.o
uuname_IMODE		= 2111
uumonitor_IDIR          = ${LIBIDIR}
uumonitor_IMODE         = 4111
uumonitor_OFILES        = getargs.o gnamef.o uucpdefs.o uumon.o
#uumonitor_IDIR          = ${LIBIDIR}
#uumonitor_IMODE		= 4111
#uumonitor_OFILES        = getargs.o gnamef.o uucpdefs.o uumon.o
uustat_OFILES		= expfile.o getargs.o getpwinfo.o gnamef.o \
			  ulockf.o utility.o uucpdefs.o uucpname.o \
			  uustat.o versys.o
uustat_IMODE		= 6755
uuclean_OFILES		= cpmv.o expfile.o mailst.o getargs.o getpwinfo.o \
			  gnamef.o ulockf.o utility.o uuclean.o uucpdefs.o \
			  uucpname.o
uucleanup_IMODE		= 110
uucleanup_IDIR		= ${LIBIDIR}
uucleanup_OFILES	= cpmv.o expfile.o getargs.o getpwinfo.o gnamef.o \
			  ulockf.o utility.o uucleanup.o uucpdefs.o uucpname.o
uusched_IMODE		= 550
uusched_IDIR		= ${LIBIDIR}
uusched_OFILES		= expfile.o getargs.o getpwinfo.o gnamef.o \
			  systat.o ulockf.o utility.o uucpdefs.o uusched.o
uupoll_OFILES		= anlwrk.o cpmv.o expfile.o getpwinfo.o gnamef.o \
			  logent.o permission.o systat.o utility.o uucpname.o \
			  uucpdefs.o versys.o getargs.o uupoll.o
uucheck_IMODE		= 550
uucheck_IDIR		= ${LIBIDIR}
uucheck_OFILES		= uucheck.o uucpname.o
uugetty_IDIR		= ${LIBIDIR}
uugetty_OFILES		= uugetty.o dbg.o
uugettydefs_IDIR	= /etc/
uugettydefs_IMODE	= 755
uuq_OFILES		= expfile.o uuq.o permission.o getargs.o getpwinfo.o \
			  gnamef.o logent.o subdir.o systat.o utility.o \
			  uucpdefs.o uucpname.o
uusend_OFILES		= uusend.o anlwrk.o cpmv.o expfile.o gename.o \
	        	  getargs.o getprm.o getpwinfo.o gnamef.o gnxseq.o \
			  logent.o mailst.o systat.o ulockf.o uucpname.o \
			  versys.o xqt.o uucpdefs.o utility.o

Devices_IDIR		= ${ETCIDIR}
Devices_IMODE		= 650
Dialcodes_IDIR		= ${ETCIDIR}
Dialcodes_IMODE		= 650
Dialers_IDIR		= ${ETCIDIR}
Dialers_IMODE		= 650
Maxuuscheds_IDIR	= ${ETCIDIR}
Maxuuscheds_IMODE	= 650
Maxuuxqts_IDIR		= ${ETCIDIR}
Maxuuxqts_IMODE		= 650
Permissions_IDIR	= ${ETCIDIR}
Permissions_IMODE	= 650
Poll_IDIR		= ${ETCIDIR}
Poll_IMODE		= 650
Systems_IDIR		= ${ETCIDIR}
Systems_IMODE		= 650
remote.unknown_IDIR	= ${LIBIDIR}
remote.unknown_IMODE	= 750
uudemon.admin_IDIR	= ${LIBIDIR}
uudemon.admin_IMODE	= 750
uudemon.cleanu_IDIR	= ${LIBIDIR}
uudemon.cleanu_IMODE	= 750
uudemon.hour_IDIR	= ${LIBIDIR}
uudemon.hour_IMODE	= 750
uudemon.poll_IDIR	= ${LIBIDIR}
uudemon.poll_IMODE	= 750
uulog_IDIR		= ${LIBIDIR}
uulog_IMODE		= 755
uupick_IDIR		= ${LIBIDIR}
uupick_IMODE		= 755
uuto_IDIR		= ${LIBIDIR}
uuto_IMODE		= 755
uutry_IDIR		= ${LIBIDIR}
uutry_IMODE		= 755
cronuucp_IMODE		= 755
cronuucp_IOWNER		= root
cronuucp_IGROUP		= system
addinsys_IDIR		= ${LIBIDIR}
addinsys_IMODE		= 750
addoutsys_IDIR		= ${LIBIDIR}
addoutsys_IMODE		= 750
addmodems_IDIR		= ${LIBIDIR}
addmodems_IMODE		= 750
configpoll_IDIR		= ${LIBIDIR}
configpoll_IMODE	= 750
uucpsetup_IDIR		= ${SBINIDIR}

INSTDIRS		= uucppublic \
			  .Admin .Corrupt .Log \
			  .Old .Sequence .Status \
			  .Workspace .Xqtdir \
			  .Log\/uucico .Log\/uucp \
			  .Log\/uux .Log\/uuxqt 

uucppublic_IDIR		= ${SPOOLIDIR}/
uucppublic_IMODE	= 777
.Admin_IDIR		= ${SPOOLIDIR}/uucp/
.Admin_IMODE		= 775
.Corrupt_IDIR		= ${SPOOLIDIR}/uucp/
.Corrupt_IMODE		= 775
.Log_IDIR		= ${SPOOLIDIR}/uucp/
.Log_IMODE		= 775
.Log/uucico_IDIR	= ${SPOOLIDIR}/uucp/
.Log/uucico_IMODE	= 775
.Log/uucp_IDIR		= ${SPOOLIDIR}/uucp/
.Log/uucp_IMODE		= 775
.Log/uux_IDIR		= ${SPOOLIDIR}/uucp/
.Log/uux_IMODE		= 775
.Log/uuxqt_IDIR		= ${SPOOLIDIR}/uucp/
.Log/uuxqt_IMODE	= 775
.Old_IDIR		= ${SPOOLIDIR}/uucp/
.Old_IMODE		= 775
.Sequence_IDIR		= ${SPOOLIDIR}/uucp/
.Sequence_IMODE		= 775
.Status_IDIR		= ${SPOOLIDIR}/uucp/
.Status_IMODE		= 775
.Workspace_IDIR		= ${SPOOLIDIR}/uucp/
.Workspace_IMODE	= 775
.Xqtdir_IDIR		= ${SPOOLIDIR}/uucp/
.Xqtdir_IMODE		= 775

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/datafiles.mk

pk0.o: pk.h
pk1.o: pk.h
gio.o: pk.h

