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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/11/11 17:07:56 $
#
# (c) COPYRIGHT 1991, DIGITAL EQUIPMENT CORPORATION
#	ALL RIGHTS RESERVED
#
#
# Makefile for shareable DAS IPS -- libips.so
#
#	Ray Giroux	2/28/92
#
VPATH			= ${MAKETOP}usr/ccs/lib/libips
LIBIPS			= ${MAKETOP}usr/ccs/lib/libips

SHARED_LIBRARIES	= libips.so

USE_SHARED_LIBRARIES	=

EXPSHLIB_TARGETS	= export_libips.so
LDFLAGS			= 
# EXPORTS		= -package libips
LIBS			= -lc -lm

ILIST			= libips.so
IDIR			= /usr/shlib/
IMODE			= 644
NOSTRIP			=

INCFLAGS		= -I${LIBIPS}

# added on 11/01/93 for prototyping error correction in file ips__convolve.c
CFLAGS			= -DIPS -DDAS_EXPAND_PROTO=1 

OFILES			= ${OBJECTS}

OBJECTS = \
ips__arithmetic.o \
ips__constant_arithmetic.o \
ips__constrain_cont.o \
ips__change_list.o \
ips__combine.o \
ips__old_combine_bits.o \
ips__compare.o \
ips__constant_compare.o \
ips__convolve.o \
ips__copy.o \
ips__copy_utils.o \
ips__create_lut_mh.o \
ips__decode.o \
ips__decode_dct.o \
ips__decode_utils.o \
ips__dct_tables.o \
ips__dither_bluenoise.o \
ips__dither_ordered.o \
ips__encode.o \
ips__encode_dct.o \
ips__fax_decode_utils.o \
ips__fax_encode_utils.o \
ips__fft.o \
ips__ffx.o \
ips__fill.o \
ips__find_deltas.o \
ips__flip_horizontal.o \
ips__flip_horizontal_bitonal.o \
ips__flip_vertical.o \
ips__freq_filter.o \
ips__freq_utils.o \
ips__geometric.o \
ips__histogram.o \
ips__histogram_sorted.o \
ips__logical.o \
ips__logical_bitonal.o \
ips__match_histogram.o \
ips__mathematics.o \
ips__memory_mgt.o \
ips__merge_planes.o \
ips__move_bits.o \
ips__movc5.o \
ips__movv5.o \
ips__point_statistics.o \
ips__promote_data_type.o \
ips__remap_services.o \
ips__rotate_bitonal.o \
ips__rotate_cont.o \
ips__separate_plane.o \
ips__scale_bitonal.o \
ips__scale_cont.o \
ips__udp_utils.o \
ips_fax_decode_black.o  \
ips_fax_decode_white.o \
ips_fax_encode_black.o \
ips_fax_encode_white.o \
ips_fax2d_table.o


include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/compiler.mk

