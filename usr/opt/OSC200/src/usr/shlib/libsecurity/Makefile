
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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.10.5 $ (DEC) $Date: 1993/10/29 17:36:39 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

VPATH		= ${MAKETOP}/usr/ccs/lib/libsecurity:${MAKETOP}/usr/ccs/lib/libsecurity/SIA:${MAKETOP}/usr/ccs/lib/libc/SIA

CATFILES	= libsec.cat
MSGHDRS		= libsec_msg.h

SHARED_LIBRARIES	= libsecurity.so

USE_SHARED_LIBRARIES	=
LIBS			= -laud -lc -lm

EXPSHLIB_TARGETS	= export_libsecurity.so

ILIST			= libsecurity.so
IDIR			= /usr/shlib/
IMODE			= 644
NOSTRIP			=
INCFLAGS		= -I${MAKETOP}usr/ccs/lib/libsecurity \
			  -I${MAKETOP}usr/ccs/lib/libsecurity/SIA \
			  -I${MAKETOP}usr/ccs/lib/libc/SIA

#CFLAGS		= ${SEC_DEFS}
HFILES		= ${MSGHDRS} libsecurity.h sia_mech.h
DATAFILES	= ${HFILES}			# XXX


STDOBJS		= accept_pw.o acllib.o authaudit.o authcap.o \
		  chacl.o chilabel.o chslabel.o compress.o data.o discr.o \
		  disk.o fields.o getclrnce.o getdvagent.o getilabel.o \
		  getpasswd.o getprdfent.o getprfient.o getprlpent.o \
		  getprpwent.o getprtcent.o getslabel.o identity.o \
		  lchilabel.o lchslabel.o lstatilabel.o lstatslabel.o \
		  mand_init.o mand_tag.o mandlib.o map_ids.o mld.o \
		  msg_acl.o msg_ilabel.o msg_slabel.o passlen.o printbuf.o \
		  privileges.o psecerror.o randomword.o sec_errlst.o \
		  security.o seed.o sem_acl.o \
		  sem_ilabel.o sem_slabel.o setclrnce.o setilabel.o \
		  setslabel.o shm_acl.o shm_ilabel.o shm_slabel.o \
		  statacl.o statilabel.o statslabel.o std_labels.o \
		  subsystems.o tod.o nlgetamsg.o

SIAOBJS		= \
		chg_finger.o chg_password.o chg_shell.o chk_invoker.o \
		chk_user.o sia_init.o ses_authent.o ses_estab.o \
		ses_init.o ses_launch.o ses_reauth.o \
		ses_release.o ses_suauth.o \
		sia_util.o login_sec.o passwd_sec.o su_secure.o ses_audit.o \
		siad_passwd.o siad_group.o

OFILES		= ${STDOBJS} ${SIAOBJS}

include ${MAKEFILEPATH}/rules.mk
