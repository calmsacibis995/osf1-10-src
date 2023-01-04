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
static char	*sccsid = "@(#)$RCSfile: print_acl.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:03:36 $";
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
/* Copyright (c) 1988-90  SecureWare, Inc.
 *   All rights reserved.
 *
 * output routine that lists the access control list for each file
 * encountered.
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


#if SEC_ACL_SWARE

#include <sys/types.h>
#include <acl.h>
#include <stdio.h>
#include <sys/errno.h>
#include <sys/security.h>
#include <prot.h>

extern int errno;
extern void printbuf();
extern char *convert_ir(), *malloc(), *realloc();

#define INIT_ACLSIZE 10

/* print the external representation of a security attribute for a file
 * path is the pathname of the file and name is the name to be printed
 */

void
print_er (path, name, col)
char	*path, *name;
int	col;
{
	char	*erbuf;
	static	acle_t *acl_ir = (acle_t *) 0;
	static	int acl_size = 0;
	static	acle_t *prev_acl_ir = (acle_t *) 0;
	static	int prev_acl_size = 0;
	static	char	*ersavebuf;
	int	ret;

	while ((ret = statacl (path, acl_ir, acl_size)) > acl_size) {
		if (ret < INIT_ACLSIZE)
			ret = INIT_ACLSIZE;
		if (acl_ir == (acle_t *) 0)
			acl_ir = (acle_t *)malloc(ret * sizeof(acle_t));
		else
			acl_ir = (acle_t *)realloc((char *)acl_ir,
					ret * sizeof(acle_t));
		if (acl_ir == (acle_t *) 0) {
			fprintf (stderr,
				MSGSTR(PRINT_ACL_1, "Memory allocation error on acl buffer.\n"));
			exit (1);
		}
		acl_size = ret;
	}

	switch (ret) {
	case	0:
		erbuf = "NULL";
		break;
	case	-1:
		if (errno == EINVAL)
			erbuf = "WILDCARD";
		else {
			perror (path);
			return;
		}
		break;
	default:
		/*
		 * If we have the same acl as that which was previously
		 * converted, use the saved er string rather than recomputing
		 * the acl again.
		 */
		if ((prev_acl_ir != (acle_t *) 0) &&
		    (prev_acl_size == acl_size) &&
		    (memcmp(acl_ir, prev_acl_ir, prev_acl_size) == 0))
			erbuf = ersavebuf;
		else  {
			erbuf = convert_ir (acl_ir, ret);
			if (erbuf == NULL)
				erbuf = MSGSTR(PRINT_ACL_2, "cannot convert ACL");
			else  {
				ersavebuf = erbuf;

				if (prev_acl_ir == (acle_t *) 0)
					prev_acl_ir = (acle_t *) malloc(ret);
				else
					prev_acl_ir =
					   (acle_t *) realloc(prev_acl_ir, ret);

				if (prev_acl_ir != (acle_t *) 0)
					(void) memcpy(prev_acl_ir, acl_ir, ret);
				prev_acl_size = ret;
			}
		}
		break;
	}
	printf("%-*s", col, name);
	printbuf(erbuf, col, "> ");
	return;
}
#endif
