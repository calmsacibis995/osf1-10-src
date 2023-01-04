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
static char *rcsid = "@(#)$RCSfile: authunix_prot.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/05/18 21:06:37 $";
#endif
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = 	"@(#)authunix_prot.c	1.5 90/07/17 4.1NFSSRC Copyr 1990 Sun Micro";
#endif

/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.17 88/02/08
*/

/*
 * authunix_prot.c
 * XDR for UNIX style authentication parameters for RPC
 */

#include <rpc/rpc.h>

/*
 * XDR for unix authentication parameters.
 */
bool_t
xdr_authunix_parms(xdrs, p)
	register XDR *xdrs;
	register struct authunix_parms *p;
{

	if (xdr_u_int(xdrs, &(p->aup_time))
	    && xdr_string(xdrs, &(p->aup_machname), MAX_MACHINE_NAME)
	    && xdr_int(xdrs, &(p->aup_uid))
	    && xdr_int(xdrs, &(p->aup_gid))
	    && xdr_array(xdrs, (caddr_t *)&(p->aup_gids),
		    &(p->aup_len), NGRPS, sizeof(int), xdr_int) ) {
		return (TRUE);
	}
	return (FALSE);
}

#ifdef KERNEL
/*
 * XDR kernel unix auth parameters.
 * NOTE: this is an XDR_ENCODE only routine.
 */
xdr_authkern(xdrs)
	register XDR *xdrs;
{
	gid_t	*gp = u.u_groups;
	int	 uid = (int)u.u_uid;
	int	 gid = (int)u.u_gid;
	int	 len = (int)u.u_ngroups;
	char	*name = hostname;

	if (xdrs->x_op != XDR_ENCODE) {
		return (FALSE);
	}

        if (xdr_int(xdrs, (int *)&time.tv_sec)
            && xdr_string(xdrs, &name, MAX_MACHINE_NAME)
            && xdr_int(xdrs, &uid)
            && xdr_int(xdrs, &gid)
	    && xdr_array(xdrs, (caddr_t)gp, (u_int *)&len, NGRPS, sizeof (int), xdr_int) ) {
                return (TRUE);
	}
	return (FALSE);
}
#endif
