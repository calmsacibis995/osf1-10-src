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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.3.19.22 $ (DEC) $Date: 1993/11/22 19:31:36 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

LIBCPATH	= ${MAKETOP}usr/ccs/lib/libc
LIBCRPATH	= ${MAKETOP}usr/ccs/lib/libc_r
VPATH		= ${LIBCPATH}:${LIBCPATH}/${target_machine}:${LIBCPATH}/SIA

USE_STATIC_LIBRARIES	=
LIBRARIES	= libc_r.a
MSGHDRS		= libc_msg.h

EXPLIB_TARGETS	= export_libc_r.a

ILIST		= libc_r.a
IDIR		= /usr/ccs/lib/
IMODE		= 644

HFILES          = sia.h siad.h siad_bsd.h
INCFLAGS	= -I${target_machine} -I../libc -I../libc/SIA
CFLAGS		= -DCOMPAT_43  -DLIBC_SCCS -D_THREAD_SAFE -D_REENTRANT ${_NAME_SPACE_WEAK_STRONG} \
		  ${${TARGET_MACHINE}CFLAGS} 

wait3.o_CFLAGS  = -D_BSD ${CFLAGS}
drand48.o_CFLAGS = -D_XOPEN_SOURCE ${CFLAGS}.

#
# strfmon() needs to be built in ANSI C mode so that its
# prototype declaration in <monetary.h> is used.  In non
# ANSI C mode, the non-prototype declaration is used
# instead, and it conflicts with the function definition.
#
strfmon.o_CFLAGS	= -std1 ${CFLAGS}

#
# scanf(), fscanf(), sscanf(), sprintf(), fread() and fwrite()
# also need to be built in ANSI C mode as indicated above.
#
scanf.o_CFLAGS		= -std1 ${CFLAGS}
sprintf.o_CFLAGS	= -std1 ${CFLAGS}
fread.o_CFLAGS		= -std1 ${CFLAGS}
fwrite.o_CFLAGS		= -std1 ${CFLAGS}

#
# syslog needs to be built in ANSI mode so the syslog.h prototype
# agrees with the syslog.c declaration.
syslog.o_CFLAGS		= -std1 ${CFLAGS}

#
# special cflags for routines needing ieee support
#	DRM stands for dynamic rounding
# any new object which does fp arithmetic or comparisons on values
#	which may not be finite must add a line here.
#
IEEE_CFLAGS		= -ieee_with_no_inexact ${CFLAGS}
IEEE_DRM_CFLAGS		= $(IEEE_CFLAGS) -fprm d

atof.o_CFLAGS		= $(IEEE_CFLAGS)
difftime.o_CFLAGS	= $(IEEE_CFLAGS)
doprnt.o_CFLAGS		= $(IEEE_CFLAGS)
doscan.o_CFLAGS		= $(IEEE_CFLAGS)
wcstod.o_CFLAGS		= $(IEEE_DRM_CFLAGS)


OFILES		= ${LIBC_R_OFILES} \
                  ${STDOBJS} \
		  ${SIAOBJS} \
		  ${ILS_REENTRANT_OBJS} \
		  ${${TARGET_MACHINE}COBJS} \
		  ${${TARGET_MACHINE}SOBJS}

LIBC_R_OFILES   = flockfile.o libc_locks.o rec_mutex.o \
		brks.o

STDOBJS	= NCchrlen.o NCcollate.o NCcoluniq.o NCdec.o NCdechr.o NCdecode.o \
	NCdecstr.o NCenc.o NCencode.o NCencstr.o NCflatchr.o NCisNLchar.o \
	abort.o alarm.o atof.o atoi.o atol.o \
	clock.o closedir.o clrerr.o catgets.o catopen.o calloc.o\
	cfspeed.o ctermid.o ctime.o cuserid.o data.o difftime.o\
	dirname.o disklabel.o \
	doprnt.o doscan.o drand48.o ecvt.o exec_args.o execlp.o execvp.o\
	exit.o fdopen.o feof.o ferror.o fgetc.o fgets.o fgetws.o\
	fileno.o findiop.o flsbuf.o fmtmsg.o fopen.o fprintf.o\
	fputc.o fputs.o fputws.o fread.o fseek.o fstab.o ftell.o\
	ftw.o fwrite.o gcvt.o getc.o getchar.o getcwd.o getdate.o\
	gethostent.o getcommon.o getnetent.o getprotoent.o getservent.o\
	getenv.o getmntinfo.o getsvc.o gets.o getwd.o\
	getttyent.o getttynam.o glob.o gettxt.o getusershll.o\
	getut.o getw.o getwc.o herror.o hsearch.o initgroups.o \
	isalpha.o isascii.o iscntrl.o isdigit.o isgraph.o islower.o\
	ispunct.o isspace.o isupper.o isxdigit.o\
	ldr_atexit.o ldr_status.o ldr_syscalls.o ldr_xcalls.o\
	ldr_dummy.o ldr_dummy2.o linkaddr.o l64a.o\
	mkstackexec.o mktemp.o mkstemp.o memccpy.o \
	nice.o ns_ntoa.o opendir.o\
	nano_timers.o pathconf.o perror.o popen.o printf.o putc.o putchar.o\
	putenv.o puts.o putw.o putwc.o rand.o readdir.o\
	regex.o remove.o rew.o rewinddir.o res_comp.o res_init.o \
	res_mkquery.o res_send.o res_query.o scanf.o setbuf.o\
	setbuffer.o setenv.o setlocale.o setvbuf.o signal.o\
	sighold.o sigops.o ssignal.o syslog.o random.o\
	sprintf.o strcasecmp.o strcspn.o strerror.o\
	strtol.o strtoul.o sysconf.o strncpy.o\
	strpbrk.o strrchr.o strspn.o strtod.o strtok.o system.o \
	tcgetattr.o tcsetattr.o tempnam.o tmpfile.o tmpnam.o tolower.o\
	toupper.o ttyname.o ttyslot.o ulimit.o ungetwc.o ungetc.o utmpname.o\
	vfprintf.o vprintf.o vsprintf.o creadir.o filbuf.o isatty.o\
	killpg.o lockf.o mkfifo.o ndbm.o rcmd.o rexec.o ruserpass.o\
	streamio.o ustat.o wcstod.o \
	wait.o wait3.o getpasswd_r.o getgroup_r.o raise.o tzset.o \
	ptsname.o grantpt.o pty.o sigset.o


SIAOBJS= sia_alloc_en.o sia_c_finger.o sia_c_invkr.o sia_c_pass.o \
	sia_c_shell.o sia_free_ent.o sia_getgroup.o sia_getpass.o \
	sia_globals.o sia_init.o sia_mk_en_pw.o sia_s_auent.o sia_s_estab.o \
	sia_s_init.o sia_s_lnch.o sia_s_reauth.o sia_s_rel.o sia_s_suauth.o \
	siad_getgrp.o siad_getpass.o

ILS_REENTRANT_OBJS	= __ispriv.o \
		  __regcomp_std.o \
		  __strcoll_sb.o __strcoll_std.o \
		  __strxfrm_sb.o __strxfrm_std.o \
		  __wcscoll_std.o __wcsxfrm_std.o \
		  back_pos.o backward.o \
		  regerror.o \
		  replacement.o strfmon.o strftime.o strptime.o \
		  wcsftime.o wcstok.o

sysconf.o_CFLAGS	= -DH_RT ${CFLAGS}

include ${target_machine}/machdep.mk
include ${MAKEFILEPATH}/standard.mk
# include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk

ctime.o data.o findiop.o setlocale.o:	rec_mutex.h

clrerr.o doprnt.o doscan.o fdopen.o feof.o ferror.o fgetc.o \
findiop.o flsbuf.o fopen.o fprintf.o fputc.o fputs.o fread.o fseek.o ftell.o \
fwrite.o getc.o getchar.o gets.o getw.o getwc.o printf.o putc.o putchar.o \
puts.o putw.o putwc.o rew.o scanf.o setbuf.o setbuffer.o setvbuf.o ungetc.o \
vfprintf.o vprintf.o:	stdio_lock.h

assert.o getopt.o getcwd.o initgroups.o \
herror.o perror.o psignal.o random.o regex.o \
regcomp.o __regcomp_std.o regerror.o regexec.o __regexec_std.o \
rcmd.o rexec.o ruserpass.o \
setjmper.o strerror.o syslog.o: libc_msg.h

.c.o:
	${_CC_} -c ${_CCFLAGS_} $*.c
#	${LD} ${_LDFLAGS_} -x -r $*.o
#	${MV} -f a.out $*.o

#
#  Added pollution_check to build rules
#
libc_r.a: ${DEPEND_LIST} ${LIBRARIES}(${_OFILES_})
	${_AR_} ${DEF_ARFLAGS} $@ $?
	${_RANLIB_} $@
	${RM} -f $?
	ksh -p `${GENLOC} /src/usr/ccs/lib/libc/pollution_check` $@

