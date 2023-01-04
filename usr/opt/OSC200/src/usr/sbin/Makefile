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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.3.18.21 $ (DEC) $Date: 1993/11/05 20:18:47 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#
#	001	30-may-1991	ccb
#		add rcmgr to SUBDIRS
#	002	31-may-1991     walker
#		added bootpd and traceroute to SUBDIRS
#	003	31-may-1991     bbrown
#		added bindsetup to SUBDIRS
#	004	03-june-1991	terry
#		added ypbind, ypserv, ypxfrd, rpc.yppasswdd, rpc.ypupdated,
#		svcsetup, ypsetup, and ypset for YP 
#	005   	05-june-1991 	terry
#		added xntpd, ntpdate, and ntpsetup for NTP
#	005   	05-june-1991 	Scott Cranston
#               added uerf
#
#	006     06-June-1991    Mike Ardehali
#		added lprsetup
#       007     06-June-1991    walker
#               added snmpd
#       008     06-June-1991    condylis
#               added automount
#	009	22-jun-1991	ccb
#		added log
#	010	24-jun-1991	miriam
#		added snmpsetup
#	011     24-jun-1991     walker
#		added netsetup and strsetup
#	012     25-jun-1991     Larry Cohen
#		added nfssetup
#	013     25-jun-1991     Diane Lebel
#		added rpcinfo
#	014     28-Jun-1991    	wca	
#		Added support to build strerr and strclean	
#	015	06-oct-1991	terry
#		removed rpc.ypupdated
#	016     06-Feb-1992     chc
#		added fddi_config
#

SEC_SUBDIRS		= acct arp chroot clri cron disklabel dcheck dumpfs \
			  edquota \
			  fastboot fasthalt fsdb getty halt hostid \
			  icheck ifconfig killall link lpd mknod \
			  mkpasswd mount ncheck newfs ping quot quota \
			  quotacheck quotaon reboot repquota \
			  renice route rshd shutdown swapon \
			  syslogd tunefs ufs_fsck umount vipw wall uerf \
			  lprsetup binlogd siainit secauthmigrate

INSTDIRS		= subsys
IDIR			= /usr/sbin/

#
# Note:  CA_MOMS depends on COMMON_AGENT tools (mtu, mirc, momgen)
#        mir_dat depends on *_mom
#
COMMON_AGENT		= mirc mold momgen mtu snmp_pe
CA_MOMS			= internet_mom fddi_mom trn_mom mir_dat

SUBDIRS			= ${SEC_SUBDIRS} \
			  ac addgroup adduser \
			  audgen auditd auditmask audit_tool audit_setup \
			  automount badsect bootpd \
			  bindsetup catman cddevsuppl comsat \
			  doconfig dump fingerd fsck fstat ftpd gated \
			  grpck inetd iprsetup kdebugd kgmon kopt log \
			  mailsetup mklost+found mkproto mountd \
			  mvdir named netsetup netstat nfsd \
			  pcnfsd nfsportmon nfssetup nfswatch fddi_config lmf \
			  nfsiod ntpdate ntpsetup pfconfig portmap \
			  prestosetup presto pwck \
			  rarpd rcinet rcmgr rdate removeuser restore rexecd \
			  ris rlogind rmt routed \
			  rpc.lockd rpc.statd rpc.yppasswdd rpcinfo \
			  rquotad rstatd rusersd rwalld \
			  rwhod s5fsck s5mkfs sa secsetup \
			  screend sendmail setld setrah settz setup \
			  sizer slattach snmpsetup \
			  sprayd srconfig strace \
			  strerr strclean strsetup svcsetup sync talkd \
			  tcpdump telnetd tftpd timed timezone traceroute trpt \
			  updadmin update xntpd ypbind ypserv ypset ypsetup \
			  ypxfrd isastrpty latcp autopush \
			  ${COMMON_AGENT} ${CA_MOMS}

# notyet - install pstat

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
