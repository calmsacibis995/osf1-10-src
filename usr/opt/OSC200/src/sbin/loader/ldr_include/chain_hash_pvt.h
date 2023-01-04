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
 *	@(#)$RCSfile: chain_hash_pvt.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:39:41 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* chain_hash_pvt.h
 * internal declarations for symbol table hashing functions
 *
 * OSF/1 Release 1.0
 */

#ifndef _H_CHAIN_HASH_PVT
#define _H_CHAIN_HASH_PVT

/* Hash table; entries array is actually larger */

struct	hashtab_hdr	{
	int		hh_nelem;	/* number of slots */
	ldr_hash_p	hh_hasher;	/* function to hash a key */
	ldr_hash_compare_p hh_comper;	/* function to compare two keys */
};

typedef	struct	{			/* internal def for htab */
	struct hashtab_hdr	h_header; /* header of table */
	chain_hash_elem		*h_entries[1]; /* actually bigger */
} int_hashtab_t;

#define	h_nelem		h_header.hh_nelem
#define	h_hasher	h_header.hh_hasher
#define	h_comper	h_header.hh_comper

/* Compute the size of a hash table of a specified number of elements */

#define	hash_table_size(nelem)	(sizeof(struct hashtab_hdr) + ((nelem) * sizeof (chain_hash_elem *)))

#endif /* _H_CHAIN_HASH_PVT */
