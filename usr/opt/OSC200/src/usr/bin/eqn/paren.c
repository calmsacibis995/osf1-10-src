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
static char	*sccsid = "@(#)$RCSfile: paren.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:29:14 $";
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

paren(leftc, p1, rightc) int p1, leftc, rightc; {
	int n, m, h1, j, b1, v;
	h1 = eht[p1]; b1 = ebase[p1];
	yyval = p1;
#ifndef NEQN
	lfont[yyval] = rfont[yyval] = 0;
	n = (h1+(6*EFFPS(ps)-1))/(6*EFFPS(ps));
#else NEQN
	n = max(b1+VERT(1), h1-b1-VERT(1)) / VERT(1);
#endif NEQN
	if( n<2 ) n = 1;
	m = n-2;
	if (leftc=='{' || rightc == '}') {
		n = n%2 ? n : ++n;
		if( n<3 ) n=3;
		m = n-3;
	}
#ifndef NEQN
	eht[yyval] = VERT(6 * ps * n);
	ebase[yyval] = b1 + (eht[yyval]-h1)/2;
	v = b1 - h1/2 + VERT( (ps*6*4)/10 );
#else NEQN
	eht[yyval] = VERT(2 * n);
	ebase[yyval] = (n)/2 * VERT(2);
	if (n%2 == 0)
		ebase[yyval] -= VERT(1);
	v = b1 - h1/2 + VERT(1);
#endif NEQN
	printf(".ds %d \\|\\v'%du'", yyval, v);
	switch( leftc ) {
		case 'n':	/* nothing */
		case '\0':
			break;
		case 'f':	/* floor */
			if (n <= 1)
				printf("\\(lf");
			else
				brack(m, "\\(bv", "\\(bv", "\\(lf");
			break;
		case 'c':	/* ceiling */
			if (n <= 1)
				printf("\\(lc");
			else
				brack(m, "\\(lc", "\\(bv", "\\(bv");
			break;
		case '{':
			printf("\\b'\\(lt");
			for(j = 0; j < m; j += 2) printf("\\(bv");
			printf("\\(lk");
			for(j = 0; j < m; j += 2) printf("\\(bv");
			printf("\\(lb'");
			break;
		case '(':
			brack(m, "\\(lt", "\\(bv", "\\(lb");
			break;
		case '[':
			brack(m, "\\(lc", "\\(bv", "\\(lf");
			break;
		case '|':
			brack(m, "|", "|", "|");
			break;
		default:
			brack(m, (char *) &leftc, (char *) &leftc, (char *) &leftc);
			break;
		}
	printf("\\v'%du'\\*(%d", -v, p1);
	if( rightc ) {
		printf("\\|\\v'%du'", v);
		switch( rightc ) {
			case 'f':	/* floor */
				if (n <= 1)
					printf("\\(rf");
				else
					brack(m, "\\(bv", "\\(bv", "\\(rf");
				break;
			case 'c':	/* ceiling */
				if (n <= 1)
					printf("\\(rc");
				else
					brack(m, "\\(rc", "\\(bv", "\\(bv");
				break;
			case '}':
				printf("\\b'\\(rt");
				for(j = 0; j< m; j += 2)printf("\\(bv");
				printf("\\(rk");
				for(j = 0; j< m; j += 2) printf("\\(bv");
				printf("\\(rb'");
				break;
			case ']':
				brack(m, "\\(rc", "\\(bv", "\\(rf");
				break;
			case ')':
				brack(m, "\\(rt", "\\(bv", "\\(rb");
				break;
			case '|':
				brack(m, "|", "|", "|");
				break;
			default:
				brack(m, (char *) &rightc, (char *) &rightc, (char *) &rightc);
				break;
		}
		printf("\\v'%du'", -v);
	}
	printf("\n");
	if(dbg)printf(".\tcurly: h=%d b=%d n=%d v=%d l=%c, r=%c\n", 
		eht[yyval], ebase[yyval], n, v, leftc, rightc);
}

brack(m, t, c, b) int m; char *t, *c, *b; {
	int j;
	printf("\\b'%s", t);
	for( j=0; j<m; j++)
		printf("%s", c);
	printf("%s'", b);
}
