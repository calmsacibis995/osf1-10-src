
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
static char rcsid[] = "@(#)$RCSfile: systat.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/09/07 16:08:10 $";
#endif
/* 
 * COMPONENT_NAME: UUCP systat.c
 * 
 * FUNCTIONS: STATNAME, callok, systat 
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
systat.c	1.4  com/cmd/uucp,3.1,9013 10/23/89 14:43:25";
*/
/*	/sccs/src/cmd/uucp/s.systat.c
	systat.c	1.3	7/29/85 16:33:29
*/
#include "uucp.h"
/* VERSION( systat.c	5.2 -  -  ); */

#define STATNAME(f, n) ((void)sprintf(f, "%s/%s", STATDIR, n))
#define S_SIZE 100

/*
 * make system status entry
 *	name -> system name
 *	text -> text string to read
 *	type -> ?
 * return:
 *	none
 */
void
systat(name, type, text, retry)
register int type;
char *name, *text;
int retry;
{
	register FILE *fp;
	int count;
	char filename[MAXFULLNAME], line[S_SIZE];
	time_t prestime;
	int presretry = 0;
	char *vec[5];

	line[0] = '\0';
	(void) time(&prestime);
	count = 0;
	STATNAME(filename, name);

	fp = fopen(filename, "r");
	if (fp != NULL) {
		if (fgets(line, S_SIZE, fp) != NULL)
			if (getargs(line, vec, 4) == 4) {
			    count = atoi(vec[1]);
			    presretry = atol(vec[3]);
			}
		if (count <= 0)
			count = 0;
		fclose(fp);
	}

	switch (type) {
		/* if there is already a status file - don't change */
	case SS_TIME_WRONG:
	case SS_LOCKED_DEVICE:
	case SS_CANT_ACCESS_DEVICE:
		if (count > 1)
		    return;
		break;

		/* reset count on successful call */
	case SS_OK:
	case SS_INPROGRESS:
	case SS_CONVERSATION:	/* failure during conversation - contact ok */
		retry = 0;
		count = 0;
		break;

		/* Startup failure means protocol mismatch--some administrative
		 * action must be taken
		 */
	case SS_STARTUP:
		retry = MAXRETRYTIME;
		count = 0;
		break;
	default:	/*  increment count and set up retry time */
		count++;
		if (!retry) {	/* use exponential backoff */
		    if (presretry < RETRYTIME)
			retry = RETRYTIME;
		    else {
			retry = presretry + presretry;
			if (retry > MAXRETRYTIME)
			    retry = MAXRETRYTIME;
		    }
		}
		else {	/* use specified time */
		    if (retry < RETRYTIME)
			retry = RETRYTIME;
		}
		break;
	}

	fp = fopen(filename, "w");
	ASSERT(fp != NULL, MSGSTR(MSG_SYSTAT_A1, "SYSTAT OPEN FAIL"), 
		filename, errno);
	(void) chmod(filename, 0666);
	(void) fprintf(fp, "%d %d %ld %ld %s %s\n",
		type, count, prestime, retry, text, name);
	(void) fclose(fp);
	return;
}

/*
 * check system status for call
 *	name -> system to check	
 * return:  
 *	0	-> ok
 *	>0	-> system status
 */
callok(name)
char *name;
{
	register FILE *fp;
	register int t;
	int retrytime;
	int count, type;
	char filename[MAXFULLNAME], line[S_SIZE];
	time_t lasttime, prestime;

	STATNAME(filename, name);
	fp = fopen(filename, "r");
	if (fp == NULL)
		return(SS_OK);

	if (fgets(line, S_SIZE, fp) == NULL) {

		/* no data */
		fclose(fp);
		(void) unlink(filename);
		return(SS_OK);
	}

	fclose(fp);
	(void) time(&prestime);
	sscanf(line, "%d%d%ld%ld", &type, &count, &lasttime, &retrytime);
	t = type;

	switch(t) {
	case SS_SEQBAD:
	case SS_LOGIN_FAILED:
	case SS_DIAL_FAILED:
	case SS_BAD_LOG_MCH:
	case SS_BADSYSTEM:
	case SS_CANT_ACCESS_DEVICE:
	case SS_WRONG_MCH:
	case SS_RLOCKED:
	case SS_RUNKNOWN:
	case SS_RLOGIN:
	case SS_UNKNOWN_RESPONSE:
	case SS_CHAT_FAILED:

		if (prestime - lasttime < retrytime) {
			logent(MSGSTR(MSG_SYSTAT_L1, "RETRY TIME NOT REACHED"), 
			       MSGSTR(MSG_SYSTAT_L2, "NO CALL"));
			DEBUG(4, "RETRY TIME (%ld) NOT REACHED\n", retrytime);
			return(type);
		}

		return(SS_OK);
	default:
		return(SS_OK);
	}
}
