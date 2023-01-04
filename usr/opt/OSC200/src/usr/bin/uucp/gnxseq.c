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
static char rcsid[] = "@(#)$RCSfile: gnxseq.c,v $ $Revision: 4.3.6.2 $ (DEC) $Date: 1993/09/07 16:06:02 $";
#endif
/* 
 * COMPONENT_NAME: UUCP gnxseq.c
 * 
 * FUNCTIONS: cmtseq, gnxseq, ulkseq 
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
gnxseq.c	1.5  com/cmd/uucp,3.1,9013 11/30/89 14:23:13";
*/
/*	/sccs/src/cmd/uucp/s.gnxseq.c
	gnxseq.c	1.1	7/29/85 16:33:06
*/
#include "uucp.h"
/* VERSION( gnxseq.c	5.2 -  -  ); */

/*
 * get next conversation sequence number
 *	rmtname	-> name of remote system
 * returns:
 *	0	-> no entery
 *	1	-> 0 sequence number
 */
gnxseq(rmtname)
char *rmtname;
{
	register FILE *fp0, *fp1;
	register struct tm *tp;
/*	extern struct tm *localtime();*/
	int count = 0, ct, ret;
	char buf[BUFSIZ], name[NAMESIZE];
	time_t clock;

	if (access(SQFILE, 0) != 0)
		return(0);

	{
		register int i;
	for (i = 0; i < 5; i++) 
		if ((ret = ulockf(SQLOCK, (time_t)  SQTIME)) == 0)
			break;
		sleep(5);
	}
	if (ret != 0) {
		logent(MSGSTR(MSG_GX1, "CAN'T LOCK"), SQLOCK);
		DEBUG(4, "can't lock %s\n", SQLOCK);
		return(0);
	}
	if ((fp0 = fopen(SQFILE, "r")) == NULL)
		return(0);
	if ((fp1 = fopen(SQTMP, "w")) == NULL) {
		fclose(fp0);
		return(0);
	}
	chmod(SQTMP, 0400);

	while (fgets(buf, BUFSIZ, fp0) != NULL) {
		ret = sscanf(buf, "%s%d", name, &ct);
		if (ret < 2)
			ct = 0;
		name[7] = '\0';
		if (ct > 9998)
			ct = 0;
		if (strncmp(rmtname, name, SYSNSIZE) != SAME) {
			fputs(buf, fp1);
			continue;
		}

		/*
		 * found name
		 */
		count = ++ct;
		time(&clock);
		tp = localtime(&clock);
		fprintf(fp1, "%s %d %d/%d-%d:%2.2d\n", name, ct,
		tp->tm_mon + 1, tp->tm_mday, tp->tm_hour,
		tp->tm_min);

		/*
		 * write should be checked
		 */
		while (fgets(buf, BUFSIZ, fp0) != NULL)
			fputs(buf, fp1);
	}
	fclose(fp0);
	fclose(fp1);
	if (count == 0) {
		rmlock(SQLOCK);
		unlink(SQTMP);
	}
	return(count);
}

/*
 * commit sequence update
 * returns:
 *	0	-> ok
 *	other	-> link failed
 */
cmtseq()
{
	register int ret;

	if ((ret = access(SQTMP, 0)) != 0) {
		rmlock(SQLOCK);
		return(0);
	}
	unlink(SQFILE);
	ret = link(SQTMP, SQFILE);
	unlink(SQTMP);
	rmlock(SQLOCK);
	return(ret);
}

/*
 * unlock sequence file
 */
ulkseq()
{
	unlink(SQTMP);
	rmlock(SQLOCK);
}
