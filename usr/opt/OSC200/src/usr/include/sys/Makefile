#
#	*****************************************************************
#	*                                                               *
#	*    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
#	*                                                               *
#	*   All Rights Reserved.  Unpublished rights  reserved  under   *
#	*   the copyright laws of the United States.                    *
#	*                                                               *
#	*   The software contained on this media  is  proprietary  to   *
#	*   and  embodies  the  confidential  technology  of  Digital   *
#	*   Equipment Corporation.  Possession, use,  duplication  or   *
#	*   dissemination of the software and media is authorized only  *
#	*   pursuant to a valid written license from Digital Equipment  *
#	*   Corporation.                                                *
#	*                                                               *
#	*   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
#	*   by the U.S. Government is subject to restrictions  as  set  *
#	*   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
#	*   or  in  FAR 52.227-19, as applicable.                       *
#	*                                                               *
#	*****************************************************************
#
# HISTORY
#
#
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.4.17.19 $ (DEC) $Date: 1993/11/19 19:14:43 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

VPATH			= ${MAKETOP}/kernel/sys:${MAKETOP}/kernel/conf:${MAKETOP}/kernel/io/common

DATAFILES		= NLchar.h access.h acct.h addrconf.h \
			  audit.h auxv.h \
			  bkmac.h buf.h \
			  callout.h clist.h cmap.h cmupty.h \
			  conf.h core.h disklabel.h dk.h \
			  dkbad.h domain.h \
			  errno.h \
			  exec.h fault.h fcntl.h file.h filsys.h \
			  fs_types.h flock.h fpsconf.h fpsreg.h fullstat.h \
			  gprof.h habitat.h id.h  inputdevice.h \
			  ioctl.h ioctl_compat.h \
			  ipc.h jctype0.h jctype1.h jioctl.h kernel.h \
			  kloadcall.h lc_core.h ldr_exec.h limits.h \
			  lmf.h lmfklic.h localedef.h \
			  lock.h lock_types.h lpio.h malloc.h map.h mbuf.h \
			  memlog.h method.h \
			  mman.h mode.h mount.h msg.h msgbuf.h \
			  mtio.h namei.h param.h pcm.h poll.h proc.h \
			  presto.h prestoioctl.h \
			  protosw.h ptrace.h psx4_nspace.h reboot.h \
			  resource.h rt_limits.h secdefines.h secioctl.h \
			  secpolicy.h security.h seek.h sem.h shm.h signal.h \
			  socket.h socketvar.h stat.h svcinfo.h swap.h systeminfo.h \
			  stream.h strkinfo.h strlog.h strstat.h cmn_err.h \
			  sysconfig.h sysinfo.h syslimits.h syslog_pri.h \
			  syslog.h systm.h table.h tablet.h termio.h \
			  termios.h time.h timeb.h timers.h times.h \
			  trace.h ttmap.h tty.h ttychars.h \
			  ttydefaults.h ttydev.h ttyloc.h types.h \
			  ucred.h uio.h un.h unix_defs.h unpcb.h \
			  user.h ustat.h utsname.h uswitch.h vadvise.h vcmd.h \
			  version.h vlimit.h vp_swap.h statvfs.h\
			  vm.h vmmac.h vmparam.h vnode.h vsio.h \
			  vtimes.h wait.h workstation.h wsdevice.h \
			  dlpihdr.h enet.h sysaio.h sad.h eucioctl.h\
			  sec_objects.h sec_es_types.h sec_et_types.h \
			  cred.h sysmacros.h \
			  usercopy.h siginfo.h procset.h \
			  priocntl.h rtpriocntl.h tspriocntl.h ucontext.h \
			  cdrom.h

SYSCALLS		= syscall.h rt_syscall.h soe_two_syscall.h \
			  svid_three_syscall.h svr_four_syscall.h \
			  sysv_syscall.h ult_syscall.h 

OTHERS			= dir.h dkio.h devio.h

INCLUDES		= ${DATAFILES} ${OTHERS} ${SYSCALLS}

ILIST			= timers.h fault.h ${SYSCALLS} \
			  lc_core.h localedef.h method.h ucontext.h
IMODE			= 644
IDIR			= /usr/sys/include/sys/
EXPDIR			= /usr/include/sys/

SYMLINKS		= timers.h fault.h ucontext.h ${SYSCALLS}
SYMLINKDIR1		= sys
SYMLINKDIR2		= /usr/include

dir.h:	${ALWAYS}
	${RM} ${_RMFLAGS_} $@
	ln -s ../ufs/$@ $@

devio.h: ${ALWAYS}
	${RM} ${_RMFLAGS_} $@
	ln -s ../io/common/$@ $@

dkio.h:	${ALWAYS}
	${RM} ${_RMFLAGS_} $@
	ln -s ../io/dec/sysap/$@ $@

%_syscall.h: makesyscalls.ksh %_syscalls.master
	ksh makesyscalls.ksh -h %_syscalls.master %_

syscall.h: makesyscalls.ksh syscalls.master
	ksh makesyscalls.ksh -h syscalls.master

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
