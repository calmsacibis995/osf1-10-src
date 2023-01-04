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

PROGRAMS		= scu

DATAFILES		= scu.hlp

GARBAGE			= \
			  cam_debug.c \
			  cam_special_data.c \
			  special_audio.c \
			  special_direct.c \
			  special_generic.c \
			  special_sequential.c

ILIST			= ${PROGRAMS} ${DATAFILES}

IDIR			= /sbin/

USE_STATIC_LIBRARIES	=

CFLAGS			= -DCAMDEBUG -DSCU

scu_OFILES		= \
			  audio_page.o \
			  cache_page.o \
			  cdrom_page.o \
			  config_page.o \
			  dec_page.o \
			  direct_page.o \
			  error_page.o \
			  flexible_page.o \
			  geometry_page.o \
			  help.o \
			  istring.o \
			  parser.o \
			  partition_page.o \
			  readahead_page.o \
			  reco_page.o \
			  unknown_page.o \
			  scu.o \
			  scu_audio.o \
			  scu_cam.o \
			  scu_capacity.o \
			  scu_defects.o \
			  scu_format.o \
			  scu_fsck.o \
			  scu_global.o \
			  scu_help.o \
			  scu_inquiry.o \
			  scu_malloc.o \
			  scu_misc.o \
			  scu_pages.o \
			  scu_playaudio.o \
			  scu_read.o \
			  scu_scan.o \
			  scu_sense.o \
			  scu_set.o \
			  scu_setaudio.o \
			  scu_show.o \
			  scu_showaudio.o \
			  scu_special.o \
			  scu_tables.o \
			  scu_tape.o \
			  scu_tests.o \
			  scu_utils.o \
			  scu_verify.o \
			  scu_write.o \
			  scu_xzadiag.o \
			  strcasecmp.o \
			  cam_debug.o \
			  cam_special_data.o \
			  special_audio.o \
			  special_direct.o \
			  special_generic.o \
			  special_sequential.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

cam_debug.c: ${MAKETOP}/kernel/io/cam/cam_debug.c
	${RM} -f cam_debug.c
	${CP} ${MAKETOP}/kernel/io/cam/cam_debug.c .

cam_special_data.c: ${MAKETOP}/kernel/data/cam_special_data.c
	${RM} -f cam_special_data.c
	${CP} ${MAKETOP}/kernel/data/cam_special_data.c .

special_audio.c: ${MAKETOP}/kernel/io/cam/special_audio.c
	${RM} -f special_audio.c
	${CP} ${MAKETOP}/kernel/io/cam/special_audio.c .

special_direct.c: ${MAKETOP}/kernel/io/cam/special_direct.c
	${RM} -f special_direct.c
	${CP} ${MAKETOP}/kernel/io/cam/special_direct.c .

special_generic.c: ${MAKETOP}/kernel/io/cam/special_generic.c
	${RM} -f special_generic.c
	${CP} ${MAKETOP}/kernel/io/cam/special_generic.c .

special_sequential.c: ${MAKETOP}/kernel/io/cam/special_sequential.c
	${RM} -f special_sequential.c
	${CP} ${MAKETOP}/kernel/io/cam/special_sequential.c .
