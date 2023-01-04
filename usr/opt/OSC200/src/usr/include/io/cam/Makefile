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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/08/02 17:20:35 $
#

VPATH			= ${MAKETOP}/kernel/io/cam

DATAFILES		= cdrom.h rzdisk.h cam.h pdrv.h uagt.h xpt.h \
			  scsi_status.h dec_cam.h scsi_sequential.h \
			  scsi_phases.h scsi_direct.h scsi_all.h \
			  cam_special.h scsi_cdbs.h scsi_opcodes.h \
			  scsi_special.h cam_logger.h scsi_rodirect.h \
			  cam_debug.h cam_nstd_raid.h
INCLUDES		= ${DATAFILES}

EXPINC_SUBDIRS		= xza

# See src/setup/postinstall.sh (search for HEADERS_SYMLINKS:)
#ILIST			= ${INCLUDES}
IDIR			= /usr/include/io/cam/
IMODE			= 644

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
include ${MAKEFILEPATH}/datafiles.mk
