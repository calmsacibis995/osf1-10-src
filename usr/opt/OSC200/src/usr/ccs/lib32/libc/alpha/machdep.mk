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
# @(#)$RCSfile: machdep.mk,v $ $Revision: 1.1.4.4 $ (DEC) $Date: 1993/11/16 22:22:41 $
#

${TARGET_MACHINE}CFLAGS = -Olimit 1000

${TARGET_MACHINE}COBJS = getrlimit_c.o index.o rindex.o setrlimit_c.o sigprocmask.o sigsuspend.o find_rtfunc.o alpha_unwind.o exc_failure.o __cfe_support.o


${TARGET_MACHINE}SOBJS = Ovfork.o _exit.o _setjmp.o access.o atod.o bcmp.o \
bcopy.o bzero.o cerror.o close.o dtoa.o dup.o execl.o \
execv.o execve.o fcntl.o fork.o fstat.o getdirentri.o getdtablesz.o \
geteuid.o getpagesize.o getpid.o getrusage.o gettimeofdy.o ioctl.o kill.o \
ldexp.o lseek.o lstat.o madvise.o memchr.o memmove.o memset.o open.o pipe.o \
read.o \
sbrk.o setitimer.o sigaction.o stat.o strcat.o strcmp.o strlen.o tenscale.o \
unlink.o wait4.o write.o memcpy.o strcpy.o _sigprocmsk.o _sigsuspend.o\
	__divl.o __divlu.o __divq.o __divqu.o Dynamic_Link.o setjmp.o \
	__reml.o __remlu.o __remq.o __remqu.o msem_tas.o divglobl.o _open.o \
sigstack.o environ.o mmap.o munmap.o statfs.o fstatfs.o \
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

umount.o_CFLAGS		= -DSYS_umount=SYS_unmount

doprnt.o_CFLAGS		= -Olimit 1000

__cfe_support.o_CFLAGS  = -32addr
#${${TARGET_MACHINE}SOBJS:.o=.S}: $${@:.S=.s}
#	${RM} -f $@
#	${CP} ${@:.S=.s} $@

${${TARGET_MACHINE}SOBJS}: $${@:.o=.s}
	${_CC_} ${_CCFLAGS_} -g0 -O -c $*.s
#	${LD} -x -r $*.o
#	${MV} -f a.out $*.o
