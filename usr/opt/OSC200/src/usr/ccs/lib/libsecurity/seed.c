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
static char	*sccsid = "@(#)$RCSfile: seed.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/04/01 20:25:43 $";
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
 * Copyright (c) 1988-90 SecureWare, Inc.
 *   All rights reserved
 */


/*
 * Based on:

 */

#include <sys/secdefines.h>
#include "libsecurity.h"

/* #if SEC_BASE */ /*{*/

#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>

static long auto_seed();
static int  compute_checksum();


#ifdef	SEED_DEBUG
main(argc, argv)
	int argc;
	char *argv[];
{
	set_auth_parameters(argc, argv);

	if (argc != 1)  {
		printf(MSGSTR(SEED_1, "use: seed\n"));
		exit(1);
	}

	set_auth_parameters(argc, argv);

	(void) printf(MSGSTR(SEED_2, "seed is %ld\n"), auto_seed(0L));
	exit(0);
}
#endif



/*
 * Returns the seed (an integer) determined from 0 or more of the
 * seed parameters passed in.  If the return value is -1, something
 * went wrong with the seed generation.
 */
long
get_seed(prpwd)
	struct pr_passwd *prpwd;
{
	long bias = 0L;

	check_auth_parameters();

	/*
	 * A checksum on the Protected Password entry gives the desired
	 * random behavior.
	 */
	bias = (long) compute_checksum((char *) prpwd, sizeof(*prpwd));

	return auto_seed(bias);
}


/*
 * Set the seed.  This routine will only set the seed once, even if
 * called from multiple sources.
 */
void
set_seed(seed)
	long seed;
{
	static int been_here_before = 0;

	if (!been_here_before)  {
		been_here_before = 1;
		srand48(seed);
	}
}


/*
 * This algorithm produces a semi-random seed based soolely on system
 * parameters, and is not meant to be reproducible by either the same
 * user or different users executing it at the same time.
 *
 * Advantage:
 *	Results not influenced by administrator or user.
 *
 * Disadvantage:
 *	Results are somewhat predictable by user with sufficient
 *	knowledge of the system and algorithm.  (Even without the
 *	exact numbers, a small range will still produce a short
 *	list of possibilities.)
 *
 *	The auto_bias is derived from the Protected Password entry,
 *	whose contents are hidden and change frequently.
 */
static long
auto_seed(auto_bias)
	long auto_bias;
{
	ushort uids;
	ushort gids;
	int pid;
	long cur_time;
	long seed_val;

	/*
	 * The UIDs, GIDs and the PID are used to help randomize the
	 * value of the seed, in addition to the time.  Heaven forbid
	 * one of these is 0, since we multiply them together to
	 * use as the seed.  If one is 0, bump it to 1 so that the seed
	 * is not 0.
	 */
	uids = starting_ruid() + starting_euid();
	gids = starting_rgid() + starting_egid();
	pid = getpid();
	cur_time = time((time_t *) 0);
	if (uids == 0)
		uids = 1;
	if (gids == 0)
		gids = 1;
	if (pid == 0)
		pid = 1;
	if (cur_time == 0L)
		cur_time = 1L;

	/*
	 * Seed is a combination of the current time, the UIDs
	 * and the GIDs of the process.  Even when used
	 * simultaneously by multiple users, the uid and gid
	 * values will produce different results.
	 */
	seed_val = auto_bias + (long) (uids * gids * pid * cur_time);
	if (seed_val < 0L)
		seed_val = -seed_val;

	return seed_val;
}


static int
compute_checksum(buf, len)
	register char *buf;
	register int len;
{
	register int i;
	register int sum;

	/*
	 * Start with a non-zero value so a zero buffer contains a non-zero
	 * checksum.
	 */
	sum = 1;

	for (i = 0; i < len; i++)  {
		if(sum & 01)
			sum = (sum >> 1) + 0x8000;
		else
			sum >>= 1;
		sum += (int) *buf;
		sum &= 0xFFFF;
		buf++;
	}

	return sum;
}

/* #endif */ /*} SEC_BASE */
