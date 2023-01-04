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
# Makefile for static DAS IDS_NOX -- libids_nox.a
#
#       Ray Giroux      2/28/92
# 

LIBRARIES		= libids_nox.a
EXPLIB_TARGETS		= export_libids_nox.a

EXPDIR			= /usr/ccs/lib/

HARDCOPY_OBJECTS = \
ids__pipe.o  \
ids_color_sixel.o \
ids_converters.o \
ids_export_ps.o \
ids_export_utils.o \
ids_extensions.o \
ids_ps_mgmt.o \
ids_rendering_mgmt.o \
ids_render_utils.o

OFILES			= ${HARDCOPY_OBJECTS}

CFLAGS 		= -DIDS_NOX -DNEW_CDA_CALLS -DDAS_EXPAND_PROTO=1 -DCDA_EXPAND_PROTO=1 -DIDS

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/datafiles.mk
include ${MAKEFILEPATH}/compiler.mk
