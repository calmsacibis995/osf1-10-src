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
static char	*sccsid = "@(#)$RCSfile: main.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/10/07 19:22:35 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: (CMDLANG) Language Utilities
 *
 * FUNCTIONS: main, buserr, free1core, free2core, free3core,
		get1core, get2core, get3core, myalloc, segviol, yyerror
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * main.c	1.7  com/cmd/lang/lex,3.1,9021 3/19/90 08:56:33"; 
 */
# include "ldefs.h"
# include "once.h"
#ifndef _BLD
# include <stdlib.h>
# include <locale.h>
#endif
#ifndef _BLD
#include "lex_msg.h"
#define MSGSTR(Num,Str) catgets(catd,MS_LEX,Num,Str)
nl_catd catd;
#else
#define MSGSTR(Num,Str) Str
#endif

	/* lex [-[drcyvntf]] [file] ... [file] */

	/* Copyright 1976, Bell Telephone Laboratories, Inc.,
	    written by Eric Schmidt, August 27, 1976   */


main(argc,argv)
  int argc;
  char **argv; {
	register int i;
# ifdef DEBUG
	signal(10,buserr);
	signal(11,segviol);
# endif

#ifndef _BLD
	setlocale(LC_ALL, "");
	catd = catopen(MF_LEX,NL_CAT_LOCALE);
#endif

	while (argc > 1 && argv[1][0] == '-' ){
		i = 0;
		while(argv[1][++i]){
			switch (argv[1][i]){
# ifdef DEBUG
				case 'd': debug++; break;
				case 'y': yydebug = TRUE; break;
# endif
				case 'r': case 'R':
					warning(MSGSTR(NORATFOR, "Ratfor not currently supported with lex"));
					ratfor=TRUE; break;
				case 'c': case 'C':
					ratfor=FALSE; break;
				case 't': case 'T':
					fout = stdout;
					errorf = stderr;
					break;
				case 'v': case 'V':
					report = 1;
					break;
				case 'n': case 'N':
					report = 0;
					break;
				default:
					error(MSGSTR(BADOPTION, "Unknown option %c"),argv[1][i]);
				}
			}
		argc--;
		argv++;
		}
	sargc = argc;
	sargv = argv;
	if (argc > 1){
		fin = fopen(argv[++fptr], "r");		/* open argv[1] */
		sargc--;
		}
	else fin = stdin;
	if(fin == NULL){
		if ( argc > 1 )
			error(MSGSTR(NOINPUT, "Can't read input file %s"),
				argv[1]);
		else
			error(MSGSTR(NOSTDIN, "Can't read standard input"));
	}

	extra = (unsigned char *)calloc(nactions, sizeof(char));

	gch();
		/* may be gotten: def, subs, sname, schar, ccl, dchar */
	get1core();
		/* may be gotten: name, left, right, nullstr, parent */
	scopy("INITIAL",sp);
	sname[0] = sp;
	sp += slength("INITIAL") + 1;
	sname[1] = 0;
	if(yyparse(0)) exit(1);	/* error return code */
		/* may be disposed of: def, subs, dchar */
	free1core();
		/* may be gotten: tmpstat, foll, positions, gotof, nexts,
		 * nchar, state, atable, sfall, cpackflg */
	get2core();
	ptail();
	mkmatch();
# ifdef DEBUG
	if(debug) pccl();
# endif
	sect  = ENDSECTION;
	if(tptr>0)cfoll(tptr-1);
# ifdef DEBUG
	if(debug)pfoll();
# endif
	cgoto();
# ifdef DEBUG
	if(debug){
		printf("Print %d states:\n",stnum+1);
		for(i=0;i<=stnum;i++)stprt(i);
		}
# endif
		/* may be disposed of: positions, tmpstat, foll, state, name,
		 * left, right, parent, ccl, schar, sname */
		/* may be gotten: verify, advance, stoff */
	free2core();
	get3core();
	layout();
		/* may be disposed of: verify, advance, stoff, nexts, nchar,
			gotof, atable, ccpackflg, sfall */
# ifdef DEBUG
	free3core();
# endif

	/*
	 * Check for an environmental override of the lex finite-state machine
	 * skeleton.
	 */
	{
	  char *skeleton = (char *)getenv("LEXER");

	  if(skeleton)
	    cname = skeleton;
	}

	fother = fopen(ratfor?ratname:cname,"r");
	if(fother == NULL)
		error(MSGSTR(NOLEXDRIV, "Lex driver missing, file %s"),
			ratfor?ratname:cname);
	while ( (i=getc(fother)) != EOF)
		putc(i,fout);

	fclose(fother);
	fclose(fout);
	if(
# ifdef DEBUG
		debug   ||
# endif
			report == 1)statistics();
	fclose(stdout);
	fclose(stderr);
	exit(0);	/* success return code */
	}
get1core(){
	ccptr =	ccl = (unsigned char *)myalloc(CCLSIZE,sizeof(*ccl));
	pcptr = pchar = (unsigned char *)myalloc(pchlen, sizeof(*pchar));
	def = (unsigned char **)myalloc(DEFSIZE,sizeof(*def));
	subs = (unsigned char **)myalloc(DEFSIZE,sizeof(*subs));
	dp = dchar = (unsigned char *)myalloc(DEFCHAR,sizeof(*dchar));
	sname = (unsigned char **)myalloc(STARTSIZE,sizeof(*sname));
	sp = schar = (unsigned char *)myalloc(STARTCHAR,sizeof(*schar));

	if(ccl == 0 || def == 0 || subs == 0 || dchar == 0 || sname == 0
		|| schar == 0)
		error(MSGSTR(NOCORE1, "Too little core to begin"));
	}
free1core(){
	cfree((void *)def,DEFSIZE,sizeof(*def));
	cfree((void *)subs,DEFSIZE,sizeof(*subs));
	cfree((void *)dchar,DEFCHAR,sizeof(*dchar));
	}
get2core(){
	register int i;

	gotof = (int *)myalloc(nstates,sizeof(*gotof));
	nexts = (int *)myalloc(ntrans,sizeof(*nexts));
	nchar = (unsigned char *)myalloc(ntrans,sizeof(*nchar));
	state = (int **)myalloc(nstates,sizeof(*state));
	atable = (int *)myalloc(nstates,sizeof(*atable));
	sfall = (int *)myalloc(nstates,sizeof(*sfall));
	cpackflg = (char *)myalloc(nstates,sizeof(*cpackflg));
	tmpstat = (unsigned char *)myalloc(tptr+1,sizeof(*tmpstat));
	foll = (int **)myalloc(tptr+1,sizeof(*foll));
	nxtpos = positions = (int *)myalloc(maxpos,sizeof(*positions));

	if(tmpstat == 0 || foll == 0 || positions == 0 ||
		gotof == 0 || nexts == 0 || nchar == 0 || state == 0 ||
		atable == 0 || sfall == 0 || cpackflg == 0 )
		error(MSGSTR(NOCORE2, "Too little core for state generation"));

	for(i=0;i<=tptr;i++)foll[i] = 0;
	}
free2core(){
	cfree((void *)positions,maxpos,sizeof(*positions));
	cfree((void *)tmpstat,tptr+1,sizeof(*tmpstat));
	cfree((void *)foll,tptr+1,sizeof(*foll));
	cfree((void *)name,treesize,sizeof(*name));
	cfree((void *)left,treesize,sizeof(*left));
	cfree((void *)right,treesize,sizeof(*right));
	cfree((void *)parent,treesize,sizeof(*parent));
	cfree((void *)nullstr,treesize,sizeof(*nullstr));
	cfree((void *)state,nstates,sizeof(*state));
	cfree((void *)sname,STARTSIZE,sizeof(*sname));
	cfree((void *)schar,STARTCHAR,sizeof(*schar));
	cfree((void *)ccl,CCLSIZE,sizeof(*ccl));
	}
get3core(){
	verify = (int *)myalloc(outsize,sizeof(*verify));
	advance = (int *)myalloc(outsize,sizeof(*advance));
	stoff = (int *)myalloc(stnum+2,sizeof(*stoff));

	if(verify == 0 || advance == 0 || stoff == 0)
		error(MSGSTR(NOCORE3, "Too little core for final packing"));
	}
# ifdef DEBUG
free3core(){
	cfree((void *)advance,outsize,sizeof(*advance));
	cfree((void *)verify,outsize,sizeof(*verify));
	cfree((void *)stoff,stnum+1,sizeof(*stoff));
	cfree((void *)gotof,nstates,sizeof(*gotof));
	cfree((void *)nexts,ntrans,sizeof(*nexts));
	cfree((void *)nchar,ntrans,sizeof(*nchar));
	cfree((void *)atable,nstates,sizeof(*atable));
	cfree((void *)sfall,nstates,sizeof(*sfall));
	cfree((void *)cpackflg,nstates,sizeof(*cpackflg));
	}
# endif
void *
myalloc(a,b)
  int a,b; {
	register long i;
	i = (long)calloc(a, b);
	if(i==0L)
		warning(MSGSTR(CALLOCFAILED, "OOPS - calloc returns a 0"));
	else if(i == -1L){
# ifdef DEBUG
		warning("calloc returns a -1");
# endif
		return(0);
		}
	return((void *)i);
	}
# ifdef DEBUG
buserr(){
	fflush(errorf);
	fflush(fout);
	fflush(stdout);
	fprintf(errorf, "Bus error\n");
	if(report == 1)statistics();
	fflush(errorf);
	}
segviol(){
	fflush(errorf);
	fflush(fout);
	fflush(stdout);
	fprintf(errorf, "Segmentation violation\n");
	if(report == 1)statistics();
	fflush(errorf);
	}
# endif

yyerror(s)
char *s;
{
	fprintf(stderr, MSGSTR(LINE, "line %d: %s\n"), yyline, s);
}
