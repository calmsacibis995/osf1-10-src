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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: hash_string.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1992/12/07 16:19:50 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* hash_string.c
 * Function to hash a string into a small integer
 *
 * OSF/1 Release 1.0
 */

#include <sys/types.h>
#ifndef _NO_PROTO
#include <loader.h>
#endif

#include <ldr_main_types.h>

#include "ldr_types.h"
#include "ldr_hash.h"
#include "hash_rbytes.h"

unsigned
#ifdef __STDC__
hash_string(const char *string, unsigned modulus)
#else
hash_string(string, modulus)
char *string;
unsigned modulus;
#endif

/* Hash a string into an unsigned integer that is modulus the
 * specified modulus value.
 */
{
	unsigned	hash;
	unsigned long	i, byte;

	i = 0;
	hash = 0;

	while ((byte = (unsigned long)*string++) != '\0')
		hash = hash + rbytes[i = (i + byte)&(HASH_RBYTES_LEN - 1)];
	return (hash % modulus);
}


