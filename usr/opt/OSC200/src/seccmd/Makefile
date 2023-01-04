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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.10.4 $ (DEC) $Date: 1993/06/17 15:53:38 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

#
# Copyright (c) 1989 SecureWare, Inc.  All rights reserved.
#

LPD			= ${MAKETOP}usr/sbin/lpd
CPIO			= ${MAKETOP}usr/bin/cpio
CONF			= ${MAKETOP}kernel/conf

VPATH			= ${LPD}:${CPIO}:${CONF}


SUBDIRS			= ${SEC_SUBDIRS}
SEC_SUBDIRS		= ${SUBDIRS_${SEC_LEVEL}}

SUBDIRS_BASE		= secpolicy
SUBDIRS_ARCH		= policy


FILE_LIST		= authck \
			  ${SEC_DEFS?setfiles auths epa mltape privs:}

ILIST			= ${PROGRAMS} ${DATAFILES} ${SCRIPTS}

SYMLINKS		= ${FILE_LIST} ${DATAFILES}
SYMLINKDIR1		= ../../var/tcb/bin
SYMLINKDIR2		= /tcb/bin

             IDIR	= /var/tcb/bin/
    initcond_IDIR	= /tcb/lib/
    pwpolicy_IDIR	= /tcb/bin/
   secswitch_IDIR	= /tcb/bin/
   setfstag_IDIR	= /tcb/bin/
SWbanner.pro_IDIR	= /tcb/files/

     aclsyn_IMODE	= 111
      auths_IMODE	= 111
        epa_IMODE	= 2111
     mltape_IMODE	= 2111
      privs_IMODE	= 2111
  secswitch_IMODE	= 2111
   pwpolicy_IMODE	= 550
  pwpolicy_IGROUP	= auth

USE_STATIC_LIBRARIES	=

    SCRIPTS		= ${SEC_SCRIPTS}
SEC_SCRIPTS		= ${SCRIPTS_${SEC_LEVEL}}
SCRIPTS_		= pwpolicy
SCRIPTS_BASE		= ${SCRIPTS_}

    PROGRAMS		= ${SEC_PROGRAMS}
SEC_PROGRAMS		= ${PROGRAMS_${SEC_LEVEL}}
PROGRAMS_		= ${FILE_LIST} initcond

PROGRAMS_BASE		= ${PROGRAMS_} secswitch
PROGRAMS_ACL_SWARE	= aclsyn 
PROGRAMS_ACL_POSIX	= 
PROGRAMS_ARCH		= setfstag 
PROGRAMS_MAC		= ismultdir lpbanner lplf mandsyn mkmultdir rmmultdir

    DATAFILES		= ${SEC_DATAFILES}
SEC_DATAFILES		= ${DATAFILES_${SEC_LEVEL}}

DATAFILES_MAC		= SWbanner.pro

MSGHDRS			= aclsyn_msg.h \
			  authck_msg.h auths_msg.h epa_msg.h \
			  initcond_msg.h ismultdir_msg.h \
			  labelcomp_msg.h lpbanner_msg.h \
			  mkmultdir_msg.h privs_msg.h \
			  rmmultdir_msg.h setfiles_msg.h setfstag_msg.h \
			  cpio_msg.h printer_msg.h sec_export_msg.h

CATFILES		= aclsyn.cat \
			  authck.cat auths.cat epa.cat initcond.cat \
			  ismultdir.cat labelcomp.cat \
			  lpbanner.cat mkmultdir.cat privs.cat \
			  rmmultdir.cat setfiles.cat setfstag.cat \
			  cpio.cat printer.cat sec_export.cat

CFLAGS			= ${SEC_LEVEL?${SEC_DEFS}:-DSEC_BASE}


  lplf_OFILES		= lplf.o printcap.o
aclsyn_OFILES		= aclsyn.o acl_parse.o
mltape_OFILES		= mltape.o sec_export.o

    mltape.o_CFLAGS	= ${SEC_DEFS} -DMLTAPE
sec_export.o_CFLAGS	= ${SEC_DEFS} -DMLTAPE


LIBS		= -lsecurity -laud
privs_LIBS	= ${LIBS} -lcurses

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/datafiles.mk
include ${MAKEFILEPATH}/scripts.mk

${MSGHDRS:_msg.h=.o}: $${@:.o=_msg.h}

acl_parse.o: acl_parse.y

mandsyn.o: mandsyn.y

sec_export.o:	sec_export.h
printcap.o:	printer_msg.h
mltape.o:	cpio.c cpio_msg.h
	rm -f ./mltape.c
	cp cpio.c ./mltape.c
	${_CC_} -c ${_CCFLAGS_} ./mltape.c
	rm -f ./mltape.c
