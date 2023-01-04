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
 *	@(#)$RCSfile: in_var.h,v $ $Revision: 4.2.5.4 $ (DEC) $Date: 1993/11/22 23:33:20 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1985, 1986 Regents of the University of California.
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
 *	Base:	in_var.h	7.4 (Berkeley) 4/22/89
 *	Merged:	in_var.h	7.6 (Berkeley) 6/28/90
 */

#ifndef _IN_VAR_H_
#define _IN_VAR_H_
/*
 * Interface address, Internet version.  One of these structures
 * is allocated for each interface with an Internet address.
 * The ifaddr structure contains the protocol-independent part
 * of the structure and is assumed to be first.
 */
struct in_ifaddr {
	struct	ifaddr ia_ifa;		/* protocol-independent info */
#define	ia_ifp		ia_ifa.ifa_ifp
#define ia_flags	ia_ifa.ifa_flags
					/* ia_{,sub}net{,mask} in host order */
	u_int	ia_net;			/* network number of interface */
	u_int	ia_netmask;		/* mask of net part */
	u_int	ia_subnet;		/* subnet number, including net */
	u_int	ia_subnetmask;		/* mask of subnet part */
	struct	in_addr ia_netbroadcast; /* to recognize net broadcasts */
	struct	in_ifaddr *ia_next;	/* next in list of internet addresses */
	struct	sockaddr_in ia_addr;	/* reserve space for interface name */
	struct	sockaddr_in ia_dstaddr; /* reserve space for broadcast addr */
#define	ia_broadaddr	ia_dstaddr
	struct	sockaddr_in ia_sockmask; /* reserve space for general netmask */
	struct	in_multi *ia_multiaddrs; /* list of multicast addresses */
};

struct	in_aliasreq {
	char	ifra_name[IFNAMSIZ];		/* if name, e.g. "en0" */
	struct	sockaddr_in ifra_addr;
	struct	sockaddr_in ifra_broadaddr;
#define ifra_dstaddr ifra_broadaddr
	struct	sockaddr_in ifra_mask;
};
/*
 * Given a pointer to an in_ifaddr (ifaddr),
 * return a pointer to the addr, or dstaddr, as a sockaddr_in.
 */
#define	IA_SIN(ia) (&(((struct in_ifaddr *)(ia))->ia_addr))
#define	IA_DSIN(ia) (&(((struct in_ifaddr *)(ia))->ia_dstaddr))

#ifdef	_KERNEL
#if	NETSYNC_LOCK
/* These may yet become blocking locks with r/w semantics... */
extern	simple_lock_data_t	inifaddr_lock;
#define INIFADDR_LOCKINIT()	simple_lock_init(&inifaddr_lock)
#define INIFADDR_LOCK()		simple_lock(&inifaddr_lock)
#define INIFADDR_UNLOCK()	simple_unlock(&inifaddr_lock)
#else
#define INIFADDR_LOCKINIT()
#define INIFADDR_LOCK()
#define INIFADDR_UNLOCK()
#endif

extern	struct	in_ifaddr *in_ifaddr;
extern	struct	ifqueue	ipintrq;	/* ip packet input queue */

/*
 * Macro for finding the interface (ifnet structure) corresponding to one
 * of our IP addresses.
 */
#define INADDR_TO_IFP(addr, ifp) \
	/* struct in_addr addr; */ \
	/* struct ifnet *ifp; */ \
{ \
	register struct in_ifaddr *ia; \
\
	for (ia = in_ifaddr; \
	    ia != NULL && IA_SIN(ia)->sin_addr.s_addr != (addr).s_addr; \
	    ia = ia->ia_next) \
		 continue; \
	(ifp) = (ia == NULL) ? NULL : ia->ia_ifp; \
}

/*
 * Macro for finding the interface (ifnet structure) corresponding to one
 * of our IP addresses.  Looks at the remote address of POINTOPOINT
 * interfaces. Courtesy Jeff Honig.
 */
#define IFADDR_TO_IFP(addr, ifp) \
	/* struct in_addr addr; */ \
	/* struct ifnet *ifp; */ \
{ \
	register struct in_ifaddr *ia; \
\
	for (ia = in_ifaddr; \
	    ia != NULL; \
	    ia = ia->ia_next) \
	    if (((ia->ia_ifp->if_flags & IFF_POINTOPOINT) ? \
		IA_DSIN(ia) : IA_SIN(ia))->sin_addr.s_addr == \
			(addr).s_addr) \
		 break; \
	(ifp) = (ia == NULL) ? NULL : ia->ia_ifp; \
}

/*
 * Macro for finding the internet address structure (in_ifaddr) corresponding
 * to a given interface (ifnet structure).
 */
#define IFP_TO_IA(ifp, ia) \
	/* struct ifnet *ifp; */ \
	/* struct in_ifaddr *ia; */ \
{ \
	for ((ia) = in_ifaddr; \
	    (ia) != NULL && (ia)->ia_ifp != (ifp); \
	    (ia) = (ia)->ia_next) \
		continue; \
}
#endif

/*
 * Internet multicast address structure.  There is one of these for each IP
 * multicast group to which this host belongs on a given network interface.
 * They are kept in a linked list, rooted in the interface's in_ifaddr
 * structure.
 */
struct in_multi {
	struct	in_addr inm_addr;	/* IP multicast address */
	struct	ifnet *inm_ifp;		/* back pointer to ifnet */
	struct	in_ifaddr *inm_ia;	/* back pointer to in_ifaddr */
	u_int	inm_refcount;		/* no. membership claims by sockets */
	u_int	inm_timer;		/* IGMP membership report timer */
	struct	in_multi *inm_next;	/* ptr to next multicast address */
};

#ifdef _KERNEL
/*
 * Structure used by macros below to remember position when stepping through
 * all of the in_multi records.
 */
struct in_multistep {
	struct in_ifaddr *i_ia;
	struct in_multi *i_inm;
};

/*
 * Macro for looking up the in_multi record for a given IP multicast address
 * on a given interface.  If no matching record is found, "inm" returns NULL.
 */
#define IN_LOOKUP_MULTI(addr, ifp, inm) \
	/* struct in_addr addr; */ \
	/* struct ifnet *ifp; */ \
	/* struct in_multi *inm; */ \
{ \
	register struct in_ifaddr *ia; \
\
	IFP_TO_IA((ifp), ia); \
	if (ia == NULL) \
		(inm) = NULL; \
	else \
		for ((inm) = ia->ia_multiaddrs; \
		    (inm) != NULL && (inm)->inm_addr.s_addr != (addr).s_addr; \
		     (inm) = inm->inm_next) \
			 continue; \
}

/*
 * Macro to step through all of the in_multi records, one at a time.
 * The current position is remembered in "step", which the caller must
 * provide.  IN_FIRST_MULTI(), below, must be called to initialize "step"
 * and get the first record.  Both macros return a NULL "inm" when there
 * are no remaining records.
 */
#define IN_NEXT_MULTI(step, inm) \
	/* struct in_multistep  step; */ \
	/* struct in_multi *inm; */ \
{ \
	if (((inm) = (step).i_inm) != NULL) \
		(step).i_inm = (inm)->inm_next; \
	else \
		while ((step).i_ia != NULL) { \
			(inm) = (step).i_ia->ia_multiaddrs; \
			(step).i_ia = (step).i_ia->ia_next; \
			if ((inm) != NULL) { \
				(step).i_inm = (inm)->inm_next; \
				break; \
			} \
		} \
}

#define IN_FIRST_MULTI(step, inm) \
	/* struct in_multistep step; */ \
	/* struct in_multi *inm; */ \
{ \
	(step).i_ia = in_ifaddr; \
	(step).i_inm = NULL; \
	IN_NEXT_MULTI((step), (inm)); \
}

#endif

#endif
