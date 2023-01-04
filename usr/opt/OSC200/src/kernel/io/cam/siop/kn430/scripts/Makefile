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
#
# OSF/1 Release 1.0

CAM			= ${MAKETOP}/kernel/io/cam
VPATH			= ${CAM}:${CAM}/siop

CFLAGS                  = -O -DCMU -D_NO_PROTO -Dconst=

DATAFILES		= scriptram_kn430.h siopdefs_kn430.h \
			  scripthost.h siopdefs.h scsi_phases.h

CCTYPE			= host
OTHERS			= script_kn430.h


ILIST   = scriptram_kn430.h siopdefs_kn430.h
IDIR    = /usr/sys/io/cam/siop/kn430/scripts/
IMODE   = 444
IOWNER  = bin
IGROUP  = bin

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/datafiles.mk

script_kn430.h: script_kn430.ss scriptram_kn430.h siopdefs_kn430.h
	rm -f foo.c
	cp script_kn430.ss foo.c
	${_CC_} -P ${CFLAGS} foo.c
	../../ssc/ssc foo.i -O ../script_kn430.h -L script_kn430.lst
