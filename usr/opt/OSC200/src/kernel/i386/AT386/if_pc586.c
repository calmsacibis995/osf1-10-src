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
static char	*sccsid = "@(#)$RCSfile: if_pc586.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:08:43 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/* 
 * OSF/1 Release 1.0
 */

/* 
 *	Olivetti PC586 Mach Ethernet driver v1.0
 *	Copyright Ing. C. Olivetti & C. S.p.A. 1988, 1989
 *	All rights reserved.
 *
 */ 
/*
  Copyright 1988, 1989 by Olivetti Advanced Technology Center, Inc.,
Cupertino, California.

		All Rights Reserved

  Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appears in all
copies and that both the copyright notice and this permission notice
appear in supporting documentation, and that the name of Olivetti
not be used in advertising or publicity pertaining to distribution
of the software without specific, written prior permission.

  OLIVETTI DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL OLIVETTI BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUR OF OR IN CONNECTION
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

 /*
  *	INTEL CORPORATION PROPRIETARY INFORMATION
  *
  *	This software is supplied under the terms of a license
  *	agreement or nondisclosure agreement with Intel Corpo-
  *	ration and may not be copied or disclosed except in
  *	accordance with the terms of that agreement.
  */


/*
 * NOTE:
 *
 *	Currently this driver doesn't support trailer protocols for
 *	packets.  Once that is added, please remove this comment.
 *
 */

#include	<pc586.h>
#if	NPC586 > 0

#include	<sys/param.h>
#include	<mach/vm_param.h>
#include	<sys/systm.h>
#include	<sys/mbuf.h>
#include	<sys/buf.h>
#include 	<sys/table.h>
#include	<sys/protosw.h>
#include	<sys/socket.h>
#include	<sys/vmmac.h>
#include	<sys/ioctl.h>
#include	<sys/errno.h>
#include	<sys/syslog.h>
#include	<vm/vm_kern.h>

#include	<net/if.h>
#include	<net/if_types.h>
#include	<net/route.h>

#include	<netinet/in.h>
#include	<netinet/in_systm.h>
#include	<netinet/in_var.h>
#include	<netinet/ip.h>
#include	<netinet/if_ether.h>

#include	<netns/ns.h>
#include	<netns/ns_if.h>

#include	<mach/vm_param.h>
#include	<vm/vm_kern.h>
#include	<i386/ipl.h>
#include	<i386/AT386/atbus.h>
#include	<i386/AT386/if_pc586.h>
#include	<i386/handler.h>
#include	<i386/dispatcher.h>

#define	SPLNET	spl6

int	pc586probe(),	pc586attach();
int	pc586intr(), pc586init(), pc586start(), pc586ioctl(), pc586reset();
int	pc586watch(), pc586rcv(), pc586xmt(), pc586bldcu(), pc586bldru();
int	pc586diag(), pc586config(), pc586wtscb();

int (*pc586intrs[])() = { pc586intr, 0 };

struct	isa_driver	pcdriver = 
	{ pc586probe, 0, pc586attach, "pc", 0, 0, 0 };

typedef struct { 
	struct	arpcom	pc586_ac;
#define	ds_if	pc586_ac.ac_if
#define	ds_addr	pc586_ac.ac_enaddr
	int	flags;
	fd_t    *begin_fd,  *end_fd;
	rbd_t   *begin_rbd, *end_rbd;
	volatile u_char	*prom;
	volatile u_char	*sram;
	ubyte   address[ETHER_ADD_SIZE];
	short	mode;
	ihandler_t	handler;
	ihandler_id_t	*handler_id;
} pc_softc_t;

pc_softc_t pc_softc[NPC586];

#define pc_to_virt(off, unit, cast)	\
	(cast)(((ushort)(off)>0x7fff)?NULL:(pc_softc[unit].sram+(ushort)(off)))
#define virt_to_pc(va, unit)	\
	(ushort)(((volatile u_char *)(va))-pc_softc[unit].sram)

#define NOOP_DELAY {asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");\
		    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");\
		    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");}

#define pc586set(unit, off, cmd) do { \
	*(volatile ushort *)(pc_softc[unit].prom + (off)) = (cmd); \
	NOOP_DELAY \
	NOOP_DELAY \
	NOOP_DELAY \
} while (0)

#define pc586inton(unit)	pc586set(unit, OFFSET_INTENAB, CMD_1)
#define pc586intoff(unit)	pc586set(unit, OFFSET_INTENAB, CMD_0)

#define pc586chatt(unit) do { \
	/* Set, then clear Channel Attention */ \
	pc586set(unit, OFFSET_CHANATT, 1); \
	pc586set(unit, OFFSET_CHANATT, 0); \
} while (0)

#define pc586ehcpy(src, dst) do { \
	/* 16-bit copy alignment required */ \
	((unsigned long  *)dst)[0] = ((unsigned long  *)src)[0]; \
	((unsigned short *)dst)[2] = ((unsigned short *)src)[2]; \
} while (0)

#ifdef	DEBUG
#define dprintf(foo)	printf foo
#else
#define dprintf(foo)
#endif

/*
 * pc586probe:
 *
 *	This function "probes" or checks for the pc586 board on the bus to see
 *	if it is there. It just verifies a plausible ethernet address prom.
 *
 * input	: address device is mapped to, and unit # being checked
 * output	: a '1' is returned if the board exists, and a 0 otherwise
 *
 */

pc586probe(dev)
struct isa_dev *dev;
{
	caddr_t	addr = dev->dev_addr;
	int	unit = dev->dev_unit;
	extern vm_size_t mem_size;

	volatile u_char	*b_prom;
	vm_offset_t	mappc586();

	if ((unsigned) unit >= NPC586)
		return 0;
	if (addr < (caddr_t)mem_size && addr > (caddr_t)0x100000)
		return 0;

	/* The 0x100000 is mostly for the Intel default 0xf00000 address.
	 * The 16k is enough to map the various prom offsets - see pc586.h. */
	b_prom = (volatile u_char *)((int)addr > 0x100000 ?
			mappc586(addr, round_page(16*1024)) :
			phystokv(addr));

	/* Ugh. But board won't work unless we do this. */
	*(b_prom + OFFSET_RESET) = 1;
	NOOP_DELAY
	NOOP_DELAY
	*(b_prom + OFFSET_RESET) = 0;
	NOOP_DELAY
	NOOP_DELAY

	/* Look for 00 AA 00 etherid */
	if (*(b_prom + OFFSET_PROM + 0) != 0x00 ||
	    *(b_prom + OFFSET_PROM + 2) != 0xAA ||
	    *(b_prom + OFFSET_PROM + 4) != 0x00) {
		if ((int)addr > 0x100000)
			unmappc586(addr, round_page(16*1024));
		return(0);
	}

	pc_softc[unit].prom = b_prom;
	/* Map sram differently if board at high address. Size=32K */
	pc_softc[unit].sram = (volatile u_char *)((int)addr > 0x100000 ?
			mappc586(addr+EXTENDED_ADDR, round_page(32*1024)) :
			(vm_offset_t)b_prom);
	return(1);
}



/*
 * pc586attach:
 *
 *	This function attaches a PC586 board to the "system".  The rest of
 *	runtime structures are initialized here (this routine is called after
 *	a successful probe of the board).  Once the ethernet address is read
 *	and stored, the board's ifnet structure is attached and readied.
 *
 * input	: i386_dev structure setup in autoconfig
 * output	: board structs and ifnet is setup
 *
 */


pc586attach(dev)
	struct isa_dev *dev;
{
	pc_softc_t	*sp;
	struct	ifnet	*ifp;
	u_char		*p_addr;
	volatile u_char	*b_addr;
	int		unit;

	unit = dev->dev_unit;	

	if ((unsigned) unit >= NPC586)
		return;
	sp = &pc_softc[unit];
	sp->handler.ih_level = dev->dev_pic;
	sp->handler.ih_handler = dev->dev_intr[0];
	sp->handler.ih_resolver = i386_resolver;
	sp->handler.ih_rdev = dev;
	sp->handler.ih_stats.intr_type = INTR_DEVICE;
	sp->handler.ih_stats.intr_cnt = 0;
	sp->handler.ih_hparam[0].intparam = unit;
	if ((sp->handler_id = handler_add(&sp->handler)) != NULL)
		handler_enable(sp->handler_id);
	else
		panic("Unable to add pc586 interrupt handler");

	sp->flags = 0;
	sp->mode = 0;
	*(sp->prom + OFFSET_RESET) = 1;
	NOOP_DELAY
	NOOP_DELAY
	*(sp->prom + OFFSET_RESET) = 0;
	b_addr = (u_char *)(sp->prom + OFFSET_PROM);
	p_addr = (u_char *)sp->ds_addr;
	*(p_addr)	= sp->address[0] = *(b_addr);
	*(p_addr + 1)	= sp->address[1] = *(b_addr + 2);
	*(p_addr + 2)	= sp->address[2] = *(b_addr + 4);
	*(p_addr + 3)	= sp->address[3] = *(b_addr + 6);
	*(p_addr + 4)	= sp->address[4] = *(b_addr + 8);
	*(p_addr + 5)	= sp->address[5] = *(b_addr + 10);
	printf("pc%d: pc586 ethernet id [%x:%x:%x:%x:%x:%x] irq = %d\n",
		unit,
		sp->address[0],sp->address[1],sp->address[2], 
		sp->address[3],sp->address[4],sp->address[5],
		dev->dev_pic);

	sp->pc586_ac.ac_bcastaddr = (u_char *)etherbroadcastaddr;
	sp->pc586_ac.ac_arphrd = ARPHRD_ETHER;

	ifp = &(sp->ds_if);
	ifp->if_unit = unit;
	ifp->if_name = "pc";
	ifp->if_mtu = ETHERMTU - 8;
	ifp->if_timer = 0;
	ifp->if_flags = IFF_BROADCAST|IFF_SIMPLEX|IFF_NOTRAILERS;
	ifp->if_type = IFT_ETHER;
	ifp->if_addrlen = 6;
	ifp->if_hdrlen = sizeof (struct ether_header) + 8; /* 8 for SNAP */

	ifp->if_init = pc586init;
	ifp->if_output = ether_output;
	ifp->if_start = pc586start;
	ifp->if_ioctl = pc586ioctl;
	ifp->if_reset = pc586reset;
	ifp->if_watchdog = pc586watch;

	if_attach(ifp);
}


/*
 * pc586reset:
 *
 *	This routine is in part an entry point for the "if" code.  Since most 
 *	of the actual initialization has already (we hope already) been done
 *	by calling pc586attach().
 *
 * input	: unit number or board number to reset
 * output	: board is reset
 *
 */

int
pc586reset(unit)
	int	unit;
{
	pc_softc[unit].ds_if.if_flags &= ~IFF_RUNNING;
	return(pc586init(unit));
}



/*
 * pc586init:
 *
 *	Another routine that interfaces the "if" layer to this driver.  
 *	Simply resets the structures that are used by "upper layers".  
 *	As well as calling pc586hwrst that does reset the pc586 board.
 *
 * input	: board number
 * output	: structures (if structs) and board are reset
 *
 */	

int
pc586init(unit)
	int	unit;
{
	struct	ifnet	*ifp;
	int	opri, stat;

	ifp = &(pc_softc[unit].ds_if);
	if (ifp->if_addrlist == NULL)
		return FALSE;
	opri = SPLNET();
	if ((stat = pc586hwrst(unit)) == TRUE) {
		ifp->if_flags |= IFF_RUNNING;
		pc_softc[unit].flags |= DSF_RUNNING;
		pc586start(ifp);
	}
	ifp->if_timer = 5;
	splx(opri);
	return(stat);
}

pc586start(ifp)
	struct ifnet *ifp;
{
	struct	mbuf	*m;
 	int opri = splimp();

again:
	IF_DEQUEUE(&ifp->if_snd, m);
	if (m != NULL) {
		ifp->if_flags |= IFF_OACTIVE;
		if (!pc586xmt(ifp->if_unit, m)) {
			m_freem(m);
			goto again;
		}
		m_freem(m);
	} else
		ifp->if_flags &= ~IFF_OACTIVE;
	splx(opri);
}


/*
 * pc586read:
 *
 *	This routine does the actual copy of data (including ethernet header
 *	structure) from the pc586 to an mbuf chain that will be passed up
 *	to the "if" (network interface) layer.  NOTE:  we currently
 *	don't handle trailer protocols.
 *
 * input	: board number, frame descriptor pointer, and len
 * output	: the packet is put into an mbuf chain, and passed up
 *
 */

pc586read(unit, fd_p, len)
	int	unit;
	fd_t	*fd_p;
	int	len;
{
	register struct ifnet *ifp;
	struct	ether_header	eh;
	struct	mbuf	*m, *top, **mp;
	rbd_t	*rbd_p;
	int	bytes_in_msg, dribble;

	rbd_p = pc_to_virt(fd_p->rbd_offset, unit, rbd_t *);
	if (rbd_p == NULL) {
		dprintf(("pc%d: invalid buffer\n", unit));
		if (pc586hwrst(unit) != TRUE)
			pc586intoff(unit);
		else
			pc586rustrt(unit);
		return;
	}

	if (len < ETHERMIN || len > ETHERMTU) {
		dprintf(("pc%d: bogon %d\n", unit, len));
		return;
	}
	m = m_gethdr(M_DONTWAIT, MT_DATA);
	if (m == NULL)
		return;
	ifp = &pc_softc[unit].ds_if;
	eh.ether_type = ntohs(fd_p->length);
	pc586ehcpy(fd_p->source, eh.ether_shost);
	pc586ehcpy(fd_p->destination, eh.ether_dhost);
	m->m_pkthdr.rcvif = ifp;
	m->m_pkthdr.len = len;
	m->m_len = MHLEN;
	top = 0;
	mp = &top;
	dribble = 0;
	while (len) {
		if (top) {
			m = m_get(M_DONTWAIT, MT_DATA);
			if (m == NULL) {
				m_freem(top);
				return;
			}
			m->m_len = MLEN;
		}
		if (len > MINCLSIZE) {	/* Use a cluster for receive? */
			MCLGET(m, M_DONTWAIT);
			if (m->m_flags & M_EXT)	/* success */
				m->m_len = MCLBYTES;
		}
		if (len < m->m_len) {
			/* Place initial small header at end of mbuf */
			if (top == 0 && len + max_linkhdr <= m->m_len)
				m->m_data += max_linkhdr;
			m->m_len = len;
		}
		while (rbd_p) {
			bytes_in_msg = ((rbd_p->status & RBD_SW_COUNT)+3) & ~3;
			if (bytes_in_msg == 0) {
				rbd_p = pc_to_virt(rbd_p->next_rbd_offset,
						   unit, rbd_t *);
				continue;
			}
			/* Fresh rbd to move into mbuf */
			if (dribble >= 0) {
				if (bytes_in_msg > m->m_len - dribble) {
					/* consumes this mbuf, w/rbd dribble */
					pc586bcopy((long *)(pc_softc[unit].sram+
							rbd_p->buffer_addr),
						(long *)(mtod(m,char*)+dribble),
						m->m_len - dribble);
					dribble = -(bytes_in_msg -
							(m->m_len - dribble));
					break;
				}
				pc586bcopy((long *)(pc_softc[unit].sram +
							rbd_p->buffer_addr),
					(long *)(mtod(m, char *) + dribble),
					bytes_in_msg);
				dribble += bytes_in_msg;
			/* Unfinished rdb to move into mbuf */
			} else {
				dribble = -dribble;
				if (dribble > m->m_len) {
					/* consumes this mbuf, w/rbd dribble */
					pc586bcopy((long *)(pc_softc[unit].sram+
						rbd_p->buffer_addr +
						bytes_in_msg - dribble),
						mtod(m, long *), m->m_len);
					dribble = -(dribble - m->m_len);
					break;
				}
				pc586bcopy((long *)(pc_softc[unit].sram +
					rbd_p->buffer_addr +
					bytes_in_msg - dribble),
					mtod(m, long *), dribble);
			}
			rbd_p = pc_to_virt(rbd_p->next_rbd_offset,unit,rbd_t *);
			if (dribble >= m->m_len) {
				dribble = 0;
				break;
			}
		}
		len -= m->m_len;
		*mp = m;
		mp = &m->m_next;
	}
	ether_input(ifp, &eh, top);
}


/*
 * pc586ioctl:
 *
 *	This routine processes an ioctl request from the "if" layer
 *	above.
 *
 * input	: pointer the appropriate "if" struct, command, and data
 * output	: based on command appropriate action is taken on the
 *	 	  pc586 board(s) or related structures
 * return	: error is returned containing exit conditions
 *
 */

int
pc586ioctl(ifp, cmd, data)
	struct ifnet	*ifp;
	int	cmd;
	caddr_t	data;
{
	register struct ifaddr *ifa = (struct ifaddr *)data;
	register pc_softc_t *is;
	int opri, error;
	short mode = 0;

	is = &pc_softc[ifp->if_unit];
 	opri = SPLNET();
	error = 0;
	switch (cmd) {
	case SIOCSIFADDR:
		ifp->if_flags |= IFF_UP;
		pc586init(ifp->if_unit);
		switch (ifa->ifa_addr->sa_family) {
		case AF_INET:
			((struct arpcom *)ifp)->ac_ipaddr =
				IA_SIN(ifa)->sin_addr;
			arpwhohas((struct arpcom *)ifp, &IA_SIN(ifa)->sin_addr);
			break;
		case AF_NS: {
			register struct ns_addr *ina = &(IA_SNS(ifa)->sns_addr);
			if (ns_nullhost(*ina))
				ina->x_host = *(union ns_host *)(is->ds_addr);
			else {
				register int i;
				for(i = 0; i < 6; i++)
					is->ds_addr[i] = ina->x_host.c_host[i];
				pc586init(ifp);
			}
			}
			break;
		}
		break;
	case SIOCSIFFLAGS:
		if (ifp->if_flags & IFF_ALLMULTI)
			mode |= MOD_ENAL;
		if (ifp->if_flags & IFF_PROMISC)
			mode |= MOD_PROM;
		/*
		 * force a complete reset if the receive multicast/
		 * promiscuous mode changes so that these take 
		 * effect immediately.
		 */
		if (is->mode != mode) {
			is->mode = mode;
			if (is->flags & DSF_RUNNING) {
				is->flags &= ~(DSF_LOCK|DSF_RUNNING);
				pc586init(ifp->if_unit);
			}
		}
		if (!(ifp->if_flags & IFF_UP) && is->flags & DSF_RUNNING) {
			dprintf(("pc%d: ioctl turning off board\n", ifp->if_unit));
			is->flags &= ~(DSF_LOCK | DSF_RUNNING);
			ifp->if_timer = 0;
			pc586intoff(ifp->if_unit);
		} else if (ifp->if_flags & IFF_UP && !(is->flags & DSF_RUNNING))
			pc586init(ifp->if_unit);
		break;
	default:
		error = EINVAL;
		break;
	}
	splx(opri);
	return (error);
}



/*
 * pc586hwrst:
 *
 *	This routine resets the pc586 board that corresponds to the 
 *	board number passed in.
 *
 * input	: board number to do a hardware reset
 * output	: board is reset
 *
 */

int 
pc586hwrst(unit)
	int unit;
{
	volatile scb_t	*p_scb;
	int	i;

	pc586set(unit, OFFSET_CHANATT,  CMD_0);
	pc586set(unit, OFFSET_RESET,    CMD_1);
	pc586set(unit, OFFSET_RESET,    CMD_0);
	pc586set(unit, OFFSET_NORMMODE, CMD_0);
	pc586set(unit, OFFSET_XFERMODE, CMD_1);

	((volatile iscp_t *)(pc_softc[unit].sram + OFFSET_ISCP))->iscp_busy = 1;
	NOOP_DELAY
	NOOP_DELAY

	pc586bldcu(pc_softc[unit].sram);
	pc586bldru(unit);
	pc586chatt(unit);

	p_scb = (volatile scb_t *)(pc_softc[unit].sram + OFFSET_SCB);
	for (i = 0; ; ) {
		if (p_scb->scb_status == (SCB_SW_CX|SCB_SW_CNA)) 
			break;
		if (++i > STATUS_TRIES)
			return FALSE;
		NOOP_DELAY
		NOOP_DELAY
		NOOP_DELAY
	}

	p_scb->scb_command = SCB_ACK_CX|SCB_ACK_CNA;
	pc586chatt(unit);
	if (pc586diag(unit) == FALSE || pc586config(unit) == FALSE)
		return(FALSE);
	/* 
	 * insert code for loopback test here
	 *
	 */
	pc586inton(unit);
	pc586set(unit, OFFSET_NORMMODE, CMD_1);
	pc586wtscb(unit);
	p_scb->scb_command = SCB_RU_STRT;
	pc586chatt(unit);
	return(TRUE);
}


/*
 * pc586watch():
 *
 *	This routine is the watchdog timer routine for the pc586 chip.  If
 *	chip wedges, this routine will fire and cause a board reset and
 *	begin again.
 *
 * input	: which board is timing out
 * output	: potential board reset if wedged
 *
 */

int 
pc586watch(unit)
	int unit;
{
	int	opri;

	if ((pc_softc[unit].ds_if.if_flags & IFF_UP) == 0)
		return;
	opri = SPLNET();
	if (pc586hwrst(unit) != TRUE) {
		pc586intoff(unit);
		pc_softc[unit].ds_if.if_timer = 1;
	} else {
		pc586rustrt(unit);
		pc586start(&pc_softc[unit].ds_if);
		pc_softc[unit].ds_if.if_timer = 5;
	}
	splx(opri);
}


/*
 * pc586intr:
 *
 *	This function is the interrupt handler for the pc586 ethernet
 *	board.  This routine will be called whenever either a packet
 *	is received, or a packet has successfully been transfered and
 *	the unit is ready to transmit another packet.
 *
 * input	: board number that interrupted
 * output	: either a packet is received, or a packet is transfered
 *
 */

int 
pc586intr(unit)
	int unit;
{
	ushort	int_type;
	volatile scb_t	*scb_p;
	struct ifnet *ifp;

	if (pc_softc[unit].sram == NULL)
		return 0;
	scb_p = (volatile scb_t *)(pc_softc[unit].sram + OFFSET_SCB);
	if (scb_p->scb_command) {
		dprintf(("pc%d: interrupt without scb ready\n"));
		return 0;
	}
	if ((int_type = (scb_p->scb_status & SCB_SW_INT)) == 0)
		return 0;
	ifp = &pc_softc[unit].ds_if;
	if ((ifp->if_flags & (IFF_UP|IFF_RUNNING)) != (IFF_UP|IFF_RUNNING)) {
		dprintf(("pc%d: interrupt turning off board\n", ifp->if_unit));
		scb_p->scb_command = int_type;
		pc586intoff(ifp->if_unit);
		return 1;
	}
again:
	scb_p->scb_command = int_type;
	pc586chatt(unit);
	if (int_type & SCB_SW_FR) {
		ifp->if_ipackets++;
		(void) pc586rcv(unit);
	}
	if (int_type & SCB_SW_RNR) {
		ifp->if_ierrors++;
		dprintf(("pc%d: receiver overrun! begin_fd = %x\n",
		       ifp->if_unit, pc_softc[unit].begin_fd));
		pc586rustrt(unit);
	}
	if (int_type & SCB_SW_CNA) {
		ifp->if_opackets++;
		pc586start(ifp);
	}
	if (scb_p->scb_command == 0 &&
	    (int_type = (scb_p->scb_status & SCB_SW_INT)))
		goto again;
	pc_softc[unit].ds_if.if_timer = 5;
	return 1;
}

/*
 * pc586rcv:
 *
 *	This routine is called by the interrupt handler to initiate a
 *	packet transfer from the board to the "if" layer above this
 *	driver.  This routine checks if a buffer has been successfully
 *	received by the pc586.  If so, the routine pc586read is called
 *	to do the actual transfer of the board data (including the
 *	ethernet header) into a packet (consisting of an mbuf chain).
 *
 * input	: number of the board to check
 * output	: if a packet is available, it is "sent up"
 *
 */

int 
pc586rcv(unit)
	int	unit;
{
	fd_t	*fd_p;
	rbd_t	*rbd_p;
	int	len, packets = 0;

	for (fd_p = pc_softc[unit].begin_fd; fd_p != (fd_t *)NULL;
	     fd_p = pc_softc[unit].begin_fd) {
		if (!(fd_p->status & AC_SW_C))
			break;
		pc_softc[unit].begin_fd = pc_to_virt(fd_p->link_offset,
						     unit, fd_t *);
		rbd_p = pc_to_virt(fd_p->rbd_offset, unit, rbd_t *);
		len = 0;
		while (rbd_p) {
			len += ((rbd_p->status & RBD_SW_COUNT) + 3) & ~3;
			if (rbd_p->status & RBD_SW_EOF)
				break;
			rbd_p = pc_softc[unit].begin_rbd =
			    pc_to_virt(rbd_p->next_rbd_offset, unit, rbd_t *);
		}
		if (rbd_p)
			rbd_p->next_rbd_offset = PC586NULL;

		if (fd_p->status & AC_SW_OK) {
			packets++;
			pc586read(unit, fd_p, len);
		}
		pc586reqfd(unit, fd_p);
	}
	return packets;
}


/*
 * pc586reqfd:
 *
 *	This routine puts rbd's used in the last receive back onto the
 *	free list for the next receive.
 *
 */
 
pc586reqfd(unit, fd_p)
	int	unit;
	fd_t	*fd_p;
{
	rbd_t	*f_rbdp, *l_rbdp;

	f_rbdp = pc_to_virt(fd_p->rbd_offset, unit, rbd_t *);
	fd_p->status = 0;
	fd_p->command = AC_CW_EL;
	fd_p->link_offset = PC586NULL;
	fd_p->rbd_offset = PC586NULL;

	pc_softc[unit].end_fd->link_offset = virt_to_pc(fd_p, unit);
	pc_softc[unit].end_fd->command = 0;
	pc_softc[unit].end_fd = fd_p;
	if (f_rbdp) {
		l_rbdp = f_rbdp;
		while (pc_to_virt(l_rbdp->next_rbd_offset, unit, int)) {
			l_rbdp->status = 0;
		   	l_rbdp = pc_to_virt(l_rbdp->next_rbd_offset,
					    unit, rbd_t *);
		}
		l_rbdp->status = 0;
		l_rbdp->size |= AC_CW_EL;
		if (pc_softc[unit].begin_rbd == NULL)
			pc_softc[unit].begin_rbd = f_rbdp;
		else {
			pc_softc[unit].end_rbd->next_rbd_offset = 
				virt_to_pc(f_rbdp, unit);
			pc_softc[unit].end_rbd->size &= ~AC_CW_EL;
		}
		pc_softc[unit].end_rbd = l_rbdp;
	}
}


/*
 * pc586rustrt:
 *
 *	This routine starts the receive unit running.  First checks if the
 *	board is actually ready, then the board is instructed to receive
 *	packets again.
 *
 */

pc586rustrt(unit)
	int unit;
{
	volatile scb_t	*scb_p;

	scb_p = (volatile scb_t *)(pc_softc[unit].sram + OFFSET_SCB);
	if (!(scb_p->scb_status & SCB_RUS_READY)) {
		/*
		 * If this doesn't fix your problem, replace bldru with hwrst
		 */
		pc586bldru(unit);
		scb_p->scb_command = SCB_RU_STRT;
		scb_p->scb_rfa_offset=virt_to_pc(pc_softc[unit].begin_fd,unit);
		pc586chatt(unit);
	}
}


/*
 * pc586xmt:
 *
 *	This routine fills in the appropriate registers and memory
 *	locations on the PC586 board and starts the board off on
 *	the transmit.
 *
 * input	: board number of interest, and a pointer to the mbuf
 * output	: board memory and registers are set for xfer and attention
 *
 */

pc586xmt(unit, m)
	int	unit;
	struct	mbuf	*m;
{
	register struct ether_header	*eh_p;
	volatile ac_t  *cb_p;
	volatile tbd_t *tbd_p;
	/*
	 * The pc586 buffer has 16-bit alignment restrictions, so it's sometimes
	 * necessary to "pull up" the mbufs into a contiguous aligned buffer.
	 */
	int	count = 0;
	static	char t_packet[ETHERMTU];

	eh_p = mtod(m, struct ether_header *);
	if (m->m_len > sizeof(*eh_p)) {
		bcopy(mtod(m, caddr_t) + sizeof(*eh_p),
			t_packet, m->m_len - sizeof(*eh_p));
		count = m->m_len - sizeof(*eh_p);
	}
	while (m = m->m_next) {
		if (m->m_len == 0)
			continue;
		if (count + m->m_len > ETHERMTU)
			return 0;
		bcopy(mtod(m, caddr_t), &t_packet[count], m->m_len);
		count += m->m_len;
	}
	if (count < ETHERMIN + sizeof (*eh_p))  {
		/* bzero is optional. We're slow enough as it is. */
		/*bzero(&t_packet[count], ETHERMIN + sizeof (*eh_p) - count);*/
		count = ETHERMIN + sizeof (*eh_p);
	}

	cb_p  = (volatile ac_t *)(pc_softc[unit].sram + OFFSET_CU);
	tbd_p = (volatile tbd_t *)(pc_softc[unit].sram + OFFSET_TBD);
	 
	/*
	 * Ensure chip is ready before tromping on the one tx descriptor.
	 */
	pc586wtscb(unit);

	cb_p->ac_status = 0;
	cb_p->ac_command = AC_CW_EL|AC_TRANSMIT|AC_CW_I;
	cb_p->ac_link_offset = OFFSET_CU;
	cb_p->cmd.transmit.tbd_offset = OFFSET_TBD;
	pc586ehcpy(eh_p->ether_dhost, cb_p->cmd.transmit.dest_addr);
	cb_p->cmd.transmit.length = eh_p->ether_type;

	tbd_p->act_count = (count|TBD_SW_EOF);
	tbd_p->next_tbd_offset = PC586NULL;
	tbd_p->buffer_addr = OFFSET_TBUF;
	tbd_p->buffer_base = 0;

	pc586bcopy((unsigned long *)t_packet,
		   (unsigned long *)(pc_softc[unit].sram + OFFSET_TBUF),
		   (count + 3) & ~3); 

	((volatile scb_t *)(pc_softc[unit].sram + OFFSET_SCB))->scb_command =
		SCB_CU_STRT;
	pc586chatt(unit);
	return 1;
}



/*
 * pc586bldcu:
 *
 *	This function builds up the command unit structures.  It inits
 *	the scp, iscp, scb, cb, tbd, and tbuf.
 *
 */

pc586bldcu(sram)
	u_char	*sram;
{
	scp_t	*scp_p;
	iscp_t	*iscp_p;
	volatile scb_t	*scb_p;
	volatile ac_t	*cb_p;
	tbd_t 	*tbd_p;

	scp_p = (scp_t *)(sram + OFFSET_SCP);
	scp_p->scp_sysbus = 0;
	scp_p->scp_iscp = OFFSET_ISCP;
	scp_p->scp_iscp_base = 0;

	iscp_p = (iscp_t *)(sram + OFFSET_ISCP);
	iscp_p->iscp_busy = 1;
	iscp_p->iscp_scb_offset = OFFSET_SCB;
	iscp_p->iscp_scb = 0;
	iscp_p->iscp_scb_base = 0;

	scb_p = (volatile scb_t *)(sram + OFFSET_SCB);
	scb_p->scb_status = 0;
	scb_p->scb_command = 0;
	scb_p->scb_cbl_offset = OFFSET_CU;
	scb_p->scb_rfa_offset = OFFSET_RU;
	scb_p->scb_crcerrs = 0;
	scb_p->scb_alnerrs = 0;
	scb_p->scb_rscerrs = 0;
	scb_p->scb_ovrnerrs = 0;

	cb_p = (volatile ac_t *)(sram + OFFSET_CU);
	cb_p->ac_status = 0;
	cb_p->ac_command = AC_CW_EL;
	cb_p->ac_link_offset = OFFSET_CU;

	tbd_p = (tbd_t *)(sram + OFFSET_TBD);
	tbd_p->act_count = 0;
	tbd_p->next_tbd_offset = PC586NULL;
	tbd_p->buffer_addr = 0;
	tbd_p->buffer_base = 0;
}


/*
 * pc586bldru:
 *
 *	This function builds the linear linked lists of fd's and
 *	rbd's.  Based on page 4-32 of 1986 Intel microcom handbook.
 *
 */

pc586bldru(unit)
	int unit;
{
	fd_t	*fd_p;
	ru_t	*rbd_p;
	int 	i;

	fd_p = (fd_t *)(pc_softc[unit].sram + OFFSET_RU);
	pc_softc[unit].begin_fd = fd_p;
	for(i = 0; i < N_FD; i++, fd_p++) {
		fd_p->status = 0;
		fd_p->command	= 0;
		fd_p->link_offset = virt_to_pc(fd_p + 1, unit);
		fd_p->rbd_offset = PC586NULL;
	}
	pc_softc[unit].end_fd = --fd_p;
	fd_p->link_offset = PC586NULL;
	fd_p->command = AC_CW_EL;
	fd_p = (fd_t *)(pc_softc[unit].sram + OFFSET_RU);

	rbd_p = (ru_t *)(pc_softc[unit].sram + OFFSET_RBD);
	pc_softc[unit].begin_rbd = (rbd_t *)rbd_p;
	fd_p->rbd_offset = virt_to_pc(rbd_p, unit);
	for(i = 0; i < N_RBD; i++, rbd_p++) {
		rbd_p->r.status = 0;
		if (i != N_RBD-1)
			rbd_p->r.next_rbd_offset = virt_to_pc(rbd_p + 1, unit);
		rbd_p->r.buffer_addr = virt_to_pc(rbd_p->rbuffer, unit);
		rbd_p->r.buffer_base = 0;
		rbd_p->r.size = RCVBUFSIZE;
	}
	pc_softc[unit].end_rbd = (rbd_t *)(--rbd_p);
	rbd_p->r.next_rbd_offset = PC586NULL;
	rbd_p->r.size |= AC_CW_EL;
}


/*
 * pc586diag:
 *
 *	This routine does a 586 op-code number 7, and obtains the
 *	diagnose status for the pc586.
 *
 */

int 
pc586diag(unit)
	int unit;
{
	volatile scb_t	*p_scb;
	volatile ac_t	*p_cb;
	int	i;

	p_scb = (volatile scb_t *)(pc_softc[unit].sram + OFFSET_SCB);
	p_cb  = (volatile ac_t *)(pc_softc[unit].sram + OFFSET_CU);

	pc586wtscb(unit);
	p_scb->scb_command = p_scb->scb_status & SCB_SW_INT;
	NOOP_DELAY
	NOOP_DELAY
	if (p_scb->scb_command) 
		pc586chatt(unit);
	pc586wtscb(unit);

	p_cb->ac_status	= 0;
	NOOP_DELAY
	NOOP_DELAY
	p_cb->ac_command = (AC_DIAGNOSE|AC_CW_EL);
	NOOP_DELAY
	NOOP_DELAY
	p_scb->scb_command = SCB_CU_STRT;
	NOOP_DELAY
	NOOP_DELAY
	pc586chatt(unit);

	for (i = 0; ; ) {
		if ((p_cb->ac_status & AC_SW_OK))
			break;
		if (++i > 0xfffff)
			return FALSE;
	}

	p_scb->scb_command = p_scb->scb_status & SCB_SW_INT;
	NOOP_DELAY
	NOOP_DELAY
	if (p_scb->scb_command) 
		pc586chatt(unit);
	return(TRUE);
}



/*
 * pc586config:
 *
 *	This routine does a standard config of the pc586 board.
 *
 */

pc586config(unit)
	int unit;
{
	volatile scb_t	*p_scb;
	volatile ac_t	*p_cb;
	int 	i;

	p_scb	= (volatile scb_t *)(pc_softc[unit].sram + OFFSET_SCB);
	p_cb	= (volatile ac_t *)(pc_softc[unit].sram + OFFSET_CU);

	pc586wtscb(unit);
	p_scb->scb_command = p_scb->scb_status & SCB_SW_INT;
	NOOP_DELAY
	NOOP_DELAY
	if (p_scb->scb_command) 
		pc586chatt(unit);
	pc586wtscb(unit);

	p_cb->ac_status	= 0;
	p_cb->ac_command = (AC_CONFIGURE|AC_CW_EL);

	/*
	 * below is the default board configuration from p2-28 from 586 book
	 */
	p_cb->cmd.configure.fifolim_bytecnt	= 0x080c;
	p_cb->cmd.configure.addrlen_mode	= 0x2600;
	p_cb->cmd.configure.linprio_interframe	= 0x6000;
	p_cb->cmd.configure.slot_time		= 0xf200;
	p_cb->cmd.configure.hardware		= 0x0000;
	p_cb->cmd.configure.min_frame_len	= 0x0040;

	p_scb->scb_command = SCB_CU_STRT;
	pc586chatt(unit);

	for (i = 0; ; ) {
		if ((p_cb->ac_status & AC_SW_OK))
			break;
		if (++i > 255)
			return FALSE;
	}

	p_scb->scb_command = p_scb->scb_status & SCB_SW_INT;
	NOOP_DELAY
	NOOP_DELAY
	if (p_scb->scb_command)
		pc586chatt(unit);
	pc586wtscb(unit);
	p_scb->scb_command = p_scb->scb_status & SCB_SW_INT;
	NOOP_DELAY
	NOOP_DELAY
	if (p_scb->scb_command)
		pc586chatt(unit);
	pc586wtscb(unit);

	p_cb->ac_status = 0;
	p_cb->ac_command = (AC_IASETUP|AC_CW_EL);

	pc586ehcpy(pc_softc[unit].address, p_cb->cmd.iasetup);

	p_scb->scb_command = SCB_CU_STRT;
	pc586chatt(unit);

	for (i = 0; ; ) {
		if ((p_cb->ac_status & AC_SW_OK))
			break;
		if (++i > 255)
			return FALSE;
	}

	p_scb->scb_command = p_scb->scb_status & SCB_SW_INT;
	pc586chatt(unit);
	return(TRUE);
}

/*
 * pc586wtscb:
 *
 *	This function waits until the board's command block is 
 *	available.  NOTE:  acceptance of a control command is 
 *	indicated by the 82586 clearing the SCB command field.
 *
 */

int 
pc586wtscb(unit)
	int unit;
{
	int	i;
	volatile scb_t *scb_p =
		(volatile scb_t *)(pc_softc[unit].sram + OFFSET_SCB);

	for (i = 0; i < 0xfffffff; i++)
		if (scb_p->scb_command == 0) 
			return;
	dprintf(("pc%d: wtscb returning without ACK\n", unit));
}

/*
 * mappc586: map the card into kernel vm and return the (virtual)
 * address.
 */
vm_offset_t
mappc586(physaddr, length)
	caddr_t physaddr;			/* start of card */
	vm_size_t length;			/* num bytes to map */
{
	vm_offset_t vmaddr;
	vm_offset_t pmap_map_bd();

	if (physaddr != (caddr_t)trunc_page(physaddr))
		panic("pc586 card not on page boundary");
	vmaddr = kmem_alloc_pageable(kernel_map, round_page(length));
	if (vmaddr == NULL)
		panic("can't alloc VM for pc586 card");
	(void)pmap_map_bd(vmaddr, (vm_offset_t)physaddr, 
			(vm_offset_t)physaddr+length, 
			VM_PROT_READ | VM_PROT_WRITE);
	return(vmaddr);
}

/*
 * Used if card not found, to free map entry.
 */
unmappc586(physaddr, length)
	caddr_t physaddr;			/* start of card */
	vm_size_t length;			/* num bytes to unmap */
{
	/* This doesn't quite undo pmap_map_bd, but it frees the kernel map */
	(void)vm_map_remove(kernel_map,trunc_page(physaddr),round_page(length));
}
#endif
