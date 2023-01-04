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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/11/11 17:07:48 $
#
# (c) COPYRIGHT 1991, DIGITAL EQUIPMENT CORPORATION
#	ALL RIGHTS RESERVED
#
#
# Makefile for shareable DAS IMG -- libimg.so
#
#	Ray Giroux	2/28/92
#

LIBIMG			= ${MAKETOP}usr/ccs/lib/libimg
VPATH			= ${LIBIMG}

SHARED_LIBRARIES	= libimg.so

LDFLAGS			= 
# EXPORTS			= -package libimg
LIBS			= -lchf -lips -lcda -lc -lm

EXPSHLIB_TARGETS	= export_libimg.so

ILIST			= libimg.so
IDIR			= /usr/shlib/
IMODE			= 644
NOSTRIP			=

INCFLAGS		= -I${LIBIMG} 

CFLAGS          	= -DNEW_CDA_SYMBOLS -DNEW_CDA_CALLS -DSV_SHLIBS -DDAS_EXPAND_PROTO=1 

OFILES			= ${OBJECTS}

OBJECTS	= img__attribute_access_utils.o \
img__attribute_utils.o \
img__block_utils.o \
img__convert_utils.o \
img__data_plane_utils.o \
img__data_utils.o \
img__ddif_prefix_mgt.o \
img__frame_utils.o \
img__itemlist_utils.o \
img__linked_list_utils.o \
img__memory_mgt.o \
img__roi.o \
img__verify_utils.o \
img__io_mgt.o \
img_adjust_comp_contrast.o \
img_attribute_access_utils.o \
img_bitmap_utils.o \
img_combine.o \
img_compress.o \
img_context_utils.o \
img_convert_utils.o \
img_convolve.o \
img_data_plane_utils.o \
img_ddif_export_frame.o \
img_ddif_export_page_break.o \
img_ddif_import_frame.o \
img_ddif_io_mgt.o \
img_decompress.o \
img_definition_utils.o \
img_dispatch.o \
img_dither.o \
img_error_handler.o \
img_export_ps.o \
img_export_sixels.o \
img_file_io_mgt.o \
img_frame_io_mgt.o \
img_filter.o \
img_flip.o \
img_frame_utils.o \
img_histogram.o \
img_lookup_tables.o \
img_lut_utils.o \
img_remap_services.o \
img_roi_utils.o \
img_rotate.o \
img_scale.o \
img_tonescale.o \
img_user_utils.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/compiler.mk
