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
static char	*sccsid = "@(#)$RCSfile: oh_nelem.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/12/07 16:20:53 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* open_hash_nelem.c
 * Determine the number of elements of an open-addressed hash table
 *
 * OSF/1 Release 1.0
 */

#include <sys/types.h>
#ifndef _NO_PROTO
#include <loader.h>
#endif

#include <ldr_main_types.h>

#include "ldr_types.h"
#include "ldr_errno.h"
#include "ldr_hash.h"
#include "open_hash.h"
#include "open_hash_pvt.h"


const static int	prime100stab[] = {
	0,	101,	211,	307,	401,
	503,	601,	701,	809,	907,
	1009,
};

const static int	prime1000stab[] = { 0,
1009,	2003,	3001,	4001,	5003,	6007,	7001,	8009,	9001,	10007,
11003,	12007,	13001,	14009,	15013,	16001,	17011,	18013,	19001,	20011,
21001,	22003,	23003,	24001,	25013,	26003,	27011,	28001,	29009,	30011,
31013,	32003,	33013,	34019,	35023,	36007,	37003,	38011,	39019,	40009,
41011,	42013,	43003,	44017,	45007,	46021,	47017,	48017,	49003,	50021,
51001,	52009,	53003,	54001,	55001,	56003,	57037,	58013,	59009,	60013,
61001,	62003,	63029,	64007,	65003,	66029,	67003,	68023,	69001,	70001,
71011,	72019,	73009,	74017,	75011,	76001,	77003,	78007,	79031,	80021,
81001,	82003,	83003,	84011,	85009,	86011,	87011,	88001,	89003,	90001,
91009,	92003,	93001,	94007,	95003,	96001,	97001,	98009,	99013,	100003,
};

int
#if __STDC__
open_hash_nelem(int nelem)
#else
open_hash_nelem(nelem)
int nelem;
#endif

/* Determine the proper number of elements in an open-addressed hash
 * table with at least nelem elements.  The proper number of elements
 * must be a prime number larger than nelem.
 * We use a table lookup for finding prime numbers.  If the number
 * of elements is less than 1000, we use the smallest prime in the
 * next larger 100's group.  If the number of elements is greater
 * than 1000, we use the nearest prime in the next larger 1000's
 * group.  We support up to 100,000 elements.
 */
{
	int	group;
	int	new_nelem;

	if (nelem < 1000) {

		group = (nelem + 99) / 100;
		new_nelem = prime100stab[group];
	} else if (nelem < 100000) {

		group = (nelem + 999) / 1000;
		new_nelem = prime1000stab[group];
	} else {
		new_nelem = LDR_EINVAL;
	}

	return(new_nelem);
}
