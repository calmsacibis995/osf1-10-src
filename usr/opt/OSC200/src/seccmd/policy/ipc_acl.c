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
/* Copyright (c) 1988-90  SecureWare, Inc.
 *   All rights reserved.
 *
 * Support routines for IPC program for access control lists.
 */

/* #ident "@(#)ipc_acl.c	3.2 14:41:16 6/15/90 SecureWare"*/

/*
 * Based on:
 *   "@(#)ipc_acl.c	2.1 11:52:08 1/25/89 SecureWare, Inc."
 */

#include <sys/secdefines.h>
#include <locale.h>
#include "policy_msg.h"

#include <sys/errno.h>

nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_POLICY,n,s)


#include <sys/security.h>
#include <acl.h>
#include <prot.h>
#include <stdio.h>

static acle_t *acl_ir = (acle_t *) 0;
static int acl_size = 0;
static privvec_t saveprivs;

extern char *calloc();
extern priv_t *privvec();
extern int errno;

#define INIT_ACLSIZE 10
#define WILDCARD "WILDCARD"
extern char *convert_ir();

#ifdef SEC_ACL_POSIX
extern int user_id, group_id;
extern int acl_or_label;
#endif

static void
init_acl_ir ()
{
	if (acl_size == 0) {
		acl_ir = (acle_t *) calloc (INIT_ACLSIZE,
					    sizeof (acle_t));
		if (acl_ir == (acle_t *) 0) {
			fprintf (stderr,
			 MSGSTR(IPC_ACL_1, "%s: memory allocation error on acl buffer.\n"),
			 command_name);
			exit (1);
		}
		acl_size = INIT_ACLSIZE;
	}
}

static void
larger_acl_ir (size)
int size;
{
	acl_ir = (acle_t *) realloc (acl_ir, sizeof (acle_t) * size);
	if (acl_ir == (acle_t *) 0) {
		fprintf (stderr,
		  MSGSTR(IPC_ACL_1, "%s: memory allocation error on acl buffer.\n"),
		  command_name);
		exit (1);
	}
	acl_size = size;
}

static void
raise_privileges()
{

	/*
	 * Raise access control override privileges for which
	 * the user is authorized.
	 */
	if (enableprivs(privvec(SEC_OWNER, SEC_ALLOWDACACCESS,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
				SEC_WRITEUPCLEARANCE,
				SEC_WRITEUPSYSHI,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1), saveprivs)) {
		fprintf(stderr, MSGSTR(IPC_ACL_2, "%s: insufficient privileges\n"), command_name);
		exit(1);
	}
	disablepriv(SEC_SUSPEND_AUDIT);
}

sem_setattr (semid, ir, size)
int semid;
char *ir;
int size;
{
	acle_t *acl = (acle_t *) ir;
	int ret;

	raise_privileges();

	ret = sem_chacl (semid, acl, size);

	seteffprivs(saveprivs, (priv_t *) 0);
	return ret;
}

sem_getattr (semid, ir, size)
int semid;
char **ir;
int size;
{
	int ret;

	init_acl_ir();

	do {
		ret = sem_statacl (semid, acl_ir, acl_size);
		if (ret > 0 && ret > acl_size)
			larger_acl_ir (ret);
	} while (ret > 0 && ret > acl_size);
	*ir = (char *) acl_ir;
	return (ret);
}

shm_setattr (shmid, ir, size)
int shmid;
char *ir;
int size;
{
	acle_t *acl = (acle_t *) ir;
	int ret;

	raise_privileges();

	ret = shm_chacl (shmid, acl, size);

	seteffprivs(saveprivs, (priv_t *) 0);
	return ret;
}

shm_getattr (shmid, ir, size)
int shmid;
char **ir;
int size;
{
	int ret;

	init_acl_ir();

	do {
		ret = shm_statacl (shmid, acl_ir, acl_size);
		if (ret > 0 && ret > acl_size)
			larger_acl_ir (ret);
	} while (ret > 0 && ret > acl_size);
	*ir = (char *) acl_ir;
	return (ret);
}

msg_setattr (msgid, ir, size)
int msgid;
char *ir;
int size;
{
	acle_t *acl = (acle_t *) ir;
	int ret;

	ret = msg_chacl (msgid, acl, size);

	seteffprivs(saveprivs, (priv_t *) 0);
	return ret;
}

msg_getattr (msgid, ir, size)
int msgid;
char **ir;
int size;
{
	int ret;

	init_acl_ir();

	do {
		ret = msg_statacl (msgid, acl_ir, acl_size);
		if (ret > 0 && ret > acl_size)
			larger_acl_ir (ret);
	} while (ret > 0 && ret > acl_size);
	*ir = (char *) acl_ir;
	return (ret);
}

int
put_msg (ret, id, type, ir)
int ret;
int id;
char *type;
char *ir;
{
	char	buf[32];
	char	errflag = 0;
	char	*er;

#ifdef SEC_ACL_POSIX
	acl_or_label = 1;
#endif

	if (ret == -1) {
		if (errno == EINVAL)
			printf ("%4d    %s\n", id, WILDCARD);
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
#ifdef SEC_ACL_SWARE
                        /* the first part of the line takes up 8 columns */
                        printf ("%4d    ", id);
                        printbuf (er, 8, "/, ");
#endif

#ifdef SEC_ACL_POSIX
                        sprintf (buf,"%s %d", type, id);
                        pacl_printbuf (buf, er, user_id, group_id);
#endif

		}
	}
	return (errflag);
}
