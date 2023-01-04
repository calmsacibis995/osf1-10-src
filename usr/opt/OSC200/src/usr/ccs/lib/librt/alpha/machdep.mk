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
# @(#)$RCSfile: machdep.mk,v $ $Revision: 1.1.10.2 $ (DEC) $Date: 1993/03/15 20:56:10 $
#
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
# OSF/1 Release 1.0

${TARGET_MACHINE}CFLAGS = -DMIPSEL -DH_RT

${TARGET_MACHINE}COBJS = schgetmax.o schgetmin.o schgetrr.o

${TARGET_MACHINE}SOBJS = schyield.o memlk.o memunlk.o get_todtimer.o\
            set_todtimer.o timer_drift.o rt_getprio.o\
	    rt_setprio.o timer_create.o timer_delete.o\
            sem_wakeup.o sem_sleep.o

schyield.o_CCTYPE	= traditional

COFF_SAVEGP=la $$28,_gp;

${${TARGET_MACHINE}SOBJS}: $${@:.o=.s}
	${_CC_} ${_CCFLAGS_} -g0 -c $*.s



