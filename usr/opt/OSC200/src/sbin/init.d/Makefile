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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.3.17.9 $ (DEC) $Date: 1993/11/03 01:31:39 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

DATAFILES		= cron motd paging sendmail uucp enlogin \
			  kloadsrv kmod named xntpd nis settime \
                          preserve xdm inet nfs recpasswd common_agent \
                          inetd nfsmount rmtmpfiles syslog lpd savecore \
                          sia acct crashdc startlmf gateway route rwho lat \
			  quota streams presto audit write

SCRIPTS			= security

ILIST			= ${DATAFILES} ${SCRIPTS}
IDIR			= /sbin/init.d/

IOWNER                  = bin
IGROUP                  = bin
IMODE                   = 750

INSTDIRS		= rc0.d rc2.d rc3.d
rc0.d_IDIR		= /sbin/
rc2.d_IDIR		= /sbin/
rc3.d_IDIR		= /sbin/

SYMLINKS 		= ${DATAFILES} ${SCRIPTS}
SYMLINKDIR1		= ../init.d
SYMLINKDIR2		= /sbin/rc0.d	/sbin/rc2.d		/sbin/rc3.d

enlogin_SYMLINK		= K00enlogin	S25enlogin		nolink
lpd_SYMLINK		= K05lpd	K00lpd			S65lpd
lat_SYMLINK		= K07lat	K03lat			S58lat
inetd_SYMLINK		= K10inetd	K05inetd		S55inetd
common_agent_SYMLINK	= K15common_agent K10common_agent	S50common_agent
xntpd_SYMLINK		= K20xntpd	K15xntpd		S45xntpd
sendmail_SYMLINK	= K25sendmail	K30sendmail		S40sendmail
nfs_SYMLINK		= K30nfs	K35nfs			S19nfs
nfsmount_SYMLINK	= K35nfsmount	K40nfsmount		S20nfsmount
presto_SYMLINK		= K31presto	K36presto		S36presto
nis_SYMLINK		= K38nis	K43nis			S18nis
xdm_SYMLINK		= K19xdm	nolink			S95xdm
named_SYMLINK		= K40named	K45named		S15named
syslog_SYMLINK		= K45syslog	K50syslog		S10syslog
gateway_SYMLINK		= K44gateway	K49gateway		S11gateway
route_SYMLINK		= K43route	K48route		S12route
rwho_SYMLINK		= K42rwho	K47rwho			S13rwho
write_SYMLINK		= K47write	K52write		S63write
inet_SYMLINK		= K50inet	K55inet			S00inet
cron_SYMLINK		= K22cron	K20cron			S57cron
quota_SYMLINK		= K52quota	K57quota		S01quota
savecore_SYMLINK	= nolink	S00savecore		nolink
paging_SYMLINK		= nolink	S05paging		nolink
recpasswd_SYMLINK	= nolink	S10recpasswd		nolink
uucp_SYMLINKDIR2	= /sbin/rc0.d	/sbin/rc2.d /sbin/rc2.d	/sbin/rc3.d
uucp_SYMLINK            = K46uucp       S15uucp     K51uucp     S04uucp
motd_SYMLINK		= nolink	nolink			S60motd
preserve_SYMLINK	= nolink	nolink			S25preserve
rmtmpfiles_SYMLINK	= nolink	nolink			S30rmtmpfiles
kloadsrv_SYMLINK	= nolink	nolink			S23kloadsrv
kmod_SYMLINK		= nolink	nolink			nolink
settime_SYMLINK		= nolink	nolink			S05settime
acct_SYMLINK		= nolink	nolink			S75acct
crashdc_SYMLINK         = nolink        nolink                  S80crashdc
startlmf_SYMLINK	= nolink	nolink			S08startlmf
streams_SYMLINK		= nolink	S35streams		nolink
security_SYMLINK	= nolink	nolink			S26security
sia_SYMLINK		= nolink	nolink			S27sia
audit_SYMLINK		= K08audit	nolink			S21audit

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
include ${MAKEFILEPATH}/scripts.mk
