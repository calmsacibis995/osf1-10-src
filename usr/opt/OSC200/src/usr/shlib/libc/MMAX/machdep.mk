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
#	@(#)$RCSfile: machdep.mk,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:54:05 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

MMAXCFLAGS =

MMAXCOBJS = calls.o fabs.o fp_const.o frexp.o getlogin.o ldexp.o modf.o \
	nlist.o sigpending.o sigprocmask.o sigsuspend.o strlen.o \
	msem_init.o msem_lock.o msem_remove.o  msem_unlock.o

MMAXSOBJS = Ovfork.o __udivsi3.o __umodsi3.o _exit.o _getlogin.o \
	_setjmp.o _sigpending.o _sigprocmsk.o _sigsuspend.o abs.o \
	accept.o access.o acct.o adjtime.o alloca.o asyncdaemon.o \
	bcmp.o bind.o bzero.o \
	cerror.o chdir.o chmod.o chown.o chroot.o \
	close.o connect.o dup.o \
	dup2.o execl.o execle.o exect.o \
	execv.o execve.o execwl.o \
	fchmod.o fchown.o fcntl.o ffs.o flock.o \
	fork.o fstat.o fstatfs.o fsync.o ftruncate.o \
	getaddrconf.o getdirentri.o getdomainnm.o getdtablesz.o \
	getegid.o geteuid.o getfh.o getfsstat.o getgid.o \
	getgroups.o gethostid.o gethostname.o getitimer.o getpagesize.o \
	getpeername.o getpgrp.o getpid.o getppid.o getpriority.o \
	getrlimit.o getrusage.o getsockname.o getsockopt.o gettimeofdy.o \
	getuid.o htonl.o htons.o index.o insque.o \
	ioctl.o kill.o kloadcall.o link.o \
	listen.o lseek.o lstat.o madvise.o mkdir.o mknod.o \
	mmap.o mount.o mprotect.o msgctl.o msgget.o \
	msgrcv.o msgsnd.o munmap.o mvalid.o msem_tas.o msleep.o mwakeup.o \
	msync.o nfssvc.o ntohl.o \
	ngetpeername.o nrecvfrom.o nsendmsg.o \
	naccept.o ngetsockname.o nrecvmsg.o \
	ntohs.o open.o pipe.o plock.o poll.o profil.o ptrace.o \
	quotactl.o read.o readlink.o readv.o reboot.o \
	recv.o recvfrom.o recvmsg.o remque.o rename.o \
	rindex.o rmdir.o select.o semctl.o \
	semget.o semop.o send.o sendmsg.o sendto.o setdomainnm.o \
	setgid.o setgroups.o sethostid.o sethostname.o setitimer.o setjmp.o \
	setlogin.o setpattr.o setpgid.o setpriority.o \
	setregid.o setreuid.o setrlimit.o setsid.o setsockopt.o \
	settimeofdy.o setuid.o shmat.o shmctl.o shmdt.o shmget.o \
	shutdown.o sigaction.o sigentry.o sigstack.o socket.o \
	socketpair.o stat.o statfs.o strcat.o strcmp.o \
	strcpy.o swapon.o symlink.o sync.o syscall.o \
	truncate.o umask.o uname.o \
	unlink.o utimes.o table.o wait4.o \
	write.o writev.o write_rnd.o chflags.o fchdir.o fchflags.o \
	setpgrp.o umount.o revoke.o
# not shared - brk.o sbrk.o

umount.o_CFLAGS	= -DSYS_umount=SYS_unmount

${MMAXSOBJS:.o=.S}: $${@:.S=.s}
	${RM} -f $@
	${CP} ${@:.S=.s} $@

${MMAXSOBJS}: $${@:.o=.S}
	${_CC_} ${_CCFLAGS_} -c $*.S
#	${LD} -x -r $*.o
#	${MV} -f a.out $*.o
