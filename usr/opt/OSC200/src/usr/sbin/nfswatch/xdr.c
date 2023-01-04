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
static char *rcsid = "@(#)$RCSfile: xdr.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/12/21 23:36:24 $";
#endif
/* Based on:
 * /home/harbor/davy/system/nfswatch/RCS/xdr.c,v 4.0 1993/03/01 19:59:00 davy Exp $";
 */

#include "os.h"

/*
 * xdr.c - XDR routines for decoding NFS packets.
 *
 * David A. Curry				Jeffrey C. Mogul
 * Purdue University				Digital Equipment Corporation
 * Engineering Computer Network			Western Research Laboratory
 * 1285 Electrical Engineering Building		250 University Avenue
 * West Lafayette, IN 47907-1285		Palo Alto, CA 94301
 * davy@ecn.purdue.edu				mogul@decwrl.dec.com
 *
 * log: xdr.c,v
 * Revision 4.0  1993/03/01  19:59:00  davy
 * NFSWATCH Version 4.0.
 *
 * Revision 3.4  1993/01/16  19:08:59  davy
 * Corrected Jeff's address.
 *
 * Revision 3.3  1993/01/15  19:33:39  davy
 * Miscellaneous cleanups.
 *
 * Revision 3.2  1993/01/15  15:43:36  davy
 * Assorted changes for porting to Solaris 2.x/SVR4.
 *
 * Revision 3.1  1993/01/13  20:18:17  davy
 * Put in OS-specific define scheme, and merged in Tim Hudson's code for
 * SGI systems (as yet untested).
 *
 * Revision 3.0  1991/01/23  08:23:39  davy
 * NFSWATCH Version 3.0.
 *
 * Revision 1.2  90/08/17  15:47:53  davy
 * NFSWATCH Version 2.0.
 * 
 * Revision 1.1  88/11/29  11:21:08  davy
 * NFSWATCH Release 1.0
 * 
 */
#include <sys/param.h>
#include <netinet/in.h>
#ifdef SVR4
#include <sys/tiuser.h>
#endif
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <rpc/auth.h>
#include <rpc/clnt.h>
#include <rpc/rpc_msg.h>
#include <rpc/svc.h>
#include <errno.h>
#include <stdio.h>

#define NFSSERVER	1

#ifdef sun
#include <sys/vfs.h>
#include <nfs/nfs.h>
#endif
#ifdef ultrix
#include <sys/types.h>
#include <sys/time.h>
#include <nfs/nfs.h>
#endif
#ifdef sgi
#include <sys/time.h>
#include "sgi.map.h"
#endif
#ifdef __alpha
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mount.h>
#include <nfs/nfs.h>
#endif

#include "nfswatch.h"

/*
 * These definitions are here in lieu of widespread availability of
 * the NFS V3 library, which should provide them.  We want to make
 * sure that the right number of bits is being referenced; on some
 * 64-bit systems, xdr_long can't be called safely with a pointer
 * to a 32-bit value embedded in a struct at a 32-bit alignment.
 */
#if	defined(pdp11)
/* other 16-bit machines? */
#define	xdr_int32	xdr_long
#define	xdr_u_int32	xdr_u_long
#else
#define	xdr_int32	xdr_int
#define	xdr_u_int32	xdr_u_int
#endif

bool_t
xdr_creatargs(xdrs, argp)
register struct nfscreatargs *argp;
register XDR *xdrs;
{
	if (xdr_diropargs(xdrs, &argp->ca_da) &&
	    xdr_sattr(xdrs, &argp->ca_sa))
		return(TRUE);

	return(FALSE);
}

bool_t
xdr_diropargs(xdrs, argp)
register struct nfsdiropargs *argp;
register XDR *xdrs;
{
	if (xdr_fhandle(xdrs, &argp->da_fhandle) &&
	    xdr_string(xdrs, &argp->da_name, NFS_MAXNAMLEN)) {
		free(argp->da_name);
		return(TRUE);
	}

	return(FALSE);
}

bool_t
xdr_fhandle(xdrs, argp)
fhandle_t *argp;
register XDR *xdrs;
{
	if (xdr_opaque(xdrs, (caddr_t) argp, NFS_FHSIZE))
		return(TRUE);

	return(FALSE);
}

bool_t
xdr_linkargs(xdrs, argp)
register struct nfslinkargs *argp;
register XDR *xdrs;
{
	if (xdr_fhandle(xdrs, &argp->la_from) &&
	    xdr_diropargs(xdrs, &argp->la_to))
		return(TRUE);

	return(FALSE);
}

bool_t
xdr_rddirargs(xdrs, argp)
register struct nfsrddirargs *argp;
register XDR *xdrs;
{
	if (xdr_fhandle(xdrs, &argp->rda_fh) &&
	    xdr_u_int32(xdrs, (u_int32 *)&argp->rda_offset) &&
	    xdr_u_int32(xdrs, (u_int32 *)&argp->rda_count))
		return(TRUE);

	return(FALSE);
}

bool_t
xdr_readargs(xdrs, argp)
register struct nfsreadargs *argp;
register XDR *xdrs;
{
	if (xdr_fhandle(xdrs, &argp->ra_fhandle) &&
	    xdr_int32(xdrs, (int32 *) &argp->ra_offset) &&
	    xdr_int32(xdrs, (int32 *) &argp->ra_count) &&
	    xdr_int32(xdrs, (int32 *) &argp->ra_totcount))
		return(TRUE);

	return(FALSE);
}

bool_t
xdr_rnmargs(xdrs, argp)
register struct nfsrnmargs *argp;
register XDR *xdrs;
{
	if (xdr_diropargs(xdrs, &argp->rna_from) &&
	    xdr_diropargs(xdrs, &argp->rna_to))
		return(TRUE);

	return(FALSE);
}

bool_t
xdr_saargs(xdrs, argp)
register struct nfssaargs *argp;
register XDR *xdrs;
{
	if (xdr_fhandle(xdrs, &argp->saa_fh) &&
	    xdr_sattr(xdrs, &argp->saa_sa))
		return(TRUE);

	return(FALSE);
}

bool_t
xdr_sattr(xdrs, argp)
register struct nfssattr *argp;
register XDR *xdrs;
{
	if (xdr_u_int32(xdrs, (u_int32 *)&argp->sa_mode) &&
	    xdr_u_int32(xdrs, (u_int32 *)&argp->sa_uid) &&
	    xdr_u_int32(xdrs, (u_int32 *)&argp->sa_gid) &&
	    xdr_u_int32(xdrs, (u_int32 *)&argp->sa_size) &&
	    xdr_timeval(xdrs, &argp->sa_atime) &&
	    xdr_timeval(xdrs, &argp->sa_mtime))
		return(TRUE);

	return(FALSE);
}

bool_t
xdr_slargs(xdrs, argp)
register struct nfsslargs *argp;
register XDR *xdrs;
{
	if (xdr_diropargs(xdrs, &argp->sla_from) &&
	    xdr_string(xdrs, &argp->sla_tnm, (u_int) MAXPATHLEN) &&
	    xdr_sattr(xdrs, &argp->sla_sa)) {
		free(argp->sla_tnm);
		return(TRUE);
	}

	return(FALSE);
}

bool_t
xdr_timeval(xdrs, argp)
register struct timeval *argp;
register XDR *xdrs;
{
	if (xdr_int32(xdrs, (int32 *)&argp->tv_sec) &&
	    xdr_int32(xdrs, (int32 *)&argp->tv_usec))
		return(TRUE);

	return(FALSE);
}

bool_t
xdr_writeargs(xdrs, argp)
register struct nfswriteargs *argp;
register XDR *xdrs;
{
	if (xdr_fhandle(xdrs, &argp->wa_fhandle) &&
	    xdr_int32(xdrs, (int32 *) &argp->wa_begoff) &&
	    xdr_int32(xdrs, (int32 *) &argp->wa_offset) &&
	    xdr_int32(xdrs, (int32 *) &argp->wa_totcount))
		return(TRUE);

	return(FALSE);
}
