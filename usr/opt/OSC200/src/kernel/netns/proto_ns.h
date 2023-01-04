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
 *	@(#)$RCSfile: proto_ns.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/06/29 18:39:47 $
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

#ifndef _PROTO_NS_H_
#define _PROTO_NS_H_

#if	(defined(__STDC__) || defined(__GNUC__)) && !defined(_NO_PROTO)
# define	P(s) s
/* Avoid scoping problems */
struct mbuf; struct socket; struct ifnet;
struct nspcb; struct ns_ifaddr;
struct sockaddr; struct sockaddr_ns;
struct ns_addr; struct in_addr;
struct route; struct ifnet_en;
struct sppcb; struct spidp; 
#else
# define P(s) ()
#endif


/* idp_usrreq.c */
int	idp_input P((struct mbuf *, struct nspcb *));
int	idp_abort P((struct nspcb *));
struct	nspcb *idp_drop P((struct nspcb *, int));
int	idp_output P((struct nspcb *, struct mbuf *));
int	idp_ctloutput P((int, struct socket *, int, int, struct mbuf **));
int	idp_usrreq P((struct socket *, int, struct mbuf *,
				struct mbuf *, struct mbuf *));
int	idp_raw_usrreq P((struct socket *, int, struct mbuf *,
				struct mbuf *, struct mbuf *));

/* ns.c */
int	ns_control P((struct socket *, int, caddr_t, struct ifnet *));
int	ns_ifscrub P((struct ifnet *, struct ns_ifaddr *));
int	ns_ifinit P((struct ifnet *, struct ns_ifaddr *,
				struct sockaddr_ns *, int));
struct	ns_ifaddr *ns_iaonnetof P((struct ns_addr *));

/* somewhere */
u_short	ns_cksum P((struct mbuf *, int));

/* ns_error.c */
int	ns_err_x P((int));
int	ns_error P((struct mbuf *, int, int));
int	ns_printhost P((struct ns_addr *));
int	ns_err_input P((struct mbuf *));
u_long	nstime P((void));
int	ns_echo P((struct mbuf *));

/* ns_input.c */
int	ns_init P((void));
int	nsintr P((void));
int	idp_ctlinput P((int, caddr_t));
int	idp_forward P((struct mbuf *));
int	idp_do_route P((struct ns_addr *, struct route *));
int	idp_undo_route P((struct route *));
int	ns_watch_output P((struct mbuf *, struct ifnet *));

/* ns_ip.c */
struct	ifnet_en *nsipattach P((void));
int	nsipioctl P((struct ifnet *, int, caddr_t));
int	idpip_input P((struct mbuf *, struct ifnet *));
int	nsipoutput P((struct ifnet_en *, struct mbuf *, struct sockaddr *));
int	nsipstart P((struct ifnet *));
int	nsip_route P((struct mbuf *));
int	nsip_free P((struct ifnet *));
int	nsip_ctlinput P((int, struct sockaddr *));
int	nsip_rtchange P((struct in_addr *));

/* ns_output.c */
int	ns_output P((struct mbuf *, struct route *, int));

/* ns_pcb.c */
int	ns_pcballoc P((struct socket *, struct nspcb *));
int	ns_pcbbind P((struct nspcb *, struct mbuf *));
int	ns_pcbconnect P((struct nspcb *, struct mbuf *));
int	ns_pcbdisconnect P((struct nspcb *));
int	ns_pcbdetach P((struct nspcb *));
int	ns_setsockaddr P((struct nspcb *, struct mbuf *));
int	ns_setpeeraddr P((struct nspcb *, struct mbuf *));
int	ns_pcbnotify P((struct ns_addr *, int, int (*)(), long));
int	ns_rtchange P((struct nspcb *));
struct	nspcb *ns_pcblookup P((struct ns_addr *, u_short, int));

/* ns_proto.c */
int	ns_config ();		/* No prototypes here */

/* spp_debug.c */
int	spp_trace P((int, u_char, struct sppcb *, struct spidp *, int));

/* spp_usrreq.c */
int	spp_init P((void));
int	spp_input P((struct mbuf *, struct nspcb *));
int	spp_reass P((struct sppcb *, struct spidp *));
int	spp_ctlinput P((int, caddr_t));
int	spp_quench P((struct nspcb *));
int	spp_fixmtu P((struct nspcb *));
int	spp_output P((struct sppcb *, struct mbuf *));
int	spp_setpersist P((struct sppcb *));
int	spp_ctloutput P((int, struct socket *, int, int, struct mbuf **));
int	spp_usrreq P((struct socket *, int, struct mbuf *,
				struct mbuf *, struct mbuf *));
int	spp_usrreq_sp P((struct socket *, int, struct mbuf *,
				struct mbuf *, struct mbuf *));
int	spp_template P((struct sppcb *));
struct	sppcb *spp_close P((struct sppcb *));
struct	sppcb *spp_usrclosed P((struct sppcb *));
struct	sppcb *spp_disconnect P((struct sppcb *));
struct	sppcb *spp_drop P((struct sppcb *, int));
int	spp_abort P((struct nspcb *));
int	spp_fasttimo P((void));
int	spp_slowtimo P((void));
struct	sppcb *spp_timers P((struct sppcb *, int));

#undef P

#endif
