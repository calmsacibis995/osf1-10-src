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
static char rcsid[] = "@(#)$RCSfile: uucpname.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/09/07 16:09:29 $";
#endif
/* 
 * COMPONENT_NAME: UUCP uucpname.c
 * 
 * FUNCTIONS: uucpname 
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
uucpname.c	1.3  com/cmd/uucp,3.1,9013 10/10/89 13:53:57";
*/
/*	/sccs/src/cmd/uucp/s.uucpname.c
	uucpname.c	1.1	7/29/85 16:33:52
*/
#include "uucp.h"
/* VERSION( uucpname.c	5.2 -  -  ); */

/*
 * get the uucp name
 * return:
 *	none
 */
void
uucpname(name)
register char *name;
{
	char *s;

#ifdef BSD4_2
	int nlen;
	char	NameBuf[MAXBASENAME + 1];

	/* This code is slightly wrong, at least if you believe what the */
	/* 4.1c manual says.  It claims that gethostname's second parameter */
	/* should be a pointer to an int that has the size of the buffer. */
	/* The code in the kernel says otherwise.  The manual also says that */
	/* the string returned is null-terminated; this, too, appears to be */
	/* contrary to fact.  Finally, the variable containing the length */
	/* is supposed to be modified to have the actual length passed back; */
	/* this, too, doesn't happen.  So I'm zeroing the buffer first, and */
	/* passing an int, not a pointer to one.  *sigh*

	/*		--Steve Bellovin	*/
	bzero(NameBuf, sizeof NameBuf);
	nlen = sizeof NameBuf;
	gethostname(NameBuf, nlen);
	s = NameBuf;
	s[nlen] = '\0';
#else /* !BSD4_2 */
#ifdef UNAME
	struct utsname utsn;
	char *p;

	uname(&utsn);
	s = utsn.nodename;
	if ((p = (char *) strchr(s,'.')) != NULL)
		*p = '\0';
#else /* !UNAME */
	char	NameBuf[MAXBASENAME + 1], *strchr();
	FILE	*NameFile;

	s = MYNAME;
	NameBuf[0] = '\0';

	if ((NameFile = fopen("/etc/whoami", "r")) != NULL) {
		/* etc/whoami wins */
		(void) fgets(NameBuf, MAXBASENAME + 1, NameFile);
		(void) fclose(NameFile);
		NameBuf[MAXBASENAME] = '\0';
		if (NameBuf[0] != '\0') {
			if ((s = strchr(NameBuf, '\n')) != NULL)
				*s = '\0';
			s = NameBuf;
		}
	}
#endif /* UNAME */
#endif /* BSD4_2 */

	(void) strncpy(name, s, MAXBASENAME);
	name[MAXBASENAME] = '\0';
	return;
}
