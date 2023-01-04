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
/* Copyright (c) 1988-90, SecureWare, Inc.
 *   All rights reserved.
 *
 * ipcpolicy driver for security attributes on IPC objects
 *
 * Usage:  ipc{policy} [ -q message_queues ] [ -s semaphores ]
 *		       [ -m memory_segments ] [ -f file | attribute ]
 *
 * if used without a level, prints out the security attribute of the IPC
 * object(s).  If used with the level, changes the security attribute of the
 * IPC object(s).  The process must have proper permission to change the objects
 */

/* #ident "@(#)ipcpolicy.c	3.2 14:42:33 6/15/90 SecureWare" */

/*
 * Based on:
 *   "@(#)ipcpolicy.c	2.5.2.2 23:26:15 1/8/90 SecureWare"
 */

#include <sys/secdefines.h>
#include <locale.h>
#include "policy_msg.h"

nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_POLICY,n,s)

#if SEC_ARCH

#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/security.h>
#include <prot.h>

#define NULL_VALUE "NULL"
#define WILDCARD "WILDCARD"

#ifdef SEC_ACL_POSIX
uid_t   user_id;
gid_t   group_id;
#endif /* SEC_ACL_POSIX */

static int put_msg();
static int *getnumbers();
extern int strspn();
extern char *calloc();

/* routines defined in the IPC policy-specific support file */

extern int sem_getattr();
extern int sem_setattr();
extern int shm_getattr();
extern int shm_setattr();
extern int msg_getattr();
extern int msg_setattr();

/* routines defined in the conversion support file */

extern char *convert_ir();
extern char *convert_er();
extern char *file_to_buf();

#ifdef SEC_ACL_POSIX
/* Used to indicate to filetobuf.c that it is dealing with labels */

int acl_or_label;
#endif

main (argc, argv)
int argc;
char *argv[];
{
	extern int optind, opterr;
	extern char *optarg;
	int c;
	char qflag = 0, sflag = 0, mflag = 0, fflag = 0, errflag = 0;
	int qnumber, snumber, mnumber;
	int *qargs, *sargs, *margs;
	char *ir;
	char setting;
	int i;
	int ret;
	char buf[100];
	int size;
	char *sl_buf;
	struct msqid_ds msg_stat ;
	struct semid_ds sem_stat ;
	struct shmid_ds shm_stat ;


        (void) setlocale( LC_ALL, "" );
        catd = catopen(MF_POLICY,NL_CAT_LOCALE);

	set_auth_parameters(argc, argv);
	initprivs();

	opterr = 0;
	while ((c = getopt (argc, argv, "f:q:s:m:")) != -1)
		switch (c) {
		case 'q':
			qargs = getnumbers (optarg, &qnumber);
			if (qargs == (int *) 0 || qflag > 0)
				errflag++;
			else
				qflag++;
			break;
		case 's':
			sargs = getnumbers (optarg, &snumber);
			if (sargs == (int *) 0 || sflag > 0)
				errflag++;
			else
				sflag++;
			break;
		case 'm':
			margs = getnumbers (optarg, &mnumber);
			if (margs == (int *) 0 || mflag > 0)
				errflag++;
			else
				mflag++;
			break;
		case 'f':
			fflag++;
			sl_buf = file_to_buf (optarg);
			if (sl_buf == (char *) 0) {
				fprintf (stderr,
				  MSGSTR(IPCPOLICY_1, "%s: contents of %s unusable\n"),
				  command_name, optarg);
				exit (1);
			}
			break ;
		default:
			errflag++;
			break;
		}

	if (errflag || argc > optind + 1 ||
	    (qflag == 0 && sflag == 0 && mflag == 0)) {
		fprintf (stderr, MSGSTR(IPCPOLICY_2,
			"Usage: %s [ -q msg_queues ] [ -s sems ] [ -m shm_segs ] [ -f file | attribute ]\n"),
		  command_name);
		exit (1);
	}

	/* if one more argument or fflag is set, want to change the levels
	 * else, just want to list them
	 */
	
	if (fflag) {
		ir = convert_er (sl_buf, &size);
		setting = 1;
	}
	else if (argc == optind + 1) {
		ir = convert_er (argv[optind], &size);
		setting = 1;
	} else {
		setting = 0;
	}

	if (qnumber > 0) {
		if (!setting)
			printf (MSGSTR(IPCPOLICY_3, "Message queues:\n"));
		for (i = 0; i < qnumber; i++) {
			ret = msgctl(qargs[i], IPC_STAT, &msg_stat) ;
			if ((ret == -1) && (errno == EINVAL)) {
				printf(MSGSTR(IPCPOLICY_4, "Message queue %4d does not exist\n"),
				       qargs[i]) ;
				continue ;
			}
#ifdef SEC_ACL_POSIX
                        user_id = msg_stat.msg_perm.uid;
                        group_id = msg_stat.msg_perm.gid;
#endif


			if (setting) {
				ret = msg_setattr (qargs[i], ir, size);
				if (ret != 0) {
					sprintf (buf,
					  MSGSTR(IPCPOLICY_5, "Message queue %4d"), qargs[i]);
					psecerror (buf);
					errflag++;
				}

			} else {
				ret = msg_getattr (qargs[i], &ir);
				errflag +=
				  put_msg (ret, qargs[i], MSGSTR(IPCPOLICY_6, "Message queue"), ir);
			}
		}
	}

	if (snumber > 0) {
		if (!setting)
			printf (MSGSTR(IPCPOLICY_7, "Semaphores:\n"));
		for (i = 0; i < snumber; i++) {
			ret = semctl(sargs[i], 0, IPC_STAT, &sem_stat) ;
			if ((ret == -1) && (errno == EINVAL)) {
				printf(MSGSTR(IPCPOLICY_8, "Semaphore %4d does not exist\n"),
				       sargs[i]) ;
				continue ;
			}
#ifdef SEC_ACL_POSIX
                        user_id = sem_stat.sem_perm.uid;
                        group_id = sem_stat.sem_perm.gid;
#endif


			if (setting) {
				ret = sem_setattr (sargs[i], ir, size);
				if (ret != 0) {
					sprintf (buf,
					  MSGSTR(IPCPOLICY_9, "Semaphore %4d"), sargs[i]);
					psecerror (buf);
					errflag++;
				}

			} else {
				ret = sem_getattr (sargs[i], &ir);
				errflag +=
				  put_msg (ret, sargs[i], MSGSTR(IPCPOLICY_10, "Semaphore"), ir);
			}
		}
	}

	if (mnumber > 0) {
		if (!setting)
			printf (MSGSTR(IPCPOLICY_11, "Shared memory:\n"));
		for (i = 0; i < mnumber; i++) {
			ret = shmctl(margs[i], IPC_STAT, &shm_stat) ;
			if ((ret == -1) && (errno == EINVAL)) {
				printf(MSGSTR(IPCPOLICY_12, "Shared Memory %4d does not exist\n"),
				       margs[i]) ;
				continue ;
			}
#ifdef SEC_ACL_POSIX
                        user_id = shm_stat.shm_perm.uid;
                        group_id = shm_stat.shm_perm.gid;
#endif


			if (setting) {
				ret = shm_setattr (margs[i], ir, size);
				if (ret != 0) {
					sprintf (buf,
					  MSGSTR(IPCPOLICY_13, "Shared segment %4d"), margs[i]);
					psecerror (buf);
					errflag++;
				}

			} else {
				ret = shm_getattr (margs[i], &ir);
				errflag +=
				  put_msg (ret, margs[i], MSGSTR(IPCPOLICY_14, "Shared segment"), ir);
			}
		}
	}

	exit (errflag);
}


static int
put_msg (ret, id, type, ir)
int ret;
int id;
char *type;
char *ir;
{
	char	buf[32];
	char	errflag = 0;
	char	*er;

	if (ret == -1) {
		if (errno == EINVAL)
#ifdef SEC_CHOTS
			printf ("%4d    %s\n", id, MSGSTR(IPCPOLICY_15, "Null set"));
#else
			printf ("%4d    %s\n", id, WILDCARD);
#endif
		else {
			sprintf (buf, "%s %4d", type, id);
			perror (buf);
			errflag++;
		}
	} else {
		er = convert_ir (ir, ret);
		if (er == (char *) 0) {
			fprintf (stderr, MSGSTR(IPCPOLICY_16, "%s %4d: Cannot convert IR to ER\n"),
				type, id);
			errflag++;
		} else {
#ifdef SEC_ACL_POSIX
		if (acl_or_label == 1) {
                        sprintf (buf,"%s %d", type, id);
                        pacl_printbuf (buf, er, user_id, group_id);
		} else
#endif
			{
                        printf ("%4d    ", id);
                        printbuf (er, 8, "/, ");
			}
		}
	}
	return (errflag);
}

static int *
getnumbers (arg, countp)
char *arg;
int *countp;
{
	int *ret;
	char *numbers = "1234567890";
	int first, last;
	register char *cp;
	int pass;
	int i;
	int count = 0;

	/* on first pass, count up how many.
	 * on second pass, fill up the return array
	 */
	for (pass = 1; pass <= 2; pass++) {
		if (pass == 2) {
			ret = (int *) calloc (count, sizeof (int));
			if (ret == (int *) 0)
				return (ret);
			count = 0;
		}
		for (cp = arg; *cp != '\0'; ) {
			/* look for a number followed by comma or number-number.
			 * during pass 1, just increment count.
			 * during pass 2, fill the ret array
			 */
			if (pass == 1 && !isdigit (*cp))
				return ((int *) 0);
			first = atoi (cp);
			cp += strspn (cp, numbers);
			switch (*cp) {
			case ',':
				cp++;
			case '\0':
				if (pass == 2)
					ret[count] = first;
				count++;
				break;
			case '-':
				cp++;
				if (pass == 1 && !isdigit (*cp))
					return ((int *) 0);
				last = atoi (cp);
				if (pass == 1 && last < first)
					return ((int *) 0);
				if (pass == 1)
					count += last - first + 1;
				else for (i = first; i <= last; i++)
					ret[count++] = i;
				cp += strspn (cp, numbers);
				if (*cp == ',')
					cp++;
				break;
			default:
				return ((int *) 0);
			}
		}
	}
	*countp = count;
	return (ret);
}
#endif /* SEC_ARCH */
