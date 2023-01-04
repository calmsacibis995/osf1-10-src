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
# 	@(#)$RCSfile: Makefile,v $ $Revision: 4.3.27.25 $ (DEC) $Date: 1993/11/19 15:36:46 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

VPATH			= ${target_machine}:SIA

LIBRARIES		= libc.a
MSGHDRS			= libc_msg.h
CATFILES		= libc.cat

EXPLIBC_TARGETS		= export_libc.a

ILIST			= libc.a
IDIR			= /usr/ccs/lib/
IMODE			= 644


USE_STATIC_LIBRARIES	=

GLINE			=
INCFLAGS		= -I${target_machine} -ISIA
CFLAGS			= -DCOMPAT_43 -DLIBC_SCCS -DRT \
			  ${_NAME_SPACE_WEAK_STRONG} ${${TARGET_MACHINE}CFLAGS} 
#HFILES			= sia.h siad.h siad_bsd.h
scandir.o_CFLAGS	= -D_BSD ${CFLAGS}
wait3.o_CFLAGS		= -D_BSD ${CFLAGS}
key_call.o_CFLAGS	= -D_BSD ${CFLAGS}
kls_ipc.o_CFLAGS	= -D_KLS_IPC_SOCK_DGRAM ${CFLAGS}

#
# wctomb() and strfmon() need to be built in ANSI C mode
# so that their prototype declarations in <stdlib.h> and
# <monetary.h> are used.  In non ANSI C mode, the
# non-prototype declarations are used instead, and they
# conflict with the function definitions.
#
wctomb.o_CFLAGS		= -std1 ${CFLAGS}
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
dsto2fp.o_CFLAGS	= $(IEEE_CFLAGS)
floor.o_CFLAGS		= $(IEEE_DRM_CFLAGS)
rint.o_CFLAGS		= $(IEEE_DRM_CFLAGS)
wcstod.o_CFLAGS		= $(IEEE_DRM_CFLAGS)


OFILES			= ${STDOBJS} \
			  ${SIAOBJS} \
			  ${ILSOBJS} \
			  ${${TARGET_MACHINE}SOBJS} \
			  ${${TARGET_MACHINE}COBJS}
GARBAGE			= *.S *.as tags

STDOBJS=NCchrlen.o NCcollate.o NCcoluniq.o NCdec.o NCdechr.o NCdecode.o \
	NCdecstr.o NCenc.o NCencode.o NCencstr.o NCflatchr.o \
	NCisNLchar.o NLchrlen.o NLctime.o NLflatstr.o NLflattab.o \
	NLgetamsg.o NLisNLcp.o NLxcol.o \
	_tolower.o _toupper.o a64l.o abort.o alarm.o \
	assert.o atof.o atoi.o atol.o auditwrite.o \
	basename.o bresvport.o bsearch.o calloc.o \
	catgetmsg.o catgets.o catopen.o cfspeed.o clock.o \
	closedir.o clrerr.o confstr.o copysign.o creadir.o crypt.o \
	ctermid.o ctime.o cuserid.o data.o \
	difftime.o dirname.o disklabel.o divide.o doprnt.o \
	doscan.o drand48.o dsto2fp.o ecvt.o errlst.o errno.o \
	ether_addr.o exec_args.o execlp.o execvp.o exit.o expacct.o \
	fdopen.o feof.o ferror.o fgetc.o \
	fgetpos.o fgets.o filbuf.o fileno.o \
	findiop.o floor.o flsbuf.o fmtmsg.o fopen.o fprintf.o \
	fputc.o fputs.o fread.o fseek.o \
	fsetpos.o fstab.o ftell.o ftok.o ftw.o \
	fwrite.o gcvt.o getc.o getchar.o getcwd.o \
	getdate.o getenv.o getmntinfo.o \
	getnetent.o getopt.o getpass.o \
	getrpcent.o gets.o getservent.o getsubopt.o gettxt.o getttyent.o \
	getttynam.o getusershll.o getut.o getvfs.o getw.o getwc.o \
	getwchar.o getwd.o gtty.o \
	herror.o hsearch.o inet_addr.o inet_lnaof.o inet_mkadr.o \
	inet_netof.o inet_netwk.o inet_ntoa.o initgroups.o innetgr.o \
	isalnum.o isalpha.o isascii.o isatty.o iscntrl.o \
	isdigit.o isgraph.o islower.o isprint.o ispunct.o \
	isspace.o isupper.o isxdigit.o killpg.o \
	kls_client.o kls_ipc.o kls_message.o \
	l3.o l64a.o labs.o \
	ldr_atexit.o ldr_status.o ldr_syscalls.o ldr_xcalls.o \
	lfind.o libdl_dummy.o liblocale.o linkaddr.o localeconv.o \
	lockf.o login_tty.o lsearch.o ldr_dummy.o \
	ldr_dummy2.o malloc.o mblen.o mbs.o mbscat.o \
	mbscpy.o mbsncmp.o mbspbrk.o mbstowcs.o mbtowc.o \
	memccpy.o mkfifo.o mk_query.o mkstackexec.o mkstemp.o mktemp.o \
	nano_timers.o nbs_stub.o ndbm.o nice.o ns_addr.o ns_ntoa.o \
	opendir.o pathconf.o \
	pause.o perror.o pfopen.o popen.o printf.o psignal.o pty.o putc.o \
	putchar.o putenv.o putgrsent.o \
	puts.o putw.o putwc.o \
	putwchar.o qsort.o quota_init.o raise.o rand.o random.o \
	rcmd.o readdir.o regex.o regexp_jacket.o remove.o res_comp.o \
	res_debug.o res_init.o res_mkquery.o res_query.o res_send.o \
	rew.o rewinddir.o rexec.o rint.o ruserpass.o \
	scandir.o scanf.o seekdir.o setbuf.o setbuffer.o setegid.o \
	setenv.o seteuid.o setjmper.o setlocale.o setrgid.o \
	setruid.o setvbuf.o sigcompat.o sighold.o siglist.o \
	signal.o sigops.o sleep.o sprintf.o ssignal.o\
	stime.o strcasecmp.o strchr.o strcoll.o strcpyn.o strcspn.o \
	strdup.o streamio.o strerror.o strncat.o strncmp.o \
	strncpy.o strpbrk.o strrchr.o strspn.o strstr.o strtod.o \
	strtok.o strtol.o strtoul.o strxfrm.o stty.o \
	swab.o sysconf.o syslog.o system.o tcdrain.o \
	tcflow.o tcflush.o tcgetattr.o tcgetpgrp.o tcsendbreak.o \
	tcsetattr.o tcsetpgrp.o tell.o telldir.o tempnam.o \
	time.o times.o tmpfile.o tmpnam.o toascii.o \
	tolower.o toupper.o tsearch.o ttylock.o ttyname.o \
	ttyslot.o tzset.o ualarm.o ulimit.o ungetc.o ungetwc.o \
	usleep.o ustat.o utime.o utmpname.o vfprintf.o vprintf.o \
	vsprintf.o wait.o wait3.o waitpid.o wcscat.o wcschr.o \
	wcscspn.o wcslen.o wcsncat.o wcsncpy.o wcspbrk.o \
	wcsrchr.o wcsspn.o wcstol.o wcstoul.o wcstombs.o wcswcs.o \
	wctomb.o creat.o \
	fgetws.o wcstod.o \
	fputws.o \
	auth_none.o auth_unix.o authunix_prot.o clnt_generic.o \
	clnt_perror.o clnt_raw.o clnt_simple.o clnt_tcp.o clnt_udp.o \
	dbx_rpc.o get_myaddress.o getrpcport.o key_call.o key_prot.o \
	netname.o openchild.o pmap_clnt.o pmap_getmaps.o \
	pmap_getport.o pmap_prot.o pmap_prot2.o pmap_rmt.o \
	rpc_callmsg.o rpc_prot.o rpccommondata.o rpcdtablesize.o rtime.o \
 	svc.o svc_auth.o svc_auth_unix.o svc_raw.o \
	svc_run.o svc_simple.o svc_tcp.o svc_udp.o xdr.o \
	xdr_array.o xdr_float.o xdr_mem.o xdr_rec.o \
	xdr_reference.o xdr_stdio.o \
	getservent_l.o getsvc.o getnetent_l.o getprotoent.o \
	getnetgrent.o gethostent.o gethostent_l.o \
	getcommon.o ypxdr.o yp_all.o yp_bind.o yp_enum.o yp_master.o \
	yp_match.o yp_order.o yperr_string.o ypmaint_xdr.o \
	ypprot_err.o ypupdate_prot.o ypv1_xdr.o \
	getgroup.o getpasswd.o login.o reentrant_stubs.o \
	getpmsg.o putpmsg.o fattach.o fdetach.o grantpt.o isastream.o \
	ptsname.o unlockpt.o priocntl.o sigsend.o _getucontext.o \
	setcontext.o fix_context.o statvfs.o tcgetsid.o sigpause.o sigset.o

SIAOBJS=sia_globals.o sia_init.o sia_mat_init.o sia_s_init.o sia_mk_en_pw.o \
	sia_log.o sia_alloc_en.o sia_free_ent.o \
	sia_s_auent.o sia_s_estab.o sia_s_lnch.o \
	sia_s_suauth.o sia_c_pass.o sia_c_finger.o \
	sia_c_shell.o sia_s_reauth.o sia_auth.o \
	sia_coll_trm.o sia_c_invkr.o sia_s_rel.o \
        siad_init.o siad_s_init.o siad_s_auth.o siad_s_estab.o \
	siad_s_lnch.o siad_s_suaut.o siad_c_pass.o \
	siad_c_fngr.o siad_c_shell.o siad_s_reaut.o \
	siad_c_user.o siad_c_invk.o siad_s_rel.o \
	sia_switch.o siad_getgrp.o siad_getpass.o \
	siad_pw_adm.o sia_getgroup.o sia_getpass.o siad_gr_adm.o \
	sia_getmsg.o sia_chdir.o

ILSOBJS= 	  __lc_load.o __mbstopcs.o \
		  __mbtopc.o \
		  __pcstombs.o \
		  __pctomb.o \
		  __getmbcurmax.o \
		  __fnmatch_sb.o __fnmatch_std.o fnmatch.o \
		  NLSsetup.o \
		  colval.o \
		  forw_pos.o forward.o \
		  wctype.o glob.o iswctype.o \
		  __iswctype_std.o \
		  iswalnum.o iswalpha.o iswcntrl.o iswdigit.o \
		  iswgraph.o iswlower.o iswprint.o iswpunct.o \
		  iswspace.o iswupper.o iswxdigit.o \
		  mbswidth.o nl_langinfo.o \
		  regcomp.o \
		  __regexec_std.o regexec.o regfree.o rpmatch.o \
		  __rpmatch_std.o \
		  towlower.o \
		  towupper.o \
		  wcscmp.o wcscoll.o \
		  wcscpy.o \
		  wcsncmp.o \
		  wcswidth.o \
		  wcsxfrm.o \
		  wcwidth.o \
		  mbsadvance.o mbsinvalid.o mbstomb.o \
		  character.o wordexp.o \
		  ${ILS_REENTRANT_OBJS}
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


assert.o getopt.o getcwd.o herror.o \
initgroups.o perror.o psignal.o random.o regex.o \
regcomp.o __regcomp_std.o regerror.o regexec.o __regexec_std.o \
setjmper.o strerror.o syslog.o: libc_msg.h

getpasswd.o getgroup.o: siad.h sia.h

${SIAOBJS}: $${@:.o=.c} sia.h siad.h siad_bsd.h libc_msg.h
	${_CC_} -Dconst=  ${_CCFLAGS_} -c $*.c

${STDOBJS}: $${@:.o=.c}
	${_CC_} -Dconst=' ' ${_CCFLAGS_} -c $*.c
#	${_CC_} -Dconst=' '  ${_CCFLAGS_} -c -v   -E $*.c
#	${LD} -x -r $*.o
#	${MV} -f a.out $*.o

.c.o:
	${_CC_} -Dconst=' ' -c  ${_CCFLAGS_} $*.c
#	${_CC_} -Dconst=' ' -c -v   -E ${_CCFLAGS_} $*.c
#	${LD} ${_LDFLAGS_} -x -r $*.o
#	${MV} -f a.out $*.o

COFF_LORDER=`lorder *.o | tsort`
A_OUT_LORDER=${OFILES}
MACHO_LORDER=${OFILES}

tags:
	ctags *.c ${target_machine}/*.c

#
#  Added pollution_check to build rules
#
libc.a: ${DEPEND_LIST} ${LIBRARIES}(${_OFILES_})
	${_AR_} ${DEF_ARFLAGS} $@ $?
	${_RANLIB_} $@
	${RM} -f $?
	ksh -p `${GENLOC} /src/usr/ccs/lib/libc/pollution_check` $@

