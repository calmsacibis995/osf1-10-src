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
static char	*sccsid = "@(#)$RCSfile: if_wd8003.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:08:51 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1990 Carnegie-Mellon University
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *   Author: Ron Weiss (rweiss)
 */

/*
 * Western Digital 8003E Mach Ethernet driver (for intel 80386)
 * Copyright (c) 1990 by Open Software Foundation (OSF).
 */

#include	<wd8003.h>
#if	NWD8003 > 0

#include	<sys/param.h>
#include	<mach/vm_param.h>
#include	<sys/systm.h>
#include	<sys/mbuf.h>
#include	<sys/table.h>
#include	<sys/buf.h>
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


#include	<i386/ipl.h>
#include	<i386/AT386/atbus.h>
#include	<i386/AT386/if_wd8003.h>
#include 	<i386/handler.h>
#include 	<i386/dispatcher.h>

#define	SPLNET	spl6

int	wd8003probe();
int	wd8003attach();
int	wd8003intr();
int	wd8003init();
int	wd8003start();
int	wd8003ioctl();
int	wd8003reset();
int	wd8003rcv();
int	wd8003watch();
int	wd8003get_CURR();
int	wd8003over_write();
static	char *wd80xxget_board_str();

int	(*wd8003intrs[])() = { wd8003intr, 0 };

struct	isa_dev	*wd8003info[NWD8003];

struct	isa_driver	wd8003driver =
	{ wd8003probe, 0, wd8003attach, "wd8003", 0, 0, 0 };

typedef struct {
	struct	arpcom	wd8003_ac;
#define	ds_if	wd8003_ac.ac_if
#define	ds_addr	wd8003_ac.ac_enaddr
	int	flags;
	volatile char *base;
	volatile char *sram;	/* beginning of the shared memory RAM buffer */
	int	read_nxtpkt;	/* rx page next */
	int	pstart_hold;	/* rx page start hold */
	int	pstop_hold;	/* rx page stop hold */
	int	tpsr_hold;	/* tx page start hold */
	int	fifo_depth;	/* NIC fifo threshold */
	int	board_id;
	u_char	address[ETHER_ADDR_SIZE];
	short	mode;
	ihandler_t	handler;
	ihandler_id_t	*handler_id;
} wd8003_softc_t;

wd8003_softc_t wd8003_softc[NWD8003];

u_char	imr_hold = PRXE|PTXE|RXEE|TXEE|OVWE;	/* Interrupt mask bits */

#ifdef	DEBUG
#define dprintf(foo)	printf foo
#else
#define dprintf(foo)
#endif

/*
 * wd8003probe:
 *
 *	This function "probes" or checks for the wd8003 board on the bus to see
 *	if it is there. As far as I can tell, the best break between this
 *	routine and the attach code is to simply determine whether the board
 *	is configured in properly. Currently my approach to this is to test the
 *	base I/O special offset for the Western Digital unique byte sequence
 *	identifier. If the bytes match we assume board is there.
 *	The config code expects to see a successful return from the probe
 *	routine before attach will be called.
 *
 * input	: address device is mapped to, and unit # being checked
 * output	: a '1' is returned if the board exists, and a 0 otherwise
 *
 */

wd8003probe(dev)
	struct isa_dev	*dev;
{
	volatile char *base = dev->dev_addr;
	int	unit = dev->dev_unit;

	if ((unsigned) unit >= NWD8003)
		return(0);
	/* check the address of the board to verify that it is a WD */
	if ((u_char) inb(base+LAR)  == (u_char) WD_NODE_ADDR_0 &&
	    (u_char) inb(base+LAR2) == (u_char) WD_NODE_ADDR_1 &&
	    (u_char) inb(base+LAR3) == (u_char) WD_NODE_ADDR_2) {
		wd8003_softc[unit].base = base;
		return (1);
	}
	return(0);
}

/*
 * wd8003attach:
 *
 *	This function attaches a WD8003 board to the "system".  The rest of
 *	runtime structures are initialized here (this routine is called after
 *	a successful probe of the board).  Once the ethernet address is read
 *	and stored, the board's ifnet structure is attached and readied.
 *
 * input	: isa_dev structure setup in autoconfig
 * output	: board structs and ifnet is setup
 *
 */

wd8003attach(dev)
	struct isa_dev	*dev;
{
	wd8003_softc_t	*sp;
	struct	ifnet	*ifp;
	int		unit;
	volatile char	*base;

	unit = dev->dev_unit;
	if ((unsigned) unit >= NWD8003)
		return;
	wd8003info[unit] = dev;
	sp = &wd8003_softc[unit];
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
		panic("Unable to add wd8003 interrupt handler");

	sp->sram = (volatile char *) phystokv(dev->dev_start);
	dev->dev_addr = (char *) phystokv(dev->dev_addr);
	base = sp->base;
	sp->flags = 0;
	sp->mode  = 0;
	*(sp->ds_addr)      = *(sp->address)      = inb(base+LAR);
	*(sp->ds_addr + 1)  = *(sp->address + 1)  = inb(base+LAR + 1);
	*(sp->ds_addr + 2)  = *(sp->address + 2)  = inb(base+LAR + 2);
	*(sp->ds_addr + 3)  = *(sp->address + 3)  = inb(base+LAR + 3);
	*(sp->ds_addr + 4)  = *(sp->address + 4)  = inb(base+LAR + 4);
	*(sp->ds_addr + 5)  = *(sp->address + 5)  = inb(base+LAR + 5);

	outb(base+MSR, MENB);			/* enable mem access to board */
	wd8003_softc[unit].board_id = wd80xxget_board_id(unit);
	if (!wd8003config(unit))
		printf("wd%d: configure failed\n", unit);

	else printf("wd%d: %s ethernet id [%x:%x:%x:%x:%x:%x] %dKB irq = %d\n",
		unit, wd80xxget_board_str(unit),
		sp->address[0],sp->address[1],sp->address[2],
		sp->address[3],sp->address[4],sp->address[5],
		sp->pstop_hold >> 2, dev->dev_pic);

	sp->wd8003_ac.ac_bcastaddr = (u_char *)etherbroadcastaddr;
	sp->wd8003_ac.ac_arphrd = ARPHRD_ETHER;

	ifp = &(sp->ds_if);
	ifp->if_unit = unit;
	ifp->if_name = "wd";
	ifp->if_mtu = ETHERMTU - 8;
	ifp->if_timer = 0;
	ifp->if_flags = IFF_BROADCAST|IFF_SIMPLEX|IFF_NOTRAILERS;
	ifp->if_type = IFT_ETHER;
	ifp->if_addrlen = 6;
	ifp->if_hdrlen = sizeof (struct ether_header) + 8; /* 8 for SNAP */

	ifp->if_init = wd8003init;
	ifp->if_output = ether_output;
	ifp->if_start = wd8003start;	
	ifp->if_ioctl = wd8003ioctl;
	ifp->if_reset = wd8003reset;
	ifp->if_watchdog = wd8003watch;

	if_attach(ifp);
}

/*
 * wd8003watch():
 *
 */

int
wd8003watch(unit)
	int	unit;
{
	if (inb(wd8003_softc[unit].base+CR) & TXP) {
		wd8003_softc[unit].ds_if.if_oerrors++;
		printf("wd%d: transmit timeout: check transceiver\n", unit);
		wd8003init(unit);
	} else {
		dprintf(("wd%d: lost transmit interrupt\n", unit));
		wd8003start(&wd8003_softc[unit].ds_if);
	}
}


/*
 * wd8003reset:
 *
 *	This routine is in part an entry point for the "if" code.  Since most
 *	of the actual initialization has already (we hope already) been done
 *	by calling wd8003attach().
 *
 * input	: unit number or board number to reset
 * output	: board is reset
 *
 */

int
wd8003reset(unit)
	int	unit;
{
	wd8003_softc[unit].ds_if.if_flags &= ~IFF_RUNNING;
	return(wd8003init(unit));
}

/*
 * wd8003init:
 *
 *	Another routine that interfaces the "if" layer to this driver.
 *	Simply resets the structures that are used by "upper layers".
 *	As well as calling wd8003hwrst that does reset the wd8003 board.
 *
 * input	: board number
 * output	: structures (if structs) and board are reset
 *
 */

int
wd8003init(unit)
	int	unit;
{
	struct	ifnet	*ifp;
	int	stat, oldpri;

	ifp = &(wd8003_softc[unit].ds_if);
	if (ifp->if_addrlist == NULL)
		return FALSE;
	oldpri = SPLNET();
	if ((stat = wd8003hwrst(unit)) == TRUE) {
		wd8003_softc[unit].ds_if.if_flags |= IFF_RUNNING;
		wd8003_softc[unit].flags |= DSF_RUNNING;
		wd8003start(ifp);
	}
	splx(oldpri);
	return stat;
}


wd8003start(ifp)
	struct ifnet *ifp;
{
	struct	mbuf	*m;
 	int opri = splimp();

again:
	IF_DEQUEUE(&ifp->if_snd, m);
	if (m) {
		ifp->if_flags |= IFF_OACTIVE;
		ifp->if_timer = 2;
		if (!wd8003xmt(ifp->if_unit, m)) {
			m_freem(m);
			goto again;
		}
		m_freem(m);
	} else {
		ifp->if_flags &= ~IFF_OACTIVE;
		ifp->if_timer = 0;
	}
	splx(opri);
}

/*
 * wd8003ioctl:
 *
 *	This routine processes an ioctl request from the "if" layer
 *	above.
 *
 * input	: pointer the appropriate "if" struct, command, and data
 * output	: based on command appropriate action is taken on the
 *	 	  wd8003 board(s) or related structures
 * return	: error is returned containing exit conditions
 *
 */

int
wd8003ioctl(ifp, cmd, data)
	struct ifnet	*ifp;
	int	cmd;
	caddr_t	data;
{
	register struct ifaddr *ifa = (struct ifaddr *)data;
	register wd8003_softc_t *is;
	int opri, error;
	short mode = 0;

	is = &wd8003_softc[ifp->if_unit];
	opri = SPLNET();
	error = 0;
	switch (cmd) {
	case SIOCSIFADDR:
		ifp->if_flags |= IFF_UP;
		wd8003init(ifp->if_unit);
		switch (ifa->ifa_addr->sa_family) {
		case AF_INET:
			((struct arpcom *)ifp)->ac_ipaddr =
			    IA_SIN(ifa)->sin_addr;
			arpwhohas((struct arpcom *)ifp, &IA_SIN(ifa)->sin_addr);
			break;
		case AF_NS: {
			register struct ns_addr *ina =
			    &(IA_SNS(ifa)->sns_addr);
			if (ns_nullhost(*ina))
				ina->x_host = *(union ns_host *)(is->ds_addr);
			else
#ifdef	notyet
				wd8003seteh(ina->x_host.c_host,
				    wd8003_softc[ifp->if_unit].base);
#else
				error = EOPNOTSUPP;
#endif
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
				wd8003init(ifp->if_unit);
			}
		}
		if ((ifp->if_flags & IFF_UP) == 0 && (is->flags & DSF_RUNNING)){
			dprintf(("wd%d ioctl: turning off board\n", ifp->if_unit));
			is->flags &= ~(DSF_LOCK | DSF_RUNNING);
			ifp->if_timer = 0;
			outb(is->base+CR, ABR|STP|PS0);
		} else if ((ifp->if_flags & IFF_UP)&&!(is->flags & DSF_RUNNING))
			wd8003init(ifp->if_unit);
		break;
	default:
		error = EINVAL;
	}
	splx(opri);
	return (error);
}

/*
 * wd8003hwrst:
 *
 *	This routine resets the wd8003 board that corresponds to the
 *	board number passed in.
 *
 * input	: board number to do a hardware reset
 * output	: board is reset
 *
 */

int
wd8003hwrst(unit)
	int unit;
{
	volatile char *base;

	if (wd8003config(unit) == FALSE) {
		printf("wd%d: failed to configure\n", unit);
		return(FALSE);
	}

	/* start board again, with page register 0, and cancel reset */
	base = wd8003_softc[unit].base;
	outb (base+CR, ABR|STA|PS0);
	outb (base+TCR, 0);		/* Enable receive */
	outb (base+IMR, imr_hold);	/* Enable interrupts */
	return(TRUE);
}

/*
 * wd8003intr:
 *
 *	This function is the interrupt handler for the wd8003 ethernet
 *	board.  This routine will be called whenever either a packet
 *	is received, or a packet has successfully been transfered and
 *	the unit is ready to transmit another packet.
 *
 * input	: board number that interrupted
 * output	: either a packet is received, or a packet is transfered
 *
 */

int
wd8003intr(unit)
{
	int	cnt;
	int	isr_status;
	volatile char *base;
	int	temp_cr;
	struct	ifnet	*ifp;

	if ((base = wd8003_softc[unit].base) == NULL)
		return 0;
	if ((isr_status = (inb(base+ISR) & 0x3f)) == 0)	/* ignore RAZ(?) bits */
		return 0;
	temp_cr = inb(base+CR);
	outb(base+CR, (temp_cr & 0x3f) | PS0);
	outb(base+IMR, 0);			/* stop board interrupts */
	ifp = &wd8003_softc[unit].ds_if;
	if ((ifp->if_flags & (IFF_UP|IFF_RUNNING)) != (IFF_UP|IFF_RUNNING)) {
		dprintf(("wd%d: interrupt: turning off board\n", unit));
		outb(base+ISR, 0xFF);
		outb(base+CR, ABR|STP|PS0);
		return 1;
	}
again:
	outb(base+ISR, isr_status);		/* clear interrupt status */
	if (isr_status & (OVW|PRX|RXE)) {
		if ((cnt = wd8003rcv(unit)) > 0)
			ifp->if_ipackets += cnt;
		if (isr_status & OVW) {
			dprintf(("wd%d: overwrite #%d.\n", unit, cnt));
			ifp->if_ierrors++;
		} else if (cnt < 0 || (isr_status & RXE))
			ifp->if_ierrors++;
	}
	if (isr_status & (TXE|PTX)) {
		if (isr_status & TXE) {
			int tsr_status = inb(base+TSR);
			if (tsr_status & (ABT|CRS|OWC|FU)) {
				ifp->if_oerrors++;
				if (tsr_status & ABT)
					printf("wd%d: cable jammed: check transceiver\n", unit);
				else if (tsr_status & OWC)
					printf("wd%d: late collision: check transceiver\n", unit);
			} else {
				if (tsr_status & COL)
					ifp->if_collisions++;
				ifp->if_opackets++;
			}
		} else
			ifp->if_opackets++;
		wd8003start(ifp);
	}
	if (isr_status = (inb(base+ISR) & 0x3f))
		goto again;
	outb(base+IMR, imr_hold);		/* re-enable board interrupts */
	return 1;
}

/*
 * wd8003rcv:
 *
 *	This routine is called by the interrupt handler to initiate a
 *	packet transfer from the board to the "if" layer above this
 *	driver.  This routine checks if a buffer has been successfully
 *	received by the wd8003.  If so, it does the actual transfer of the
 *	board data (including the ethernet header) into a packet (consisting
 *	of an mbuf chain) and enqueues it to a higher level.
 *	Then check again whether there are any packets in the receive ring,
 *	if so, read the next packet, until there are no more.
 *
 * input	: number of the board to check
 * output	: if a packet is available, it is "sent up"
 */

int
wd8003rcv(unit)
	int unit;
{
	register wd8003_softc_t *is;
	register struct ifnet *ifp;
	struct ether_header eh;
	struct mbuf *m, *top, **mp;	/* initial allocation of mem; temp */
	int len, wrap_len;		/* decremented value of pkt size */
	volatile char *base;
	int temp_cr;
	int packets = 0;
	int nxtpkt;			/* NIC's next pkt ptr in rcv header */
	volatile char *sram_pkt_ptr;	/* mem location of packet/data */
	struct wdstat {
		u_char status;
		u_char nxtpkt;
		short  len;
	} rcvstatus;

	is = &wd8003_softc[unit];
	ifp = &is->ds_if;
	base = is->base;

	while (is->read_nxtpkt != wd8003get_CURR(unit)) {

		/*
		 * Contrary to the manual, the chip writes the status
		 * words LAST, at least as far as the CPU sees them.
		 */
		sram_pkt_ptr = is->sram + (is->read_nxtpkt << 8);
#define TRIES	100	/* 1 has proved sufficient */
		for (wrap_len = 0; wrap_len < TRIES; ++wrap_len) {
			rcvstatus = *((volatile struct wdstat *)(sram_pkt_ptr));
			nxtpkt = rcvstatus.nxtpkt;
			len = rcvstatus.len;
			if (nxtpkt >= is->pstart_hold &&
			    nxtpkt < is->pstop_hold && len > 0)
				break;
		}
		if (wrap_len) dprintf(("wd%d: status waited %d %s\n",
			unit, wrap_len, wrap_len < TRIES ? "ok" : "ng"));

		if (nxtpkt < is->pstart_hold || nxtpkt >= is->pstop_hold) {
			dprintf(("wd%d: nextpkt %d out of range!\n",unit,nxtpkt));
			wd8003hwrst(unit);
			if (packets == 0)
				packets = -1;
			break;
		}

		packets++;
		top = 0;
		len -= (sizeof eh + 4);		/* drop header and checksum */
		if (len < ETHERMIN || len > ETHERMTU) {
			dprintf(("wd%d: bogon %d\n", unit, len));
			goto skip;
		}

		sram_pkt_ptr += NIC_HEADER_SIZE;
		eh = *(struct ether_header *)(sram_pkt_ptr);
		sram_pkt_ptr += sizeof eh;
		NTOHS(eh.ether_type);

		m = m_gethdr(M_DONTWAIT, MT_DATA);
		if (m == NULL) {
nomem:			wd8003set_BNDY(unit, wd8003get_CURR(unit));
			break;
		}

		m->m_pkthdr.rcvif = ifp;
		m->m_pkthdr.len = len;
		m->m_len = MHLEN;
		mp = &top;
		/* Bcopy optimization: longword align with no dribble. */
		sram_pkt_ptr -= 2;
		len = (len + 5) & ~3;
		while (len > 3) {
			if (top) {
				m = m_get(M_DONTWAIT, MT_DATA);
				if (m == NULL) {
					m_freem(top);
					goto nomem;
				}
				m->m_len = MLEN;
			}
			if (len > MINCLSIZE) {
				MCLGET(m, M_DONTWAIT);
				if (m->m_flags & M_EXT)
					m->m_len = MCLBYTES;
			}
			if (len < m->m_len) {
				/*
				* Place initial small header at end of mbuf
				*/
				if (top == 0 && len + max_linkhdr <= m->m_len)
					m->m_data += max_linkhdr;
				m->m_len = len;
			}

			wrap_len = (is->sram+(is->pstop_hold<<8))-sram_pkt_ptr;
			if (m->m_len < wrap_len) {
				bcopy(sram_pkt_ptr, mtod(m, char *), m->m_len);
				sram_pkt_ptr += m->m_len;
			} else {
				bcopy(sram_pkt_ptr, mtod(m, char *), wrap_len);
				sram_pkt_ptr = is->sram+(is->pstart_hold << 8);
				if (m->m_len > wrap_len) {
					bcopy(sram_pkt_ptr,
						mtod(m, char *) + wrap_len,
						m->m_len - wrap_len);
					sram_pkt_ptr += (m->m_len - wrap_len);
				}
			}
			len -= m->m_len;
			*mp = m;
			mp = &m->m_next;
		}
skip:
		/* Update next read operation and DMA boundary register */
		wd8003set_BNDY(unit, nxtpkt);

		if (top) {
			/* Drop extra 2 bytes of alignment */
			top->m_data += 2; top->m_len -= 2;
			/* Remainder/overage: 0/2 1/1 2/0 3/3 */
			if (len == 0)
				m->m_len -= ((2 - (top->m_pkthdr.len & 3)) & 3);
			ether_input(ifp, &eh, top);
		}
	}
	return packets;
}

/*
 *  wd8003set_BNDY
 *  this routine is called to set the next receive and DMA boundary.
 */
wd8003set_BNDY(unit, nxt)
	int unit, nxt;
{
	int temp_cr;
	wd8003_softc_t *is;
	volatile char  *base;

	is = &wd8003_softc[unit];
	base = is->base;
	/* XXX This is how we stay in sync with the chip's receiver */
	do {
		*((long *)(is->sram + (is->read_nxtpkt << 8))) = 0;
		if (++is->read_nxtpkt == is->pstop_hold)
			is->read_nxtpkt = is->pstart_hold;
	} while (is->read_nxtpkt != nxt);
	/* XXX The boundary needs to be back by 1 to make the chip happy. */
	if (--nxt < is->pstart_hold)
		nxt = is->pstop_hold-1;
	temp_cr = inb(base+CR);
	outb(base+CR, (temp_cr & 0x3F) | PS0);
	outb(base+BNDY, nxt);
}

/*
 * wd8003get_CURR():
 *
 * Returns the value of the register CURR, which points to the next
 * available space for NIC to receive from network into receive ring.
 *
 */

int
wd8003get_CURR(unit)
	int unit;
{
	int temp_cr, curr;
	volatile char *base = wd8003_softc[unit].base;

	temp_cr = inb(base+CR);			/* get current CR value */
	outb(base+CR, (temp_cr & 0x3F) | PS1);	/* select page 1 registers */
	curr = inb(base+CURR) & 0xFF;		/* read CURR value */
	outb(base+CR, temp_cr);			/* restore CR page */
	return curr;
}

/*
 * wd8003xmt:
 *
 *	This routine fills in the appropriate registers and memory
 *	locations on the WD8003 board and starts the board off on
 *	the transmit.
 *
 * input	: board number of interest, and a pointer to the mbuf
 * output	: board memory and registers are set for xfer and attention
 *
 */

wd8003xmt(unit, m)
	int	unit;
	struct	mbuf	*m;
{
	int count = 0;			/* amount of data already copied */
	volatile char *base;
	wd8003_softc_t *is;
	volatile char *sram_write_pkt;	/* tx buffer phys address */

	is = &wd8003_softc[unit];
	base = is->base;
	sram_write_pkt = is->sram;	/* + (is->tpsr_hold << 8) (== 0) */

	do {
		if (m->m_len == 0)
			continue;
		if (count + m->m_len > ETHERMTU+sizeof(struct ether_header))
			return 0;
		bcopy(mtod(m, caddr_t), sram_write_pkt + count, m->m_len);
		count += m->m_len;
	} while (m = m->m_next);
	if (count < ETHERMIN + sizeof(struct ether_header)) {
		/* optional bzero(sram_write_pkt,
			ETHERMIN + sizeof(struct ether_header) - count);*/
		count = ETHERMIN + sizeof(struct ether_header);
	}
	outb(base+CR, ABR|STA|PS0);		/* select page 0 */
	outb(base+TPSR, is->tpsr_hold);		/* xmt page start at 0 of RAM */
	outb(base+TBCR1, count >> 8);		/* upper byte of count */
	outb(base+TBCR0, count & 0xFF);		/* lower byte of count */
	outb(base+CR, TXP|ABR|STA|PS0);		/* start transmission */
	return 1;
}

/*
 * wd8003config:
 *
 *	This routine does a standard config of a wd8003 family board, with
 *	the proper modifications to different boards within this family.
 *
 */

wd8003config(unit)
	int	unit;
{
	int	i, temp, RAMsize;
	volatile char *base, *RAMbase;
	wd8003_softc_t *is;

	is = &wd8003_softc[unit];
	base = is->base;
	is->tpsr_hold = 0;		/* transmit page start hold - 6 pgs*/
	is->pstart_hold = 6;		/* receive page start hold */
	is->fifo_depth = 8;		/* NIC fifo threshold */

	switch (is->board_id & RAM_SIZE_MASK) {
	default:	/* if RAM size unknown, assume 8K */
	case RAM_SIZE_8K:  RAMsize = 0x2000; break;
	case RAM_SIZE_16K: RAMsize = 0x4000; break;
	case RAM_SIZE_32K: RAMsize = 0x8000; break;
	case RAM_SIZE_64K: RAMsize = 0x10000; break;
	}

	/* enable 16 bit access from lan controller */
	if (((is->board_id & WD8013EBT) == WD8013EBT) &&
	    (is->board_id & SLOT_16BIT))
		outb(base+LAAR, LAN16ENB | LA19 | MEM16ENB | SOFTINT);

	RAMbase = wd8003info[unit]->dev_start;
	temp = ((int)(RAMbase) >> 13) & 0x3f;	/* convert for MSR */

	outb (base+MSR, temp | MENB);		/* initialize MSR */
	outb (base+CR, ABR|STP|PS0);		/* soft reset and page 0 */
	for (i = 0; ; ) {
		if (inb(base+ISR) & RST)
			break;
		if (++i == 4096)
			return FALSE;
	}
	bzero(is->sram, RAMsize);
	temp = ((is->fifo_depth & 0x0c) << 3) | BMS;	/* fifo depth | !loop */
	if (is->board_id & SLOT_16BIT)
		temp |= WTS;		/* word xfer select (16 bit cards ) */
	outb (base+DCR, temp);
	outb (base+RBCR0, 0);		/* clear remote byte count */
	outb (base+RBCR1, 0);
	outb (base+RCR, AB);		/* receive configuration register */
	outb (base+TCR, 0);
	/** Init of Receive Ring RAM Buffer **/
	outb (base+PSTART, is->pstart_hold);/* receive ring start */
	is->pstop_hold = (((int)RAMsize >> 8) & 0xff);/* rcv page stop hold */
	outb (base+PSTOP, is->pstop_hold);  /* stop at last RAM buffer loc */
	outb (base+BNDY, is->pstop_hold);   /* boundary pointer for page 0 */
	is->read_nxtpkt = is->pstart_hold;
	outb (base+ISR, 0xff);		/* clear all interrupt status bits */
	outb (base+IMR, 0);		/* no interrupts yet */
	outb (base+CR, ABR|STP|PS1);	/* maintain rst | sel page 1 */
	outb (base+CURR, is->pstart_hold); /* Current page register */
	for(i=0; i<ETHER_ADDR_SIZE; i++) {
		outb (base+PAR0+i, wd8003_softc[unit].ds_addr[i]);
		/* load physical address */
		outb (base+MAR0+i, 0);	/* other multicast bits are 0 */
	}
	outb (base+MAR6, 0);		/* more multicast address registers */
	outb (base+MAR7, 0);
	return TRUE;
}

/*
 *	wd80xxget_board_id:
 *
 *	determine which board is being used.
 *	Currently supports:
 *		wd8003E
 *		wd8003EBT
 *		wd8013EBT
 *
 */
int
wd80xxget_board_id(unit)
	int unit;
{

	wd8003_softc_t *sp;
	long		board_id = 0;
	volatile char *base;
	int		reg_temp;
	int		rev_num;			/* revision number */
	int		ram_flag;
	int		intr_temp;

	sp = &wd8003_softc[unit];
	base = sp->base;
	board_id |= ETHERNET_MEDIA;		/* assume Ethernet media */

	/* check if board is a WD8013EBT */
	if (inb(base+0x0e) == 0x05 ) {
		/* board is 16 bits */
		board_id |= BOARD_16BIT;
		reg_temp &= 0xFE;
		outb(base + REG_1, reg_temp);
		/* check if in 16 bit slot */
		if (inb(base + REG_1) & BID_SIXTEEN_BIT_BIT)
			board_id |= SLOT_16BIT;
	}

	rev_num = (inb(base + BOARD_ID_BYTE) & BID_REV_MASK) >> 1;
	if (rev_num < 2) {
		if (board_id & BOARD_16BIT)
			board_id |= RAM_SIZE_16K;
		else {
			if (board_id & INTERFACE_CHIP) {
				if (inb(base + REG_1) & BID_MSZ_583)
					board_id |= RAM_SIZE_32K;
				else
					board_id |= RAM_SIZE_8K;
			} /* else decide later? */
		}
	} else {
		ram_flag = inb(base + BOARD_ID_BYTE) & BID_RAM_SIZE_BIT;
		switch ((int) (board_id & STATIC_ID_MASK)) {
		case WD8003E:
		case WD8003EB:
			if (ram_flag)
				board_id |= RAM_SIZE_32K;
			else
				board_id |= RAM_SIZE_8K;
			break;
		case WD8013EBT:
			if (ram_flag)
				board_id |= RAM_SIZE_32K;
			else
				board_id |= RAM_SIZE_16K;
			break;
		default:
			board_id |= RAM_SIZE_8K;
		}
	}
	if ((board_id & WD8003EB) == WD8003EB) {
	/* program the WD83C583 EEPROM registers */
		if (ram_flag)
			outb(base+ICR, inb(base+ICR) | DMAE | IOPE | MSZ);
		else
			outb(base+ICR, inb(base+ICR) | DMAE | IOPE);
		/* attempt to set interrupt according to assigned pic */
		switch(wd8003info[unit]->dev_pic) {
		case 2: outb(base+IRR, IEN);
			break;
		case 3: outb(base+IRR, IEN|IR0);
			break;
		case 4: outb(base+IRR, IEN|IR1);
			break;
		case 7: outb(base+IRR, IEN|IR0|IR1);
			break;
		default: printf("\nWD8003: unsupported pic line %d - must be [2,3,4,7]\n",
			wd8003info[unit]->dev_pic);
		}
	}

	return (board_id);
}

static char *
wd80xxget_board_str(unit)
	int unit;
{
	switch (wd8003_softc[unit].board_id & STATIC_ID_MASK) {
	case WD8003E:
		return "wd8003E";
	case WD8003EB:
		return "wd8003EB";
	case WD8013EBT:
		return "wd8013EBT";
	default:
		return "wd80???";
	}
}

#ifdef DEBUG
wdpr(unit)
{
	volatile char *base = wd8003_softc[unit].base;
	int temp_cr;
	
	base += CR;
	temp_cr = inb(base);			/* get current CR value */

	printf("CR %x, BNDRY %x, TSR %x, NCR %x, FIFO %x, ISR %x, RSR %x\n",
		inb(base+0x0), inb(base+0x3), inb(base+0x4), inb(base+0x5),
		inb(base+0x6), inb(base+0x7), inb(base+0xc));
	printf("CLD %x:%x, CRD %x:%x, FR %x, CRC %x, Miss %x\n",
		inb(base+0x1), inb(base+0x2),
		inb(base+0x8), inb(base+0x9),
		inb(base+0xd), inb(base+0xe), inb(base+0xf));

	
	outb(base, (temp_cr&0x3f)|PS1);		/* page 1 CR value */
	printf("PHYS %x:%x:%x:%x:%x CUR %x\n",
		inb(base+0x1), inb(base+0x2), inb(base+0x3),
		inb(base+0x4), inb(base+0x5), inb(base+0x6),
		inb(base+0x7));
	printf("MAR %x:%x:%x:%x:%x:%x:%x:%x\n",
		inb(base+0x8), inb(base+0x9), inb(base+0xa), inb(base+0xb),
		inb(base+0xc), inb(base+0xd), inb(base+0xe), inb(base+0xf));
	outb(base, temp_cr);			/* restore current CR value */
}
#endif
#endif
