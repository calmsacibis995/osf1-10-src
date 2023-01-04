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
static char	*sccsid = "@(#)$RCSfile: expand.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:58:43 $";
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
#if !defined( lint) && !defined(_NOIDENT)

#endif
/*
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 
#if !lint
static char sccsid[] = "expand.c	1.1  com/cmd/acct/lib,3.1,8935 5/15/89 17:17:04";
#endif
#include <sys/types.h>
#include <sys/param.h>
#include <sys/acct.h>

/*
 *			ATTENTION
 *	This expand() routine is the "inverse" function of the
 *	compress() routine in kernel/bsd/kern_acct.c, which
 *	converts the kernel values into the pseudo floating
 *	point format.
 *	Changes of the compress() routine affect expand()!
 */

double
expand(t)
comp_t t;
{
	register long nt;
	register exp;
	double e;

	nt = t&017777;
	exp = (t >> 13) & 07;
	while (exp-- > 0)
		nt <<= 3;

	e = (double) (nt/AHZ) + (((double)(nt%AHZ))/AHZ);
	return(e);
}

