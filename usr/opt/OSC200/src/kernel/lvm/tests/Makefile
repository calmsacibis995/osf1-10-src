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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/29 14:22:07 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

USE_STATIC_LIBRARIES	=

# VPATH is only really necessary for mkpv on the mmax.
VPATH			= ${MAKETOP}/usr/sbin/newfs

PROGRAMS		= queues openclose blocktest vgcreate mkpv pvdump \
			  pvattach vgactivate vgdeactivate lvtest lvmk vgquery\
			  lvreduce lvquery lvdelete lvchange pvinstall dudley \
			  avoidchk resync pvquery pvpquery vgkeys lvrealloc \
			  setstale pvremove pvdelete lvquerymap pvchange \
			  lvextend msck

HDRS			= alloc.h queueproto.h queuedefs.h lv_qmacros.h
INCFLAGS		= -I../.. -I${MAKETOP}/usr/sbin/newfs
CFLAGS			= 
COMMON_OFILES		= parse_opts.o error_util.o
avoidchk_OFILES		= avoidchk.o $(COMMON_OFILES)
blocktest_OFILES	= blocktest.o lv_block.o util.o
queues_OFILES		= queues.o util.o
lvdelete_OFILES		= lvdelete.o error_util.o
lvquery_OFILES		= lvchange.o $(COMMON_OFILES)
lvquerymap_OFILES	= lvchange.o $(COMMON_OFILES)
lvchange_OFILES		= lvchange.o $(COMMON_OFILES)
lvextend_OFILES		= lvextend.o $(COMMON_OFILES)
lvreduce_OFILES		= lvreduce.o $(COMMON_OFILES)
lvrealloc_OFILES	= lvrealloc.o error_util.o
lvmk_OFILES		= lvmk.o error_util.o
pvchange_OFILES		= pvchange.o $(COMMON_OFILES)
resync_OFILES		= resync.o $(COMMON_OFILES)
vgactivate_OFILES	= vgactivate.o $(COMMON_OFILES)
vgcreate_OFILES		= vgcreate.o $(COMMON_OFILES)
vgquery_OFILES		= vgquery.o error_util.o
setstale_OFILES		= setstale.o $(COMMON_OFILES)
pvremove_OFILES		= pvdelete.o $(COMMON_OFILES)
pvdelete_OFILES		= pvdelete.o $(COMMON_OFILES)
msck_OFILES		= msck.o $(COMMON_OFILES)
GARBAGE			= lv_block.c

TAGSFILES	= $(TAGSFILES) $(HDRS)

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/depend.mk

lv_block.c: ../lv_block.c
	cp $? ./$@.out
	mv -f ./$@.out ./$@

-include depend.mk
