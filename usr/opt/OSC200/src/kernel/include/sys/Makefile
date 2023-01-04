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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.5.18.21 $ (DEC) $Date: 1993/11/23 22:47:30 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#

VPATH			= ${MAKETOP}/kernel/sys:${MAKETOP}/kernel/io/dec/ws

DATAFILES		= NLchar.h fbinfo.h map.h select.h tty.h \
			access.h fcntl.h mbuf.h sem.h ttychars.h \
			acct.h file.h shm.h ttydefaults.h \
			addrconf.h filsys.h mman.h signal.h ttydev.h \
			audit.h flock.h mode.h socket.h ttyloc.h \
			auxv.h fpsconf.h mount.h socketvar.h types.h \
			biostats.h fpsreg.h msg.h specdev.h ucred.h \
			bkmac.h msgbuf.h stat.h uio.h \
			buf.h fs_types.h mtio.h stream.h cred.h un.h \
			callout.h fullstat.h namei.h strkinfo.h unix_defs.h \
			gprof.h open.h param.h strlog.h unpcb.h \
			clist.h habitat.h pcm.h stropts.h user.h \
			id.h pfcntr.h poll.h strstat.h ustat.h \
			cmap.h inputdevice.h preempt.h svcinfo.h utctime.h \
			cmn_err.h ioctl.h presto.h sysconfig.h utsname.h \
			cmupty.h ioctl_compat.h prestoioctl.h sysinfo.h \
			vadvise.h conf.h ipc.h proc.h syslimits.h vcmd.h \
			config.h jctype0.h protosw.h syslog.h syslog_pri.h \
			version.h \
			core.h jctype1.h ptrace.h systm.h vlimit.h \
			jioctl.h systrace.h vm.h \
			disklabel.h kernel.h reboot.h table.h vmmac.h \
			dk.h kloadcall.h resource.h tablet.h vmparam.h \
			dkbad.h ldr_exec.h termio.h vnode.h \
			domain.h limits.h sched_mon.h termios.h vsio.h \
			lk201.h sec_objects.h sec_et_types.h sec_es_types.h \
			sec_stream.h \
			time.h vtimes.h \
			secdefines.h timeb.h wait.h \
			errno.h lock.h secioctl.h times.h workstation.h \
			exec.h lock_types.h secpolicy.h trace.h wsdevice.h \
			exec_incl.h lpio.h security.h ttmap.h  rt_limits.h \
			psx4_nspace.h dlpihdr.h enet.h vp_swap.h vfs_ubc.h statvfs.h\
			uswitch.h lwc.h \
                        exit_actn.h lmf.h lmfklic.h sysaio.h old_rzdisk.h \
			pxinfo.h pvinfo.h vgainfo.h keysyms.h vfs_proto.h \
			sad.h seek.h usercopy.h malloc.h memlog.h\
			str_support.h eucioctl.h strtty.h sysmacros.h\
			fdfs.h fsid.h fstyp.h procset.h siginfo.h \
			priocntl.h rtpriocntl.h tspriocntl.h \
			swap.h fuser.h systeminfo.h uadmin.h kdebug.h cdrom.h \
			bt459.h pq.h px.h stamp.h

SYMLINKS	= dir.h machine
SYMLINKDIR1	= ../ufs
machine_SYMLINKDIR1	= ..
SYMLINKDIR2	= /usr/sys/include/sys

EXTRAS		= rt_syscall.h soe_two_syscall.h svid_three_syscall.h \
		  svr_four_syscall.h sysv_syscall.h syscall.h 

ILIST   = ${DATAFILES} ${EXTRAS} ${OTHERS}
IDIR    = /usr/sys/include/sys/
IMODE   = 444
IGROUP  = bin
IOWNER  = bin

OTHERS		      =	dkio.h devio.h

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk

dkio.h: ${ALWAYS}
	${RM} ${_RMFLAGS_} $@
	ln -s ../io/dec/sysap/$@ $@

devio.h: ${ALWAYS}
	${RM} ${_RMFLAGS_} $@
	ln -s ../io/common/$@ $@
