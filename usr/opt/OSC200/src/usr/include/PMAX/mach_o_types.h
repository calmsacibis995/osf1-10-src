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
 *	@(#)$RCSfile: mach_o_types.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:07:00 $
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
 * mach_o_types.h
 * Machine-dependent type declarations used to reference Mach-O object files.
 * This header file is included by mach_o_format.h.
 * 
 * Version for the MIPS DECstation (PMAX, PMIN)
 *
 * NOTE:  THIS IS PRELIMINARY AND WILL CHANGE
 *
 * OSF/1 Release 1.0
 */

#ifndef _H_MACH_O_TYPES
#define _H_MACH_O_TYPES

#include <mach_o_vals.h>

/* declarations for typedefs used in place of C base types */
/*
 * mo_byte_t is the data type used for pointer arithmetic by programs
 * referencing portions of a Mach-O object file.  Therefore offsets
 * are in terms of this data type and pointers are cast to this
 * data type.
 */

typedef unsigned short		mo_short_t;	/* half word (16 bits) */
typedef unsigned long		mo_long_t;	/* whole word (32 bits) */
typedef unsigned char		mo_byte_t;	/* byte */

#ifdef __STDC__
typedef void *			mo_ptr_t;
#else
typedef char *			mo_ptr_t;
#endif   /* __STDC__ */

#include <sys/types.h>

/* the type used for file offsets; off_t is not used because it is
 * sometimes signed */
typedef unsigned long		mo_offset_t;
 
typedef caddr_t			mo_vm_addr_t;	/* VM address */

typedef   unsigned long		mo_cpu_type_t;
typedef   unsigned long		mo_cpu_subtype_t;
typedef   unsigned long		mo_vendor_type_t;

/* definitions of macros to test for invalid values for certain data types. */
#define VALID_MO_LONG_MASK	0x3
#define VALID_MO_LONG_PTR(p)	(!((unsigned long)(p) & VALID_MO_LONG_MASK))


/* definition of an invalid vm address in a region load command;
 * zero is not used because that is a possible address
 */

#define MO_REG_INVALID_VM_ADDR  0xffffffff

/* definitions of more invalid field values */
#define MO_INVALID_LCID		0xffffffff
#define MO_INVALID_PKG_INDEX	0xffff

/* tentative definitions of region usage types for the MIPS */

#define REG_TEXT_T	1
#define REG_DATA_T	2
#define REG_RDATA_T	3
#define REG_SDATA_T	4
#define REG_BSS_T	5
#define REG_SBSS_T	6
#define REG_GLUE_T      7

#define MAX_USAGE_TYPE	7		/* highest usage type value defined */

/* declarations for function descriptor tuples, if they are implemented */

/* definitions for relocation types */
#define R_ABS_T		0
#define R_REFHALF_T	1
#define R_REFWOR_T	2
#define R_JMPADDR_T	3
#define R_REFHI_T	4
#define R_REFLO_T	5
#define R_GPREL_T	6
#define R_LITERAL_T	7

/* definition for the value of the mo_header_t.moh_magic field
 * that is used on this machine; this represents the way the canonical
 * form of the magic number looks 
 */

#define OUR_MOH_MAGIC		MOH_MAGIC_LSB

/* definition for the value of the mo_header_t.moh_byte_order field
 * that is used on this machine
 */

#define OUR_BYTE_ORDER		BO_LSB

/* definition for the value of the mo_header_t.moh_data_rep_id field
 * that is used on this machine
 */

#define OUR_DATA_REP_ID		DREP_GCC_LSB

/* definition for the value of the mo_header_t.moh_cpu_type field
 * that is used on this machine 
 */

#define OUR_CPU_TYPE		MO_CPU_TYPE_MIPS

/* definition for the value of the mo_header_t.moh_cpu_subtype field
 * that is used on this machine
 */

#define OUR_CPU_SUBTYPE		MO_CPU_SUBTYPE_PMAX

#endif /* _H_MACH_O_TYPES */

