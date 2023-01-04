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
static char	*sccsid = "@(#)$RCSfile: io.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/11/24 10:46:44 $";
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
#define	MAXLINE	1200	/* maximum input line */

char	in[MAXLINE];	/* input buffer */
int	eqnexit();
int noeqn;

main(argc,argv) int argc; char *argv[];{

	eqnexit(eqn(argc, argv));
}

eqnexit(n) {
#ifdef gcos
	if (n)
		fprintf(stderr, "run terminated due to eqn error\n");
	exit(0);
#endif
	exit(n);
}

eqn(argc,argv) int argc; char *argv[];{
	int i, type;

	setfile(argc,argv);
	init_tbl();	/* install keywords in tables */
	while ((type=getline(in)) != EOF) {
		eqline = linect;
		if (in[0]=='.' && in[1]=='E' && in[2]=='Q') {
			for (i=11; i<100; used[i++]=0);
			printf("%s",in);
			printf(".nr 99 \\n(.s\n.nr 98 \\n(.f\n");
			markline = 0;
			init();
			yyparse();
			if (eqnreg>0) {
				printf(".nr %d \\w'\\*(%d'\n", eqnreg, eqnreg);
				/* printf(".if \\n(%d>\\n(.l .tm too-long eqn, file %s, between lines %d-%d\n",	*/
				/*	eqnreg, svargv[ifile], eqline, linect);	*/
				printf(".nr MK %d\n", markline);	/* for -ms macros */
				printf(".if %d>\\n(.v .ne %du\n", eqnht, eqnht);
				printf(".rn %d 10\n", eqnreg);
				if(!noeqn)printf("\\*(10\n");
			}
			printf(".ps \\n(99\n.ft \\n(98\n");
			printf(".EN");
			if (lastchar == EOF) {
				putchar('\n');
				break;
			}
			if (putchar(lastchar) != '\n')
				while (putchar(gtc()) != '\n');
		}
		else if (type == lefteq && type != NULL)	      /* 001 */
			eqn_inline();
		else
			printf("%s",in);
	}
	return(0);
}

getline(s) register char *s; {
	register c;
	while((*s++=c=gtc())!='\n' && c!=EOF && c!=lefteq)
		if (s >= in+MAXLINE) {
			error( !FATAL, "input line too long: %.20s\n", in);
			in[MAXLINE] = '\0';
			break;
		}
	if (c==lefteq)
		s--;
	*s++ = '\0';
	return(c);
}

eqn_inline() {
	int ds;

	printf(".nr 99 \\n(.s\n.nr 98 \\n(.f\n");
	ds = oalloc();
	printf(".rm %d \n", ds);
	do{
		if (*in)
			printf(".as %d \"%s\n", ds, in);
		init();
		yyparse();
		if (eqnreg > 0) {
			printf(".as %d \\*(%d\n", ds, eqnreg);
			ofree(eqnreg);
		}
		printf(".ps \\n(99\n.ft \\n(98\n");
	} while (getline(in) == lefteq);
	if (*in)
		printf(".as %d \"%s", ds, in);
	printf(".ps \\n(99\n.ft \\n(98\n");
	printf("\\*(%d\n", ds);
	ofree(ds);
}

putout(p1) int p1; {
	extern int gsize, gfont;
	int before, after;
	if(dbg)printf(".\tanswer <- S%d, h=%d,b=%d\n",p1, eht[p1], ebase[p1]);
	eqnht = eht[p1];
	printf(".ds %d \\x'0'", p1);
	/* suppposed to leave room for a subscript or superscript */
#ifndef NEQN
	before = eht[p1] - ebase[p1] - VERT((ps*6*12)/10);
#else NEQN
	before = eht[p1] - ebase[p1] - VERT(3);	/* 3 = 1.5 lines */
#endif NEQN
	if (before > 0)
		printf("\\x'0-%du'", before);
	printf("\\f%c\\s%d\\*(%d%s\\s\\n(99\\f\\n(98",
		gfont, gsize, p1, rfont[p1] == ITAL ? "\\|" : "");
#ifndef NEQN
	after = ebase[p1] - VERT((ps*6*2)/10);
#else NEQN
	after = ebase[p1] - VERT(1);
#endif NEQN
	if (after > 0)
		printf("\\x'%du'", after);
	putchar('\n');
	eqnreg = p1;
}

max(i,j) int i,j; {
	return (i>j ? i : j);
}

oalloc() {
	int i;
	for (i=11; i<100; i++)
		if (used[i]++ == 0) return(i);
	error( FATAL, "no eqn strings left", i);
	return(0);
}

ofree(n) int n; {
	used[n] = 0;
}

setps(p) int p; {
	printf(".ps %d\n", EFFPS(p));
}

nrwid(n1, p, n2) int n1, p, n2; {
	printf(".nr %d \\w'\\s%d\\*(%d'\n", n1, EFFPS(p), n2);
}

setfile(argc, argv) int argc; char *argv[]; {
	static char *nullstr = "-";

	svargc = --argc;
	svargv = argv;
	while (svargc > 0 && svargv[1][0] == '-') {
		switch (svargv[1][1]) {

		case 'd': lefteq=svargv[1][2]; righteq=svargv[1][3]; break;
		case 's': gsize = atoi(&svargv[1][2]); break;
		case 'p': deltaps = atoi(&svargv[1][2]); break;
		case 'f': gfont = svargv[1][2]; break;
		case 'e': noeqn++; break;
		case 0:	goto endargs; 
		default: dbg = 1;
		}
		svargc--;
		svargv++;
	}
  endargs:
	ifile = 1;
	linect = 1;
	if (svargc <= 0) {
		curfile = stdin;
		svargv[1] = nullstr;
	}
	else
		openinfile();	/* opens up the first input file */
}

yyerror() {;}

init() {
	ct = 0;
	ps = gsize;
	ft = gfont;
	setps(ps);
	printf(".ft %c\n", ft);
}

error(fatal, s1, s2) int fatal; char *s1, *s2; {
	if (fatal>0)
	{ /* start first KAK-0002 */
#ifndef NEQN
		printf("eqn: fatal error: ");
#else NEQN
		printf("neqn: fatal error: ");
#endif NEQN
	} /* end first KAK-0002 */
	printf(s1,s2);
	printf("\nfile %s, between lines %d and %d\n",
		 svargv[ifile], eqline, linect);
#ifndef NEQN /* start second KAK-0002 */
	fprintf(stderr, "eqn: ");
#else NEQN
	fprintf(stderr, "neqn: ");
#endif NEQN /* end second KAK-0002 */
	if (fatal>0)
		fprintf(stderr, "fatal error: ");
	fprintf(stderr, s1, s2);
	fprintf(stderr, "\nfile %s, between lines %d and %d\n",
		 svargv[ifile], eqline, linect);
	if (fatal > 0)
		eqnexit(1);
}
