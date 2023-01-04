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
static char *sccsid = "@(#)$RCSfile: dli_init.c,v $ $Revision: 4.4.6.6 $ (DEC) $Date: 1993/08/31 20:01:54 $";
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
 */

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/domain.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/netisr.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <dli/dli_var.h>

/* general dli data */
struct ifqueue dli_intrq;			/* input packet queue */

/* MOP loopback data */
const union dli_addr dli_loopback_mcast = { LBACKMCAST };	/* loopback protocol mc addr */

const struct sockaddr_edl dli_loopback_ethernet = {
    DLI_NORMAL, 0, ETHERTYPE_LBACK, NO_ADDR, NO_ADDR
};

#define	dli_loopback_protoid	dli_loopback_snapsap.eh_802.osi_pi

const struct sockaddr_802 dli_loopback_snapsap = {
    DLI_NORMAL, TYPE1,
    { NO_ADDR, NO_ADDR, 0, SNAP_SAP, SNAP_SAP, { UI_NPCMD }, PROTOID_LBACK, 0}
};

/* MOP sysid data */
const union dli_addr dli_sysid_mcast = { SYSIDMCAST };	/* mcast addr for sysid */

const struct sockaddr_edl dli_moprc_ethernet = {
    DLI_NORMAL, DLI_ETHERPAD, ETHERTYPE_MOPRC, NO_ADDR, NO_ADDR
};

#define	dli_moprc_protoid	dli_moprc_snapsap.eh_802.osi_pi

const struct sockaddr_802 dli_moprc_snapsap = {
    DLI_NORMAL, TYPE1,
    { NO_ADDR, NO_ADDR, 0, SNAP_SAP, SNAP_SAP, { UI_NPCMD }, PROTOID_MOPRC, 0}
};

const struct dli_fagamap dli_faga_emptymap[] = {
    { DLI_GA_ENDOFLIST },
};

const struct dli_fagamap dli_faga_defaultmap[] = {
    { DLI_GA_DECNET_PRIME_ROUTERS, DLI_FILTER_ETYPE,	{ DLI_ETHER_SNAPSAP( ETHERTYPE_DECnet) },
	  { { 0x09, 0x00, 0x2b, 0x02, 0x01, 0x0a },	{ 0x03, 0x00, 0x08, 0x00, 0x00, 0x00 } },
      },
    { DLI_GA_DECNET_ALL_ROUTERS, DLI_FILTER_ETYPE,	{ DLI_ETHER_SNAPSAP( ETHERTYPE_DECnet) },
	  { { 0xab, 0x00, 0x00, 0x03, 0x00, 0x00 },	{ 0x03, 0x00, 0x08, 0x00, 0x00, 0x00 } },
      },
    { DLI_GA_DECNET_ALL_UNKDEST, DLI_FILTER_ETYPE,	{ DLI_ETHER_SNAPSAP( ETHERTYPE_DECnet) },
	  { { 0x09, 0x00, 0x2b, 0x02, 0x01, 0x0b },	{ 0x03, 0x00, 0x10, 0x00, 0x00, 0x00 } },
      },
    { DLI_GA_DECNET_ALL_ENDNODES, DLI_FILTER_ETYPE,	{ DLI_ETHER_SNAPSAP( ETHERTYPE_DECnet) },
	  { { 0xab, 0x00, 0x00, 0x04, 0x00, 0x00 },	{ 0x03, 0x00, 0x10, 0x00, 0x00, 0x00 } },
      },
    { DLI_GA_ESIS_ALL_ES, DLI_FILTER_ISAP,		{ LLC_ISO_LSAP, LLC_ISO_LSAP, LLC_UI },
	  { { 0x09, 0x00, 0x2b, 0x00, 0x00, 0x04 },	{ 0x03, 0x00, 0x00, 0x00, 0x02, 0x00 } },
      },
    { DLI_GA_ESIS_ALL_IS, DLI_FILTER_ISAP,		{ LLC_ISO_LSAP, LLC_ISO_LSAP, LLC_UI },
	  { { 0x09, 0x00, 0x2b, 0x00, 0x00, 0x05 },	{ 0x03, 0x00, 0x00, 0x00, 0x01, 0x00 } },
      },
    { DLI_GA_LBACK_V3_ASSIST, DLI_FILTER_ETYPE,		{ DLI_ETHER_SNAPSAP( ETHERTYPE_LBACK) },
	  { { LBACKMCAST },				{ 0x03, 0x00, 0x00, 0x08, 0x00, 0x00 } },
      },
    { DLI_GA_LBACK_V4_ASSIST, DLI_FILTER_SNAPSAP,	{ DLI_DEC_SNAPSAP(   ETHERTYPE_LBACK) },
	  { { LBACKMCAST },				{ 0x03, 0x00, 0x00, 0x08, 0x00, 0x00 } },
      },
    { DLI_GA_MOPRC_V3_SYSIDS, DLI_FILTER_ETYPE,		{ DLI_ETHER_SNAPSAP( ETHERTYPE_MOPRC) },
	  { { SYSIDMCAST },				{ 0x03, 0x00, 0x04, 0x00, 0x00, 0x00 } },
      },
    { DLI_GA_MOPRC_V4_SYSIDS, DLI_FILTER_SNAPSAP,	{ DLI_DEC_SNAPSAP(   ETHERTYPE_MOPRC) },
	  { { SYSIDMCAST },				{ 0x03, 0x00, 0x04, 0x00, 0x00, 0x00 } },
      },
    { DLI_GA_MOPDL_V3_ASSIST, DLI_FILTER_ETYPE,		{ DLI_ETHER_SNAPSAP( ETHERTYPE_MOPDL) },
	  { { 0xab, 0x00, 0x00, 0x01, 0x00, 0x00 },	{ 0x03, 0x00, 0x02, 0x00, 0x00, 0x00 } },
      },
    { DLI_GA_MOPDL_V4_ASSIST, DLI_FILTER_SNAPSAP,	{ DLI_DEC_SNAPSAP(   ETHERTYPE_MOPDL) },
	  { { 0xab, 0x00, 0x00, 0x01, 0x00, 0x00 },	{ 0x03, 0x00, 0x02, 0x00, 0x00, 0x00 } },
      },
    { DLI_GA_DECNET_ALL_L2ROUTERS, DLI_FILTER_ETYPE,	{ DLI_ETHER_SNAPSAP( ETHERTYPE_DECnet) },
	  { { 0x09, 0x00, 0x2b, 0x02, 0x00, 0x00 },	{ 0x03, 0x00, 0x08, 0x00, 0x00, 0x00 } },
      },
    { DLI_GA_LAST_GROUP0, DLI_FILTER_SNAPSAP,		{ DLI_DEC_SNAPSAP(   ETHERTYPE_LAST) },
	  { { 0x09, 0x00, 0x2b, 0x04, 0x00, 0x00 },	{ 0x03, 0x00, 0x00, 0x04, 0x00, 0x00 } },
      },
    { DLI_GA_DECDNS_ADVERS, DLI_FILTER_SNAPSAP,		{ DLI_DEC_SNAPSAP(   ETHERTYPE_DECDNS) },
	  { { 0x09, 0x00, 0x2b, 0x02, 0x01, 0x00 },	{ 0x03, 0x00, 0x00, 0x00, 0x08, 0x00 } },
      },
    { DLI_GA_DECDNS_SOLICITS, DLI_FILTER_SNAPSAP,	{ DLI_DEC_SNAPSAP(   ETHERTYPE_DECDNS) },
	  { { 0x09, 0x00, 0x2b, 0x02, 0x01, 0x01 },	{ 0x03, 0x00, 0x00, 0x00, 0x10, 0x00 } },
      },
    { DLI_GA_DTSS_SOLICITS, DLI_FILTER_SNAPSAP,		{ DLI_DEC_SNAPSAP(   ETHERTYPE_DTSS) },
	  { { 0x09, 0x00, 0x2b, 0x02, 0x01, 0x02 },	{ 0x03, 0x00, 0x00, 0x00, 0x20, 0x00 } },
      },
    { DLI_GA_PCSA_NETBIOS, DLI_FILTER_ETYPE,		{ DLI_ETHER_SNAPSAP( ETHERTYPE_NETBIOS) },
	  { { 0x09, 0x00, 0x2b, 0x00, 0x00, 0x07 },	{ 0x03, 0x00, 0x02, 0x00, 0x00, 0x00 } },
      },
    { DLI_GA_LAT_SERVERS, DLI_FILTER_ETYPE,		{ DLI_ETHER_SNAPSAP( ETHERTYPE_LAT) },
	  { { 0x09, 0x00, 0x2b, 0x00, 0x00, 0x0f },	{ 0x03, 0x00, 0x40, 0x00, 0x00, 0x00 } },
      },
    { DLI_GA_LAT_SLAVES, DLI_FILTER_ETYPE,		{ DLI_ETHER_SNAPSAP( ETHERTYPE_LAT) },
	  { { 0x09, 0x00, 0x2b, 0x02, 0x01, 0x04 },	{ 0x03, 0x00, 0x80, 0x00, 0x00, 0x00 } },
      },
    { DLI_GA_LAT_SC3_SOLICITS, DLI_FILTER_ETYPE,	{ DLI_ETHER_SNAPSAP( ETHERTYPE_LAT) },
	  { { 0x09, 0x00, 0x2b, 0x02, 0x01, 0x07 },	{ 0x03, 0x00, 0x00, 0x02, 0x00, 0x00 } },
      },
    { DLI_GA_8021E_LOAD_SERVERS, DLI_FILTER_ISAP,	{ 0x02, 0x02, LLC_UI },
	  { { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x11 },	{ 0x03, 0x00, 0x00, 0x00, 0x40, 0x00 } },
      },
    { DLI_GA_8021E_LOAD_DEVICES, DLI_FILTER_ISAP,	{ 0x02, 0x02, LLC_UI },
	  { { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x12 },	{ 0x03, 0x00, 0x00, 0x00, 0x20, 0x00 } },
      },
    { DLI_GA_8021E_MGMT_STATIONS, DLI_FILTER_ISAP,	{ 0x02, 0x02, LLC_UI },
	  { { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x18 },	{ 0x03, 0x00, 0x00, 0x00, 0x40, 0x00 } },
      },
    { DLI_GA_8021E_AGENT_STATIONS, DLI_FILTER_ISAP,	{ 0x02, 0x02, LLC_UI },
	  { { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x1A },	{ 0x03, 0x00, 0x00, 0x40, 0x00, 0x00 } },
      },

    { DLI_GA_ENDOFLIST },
};

/*
 *		d l i _ i n i t
 *
 * This routine is called during the system boot sequence to initialise the
 * DLI protocol module.
 *
 * Outputs:		None.
 *
 * Inputs:		None.  
 *
 * Version History:
 */
void
dli_init(
    void)
{
    register struct ifnet *ifp;

#define	alignment(o, u)		((o) & ~(sizeof(u)-1))
#define	alignmentof(s, m, u)	(alignment(offsetof(s, m), u))

    ASSERT(alignmentof(struct sockaddr_dl,
		       choose_addr.dli_802addr.eh_802.dst[0],
		       LW_T) == 2);
    ASSERT(alignmentof(struct sockaddr_dl,
		       choose_addr.dli_eaddr.dli_target[0],
		       LW_T) == 0);
    ASSERT(alignment(offsetof(struct sockaddr, sa_data[0]) +
		     offsetof(struct ether_header, ether_dhost[0]),
		     LW_T) == 0);

    IFQ_LOCKINIT(&dli_intrq);
    dli_intrq.ifq_maxlen = dli_ifq_maxlen;

    for (ifp = ifnet; ifp != NULL; ifp = ifp->if_next) {
	if (ifp->if_output == ether_output) {
	    struct dli_filter *filter;
	    struct dli_ifnet *dlif;
	    struct ifdevea devea;
	    u_int idx;

	    dlif = (struct dli_ifnet *) kalloc(sizeof(*dlif));
	    if (dlif == NULL)
		continue;

	    bzero(dlif, sizeof(*dlif));

	    DLI_DLIF_LOCKINIT(&dlif);
	    DLI_DLIF_FILTERS_LOCKINIT(dlif);
	    dlif->dlif_ifp = ifp;
	    dlif->dlif_ifa.ifa_addr = &dlif->dlif_ifa_addr;
	    dlif->dlif_ifa_addr.sa_family = AF_DLI;
	    dlif->dlif_ifa_addr.sa_len = sizeof(dlif->dlif_ifa_addr);
	    dli_addrset_init(dlif, NULL, &dlif->dlif_addrset);
	    for (idx = 0; idx < DLI_ADDRHASH_MAXBUCKETS; idx++)
		dlif->dlif_addr_buckets[idx] = DLI_ADDRHASH_ENDOFLIST;

	    dlif->dlif_rcv = dli_rcv_addr_filter;
	    dlif->dlif_fagamap = dli_faga_emptymap;

	    dlif->dlif_mopv3_functions = FNC_LOOP | FNC_CTRS;
	    dlif->dlif_mopv4_functions = FNC_LOOP | FNC_CTRS;

	    if (ifp->if_type == IFT_FDDI)
		dlif->dlif_mopv3_functions &= ~FNC_CTRS;

	    if (ifp->if_type == IFT_ISO88025) {
		dlif->dlif_fagamap = dli_faga_defaultmap;
		dlif->dlif_mopv3_functions &= ~FNC_CTRS;
		dlif->dlif_mopv4_functions &= ~FNC_CTRS;
	    }
	    /*
	     * Set the hardware address into the list of addresses for this interface.
	     */
	    dli_getphysaddr(dlif, &devea);
	    dlif->dlif_default_addr.un_w_addr[0] = *(W_T *) &devea.default_pa[0];
	    dlif->dlif_default_addr.un_w_addr[1] = *(W_T *) &devea.default_pa[2];
	    dlif->dlif_default_addr.un_l_addr[1] = *(W_T *) &devea.default_pa[4];
	    /* dli_addrset_add(dlif, NULL, &dlif->dlif_addrset, &dlif->dlif_default_addr); */

	    /*
	     * check to see if sysid support is needed for device.
	     */
	    if (dlif->dlif_ifp->if_sysid_type != 0) {
		dlif->dlif_to = 1 + (dlif->dlif_default_addr.un_l_addr[1] + 150000) * 4/500;
		dlif->dlif_tr = dlif->dlif_to;
	    }

	    /*
	     * Create the interface's filterset for the MOPRC and LOOPBACK filters
	     * Enable MOP Loopback Multicast Address on each Lookback filter.
	     */
	    if (dli_filter_add_ether(dlif, &dli_moprc_ethernet,
				     DLI_FILTER_SPECIAL, &filter) == 0) {
		filter->fltr_ctx = (caddr_t) dlif;
		filter->fltr_match = dli_rcv_moprc;
		dlif->dlif_mopv3_filter = filter;
	    }

	    if (dli_filter_add_snapsap(dlif, &dli_moprc_snapsap,
				       DLI_FILTER_SPECIAL, &filter) == 0) {
		filter->fltr_ctx = (caddr_t) dlif;
		filter->fltr_match = dli_rcv_moprc;
		dlif->dlif_mopv4_filter = filter;
	    }

	    if (dli_filter_add_ether(dlif, &dli_loopback_ethernet,
				     DLI_FILTER_SPECIAL, &filter) == 0) {
		filter->fltr_ctx = (caddr_t) dlif;
		filter->fltr_match = dli_rcv_loopback;
		dli_addrset_add(dlif, filter, &filter->fltr_addrset, &dli_loopback_mcast);
	    }

	    if (dli_filter_add_snapsap(dlif, &dli_loopback_snapsap,
				       DLI_FILTER_SPECIAL, &filter) == 0) {
		filter->fltr_ctx = (caddr_t) dlif;
		filter->fltr_match = dli_rcv_loopback;
		dli_addrset_add(dlif, filter, &filter->fltr_addrset, &dli_loopback_mcast);
	    }

	    dlif->dlif_next = dli_dlifs;
	    dli_dlifs = dlif;
	    dli_ifp2arpcom(dlif->dlif_ifp)->ac_dlif = dlif;
	}
    }
    (void) netisr_add(NETISR_DLI, dli_intr, &dli_intrq, &dlidomain);
}

void
dli_dlif_start(
    struct dli_ifnet *dlif)
{
    /*
     * If this is the first user on this interface,
     * turn on the interface.
     */
    if ((dlif->dlif_flags & DLIF_UP) == 0) {
	dlif->dlif_flags |= DLIF_UP;
	dlif->dlif_ifa.ifa_next = dlif->dlif_ifp->if_addrlist;
	dlif->dlif_ifp->if_addrlist = &dlif->dlif_ifa;
	getutc(&dlif->dlif_createtime);
    }
    if ((dlif->dlif_flags & DLIF_RUNNING) == 0) {
	if (0 == dli_setifaddr(dlif, &dlif->dlif_ifa))
	    dlif->dlif_flags |= DLIF_RUNNING;
    }
}

