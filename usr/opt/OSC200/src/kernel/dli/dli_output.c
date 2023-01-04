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
static char *sccsid = "@(#)$RCSfile: dli_output.c,v $ $Revision: 4.4.8.4 $ (DEC) $Date: 1993/12/15 20:12:04 $";
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
 *		DECnet-Ultrix	V2.0
 *
 * Added sysid and point-to-point support
 *
 */

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <kern/assert.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <dli/dli_var.h>

/*
 * DLI output subroutine.
 *
 * Attempt to transmit all pending messages for the specified virtual circuit.
 *
 * Outputs:		None.
 *
 * Inputs:
 *	uentry		= Pointer to the user's line descriptor.
 *	ifp		= Pointer to the interface to xmit over.
 *	m		= Pointer to mbuf chain to be transmitted.
 *	dst_addr	= Pointer to structure containing target address.
 */
int
dli_send(
    struct dli_pcb *pcb,
    struct mbuf *m,
    struct sockaddr_dl *dst)
{
    struct dli_ifnet *dlif;
    struct dli_filter *filter;
    union dli_llcdata llchdr;
    union dli_addr addr;
    int error;

    if (pcb == NULL) {
	m_freem(m);
	return EHOSTDOWN;
    }

    if (pcb->dli_state != DLS_ON) {
	m_freem(m);
	return ENETDOWN;
    }

    filter = pcb->dli_filter;
    if (filter->fltr_flags & DLI_FILTER_ISAP) {
	llchdr.un_llc.llc_dsap = pcb->dli_lineid.choose_addr.dli_802addr.eh_802.dsap;
	llchdr.un_llc.llc_ssap = pcb->dli_lineid.choose_addr.dli_802addr.eh_802.ssap;
	llchdr.un_llc.llc_is_control = pcb->dli_lineid.choose_addr.dli_802addr.eh_802.ctl.I_S_fmt;
    } else {
	llchdr = filter->fltr_llcdata;
    }

    if (dst != NULL) {
	if (pcb->dli_dlif != dli_devid2dlif(&dst->dli_device)
	        || pcb->dli_lineid.dli_substructype != dst->dli_substructype) {
	    m_freem(m);
	    return EINVAL;
	}

	if (filter->fltr_flags & DLI_FILTER_ISAP) {
	    llchdr.un_llc.llc_dsap = dst->choose_addr.dli_802addr.eh_802.dsap;
	    llchdr.un_llc.llc_ssap |= (dst->choose_addr.dli_802addr.eh_802.ssap & 1);
	    if ((filter->fltr_flags & DLI_FILTER_SVC_TYPE1) == 0)
		llchdr.un_llc.llc_is_control = dst->choose_addr.dli_802addr.eh_802.ctl.I_S_fmt;
	}
	if (filter->fltr_flags & DLI_FILTER_ETYPE) {
	    DLI_ADDR_COPY(dst->choose_addr.dli_eaddr.dli_target, &addr);
	} else {
	    DLI_ADDR_COPY(dst->choose_addr.dli_802addr.eh_802.dst, &addr);
	}
    } else {
	addr = pcb->dli_dstaddr;
    }


    error = dli_add_header(pcb->dli_dlif, &m, &llchdr, filter->fltr_flags);
    if (error)
	return error;
    return dli_output(pcb->dli_dlif, filter, m, &addr);
}

int
dli_add_header(
    struct dli_ifnet *dlif,
    struct mbuf **mp,
    const union dli_llcdata *llc,
    int flags)
{
    union dli_llcdata llchdr;
    struct mbuf *m = *mp;
    int addmtu = 0;
    int llclen = 0;
    /*
     * for 802 packets, validate user data length
     * and then build the 802 header for transmission
     */
    if (flags & DLI_FILTER_ISAP) {
	if (dlif->dlif_ifflags & IFF_SNAP)
	    addmtu = MAX802DATANP;
	if (llc != NULL) {
	    if (LLC_FMT(llc->un_llc.llc_control) == LLC_U_FMT)
		llclen = 3;
	    else
		llclen = 4;
	}
    } else if (flags & DLI_FILTER_ETYPE) {
	if (flags & DLI_FILTER_ETHERPAD) {
	    LE_INS16(llchdr.un_data, m->m_pkthdr.len);
	    llclen = 2;
	    llc = &llchdr;
	}
    } else if ((flags & DLI_FILTER_SNAPSAP) && llc != NULL) {
	llclen = 8;
    }

    if (m->m_pkthdr.len + llclen > dlif->dlif_ifmtu + addmtu) {
	m_freem(m);
	*mp = NULL;
	return EMSGSIZE;
    }

    if (llclen > 0) {
	M_PREPEND(m, llclen, M_DONTWAIT);
	*mp = m;
	if (m == NULL)
	    return ENOBUFS;
	bcopy((caddr_t) llc->un_data, mtod(m, caddr_t), llclen);
    }
    return 0;
}

int
dli_output(
    struct dli_ifnet *dlif,
    struct dli_filter *filter,
    struct mbuf *m,
    const union dli_addr *dst)
{
    struct sockaddr sa;

    sa.sa_family = AF_UNSPEC;
    if (filter->fltr_flags & DLI_FILTER_ETYPE) {
	DLI_EHDR(&sa)->ether_type = ntohs(filter->fltr_llc.llc_snap.ether_type);
    } else {
	DLI_EHDR(&sa)->ether_type = 0;
    }

    if (DLI_ADDR_IS_MULTI(dst)) {
	const struct dli_fagamap *map;
	if (dlif->dlif_fagamap != NULL
	        && (map = dli_addr_remap(dlif, filter, dst, TRUE)) != NULL)
	    dst = &map->map_addrs[DLI_FAGAMAP_FUNCADDR];
	filter->fltr_pdus_sent[1]++;
	filter->fltr_octets_sent[1] += m->m_pkthdr.len;
    } else {
	filter->fltr_pdus_sent[0]++;
	filter->fltr_octets_sent[0] += m->m_pkthdr.len;
    }
    DLI_DSTADDR_TO_EHDR(dst, DLI_EHDR(&sa));

    return dlif_output(dlif, m, &sa);
}
