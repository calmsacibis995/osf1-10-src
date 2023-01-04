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
 *	@(#)$RCSfile: ldr_hash.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/01/29 14:30:49 $
 */ 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* ldr_hash.h
 * Common declarations for all loader hashing functions
 *
 * OSF/1 Release 1.0
 */

#ifndef _H_LDR_HASH
#define _H_LDR_HASH


/* The following declaration describes the action to be performed when
 * searching a hash table.  The following combinations are useful:
 *
 * LDR_HASH_LOOKUP		look up the specified key.  Return success
 *				and value if present, error if not present
 * LDR_HASH_INSERT		insert the specified key/value.  No error
 *				if the key is already present (this may
 *				result in duplicate keys).
 * LDR_HASH_DELETE		delete the specified key/value.  Does not
 *				return the value.  Return error if not present.
 * LDR_HASH_LOOKUP|LDR_HASH_INSERT insert the specified key/value.  If key is
 *				already present, don't insert duplicate, but
 *				return success and current value.
 * LDR_HASH_LOOKUP|LDR_HASH_INSERT delete the specified key/value.  Return
 *				the value on success, or error if not present.
 * LDR_HASH_INSERT|LDR_HASH_DELETE unspecified results.
 */

typedef	int	ldr_hash_action;	/* hash operation to perform: */
#define	LDR_HASH_LOOKUP ((ldr_hash_action)1) /* lookup key (no error if present) */
#define	LDR_HASH_INSERT ((ldr_hash_action)2) /* insert key (no error if not present) */
#define	LDR_HASH_DELETE ((ldr_hash_action)4) /* delete key (no error if present) */

/* Declarations of functions to hash and compare keys */

/* Hash a key, returning a value in the range 0..modulus. */

typedef	unsigned (*ldr_hash_p) __((const univ_t key, unsigned modulus));

/* Compare two keys, returning 0 for equality, nonzero for inequality 
 * (NOTE: like strcmp or memcmp, not like == )
 */

typedef int	 (*ldr_hash_compare_p) __((const univ_t key1, const univ_t key2));

/* Hash a string into an unsigned integer that is modulus the
 * specified modulus value.
 */

#ifdef __STDC__
extern unsigned
hash_string __((const char *string, unsigned modulus));
#else
extern unsigned hash_string();
#endif


#endif /* _H_LDR_HASH */
