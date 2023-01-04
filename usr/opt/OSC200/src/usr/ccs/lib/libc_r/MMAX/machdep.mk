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
#	@(#)$RCSfile: machdep.mk,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:53:15 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

MMAXCOBJS = getlogin.o sigpending.o sigprocmask.o

MMAXSOBJS = _getlogin.o _sigpending.o _sigprocmsk.o _sigsuspend.o accept.o \
	    access.o acct.o adjtime.o asyncdaemon.o bind.o brk.o cerror.o \
	    chdir.o chflags.o chmod.o chown.o chroot.o close.o connect.o \
	    dup.o dup2.o execve.o execwl.o fchdir.o \
	    fchflags.o fchmod.o fchown.o fcntl.o flock.o fork.o fstat.o \
	    fstatfs.o fsync.o ftruncate.o getaddrconf.o getdirentri.o \
	    getdomainnm.o getdtablesz.o getfh.o getfsstat.o getgid.o \
	    getgroups.o gethostid.o gethostname.o getitimer.o getpagesize.o \
	    getpeername.o getpgrp.o getpid.o getpriority.o getrlimit.o \
	    getrusage.o getsockname.o getsockopt.o gettimeofdy.o getuid.o \
	    ioctl.o kill.o kloadcall.o link.o listen.o lseek.o \
	    lstat.o mkdir.o mknod.o mmap.o mount.o mprotect.o \
	    msgctl.o msgget.o msgrcv.o msgsnd.o msleep.o msync.o munmap.o \
	    mvalid.o mwakeup.o nfssvc.o open.o pipe.o plock.o poll.o profil.o \
	    ptrace.o read.o readlink.o readv.o reboot.o recv.o \
	    recvfrom.o recvmsg.o rename.o rmdir.o sbrk.o select.o semctl.o \
	    semget.o semop.o send.o sendmsg.o sendto.o setdomainnm.o \
	    setgid.o setgroups.o sethostid.o sethostname.o setitimer.o \
	    setlogin.o setpattr.o setpgid.o setpgrp.o setpriority.o \
	    setregid.o setreuid.o setrlimit.o setsid.o \
	    setsockopt.o settimeofdy.o setuid.o shmat.o shmctl.o shmdt.o \
	    shmget.o shutdown.o sigaction.o \
	    sigstack.o socket.o socketpair.o \
	    stat.o statfs.o swapon.o symlink.o sync.o syscall.o table.o \
	    truncate.o umask.o umount.o uname.o unlink.o utimes.o \
	    wait4.o write.o writev.o

INCFLAGS = ${INCFLAGS} -I../libc/${TARGET_MACHINE}

umount.o_CFLAGS	= -DSYS_umount=SYS_unmount

${MMAXSOBJS:.o=.S}: $${@:.S=.s}
	${RM} -f $@
	${CP} ${@:.S=.s} $@

${MMAXSOBJS}: $${@:.o=.S}
	${_CC_} ${_CCFLAGS_} -c $*.S
#	${LD} -x -r $*.o
#	${MV} -f a.out $*.o
