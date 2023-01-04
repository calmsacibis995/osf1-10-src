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
static char	*sccsid = "@(#)$RCSfile: from_sec.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:34:13 $";
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
 * SecureWare, Inc.
 *
 * Copyright (c) 1990, All rights reserved.
 *
 * From(1) mail handling hook routines
 */

#include <sys/secdefines.h>

#if SEC_MAC



#include <sys/types.h>
#include <sys/security.h>
#include <mandatory.h>
#include <prot.h>
#include <errno.h>
#include <stdio.h>

static mand_ir_t	*mailbox_level;

extern int		sec_errno;

/*
 * from_init()-initialization for the hook routines
 */

void
from_init(argc, argv)
	int	argc;
	char	*argv[];
{
	set_auth_parameters(argc, argv);
	initprivs();

	if ((mailbox_level = mand_alloc_ir()) == NULL) {
		fprintf(stderr, "%s: can't initialize for sensitivity labels\n",
			command_name);
		exit(1);
	}
}


/*
 * See if the current process sensitivity level dominates that of the
 * mailbox.  If not, we infer that the user is not logged in at his
 * clearance.
 */

int
from_check_mailbox(file)
	char	*file;
{

	/*
	 * If we are able to retrieve the file's sensitivity label, then
	 * our process level dominates (or equals) the mailbox level, or
	 * we have the MAC override privilege.  Assuming we don't have the
	 * privilege, then if the mailbox is our own and is properly labeled,
	 * that means we are logged in at our clearance.  Conversely, if we
	 * can't get the file's sensitivity label, that means we are logged
	 * in below our clearance and need to be admonished.
	 *
	 * If the mailbox is someone else's, the open in the main program will
	 * fail unless we have the DAC override privilege.
	 */

	if (statslabel(file, mailbox_level) == -1 && errno == EACCES &&
			sec_errno == ESEC_MAC_SDOM)
		return 0;

	return 1;
}

#endif /* SEC_MAC */
