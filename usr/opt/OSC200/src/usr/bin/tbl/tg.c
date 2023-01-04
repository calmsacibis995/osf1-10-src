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
static char rcsid[] = "@(#)$RCSfile: tg.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/06/10 22:49:11 $";
#endif
/*
 * HISTORY
 */
/*
static	char	*sccsid = "@(#)tg.c	8.1	(Japanese ULTRIX)  2/19/91";
static char sccsid[] = "@(#)tg.c	4.2 8/11/83";
*/

 /* tg.c: process included text blocks */
# include "t..c"
gettext(sp, ilin,icol, fn, sz)
wchar_t *sp;
char *fn, *sz;
{
	/* get a section of text */
	register char *cp;
	wchar_t wline[256];
	int oname;
	char *vs;

	if (texname==0) error(catgets(catd, 1, 29, "Too many text block diversions"));
	if (textflg==0)
	{
		fprintf(tabout, ".nr %d \\n(.lu\n", SL); /* remember old line length */
		textflg=1;
	}
	fprintf(tabout, ".eo\n");
	fprintf(tabout, ".am %02d\n", icol+80);
	fprintf(tabout, ".br\n");
	fprintf(tabout, ".di %c+\n", texname);
	rstofill();
	if (fn && *fn) fprintf(tabout, ".nr %d \\n(.f\n.ft %s\n", S1, fn);
	fprintf(tabout, ".ft \\n(.f\n"); /* protect font */
	vs = vsize[stynum[ilin]][icol];
	if ((sz && *sz) || (vs && *vs))
	{
		fprintf(tabout, ".nr %d \\n(.v\n", S2);
		if (vs==0 || *vs==0) vs= "\\n(.s+2";
		if (sz && *sz)
			fprintf(tabout, ".ps %s\n",sz);
		fprintf(tabout, ".vs %s\n",vs);
		fprintf(tabout, ".if \\n(%du>\\n(.vu .sp \\n(%du-\\n(.vu\n", S2,S2);
	}
	if (cll[icol][0])
		fprintf(tabout, ".ll %sn\n", cll[icol]);
	else
		fprintf(tabout, ".ll \\n(%du*%du/%du\n",SL,ctspan(ilin,icol),ncol+1);
	fprintf(tabout,".if \\n(.l<\\n(%d .ll \\n(%du\n", icol+CRIGHT, icol+CRIGHT);
	if (ctype(ilin,icol)=='a')
		fprintf(tabout, ".ll -2n\n");
	fprintf(tabout, ".in 0\n");
	while (wgets1(wline))
	{
		if (wline[0]=='T' && wline[1]=='}' && wline[2]== (wchar_t)tab) break;
		if (wmatch(WCS_Trbrace, wline)) break;
		fprintf(tabout, "%s\n", cp = Tombs(wline));
		XFREE(cp);
	}
	if (fn && *fn) fprintf(tabout, ".ft \\n(%d\n", S1);
	if (sz && *sz) fprintf(tabout, ".br\n.ps\n.vs\n");
	fprintf(tabout, ".br\n");
	fprintf(tabout, ".di\n");
	fprintf(tabout, ".nr %c| \\n(dn\n", texname);
	fprintf(tabout, ".nr %c- \\n(dl\n", texname);
	fprintf(tabout, "..\n");
	fprintf(tabout, ".ec \\\n");
	/* copy remainder of line */
	if (wline[2])
		wtcopy (sp, wline+3);
	else
		*sp=0;
	oname=texname;
	texname = texstr[++texct];
	return(oname);
}
untext()
{
	rstofill();
	fprintf(tabout, ".nf\n");
	fprintf(tabout, ".ll \\n(%du\n", SL);
}
