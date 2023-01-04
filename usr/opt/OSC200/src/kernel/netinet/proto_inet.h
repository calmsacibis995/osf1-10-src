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
 *	@(#)$RCSfile: proto_inet.h,v $ $Revision: 4.3.7.3 $ (DEC) $Date: 1993/06/29 18:12:32 $
 */ 
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
 *	Revision History:
 *
 * 10-Oct-91	Heather Gray
 *	add ip_dealloc()
 *
 * 5-June-91	Heather Gray
 *	OSF 1.0.1 patches.
 *
 */
#ifndef _PROTO_INET_H_
#define _PROTO_INET_H_

#if	(defined(__STDC__) || defined(__GNUC__)) && !defined(_NO_PROTO)
# define	P(s) s
/* Avoid scoping problems */
struct arpcom; struct in_addr; struct arptab;
struct mbuf; struct ifnet;
struct sockaddr; struct sockaddr_in; struct socket;
struct in_ifaddr; struct inpcb; struct ipq;
struct ipasfrag; struct route; struct ip;
struct tcpcb; struct tcpiphdr;
/* The tcp_seq typedef is a problem. Better to cast or rearrange? */
#define proto_tcp_seq	u_int
#else
# define P(s) ()
#endif


/* if_ether.c */
int	arptimer P((void));
void	arpinit P((void));
void	arpintr P((void));
void	arpinput P((struct arpcom *, struct mbuf *));
void	in_arpinput P((struct arpcom *, struct mbuf *));
void	arpoutput P((struct arpcom *, struct mbuf *, u_char *, u_short));
void	arptfree P((struct arptab *));
struct	arptab *arptnew P((struct in_addr *, struct ifnet *));
/* arpwhohas, arpioctl are in net/if_ethersubr.c */

/* igmp.c */
void	igmp_init P((void));
void	igmp_input P((struct mbuf *, int));
void	igmp_fasttimo P((void));
void	igmp_sendreport P((struct in_multi *));

/* in.c */
struct	in_addr in_makeaddr P((u_int, u_int));
u_int	in_netof P((struct in_addr));
void	in_sockmaskof P((struct in_addr, struct sockaddr_in *));
u_int	in_lnaof P((struct in_addr));
int	in_localaddr P((struct in_addr));
int	in_canforward P((struct in_addr));
int	in_control P((struct socket *, int, caddr_t, struct ifnet *));
void	in_ifscrub P((struct ifnet *, struct in_ifaddr *));
int	in_ifinit P((struct ifnet *, struct in_ifaddr *, struct sockaddr_in *,
				int));
struct	in_ifaddr *in_iaonnetof P((u_int));
int	in_broadcast P((struct in_addr));
struct	in_multi *in_addmulti P((struct in_addr *, struct ifnet *));
int	in_delmulti P((struct in_multi *));

/* in_cksum.c or machine dependent */
int	in_cksum P((struct mbuf *, int));

/* in_pcb.c */
int	in_pcballoc P((struct socket *, struct inpcb *));
int	in_pcbbind P((struct inpcb *, struct mbuf *));
int	in_pcbconnect P((struct inpcb *, struct mbuf *));
void	in_pcbdisconnect P((struct inpcb *));
void	in_pcbdetach P((struct inpcb *));
void	in_pcbfree P((struct inpcb *));
void	in_setsockaddr P((struct inpcb *, struct mbuf *));
void	in_setpeeraddr P((struct inpcb *, struct mbuf *));
void	in_pcbnotify P((struct inpcb *, struct sockaddr *, u_short,
				struct in_addr, u_short, int, void (*)()));
void	in_losing P((struct inpcb *));
void	in_rtchange P((struct inpcb *));
struct	inpcb *in_pcblookup P((struct inpcb *, struct in_addr,
				u_short, struct in_addr, u_short, int));
struct	inpcb *in_pcbmatch P((struct inpcb *, struct in_addr,
				u_short, struct in_addr, u_short));

/* in_proto.c */
int	inet_config ();		/* No prototypes here */

/* ip_icmp.c */
void	icmp_error P((struct mbuf *, int, int, struct in_addr));
void	icmp_input P((struct mbuf *, int));
void	icmp_reflect P((struct mbuf *));
struct	in_ifaddr *ifptoia P((struct ifnet *));
void	icmp_send P((struct mbuf *, struct mbuf *));
n_time	iptime P((void));

/* ip_input.c */
void	ip_init P((void));
void	ipintr P((void));
struct	mbuf *ip_reass P((struct mbuf *, struct ipq *));
void	ip_dealloc P((struct mbuf *));
void	ip_freef P((struct ipq *));
void	ip_enq P((struct ipasfrag *, struct ipasfrag *));
void	ip_deq P((struct ipasfrag *));
void	ip_slowtimo P((void));
void	ip_reastimo P((struct ipq *));
void	ip_drain P((void));
int	ip_dooptions P((struct mbuf *));
struct	in_ifaddr *ip_rtaddr P((struct in_addr));
void	save_rte P((u_char *, struct in_addr));
struct	mbuf *ip_srcroute P((void));
void	ip_stripoptions P((struct mbuf *, struct mbuf *, struct ipoption *ipopt));
void	ip_forward P((struct mbuf *, int));
void	mach_net_ipinit P((void));
void	mach_net_ipdone P((void));
int	mach_net_ipsend P((caddr_t, int));
struct	mbuf *mach_net_ipreceive P((struct mbuf *, int));

/* ip_output.c */
int	ip_output P((struct mbuf *, struct mbuf *, struct route *, 
				int, struct ip_moptions *));
struct	mbuf *ip_insertoptions P((struct mbuf *, struct mbuf *, int *));
int	ip_optcopy P((struct ip *, struct ip *));
int	ip_ctloutput P((int, struct socket *, int, int, struct mbuf **));
int	ip_pcbopts P((struct mbuf **, struct mbuf *));
void	ip_mloopback P((struct ifnet *, struct mbuf *, struct sockaddr_in *));

/* ip_screen.c */
void	ip_forwardscreen P((struct mbuf *, int));
void	ip_gwbounce P((struct mbuf *, int));

/* raw_ip.c */
void	rip_input P((struct mbuf *));
int	rip_output P((struct mbuf *, struct socket *));
int	rip_ctloutput P((int, struct socket *, int, int, struct mbuf **));
int	rip_usrreq P((struct socket *, int, struct mbuf *, struct mbuf *,
				struct mbuf *));

/* tcp_debug.c */
void	tcp_trace P((int, int, struct tcpcb *, struct tcpiphdr *, int));

/* tcp_input.c */
int	tcp_reass P((struct tcpcb *, struct tcpiphdr *, struct mbuf *));
void	tcp_input P((struct mbuf *, int));
void	tcp_dooptions P((struct tcpcb *, struct mbuf *, struct tcpiphdr *));
void	tcp_pulloutofband P((struct socket *, struct tcpiphdr *,
				struct mbuf *));
void	tcp_xmit_timer P((struct tcpcb *));
int	tcp_mss P((struct tcpcb *, u_short));

/* tcp_output.c */
int	tcp_output P((struct tcpcb *));
void	tcp_setpersist P((struct tcpcb *));

/* tcp_subr.c */
void	tcp_init P((void));
void	tcp_template P((struct tcpcb *));
void	tcp_respond P((struct tcpcb *, struct tcpiphdr *, struct mbuf *,
				proto_tcp_seq, proto_tcp_seq, int));
struct	tcpcb *tcp_newtcpcb P((struct inpcb *));
struct	tcpcb *tcp_drop P((struct tcpcb *, int));
struct	tcpcb *tcp_close P((struct tcpcb *));
void	tcp_drain P((void));
void	tcp_notify P((struct inpcb *, int));
void	tcp_ctlinput P((int, struct sockaddr *, struct ip *));
void	tcp_quench P((struct inpcb *));

/* tcp_timer.c */
void	tcp_fasttimo P((void));
void	tcp_slowtimo P((void));
void	tcp_canceltimers P((struct tcpcb *));
struct	tcpcb *tcp_timers P((struct tcpcb *, int));

/* tcp_usrreq.c */
int	tcp_usrreq P((struct socket *, int, struct mbuf *, struct mbuf *,
				struct mbuf *));
int	tcp_ctloutput P((int, struct socket *, int, int, struct mbuf **));
int	tcp_attach P((struct socket *));
struct	tcpcb *tcp_disconnect P((struct tcpcb *));
struct	tcpcb *tcp_usrclosed P((struct tcpcb *));

/* udp_usrreq.c */
void	udp_init P((void));
void	udp_input P((struct mbuf *, int));
struct	mbuf *udp_saveopt P((caddr_t, int, int));
void	udp_notify P((struct inpcb *, int));
void	udp_ctlinput P((int, struct sockaddr *, struct ip *));
int	udp_output P((struct inpcb *, struct mbuf *, struct mbuf *,
				struct mbuf *));
int	udp_usrreq P((struct socket *, int, struct mbuf *, struct mbuf *,
				struct mbuf *));

#undef P

#endif
