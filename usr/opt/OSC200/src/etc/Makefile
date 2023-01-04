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
# HISTORY
#
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.3.13.23 $ (DEC) $Date: 1993/12/17 19:50:23 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#
#	001	10-oct-1991	ccb
#		add lib_admin.dir to SUBDIRS

SEC_SUBDIRS		= auth policy
UNSEC_SUBDIRS		= auth
INSTDIRS		= ${SEC_DEFS?${SEC_SUBDIRS}:${UNSEC_SUBDIRS}}

EXPLIB_SUBDIRS		= eca

SUBDIRS			= magic nls sia termcap namedb yp zoneinfo sec eca \
			  ${SEC_DEFS?${SEC_SUBDIRS}:${UNSEC_SUBDIRS}}

MRG_FILES		= disktab exports fstab gettydefs hosts.equiv \
			  inetd.conf protocols securettys services setup.conf

DATAFILES		= binlog.conf disktab exports gettydefs inittab \
			  inetd.conf  csh.login\
			  motd ntp.conf profile rc.config rpc services shells \
                          protocols syslog.conf svc.conf securettys TIMEZONE \
			  ${SEC_DEFS?:group passwd} \
			  svid2_path svid2_profile svid2_login \
			  svid3_path svid3_profile svid3_login \
			  hosts hosts.equiv networks fstab ftpusers \
			  ultrix_path ultrix_profile ultrix_login \
			  autopush.conf strsetup.conf ${MRG_FILES/*/.mrg..&} 

SEC_OTHERS		= ${SEC_DEFS?group passwd:}
OTHERS			= ${SEC_OTHERS}

SEC_ILIST		= ${SEC_OTHERS}
ILIST			= ${DATAFILES} ${OTHERS}
IDIR			= /etc/
SYMLINKS		= yp 
SYMLINKDIR1		= ../var
SYMLINKDIR2		= /etc

auth_IMODE		= 771
auth_IOWNER		= auth
auth_IGROUP		= auth
policy_IMODE		= 555

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
include ${MAKEFILEPATH}/datafiles.mk

${SEC_OTHERS}: $${@:=.${SEC_LEVEL}}
	${RM} ${_RMFLAGS_} $@
	${CP} ${@:=.${SEC_LEVEL}} $@
