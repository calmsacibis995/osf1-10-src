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
static char	*sccsid = "@(#)$RCSfile: acucl.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:53:57 $";
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
 *  Copyright (c) 1990 SecureWare, Inc.
 *   All rights reserved
 */



/* Set/display user clearance support routine */

#include <sys/secdefines.h>

#if SEC_MAC /*{*/

#include <stdio.h>
#include <ctype.h>
#include <sys/security.h>
#include <prot.h>
#include <mandatory.h>
#include "userif.h"
#include "valid.h"
#include "logging.h"
#include "UIMain.h"
#include "Accounts.h"

int
do_acsucl(argv)
	char **argv;
{
	struct prpw_if pr;
	int ret;
	mand_ir_t *ir;
	mand_ir_t *user_cl;
	mand_ir_t *dflt_cl;
	char use_default;

	ENTERFUNC("acucl_doit");

	ret = GetUserInfo(gl_user, &pr);

	if (ret != SUCCESS)
		return 1;

	if (pr.prpw.sflg.fg_clearance)
		dflt_cl = &pr.prpw.sfld.fd_clearance;
	else {
		pop_msg("There is no system default clearance.",
		  "Please set one before attempting to use this screen.");
		return 1;
	}

	ir = mand_alloc_ir();
	if (ir == (mand_ir_t *) 0)
		MemoryError();

	if (pr.prpw.uflg.fg_clearance) {
		user_cl = &pr.prpw.ufld.fd_clearance;
		use_default = 0;
	} else {
		user_cl = dflt_cl;
		use_default = 1;
	}

	mand_copy_ir(user_cl, ir);
			
	if (aif_label("SET USER CLEARANCE", ir, NULL, NULL,
			&use_default, dflt_cl) == 0) {

		if (use_default)
			pr.prpw.uflg.fg_clearance = 0;
		else {
			mand_copy_ir(ir, &pr.prpw.ufld.fd_clearance);
			pr.prpw.uflg.fg_clearance = 1;
		}

		ret = WriteUserInfo(&pr);

		if (ret == 1)
			CantUpdateUserMsg();
	}

	mand_free_ir(ir);

	EXITFUNC("acucl_doit");
	return 0;
}

int
do_acducl(argv)
	char **argv;
{
	struct prpw_if pr;
	int ret;
	mand_ir_t *ir;
	mand_ir_t *user_cl;
	mand_ir_t *dflt_cl;
	char use_default;

	ENTERFUNC("acdcl_doit");

	ret = GetUserInfo(gl_user, &pr);

	if (ret != SUCCESS)
		return 1;

	if (!pr.prpw.uflg.fg_clearance) {
		pop_msg("The user does not have a specific clearance.",
		  "The system enforces the system default clearance.");
		return 1;
	}

	ir = mand_alloc_ir();
	if (ir == (mand_ir_t *) 0)
		MemoryError();

	user_cl = &pr.prpw.ufld.fd_clearance;

	mand_copy_ir(user_cl, ir);
			
	aif_label("DISPLAY USER CLEARANCE", ir, NULL, NULL,
			&use_default, NULL);

	mand_free_ir(ir);

	EXITFUNC("acdcl_doit");
	return 0;
}

#endif /* SEC_MAC */
