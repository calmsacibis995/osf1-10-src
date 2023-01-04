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
#	@(#)$RCSfile: standard.mk,v $ $Revision: 4.5.22.10 $ (DEC) $Date: 1993/09/30 15:18:40 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

#
#  Default rule - All other rules appear after variable definitions
#
_default_: build_all;@

#
#  Debugging entry for checking environment
#
print_env:
	@printenv

#
#  Use this as a dependency for any rule which should always be triggered
#  (e.g. recursive makes).
#
ALWAYS=_ALWAYS_

#
#  Shortened for macro definitions, not to be used within a Makefile.
#
_T_M_=${TARGET_MACHINE}
_H_M_=${host_machine}
_H_OS_T_=${host_os_type}

#
#  Definitions for object file format - A_OUT, COFF or MACHO
#
_T_M_OBJECT_FORMAT_=${${_T_M_}_OBJECT_FORMAT?${${_T_M_}_OBJECT_FORMAT}:COFF}
OBJECT_FORMAT=${OBJECT_FORMAT?${OBJECT_FORMAT}:${_T_M_OBJECT_FORMAT_}}

#
#  Definitions for archive file format - ASCARCH or COFF
#
_T_M_ARCHIVE_FORMAT_=${${_T_M_}_ARCHIVE_FORMAT?${${_T_M_}_ARCHIVE_FORMAT}:COFF}
ARCHIVE_FORMAT=${ARCHIVE_FORMAT?${ARCHIVE_FORMAT}:${_T_M_ARCHIVE_FORMAT_}}

#
#  Set defaults for input variables which are not already defined
#
DEF_RMFLAGS=${DEF_RMFLAGS?${DEF_RMFLAGS}:-f}
DEF_ARFLAGS=${DEF_ARFLAGS?${DEF_ARFLAGS}:cr}
DEF_MDFLAGS=${DEF_MDFLAGS?${DEF_MDFLAGS}:-rm}

ROOT_OWNER=${ROOT_OWNER?${ROOT_OWNER}:root}
KMEM_GROUP=${KMEM_GROUP?${KMEM_GROUP}:kmem}
TTY_GROUP=${TTY_GROUP?${TTY_GROUP}:tty}
SETUID_MODE=${SETUID_MODE?${SETUID_MODE}:4711}
SETGID_MODE=${SETGID_MODE?${SETGID_MODE}:2711}

IOWNER=${IOWNER?${IOWNER}:${OWNER?${OWNER}:bin}}
IGROUP=${IGROUP?${IGROUP}:${GROUP?${GROUP}:bin}}
IMODE=${IMODE?${IMODE}:755}

#
#  Program macros
#
AR=ar
AS=as
AWK=awk
CC=cc
M4=m4
CHMOD=chmod
ULTRIX_CHOWN=/etc/chown
OSF1_CHOWN=/usr/bin/chown
CHOWN=${${_H_OS_T_}_CHOWN}
CHGRP=chgrp
CP=cp
CTAB=ctab
ECHO=echo
GENCAT=gencat
GENLOC=genloc
GENPATH=genpath
LD=ld
LIBLOC=libloc
LINT=${${OBJECT_FORMAT}_${CC_SUITE}_EXEC_PREFIX}lint
LN=ln
MAKEPATH=makepath
CD=cd
MKDIR=mkdir
MKSYMLINKS=mksymlinks
INSTDIR=instdir
MD=md
MIG=mig
MKCATDEFS=mkcatdefs
MV=mv
PC=pc
RANLIB=
RELEASE=release
RM=rm
SED=sed
SIZE=${${OBJECT_FORMAT}_${CC_SUITE}_EXEC_PREFIX}size
SORT=sort
TAGS=ctags
TOUCH=touch
TR=tr
XSTR=xstr
YACC=yacc

#
#  Define ${_T_M_}_VA_ARGV to be either VA_ARGV_IS_RECAST
#  to recast to char **, otherwise define VA_ARGV_IS_ROUTINE
#  If not defined here, we become VA_ARGV_UNKNOWN which should invoke
#  a #error directive where needed.
#
_HP_M68K_VA_ARGV=VA_ARGV_IS_RECAST
_HP300_VA_ARGV=VA_ARGV_IS_RECAST
_IBMRT_VA_ARGV=VA_ARGV_IS_RECAST
_MACII_VA_ARGV=VA_ARGV_IS_RECAST
_MMAX_VA_ARGV=VA_ARGV_IS_RECAST
_MIPS_VA_ARGV=VA_ARGV_IS_RECAST
_ALPHA_VA_ARGV=VA_ARGV_IS_RECAST
_SUN3_VA_ARGV=VA_ARGV_IS_RECAST
_SUN4_VA_ARGV=VA_ARGV_IS_RECAST
_SUN_VA_ARGV=VA_ARGV_IS_RECAST
_VAX_VA_ARGV=VA_ARGV_IS_RECAST
_AT386_VA_ARGV=VA_ARGV_IS_RECAST
${_T_M_}_VA_ARGV=${_${_T_M_}_VA_ARGV?${_${_T_M_}_VA_ARGV}:VA_ARGV_UNKNOWN}

#
#  Defined whether characters are sign or zero extended
#
_HP_M68K_CHAR_EXTEND=CHARS_EXTEND_ZERO
_HP300_CHAR_EXTEND=CHARS_EXTEND_SIGN
_IBMRT_CHAR_EXTEND=CHARS_EXTEND_ZERO
_MACII_CHAR_EXTEND=CHARS_EXTEND_SIGN
_MMAX_CHAR_EXTEND=CHARS_EXTEND_ZERO
_MIPS_CHAR_EXTEND=CHARS_EXTEND_ZERO
_ALPHA_CHAR_EXTEND=CHARS_EXTEND_SIGN
_SUN3_CHAR_EXTEND=CHARS_EXTEND_SIGN
_SUN4_CHAR_EXTEND=CHARS_EXTEND_SIGN
_SUN_CHAR_EXTEND=CHARS_EXTEND_SIGN
_VAX_CHAR_EXTEND=CHARS_EXTEND_SIGN
_AT386_CHAR_EXTEND=CHARS_EXTEND_SIGN
${_T_M_}_CHAR_EXTEND=${_${_T_M_}_CHAR_EXTEND?${_${_T_M_}_CHAR_EXTEND}:CHARS_EXTEND_UNKNOWN}

#
#  Define MAILSYSTEM for SITE somehow...
#
MAILSYSTEM=${MAILSYSTEM?${MAILSYSTEM}:SENDMAIL}

#
#  Security stuff

include security.mk

#
#  Default locale/language information
#
LOCALE=${LOCALE?${LOCALE}:C}
LOCALEPATH=${MAKETOP}usr/lib/nls/loc
MSGLANG=${MSGLANG?${MSGLANG}:en_US.ISO8859-1}
MSGLANGPATH=${MAKETOP}usr/lib/nls/msg/${MSGLANG}

#
#  Internationalization flag
#
I18N_FLAG=${I18N_FLAG?${I18N_FLAG}:NLS}

#
#  wchar_t CC flags
#

_WCHAR_T_LEN_1_DEFS_=-D__WCHAR_T_LEN=1 -Wf,-wchar8
_WCHAR_T_LEN_2_DEFS_=-D__WCHAR_T_LEN=2 -Wf,-wchar16 
_WCHAR_T_LEN_4_DEFS_=-D__WCHAR_T_LEN=4 -Wf,-wchar32 

WCHAR_T_LEN=${WCHAR_T_LEN?${WCHAR_T_LEN}:4}
_WCHAR_T_LEN_DEFS_=${_WCHAR_T_LEN_${WCHAR_T_LEN}_DEFS_}

#
#  Internationalization CC flags
#
_NLS_I18N_DEFS_=-DNLS
_KJI_I18N_DEFS_=-DKJI
_I18N_DEFS_=${_${I18N_FLAG}_I18N_DEFS_} ${_WCHAR_T_LEN_DEFS_}

#
#  Definitions for name space pollution
#
_NAME_SPACE_WEAK_STRONG=${NO_NAME_SPACE?:-D_NAME_SPACE_WEAK_STRONG}

#
#  Flags to get the loader to strip symbols
#
MCC_LDSTRIP=-s
ACC_LDSTRIP=-s -L -w2 
GCC_LDSTRIP=-%ld," -x" -%ld," -S"

REL	=	${${CC_SUITE}_REL}



LDSTRIP=${${CC_SUITE}_LDSTRIP}

#
#  Shared libraries definitions
#
_SET_USE_SHLIBS_=${NO_SHARED_LIBRARIES?:${SHARED_LIBRARIES?USE_SHARED_LIBRARIES:}}
${_SET_USE_SHLIBS_}=

#
#  C compiler variations
#
CCTYPE=${CCTYPE?${CCTYPE}:ansi}
_CCTYPE_=${${@}_CCTYPE?${${@}_CCTYPE}:${CCTYPE}}

_CC_EXEC_PREFIX_=${${OBJECT_FORMAT}_${CC_SUITE}_EXEC_PREFIX}

_host_AENV			=
_host_CENV			=
_host_MIGENV			=
_ansi_AENV			=	${${CC_SUITE}_AENV}
_ansi_CENV			=	${${CC_SUITE}_CENV}
_ansi_MIGENV			=	${${CC_SUITE}_MIGENV}
_traditional_AENV		=	${${CC_SUITE}_AENV}
_traditional_CENV		=	${${CC_SUITE}_CENV}
_traditional_MIGENV		=	${${CC_SUITE}_MIGENV}
_writable_strings_AENV		=	${${CC_SUITE}_AENV}
_writable_strings_CENV		=	${${CC_SUITE}_CENV}
_writable_strings_MIGENV	=	${${CC_SUITE}_MIGENV}

AENV = ${${CCTYPE}_AENV}
CENV = ${CENV?${CENV}:${${CCTYPE}_CENV}}
MIGENV = ${${CCTYPE}_MIGENV}
#
# If you don't have an ANSI compiler just set ${CC_SUITE}_STD_FLAG in 
# the rc_file.
#

STD_FLAG=${STD_FLAG?${STD_FLAG}:${${CC_SUITE}_STD_FLAG?${${CC_SUITE}_STD_FLAG}:-std0}}

# (-w means no warnings), use "${CC_SUITE}_WARN_FLAG= " to enable 
# compiler warnings from the rc_file.  
WARN_FLAG_DEFAULT=${WARN_FLAG_DEFAULT?${WARN_FLAG_DEFAULT}: }
# WARN_FLAG_DEFAULT=${WARN_FLAG_DEFAULT?${WARN_FLAG_DEFAULT}:-w}
WARN_FLAG=${${CC_SUITE}_WARN_FLAG?${${CC_SUITE}_WARN_FLAG}:${WARN_FLAG_DEFAULT}}


#
# The concept here is to build a executable that will run on the
# current machine.
#

HOST_ENV= env - COMP_HOST_ROOT=/ COMP_TARGET_ROOT=/

_mips_OSF1_CC_    = ${HOST_ENV} /usr/bin/cc
_mips_ULTRIX_CC_ = ${HOST_ENV} /usr/bin/cc 
_alpha_OSF1_CC_   = ${HOST_ENV} /usr/bin/cc

_host_CC_=${HOST_CC?${HOST_CC}:${_${_H_M_}_${_H_OS_T_}_CC_}} ${WARN_FLAG} -EL

_ansi_CC_=${ANSI_CC?${ANSI_CC}:${_CC_EXEC_PREFIX_}cc ${WARN_FLAG} ${STD_FLAG} -EL}
_traditional_CC_=${TRADITIONAL_CC?${TRADITIONAL_CC}:${_CC_EXEC_PREFIX_}cc -std0 -EL ${WARN_FLAG}}
_writable_strings_CC_=${WRITABLE_STRINGS_CC?${WRITABLE_STRINGS_CC}:${_ansi_CC_}}

_CC_=${_${_CCTYPE_}_CC_}

_AR_=${_CC_EXEC_PREFIX_}ar
_RANLIB_=${_AR_} ts

_host_PC_=${HOST_PC?${HOST_PC}:${_CC_EXEC_PREFIX_}pc} ${WARN_FLAG} -EL
_ansi_PC_=${ANSI_PC?${ANSI_PC}:${_CC_EXEC_PREFIX_}pc} ${WARN_FLAG} -EL
_PC_=${_${_CCTYPE_}_PC_}

_host_AS_=${HOST_AS?${HOST_AS}:${_CC_EXEC_PREFIX_}as} ${WARN_FLAG} -EL
_ansi_AS_=${ANSI_AS?${ANSI_AS}:${_CC_EXEC_PREFIX_}as} ${WARN_FLAG} -EL
_AS_=${_${_CCTYPE_}_AS_}


_host_LD_=${HOST_LD?${HOST_LD}:${_CC_EXEC_PREFIX_}ld}
_ansi_LD_=${ANSI_LD?${ANSI_LD}:${_CC_EXEC_PREFIX_}ld}
_traditional_LD_=${TRADITIONAL_LD?${TRADITIONAL_LD}:${_ansi_LD_}}
_writable_strings_LD_=${WRITABLE_STRINGS_LD?${WRITABLE_STRINGS_LD}:${_ansi_LD_}}
_LD_=${_${_CCTYPE_}_LD_}

_host_NOSTDINC_=
_ansi_NOSTDINC_=-I 
_traditional_NOSTDINC_=-I
_writable_strings_NOSTDINC_=-I
_CC_NOSTDINC_=${_${_CCTYPE_}_NOSTDINC_}

_host_GENINC_=`${GENPATH} -I.`
_ansi_GENINC_=`${GENPATH} -I.`
_traditional_GENINC_=`${GENPATH} -I.`
_writable_strings_GENINC_=`${GENPATH} -I.`
_CC_GENINC_=${_${_CCTYPE_}_GENINC_}

#
# _CMPLRS_??DEFS___ is defined in the compiler.mk
# makefile and is used only for compiler components.
#

_ALPHA_CCDEFS_=
_MIPS_CCDEFS_=-Dconst=

_CCDEFS_=-DBSD44 -DMSG ${_I18N_DEFS_} -DMACH -DCMU -D${SITE} \
	 -D${target_machine} -D__${target_machine}__ -D__${target_machine} \
	 -Dunix -D__unix__ \
	 ${_${TARGET_MACHINE}_CCDEFS_} ${_CMPLRS_CCDEFS_}

_PCDEFS_=${_CMPLRS_PCDEFS_}
_ASDEFS_=${_CMPLRS_ASDEFS_}

_SHCCDEFS_=${NO_SHARED_LIBRARIES?:${USE_SHARED_LIBRARIES?-D_SHARED_LIBRARIES:${USE_STATIC_LIBRARIES?:-D_SHARED_LIBRARIES}}}

_MIPS_MACHO_CFLAGS_=-G 0
_MMAX_MACHO_CFLAGS_=-mnosb
_O_F_CFLAGS_=${_${_T_M_}_${OBJECT_FORMAT}_CFLAGS_}

_mips_ULTRIX_CFLAGS= -D__${_H_M_}__ -D__MIPSEL__
_mips_OSF1_CFLAGS=
_alpha_OSF1_CFLAGS=

_host_CFLAGS_=${_${_H_M_}_${_H_OS_T_}_CFLAGS}
_ansi_CFLAGS_=${_O_F_CFLAGS_} ${_CCDEFS_} ${_SHCCDEFS_}
_traditional_CFLAGS_=${_ansi_CFLAGS_}
_writable_strings_CFLAGS_=${_ansi_CFLAGS_}
_CC_CFLAGS_=${_${_CCTYPE_}_CFLAGS_}

_PC_PFLAGS_= ${_PCDEFS_}

_AS_AFLAGS_= ${_ASDEFS_}

_mips_ULTRIX_INCDIRS= -I/usr/include
_mips_OSF1_INCDIRS= -I/usr/include
_alpha_OSF1_INCDIRS= -I/usr/include

_host_INCDIRS_=${HOST_INCDIRS?${HOST_INCDIRS}:${_${_H_M_}_${_H_OS_T_}_INCDIRS}}

# Note: if HOST_CC set, then pass no CFLAGS.
_host_CFLAGS_=${HOST_CC? :${_O_F_CFLAGS_} ${_CCDEFS_} ${_SHCCDEFS_}}
_ansi_INCDIRS_=${_CMPLRS_INCDIRS_} ${INCDIRS}
_traditional_INCDIRS_=${INCDIRS}
_writable_strings_INCDIRS_=${INCDIRS}
_CC_INCDIRS_=${_${_CCTYPE_}_INCDIRS_}

_host_GENLIB_=
_ansi_GENLIB_=
_traditional_GENLIB_=
_writable_strings_GENLIB_=
_CC_GENLIB_=${_${_CCTYPE_}_GENLIB_}

_host_LDFLAGS_=${HOST_LDFLAGS}
_ansi_LDFLAGS_=${_BLD?:-tr -h`${GENLOC} -d /export/${target_machine}/usr/lib/cmplrs/cc${REL}/crt0.o` -B}
_traditional_LDFLAGS_=${_ansi_LDFLAGS_}
_writable_strings_LDFLAGS_=${_ansi_LDFLAGS_}
_CC_LDFLAGS_=${_${_CCTYPE_}_LDFLAGS_}

_NON_SHARED_FLAG_=${${@}_LINKSHARED? : ${LINKSHARED? :-non_shared}}
_GLUE_=${NO_SHARED_LIBRARIES?${_NON_SHARED_FLAG_}:${USE_SHARED_LIBRARIES?-call_shared:${USE_STATIC_LIBRARIES?${_NON_SHARED_FLAG_}:-call_shared}}}
_mips_ULTRIX_GLUE= 
_mips_OSF1_GLUE= 
_alpha_OSF1_GLUE=${_GLUE_}
_host_GLUE_= ${_${_H_M_}_${_H_OS_T_}_GLUE}
_ansi_GLUE_=${_GLUE_}
_traditional_GLUE_=${_GLUE_}
_writable_strings_GLUE_=${_GLUE_}
_CC_GLUE_=${_${_CCTYPE_}_GLUE_}

_mips_ULTRIX_LIBDIRS= -L/usr/lib -L/lib
_mips_OSF1_LIBDIRS= -L/usr/lib -L/lib
_alpha_OSF1_LIBDIRS= -L/usr/lib -L/lib

_host_LIBDIRS_=${HOST_LIBDIRS?${HOST_LIBDIRS}:${_${_H_M_}_${_H_OS_T_}_LIBDIRS}}
_ansi_LIBDIRS_=${NO_SHARED_LIBRARIES?:${USE_SHARED_LIBRARIES?${SHLIBDIRS}:${USE_STATIC_LIBRARIES?:${SHLIBDIRS}}}} ${_CMPLRS_LIBDIRS_} ${LIBDIRS}
_traditional_LIBDIRS_=${_ansi_LIBDIRS_}
_writable_strings_LIBDIRS_=${_ansi_LIBDIRS_}
_CC_LIBDIRS_=${_${_CCTYPE_}_LIBDIRS_}

_PICLIB_=
_host_PICLIB_=
_ansi_PICLIB_=${_PICLIB_}
_traditional_PICLIB_=${_PICLIB_}
_writable_strings_PICLIB_=${_PICLIB_}
_CC_PICLIB_=${_${_CCTYPE_}_PICLIB_}

#
#  Compilation optimization level.  This should be set to whatever
#  combination of -O and -g flags you desire.
#
#  Allow these flags to be overridden per target
#
# OPT_LEVEL_DEFAULT=${OPT_LEVEL_DEFAULT?${OPT_LEVEL_DEFAULT}:-O}
OPT_LEVEL_DEFAULT=${OPT_LEVEL_DEFAULT?${OPT_LEVEL_DEFAULT}:-O2 -Olimit 5000}
CC_OPT_LEVEL=${CC_OPT_LEVEL?${CC_OPT_LEVEL}:${OPT_LEVEL_DEFAULT}}
_CC_OPT_LEVEL_=${${@}_CC_OPT_LEVEL?${${@}_CC_OPT_LEVEL}:${CC_OPT_LEVEL}}
_CC_OL_=${${@}_OPT_LEVEL?${${@}_OPT_LEVEL}:${OPT_LEVEL?${OPT_LEVEL}:${_CC_OPT_LEVEL_}}}
LD_OPT_LEVEL=${LD_OPT_LEVEL?${LD_OPT_LEVEL}:}
_LD_OPT_LEVEL_=${${@}_LD_OPT_LEVEL?${${@}_LD_OPT_LEVEL}:${LD_OPT_LEVEL}}
_LD_OL_=${${@}_OPT_LEVEL?${${@}_OPT_LEVEL}:${OPT_LEVEL?${OPT_LEVEL}:${_LD_OPT_LEVEL_}}}
PC_OPT_LEVEL=${PC_OPT_LEVEL?${PC_OPT_LEVEL}:${OPT_LEVEL_DEFAULT}}
_PC_OPT_LEVEL_=${${@}_PC_OPT_LEVEL?${${@}_PC_OPT_LEVEL}:${PC_OPT_LEVEL}}
_PC_OL_=${${@}_OPT_LEVEL?${${@}_OPT_LEVEL}:${OPT_LEVEL?${OPT_LEVEL}:${_PC_OPT_LEVEL_}}}
AS_OPT_LEVEL=${AS_OPT_LEVEL?${AS_OPT_LEVEL}:${OPT_LEVEL_DEFAULT}}
_AS_OPT_LEVEL_=${${@}_AS_OPT_LEVEL?${${@}_AS_OPT_LEVEL}:${AS_OPT_LEVEL}}
_AS_OL_=${${@}_OPT_LEVEL?${${@}_OPT_LEVEL}:${OPT_LEVEL?${OPT_LEVEL}:${_AS_OPT_LEVEL_}}}

#
#  Program flags for makefile, environment and command line args
#
_GENINC_=${_CC_GENINC_} `${GENPATH} ${${@}_INCARGS?${${@}_INCARGS}:${INCARGS}} ${${@}_INCFLAGS?${${@}_INCFLAGS}:${INCFLAGS}} ${${@}_INCENV?${${@}_INCENV}:${INCENV}}` ${_CC_INCDIRS_}
_GENLIB_=${_CC_GENLIB_} `${GENPATH} ${${@}_LIBARGS?${${@}_LIBARGS}:${LIBARGS}} ${${@}_LIBFLAGS?${${@}_LIBFLAGS}:${LIBFLAGS}} ${${@}_LIBENV?${${@}_LIBENV}:${LIBENV}}`
_LIBS_=${${@}_LIBSENV?${${@}_LIBSENV}:${LIBSENV}} ${${@}_LIBS?${${@}_LIBS}:${LIBS}} ${${@}_LIBSARGS?${${@}_LIBSARGS}:${LIBSARGS}} ${GLINE_LIBC?-lc_g:}
_CCFLAGS_=${_CC_CFLAGS_} \
	  ${_CC_OL_} \
	  ${${@}_CENV?${${@}_CENV}:${CENV}} \
	  ${${@}_CFLAGS?${${@}_CFLAGS}:${CFLAGS}} \
	  ${${@}_CARGS?${${@}_CARGS}:${CARGS}} \
	  ${_CC_NOSTDINC_} ${_GENINC_} ${_CC_PICLIB_}

_PCFLAGS_=${_PC_PFLAGS_} \
	  ${_PC_OL_} \
	  ${${@}_PENV?${${@}_PENV}:${PENV}} \
	  ${${@}_PFLAGS?${${@}_PFLAGS}:${PFLAGS}}  \
	  ${${@}_PARGS?${${@}_PARGS}:${PARGS}} \
	  ${_GENINC_}

_ASFLAGS_=${_AS_AFLAGS_} \
	  ${_AS_OL_} \
	  ${${@}_AENV?${${@}_AENV}:${AENV}} \
	  ${${@}_AFLAGS?${${@}_AFLAGS}:${AFLAGS}}  \
	  ${${@}_AARGS?${${@}_AARGS}:${AARGS}} \
	  ${_GENINC_}

_COMMON_LDFLAGS_=${_CC_LDFLAGS_} ${_LD_OL_} ${${@}_LD_OPT_LEVEL?${${@}_LD_OPT_LEVEL}:${LD_OPT_LEVEL}} ${${@}_LDENV?${${@}_LDENV}:${LDENV}} ${${@}_LDFLAGS?${${@}_LDFLAGS}:${LDFLAGS}} ${${@}_LDARGS?${${@}_LDARGS}:${LDARGS}} ${_GENLIB_} ${_CC_LIBDIRS_}
_LDFLAGS_=${_CC_GLUE_} ${_COMMON_LDFLAGS_}

#
#  Define REGISTRY_OPTION to -update_registry when adding
#  new libraries into the registry.
#
REGISTRY_OPTION=${REGISTRY_OPTION?${REGISTRY_OPTION}:-check_registry}
SO_LOCATIONS=`${GENLOC} /obj/${target_machine}/usr/shlib/${target_machine}/so_locations`
_SHLDFLAGS_=-shared -set_version osf.1 -soname $@ ${REGISTRY_OPTION} ${SO_LOCATIONS} ${EXPORTS} ${_LD_OL_} ${${@}_LD_OPT_LEVEL?${${@}_LD_OPT_LEVEL}:${LD_OPT_LEVEL}} ${${@}_LDENV?${${@}_LDENV}:${LDENV}} ${${@}_LDFLAGS?${${@}_LDFLAGS}:${LDFLAGS}} ${${@}_LDARGS?${${@}_LDARGS}:${LDARGS}} ${_GENLIB_} ${_CC_LIBDIRS_}

_YFLAGS_=${${@}_YENV?${${@}_YENV}:${YENV}} ${${@}_YFLAGS?${${@}_YFLAGS}:${YFLAGS}} ${${@}_YARGS?${${@}_YARGS}:${YARGS}}
_LINTFLAGS_=${${@}_LINTENV?${${@}_LINTENV}:${LINTENV}} ${${@}_LINTFLAGS?${${@}_LINTFLAGS}:${LINTFLAGS}} ${${@}_LINTARGS?${${@}_LINTARGS}:${LINTARGS}} ${_GENINC_}
_TAGSFLAGS_=${${@}_TAGSENV?${${@}_TAGSENV}:${TAGSENV}} ${${@}_TAGSFLAGS?${${@}_TAGSFLAGS}:${TAGSFLAGS}} ${${@}_TAGSARGS?${${@}_TAGSARGS}:${TAGSARGS}}
_RMFLAGS_=${${@}_DEF_RMFLAGS?${${@}_DEF_RMFLAGS}:${DEF_RMFLAGS}}
_MIGFLAGS_=-cpp ${_CC_EXEC_PREFIX_}cpp ${${@}_MIGENV?${${@}_MIGENV}:${MIGENV}} ${${@}_MIGFLAGS?${${@}_MIGFLAGS}:${MIGFLAGS}} ${${@}_MIGARGS?${${@}_MIGARGS}:${MIGARGS}} ${_CC_NOSTDINC_} ${_GENINC_} -D__${target_machine}
_MDFLAGS_=${${@}_DEF_MDFLAGS?${${@}_DEF_MDFLAGS}:${DEF_MDFLAGS}} ${_GENINC_}

#
#  Define these with default options added
#
_RELEASE_=${RELEASE_PREFIX}${RELEASE} ${RELEASE_OPTIONS}

#
#  Define binary targets
#
_SET_BINARIES_=${PROGRAMS?BINARIES:${LIBRARIES?BINARIES:${OBJECTS?BINARIES:}}}
${_SET_BINARIES_}=${PROGRAMS} ${LIBRARIES} ${OBJECTS}

#
# definitions for build
#
_MIG_HDRS_=${MIG_DEFS:.defs=.h}
_MIG_USRS_=${MIG_DEFS:.defs=User.c}
_MIG_SRVS_=${MIG_DEFS:.defs=Server.c}

#
# definitions for clean
#
_CLEANFILES_=${CLEANFILES?${CLEANFILES}:${$%_CLEANFILES?${$%_CLEANFILES}:$%.X ${_CLEAN_OFILES_} ${$%_GARBAGE?${$%_GARBAGE}:${GARBAGE}}}}

#
# definitions for lint
#
_LINTFILES_=${LINTFILES?${LINTFILES}:${$%_LINTFILES?${$%_LINTFILES}:${_LINT_OFILES_:.o=.c}}}

#
# definitions for datafiles
#
_DATAFILES_=${DATAFILES} ${NEW_FILES?${NEW_FILES/*/.new..&}:} ${UPD_FILES?${UPD_FILES/*/.upd..&}:}

#
# definitions for tags
#
_TAGSFILES_=${TAGSFILES?${TAGSFILES}:${$%_TAGSFILES?${$%_TAGSFILES}:${_TAGS_OFILES_:.o=.c}}}

#
# definitions for export
#
_EXPDIR_=${$%_EXPDIR?${$%_EXPDIR}:${EXPDIR?${EXPDIR}:${$%_IDIR?${$%_IDIR}:${IDIR}}}}/
_EXPORTBASE_=${EXPORTBASE?${EXPORTBASE}:_EXPORTBASE_}
_EXPLINKS_=${$%_EXPLINKS?${$%_EXPLINKS}:${EXPLINKS}}
_DO_EXPLINKS_=\
	for i in ${_EXPLINKS_}; do ${RM} ${_RMFLAGS_} $$i; ${LN} $% $$i; done
_MAKE_EXPLINKS_=${$%_EXPLINKS?${_DO_EXPLINKS_}:${EXPLINKS?${_DO_EXPLINKS_}:@}}

#
# definitions for install
#
_ITARGET_=${TOSTAGE?_TOSTAGE_:_NO_TOSTAGE_}

_INSTALL_TARGET_=${_ITARGET_}$%

_STAGEFILES_=${TOSTAGE?${FROMSTAGE?:_TOSTAGE_%}:}
_STAGEBASES_=${TOSTAGE?${FROMSTAGE?_TOSTAGE_%:}:}

#
#  Default single suffix compilation rules
#
.SUFFIXES:
.SUFFIXES: .o .s .pp .c .y .l .p .sh .csh .ksh .txt .uu

#
#  Default double suffix compilation rules
#
${MSGHDRS}: $${@:_msg.h=.msg}
	${MKCATDEFS} ${@:_msg.h=} ${@:_msg.h=.msg} > ${@:_msg.h=.cat.in}
	-${RM} ${_RMFLAGS_} ${MSGLANGPATH}/${@:_msg.h=.cat}
	${MAKEPATH} ${MSGLANGPATH}/${@:_msg.h=.cat}
	-${GENCAT} ${MSGLANGPATH}/${@:_msg.h=.cat} ${@:_msg.h=.cat.in}
	${RM} ${_RMFLAGS_} ${@:_msg.h=.cat.in}

${CATFILES}: $${@:.cat=.msg}
	${MKCATDEFS} ${@:.cat=} ${@:.cat=.msg} > $@.in
	-${RM} ${_RMFLAGS_} ${MSGLANGPATH}/$@
	${MAKEPATH} ${MSGLANGPATH}/$@
	-${GENCAT} ${MSGLANGPATH}/$@ $@.in
	${RM} ${_RMFLAGS_} $@.in

#
#  Special rules
#

#
#  Prefix used for targets which are neither installed nor built by
#  default at the local site.  They can still be named specifically but
#  will not be included by "clean_all", "build_all", "lint_all" or
#  "install_all" rules.
#
NOTSITE=_NOTSITE_

#
#  Use this as a dependency for any rule which should always be triggered
#  (e.g. recursive makes).
#
${ALWAYS}:;@

TAR=tar

#
#  Include pass definitions for standard targets
#
include passes.mk

#
#  Compilation rules
#
all: build_all;@

build_all: $${_BUILD_TARGETS_};@

_build_prefix_%: %;@

_build_prefix_${NOTSITE}%:
	@echo % is not used at ${SITE}

dopass_all: $${_DOPASS_TARGETS_};@

comp_all: $${_COMP_TARGETS_};@

_comp_prefix_%: %;@

_comp_prefix_${NOTSITE}%:
	@echo % is not used at ${SITE}

#
# Rules to handle NEW_FILES and UPD_FILES for update installation
#
.new..%: %
	${CP} % $@

.upd..%: %
	${CP} % $@

#
#  Clean up rules
#
clean_all: $${_CLEAN_TARGETS_}
	-${RM} ${_RMFLAGS_} core

clean_%: _clean_prefix_%;@

_clean_prefix_%:
	-${RM} ${_RMFLAGS_} ${_CLEANFILES_}

_clean_prefix_${NOTSITE}%:;@

rmtarget_all: $${_RMTARGET_TARGETS_}
	-${RM} ${_RMFLAGS_} core

rmtarget_%: _rmtarget_prefix_%;@

_rmtarget_prefix_%:
	-${RM} ${_RMFLAGS_} %

_rmtarget_prefix_${NOTSITE}%:;@

clobber_all: $${_CLOBBER_TARGETS_}
	-${RM} ${_RMFLAGS_} core depend.mk

clobber_%: _clobber_prefix_%;@

_clobber_prefix_%: _clean_prefix_% _rmtarget_prefix_%;@

_clobber_prefix_${NOTSITE}%:;@

#
#  Lint rules
#
lint_all: $${_LINT_TARGETS_};@

lint_%: _lint_prefix_%;@

_lint_prefix_%: $${_LINTFILES_}
	${LINT} ${_LINTFLAGS_} $>

_lint_prefix_${NOTSITE}%:;@

#
#  Tags rules
#
tags_all: $${_TAGS_TARGETS_};@

tags_%: _tags_prefix_%;@

_tags_prefix_%: $${_TAGSFILES_}
	${TAGS} ${_TAGSFLAGS_} $>

_tags_prefix_${NOTSITE}%:;@

#
#  Export rules
#
export_all: $${_EXPORT_TARGETS_};@

export_%: _export_prefix_%;@

_export_prefix_%: ${_EXPORTBASE_}${_EXPDIR_}%;@

${_EXPORTBASE_}%2/%: %
	-${RM} ${_RMFLAGS_} $@
	${MAKEPATH} $@
	${_MAKE_EXPLINKS_}
	${TAR} cf - % ${_EXPLINKS_} | (cd ${_EXPORTBASE_}%2; tar xf -)

_export_prefix_${NOTSITE}%:;@

#
#  Installation/release rules
#
install_all: $${_INSTALL_TARGETS_};@

install_%: _install_prefix_%;@

_install_prefix_%: $${_INSTALL_TARGET_};@

_install_prefix_${NOTSITE}%:;@

${_STAGEFILES_}: ${ALWAYS}
	${_RELEASE_} ${%_NOSTRIP?-nostrip:${NOSTRIP?-nostrip:}}\
		-o ${%_IOWNER?${%_IOWNER}:${IOWNER}}\
		-g ${%_IGROUP?${%_IGROUP}:${IGROUP}}\
		-m ${%_IMODE?${%_IMODE}:${IMODE}}\
		-tostage ${TOSTAGE}\
		-fromfile %\
		${%_IDIR?${%_IDIR}:${IDIR}}/%\
		${%_ILINKS?${%_ILINKS}:${ILINKS}}

${_STAGEBASES_}: ${ALWAYS}
	${_RELEASE_} ${%_NOSTRIP?-nostrip:${NOSTRIP?-nostrip:}}\
		-o ${%_IOWNER?${%_IOWNER}:${IOWNER}}\
		-g ${%_IGROUP?${%_IGROUP}:${IGROUP}}\
		-m ${%_IMODE?${%_IMODE}:${IMODE}}\
		-tostage ${TOSTAGE}\
		-fromstage ${FROMSTAGE}\
		${%_IDIR?${%_IDIR}:${IDIR}}/%\
		${%_ILINKS?${%_ILINKS}:${ILINKS}}

#
# mkdirs
#

instdir_all: $${_INSTDIR_TARGETS_};@

instdir_%: _instdir_prefix_%;@

_instdir_prefix_%: ${TOSTAGE?_inst_directory_%:};@

_instdir_prefix_${NOTSITE}%:;@

_inst_directory_%: ${ALWAYS}
	${INSTDIR} -o ${%_IOWNER?${%_IOWNER}:${IOWNER}} \
		   -g ${%_IGROUP?${%_IGROUP}:${IGROUP}} \
		   -m ${%_IMODE?${%_IMODE}:${IMODE}} \
		      ${TOSTAGE}${%_IDIR?${%_IDIR}:${IDIR}}/%

#
# symlinks
#

symlink_all: $${_SYMLINK_TARGETS_};@

symlink_%: _symlink_prefix_%;@

_symlink_prefix_%: ${TOSTAGE?_inst_symlinks_%:};@

_symlink_prefix_${NOTSITE}%:;@

_inst_symlinks_%: ${ALWAYS}
	@${MKSYMLINKS} ${TOSTAGE} \
		"${%_SYMLINKDIR1?${%_SYMLINKDIR1}:${SYMLINKDIR1}}/%" \
		"${%_SYMLINKDIR2?${%_SYMLINKDIR2}:${SYMLINKDIR2}}" \
		"${%_SYMLINK?${%_SYMLINK}:%}"

