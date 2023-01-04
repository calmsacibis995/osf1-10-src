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
static char *rcsid = "@(#)$RCSfile: conv_sl.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1992/04/14 17:18:31 $";
#endif
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/* @(#)conv_sl.c	6.2 17:48:00 2/5/91 SecureWare */

/* Copyright (c) 1988-90, SecureWare, Inc.
 *   All rights reserved.
 *
 * conversion support routines for sensitivity labels.
 *
 */

/*
 * Based on:
 *   "@(#)conv_sl.c	10.1.1.1 14:03:18 7/31/90 SecureWare, Inc."
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

#if SEC_MAC

#include <sys/security.h>
#include <mandatory.h>
#include <prot.h>
#include <stdio.h>

#define WILDCARD "WILDCARD"

extern priv_t *privvec();

/*
 * convert an internal representation to a string.
 * returns a NULL pointer if it cannot convert.
 */

char *
convert_ir (s, size)
char *s;
int *size;
{
	mand_init();
	return mand_ir_to_er ((mand_ir_t *) s);
}

/* convert an external representation to an internal one.
 * return an untyped pointer so the driver code can deal with it
 * for any policy.
 */

char *
convert_er (string,size)
char *string;
int *size;
{
	mand_ir_t *ir;

	mand_init();
	/* setting the sl, look it up in the database */
	if (strcmp (WILDCARD, string) == 0) {
		*size = 1;
		ir = (mand_ir_t *) 0;
	} else {
		ir = mand_er_to_ir (string);
		if (ir == (mand_ir_t *) 0) {
			fprintf (stderr, MSGSTR(CONV_SL_1, "%s: cannot convert \'%s\'\n"),
			  command_name, string);
			exit (1);
		}
	}
	return ((char *) ir);
}

/* change the attribute of a file
 * Temporarily add DOWNGRADE, WRITEUPSYSHI, WRITEUPCLEARANCE, and ALLOWMAC
 * to the effective, which if allowed will give the user the
 * ability to change the file level up or down.
 */

int
change_attr (file, attr, size)
char *file;
char *attr;
int *size;
{
	mand_ir_t *ir;
	int ret;
	static int auth_checked = 0;
	static privvec_t newprivs;
	privvec_t saveprivs;

	mand_init();
	ir = (mand_ir_t *) attr;

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
				MSGSTR(CONV_SL_2, "%s: need downgrade or upgrade authorization\n"),
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
				SEC_DOWNGRADE,
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1), (priv_t *) 0)) {
		fprintf(stderr, MSGSTR(CONV_SL_3, "%s: insufficient privileges\n"), command_name);
		exit(1);
	}
	disablepriv(SEC_SUSPEND_AUDIT);

	/* The actual call to regrade the file */

	ret = chslabel (file, ir);

	/* Drop privileges back to what they were before the call */

	seteffprivs(saveprivs, (priv_t *) 0);

	return (ret);
}
#endif
