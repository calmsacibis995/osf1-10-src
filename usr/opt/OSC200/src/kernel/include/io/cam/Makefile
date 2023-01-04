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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.11.6 $ (DEC) $Date: 1993/09/03 16:05:51 $
#
#
#       @(#)Makefile    3.3     (ULTRIX/OSF)    6/23/91
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
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#

VPATH                   = ${MAKETOP}/kernel/io/cam

SUBDIRS			= xza siop spo

DATAFILES               = cam_config.h cam_debug.h cam_disk.h \
                          cam_errlog.h cam_tape.h ccfg.h dme.h \
                          dme_3min_94_dma.h \
                          dme_pmax_sii_ram.h dme_turbo_94_ram.h sim.h \
			  sim_94.h sim_cirq.h sim_common.h sim_config.h \
			  sim_sii.h sim_target.h sim_xpt.h sim_kzq.h \
			  dme_kzq_ram.h dme_tcds_94_dma.h dme_common.h \
			  tcds_adapt.h  \
                          cam.h pdrv.h uagt.h xpt.h scsi_status.h dec_cam.h \
			  scsi_processor.h \
                          scsi_sequential.h scsi_phases.h scsi_direct.h \
                          scsi_all.h cam_special.h scsi_cdbs.h scsi_opcodes.h \
                          scsi_special.h cam_logger.h scsi_rodirect.h \
			  cdrom.h rzdisk.h aha1740.h cam_nstd_raid.h

ILIST   = ${DATAFILES}
IDIR    = /usr/sys/include/io/cam/
IMODE   = 444
IOWNER  = bin
IGROUP  = bin

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
include ${MAKEFILEPATH}/datafiles.mk
