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
static char	*sccsid = "@(#)$RCSfile: ipc_ilabel.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:03:19 $";
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
/* Copyright (c) 1989-90  SecureWare, Inc.
 *   All rights reserved.
 *
 * Support routines for IPC program for sensitivity labels.
 */

#include <sys/secdefines.h>


#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "policy_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_POLICY,n,s)
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif

#if SEC_ILB

#include <sys/security.h>
#include <mandatory.h>
#include <prot.h>
#include <stdio.h>



/*
 * Based on:

 */

static ilb_ir_t *ilb_ir = (ilb_ir_t *) 0;
static privvec_t saveprivs;

extern priv_t *privvec();

static void
init_ilb_ir ()
{
#if SEC_MAC
	mand_init();
#endif
	if (ilb_ir == (ilb_ir_t *) 0) {
		ilb_ir = ilb_alloc_ir();
		if (ilb_ir == (ilb_ir_t *) 0) {
			fprintf (stderr,
			  MSGSTR(IPC_ILABEL_1, "%s: Information labels not configured\n"),
			  command_name);
			exit (1);
		}
	}
}

static void
raise_privileges()
{
	/*
	 * Make sure user is authorized to change information labels.
	 */
	if (!authorized_user("chilevel")) {
		fprintf(stderr, MSGSTR(IPC_ILABEL_2, "%s: need chilevel authorization\n"),
			command_name);
		exit(1);
	}

	/*
	 * Raise the ILB regrading privilege.
	 * Also raise access control override privileges for which
	 * user is authorized.
	 */
	if (forceprivs(privvec(SEC_ALLOWILBACCESS, -1), saveprivs) ||
	    enableprivs(privvec(SEC_OWNER, SEC_ALLOWDACACCESS,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
				SEC_WRITEUPCLEARANCE,
				SEC_WRITEUPSYSHI,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1), saveprivs)) {
		fprintf(stderr, MSGSTR(IPC_ILABEL_3, "%s: insufficient privileges\n"), command_name);
		exit(1);
	}
	disablepriv(SEC_SUSPEND_AUDIT);
}

sem_setattr (semid, ir)
int semid;
char *ir;
{
	ilb_ir_t *ilb_ir = (ilb_ir_t *) ir;
	int ret;

#if SEC_MAC
	mand_init();
#endif
	raise_privileges();

	ret = sem_chilabel (semid, ilb_ir);

	seteffprivs(saveprivs, (priv_t *) 0);
	return ret;
}

sem_getattr (semid, ir)
int semid;
char **ir;
{
	int ret;

	init_ilb_ir();
	ret =  sem_statilabel (semid, ilb_ir);
	*ir = (char *) ilb_ir;
	return (ret);
}

shm_setattr (shmid, ir)
int shmid;
char *ir;
{
	ilb_ir_t *ilb_ir = (ilb_ir_t *) ir;

#if SEC_MAC
	mand_init();
#endif
	raise_privileges();

	ret = shm_chilabel (shmid, ilb_ir);

	seteffprivs(saveprivs, (priv_t *) 0);
	return ret;
}

shm_getattr (shmid, ir)
int shmid;
char **ir;
{
	int ret;

	init_ilb_ir();
	ret =  shm_statilabel (shmid, ilb_ir);
	*ir = (char *) ilb_ir;
	return (ret);
}

msg_setattr (msgid, ir)
int msgid;
char *ir;
{
	ilb_ir_t *ilb_ir = (ilb_ir_t *) ir;
	int ret;

#if SEC_MAC
	mand_init();
#endif
	raise_privileges();

	ret = msg_chilabel (msgid, ilb_ir);

	seteffprivs(saveprivs, (priv_t *) 0);
	return ret;
}

msg_getattr (msgid, ir)
int msgid;
char **ir;
{
	int ret;

	init_ilb_ir();
	ret =  msg_statilabel (msgid, ilb_ir);
	*ir = (char *) ilb_ir;
	return (ret);
}
#endif /* SEC_ILB */
