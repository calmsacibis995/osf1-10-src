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
static char	*sccsid = "@(#)$RCSfile: print_sl.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:03:42 $";
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
 * output routine that lists the sensitivity label for each file
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


#if SEC_MAC

#include <sys/types.h>
#include <sys/security.h>
#include <mandatory.h>
#include <stdio.h>
#include <errno.h>

extern void printbuf();
extern char *convert_ir();
extern char *sys_errlist[];

/* print the external representation for a file */

void
print_er (path, name, col)
char	*path, *name;
int	col;
{
	char	*erbuf;
	static	mand_ir_t *mand_ir = (mand_ir_t *) 0;
	static	mand_ir_t *prev_mand_ir = (mand_ir_t *) 0;
	static	char	*ersavebuf;
	int	ret;

	/* The most recent label conversion is cached. */

	mand_init();
	if (mand_ir == (mand_ir_t *) 0) {
		mand_ir = mand_alloc_ir ();
		if (mand_ir == (mand_ir_t *) 0) {
			fprintf (stderr,
			  MSGSTR(PRINT_SL_1, "Memory allocation error on label buffer.\n"));
			exit (1);
		}
	}
	ret = statslabel (path, mand_ir);
	switch (ret) {
	case	-1:
		if (errno == EINVAL)
			erbuf = "WILDCARD";
		else {
			perror(path);
			return;
		}
		break;
	default:
		/*
		 * If we have the same level as that which was previously
		 * converted, use the saved er string rather than recomputing
		 * the level again.
		 */
		if ((prev_mand_ir != (mand_ir_t *) 0) &&
		    (memcmp(mand_ir, prev_mand_ir, mand_bytes()) == 0))
			erbuf = ersavebuf;
		else  {
			erbuf = convert_ir (mand_ir, 0);
			if (erbuf == NULL)
				erbuf = MSGSTR(PRINT_SL_2, "cannot convert sensitivity label");
			else  {
				ersavebuf = erbuf;
				if (prev_mand_ir == (mand_ir_t *) 0)
					prev_mand_ir = mand_alloc_ir();
				if (prev_mand_ir != (mand_ir_t *) 0)
					mand_copy_ir(mand_ir, prev_mand_ir);
			}
		}
		break;
	}
	printf("%-*s", col, name);
	printbuf (erbuf, col, "/, ");
	return;
}
#endif
