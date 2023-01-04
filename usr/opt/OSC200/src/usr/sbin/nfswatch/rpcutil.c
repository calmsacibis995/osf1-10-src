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
static char *rcsid = "@(#)$RCSfile: rpcutil.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/12/21 23:36:18 $";
#endif
/* Based on:
 * static char *RCSid = "/home/harbor/davy/system/nfswatch/RCS/rpcutil.c,v 4.0 1993/03/01 19:59:00 davy Exp";
 */

#include "os.h"

/*
 * rpcutil.c - routines for emulating RPC library functions without really
 *	       receiving packets.
 *
 * David A. Curry				Jeffrey C. Mogul
 * Purdue University				Digital Equipment Corporation
 * Engineering Computer Network			Western Research Laboratory
 * 1285 Electrical Engineering Building		250 University Avenue
 * West Lafayette, IN 47907-1285		Palo Alto, CA 94301
 * davy@ecn.purdue.edu				mogul@decwrl.dec.com
 *
 * log: rpcutil.c,v
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
 * Revision 3.0  1991/01/23  08:23:22  davy
 * NFSWATCH Version 3.0.
 *
 * Revision 1.2  90/08/17  15:47:46  davy
 * NFSWATCH Version 2.0.
 * 
 * Revision 1.1  88/11/29  11:20:57  davy
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
#ifdef SVR4
#include <rpc/clnt_soc.h>
#endif
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
#include "externs.h"
#include "rpcdefs.h"

/*
 * Operations on the SVCXPRT structure.  We're only going to use
 * the one to get arguments from it.
 */
static struct xp_ops xp_ops = {
	NULL, NULL, rpcxdr_getargs, NULL, NULL, NULL
};

static SVCXPRT	*xprt;		/* the service description		*/

/*
 * setup_rpcxdr - set up for decoding RPC XDR stuff.  Sort of a svcudp_create
 *		  without the socket code.
 */
void
setup_rpcxdr()
{
	register struct svcudp_data *su;

	/*
	 * Allocate the SVCXPRT structure.
	 */
	if ((xprt = (SVCXPRT *) malloc(sizeof(SVCXPRT))) == NULL) {
		(void) fprintf(stderr, "%s: out of memory.\n", pname);
		finish(-1);
	}

	/*
	 * Allocate UDP service data.
	 */
	if ((su = (struct svcudp_data *) malloc(sizeof(struct svcudp_data))) == NULL) {
		(void) fprintf(stderr, "%s: out of memory.\n", pname);
		finish(-1);
	}

	/*
	 * This is the maximum size of a packet.
	 */
	su->su_iosz = ((UDPMSGSIZE + 3) / 4) * 4;

	/*
	 * Get a buffer to store stuff in.
	 */
	if ((rpc_buffer(xprt) = (char *) malloc(su->su_iosz)) == NULL) {
		(void) fprintf(stderr, "%s: out of memory.\n", pname);
		finish(-1);
	}

	/*
	 * Fill in the SVCXPRT structure.  This is a standard RPC routine.
	 */
	(void) xdrmem_create(&(su->su_xdrs), rpc_buffer(xprt), su->su_iosz,
		XDR_DECODE);

	xprt->xp_ops = &xp_ops;
	xprt->xp_p2 = (caddr_t) su;
	xprt->xp_verf.oa_base = su->su_verfbody;
}

/*
 * udprpc_recv - pretend we've received an RPC packet - this is sort of like
 *		 svcudp_recv.
 */
int udprpc_recv(data, length, msg, xp)
register struct rpc_msg *msg;
register u_int length;
register char *data;
SVCXPRT **xp;
{
	register XDR *xdrs;
	register struct svcudp_data *su;

	su = su_data(xprt);
	xdrs = &(su->su_xdrs);

	/*
	 * Too short.
	 */
	if (length < (4 * sizeof(u_int32)))
		return(FALSE);

	if (length > truncation)
		length = truncation;

	/*
	 * Copy the data.
	 */
	(void) bcopy(data, rpc_buffer(xprt), min(length, su->su_iosz));

	xdrs->x_op = XDR_DECODE;

	/*
	 * Set the XDR routines to the start of the buffer.
	 */
	(void) XDR_SETPOS(xdrs, 0);

	/*
	 * Decode the RPC message structure.
	 */
	if (!xdr_callmsg(xdrs, msg))
		return(FALSE);

	su->su_xid = msg->rm_xid;
	*xp = xprt;

	return(TRUE);
}

/*
 * rpcxdr_getargs - called by SVC_GETARGS.
 */
static bool_t
rpcxdr_getargs(xprt, xdr_args, args_ptr)
register xdrproc_t xdr_args;
register caddr_t args_ptr;
register SVCXPRT *xprt;
{
	return((*xdr_args)(&(su_data(xprt)->su_xdrs), args_ptr));
}
