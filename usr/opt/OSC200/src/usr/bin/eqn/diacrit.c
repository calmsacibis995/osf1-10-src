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
static char	*sccsid = "@(#)$RCSfile: diacrit.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:28:25 $";
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
# include "e.def"

diacrit(p1, type) int p1, type; {
	int c, t;
#ifndef NEQN
	int effps;
#endif NEQN

	c = oalloc();
	t = oalloc();
#ifdef NEQN
	nrwid(p1, ps, p1);
	printf(".nr 10 %du\n", max(eht[p1]-ebase[p1]-VERT(2),0));
#else NEQN
	effps = EFFPS(ps);
	nrwid(p1, effps, p1);
	printf(".nr 10 %du\n", VERT(max(eht[p1]-ebase[p1]-6*ps,0)));	/* vertical shift if high */
	printf(".if \\n(ct>1 .nr 10 \\n(10+\\s%d.25m\\s0\n", effps);
	printf(".nr %d \\s%d.1m\\s0\n", t, effps);	/* horiz shift if high */
	printf(".if \\n(ct>1 .nr %d \\s%d.15m\\s0\n", t, effps);
#endif NEQN
	switch(type) {
		case VEC:	/* vec */
#ifndef NEQN
			printf(".ds %d \\v'-.4m'\\s%d\\(->\\s0\\v'.4m'\n",
			    c, max(effps-3, 6));
			break;
#endif NEQN
		case DYAD:	/* dyad */
#ifdef NEQN
			printf(".ds %d \\v'-1'_\\v'1'\n", c);
#else NEQN
			printf(".ds %d \\v'-.4m'\\s%d\\z\\(<-\\(->\\s0\\v'.4m'\n",
			    c, max(effps-3, 6));
#endif NEQN
			break;
		case HAT:
			printf(".ds %d ^\n", c);
			break;
		case TILDE:
			printf(".ds %d ~\n", c);
			break;
		case DOT:
#ifndef NEQN
			printf(".ds %d \\s%d\\v'-.67m'.\\v'.67m'\\s0\n", c, effps);
#else NEQN
			printf(".ds %d \\v'-1'.\\v'1'\n", c);
#endif NEQN
			break;
		case DOTDOT:
#ifndef NEQN
			printf(".ds %d \\s%d\\v'-.67m'..\\v'.67m\\s0'\n", c, effps);
#else NEQN
			printf(".ds %d \\v'-1'..\\v'1'\n", c);
#endif NEQN
			break;
		case BAR:
#ifndef NEQN
			printf(".ds %d \\s%d\\v'.18m'\\h'.05m'\\l'\\n(%du-.1m\\(rn'\\h'.05m'\\v'-.18m'\\s0\n",
				c, effps, p1);
#else NEQN
			printf(".ds %d \\v'-1'\\l'\\n(%du'\\v'1'\n", 
				c, p1);
#endif NEQN
			break;
		case UNDER:
#ifndef NEQN
			printf(".ds %d \\l'\\n(%du\\(ul'\n", c, p1);
			printf(".nr %d 0\n", t);
			printf(".nr 10 0-%d\n", ebase[p1]);
#else NEQN
			printf(".ds %d \\l'\\n(%du'\n", c, p1);
#endif NEQN
			break;
		}
	nrwid(c, ps, c);
#ifndef NEQN
	if (lfont[p1] != ITAL)
		printf(".nr %d 0\n", t);
	printf(".as %d \\h'-\\n(%du-\\n(%du/2u+\\n(%du'\\v'0-\\n(10u'\\*(%d", 
		p1, p1, c, t, c);
	printf("\\v'\\n(10u'\\h'-\\n(%du+\\n(%du/2u-\\n(%du'\n", c, p1, t);
	/* BUG - should go to right end of widest */
#else NEQN
	printf(".as %d \\h'-\\n(%du-\\n(%du/2u'\\v'0-\\n(10u'\\*(%d", 
		p1, p1, c, c);
	printf("\\v'\\n(10u'\\h'-\\n(%du+\\n(%du/2u'\n", c, p1);
#endif NEQN
#ifndef NEQN
	if (type != UNDER)
		eht[p1] += VERT( (6*ps*15) / 100);	/* 0.15m */
	if(dbg)printf(".\tdiacrit: %c over S%d, lf=%c, rf=%c, h=%d,b=%d\n",
		type, p1, lfont[p1], rfont[p1], eht[p1], ebase[p1]);
#else NEQN
	if (type != UNDER)
		eht[p1] += VERT(1);
	if (dbg) printf(".\tdiacrit: %c over S%d, h=%d, b=%d\n", type, p1, eht[p1], ebase[p1]);
#endif NEQN
	ofree(c); ofree(t);
}
