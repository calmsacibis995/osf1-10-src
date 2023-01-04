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
static char	*sccsid = "@(#)$RCSfile: mach_net_vmtp.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:25:58 $";
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
 * File:	mach_net_vmtp.c
 * Purpose:
 *	Message interface to VMTP. To be used primarily by the network server,
 *	in cooperation with the kernel IPC code.
 */

#include <mach_vmtp.h>
#include <mach_np.h>

#include <sys/types.h>
#include <sys/param.h>
#include <mach/kern_return.h>
#include <mach/port.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/errno.h>
#include <sys/kernel.h>
#include <sys/ioctl.h>

#include <net/if.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_pcb.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>

#include <netinet/vmtp_so.h>
#include <netinet/vmtp.h>
#include <netinet/vmtp_ip.h>
#include <netinet/vmtp_var.h>
#include <netinet/vmtp_send.h>
#include <netinet/vmtp_group.h>
#include <netinet/esp_cache.h>
#include <netinet/esp.h>

#include <kern/parallel.h>


/*
 * Definition copied from mach_ipc_vmtp.c
 */
#define vm_netport	vm_shortdata[2]

/*
 * Data structures for the queue of free eid/entities.
 *
 * eid's are stored in a linked list of 'slots'. We maintain
 * a queue of slots containing each a free eid, and a queue of
 * empty slots. The slots are allocated in mbuf's.
 */
struct eidq {
	struct eidq	*next;
	struct vmtpeid	eid;
};
static struct eidq	*vmtp_free_eid = 0;
static struct eidq	*vmtp_eid_slots = 0;


kern_return_t
mach_vmtp_init(ServPort, vmtp_port)
port_t		ServPort;
port_t		*vmtp_port;
{
	/*
	 * Initialize vmtp_hostid if necessary.
	 * Use the address in the first network interface for
	 * lack of anything better... XXX
	 */
	if (!vmtp_hostid) {
		vmtp_hostid = IA_SIN(in_ifaddr)->sin_addr.s_addr;
	}
}


kern_return_t
mach_vmtp_alloceid(ServPort, eid)
port_t			ServPort;
register struct vmtpeid	*eid;
{
	extern struct vmtpeid vmtp_alloceid();
	register struct eidq	*slot;
	register struct vmtpcsr	*csr;
	int			s;

	unix_master();

	s = splimp();
	if (slot = vmtp_free_eid) {
		vmtp_free_eid = slot->next;
		slot->next = vmtp_eid_slots;
		vmtp_eid_slots = slot;
		splx(s);
		*eid = slot->eid;
	} else {
		splx(s);

		do {
			*eid = vmtp_alloceid();
		} while ((vmtp_maptoserver(*eid) != NULL)
			|| (vmtp_maptocsr(*eid, vlocalcsrmap) != NULL));
		csr = vmtp_newcsr(*eid, vlocalcsrmap);
		if (csr == NULL) {
			unix_release();
			return KERN_NO_SPACE;
		}
		csr->vc_state = VC_IDLE; 
		csr->vc_hdltimeo = vmtp_hdllcsrtimeo;
		csr->vc_processid.pi_local = 0;
		csr->vc_processid.pi_host.s_addr = vmtp_hostid;
		csr->vc_principalid.pc_local = 0;
		csr->vc_principalid.pc_host.s_addr = vmtp_hostid;
		csr->vc_effprincipalid = csr->vc_principalid;
		csr->vc_timelimit = 0xffffffff;		/* forever */
		csr->vc_so = 0;
	}
	unix_release();
	return KERN_SUCCESS;
}


kern_return_t
mach_vmtp_freeeid(ServPort, eid)
port_t		ServPort;
struct vmtpeid	eid;
{
	register struct eidq	*slot, *previous_slot, *last_slot;
	struct mbuf		*m;
	int			i;
	int			s;

	unix_master();

	s = splimp();
	if (slot = vmtp_eid_slots) {
		vmtp_eid_slots = slot->next;
		slot->eid = eid;
		slot->next = vmtp_free_eid;
		vmtp_free_eid = slot;
		splx(s);
	} else {
		splx(s);

		MGET(m, M_WAIT, MT_DATA);
		if (m == NULL) {
			unix_release();
			return KERN_NO_SPACE;
		}
		m->m_len = MLEN;
		slot = mtod(m, struct eidq *);
		last_slot = slot;
		previous_slot = NULL;
		for (i = 0; i < (MLEN / sizeof(struct eidq)); i++) {
			slot->next = previous_slot;
			previous_slot = slot;
			slot++;
		}
		slot--;		/* compensate for the end-of-loop increment */
		slot->eid = eid;
		s = splimp();
/*		m->m_next = dtom(vmtp_free_eid); */
		last_slot->next = vmtp_eid_slots;
		vmtp_eid_slots = slot->next;
		slot->next = vmtp_free_eid;
		vmtp_free_eid = slot;
		splx(s);
	}
	unix_release();
	return KERN_SUCCESS;
}


kern_return_t
mach_vmtp_initserver(ServPort, eid, server_port)
port_t		ServPort;
struct vmtpeid	eid;
port_t		server_port;
{
	register struct vmtpcsr	*csr;
	register struct vmtpser	*ser;
	register int		s;

	unix_master();

	if (vmtp_maptoserver(eid) != NULL) {
		unix_release();
		return KERN_FAILURE;
	}

	csr = vmtp_maptocsr(eid, vlocalcsrmap);
	if (!csr) {
		csr = vmtp_newcsr(eid, vlocalcsrmap);
		if (csr == NULL) {
			unix_release();
			return KERN_NO_SPACE;
		}
		csr->vc_state = VC_IDLE; 
		csr->vc_hdltimeo = vmtp_hdllcsrtimeo;
		csr->vc_processid.pi_local = 0;
		csr->vc_processid.pi_host.s_addr = vmtp_hostid;
		csr->vc_principalid.pc_local = 0;
		csr->vc_principalid.pc_host.s_addr = vmtp_hostid;
		csr->vc_effprincipalid = csr->vc_principalid;
		csr->vc_timelimit = 0xffffffff;		/* forever */
		csr->vc_so = 0;
	}

	s = splimp();
	ser = vmtp_newserver(eid);
	if (ser == NULL) {
		splx(s);
		unix_release();
		return KERN_FAILURE;
	}
	ser->vs_flags = VS_MESGPORT;

	splx(s);

	/*
	 * XXX Store the server_port in the appropriate place. XXX
	 */
	unix_release();
	return KERN_SUCCESS;
}


kern_return_t
mach_vmtp_ipcinit(ServPort, eid)
port_t		ServPort;
struct vmtpeid	eid;
{
	/* XXX Do something XXX */
}


kern_return_t
mach_vmtp_invoke(ServPort, eid, func, mcb, segment, segmentCnt)
port_t			ServPort;
struct vmtpeid		eid;
int			func;
register struct vmtpmcb	*mcb;
char			*segment;
u_long			*segmentCnt;
{
	register struct vmtpcsr *csr;
	register struct vmtpsce *sc;	/* server cache entry */
	u_long			maxlen = *segmentCnt;
	int			writeaccess;
	struct vmtpeid dest;	/* destination (server or coresident) */
	int			error;
	u_long			control = 0;
	int			s;

	unix_master();

	*segmentCnt = 0;
	csr = vmtp_maptocsr(eid, vlocalcsrmap);
	if (csr == NULL) {
		mcb->vm_code = VMTP_NONEXIST;
		unix_release();
		return KERN_INVALID_ARGUMENT;
	}
	csr->vc_encryptqual = ENCRYPT_NONE;
	if (func & INVOKE_REQ) {
		csr->vc_transid++;
		csr->vc_rexmtcnt = 0;
		csr->vc_retrycnt = 0;

		if (vmtp_segmentdata(mcb->vm_code)) {
			if (mcb->vm_segsize < 0 || mcb->vm_segsize > VMTP_MAXSEGSIZE) {
				unix_release();
				return KERN_INVALID_ARGUMENT;
			}
		
			csr->vc_segptr =  segment;
			csr->vc_flags = ((func & INVOKE_RESP) ? VCF_NONE : VCF_MBUF);
			csr->vc_txmsk = vmtp_segsizetomsk(mcb->vm_segsize);
		} else {
			csr->vc_segptr =  NULL;
			csr->vc_flags = VCF_NONE;	
			csr->vc_txmsk = 0;
	  	}
	
		mcb->vm_code |= VU_REQ;

		csr->vc_server = mcb->vm_eid;
		csr->vc_origserver = mcb->vm_eid;
		csr->vc_ucb = mcb->vm_ucb;
		dest = csr->vc_server;
		s = splimp();
		if ((sc = esp_findentry(dest)) != NULL) {
			csr->vc_hostaddr = sc->ve_hostaddr;
			csr->vc_ifp = sc->ve_ifp;
			csr->vc_inpktgap = sc->ve_gap;
			csr->vc_mtu = sc->ve_mtu;
			csr->vc_roundtrip = sc->ve_roundtrip;
		} else {
			csr->vc_hostaddr.va_type = VAT_INET;
			csr->vc_hostaddr.va_inet = vmtp_eidtoinaddr(dest);
			csr->vc_inpktgap = VMTP_INPKTGAP;
			csr->vc_mtu = VMTP_MAXBLKPACKET;
			csr->vc_roundtrip = VTC_ROUNDTRIP * 20;
		}
		splx(s);

		csr->vc_vihdr->vi_addr = csr->vc_hostaddr;
		csr->vc_vihdr->vi_ifp = csr->vc_ifp;
		setvi_fnctcode(csr->vc_vihdr, VMTP_REQ);
		setvi_priority(csr->vc_vihdr, VPR_NORMAL);
		setvi_inpktgap(csr->vc_vihdr, VMTP_INPKTGAP);
		setvi_control(csr->vc_vihdr, control);

		csr->vc_netport = mcb->vm_netport;

		vmtp_sendpg(csr, VMTP_TRANSMIT, 0);

		csr->vc_state = VC_AWAITRESP;
		csr->vc_action = VCA_NOOP;

		vmtp_starttimer(csr, vtc_awaitresp(csr));
	}
	if (func & INVOKE_RESP) {
		writeaccess = vmtp_writeaccess(mcb->vm_code);
		while (csr->vc_action == VCA_NOOP)
			sleep(csr,PZERO+1);
		while (csr->vc_action == VCA_RETRANS) {
	        	vmtp_sendpg(csr, VMTP_RETRANSMIT, 0);
			csr->vc_action = VCA_NOOP;
			vmtp_starttimer(csr, csr->vc_tleft);
			while (csr->vc_action == VCA_NOOP)
				sleep(csr,PZERO+1);
		}
		
		mcb->vm_eid = csr->vc_server;
		mcb->vm_ucb = csr->vc_ucb;

		mcb->vm_netport = csr->vc_netport;

		*segmentCnt = 0;
		if (vmtp_segmentdata(mcb->vm_code) && writeaccess) {
			if (csr->vc_dataseg != NULL) {
				*segmentCnt = MIN(maxlen, mcb->vm_segsize);
				error = putsegment(csr->vc_dataseg, segment, 
					segmentCnt, 0);
				if (error) {
					/* XXX Should free message XXX */
					unix_release();
					return KERN_FAILURE;
				}
			}
		} else if ((csr->vc_dataseg != NULL) &&
			   (!(func & INVOKE_REQ))) {
			m_freem(csr->vc_dataseg);
		}
		csr->vc_dataseg = NULL;
	}
	unix_release();
	return KERN_SUCCESS;
}


kern_return_t
mach_vmtp_reply(ServPort, eid, mcb, segment, segmentCnt)
port_t				ServPort;
struct vmtpeid			eid;
register struct vmtpmcb 	*mcb;
char				*segment;
u_long				segmentCnt;
{
	register struct vmtpcsr	*csr;
	struct vmtpser		*ser;
/* 	u_long			control; */

	unix_master();

	ser = vmtp_maptoserver(eid);
	if (ser == NULL) {
		unix_release();
		return KERN_INVALID_ARGUMENT;
	}
	csr = vmtp_dequeuereq(ser, mcb->vm_eid);
	if (csr == NULL) {
		unix_release();
		return KERN_INVALID_ARGUMENT;
	}

/*	control = getvi_control(csr->vc_vihdr); */

	mcb->vm_code &= ~VU_REQ;

	if (vcf_nilfwdrq(csr)) {
		mcb->vm_code &= ~VU_DGM;
	}

	csr->vc_server = eid;		/* needed for multicast requests */
	csr->vc_ucb = mcb->vm_ucb;

	if (vmtp_respdiscarded(csr->vc_code)) {
		goto noreply;
	}

	if (vmtp_segmentdata(mcb->vm_code)) {
		if (mcb->vm_segsize < 0 || mcb->vm_segsize > VMTP_MAXSEGSIZE) {
			unix_release();
			return KERN_INVALID_ARGUMENT;
		}
		csr->vc_segptr = segment;
		csr->vc_txmsk = vmtp_segsizetomsk(mcb->vm_segsize);
		csr->vc_flags |= VCF_MBUF;
	} else {
		csr->vc_dataseg =  NULL;
		csr->vc_flags &= ~VCF_MBUF;
		csr->vc_txmsk = 0;
  	}

	csr->vc_rexmtcnt = 0;
	csr->vc_retrycnt = 0;

	csr->vc_vihdr->vi_addr = csr->vc_hostaddr;
	csr->vc_vihdr->vi_ifp = csr->vc_ifp;
	setvi_fnctcode(csr->vc_vihdr, VMTP_RESP);
	setvi_priority(csr->vc_vihdr, VPR_NORMAL);
	setvi_pgcount(csr->vc_vihdr, 0);
	setvi_control(csr->vc_vihdr, 0);

	vmtp_sendpg(csr, VMTP_TRANSMIT, 0);

noreply:
	csr->vc_state = VC_RESPONDED;

	vmtp_starttimer(csr, vmtp_acknowledgepg(getvi_control(csr->vc_vihdr)) 
				? vts_retransmit(csr) : vts_discardcsr(csr));
	unix_release();
	return KERN_SUCCESS;
}


kern_return_t
mach_vmtp_getrequest(ServPort, eid, mcb, segment, segmentCnt)
port_t			ServPort;
struct vmtpeid		eid;
register struct vmtpmcb	*mcb;
char			*segment;
u_long			*segmentCnt;
{
	register struct vmtpser	*ser;
	register struct vmtpcsr *csr;
	u_long			maxlen = *segmentCnt;
	int			error;
	int			s;

	unix_master();

	ser = vmtp_maptoserver(eid);

	if (ser == NULL) {
		unix_release();
		return KERN_INVALID_ARGUMENT;
	}

	s = splimp();
	if (!(vmtp_requestqueued(ser))) {
		splx(s);
		unix_release();
		return KERN_FAILURE;
	}

	csr = vmtp_retrievereq(ser);
	splx(s);

	if (csr == NULL) {
		panic("mach_vmtp_getrequest(): ser inconsistent");
	}

	mcb->vm_eid = csr->vc_client;
	mcb->vm_ucb = csr->vc_ucb;

	*segmentCnt = 0;
	if (vmtp_segmentdata(mcb->vm_code)) {
		if (csr->vc_dataseg != NULL) {
			*segmentCnt = MIN(maxlen, mcb->vm_segsize);
			error = putsegment(csr->vc_dataseg, segment,
				segmentCnt,0);
			if (error) {
				/* XXX Should free message XXX */
				unix_release();
				return KERN_FAILURE;
			}
		}
	} else if (csr->vc_dataseg != NULL) {
		printf("vc_dataseg = %x\n",( int) csr->vc_dataseg);
		/* m_freem(csr->vc_dataseg); */
	}
	csr->vc_dataseg = NULL;

	unix_release();
	return KERN_SUCCESS;
}
