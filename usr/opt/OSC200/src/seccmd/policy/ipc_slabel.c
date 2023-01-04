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
 * Support routines for IPC program for sensitivity labels.
 */

/* #ident "@(#)ipc_slabel.c	5.2 09:50:36 8/30/90 SecureWare" */

/*
 * Based on:
 *   "@(#)ipc_slabel.c	10.1.1.2 15:21:40 8/8/90"
 */

#include <sys/secdefines.h>
#include <locale.h>
#include "policy_msg.h"
#include <sys/errno.h>


nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_POLICY,n,s)

#ifdef SEC_ACL_POSIX
extern int acl_or_label;
#endif

#ifdef SEC_MAC /*{*/

#include <sys/security.h>
#include <mandatory.h>
#include <prot.h>
#include <stdio.h>

static mand_ir_t *mand_ir = (mand_ir_t *) 0;
static privvec_t saveprivs;

extern priv_t *privvec();
extern int errno;

#define WILDCARD "WILDCARD"
extern char *convert_ir();

static void
init_mand_ir ()
{
	mand_init();
	if (mand_ir == (mand_ir_t *) 0) {
		mand_ir = mand_alloc_ir();
		if (mand_ir == (mand_ir_t *) 0) {
			fprintf (stderr,
			  MSGSTR(IPC_SLABEL_1, "%s: Mandatory access control not configured\n"),
			  command_name);
			exit (1);
		}
	}
}

static void
raise_privileges()
{
	static int auth_checked = 0;
	static privvec_t newprivs;

	/*
	 * Compute the set of regrading privileges for which the
	 * user is authorized.  Do this only once to avoid repeated
	 * auditing of authorization use.
	 */
	if (!auth_checked) {
		if (authorized_user("downgrade")) {
			ADDBIT(newprivs, SEC_DOWNGRADE);
			auth_checked = 1;
		}
		if (authorized_user("upgrade")) {
			auth_checked = 1;
			if (authorized_user("syshi"))
				ADDBIT(newprivs, SEC_WRITEUPSYSHI);
			else
				ADDBIT(newprivs, SEC_WRITEUPCLEARANCE);
		}
		if (!auth_checked) {
			fprintf(stderr,
				MSGSTR(IPC_SLABEL_2, "%s: need downgrade or upgrade authorization\n"),
				command_name);
			exit(1);
		}
	}

	/*
	 * Raise the regrading privileges.
	 * Also raise any access control override privileges for which
	 * user is authorized.
	 */
	if (forceprivs(newprivs, saveprivs) ||
	    enableprivs(privvec(SEC_OWNER, SEC_ALLOWDACACCESS,
				SEC_ALLOWMACACCESS,
				SEC_WRITEUPCLEARANCE,
				SEC_WRITEUPSYSHI,
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1), (priv_t *) 0)) {
		fprintf(stderr, MSGSTR(IPC_SLABEL_3, "%s: insufficient privileges\n"), command_name);
		exit(1);
	}
	disablepriv(SEC_SUSPEND_AUDIT);
}

sem_setattr (semid, ir)
int semid;
char *ir;
{
	mand_ir_t *mand_ir = (mand_ir_t *) ir;
	int ret;

	mand_init();
	raise_privileges();

	ret = sem_chslabel (semid, mand_ir);

	seteffprivs(saveprivs, (priv_t *) 0);
	return ret;
}

sem_getattr (semid, ir)
int semid;
char **ir;
{
	int ret;

	init_mand_ir();
	ret =  sem_statslabel (semid, mand_ir);
	*ir = (char *) mand_ir;
	return (ret);
}

shm_setattr (shmid, ir)
int shmid;
char *ir;
{
	mand_ir_t *mand_ir = (mand_ir_t *) ir;
	int ret;

	mand_init();
	raise_privileges();

	ret = shm_chslabel (shmid, mand_ir);

	seteffprivs(saveprivs, (priv_t *) 0);
	return ret;
}

shm_getattr (shmid, ir)
int shmid;
char **ir;
{
	int ret;

	init_mand_ir();
	ret =  shm_statslabel (shmid, mand_ir);
	*ir = (char *) mand_ir;
	return (ret);
}

msg_setattr (msgid, ir)
int msgid;
char *ir;
{
	mand_ir_t *mand_ir = (mand_ir_t *) ir;
	int ret;

	mand_init();
	raise_privileges();

	ret = msg_chslabel (msgid, mand_ir);

	seteffprivs(saveprivs, (priv_t *) 0);
	return ret;
}

msg_getattr (msgid, ir)
int msgid;
char **ir;
{
	int ret;

	init_mand_ir();
	ret =  msg_statslabel (msgid, mand_ir);
	*ir = (char *) mand_ir;
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
	acl_or_label = 0;
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
                        /* the first part of the line takes up 8 columns */
                        printf ("%4d    ", id);
                        printbuf (er, 8, "/, ");
		}
	}
	return (errflag);
}
#endif /*} SEC_MAC */
