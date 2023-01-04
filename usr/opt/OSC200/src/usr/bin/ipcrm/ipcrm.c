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
static char	*sccsid = "@(#)$RCSfile: ipcrm.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/11 16:59:39 $";
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
 * COMPONENT_NAME: (CMDIPC) ipc commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27 3
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/* ipcrm.c	1.6  com/cmd/ipc,3.1,9021 3/18/90 08:21:43 */

#include	<sys/types.h>
#include	<sys/ipc.h>
#include	<sys/msg.h>
#include	<sys/sem.h>
#include	<sys/shm.h>
#include	<sys/errno.h>
#include 	<grp.h>
#include	<stdio.h>
#include	<locale.h>
#include 	<nl_types.h>
#include 	"ipcrm_msg.h"

nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_IPCRM,Num,Str)

char opts[] = "q:m:s:Q:M:S:";	/* allowable options for getopt */
extern char	*optarg;	/* arg pointer for getopt */
extern int	optind; 	/* option index for getopt */
extern int	errno;		/* error return */

/*
 * NAME:	ipcrm - IPC remove
 * FUNCTION:	Remove specified message queues, semaphore sets and shared 
 *		memory ids.
 */

main(argc, argv)
int	argc;	/* arg count */
char	**argv; /* arg vector */
{
	register int	o;	/* option flag */
	register int	err;	/* error count */
	register int	ipc_id; /* id to remove */
	register key_t	ipc_key;/* key to remove */
	extern	int	atoi();
	int     ruid;           /* real user id                 */

	catd = catopen(MF_IPCRM,NL_CAT_LOCALE);
	(void) setlocale (LC_ALL,"");
		/* if the caller's real uid is not root and  */
		/* the caller is not in the system group then set the     */
		/* effective uid back to the real uid 		  	  */
        ruid = getuid();
        if (ruid && !sysgrp())
        {
		setuid(ruid);
        }

	/* Go through the options */
	err = 0;
	while ((o = getopt(argc, argv, opts)) != EOF)
		switch(o) {

		case 'q':	/* message queue */
			ipc_id = atoi(optarg);
			if (msgctl(ipc_id, IPC_RMID, 0) == -1)
				oops("msqid", (long)ipc_id);
			break;

		case 'm':	/* shared memory */
			ipc_id = atoi(optarg);
			if (shmctl(ipc_id, IPC_RMID, 0) == -1)
				oops("shmid", (long)ipc_id);
			break;

		case 's':	/* semaphores */
			ipc_id = atoi(optarg);
			if (semctl(ipc_id, IPC_RMID, 0) == -1)
				oops("semid", (long)ipc_id);
			break;

		case 'Q':	/* message queue (by key) */
			ipc_key = (key_t)atoi(optarg);
			if ((ipc_id=msgget(ipc_key, 0)) == -1
				|| msgctl(ipc_id, IPC_RMID, 0) == -1)
				oops("msgkey", ipc_key);
			break;

		case 'M':	/* shared memory (by key) */
			ipc_key = (key_t)atoi(optarg);
			if ((ipc_id=shmget(ipc_key, 0, 0)) == -1
				|| shmctl(ipc_id, IPC_RMID, 0) == -1)
				oops("shmkey", ipc_key);
			break;

		case 'S':	/* semaphores (by key) */
			ipc_key = (key_t)atoi(optarg);
			if ((ipc_id=semget(ipc_key, 0, 0)) == -1
				|| semctl(ipc_id, IPC_RMID, 0) == -1)
				oops("semkey", ipc_key);
			break;

		default:
		case '?':	/* anything else */
			err++;
			break;
		}
	if (err || (optind < argc)) {
		fprintf(stderr,MSGSTR(USAGE1, "usage: ipcrm [-q msqid] [-m shmid] [-s semid]\n")); /*MSG*/
		fprintf(stderr,MSGSTR(USAGE2, "	[-Q msgkey] [-M shmkey] [-S semkey]\n")); /*MSG*/
		exit(1);
	}
}

oops(s, i)
char *s;
long   i;
{
	char *e;

	switch (errno) {

	case	ENOENT: /* key not found */
	case	EINVAL: /* id not found */
	case	EIDRM:	/* id no longer in use */
		e = MSGSTR(ONOTFOUND,"not found"); /*MSG*/
		break;

	case	EPERM:	/* permission denied */
		e = MSGSTR(ONOPERM,"permission denied"); /*MSG*/
		break;
	default:
		e = MSGSTR(OUNKNOWN,"unknown error"); /*MSG*/
	}

	fprintf(stderr, "ipcrm: %s(%ld): %s\n", s, i, e);
}

/*
 * sysgrp
 *      return TRUE if (one of) our groups is the system group (0).
 *      on machines where each process has exactly one group,
 *      return !getgid();
 */
sysgrp()
{
        extern char *malloc();
        int *gidset = (int *)malloc(NGROUPS * sizeof(int));
        int ngr;

        if(gidset == NULL) {
                ngr = getgid();
                return (!ngr);
        }
        ngr = getgroups(NGROUPS, gidset);


        while (ngr-- > 0)
                if (gidset[ngr] == 0)
                        return TRUE;

        return FALSE;
} /* sysgrp */
