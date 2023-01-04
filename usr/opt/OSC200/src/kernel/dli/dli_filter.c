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
static char *rcsid = "@(#)$RCSfile: dli_filter.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/12/20 19:11:32 $";
#endif

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <kern/assert.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <dli/dli_var.h>

int
dli_filter_create(
    struct dli_ifnet *dlif,
    struct dli_filter **filter_p,
    unsigned ioctlflg,
    u_char *srcaddr)
{
    struct dli_filter *filter;
    if ((filter = (struct dli_filter *) kalloc(sizeof(*filter))) == NULL)
        return ENOMEM;

    bzero(filter, sizeof(*filter));
    switch (ioctlflg) {
    case DLI_EXCLUSIVE:
        filter->fltr_flags |= DLI_FILTER_EXCLUSIVE;
	break;
    case DLI_DEFAULT:
        filter->fltr_flags |= DLI_FILTER_DEFAULT;
	break;
    case DLI_NORMAL:
	if ((srcaddr[0] & 1) == 0) {
	    filter->fltr_srcaddr[0] = srcaddr[0];
	    filter->fltr_srcaddr[1] = srcaddr[1];
	    filter->fltr_srcaddr[2] = srcaddr[2];
	    filter->fltr_srcaddr[3] = srcaddr[3];
	    filter->fltr_srcaddr[4] = srcaddr[4];
	    filter->fltr_srcaddr[5] = srcaddr[5];
	    break;
	}
	/* fall through */
    default:
	kfree(filter, sizeof(*filter));
	return EINVAL;
    }
    dli_addrset_init(dlif, filter, &filter->fltr_addrset);
    filter->fltr_match = dli_rcv_and_drop;
    *filter_p = filter;
    return 0;
}

int
dli_filter_add(
    struct dli_ifnet *dlif,
    const struct sockaddr_dl *addr,
    struct dli_filter **filter_p)
{
    const struct sockaddr_802 *dli_802 = &addr->choose_addr.dli_802addr;
    int (*filter_add)(struct dli_ifnet *, const struct sockaddr_802 *, int, struct dli_filter **);

    if (addr->dli_substructype == DLI_ETHERNET) {
	return dli_filter_add_ether(dlif, &addr->choose_addr.dli_eaddr, 0, filter_p);
    }

    if (dli_802->eh_802.ssap == SNAP_SAP) {
	filter_add = dli_filter_add_snapsap;
    } else {
	filter_add = dli_filter_add_sap;
    }

    return (*filter_add)(dlif, dli_802, 0, filter_p);
}

int
dli_filter_add_ether(
    struct dli_ifnet *dlif,
    const struct sockaddr_edl *edl,
    int flags,
    struct dli_filter **filter_p)
{
    struct dli_filter *filter;
    int error;

    if (edl->dli_protype <= ETHERMTU)
	return EINVAL;
    /* 
     * Protocol types hardwired in <net/if_ethersubr.c>.
     */
    if (dli_ifp2arpcom(dlif->dlif_ifp)->ac_ipaddr.s_addr != 0 &&
	    (edl->dli_protype == ETHERTYPE_IP
	     || edl->dli_protype == ETHERTYPE_ARP
	     || (edl->dli_protype >= ETHERTYPE_TRAIL
		 && edl->dli_protype < ETHERTYPE_TRAIL + ETHERTYPE_NTRAILER)))
        return EACCES;

    if (error = dli_filter_create(dlif, &filter, edl->dli_ioctlflg, edl->dli_target))
	return error;
    filter->fltr_flags |= DLI_FILTER_ETYPE | flags;
    if (edl->dli_options & DLI_ETHERPAD)
        filter->fltr_flags |= DLI_FILTER_ETHERPAD;
    if (dlif->dlif_ifp->if_flags & IFF_SNAP) {
        filter->fltr_flags |= DLI_FILTER_SNAPSAP;
	filter->fltr_llc.llc_dsap = SNAP_SAP;
	filter->fltr_llc.llc_ssap = SNAP_SAP;
	filter->fltr_llc.llc_control = UI_NPCMD;
	DLI_FILTER_SET_LLCMASK_CMP_ALL(filter);
    } else {
	filter->fltr_mask.llc_snap.ether_type = ~0;
    }
    filter->fltr_llc.llc_snap.ether_type = ntohs(edl->dli_protype);

    error = dli_filter_insert(dlif, filter);
    if (error) {
	kfree(filter, sizeof(*filter));
	filter = NULL;
    } else {
	dlif->dlif_filters_active++;
    }
    *filter_p = filter;
    return error;
}

int
dli_filter_add_sap(
    struct dli_ifnet *dlif,
    const struct sockaddr_802 *addr,
    int flags,
    struct dli_filter **filter_p)
{
    struct dli_filter *filter;
    int error;

    if (addr->eh_802.ssap == NULL_SAP || LLC_ISGSAP(addr->eh_802.ssap))
	return EINVAL;

    if (addr->svc == TYPE1
	    && addr->eh_802.ctl.U_fmt != UI_NPCMD
	    && addr->eh_802.ctl.U_fmt != TEST_PCMD
	    && addr->eh_802.ctl.U_fmt != TEST_NPCMD 
	    && addr->eh_802.ctl.U_fmt != XID_PCMD
	    && addr->eh_802.ctl.U_fmt != XID_NPCMD)
	return EINVAL;

    if (error = dli_filter_create(dlif, &filter, addr->ioctl, addr->eh_802.dst))
	return error;
    filter->fltr_flags |= DLI_FILTER_ISAP | flags;
    filter->fltr_llc.llc_dsap = addr->eh_802.ssap;
    filter->fltr_mask.llc_dsap = ~0;
    if (addr->svc == TYPE1) {
	filter->fltr_flags |= DLI_FILTER_SVC_TYPE1;
	filter->fltr_llc.llc_control = LLC_U_FMT;
	filter->fltr_mask.llc_control = LLC_U_FMT;
    }

    error = dli_filter_insert(dlif, filter);
    if (error) {
	kfree(filter, sizeof(*filter));
	filter = NULL;
    } else {
	dlif->dlif_filters_active++;
    }
    *filter_p = filter;
    return error;
}

int
dli_filter_add_snapsap(
    struct dli_ifnet *dlif,
    const struct sockaddr_802 *addr,
    int flags,
    struct dli_filter **filter_p)
{
    struct dli_filter *filter;
    int error;

    if (addr->eh_802.ssap != SNAP_SAP
	    || addr->eh_802.ctl.U_fmt != UI_NPCMD
	    || addr->svc != TYPE1)
        return EINVAL;

    if (error = dli_filter_create(dlif, &filter, addr->ioctl, addr->eh_802.dst))
	return error;

    filter->fltr_flags |= DLI_FILTER_SNAPSAP | flags;
    filter->fltr_llc.llc_dsap = SNAP_SAP;
    filter->fltr_llc.llc_ssap = SNAP_SAP;
    filter->fltr_llc.llc_control = UI_NPCMD;
    filter->fltr_llc.llc_snap.org_code[0] = addr->eh_802.osi_pi[0];
    filter->fltr_llc.llc_snap.org_code[1] = addr->eh_802.osi_pi[1];
    filter->fltr_llc.llc_snap.org_code[2] = addr->eh_802.osi_pi[2];
#if BYTE_ORDER == LITTLE_ENDIAN || BYTE_ORDER == PDP_ENDIAN
    filter->fltr_llc.llc_snap.ether_type =
    			(addr->eh_802.osi_pi[4] << 8) | addr->eh_802.osi_pi[3];
#elif
    filter->fltr_llc.llc_snap.ether_type =
    			(addr->eh_802.osi_pi[3] << 8) | addr->eh_802.osi_pi[4];
#endif

    DLI_FILTER_SET_LLCMASK_CMP_ALL(filter);

    error = dli_filter_insert(dlif, filter);
    if (error) {
	kfree(filter, sizeof(*filter));
	filter = NULL;
    } else {
	dlif->dlif_filters_active++;
    }
    *filter_p = filter;
    return error;
}

int
dli_filter_insert(
    struct dli_ifnet *dlif,
    struct dli_filter *new_filter)
{
    struct dli_filter **listhead, **filter_p, **next_p;

    if ((new_filter->fltr_flags & DLI_FILTER_SPECIAL) == 0
	    && (dlif->dlif_flags & (DLIF_UP|DLIF_RUNNING)) != (DLIF_UP|DLIF_RUNNING))
	dli_dlif_start(dlif);

    DLI_DLIF_FILTERS_LOCK(dlif);

    if (new_filter->fltr_flags & DLI_FILTER_ISAP) {
	listhead = &dlif->dlif_isaps[LLC_SAPIDX(new_filter->fltr_llc.llc_dsap)];
    } else {
	unsigned hash;
	if (new_filter->fltr_flags & DLI_FILTER_SNAPSAP)
	    listhead = dlif->dlif_protoids;
	else
	    listhead = dlif->dlif_ethers;
	hash = DLI_ETYPE_MKHASH(&new_filter->fltr_llc);
	hash = DLI_ETYPE_DOHASH(hash);
	listhead += hash;
    }

    next_p = &new_filter->fltr_next;
    for (filter_p = listhead; (*filter_p) != NULL; filter_p = &(*filter_p)->fltr_next) {
	struct dli_filter *filter = (*filter_p);

	if (DLI_FILTER_EQUAL(filter, new_filter)) {
	    int first = 1, found_default = 0;
	    if (new_filter->fltr_flags & DLI_FILTER_SPECIAL) {
		if (filter->fltr_flags & DLI_FILTER_SPECIAL) {
		    DLI_DLIF_FILTERS_UNLOCK(dlif);
		    return EADDRNOTAVAIL;
		}
		new_filter->fltr_next = *filter_p;
		new_filter->fltr_sibling = (*filter_p)->fltr_sibling;
		*filter_p = new_filter;
		DLI_DLIF_FILTERS_UNLOCK(dlif);
		return 0;
	    }
	    for (first = 1; (*filter_p) != NULL; first = 0, 
		    filter_p = &(*filter_p)->fltr_sibling) {
		filter = (*filter_p);
		if (filter->fltr_flags & DLI_FILTER_SPECIAL)
		    continue;
		if ((filter->fltr_flags|new_filter->fltr_flags) & DLI_FILTER_EXCLUSIVE) {
		    DLI_DLIF_FILTERS_UNLOCK(dlif);
		    return EADDRNOTAVAIL;
		}
		if (filter->fltr_flags & DLI_FILTER_DEFAULT) {
		    found_default = 1;
		} else if (DLI_ADDR_EQUAL(&filter->fltr_src, &new_filter->fltr_src)) {
		    DLI_DLIF_FILTERS_UNLOCK(dlif);
		    return EADDRNOTAVAIL;
		}
	    }
	    if (found_default && (new_filter->fltr_flags & DLI_FILTER_DEFAULT)) {
		DLI_DLIF_FILTERS_UNLOCK(dlif);
		return EADDRNOTAVAIL;
	    }
	    if (!first)
		next_p = &new_filter->fltr_sibling;
	    break;
	}
    }

    /*
     * We still haven't haven't insert the new filter into list of filters.
     * But first, if the filter doesn't have a port and the DLIMGT stuff is
     * installed, try to open one.
     */
    if (dlimgtsw != NULL && (new_filter->fltr_flags & (DLI_FILTER_SPECIAL|DLI_FILTER_HAVEPORT)) == 0) {
	int error = (*dlimgtsw->mgt_openport)(dlif, new_filter);
	if (error) {
	    DLI_DLIF_FILTERS_UNLOCK(dlif);
	    return error;
	}
    }

    *next_p = *filter_p;
    *filter_p = new_filter;

    DLI_DLIF_FILTERS_UNLOCK(dlif);
    return 0;
}

void
dli_filter_destroy(
    struct dli_ifnet *dlif,
    struct dli_filter **old_filter_p)
{
    struct dli_filter **listhead, **filter_p;
    struct dli_filter *old_filter = *old_filter_p;

    DLI_DLIF_FILTERS_LOCK(dlif);

    old_filter->fltr_match = dli_rcv_and_drop;

    if (old_filter->fltr_flags & DLI_FILTER_ISAP) {
	listhead = &dlif->dlif_isaps[LLC_SAPIDX(old_filter->fltr_llc.llc_dsap)];
    } else {
	unsigned hash;
	if (old_filter->fltr_flags & DLI_FILTER_SNAPSAP)
	    listhead = dlif->dlif_protoids;
	else
	    listhead = dlif->dlif_ethers;
	hash = DLI_ETYPE_MKHASH(&old_filter->fltr_llc);
	hash = DLI_ETYPE_DOHASH(hash);
	listhead += hash;
    }

    for (filter_p = listhead; (*filter_p) != NULL; filter_p = &(*filter_p)->fltr_next) {
	struct dli_filter *filter = (*filter_p);

	if (DLI_FILTER_EQUAL(filter, old_filter)) {
	    int first;
	    for (first = 1; (*filter_p) != NULL; first = 0, 
		    filter_p = &(*filter_p)->fltr_sibling) {
		filter = *filter_p;
		if (filter == old_filter) {
		    if (first && filter->fltr_sibling != NULL) {
			(*filter_p) = filter->fltr_sibling;
			(*filter_p)->fltr_next = filter->fltr_next;
		    } else if (first) {
			(*filter_p) = filter->fltr_next;
		    } else {
			(*filter_p) = filter->fltr_sibling;
		    }
		    dli_addrset_destroy(dlif, old_filter, &old_filter->fltr_addrset);
		    dli_filter_disable_all_gsaps(dlif, old_filter);
		    *old_filter_p = NULL;
		    dlif->dlif_filters_active--;
		    if (old_filter->fltr_flags & DLI_FILTER_HAVEPORT)
			(*dlimgtsw->mgt_closeport)(dlif, old_filter);

		    dlif->dlif_pdus_rcvd[0] += old_filter->fltr_pdus_rcvd[0];
		    dlif->dlif_pdus_rcvd[1] += old_filter->fltr_pdus_rcvd[1];
		    dlif->dlif_pdus_sent[0] += old_filter->fltr_pdus_sent[0];
		    dlif->dlif_pdus_sent[1] += old_filter->fltr_pdus_sent[1];
		    dlif->dlif_octets_rcvd[0] += old_filter->fltr_octets_rcvd[0];
		    dlif->dlif_octets_rcvd[1] += old_filter->fltr_octets_rcvd[1];
		    dlif->dlif_octets_sent[0] += old_filter->fltr_octets_sent[0];
		    dlif->dlif_octets_sent[1] += old_filter->fltr_octets_sent[1];
		    DLI_DLIF_FILTERS_UNLOCK(dlif);
		    kfree(old_filter, sizeof(*old_filter));
		    return;
		}
	    }
	}
    }

    panic("dli_filter_destroy: didn't find filter!");
}

void
dli_filter_input(
    struct dli_recv *rcv,
    struct mbuf *m)
{
    struct dli_filter *filter;
    struct llc *llc = &rcv->rcv_llc;
    struct dli_filter *listhead;
    struct dli_ifnet *dlif = rcv->rcv_dlif;

    DLI_DLIF_FILTERS_READLOCK(dlif);

    if (rcv->rcv_flags & DLI_FILTER_ISAP) {
	if (LLC_ISGSAP(llc->llc_dsap)) {
	    dli_filter_by_gsap(rcv, m);
	    DLI_DLIF_FILTERS_UNLOCK(dlif);
	    return;
	}
	listhead = dlif->dlif_isaps[LLC_SAPIDX(llc->llc_dsap)];
    } else {
	unsigned hash = DLI_ETYPE_MKHASH(&rcv->rcv_llc);
	hash = DLI_ETYPE_DOHASH(hash);
	if (rcv->rcv_flags & DLI_FILTER_SNAPSAP)
	    listhead = dlif->dlif_protoids[hash];
	else
	    listhead = dlif->dlif_ethers[hash];
    }

    for (filter = listhead; filter != NULL; filter = filter->fltr_next) {
	if (DLI_FILTER_LLC_MATCH(filter, &rcv->rcv_llcdata)) {
	    struct dli_filter *default_filter = NULL;
	    for (; filter != NULL; filter = filter->fltr_sibling) {
		if (filter->fltr_flags & DLI_FILTER_ABNORMAL) {
		    if (filter->fltr_flags & DLI_FILTER_DEFAULT) {
			default_filter = filter;
		    } else if ((*dlif->dlif_rcv)(rcv, m, filter)) {
			DLI_DLIF_FILTERS_UNLOCK(dlif);
			return;
		    }
		} else if (DLI_ADDR_EQUAL(&filter->fltr_src, &rcv->rcv_src)) {
		    if ((*dlif->dlif_rcv)(rcv, m, filter)) {
			DLI_DLIF_FILTERS_UNLOCK(dlif);
			return;
		    }
		}
	    }
	    if (default_filter != NULL) {
		(void) (*dlif->dlif_rcv)(rcv, m, default_filter);
		DLI_DLIF_FILTERS_UNLOCK(dlif);
		return;
	    } else {
		break;	/* it's redundant but it saves a compare */
	    }
	}
    }
    DLI_DLIF_FILTERS_UNLOCK(dlif);
    dlif->dlif_unrecog_pdus_rcvd[DLI_ADDR_IS_MULTI(&rcv->rcv_dst)]++;
    dlif->dlif_octets_rcvd[DLI_ADDR_IS_MULTI(&rcv->rcv_dst)] += m->m_pkthdr.len - rcv->rcv_hdrlen;
    if (dlimgtsw != NULL)
        (*dlimgtsw->mgt_log_unrecog_pdu)(rcv, m);
    m_freem(m);
    return;
}

int
dli_filter_add_gsap(
    struct dli_ifnet *dlif,
    struct dli_filter *filter,
    u_int gsap)
{
    if (!LLC_ISGSAP(gsap) || gsap < 3 || gsap > 255)
        return EINVAL;
    if (DLI_FILTER_ISAP != (filter->fltr_flags & DLI_FILTER_CLASS))
        return ENOPROTOOPT;
    if (DLI_TSTSAP(gsap, &filter->fltr_gsaps))
        return 0;

    DLI_DLIF_FILTERS_LOCK(dlif);
    /*
     * Since the GSAP table is rather large and not frequently used,
     * it only allocated when a user enabled a GSAP.
     */
    if (dlif->dlif_gsaps == NULL) {
	dlif->dlif_gsaps = (struct dli_sapset *)
	    kalloc(sizeof(struct dli_sapset [NISAPS]));
	if (dlif->dlif_gsaps == NULL) {
	    DLI_DLIF_FILTERS_UNLOCK(dlif);
	    return ENOMEM;
	}
	bzero(dlif->dlif_gsaps, sizeof(struct dli_sapset [NISAPS]));
    }

    /*
     * Show that we have enabled the GSAP for this filter.
     */
    DLI_SETSAP(gsap, &filter->fltr_gsaps);
    /*
     * If the ISAP bit for the GSAP is already set, then this ISAP/GSAP pair
     * has already been enabled we don't count it (or set it) again.
     */
    if (DLI_TSTSAP(filter->fltr_llc.llc_dsap, &dlif->dlif_gsaps[LLC_SAPIDX(gsap)])) {
	DLI_DLIF_FILTERS_UNLOCK(dlif);
        return 0;
    }
    DLI_SETSAP(filter->fltr_llc.llc_dsap, &dlif->dlif_gsaps[LLC_SAPIDX(gsap)]);
    dlif->dlif_sap_pairs_active++;
    DLI_DLIF_FILTERS_UNLOCK(dlif);
    return 0;
}

int
dli_filter_test_gsap(
    struct dli_filter *filter,
    u_int gsap)
{
    if (!LLC_ISGSAP(gsap) || gsap < 3 || gsap > 255)
        return EINVAL;
    if (DLI_FILTER_ISAP != (filter->fltr_flags & DLI_FILTER_CLASS))
        return ENOPROTOOPT;

    return DLI_TSTSAP(gsap, &filter->fltr_gsaps);
}

int
dli_filter_remove_gsap(
    struct dli_ifnet *dlif,
    struct dli_filter *filter,
    u_int gsap)
{
    if (!LLC_ISGSAP(gsap) || gsap < 3 || gsap > 255)
        return EINVAL;
    if (DLI_FILTER_ISAP != (filter->fltr_flags & (DLI_FILTER_CLASS|DLI_FILTER_SVC_TYPE1)))
        return ENOPROTOOPT;

    DLI_DLIF_FILTERS_LOCK(dlif);
    if (dlif->dlif_gsaps == NULL || !DLI_TSTSAP(gsap, &filter->fltr_gsaps)) {
	DLI_DLIF_FILTERS_UNLOCK(dlif);
        return EADDRNOTAVAIL;
    }

    dli_filter_disable_gsap(dlif, filter, gsap);
    DLI_DLIF_FILTERS_UNLOCK(dlif);
    return 0;
}

void
dli_filter_disable_gsap(
    struct dli_ifnet *dlif,
    struct dli_filter *filter,
    u_int gsap)
{
    struct dli_filter *sibling;
    /*
     * Mark that the GSAP is no longer enabled for this filter.
     */
    DLI_CLRSAP(gsap, &filter->fltr_gsaps);
    /*
     * Now we need to check to if any othe sibling filters on this ISAP
     * have this GSAP enabled.  If so, we can just return.
     */
    sibling = dlif->dlif_isaps[LLC_SAPIDX(filter->fltr_llc.llc_dsap)];
    for (; sibling != NULL; sibling = sibling->fltr_sibling) {
	if (DLI_TSTSAP(gsap, &sibling->fltr_gsaps))
	    return;
    }
    /*
     * Otherwise we clear ISAP/GSAP bit for this ISAP/GSAP pait.  If this
     * was the last ISAP/GSAP pair active, we deallocate the GSAP array.
     */
    DLI_CLRSAP(filter->fltr_llc.llc_dsap, &dlif->dlif_gsaps[LLC_SAPIDX(gsap)]);
    if (--dlif->dlif_sap_pairs_active == 0) {
	kfree(dlif->dlif_gsaps, sizeof(struct dli_sapset [NISAPS]));
	dlif->dlif_gsaps = NULL;
    }
    return;
}

void
dli_filter_disable_all_gsaps(
    struct dli_ifnet *dlif,
    struct dli_filter *filter)
{
    struct dli_filter *sibling;
    u_int gsap;

    if (DLI_FILTER_ISAP != (filter->fltr_flags & (DLI_FILTER_CLASS|DLI_FILTER_SVC_TYPE1)))
        return;

    if (dlif->dlif_gsaps == NULL)
	return;

    for (gsap = 3; gsap < 256 && dlif->dlif_sap_pairs_active > 0; gsap += 2) {
	if (DLI_TSTSAP(gsap, &filter->fltr_gsaps))
	    dli_filter_disable_gsap(dlif, filter, gsap);
    }
}

void
dli_filter_by_gsap(
    struct dli_recv *rcv,
    struct mbuf *m)
{
    struct dli_ifnet *dlif = rcv->rcv_dlif;
    u_int gsap = rcv->rcv_llc.llc_dsap;
    struct dli_sapset *isaps;
    struct dli_filter *last_filter = NULL;
    u_int isap;

    if (dlif->dlif_gsaps == NULL) {
	m_freem(m);
	return;
    }

    isaps = &dlif->dlif_gsaps[LLC_SAPIDX(gsap)];
    for (isap = 2; isap < 256; isap += 2) {
	if (DLI_TSTSAP(isap, isaps)) {
	    register struct dli_filter *filter;
	    for (filter = dlif->dlif_isaps[LLC_SAPIDX(isap)]; filter != NULL;
		   filter = filter->fltr_sibling) {
		if (DLI_TSTSAP(gsap, &filter->fltr_gsaps)
		        && ((filter->fltr_flags & DLI_FILTER_ABNORMAL)
			    || DLI_ADDR_EQUAL(&filter->fltr_src, &rcv->rcv_src))) {
		    if (last_filter != NULL) {
			struct mbuf *m0 = m_copym(m, 0, M_COPYALL, M_DONTWAIT);
			if (m0 != NULL) {
			    (void) (*last_filter->fltr_match)(rcv, m0, last_filter);
			}
		    }
		    last_filter = filter;
		}
	    }
	}
    }

    if (last_filter != NULL) {
	(void) (*last_filter->fltr_match)(rcv, m, last_filter);
    } else {
	m_freem(m);
    }
}
