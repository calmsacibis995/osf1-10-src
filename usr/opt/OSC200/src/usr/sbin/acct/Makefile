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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.3.4 $ (DEC) $Date: 1992/05/05 16:54:10 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

#

#

SUBDIRS			= acctlib
SEC_PROGRAMS		= accton
PROGRAMS		= acctcms acctcom acctcon1 acctcon2 acctdisk \
			  acctdusg acctmerg accton acctprc1 acctprc2 \
			  acctwtmp diskusg fwtmp printpw wtmpfix
SCRIPTS			= chargefee ckpacct dodisk lastlogin monacct nulladm \
			  prctmp prdaily prtacct remove runacct shutacct \
			  startup turnacct .login .profile
MSGHDRS			= acct_msg.h
CATFILES		= acct.cat
DATAFILES		= holidays ptecms.awk ptelus.awk

SEC_ILIST		= ${SEC_PROGRAMS}
ILIST			= ${PROGRAMS} ${SCRIPTS} ${DATAFILES}
IDIR			= /usr/sbin/acct/
INSTDIRS		= acct acct\/nite acct\/sum acct\/fiscal
acct_IDIR		= /var/adm/
acct_IOWNER		= adm
acct_IGROUP		= adm
acct_IMODE		= 755
acct/nite_IDIR		= /var/adm/
acct/nite_IOWNER	= adm
acct/nite_IGROUP	= adm
acct/nite_IMODE		= 755
acct/sum_IDIR		= /var/adm/
acct/sum_IOWNER		= adm
acct/sum_IGROUP		= adm
acct/sum_IMODE		= 755
acct/fiscal_IDIR	= /var/adm/
acct/fiscal_IOWNER	= adm
acct/fiscal_IGROUP	= adm
acct/fiscal_IMODE	= 755
acctcom_IDIR		= /usr/bin/
accton_IOWNER		= root
accton_IGROUP		= adm
accton_IMODE		= 4750
.login_IDIR		= /var/adm/
.login_IOWNER		= adm
.login_IGROUP		= adm
.login_IMODE		= 640
.profile_IDIR		= /var/adm/
.profile_IOWNER		= adm
.profile_IGROUP		= adm
.profile_IMODE		= 640


accton.o_CFLAGS         = ${SEC_DEFS}
accton_LIBS             = ${SEC_LIBS}

HFILES			= ${MSGHDRS} acctdef.h ctmp.h ptmp.h table.h tacct.h

acctcms_LIBS		= acctlib/acctlib.a
acctcom_LIBS		= acctlib/acctlib.a
acctcon1_LIBS		= acctlib/acctlib.a
acctmerg_LIBS		= acctlib/acctlib.a
acctprc1_LIBS		= acctlib/acctlib.a
acctprc2_LIBS		= acctlib/acctlib.a

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/scripts.mk
include ${MAKEFILEPATH}/datafiles.mk
