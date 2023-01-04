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
static char *rcsid = "@(#)$RCSfile: prot_freeall.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/12/11 16:53:27 $";
#endif
#ifndef lint
static char sccsid[] = "@(#)prot_freeall.c	1.2 90/07/23 NFSSRC4.0 1.3 88/02/07 Copyr 1984 Sun Micro";
#endif

	/*
	 * Copyright (c) 1988 by Sun Microsystems, Inc.
	 */

	/*
	 * prot_freeall.c consists of subroutines that implement the
	 * DOS-compatible file sharing services for PC-NFS
	 */

#include <stdio.h>
#include <sys/file.h>
#include "prot_lock.h"

extern int debug;
extern int grace_period;
extern char *xmalloc();
extern void xfree();
extern bool_t obj_cmp();

void *
proc_nlm_freeall(Rqstp, Transp)
	struct svc_req *Rqstp;
	SVCXPRT *Transp;
{
	nlm_notify	req;
/*
 * Allocate space for arguments and decode them
 */

	req.name = NULL;
	if (!svc_getargs(Transp, xdr_nlm_notify, &req)) {
		svcerr_decode(Transp);
		return;
	}

	if (debug) {
		printf("proc_nlm_freeall from %s\n",
			req.name);
	}
	destroy_client_shares(req.name);

	free(req.name);
	svc_sendreply(Transp, xdr_void, NULL);
}

