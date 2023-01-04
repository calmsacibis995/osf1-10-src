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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.4.20.14 $ (DEC) $Date: 1993/11/12 21:04:18 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

VPATH			= ${target_machine}:${MAKETOP}/usr/ccs/lib/libcda:${MAKETOP}/usr/ccs/lib/libdvs:${MAKETOP}/usr/ccs/lib/DECthreads/COMMON:${MAKETOP}/usr/bin/isam/include:${MAKETOP}/usr/ccs/lib/libc/SIA:${MAKETOP}/usr/ccs/lib/libmld:${MAKETOP}/usr/ccs/lib/libdnet:${MAKETOP}/usr/ccs/lib/libcmalib

COMMON_SUBDIRS		= arpa loader ntp protocols rpc rpcsvc sdm \
			  setld tli dce img netdnet
KERNEL_SUBDIRS		= sys net netinet netns nfs s5fs ufs cdfs lvm \
			  vm procfs lat io tty
MACH_SUBDIRS		= mach mach_debug servers
SUBDIRS			= ${COMMON_SUBDIRS} ${KERNEL_SUBDIRS} ${MACH_SUBDIRS} \
			  ${${target_machine}_SUBDIRS}
SIAFILES		= sia.h siad.h
EXPFILES		= AFdefs.h NLctype.h \
			  acl.h aio.h ar.h assert.h binsem.h c_asm.h cpio.h \
			  cpyrght.h crypt.h ctype.h curses.h curshdr.h cvt.h dbm.h \
			  defenv.h dirent.h disktab.h dlfcn.h dmpfmt.h \
			  dumprestor.h fatal.h fcntl.h fmtmsg.h \
			  fnmatch.h iconv.h \
			  fp.h fstab.h ftw.h getopt.h glob.h gmon.h grp.h jctype.h \
			  langinfo.h lastlog.h ldfcn.h lib_data.h loader.h \
			  locale.h login.h lprio.h mach.h \
			  mach_error.h mach_exception.h \
			  mach_init.h mach_o_format.h mach_o_header.h \
			  mach_o_vals.h macros.h malloc.h mandatory.h \
			  mbstr.h memory.h mesg.h mlock.h mon.h \
			  monetary.h mp.h msg01.h \
			  msg04.h msg06.h msg08.h mtab.h ndbm.h \
			  netdb.h nl_types.h paths.h pcc.h poll.h \
			  prot.h protcmd.h pty.h pwd.h random.h \
			  regex.h regexp.h resolv.h sched.h search.h sgtty.h \
			  stab.h standards.h std_labels.h stddef.h \
			  stdio.h stdlib.h string.h strings.h \
			  stropts.h struct.h sysexits.h tar.h time.h \
			  ttyent.h tzfile.h ulimit.h unctrl.h unistd.h \
			  userpw.h utime.h utmp.h vfont.h wchar.h \
			  wcstr.h wordexp.h wstring.h capsar.h \
			  ${${target_machine}_DATAFILES} ${SIAFILES}
XTIFILES		= tiuser.h xti.h xtiext.h
STRFILES		= stropts.h
CDAIFILES		= cda_msg.h cda_def.h ddif_def.h dtif_def.h cda_ptp.h \
			  cda_typ.h dvr_cc_def.h dvr_cc_ptp.h dvr_msg.h \
			  cdamsg.h cdadef.h ddifdef.h dtifdef.h cdaptp.h \
			  cdatyp.h dvrcdef.h dvrcptp.h dvrmsg.h cdatrans.h
CDAPVTFILES		= cdaityp.h cdaident.h dotsdef.h ddisint.h ddisprt.h \
			  ddisptp.h ddisdef.h ddismsg.h \
			  dvsdef.h dvsint.h dvsptp.h dvsarr.h dvrint.h
CMAIFILES		= cma_library.h
DATAFILES		= ${EXPFILES} ${XTIFILES} ${STRFILES} \
			  ${CDAIFILES} ${CDAPVTFILES} lib_lock.h ${CMAIFILES}
SYS_LINKS		= NLchar.h errno.h limits.h \
			  signal.h syscall.h rt_syscall.h sysv_syscall.h \
			  ult_syscall.h syslog.h syslog_pri.h termio.h \
			  termios.h timers.h ustat.h lmf.h wait.h siginfo.h \
			  ucontext.h

MACH_LINKS		= exception.h mach_types.h mig_errors.h msg_type.h
CMA_LINKS		= cma.h pthread.h pthread_exc.h exc_handling.h
OTHERS			= machine math.h term.h \
			  ${SYS_LINKS} ${MACH_LINKS} ${CMA_LINKS} \
			  ${${target_machine}_OTHERS}
INCLUDES		= ${DATAFILES} ${OTHERS}

EXPINC_SUBDIRS		= ${SUBDIRS} cmplrs eca

IDIR			= /usr/include/
IMODE			= 444

ILIST			= ${EXPFILES} ${OTHERS} ${XTIFILES} ${STRFILES} \
			  ${CDAIFILES} ${CMAIFILES}

SYMLINKS		= dec mach_debug ${target_machine} net netinet \
			  netns nfs s5fs sys ufs mach io lvm \
			  cdfs dli excpt.h

excpt.h_SYMLINK 	= excepthdr.h
excpt.h_SYMLINKDIR1 	= .
${target_machine}_SYMLINKDIR1	= ../sys/include/arch/
SYMLINKDIR1		= ../sys/include
SYMLINKDIR2		= /usr/include


include ${target_machine}/machdep.mk
include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
include ${MAKEFILEPATH}/datafiles.mk
include ${MAKEFILEPATH}/objects.mk

machine: ${ALWAYS}
	${RM} -r ${_RMFLAGS_} machine
	ln -s ${target_machine} machine

math.h:	math-hdr ${target_machine}/math.h
	cat math-hdr ${target_machine}/math.h > $@

${MACH_LINKS}: ${ALWAYS}
	${RM} ${_RMFLAGS_} $@
	ln -s mach/$@ $@

${SYS_LINKS}: ${ALWAYS}
	${RM} ${_RMFLAGS_} $@
	ln -s sys/$@ $@

${CMA_LINKS}: ${ALWAYS}
	${RM} ${_RMFLAGS_} $@
	ln -s dce/$@ $@

${XTIFILES}: ${MAKETOP}/kernel/streamsm/$${@}
	${RM} ${_RMFLAGS_} $@
	${CP} $? $@

${STRFILES}: ${MAKETOP}/kernel/sys/$${@}
	${RM} ${_RMFLAGS_} $@
	${CP} $? $@

term.h: ${MAKETOP}/usr/ccs/lib/libcurses/caps \
	${MAKETOP}/usr/ccs/lib/libcurses/maketerm.ex
	${RM} ${_RMFLAGS_} caps.e
	${CP} ${MAKETOP}/usr/ccs/lib/libcurses/caps caps.e
	@ed < ${MAKETOP}/usr/ccs/lib/libcurses/maketerm.ex
	${RM} ${_RMFLAGS_} caps.e capnames.c




