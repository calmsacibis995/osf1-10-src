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
# This section for Makefiles with sub-directories


_SED_OPTIONS	= -e 's+%TCB%++' -e t
C2_SED_OPTIONS	= -e 's+%TCB%+/tcb/bin:+' -e t
B1_SED_OPTIONS	= ${C2_SED_OPTIONS}

real.profile_SED_OPTIONS = ${${SEC_LEVEL}_SED_OPTIONS}

SUBDIRS			= upgrade
# EXPINC_SUBDIRS	=
# EXPLIB_SUBDIRS	=


# This section is for Makefiles with targets.

SCRIPTS			= real.profile .login .cshrc
DATAFILES		= DXsession .mrg..DXsession

ILIST			= ${SCRIPTS} ${DATAFILES}
IDIR			= /
IMODE			= 740
IOWNER			= root
IGROUP			= system


include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
include ${MAKEFILEPATH}/subdirs.mk
include ${MAKEFILEPATH}/scripts.mk
