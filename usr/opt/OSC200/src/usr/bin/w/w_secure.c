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
static char	*sccsid = "@(#)$RCSfile: w_secure.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:09:06 $";
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

#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * Rewritten from:

 */

/*
 * This file contains those routines that are added to the w command.
 * If the system supports mandatory access control, filtering must be
 * done on target processes to remove those the process does not
 * dominate if the user is not authorized for allowmacaccess.
 *
 * w_secure.c	3.4 15:08:13 6/20/90 SecureWare
 */

#include <sys/secdefines.h>

#if SEC_MAC /*{*/

#include <stdio.h>

#include <sys/security.h>
#include <sys/secpolicy.h>
#include <sys/audit.h>
#include <mandatory.h>
#include <prot.h>

#ifdef KJI
#include <NLchar.h>
#endif

#include <nl_types.h>
#include "w_msg.h"
nl_catd	catd;
#define MSGSTR_SEC(num,str)	catgets(catd,MS_W_SEC,num,str)

/*
 * Return TRUE if process dominates the target process.
 * Attempt a getlabel(2) call on the target process's sensitivity label.
 */

w_proc_dominate(pid)
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
			fprintf(stderr, MSGSTR_SEC(MACFAILED,
				"%s: MAC initialization failed\n"),
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
