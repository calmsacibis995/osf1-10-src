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
static char *rcsid = "@(#)$RCSfile: ip_mroute.c,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1994/01/11 22:43:47 $";
#endif
/*
 * Procedures for the kernel part of DVMRP,
 * a Distance-Vector Multicast Routing Protocol.
 * (See RFC-1075.)
 *
 * Written by David Waitzman, BBN Labs, August 1988.
 * Modified by Steve Deering, Stanford, February 1989.
 *
 * MROUTING 1.3
 */

#include "machine/endian.h"
#include "net/net_globals.h"
#include "sys/param.h"
#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/protosw.h"
#include "sys/errno.h"
#include "sys/time.h"
#include "sys/ioctl.h"
#include "sys/syslog.h"
#include "net/if.h"
#include "net/route.h"
#include "net/raw_cb.h"
#include "netinet/in.h"
#include "netinet/in_systm.h"
#include "netinet/ip.h"
#include "netinet/in_pcb.h"
#include "netinet/in_var.h"
#include "netinet/ip_var.h"
#include "netinet/igmp.h"
#include "netinet/igmp_var.h"
#include "netinet/ip_mroute.h"
#include "net/net_malloc.h"

#define MROUTING
#ifndef MROUTING
/*
 * Dummy routines and globals used when multicast routing is not compiled in.
 */

struct socket  *ip_mrouter  = NULL;
u_int		ip_mrtproto = 0;

int
ip_mrouter_cmd(cmd, so, m)
	int cmd;
	struct socket *so;
	struct mbuf *m;
{
	return(EOPNOTSUPP);
}

int
ip_mrouter_done()
{
	return(0);
}

int
ip_mforward(m, ifp)
	struct mbuf *m;
	struct ifnet *ifp;
{
	return(0);
}
#else /* MROUTING */

static void phyint_send();
static void srcrt_send();
static void encap_send();

void multiencap_decap();

#define INSIZ		sizeof(struct in_addr)
#define	same(a1, a2) \
	(bcmp((caddr_t)(a1), (caddr_t)(a2), INSIZ) == 0)

/*
 * Globals.  All but ip_mrouter and ip_mrtproto could be static,
 * except for netstat or debugging purposes.
 */
struct socket  *ip_mrouter  = NULL;
int		ip_mrtproto = IGMP_DVMRP;    /* for netstat only */

struct mbuf    *mrttable[MRTHASHSIZ];
struct vif	viftable[MAXVIFS];
struct mrtstat	mrtstat;
u_int		mrtdebug = 0;	  /* debug level */

/*
 * 'Interface' associated with decapsulator (so we can tell
 * packets that went through it from ones that get reflected
 * by a broken gateway).  This interface is never linked into
 * the system ifnet list & no routes point to it.  I.e., packets
 * can't be sent this way.  It only exists as a placeholder for
 * multicast source verification.
 */
struct ifnet multicast_decap_if = {
	0,
	"mdecap",
	0,		/* unit */
};

#define ENCAP_TTL 64
#define ENCAP_PROTO 4

/* prototype IP hdr for encapsulated packets */
struct ip multicast_encap_iphdr = {
	(IPVERSION << 4) | sizeof(struct ip) >> 2,
	0,				/* tos */
	sizeof(struct ip),		/* total length */
	0,				/* id */
	0,				/* frag offset */
	ENCAP_TTL, ENCAP_PROTO,	
	0,				/* checksum */
};

/*
 * Private variables.
 */
static vifi_t	   numvifs = 0;
static void (*encap_oldrawip)();

/*
 * one-back cache used by multiencap_decap to locate a tunnel's vif
 * given a datagram's src ip address.
 */
static u_int last_encap_src;
static struct vif *last_encap_vif;

/*
 * A simple hash function: returns MRTHASHMOD of the low-order octet of
 * the argument's network or subnet number.
 */ 
static u_int
nethash(n)
    register u_int n;
{
    struct in_addr in;

    in.s_addr = n;
    n = in_netof(in);
    while ((n & 0xff) == 0)
	n >>= 8;
    return (MRTHASHMOD(n));
}

/*
 * this is a direct-mapped cache used to speed the mapping from a
 * datagram source address to the associated multicast route.  Note
 * that unlike mrttable, the hash is on IP address, not IP net number.
 */
#define MSRCHASHSIZ 1024
#define MSRCHASH(a) ((((a) >> 20) ^ ((a) >> 10) ^ (a)) & (MSRCHASHSIZ - 1))
struct mrt *mrtsrchash[MSRCHASHSIZ];

/*
 * Find a route for a given origin IP address.
 */
#define MRTFIND(o, rt) { \
	register u_int _mrhash = o; \
	_mrhash = MSRCHASH(_mrhash); \
	++mrtstat.mrts_mrt_lookups; \
	rt = mrtsrchash[_mrhash]; \
	if (rt == NULL || \
	    (o & rt->mrt_originmask.s_addr) != rt->mrt_origin.s_addr) \
		if ((rt = mrtfind(o)) != NULL) \
		    mrtsrchash[_mrhash] = rt; \
}

static struct mrt *
mrtfind(origin)
    u_int origin;
{
    register struct mbuf *mb_rt;
    register struct mrt *rt;
    register u_int hash;

    mrtstat.mrts_mrt_misses++;

    hash = nethash(origin);
    for (mb_rt = mrttable[hash]; mb_rt; mb_rt = mb_rt->m_next) {
	rt = mtod(mb_rt, struct mrt *);
	if ((origin & rt->mrt_originmask.s_addr) == rt->mrt_origin.s_addr)
	    return (rt);
    }
    return NULL;
}

/*
 * Handle DVMRP setsockopt commands to modify the multicast routing tables.
 */
int
ip_mrouter_cmd(cmd, so, m)
    int cmd;
    struct socket *so;
    struct mbuf *m;
{
    register int error = 0;

    if (cmd != DVMRP_INIT && so != ip_mrouter)
		error = EACCES;
    else switch (cmd) {
	case DVMRP_INIT:
		error = ip_mrouter_init(so);
		break;
	case DVMRP_DONE:
		error = ip_mrouter_done();
		break;
	case DVMRP_ADD_VIF:
		if (m == NULL || m->m_len < sizeof(struct vifctl))
			error = EINVAL;
		else
			error = add_vif(mtod(m, struct vifctl *));
		break;
	case DVMRP_DEL_VIF:
		if (m == NULL || m->m_len < sizeof(vifi_t))
			error = EINVAL;
		else
			error =  del_vif(mtod(m, vifi_t *));
		break;
	case DVMRP_ADD_LGRP:
		if (m == NULL || m->m_len < sizeof(struct lgrplctl))
			error = EINVAL;
		else
			error = add_lgrp(mtod(m, struct lgrplctl *));
		break;
	case DVMRP_DEL_LGRP:
		if (m == NULL || m->m_len < sizeof(struct lgrplctl))
			error = EINVAL;
		else
			error =  del_lgrp(mtod(m, struct lgrplctl *));
		break;
	case DVMRP_ADD_MRT:
		if (m == NULL || m->m_len < sizeof(struct mrtctl))
			error = EINVAL;
		else
			error = add_mrt(mtod(m, struct mrtctl *));
		break;
	case DVMRP_DEL_MRT:
		if (m == NULL || m->m_len < sizeof(struct in_addr))
			error = EINVAL;
		else
			error = del_mrt(mtod(m, struct mrtctl *));
		break;
	default:
		error = EOPNOTSUPP;
		break;
    }
    return (error);
}

/*
 * Enable multicast routing
 */
int
ip_mrouter_init(so)
    struct socket *so;
{
    if (so->so_type != SOCK_RAW ||
	so->so_proto->pr_protocol != IPPROTO_IGMP)
	    return EOPNOTSUPP;

    if (ip_mrouter != NULL)
	    return EADDRINUSE;

    ip_mrouter = so;

    if (mrtdebug)
	log(LOG_DEBUG, "ip_mrouter_init");

    return 0;
}

/*
 * Disable multicast routing
 */
int
ip_mrouter_done()
{
    vifi_t vifi;
    int i;
    struct ifnet *ifp;
    struct ifreq ifr;
    int s;

    s = splnet();

    /*
     * For each phyint in use, free its local group list and
     * disable promiscuous reception of all IP multicasts.
     */
    for (vifi = 0; vifi < numvifs; vifi++) {
	if (viftable[vifi].v_lcl_addr.s_addr != 0 &&
	    !(viftable[vifi].v_flags & VIFF_TUNNEL)) {
		if (viftable[vifi].v_lcl_groups)
		    m_freem(viftable[vifi].v_lcl_groups);
	        ((struct sockaddr_in *)&(ifr.ifr_addr))->sin_family = AF_INET;
	        ((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr.s_addr
								= INADDR_ANY;
	        ifp = viftable[vifi].v_ifp;
	        if_delmulti(ifp, (caddr_t)&ifr);
	}
    }
    bzero((caddr_t)viftable, sizeof(viftable));
    numvifs = 0;

    /*
     * Free any multicast route entries.
     */
    for (i = 0; i < MRTHASHSIZ; i++)
        m_freem(mrttable[i]);
    bzero((caddr_t)mrttable, sizeof(mrttable));
    bzero((caddr_t)mrtsrchash, sizeof(mrtsrchash));

    ip_mrouter = NULL;

    splx(s);

    if (mrtdebug)
	log(LOG_DEBUG, "ip_mrouter_done");

    return 0;
}

/*
 * Add a vif to the vif table
 */
int
add_vif(vifcp)
    register struct vifctl *vifcp;
{
    register struct vif *vifp;
    static struct sockaddr_in sin = { sizeof(sin),AF_INET };
    struct ifaddr *ifa;
    struct ifnet *ifp;
    struct ifreq ifr;
    int error, s;

    if (vifcp->vifc_vifi >= MAXVIFS)
	return EINVAL;

    vifp = viftable + vifcp->vifc_vifi;
    if (vifp->v_lcl_addr.s_addr != 0)
	return EADDRINUSE;

    /* Find the interface with an address in AF_INET family */
    sin.sin_addr = vifcp->vifc_lcl_addr;
    ifa = ifa_ifwithaddr(&sin);
    if (ifa == 0)
	return EADDRNOTAVAIL;
    ifp = ifa->ifa_ifp;

    if (vifcp->vifc_flags & VIFF_TUNNEL) {
	if ((vifcp->vifc_flags & VIFF_SRCRT) == 0) {
	    /*
	     * An encapsulating tunnel is wanted.  If we haven't done so
	     * already, put our decap routine in front of raw_input so we
	     * have a chance to decapsulate incoming packets.  Then set
	     * the arrival 'interface' to be the decapsulator.
	     */
	    if (encap_oldrawip == 0) {
		extern u_char ip_protox[];
		register int pr = ip_protox[ENCAP_PROTO];

		encap_oldrawip = inetsw[pr].pr_input;
		inetsw[pr].pr_input = multiencap_decap;
	    }
	    ifp = &multicast_decap_if;
	} else {
	    ifp = 0;
	}
    } else {
	/* Make sure the interface supports multicast */
	if ((ifp->if_flags & IFF_MULTICAST) == 0)
	    return EOPNOTSUPP;

	/* Enable promiscuous reception of all IP multicasts from the if */
	((struct sockaddr_in *)&(ifr.ifr_addr))->sin_family = AF_INET;
	((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr.s_addr = INADDR_ANY;
	s = splnet();
	error = if_addmulti(ifp, (caddr_t)&ifr);
	splx(s);
	if (error)
	    return error;
    }

    vifp->v_flags     = vifcp->vifc_flags;
    vifp->v_threshold = vifcp->vifc_threshold;
    vifp->v_lcl_addr  = vifcp->vifc_lcl_addr;
    vifp->v_rmt_addr  = vifcp->vifc_rmt_addr;
    vifp->v_ifp       = ifp;

    /* Adjust numvifs up if the vifi is higher than numvifs */
    if (numvifs <= vifcp->vifc_vifi)
	numvifs = vifcp->vifc_vifi + 1;

    if (mrtdebug)
	log(LOG_DEBUG, "add_vif #%d, lcladdr %x, %s %x, thresh %x",
	    vifcp->vifc_vifi, 
	    ntohl(vifcp->vifc_lcl_addr.s_addr),
	    (vifcp->vifc_flags & VIFF_TUNNEL) ? "rmtaddr" : "mask",
	    ntohl(vifcp->vifc_rmt_addr.s_addr),
	    vifcp->vifc_threshold);
    
    return 0;
}

/*
 * Delete a vif from the vif table
 */
int
del_vif(vifip)
    vifi_t *vifip;
{
    register struct vif *vifp;
    register vifi_t vifi;
    struct ifnet *ifp;
    struct ifreq ifr;
    int s;

    if (*vifip >= numvifs)
	return EINVAL;

    vifp = viftable + *vifip;
    if (vifp->v_lcl_addr.s_addr == 0)
	return EADDRNOTAVAIL;

    s = splnet();

    if (!(vifp->v_flags & VIFF_TUNNEL)) {
	if (vifp->v_lcl_groups)
		m_freem(vifp->v_lcl_groups);
	((struct sockaddr_in *)&(ifr.ifr_addr))->sin_family = AF_INET;
	((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr.s_addr = INADDR_ANY;
	ifp = vifp->v_ifp;
	if_delmulti(ifp, (caddr_t)&ifr);
    }

    if (vifp == last_encap_vif) {
	last_encap_vif = 0;
	last_encap_src = 0;
    }
    bzero((caddr_t)vifp, sizeof (*vifp));

    /* Adjust numvifs down */
    for (vifi = numvifs; vifi > 0; vifi--)
      if (viftable[vifi-1].v_lcl_addr.s_addr != 0)
	break;
    numvifs = vifi;

    splx(s);

    if (mrtdebug)
      log(LOG_DEBUG, "del_vif %d, numvifs %d", *vifip, numvifs);

    return 0;
}

/*
 * Add the multicast group in the lgrpctl to the list of local multicast
 * group memberships associated with the vif indexed by gcp->lgc_vifi.
 */
int
add_lgrp(gcp)
    register struct lgrplctl *gcp;
{
    register struct vif *vifp;
    register struct mbuf *lp, *prev_lp;
    int s;

    if (mrtdebug)
      log(LOG_DEBUG,"add_lgrp %x on %d",
	  ntohl(gcp->lgc_gaddr.s_addr), gcp->lgc_vifi);

    if (gcp->lgc_vifi >= numvifs)
	return EINVAL;

    vifp = viftable + gcp->lgc_vifi;
    if (vifp->v_lcl_addr.s_addr == 0 || (vifp->v_flags & VIFF_TUNNEL))
	return EADDRNOTAVAIL;

    /* try to find a group list mbuf that has free space left */
    for (lp = vifp->v_lcl_groups, prev_lp = NULL
	 ; lp && (lp->m_len / INSIZ) == GRPLSTLEN
         ; prev_lp = lp, lp = lp->m_next)
      ;

    s = splnet();

    if (lp == NULL) {	    /* no group list mbuf with free space was found */
	MGET(lp, M_DONTWAIT, M_MRTABLE);
	if (lp == NULL) {
	  splx(s);
	  return ENOBUFS;
	}
	if (prev_lp == NULL)
	  vifp->v_lcl_groups = lp;
	else
	  prev_lp->m_next = lp;
	lp->m_len = 0;
    }

    (mtod(lp,struct grplst *)->gl_gaddr)[lp->m_len / INSIZ].s_addr
      = gcp->lgc_gaddr.s_addr;
    lp->m_len += INSIZ;

    if (gcp->lgc_gaddr.s_addr == vifp->v_cached_group)
	vifp->v_cached_result = 1;

    splx(s);

    return 0;
}

/*
 * Delete the the local multicast group associated with the vif
 * indexed by gcp->lgc_vifi.
 * Does not pullup the values from other mbufs in the list unless the
 * current mbuf is totally empty.  This is a ~bug.
 */

int
del_lgrp(gcp)
    register struct lgrplctl *gcp;
{
    register struct vif *vifp;
    register u_long i;
    register struct mbuf *lp, *next_lp, *prev_lp = NULL;
    register struct grplst *glp;
    int cnt, s;

    if (mrtdebug)
      log(LOG_DEBUG,"del_lgrp %x on %d",
	  ntohl(gcp->lgc_gaddr.s_addr), gcp->lgc_vifi);

    if (gcp->lgc_vifi >= numvifs)
	return EINVAL;

    vifp = viftable + gcp->lgc_vifi;
    if (vifp->v_lcl_addr.s_addr == 0 || (vifp->v_flags & VIFF_TUNNEL))
	return EADDRNOTAVAIL;

    s = splnet();

    if (gcp->lgc_gaddr.s_addr == vifp->v_cached_group)
	vifp->v_cached_result = 0;

    /* for all group list mbufs */
    for (lp = vifp->v_lcl_groups; lp; lp = next_lp) {
	/* for all group addrs in an mbuf */
	for (cnt = lp->m_len / INSIZ, i = 0; i < cnt; i++)
	  /* if this is the addr to delete */
	  if (same(&gcp->lgc_gaddr,
		   &mtod(lp,struct grplst *)->gl_gaddr[i])) {
	      lp->m_len -= INSIZ;
	      cnt--;       
	      if (lp->m_len == 0)  {
		  /* the mbuf is now empty */
		  if (prev_lp) {
		      MFREE(lp, prev_lp->m_next);
		  } else {
		      MFREE(lp, vifp->v_lcl_groups);
		  }
	      } else
		/* move all other group addresses down one address */
		/* djw- could use ovbcopy? */
		for (glp = mtod(lp, struct grplst *); i < cnt; i++)
		  glp->gl_gaddr[i] = glp->gl_gaddr[i + 1];
	      splx(s);
	      return 0;
	  }
	prev_lp = lp;
	next_lp = lp->m_next;
    }
    splx(s);
    return EADDRNOTAVAIL;		/* not found */
}

/*
 * Return 1 if gaddr is a member of the local group list for vifp.
 */
static int
grplst_member(vifp, gaddr)
    struct vif *vifp;
    struct in_addr gaddr;
{
    register int i;
    register u_int addr;
    register struct in_addr *gl;
    register struct mbuf *mb_gl;
    int s;

    mrtstat.mrts_grp_lookups++;

    addr = gaddr.s_addr;
    if (addr == vifp->v_cached_group)
	return (vifp->v_cached_result);

    mrtstat.mrts_grp_misses++;

    for (mb_gl = vifp->v_lcl_groups; mb_gl; mb_gl = mb_gl->m_next) {
      for (gl = mtod(mb_gl, struct in_addr *), i = mb_gl->m_len / INSIZ;
	   i; gl++, i--) {
	if (addr == gl->s_addr) {
	  vifp->v_cached_group  = addr;
	  vifp->v_cached_result = 1;
	  return 1;
	}
      }
    }
    vifp->v_cached_group  = addr;
    vifp->v_cached_result = 0;
    return 0;
}

/*
 * Add an mrt entry
 */
int
add_mrt(mrtcp)
    struct mrtctl *mrtcp;
{
    struct mrt *rt;
    struct mbuf *mb_rt;
    u_int hash;
    int s;

    rt = mrtfind(mrtcp->mrtc_origin.s_addr);
    if (rt) {
	if (mrtdebug)
	  log(LOG_DEBUG,"add_mrt update o %x m %x p %x c %x l %x",
	      ntohl(mrtcp->mrtc_origin.s_addr),
	      ntohl(mrtcp->mrtc_originmask.s_addr),
	      mrtcp->mrtc_parent, mrtcp->mrtc_children, mrtcp->mrtc_leaves);

	/* Just update the route */
	s = splnet();
	rt->mrt_parent = mrtcp->mrtc_parent;
	VIFM_COPY(mrtcp->mrtc_children, rt->mrt_children);
	VIFM_COPY(mrtcp->mrtc_leaves,   rt->mrt_leaves);
	splx(s);
	return 0;
    }

    if (mrtdebug)
      log(LOG_DEBUG,"add_mrt o %x m %x p %x c %x l %x",
	  ntohl(mrtcp->mrtc_origin.s_addr),
	  ntohl(mrtcp->mrtc_originmask.s_addr),
	  mrtcp->mrtc_parent, mrtcp->mrtc_children, mrtcp->mrtc_leaves);

    s = splnet();

    MGET(mb_rt, M_DONTWAIT, M_MRTABLE);
    if (mb_rt == 0) {
	splx(s);
	return ENOBUFS;
    }
    rt = mtod(mb_rt, struct mrt *);

    /*
     * insert new entry at head of hash chain
     */
    rt->mrt_origin     = mrtcp->mrtc_origin;
    rt->mrt_originmask = mrtcp->mrtc_originmask;
    rt->mrt_parent     = mrtcp->mrtc_parent;
    VIFM_COPY(mrtcp->mrtc_children, rt->mrt_children); 
    VIFM_COPY(mrtcp->mrtc_leaves,   rt->mrt_leaves);     
    /* link into table */
    hash = nethash(mrtcp->mrtc_origin.s_addr);
    mb_rt->m_next  = mrttable[hash];
    mrttable[hash] = mb_rt;

    splx(s);

    return 0;
}

/*
 * Delete an mrt entry
 */
int
del_mrt(origin)
    struct in_addr *origin;
{
    register struct mrt *rt;
    struct mbuf *mb_rt, *prev_mb_rt;
    register u_int hash = nethash(origin->s_addr);
    register struct mrt **cmrt, **cmrtend;
    int s;

    if (mrtdebug)
      log(LOG_DEBUG,"del_mrt orig %x",
	  ntohl(origin->s_addr));

    for (prev_mb_rt = mb_rt = mrttable[hash]
	 ; mb_rt
	 ; prev_mb_rt = mb_rt, mb_rt = mb_rt->m_next) {
        rt = mtod(mb_rt, struct mrt *);
	if (origin->s_addr == rt->mrt_origin.s_addr)
	    break;
    }
    if (!mb_rt) {
	return ESRCH;
    }

    s = splnet();

    cmrt = mrtsrchash;
    cmrtend = cmrt + MSRCHASHSIZ;
    for ( ; cmrt < cmrtend; ++cmrt)
	if (*cmrt == rt)
		*cmrt = 0;

    if (prev_mb_rt != mb_rt) {	/* if moved past head of list */
	MFREE(mb_rt, prev_mb_rt->m_next);
    } else			/* delete head of list, it is in the table */
        mrttable[hash] = m_free(mb_rt);

    splx(s);

    return 0;
}

/*
 * IP multicast forwarding function. This function assumes that the packet
 * pointed to by "ip" has arrived on (or is about to be sent to) the interface
 * pointed to by "ifp", and the packet is to be relayed to other networks
 * that have members of the packet's destination IP multicast group.
 *
 * The packet is returned unscathed to the caller, unless it is tunneled
 * or erroneous, in which case a non-zero return value tells the caller to
 * discard it.
 */

#define IP_HDR_LEN  20	/* # bytes of fixed IP header (excluding options) */
#define TUNNEL_LEN  12  /* # bytes of IP option for tunnel encapsulation  */

int
ip_mforward(m, ifp)
    struct mbuf *m;
    struct ifnet *ifp;
{
    register struct ip *ip = mtod(m, struct ip *);
    register struct mrt *rt;
    register struct vif *vifp;
    register int vifi;
    register u_char *ipoptions;
    u_int tunnel_src;

    if (mrtdebug > 1)
      log(LOG_DEBUG, "ip_mforward: src %x, dst %x, ifp %x",
	  ntohl(ip->ip_src.s_addr), ntohl(ip->ip_dst.s_addr), ifp);

    if ((ip->ip_vhl & 0x0f) < (IP_HDR_LEN + TUNNEL_LEN) >> 2 ||
       (ipoptions = (u_char *)(ip + 1))[1] != IPOPT_LSRR ) {
	/*
	 * Packet arrived via a physical interface.
	 */
	tunnel_src = 0;
    } else {
	/*
	 * Packet arrived through a source-route tunnel.
	 *
	 * A source-route tunneled packet has a single NOP option and a
	 * two-element
	 * loose-source-and-record-route (LSRR) option immediately following
	 * the fixed-size part of the IP header.  At this point in processing,
	 * the IP header should contain the following IP addresses:
	 *
	 *	original source          - in the source address field
	 *	destination group        - in the destination address field
	 *	remote tunnel end-point  - in the first  element of LSRR
	 *	one of this host's addrs - in the second element of LSRR
	 *
	 * NOTE: RFC-1075 would have the original source and remote tunnel
	 *	 end-point addresses swapped.  However, that could cause
	 *	 delivery of ICMP error messages to innocent applications
	 *	 on intermediate routing hosts!  Therefore, we hereby
	 *	 change the spec.
	 */

	/*
	 * Verify that the tunnel options are well-formed.
	 */
	if (ipoptions[0] != IPOPT_NOP ||
	    ipoptions[2] != 11 ||	/* LSRR option length   */
	    ipoptions[3] != 12 ||	/* LSRR address pointer */
	    (tunnel_src = *(u_int *)(&ipoptions[4])) == 0) {
	    mrtstat.mrts_bad_tunnel++;
	    if (mrtdebug)
		log(LOG_DEBUG,
		"ip_mforward: bad tunnel from %u (%x %x %x %x %x %x)",
		ntohl(ip->ip_src.s_addr),
		ipoptions[0], ipoptions[1], ipoptions[2], ipoptions[3],
		*(u_int *)(&ipoptions[4]), *(u_int *)(&ipoptions[8]));
	    return 1;
	}

	/*
	 * Delete the tunnel options from the packet.
	 */
  	ovbcopy((caddr_t)(ipoptions + TUNNEL_LEN), (caddr_t)ipoptions,
	      (unsigned)(m->m_len - (IP_HDR_LEN + TUNNEL_LEN)));
	m->m_len   -= TUNNEL_LEN;
	m->m_pkthdr.len   -= TUNNEL_LEN;
	ip->ip_len -= TUNNEL_LEN;
	ip->ip_vhl = (ip->ip_vhl & 0xf0) | 
		((ip->ip_vhl & 0x0f) - (TUNNEL_LEN >> 2));
	ifp = 0;
    }

    /*
     * Don't forward a packet with time-to-live of zero or one,
     * or a packet destined to a local-only group.
     */
    if (ip->ip_ttl <= 1 ||
	ntohl(ip->ip_dst.s_addr) <= INADDR_MAX_LOCAL_GROUP)
	return (int)tunnel_src;

    /*
     * Don't forward if we don't have a route for the packet's origin.
     */
    MRTFIND(ip->ip_src.s_addr, rt)
    if (rt == NULL) {
	mrtstat.mrts_no_route++;
	if (mrtdebug)
	    log(LOG_DEBUG, "ip_mforward: no route for %u",
				ntohl(ip->ip_src.s_addr));
	return (int)tunnel_src;
    }

    /*
     * Don't forward if it didn't arrive from the parent vif for its origin.
     * Notes: v_ifp is zero for src route tunnels, multicast_decap_if
     * for encapsulated tunnels and a real ifnet for non-tunnels so
     * the first part of the if catches wrong physical interface or
     * tunnel type; v_rmt_addr is zero for non-tunneled packets so
     * the 2nd part catches both packets that arrive via a tunnel
     * that shouldn't and packets that arrive via the wrong tunnel.
     */
    vifi = rt->mrt_parent;
    if (viftable[vifi].v_ifp != ifp ||
	(ifp == 0 && viftable[vifi].v_rmt_addr.s_addr != tunnel_src)) {
	/* came in the wrong interface */
	++mrtstat.mrts_wrong_if;
	return (int)tunnel_src;
    }

    /*
     * For each vif, decide if a copy of the packet should be forwarded.
     * Forward if:
     *		- the ttl exceeds the vif's threshold AND
     *		- the vif is a child in the origin's route AND
     *		- ( the vif is not a leaf in the origin's route OR
     *		    the destination group has members on the vif )
     *
     * (This might be speeded up with some sort of cache -- someday.)
     */
    for (vifp = viftable, vifi = 0; vifi < numvifs; vifp++, vifi++) {
	if (ip->ip_ttl > vifp->v_threshold &&
	    VIFM_ISSET(vifi, rt->mrt_children) &&
	    (!VIFM_ISSET(vifi, rt->mrt_leaves) ||
	     grplst_member(vifp, ip->ip_dst))) {
		if (vifp->v_flags & VIFF_SRCRT)
		    srcrt_send(ip, vifp, m);
		else if (vifp->v_flags & VIFF_TUNNEL)
		    encap_send(ip, vifp, m);
		else
		    phyint_send(ip, vifp, m);
	}
    }

    return (int)tunnel_src;
}

static void
phyint_send(ip, vifp, m)
    struct ip *ip;
    struct vif *vifp;
    struct mbuf *m;
{
    register struct mbuf *mb_copy = 0;
    register struct ip_moptions *imo;
    int error;
    int len = ip->ip_len;

    if (m->m_flags & M_EXT) {
	/*
	 * need new header for ip output modification
	 */
	MGETHDR(mb_copy, M_DONTWAIT, MT_HEADER);
	if (mb_copy == NULL)
		return;
	mb_copy->m_len = 0;
	mb_copy->m_data += max_linkhdr;
	if ((mb_copy->m_next = m_copy(m, 0, M_COPYALL)) == NULL) {
		m_freem(mb_copy);
		return;
	}
	mb_copy->m_pkthdr.len = len;
	mb_copy = m_pullup_exact(mb_copy, sizeof(struct ip));
    } else
	mb_copy = m_copy(m, 0, M_COPYALL);

    if (mb_copy == NULL)
	return;

    NET_MALLOC(imo, struct ip_moptions *, sizeof *imo, M_IPMOPTS, M_NOWAIT);
    if (imo == NULL) {
	m_freem(mb_copy);
	return;
    }

    imo->imo_multicast_ifp  = vifp->v_ifp;
    imo->imo_multicast_ttl  = ip->ip_ttl - 1;
    imo->imo_multicast_loop = 1;

    error = ip_output(mb_copy, (struct mbuf *)0, (struct route *)0,
				IP_FORWARDING, imo);
    NET_FREE(imo, M_IPMOPTS);
    if (mrtdebug > 1)
	log(LOG_DEBUG, "phyint_send on vif %d err %d", vifp-viftable, error);
}

static void
srcrt_send(ip, vifp, m)
    struct ip *ip;
    struct vif *vifp;
    struct mbuf *m;
{
    struct mbuf *mb_copy, *mb_opts;
    register struct ip *ip_copy;
    int error;
    u_char *cp;

    /*
     * Make sure that adding the tunnel options won't exceed the
     * maximum allowed number of option bytes.
     */
    if ((ip->ip_vhl & 0x0f) > (60 - TUNNEL_LEN) >> 2) {
	mrtstat.mrts_cant_tunnel++;
	if (mrtdebug)
	    log(LOG_DEBUG, "srcrt_send: no room for tunnel options, from %u",
					ntohl(ip->ip_src.s_addr));
	return;
    }

    mb_copy = m_copy(m, 0, M_COPYALL);
    if (mb_copy == NULL)
      return;
    ip_copy = mtod(mb_copy, struct ip *);
    ip_copy->ip_ttl--;
    ip_copy->ip_dst = vifp->v_rmt_addr;	  /* remote tunnel end-point */
    /*
     * Adjust the ip header length to account for the tunnel options.
     */
    ip_copy->ip_vhl = (ip_copy->ip_vhl & 0xf0) | 
		((ip_copy->ip_vhl & 0x0f) + (TUNNEL_LEN >> 2));
    ip_copy->ip_len += TUNNEL_LEN;
    MGETHDR(mb_opts, M_DONTWAIT, MT_HEADER);
    if (mb_opts == NULL) {
	m_freem(mb_copy);
	return;
    }
    /*
     * 'Delete' the base ip header from the mb_copy chain
     */
    mb_copy->m_len -= IP_HDR_LEN;
    mb_copy->m_pkthdr.len -= IP_HDR_LEN;
    mb_copy->m_data += IP_HDR_LEN;
    /*
     * Make mb_opts be the new head of the packet chain.
     * Any options of the packet were left in the old packet chain head
     */
    mb_opts->m_next = mb_copy;
    mb_opts->m_data += 16;
    mb_opts->m_len = IP_HDR_LEN + TUNNEL_LEN;
    /*
     * Copy the base ip header from the mb_copy chain to the new head mbuf
     */
    bcopy((caddr_t)ip_copy, mtod(mb_opts, caddr_t), IP_HDR_LEN);
    /*
     * Add the NOP and LSRR after the base ip header
     */
    cp = mtod(mb_opts, u_char *) + IP_HDR_LEN;
    *cp++ = IPOPT_NOP;
    *cp++ = IPOPT_LSRR;
    *cp++ = 11; /* LSRR option length */
    *cp++ = 8;  /* LSSR pointer to second element */
    *(u_int*)cp = vifp->v_lcl_addr.s_addr;	/* local tunnel end-point */
    cp += 4;
    *(u_int*)cp = ip->ip_dst.s_addr;		/* destination group */

    error = ip_output(mb_opts, (struct mbuf *)0, (struct route *)0,
				IP_FORWARDING, (struct ip_moptions *)0);
    if (mrtdebug > 1)
	log(LOG_DEBUG, "srcrt_send on vif %d err %d", vifp-viftable, error);
}

static void
encap_send(ip, vifp, m)
	register struct ip *ip;
	register struct vif *vifp;
	register struct mbuf *m;
{
	register struct mbuf *mb_copy = 0;
	register struct ip *ip_copy;
	register int len = ip->ip_len;
	int i;

	/*
	 * copy the old packet & pullup it's IP header into the
	 * new mbuf so we can modify it.  
	 */
	MGETHDR(mb_copy, M_DONTWAIT, MT_HEADER);
	if (mb_copy == NULL)
		return;
	mb_copy->m_data += max_linkhdr;
	mb_copy->m_len = sizeof(multicast_encap_iphdr);
	mb_copy->m_pkthdr.len = sizeof(multicast_encap_iphdr);
	if ((mb_copy->m_next = m_copy(m, 0, M_COPYALL)) == NULL) {
		m_freem(mb_copy);
		return;
	}
	mb_copy->m_pkthdr.len += len;
	mb_copy = m_pullup_exact(mb_copy, 
			sizeof(struct ip)+sizeof(multicast_encap_iphdr));
	if (mb_copy == NULL)
		return;

	/*
	 * fill in the encapsulating IP header.
	 */
	ip_copy = mtod(mb_copy, struct ip *);
	*ip_copy = multicast_encap_iphdr;
	ip_copy->ip_id = htons(ip_id++);
	ip_copy->ip_len += len;
	ip_copy->ip_src = vifp->v_lcl_addr;
	ip_copy->ip_dst = vifp->v_rmt_addr;

	/*
	 * turn the encapsulated IP header back into a valid one.
	 */
	ip = (struct ip *)((caddr_t)ip_copy + sizeof(multicast_encap_iphdr));
	--ip->ip_ttl;
	ip->ip_len = htons(ip->ip_len);
	ip->ip_off = htons(ip->ip_off);
	ip->ip_sum = 0;
#ifdef LBL
	ip->ip_sum = ~oc_cksum((caddr_t)ip, ip->ip_hl << 2, 0);
#else
	mb_copy->m_data += sizeof(multicast_encap_iphdr);
	ip->ip_sum = in_cksum(mb_copy, (ip->ip_vhl & 0x0f) << 2);
	mb_copy->m_data -= sizeof(multicast_encap_iphdr);
#endif
	if (mrtdebug > 1)
		log(LOG_DEBUG, "encap_send src %x, dst %x, len %d",
		    ntohl(ip->ip_src.s_addr), ntohl(ip->ip_dst.s_addr),
		    ntohs(ip->ip_len));
	ip_output(mb_copy, (struct mbuf *)0, (struct route *)0,
		  IP_FORWARDING, (struct ip_moptions *)0);
}

/*
 * De-encapsulate a packet and feed it back through ip input (this
 * routine is called whenever IP gets a packet with proto type
 * ENCAP_PROTO and a local destination address).
 */
void
multiencap_decap(m, hlen)
	register struct mbuf *m;
	int hlen;
{
	register struct ip *ip = mtod(m, struct ip *);
	register int s;
	register struct ifqueue *ifq;
	register struct vif *vifp;

	if (ip->ip_p != ENCAP_PROTO) {
		(encap_oldrawip)(m, hlen);
		return;
	}
	/*
	 * dump the packet if it's not to a multicast destination or if
	 * we don't have an encapsulating tunnel with the source.
	 */
	if (! IN_MULTICAST(ntohl(((struct ip *)((char *)ip + hlen))->ip_dst.s_addr))) {
		++mrtstat.mrts_bad_tunnel;
		m_freem(m);
		return;
	}
	if (ip->ip_src.s_addr != last_encap_src) {
		register struct vif *vife;

		vifp = viftable;
		vife = vifp + numvifs;
		last_encap_src = ip->ip_src.s_addr;
		last_encap_vif = 0;
		for ( ; vifp < vife; ++vifp)
			if (vifp->v_rmt_addr.s_addr == ip->ip_src.s_addr) {
				if ((vifp->v_flags & (VIFF_TUNNEL|VIFF_SRCRT))
				    == VIFF_TUNNEL)
					last_encap_vif = vifp;
				break;
			}
	}
	if ((vifp = last_encap_vif) == 0) {
		mrtstat.mrts_cant_tunnel++;
		m_freem(m);
		if (mrtdebug)
			log(LOG_DEBUG, "multiencap: no tunnel with %u",
			    ntohl(ip->ip_src.s_addr));
		return;
	}
	m->m_pkthdr.len = ip->ip_len;
	m->m_data += hlen;
	m->m_len -= hlen;
	m->m_pkthdr.rcvif = vifp->v_ifp;

	ifq = &ipintrq;
	s = splimp();
	if (IF_QFULL(ifq)) {
		IF_DROP(ifq);
		m_freem(m);
	} else {
		IF_ENQUEUE(ifq, m);
		/*
		 * normally we would need a "netisr_input(NETISR_IP)"
		 * here but we were called by ip_input and it is going
		 * to loop back & try to dequeue the packet we just
		 * queued as soon as we return so we avoid the
		 * unnecessary software interrrupt.
		 */
	}
	splx(s);
}

#endif /* MROUTING */
