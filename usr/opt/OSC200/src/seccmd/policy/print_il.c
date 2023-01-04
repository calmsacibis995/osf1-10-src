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
static char	*sccsid = "@(#)$RCSfile: print_il.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:03:39 $";
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
/* Copyright (c) 1989-90  SecureWare, Inc.
 *   All rights reserved.
 *
 * output routine that lists the information label for each file
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


#if SEC_ILB

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
	static	ilb_ir_t *ilb_ir = (ilb_ir_t *) 0;
	static	ilb_ir_t *prev_ilb_ir = (ilb_ir_t *) 0;
	static	char	*ersavebuf;
	int	ret;

	/* The most recent label conversion is cached. */

#if SEC_MAC
	mand_init();
#endif
	if (ilb_ir == (ilb_ir_t *) 0) {
		ilb_ir = ilb_alloc_ir ();
		if (ilb_ir == (ilb_ir_t *) 0) {
			fprintf (stderr,
			  MSGSTR(PRINT_IL_1, "Memory allocation error on label buffer.\n"));
			exit (1);
		}
	}
	ret = statilabel (path, ilb_ir);
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
		if ((prev_ilb_ir != (ilb_ir_t *) 0) &&
		    (memcmp(ilb_ir, prev_ilb_ir, ilb_bytes()) == 0))
			erbuf = ersavebuf;
		else  {
			erbuf = convert_ir (ilb_ir, 0);
			if (erbuf == NULL)
				erbuf = MSGSTR(PRINT_IL_2, "cannot convert information label");
			else  {
				ersavebuf = erbuf;
				if (prev_ilb_ir == (ilb_ir_t *) 0)
					prev_ilb_ir = ilb_alloc_ir();
				if (prev_ilb_ir != (ilb_ir_t *) 0)
					ilb_copy_ir(ilb_ir, prev_ilb_ir);
			}
		}
		break;
	}
	printf("%-*s", col, name);
	printbuf (erbuf, col, "/, ");
	return;
}
#endif
