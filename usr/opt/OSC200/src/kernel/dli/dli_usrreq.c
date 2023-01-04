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
#ifndef	lint
static char *sccsid = "@(#)$RCSfile: dli_usrreq.c,v $ $Revision: 4.4.6.3 $ (DEC) $Date: 1993/10/22 22:21:05 $";
#endif

/*
 *   COPYRIGHT (c) DIGITAL EQUIPMENT CORPORATION 1991.  ALL
 *   RIGHTS RESERVED.
 *
 *   THIS  SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE
 *   USED  AND  COPIED ONLY IN ACCORDANCE WITH THE TERMS OF
 *   SUCH  LICENSE  AND  WITH  THE  INCLUSION  OF THE ABOVE
 *   COPYRIGHT  NOTICE.   THIS SOFTWARE OR ANY OTHER COPIES
 *   THEREOF   MAY   NOT  BE  PROVIDED  OR  OTHERWISE  MADE
 *   AVAILABLE  TO  ANY  OTHER  PERSON.   NO  TITLE  TO AND
 *   OWNERSHIP OF THE SOFTWARE IS HEREBY TRANSFERRED.
 *
 *   THE  INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE
 *   WITHOUT  NOTICE  AND  SHOULD  NOT  BE  CONSTRUED  AS A
 *   COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 *
 *   DIGITAL  ASSUMES  NO  RESPONSIBILITY  FOR  THE  USE OR
 *   RELIABILITY  OF  ITS SOFTWARE ON EQUIPMENT THAT IS NOT
 *   SUPPLIED BY DIGITAL.
 *
 *
 *   Telecommunications & Networks
 *
 * IDENT HISTORY:
 *
 * 1.00 10-Jul-1985
 *      DECnet-ULTRIX   V1.0
 * 4.1.1.3 28-Apr-1988
 *      DECnet-ULTRIX   V2.4 - do not free mbuf on getsockopt failure.
 *
 */

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/protosw.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <kern/assert.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <dli/dli_var.h>

/*
 * DLI protocol interface to socket abstraction.
 */

/*
 *		d l i _ u s r r e q
 *
 * Process a DLI request.
 *
 * Returns:		Nothing
 *
 * Inputs:
 *	so		= Pointer to the socket for this request.
 *	req		= Request function code.
 *	m		= Pointer to an MBUF chain.
 *	nam		= Pointer to an MBUF chain for addressing.
 *	rights		= Pointer to an MBUF chain for access rights.
 */
int
dli_usrreq(
    register struct socket *so,
    int req,
    struct mbuf *m,
    struct mbuf *nam,
    struct mbuf *rights)
{
    int error = 0;

    if (so->so_pcb == NULL && (req != PRU_ATTACH && req != PRU_BIND && req != PRU_CONTROL)) {
	if (req == PRU_SEND)
	    m_free(m);
	return ENOTCONN;
    }

    /* 
     * Service request.
     */
    switch (req) {
	/* 
	 * DLI attaches to a socket via PRU_ATTACH, reserving space.
	 */
	case PRU_ATTACH: 
	    if (dli_mustberoot && (so->so_state & SS_PRIV) == 0) {
		error = EACCES;
		break;
	    }
	    error = soreserve (so, DLI_SENDSPACE, DLI_RECVSPACE);
	    break;

	/* 
	 * PRU_DISCONNECT set up as a NOP so that close() returns success.
	 */
	case PRU_DISCONNECT: 
	    break;

	/* 
	 * PRU_DETACH detaches the DLI protocol from the socket.
	 */
	case PRU_DETACH:
	    if (so->so_pcb != NULL)
		dli_unbind((struct dli_pcb **) &so->so_pcb);
	    sbflush (&so->so_rcv);
	    sbflush (&so->so_snd);
	    soisdisconnected (so);
	    break;

	/* 
	 * Bind name to socket
	 */
	case PRU_BIND: {
	    struct dli_pcb *pcb;
	    if ((error = dli_validate_addr(nam)) != 0)
		break;
	    if (so->so_pcb != NULL) {
	        error = EADDRINUSE;
		break;
	    }
            error = dli_bind(&pcb, mtod(nam, struct sockaddr_dl *));
	    if (error)
		break;
	    pcb->dli_so = so;
	    so->so_pcb = (caddr_t) pcb;
	    soisconnected(so);
	    break;
	}

	/* 
	 * Data is available for transmission.
	 */
	case PRU_SEND: {
	    register struct sockaddr_dl *dst = NULL;
	    if (nam != NULL) {
		if ((error = dli_validate_addr(nam)) != 0)
		    break;
		dst = mtod(nam, struct sockaddr_dl *);
	    }
	    error = dli_send(so2dlipcb(so), m, dst);
	    break;
	}

	/* 
	 * Return value of user's bound socket address.
	 */
	case PRU_SOCKADDR: 
	    if (M_TRAILINGSPACE(nam) < sizeof(struct sockaddr_dl)) {
		MCLGET(nam, M_DONTWAIT);
		if (M_TRAILINGSPACE(nam) < sizeof(struct sockaddr_dl)) {
		    error = ENOBUFS;
		    break;
		}
	    }
	    nam->m_len = sizeof(so2dlipcb(so)->dli_lineid);
	    *mtod (nam, struct sockaddr_dl *) = so2dlipcb(so)->dli_lineid;
	    break;

	/* 
	 * An ioctl() was issued
	 */
	case PRU_CONTROL: {
	    struct ifnet *ifp = (struct ifnet *) rights;
	    struct dli_ifnet *dlif = NULL;
	    if (ifp != NULL && (dlif = dli_ifp2dlif(ifp)) == NULL) {
		error = ENXIO;
		break;
	    }
	    error = dli_control(so2dlipcb(so), (int) m, (caddr_t) nam, dlif);
	    break;
	}

	default: 
	    error = EOPNOTSUPP;
	    break;
    }
    return error;
}

/*
 *              d l i _ c t l o u t p u t
 *
 *
 * All set and get socket options from the socket layer come
 * through here.  For now, this routine performs some functions
 * previously done by the socket layer and dli_usrreq().
 *
 * Note: The socket lock is not asserted when this routine is called.
 *
 * Returns:             status
 *
 * Inputs:
 *      op              = Option request code.
 *      so              = Pointer to the socket for this request.
 *      level           = Socket level.
 *      optname         = Option name passed by application.
 *      m               = Pointer to an MBUF containing or to contain
 *                              option data.
 */
int
dli_ctloutput(
    int op,
    struct socket *so,
    long level,
    long optname,
    struct mbuf **mp)
{
    int error = 0;

    switch (op) {
	/* 
	 * A getsockopt() was issued.
	 */
	case PRCO_GETOPT:  {
	    struct dli_pcb *pcb = so2dlipcb(so);
	    struct mbuf *m;
	    MGET(m, M_DONTWAIT, MT_SOOPTS);
	    if (m == NULL) {
		error = ENOBUFS;
		break;
	    }
	    *mp = m;
	    MCLGET(m, M_DONTWAIT);
	    if (M_TRAILINGSPACE(m) <= MLEN) {
		error = ENOBUFS;
		break;
	    }

	    /* if the operation is not one of the generic DLI operations
	     * or the protocol number is not DLPROTO_DLI, and if
	     * there is a protocol switch table registered for this
	     * DLI client, then call the ctloutput routine for it.
	     * This is how the NETMAN subset can extend the set of
	     * of operations that base system DLI supports
	     */
	    if (so->so_proto->pr_protocol != level || optname == DLI_LOCALENTITYNAME) {
		if (pcb->dli_dlif && pcb->dli_dlif->dlif_proto) {
		    error = (*pcb->dli_dlif->dlif_proto->pr_ctloutput)(op, pcb->dli_filter->fltr_lanport, level, optname, mp);
		} else
		    error = EOPNOTSUPP;
	    } else {
		error = dli_getopt(pcb, mtod(m, u_char *), &m->m_len, optname);
	    }
	    break;
	}

	/* 
	 * A setsockopt() was issued.
	 */
	case PRCO_SETOPT: {
	    struct dli_pcb *pcb = so2dlipcb(so);
	    if (*mp == NULL) {
		error = EFAULT;
		break;
	    }
	    /* see comment above for PRCO_GETOPT */
	    if (so->so_proto->pr_protocol != level || optname == DLI_LOCALENTITYNAME) {
		if (pcb->dli_dlif && pcb->dli_dlif->dlif_proto) {
		    error = (*pcb->dli_dlif->dlif_proto->pr_ctloutput)(op, pcb->dli_filter->fltr_lanport, level, optname, mp);
		} else {
		    error = EOPNOTSUPP;
		}
	    } else {
		error = dli_setopt(pcb, mtod(*mp, u_char *), (*mp)->m_len, optname);
	    }
	    if (*mp)
		m_freem (*mp);
	    break;

	}

	 /*
	  * This function is only used by applications which have
	  * a dependency on the NETMAN subset (e.g. DECnet-OSI, X.25)
	  */
	case PRCO_PIF:
	    if (dlimgtsw != NULL) {
		error = (*dlimgtsw->mgt_prco_pif)(op, so, level, optname, mp);
	    } else {
		error = EOPNOTSUPP;
	    }
	    break;

	default: 
	    error = EOPNOTSUPP;
	    break;
    }

    return (error);
}

/*
 *              d l i _ c o n t r o l
 *
 *
 * All ioctls from the socket layer or if.c come
 * through here.
 *
 * Returns:             status
 *
 * Inputs:
 *      so              = Pointer to the socket for this request (locked)
 *      cmd             = ioctl to be performed
 *      data            = data for ioctl
 *      ifp             = ifnet structure associated with ioctl (maybe 0)
 */
int
dli_control(
    struct dli_pcb *pcb,
    unsigned int cmd,
    caddr_t data,
    struct dli_ifnet *dlif)
{
    int error;

    if (dlif == NULL && pcb != NULL)
	dlif = pcb->dli_dlif;
    if (dlimgtsw != NULL && (*dlimgtsw->mgt_control)(pcb, cmd, data, dlif, &error))
        return error;

    return dlif_ioctl(dlif, cmd, data);
}

int
dli_validate_addr(
    struct mbuf *nam)
{
    register struct sockaddr_dl *dst;
    register int len;

    dst = mtod(nam, struct sockaddr_dl *);
    if (nam->m_len < dst->dli_len)
	return EINVAL;
    if (dst->dli_len < offsetof(struct sockaddr_dl, choose_addr))
	return EINVAL;
    if (dst->dli_family != AF_DLI)
	return EAFNOSUPPORT;
    switch (dst->dli_substructype) {
	case DLI_802: 
	    len = sizeof (struct sockaddr_802);
	    break;
	case DLI_ETHERNET: 
	    len = sizeof (struct sockaddr_edl);
	    break;
	default: 
	    return EINVAL;
    }
    if (offsetof(struct sockaddr_dl, choose_addr) + len > dst->dli_len)
	return EINVAL;
    return 0;
}
