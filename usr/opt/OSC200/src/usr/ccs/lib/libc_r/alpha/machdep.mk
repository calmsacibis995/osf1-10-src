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

ALPHACFLAGS = -Olimit 2000

ALPHACOBJS = getlogin.o fp_const.o frexp.o msem_init.o msem_lock.o \
	msem_remove.o msem_unlock.o sigpending.o sigprocmask.o \
	sigsuspend.o find_rtfunc.o \
        index.o rindex.o \
        write_rnd.o  nlist.o modf.o remque.o insque.o


ALPHASOBJS = bcmp.o bcopy.o brk.o bzero.o \
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
	madvise.o memchr.o memcmp.o memcpy.o memmove.o memset.o mkdir.o \
	mknod.o mmap.o mount.o \
	mprotect.o msgctl.o msgget.o msgrcv.o msgsnd.o msleep.o msync.o \
	munmap.o mvalid.o mwakeup.o nfssvc.o ngetsockname.o nrecvmsg.o \
	nsendmsg.o open.o plock.o poll.o profil.o read.o readlink.o \
	readv.o reboot.o recv.o recvfrom.o recvmsg.o rename.o rmdir.o \
	sbrk.o select.o semctl.o semget.o semop.o sendmsg.o sendto.o \
	setdomainnm.o \
	setgid.o setgroups.o sethostid.o sethostname.o setitimer.o \
	setpattr.o setpgid.o setpgrp.o setpriority.o \
	setregid.o setreuid.o setrlimit.o setsid.o setsockopt.o settimeofdy.o \
	setuid.o shmat.o shmctl.o shmdt.o shmget.o shutdown.o sigreturn.o sigstack.o \
	socket.o socketpair.o strcat.o strcmp.o strcpy.o strlen.o swapon.o \
	symlink.o sync.o syscall.o truncate.o umask.o umount.o uname.o \
	unlink.o usleep_thread.o uswitch.o utimes.o write.o writev.o \
	cacheflush.o exportfs.o fabs.o fstatfs.o pipe.o _open.o ptrace.o \
	send.o statfs.o sigaction.o execve.o cerror.o htonl.o htons.o \
        ntohl.o ntohs.o execl.o execv.o execle.o \
	environ.o atod.o dtoa.o tenscale.o \
	__divl.o __divlu.o __divq.o __divqu.o \
	__reml.o __remlu.o __remq.o __remqu.o msem_tas.o \
	ts_setjmp.o ts_longjmp.o _setjmp.o _longjmp.o sigjmp.o \
	_longjmp_internal.o 	

#
# These spell out the dependencies of the setjmp entry point files
# on their .s include files.
#
ts_setjmp.s 		: _setjmp_incl.s
ts_longjmp.s		: _longjmp_incl.s ljresume_incl.s
_setjmp.s		: _setjmp_incl.s
_longjmp.s		: _longjmp_incl.s
sigjmp.s		: setjmp_incl.s
_longjmp_internal.s	: longjmp_incl.s

#
# These syscalls are in the PMAX directory and have not been ported to ALPHA,
# yet.  Some of these are mips specific and will not be needed for ALPHA.
#
ALPHACNOTYET = calls.o emulate_br.o

ALPHASNOTYET = setquota.o

umount.o_CFLAGS		= -DSYS_umount=SYS_unmount ${CFLAGS}

ALPHACNOTREENTRANT = alloc.o

ALPHASNOTREENTRANT =  setjmp.o  


#${ALPHASOBJS:.o=.S}: $${@:.S=.s}
#	${RM} -f $@
#	${CP} ${@:.S=.s} $@

${ALPHASOBJS}: $${@:.o=.s}
	${_CC_} ${_CCFLAGS_} -g0 -O -c $*.s
#	${LD} -x -r $*.o
#	${MV} -f a.out $*.o
