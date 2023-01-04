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
static char rcsid[] = "@(#)$RCSfile: t2.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/06/10 17:53:05 $";
#endif
/*
 * HISTORY
 */
/*
static	char	*sccsid = "@(#)t2.c	8.1	(Japanese ULTRIX)  2/19/91";
static char sccsid[] = "@(#)t2.c	4.2 8/11/83";
*/

 /* t2.c:  subroutine sequencing for one table */
# include "t..c"
tableput()
{
	saveline();
	savefill();
	ifdivert();
	cleanfc();
	getcomm();
	getspec();
	gettbl();
	getstop();
	checkuse();
	choochar();
	maktab();
	runout();
	release();
	rstofill();
	endoff();
	restline();
}
