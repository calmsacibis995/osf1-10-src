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
#	@(#)$RCSfile: template.mk,v $ $Revision: 4.6.25.4 $ (DEC) $Date: 1993/05/24 17:34:58 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# Copyright (c) 1988 Carnegie-Mellon University
# Copyright (c) 1987 Carnegie-Mellon University
# Copyright (c) 1986 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#  
#
# OSF/1 Release 1.0

#
# Copyright (c) 1980, 1986 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#

#
#  This must be here before any rules are possibly defined by the
#  machine dependent makefile fragment so that a plain "make" command
#  always works.  The config program will emit an appropriate rule to
#  cause "all" to depend on every kernel configuration it generates.
#

VPATH=..

#
# Set up values that need to work in ODE as well as outside 
# of ODE.
#

alpha_CC_SUITE	= ACC
alpha_cc_suite  = acc
mips_CC_SUITE	= MCC
mips_cc_suite  	= mcc
alpha_TARGET_MACHINE	= ALPHA
mips_TARGET_MACHINE	= MIPS
alpha_CURRENT_MACHINE	= ALPHA
mips_CURRENT_MACHINE	= MIPS

target_machine = ${target_machine?${target_machine}:${MACHINE}}
current_machine = ${current_machine?${current_machine}:${MACHINE}}
#
# Set upper case values.
TARGET_MACHINE = ${${target_machine}_TARGET_MACHINE}
CURRENT_MACHINE = ${${current_machine}_CURRENT_MACHINE}

CC_SUITE = ${CC_SUITE?${CC_SUITE}:${${target_machine}_CC_SUITE}}
cc_suite = ${cc_suite?${cc_suite}:${${target_machine}_cc_suite}}

#  Shortened for macro definitions.
_C_M_ = ${CURRENT_MACHINE}
_T_M_ = ${TARGET_MACHINE}


# 
# Determine development environment, based on current machine and
# target machine.  The default is a native development environment.
#
# ${CURRENT_MACHINE}_${TARGET_MACHINE}_DEVENV

MIPS_ALPHA_DEVENV = CROSS
DEVENV = ${${_C_M_}_${_T_M_}_DEVENV?${_C_M_}_${_T_M_}_DEVENV:NATIVE}

build_all: all

_build_prefix_%: %

cpu=arch/${target_machine}

#
#  In order to move the link commands out of config and back into the
#  template makefile, config has been changed to emit a dendency rule
#  of the form *vmunix.sys: *vmunix.swap for each configuration so that
#  the common commands for building each configuration can be placed
#  directly in the template in the default rule for the .sys.swap
#  dependency.
#
.SUFFIXES: .swap .sys

#
#  Make sure we don't remove this by accident if interrupted at the wrong
#  time.
#
.PRECIOUS: Makefile

#
#  Tools needed
#
AWK=		awk
BASENAME=	basename
CAT=		cat
CD=		cd
CP=		cp
CSPLIT=		csplit
CTAGS=		ctags
DIRNAME=	dirname
ECHO=		echo
ED=		ed
EGREP=		egrep
ETAGS=		etags
EX=		ex
GREP=		grep
KSH=		ksh
LINT=		lint
LN=		ln
LS=		ls
MAKE=		make
MKDIR=		mkdir
MV=		mv
RM=		rm -f
SED=		sed
SHELL=		sh
SORT=		sort
TOUCH=		touch
UNIQ=		uniq

#
#  Compilation components
#

# Contrain the search path of CC, CPP and C2 instead of absolute paths.

OBJECT_FORMAT = ${OBJECT_FORMAT?${OBJECT_FORMAT}:COFF}
_CC_EXEC_PREFIX = ${${OBJECT_FORMAT}_${CC_SUITE}_EXEC_PREFIX}
CC=	${_CC_EXEC_PREFIX}cc
CPP=	${CC} -E
LD=	${_CC_EXEC_PREFIX}ld
NM=	${_CC_EXEC_PREFIX}nm

ENCODE=	uuencode
DECODE=	uudecode

GCC_VOLATILE = 
MCC_VOLATILE ="-Dvolatile= "
ACC_VOLATILE ="-Dvolatile= "
VOLATILE=${${CC_SUITE}_VOLATILE}
FORCE_VOLATILE=

VMUNIX_LIB=

#
#  Pre-processor environment
#
DEPENDS= -MD 
INCLUDES= -I -I. -I.. -I../include
ALLDEFINES= -DKERNEL -D_KERNEL -D_BSD
ALLOPTS= ${INCLUDES} ${IDENT} ${ALLDEFINES}
LINTOPTS=${ALLOPTS} ${VOLATILE} -DBINARY

#
#  Common C-compilation flags
#
#  To build a DEBUGable kernel define CDEBUGOPTS=-g in config file
# 

MCC_OPTS = -std0 -EL -signed
ACC_OPTS = -signed
GCC_OPTS =

COPTS_P=${CDEBUGOPTS} ${DEPENDS} ${ALLOPTS} ${${CC_SUITE}_OPTS}

ACC_CFLAGS = -no_excpt -Wb,-static -Wco,-nofloat -Olimit 1000
ACC_CFOPTS = -no_excpt -Wb,-static -Olimit 1000
MCC_CFLAGS =
MCC_CFOPTS =

CFLAGS = ${COPTS_P} ${${@}_MIG?-Imach:} ${${CC_SUITE}_CFLAGS}
COPTS  = ${COPTS_P} ${${@}_MIG?-Imach:} ${${CC_SUITE}_CFLAGS}
CFOPTS  = ${COPTS_P} ${${@}_MIG?-Imach:} ${${CC_SUITE}_CFOPTS}

#
#  Common assembly flags

SFLAGS=	${DEPENDS} ${ALLOPTS} ${${CC_SUITE}_OPTS} -DASSEMBLER

#
#  Path name of the version identification file.

FEATURES=${cpu}/FEATURES.h
VERSION_FILES= \
	conf/version.type \
	conf/version.major \
	conf/version.minor \
	conf/version.variant \
	conf/version.build \
	conf/version.patch

#  LOBJS is a list of files that are unique to builds from BINARY systems.
#  That is, systems which do not have access to source code.  These files
#  are the files which will be compiled on each system because they tend
#  to be "tuned" to the specific system.
# For alpha - afd - note: scb is in machine/template.mk with entry.o
#	config program makes scb_vec.c for mips
ALPHA_LOBJS=	param.o ioconf.o
MIPS_LOBJS=	param.o ioconf.o scb_vec.o

LOBJS=${${_T_M_}_LOBJS}

MIPS_LCFILES= ioconf.c param.c scb_vec.c
ALPHA_LCFILES= ioconf.c param.c

LCFILES=${${_T_M_}_LCFILES}

#
#  LDOBJS is the set of object files which comprise the kernel.
#  It is used both in the dependency list for each *vmunix.swap
#  rule emitted by config and also in the .sys.swap rule
#  below which links the kernel from these objects.
#  
#  LDOBJS_PREFIX and LDOBJS_SUFFIX are defined in the machine
#  dependent Makefile (if necessary).

LDOBJS=${LDOBJS_PREFIX} ${OBJS} ${LOBJS} ${LDOBJS_SUFFIX}

#
#  LDDEPS is the set of extra dependencies associated with each
#  *vmunix.swap rule (in addition to $LDOBJS and the swap*.o file).
#
#  LDDEPS_PREFIX is defined in the machine dependent Makefile
#  (if necessary).

LDDEPS=${LDDEPS_PREFIX} ${VERSION_FILES} \
	conf/version.banner conf/newvers.sh conf/copyright

#
#  PRELDDEPS is another set of extra dependencies associated with each
#  *vmunix.swap rule (in addition to $LDOBJS and the swap*.o file).
#  It is defined in the machine dependent Makefile (if necessary).
#
#  The generated rule looks like
#	vmunix.sys : ${PRELDDEPS} ${LDOBJS} ${LDDEPS}
#

#
#  SWAPDEPS is the set of extra dependencies associated with each
#  swap*.o rule emitted by config (in addition to the
#  ../${cpu}/swap*.c file).
#
#  SWAPDEPS_PREFIX is defined in the machine dependent Makefile
#  (if necessary).
#
SWAPDEPS=${SWAPDEPS_PREFIX} 

#
#  SYSDEPS is the set of extra dependencies associated with each
#  *vmunix rule (in addition to *vmunix.sys).
#
#  SYSDEPS_PREFIX is defined in the machine dependent Makefile
#  (if necessary).
#
SYSDEPS=${SYSDEPS_PREFIX}



#
#  Compilation rules to generate *vmunix from *vmunix.sys
#
SYS_RULE_1=@eval `${AWK} 'NR==1{S=$$1;next;};\
END {\
  print "rm -f $@";\
  C = "ln $@.sys $@";\
  if (S != "") {\
    C = C "; ln $@ $@" S; \
    printf "rm -f $@%s; ", S; \
  }\
  printf "echo \"%s\"; %s\n", C, C; \
}' vers.suffix`
SYS_RULE_2=
SYS_RULE_3=
SYS_RULE_4=

#
# Compilation rules to generate .o from .b
#
B_RULE_1A=${RM} $*.o; ${DECODE} 
B_RULE_1B=$*.b

#
# Compilation rules to generate .o from .o
#
O_RULE_1A=${RM} $*.o; cp 
O_RULE_1B=$*.o .

#
# Compilation rules to generate .o from .s
#
S_RULE_1A=${KCPP} ${SFLAGS} 
S_RULE_1B=$*.s >$*.i 
S_RULE_2=${AS} -o $*.o $*.i
S_RULE_3=${RM} $*.i

#
# Compilation rules to generate .o from .c for normal files
#
C_RULE_1A=${KCC} -c ${CFLAGS}
C_RULE_1B=$*.c
C_RULE_2=
C_RULE_3=
C_RULE_4=

#
# Compilation rules to generate .o from .c for driver files
#
C_RULE_1A_D=${C_RULE_1A}
C_RULE_1B_D=${C_RULE_1B}
C_RULE_2_D=${C_RULE_2}
C_RULE_3_D=${C_RULE_3}
C_RULE_4_D=${C_RULE_4}

#
# Compilation rules to generate .o from .c for profiling routine files
#
C_RULE_1A_P=${KCC} -c -S ${COPTS_P} 
C_RULE_1B_P=${C_RULE_1B}
C_RULE_2_P=ex - $*.s < ${GPROF.EX}
C_RULE_3_P=${AS} -o $*.o $*.s
C_RULE_4_P=${RM} $*.s


gprof: kgmon_on all kgmon_off

kgmon_on:
	kgmon -r -b

kgmon_off:
	kgmon -h -p
	gprof /mach >GPROF

MIPS_TEXTBASE=${LOADADDR?${LOADADDR}:80030000}
ALPHA_TEXTBASE=${LOADADDR?${LOADADDR}:ffffffff00000000}
TEXTBASE=${${_T_M_}_TEXTBASE}

#
#  Theses macros are filled in by the config program depending on the
#  current configuration.  The MACHDEP macro is replaced by the
#  contents of the machine dependent makefile template and the others
#  are replaced by the corresponding symbol definitions for the
#  configuration.
#

%OBJS

%CFILES

%SFILES

%BFILES

%ORDERED

%MACHDEP

CFILES=${CFILES1} ${CFILES2} ${CFILES3} ${CFILES4}

#
#
# echo the list of CFILES used to build the system
#
#
cfiles:
	@${ECHO} ${CFILES}

objfiles:
	@${ECHO} ${OBJS}
#
#
# export flags for use by kernel lib makefiles
#
#
asflags:
	@${ECHO} ${CCASFLAGS}

ccnflags:
	@${ECHO} ${CCNFLAGS}
#
#  Default rule used to build a *vmunix.sys configuration from the
#  object list and a particular *vmunix.swap module.  The *vmunix.swap
#  module is a normal object file compiled from the appropriate swap*.c
#  file and then copied to the *vmunix.swap name to trigger the full
#  kernel link using this default rule.
#

#
#  OBJSDEPS is the set of files (defined in the machine dependent
#  template if necessary) which all objects depend on (such as an
#  in-line assembler expansion filter
#

${OBJS}: ${OBJSDEPS}



#
#  This macro is replaced by three rules per configuration, plus a
#  final definition for the LOAD symbol consisting of all configuration
#  names and a dependency of "all" on this symbol (so that the
#  "default" rule above will build everything).  The three rules in
#  each configuration are:
#
#  - a *vmunix from *vmunix.sys dependency using the SYS_RULE_* rules,
#  - a *vmunix.swap from LDDEPS, swap*.o, and LDDEPS dependency that
#    simply copies the swap file to the target name, and
#  - a swap*.o from swap*.c and SWAPDEPS dependency using the C_RULE_* rules.
#

%LOAD

#
# Special rules to make LOBJS
#
#
#  Rules for components which are not part of the kernel proper or that
#  need to be built in a special manner.
#


ioconf.o: ioconf.c
	-${RM} ioconf.o
	${KCC} ${CCNFLAGS} ioconf.c

param.c: conf/param.c
	-${RM} param.c
	${CP} conf/param.c .

param.o: param.c
	-${RM} param.o
	${KCC} ${CCNFLAGS} ${PARAM} param.c

scb_vec.o: scb_vec.c
	-${RM} scb_vec.o
	${KCC} ${CCNFLAGS} scb_vec.c

scalls: ${SYSENT}

HOST_CC		= env - COMP_HOST_ROOT=/ COMP_TARGET_ROOT=/ /bin/cc

ACC_GENASSYM_CFLAGS = -no_excpt
MCC_GENASSYM_CFLAGS =

GENASSYM_CFLAGS = -std0 -EL ${ALLOPTS} ${VOLATILE} ${${CC_SUITE}_GENASSYM_CFLAGS}

CROSS_genassym.o: ${cpu}/genassym.c
	${CC} ${GENASSYM_CFLAGS} -c ${cpu}/genassym.c

NATIVE_genassym.o: ${cpu}/genassym.c
	${HOST_CC} ${GENASSYM_CFLAGS} -c ${cpu}/genassym.c

#
# Use macros to get the correct command to build and run genassym
# for multiarchitecutures and cross development environments.
# 

NATIVE_RUN_GENASSYM	= ./genassym > assym.s
CROSS_RUN_GENASSYM	= /bin/sh ${TARGET_EXEC_PREFIX}/run-genassym
RUN_GENASSYM		= ${${DEVENV}_RUN_GENASSYM}

CROSSENVPATH		= ${COMP_HOST_ROOT}/usr/lib/cmplrs/${cc_suite}

MIPS_NATIVE_GENASSYM	= ${HOST_CC} -std0 -EL -o genassym genassym.o
ALPHA_NATIVE_GENASSYM	= ${HOST_CC} -std0 -EL -non_shared -o genassym genassym.o
ALPHA_CROSS_GENASSYM	= ${LD} -o genassym -L -L${CROSSENVPATH} \
				${CROSSENVPATH}/crt0_sa.o genassym.o \
				${CROSSENVPATH}/libc_sa.a -N -T 20000000

GENASSYM 		= ${${TARGET_MACHINE}_${DEVENV}_GENASSYM}

assym.s: ${DEVENV}_genassym.o
	${GENASSYM}
	${RUN_GENASSYM}
	@${RM} genassym
	@${LN} -fs genassym.o ${DEVENV}_genassym.o

locore.o: assym.s ${LOCORE_HEADS}

#
#  Mach IPC-based interfaces
#

MACH_DEFSFILES=\
	mach/std_types.defs \
	mach/mach_types.defs \
	mach/mach.defs \
	mach/mach_host.defs \
	mach/exc.defs \
	mach/memory_object.defs \
	mach/memory_object_default.defs \
	mach/device_types.defs \
	mach/device.defs \
	mach/monitor.defs

MACH_GENFILES=\
	./mach/mach_server.c \
	./mach/mach_user_internal.h \
	./mach/mach_interface.h \
	./mach/mach_host_server.c \
	./mach/mach_host.h \
	./mach/memory_object_server.c \
	./mach/memory_object_user.c \
	./mach/memory_object_user.h \
	./mach/memory_object_default_server.c \
	./mach/memory_object_default_user.c \
	./mach/memory_object_default.h \
	./mach/device_server.c \
	./mach/exc_server.c \
	./mach/exc_user.c \
	./mach/exc.h \
	./mach/monitor_server.c \
	./mach/monitor.h

MACH_MKFILE=	mach/Makefile

$(MACH_GENFILES) : migfiles

migfiles :: $(MACH_DEFSFILES) $(MACH_MKFILE)
	${RM} -rf ./mach
	${MKDIR} ./mach
	${CD} ./mach && \
	 ${MAKE} ${MFLAGS} VPATH=../../mach && ${TOUCH} ../migfiles

MACH_DEBUG_DEFSFILES=\
	mach_debug/mach_debug_types.defs \
	mach_debug/mach_debug.defs

MACH_DEBUG_GENFILES=\
	./mach_debug/mach_debug_server.c

MACH_DEBUG_MKFILE=	mach_debug/Makefile

$(MACH_DEBUG_GENFILES) : migdebugfiles

migdebugfiles :: $(MACH_DEBUG_DEFSFILES) $(MACH_DEBUG_MKFILE)
	${RM} -rf ./mach_debug
	${MKDIR} ./mach_debug
	${CD} ./mach_debug && \
	 ${MAKE} ${MFLAGS} VPATH=../../mach_debug && ${TOUCH} ../migdebugfiles

MIG_ALL_FILES = migfiles migdebugfiles

#
#  Update the Makefile when any dependency files exist rather than only
#  when they are newer than the makefile (since it often gets touched
#  during reconfigurations when dependency files have yet to be
#  integrated).  The makefile is copied before doing any potential
#  integration so that the new and old versions can be compared when
#  done (by the SYS_RULE_* rules) to prevent re-building things when no
#  dependencies change.
#
#  N.B.  There is always an empty artificial dependency file M.d
#  (created by config) which will never be integrated and simply
#  prevents the wild-card expansion below from failing.
#
#  The default exit rule now brings the Makefile up to date, so it
#  shouldn't be necessary to execute this rule manually.
#

.EXIT:

DEPFILES= ${cpu}/Locore.c ${cpu}/genassym.c ${cpu}/swapgeneric.c \
		${LCFILES} assym.s ${CFILES} ${SFILES} 

depend: makedep makelint 
	${ECHO} '/^# DO NOT DELETE THIS LINE/+1,$$d' >eddep
	${ECHO} '$$r makelint' >>eddep;
	${ECHO} '$$r makedep' >>eddep
	${ECHO} 'w' >>eddep
	${CP} Makefile Makefile.bak
	${EX} - Makefile < eddep
	${RM} eddep makedep1 makedeperrs

makedep: makedepfiles
	${CAT} /dev/null >makedeptmp
	${CAT} /dev/null >maketagstmp
	-(${CSPLIT} -ks -fmakedeptmp makedepfiles 100 {25} 1>/dev/null 2>&1; exit 0)
	-for i in makedeptmp?? ; do \
		(for f in `${CAT} $$i` ; do \
			${CC} -M ${ALLOPTS} ${DEFINES} $$f | \
			${AWK} '{ print $$0 >> "makedeptmp" }; \
			        { print $$2 >> "maketagstmp" }'; \
		done) 2>makedeperrs; \
	done
	${SORT} -u maketagstmp | ${GREP} -v '\.s' > maketagfiles
	${RM} maketagstmp
	-${SED} -e 's+\.\./.*/src/kernel/++' \
		-e 's+/.*/src/kernel/++' \
		-e '/\.\.\/.*\.h/ b'  \
		-e 's+\.\./++' makedeptmp | \
		${SED} -e 's+^\(.*\)\.o:+\1.o \1.ln:+' | \
			${SORT} -u > makedep1
	${RM} makedeptmp*
	${AWK} ' { if ($$1 != prev) { \
			print rec; rec = $$0; prev = $$1; \
			if ( NR > 1 ) print "" \
		} else { \
			if (length(rec $$3) > 78) { \
				print rec; rec = $$0; \
			} else \
				rec = rec " " $$3 \
			} } \
		END { print rec } ' makedep1 > makedep
	${CAT} makedeperrs
	(if [ -s makedeperrs ]; then false; else true; fi)

makedepfiles: ${DEPFILES}
	${CAT} /dev/null > makedepfiles
	-for i in ${cpu}/Locore.c ${cpu}/genassym.c ${cpu}/swapgeneric.c \
			${LCFILES} assym.s ${SFILES} ; do \
		echo "$$i" >> makedepfiles; \
	done
	-for i in ${CFILES1} ; do \
		echo "$$i" >> makedepfiles; \
	done
	-for i in ${CFILES2} ; do \
		echo "$$i" >> makedepfiles; \
	done
	-for i in ${CFILES3} ; do \
		echo "$$i" >> makedepfiles; \
	done
	-for i in ${CFILES4} ; do \
		echo "$$i" >> makedepfiles; \
	done

LINTFILES=
LINTFLAGS= -n -whpCDPR -XE -Nn6000 -Nd8000 ${LINTOPTS}
LINTFILTER= ${GREP} -v 'struct/union .* never defined' | \
	    ${GREP} -v 'possible pointer alignment problem'

makelint: makelintfiles
	${CAT} /dev/null > makelint
	${AWK} 'BEGIN { printf("\nLINTOBJS=") } \
		{ printf(" \\\n\t%s.ln", $$2) } \
		END { printf("\n\n") }' makelintfiles >> makelint
	${AWK} 'BEGIN { DOL=36; \
		printf("lint:\t%c{LINTOBJS}\n", DOL); \
		printf("\t@%c{ECHO} \"%c{LINT} %c{LINTFLAGS} *.ln > linterrs 2>&1\"\n", DOL, DOL, DOL); \
		printf("\t@%c{LINT} %c{LINTFLAGS} %c{LINTOBJS} > linterrs 2>&1\n", DOL, DOL, DOL); \
		printf("\t%c{TOUCH} lint\n\n", DOL) }' /dev/null >> makelint
	${AWK} 'BEGIN { DOL=36; \
		printf("flint:\t%c{LINTFILES}\n", DOL); \
		printf("\t@%c{ECHO} \"%c{LINT} %c{LINTFLAGS} *.ln > linterrs 2>&1\"\n", DOL, DOL, DOL); \
		printf("\t@%c{LINT} %c{LINTFLAGS} %c{LINTOBJS} %c{LINTOBJS1} > linterrs 2>&1\n", DOL, DOL, DOL, DOL); \
		printf("\n") }' /dev/null >> makelint
	${AWK} '{ DOL=36; \
		printf("%s.ln: %s\n", $$2, $$1); \
		printf("\t-%c{LINT} -c %c{LINTFLAGS} %s | \\\n", DOL, DOL, $$1); \
		printf("\t\t(%c{LINTFILTER} ; exit 0) > %s.lint 2>&1\n\n", DOL, $$2) }' \
		makelintfiles >> makelint

makelintfiles: makedepfiles
	${CAT} /dev/null > makelinttmp
	-${GREP} "\.c" makedepfiles | (${CSPLIT} -ks -fmakelinttmp - 100 {25} 1>/dev/null 2>&1; exit 0)
	-for i in makelinttmp?? ; do \
		(for f in `${CAT} $$i` ; do \
			${ECHO} "$$f `${BASENAME} $$f .c`" >> makelinttmp; \
		done) \
	done
	-${SED} -e 's+^.*/src/kernel/++' \
		-e 's+\.\./++' makelinttmp | ${SORT} +1 -u > makelintfiles
	${RM} makelinttmp*

#
#  Target to generate ctags/etags files.
#

CTAGSFLAGS=	-wt
ETAGSFLAGS=	-wt

tags:	maketagfiles
	${CAT} /dev/null >tags
	-(${CSPLIT} -ks -fmaketagstmp maketagfiles 100 {25} 1>/dev/null 2>&1; exit 0)
	-for i in maketagstmp?? ; do \
		${CTAGS} -a ${CTAGSFLAGS} `${CAT} $$i`; \
	done
	-if [ -n ${ETAGS} ] && [ `${KSH} -c 'whence ${ETAGS}'` ]; then \
		${CAT} /dev/null >TAGS; \
		for i in maketagstmp??; do \
			${ETAGS} -ae ${ETAGSFLAGS} `${CAT} $$i`; \
		done \
	fi
	${RM} maketags*

maketagfiles: makedepfiles
	${CAT} /dev/null > maketagstmp
	-${GREP} "\.c" makedepfiles | (${CSPLIT} -ks -fmaketagstmp - 100 {25} 1>/dev/null 2>&1; exit 0)
	-for i in maketagstmp?? ; do \
		(for f in `${CAT} $$i` ; do \
			${CC} -M ${ALLOPTS} ${DEFINES} $$f | \
			${AWK} '{ print $$2 }' >> maketagstmp; \
		done) 2>/dev/null; \
	done
	${SORT} -u maketagstmp > maketagfiles
	${RM} maketagstmp*

#
#  Permit manually forced checkout of modules when the dependency
#  information is not current.
#

checkout:	${USE}



#
#  Clean up the current build directory.
#

clean:
	@for f in ${LOAD}; do ${ECHO} ${RM} $$f $$f.* && ${RM} $$f $$f.*; done
	${RM} eddep tags TAGS *.o locore.i \
		$(MIG_ALL_FILES) \
		errs linterrs makelinks *.log *LOG* ${CLEAN_EXTRA}
	${RM} makedep makedeperrs makedepfiles maketagfiles 
	${RM} makelint makelintfiles lint *.ln *.lint


MACHINEDIR=${cpu}

symbols.sort: ${MACHINEDIR}/symbols.raw
	-${GREP} -v '^#' ${MACHINEDIR}/symbols.raw \
	    | ${SED} 's/^	//' | ${SORT} -u > symbols.tmp
	${MV} -f symbols.tmp symbols.sort

#
#  This macro is replaced by the set of rules which build everything in
#  OBJS.
#

%DYNAMIC

%RULES


