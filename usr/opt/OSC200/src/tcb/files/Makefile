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
# @(#)$RCSfile: Makefile,v $ $Revision: 4.2.13.4 $ (DEC) $Date: 1993/06/15 17:27:37 $
# 
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#

SUBDIRS		= ${SUBDIRS_${SEC_LEVEL}}
SUBDIRS_BASE	= auth.${SEC_LEVEL} skel subsys
SUBDIRS_	= auth.C2 skel subsys
SEC_SUBDIRS	= ${SUBDIRS}
DATAFILES	= ${DATAFILES_${SEC_LEVEL}}
SEC_DATAFILES	= ${DATAFILES}
DATAFILES_ACL_SWARE	= sp_config
DATAFILES_ACL_SWARE	= sp_config
DATAFILES_MAC	= ${ACL_POLICY?:sp_config}


OTHERS_ACL_SWARE	= ACLDBASE spdinitrc
OTHERS_ACL_POSIX	= PACLDBASE spdinitrc
OTHERS_MAC	= MACDBASE ${ACL_POLICY?:spdinitrc}

ACL_SWARE_DAEMON	= acld
ACL_POSIX_DAEMON	= pacld
DAEMON	= ${${ACL_POLICY}_DAEMON}

OTHERS	= ${OTHERS_${SEC_LEVEL}}


ILIST		= ${DATAFILES} ${OTHERS}
SEC_ILIST	= ${ILIST}
IDIR		= /tcb/files/
subsys_IDIR	= /var/tcb/files/
INSTDIRS	= auth 
auth_IMODE	= 770
auth_IOWNER	= auth
auth_IGROUP	= auth

SYMLINKS	= subsys
SYMLINKDIR1	= ../../var/tcb/files
SYMLINKDIR2	= /tcb/files

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
include ${MAKEFILEPATH}/datafiles.mk
include ${MAKEFILEPATH}/others.mk

ACLDBASE:
	-@rm -f ACLDBASE
	../../seccmd/secpolicy/mkdb ACLDBASE 1024 60

PACLDBASE:
	-@rm -f PACLDBASE
	../../seccmd/secpolicy/mkdb PACLDBASE 1024 60

MACDBASE:
	-@rm -f MACDBASE
	../../seccmd/secpolicy/mkdb MACDBASE 1024 60

sp_config: sp_config.${SEC_LEVEL}
	${RM} ${_RMFLAGS_} sp_config
	${SED} -e "s/ACLDAEMON/${DAEMON}/" sp_config.${SEC_LEVEL} > sp_config.X
	${MV} sp_config.X sp_config
