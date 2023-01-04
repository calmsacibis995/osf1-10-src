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
 *	@(#)$RCSfile: open_hash_pvt.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:39:56 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* open_hash_pvt.h
 * internal declarations for symbol table hashing functions
 * Depends on <standards.h>, "ldr_hash.h", "open_hash.h"
 *
 * OSF/1 Release 1.0
 */

#ifndef _H_OPEN_HASH_PVT
#define _H_OPEN_HASH_PVT

#ifndef	OPEN_HASH_MAXELEM
#define OPEN_HASH_MAXELEM	1
#endif /* OPEN_HASH_MAXELEM */

/* Hash table entry; just contains key and value */

typedef struct	{
	univ_t	key;			/* key */
	univ_t	value;			/* value */
} hashtab_entry;

/* Hash table; entries array is actually larger */

struct	hashtab_hdr	{
	int		hh_maxelem;	/* maximum allowable elements */
	int		hh_nelem;	/* current number of elements */
	ldr_hash_p	hh_hasher;	/* function to hash a key */
	ldr_hash_compare_p hh_comper;	/* function to compare two keys */
	open_hash_flags_t hh_flags;	/* flags for this table */
};

typedef	struct	{			/* internal def for htab */
	struct hashtab_hdr	h_header; /* header of table */
	hashtab_entry		h_entries[OPEN_HASH_MAXELEM]; /* actually bigger */
} int_hashtab_t;

#define	h_maxelem	h_header.hh_maxelem
#define	h_nelem		h_header.hh_nelem
#define	h_hasher	h_header.hh_hasher
#define	h_comper	h_header.hh_comper
#define	h_flags		h_header.hh_flags

/* Compute the size of a hash table of a specified number of elements */

#define	hash_table_size(nelem)	(sizeof(struct hashtab_hdr) + ((nelem) * sizeof (hashtab_entry)))

/* Internal procedures */

extern int
open_hash_nelem __((int nelem));

#endif /* _H_OPEN_HASH_PVT */
