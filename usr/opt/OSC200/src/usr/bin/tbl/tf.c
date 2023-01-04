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
static char rcsid[] = "@(#)$RCSfile: tf.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/06/10 18:29:37 $";
#endif
/*
 * HISTORY
 */
/*
static	char	*sccsid = "@(#)tf.c	8.1	(Japanese ULTRIX)  2/19/91";
static char sccsid[] = "@(#)tf.c	4.2 8/11/83";
*/

 /* tf.c: save and restore fill mode around table */
# include "t..c"
savefill()
{
	/* remembers various things: fill mode, vs, ps in mac 35 (SF) */
	fprintf(tabout, ".de %d\n",SF);
	fprintf(tabout, ".ps \\n(.s\n");
	fprintf(tabout, ".vs \\n(.vu\n");
	fprintf(tabout, ".in \\n(.iu\n");
	fprintf(tabout, ".if \\n(.u .fi\n");
	fprintf(tabout, ".if \\n(.j .ad\n");
	fprintf(tabout, ".if \\n(.j=0 .na\n");
	fprintf(tabout, "..\n");
	fprintf(tabout, ".nf\n");
	/* set obx offset if useful */
	fprintf(tabout, ".nr #~ 0\n");
	fprintf(tabout, ".if n .nr #~ 0.6n\n");
}
rstofill()
{
	fprintf(tabout, ".%d\n",SF);
}
endoff()
{
	int i;
	for(i=0; i<MAXHEAD; i++)
		if (linestop[i])
			fprintf(tabout, ".nr #%c 0\n", 'a'+i);
	for(i=0; i<texct; i++)
		fprintf(tabout, ".rm %c+\n",texstr[i]);
	fprintf(tabout, "%s\n", last);
	XFREE(last);
}
ifdivert()
{
	fprintf(tabout, ".ds #d .d\n");
	fprintf(tabout, ".if \\(ts\\n(.z\\(ts\\(ts .ds #d nl\n");
}
saveline()
{
	fprintf(tabout, ".if \\n+(b.=1 .nr d. \\n(.c-\\n(c.-1\n");
	linstart=iline;
}
restline()
{
	fprintf(tabout,".if \\n-(b.=0 .nr c. \\n(.c-\\n(d.-%d\n", iline-linstart);
	linstart = 0;
}
cleanfc()
{
	fprintf(tabout, ".fc\n");
}
