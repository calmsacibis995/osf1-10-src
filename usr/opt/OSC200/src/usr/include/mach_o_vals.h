/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
/*	
 *	@(#)$RCSfile: mach_o_vals.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:17:30 $
 */ 
/*
 */
/*
 * Copyright (c) 1990
 * Open Software Foundation, Inc.
 * 
 * Permission is hereby granted to use, copy, modify and freely distribute
 * the software in this file and its documentation for any purpose without
 * fee, provided that the above copyright notice appears in all copies and
 * that both the copyright notice and this permission notice appear in
 * supporting documentation.  Further, provided that the name of Open
 * Software Foundation, Inc. ("OSF") not be used in advertising or
 * publicity pertaining to distribution of the software without prior
 * written permission from OSF.  OSF makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */
/*
 * mach_o_vals.h
 * Machine-independent definitions that are either used in machine-dependent
 * files or that should be in place that is "central" to all Mach-O
 * (OSF/ROSE) header files.
 *
 * OSF/1 Release 1.0
 */

#ifndef _H_MACH_O_VALS
#define _H_MACH_O_VALS

/* definitions for the mo_header_t.moh_magic field */

#define MOH_MAGIC_LSB		0xcefaefbe
#define MOH_MAGIC_MSB		0xbeefface

/* definitions for the mo_header_t.moh_byte_order field */

#define BO_LSB			1
#define BO_MSB			2

/* definitions for the mo_header_t.moh_data_rep_id field */

#define DREP_GCC_LSB		0x1	/* GCC, little endian */

/*
 * definitions for the mo_header_t.moh_cpu_type field; 
 * (the values may be changed/refined but will probably not duplicate
 * those in mach/machine.h -- these are for the loader to know
 * what kind of machines the code will run on)
 */

#define MO_CPU_TYPE_MIPS	((mo_cpu_type_t) 1)
#define MO_CPU_TYPE_NS32000	((mo_cpu_type_t) 2)
#define MO_CPU_TYPE_I386	((mo_cpu_type_t) 3)
#define MO_CPU_TYPE_M68000	((mo_cpu_type_t) 4)

/* definitions for the mo_header_t.moh_cpu_subtype field;
 * these will be changed/refined as well -- eventually there should be
 * a separate list for each class of machines
 */

#define MO_CPU_SUBTYPE_PMAX	((mo_cpu_subtype_t) 1)
#define MO_CPU_SUBTYPE_MMAX	((mo_cpu_subtype_t) 2)
#define MO_CPU_SUBTYPE_AT386    ((mo_cpu_subtype_t) 3)

/* definitions for the mo_header_t.moh_vendor_type field */

#define MO_VENDOR_TYPE_OSF	1
#define OUR_VENDOR_TYPE		MO_VENDOR_TYPE_OSF

/* definitions for the Mach-O (OSF/ROSE)-specific error values */

#define MO_ERROR_BAD_MAGIC		-100
#define MO_ERROR_BAD_HDR_VERS		-101
#define MO_ERROR_BAD_RAW_HDR_VERS 	-102
#define MO_ERROR_BUF2SML		-103
#define MO_ERROR_OLD_RAW_HDR_FILE	-104
#define MO_ERROR_UNSUPPORTED_VERS	-105

#define MO_HDR_CONV_SUCCESS		0

#endif /* _H_MACH_O_VALS */
