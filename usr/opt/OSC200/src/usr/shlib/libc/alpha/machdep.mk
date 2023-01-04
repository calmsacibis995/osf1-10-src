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

EXPORTS	=
LIBS	=

${TARGET_MACHINE}CFLAGS = -Dconst=' ' -Olimit 2000

# IEEE cflags see libc/Makefile
itrunc.o_CFLAGS		= $(IEEE_DRM_CFLAGS)
uitrunc.o_CFLAGS	= $(IEEE_DRM_CFLAGS)
frexp.o_CFLAGS		= $(IEEE_CFLAGS)
modf.o_CFLAGS		= $(IEEE_CFLAGS)
read_rnd.o_CFLAGS	= $(IEEE_CFLAGS)
write_rnd.o_CFLAGS	= $(IEEE_CFLAGS)

${TARGET_MACHINE}COBJS = alloca.o fp_const.o getlogin.o \
	msem_init.o msem_remove.o msem_lock.o msem_unlock.o \
	nlist.o sigpending.o sigprocmask.o sigsuspend.o \
	index.o rindex.o __cfe_support.o \
	remque.o insque.o frexp.o modf.o \
	read_rnd.o write_rnd.o itrunc.o uitrunc.o \
	libloader.o excdum.o find_rtfunc.o exc_failure.o alpha_unwind.o \
	ieee.o

${TARGET_MACHINE}SOBJS = audgen.o audcntl.o Ovfork.o _exit.o _getlogin.o \
	_sigpending.o _sigprocmsk.o _sigsuspend.o abs.o \
	accept.o access.o acct.o adjtime.o asyncdaemon.o \
	bind.o bcmp.o bcopy.o bzero.o \
	cacheflush.o cerror.o chdir.o chmod.o chown.o chroot.o \
	close.o connect.o dup.o lchown.o\
	dup2.o execl.o execle.o \
	execv.o execve.o execwl.o \
	fabs.o fchmod.o fchown.o fcntl.o ffs.o flock.o \
	fork.o fp_class.o fp_control.o fstat.o \
	fstatfs.o fsync.o ftruncate.o getaddrconf.o \
	getdirentri.o getdomainnm.o getdtablesz.o \
	getegid.o geteuid.o getfh.o getfsstat.o getgid.o \
	getgroups.o gethostid.o gethostname.o getitimer.o getpagesize.o \
	getpeername.o getpgrp.o getpgid.o getpid.o getppid.o getpriority.o \
	getrlimit.o getrusage.o getsockname.o getsockopt.o getsysinfo.o \
	gettimeofdy.o getuid.o htonl.o htons.o \
	ioctl.o kill.o kloadcall.o ldexp.o link.o listen.o lseek.o lstat.o \
	madvise.o memcntl.o memcmp.o memchr.o memcpy.o memmove.o memset.o \
	mkdir.o \
	mknod.o mmap.o mount.o mprotect.o msem_tas.o msgctl.o msgget.o \
	msgrcv.o msgsnd.o munmap.o mvalid.o nfssvc.o ntohl.o \
	msleep.o msync.o mwakeup.o \
	naccept.o ngetsockname.o nrecvfrom.o  \
	ngetpeername.o nrecvmsg.o nsendmsg.o \
	ntohs.o open.o _open.o pipe.o plock.o poll.o profil.o ptrace.o \
	quotactl.o read.o readlink.o readv.o reboot.o \
	recv.o recvfrom.o recvmsg.o rename.o \
	rmdir.o \
	security.o select.o semctl.o semget.o \
	semop.o send.o sendmsg.o sendto.o setdomainnm.o setgid.o \
	setgroups.o sethostid.o sethostname.o setitimer.o \
	setlogin.o setpattr.o setpgid.o setpriority.o setsysinfo.o \
	setregid.o setreuid.o setrlimit.o setsid.o setsockopt.o \
	settimeofdy.o setuid.o shmat.o shmctl.o shmdt.o shmget.o \
	shutdown.o sigaction.o sigreturn.o sigstack.o socket.o \
	socketpair.o stat.o statfs.o strcat.o strcmp.o strcpy.o strlen.o \
	swapctl.o swapon.o symlink.o sync.o syscall.o table.o \
	truncate.o umask.o uname.o \
	unlink.o usleep_thread.o uswitch.o utimes.o wait4.o \
	write.o writev.o chflags.o fchdir.o fchflags.o \
	setpgrp.o umount.o revoke.o exportfs.o getsid.o \
	environ.o atod.o dtoa.o tenscale.o \
	priocntlset.o sigaltstack.o sigsendset.o waitid.o \
	_sigaltstack.o getcontext.o \
	setjmp.o _setjmp.o sigjmp.o longjmp.o _longjmp.o _longjmp_internal.o \
	__divl.o __divlu.o __divq.o __divqu.o \
	__reml.o __remlu.o __remq.o __remqu.o divglobl.o \
	sysfs.o uadmin.o sysinfo.o fuser.o

#
# These spell out the dependencies of the setjmp entry point files
# on their .s include files.
#
setjmp.s		: setjmp_incl.s
longjmp.s		: longjmp_incl.s
_setjmp.s		: _setjmp_incl.s
_longjmp.s		: _longjmp_incl.s ljresume_incl.s
sigjmp.s		: setjmp_incl.s
_longjmp_internal.s	: longjmp_incl.s

#not shared - brk.o sbrk.o

umount.o_CFLAGS		= -DSYS_umount=SYS_unmount ${CFLAGS}

${${TARGET_MACHINE}SOBJS}: $${@:.o=.s}
	${_CC_} ${_CCFLAGS_} -g0 -O2 -c $*.s

