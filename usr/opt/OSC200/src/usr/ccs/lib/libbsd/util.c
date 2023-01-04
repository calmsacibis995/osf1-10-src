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
static char	*sccsid = "@(#)$RCSfile: util.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:44:44 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 

/*
 * COMPONENT_NAME: (LIBBSD)  Berkeley Compatibility Library
 *
 * FUNCTIONS: move, xalloc, fatal, xfree, mcan, itom, mcmp, xtoi, xtom, itox,
 *	      mtox, mfree 
 *
 * ORIGINS: 26 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

char *malloc();
void mfree();
#ifdef lint
int xv_oid;
#endif
#include <stdio.h>
#include "mp.h"


/*
 * Convert hex digit to binary value
 */
static int
xtoi(c)
	char c;
{
	if (c >= '0' && c <= '9') {
		return(c - '0');
	} else if (c >= 'a' && c <= 'f') {
		return(c - 'a' + 10);
	} else {
		return(-1);
	}
}



/*
 * Convert hex key to MINT key
 */
MINT *
xtom(key)
	char *key;
{	
	int digit;
	MINT *m = itom(0);
	MINT *d;
	MINT *sixteen;

	sixteen = itom(16);
	for (; *key; key++) {
		digit = xtoi(*key);
		if (digit < 0) {
			return(NULL);
		}
		d = itom(digit);
		mult(m,sixteen,m);
		madd(m,d,m);
		mfree(d);
	}
	mfree(sixteen);
	return(m);
}

static char
itox(d)
	short d;
{
	d &= 15;
	if (d < 10) {
		return('0' + d);
	} else {
		return('a' - 10 + d);
	}
}

/*
 * Convert MINT key to hex key
 */
char *
mtox(key)
	MINT *key;
{
	MINT *m = itom(0);
	MINT *zero = itom(0);
	short r;
	char *p;
	char c;
	char *s;
	char *hex;
	int size;

#	define BASEBITS	(8*sizeof(short) - 1)

	if (key->len >= 0) {
		size = key->len;
	} else {	
		size = -key->len; 
	}
	hex = malloc((unsigned) ((size * BASEBITS + 3)) / 4 + 1);
	if (hex == NULL) {
		return(NULL);
	}
	move(key,m);
	p = hex;
	do {
		sdiv(m,16,m,&r);
		*p++ = itox(r);
	} while (mcmp(m,zero) != 0);	
	mfree(m);
	mfree(zero);

	*p = 0;
	for (p--, s = hex; s < p; s++, p--) {
		c = *p;
		*p = *s;
		*s = c;
	}
	return(hex);
}

/*
 * Deallocate a multiple precision integer
 */
void
mfree(a)
	MINT *a;
{
	xfree(a);
	free((char *)a);
}
