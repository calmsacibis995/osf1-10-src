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
static char	*sccsid = "@(#)$RCSfile: conv_il.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:03:03 $";
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
/* Copyright (c) 1989-90 SecureWare, Inc.
 *   All rights reserved.
 *
 * conversion support routines for information labels.
 *
 */


/*
 * Based on:

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

#define WILDCARD "WILDCARD"

extern priv_t *privvec();

/*
 * convert an internal representation to a string.
 * returns a NULL pointer if it cannot convert.
 */

char *
convert_ir (s, size)
char *s;
int size;
{
#if SEC_MAC
	mand_init();
#endif
	return ilb_ir_to_er ((ilb_ir_t *) s);
}

/*
 * convert an internal representation of a MAC label to a string.
 * returns a NULL pointer if it cannot convert.
 */

char *
convert_sl_ir (s, size)
char *s;
int size;
{
#if SEC_MAC
	mand_init();
#endif
	return mand_ir_to_er ((mand_ir_t *) s);
}

/*
 * convert an internal representation of a clearance to a string.
 * returns a NULL pointer if it cannot convert.
 */

char *
convert_cl_ir (s, size)
char *s;
int *size;
{
#if SEC_MAC
	mand_init();
#endif
	return clearance_ir_to_er ((mand_ir_t *) s);
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
	ilb_ir_t *ir;

#if SEC_MAC
	mand_init();
#endif
	/* setting the il, look it up in the database */
	if (strcmp (WILDCARD, string) == 0) {
		*size = 1;
		ir = (ilb_ir_t *) 0;
	} else {
		ir = ilb_er_to_ir (string);
		if (ir == (ilb_ir_t *) 0) {
			fprintf (stderr, MSGSTR(CONV_IL_1, "%s: cannot convert \'%s\'\n"),
			  command_name, string);
			exit (1);
		}
	}
	return ((char *) ir);
}

/* convert an external representation of a sensitivity label to an internal
 * one.  return an untyped pointer so the driver code can deal with it
 * for any policy.
 */

char *
convert_sl_er (string,size)
char *string;
int *size;
{
	mand_ir_t *ir;

#if SEC_MAC
	mand_init();
#endif
	if (strcmp (WILDCARD, string) == 0) {
		*size = 1;
		ir = (mand_ir_t *) 0;
	} else {
		ir = mand_er_to_ir (string);
		if (ir == (mand_ir_t *) 0) {
			fprintf (stderr, MSGSTR(CONV_IL_1, "%s: cannot convert \'%s\'\n"),
			  command_name, string);
			exit (1);
		}
	}
	return ((char *) ir);
}

/* convert an external representation of a clearance to an internal one.
 * return an untyped pointer so the driver code can deal with it
 * for any policy.
 */

char *
convert_cl_er (string,size)
char *string;
int *size;
{
	mand_ir_t *ir;

#if SEC_MAC
	mand_init();
#endif
	if (strcmp (WILDCARD, string) == 0) {
		*size = 1;
		ir = (mand_ir_t *) 0;
	} else {
		ir = clearance_er_to_ir (string);
		if (ir == (mand_ir_t *) 0) {
			fprintf (stderr, MSGSTR(CONV_IL_2, "%s: cannot convert clearance \'%s\'\n"),
			  command_name, string);
			exit (1);
		}
	}
	return ((char *) ir);
}

/* change the attribute of a file */

int
change_attr (file, attr, size)
char *file;
char *attr;
int *size;
{
	ilb_ir_t *ir;
	int ret;
	privvec_t saveprivs;

#if SEC_MAC
	mand_init();
#endif
	ir = (ilb_ir_t *) attr;

	if (!authorized_user("chilevel")) {
		fprintf(stderr, MSGSTR(CONV_IL_3, "%s: need chilevel authorization\n"),
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
				-1), (priv_t *) 0)) {
		fprintf(stderr, MSGSTR(CONV_IL_4, "%s: insufficient privileges\n"), command_name);
		exit(1);
	}
	disablepriv(SEC_SUSPEND_AUDIT);

	/* The actual call to regrade the file */

	ret = chilabel (file, ir);

	/* Drop privileges back to what they were before the call */

	seteffprivs(saveprivs, (priv_t *) 0);

	return (ret);
}

#endif
