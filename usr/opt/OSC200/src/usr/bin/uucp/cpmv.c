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
static char rcsid[] = "@(#)$RCSfile: cpmv.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/09/07 16:04:38 $";
#endif
/* 
 * COMPONENT_NAME: UUCP cpmv.c
 * 
 * FUNCTIONS: toCorrupt, uidxcp, xcp, xfappend, xmv 
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
cpmv.c	1.4  com/cmd/uucp,3.1,9013 10/23/89 14:39:58";
*/
/*	/sccs/src/cmd/uucp/s.cpmv.c
	cpmv.c	1.4	7/29/85 16:32:50
*/
#include "uucp.h"
/* VERSION( cpmv.c	5.2 -  -  ); */

/*
 * copy f1 to f2 locally
 *	f1	-> source file name
 *	f2	-> destination file name
 * return:
 *	0	-> ok
 *	FAIL	-> failed
 */

xcp(f1, f2)
char *f1, *f2;
{
	register FILE *fp1, *fp2;
	register int n;
	char buf[BUFSIZ];
	char full[MAXFULLNAME];

	if ((fp1 = fopen(f1, "r")) == NULL)
		return(FAIL);
	(void) strcpy(full, f2);
	if (DIRECTORY(f2)) {
	    (void) strcat(full, "/");
	    (void) strcat(full, BASENAME(f1, '/'));
	    (void) strcpy(f2, full);
	}

	DEBUG(4, "full %s\n", full);
	if ((fp2 = fopen(full, "w")) == NULL) {
	    (void) fclose(fp1);
	    return(FAIL);
	}
	(void) chmod(full, 0666);

	/* copy -- check errors later */
	while ( (n = fread(buf, sizeof (char), sizeof buf, fp1)) != 0)
	    (void) fwrite(buf, sizeof (char), n, fp2);

	/* check for any errors */
	n = ferror(fp1) | ferror(fp2);
	n |= fclose(fp1) | fclose(fp2);

	if (n)
	    return(FAIL);
	return(0);
}


/*
 * move f1 to f2 locally
 * returns:
 *	0	-> ok
 *	FAIL	-> failed
 */

xmv(f1, f2)
register char *f1, *f2;
{
	register int ret;

	(void) unlink(f2);   /* i'm convinced this is the right thing to do */
	if ( (ret = link(f1, f2)) < 0) {
	    /* copy file */
	    ret = xcp(f1, f2);
	}

	if (ret == 0)
	    (void) unlink(f1);
	return(ret);
}


/* toCorrupt - move file to CORRUPTDIR
 * return - none
 */

void
toCorrupt(file)
char *file;
{
	char corrupt[MAXFULLNAME];

	(void) sprintf(corrupt, "%s/%s", CORRUPTDIR, BASENAME(file, '/'));
	(void) link(file, corrupt);
	ASSERT(unlink(file) == 0, Ct_UNLINK, file, errno);
	return;
}

/*
 * append f1 to f2
 *	f1	-> source FILE pointer
 *	f2	-> destination FILE pointer
 * return:
 *	SUCCESS	-> ok
 *	FAIL	-> failed
 */
xfappend(fp1, fp2)
register FILE	*fp1, *fp2;
{
	register int nc;
	char	buf[BUFSIZ];

	while ((nc = fread(buf, sizeof (char), BUFSIZ, fp1)) > 0)
		(void) fwrite(buf, sizeof (char), nc, fp2);

	return(ferror(fp1) || ferror(fp2) ? FAIL : SUCCESS);
}


/*
 * copy f1 to f2 locally under uid of uid argument
 *	f1	-> source file name
 *	f2	-> destination file name
 *	Uid and Euid are global
 * return:
 *	0	-> ok
 *	FAIL	-> failed
 * NOTES:
 *  for V7 systems, flip-flop between real and effective uid is
 *  not allowed, so fork must be done.  This code will not
 *  work correctly when realuid is root on System 5 because of
 *  a bug in setuid.
 */

uidxcp(f1, f2)
char *f1, *f2;
{
	int status;
	char full[MAXFULLNAME];

	(void) strcpy(full, f2);
	if (DIRECTORY(f2)) {
	    (void) strcat(full, "/");
	    (void) strcat(full, BASENAME(f1, '/'));
	}

	/* create full owned by uucp */
	(void) close(creat(full, 0666));
	(void) chmod(full, 0666);

	/* do file copy as read uid */
#ifndef V7
	(void) setuid(Uid);
	status = xcp(f1, full);
	(void) setuid(Euid);
	return(status);

#else

	if (vfork() == 0) {
	    setuid(Uid);
	    _exit (xcp(f1, full));
	}
	wait(&status);
	return(status);
#endif
}
