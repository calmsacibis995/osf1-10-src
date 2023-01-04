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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.5.9 $ (DEC) $Date: 1994/01/10 23:27:24 $
#


LIBC                    = ${MAKETOP}usr/ccs/lib/libc
VPATH                   = ${target_machine}:${LIBC}:${LIBC}/${target_machine}:${LIBC}/SIA

LIBRARIES               = libc.a
MSGHDRS                 = libc_msg.h
CATFILES                = libc.cat

EXPLIBC_TARGETS         = export_libc.a

ILIST                   = libc.a
IDIR                    = /usr/opt/alt/usr/lib/
IMODE                   = 644


USE_STATIC_LIBRARIES    =

GLINE                   =
INCFLAGS                = -I${target_machine} -I${LIBC}/SIA \
                          -I${LIBC} -I${LIBC}/${target_machine} 
CFLAGS                  = -DCOMPAT_43  -DLIBC_SCCS  ${_NAME_SPACE_WEAK_STRONG} \
                          ${${TARGET_MACHINE}CFLAGS}

IEEE_CFLAGS             = -ieee_with_no_inexact ${CFLAGS}
IEEE_DRM_CFLAGS         = $(IEEE_CFLAGS) -fprm d

HFILES                  = sia.h siad.h siad_bsd.h
scandir.o_CFLAGS        = -D_BSD ${CFLAGS}
wait3.o_CFLAGS          = -D_BSD ${CFLAGS}
key_call.o_CFLAGS       = -D_BSD ${CFLAGS}
kls_ipc.o_CFLAGS        = -D_KLS_IPC_SOCK_DGRAM ${CFLAGS}

#
# scanf(), fscanf(), sscanf(), sprintf(), fread() and fwrite()
# also need to be built in ANSI C mode as indicated above.
#
scanf.o_CFLAGS 		= -std1 ${CFLAGS}
sprintf.o_CFLAGS	= -std1 ${CFLAGS}
fread.o_CFLAGS		= -std1 ${CFLAGS}
fwrite.o_CFLAGS		= -std1 ${CFLAGS}
dtoa_ieee.o_CFLAGS      = $(IEEE_DRM_CFLAGS)

OFILES			= ${STDOBJS} \
			  ${OBJ64} \
			  ${SIAOBJS} \
			  ${${TARGET_MACHINE}SOBJS} \
			  ${${TARGET_MACHINE}COBJS}
GARBAGE			= *.S *.as tags

STDOBJS= NCchrlen.o data.o NCdec.o NLchrlen.o NLgetamsg.o abort.o assert.o \
         atof.o atoi.o bsearch.o calloc.o catgets.o catopen.o closedir.o \
         ctime.o doprnt.o  ecvt.o exec_args.o execvp.o exit.o \
         findiop.o flsbuf.o fopen.o fprintf.o fputs.o fread.o fseek.o ftell.o \
         getenv.o getwd.o isalnum.o isatty.o isdigit.o isspace.o isupper.o \
         ldr_dummy2.o ldr_status.o malloc.o memccpy.o mktemp.o nl_langinfo.o \
         perror.o raise.o readdir.o setjmper.o sigcompat.o signal.o sigops.o \
         sprintf.o strchr.o strdup.o strncmp.o strncpy.o strrchr.o system.o \
         tzset.o ungetc.o wait.o waitpid.o filbuf.o  fwrite.o ldr_atexit.o \
         opendir.o sleep.o time.o NCenc.o errlst.o liblocale.o printf.o \
         popen.o fputc.o strncat.o isprint.o qsort.o puts.o errno.o \
	 __getmbcurmax.o  localeconv.o wctomb.o wcslen.o \
	 wcstombs.o iswctype.o setlocale.o NLSsetup.o __lc_load.o fgets.o \
	 __ispriv.o basename.o libdl_dummy.o getcwd.o sigpause.o sigset.o

SIAOBJS=

OBJ64= 


-include ${target_machine}/machdep.mk
include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk


assert.o crt0msg.o getopt.o getwd.o initgroups.o \
perror.o psignal.o random.o regex.o setjmper.o syslog.o: libc_msg.h

${SIAOBJS}: $${@:.o=.c}
	${_CC_} -Dconst=  -DCRYPT ${_CCFLAGS_} -32addr -c $*.c

${OBJ64}: $${@:.o=.c}
	${_CC_} -Dconst=  -DCRYPT ${_CCFLAGS_} -c $*.c

${STDOBJS}: $${@:.o=.c}
	${_CC_} -Dconst=' ' -DCRYPT ${_CCFLAGS_} -32addr -c $*.c
#	${_CC_} -Dconst=' '  ${_CCFLAGS_} -c -v   -E $*.c
#	${LD} -x -r $*.o
#	${MV} -f a.out $*.o

.c.o:
	${_CC_} -Dconst=' ' -c  -DCRYPT ${_CCFLAGS_} $*.c
#	${_CC_} -Dconst=' ' -c -v   -E ${_CCFLAGS_} $*.c
#	${LD} ${_LDFLAGS_} -x -r $*.o
#	${MV} -f a.out $*.o

COFF_LORDER=`lorder *.o | tsort`
A_OUT_LORDER=${OFILES}
MACHO_LORDER=${OFILES}

tags:
	ctags *.c ${target_machine}/*.c

