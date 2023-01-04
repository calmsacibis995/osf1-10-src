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
static char *sccsid = "@(#)$RCSfile: dli_input.c,v $ $Revision: 4.4.15.3 $ (DEC) $Date: 1993/08/31 19:02:50 $";
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
 *   Telecommunications & Networks
 *
 *   IDENT HISTORY:
 *
 *	1.00 10-Jul-1985
 *      	DECnet-ULTRIX   V1.0
 *
 *	2.00 18-Apr-1986
 *		DECnet-Ultrix	V2.0
 *
 *   EDIT HISTORY
 *
 *	2.01 18-Mar-1988
 *      DECnet-ULTRIX   V2.4
 *		- Allowed use of reserved bit in individual and group SAPs
 *		- Fixed request counters and request sysid processing code
 *			to examine packet count.
 *
 *	2.02 02-Jun-1988
 *              DECnet-Ultrix   V2.4
 *              Added support for ln driver.
 *
 *	2.03 25-Jul-1988
 *              DECnet-Ultrix   V3.0
 *              Restore sysid destination to Console Carrier multicast
 *                      after transmitting to specific node.
 *
 * 	19-Dec-1988	Matt Thomas
 *		Fix unaligned accesses for PMAX (for sysid and rqctrs).
 *
 *	2.04 11-Oct-1990     Indrajit Mitra
 *      	Pass dli_mcabuffsize field in match_mcast call
 */

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <kern/assert.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/if_fddi.h>
#include <netinet/if_trn.h>
#include <dli/dli_var.h>

/*
 * DLI domain received message processing.
 */

 /*
 *		d l i i n t r
 *
 * DLI domain input routine. This routine is 'called' from the network software
 * ISR routine to process incoming packets. The first MBUF in any chain
 * contains a DLI receive descriptor containing the received ethernet header
 * and a pointer to the interface structure.
 *
 * Outputs:		None.
 *
 * Inputs:		None.
 */

void
dli_intr(
    void)
{
    for (;;) {
	register struct mbuf *m;
	struct ifnet *ifp;
	struct dli_recv recv;
	int hdrlen;

	int s = splimp();
	IF_DEQUEUE(&dli_intrq, m);
	splx(s);
	if (m == NULL)
	    break;
	if ((m->m_flags & M_PKTHDR) == 0)
	    panic("dliintr no HDR");
	ifp = m->m_pkthdr.rcvif;

	bzero(&recv, sizeof(recv));	/* XXX */

	if (ifp == NULL || (recv.rcv_dlif = dli_ifp2dlif(ifp)) == NULL) {
	    m_freem(m);
	    continue;
	}

	if ((recv.rcv_dlif->dlif_flags & (DLIF_UP|DLIF_RUNNING)) != (DLIF_UP|DLIF_RUNNING))
	    dli_dlif_start(recv.rcv_dlif);

	switch (ifp->if_type) {
	    case IFT_ETHER:
	    case IFT_ISO88023: {
		register struct ether_header *eh = mtod(m, struct ether_header *);
		register u_int etype = eh->ether_type;

		/*
		 * We get 0-length packets if the 802 length is set to 0.
		 * Drop them right away since we can't do anything with them.
		 */
		if (m->m_pkthdr.len <= sizeof(*eh)) {
		    m_freem(m);
		    continue;
		}

		if (etype > ETHERMTU) {
		    recv.rcv_flags |= DLI_FILTER_ETYPE;
		    recv.rcv_etype = htons(etype);
		}
		DLI_ADDR_COPY(eh->ether_shost, &recv.rcv_src);
		DLI_ADDR_COPY(eh->ether_dhost, &recv.rcv_dst);
		recv.rcv_hdrlen = sizeof(*eh);
		break;
	    }

	    case IFT_FDDI: {
		register struct fddi_header *fh = mtod(m, struct fddi_header *);
		DLI_ADDR_COPY(fh->fddi_shost, &recv.rcv_src);
		DLI_ADDR_COPY(fh->fddi_dhost, &recv.rcv_dst);
		recv.rcv_flags |= fh->fddi_fc & FDDIFC_LLC_PRI7 ? DLI_FILTER_ONRING : 0;
		recv.rcv_hdrlen = sizeof(*fh);
		break;
	    }
	    case IFT_ISO88025: {
		struct trn_header *trh = mtod(m, struct trn_header *);
		DLI_ADDR_COPY(trh->trn_shost, &recv.rcv_src);
		DLI_ADDR_COPY(trh->trn_dhost, &recv.rcv_dst);
		/*
		 * Clear the RII bit before passing the frame to the
		 * upper layers, independently of whether Source
		 * Routing is supported or not.
		 */
		recv.rcv_hdrlen = sizeof(struct trn_preamble);
		if (DLI_ADDR_IS_MULTI(&recv.rcv_src)) {
		    recv.rcv_src.un_l_addr[0] &= ~1;
		    recv.rcv_hdrlen += trh->trn_hdr_rif.rif_lth;
		}
		break;
	    }
	}
        if ((recv.rcv_flags & DLI_FILTER_ETYPE) == 0) {
	    caddr_t dp = mtod(m, caddr_t) + recv.rcv_hdrlen;
	    int mlen = m->m_len - recv.rcv_hdrlen;

	    if (mlen == 0) {
		dp = mtod(m->m_next, caddr_t);
		mlen = m->m_next->m_len;
	    }

	    recv.rcv_llcdata.un_l_data[0] = *(LW_T *) &dp[0];
	    if (mlen > sizeof(LW_T)) {
		recv.rcv_llcdata.un_l_data[1] = *(LW_T *) &dp[4];
	    } else {
		assert(m->m_pkthdr.len <= sizeof(u_int));
		recv.rcv_llcdata.un_l_data[1] = 0;
	    }
	    if (LLC_IS_SNAPSAP(recv.rcv_llcdata.un_l_data)) {
		recv.rcv_flags |= DLI_FILTER_SNAPSAP;
		if (LLC_IS_ENCAP_ETHER(recv.rcv_llcdata.un_l_data))
		    recv.rcv_flags |= DLI_FILTER_ETYPE;
	    } else {
		recv.rcv_flags |= DLI_FILTER_ISAP;
	    }
        }
	dli_filter_input(&recv, m);
    }
}

int
dli_rcv_addr_filter(
    register struct dli_recv *rcv,
    register struct mbuf *m,
    struct dli_filter *filter)
{
    unsigned length = m->m_pkthdr.len;

    /*
     * Here we assume that someone else has done the filtering on the individual
     * addresses and here we only have to worry about multicast filtering.
     */
    if (DLI_ADDR_IS_MULTI(&rcv->rcv_dst)) {
	union dli_addr dst = rcv->rcv_dst;
	const struct dli_fagamap *map;
	struct dli_mcast *mcast;

	if ((map = dli_addr_remap(rcv->rcv_dlif, filter, &dst, FALSE)) != NULL)
	    dst = map->map_addrs[DLI_FAGAMAP_GROUPADDR];
	if ((mcast = dli_addr_lookup(rcv->rcv_dlif, filter, &dst, map)) == NULL)
	    return 0;

	if (!DLI_TSTADDR_IDX(mcast->dlm_index, &filter->fltr_addrset)
		&& (filter->fltr_flags & DLI_FILTER_ALLMULTI) == 0)
	    return 0;
	rcv->rcv_dst = dst;
    }


    if ((*filter->fltr_match)(rcv, m, filter) == 0)
	return 0;
    filter->fltr_pdus_rcvd[DLI_ADDR_IS_MULTI(&rcv->rcv_dst)]++;
    filter->fltr_octets_rcvd[DLI_ADDR_IS_MULTI(&rcv->rcv_dst)] += length - rcv->rcv_hdrlen;
    return 1;
}

/*
 *		l o o p b a c k _ l a n _ m s g
 *
 *		This routine processes Ethernet loopback messages.  If the
 *		function code is "forward data," then the message
 *		is forwarded to its next destination.  Otherwise,
 *		nothing is done.
 *
 * Outputs:		mbuf chain given to driver if message to be forwared.
 *			returns NULL if message looped, otherwise mbuf pointer returned.
 *
 * Inputs:		m = mbuf chain containing packet.  
 *			rcv = pointer to data link header structure.
 */
int
dli_rcv_loopback(
    register struct dli_recv *rcv,
    register struct mbuf *m,
    struct dli_filter *filter)
{
    struct dli_ifnet *dlif = rcv->rcv_dlif;
    union dli_addr dst;
    struct mbuf *m0;
    u_char *loop_msg;
    u_int loop_skip_count;
    int hdrlen;

    /*
     * The mbuf chain contains the entire PDU including media header.
     * The first mbuf will contain either just media header (without LLC)
     * or the entire first of the packet and the media header.  If this
     * was received over FDDI or Token Ring or it's MOP V4.
     */
    hdrlen = 0;
    if (filter->fltr_flags & DLI_FILTER_SNAPSAP)
	hdrlen += 8;
    if (m->m_len > rcv->rcv_hdrlen) {
	m0 = m;
	hdrlen += rcv->rcv_hdrlen;
    } else {
	m0 = m->m_next;
    }

    if (m0->m_len < 2 + 2 + 6 + hdrlen)
        return FALSE;

    loop_msg = mtod(m0, u_char *) + hdrlen;
    loop_skip_count = LE_EXT16(loop_msg) + 2;
    /*
     *  There's need to be at least 8 bytes beyond the skipped amount
     *  for a new opcode and a LAN address.  Alost the skip count must
     *  be an even number.
     */
    if (hdrlen + loop_skip_count + 10 > m->m_len || (loop_skip_count & 1))
	return FALSE;

    if (LE_EXT16(&loop_msg[loop_skip_count]) != DLI_LBACK_FWD)
	return FALSE;
    loop_skip_count += 2;	/* advance past the loop code */

    /*
     * Make sure the destination address is vaild (ie NOT multicast)
     */
    DLI_ADDR_COPY(&loop_msg[loop_skip_count], &dst);
    if (DLI_ADDR_IS_MULTI(&dst))
        return FALSE;

    loop_skip_count += DLI_EADDRSIZE;
    LE_INS16(loop_msg, loop_skip_count - 2);

    m_adj(m, rcv->rcv_hdrlen);
    dli_output(dlif, filter, m, &dst);
    return TRUE;
}

/*
 *		d l i _ r c v _ m o p r c 
 *
 * This routine is called to process requests for sysid 
 * If this is a request for a sysid from a node
 *    find the matching sysid_to struct to tx on
 *    copy requestor node address to the target
 *    transmit the sysid 
 *
 * Outputs:		None.
 *
 * Inputs:		pointer to mbuf chain from driver
 *              pointer to dli_recv structure.
 *
 * Version History:
 * 1.0	JA
 *
 */
int
dli_rcv_moprc(
    struct dli_recv *rcv,
    struct mbuf *m,
    struct dli_filter *filter)
{
    struct mbuf *m0;
    int msglen, pktlen, hdrlen;
    u_char *dp;

    /*
     * The mbuf chain contains the entire PDU including media header.
     * The first mbuf will contain either just media header (without LLC)
     * or the entire first of the packet and the media header.  If this
     * was received over FDDI or Token Ring or it's MOP V4.
     */
    hdrlen = 0;
    if (filter->fltr_flags & DLI_FILTER_SNAPSAP)
	hdrlen += 8;
    if (m->m_len > rcv->rcv_hdrlen) {
	m0 = m;
	hdrlen += rcv->rcv_hdrlen;
    } else {
	m0 = m->m_next;
    }

    dp = mtod(m0, u_char *) + hdrlen;
    pktlen = m->m_pkthdr.len;

    if (filter->fltr_flags & DLI_FILTER_ETYPE) {
	msglen = LE_EXT16(dp);
	if (msglen + 2 > pktlen) {
	    m_freem(m);
	    return TRUE;
	}
	dp += 2, pktlen -= 2;
    } else {
	msglen = pktlen;
    }

    if (LE_EXT8(dp) == DLI_REQSYSID) {
	if (msglen >= 4) {
	    u_int receipt = LE_EXT16(dp + 2);
	    if (rcv->rcv_dlif->dlif_ifp->if_sysid_type) {
		dli_snd_sysid(rcv->rcv_dlif, filter, receipt, &rcv->rcv_src);
	    }
	}
	m_freem(m);
	return TRUE;
    }
    if (LE_EXT8(dp) == DLI_REQCTRS) {
	if (msglen >= 3) {
	    u_int receipt = LE_EXT16(dp + 1);
	    m_freem(m);
	    dli_snd_mopctrs(rcv->rcv_dlif, filter, receipt, &rcv->rcv_src);
	}
	return TRUE;
    }
    return FALSE;
}
