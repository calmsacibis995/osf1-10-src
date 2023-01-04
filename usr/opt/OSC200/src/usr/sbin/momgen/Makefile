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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.8 $ (DEC) $Date: 1993/10/21 23:05:10 $
#
# /usr/sbin/momgen/Makefile
# Makefile for producing MOMGEN
# MOMGEN is not installed, but used during the build for creating MOMs

PROGRAMS	= momgen
OTHERS		= ntv_momgen
ILIST		= momgen
IDIR		= /usr/sbin
CFLAGS		= -std1 -D_XOPEN_SOURCE -DUNIXPLATFORM -DMCC -DMOSS_MATCH
INCFLAGS        = -I. -I/usr/include -I${MAKETOP}../../export/${target_machine}/usr/include/eca
LIBS		=  -lmir -lmoss -lpthreads -lmach -lc_r -lc  -lbsd 
 
HFILES		= file_defs.h insert_code_defs.h man_status.h mg_common.h mg_prototypes.h mom_defs.h \
		  string_defs.h types_setup.h

OFILES		= mg_common.o mg_extern.o mg_get.o mg_init.o mg_mir.o mg_oid.o mg_set.o \
		  mg_translate.o mg_umain.o mg_action.o mg_create.o mg_function.o mg_getnext.o \
		  mg_make.o mg_mms_com.o mg_parse.o mg_struct.o mg_traps.o

ntv_momgen:	${OFILES}
	 ${CC} -non_shared -o ntv_momgen ${OFILES} -L${MAKETOP}../../export/${target_machine}/usr/ccs/lib -lmir -lmoss -lpthreads -lc_r -lmach -lc_r -lc -lbsd -lc
include ${MAKEFILEPATH}/rules.mk
