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
static char	*sccsid = "@(#)$RCSfile: auditlib.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/04/01 20:21:56 $";
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
	Audit Reduction/Selection Interface Routines
	Copyright (c) 1988, SecureWare, Inc. All Rights Reserved.

*** OBSOLETE ***

	This module contains a collection of routines that provide
	an interface for third-party products to read audit trails
	and apply audit selection criteria against those records.

	audit_open(session)-open an audit session
	audit_close()-close an audit session
	audit_read_next()-read the next sequential record
	audit_set_select(select)-set audit selection criteria
	audit_select(record)-determine if record selected
*/




#include <sys/types.h>

#include <sys/security.h>
#include <sys/audit.h>	/* this must be before grp.h to avoid cc warnings */

#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>

#include "libsecurity.h"

audit_open(session)
int session;
{
	fprintf(stderr, "audit_open: obsolete.\n");
	   return(-1);
}

/*
	audit_close()-close the current audit session, releasing the
	malloc buffers and resetting the file descriptors such that
	another session could be opened.
*/

audit_close()
{
	fprintf(stderr, "audit_close: obsolete.\n");
	   return(-1);
}

/*
	audit_read()-read the next sequential record from the audit
	trail.
*/

struct audit_header *
audit_read()
{
	fprintf(stderr, "audit_read: obsolete.\n");
		return((struct audit_header *) 0);
}

