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
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

# IEEE specific flags, see libc/Makefile
itrunc.o_CFLAGS		= $(IEEE_DRM_CFLAGS)
uitrunc.o_CFLAGS	= $(IEEE_DRM_CFLAGS)
frexp.o_CFLAGS		= $(IEEE_CFLAGS)
modf.o_CFLAGS		= $(IEEE_CFLAGS)
read_rnd.o_CFLAGS	= $(IEEE_CFLAGS)
write_rnd.o_CFLAGS	= $(IEEE_CFLAGS)

${TARGET_MACHINE}COBJS = alloca.o getlogin.o ieee.o fp_const.o frexp.o msem_init.o \
	msem_lock.o msem_remove.o msem_unlock.o sigpending.o \
	sigprocmask.o sigsetjmp.o sigsuspend.o \
        index.o rindex.o __cfe_support.o read_rnd.o \
        write_rnd.o  itrunc.o uitrunc.o nlist.o modf.o remque.o insque.o \
	find_rtfunc.o alpha_unwind.o exc_failure.o


${TARGET_MACHINE}SOBJS = audcntl.o audgen.o bcmp.o bcopy.o bzero.o \
	getfsstat.o quotactl.o fchdir.o wait4.o revoke.o _getlogin.o \
	setlogin.o naccept.o nrecvfrom.o ngetpeername.o table.o \
	getsysinfo.o setsysinfo.o getpgrp.o ioctl.o lstat.o fstat.o stat.o \
	Ovfork.o _exit.o _sigpending.o _sigprocmsk.o _sigsuspend.o \
	abs.o accept.o access.o acct.o adjtime.o asyncdaemon.o bind.o \
	chdir.o chflags.o chmod.o chown.o chroot.o close.o \
	connect.o dup.o dup2.o execwl.o fchflags.o \
	fchmod.o fchown.o fcntl.o ffs.o flock.o fork.o fp_class.o fp_control.o \
	fsync.o ftruncate.o getaddrconf.o getdirentri.o getdomainnm.o \
	getdtablesz.o getegid.o geteuid.o getfh.o getgid.o getgroups.o \
	gethostid.o gethostname.o getitimer.o getpagesize.o getpeername.o \
	getpgid.o getpid.o getppid.o getpriority.o getrlimit.o getrusage.o getsid.o \
	getsockname.o getsockopt.o gettimeofdy.o getuid.o \
	kill.o kloadcall.o lchown.o ldexp.o link.o listen.o lseek.o \
	madvise.o memchr.o memcntl.o memcmp.o memcpy.o memmove.o memset.o \
	mkdir.o mknod.o mmap.o mount.o \
	mprotect.o msgctl.o msgget.o msgrcv.o msgsnd.o msleep.o msync.o \
	munmap.o mvalid.o mwakeup.o nfssvc.o ngetsockname.o nrecvmsg.o \
	nsendmsg.o open.o _open.o plock.o poll.o profil.o read.o readlink.o \
	readv.o reboot.o recv.o recvfrom.o recvmsg.o rename.o rmdir.o \
	select.o semctl.o semget.o semop.o sendmsg.o sendto.o \
	setdomainnm.o \
	setgid.o setgroups.o sethostid.o sethostname.o setitimer.o \
	_setjmp.o setjmp.o setpattr.o setpgid.o setpgrp.o setpriority.o \
	setregid.o setreuid.o setrlimit.o setsid.o setsockopt.o settimeofdy.o \
	setuid.o shmat.o shmctl.o shmdt.o shmget.o shutdown.o sigreturn.o sigstack.o \
	socket.o socketpair.o strcat.o strcmp.o strcpy.o strlen.o swapctl.o \
	swapon.o \
	symlink.o sync.o syscall.o truncate.o umask.o umount.o uname.o \
	unlink.o usleep_thread.o uswitch.o utimes.o write.o writev.o \
	cacheflush.o exportfs.o fabs.o fstatfs.o pipe.o ptrace.o \
	security.o \
	send.o statfs.o sigaction.o execve.o sbrk.o cerror.o htonl.o htons.o \
        ntohl.o ntohs.o execl.o execv.o execle.o brk.o \
	environ.o atod.o dtoa.o tenscale.o \
	priocntlset.o sigaltstack.o sigsendset.o waitid.o \
	__divl.o __divlu.o __divq.o __divqu.o \
	__reml.o __remlu.o __remq.o __remqu.o divglobl.o msem_tas.o \
	sysfs.o fuser.o sysinfo.o uadmin.o _sigaltstack.o getcontext.o \
	sigjmp.o longjmp.o _longjmp.o _longjmp_internal.o

#
# These spell out the dependencies of the setjmp entry point files
# on their .s include files.
setjmp.s		: setjmp_incl.s
longjmp.s		: longjmp_incl.s
_setjmp.s		: _setjmp_incl.s
_longjmp.s		: _longjmp_incl.s ljresume_incl.s
sigjmp.s		: setjmp_incl.s
_longjmp_internal.s	: longjmp_incl.s

#
# These syscalls are in the PMAX directory and have not been ported to alpha,
# yet.  Some of these are mips specific and will not be needed for alpha.
#
${TARGET_MACHINE}CNOTYET = calls.o emulate_br.o

${TARGET_MACHINE}SNOTYET = setquota.o

umount.o_CFLAGS		= -DSYS_umount=SYS_unmount ${CFLAGS}

#${alphaSOBJS:.o=.S}: $${@:.S=.s}
#	${RM} -f $@
#	${CP} ${@:.S=.s} $@

${${TARGET_MACHINE}SOBJS}: $${@:.o=.s}
	${_CC_} ${_CCFLAGS_} -c $*.s
#	${LD} -x -r $*.o
#	${MV} -f a.out $*.o
