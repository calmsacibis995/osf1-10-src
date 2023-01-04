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
static char *rcsid = "@(#)$RCSfile: conv_acl.c,v $ $Revision: 4.2.10.2 $ (DEC) $Date: 1993/04/01 20:15:43 $";
#endif
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/* @(#)conv_acl.c	6.3 17:43:18 2/5/91 SecureWare */
/* Copyright (c) 1988-90, SecureWare, Inc.
 *   All rights reserved.
 *
 * conversion support routines for access control lists.
 *
 */

/*
 * Based on:
 *   "@(#)conv_acl.c	2.5 12:52:29 5/20/89 SecureWare, Inc."
 */

#include <sys/secdefines.h>




#include <sys/security.h>
#include <acl.h>
#include <prot.h>
#include <stdio.h>

#include "policy_msg.h"

nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_POLICY,n,s)

#define NULL_VALUE "NULL"
#define WILDCARD "WILDCARD"

extern priv_t *privvec();

/*
 * convert an internal representation to a string
 * return NULL_VALUE if ACL is empty
 */


char *
convert_ir (s, size)
	char	*s;
	int	size;
{
	char *ret;

	ret = acl_ir_to_er((acle_t *)s, size);
	return ((strlen(ret) == 0 && size == 0) ? NULL_VALUE : ret);
}

/* convert a string to an internal representation */

char *
convert_er(string, size)
	char	*string;
	int	*size;
{
	acle_t	*ir;

	if (strcmp (string, WILDCARD) == 0) {
		ir = ACL_DELETE;
		*size = -1;
	} else {
#ifdef SEC_ACL_POSIX
                {
                        char    *ptr = string;
                        /*
                         * user::rwx,group::rwx,mask::rwx
                         * Replace the ',' with '\n' and
                         * cathe library routine.
                         */
                        while (ptr && *ptr) {
                                if (*ptr == ',')
                                        *ptr = '\n';
                                ptr++;
                        }
                }
#endif

		ir = acl_er_to_ir(string, size);
		if (ir == (acle_t *) 0) {
			fprintf (stderr, MSGSTR(CONV_ACL_1, "%s: cannot convert \'%s\'\n"),
			  command_name, string);
			exit (1);
		}
	}
	return (char *) ir;
}

/* change the acl of a file */

change_attr(filename, attr, size)
	char	*filename;
	char	*attr;
	int	size;
{
	acle_t	*acl;
	int ret;
	privvec_t saveprivs;

	acl = (acle_t *)attr;

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
		fprintf(stderr, MSGSTR(CONV_ACL_2, "%s: insufficient privileges\n"), command_name);
		exit(1);
	}
	disablepriv(SEC_SUSPEND_AUDIT);

	/* change the acl on the file */

	ret = chacl(filename, acl, size);

	/* restore previous effective privilege set */

	seteffprivs(saveprivs, (priv_t *) 0);

	return(ret);
}

