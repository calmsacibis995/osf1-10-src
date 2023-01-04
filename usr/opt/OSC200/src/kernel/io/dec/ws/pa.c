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
static char *rcsid = "@(#)$RCSfile: pa.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/06/24 22:43:29 $";
#endif
#ifndef lint
static char *sccsid = "%W%      (ULTRIX/OSF)  %G%";
#endif lint
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
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
#define _PA_C_
/*
 * PMAG-BC specific routines (DS5000 2D-Accelerator [PX])
 *
 * Modification History
 *
 * 00-Dec-1991	Lloyd Wheeler
 *		Finished conversion to OSF (incl. new ws_map_region() interface)
 *
 * 00-Aug-1991	Sam Hsu
 *		Began conversion to OSF.
 *
 * 13-Sep-1990	Sam Hsu
 *		Cleanup and merge changes to ga.c since original port, 4.ti.
 *
 * 27-May-1990	Win Treese
 *		Created, based on vfb03.c and io/tc/ga.c
 */
#  include <machine/pmap.h>		/* get vm stuff, inc. svatophys() */
#  include <data/ws_data.c>
#  include <data/bt_data.c>
#  include <data/px_data.c>
#  include <data/pa_data.c>

#if 0
#include "PXdbg.h"
#endif /* 0 */
/*
 * Attach the PX graphic device.
 */
pa_attach(ctlr, pxp)
    struct controller *ctlr;
    px_info *pxp;
{
    /* Set up the 2DA. */
    return pa_setup(pxp, ctlr->addr, ctlr->ctlr_num, ctlr->flags);
}


pa_bootmsg(ctlr, pxp)
    struct controller *ctlr;
{
    printf("pa%d (5x1 8+8+0+0)\n", ctlr->ctlr_num);
}


#define SUBMIT_PACKET(i) (status = *pCom2d->intr_qpoll[i])


void pa_enable_interrupt(closure)
    struct bt459info *closure;		/* XXX */
{
    register px_info *pxp = &px_softc[closure->unit];
    register stic_regs *stic = PA_STIC(pxp);

    /* intr's are normally on, so just allow vblank intr's to come thru */
    stic->ipdvint = (STIC_INT_V_WE | STIC_INT_V_EN);
}

/*
 * Handle an interrupt from the STIC.
 */
void pa_interrupt(ctlr, pxp)
    struct controller *ctlr;
    px_info *pxp;
{
    register stic_regs *stic = PA_STIC(pxp);
    pa_info *pap = (pa_info *) pxp->dev_closure;
    unsigned int fake = 0;

    /*
     * Packet interrupt.
     */
    if (stic->ipdvint & STIC_INT_P) {
	register pa_ComAreaPtr cmd_area = (pa_ComAreaPtr) pxp->ringbuffer;
	register pa_Com2dPtr pCom2d = (pa_Com2dPtr) &cmd_area->SRV2DCom;
	register int cp;
	register pa_PacketPtr packet;
	register status;

	fake = ++(pap->pkt_intr_count);
	/*
	 * clear *only* packet done interrupt
	 */
	stic->ipdvint = STIC_INT_P_WE | (stic->ipdvint & STIC_INT_P_EN);

	/*
	 * if we were idle, dismiss this as a spurious interrupt
	 */
	if (pCom2d->lastRead == pCom2d->lastWritten
	    || !(pCom2d->intr_status & PA_INTR_ACTIVE))
	    return;

	/*
	 * If we are in the process of sending a packet through a
	 * cliplist, fix up the packet with the next cliprect
	 * and resubmit.  If the last cliprect just completed,
	 * clear the clipping status.
	 */
	if (pCom2d->intr_status & PA_INTR_CLIP) {
	    if (pCom2d->numCliprect-- > 0) {
		*pCom2d->fixCliprect = *pCom2d->pCliprect++;
		SUBMIT_PACKET(pCom2d->lastRead);
		return;
	    } else {
		pCom2d->intr_status &= ~PA_INTR_CLIP;
	    }
	}
     loop:
	/* Point past packet just completed. */

	cp = pCom2d->lastRead = NEXT_BUF(pCom2d, pCom2d->lastRead);

	/* If there are no more packets to process, set idle status. */

	if (cp == pCom2d->lastWritten) {
	    pCom2d->intr_status &= ~PA_INTR_ACTIVE;
	    return;
	}

	packet = (pa_PacketPtr) cmd_area->IntrBuf[cp];

	if (packet->un.un.opcode == N_PASSPACKET) {
	    int clip_idx = (int) (unsigned short)
		packet->un.PassPacket.cliplist_sync;
	    if (clip_idx != N_NO_CLIPLIST) {
		int nrects =
		    cmd_area->ClipL[clip_idx].numClipRects;
		int fix_off =
		    DECODE_CLIP_INDEX(packet->un.PassPacket.data);
		pa_StampClipRect *pFixup =
		    (pa_StampClipRect *)
			&packet->un.PassPacket.data[fix_off];
		*pFixup =
		    cmd_area->ClipL[clip_idx].clipRects[0];
		if (nrects > 1) {
		    pCom2d->pCliprect =
			&cmd_area->ClipL[clip_idx].clipRects[1];
		    pCom2d->numCliprect = nrects - 1;
		    pCom2d->fixCliprect = pFixup;
		    pCom2d->intr_status |= PA_INTR_CLIP;
		}
	    }
	    SUBMIT_PACKET(cp);
	    return;
	} else {			/* not a pass packet, skip it */
	    goto loop;
	}
    }

    /*
     * Vertical retrace interrupt (one-shot).  Hopefully, we won't get a packet
     * and vblank intr at the same time, so do the packet dispatch first since
     * it's the common case.
     */
#define STIC_INT_VBL (STIC_INT_V|STIC_INT_V_EN)

    if ((stic->ipdvint & STIC_INT_VBL) == STIC_INT_VBL) {
	register struct bt459info *bti = (struct bt459info *)pxp->cf.cc;

	fake = ++(pap->vert_intr_count);
	if (bti->dirty_colormap) bt_clean_colormap(pxp->cmf.cmc);
	if (bti->dirty_cursor)	 bt_load_formatted_cursor(bti);
	stic->ipdvint = STIC_INT_V_WE;
    }

    /*
     * Error interrupt.
     */
    if (stic->ipdvint & STIC_INT_E) {
	fake = ++(pap->err_intr_count);
	px_init_stic(pxp);
	printf("STIC intr 0x%x CSR 0x%x B:CSR 0x%x addr 0x%x dat 0x%x\n",
		stic->ipdvint,
		stic->sticsr, stic->buscsr,
		stic->busadr, stic->busdat);
    }

    /*
     * stray ???
     */
    if (!fake) (pap->stray_intr_count)++;
}


/*
 * Set up the 2DA.
 */
pa_setup(pxp, address, unit, flags)
    px_info *pxp;
    caddr_t address;
    int unit;
    int flags;
{
    pa_info *pap = &pa_softc[unit];
    struct bt459info *bti = (struct bt459info *)pxp->cf.cc;
    long size;
    long tmp, top;

    /* Initialize the device closure. */
#ifndef __alpha
    address = (caddr_t) PHYS_TO_K1(address);
#endif

    pxp->pxo = address;			/* Board. */
    pxp->stamp += (long)address;	/* Stamp registers. */
    pxp->stic += (long)address;		/* STIC registers. */
    bzero((caddr_t)pap, sizeof(pa_info));
    pxp->dev_closure = (caddr_t) pap;

#ifdef ultrix
    /*
     * The STIC DMA area for the 2D accelerator has unusual alignment
     * requirements: the physical starting address must be a
     * 32K-aligned address.
     * Because all stamp packets have a 3-longword microcode header,
     * the ring buffer must start 12 bytes before that.
     */
    pxp->ringbuffer = (int *) svtophy(pxp->memory);

    /*
     * If the ringbuffer wasn't allocated in the first 8 Meg, we lose.
     * Propogate a "device not configured" back to the caller
     * and pretend the board isn't there, since it can't be supported
     * properly.  Ditto if the memory isn't physically contiguous.
     */
    if ( (long) pxp->ringbuffer >= (0x800000 - PX_RB_SIZE) ) {
	printf("pa%d: 0x%x option not enabled (rb %x)\n",
	       unit, address, pxp->ringbuffer);
	return 0;			/* can't support this option! */
    }

    top = (long)pxp->ringbuffer + PX_RB_SIZE;
    for (tmp = (long)pxp->ringbuffer, run = pxp->memory; tmp < top; tmp += 4096, run += 4096) {
	if ((long)svtophy(run) != tmp) {
	    printf("pa%d: 0x%x option not enabled (adr %x %x=%x)\n",
		   unit, address, tmp, svtophy(run), run);
	    return 0;
	}
    }

    /*
     * Now align to 32K.  Actually, the ringbuffer pointer gets set to
     * be 12 bytes before the actual STIC packet.  STIC can see 128KB
     * of 32KB-aligned contiguous memory.
     */
#define	TMPMASK	( (1 << 15) - 1)	/* Mask off to 32Kbytes. */
    tmp = (((long)pxp->ringbuffer) + TMPMASK & ~TMPMASK) - 12;
#undef TMPMASK
    /* If this is before memory[], adjust it upwards by 32Kbytes. */
    if (tmp < (long)pxp->ringbuffer)
	pxp->ringbuffer = (int *) (tmp + (1 << 15));
    else
	pxp->ringbuffer = (int *) tmp;
    /*
     * Finally, convert this address back into a "virtual" one (K1 space).
     */
    pxp->ringbuffer = (int *) PHYS_TO_K1(pxp->ringbuffer);
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
	extern vm_offset_t px_map_pkt;
	extern vm_offset_t px_map_inf;
	extern vm_offset_t px_mem_bot;
	extern vm_offset_t px_mem_top;

	pxp->ringbuffer = (int *) PHYS_TO_K0(px_map_pkt);
	pxp->px = (pxInfo *) PHYS_TO_K0(px_map_inf);
    }
#endif /*ultrix*/

    /*
     * Interrupt polling addresses for server packet buffers.
     */
    {
	register int i;
	register pa_ComAreaPtr cmd_area = (pa_ComAreaPtr) pxp->ringbuffer;
	register caddr_t poll_addr = (caddr_t) PA_POLL(pxp);

	for (i = 0; i < PA_QUEUE_PACKETS; i++) {
	    cmd_area->SRV2DCom.intr_qpoll[i] =
		(long *) (poll_addr + PX_SYS_TO_DMA(&cmd_area->IntrBuf[i][3]));
	}
    }

    /* now install our vblank interrupt enable routine */
    bti->enable_interrupt = pa_enable_interrupt;
    
    return 1;				/* setup'd OK */
}


/*
 * Allocate a PixelStamp packet.
 * There are two buffers, selected, by bufselect, which is toggled on
 * each invocation.  The extra offset of 12 allows for the 3-longword
 * microcode header.
 */
int *pa_getPacket(pxp)
    px_info *pxp;
{
    pa_info *pap = (pa_info *) pxp->dev_closure;
    register int *buf;

    buf = pxp->ringbuffer + ( (PA_CMDBUF0_SIZE >> 2) * pap->bufselect) + 3;
    pap->bufselect ^= 1;

    return(buf);
}


/*
 * Send a PixelStamp packet.
 */
void pa_sendPacket(pxp, buf)
    px_info *pxp;
    caddr_t buf;
{
    register stic_regs *stic = PA_STIC(pxp);
    pa_info *pap = (pa_info *) pxp->dev_closure;
    volatile u_long *poll;
    register u_int i, save_ipdvint;

    poll = (u_long *) ((char *) PA_POLL(pxp)
		       + PX_SYS_TO_DMA(buf));

    /* Disable packet-done interrupts. */
#define STIC_INT_PKT (STIC_INT_P|STIC_INT_P_EN)

    save_ipdvint = (stic->ipdvint & STIC_INT_PKT) | STIC_INT_P_WE;
    stic->ipdvint = save_ipdvint & (STIC_INT_P|STIC_INT_P_WE);
    wbflush();

    /*
     * Wait for STIC to be ready to accept the next packet.
     * Note that we never wait forever.   If we time out, we check
     * to see if it is ready anyway.  If it isn't, then there's a
     * problem.  This gets around the race condition when we disabled
     * the PINT.
     */
    for (i = 0; i < STAMP_RETRIES; i++) {
	if (stic->ipdvint & STIC_INT_P) break;
	DELAY(STAMP_DELAY);
    }

    /*
     * Restore the interrupt state, clearing the STIC_INT_P bit.
     * If interrupts were enabled before, they will be re-enabled.
     */
    stic->ipdvint = save_ipdvint & (STIC_INT_P_EN|STIC_INT_P_WE);
    wbflush();

    if (*poll != STAMP_GOOD) {
	printf("STIC dead?! CSR 0x%x Bus CSR 0x%x addr 0x%x dat 0x%x int 0x%x\n",
	       stic->sticsr, stic->buscsr,
	       stic->busadr, stic->busdat, stic->ipdvint);
	px_init_stic(pxp);
	wbflush();
	/* XXX is this kosher? */
	i = *poll;
    }
}


/*
 * Map the screen.
 */
pa_map_screen(pxp, dp, sp, mp, dev)
    px_info *pxp;			/* closure */
    ws_depth_descriptor *dp;		/* depths */
    ws_screen_descriptor *sp;		/* screens */
    ws_map_control *mp;			/* control */
{
    pa_info *pap = (pa_info *) pxp->dev_closure;
    u_int poll_addr;			/* "virtual" address of poll space. */
    long rb_poll_off;			/* offset from start of poll space. */
    long rb_phys_addr;			/* physical address of ringbuffer. */
    caddr_t pxInfo_virt_addr;		/* virtual address of pxInfo struct */
    caddr_t rb_virt_addr;		/* virtual address of ringbuffer */
    int *pxo;				/* board virt addr */
#ifdef ultrix
    int tmp1, tmp2, tmp3;
#else /* ultrix */
    long int tmp1, tmp2, tmp3;
    long int tmp1rb, tmp2rb, tmp3rb;
    int num;
    extern vm_offset_t px_map_pkt;	/* packet/ring buffers */
    extern vm_offset_t px_map_inf;	/* pxInfo structures */
    extern vm_offset_t px_mem_bot;
    extern vm_offset_t px_mem_top;
#endif /* ultrix */

    /* depth and screen already checked in ws_device.c */
    if (mp->map_unmap != MAP_SCREEN) return EINVAL;

    /*
     * Map the pxInfo structure and the ringbuffer into the server's address
     * space.  In the BSD kernel, this requires only one region mapping
     * because the two datastructures are adjacent.  In the OSF kernel, we
     * need to perform two mappings.
     */
#ifdef ultrix
    /*
     * everything will be page-aligned, including size, by ws_map_region
     */
    tmp1 = (long) svtophy(&pxp->px);
    tmp3 = sizeof(pxInfo) + PX_RB_SIZE;
    if ((tmp2 = (long) ws_map_region((caddr_t)PHYS_TO_K1(tmp1),
				    tmp3, 0600)
	 ) == NULL) return ENOMEM;
#else /* ultrix */
    /*
     * Which entry in px_softc[] is this?
     */
    num = (int)(pxp - px_softc);	/* return 0, 1, 2, ... */

    /*
     * Figure out which pxInfo element to map, and then map it.
     */
    tmp1 = (vm_offset_t) ((char *)px_map_inf + ((num*32)<<2)); /* nth pxInfo */
    pxp->px = (pxInfo *) PHYS_TO_K0(tmp1);
    tmp3 = sizeof(pxInfo);
    tmp2 = (vm_offset_t) 
	    ws_map_region((caddr_t)pxp->px, NULL, tmp3, 0600, (int *)NULL);

    /*
     * Figure out which ringbuffer element to map, and then map it.
     */
#define     _16KB  (1 << 14)
#define     _96KB  (6*_16KB)
    tmp1rb = (vm_offset_t) ((char *)px_map_pkt + (num*_96KB));
    pxp->ringbuffer = (int *) PHYS_TO_K0(tmp1rb);
    tmp3rb = PA_RBUF_SIZE;
    tmp2rb = (vm_offset_t)
	ws_map_region((caddr_t)pxp->ringbuffer, NULL, tmp3rb, 0600, (int *)NULL);
#undef      _16KB
#undef      _96KB
#endif

    /*
     * complete user virtual address of pxInfo struct.  this will get picked up
     * by the server in a GET_DEPTH_INFO ioctl
     */
#if 0
#ifdef ultrix
    pxInfo_virt_addr = (caddr_t)(tmp2 | ((long)(&(pxp->px))&(CLBYTES-1)));
#else
    pxInfo_virt_addr = (caddr_t)(tmp2 | ((long)(pxp->px & (PAGE_SIZE-1))));
#endif
#endif /* 0 */
    pxInfo_virt_addr = (caddr_t)tmp2;

    /*
     * PX only supports root depth
     */
    dp->pixmap = pxInfo_virt_addr;

    /*
     * The server uses the "physical" address to figure out
     * which address in STIC polling space to read in order to
     * dispatch a packet.  However, the STIC's physical addresses
     * do not map directly onto host physical addresses.  Furthermore,
     * we are not interested in the address of the ucode packet,
     * but rather the embedded STIC packet within.  This is the origin
     * of the mysterious fudge factor of 12.  The physical address
     * points to the start of the ring buffer, but the STIC address
     * points to the first STIC packet in the ring buffer.
     */
#ifdef ultrix
    rb_phys_addr = (long) svtophy(pxp->ringbuffer);
    rb_poll_off = PX_SYS_TO_DMA(rb_phys_addr + 12);
    /*
     * ringbuffer points somewhere into memory[].  find the offset, and
     * make the equivalent user virtual address rb_virt_addr.
     */
    tmp1 = (long)rb_phys_addr  - (long)svtophy(pxp->memory);
    rb_virt_addr = pxInfo_virt_addr + (long)(sizeof(pxInfo) + tmp1);
#else /* ultrix */
    (void) svatophys((vm_offset_t)pxp->ringbuffer, &rb_phys_addr);
    rb_poll_off = PX_SYS_TO_DMA(rb_phys_addr + 12);
    /*
     * pxp->ringbuffer points to the first byte of the ring buffer.  Make
     * the equivalent virtual address rb_virt_addr.
     */
#if 0
    rb_virt_addr = (caddr_t)(tmp2rb | ((long)(pxp->ringbuffer & (PAGE_SIZE-1))));
#endif /* 0 */
    rb_virt_addr = (caddr_t)tmp2rb;
#endif /* ultrix */

    /*
     * 2DA option board addresses - whole shebang.  No need to offset/mask
     * for lower-order bits of pxo address - already (hopefully) page aligned.
     */
    if ((pxo = (int *)
#ifdef ultrix
	 ws_map_region((caddr_t)PHYS_TO_K1(pxp->pxo),
				    sizeof(pa_map), 0600)
#else
	 ws_map_region((caddr_t)PHYS_TO_K1(pxp->pxo), NULL, 
				    sizeof(pa_map), 0600, (int *)NULL)
#endif
	) == NULL)
	return (ENOMEM);

    poll_addr = (u_int) &(((pa_map *)pxo)->stic_poll_reg);

    /* Now finish filling in the data structures. */

    pxp->px__stamp_width = pxp->stamp_width;
    pxp->px__stamp_height = pxp->stamp_height;
    pxp->px__n10_present = -2;
    pxp->px__zplanes = pxp->px__xplanes = 0;
    pxp->px__rb_size = PA_RBUF_SIZE;
    pxp->px__rb_phys = PX_SYS_TO_STIC(rb_phys_addr + 12);
    pxp->px__rb_addr = (int *) rb_virt_addr;
    pxp->px__ib_size = PA_IMAGE_BUFFER_SIZE;

    pxp->px__dev.pa.pxo = pxo;
    pxp->px__dev.pa.stic_regs = &(((pa_map *)pxo)->stic_reg);
    pxp->px__dev.pa.stic_dma_rb = (int *)(poll_addr + rb_poll_off);

    /*
     * These addresses are used by the server when it is directly
     * submitting packets to the STIC.  This is fairly rare, as the
     * server prefers to generate an interrupt to kickstart the world.
     */
    {					/* Initialize interrupt buffers. */
	register int i;
	register pa_ComAreaPtr cmd_area = (pa_ComAreaPtr) pxp->ringbuffer;

	for (i = 0; i < PA_QUEUE_PACKETS; i++) {
	    cmd_area->SRV2DCom.srv_qpoll[i] =
		(long *)(poll_addr + PX_SYS_TO_DMA(&cmd_area->IntrBuf[i][3]));
	}
    }
    return 0;
}


/******************************************************************
 ** Graphic device ioctl routine.                                **
 ******************************************************************/
pa_ioctl(pxp, cmd, data, flag)
    register px_info *pxp;		/* closure */
    caddr_t data;
{
    register px_ioc *pxi = (px_ioc *) data;

    switch( pxi->cmd )
    {
     case PX_MAP_OPTION:
	if (pxi->data)
	    copyout(&pxp->px, (caddr_t)pxi->data, sizeof(pxInfo));
#ifdef ultrix
	if ((pxi->data = (u_long)ws_map_region((caddr_t)PHYS_TO_K1(pxp->pxo),
					       sizeof(pa_map), 0600))
	    == NULL)
#else /* ultrix */
	if ((pxi->data = (u_long)ws_map_region((caddr_t)PHYS_TO_K1(pxp->pxo),
				    NULL, sizeof(pa_map), 0600, (int *) NULL))
	    == NULL)
#endif /* ultrix */
	    return (ENOMEM);

	pxi->screen = (short) PX_DTYPE;
	break;

     default:				/* not ours ??  */
	return -1;
    }
    return 0;
}
