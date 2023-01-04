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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: t1.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/10/11 19:12:33 $";
#endif
/*
 * HISTORY
 */
/*
static	char	*sccsid = "@(#)t1.c	8.2	(Japanese ULTRIX)  3/1/91";
static char sccsid[] = "@(#)t1.c	4.2 8/11/83";
*/

 /* t1.c: main control and input switching */

# include "t..c"
#include <signal.h>
# ifdef gcos
/* required by GCOS because file is passed to "tbl" by troff preprocessor */
# define _f1 _f
extern FILE *_f[];
# endif

# ifdef unix
# define MACROS "/usr/lib/tmac/tmac.s"
# define PYMACS "/usr/lib/tmac/tmac.m"
# endif

# ifdef gcos
# define MACROS "cc/troff/smac"
# define PYMACS "cc/troff/mmac"
# endif

# define ever (;;)

main(argc,argv)
char *argv[];
{
# ifdef unix
	int badsig();
	signal(SIGPIPE, badsig);
# endif
# ifdef gcos
	if(!intss()) tabout = fopen("qq", "w"); /* default media code is type 5 */
# endif

	setlocale(LC_ALL,"");
	defwcsconst();

	catd = catopen(CATFILE, NL_CAT_LOCALE);

	exit(tbl(argc,argv));
}


tbl(argc,argv)
char *argv[];
{
	char line[BUFSIZ];
	/* required by GCOS because "stdout" is set by troff preprocessor */
	tabin=stdin; 
	tabout=stdout;

	setinp(argc,argv);
	while (gets1(line))
	{
		fprintf(tabout, "%s\n",line);
		if (prefix(".TS", line))
			tableput();
	}
	fclose(tabin);
	return(0);
}
int sargc;
char **sargv;
setinp(argc,argv)
char **argv;
{
	sargc = argc;
	sargv = argv;
	sargc--; 
	sargv++;
	if (sargc>0)
		swapin();
}
swapin()
{
	while (sargc>0 && **sargv=='-') /* Mem fault if no test on sargc */
	{
		if (sargc<=0) return(0);
		if (match("-ms", *sargv))
		{
			*sargv = MACROS;
			break;
		}
		if (match("-mm", *sargv))
		{
			*sargv = PYMACS;
			break;
		}
		if (match("-TX", *sargv))
			pr1403=1;
		sargc--; 
		sargv++;
	}
	if (sargc<=0) return(0);
# ifdef unix
	/* file closing is done by GCOS troff preprocessor */
	if (tabin!=stdin) fclose(tabin);
# endif
	tabin = fopen(ifile= *sargv, "r");
	iline=1;
# ifdef unix
	/* file names are all put into f. by the GCOS troff preprocessor */
	fprintf(tabout, ".ds f. %s\n",ifile);
# endif
	if (tabin==NULL)
		error(catgets(catd, 1, 3, "Can't open file"));
	sargc--;
	sargv++;
	return(1);
}
# ifdef unix
badsig()
{
	signal(SIGPIPE, SIG_IGN);
	exit(0);
}
# endif

wchar_t *WCS_dTE;
wchar_t *WCS_dTC;
wchar_t *WCS_dTand;
wchar_t *WCS_Tlbrace;
wchar_t *WCS_Trbrace;
wchar_t *WCS_SPAN;

defwcsconst()
{
	WCS_dTE = towcs(.TE);
	WCS_dTC = towcs(.TC);
	WCS_dTand = towcs(.T&);
	WCS_Tlbrace = towcs(T{);
	WCS_Trbrace = towcs(T});
	WCS_SPAN = towcs(\\^);
}
