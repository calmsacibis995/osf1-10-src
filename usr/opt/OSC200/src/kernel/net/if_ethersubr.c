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
static char	*sccsid = "@(#)$RCSfile: if_ethersubr.c,v $ $Revision: 4.4.12.19 $ (DEC) $Date: 1993/12/10 15:39:53 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/*
 * Copyright (c) 1982, 1989 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	Base:	if_ethersubr.c	7.5 (Berkeley) 9/20/89
 *	Merged:	if_ethersubr.c	7.10 (Berkeley) 6/28/90
 */
/*
 *	Revision History:
 *
 * 6-Feb-91     Chran-Ham Chang
 *	Added 802.5 support and fixed XID response frame format id 
 *
 * 10-Oct-91	Heather Gray
 *	Conditionalise test for full interface queue.
 *
 * 5-June-91	Heather Gray
 *	OSF 1.0.1 patch.
 */

#include "machine/endian.h"
#include "net/net_globals.h"

#include "sys/ioctl.h"
#include "sys/param.h"
#include "sys/time.h"
#include "sys/errno.h"

#include "sys/mbuf.h"
#include "sys/socket.h"

#include "net/if.h"
#include "net/if_llc.h"
#include "net/netisr.h"
#include "net/net_malloc.h"
#include "net/route.h"
#include "net/pfilt.h"

/* Domain stuff will go away with dynamic protocols */
#include "netinet/in.h"
#include "netinet/in_var.h"
#include "netinet/if_ether.h"
#include "netinet/if_fddi.h"
#include "netinet/if_trn.h"
#include "netns/ns.h"
#include "netns/ns_if.h"
#include "net/ether_driver.h"

LOCK_ASSERTL_DECL

CONST u_char	etherbroadcastaddr[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
CONST u_char	ether_ipmulticast_min[6] = { 0x01, 0x00, 0x5e, 0x00, 0x00, 0x00 };
CONST u_char	ether_ipmulticast_max[6] = { 0x01, 0x00, 0x5e, 0x7f, 0xff, 0xff };
union ns_host ns_thishost;	/* XXX */
extern	struct ifnet loif;
extern int fddi_units;
extern int pfactive;	/* from pfilt.c, number of currently active filters */
int fddi_units = 0;
int trn_units = 0;

int SR_enabled = 0;  	/* Global variable for TRN Source Routing */
extern int (*sr_search_func)();
extern void (*srtab_update_func)();

/*
 * Ethernet output routine.
 * Encapsulate a packet of type family for the local net.
 * Use trailer local net encapsulation if enough data in first
 * packet leaves a multiple of 512 bytes of data in remainder.
 * Assumes that ifp is actually pointer to arpcom structure.
 */
ether_output(ifp, m, dst, rt)
	register struct ifnet *ifp;
	register struct mbuf *m;
	struct sockaddr *dst;
	struct rtentry *rt;
{
	u_int type = 0;
	u_int htype = 0;
	int s, error = 0;
	int has_rif = 0;
	int hdrlen;
	u_char edst[6], *hsrc, *hdst;
	struct in_addr *p_idst = NULL;
	struct in_addr idst;
	struct mbuf *mcopy = NULL;
	struct mbuf *mcopy2 = NULL;
	struct trn_rif trn_rif, *rif;
	struct trn_header *trh;

#define	ac ((struct arpcom *)ifp)

	if ((ifp->if_flags & (IFF_UP|IFF_RUNNING)) != (IFF_UP|IFF_RUNNING)) {
		error = ENETDOWN;
		goto bad;
	}

	switch (dst->sa_family) {
	case AF_INET: {
		int usetrailers, off;
		idst = ((struct sockaddr_in *)dst)->sin_addr;
		p_idst = &idst;
		if (!arpresolve(ac, m, &idst, edst, &usetrailers))
			return (0);	/* if not yet resolved */
		if ((ifp->if_flags & IFF_SIMPLEX) && (edst[0] & 1)) {
		        if (bcmp((caddr_t)etherbroadcastaddr, (caddr_t)edst,
	    			sizeof(etherbroadcastaddr)) == 0) {
			    mcopy = m_copym(m, 0, (int)M_COPYALL, M_DONTWAIT);
			    if (mcopy == NULL) {
				error = ENOBUFS;
				goto bad;
			    }
			    mcopy->m_flags |= M_BCAST;
			}
		}
		htype = type = ETHERTYPE_IP;
		off = m->m_pkthdr.len - m->m_len;
		p_idst = &idst;
		if (usetrailers && off > 0 && (off & 0x1ff) == 0 &&
		    (m->m_flags & M_EXT) == 0 &&
		    m->m_data >= m->m_pktdat + 2 * sizeof (u_short)) {
			struct mbuf *m0 = m;
			m->m_data -= 2 * sizeof (u_short);
			m->m_len  += 2 * sizeof (u_short);
			mtod(m, u_short *)[0] = htons(type);
			mtod(m, u_short *)[1] = htons(m->m_len);
			while (m->m_next)
			    m = m->m_next;
			m->m_next = m0;
			m = m0->m_next;
			m0->m_next = 0;
			htype = type = ETHERTYPE_TRAIL + (off >> 9);
		}
		break;
	}

	case AF_NS: {
		htype = type = ETHERTYPE_NS;
		bcopy((caddr_t)&(((struct sockaddr_ns *)dst)->sns_addr.x_host),
		    (caddr_t)edst, sizeof (edst));
		if (!bcmp((caddr_t)edst, (caddr_t)&ns_thishost, sizeof(edst)))
			return (looutput(&loif, m, dst, rt));
		if ((ifp->if_flags & IFF_SIMPLEX) && (edst[0] & 1)) {
		        if (bcmp((caddr_t)etherbroadcastaddr, (caddr_t)edst,
	    			sizeof(etherbroadcastaddr)) == 0) {
			    mcopy = m_copym(m, 0, (int)M_COPYALL, M_DONTWAIT);
			    if (mcopy == NULL) {
				error = ENOBUFS;
				goto bad;
			    }
			    mcopy->m_flags |= M_BCAST;
			}
		}
		break;
	}

	case AF_UNSPEC: {
	    register struct ether_header *eh;
	    register struct arphdr *ah;

	    eh = (struct ether_header *)dst->sa_data;
	    bcopy((caddr_t)eh->ether_dhost, (caddr_t)edst, sizeof (edst));
	    htype = type = eh->ether_type;

	    if ((ifp->if_type == IFT_ISO88025) && SR_enabled &&
	       (htype == ETHERTYPE_ARP))  {
		    ah = mtod(m, struct arphdr *);
		    p_idst = (struct in_addr *)AR_TPA(ah);
	    }
	    break;
	}

	case AF_IMPLINK: {	/* packetfilter write uses this type */
	    type = 0;
	    switch (ifp->if_type) {
		case IFT_ISO88023:
		case IFT_ETHER: {
		    struct ether_header *eh = mtod(m, struct ether_header *);
		    hsrc = eh->ether_shost;
	    	    bcopy((caddr_t)eh->ether_dhost, (caddr_t)edst, sizeof (edst));
		    type = ntohs(eh->ether_type);
		    hdrlen = sizeof(struct ether_header);
		    break;
		}
		case IFT_FDDI: {
		    struct fddi_header *fh = mtod(m, struct fddi_header *);
		    struct llc *l = (struct llc *) (fh + 1);
#ifdef FUTURE  /* not all drivers (ie. defta) give us quad-word aligned data */
		    unsigned long *ll = (unsigned long *) l;
#else
		    unsigned int *ll = (unsigned int *) l;
#endif
		    /* 
		     * Extract Frame Control stuff and sanity-check it.
		     * These checks probably need improvement.
		     * We only allow 48-bit address mode.
		     */
		    switch (fh->fddi_fc & (FDDIFC_C|FDDIFC_L|FDDIFC_F)) {
			case FDDIFC_LLC_ASYNC: {
			    /* legal priorities are 0 through 7 */
			    if ((fh->fddi_fc & FDDIFC_Z) > 7) {
				error = EPROTONOSUPPORT;
				goto bad;
			    }
			    break;
			}
			case FDDIFC_LLC_SYNC: {
			    /* FDDIFC_Z bits reserved, must be zero */
			    if (fh->fddi_fc & FDDIFC_Z) {
				error = EPROTONOSUPPORT;
				goto bad;
			    }
			    break;
			}
			case FDDIFC_SMT: {
			    /* FDDIFC_Z bits must be non zero */
			    if ((fh->fddi_fc & FDDIFC_Z) == 0) {
				error = EPROTONOSUPPORT;
				goto bad;
			    }
			    break;
			}
			default: {
			    /* anything else is too dangerous */
			    error = EPROTONOSUPPORT;
			    goto bad;
			}
		    }
	    	    bcopy((caddr_t)fh->fddi_dhost, (caddr_t)edst, sizeof (edst));
		    hsrc = fh->fddi_shost;
		    hdrlen = sizeof(struct fddi_header);
		    if (LLC_IS_ENCAP_ETHER(ll))
			    type = ntohs(l->llc_snap.ether_type);

		    /* Fill in must-be-right parts of FDDI header */
		    bcopy((caddr_t)ac->ac_enaddr, (caddr_t)fh->fddi_shost,
			sizeof(ac->ac_enaddr));
		    fh->fddi_ph[0] = FDDIPH0;
		    fh->fddi_ph[1] = FDDIPH1;
		    fh->fddi_ph[2] = FDDIPH2;
		    break;
		}

		case IFT_ISO88025: {

		    trh = mtod(m, struct trn_header *);
		    hsrc = trh->trn_shost;
		    hdst = trh->trn_dhost;
	    	    bcopy((caddr_t)trh->trn_dhost, (caddr_t)edst, sizeof (edst));
                    /*
                     * Feed in the AC and FC fields.
                     */
                     trh->trn_ac = TRN_AC;  /* Priority=3; TI=0; Reserv.=0 */
                     trh->trn_fc = TRN_FC;  /* LLC Frame */
                    break;
		}

		default: {
		    error = EAFNOSUPPORT;
		    goto bad;
		}
	    } /* end switch(ifp->if_type) */

	    if (dst->sa_data[0] == 0)
		bcopy((caddr_t)ac->ac_enaddr, (caddr_t)hsrc, ifp->if_addrlen);

	    /* Is this outgoing packetfilter packet a multicast or broadcast,
	     * or possibly a packet directed to our own ether addr?  If so,
	     * save a (protocol header aligned) copy for the loopback driver,
	     * so we will hear it as an input packet.  For now, we only
	     * support IP, ARP and DECnet, since these are the most likely
	     * protocols we'll want to hear as our own output packets.
	     */
	    if ((ifp->if_flags & IFF_SIMPLEX) &&
		((type == ETHERTYPE_IP) ||
		 (type == ETHERTYPE_ARP) ||
		 (type == ETHERTYPE_DECnet)) &&
		((edst[0] & 1) || (bcmp((caddr_t)edst, (caddr_t)ac->ac_enaddr, ifp->if_addrlen) == 0))) {
		struct mbuf *mp;

		mcopy2 = m_copym(m, 0, (int)M_COPYALL, M_DONTWAIT);
		if (mcopy2 == NULL) {
			error = ENOBUFS;
			goto bad;
		}

		/* The mbuf contains the entire packet, including
		 * the ethernet header.  Swap the copies and use the one
		 * we wrote through pfilt_write, since it has an
		 * aligned IP header, etc.  We can't call m_copym() with
		 * an ether_header sized offset because then it won't
		 * copy the m_pkthdr into the new chain (and looutput
		 * requires an m_pkthdr).
		 */
		mp = mcopy2;
		mcopy2 = m;
		m = mp;
		if (edst[0] & 1) {
		    /* this packet is broadcast/multicast, for loopback */
		    if (bcmp((caddr_t)etherbroadcastaddr, (caddr_t)edst,
	    		sizeof(etherbroadcastaddr)) == 0)
			mcopy2->m_flags |= M_BCAST;
		    else
			mcopy2->m_flags |= M_MCAST;
		}
		mcopy2->m_pkthdr.rcvif = ifp; /* ARP needs rcv interface set */
	    }
	    goto gotheader;
	    break;
	} /* end case AF_IMPLINK */

	default: {
		printf("%s%d: can't handle af%d\n", ifp->if_name, ifp->if_unit,
			dst->sa_family);
		error = EAFNOSUPPORT;
		goto bad;
	  }

	} /* end switch (dst->sa_family) */


	if ((ifp->if_flags & IFF_SNAP) && (type != 0)) {
		register struct llc *l;
		M_PREPEND(m, LLC_SNAP_LEN, M_DONTWAIT);
		if (m == NULL) {
			error = ENOBUFS;
			goto bad;
		}

		type = htons((u_short)type);
		l = mtod(m, struct llc *);
		/*
		 * Fill out encapsulated ethernet (SNAP-SAP) 802.2 header
		 * in accordance with RFC1103 and RFC1042:
		 *
		 *      - DSAP = SNAP = 170 (AA Hex)
		 *      - SSAP = SNAP = 170 (AA Hex)
		 *      - Control = UI = 3
		 *      - PID contains:
		 *              - 3-Octet Organization Code = 00-00-00
		 *              - 2-Octet Ether Type (Encapsulated Ethertype)
		 */
		l->llc_dsap = l->llc_ssap = LLC_SNAP_LSAP;
		l->llc_snap.control = LLC_UI;
		l->llc_snap.org_code[0] =
			l->llc_snap.org_code[1] =
				l->llc_snap.org_code[2] = 0;
		bcopy((caddr_t)&type, (caddr_t)&l->llc_snap.ether_type,
			sizeof(l->llc_snap.ether_type));
		type = 0;
	}
	/*
	 * Add MAC header
	 */
	switch (ifp->if_type) {
	case IFT_ISO88023: {
		type = 0;
	}
	/* FALL THROUGH */
	case IFT_ETHER: {
		register struct ether_header *eh;
		/*
		 * Add Ethernet dhost, shost, and type. 
		 */
		if (type == 0)
			type = m->m_pkthdr.len;
		M_PREPEND(m, sizeof (struct ether_header), M_DONTWAIT);
		if (m == NULL) {
			error = ENOBUFS;
			goto bad;
		}
		eh = mtod(m,struct ether_header *);
		((u_char *) &eh->ether_type)[0] = type >> 8;
		((u_char *) &eh->ether_type)[1] = type & 0xFF;
		hdst = eh->ether_dhost;
		hsrc = eh->ether_shost;
		break;
	}
	case IFT_FDDI: {
		register struct fddi_header *fh;
		M_PREPEND(m, sizeof (struct fddi_header), M_DONTWAIT);
		if (m == NULL) {
			error = ENOBUFS;
			goto bad;
		}
		fh = (struct fddi_header *) mtod(m, struct fddi_header *);
		fh->fddi_ph[0] = FDDIPH0;
		fh->fddi_ph[1] = FDDIPH1;
		fh->fddi_ph[2] = FDDIPH2;
		fh->fddi_fc = FDDIFC_LLC_ASYNC | FDDIFC_LLC_PRI4;
		hdst = fh->fddi_dhost;
		hsrc = fh->fddi_shost;
		break;
	}

	case IFT_ISO88025: {
	    struct trn_header *trh;
	    struct trn_rif rif;
	    int hdrlen = sizeof(struct trn_preamble);
           /*
            * Add MAC header for Token Ring. If Source Routing is enabled
            * and if source routing information exists access and insert
            * it to the frame.
            */
            if (SR_enabled)   /* For TRN Source Routing only */
            {
              (*sr_search_func)(ifp, htype, m, edst, p_idst, &trn_rif);
              if (trn_rif.rif_lth > 0)
              {
                    M_PREPEND(m, trn_rif.rif_lth, M_DONTWAIT);
                    if (m == 0)
                    {
                            error = ENOBUFS;
                            goto bad;
                    }
                    bcopy(&trn_rif, mtod(m, struct trn_rif *),
                          trn_rif.rif_lth);
		    has_rif = RIIndicator;	/* need to set the RII bit */
              }
             }       /* end if SR_enabled) */

	    M_PREPEND(m, hdrlen, M_DONTWAIT);

	    if (m == NULL) {
		error = ENOBUFS;
		goto bad;
	    }
	    trh = mtod(m, struct trn_header *);
	    hsrc = trh->trn_shost;
	    hdst = trh->trn_dhost;
	    trh->trn_ac = TRN_AC;
	    trh->trn_fc = TRN_FC;
	    break;
	}

	default: {
		error = EAFNOSUPPORT;
		goto bad;
	}
	} /* end switch (ifp->if_type) */

	bcopy((caddr_t)edst, (caddr_t)hdst, ifp->if_addrlen);
	bcopy((caddr_t)ac->ac_enaddr, (caddr_t)hsrc, ifp->if_addrlen);
	hsrc[0] |= has_rif;
gotheader:

	/*
	 * Queue message on interface, and start output if interface
	 * not yet active.
	 */

	/*
	 * We might be sending something the packetfilter wants to see.
	 *
	 * TODO: use per interface ifp->pfactive flag
	 */
	if (pfactive && ((ifp->if_flags & IFF_PFCOPYALL) || edst[0] & 1) ) {
		int loopback = 1;
		u_short encap_type = 0;
		int istrailer = 0;
		struct ether_driver *edp;
		struct ether_header *ehp;

		edp = (struct ether_driver *)ifp;		
		ehp = mtod(m, struct ether_header *);

		/* set proper m_flags for pfilt_filter() and HowReceived */
		if (edst[0] & 1) {
		    if (bcmp((caddr_t)ac->ac_bcastaddr, (caddr_t)edst, ifp->if_addrlen) == 0) {
			m->m_flags |= M_BCAST;
		    } else {
			m->m_flags |= M_MCAST;
		    }
		}
		m = (struct mbuf *) pfilt_filter(edp, m, ehp, istrailer,
			encap_type, loopback);
	}

	s = splimp();
	/*
	 * Interface queue has been tested if it's an inet packet. Results 
	 * in soft queue limit to avoid dropping fragments of a datagram.
	 */
	IFQ_LOCK(&ifp->if_snd);
	if (dst->sa_family != AF_INET && IF_QFULL(&ifp->if_snd)) {
		IF_DROP(&ifp->if_snd);
		IFQ_UNLOCK(&ifp->if_snd);
		splx(s);
		error = ENOBUFS;
		goto bad;
	}
	IF_ENQUEUE_NOLOCK(&ifp->if_snd, m);
	IFQ_UNLOCK(&ifp->if_snd);
	NETSTAT_LOCK(&ifp->if_slock);
	microtime(&ifp->if_lastchange);
	/*
	 * Accounting for the bytes being transmited
	 */
	ifp->if_obytes += m->m_pkthdr.len;
	if (edst[0] & 1)
		ifp->if_omcasts++;
	NETSTAT_UNLOCK(&ifp->if_slock);
	if ((ifp->if_flags & IFF_OACTIVE) == 0)
		(*ifp->if_start)(ifp);
	splx(s);
	if (mcopy) { 
		/* if mcopy exists, we sent a broadcast */
		(void) looutput(&loif, mcopy, dst, rt);
	} 
	if (mcopy2) {
		/*
		 * loopback packets sent out via the packetfilter
		 * For now, we only allow IP, ARP, and DECnet packets.
		 */
		int isr = -1;

		m_adj(mcopy2, hdrlen); /* skip the ether header */

		switch (type) {
		    case ETHERTYPE_IP: {
			if (ac->ac_ipaddr.s_addr != 0)
			    isr = NETISR_IP;
			break;
		    }
		    case ETHERTYPE_ARP: {
			if (ac->ac_ipaddr.s_addr != 0)
			    isr = NETISR_ARP;
			break;
		    }
		    case ETHERTYPE_DECnet: {
			isr = NETISR_DN;
			break;
		    }
		} /* end switch(type) */

		if (netisr_input(isr, mcopy2, (struct ether_header *)NULL, 0)) {
		    s = splimp();
		    NETSTAT_LOCK(&ifp->if_slock);
		    ifp->if_noproto++;
		    NETSTAT_UNLOCK(&ifp->if_slock);
		    splx(s);
		}
	}
	return (error);

bad:
	if (mcopy2)
		m_freem(mcopy2);
	if (mcopy)
		m_freem(mcopy);
	if (m)
		m_freem(m);
	return (error);
}

extern struct ifqueue bootpq;	/* hooks into machdep.c */
extern int bootp_active;	/* whether we are currently using bootp */

/*
 * Process a received Ethernet packet;
 * the packet is in the mbuf chain m without
 * the ether header, which is provided separately.
 */
void
ether_input(edp, ehp, m, istrailer)
	struct ether_driver *edp;
	struct ether_header *ehp;
	struct mbuf *m;
	int istrailer;
{
	register struct llc *l;
	int s, isr = -1;
	u_char *src, *dst;
	register struct ifnet *ifp = &edp->ess_if;
	int hdrlen, pktlen, droplen = 0;
	u_int etype = 0;
	u_short encap_etype = 0;
	u_char *hdr = (u_char *)ehp; /* no ether header in the mbuf, use ehp */
	u_char trn_sbuf[MAC_ADDR_LEN]; /* used for trn source addr if RII set */
	int arp_m_adj = 0;  /* for trn source routing if arp mbuf is adjusted */
	
	switch (ifp->if_type) {
	    case IFT_FDDI: {
		struct fddi_header *fh;

		/*
		 * Convert FDDI headers to ethernet header for upper levels.
		 */
		hdrlen = sizeof(struct fddi_header);
		fh = (struct fddi_header *) hdr;
		src = fh->fddi_shost;
		dst = fh->fddi_dhost;
		etype = 0;
		break;
	    }
	    case IFT_ISO88023:
	    case IFT_ETHER: {
	        struct ether_header *eh;

		hdrlen = sizeof(struct ether_header);
		eh = (struct ether_header *) hdr;
		src = eh->ether_shost;
		dst = eh->ether_dhost;
		etype = eh->ether_type; /* already in host order! */
		break;
	    }

	    case IFT_ISO88025: {
		struct trn_header *trh;

		hdrlen = sizeof(struct trn_preamble);
		/*
		 * Convert Token Ring headers to ethernet header for 
		 * upper levels.
		 */
		trh = (struct trn_header *) hdr;
		src = trh->trn_shost;
		dst = trh->trn_dhost;
		etype = 0;
		if (src[0] & RIIndicator)  
		{
		/* 
		 * SR information present. Copy to a seperate source addr 
		 * buffer and clear the RII bit.
		 */ 
			bcopy(src, trn_sbuf, MAC_ADDR_LEN);
			src = trn_sbuf;
			src[0] &= ~RIIndicator;
			/*
			 * the size of struct trn_preamble is the same as
			 * the struct ether_header.
			 */
			hdrlen += trh->trn_hdr_rif.rif_lth;
		  }
	     }
        }

	if (*src & 1)
	    goto dropanyway;
	if (*dst & 1) {
	    if (bcmp((caddr_t)ac->ac_bcastaddr, (caddr_t)dst, ifp->if_addrlen) == 0) {
		m->m_flags |= M_BCAST;
	    } else {
		m->m_flags |= M_MCAST;
	    }
	}

	pktlen = m->m_pkthdr.len;	/* don't subtract the header len */

	s = splimp();
	NETSTAT_LOCK(&ifp->if_slock);
	microtime(&ifp->if_lastchange);
	ifp->if_ibytes += pktlen;
	if (m->m_flags & (M_BCAST|M_MCAST))
		ifp->if_imcasts++;
	NETSTAT_UNLOCK(&ifp->if_slock);
	splx(s);

	if (pfactive || (ifp->if_flags & IFF_PROMISC)) {
		m = (struct mbuf *) pfilt_filter(edp, m, ehp,
				istrailer, encap_etype, 0);
		/*
		 * If the packet filter consumed the mbuf it was not
		 * destined for the local node, so just return.
		 */
		if (m == NULL)
			return;
	}

	isr = NETISR_OTHER;
	if (etype <= ETHERMTU) {
	    if (ifp->if_type == IFT_ETHER || ifp->if_type == IFT_ISO88023) {
		if (etype > pktlen) {
		    goto dropanyway;
		} else if ((pktlen == ETHERMIN) || (etype < pktlen)) {
		    /*
		     * If this is a min sized 802.3 packet,
		     * trim off the padding.
		     */
		    m_adj(m, etype - pktlen);
		    m->m_pkthdr.len = etype; /* + hdrlen */
		}
	    }

	    l = mtod(m, struct llc *);
	    switch (l->llc_control) {
		case LLC_UI: {
		    /* LLC_UI_P forbidden in class 1 service */
#ifdef FUTURE  /* not all drivers (ie. defta) give us quad-word aligned data */
		    long *ll = (long *) l;
#else
		    int *ll = (int *) l;
#endif
		    if (pktlen < LLC_UICMD_LEN)
			goto dropanyway;
		    if (LLC_IS_SNAPSAP(ll)) {
			if (pktlen < LLC_SNAP_LEN)
			    goto dropanyway;
			if (LLC_IS_ENCAP_ETHER(ll)) {
			    etype = ntohs(l->llc_snap.ether_type);
			    droplen = LLC_SNAP_LEN;
			}
			/* hdrlen += LLC_SNAP_LEN; */
			break;
		    }
		    /* hdrlen += LLC_UICMD_LEN; */
		    if (LLC_IS_ISOSAP(ll)) {
			isr = NETISR_ISO;
			break;
		    }
		    isr = NETISR_DLI;
		    break;
		}

		case LLC_XID:
		case LLC_XID_P: {
		    if (m->m_len < LLC_XID_LEN)
			goto dropanyway;
		    if (l->llc_dsap != 0) {
			/* only respond for a request to the NULL SAP */
			isr = NETISR_DLI;
			/* hdrlen += LLC_XID_LEN; */
			break;
		    }

		    if ((ifp->if_type == IFT_ISO88025) && SR_enabled)
		    {
			struct trn_header *trh;

		        /* For TRN Source Routing only
		         * Command or Response XID to the NULL SAP, w/ or w/out
		         * SR Info; update the SR Table.
		         */
			trh = (struct trn_header *) hdr;
		    	(*srtab_update_func)(ifp, trh, NULL);
		    }

		    l->llc_window = 0;
		    l->llc_fid = 129;
		    l->llc_class = 1;
		    /* Fall through to */
		}

		case LLC_TEST:
		case LLC_TEST_P: {
		    struct sockaddr sa;
		    register struct ether_header *eh2;
		    u_char c = l->llc_dsap;
		    if (l->llc_dsap != 0) { /* only respond for the NULL SAP */
			isr = NETISR_DLI;
			/* hdrlen += LLC_TEST_LEN; */
			break;
		    }
		    if ((ifp->if_type == IFT_ISO88025) && SR_enabled)
		    {
		    /* For TRN Source Routing only
		     * Do not re-update the SR Table if it is an XID frame.
		     */
		    	if ((l->llc_control == LLC_TEST) ||
			    (l->llc_control == LLC_TEST_P))
			{
			        struct trn_header *trh;

			        trh = (struct trn_header *) hdr;
				(*srtab_update_func)(ifp, trh, NULL);
			}
		    }

		    if (l->llc_ssap & 1)
			goto dropanyway;
		    l->llc_dsap = l->llc_ssap;
		    l->llc_ssap = (c | 1);
		    if (m->m_flags & (M_BCAST | M_MCAST))
			dst = ac->ac_enaddr;
		    sa.sa_family = AF_UNSPEC;
		    sa.sa_len = sizeof(sa);
		    eh2 = (struct ether_header *)sa.sa_data;
		    bcopy(src, eh2->ether_dhost, 6);
		    eh2->ether_type = 0;
		    /* m_adj(m, hdrlen);  No header in mbuf! */
		    (*ifp->if_output)(ifp, m, &sa, (struct rtentry *) NULL);
		    return;
		}
		default: {
		    isr = NETISR_DLI;
		    break;
		}
	    }
	}

	switch (etype) {

	    case ETHERTYPE_IP: {
		if (bootp_active > 0) { /* standalone usage only */
		    struct ifqueue *ifq;

		    if (droplen > 0)
			m_adj(m, droplen);

		    /* packets go to special queue for bootp */
		    ifq = &bootpq;
		    IF_ENQUEUE_NOLOCK(ifq, m);
		    return;
		}
		if (ac->ac_ipaddr.s_addr != 0)
		    isr = NETISR_IP;
		break;
	    }

	    case ETHERTYPE_ARP: {
		if (ac->ac_ipaddr.s_addr == 0)
		    break;
		/* 
		 * Update SR table if Source Routing is enabled.
		 */
		if ((ifp->if_type == IFT_ISO88025) && SR_enabled)
		{
		   /* update only if it's not from me. */
		   if (bcmp((caddr_t)src,
			(caddr_t)(((struct arpcom *)ifp)->ac_hwaddr),
			MAC_ADDR_LEN))
		   {
			struct arphdr *ah;
			struct in_addr isaddr, itaddr;
			struct in_ifaddr *ia;

			if (droplen > 0) {
				m_adj(m, droplen);
				arp_m_adj = 1;
			}
			ah = mtod(m, struct arphdr *);
			bcopy((caddr_t)AR_TPA(ah), (caddr_t)&itaddr, sizeof(itaddr));
			bcopy((caddr_t)AR_SPA(ah), (caddr_t)&isaddr, sizeof(isaddr));
			if (itaddr.s_addr == ((struct arpcom *)ifp)->ac_ipaddr.s_addr)
			{
			  struct trn_header *trh;

			  trh = (struct trn_header *) hdr;
	    		  (*srtab_update_func)(ifp, trh, &isaddr);
			}
			else
			{
			   /*
			    * OSF allows a list of ip addresses per interface
			    */
			   for (ia = in_ifaddr; ia; ia = ia->ia_next)
				if (ia->ia_ifp == ifp)
				   if (itaddr.s_addr == ia->ia_addr.sin_addr.s_addr)
				   {
					struct trn_header *trh;

					trh = (struct trn_header *) hdr;
					(*srtab_update_func)(ifp, trh, &isaddr);
					break;
				   }
			}
		   }
		} /* SR_enabled */

		isr = NETISR_ARP;
		break;
	    }

	    case ETHERTYPE_DECnet: {
		isr = NETISR_DN;
		break;
	    }

	    case ETHERTYPE_LAT: {
		isr = NETISR_LAT;
		break;
	    }

	    case ETHERTYPE_LBACK: {
		/* 
		 * Update SR table if the frame is:
		 * an Ether Loopback frame.
		 * and Source Routing is enabled.
		 */
		if ((ifp->if_type == IFT_ISO88025) &&
		    SR_enabled)
		{ 
		  struct trn_header *trh;

		  trh = (struct trn_header *) hdr;
		  (*srtab_update_func)(ifp, trh, NULL);
                }
		break;
	    }
	}

	if (droplen > 0 && (isr == NETISR_IP || (isr == NETISR_ARP && !arp_m_adj)))
	    m_adj(m, droplen);
	if (netisr_input(isr, m, (caddr_t)ehp, hdrlen)) {
	    s = splimp();
	    NETSTAT_LOCK(&ifp->if_slock);
	    ifp->if_noproto++;
	    NETSTAT_UNLOCK(&ifp->if_slock);
	    splx(s);
	}
	return;

dropanyway:
	m_freem(m);
	return;
}
#undef	ac

/*
 * Stubs for dynamic attach of ARP to drivers (XXX).
 */
void (*arpreq)();
void (*arpreqall)();
void (*arpalias)();
int  (*arpres)();
int  (*arpctl)();

int
arpioctl(cmd, data)
        unsigned int cmd;
        caddr_t data;
{
        if (arpctl)
                return (*arpctl)(cmd, data);
        return EINVAL;
}

void
arpwhohas(ac, addr)
        struct arpcom *ac;
        struct in_addr *addr;
{
        if (arpreq)
                (*arpreq)(ac, addr, ac->ac_bcastaddr);
}

void
arpaliaswhohas(ac, addr)
        struct arpcom *ac;
        struct in_addr *addr;
{
        if (arpalias)
                (*arpalias)(ac, addr, ac->ac_bcastaddr);
}

void
rearpwhohas(ac, addr)
        struct arpcom *ac;
        struct in_addr *addr;
{
        if (arpreqall)
                (*arpreqall)(ac, addr, ac->ac_bcastaddr);
}

int
arpresolve(ac, m, destip, desten, usetrailers)
	register struct arpcom *ac;
	struct mbuf *m;
	register struct in_addr *destip;
	register u_char *desten;
	int *usetrailers;
{
	if (arpres)
		return (*arpres)(ac, m, destip, desten, usetrailers);
	return EINVAL;
}

/*
 * Convert Ethernet address to printable (loggable) representation.
#if	NETSYNC_LOCK
 * This is not currently protected by locks, but used only by if_qe.
 * If anyone else wants to use this, it would be best to pass in &etherbuf.
#endif
 */
CONST static char digits[] = "0123456789abcdef";
char *
ether_sprintf(ap)
	register u_char *ap;
{
	register i;
	static char etherbuf[18];
	register char *cp = etherbuf;

	for (i = 0; i < 6; i++) {
		*cp++ = digits[*ap >> 4];
		*cp++ = digits[*ap++ & 0xf];
		*cp++ = '-';
	}
	*--cp = 0;
	return (etherbuf);
}

/*
 * Initialize the part of the ether_driver structure concerned with
 * the packet filter, and tell the packet filter driver about us
 */
attachpfilter(edp)
struct ether_driver *edp;
{
	switch(edp->ess_if.if_type) {
		case IFT_FDDI:
			attachpfilter_fddi(edp);
			break;
		case IFT_ETHER:
			attachpfilter_ethernet(edp);
			break;
		default:
			printf("attachpfilter: unknown IF type %d\n",
					edp->ess_if.if_type);
			break;
	}
}

#ifndef ETHERMAX
/* Should really be in if_ether.h */
#define ETHERMAX        (ETHERMTU + sizeof(struct ether_header))
#endif  ETHERMAX

attachpfilter_ethernet(edp)
struct ether_driver *edp;
{
	struct endevp enp;
	struct ifnet *ifp = &edp->ess_if;

	enp.end_dev_type = ENDT_10MB;
	enp.end_addr_len = ifp->if_addrlen;
	enp.end_hdr_len = sizeof(struct ether_header);
	enp.end_MTU = ETHERMAX;
	bcopy((caddr_t)(edp->ess_addr),
		    (caddr_t)(enp.end_addr), enp.end_addr_len);
	bcopy((caddr_t)etherbroadcastaddr,
		    (caddr_t)(enp.end_broadaddr), sizeof(etherbroadcastaddr));
	edp->ess_enetunit = pfilt_attach(ifp, &enp);
	edp->ess_missed = 0;
}

attachpfilter_fddi(edp)
struct ether_driver *edp;
{
	struct endevp enp;
	struct ifnet *ifp = &edp->ess_if;

	enp.end_dev_type = ENDT_FDDI;
	enp.end_addr_len = ifp->if_addrlen;
	enp.end_hdr_len = sizeof(struct fddi_header);
	enp.end_MTU = FDDIMAX - 4;	/* but not the CRC bits */
	bcopy((caddr_t)(edp->ess_addr),
		    (caddr_t)(enp.end_addr), enp.end_addr_len);
	bcopy((caddr_t)etherbroadcastaddr,
		    (caddr_t)(enp.end_broadaddr),sizeof(etherbroadcastaddr));

	edp->ess_enetunit = pfilt_attach(ifp, &enp);
	edp->ess_missed = 0;
}

/*
 * returns a printable string version of an internet address
 */
char *
inet_ntoa(in)
        struct in_addr in;
{
        register unsigned char *p;
        register char *b;
        static char buf[20];
        int i;

        p = (unsigned char *)(&in);
        b = buf;
        for (i=0; i<4; i++) {
                if (i) *b++ = '.';
                if (*p > 99)
                        *b++ = '0' + (*p / 100);
                if (*p > 9)
                        *b++ = '0' + ((*p / 10) % 10);
                *b++ = '0' + (*p % 10);
                p++;
        }
        *b++ = 0;
        return(buf);
}

/*
 * Add an Ethernet multicast address or range of addresses to the list for a
 * given interface.
 */
int
if_addmulti(ifp, ifr)
	struct ifnet *ifp;
	struct ifreq *ifr;
{
	struct ifmulti *ifm;
	struct sockaddr_in *sin;
	u_char addrlo[6];
	u_char addrhi[6];
	int ipfullrange = 0;
	int error = 0;

	if (ifp->if_ioctl == 0)
		return (EOPNOTSUPP);

	switch (ifr->ifr_addr.sa_family) {

	case AF_UNSPEC:
		bcopy(ifr->ifr_addr.sa_data, addrlo, 6);
		bcopy(addrlo, addrhi, 6);
		break;

	case AF_INET:
		sin = (struct sockaddr_in *)&(ifr->ifr_addr);
		if (sin->sin_addr.s_addr == INADDR_ANY) {
			/*
			 * An IP address of INADDR_ANY means listen to all
			 * of the Ethernet multicast addresses used for IP.
			 * (This is for the sake of IP multicast routers.)
			 */
			bcopy(ether_ipmulticast_min, addrlo, 6);
			bcopy(ether_ipmulticast_max, addrhi, 6);
			ipfullrange = 1;
		}
		else {
			ETHER_MAP_IP_MULTICAST(&sin->sin_addr, addrlo);
			bcopy(addrlo, addrhi, 6);
		}
		break;

	default:
		return (EAFNOSUPPORT);
	}

	/*
	 * Verify that we have valid Ethernet multicast addresses.
	 */
	if ((addrlo[0] & 0x01) != 1 || (addrhi[0] & 0x01) != 1)
		return (EINVAL);

	/*
	 * See if the address range is already in the list.
	 */
	IF_LOOKUP_MULTI(addrlo, addrhi, ifp, ifm);
	if (ifm != NULL) {
		/*
		 * Found it; just increment the reference count.
		 */
		++ifm->ifm_refcount;
		return (0);
	}
	/*
	 * New address or range; create a new multicast record
	 * and link it into the interface's multicast list.
	 */
	NET_MALLOC(ifm, struct ifmulti *, sizeof *ifm, M_IFMADDR, M_NOWAIT);
	if (ifm == NULL)
		return (ENOBUFS);

	bcopy(addrlo, ifm->ifm_addrlo, 6);
	bcopy(addrhi, ifm->ifm_addrhi, 6);
	ifm->ifm_ifnet = ifp;
	ifm->ifm_refcount = 1;
	ifm->ifm_next = ifp->if_multiaddrs;
	ifp->if_multiaddrs = ifm;
	ifp->if_multicnt++;

	bcopy(addrlo, ifr->ifr_addr.sa_data, 6);
	if (!ipfullrange)
		error = (*ifp->if_ioctl)(ifp, SIOCADDMULTI, ifr);

	/*
	 * If there are no more slots in the driver's table, or if the 
	 * full range of IP multicasts were requested, tell driver to 
	 * receive all multicasts, if not doing so already. 
	 */
	if (error == ENOBUFS || ipfullrange) {
		if (ifp->if_flags & IFF_ALLMULTI)
			error = 0;
		else {
			ifr->ifr_flags = (ifp->if_flags |= IFF_ALLMULTI);
			ifp->if_flags = (ifp->if_flags & IFF_CANTCHANGE) | 
				(ifr->ifr_flags &~ IFF_CANTCHANGE);
			error = (*ifp->if_ioctl)(ifp, SIOCSIFFLAGS, ifr);
		}
		ifp->if_allmulticnt++;
	}

	return (error);
}

/*
 * Delete a multicast address record.
 */
int
if_delmulti(ifp, ifr)
	struct ifnet *ifp;
	struct ifreq *ifr;
{
	register struct ifmulti *ifm;
	register struct ifmulti **p;
	struct sockaddr_in *sin;
	u_char addrlo[6];
	u_char addrhi[6];
	int ipfullrange = 0;
	int error = 0;

	if (ifp->if_ioctl == 0)
		return (EOPNOTSUPP);

	switch (ifr->ifr_addr.sa_family) {

	case AF_UNSPEC:
		bcopy(ifr->ifr_addr.sa_data, addrlo, 6);
		bcopy(addrlo, addrhi, 6);
		break;

	case AF_INET:
		sin = (struct sockaddr_in *)&(ifr->ifr_addr);
		if (sin->sin_addr.s_addr == INADDR_ANY) {
			/*
			 * An IP address of INADDR_ANY means stop listening
			 * to the range of Ethernet multicast addresses used
			 * for IP.
			 */
			bcopy(ether_ipmulticast_min, addrlo, 6);
			bcopy(ether_ipmulticast_max, addrhi, 6);
			ipfullrange = 1;
		}
		else {
			ETHER_MAP_IP_MULTICAST(&sin->sin_addr, addrlo);
			bcopy(addrlo, addrhi, 6);
		}
		break;

	default:
		return (EAFNOSUPPORT);
	}

	/*
	 * Look up the address in our list.
	 */
	IF_LOOKUP_MULTI(addrlo, addrhi, ifp, ifm);
	if (ifm == NULL)
		return (ENXIO);

	if (--ifm->ifm_refcount != 0)
		/*
		 * Still some claims to this record.
		 */
		return (0);

	/*
	 * No remaining claims to this record; unlink and free it.
	 */
	for (p = &ifm->ifm_ifnet->if_multiaddrs;
	     *p != NULL && *p != ifm;
	     p = &(*p)->ifm_next)
		continue;
	if (*p == NULL)
		panic("if_delmulti: ifmulti not found");
	*p = (*p)->ifm_next;
	NET_FREE(ifm, M_IFMADDR);

	bcopy(addrlo, ifr->ifr_addr.sa_data, 6);
	error = (*ifp->if_ioctl)(ifp, SIOCDELMULTI, ifr);
	if (!error)
		ifp->if_multicnt--;

	return (error);
}
