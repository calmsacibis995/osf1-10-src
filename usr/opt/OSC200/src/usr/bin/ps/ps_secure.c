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
static char	*sccsid = "@(#)$RCSfile: ps_secure.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:46:12 $";
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
/*
 * Copyright (c) 1988-90 SecureWare, Inc.  All rights reserved.
 */


/*
 * Rewritten from:

 */

/*
 * This file is part of a library to make commands more secure.
 * This file contains those routines that are added to the
 * ps command.
 * If the system supports a ps_data file, hooks are required to create
 * it with the proper attributes.
 * If the system supports mandatory access control, filtering must be
 * done on target processes to remove those the process does not
 * dominate if the user is not authorized for allowmacaccess.
 */

#include <sys/secdefines.h>

#if SEC_BASE
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>
#if SEC_MAC
#include <sys/secpolicy.h>
#include <mandatory.h>
#endif

extern priv_t *privvec();

#ifndef _OSF_SOURCE
/*
 * Open the ps data file securely and return the file descriptor of the
 * new file, set for writing.
 */
int
ps_open_file_securely(file)
	char *file;
{
	int cfs_status;
	privvec_t saveprivs;

	/* Save the effective privileges already in effect */

	if (forceprivs(privvec(SEC_ALLOWDACACCESS, SEC_OWNER, SEC_CHOWN,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
				SEC_ILNOFLOAT,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1), saveprivs)) {
		fprintf(stderr, "%s: insufficient privileges\n", command_name);
		exit(1);
	}
	unlink(file);
	cfs_status = create_file_securely(file, AUTH_VERBOSE,
			     "store static operating system parameters");
	if (cfs_status == CFS_GOOD_RETURN) {
		enter_quiet_zone();
		cfs_status = open(file, O_WRONLY);
	} else
		cfs_status = -1;
#if SEC_ILB
	/*
	 * Leave ilnofloat raised to avoid potential problems when
	 * the ps_data file is written.
	 */
	if (cfs_status >= 0)
		ADDBIT(saveprivs, SEC_ILNOFLOAT);
#endif
	seteffprivs(saveprivs, (priv_t *) 0);
	return(cfs_status);
}


/*
 * Restore the signals that were set to SIG_IGN when the data file
 * was opened so that the file data would not be interrupted.
 */
void
ps_cleanup_new_file()
{
	exit_quiet_zone();
}
#endif

#if SEC_MAC /*{*/

/*
 * Return TRUE if process dominates the target process.
 * Attempt a getlabel(2) call on the target process's sensitivity label.
 */

ps_proc_dominate(pid)
	int	pid;
{
	static attr_t	attr;
	static mand_ir_t *mand_ir = (mand_ir_t *) 0;
	static int has_auth;
	static int first_time = 1;
	obj_t	obj;

	if (first_time) {
		mand_ir = mand_alloc_ir();
		if (mand_ir == (mand_ir_t *) 0) {
			fprintf(stderr, "%s: MAC initialization failed\n",
				command_name);
			exit(1);
		}
		attr.ir_length = mand_bytes();
		attr.ir = (char *) mand_ir;
		has_auth = authorized_user("macquery");
		first_time = 0;
	}
	if (has_auth)
		return 1;

	obj.o_pid = pid;

	/*
	 * Attempt to retrieve the target process's label.
	 * This will only succeed if we dominate the target process.
	 */
	return getlabel(mand_config.policy, MAND_SUBJ_SL_TAG, &attr,
			OT_PROCESS, &obj) == 0;
}
#endif /*} SEC_MAC */
#endif SEC_BASE
