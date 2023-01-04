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
static char	*sccsid = "@(#)$RCSfile: sqrt.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:29:26 $";
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
#ifndef lint

#endif

# include "e.h"

sqrt(p2) int p2; {
#ifndef NEQN
	int nps;

	nps = EFFPS(((eht[p2]*9)/10+5)/6);
#endif NEQN
	yyval = p2;
#ifndef NEQN
	eht[yyval] = VERT( (nps*6*12)/10 );
	if(dbg)printf(".\tsqrt: S%d <- S%d;b=%d, h=%d\n", 
		yyval, p2, ebase[yyval], eht[yyval]);
	if (rfont[yyval] == ITAL)
		printf(".as %d \\|\n", yyval);
#endif NEQN
	nrwid(p2, ps, p2);
#ifndef NEQN
	printf(".ds %d \\v'%du'\\s%d\\v'-.2m'\\(sr\\l'\\n(%du\\(rn'\\v'.2m'\\s%d", 
		yyval, ebase[p2], nps, p2, ps);
	printf("\\v'%du'\\h'-\\n(%du'\\*(%d\n", -ebase[p2], p2, p2);
	lfont[yyval] = ROM;
#else NEQN
	printf(".ds %d \\v'%du'\\e\\L'%du'\\l'\\n(%du'",
		p2, ebase[p2], -eht[p2], p2);
	printf("\\v'%du'\\h'-\\n(%du'\\*(%d\n", eht[p2]-ebase[p2], p2, p2);
	eht[p2] += VERT(1);
	if(dbg)printf(".\tsqrt: S%d <- S%d;b=%d, h=%d\n", 
		p2, p2, ebase[p2], eht[p2]);
#endif NEQN
}
