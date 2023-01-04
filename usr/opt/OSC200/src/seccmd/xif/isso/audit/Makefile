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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:07:35 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
# OSF/1 Release 1.0
#
#

#

SEC_LIBRARIES	= libaudit.a
LIBRARIES	= ${SEC_LIBRARIES}

CFLAGS		= ${SEC_DEFS}

INCFLAGS	= -I../../common

OFILES		= DirLst.o EvMask.o ListSess.o PFiles.o Parameters.o \
		  Selection.o UsrGrp.o XAudColnsl.o XAudColxsl.o \
		  XAudSelnos.o XAudSelnss.o XAudSelxos.o XAudSelxss.o \
		  XAudUtils.o XAudit.o XAuditStat.o XBackupDel.o \
		  XDirLst.o XEvents.o XMaint.o XOptions.o XParameter.o \
		  XReduce.o XRestore.o XSelectFil.o XSelection.o \
		  XShowRep.o XUsrGrp.o


include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk
