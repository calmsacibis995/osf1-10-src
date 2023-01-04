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
static char	*sccsid = "@(#)$RCSfile: if_emc.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:43:57 $";
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
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *        Copyright 1986 Encore Computer Corporation
 *
 * ALL RIGHTS RESERVED. Licensed Material - Property of Encore Computer
 * Corporation. This software is made available solely pursuant to the
 * terms of a software license agreement which governs its use. 
 * Unauthorized duplication, distribution or sale are strictly prohibited.
 *
 * Module Function:
 * 	EMC Ethernet driver.
 *
 * Original Author: F. Oliveira	    Created on: 06/05/86
 *
 */

#include <mmax_debug.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/buf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/vmmac.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/table.h>

#include <net/if.h>
#include <net/if_types.h>
#include <net/netisr.h>
#include <net/route.h>

/* Domain specific stuff */
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>

#include <netns/ns.h>
#include <netns/ns_if.h>
/* end domain specific */

#include <kern/lock.h>
#include <kern/queue.h>

#include <mmax/isr_env.h>
#include <mmaxio/io.h>
#include <mmaxio/crqdefs.h>
#include <mmaxio/emcdefs.h>
#include <mmaxif/if_emc.h>
 
#include <emc.h>

/*
 *	Useful mbuf macros.
 */

/* Usable length of an mbuf for transmission due to alignment problem */
#define EN_T_MLEN	(MLEN & ~7)

/* Macro to add a cluster to an mbuf */
#define EN_MCLADD(m,w,fail_stmt) \
	(m)->m_len = 0; MCLGET(m, w); \
	if (((m)->m_flags & M_EXT) == 0) fail_stmt

extern	struct	devaddr	emc_devaddr[];

struct mbuf *m_get_special();
int	mcladd_special();
int	enattach();
int	en_lrecv(), en_srecv(), en_senddone();
int	eninit(),enoutput(),enioctl(),enreset(),enwatch();
int	ensrcmdpost(),enlrcmdpost(),engetstats();
char	eth_attn_msg[] = "\nEthernet Attention received:\nslot = %d, code = 0x%x, status = 0x%x, Xstatus = 0x%x\n";

/*
 * Ethernet software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * en_if, which the routing code uses to locate the interface.
 * This structure contains the output queue for the interface, its address, ...
 */
struct en_softc {
	struct	arpcom en_ac;		/* Ethernet common part */
#define en_if	en_ac.ac_if		/* network-visible interface */
#define en_addr	en_ac.ac_enaddr		/* hardware Ethernet address */

	u_short en_istate;		/* interface state */
	u_short en_dstate;		/* device state */

	struct	crq_maxsize_msg    sap_attn[NO_EN_SATTN];
	crq_t	en_send_crq;
	struct	crq_en_xmit_msg	scp_list[NO_EN_SENDS];
	mpqueue_head_t	en_free_que;	/* free send bufs */

	/* Large receive frame CRQ and associated attention blocks */
	crq_t	en_lrbuf_crq;
	struct	crq_maxsize_msg    rap_lattn[NO_EN_LRATTN];
	mpqueue_head_t en_ldead_que;	/* dead lrecv cmd blocks */

	/* Small receive frame CRQ and associated attention blocks */
	crq_t	en_srbuf_crq;
	struct	crq_maxsize_msg    rap_sattn[NO_EN_SRATTN];
	mpqueue_head_t en_sdead_que;	/* dead srecv cmd blocks */

	struct	crq_en_rcv_msg	rcp_list[NO_EN_LRCVS + NO_EN_SRCVS];

	/* Statistics buffer for read and reset statistics operation */
	struct	eth_stats	en_stats;
} en_softc[NEMC];

/*
 * For statistics gathering.
 */
struct	eth_stats	*en0stats = &en_softc[0].en_stats;

#define	MMAX_FASTSTATS
#undef	MMAX_FASTSTATS

#ifdef	MMAX_FASTSTATS
static	void	faststats();
#endif

/*
 * Make interface available by filling in network interface
 * record.  System will initialize the interface when it is ready
 * to accept packets.
 */
enattach(unit)
{
	struct	en_softc	*en = &en_softc[unit];
	struct	ifnet	*ifp = &en->en_if;
	struct	crq_en_rcv_msg	*rmsg;
	struct	crq_en_xmit_msg	*tmsg;
	int	errno, i;

	if(unit >= NEMC) {
		printf("EN attach - unit %d out of range\n", unit);
		return (EIO);
	}

	if (en->en_istate != 0) {
	    return (EIO);
	}
	
	init_crq(&en->en_lrbuf_crq, CRQMODE_INTR, 0,
	     MAKEUNITID(0, emc_devaddr[unit].v_chan, 0, EMCLUN_EN_RCVL), NULL);
	mpqueue_init(&en->en_ldead_que);

	if ((errno = alloc_vector(&en->en_lrbuf_crq.crq_master_vect,
		en_lrecv, en, INTR_DEVICE)) ||
	(errno = polled_create_chan (&en->en_lrbuf_crq, en_lrecv, en))) {
	    printf("EN attach - Can not create large rcv channel (%d) on unit %d\n", errno, unit);
	    goto init_exit;
	}

	for(i = 0; i < NO_EN_LRATTN; i++)
		put_free(&en->rap_lattn[i], &en->en_lrbuf_crq);

	for(rmsg = en->rcp_list;rmsg < &en->rcp_list[NO_EN_LRCVS];rmsg++) 
		(void) enlrcmdpost(rmsg, en, M_WAIT);

	init_crq(&en->en_srbuf_crq, CRQMODE_INTR, 0,
	     MAKEUNITID(0, emc_devaddr[unit].v_chan, 0, EMCLUN_EN_RCVS), NULL);
	mpqueue_init(&en->en_sdead_que);

	if ((errno = alloc_vector (&en->en_srbuf_crq.crq_master_vect,
		en_srecv, en, INTR_DEVICE)) ||
	(errno = polled_create_chan (&en->en_srbuf_crq, en_srecv, en))) {
	    printf("EN attach - Can not create small rcv channel (%d) on unit %d\n",errno, unit);
	    (void) polled_delete_chan (&en->en_lrbuf_crq);
	    (void) dealloc_vector (&en->en_lrbuf_crq.crq_master_vect);
	    goto init_exit;
	}

	for (i = 0; i < NO_EN_SRATTN; i++)
		put_free(&en->rap_sattn[i], &en->en_srbuf_crq);

	for(; rmsg < &en->rcp_list[NO_EN_SRCVS+NO_EN_LRCVS];rmsg++) 
		(void) ensrcmdpost(rmsg, en, M_WAIT);
	   
	/* Initialize the fields in the send crq. */
	init_crq (&en->en_send_crq, CRQMODE_INTR, 0,
	     MAKEUNITID (0, emc_devaddr[unit].v_chan, 0, EMCLUN_EN_CMD), NULL);

	for (i = 0; i < NO_EN_SATTN; i++)
		put_free(&en->sap_attn[i], &en->en_send_crq);

	mpqueue_init(&en->en_free_que);
	for(tmsg = en->scp_list; tmsg < &en->scp_list[NO_EN_SENDS]; tmsg++) {
		tmsg->en_xmit_hdr.em_msg_hdr.crq_msg_status = STS_FREE;
		mpenqueue_tail(&en->en_free_que, tmsg);
	}

	/* Tell the ethernet about us by creating a command channel,
	 * filling in the channel vectors.
	 */
	if ((errno = alloc_vector(&en->en_send_crq.crq_master_vect,
		en_senddone, en, INTR_DEVICE)) ||
	(errno = polled_create_chan (&en->en_send_crq, en_senddone, en))) {
	    printf("EN attach - Can not allocate send channel (%d) on unit %d\n",errno, unit);
	    (void) polled_delete_chan (&en->en_lrbuf_crq);
	    (void) dealloc_vector (&en->en_lrbuf_crq.crq_master_vect);
	    (void) polled_delete_chan (&en->en_srbuf_crq);
	    (void) dealloc_vector (&en->en_srbuf_crq.crq_master_vect);
	    goto init_exit;
	}
	
	/* now send a read and reset stats command to the device to
		get hardware address.  (returned stats are junk.) */

	en->en_istate = EN_SINITING;	/* will be EN_SINIT when read&reset
						stats cmd. completes */
	if (engetstats(en)) {
	    printf("EN attach: aborting attach operation for unit %d.\n", unit);
	    (void) polled_delete_chan (&en->en_lrbuf_crq);
	    (void) dealloc_vector (&en->en_lrbuf_crq.crq_master_vect);
	    (void) polled_delete_chan (&en->en_srbuf_crq);
	    (void) dealloc_vector (&en->en_srbuf_crq.crq_master_vect);
	    (void) polled_delete_chan (&en->en_send_crq);
	    (void) dealloc_vector (&en->en_send_crq.crq_master_vect);
	    en->en_istate = 0;
	    goto init_exit;
	}
	
	en->en_ac.ac_bcastaddr = (u_char *)etherbroadcastaddr;
	en->en_ac.ac_arphrd = ARPHRD_ETHER;

	/* setup if interface so system knows about us */
	ifp->if_unit = unit;
	ifp->if_name = "en";
	ifp->if_mtu = ETHERMTU;
	ifp->if_type = IFT_ETHER;
	ifp->if_addrlen = 6;
	ifp->if_hdrlen = sizeof (struct ether_header) + 8;	/* 8 for SNAP */

	ifp->if_init = eninit;
	ifp->if_output = ether_output;
	ifp->if_start = enoutput;
	ifp->if_ioctl = enioctl;
	ifp->if_reset = enreset;

	ifp->if_flags |= IFF_BROADCAST;
	if_attach(ifp);

init_exit:
	return;
}

#ifdef	MMAX_FASTSTATS
static void
faststats(en)
struct	en_softc *en;
{
	extern int hz;

	if(en->en_istate == EN_SINIT) {
		engetstats(en);

		timeout(faststats, en, hz);
	}
}
#endif	/* MMAX_FASTSTATS */

/*
 * Initialization of interface; clear recorded pending
 * operations.
 * Called at ifconfig time via enioctl, with interrupts disabled.
 */
eninit(unit)
	int unit;
{
	struct en_softc *en = &en_softc[unit];
	struct ifnet *ifp = &en->en_if;
	struct	crq_en_cntl_msg	*c_msg;

	if (ifp->if_flags & IFF_RUNNING)
		return;

	mpdequeue_head(&en->en_free_que, (queue_entry_t *)&c_msg);

	if(c_msg == 0) {
		printf("EN init: could not get cmd buffer for unit %d\n", unit);
		en->en_if.if_flags &= ~IFF_UP;
		return;
	}

	c_msg->en_cntl_hdr.em_msg_hdr.crq_msg_code = CRQOP_EN_CONTROL;
	c_msg->en_cntl_hdr.em_msg_hdr.crq_msg_unitid=en->en_send_crq.crq_unitid;
	c_msg->en_cntl_subcode = ENSUB_SET_ONLINE;

	send_cmd(c_msg, &en->en_send_crq);
	enstart(unit);				/* start transmits */
	ifp->if_watchdog = enwatch;
	ifp->if_timer = EN_TIMEOUT;
	en->en_if.if_flags |= IFF_RUNNING;
}

/*
 * Start or re-start output on interface.
 * Get another datagram to send off of the interface queue,
 * and map it to the interface before starting the output.
 * This routine is called by eninit(), enoutput(), and en_senddone().
 * In all cases, interrupts by EXOS are disabled.
 */
enstart(unit)
	int unit;
{
struct en_softc *en = &en_softc[unit];
struct crq_en_xmit_msg *tmsg;
struct mbuf *m;
caddr_t	phys, pmap_resident_extract();
int len;

	if(en->en_dstate != EN_ONLINE)
		return;
	IF_DEQUEUE(&en->en_if.if_snd, m);
	if (m == 0)
		return;

	/* Get a command block */
	mpdequeue_head(&en->en_free_que, (queue_entry_t *)&tmsg);
	if(tmsg == 0)
		goto clean_up;

	tmsg->en_bdl=(bd_t *)((int)&tmsg->bdl[1] & ~(sizeof(bd_t)-1));

	if (m->m_flags & M_PKTHDR)	/* oughta be */
		len = m->m_pkthdr.len;
	else {
		register struct mbuf *mtemp = m;
		for(len = 0; mtemp != NULL; mtemp = mtemp->m_next)
			len += mtemp->m_len;
	}
	/*
	 * 	The msg has to be copied so that it will fit in at most 2 
	 *	contiguous regions.  If the msg is less than 108 bytes it
	 *	will fit in one mbuf.  Otherwise it will be put into
	 *	click buffers.
	 */

	if(len > EN_T_MLEN) {		/* need click buffers */

		if((tmsg->en_mbuf = m_get(M_DONTWAIT, MT_DATA)) == 0)
			goto clean_up;

		EN_MCLADD(tmsg->en_mbuf, M_DONTWAIT, goto clean_up);

		tmsg->en_mbuf->m_len = MIN(len, MCLBYTES);
		phys = pmap_resident_extract(pmap_kernel(),
			mtod(tmsg->en_mbuf, int));
		BD_NONALIGN(&tmsg->en_bdl[0], phys, tmsg->en_mbuf->m_len);

		if(len > MCLBYTES) {
			
			if((tmsg->en_mbuf->m_next =
					m_get(M_DONTWAIT, MT_DATA)) == 0)
				goto clean_up;
			EN_MCLADD(tmsg->en_mbuf->m_next, M_DONTWAIT, goto clean_up);

			tmsg->en_mbuf->m_next->m_len = len - MCLBYTES;
			if (tmsg->en_mbuf->m_next->m_len > MCLBYTES)
				goto too_big;
			phys = pmap_resident_extract(pmap_kernel(),
				mtod(tmsg->en_mbuf->m_next, int));
			BD_NONALIGN(&tmsg->en_bdl[1], phys,
				tmsg->en_mbuf->m_next->m_len);
			BD_TAIL(&tmsg->en_bdl[2]);
		}
		else 
			BD_TAIL(&tmsg->en_bdl[1]);
	}
	else {				/* else can use an mbuf */
		if((tmsg->en_mbuf = m_get(M_DONTWAIT, MT_DATA)) == 0)
			goto	clean_up;

		tmsg->en_mbuf->m_len = len;
		tmsg->en_mbuf->m_data = (caddr_t)((long)(tmsg->en_mbuf->m_data + 7) & ~7);
		phys = pmap_resident_extract(pmap_kernel(),
			mtod(tmsg->en_mbuf, int));
		BD_NONALIGN(&tmsg->en_bdl[0], phys, tmsg->en_mbuf->m_len);
		BD_TAIL(&tmsg->en_bdl[1]);
	}

	if(en_m_copy(m, tmsg->en_mbuf) != 0) {
too_big:
		printf("Dropped oversize packet on unit %d, len = %d\n", 
			unit,len);
		goto clean_up;
	}

	tmsg->en_xmit_hdr.em_msg_hdr.crq_msg_code = CRQOP_EN_XMIT_FRAME;
	tmsg->en_xmit_hdr.em_msg_hdr.crq_msg_unitid=en->en_send_crq.crq_unitid;

	tmsg->en_xmit_bytes = len;

	send_cmd(tmsg, &en->en_send_crq);

	en->en_if.if_opackets++;
	m_freem(m);
	return;

clean_up:
	if(tmsg) {
		if(tmsg->en_mbuf) {
			m_freem(tmsg->en_mbuf);
			tmsg->en_mbuf = 0;
		}
		mpenqueue_tail(&en->en_free_que, tmsg);
	}

	if(m) m_freem(m);

	return;
}

/*
 * Process Ethernet small packet receive completion:
 *	Examine packet to determine type.  If can't determine length
 *	from type, then have to drop packet.  Otherwise decapsulate
 *	packet based on type and pass to type-specific higher-level
 *	input routine.
 */

en_srecv(ihp)
ihandler_t *ihp;
{
struct	en_softc	*en = (struct en_softc *)(ihp->ih_hparam[0].intparam);
struct	crq_en_rcv_msg	*rcp;
struct	emc_atn_msg	*attn, *rec_attn();
crq_t	*chan = &en->en_srbuf_crq;
	
	while ((attn = rec_attn(chan)) != NULL) {
		printf(eth_attn_msg, GETSLOT(chan->crq_unitid),
			attn->emc_atn_hdr.crq_msg_code, attn->emc_atn_status,
			attn->emc_atn_xtnd_stat);
		put_free(attn, chan);
	}

	while((rcp = (struct crq_en_rcv_msg *)rec_rsp(chan)) != NULL) {
		en->en_if.if_ipackets++;
		if (rcp->en_rcv_hdr.em_msg_hdr.crq_msg_status != STS_SUCCESS) {
			en->en_if.if_ierrors++;
			printf("EN srecv - EMC resp error, sts = %d, emc sts = %d\n",
			       rcp->en_rcv_hdr.em_msg_hdr.crq_msg_status,
			       rcp->en_rcv_hdr.em_status_code);
		}
        	rcp->en_mbuf->m_len = rcp->en_rcv_hdr.em_compltn_cnt;
		en_recv(en, rcp->en_mbuf, rcp->en_rcv_hdr.em_compltn_cnt);

		(void) ensrcmdpost(rcp, en, M_DONTWAIT);
	}
}

/*
 * Ethernet small receive command post routine
 * Assemble and initialize small receive command, then send to device.
 * Failure causes command to be put on sdead (small dead) queue for
 * later recovery.
 */
ensrcmdpost(rcp, en, canwait)
struct	crq_en_rcv_msg	*rcp;
struct	en_softc *en;
{
crq_t	*chan = &en->en_srbuf_crq;
caddr_t	phys, pmap_resident_extract();

#if	NETNCPUS > 1
	if((rcp->en_mbuf = m_get_special(canwait, MT_DATA)) == 0) {
#else
	if((rcp->en_mbuf = m_get(canwait, MT_DATA)) == 0) {
#endif
		printf("EN srcmdpost: ran out of mbufs\n");
		goto out;
	}
	rcp->en_mbuf->m_len = MHLEN;
	rcp->en_mbuf->m_flags |= M_PKTHDR;	/* will fill in pkthdr later */
	rcp->en_mbuf->m_pkthdr.len = 0x12345678;
	rcp->en_mbuf->m_pkthdr.rcvif = (struct ifnet *)0x87654321;

/* The only reason we have to do this is the damn EMC board which
 * "knows" that little mbufs have 112 bytes. What a botch. :-( */
#if	NETNCPUS > 1
	if (mcladd_special(rcp->en_mbuf, canwait) == 0) {
		printf("EN srcmdpost: ran out of clusters\n");
		goto out;
	}
#else
	EN_MCLADD(rcp->en_mbuf, canwait,
		{ printf("EN srcmdpost: ran out of clusters\n"); goto out; })
#endif
	rcp->en_mbuf->m_len = MCLBYTES;
/* ... */

	rcp->en_bdl=(bd_t *)((int)&rcp->bdl[1] & ~(sizeof(bd_t)-1));
	phys = pmap_resident_extract(pmap_kernel(),
		mtod(rcp->en_mbuf, int));
	BD_NONALIGN(&rcp->en_bdl[0], phys, rcp->en_mbuf->m_len);
	BD_TAIL(&rcp->en_bdl[1]);

	rcp->en_rcv_hdr.em_msg_hdr.crq_msg_code = CRQOP_EN_RCV_FRAME;
	rcp->en_rcv_hdr.em_msg_hdr.crq_msg_unitid = chan->crq_unitid;
	send_cmd(rcp, chan);
	return(0);
out:
	if (rcp->en_mbuf) {
		m_freem(rcp->en_mbuf);
		rcp->en_mbuf = 0;
	}
	mpenqueue_tail(&en->en_sdead_que, rcp);
	return(1);
}

/*
 * Process Ethernet large packet receive completion:
 *	Examine packet to determine type.  If can't determine length
 *	from type, then have to drop packet.  Otherwise decapsulate
 *	packet based on type and pass to type-specific higher-level
 *	input routine.
 */

en_lrecv(ihp)
ihandler_t *ihp;
{
struct	en_softc	*en = (struct en_softc *)(ihp->ih_hparam[0].intparam);
struct	crq_en_rcv_msg	*rcp;
struct	emc_atn_msg	*attn, *rec_attn();
crq_t	*chan = &en->en_lrbuf_crq;
short	size;
	
	while ((attn = rec_attn(chan)) != NULL) {
		printf(eth_attn_msg, GETSLOT(chan->crq_unitid),
			attn->emc_atn_hdr.crq_msg_code, attn->emc_atn_status,
			attn->emc_atn_xtnd_stat);
		put_free(attn, chan);
	}

	while ((rcp = (struct crq_en_rcv_msg *) rec_rsp (chan)) != NULL) {

		en->en_if.if_ipackets++;
		if (rcp->en_rcv_hdr.em_msg_hdr.crq_msg_status != STS_SUCCESS) {
			en->en_if.if_ierrors++;
			printf("EN lrecv - EMC resp error, sts = %d, emc sts = %d\n",
			       rcp->en_rcv_hdr.em_msg_hdr.crq_msg_status,
			       rcp->en_rcv_hdr.em_status_code);
		}
		size = rcp->en_rcv_hdr.em_compltn_cnt;
		if(size > MCLBYTES) {
			rcp->en_mbuf->m_len = MCLBYTES;
			rcp->en_mbuf->m_next->m_len = size - MCLBYTES;
			if (rcp->en_mbuf->m_next->m_len > MCLBYTES)
				/* now what? */ ;
		}
		else {
			if(rcp->en_mbuf->m_next) {
				(void) m_free(rcp->en_mbuf->m_next);
				rcp->en_mbuf->m_next = NULL;
			}
			rcp->en_mbuf->m_len = size;
		}
		en_recv(en, rcp->en_mbuf, size);

		(void) enlrcmdpost(rcp, en, M_DONTWAIT);
	}
}

/*
 * Ethernet large receive command post routine.
 * Assemble and initialize large receive command.  Then send to device.
 * Failure causes command to be put on ldead (large dead) queue for
 * later recovery.
 */
enlrcmdpost(rcp, en, canwait)
struct	crq_en_rcv_msg *rcp;
struct	en_softc *en;
{
crq_t	*chan = &en->en_lrbuf_crq;
caddr_t	phys, pmap_resident_extract();
int	out_of_what;

#define OUT_OF_MBUFS	1
#define OUT_OF_CLUSTERS 2

#if	NETNCPUS > 1
	if((rcp->en_mbuf = m_get_special(canwait, MT_DATA)) == 0) {
#else
	if((rcp->en_mbuf = m_get(canwait, MT_DATA)) == 0) {
#endif
		out_of_what = OUT_OF_MBUFS;
		goto out;
	}
#if	NETNCPUS > 1
	if (mcladd_special(rcp->en_mbuf, canwait) == 0) {
		out_of_what = OUT_OF_CLUSTERS;
		goto out;
	}
#else
	EN_MCLADD(rcp->en_mbuf, canwait,
		{ out_of_what = OUT_OF_CLUSTERS; goto out; })
#endif
	rcp->en_mbuf->m_len = MCLBYTES;
	rcp->en_mbuf->m_flags |= M_PKTHDR;
	rcp->en_mbuf->m_pkthdr.len = 0x12345678;
	rcp->en_mbuf->m_pkthdr.rcvif = (struct ifnet *)0x87654321;

	if(rcp->en_mbuf->m_len < ENBUF_SIZE) {
#if	NETNCPUS > 1
		if((rcp->en_mbuf->m_next = m_get_special(canwait, MT_DATA)) == 0) {
			out_of_what = OUT_OF_MBUFS;
			goto out;
		}

		if (mcladd_special(rcp->en_mbuf->m_next, canwait) == 0) {
			out_of_what = OUT_OF_CLUSTERS;
			goto out;
		}
#else
		if((rcp->en_mbuf->m_next = m_get(canwait, MT_DATA)) == 0) {
			out_of_what = OUT_OF_MBUFS;
			goto out;
		}
		EN_MCLADD(rcp->en_mbuf->m_next, canwait,
			{ out_of_what = OUT_OF_CLUSTERS ; goto out; })
#endif
		rcp->en_mbuf->m_next->m_len = MCLBYTES;
	}

	rcp->en_bdl=(bd_t *)((int)&rcp->bdl[1] & ~(sizeof(bd_t)-1));
	phys = pmap_resident_extract(pmap_kernel(),
		mtod(rcp->en_mbuf, int));
	BD_NONALIGN(&rcp->en_bdl[0], phys, rcp->en_mbuf->m_len);

	if(rcp->en_mbuf->m_len < ENBUF_SIZE) {
/* if rcp->en_mbuf->m_len + rcp->en_mbuf->m_next->m_len < ENBUF_SIZE ??? */
		phys = pmap_resident_extract(pmap_kernel(),
			mtod(rcp->en_mbuf->m_next, int));
		BD_NONALIGN(&rcp->en_bdl[1], phys, rcp->en_mbuf->m_next->m_len);
		BD_TAIL(&rcp->en_bdl[2]);
	} else
		BD_TAIL(&rcp->en_bdl[1]);

	rcp->en_rcv_hdr.em_msg_hdr.crq_msg_code = CRQOP_EN_RCV_FRAME;
	rcp->en_rcv_hdr.em_msg_hdr.crq_msg_unitid = chan->crq_unitid;
	send_cmd(rcp, chan);
	return(0);
out:
	if (out_of_what == OUT_OF_MBUFS)
		printf("EN lrcmdpost: ran out of mbufs\n");
	else
		printf("EN lrcmdpost: ran out of CLUSTERS\n");
		
	if(rcp->en_mbuf) {
		m_freem(rcp->en_mbuf);
		rcp->en_mbuf = 0;
	}
	mpenqueue_tail(&en->en_ldead_que, rcp);
	return(1);
}

/*
 * Ethernet common receive code.
 */
en_recv(en, m, len)
struct en_softc *en;
struct mbuf *m;
{
	struct	ether_header	*eh;
	u_short	ethertype;
	int	off;
	struct	ifqueue	*inq;
	struct	mbuf	*ethrtrail();
	int	s;


	   if (m==NULL)
		panic("EN recv: no frame");
	   if ((m->m_flags & M_PKTHDR) == 0 ||
		m->m_pkthdr.len != 0x12345678 ||
		m->m_pkthdr.rcvif != (struct ifnet *)0x87654321)
		panic("EN recv: not a header");
	   m->m_pkthdr.rcvif = &en->en_if;
	   m->m_pkthdr.len = len;

	   /* The first mbuf will always contain the Ethernet header of the
	    * received frame.  Get protocol type and frame length.
	    */
	   eh = mtod(m, struct ether_header *);
	   eh->ether_type = ntohs((u_short)eh->ether_type);

	   /*
	    * Deal with trailer protocol
	    */
	   if (eh->ether_type >= ETHERTYPE_TRAIL &&
	       eh->ether_type < ETHERTYPE_TRAIL+ETHERTYPE_NTRAILER) {
		   off = (eh->ether_type - ETHERTYPE_TRAIL) * 512;

		   if (off >= ETHERMTU) {
			m_freem(m);
			return;
		   }

		   /* Rearrange frame with Ethernet header first, Inet
		    * trailing header next, then data.
		    */
		   m = ethrtrail(m, len, off);
		   /* Couldn't rearrange frame, mbufs already deallocated,
		    * continue with next frame.
		    */
		   if (m == NULL)
			return;
		   /* Get pointer to Ethernet header again, may have moved
		    */
		   eh = mtod(m, struct ether_header *);
	    }

	/* Throw away hardware ether_header, pass up */

	m->m_data += sizeof(struct ether_header);
	m->m_len  -= sizeof(struct ether_header);
	m->m_pkthdr.len -= sizeof(struct ether_header);

	ether_input(&en->en_if, eh, m);
}

struct mbuf *
ethrtrail(m, len, off)
struct mbuf *m;
int len;
int off;
{
struct mbuf *header, *trailer, *n;
int moff;
u_short resid_len;
struct ether_header *eh;

eh = mtod(m, struct ether_header *);

 /*
  * Deal with trailer protocol: if type is PUP trailer
  * get true type from first 16-bit word past data.
  */
	trailer = m;
	moff = off + sizeof(struct ether_header);
	n = NULL;
	/* Find out which mbuf the trailer begins in.  On completion of
	 * this loop, trailer points to the mbuf the trailer starts in,
	 * moff contains the offset of the trailer in the mbuf data, and
	 * n points to the mbuf preceding the trailer mbuf.
	 */
	while(trailer && moff >= trailer->m_len) {
	    moff -= trailer->m_len;
	    n = trailer;
	    trailer = trailer->m_next;
	}
	if (trailer == NULL) {
		m_freem(m);
		return(NULL);
	}


/* Get the real protocol type and the length of the trailer from the first
 * two short integers of the trailer.  Verify that the trailer residual
 * length, the frame length, and the offset to the trailer are all consistent.
 * If they are not consistent, discard the frame.
 */
	
	eh->ether_type = ntohs(*(u_short *)(mtod(trailer, caddr_t) + moff));

	resid_len = ntohs(*(u_short *)(mtod(trailer, caddr_t) + moff + 2));
	if (off + resid_len > len) {
		m_freem(m);
		return(NULL);
	}
	len = off + resid_len;
	if (len == 0) {
		m_freem(m);
		return(NULL);
	}

 /*
  * Move the Ethernet header to its own mbuf and put it at the front.
  * Move the trailing header to another mbuf or mbufs, dropping
  * the type and length at the beginning, and place the mbuf(s)
  * just after the Ethernet header mbuf.
  */

 /* Copy the part of the trailer sharing an mbuf with data
  * and just move the rest of the mbufs containing the trailer.
  */

	if (moff != 0) {
		n = trailer;
		trailer = m_copy(n, moff, trailer->m_len - moff);
		if (trailer == NULL) {
			m_freem(m);
			return(NULL);
		}
		n->m_len -= trailer->m_len;
		trailer->m_data += 2 * sizeof (u_short);
		trailer->m_len  -= 2 * sizeof (u_short);
		trailer->m_next = n->m_next;
	}
	/* Break the previous mbuf's link to the trailer mbuf(s)
	 */
	if (n != NULL)
		n->m_next = NULL;
 /* Copy the Ethernet header to its own mbuf, if not already
  * in its own.  Link chain with header first, trailer second,
  * and remainder of data (m) last.
  */
	if (m->m_len == sizeof(struct ether_header)) {
		header = m;
		m = m->m_next;
	}
	else {
	   header = m_gethdr(M_DONTWAIT, MT_DATA);
	   if (header == NULL) {
		m_freem(m);
		m_freem(trailer);
		return(NULL);
	   }
	   bcopy(mtod(m, caddr_t), mtod(header, caddr_t),
			sizeof(struct ether_header));
	   header->m_len = sizeof(struct ether_header);
	   header->m_pkthdr.len = m->m_pkthdr.len - 2 * sizeof (u_short);
	   header->m_pkthdr.rcvif = m->m_pkthdr.rcvif;
	   m->m_flags &= ~M_PKTHDR;
	   m->m_data += sizeof(struct ether_header);
	   m->m_len  -= sizeof(struct ether_header);
	}
	header->m_next = trailer;

	/* Find the last mbuf of the trailer mbuf(s) and point it to
	 * the first mbuf of the frame data.
	 */
	n = trailer;
	while(n->m_next != NULL)
		n = n->m_next;
	n->m_next = m;

	/* Return new first mbuf in frame
	 */
	return(header);
}

/*
 * Process Ethernet send packet completion:
 *	Give back mbufs and requeue cmd buffers
 */

en_senddone(ihp)
ihandler_t *ihp;
{
struct	en_softc	*en = (struct en_softc *)(ihp->ih_hparam[0].intparam);
struct	crq_en_xmit_msg	*rcp;
struct	mbuf *m;
crq_t	*chan = &en->en_send_crq;
struct	emc_atn_msg    *attn, *rec_attn();

	while ((attn = rec_attn(chan)) != NULL) {
		printf(eth_attn_msg, GETSLOT(chan->crq_unitid),
			attn->emc_atn_hdr.crq_msg_code, attn->emc_atn_status,
			attn->emc_atn_xtnd_stat);
		put_free(attn, chan);
	}

	while((rcp = (struct crq_en_xmit_msg *) rec_rsp (chan)) != NULL) {
		if (rcp->en_xmit_hdr.em_msg_hdr.crq_msg_status != STS_SUCCESS)
			printf("EN senddone - EMC resp error, sts = %d, emc sts = %d\n",
			       rcp->en_xmit_hdr.em_msg_hdr.crq_msg_status,
			       rcp->en_xmit_hdr.em_status_code);

	/* do opcode specific processing */
		switch (rcp->en_xmit_hdr.em_msg_hdr.crq_msg_code) {
		case CRQOP_EN_CONTROL:
		        en_cntl_resp(en, (struct crq_en_cntl_msg *)rcp);
			break;

		case CRQOP_EN_READ_RST_STAT:
			en_stat_update(en);
			break;
			
		case CRQOP_EN_XMIT_FRAME:
			m_freem(rcp->en_mbuf);
			break;	
		default:
			printf("EN senddone: unexpected crqop code: %d\n",
				rcp->en_xmit_hdr.em_msg_hdr.crq_msg_code);
			break;
		}

		rcp->en_xmit_hdr.em_msg_hdr.crq_msg_status = STS_FREE;
		rcp->en_mbuf = 0;
		mpenqueue_tail(&en->en_free_que, rcp);
	}

	/* done processing send responses, now restart unit */
	enstart(en-en_softc);
}

en_cntl_resp(en, scp)
struct	en_softc *en;
struct	crq_en_cntl_msg *scp;
{
	
	switch (scp->en_cntl_subcode) {
	case ENSUB_SET_OFFLINE:
		en->en_dstate &= ~EN_ONLINE;
		printf("EN - Device offline, unit %x.\n", en-en_softc);
	        break;

	case ENSUB_RESET:
		en->en_dstate = 0;
		printf ("EN - Device reset, unit %x.\n", en-en_softc);
	        break;

	case ENSUB_SET_ONLINE:
		en->en_dstate |= EN_ONLINE;
		printf ("EN - Device online, unit %x.\n", en-en_softc);
	        break;

	default:
	        printf("EN: en_command resp error, unit %x\n", en-en_softc);
	        break;
	}
}

/*
 * Ethernet statistics update routine.
 * If first update (i.e. interface state is not SINIT) get hardware address,
 * ignore statistics, and set interface state to SINIT.
 * Else update error counts.
 */
en_stat_update(en)
struct en_softc *en;
{
struct eth_stats *statp = &en->en_stats;

	if (en->en_istate != EN_SINIT) {
		bcopy((caddr_t)statp->st_ethr_addr,
			(caddr_t)en->en_addr, sizeof(en->en_addr));
		en->en_istate = EN_SINIT;
#ifdef	MMAX_FASTSTATS
		faststats(en);
#endif
	}
	else {
		en->en_if.if_oerrors += statp->st_xmit_exccoll;
		en->en_if.if_oerrors += statp->st_xmit_undrnerr;
		en->en_if.if_ierrors += statp->st_rcv_crcerr;
		en->en_if.if_ierrors += statp->st_rcv_alignerr;
		en->en_if.if_ierrors += statp->st_rcv_rscerr;
		en->en_if.if_ierrors += statp->st_rcv_ovrnerr;
		en->en_if.if_ierrors += statp->st_rcv_hostbufs;
	/* can't actually count collisions on this interface, hence
	if_collisions is a count of number of packets that collided. */
		en->en_if.if_collisions += statp->st_xmit_onecoll;
		en->en_if.if_collisions += statp->st_xmit_mulcoll;
		en->en_if.if_collisions += statp->st_xmit_exccoll;
	}
}

/*
 * Ethernet get statistics routine - send read & reset stats command to dev.
 */
engetstats(en)
struct en_softc *en;
{
struct	crq_en_rdstats_msg *s_msg;
	
	mpdequeue_head(&en->en_free_que, (queue_entry_t *)&s_msg);
	
	if(s_msg == 0) {
		printf("EN getstats: could not get msg buffer for rdstats\n");
		return(1);
	}
	
	s_msg->en_rdstats_hdr.em_msg_hdr.crq_msg_code=CRQOP_EN_READ_RST_STAT;
	s_msg->en_rdstats_hdr.em_msg_hdr.crq_msg_unitid=en->en_send_crq.crq_unitid;
	s_msg->en_rdstats = &en->en_stats;
	
	send_cmd(s_msg, &en->en_send_crq);
	
	return(0);
}

/*
 * Ethernet output routine.
 * Encapsulate a packet of type family for the local net.
 * Use trailer local net encapsulation if enough data in first
 * packet leaves a multiple of 512 bytes of data in remainder.
 */
enoutput(ifp)
	register struct ifnet *ifp;
{
	int s;
	s = splimp();
	enstart(ifp->if_unit);
	splx(s);
	return (0);
}

/*
 * Watchdog routine - place stats request to device and attempt to
 * 	recover dead receive commands.
 */
enwatch(unit)
	int unit;
{
	register struct en_softc *en = &en_softc[unit];
	struct crq_en_rcv_msg *r_msg;
	unsigned s;

	s = splimp();

#ifndef	MMAX_FASTSTATS
	/* send read and reset stats command to device. */
	(void) engetstats(en);
#endif

	/* Try to recover dead receive commands.  Post routine returns
		1 on failure -- don't retry if this happens. */
	mpdequeue_head(&en->en_ldead_que, (queue_entry_t *)&r_msg);
	while (r_msg) {
		if (enlrcmdpost(r_msg, en, M_DONTWAIT)) {
			mpenqueue_tail(&en->en_ldead_que, r_msg);
			break;
		}
		mpdequeue_head(&en->en_ldead_que, (queue_entry_t *)&r_msg);
	}

	mpdequeue_head(&en->en_sdead_que, (queue_entry_t *)&r_msg);
	while (r_msg) {
		if(ensrcmdpost(r_msg, en, M_DONTWAIT)) {
			mpenqueue_tail(&en->en_sdead_que, r_msg);
			break;
		}
		mpdequeue_head(&en->en_sdead_que, (queue_entry_t *)&r_msg);
	}

	splx(s);
}

/*
 * Process an ioctl request.
 */
enioctl(ifp, cmd, data)
	register struct ifnet *ifp;
	int cmd;
	caddr_t data;
{
	register struct ifaddr *ifa = (struct ifaddr *)data;
	int s = splimp(), error = 0;

	switch (cmd) {

	case SIOCSIFADDR:
       		if(en_softc[ifp->if_unit].en_istate != EN_SINIT) {
			printf("EN ioctl: SIOCSIFADDR - device not initialized\n");
			error = EIO;
			break;
		}
	        ifp->if_flags |= IFF_UP;
                eninit(ifp->if_unit);

                switch (ifa->ifa_addr->sa_family) {
		case AF_INET:
			((struct arpcom *)ifp)->ac_ipaddr =
				IA_SIN(ifa)->sin_addr;
			arpwhohas((struct arpcom *)ifp, &IA_SIN(ifa)->sin_addr);
			break;
		case AF_NS:
			IA_SNS(ifa)->sns_addr.x_host =
				* (union ns_host *) 
				     (en_softc[ifp->if_unit].en_addr);
			break;
		}
		break;

	default:
		error = EINVAL;
	}
	splx(s);
	return (error);
}

/*
 * Copy an mbuf chain to another.
 */
en_m_copy(from, to)
register struct mbuf *from, *to;
{
int	len, to_len, to_off;
int	from_len, from_off;

	to_len = to->m_len;
	to_off = 0;

	from_len = from->m_len;
	from_off = 0;

#if	MMAX_DEBUG
	if(to == 0 || from == 0)
		printf("en_m_copy: To = 0x%x, From = 0x%x\n", to, from);
#endif

	while (from != 0) {
		if (to == 0) {
#if	MMAX_DEBUG
			printf("en_m_copy: dropped oversize packet.\n");
			printf("From has %d remaining\n", from_len);
#endif
			return(1);	/* error */
		}

		len = MIN(from_len, to_len);

		bcopy(mtod(from, caddr_t) + from_off,
				mtod(to, caddr_t) + to_off, len);

		from_len -= len;
		from_off += len;
		while(from && (from_len == 0)) {
			from = from->m_next;
			if ( from != 0) {
				from_len = from->m_len;
				from_off = 0;
			}
		}
			

		to_len -= len;
		to_off += len;
		while (to && (to_len == 0)) {
			to = to->m_next;
			if (to != 0) {
				to_len = to->m_len;
				to_off = 0;
			}
		}
	}
	return(0);
}

/* enreset - send reset command to unit, then init and restart.  Reset
	command actually allocates memory in hardware controller, therefore
	ignore 3rd and subsequent comands to avoid trouble in hardware. */

static int	num_enreset = 0;

enreset(unit)
int unit;
{
	struct en_softc *en = &en_softc[unit];
	struct ifnet *ifp = &en->en_if;
	struct	crq_en_cntl_msg	*c_msg;

	if(++num_enreset >= 3) {
		printf("enreset - Too many resets; device not reset.\n");
		return;
	}

	mpdequeue_head(&en->en_free_que, (queue_entry_t *)&c_msg);

	if(c_msg == 0) {
		printf("EN init: could not get cmd buffer\n");
		en->en_if.if_flags &= ~IFF_UP;
		return;
	}

	c_msg->en_cntl_hdr.em_msg_hdr.crq_msg_code = CRQOP_EN_CONTROL;
	c_msg->en_cntl_hdr.em_msg_hdr.crq_msg_unitid=en->en_send_crq.crq_unitid;
	c_msg->en_cntl_subcode = ENSUB_RESET;

	send_cmd(c_msg, &en->en_send_crq);

	eninit(unit);				/* start transmits */
}

static struct mbuf *
m_get_special(canwait, type)
int	canwait;
int	type;
{
	int		try, loop, x;
	struct mbuf	*m = (struct mbuf *) 0;

#define	MAXTRY	20

	for (try = 0; try < MAXTRY; ++try) {
		if ((m = m_get(canwait, type)) != (struct mbuf *) 0)
			break;
		/* processor/cache dependent */
		for (loop = 0; loop < (try+1)*(try+2)*(try+3); ++loop)
		     x++;
#ifdef	lint
		try = x;
#endif
	}
	return (m);
}

static int
mcladd_special(m, w)
struct mbuf *m;
{
	register int	try, loop, x;

	for (try = 0; try < MAXTRY; ++try) {
		m->m_len = 0;
		MCLGET(m, w);
		if ((m->m_flags & M_EXT) != 0)
			return (1);
		/* processor/cache dependent */
		for (loop = 0; loop < (try+1)*(try+2)*(try+3); ++loop)
		     x++;
#ifdef	lint
		try = x;
#endif
	}
	return (0);
}
