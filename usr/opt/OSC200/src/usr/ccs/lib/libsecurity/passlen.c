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
static char	*sccsid = "@(#)$RCSfile: passlen.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/04/01 20:24:42 $";
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
 * Copyright (c) 1987-1990 SecureWare, Inc.  All Rights Reserved.
 */


/*
 * Based on:

 */

/*LINTLIBRARY*/

/*
 * This file contains a routine used to make programs
 * more secure.  Specifically, this routine will compute the
 * minimum password length based on such parameters as:
 * the password lifetime, guess rate,
 * probability of guessing a password, and the alphabet size.
 *
 * The equations are taken directly from the DoD Password
 * Management Guideline (Green Book), CSC-STD-002-85, 12 April 1985.
 *
 * The one-character symbols used herein match those of the Green Book.
 */

#include <sys/secdefines.h>

/* #if SEC_BASE */ /*{*/

#include <sys/types.h>
#include <math.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>

#define	P		(1e-6)	/* prob of guessing passwd during lifetime */


/*
 * Compute and return the minimum acceptable password length allowable
 * given the password lifetime parameters and the login delay, both in
 * seconds.
 */
int
passlen(life_dur, login_delay, alphabet_size)
	time_t life_dur;
	time_t login_delay;
	int alphabet_size;
{
	double p;
	double l;
	double r;
	double g;
	double s;
	double a;
	double m;

	check_auth_parameters();

	if (login_delay == 0 )
		login_delay = (time_t) 1;

	/*
	 * All quantities dealing with time deal with minutes.
	 */
	l = (double) life_dur / 60.0;	/* l is in minutes */
	p = P;
	r = 60.0 / login_delay;
	a = (double) alphabet_size;


	/*
	 * Total guesses in lifetime.
	 */
	g = l * r;

	/*
	 * Total combinations of passwords possible.
	 */
	s = g/p;

	/*
	 * S = A^M, so  ln S = M ln A, solving for M, ==>
	 */
	m = log(s) / log(a);

	/*
	 * ==> the answer!   (Note that on fractional answers, we round UP
	 * to err on the side of extra security.
	 */
	return (int) ceil(m);
}
/* #endif */ /*} SEC_BASE */
