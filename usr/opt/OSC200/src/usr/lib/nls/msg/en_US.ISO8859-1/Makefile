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
#	@(#)$RCSfile: Makefile,v $ $Revision: 1.1.3.8 $ (DEC) $Date: 1993/11/29 16:29:05 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#
# Modification History
# ~~~~~~~~~~~~~~~~~~~~
# 001	David Lindner Mon Jun  3 16:00:02 EDT 1991
#	- Added ic.cat
#
# 002	Peter Derr	June 6, 1991
#	- Added cdacmds.cat
#
# 003	Webb Scales	1 July 1991
#	- Added dcethreads.cat
#

SEC_ILIST_C2		= \
 			  epa.cat auths.cat privs.cat \
			  secpolicy.cat 

SEC_ILIST_B1		= ${SEC_ILIST_C2} ismultdir.cat mkmultdir.cat \
			  rmmultdir.cat lpbanner.cat policy.cat \
			  setfstag.cat aclsyn.cat

SEC_ILIST		= ${SEC_ILIST_${SEC_LEVEL}}

ILIST			= Mail.cat ac.cat acct.cat  \
			  addgroup.cat adduser.cat admin.cat apply.cat ar.cat \
			  autopush.cat awk.cat nawk.cat banner.cat \
			  bc.cat bdiff.cat bfs.cat binmail.cat bsd.cat \
			  cal.cat calendar.cat capsar.cat cat.cat catman.cat \
			  cb.cat cdacmds.cat cfgmgr.cat cflow.cat checknr.cat \
			  chf.cat chgrp.cat chmod.cat chown.cat chroot.cat \
			  cm.cat cmp.cat co.cat col.cat colcrt.cat \
			  colrm.cat comb.cat comm.cat compress.cat \
			  comsat.cat cp.cat cpio.cat cron.cat csh.cat \
			  csplit.cat ctools.cat cut.cat \
			  cxref.cat date.cat dc.cat dcethd.cat dd.cat \
			  delta.cat deroff.cat df.cat diction.cat diff.cat diff3.cat \
			  diffmk.cat dircmp.cat dprog.cat du.cat \
			  dump.cat ed.cat edquota.cat \
			  env.cat eucset.cat ex.cat expand.cat \
			  explain.cat expr.cat fastboot.cat \
			  fasthalt.cat fdetach.cat find.cat \
			  finger.cat fingerd.cat flex.cat fold.cat from.cat \
			  ftp.cat ftpd.cat gated.cat genxlt.cat get.cat \
			  getconf.cat \
			  getopt.cat getty.cat grep.cat groups.cat \
			  grpck.cat halt.cat head.cat help.cat \
			  hostid.cat hostname.cat iconv.cat id.cat \
			  inetd.cat init.cat install.cat installbsd.cat \
			  iostat.cat ipcrm.cat ipcs.cat \
			  join.cat kill.cat killall.cat ksh.cat \
			  last.cat lastcomm.cat \
			  learn.cat leave.cat lex.cat libc.cat libcapsar.cat \
			  libcurses.cat libtli.cat \
			  link.cat lint.cat ln.cat locale.cat localedef.cat lock.cat \
			  login.cat logger.cat look.cat ls.cat lprsetup.cat \
			  ltf.cat lvm.cat m4.cat mailstats.cat make.cat pmake.cat\
			  man.cat mesg.cat mkdir.cat mkfifo.cat \
			  mklost+found.cat mknod.cat \
			  mkpasswd.cat mkproto.cat mkstr.cat more.cat \
			  msgfac.cat mt.cat mv.cat mvdir.cat named.cat \
			  newgrp.cat news.cat nice.cat nl.cat nm.cat \
			  nohup.cat nslookup.cat nwords.cat od.cat \
			  odump.cat pack.cat passwd.cat paste.cat \
			  patch.cat \
			  pathchk.cat pax.cat \
			  pcat.cat pg.cat ping.cat pr.cat \
			  printer.cat printf.cat prs.cat ps.cat pw.cat \
			  pwck.cat quot.cat quota.cat quotacheck.cat \
			  quotaon.cat rcp.cat rdate.cat rdist.cat reboot.cat \
			  removeuser.cat renice.cat repquota.cat \
			  restore.cat rev.cat rexecd.cat rlogin.cat \
			  rlogind.cat rm.cat rmail.cat rmchg.cat \
			  rmt.cat routed.cat \
			  rsh.cat rshd.cat ruptime.cat rwho.cat \
			  rwhod.cat sa.cat sccsdiff.cat sccshelp.cat \
			  script.cat sdiff.cat sed.cat \
			  sendmail.cat sh.cat \
			  shutdown.cat size.cat sleep.cat sort.cat \
			  spell.cat split.cat \
			  strace.cat strchg.cat strclean.cat \
			  strconf.cat strerr.cat strings.cat \
			  stty.cat style.cat su.cat sum.cat swapon.cat \
			  tabs.cat tail.cat talk.cat talkd.cat tar.cat \
			  tee.cat telnet.cat telnetd.cat termc.cat \
			  test.cat tftp.cat tftpd.cat tic.cat time.cat \
			  timed.cat tip.cat touch.cat tput.cat tr.cat \
			  trbsd.cat trpt.cat tset.cat tsort.cat tty.cat \
			  ul.cat unget.cat unifdef.cat uniq.cat \
			  unpack.cat users.cat uucp.cat uudecode.cat \
			  uuencode.cat uusend.cat vac.cat val.cat \
			  vipw.cat vmstat.cat w.cat wall.cat what.cat wc.cat \
			  whereis.cat which.cat who.cat whoami.cat \
			  whois.cat window.cat write.cat writesrv.cat \
			  xargs.cat xstr.cat yacc.cat \
			  authck.cat initcond.cat libsec.cat \
			  ${SEC_DEFS?${SEC_ILIST}:} \
			  extract.cat trans.cat
IDIR			= /usr/lib/nls/msg/en_US.ISO8859-1/
IMODE			= 644

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
