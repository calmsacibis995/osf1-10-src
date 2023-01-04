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
#	@(#)$RCSfile: machdep.mk,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:50:48 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

AT386COBJS = calls.o fixunsdfsi.o \
	    msem_init.o msem_remove.o msem_lock.o msem_unlock.o \
	    nlist.o setjmperr.o \
	    sigpending.o sigprocmask.o sigsuspend.o \
	    strcat.o strcpy.o strcmp.o strlen.o write_rnd.o \
	    fp_const.o rindex.o index.o bcopy.o bcmp.o sigsuspend.o \
	    bzero.o getlogin.o

AT386SOBJS = Ovfork.o __udivsi3.o __divsi3.o _exit.o _getlogin.o _setjmp.o \
	     _sigpending.o _sigprocmask.o _sigsuspend.o abs.o \
	     accept.o access.o acct.o adjtime.o alloca.o \
	     asyncdaemon.o bind.o brk.o cerror.o chdir.o \
	     chflags.o chmod.o chown.o chroot.o close.o \
	     connect.o dup.o dup2.o execl.o execle.o \
	     exect.o execv.o execwl.o execve.o fabs.o fchdir.o \
	     fchflags.o fchmod.o fchown.o fcntl.o fixdfsi.o \
	     ffs.o flock.o fork.o fpinit.o frexp.o fstat.o fstatfs.o \
	     fsync.o ftruncate.o getaddrconf.o getdirentri.o \
	     getdirentries.o getdomainname.o getdomainnm.o \
	     getdtablesz.o getdtablesize.o \
	     getegid.o geteuid.o getfh.o getfsstat.o getgid.o \
	     getgroups.o gethostid.o gethostname.o getitimer.o \
	     getpagesize.o getpeername.o getpgrp.o getpid.o \
	     getppid.o getpriority.o getrlimit.o getrusage.o \
	     getsockname.o getsockopt.o gettimeofday.o gettimeofdy.o \
	     getuid.o htonl.o htons.o ioctl.o insque.o kill.o  kloadcall.o\
	     ldexp.o link.o listen.o lseek.o lstat.o madvise.o mkdir.o \
	     mknod.o mmap.o modf.o mount.o mprotect.o \
	     msgctl.o msgget.o msgrcv.o msgsnd.o msleep.o msync.o \
	     munmap.o mvalid.o mwakeup.o msem_tas.o \
	     ngetpeername.o nrecvfrom.o nsendmsg.o \
	     naccept.o ngetsockname.o nrecvmsg.o \
	     nfssvc.o ntohl.o ntohs.o open.o pipe.o plock.o poll.o profil.o \
	     ptrace.o quotactl.o read.o readlink.o readv.o \
	     reboot.o recv.o recvfrom.o recvmsg.o rename.o remque.o revoke.o\
	     rmdir.o sbrk.o select.o semctl.o semget.o \
	     semop.o send.o sendmsg.o sendto.o setgid.o \
	     setdomainname.o setdomainnm.o \
	     setgroups.o sethostid.o sethostname.o setitimer.o \
	     setjmp.o setlogin.o setpattr.o setpgid.o  \
	     setpriority.o setregid.o setreuid.o \
	     setrlimit.o setsid.o setsockopt.o settimeofday.o \
	     settimeofdy.o setuid.o shutdown.o shmat.o shmctl.o \
	     shmdt.o shmget.o sigaction.o \
	     sigreturn.o sigstack.o \
	     socket.o socketpair.o stat.o statfs.o \
	     swapon.o symlink.o sync.o syscall.o table.o truncate.o \
	     umask.o uname.o unlink.o  \
	     utimes.o wait4.o write.o writev.o \
	     setpgrp.o umount.o

umount.o_CFLAGS		= -DSYS_umount=SYS_unmount
crt0.o_CFLAGS	= -DCRT0 -DMACH


${AT386SOBJS}: $${@:.o=.s}
	${CP} $*.s $*.S
	${_CC_} -traditional ${_CCFLAGS_} ${ALLSFLAGS} -E $*.S > $*.i
	${RM} -f $*.S
	grep -v "^#" $*.i > $*.as
	${RM} -f $*.i
	${AS} ${ASFLAGS} $*.as -o $*.o
	${RM} -f $*.as
	${LD} -x -r $*.o
	${MV} -f a.out $*.o
