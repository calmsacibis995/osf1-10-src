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
 *	@(#)$RCSfile: mach_o_header_md.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:06:57 $
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
/* mach_o_header_md.h
 * 
 * This is the machine-dependent declaration of the header for the
 * OSF version of the Mach-O object file format, describing
 * the header as it is in the file.  Since a Mach-O header has
 * the same bit pattern on all machines, the way it is described
 * in C is machine-dependent.
 *
 * NOTE:  THIS IS PRELIMINARY AND WILL PROBABLY CHANGE
 *
 * OSF/1 Release 1.0
 */

#ifndef _H_MACH_O_HEADER_MD
#define _H_MACH_O_HEADER_MD

/*
* The only programs that should use this header file are
 * encode_mach_o_hdr and decode_mach_o_hdr.  All other programs
 * that have to look at the object header should call them and
 * use the header file mach_o_header.h.
 *
 * Version for the MIPS DECstation (PMAX, PMIN)
 *
 * The header appears at the very beginning of the file.
 * All header fields are stored in network byte order (big endian),
 * All fields are aligned in the natural way for 32-bit word machines
 * and are 2 or 4 bytes long, with no implicit padding between fields.
*/
/* For the MIPS, all the field sizes can be represented in normal C,
 * but the byte order is little endian.
 */

typedef struct raw_mo_header_t {
          unsigned long		rmoh_magic;    /* magic number = 0xdeafbead */
	  unsigned short	rmoh_major_version;
	  unsigned short	rmoh_minor_version;
	  unsigned short	rmoh_header_version;
	  unsigned short	rmoh_max_page_size; /* max linked for */
	  unsigned short	rmoh_byte_order;
          unsigned short	rmoh_data_rep_id; /* data rep aspects */
          unsigned long		rmoh_cpu_type;
          unsigned long		rmoh_cpu_subtype;
	  unsigned long		rmoh_vendor_type;
	  unsigned long		rmoh_flags;
	  unsigned long		rmoh_load_map_cmd_off; 
	  unsigned long		rmoh_first_cmd_off;
	  unsigned long		rmoh_sizeofcmds; /* size in bytes of cmds+map */
	  unsigned long		rmoh_n_load_cmds;
	  unsigned long		rmoh_reserved [2]; /* = 0, for future use */
	} raw_mo_header_t;


/* definition of the current value of the header version; each header .h file
 * should have its own definition of the value in case the files get out of 
 * synch.
 */

#define MO_RAW_HEADER_VERSION	1 


#endif /* _H_MACH_O_HEADER_MD */
