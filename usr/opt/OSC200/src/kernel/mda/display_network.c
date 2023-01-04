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
static char	*sccsid = "@(#)$RCSfile: display_network.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:37:14 $";
#endif 
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
#include <sys/unix_defs.h>
#include <net/net_globals.h>

#include "mda.h"
#include <mmax_apc.h>
#include <sys/param.h>

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/protosw.h"

#include "net/if.h"
#include "net/route.h"

#include "netinet/in.h"
#include "netinet/in_pcb.h"
#include "netinet/in_systm.h"
#include "netinet/ip.h"
#include "netinet/ip_var.h"
#include "netinet/tcp.h"
#include "netinet/tcp_fsm.h"
#include "netinet/tcp_seq.h"
#include "netinet/tcp_timer.h"
#include "netinet/tcp_var.h"
#include "netinet/tcpip.h"
#include "netinet/tcp_debug.h"

extern	char	Canttranslate[];
extern	char	*truefalse();

display_socket(vaddr)
int	vaddr;
{
	char	*type;
	char	buf[16];
	int	paddr;
	struct	socket	*so;

	if(phys(vaddr, &paddr, ptb0) != SUCCESS) {
		printf(Canttranslate, vaddr);
		return(FAILED);
	}

	so = (struct socket *)(MAPPED(paddr));
	switch(so->so_type) {
		case SOCK_STREAM:	type = "Stream";
					break;

		case SOCK_DGRAM:	type = "Dgram";
					break;

		case SOCK_RAW:		type = "Raw";
					break;

		case SOCK_RDM:		type = "RDM";
					break;

		case SOCK_SEQPACKET:	type = "Sequenced";
					break;

#if	MACH_VMTP
		case SOCK_TRANSACT:	type = "Transact";
					break;
#endif	MACH_VMTP

		default:		sprintf(buf, "Unknown(%u)",
					    so->so_type);
					type = buf;
					break;
	}

	printthree("sxx", "Type", "Options", "Linger",
		type, so->so_options, so->so_linger);
	printthree("xxx", "state", "pcb", "proto",
		so->so_state, so->so_pcb, so->so_proto);

	printthree("xxx", "so_head", "so_q0", "so_q0len",
		so->so_head, so->so_q0, so->so_q0len);
	printthree("xxx", "so_q", "so_qlen", "so_qlimit",
		so->so_q, so->so_qlen, so->so_qlimit);
	printthree("xxx", "so_timeo", "so_error", "so_pgid",
		so->so_timeo, so->so_error, so->so_pgid);
	printthree("x  ", "so_oobmark", "", "",
		so->so_oobmark, 0, 0);

	prsockbuf("Receive", &so->so_rcv, vaddr);
	prsockbuf("Send", &so->so_snd, vaddr);
#if	MMAX_MP
	prsocklocks(so->so_lock);
#endif	MMAX_MP
}

prsockbuf(which, sb, vaddr)
char	*which;
struct	sockbuf *sb;
int	vaddr;
{
	printf("\n%s Sockbuf:\n", which);

	printthree("xxx", "sb_cc", "sb_hiwat", "sb_mbcnt",
		sb->sb_cc, sb->sb_hiwat, sb->sb_mbcnt);
	printthree("xxx", "sb_mbmax", "sb_lowat", "sb_timeo",
		sb->sb_mbmax, sb->sb_lowat, sb->sb_timeo);
#if	MMAX_MP
	printthree("x x", "sb_mb", "sb_selq", "sb_flags",
		sb->sb_mb, &sb->sb_selq, sb->sb_flags);
				/* &sb->sb_selq is wrong */
#else	MMAX_MP
	printthree("xxx", "sb_mb", "sb_sel", "sb_flags",
		sb->sb_mb, sb->sb_sel, sb->sb_flags);
#endif	MMAX_MP
}

#if	MMAX_MP
prsocklocks(vaddr)
int	vaddr;
{
	int	paddr;
	struct	socklocks *ptr;

	if(phys(vaddr, &paddr, ptb0) != SUCCESS) {
		printf(Canttranslate, vaddr);
		return(FAILED);
	}

	ptr = (struct socklocks *)(MAPPED(paddr));

	printf("\nSocket lock @ %#x:\n", vaddr);
	display_lock(&ptr->so_lock);
	printf("\nReceive buffer lock:\n");
	display_lock(&ptr->snd_lock);
	printf("\nSend buffer lock:\n");
	display_lock(&ptr->rcv_lock);

	printf("\n");
	printthree("x  ", "Refcnt", "", "", ptr->refcnt, 0, 0);
}
#endif	MMAX_MP

display_inpcb(inp)
struct	inpcb	*inp;
{
	printthree("xxx", "inp_next", "inp_prev", "inp_head",
		inp->inp_next, inp->inp_prev, inp->inp_head);
	printthree("xxx", "inp_faddr", "inp_fport", "inp_laddr",
		inp->inp_faddr.s_addr, inp->inp_fport, inp->inp_laddr.s_addr);
	printthree("xxx", "inp_lport", "inp_socket", "inp_ppcb",
		inp->inp_lport, inp->inp_socket, inp->inp_ppcb);
	printthree("xxx", "route.ro_rt", "route.ro_dst", "inp_options",
		inp->inp_route.ro_rt, inp->inp_route.ro_dst, inp->inp_options);
#if	MMAX_MP && PHASEII
#if	TTLCONTROL
	printthree("xxs", "inp_ttl", "refcnt", "closed",
		inp->inp_ttl, inp->refcnt, truefalse(inp->closed));
#else	TTLCONTROL
	printthree("xs ", "refcnt", "closed", "",
		inp->refcnt, truefalse(inp->closed));
#endif	TTLCONTROL
	display_lock(&inp->inp_lock);
#else	MMAX_MP && PHASEII
#if	TTLCONTROL
	printthree("x  ", "inp_ttl", "", "",
		inp->inp_ttl, 0, 0);
#endif	TTLCONTROL
#endif	MMAX_MP && PHASEII
}

static	char *tcpstates[] = {
	"CLOSED",	"LISTEN",	"SYN_SENT",	"SYN_RCVD",
	"ESTABLISHED",	"CLOSE_WAIT",	"FIN_WAIT_1",	"CLOSING",
	"LAST_ACK",	"FIN_WAIT_2",	"TIME_WAIT",
};

display_tcpcb(tp)
struct	tcpcb	*tp;
{
	char	buf[80], *p;
	int	i;

	if(tp->t_state < 0 || tp->t_state > 10) {
		sprintf(buf, "Unknown(%u)", tp->t_state);
		p = buf;
	} else
		p = tcpstates[tp->t_state];

	printthree("xxs", "next", "prev", "state",
		tp->seg_next, tp->seg_prev, p);
	printthree("xxx", "TCPT_REXMT", "TCPT_PERSIST", "TCPT_KEEP",
		tp->t_timer[TCPT_REXMT], tp->t_timer[TCPT_PERSIST], tp->t_timer[TCPT_KEEP]);
	printthree("xxx", "TCPT_2MSL", "rxtshift", "rxtcur",
		tp->t_timer[TCPT_2MSL], tp->t_rxtshift, tp->t_rxtcur);
	printthree("xxs", "dupacks", "maxseg", "force",
		tp->t_dupacks, tp->t_maxseg, truefalse(tp->t_force));
	printthree("xxx", "template", "inpcb", "snd_una",
		tp->t_template, tp->t_inpcb, tp->snd_una);
	printthree("xxx", "snd_nxt", "snd_up", "snd_wl1", 
		tp->snd_nxt, tp->snd_up, tp->snd_wl1);
	printthree("xxx", "snd_wl2", "iss", "snd_wnd", 
		tp->snd_wl2, tp->iss, tp->snd_wnd);
	printthree("xxx", "rcv_wnd", "rcv_nxt", "rcv_up", 
		tp->rcv_wnd, tp->rcv_nxt, tp->rcv_up);
	printthree("xxx", "irs", "rcv_adv", "snd_max", 
		tp->irs, tp->rcv_adv, tp->snd_max);
	printthree("xxx", "snd_cwnd", "snd_ssthresh", "t_idle", 
		tp->snd_cwnd, tp->snd_ssthresh, tp->t_idle);
	printthree("xxx", "t_rtt", "t_rtseq", "t_srtt", 
		tp->t_rtt, tp->t_rtseq, tp->t_srtt);
	printthree("xxx", "t_rttvar", "max_rcvd", "max_sndwnd", 
		tp->t_rttvar, tp->max_rcvd, tp->max_sndwnd);
	printthree("xx ", "t_oobflags", "t_iobc",  "",
		tp->t_oobflags, tp->t_iobc, 0);

	buf[0] = '<';
	buf[1] = '\0';
	i = 0;
	if(tp->t_flags & TF_ACKNOW) {
		i++;
		strcat(buf, "ACKNOW");
	}
	if(tp->t_flags & TF_DELACK) {
		if(i++)
			strcat(buf, ",");
		strcat(buf, "DELACK");
	}
	if(tp->t_flags & TF_NODELAY) {
		if(i++)
			strcat(buf, ",");
		strcat(buf, "NODELAY");
	}
	if(tp->t_flags & TF_NOOPT) {
		if(i++)
			strcat(buf, ",");
		strcat(buf, "NOOPT");
	}
	if(tp->t_flags & TF_SENTFIN) {
		if(i++)
			strcat(buf, ",");
		strcat(buf, "SENTFIN");
	}
	strcat(buf, ">");
	printthree("s  ", "flags", "", "", buf, 0, 0);
}

display_tcp_chain()
{
	struct	inpcb	*tcp, *vaddr, *paddr;
	struct	tcpcb	*tcpcb;

	if(get_address("_tcb", &vaddr) != SUCCESS) {
		printf("mda: Couldn't get address of 'tcb'\n");
		return(FAILED);
	}

	if(phys(vaddr, &paddr, ptb0) == SUCCESS) {
		printf("TCP CB Head @ %#x\n", paddr);
		tcp = (struct inpcb *)(MAPPED(paddr));
		display_inpcb(tcp);
		vaddr = tcp->inp_next;
	} else {
		printf("mda: phys of tcb failed\n");
		return(FAILED);
	}

	while(1) {
		if(phys(vaddr, &paddr, ptb0) != SUCCESS) {
			printf("mda: phys of tcb chain failed\n");
			return(FAILED);
		}
		paddr = (struct inpcb *)(MAPPED(paddr));
		if(paddr == tcp)
			return(SUCCESS);
		printf("\n*****\nINPCB @ %#x\n", vaddr);
		display_inpcb(paddr);
		tcpcb = paddr->inp_ppcb;
		printf("\nTCP CB @ %#x\n", tcpcb);
		if(phys(tcpcb, &tcpcb, ptb0) != SUCCESS) {
			printf("mda: phys of tcb chain failed\n");
			return(FAILED);
		}
		display_tcpcb(MAPPED(tcpcb));
		vaddr = paddr->inp_next;
	}
}
