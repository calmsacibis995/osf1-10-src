
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

#ifndef _DLI_DLI_KERNEL_H
#define	_DLI_DLI_KERNEL_H	/* ?? */

#include <dli/dli_types.h>
#include <sys/utctime.h>
#include <net/if_llc.h>

#ifndef offsetof
#define	offsetof(s, m)	((caddr_t) &((s *) NULL)->m - ((caddr_t) NULL))
#endif

/*
 * DLI socket literals used only internal to the kernel
 */
#define DLPROTO_CSMACD  	1		/* CSMA-CD protocol number */
#define DLPROTO_LOOP    	2		/* Loopback protocol number */
#define DLPROTO_FDDI    	4		/* FDDI protocol number */
#if defined(ultrix) || defined(__ultrix)
#define DLPROTO_HDLC    	7		/* HDLC protocol number */
#define DLPROTO_LAPB		9		/* LAPB protocol number */
#endif
#define DLPROTO_TRN 		14		/* ISO 8802.5 (Token Ring) */

/*
 * Generic dli event codes from data link devices.
 */
#if defined(ultrix) || defined(__ultrix)
#define DLIPROTO_DISABLED	0
#define DLIPROTO_ENABLED	1
#define DLIPROTO_HALTED		2
#define DLIPROTO_INITIALIZING	3
#define DLIPROTO_RUNNING	4
#define DLIPROTO_DATALOSS	5
#define DLIPROTO_ENTERMAINT	6
#define DLIPROTO_EXITMAINT	7
#define DLIPROTO_RETRYMAX       8
#endif

/*
 * Generic dli protocol operation i/f codes.
 */
#define DLIPIF_OPENPORT		0
#define DLIPIF_CLOSEPORT	1
#if defined(ultrix) || defined(__ultrix)
#define DLIPIF_INITLINK		2
#define DLIPIF_STOPLINK		3
#define DLIPIF_SHOWLINK		4
#define DLIPIF_ENTERMAINT       5
#define DLIPIF_EXITMAINT        6
#endif
#define DLIPIF_NAMETOIF		7
#define DLIPIF_MAX		8

union dli_llcdata {
    u_char		un_data[sizeof(struct llc)];
    struct llc		un_llc;
    W_T			un_w_data[sizeof(struct llc)/sizeof(W_T)];
    LW_T		un_l_data[sizeof(struct llc)/sizeof(LW_T)];
#ifdef QW_T
    QW_T		un_q_data[sizeof(struct llc)/sizeof(QW_T)];
#endif
};

union dli_addr {
    u_char		un_addr[8];
    W_T			un_w_addr[8/sizeof(W_T)];
    LW_T		un_l_addr[8/sizeof(LW_T)];
#ifdef QW_T
    QW_T		un_q_addr[8/sizeof(QW_T)];
#endif
};

#define LLC_FMT(c)	(((c) & 1) ? (((c) & 2) ? LLC_U_FMT : LLC_S_FMT) : LLC_I_FMT)
#define LLC_U_FMT	0x03	/* unnumbered commands and responses */
#define LLC_S_FMT	0x01	/* supervisory commands and responses */
#define LLC_I_FMT	0x00	/* informational commands and responses */
#define	LLC_PF_FLAG	0x10	/* Poll/Final bit */
#define LLC_ISGSAP(s)	((s) & 1)
#define LLC_SAPIDX(s)	((s) >> 1)

struct dli_sapset {
    DLI_DECL_BITSET(ss_subsets, NISAPS);
};

#define	DLI_SETSAP(sap, set)	DLI_SETBIT(LLC_SAPIDX(sap), (set)->ss_subsets)
#define	DLI_TSTSAP(sap, set)	DLI_TSTBIT(LLC_SAPIDX(sap), (set)->ss_subsets)
#define	DLI_CLRSAP(sap, set)	DLI_CLRBIT(LLC_SAPIDX(sap), (set)->ss_subsets)

struct dli_recv	{			/* received message prefix */
    struct dli_ifnet *	rcv_dlif;	/* received interface */
    struct llc *	rcv_llc;	/* pointer to llc data in mbuf */
    u_int		rcv_flags;	/* Flags for dli_filter_input */
    u_int		rcv_hdrlen;	/* size of the media hdr (ie. no LLC) */
    union dli_addr	rcv_src;	/* Aligned Source Address */
    union dli_addr	rcv_dst;	/* Aligned Destination Address */
    union dli_llcdata	rcv_llcdata;
#define	rcv_llc		rcv_llcdata.un_llc
#define rcv_etype	rcv_llcdata.un_llc.llc_snap.ether_type
};

#define	DLI_ADDRLIST_INC	16
#define	DLI_ADDRLIST_MAXSIZE	256
#define	DLI_ADDRHASH_ENDOFLIST	DLI_ADDRLIST_MAXSIZE
#define	DLI_ADDRHASH_MAXBUCKETS	64

struct dli_mcast {
    union dli_addr	dlm_addr;
    u_int		dlm_index;
    u_int		dlm_map_idx;
    u_int		dlm_next_index;
    u_int		dlm_references;
};

struct dli_addrset {
    DLI_DECL_BITSET(as_subsets, DLI_ADDRLIST_MAXSIZE);
};

#define	DLI_SETADDR_IDX(idx, set)	DLI_SETBIT(idx, (set)->as_subsets)
#define	DLI_TSTADDR_IDX(idx, set)	DLI_TSTBIT(idx, (set)->as_subsets)
#define	DLI_CLRADDR_IDX(idx, set)	DLI_CLRBIT(idx, (set)->as_subsets)

struct dli_filter;				/* need these for the typedef */

#define	DLI_FILTER_MATCH_ARGS	struct dli_recv *rcv, struct mbuf *m, struct dli_filter *filter

typedef	int (*dli_filter_match_t)(DLI_FILTER_MATCH_ARGS);

struct dli_filter {
    struct dli_filter *	fltr_next;		/* does not match LLC */
    struct dli_filter *	fltr_sibling;		/* matches LLC, but not source address */
    unsigned		fltr_flags;
#define	DLI_FILTER_ISAP		0x10000
#define	DLI_FILTER_ETYPE	0x20000
#define	DLI_FILTER_SNAPSAP	0x40000
#define	DLI_FILTER_CLASS	(DLI_FILTER_ISAP|DLI_FILTER_ETYPE|DLI_FILTER_SNAPSAP)
#define	DLI_FILTER_EXCLUSIVE	0x01000
#define	DLI_FILTER_DEFAULT	0x02000
#define	DLI_FILTER_SPECIAL	0x04000		/* special filter (MOPRC, LOOPBACK) */
#define	DLI_FILTER_ABNORMAL	(DLI_FILTER_EXCLUSIVE|DLI_FILTER_DEFAULT|DLI_FILTER_SPECIAL)
#define	DLI_FILTER_SVC_TYPE1	0x00100
#define	DLI_FILTER_ETHERPAD	0x00200
#define	DLI_FILTER_HAVEPORT	0x00400
#define	DLI_FILTER_ALLMULTI	0x00800
#define	DLI_FILTER_ONRING	0x00010
    union dli_addr	fltr_src;
    union dli_llcdata	fltr_llcdata;
    union dli_llcdata	fltr_maskdata;
    dli_filter_match_t	fltr_match;
    caddr_t 		fltr_ctx;
    struct dli_lanport *fltr_lanport;		/* port id returned by dlimgt */
    struct dli_addrset	fltr_addrset;
    struct dli_sapset	fltr_gsaps;
    u_long		fltr_pdus_rcvd[2];	/* I/G PDUs received */
    u_long		fltr_octets_rcvd[2];	/* I/G octets received */
    u_long		fltr_pdus_sent[2];	/* I/G PDUs sent */
    u_long		fltr_octets_sent[2];	/* I/G octets sent */
};

#define fltr_srcaddr	fltr_src.un_addr
#define	fltr_llc	fltr_llcdata.un_llc
#define	fltr_mask	fltr_maskdata.un_llc

#define	DLI_ETYPEHASH_MAXBUCKETS	64
#define	DLI_ETYPE_MKHASH(llcp)	((llcp)->llc_snap.ether_type ^ (llcp)->llc_snap.org_code[2])
#define	DLI_ETYPE_DOHASH(hash)	((((hash) ^ ((hash << 5) & 0xEFC0) ^ (hash << 8)) & 0xffff) >> 10)

struct dli_ifnet {
    struct ifaddr	dlif_ifa;		/* ifaddr for ifp list */
    struct dli_ifnet *	dlif_next;		/* pointer to next dlif */
    struct dli_filter *	dlif_isaps[NISAPS];	/* Individual SAP filters */
    struct dli_filter *	dlif_ethers[DLI_ETYPEHASH_MAXBUCKETS]; /* ETHERNET filters */
    struct dli_filter *	dlif_protoids[DLI_ETYPEHASH_MAXBUCKETS]; /* SNAP SAP filters */
    struct dli_filter *	dlif_mopv3_filter;	/* for sysids, etc. */
    struct dli_filter *	dlif_mopv4_filter;	/* for sysids, etc. */
    struct dli_lanstation *dlif_station;	/* for dligmt */
    struct protosw *	dlif_proto;		/* for dlimgt */
    dli_filter_match_t	dlif_rcv;		/* for dlimgt/trn */
    u_int		dlif_mopv3_functions;	/* mop functions for sysid */
    u_int		dlif_mopv4_functions;	/* mop functions for sysid */
    u_int		dlif_filters_active;	/* count of active filters */
    u_int		dlif_to;		/* remaining sysid time */
    u_int		dlif_tr;		/* sysid time interval */
    u_int		dlif_flags;		/* DLI interface flags */
#define	DLIF_UP		0x0010			/* Station is attached */
#define	DLIF_RUNNING	0x0020			/* Station is running */
    struct dli_addrset	dlif_addrset;		/* bitset of addresses */
    union dli_addr	dlif_default_addr;	/* hardware addr of this station */
    struct dli_mcast *	dlif_addrlist;		/* list of ... */
    const struct dli_fagamap	*dlif_fagamap;	/* faga for this station */
    u_int		dlif_addrlist_size;	/* current size of list of */
    u_int		dlif_addrlist_active;	/* active addrs in list */
    u_int		dlif_addr_buckets[DLI_ADDRHASH_MAXBUCKETS]; /* address hash buckets */
    u_int		dlif_sap_pairs_active;	/* ISAP/GSAP pairs active */
    struct dli_sapset *	dlif_gsaps;		/* GSAP is index */
    struct sockaddr	dlif_ifa_addr;		/* sockaddr for ifaddr */
    struct utc		dlif_createtime;	/* time the first setifaddr was done */
    u_long		dlif_unrecog_pdus_rcvd[2];	/* unrecognized I/G PDUs received */
    u_long		dlif_pdus_rcvd[2];	/* I/G PDUs received */
    u_long		dlif_octets_rcvd[2];	/* I/G octets received */
    u_long		dlif_pdus_sent[2];	/* I/G PDUs sent */
    u_long		dlif_octets_sent[2];	/* I/G octets sent */
};

#define	dlif_ifp	dlif_ifa.ifa_ifp
#define	dlif_hdrlen	dlif_ifp->if_hdrlen
#define	dlif_ifflags	dlif_ifp->if_flags
#define	dlif_iftype	dlif_ifp->if_type
#define	dlif_ifmtu	dlif_ifp->if_mtu

#define	dlif_output(dlif, m, sa) \
	((*(dlif)->dlif_ifp->if_output)((dlif)->dlif_ifp, m, sa, NULL))
#define	dlif_ioctl(dlif, cmd, data) \
	((*(dlif)->dlif_ifp->if_ioctl)((dlif)->dlif_ifp, cmd, data))
#define so2dlipcb(so)	\
	((struct dli_pcb *)(so)->so_pcb)

#define dli_getmopctrs(dlif, ctrs)	(dlif_ioctl(dlif, SIOCRDCTRS, (caddr_t) ctrs))
#define	dli_setifaddr(dlif, ifa)	(dlif_ioctl(dlif, SIOCSIFADDR, (caddr_t) ifa))
#define	dli_getphysaddr(dlif, ifr)	(dlif_ioctl(dlif, SIOCRPHYSADDR, (caddr_t) ifr))
#define	dli_setphysaddr(dlif, ifr)	(dlif_ioctl(dlif, SIOCSPHYSADDR, (caddr_t) ifr))
#ifdef IFF_MULTICAST
#define	dli_addmulti(dlif, ifr)		(if_addmulti((dlif)->dlif_ifp, ifr))
#define	dli_delmulti(dlif, ifr)		(if_delmulti((dlif)->dlif_ifp, ifr))
#else
#define	dli_addmulti(dlif, ifr)		(dlif_ioctl(dlif, SIOCADDMULTI, (caddr_t) ifr))
#define	dli_delmulti(dlif, ifr)		(dlif_ioctl(dlif, SIOCDELMULTI, (caddr_t) ifr))
#endif
/*
 * DLI Protocol Control Block
 */
struct dli_pcb {
    struct dli_ifnet *	dli_dlif;	/* device interface to use */
    struct socket *	dli_so;		/* back ptr to owning socket */
    struct protosw *	dli_proto;      /* proto of data link mgt module */
    struct dli_filter *	dli_filter;	/* next port of this type */
    union dli_addr	dli_dstaddr;	/* default destination address */
    u_short		dli_state;	/* state of device */
    u_short		dli_iloop;	/* loopback state */
    struct sockaddr_dl	dli_lineid;	/* link user to target */
    struct sockaddr_dl	dli_rcvaddr;	/* saved sockaddr to speed up rcvs */
    u_int		dli_mopfunctions; /* MOP SYSID functions */
};


#define	DLI_DEC_OUI	0x08, 0x00, 0x2B
#define	DLI_ETHER_OUI	0x00, 0x00, 0x00
#define	DLI_SNAP_HDR	SNAP_SAP, SNAP_SAP, UI_NPCMD
#define	DLI_DEC_SNAPSAP(etype)		DLI_SNAP_HDR, DLI_DEC_OUI, (etype >> 8), (etype & 0xFF)
#define	DLI_ETHER_SNAPSAP(etype)	DLI_SNAP_HDR, DLI_ETHER_OUI, (etype >> 8), (etype & 0xFF)
#define	ETHERTYPE_LAST		0x8041
#define	ETHERTYPE_DECDNS	0x803C
#define	ETHERTYPE_DTSS		0x803E
#define	ETHERTYPE_NETBIOS	0x8040

#define	DLI_FAGAMAP_GROUPADDR	0
#define	DLI_FAGAMAP_FUNCADDR	1

enum dli_fagamap_index {
    DLI_GA_ENDOFLIST,
    DLI_GA_LBACK_V3_ASSIST,
    DLI_GA_LBACK_V4_ASSIST,
    DLI_GA_MOPRC_V3_SYSIDS,
    DLI_GA_MOPRC_V4_SYSIDS,
    DLI_GA_MOPDL_V3_ASSIST,
    DLI_GA_MOPDL_V4_ASSIST,
    DLI_GA_DECNET_ALL_ROUTERS,
    DLI_GA_DECNET_ALL_ENDNODES,
    DLI_GA_DECNET_ALL_L2ROUTERS,
    DLI_GA_DECNET_PRIME_ROUTERS,
    DLI_GA_DECNET_ALL_UNKDEST,
    DLI_GA_ESIS_ALL_ES,
    DLI_GA_ESIS_ALL_IS,
    DLI_GA_LAST_GROUP0,
    DLI_GA_DECDNS_ADVERS,
    DLI_GA_DECDNS_SOLICITS,
    DLI_GA_DTSS_SOLICITS,
    DLI_GA_PCSA_NETBIOS,
    DLI_GA_LAT_SERVERS,
    DLI_GA_LAT_SLAVES,
    DLI_GA_LAT_SC3_SOLICITS,
    DLI_GA_8021E_LOAD_SERVERS,
    DLI_GA_8021E_LOAD_DEVICES,
    DLI_GA_8021E_MGMT_STATIONS,
    DLI_GA_8021E_AGENT_STATIONS
};

struct dli_fagamap {
    enum dli_fagamap_index map_idx;		/* index of entry, DLI_GA_ENDOFLIST terminates */
    u_int map_flags;				/* filter flags for matching */
    union dli_llcdata map_llc;			/* LLC information for matching */
    union dli_addr map_addrs[2];		/* The addresses to map */
};

#define	DLI_FILTER_EQUAL(f1, f2) \
	(DLI_FILTER_LLC_MATCH(f1, &(f2)->fltr_llcdata) || DLI_FILTER_LLC_MATCH(f2, &(f1)->fltr_llcdata))

#ifdef QW_T
#define	DLI_FILTER_LLC_MATCH(filter, llcdata) \
	(((llcdata)->un_q_data[0] & (filter)->fltr_maskdata.un_q_data[0]) \
		== (filter)->fltr_llcdata.un_q_data[0])

#define	DLI_ADDR_EQUAL(a1, a2) \
	((a1)->un_q_addr[0] == (a2)->un_q_addr[0])

#define	DLI_ADDR_IS_MULTI(a1) \
	((a1)->un_q_addr[0] & 1)

#define DLI_ADDR_COPY(from, to) \
	((to)->un_q_addr[0] = QW_W0(*(W_T *) &(from)[0]) \
		|QW_W1(*(W_T *) &(from)[2])|QW_W2(*(W_T *) &(from)[4]))

#define	DLI_ADDR_COPY_QW_PLUS2(from, addr) \
	((addr)->un_q_addr[0] = *(QW_T *) &(from)[-2] >> 16)

#define	DLI_ADDR_COPY_QW_ALIGNED(from, addr) \
	((addr)->un_q_addr[0] = *(QW_T *) &(from)[0] & 0x0000FFFFFFFFFFFFL)

#define	DLI_ADDR_COPY_LW_ALIGNED(from, addr) \
	((addr)->un_q_addr[0] = (QW_LW0(*(LW_T *) &(from)[0]) | QW_LW1(*(LW_T *) &(from)[4])) & 0x0000FFFFFFFFFFFFL)

#define	DLI_FILTER_SET_LLCMASK_CMP_ALL(filter) \
	((filter)->fltr_maskdata.un_q_data[0] = ~0)

#define	DLI_LLC_COPY_QW_ALIGNED(from, llcdata) \
	((llcdata)->un_q_data[0] = *(QW_T *) &(from)[0])

#if 0
#define	DLI_FILTER_EQUAL(f1, f2) \
	 (  (f1)->fltr_llcdata.un_q_data[0] == (f2)->fltr_llcdata.un_q_data[0] \
	&& (f1)->fltr_maskdata.un_q_data[0] == (f2)->fltr_maskdata.un_q_data[0])
#endif

#define DLI_ADDR_EXTRACT(ap, dst) \
	(((QW_T *) (dst))[0] = (ap)->un_q_addr[0])
#else
#define	DLI_FILTER_LLC_MATCH(filter, llcdata) \
	(((llcdata)->un_l_data[0] & (filter)->fltr_maskdata.un_l_data[0]) \
		== (filter)->fltr_llcdata.un_l_data[0] && \
	 ((llcdata)->un_l_data[1] & (filter)->fltr_maskdata.un_l_data[1]) \
		== (filter)->fltr_llcdata.un_l_data[1])

#define	DLI_ADDR_EQUAL(a1, a2) \
	((a1)->un_l_addr[1] == (a2)->un_l_addr[1] \
		&& (a1)->un_l_addr[0] == (a2)->un_l_addr[0])

#define	DLI_ADDR_IS_MULTI(a1) \
	((a1)->un_l_addr[0] & 1)

#define DLI_ADDR_COPY(from, to) \
	((to)->un_l_addr[0] = LW_W0(*(W_T *) &(from)[0]) \
	 	|LW_W1(*(W_T *) &(from)[2]), \
	 (to)->un_l_addr[1] = LW_W0(*(W_T *) &(from)[4]))

#define DLI_ADDR_COPY_QW_PLUS2	DLI_ADDR_COPY

#define	DLI_ADDR_COPY_LW_ALIGNED(from, addr) \
	((addr)->un_l_addr[0] = *(LW_T *) &(from)[0], \
	 (addr)->un_l_addr[1] = *(LW_T *) &(from)[4] & 0x0000FFFF)

#define	DLI_ADDR_COPY_QW_ALIGNED	DLI_ADDR_COPY_LW_ALIGNED

#define	DLI_FILTER_SET_LLCMASK_CMP_ALL(filter) \
	((filter)->fltr_maskdata.un_l_data[0] = (filter)->fltr_maskdata.un_l_data[1] = ~0)

#define	DLI_LLC_COPY_QW_ALIGNED(from, llcdata) \
	((llcdata)->un_l_data[0] = *(LW_T *) &(from)[0], \
	 (llcdata)->un_l_data[1] = *(LW_T *) &(from)[1])

#if 0
#define	DLI_FILTER_EQUAL(f1, f2) \
	(  (f1)->fltr_llcdata.un_l_data[0] == (f2)->fltr_llcdata.un_l_data[0] \
	&& (f1)->fltr_llcdata.un_l_data[1] == (f2)->fltr_llcdata.un_l_data[1] \
	&& (f1)->fltr_maskdata.un_l_data[0] == (f2)->fltr_maskdata.un_l_data[0] \
	&& (f1)->fltr_maskdata.un_l_data[1] == (f2)->fltr_maskdata.un_l_data[1])
#endif

#define DLI_ADDR_EXTRACT(ap, dst) \
	(((LW_T *) (dst))[0] = (ap)->un_l_addr[0], ((LW_T *) (dst))[1] = (ap)->un_l_addr[1])
#endif

#define	dli_ifp2arpcom(ifp)	((struct arpcom *) ifp)
#define	dli_ifp2dlif(ifp)	(dli_ifp2arpcom(ifp)->ac_dlif)

#define	DLI_EHDR(sa)		((struct ether_header *) (sa)->sa_data)

#if defined(LW_T)
#define	DLI_802ADDR_TO_EHDR(dli_802, eh) \
	do { \
	    *(W_T *) &(eh)->ether_dhost[0] = *(W_T *) &(dli_802)->eh_802.dst[0];\
	    *(W_T *) &(eh)->ether_dhost[2] = *(W_T *) &(dli_802)->eh_802.dst[2];\
	    *(W_T *) &(eh)->ether_dhost[4] = *(W_T *) &(dli_802)->eh_802.dst[4];\
	    *(W_T *) &(eh)->ether_type = 0; \
	} while (0)

#define	DLI_EADDR_TO_EHDR(eaddr, eh) \
	do { \
	    *(W_T *) &(eh)->ether_dhost[0] = *(W_T *) &(eaddr)->dli_target[0];\
	    *(W_T *) &(eh)->ether_dhost[2] = *(W_T *) &(eaddr)->dli_target[2];\
	    *(W_T *) &(eh)->ether_dhost[4] = *(W_T *) &(eaddr)->dli_target[4];\
	    *(W_T *) &(eh)->ether_type = (eaddr)->dli_protype; \
	} while (0)

#define	DLI_DSTADDR_TO_EHDR(addr, eh) \
	( *(W_T *) &(eh)->ether_dhost[0] = (addr)->un_w_addr[0], \
	  *(W_T *) &(eh)->ether_dhost[2] = (addr)->un_w_addr[1], \
	  *(W_T *) &(eh)->ether_dhost[4] = (addr)->un_w_addr[2] )
#endif

/*
 * Loopback module.
 */
#define DLOOP_OPENPORT	DLIPIF_OPENPORT
#define DLOOP_CLOSEPORT	DLIPIF_CLOSEPORT

struct dlp_port {
    struct protosw *dlp_pr;		/* user's protosw */
    u_int dlp_id;			/* id returned by loop module */
};

/*
 * ethernet address structure used as a cast to copy ethernet
 * physical addresses.
 */
struct ether_pa {
    u_char ether_addr_octet[6];
};

#define	DLI_PCB_LOCKINIT(pcb)
#define	DLI_PCB_LOCK(pcb)
#define	DLI_PCB_UNLOCK(pcb)
#define	DLI_SOCKET_LOCK(so)
#define	DLI_SOCKBUF_LOCK(sb)
#define	DLI_SOCKBUF_UNLOCK(sb)
#define	DLI_SOCKET_UNLOCK(so)

#define	DLI_DLIF_LOCKINIT(dlif)
#define	DLI_DLIF_LOCK(dlif)
#define	DLI_DLIF_UNLOCK(dlif)

#define	DLI_DLIF_FILTERS_LOCKINIT(dlif)
#define	DLI_DLIF_FILTERS_LOCK(dlif)
#define	DLI_DLIF_FILTERS_READLOCK(dlif)
#define	DLI_DLIF_FILTERS_UNLOCK(dlif)


extern int			dli_mustberoot;
extern int			dli_ifq_maxlen;
extern int			dli_debug;
extern struct domain		dlidomain;
extern struct ifqueue		dli_intrq;
extern struct dlimgt_routines *	dlimgtsw;
extern struct dli_ifnet	*	dli_dlifs;
const extern union dli_addr	dli_sysid_mcast;
const extern union dli_addr	dli_loopback_mcast;
const extern struct sockaddr_802 dli_moprc_snapsap;
const extern struct sockaddr_802 dli_loopback_snapsap;
#define	dli_moprc_protoid	dli_moprc_snapsap.eh_802.osi_pi
#define	dli_loopback_protoid	dli_loopback_snapsap.eh_802.osi_pi

/* dli_bind.c */

extern int dli_bind(struct dli_pcb **, struct sockaddr_dl *);
extern int dli_unbind(struct dli_pcb **);
extern int dli_rcv_user(DLI_FILTER_MATCH_ARGS);

/* dli_filter.c */

extern int dli_filter_create(struct dli_ifnet *, struct dli_filter **, unsigned, u_char *);
extern int dli_filter_add(struct dli_ifnet *, const struct sockaddr_dl *, struct dli_filter **);
extern int dli_filter_add_ether(struct dli_ifnet *, const struct sockaddr_edl *, int, struct dli_filter **);
extern int dli_filter_add_sap(struct dli_ifnet *, const struct sockaddr_802 *, int, struct dli_filter **);
extern int dli_filter_add_snapsap(struct dli_ifnet *, const struct sockaddr_802 *, int, struct dli_filter **);
extern int dli_filter_insert(struct dli_ifnet *, struct dli_filter *);
extern void dli_filter_destroy(struct dli_ifnet *, struct dli_filter **);
extern void dli_filter_input(struct dli_recv *, struct mbuf *);

extern int dli_filter_add_gsap(struct dli_ifnet *, struct dli_filter *, u_int);
extern int dli_filter_test_gsap(struct dli_filter *, u_int);
extern int dli_filter_remove_gsap(struct dli_ifnet *, struct dli_filter *, u_int);
extern void dli_filter_disable_gsap(struct dli_ifnet *, struct dli_filter *, u_int);
extern void dli_filter_disable_all_gsaps(struct dli_ifnet *, struct dli_filter *);
extern void dli_filter_by_gsap(struct dli_recv *, struct mbuf *);

/* dli_init.c */

extern void dli_init(void);
extern void dli_dlif_start(struct dli_ifnet *);

/* dli_input.c */

extern void dli_intr(void);
extern int dli_rcv_addr_filter(DLI_FILTER_MATCH_ARGS);
extern int dli_rcv_loopback(DLI_FILTER_MATCH_ARGS);
extern int dli_rcv_moprc(DLI_FILTER_MATCH_ARGS);

/* dli_mopctrs.c */

extern struct mbuf *dli_get_ether_ctrs(struct dli_ifnet *, u_int);
extern struct mbuf *dli_get_fddi_ctrs(struct dli_ifnet *, u_int);
extern void dli_snd_mopctrs(struct dli_ifnet *, struct dli_filter *, u_int, const union dli_addr *);

/* dli_output.c */

extern int dli_send(struct dli_pcb *, struct mbuf *m, struct sockaddr_dl *);
extern int dli_add_header(struct dli_ifnet *, struct mbuf **m, const union dli_llcdata *, int);
extern int dli_output(struct dli_ifnet *, struct dli_filter *, struct mbuf *, const union dli_addr *);

/* dli_sockopt.c */

extern int dli_getopt(struct dli_pcb *, u_char *, int *, int);
extern int dli_setopt(struct dli_pcb *, u_char *, int, int);

/* dli_subr.c */

extern int dli_llc_responder(struct dli_recv *, struct mbuf *, struct dli_filter *);
extern u_int dli_addr_hash(const union dli_addr *);
extern struct dli_mcast *dli_addr_lookup(struct dli_ifnet *, const struct dli_filter *filter, const union dli_addr *, const struct dli_fagamap *);
extern void dli_addrset_init(struct dli_ifnet *, const struct dli_filter *, struct dli_addrset *);
extern int dli_addrset_add(struct dli_ifnet *, const struct dli_filter *, struct dli_addrset *, const union dli_addr *);
extern int dli_addrset_test(struct dli_ifnet *, const struct dli_filter *, struct dli_addrset *, const union dli_addr *);
extern void dli_addrset_remove_int(struct dli_ifnet *, struct dli_addrset *, struct dli_mcast *, const struct dli_fagamap *);
extern int dli_addrset_remove(struct dli_ifnet *, const struct dli_filter *, struct dli_addrset *, const union dli_addr *);
extern void dli_addrset_destroy(struct dli_ifnet *, const struct dli_filter *, struct dli_addrset *);
extern struct dli_ifnet *dli_devid2dlif(struct dli_devid *);
#ifndef dli_ifp2dlif
extern struct dli_ifnet *dli_ifp2dlif(struct ifnet *);
#endif
extern struct mbuf *dli_trim_etherpad(struct mbuf *);
extern const struct dli_fagamap *dli_addr_remap(struct dli_ifnet *, const struct dli_filter *, const union dli_addr *, int);
extern int dli_rcv_and_drop(DLI_FILTER_MATCH_ARGS);

/* dli_timers.c */

extern void dli_slowtimo(void);
extern void dli_snd_sysid(struct dli_ifnet *, struct dli_filter *, int, const union dli_addr *);

/* dli_usrreq.c */

extern int dli_usrreq(struct socket *so, int, struct mbuf *, struct mbuf *, struct mbuf *);
extern int dli_ctloutput(int, struct socket *, long, long, struct mbuf **);
extern int dli_control(struct dli_pcb *, unsigned int, caddr_t, struct dli_ifnet *);
extern int dli_validate_addr(struct mbuf *);

/*
 * these functions are present only if the DNxDLI (DLIMGT) subset is installed
 */
extern int dlimgt_openport(struct dli_ifnet *, struct dli_filter *);
extern void dlimgt_closeport(struct dli_ifnet *, struct dli_filter *);
extern int dlimgt_prco_pif(int, struct socket *, int, int, struct mbuf **);
extern int dlimgt_control(struct dli_pcb *, u_int, caddr_t, struct dli_ifnet *, int *);
extern void dlimgt_sysid(struct dli_ifnet *, int, struct mbuf *);
extern int dlimgt_log_unrecog_pdu(struct dli_recv *, struct mbuf *);
extern struct mbuf *dlimgt_get_dlctrs(struct dli_ifnet *, int);

struct dlimgt_routines {
    /* these function pointers are nonNULL only if the NETMAN subset is installed */
   int (*mgt_openport)(struct dli_ifnet *, struct dli_filter *);
   void (*mgt_closeport)(struct dli_ifnet *, struct dli_filter *);
   int (*mgt_control)(struct dli_pcb *, u_int, caddr_t, struct dli_ifnet *, int *);
   int (*mgt_prco_pif)(int, struct socket *, int, int, struct mbuf **);
   void (*mgt_sysid_extra)(struct dli_ifnet *, int, struct mbuf *);
   int (*mgt_log_unrecog_pdu)(struct dli_recv *, struct mbuf *);
   struct mbuf *(*mgt_get_dlctrs)(struct dli_ifnet *, int);
};

#endif /* _DLI_DLI_KERNEL_H */

