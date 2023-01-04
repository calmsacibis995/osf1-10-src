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
static char	*sccsid = "@(#)$RCSfile: ipcs.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/10/11 16:59:47 $";
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
 * ipcs.c	1.8  com/cmd/ipc,3.1,9021 3/18/90 08:21:59
 */

#include	<sys/secdefines.h>
#if SEC_BASE
#include	<sys/security.h>
#endif

#include	<sys/types.h>
#include	<sys/ipc.h>
#include	<sys/msg.h>
#include	<sys/sem.h>
#include	<sys/shm.h>
#include	<a.out.h>
#include	<fcntl.h>
#include	<time.h>
#include	<grp.h>
#include	<pwd.h>
#include	<stdio.h>
#include	<mach/boolean.h>
#include	<sys/table.h>

#include	<locale.h>
#include	<nl_types.h>
#include	"ipcs_msg.h"
nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_IPCS,Num,Str)
#if SEC_BASE
#define MSGSTR_SEC(Num,Str) catgets(catd,MS_IPCS_SEC,Num,Str)
#endif

char	chdr[80],
	chdr2[80],
				/* c option header format */
	opts[] = "abcmopqst";/* allowable options for getopt */
extern char	*optarg;	/* arg pointer for getopt */
int		bflg,		/* biggest size:
					segsz on m; qbytes on q; nsems on s */
		cflg,		/* creator's login and group names */
		mflg,		/* shared memory status */
		oflg,		/* outstanding data:
					nattch on m; cbytes, qnum on q */
		pflg,		/* process id's: lrpid, lspid on q;
					cpid, lpid on m */
		qflg,		/* message queue status */
		sflg,		/* semaphore status */
		tflg,		/* times: atime, ctime, dtime on m;
					ctime, rtime, stime on q;
					ctime, otime on s */
		err;		/* option error count */
extern int	optind; 	/* option index for getopt */

extern long		lseek();

/*
 * NAME:	ipcs - IPC status
 * FUNCTION: 	Examine and print certain things about message queues, 
 *		semaphores, and shared memory.
 */

main(argc, argv)
int	argc;	/* arg count */
char	**argv; /* arg vector */
{
	int		id;	/* message queue id */
	register int	i,	/* loop control */
			md,	/* memory file file descriptor */
			o;	/* option flag */
	struct shmid_ds mds;	/* shared memory data structure */
	int shmmni;	 	/* number of shared memory identifiers */
	struct msqid_ds qds;	/* message queue data structure */
	int msgmni;		/* number of message queue identifiers */
	struct semid_ds sds;	/* semaphore data structure */
	int semmni;		/* number of semaphore identifiers */
	key_t local_key;	/* local key for a remote queue */
	boolean_t none_allocated;

	catd = catopen(MF_IPCS,NL_CAT_LOCALE);
	(void) setlocale (LC_ALL,"");

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();
	if (!forcepriv(SEC_DEBUG)) {
		fprintf(stderr,
			MSGSTR_SEC(PRIV, "ipcs: insufficient privileges\n"));
		exit(1);
	}
#endif

	strcpy(chdr, MSGSTR(CHDR, "T     ID     KEY      MODE       OWNER    GROUP"));	/*MSG*/
	strcpy(chdr2, MSGSTR(CHDR2, "  CREATOR   CGROUP")); /*MSG*/
	/* Go through the options and set flags. */
	while((o = getopt(argc, argv, opts)) != EOF)
		switch(o) {
		case 'a':
			bflg = cflg = oflg = pflg = tflg = 1;
			break;
		case 'b':
			bflg = 1;
			break;
		case 'c':
			cflg = 1;
			break;
		case 'm':
			mflg = 1;
			break;
		case 'o':
			oflg = 1;
			break;
		case 'p':
			pflg = 1;
			break;
		case 'q':
			qflg = 1;
			break;
		case 's':
			sflg = 1;
			break;
		case 't':
			tflg = 1;
			break;
		case '?':
			err++;
			break;
		}
	if(err || (optind < argc)) {
		fprintf(stderr, MSGSTR(USAGE,
		    "usage:  ipcs [-abcmopqst]\n"));
		exit(1);
	}
	if((mflg + qflg + sflg) == 0)
		mflg = qflg = sflg = 1;

	/* Print Message Queue status report. */
	if(qflg) {
		if (table(TBL_MSGINFO, MSGINFO_MNI, &msgmni, 1, sizeof(int))) {
			i = 0;
			printf(MSGSTR(QMSG,"\nMessage Queues:\n"));
			fputs(chdr, stdout);
			
			if(cflg || oflg || bflg || pflg || tflg) {
				fputs(cflg ? chdr2 : "", stdout);
				fputs(oflg ? MSGSTR(QOMSG, " CBYTES  QNUM") : "", stdout);
				fputs(bflg ? MSGSTR(QBMSG, " QBYTES") : "", stdout);
				fputs(pflg ? MSGSTR(QPMSG, " LSPID LRPID") : "", stdout);
				fputs(tflg ? MSGSTR(QTMSG, "  STIME    RTIME    CTIME ") : "", stdout);
			}
			putchar('\n');
		} else {
			i = msgmni;
			printf(MSGSTR(MQNIS,"Error accessing Message Queue facility.\n"));
		}

		none_allocated = TRUE;
		while(i < msgmni) {
			table(TBL_MSGDS, i, &qds, 1, sizeof(qds));
				
			if(!(qds.msg_perm.mode & IPC_ALLOC)) {
				i++;
				continue;
			}
#if SEC_MAC
			if (!ipcs_visible(i, 'q')) {
				i++;
				continue;
			}
#endif

			none_allocated = FALSE;
			hp('q',"SRrw-rw-rw-",&qds.msg_perm,i++,msgmni);
			if(oflg)
				printf("%7u%6u", qds.msg_cbytes, qds.msg_qnum);
			if(bflg)
				printf("%7u", qds.msg_qbytes);
			if(pflg)
				printf("%6u%6u", qds.msg_lspid, qds.msg_lrpid);
			if(tflg) {
				tp(qds.msg_stime);
				tp(qds.msg_rtime);
				tp(qds.msg_ctime);
			}
			putchar('\n');
		}
		if (none_allocated)
			printf("*** No message queues are currently defined ***\n");
	}	

	/* Print Shared Memory status report. */
	if(mflg) {
		if (table(TBL_SHMINFO, SHMINFO_MNI, &shmmni, 1, sizeof(int))) {
			i = 0;
			printf(MSGSTR(MMSG,"\nShared Memory:\n")); /*MSG*/
			fputs(chdr, stdout);

			if(cflg || oflg || bflg || tflg || pflg) {
			    fputs(cflg ? chdr2 : "", stdout);
			    fputs(oflg ? MSGSTR(MOMSG," NATTCH") : "",stdout);
			    fputs(bflg ? MSGSTR(MBMSG,"  SEGSZ") : "", stdout);
			    fputs(pflg ? MSGSTR(MPMSG,"  CPID  LPID") : "", stdout);
			    fputs(tflg ? MSGSTR(MTMSG,"   ATIME    DTIME    CTIME ") : "", stdout);
		       }
			putchar('\n');
		} else {
			i = shmmni;
			printf(MSGSTR(NOSHMEM, "Error accessing Shared Memory facility.\n"));
		}

		none_allocated = TRUE;
		while(i < shmmni) {
			table(TBL_SHMDS, i, &mds, 1, sizeof(mds));
			if(!(mds.shm_perm.mode & IPC_ALLOC)) {
				i++;
				continue;
			}
#if SEC_MAC
			if (!ipcs_visible(i, 'm')) {
				i++;
				continue;
			}
#endif

			none_allocated = FALSE;
			hp('m',"DCrw-rw-rw-",&mds.shm_perm,i++, shmmni);
			if(oflg)
				printf("%7u", mds.shm_nattch);
			if(bflg)
				printf("%7d", mds.shm_segsz);
			if(pflg)
				printf("%6u%6u", mds.shm_cpid, mds.shm_lpid);
			if(tflg) {
				tp(mds.shm_atime);
				tp(mds.shm_dtime);
				tp(mds.shm_ctime);
			}
			putchar('\n');
		}

		if (none_allocated)
			printf("*** No shared memory segments are currently defined ***\n");
	}

	/* Print Semaphore facility status. */
	if(sflg) {
		if (table(TBL_SEMINFO, SEMINFO_MNI, &semmni, 1, sizeof(int))) {
			i = 0;
			printf(MSGSTR(SMSG,"\nSemaphores:\n"));
			fputs(chdr, stdout);

			if(cflg || bflg || tflg) {
			    fputs(cflg ? chdr2 : "", stdout);
			    fputs(bflg ? MSGSTR(SBMSG," NSEMS") : "", stdout);
			    fputs(tflg ? MSGSTR(STMSG, "   OTIME    CTIME ") : "", stdout);
			}
			putchar('\n');
		} else {
			i = semmni;
			printf(MSGSTR(SFNIS,"Error accessing Semaphore facility.\n"));
		}

		none_allocated = TRUE;
		while(i < semmni) {
			table(TBL_SEMDS, i, &sds, 1, sizeof(sds));
			
			if(!(sds.sem_perm.mode & IPC_ALLOC)) {
				i++;
				continue;
			}
#if SEC_MAC
			if (!ipcs_visible(i, 's')) {
				i++;
				continue;
			}
#endif

			none_allocated = FALSE;
			hp('s',"--ra-ra-ra-",&sds.sem_perm,i++, semmni);
			if(bflg)
				printf("%6u", sds.sem_nsems);
			if(tflg) {
				tp(sds.sem_otime);
				tp(sds.sem_ctime);
			}
			putchar('\n');
		}
		if (none_allocated)
			printf("*** No semaphores are currently defined ***\n");

	}
	putchar ('\n');
	exit(0);
}

/*
**	hp - common header print
*/

hp(type, modesp, permp, slot, mni)
char				type,	/* facility type */
				*modesp;/* ptr to mode replacement characters */
register struct ipc_perm	*permp; /* ptr to permission structure */
int				slot;	/* facility slot number */
int				mni;	/* xxxinfo.xxxmni field */
{
	register int		i,	/* loop control */
				j;	/* loop control */
	register struct group	*g;	/* ptr to group group entry */
	register struct passwd	*u;	/* ptr to user passwd entry */
	int	id;

	id = mni * permp->seq + slot;

	printf("%c%7d%s%#8d ", type, id, " ", permp->key);
	for(i = 02000;i;modesp++, i >>= 1)
		printf("%c", (permp->mode & i) ? *modesp : '-');
	if((type == 'Q') || (u = getpwuid(permp->uid)) == NULL)
		printf("%9d", permp->uid);
	else
		printf("%9.8s", u->pw_name);
	if((type == 'Q') || (g = getgrgid(permp->gid)) == NULL)
		printf("%9d", permp->gid);
	else
		printf("%9.8s", g->gr_name);
	if(cflg) {
		if((type == 'Q')||(u = getpwuid(permp->cuid)) == NULL)
			printf("%9d", permp->cuid);
		else
			printf("%9.8s", u->pw_name);
		if((type == 'Q')||(g = getgrgid(permp->cgid)) == NULL)
			printf("%9d", permp->cgid);
		else
			printf("%9.8s", g->gr_name);
	}
}

/*
**	tp - time entry printer
*/

tp(time)
time_t	time;	/* time to be displayed */
{
	register struct tm	*t;	/* ptr to converted time */

	if(time) {
		t = localtime(&time);
		printf(" %2d:%2.2d:%2.2d", t->tm_hour, t->tm_min, t->tm_sec);
	} else
		printf(MSGSTR(NOENTRY," no-entry")); /*MSG*/
}


#if SEC_MAC

#include <mandatory.h>

/*
 * Determine whether or not an ipc object is visible to the current
 * process under the MAC policy.
 */

ipcs_visible(id, type)
	int	id;
	char	type;
{
	static mand_ir_t *mand_ir = (mand_ir_t *) 0;
	static int has_auth;
	static int first_time = 1;

	if (first_time) {
		first_time = 0;
		if (has_auth = authorized_user("macquery"))
			return 1;
		mand_ir = mand_alloc_ir();
		if (mand_ir == (mand_ir_t *) 0) {
		    fprintf(stderr, MSGSTR_SEC(MACINIT,
					"ipcs: MAC initialization failed\n"));
		    exit(1);
		}
	}
	if (has_auth)
		return 1;

	/*
	 * Attempt to retrieve the target object's label.
	 * This will only succeed if we dominate the object.
	 */
	switch (type) {
	    case 'q':
		return msg_statslabel(id, mand_ir) == 0;
	    case 's':
		return sem_statslabel(id, mand_ir) == 0;
	    case 'm':
		return shm_statslabel(id, mand_ir) == 0;
	    default:
		return 0;
	}
}
#endif
