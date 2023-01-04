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
static char	*sccsid = "@(#)$RCSfile: acdcl.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:52:54 $";
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



/* routines for default clearance and single-user sensitivity level */

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

/* Default clearance */

int
do_acdcl(argv)
char **argv;
{
	int ret;
	mand_ir_t *ir;
	mand_ir_t *sl;
	struct sdef_if sd;

	ENTERFUNC("do_acdcl");

	ret = XGetSystemInfo(&sd);

	if (ret)
		return (ret);

	ir = mand_alloc_ir();
	if (ir == (mand_ir_t *) 0)
		MemoryError();

	/* If system default clearance set use that.
	 * If not then set a system default from syslo (minsl)
	 */

	if (sd.df.prg.fg_clearance)
		sl      = &sd.df.prd.fd_clearance;
	else {
		/* Choose a default of lowest clearance allowed */
#if SEC_ENCODINGS
		sl      = mand_minclrnce;
#else
		sl      = mand_syslo;
#endif
	}
	mand_copy_ir(sl, ir);

	ret = aif_label("SET DEFAULT CLEARANCE", ir,
		NULL, NULL, NULL, NULL);

	if (ret == 0) {
		mand_copy_ir(ir, &sd.df.prd.fd_clearance);
		sd.df.prg.fg_clearance = 1;
		XWriteSystemInfo (&sd);
	}

	mand_free_ir(ir);

	EXITFUNC("do_acdcl");
	return ret;
}

/* default single-user sensitivity label */

int
do_acdssl(argv)
char **argv;
{
	int ret;
	mand_ir_t *ir;
	mand_ir_t *sl;
	struct sdef_if sd;

	ENTERFUNC("do_acdssl");

	ret = XGetSystemInfo(&sd);

	if (ret)
		return (ret);

	ir = mand_alloc_ir();
	if (ir == (mand_ir_t *) 0)
		MemoryError();

	/* If single user SL already set use that.
	 * If not then set a system default from syslo (minsl)
	 */

	if (sd.df.sflg.fg_single_user_sl)
		sl      = sd.df.sfld.fd_single_user_sl;
	else
#if SEC_ENCODINGS
		sl = mand_minsl;
#else
		sl = mand_syslo;
#endif
	mand_copy_ir(sl, ir);

	ret = aif_label("SET SINGLE USER SENSITIVITY LABEL", ir,
		NULL, NULL, NULL, NULL);

	if (ret == 0) {
		mand_copy_ir(ir, sd.df.sfld.fd_single_user_sl);
		sd.df.sflg.fg_single_user_sl = 1;
		XWriteSystemInfo (&sd);
	}

	mand_free_ir(ir);

	EXITFUNC("do_acdssl");
	return ret;
}

#endif /*} SEC_MAC */
