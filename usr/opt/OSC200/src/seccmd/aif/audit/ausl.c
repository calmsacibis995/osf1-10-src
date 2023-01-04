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
static char	*sccsid = "@(#)$RCSfile: ausl.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:55:35 $";
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
 * Copyright(c) 1990 SecureWare, Inc.
 *   All rights reserved
 */



/* Audit sensitivity label screen support */

#include <sys/secdefines.h>

#if SEC_MAC

#include "gl_defs.h"
#include "IfAudit.h"
#include "userif.h"

/* field titles (defined in au_scrns.c) */

extern uchar aucpncl_title[];	/* min SL */
extern uchar aucpxcl_title[];	/* max SL */

/*
 * Audit collection minimum sensitivity level
 */

int
aucpncl(argv)
	char **argv;
{
	AudSL_fillin aubuf, *aufill = &aubuf;
	mand_ir_t	*min_sl;
	mand_ir_t	*max_sl;
	int		ret;

	if (GetAuditSL(aufill))
		return 1;

	min_sl = mand_alloc_ir();

	if (min_sl == (mand_ir_t *) 0)
		MemoryError();

	mand_copy_ir(aufill->min_ir_ptr, min_sl);

	ret = aif_label(aucpncl_title, min_sl,
			&aufill->this_session, &aufill->future_sessions,
			NULL, NULL);

	if (CompareSL(min_sl, aufill->max_ir_ptr)) {
		pop_msg("Audit collection minimum sensitivity level must be",
		  "dominated by audit collection maximum sensitivity level.");
		ret = 1;
	}

	if (ret) {
		mand_free_ir(min_sl);
		return 1;
	}

	mand_copy_ir(min_sl, aufill->min_ir_ptr);
	mand_free_ir(min_sl);

	ret = WriteAuditSL(aufill);
	mand_free_ir(aufill->min_ir_ptr);
	mand_free_ir(aufill->max_ir_ptr);
	return ret;
}

/*
 * Audit collection minimum sensitivity level
 */

int
aucpxcl(argv)
	char **argv;
{
	AudSL_fillin aubuf, *aufill = &aubuf;
	mand_ir_t	*max_sl;
	int		ret;

	if (GetAuditSL(aufill))
		return 1;

	max_sl = mand_alloc_ir();

	if (max_sl == (mand_ir_t *) 0) {
#ifdef NEW_ERRORS
		error message here
#endif
		return 1;
	}

	mand_copy_ir(aufill->max_ir_ptr, max_sl);

	ret = aif_label(aucpncl_title, max_sl,
			&aufill->this_session, &aufill->future_sessions,
			NULL, NULL);

	if (CompareSL(aufill->min_ir_ptr, max_sl)) {
		pop_msg("Audit collection minimum sensitivity level must be",
		  "dominated by audit collection maximum sensitivity level.");
		ret = 1;
	}

	if (ret) {
		mand_free_ir(max_sl);
		return 1;
	}

	mand_copy_ir(max_sl, aufill->max_ir_ptr);
	mand_free_ir(max_sl);

	ret = WriteAuditSL(aufill);
	mand_free_ir(aufill->min_ir_ptr);
	mand_free_ir(aufill->max_ir_ptr);
	return ret;
}

/* Compare min and max sensitivity levels */

CompareSL(min_sl, max_sl)
	mand_ir_t *min_sl;
	mand_ir_t *max_sl;
{
	privvec_t s;
	int ret;

	if (min_sl == (mand_ir_t *) 0 || max_sl == (mand_ir_t *) 0)
		return 0;

	/*
	 * Determine the relationship after raising appropriate privilege
	 */

	forceprivs(privvec(SEC_ALLOWMACACCESS,
#if SEC_ILB
			   SEC_ILNOFLOAT,
#endif
			   -1), s);
	
	ret = mand_ir_relationship(min_sl, max_sl);
	seteffprivs(s, NULL);

	if (ret == MAND_ODOM || ret == MAND_EQUAL)
		return 0;
	else
		return 1;
}

#endif /* SEC_MAC */
