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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:48:23 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

VPATH			= ..

dofile.o_CFLAGS		= -DPDA

MSGHDRS			= cm_msg.h co_msg.h delta_msg.h

INCFLAGS		= -I..

LIBRARIES		= libsccs.a
OFILES			= \
			  auxf.o\
			  chkid.o\
			  chksid.o\
			  date_ab.o\
			  date_ba.o\
			  del_ab.o\
			  del_ba.o\
			  dodelt.o\
			  dofile.o\
			  dohist.o\
			  doie.o\
			  dolist.o\
			  eqsid.o\
			  flushto.o\
			  fmalloc.o\
			  fmterr.o\
			  getline.o\
			  logname.o\
			  newstats.o\
			  permiss.o\
			  pf_ab.o\
			  putline.o\
			  rdmod.o\
			  setup.o\
			  sid_ab.o\
			  sid_ba.o\
			  sidtoser.o\
			  sinit.o\
			  stats_ab.o ${NEED_FOR_PORTING}

HFILES			= ${MSGHDRS}

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk

auxf.o: defines.h
dohist.o: delta_msg.h
