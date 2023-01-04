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
static char *rcsid = "@(#)$RCSfile: pq.c,v $ $Revision: 1.1.7.3 $ (DEC) $Date: 1993/11/17 23:13:48 $";
#endif

#ifndef lint
static char *sccsid = "@(#)pq.c	5.1      (ULTRIX)  6/19/91";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
#define _PQ_C_
/************************************************************************
 *
 * 00-Dec-1991	Lloyd Wheeler
 *		Tweaked includes for OSF kernel build environment.
 *		Modified vm-related calls to work in OSF environment and
 *		otherwise 'OSF-ized'.
 *
 * 13-Aug-90	Sam Hsu
 *		Created, based on gq.c and pa.c
 *
 ************************************************************************/

#include <machine/pmap.h>		/* get vm stuff, inc. svatophys() */
#include <machine/hal/kn15aa.h>	        /* get DENSE macro */
#include <data/ws_data.c>
#include <data/bt_data.c>
#include <data/px_data.c>
#include <data/pq_data.c>
#include <kern/thread.h>

#if 0
#include "PXdbg.h"
#endif /* 0 */

#ifndef PDEVCMD_DMA
#define	PDEVCMD_DMA		2
#endif

#if 0
#ifndef	PMAP_COPROC_INVALIDATE_STLB
#define PMAP_COPROC_INVALIDATE_STLB  0
#define PMAP_COPROC_EXIT             1
#endif
#endif

#define	DO_NEW_STYLE_INVALIDATE

int PXG_DBG_FLAG = 0;

extern vm_offset_t px_map_pkt;
extern vm_offset_t px_map_inf;
extern vm_offset_t px_soft_tlb;
extern vm_offset_t px_mem_bot;
extern vm_offset_t px_mem_top;

extern caddr_t ws_map_region();

void pq_intr_noop(), pq_intr_pagein(), pq_intr_xlate(), pq_intr_vblank();
int  pq_invalidate_gcp_tlb(), pq_probe_ram();

#ifdef __alpha
extern pt_entry_t *pmap_pte();
#endif /* __alpha */

static void (*pq_intr_vec[])() = {
    pq_intr_noop,			/* 0 */
    pq_intr_pagein,			/* 1 */
    pq_intr_xlate,			/* 2 */
    pq_intr_vblank,			/* 3 */
    pq_intr_noop,			/* 4 */
    pq_intr_noop,			/* 5 */
    pq_intr_noop,			/* 6 */
    pq_intr_noop,			/* 7 */
    pq_intr_noop,			/* 8 */
    pq_intr_noop,			/* 9 */
    pq_intr_noop,			/* 10 */
    pq_intr_noop,			/* 11 */
    pq_intr_noop,			/* 12 */
    pq_intr_noop,			/* 13 */
    pq_intr_noop,			/* 14 */
    pq_intr_noop,			/* 15 */
};

/*
 * These are really for internal use, but declared global so nkvar can
 * access and change them.  Assumed to be ZFOD;
 */
#include <sys/proc.h>
#include <sys/user.h>

struct proc *_pq_serverp;

caddr_t pxod_uaddr;                     /* User addr of the option */

u_int _pq_intr_send[16];
u_int _pq_intr_recv[16];

pq_ptpt *_pq_ptpt = (pq_ptpt *) NULL;
int _pq_ptpt_size = 0;

u_int _pq_intr_total;
u_int _pq_debug;			/* flags controlling exceptional */
					/* behavior */
#define		PQ_DIEonTIMEOUT	0x1


/*
 * wait for stic to be ready to accept next packet.  note that we never
 * wait forever.  we'll time out and go ahead and see if the stic will
 * accept a packet anyway.  if not, _then_ we consider complaining...
 */
#define _POLL_STIC_PINT(S,Q)					\
{   register u_int k;						\
    for (k = 0; k < STAMP_RETRIES; k++) {			\
	if ((S)->ipdvint & STIC_INT_P)				\
	    break;						\
	DELAY(20);					\
    }								\
    if (k == STAMP_RETRIES) (Q)->stic_timeout++;		\
}


/******************************************************************
 **                                                              **
 ** Routine to attach to the graphic device.                     **
 **                                                              **
 ******************************************************************/
pq_attach(ctlr, pxp)
    struct controller *ctlr;
    px_info *pxp;
{
    /* removed call to pq_setup() here; already done in px_attach() */
    /* which was just called from pxattach() before the call to here */
    return (1);
}


pq_bootmsg(ctlr, pxp)
    struct controller *ctlr;
    px_info *pxp;
{
    int fb2, fb3;

    fb2 = px_probe_fb(pxp, 2, -1);
    if (fb2 < 0)
	fb2 = 0;
    fb3 = px_probe_fb(pxp, 3, -1);
    if (fb3 < 0)
	fb3 = 0;

    printf("pq%d (%dx%d %d+%d+%d+%d %dKB)\n", ctlr->ctlr_num,
	   pxp->stamp_width, pxp->stamp_height,
	   pxp->depth[0].depth, pxp->depth[0].depth,
	   fb2, fb3, pq_probe_ram(pxp)/1024);
}


void pq_enable_interrupt(closure)
    struct bt459info *closure;		/* XXX */
{
    register px_info *pxp = &px_softc[closure->unit];
    register stic_regs *stic = PQ_STIC(pxp);

    /* intr's are normally on; allow vblank intr's; N10 will turn it off */
    stic->ipdvint = (STIC_INT_V_WE | STIC_INT_V_EN);
    wbflush();
}

void pq_interrupt(ctlr, pxp)
    struct controller *ctlr;
    px_info *pxp;
{
    register u_int intr_cause = *(volatile int *) &PQ_RAM(pxp)->intr_host;
    register u_int intr_proc = intr_cause & HST_INTR_WHAT;

    _pq_intr_total++;

    (_pq_intr_recv[intr_proc])++;

    if (_pq_serverp) {
	(*pq_intr_vec[intr_proc])(pxp, intr_cause);
    }
    else
	pq_intr_noop(pxp, intr_cause);
}


/*
 * Set up the 3da.
 */
pq_setup(pxp, address, unit, flags)
    px_info *pxp;
    caddr_t address;
    int unit;
    int flags;
{
    struct bt459info *bti = (struct bt459info *)pxp->cf.cc;

    pq_info *pqp = &pq_softc[unit];

    /*
     * Initialize the device closure.
     */
#ifndef __alpha
    address = (caddr_t) PHYS_TO_K1(address);
#endif
    pxp->pxo = address;			/* Board. */
#ifdef __alpha
    pxp->pxod = (caddr_t) PHYS_TO_KSEG(DENSE(KSEG_TO_PHYS((vm_offset_t)address)));
#endif
    pxp->stamp += (long) address;		/* Stamp registers. */
    pxp->stic += (long) address;		/* STIC registers. */
#ifdef ultrix
    pxp->ringbuffer = (int *) svtophy(pxp->memory);
#else /* ultrix */
    /*
     * Allocation of properly aligned memory for the OSF version occurs in
     * mips_init.c right now.  The physical address is then converted into
     * a KSEG0 address in px.c:px_cons_init().
     *
     * HOWEVER, if px_cons_init() is not invoked (eg, if the console
     * environment variable 'console' is set to 's', so that an attached
     * terminal is used as the console instead), we must perform these
     * initializations here.  We've duplicated the code (yuk!) rather than
     * make a new routine, because there is so little to do.  Check to see 
     * if one of the variables which ought to have been initialized *is* 
     * initialized, and if not, initialize it.
     */
    if (pxp->ringbuffer == 0) {
	pxp->ringbuffer = (int *) PHYS_TO_K0(px_map_pkt);
	pxp->px = (pxInfo *) PHYS_TO_K0(px_map_inf);
    }
#endif /* ultrix */
    bzero((caddr_t)pqp, sizeof(pq_info));
    pxp->dev_closure = (caddr_t) pqp;

    *PQ_RESET(pxp) = 0;			/* halt the N10 */
    wbflush();
    *PQ_INTRH(pxp) = 0;			/* clear intr word */
    wbflush();

    /* now install our vblank interrupt enable routine */
    bti->enable_interrupt = pq_enable_interrupt;

    /* NOTE: px_clear_screen() moved from here to px_cons_init() */

    return 1;
}


int *pq_getPacket(pxp)
    px_info *pxp;		/* closure */
{
    register int *buf;
    register pq_info *pqp = (pq_info *) pxp->dev_closure;
    register int bufselect = pqp->bufselect;

    /*
     * don't collide with N10 over SRAM when xcons not enabled!
     * should not be the common case.
     */
    if (pqp->n10K) {			/* ask N10 which packet buffer */
	bufselect = pq_intr_coproc(pxp, PQ_INTR_PAUS);
	if (bufselect < 0) {		/* we may use. */
	    PX_DEBUG(PX_TERSE,
		     printf("pq_getPacket: bad reply from i860\n");
		     );
	    /*
	     * if we don't like the answer, then kill the respondent
	     * and use what we were going to use anyway...
	     */
	    *PQ_RESET(pxp) = pqp->n10K = 0;
	    wbflush();
	    bufselect = pqp->bufselect;
	}
	else {
	    stic_regs *stic = PQ_STIC(pxp);
	    PX_DEBUG(PX_GAB,
		     printf("pq_getPacket: i860->%d\n", pqp->bufselect);
		     );
	    _POLL_STIC_PINT(stic,pqp);	/* then wait for stic to be idle */
	}				/* since this buffer may be currently */
    }					/* executing */
    buf = PQ_REQBUF(pxp, bufselect);
    pqp->bufselect = bufselect ^ 0x1;

    return (buf);
}


void pq_sendPacket(pxp, buf)
    px_info *pxp;			/* closure */
    char *buf;				/* virtual addr */
{
    volatile int *poll;
    register int addr;
    register stic_regs *stic = PQ_STIC(pxp);
    register pq_info *pqp = (pq_info *) pxp->dev_closure;

    /* ... to sram phys addr ... */
    addr = PQ_SYS_TO_PHYS(pxp, buf);

#ifdef __alpha
    poll = (int *) ((char *)PQ_POLL(pxp) + 2 * PX_SYS_TO_DMA(addr));
#else
    poll = (int *) ((char *)PQ_POLL(pxp) + PX_SYS_TO_DMA(addr));
#endif


    _POLL_STIC_PINT(stic,pqp);
    stic->ipdvint = STIC_INT_P_WE;	/* clear pkt done intr bit */

    wbflush();

    if (*poll != STAMP_GOOD)
    {
	px_init_stic(pxp);
	pqp->unwedge_stic++;
	if (*poll != STAMP_GOOD) {
	    pqp->dropped_packet++;
	    PX_DEBUG(PX_TERSE,
		     printf("STIC:CSR 0x%x B:SR 0x%x A 0x%x D 0x%x I 0x%x\n",
			     stic->sticsr, stic->buscsr,
			     stic->busadr, stic->busdat,
			     stic->ipdvint);
		     );
	    return;
	}
    }					/* if N10 running, then must have */
    if (pqp->n10K) {			/* asked permission to do console */
	_POLL_STIC_PINT(stic,pqp);	/* output, so N10 must be waiting */
	PQ_RAM(pxp)->intr_coproc_high = 0;
	PQ_RAM(pxp)->intr_coproc = 0;	/* for OK to proceed... */
    }
}


/******************************************************************
 ** Graphic device ioctl routine.                                **
 ******************************************************************/
pq_ioctl(pxp, cmd, data, flag)
    px_info *pxp;			/* closure */
    caddr_t data;
{
    register pq_info *pqp = (pq_info *) pxp->dev_closure;
    register px_ioc *pqi = (px_ioc *) data;

    switch( pqi->cmd ) 
    {
	/* QIOCGINFO replaced by map_screen */

     case PQ_N10_RESET:
	PX_DEBUG(PX_TALK,
		 printf("pq_ioctl: stop %x\n", pxp);
		 );
	*PQ_RESET(pxp) = pqp->n10K = 0;
	wbflush();
	break;

     case PQ_N10_START:
	if (_pq_serverp == 0) {
	    _pq_serverp = u.u_procp;
	    timeout(px_cpu_idle, (char *)0, PX_CPU_IDLE);
	}
	PX_DEBUG(PX_TALK,
		 printf("pq_ioctl: go %x\n", pxp);
		 );
	_pq_ptpt = (pq_ptpt *) PHYS_TO_K1( px_soft_tlb );
	_pq_ptpt_size = PQ_PTPT_SIZE;
	bzero(_pq_ptpt, _pq_ptpt_size);
	bzero(_pq_intr_send, sizeof(_pq_intr_send));
	bzero(_pq_intr_recv, sizeof(_pq_intr_recv));
	*PQ_START(pxp) = pqp->n10K = 1;
	wbflush();

	/*
	 * express interest in server's vm activity...
	 */
        {
          extern int ws_register_vm_callback();

          ws_register_vm_callback( pxp->screen.screen, pxp->vmHook, pxp );
        }
	break;

     case PQ_N10_INTR:
	pqi->data = pq_intr_coproc(pxp, pqi->data);
	break;

     case PX_MAP_OPTION:		/* PQ_N10_MAP */
	if (pqi->data)
	    copyout(&pxp->px, (caddr_t)pqi->data, sizeof(pxInfo));
#ifdef ultrix
	if ((pqi->data = (u_long)ws_map_region((caddr_t)PHYS_TO_K1(pxp->pxo),
					       sizeof(pq_map), 0600))
	    == NULL)
#else /* ultrix */
	if ((pqi->data = (u_long)ws_map_region((caddr_t)pxp->pxod,
				    pxod_uaddr, sizeof(pq_mapd), 0600, (int *) NULL))
	    == NULL)
#endif /* ultrix */
	    return (ENOMEM);
	pqi->screen = (short) PXG_DTYPE;
	break;

     default:				/* not ours ??  */
	return -1;
    }
    return 0;
}

#include <sys/buf.h>

pq_map_screen(pxp, dp, sp, mp)
    px_info *pxp;			/* closure */
    ws_depth_descriptor *dp;		/* depths */
    ws_screen_descriptor *sp;		/* screens */
    ws_map_control *mp;			/* control */
{
    pq_info *pqp = (pq_info *) pxp->dev_closure;
    int *pxo;				/* board virt addr */
    int *pxod;				/* dense board virt addr */
    u_long rb_phys_addr;		/* physical address of ringbuffer. */
    caddr_t pxInfo_virt_addr;		/* virtual address of pxInfo struct */
    caddr_t rb_virt_addr;		/* virtual address of ringbuffer */
    unsigned long tmp1, tmp2, tmp3, tmp4;
    unsigned long tmp1rb, tmp2rb, tmp3rb;
    int num;

    /* depth and screen already checked in ws_device.c */
    if (mp->map_unmap != MAP_SCREEN) return EINVAL;

    /*
     * map pxInfo and ringbuffer.  everything will be page-aligned,
     * including size, by ws_map_region.  nb: we don't have to map
     * uncached, since the server will invalidate the cache after
     * dma reads from the i860.
     *
     * In BSD, this requires only one region mapping, because the two
     * datastructures are adjacent.  In the OSF kernel, we need to perform
     * two mappings.
     */
#ifdef ultrix
    if ( (tmp2 = (long) ws_map_region((caddr_t)&(pxp->px),
				     sizeof(pxInfo) + PX_RB_SIZE, 0600))
	== NULL)
	return ENOMEM;
#else /* ultrix */
    /*
     * Which entry in px_softc[] is this?
     */
    num = (int)(pxp - px_softc);	/* return 0, 1, 2, ... */

    /*
     * Figure out which pxInfo element to map, and then map it.
     *
     * The address calculations assume that *all* pxinfo structs will fit
     * within a page.  If a valid info struct can span a physical memory page,
     * this code will need work.  -mcb
     */
    tmp1 = (vm_offset_t) ((char *)px_map_inf + ((num*32)<<2)); /* nth pxInfo */
    pxp->px = (pxInfo *) PHYS_TO_K0(tmp1);
    tmp1 = (unsigned long) pxp->px & ~(CLBYTES-1);
    tmp4 = (unsigned long) pxp->px & (CLBYTES-1);

    tmp2 = (long ) ws_map_region((caddr_t) pxp->px, NULL, 
			CLBYTES, 0600, (int *)NULL);

    if (tmp2 == (unsigned long) NULL)
	return ENOMEM;
    tmp2 += tmp4;

    /*
     * Figure out which ringbuffer element to map, and then map it.
     *
     * NOTE:  PX_RB_SIZE is 128K + CLBYTES!  Is this actually compatible with
     * the _96KB increment used below and in steal_mem.c?  Check this!  -mcb
     */

    tmp1rb = (vm_offset_t) ((char *)px_map_pkt + (num*PX_RB_SIZE));
    pxp->ringbuffer = (int *) PHYS_TO_K0(tmp1rb);

    tmp3rb = (PX_RB_SIZE);
    tmp2rb = (long) ws_map_region((caddr_t)pxp->ringbuffer, NULL, 
				    tmp3rb, 0600, (int *)NULL);
    if (tmp2rb == (unsigned long) NULL)
	return ENOMEM;

#endif /* ultrix */


    /*
     * complete user virtual address of pxInfo struct.  this will get picked up
     * by the server in a GET_DEPTH_INFO ioctl.
     */
#if 0
    pxInfo_virt_addr = (caddr_t) (tmp2 | ((long) (&(pxp->px))&(CLBYTES-1)));
#endif /* 0 */
    pxInfo_virt_addr = (caddr_t) tmp2;

    dp->pixmap = pxInfo_virt_addr;

    /*
     * Save the ringbuffer physical and virtual addresses so that we can
     * use them later (and read the code later, too).
     */
#ifdef ultrix
    rb_phys_addr = (u_long) svtophy(pxp->ringbuffer);
    /*
     * ringbuffer is virtually contiguous with the pxInfo struct, so take a
     * shortcut and compute the ringbuffer virtual address from the 
     * pxInfo_virtual_address.
     */
    rb_virt_addr = (caddr_t) (pxInfo_virt_addr + sizeof(pxInfo));
#else /* ultrix */
    rb_phys_addr = (u_long) tmp1rb;
    /* (void) svatophys((vm_offset_t)pxp->ringbuffer, &rb_phys_addr); */
    /*
     * pxp->ringbuffer points to the first byte of the ring buffer.  Make
     * the equivalent process virtual address rb_virt_addr.
     */
    rb_virt_addr = (caddr_t)tmp2rb;
#endif /* ultrix */

    /*
     * 3DA option board addresses - whole shebang.  No need to offset/mask
     * for lower-order bits of pxo address - already page aligned.
     */
#ifdef ultrix
    if ((pxo = (long *)ws_map_region((caddr_t)PHYS_TO_K1(pxp->pxo),
				    sizeof(pq_map), 0600))
	== NULL)
	return (ENOMEM);
#else /* ultrix */
    if ((pxo = (int *)ws_map_region((caddr_t)pxp->pxo, NULL, 
				    sizeof(pq_map), 0600, (int *) NULL))
	== (int *) NULL)
	return (ENOMEM);

#ifdef __alpha

    if ((pxod = (int *) ws_map_region((caddr_t) pxp->pxod, NULL,
				      sizeof(pq_mapd), 0600, (int *) NULL))
        == (int *) NULL )
	return (ENOMEM);

    pxod_uaddr = (caddr_t)pxod;  /* Save the dense mapping for later use */

#endif /* __alpha */

#endif /* ultrix */

    /* Now finish filling in the data structures. */

    pxp->px__stamp_width = pxp->stamp_width;
    pxp->px__stamp_height = pxp->stamp_height;
    pxp->px__n10_present = 1;
    pxp->px__zplanes = px_probe_fb(pxp, 2, -1);
    pxp->px__xplanes = px_probe_fb(pxp, 3, -1);
    pxp->px__rb_size = (PX_RB_SIZE);
    pxp->px__rb_phys = (u_long) pxp->ringbuffer; /* This actually K0addr--LW */
    pxp->px__rb_addr = (int *) rb_virt_addr;
    pxp->px__ib_size = N10_IMAGE_BUFFER_SIZE;

    bzero( (caddr_t) &(pxp->px__dev.pq), sizeof(pqInfo) );
    pxp->px__dev.pq.pxo = pxo;
#ifdef __alpha
    pxp->px__dev.pq.pxod = pxod;
#endif /* __alpha */
    pxp->px__dev.pq.ram = (int *) &((pq_map *)pxo)->ram;
#ifdef __alpha
    pxp->px__dev.pq.ramd = (int *) &((pq_mapd *)pxod)->ram;
#endif /* __alpha */
    pxp->px__dev.pq.ptpt_phys = px_soft_tlb;
    pxp->px__dev.pq.ptpt_size = PQ_PTPT_ENTRIES;
    pxp->px__dev.pq.ram_size = pq_probe_ram(pxp);

#ifdef PROTO_3DA
# ifdef ultrix
    /*
     * unID'd board in slot 2 of low/mid-3D is a smorg-as-board counter
     * board, so map its 2 registers in the unused 2D fields
     */
    if (infop->stamp_height == 1) {
	extern unsigned px_slots;
	u_long slot_addr;

	PX_DEBUG(PX_GAB,
		 printf("pq_ioctl: looking for smorg-as-board...\n");
		 );

	if ((px_slots & (1<<2)) == 0) {
	    PX_DEBUG(PX_GAB,
		     printf("pq_ioctl: slot 2 empty\n");
		     );
	    goto NoTimer;
	}

	slot_addr = PHYS_TO_K1(tc_slotaddr[2]);

	if ((smid = vm_system_smget(PX_DWN_(slot_addr),
				    PX_RND_(0x4),
				    0600)) < 0)
	    {
		printf("pq_ioctl: failed to map counter = %d\n",
		       u.u_error);
		PX_DEBUG(PX_GAB,
			 printf("pq_ioctl: 0x%x 0x%x 0600\n",
				 PX_DWN_(slot_addr), PX_RND_(0x80004));
			 );
		goto NoTimer;
	    }
	if ((tmp1 = (long) smat(smid, 0, 0)) < 0) {
	    PX_DEBUG(PX_GAB,
		     printf("pq_ioctl: failed to attach slot 2 = %d\n",
			     u.u_error);
		     );
	    goto NoTimer;
	}
	if (smctl(smid, IPC_RMID, 0)) {
	    PX_DEBUG(PX_GAB,
		     printf("pq_ioctl: failed to ctl slot 2\n");
		     );
	    goto NoTimer;
	}

	infop->stic_dma_rb = (int *) tmp1;
	infop->stic_reg = (int *) tmp1;

	PX_DEBUG(PX_GAB,
		 printf("pq_ioctl: smorg-as-board mapped 0x%x\n",
			 tmp1);
		 );

     NoTimer:
	;
    }
# endif /* ultrix */
#endif /* PROTO_3DA */

    return 0;
}


pq_probe_ram(pxp)
    px_info *pxp;
{    
    /* determine whether there's 128 or 256 KB of SRAM */
    int *m1p = (int *) &((pq_map *)pxp->pxo)->ram;
    int *m2p = m1p + (2 * N10_RAM_SIZE/sizeof(int) );

    *m1p = 1;
    *m2p = 2;

    wbflush();

    if (*m1p == *m2p)
	return N10_RAM_SIZE;
    else
	return (N10_RAM_SIZE << 1);
}    


void pq_intr_noop(pxp, va)
    px_info *pxp;
    int va;
{
    stic_regs *stic = PQ_STIC(pxp);
    
    PX_DEBUG(PX_GAB,
	     printf("pq_intr_noop: 0x%x (%d)\n", va, _pq_intr_recv[0]);
	     );

    if (stic->ipdvint & STIC_INT_E) {
	px_init_stic(pxp);
	printf("STIC intr 0x%x CSR 0x%x B:CSR 0x%x addr 0x%x dat 0x%x\n",
		stic->ipdvint,
		stic->sticsr, stic->buscsr,
		stic->busadr, stic->busdat);
    }
    else {
	pq_info *pqp = (pq_info *) pxp->dev_closure;
	/*
	 * If we got a bad/spurious intr that isn't from the STIC, then
	 * assume the source and punish it.
	 */

	*PQ_RESET(pxp) = pqp->n10K = 0;
	wbflush();
	*PQ_INTRH_HIGH(pxp) = 0;
        wbflush();
	*PQ_INTRH(pxp) = 0;
	wbflush();
	stic->ipdvint = STIC_INT_CLR;
	wbflush();
    }
}


void pq_intr_pagein(pxp, va)
    px_info *pxp;
    vm_offset_t va;
{
    extern vm_offset_t pmap_extract();
    register pqInfo *pqi = (pqInfo *) &pxp->px__dev.pq;
    pmap_t server_map = _pq_serverp->task->map->vm_pmap;   /* Servers pmap */
    pq_info *pqp = (pq_info *) pxp->dev_closure;
    stic_regs *stic = PQ_STIC(pxp);
    long s;

    /* need to do a pagein */
    PX_DEBUG(PX_YAK,
	     printf("PI(0x%x)\n", va & HST_INTR_MASK);
	     );

    pqi->pgin_vaddr = (int *) ( ( (long)(*PQ_INTRH_HIGH(pxp)) << 32 ) 
                                | (va & 0x00000000ffffe000 ) );

    pqi->pgin_pages = (va & HST_INTR_PADD) >> HST_INTR_SHFT;
    pqi->pgin_dirty = ((va & HST_INTR_DRTY) == 0);

    *PQ_INTRH(pxp) = va;		/* clear the R3K interrupt line */
    wbflush();

    /*
     * If server is in  memory and not being swapped
     */

    s = splvm();   /* Raise ipl to VM level. It's higher than device ipl.  Hmmm ...*/

    if (_pq_serverp->p_flag & SLOAD ) {
#ifdef __alpha

        int vpn = alpha_btop(pqi->pgin_vaddr);
	pt_entry_t *pte = pmap_pte(server_map, pqi->pgin_vaddr);

#else /* __alpha */

        int vpn = btop((u_int) pqi->pgin_vaddr);
	pt_entry_t *pte = vtopte(_pq_serverp, vpn);

#endif /* __alpha */

	/*
	 * if va valid and page valid, fill and set modify bit
	 */

	if ( pmap_extract( server_map, pqi->pgin_vaddr ) ) { 
	    if (pqi->pgin_dirty) {
#ifdef __alpha
	      extern void pmap_set_modify();
	      vm_offset_t phys = alpha_ptob( pte->pg_pfn );
	      pmap_set_modify( phys );
#else
	      pte->pg_m = 1;
#endif
	    }
#ifdef OSF 
	    {
	        pq_ptpt pq;
                int *tmp = (int *) &pq;

		/*
		 * Either STLB entry is bad, or page is really out
		 */
		pq.pq_pg_tag_lo = pq_vtovsn_lo(pqi->pgin_vaddr);
		pq.pq_pg_tag_hi = pq_vtovsn_hi(pqi->pgin_vaddr);
		pq.pq_pg_v = 1;
		pq.pq_pg_pfnum = pte->pg_pfn;
		_pq_ptpt[PQ_STLB(vpn)] = pq;

                tmp++;
                *PQ_INTRH_HIGH(pxp) = *tmp;
                wbflush();
                tmp--;
		*PQ_INTRH(pxp) = *tmp;
                wbflush();
	    }
#else
	    *PQ_INTRH(pxp) = 0;
#endif /* OSF */

	    wbflush();
	    return;
	}

        psignal(_pq_serverp, SIGURG); /* cause a pagein */

        return;

    }

    splx(s);

   /* 
    *	 If the server is gone slap the N10. This should be done in the server
    *    close path, but will double check here for now. This should be
    *    removed when the i860 ucode is working properly.
    */
    
    *PQ_RESET(pxp) = pqp->n10K = 0;
    wbflush();
    *PQ_INTRH_HIGH(pxp) = 0;
    wbflush();
    *PQ_INTRH(pxp) = 0;
    wbflush();
}


void pq_intr_xlate(pxp, va)
    px_info *pxp;
    int va;
{
#ifdef OSF
    pq_intr_pagein(pxp, va);
#else
    register long tmpVA, vsn;
    register pt_entry_t *pte;

    /*
     * If the server isn't in core, or is being swapped out, then...
     */
    if ((_pq_serverp->p_sched & SLOAD) == 0 || (_pq_serverp->p_vm & SLOCK)) {
        pq_intr_pagein(pxp, va);
	return;
    }

    tmpVA = (va & HST_INTR_MASK);
    vsn = PX_PVA_TAG( tmpVA );

    PX_DEBUG(PX_BLAB,
	     printf("xlate: vpn 0x%x vsn 0x%x\n", btop(tmpVA), vsn);
	     );

#ifdef __alpha
    pte = pmap_pte( _pq_serverp->task->map->vm_pmap, tmpVA );
#else /* __alpha */
    pte = vtopte( _pq_serverp, btop(tmpVA) );
#endif /* __alpha */

    PX_DEBUG(PX_GAB,
	     printf("xlate: sp 0x%x (0x%x+0x%x) pte 0x%x\n",
		     _pq_serverp,
		     _pq_serverp->p_datastart,
		     _pq_serverp->p_dsize,
		     pte);
	     );

    if (pte == 0) {
	printf("pq_intr_xlate: bad VA 0x%x\n", tmpVA);
	/* let server segv while handling signal */
	pq_intr_pagein(pxp, va);
	return;
    }

    if (_pq_ptpt[vsn].pq_pg_pfnum)
        printf("pq_intr_xlate(va=0x%x) already set\n", tmpVA);
    _pq_ptpt[vsn].pq_pg_pfnum = btop(svtophy(pte));
    _pq_ptpt[vsn].pq_pg_tag = vsn;

    PX_DEBUG(PX_GAB,
	     printf("pq_intr_xlate(va=0x%x) -> ptpt[0x%x]=0x%x\n",
		     tmpVA, vsn, _pq_ptpt[vsn] );
	     );
    *PQ_INTRH(pxp) = 0;
    wbflush();

#endif /* OSF */

    return;
}


/******************************************************************
 **                                                              **
 ** Graphics device end-of-frame interrupt service routine.      **
 **                                                              **
 ******************************************************************
 *
 * For the 3MAX/3D boards, N10 receives VINT.  VINT is normally off.
 * When vblank intr desired, then intr enabled (twiddle STIC directly).
 * When N10 recv's intr, it propogates to host.  Hopefully,
 * not too much time has passed before we notice it...
 */
void pq_intr_vblank(pxp, count)
    px_info *pxp;
    u_int count;
{
    register struct bt459info *bti = (struct bt459info *)pxp->cf.cc;
    register stic_regs *stic = PQ_STIC(pxp);

    *PQ_INTRH_HIGH(pxp) = 0;
    wbflush();
    *PQ_INTRH(pxp) = 0;			/* since N10 is waiting...*/
    wbflush();

#if 0
    count >>= HST_INTR_SHFT;
    if (count != _pq_intr_recv[HST_INTR_VSYN]) {
	_pq_intr_recv[HST_INTR_VSYN] = count;
    }
#endif
    if (! (bti->dirty_colormap || bti->dirty_cursor))
        _pq_intr_recv[0]++;

    if (bti->dirty_colormap)	bt_clean_colormap(pxp->cmf.cmc);
    if (bti->dirty_cursor)	bt_load_formatted_cursor(bti);

    stic->ipdvint = STIC_INT_V_WE;
    wbflush();

    return;
}


pq_intr_coproc(pxp, cmd)
    px_info *pxp;
    int cmd;
{
    register int i, ans;
    volatile int *intr_coproc = PQ_INTRC(pxp);
    volatile int *read_coproc = &PQ_RAM(pxp)->intr_coproc;
    pq_info *pqp = (pq_info *) pxp->dev_closure;

    if (!pqp->n10K) {
	PX_DEBUG(PX_YAK,
		 printf("pq_intr_coproc: i860 ~live\n");
		 );
	return -1;
    }

    *intr_coproc = cmd;
    wbflush();

    _pq_intr_send[(cmd & PQ_INTR_WHAT) >> PQ_INTR_WSHF]++;

    PX_DEBUG(PX_YAK,
	     printf("pq_intr_coproc(cmd=0x%x)\n", cmd);
	     );

    for ( i = 1000000; i > 0; i-- )
    {
	DELAY(2);			/* 150 cycles for N10 intr roundtrip */
	ans = *read_coproc;		/* 100 cycles to get to handler? */
	switch (ans)			/* N10 running at 25-30ns clock */
	{
	 case 0:
	    return -1;
	 case PQ_INTR_BUF0:		/* 1 */
	    return 0;
	 case PQ_INTR_BUF1:		/* 2 */
	    return 1;
	 default:
	    if (ans != cmd) {
		printf("pq_intr_coproc: i860 bad reply = 0x%x\n", ans);
		break;
	    }
	}
    }

    PX_DEBUG(PX_TERSE,
	     printf("pq_intr_coproc(cmd=0x%x)->i860 dead?!\n", cmd);
	     );

    if (_pq_debug & PQ_DIEonTIMEOUT) {
	/*
	 * if N10 doesn't respond in 3 seconds, close up shop...
	 */
	*PQ_RESET(pxp) = pqp->n10K = 0;	/* kill the N10 */
	wbflush();
	if (_pq_serverp)
	    psignal(_pq_serverp, SIGKILL); /* kill the server */
    }
    else
	pqp->n10K = 0;

    return -1;
}


pq_invalidate_gcp_tlb( cmd, va, data )
    int cmd;
    vm_offset_t va;
    caddr_t data;
{
    px_info  *pxp = (px_info *)data;
    int       vpn;
    

    vpn = alpha_btop(va); /* Get page number */

    switch (cmd) {

#ifdef DO_NEW_STYLE_INVALIDATE

     case PMAP_COPROC_INVALIDATE_STLB:
	PX_DEBUG(PX_NEVER,
		 printf("pq_invalidate_gcp_tlb(cmd=one,vpn=0x%x)\n", vpn);
		 );
	*(long *) &_pq_ptpt[PQ_STLB(vpn)] = 0;
    	vpn = (long)ptob(vpn) & PQ_INTR_MASK; /* now va, page aligned */
        pq_intr_coproc( pxp, vpn | PQ_INTR_INV1 );
	break;

     case PMAP_COPROC_EXIT:
	PX_DEBUG(PX_NEVER,
		 printf("pq_invalidate_gcp_tlb(cmd=all,vpn=0x%x)\n", vpn);
		 );
	bzero(_pq_ptpt, _pq_ptpt_size);
        pq_intr_coproc( pxp, PQ_INTR_INVA );
	break;

#else	/* DO_NEW_STYLE_INVALIDATE */

     case PDEVCMD_ONE:
	PX_DEBUG(PX_NEVER,
		 printf("pq_invalidate_gcp_tlb(cmd=one,vpn=0x%x)\n", vpn);
		 );
	*(long *) &_pq_ptpt[PQ_STLB(vpn)] = 0;
    	vpn = (long)ptob(vpn) & PQ_INTR_MASK; /* now va, page aligned */
        pq_intr_coproc( pxp, vpn | PQ_INTR_INV1 );
	break;

     case PDEVCMD_ALL:
     case PDEVCMD_DMA:
	PX_DEBUG(PX_NEVER,
		 printf("pq_invalidate_gcp_tlb(cmd=all,vpn=0x%x)\n", vpn);
		 );
	bzero(_pq_ptpt, _pq_ptpt_size);
        pq_intr_coproc( pxp, PQ_INTR_INVA );
	break;

#endif	/* DO_NEW_STYLE_INVALIDATE */

     default:
	PX_DEBUG(PX_GAB,
		 printf("pq_invalidate_gcp_tlb(cmd=bad 0x%x)\n", cmd);
		 );
	panic("pq_invalidate_gcp_tlb");
    }
    return 0;
}


void
pq_close(pxp)
    px_info *pxp;
{
    pq_info *pqp = (pq_info *) pxp->dev_closure;

    /* unexpress interest in server's vm activity... */
    if (_pq_serverp) {
      extern int ws_unregister_vm_callback();

      PX_DEBUG(PX_GAB, 
               printf("pq_close: pid=%d\n", _pq_serverp->p_pid);
              );
      ws_unregister_vm_callback( pxp->screen.screen );
      _pq_serverp = 0;
    }
    *PQ_RESET(pxp) = pqp->n10K = 0;	/* halt N10 */
    wbflush();
    pxp->px__n10_present = 0;
}


void pq_getImageBuffer(pxp, p_sva, p_stic)
    px_info *pxp;
    int **p_sva;			/* sys virt addr */
    int *p_stic;			/* stic phys addr */
{
    unsigned int sram_addr;

    *p_sva = PQ_RAMD(pxp)->pixbuf; 		/* K1 addr */
    sram_addr = PQ_SYS_TO_PHYS(pxp, *p_sva);	/* SRAM offset */
    *p_stic = PX_SYS_TO_STIC(sram_addr);	/* STIC addr */
}
