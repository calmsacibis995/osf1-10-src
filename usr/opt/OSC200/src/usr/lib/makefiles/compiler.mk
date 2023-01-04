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
#        @(#)$RCSfile: compiler.mk,v $ $Revision: 4.3.3.5 $ (DEC) $Date: 1992/12/03 11:19:49 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

USE_STATIC_LIBRARIES=1

STD_FLAG=-std0

OSUFFIX=.o

HOST_LDFLAGS=-non_shared

_CMPLRS_ULT_INCDIRS_=`${GENPATH} -I.` -I${BOOT_INCDIR} -I/usr/include
_CMPLRS_OSF_INCDIRS_=`${GENPATH} -I.` -I${BOOT_INCDIR}
_CMPLRS_INCDIRS_=${_BLD ? ${_CMPLRS_ULT_INCDIRS_} : ${_CMPLRS_OSF_INCDIRS_}}

_CMPLRS_ULT_DEFS_= ${_BLD?-D__ANSI_COMPAT:} \
			 ${_BLD?-D__ultrix:}
# afdfix: do we want any alpha defines here?
#_CMPLRS_OSF_DEFS_=-Dmips -Dhost_mips -DMIPSEL
#jpm: changed define of bsd to not give warns (use value in param.h)
_CMPLRS_DEFS_=-DBSD=198911 -D_BSD -Dultrix -D__ultrix -D__osf__ -D_ARCH1 \
		${ULTRIX_BLD ?  ${_CMPLRS_ULT_DEFS_}: \
				${_CMPLRS_OSF_DEFS_}} 

_CMPLRS_ULT_CCDEFS_=-DLANGUAGE_C -D__LANGUAGE_C -D__LANGUAGE_C__
_CMPLRS_OSF_CCDEFS_=
_CMPLRS_ULT_PCDEFS_=-DLANGUAGE_PASCAL -D__LANGUAGE_PASCAL \
		    -D__LANGUAGE_PASCAL__
_CMPLRS_OSF_PCDEFS_=
_CMPLRS_ULT_ASDEFS_=-DLANGUAGE_ASSEMBLY -D__LANGUAGE_ASSEMBLY \
		    -D__LANGUAGE_ASSEMBLY__ -DASSEMBLER
_CMPLRS_OSF_ASDEFS_=-DASSEMBLY

_CMPLRS_CCDEFS_=${_CMPLRS_DEFS_} \
		 ${ULTRIX_BLD ? ${_CMPLRS_ULT_CCDEFS_}: \
				${_CMPLRS_OSF_CCDEFS_}}
_CMPLRS_PCDEFS_=${_CMPLRS_DEFS_} \
		 ${ULTRIX_BLD ? ${_CMPLRS_ULT_PCDEFS_}: \
				${_CMPLRS_OSF_PCDEFS_}}
_CMPLRS_ASDEFS_=${_CMPLRS_DEFS_} \
		 ${ULTRIX_BLD ? ${_CMPLRS_ULT_ASDEFS_}: \
				${_CMPLRS_OSF_ASDEFS_}}

_CMPLRS_LIBDIR_=${_BLD ? -L${COMP_TARGET_ROOT}/usr/lib/cmplrs/cc${REL} : \
		${SHLIBDIRS} ${LIBDIRS/*/&\/cmplrs\/cc${REL}} }

#jpm was:		${SHLIBDIRS} ${LIBDIRS/*/&\/..\/..\/lib\/cmplrs\/cc${REL}/}}

LIBEXC		= -lexc
LIBFE		= -lfe
LIBM		= -lm 
LIBMLD		= -lmld
LIBMISC		= -lmisc
LIBP		= -lp
LIBUS		= -lus
LIBU		= -lu
LIBXMALLOC	= -lxmalloc
LIBL		= -ll
LIBDPI		= -ldpi
LIBXPROC	= -lxproc
LIBTERMCAP	= -ltermcap
#jpm: LIBLOADER is gone; dont define it... dont use it..
LIBC		= ${ULTRIX_BLD ? : ${_BLD ? -lc : }}

SYSLIBS		= ${_BLD ? ${LIBC} : }

_CMPLRS_LIBDIRS_=${_CMPLRS_LIBDIR_} \
		  ${_BLD ? -L/usr/lib/cmplrs/cc -L/usr/lib -L/lib : }
