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
static char *rcsid = "@(#)$RCSfile: setrlimit_c.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/05/25 21:47:35 $";
#endif
extern void asm( const char *,...);
#pragma intrinsic( asm )

struct ints {
	unsigned int i0;
	unsigned int i1;
};

struct longs {
	unsigned long long l0;
	unsigned long long l1;
};

int setrlimit(int n, struct ints *si)
    {
	struct longs sl;
	int status;

	sl.l0 = si->i0;
	if (sl.l0 >= 0x7fffffff) sl.l0 = 0x7fffffffffffffff;
	sl.l1 = si->i1;
	if (sl.l1 >= 0x7fffffff) sl.l1 = 0x7fffffffffffffff;
	asm("ldiq %v0, 145; call_pal 0x83", n, &sl);
	asm("stl	%v0, 0(%0)", &status);

	return status;
    }
