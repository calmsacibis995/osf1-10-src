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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: gtcfile.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/09/07 16:06:08 $";
#endif
/* 
 * COMPONENT_NAME: UUCP gtcfile.c
 * 
 * FUNCTIONS: commitall, gtcfile, svcfile, wfabort, wfcommit 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/
/*
gtcfile.c	1.3  com/cmd/uucp,3.1,9013 10/10/89 13:41:59";
*/

/*	/sccs/src/cmd/uucp/s.gtcfile.c
	gtcfile.c	1.1	7/29/85 16:33:07
*/
#include "uucp.h"
/* VERSION( gtcfile.c	5.2 -  -  ); */

#define NCSAVE	30	/* no more than 30 saved C files, please */
static int ncsave;
static struct {
	char	file[NAMESIZE];
	char	sys[NAMESIZE];
} csave[NCSAVE];

/*
 *	svcfile  - save the name of a C. file for system sys for re-using
 *	returns
 *		none
 */

svcfile(file, sys)
char	*file, *sys;
{
	ASSERT(ncsave < NCSAVE, MSGSTR(MSG_GTC1, "TOO MANY SAVED C FILES"),
			 "", ncsave);
	(void) strcpy(csave[ncsave].file, BASENAME(file, '/'));
	(void) strcpy(csave[ncsave].sys, sys);
	ncsave++;
	return;
}

/*
 *	gtcfile - copy into file the name of the saved C file for system sys
 *
 *	returns
 *		SUCCESS	-> found one
 *		FAIL	-> none saved
 *		
 */

gtcfile(file, sys)
char	*file, *sys;
{
	register int	i;

	for (i = 0; i < ncsave; i++)
		if (strncmp(sys, csave[i].sys, SYSNSIZE) == SAME) {
			(void) strcpy(file, csave[i].file);
			return(SUCCESS);
		}
	
	return(FAIL);
}

/*	commitall()
 *
 *	commit any and all saved C files
 *
 *	returns
 *		nothing
 */

commitall()
{
	/* not an infinite loop; wfcommit() decrements ncsave */
	while (ncsave > 0)
		wfcommit(csave[0].file, csave[0].file, csave[0].sys);
}

/*
 *	wfcommit - move wfile1 in current directory to SPOOL/sys/wfile2
 *	return
 *		none
 */

wfcommit(wfile1, wfile2, sys)
char	*wfile1, *wfile2, *sys;
{
	int	i;
	char	cmitfile[MAXFULLNAME];
	char	*file1Base, *file2Base;

	DEBUG(6, "commit %s ", wfile2);
	mkremdir(sys);		/* sets RemSpool */
	file1Base = BASENAME(wfile1, '/');
	file2Base = BASENAME(wfile2, '/');
	sprintf(cmitfile, "%s/%s", RemSpool, file2Base);
	DEBUG(6, "to %s\n", cmitfile);
	
	ASSERT(access(cmitfile, 0) != 0, Fl_EXISTS, cmitfile, 0);
	ASSERT(xmv(wfile1, cmitfile) == 0, Ct_LINK, cmitfile, errno);

	/* if wfile1 is a saved C. file, purge it from the saved list */
	for (i = 0; i < ncsave; i++) {
		if (EQUALS(file1Base, csave[i].file)) {
			--ncsave;
			(void) strcpy(csave[i].file, csave[ncsave].file);
			(void) strcpy(csave[i].sys, csave[ncsave].sys);
			break;
		}
	}
	return;
}

/*	wfabort - unlink any and all saved C files
 *	return
 *		none
 */

wfabort()
{
	register int	i;

	for (i = 0; i < ncsave; i++)
		(void) unlink(csave[i].file);
	ncsave = 0;
	return;
}
