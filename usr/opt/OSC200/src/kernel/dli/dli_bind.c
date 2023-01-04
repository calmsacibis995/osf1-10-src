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
static char *sccsid = "@(#)$RCSfile: dli_bind.c,v $ $Revision: 4.4.7.3 $ (DEC) $Date: 1993/10/22 22:20:29 $";
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
 *
 * 2.00 18-Apr-1986
 *      DECnet-ULTRIX   V2.0
 *		- added sysid and point to point support
 *
 * 2.01 18-Mar-1988
 *      DECnet-ULTRIX   V2.4
 *		- Allowed use of reserved bit in individual and group SAPs
 *
 * 2.02 11-Oct-1990
 *      DECtrade-ULTRIX V1.0
 *              - Added mcast buffer size (in MCAST_SIZE units) in
 *                dli_openport routine. Comparison is done based on this
 *                size and *NOT* on MCAST_MAXNUM ( as was done previously )
 * 
 */

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/errno.h>
#include <kern/assert.h>
#include <net/if.h>
#include <dli/dli_var.h>

/*
 *		d l i _ b i n d
 *
 * Process a DLI bind request by binding to a particular device and
 * optionally to an address.
 *
 * Returns:		Error code if error occurs, otherwise 0.
 *
 * Inputs:
 *	port		= Pointer to the user port for this request.
 *	addr		= Pointer to structure containing address.
 */
int
dli_bind(
    register struct dli_pcb **pcb_p,
    register struct sockaddr_dl *addr)
{
    register struct dli_pcb *pcb;
    struct dli_ifnet *dlif;
    struct dli_filter *filter;
    int error = 0;

    if (addr->dli_family != AF_DLI)
	return EAFNOSUPPORT;

    if ((dlif = dli_devid2dlif(&addr->dli_device)) == NULL)
	return ENXIO;

    if ((pcb = (struct dli_pcb *) kalloc(sizeof(*pcb))) == NULL)
	return ENOBUFS;

    error = dli_filter_add(dlif, addr, &filter);
    if (error) {
	kfree(pcb, sizeof(*pcb));
	return error;
    }

    *pcb_p = pcb;
    bzero(pcb, sizeof(*pcb));
    DLI_PCB_LOCKINIT(pcb);
    DLI_PCB_LOCK(pcb);
    pcb->dli_filter = filter;
    pcb->dli_filter->fltr_ctx = (caddr_t) pcb;
    pcb->dli_filter->fltr_match = dli_rcv_user;
    pcb->dli_state = DLS_ON;
    pcb->dli_lineid = *addr;
    pcb->dli_rcvaddr = *addr;
    pcb->dli_dlif = dlif;
    if (filter->fltr_flags & DLI_FILTER_ETYPE) {
	DLI_ADDR_COPY(addr->choose_addr.dli_eaddr.dli_target, &pcb->dli_dstaddr);
    } else {
	DLI_ADDR_COPY(addr->choose_addr.dli_802addr.eh_802.dst, &pcb->dli_dstaddr);
    }
    DLI_PCB_UNLOCK(pcb);
    return 0;
}

/*
 *		d l i _ u n b i n d
 *
 * Delete a DLI socket socket
 *
 * Returns:		Nothing
 *
 * Inputs:
 *	pcb_p		= Pointer to a pointer to the PCB for this request.
 */
int
dli_unbind(
    struct dli_pcb **pcb_p)
{
    register struct dli_pcb *pcb = *pcb_p;

    dli_filter_destroy(pcb->dli_dlif, &pcb->dli_filter);

    DLI_PCB_LOCK(pcb);
    (*pcb_p) = NULL;

    DLI_PCB_UNLOCK(pcb);
    kfree(pcb, sizeof(*pcb));
    return 0;
}

/*
 *		f o u n d _ u s e r
 *
 * This routine places an mbuf chain on a user's receive queue.
 *
 * Note:		Both line table entry and socket must be locked
 *				before this routine called.
 *
 * Outputs:		None.
 *
 * Inputs:		m = Pointer to mbuf chain; first mbuf is
 *				garbage.
 * 			so = Pointer to user's socket structure.
 *			rcv = pointer to data link header structure.
 */
int
dli_rcv_user(
    register struct dli_recv *rcv,
    register struct mbuf *m,
    register struct dli_filter *filter)
{
    register struct dli_pcb *pcb = (struct dli_pcb *) filter->fltr_ctx;
    struct socket *so = pcb->dli_so;
    u_short *so_src, *so_dst;
    u_int hdrlen = rcv->rcv_hdrlen;

    /*
     * The mbuf chain contains the entire PDU including media header.
     * The first mbuf will contain either just media header (without LLC)
     * or the entire first of the packet and the media header.
     */

    /*
     * We reverse source and destination so that sendto(2) will work 
     * on the sockaddr_dl that's returned from recvfrom(2).
     */
    if (filter->fltr_flags & DLI_FILTER_ISAP) {
	struct sockaddr_802 *addr = &pcb->dli_rcvaddr.choose_addr.dli_802addr;
	if (rcv->rcv_llc.llc_control == UI_NPCMD) {
	    addr->eh_802.ctl.U_fmt = UI_NPCMD;
	    hdrlen += 3;
	} else if (filter->fltr_flags & DLI_FILTER_SVC_TYPE1) {
	    unsigned ctl = rcv->rcv_llc.llc_control & ~LLC_PF_FLAG;
	    if (ctl != TEST_NPCMD && ctl != XID_NPCMD)
		return FALSE;
	    if (dli_llc_responder(rcv, m, filter))
	        return TRUE;
	    if (pcb->dli_lineid.choose_addr.dli_802addr.eh_802.ctl.U_fmt
		    != rcv->rcv_llc.llc_control)
		return FALSE;
	    addr->eh_802.ctl.U_fmt = rcv->rcv_llc.llc_control;
	    hdrlen += 3;
	} else if (LLC_FMT(rcv->rcv_llc.llc_control) == LLC_U_FMT) {
	    addr->eh_802.ctl.U_fmt = rcv->rcv_llc.llc_control;
	    hdrlen += 3;
	} else {
	    addr->eh_802.ctl.I_S_fmt = rcv->rcv_llc.llc_is_control;
	    hdrlen += 4;
	}
	addr->eh_802.ssap = rcv->rcv_llc.llc_dsap;	/* swap 'em */
	addr->eh_802.dsap = rcv->rcv_llc.llc_ssap;	/* swap 'em */
	so_src = (u_short *) addr->eh_802.dst;	/* swap 'em */
	so_dst = (u_short *) addr->eh_802.src;	/* swap 'em */
	if (m->m_pkthdr.len < hdrlen) {
	    m_freem(m);
	    return TRUE;
	} else {
	    m_adj(m, hdrlen);		/* nuke the datalink header & llc */
	}
    } else if (filter->fltr_flags & DLI_FILTER_ETYPE) {
	struct sockaddr_edl *addr = &pcb->dli_rcvaddr.choose_addr.dli_eaddr;
	so_src = (u_short *) addr->dli_target;		/* swap 'em */
	so_dst = (u_short *) addr->dli_dest;		/* swap 'em */
	if (filter->fltr_flags & DLI_FILTER_SNAPSAP)
	    hdrlen += 8;
	if (m->m_pkthdr.len < hdrlen) {
	    m_freem(m);
	    return TRUE;
	} else {
	    m_adj(m, hdrlen);		/* nuke the datalink header & llc */
	}
	if (addr->dli_options & DLI_ETHERPAD) {
	    if ((m = dli_trim_etherpad(m)) == NULL)
		return TRUE;
	}
    } else /* it must be a SNAP SAP */ {
	struct sockaddr_802 *addr = &pcb->dli_rcvaddr.choose_addr.dli_802addr;
	so_src = (u_short *) addr->eh_802.dst;		/* swap 'em */
	so_dst = (u_short *) addr->eh_802.src;		/* swap 'em */
	if (m->m_pkthdr.len <= hdrlen) {
	    m_freem(m);
	    return TRUE;
	} else {
	    m_adj(m, hdrlen + 8);	/* nuke the datalink header & llc */
	}
    }
    DLI_PCB_LOCK(pcb);

    so_src[0] = rcv->rcv_src.un_w_addr[0];
    so_src[1] = rcv->rcv_src.un_w_addr[1];
    so_src[2] = rcv->rcv_src.un_w_addr[2];

    so_dst[0] = rcv->rcv_dst.un_w_addr[0];
    so_dst[1] = rcv->rcv_dst.un_w_addr[1];
    so_dst[2] = rcv->rcv_dst.un_w_addr[2];

    DLI_SOCKET_LOCK(so);
    DLI_SOCKBUF_LOCK(&so->so_rcv);
    if (sbappendaddr(&so->so_rcv, (struct sockaddr *) &pcb->dli_rcvaddr,
		     m, NULL) == 0) {
	DLI_SOCKBUF_UNLOCK(&so->so_rcv);
	DLI_SOCKET_UNLOCK(so);
	m_freem(m);
    } else {
	DLI_SOCKBUF_UNLOCK(&so->so_rcv);
	sorwakeup(pcb->dli_so);
	DLI_SOCKET_UNLOCK(so);
    }
    DLI_PCB_UNLOCK(pcb);
    return TRUE;
}
