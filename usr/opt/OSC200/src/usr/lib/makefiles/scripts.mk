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
#	@(#)$RCSfile: scripts.mk,v $ $Revision: 4.2.9.2 $ (DEC) $Date: 1993/04/01 20:27:04 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

#
#  Definitions for rules using sed
#
_SED_STRIP_=-e '1s;^#!;&;' -e t
_N_A_S_F_=THIS IS NOT A SOURCE FILE - DO NOT EDIT
_SED_SOURCEWARNING_=-e 's;^#\(.*\)\@SOURCEWARNING\@;\1${_N_A_S_F_};' -e t
_SED_KEEPVERSION_=-e 's;\@(#);&;' -e t
_SED_KEEPCOPYRIGHT_=-e 's;\@DIGITAL_COPYRIGHT;&;' -e t \
	-e 's;\@DEC_COPYRIGHT;&;' -e t
#_SED_CSH_STRIP_=-e '/^[ 	]*\#/d'
_SED_CSH_STRIP_=${SED_CSH_STRIP?${SED_CSH_STRIP}:-e '/^[ 	]*\#/d'}
_SED_KSH_STRIP_=${SED_KSH_STRIP?${SED_KSH_STRIP}:-e '/^[ 	]*\#/d'}
#_SED_SH_STRIP_=-e '/^[ 	]*[\#:]/d'
_SED_SH_STRIP_=${SED_SH_STRIP?${SED_SH_STRIP}:-e '/^[ 	]*[\#:]/d'}

#
#  Default single suffix compilation rules
#
.csh:
	${SED}\
	 ${_SED_STRIP_}\
	 ${_SED_SOURCEWARNING_}\
	 ${_SED_KEEPVERSION_}\
	 ${_SED_KEEPCOPYRIGHT_}\
	 ${${@}_SED_OPTIONS?${${@}_SED_OPTIONS}:${SED_OPTIONS}}\
	 ${_SED_CSH_STRIP_} $< >$*.X
	${CHMOD} +x $*.X
	${MV} -f $*.X $*
.ksh:
	${SED}\
	 ${_SED_STRIP_}\
	 ${_SED_SOURCEWARNING_}\
	 ${_SED_KEEPVERSION_}\
	 ${_SED_KEEPCOPYRIGHT_}\
	 ${${@}_SED_OPTIONS?${${@}_SED_OPTIONS}:${SED_OPTIONS}}\
	 ${_SED_KSH_STRIP_} $< >$*.X
	${CHMOD} +x $*.X
	${MV} -f $*.X $*

.sh:
	${SED}\
	 ${_SED_STRIP_}\
	 ${_SED_SOURCEWARNING_}\
	 ${_SED_KEEPVERSION_}\
	 ${_SED_KEEPCOPYRIGHT_}\
	 ${${@}_SED_OPTIONS?${${@}_SED_OPTIONS}:${SED_OPTIONS}}\
	 ${_SED_SH_STRIP_} $< >$*.X
	${CHMOD} +x $*.X
	${MV} -f $*.X $*
