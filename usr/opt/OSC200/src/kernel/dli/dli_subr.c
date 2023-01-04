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
static char *sccsid = "@(#)$RCSfile: dli_subr.c,v $ $Revision: 4.4.7.3 $ (DEC) $Date: 1993/08/27 17:03:56 $";
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
 * 1.00 22-May-1986
 *      DECnet-ULTRIX   V2.0
 *
 * 2.01 18-Mar-1988
 *      DECnet-ULTRIX   V2.4
 *		- Allowed use of reserved bit in individual and group SAPs
 *
 *      DECtrade-ULTRIX V1.0
 *		- Added mcast buffer array size (in MCAST_SIZE units) in
 *		  match_mcast routine. Comparison is done based on this
 *		  size and *NOT* on MCAST_ASIZE ( as was done previously )
 */

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <kern/assert.h>
#include <net/if.h>
#include <dli/dli_var.h>

/*
 * process the control field on incoming packets for class 1 service only
 */
int
dli_llc_responder(
    struct dli_recv *rcv,
    struct mbuf *m,
    struct dli_filter *filter)
{
    u_int ctl = rcv->rcv_llc.llc_control & ~LLC_PF_FLAG;
    struct llc *llc;

    if (ctl != XID_NPCMD && ctl != TEST_NPCMD) {
	m_freem(m);
	return TRUE;
    }
    if ((rcv->rcv_llc.llc_ssap & 1) == 1)
	return FALSE;

    m_adj(m, rcv->rcv_hdrlen);

    if (m->m_len == 0) {
	llc = mtod(m->m_next, struct llc *);
    } else {
	llc = mtod(m, struct llc *);
    }
    llc->llc_ssap = filter->fltr_llc.llc_dsap | 1;
    llc->llc_dsap = rcv->rcv_llc.llc_ssap;

    if (ctl == XID_NPCMD) {
	if (m->m_pkthdr.len != 6) {
	    m_freem(m);
	    return TRUE;
	}
	llc->llc_fid = 129;
	llc->llc_class = 1;
	llc->llc_window = 0;
    }

    dli_output(rcv->rcv_dlif, filter, m, &rcv->rcv_src);
    return TRUE;
}

/*
 *		d l i _ m c a s t _ h a s h
 *
 * This routine looks at the [multicast] address and returns its
 * hash value;
 *
 * Outputs:		integer from 0 to DLI_ADDR_HASHSIZE-1.
 *
 * Inputs:		addr	= pointer to address
 */

u_int
dli_addr_hash(
    const union dli_addr *addr)
{
    u_int hash, xadr;

    ASSERT(DLI_ADDRHASH_MAXBUCKETS == 64);

#if BYTE_ORDER == LITTLE_ENDIAN || BYTE_ORDER == PDP_ENDIAN
    xadr = (addr->un_addr[2] << 8) | addr->un_addr[1];
    hash = addr->un_w_addr[0] ^ xadr ^ addr->un_w_addr[2];
    return ((hash >> 11) ^ (hash >> 6) ^ hash) & 63;
#elif BYTE_ORDER == BIG_ENDIAN
    xadr = (addr->un_addr[1] << 8) | addr->un_addr[0];
    hash = addr->un_w_addr[0] ^ xadr ^ addr->un_w_addr[2];
    return ((hash << 11) ^ (hash << 6) ^ hash) >> 10;
#endif
}

/*
 *		d l i _ m a t c h _ l o o k u p
 *
 * This routine looks at the destination address, and if it is
 * multicast, it checks to see if the address matches the user's
 * multicast addresses.  Note that if the user hasn't specified
 * any, the test fails.
 *
 * Outputs:		NULL if no match, otherwise 1.
 *
 * Inputs:		mcast1 = string containing Ethernet dest address.
 * 			mcast2 = string containing user's multicast address(es)
 *			num    = size of mca buffer in MCAST_SIZE units.
 */

struct dli_mcast *
dli_addr_lookup(
    struct dli_ifnet *dlif,
    const struct dli_filter *filter,
    const union dli_addr *addr,
    const struct dli_fagamap *map)
{
    u_int idx;

    idx = dlif->dlif_addr_buckets[dli_addr_hash(addr)];
    while (idx != DLI_ADDRHASH_ENDOFLIST) {
	struct dli_mcast *mcast = &dlif->dlif_addrlist[idx];
	if (DLI_ADDR_EQUAL(addr, &mcast->dlm_addr)
		&& (map == NULL || mcast->dlm_map_idx == map->map_idx)) {
	    return mcast;
	}
	idx = mcast->dlm_next_index;
    }
    return NULL;
}

void
dli_addrset_init(
    struct dli_ifnet *dlif,
    const struct dli_filter *filter,
    struct dli_addrset *addrset)
{
    bzero(addrset, sizeof(*addrset));
}

int
dli_addrset_add(
    struct dli_ifnet *dlif,
    const struct dli_filter *filter,
    struct dli_addrset *addrset,
    const union dli_addr *addr)
{
    const struct dli_fagamap *map;
    struct dli_mcast *mcast;
    struct ifreq ifreq;
    u_int idx, *hash_p;
    int error;

    map = dli_addr_remap(dlif, filter, addr, TRUE);
    mcast = dli_addr_lookup(dlif, filter, addr, map);
    if (mcast != NULL) {
	mcast->dlm_references++;
	DLI_SETADDR_IDX(mcast->dlm_index, addrset);
	return 0;
    }

    ifreq.ifr_addr.sa_family = AF_UNSPEC;
    if (map) {
	*(W_T *) &ifreq.ifr_addr.sa_data[0] = map->map_addrs[DLI_FAGAMAP_FUNCADDR].un_w_addr[0];
	*(W_T *) &ifreq.ifr_addr.sa_data[2] = map->map_addrs[DLI_FAGAMAP_FUNCADDR].un_w_addr[1];
	*(W_T *) &ifreq.ifr_addr.sa_data[4] = map->map_addrs[DLI_FAGAMAP_FUNCADDR].un_w_addr[2];
    } else {
	*(W_T *) &ifreq.ifr_addr.sa_data[0] = addr->un_w_addr[0];
	*(W_T *) &ifreq.ifr_addr.sa_data[2] = addr->un_w_addr[1];
	*(W_T *) &ifreq.ifr_addr.sa_data[4] = addr->un_w_addr[2];
    }

    if (dlif->dlif_addrlist_active == dlif->dlif_addrlist_size) {
	int new_addrlist_size = dlif->dlif_addrlist_size + DLI_ADDRLIST_INC;
	struct dli_mcast *new_addrlist;

	if (dlif->dlif_addrlist_size == DLI_ADDRLIST_MAXSIZE)
	    return ENOSPC;

	new_addrlist = (struct dli_mcast *) kalloc(new_addrlist_size * sizeof(*mcast));
	if (new_addrlist == NULL)
	    return ENOSPC;

	bzero(&new_addrlist[dlif->dlif_addrlist_size],
	      DLI_ADDRLIST_INC * sizeof(*mcast));
	if (dlif->dlif_addrlist != NULL) {
	    bcopy(dlif->dlif_addrlist, new_addrlist,
		  dlif->dlif_addrlist_size * sizeof(*mcast));
	    kfree(dlif->dlif_addrlist, dlif->dlif_addrlist_size * sizeof(*mcast));
	}
	dlif->dlif_addrlist = new_addrlist;

	idx = dlif->dlif_addrlist_size;
	mcast = &dlif->dlif_addrlist[idx];
	while (dlif->dlif_addrlist_size < new_addrlist_size) {
	    mcast->dlm_next_index = DLI_ADDRHASH_ENDOFLIST;
	    dlif->dlif_addrlist_size++, mcast++;
	}
    } else {
	for (idx = dlif->dlif_addrlist_size - 1; idx >= 0; idx--) {
	    if (!DLI_TSTADDR_IDX(idx, &dlif->dlif_addrset))
		break;
	}
    }

    if (DLI_ADDR_IS_MULTI(addr)) {
	if ((error = dli_addmulti(dlif, &ifreq)) != 0)
	    return error;
    } else {
	return EINVAL;
#ifdef notyet
	if (!DLI_ADDR_EQUAL(&dlif->dlif_default_addr, addr)) {
	    if ((error = dli_setphysaddr(dlif, &ifreq)) != 0)
		return error;
	}
#endif
    }

    mcast = &dlif->dlif_addrlist[idx];
    mcast->dlm_addr = *addr;
    mcast->dlm_map_idx = map ? map->map_idx : DLI_GA_ENDOFLIST;
    mcast->dlm_index = idx;
    mcast->dlm_references++;

    hash_p = &dlif->dlif_addr_buckets[dli_addr_hash(addr)];
    mcast->dlm_next_index = (*hash_p);
    (*hash_p) = mcast->dlm_index;

    if (addrset != &dlif->dlif_addrset)
	DLI_SETADDR_IDX(idx, addrset);

    /*
     * If someone enables the MOP multicast address (AB-00-00-02-00-00),
     * then we need to allow the MOP RQSYSID/RQCNTR responders to receive
     * packets to that multicast.
     */
    if (map != NULL) {
	if (map->map_idx == DLI_GA_MOPRC_V3_SYSIDS)
	    DLI_SETADDR_IDX(idx, &dlif->dlif_mopv3_filter->fltr_addrset);
	else if (map->map_idx == DLI_GA_MOPRC_V4_SYSIDS)
	    DLI_SETADDR_IDX(idx, &dlif->dlif_mopv4_filter->fltr_addrset);
    } else if (DLI_ADDR_EQUAL(addr, &dli_sysid_mcast)) {
	DLI_SETADDR_IDX(idx, &dlif->dlif_mopv3_filter->fltr_addrset);
	DLI_SETADDR_IDX(idx, &dlif->dlif_mopv4_filter->fltr_addrset);
    }

    DLI_SETADDR_IDX(idx, &dlif->dlif_addrset);
    dlif->dlif_addrlist_active++;
    return 0;
}

/*
 *		m a t c h _ m c a s t
 *
 * This routine test to see if the supplied [multicast] address is already
 * enabled on the port.
 *
 * Outputs:		TRUE on successful match, FALSE is not enabled
 *
 * Inputs:		pcb	= pointer to socket's user pcb
 * 			addr	= pointer to address to be tested
 */

int
dli_addrset_test(
    struct dli_ifnet *dlif,
    const struct dli_filter *filter,
    struct dli_addrset *addrset,
    const union dli_addr *addr)
{
    const struct dli_fagamap *map;
    struct dli_mcast *mcast;

    map = dli_addr_remap(dlif, filter, addr, TRUE);
    mcast = dli_addr_lookup(dlif, filter, addr, map);
    if (mcast == NULL)
	return FALSE;

    return DLI_TSTADDR_IDX(mcast->dlm_index, addrset);
}

void
dli_addrset_remove_int(
    struct dli_ifnet *dlif,
    struct dli_addrset *addrset,
    struct dli_mcast *mcast,
    const struct dli_fagamap *map)
{
    DLI_CLRADDR_IDX(mcast->dlm_index, addrset);
    if (--mcast->dlm_references == 0) {
	struct ifreq ifreq;
	u_int *hash_p;

	ifreq.ifr_addr.sa_family = AF_UNSPEC;
	if (map) {
	    *(W_T *) &ifreq.ifr_addr.sa_data[0] = map->map_addrs[DLI_FAGAMAP_FUNCADDR].un_w_addr[0];
	    *(W_T *) &ifreq.ifr_addr.sa_data[2] = map->map_addrs[DLI_FAGAMAP_FUNCADDR].un_w_addr[1];
	    *(W_T *) &ifreq.ifr_addr.sa_data[4] = map->map_addrs[DLI_FAGAMAP_FUNCADDR].un_w_addr[2];
	} else {
	    *(W_T *) &ifreq.ifr_addr.sa_data[0] = mcast->dlm_addr.un_w_addr[0];
	    *(W_T *) &ifreq.ifr_addr.sa_data[2] = mcast->dlm_addr.un_w_addr[1];
	    *(W_T *) &ifreq.ifr_addr.sa_data[4] = mcast->dlm_addr.un_w_addr[2];
	}

	if (DLI_ADDR_IS_MULTI(&mcast->dlm_addr)) {
	    (void) dli_delmulti(dlif, &ifreq);
	} else {
#if 0
	    (void) dli_delphysaddr(dlif, &ifreq);
	    addrset->as_haddr_p = &dlif->dlif_addrset->as_haddr;
	    return;
#endif
	}

	hash_p = &dlif->dlif_addr_buckets[dli_addr_hash(&mcast->dlm_addr)];
	while ((*hash_p) != DLI_ADDRHASH_ENDOFLIST) {
	    struct dli_mcast *mcast2 = &dlif->dlif_addrlist[*hash_p];
	    if (mcast2 == mcast) {
		(*hash_p) = mcast->dlm_next_index;
		break;
	    }
	    hash_p = &mcast2->dlm_next_index;
	}

	/*
	 * If someone had enabled the MOP multicast address (AB-00-00-02-00-00),
	 * then we allowed the MOP RQSYSID/RQCNTR responders to receive packets
	 * to that multicast.  Now we get to back that out.
	 */
	if (map != NULL) {
	    if (map->map_idx == DLI_GA_MOPRC_V3_SYSIDS)
		DLI_CLRADDR_IDX(mcast->dlm_index, &dlif->dlif_mopv3_filter->fltr_addrset);
	    else if (map->map_idx == DLI_GA_MOPRC_V4_SYSIDS)
		DLI_CLRADDR_IDX(mcast->dlm_index, &dlif->dlif_mopv4_filter->fltr_addrset);
	} else if (DLI_ADDR_EQUAL(&mcast->dlm_addr, &dli_sysid_mcast)) {
	    DLI_CLRADDR_IDX(mcast->dlm_index, &dlif->dlif_mopv3_filter->fltr_addrset);
	    DLI_CLRADDR_IDX(mcast->dlm_index, &dlif->dlif_mopv4_filter->fltr_addrset);
	}

	DLI_CLRADDR_IDX(mcast->dlm_index, &dlif->dlif_addrset);
	dlif->dlif_addrlist_active--;
    }
}

int
dli_addrset_remove(
    struct dli_ifnet *dlif,
    const struct dli_filter *filter,
    struct dli_addrset *addrset,
    const union dli_addr *addr)
{
    const struct dli_fagamap *map;
    struct dli_mcast *mcast;

    map = dli_addr_remap(dlif, filter, addr, TRUE);
    mcast = dli_addr_lookup(dlif, filter, addr, map);
    if (mcast == NULL || !DLI_TSTADDR_IDX(mcast->dlm_index, addrset))
        return EADDRNOTAVAIL;

#ifdef notyet
    if (mcast->dlm_index == dlif->dlif_addrset.as_haddr)
        return EADDRNOTAVAIL;
#endif

    dli_addrset_remove_int(dlif, addrset, mcast, map);
    return 0;
}

void
dli_addrset_destroy(
    struct dli_ifnet *dlif,
    const struct dli_filter *filter,
    struct dli_addrset *addrset)
{
    int idx;
    for (idx = dlif->dlif_addrlist_size - 1; idx >= 0; idx--) {
	if (DLI_TSTADDR_IDX(idx, addrset)) {
	    dli_addrset_remove(dlif, filter, addrset, &dlif->dlif_addrlist[idx].dlm_addr);
	}
    }
}
/*
 *		d l i _ d e v i c e 2 d l i f
 *
 * Find the dli_ifnet interface for the selected device.
 *
 * Returns:		= Pointer to dli_ifnet structure if found,
 *			  otherwise, NULL.
 *
 * Inputs:
 *	devid		= Pointer to the structure containing the device id.
 */
struct dli_ifnet *
dli_devid2dlif (
    struct dli_devid *devid)
{
    struct dli_ifnet *dlif = dli_dlifs;

    /* 
     * validate device name.
     */
    if (strlen(devid->dli_devname) >= DLI_DEVSIZE)
	return 0;

    /* 
     * search ifnet linked list for device.
     */
    for (dlif = dli_dlifs; dlif; dlif = dlif->dlif_next) {
	if (strcmp(dlif->dlif_ifp->if_name, devid->dli_devname) == 0
		&& dlif->dlif_ifp->if_unit == devid->dli_devnumber) {
	    return dlif;
	}
    }
    return NULL;
}

#ifndef dli_ifp2dlif
struct dli_ifnet *
dli_ifp2dlif(
    struct ifnet *ifp)
{
    struct dli_ifnet *dlif;

    for (dlif = dli_dlifs; dlif != NULL; dlif = dlif->dlif_next) {
	if (dlif->dlif_ifp == ifp)
	    break;
    }
    return dlif;
}
#endif

struct mbuf *
dli_trim_etherpad(
    struct mbuf *m)
{
    int padlen;
    if (m->m_pkthdr.len < 2) {
	m_freem(m);
        return NULL;
    }
    padlen = LE_EXT16(mtod(m, u_char *));
    m_adj(m, 2);
    if (padlen > m->m_pkthdr.len) {
	m_freem(m);
	return NULL;
    }
    if (padlen < m->m_pkthdr.len) {
	/*
	 * Trim off the excess data
	 */
	m_adj(m, padlen - m->m_pkthdr.len);
    }
    return m;
}

const struct dli_fagamap *
dli_addr_remap(
    struct dli_ifnet *dlif,
    const struct dli_filter *filter,
    const union dli_addr *addr,
    int sending)
{
    const struct dli_fagamap *map;
    int from, to;

    if (dlif->dlif_fagamap->map_idx == DLI_GA_ENDOFLIST)
	return NULL;

    if (sending) {
	from = DLI_FAGAMAP_GROUPADDR;
	to = DLI_FAGAMAP_FUNCADDR;
    } else {
	from = DLI_FAGAMAP_FUNCADDR;
	to = DLI_FAGAMAP_GROUPADDR;
    }

    for (map = dlif->dlif_fagamap; map->map_idx != DLI_GA_ENDOFLIST; map++) {
	if (DLI_ADDR_EQUAL(addr, &map->map_addrs[from])
		&& (map->map_flags & filter->fltr_flags)
	        && DLI_FILTER_LLC_MATCH(filter, &map->map_llc)) {
	    return map;
	}
    }
    return NULL;
}

int
dli_rcv_and_drop(
    struct dli_recv *rcv,
    struct mbuf *m,
    struct dli_filter *filter)
{
    m_freem(m);
    return 1;
}
