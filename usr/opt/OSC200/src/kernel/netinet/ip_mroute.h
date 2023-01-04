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
/*
 * @(#)$RCSfile: ip_mroute.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/06/29 18:12:22 $
 */
/*
 * Definitions for the kernel part of DVMRP,
 * a Distance-Vector Multicast Routing Protocol.
 * (See RFC-1075.)
 *
 * Written by David Waitzman, BBN Labs, August 1988.
 * Modified by Steve Deering, Stanford, February 1989.
 *
 * MROUTING 1.1
 */

#ifndef _IP_MROUTE_H_
#define _IP_MROUTE_H_

/*
 * DVMRP-specific setsockopt commands.
 */
#define DVMRP_INIT	100
#define DVMRP_DONE	101
#define DVMRP_ADD_VIF	102
#define DVMRP_DEL_VIF	103
#define DVMRP_ADD_LGRP	104
#define DVMRP_DEL_LGRP	105
#define DVMRP_ADD_MRT	106
#define DVMRP_DEL_MRT	107


/*
 * Types and macros for handling bitmaps with one bit per virtual interface.
 */
#define MAXVIFS 32
typedef u_int vifbitmap_t;
typedef u_short vifi_t;		/* type of a vif index */

#define	VIFM_SET(n, m)		((m) |=  (1 << (n)))
#define	VIFM_CLR(n, m)		((m) &= ~(1 << (n)))
#define	VIFM_ISSET(n, m)	((m) &   (1 << (n)))
#define VIFM_CLRALL(m)		((m) = 0x00000000)
#define VIFM_COPY(mfrom, mto)	((mto) = (mfrom))
#define VIFM_SAME(m1, m2)	((m1) == (m2))


/*
 * Agument structure for DVMRP_ADD_VIF.
 * (DVMRP_DEL_VIF takes a single vifi_t argument.)
 */
struct vifctl {
    vifi_t	    vifc_vifi;	    /* the index of the vif to be added   */
    u_char	    vifc_flags;     /* VIFF_ flags defined below          */
    u_char	    vifc_threshold; /* min ttl required to forward on vif */
    struct in_addr  vifc_lcl_addr;  /* local interface address            */
    struct in_addr  vifc_rmt_addr;  /* remote address (tunnels only)      */
};

#define VIFF_TUNNEL	  0x1	    /* vif represents a tunnel end-point */
#define VIFF_SRCRT	  0x2	    /* tunnel uses IP src routing        */


/*
 * Argument structure for DVMRP_ADD_LGRP and DVMRP_DEL_LGRP.
 */
struct lgrplctl {
    vifi_t         lgc_vifi;
    struct in_addr lgc_gaddr;
};


/*
 * Argument structure for DVMRP_ADD_MRT.
 * (DVMRP_DEL_MRT takes a single struct in_addr argument, containing origin.)
 */
struct mrtctl {
    struct in_addr  mrtc_origin;	/* subnet origin of multicasts      */
    struct in_addr  mrtc_originmask;	/* subnet mask for origin           */
    vifi_t	    mrtc_parent;    	/* incoming vif                     */
    vifbitmap_t	    mrtc_children;	/* outgoing children vifs           */
    vifbitmap_t	    mrtc_leaves;	/* subset of outgoing children vifs */
};



#ifdef _KERNEL

/*
 * The kernel's virtual-interface structure.
 */
struct vif {
    u_char	   v_flags;         /* VIFF_ flags defined above           */
    u_char	   v_threshold;	    /* min ttl required to forward on vif  */
    struct in_addr v_lcl_addr;      /* local interface address             */
    struct in_addr v_rmt_addr;      /* remote address (tunnels only)       */
    struct ifnet  *v_ifp;	    /* pointer to interface                */
    struct mbuf	  *v_lcl_groups;    /* list of local groups (phyints only) */
    u_int	   v_cached_group;  /* last group looked-up (phyints only) */
    int		   v_cached_result; /* last look-up result  (phyints only) */
};

#define GRPLSTLEN (MLEN/4)	/* number of groups in a grplst */

struct grplst {
    struct in_addr gl_gaddr[GRPLSTLEN];
};


/*
 * The kernel's multicast route structure.
 */
struct mrt {
    struct in_addr  mrt_origin;		/* subnet origin of multicasts      */
    struct in_addr  mrt_originmask;	/* subnet mask for origin           */
    vifi_t	    mrt_parent;    	/* incoming vif                     */
    vifbitmap_t	    mrt_children;	/* outgoing children vifs           */
    vifbitmap_t	    mrt_leaves;		/* subset of outgoing children vifs */
};


#define MRTHASHSIZ	256
#if (MRTHASHSIZ & (MRTHASHSIZ - 1)) == 0
#define MRTHASHMOD(h)	((h) & (MRTHASHSIZ - 1))
#else
#define MRTHASHMOD(h)	((h) % MRTHASHSIZ)
#endif


/*
 * The kernel's multicast routing statistics.
 */
struct mrtstat {
    u_long	mrts_mrt_lookups;	/* # multicast route lookups       */
    u_long	mrts_mrt_misses;	/* # multicast route cache misses  */
    u_long	mrts_grp_lookups;	/* # group address lookups         */
    u_long	mrts_grp_misses;	/* # group address cache misses    */
    u_long	mrts_no_route;		/* no route for packet's origin    */
    u_long	mrts_bad_tunnel;	/* malformed tunnel options        */
    u_long	mrts_cant_tunnel;	/* no room for tunnel options      */
    u_long	mrts_wrong_if;		/* arrived on wrong interface	   */
};


#endif /* _KERNEL */


#endif
