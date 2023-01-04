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
static char *rcsid = "@(#)$RCSfile: steal_mem.c,v $ $Revision: 1.1.9.4 $ (DEC) $Date: 1993/11/17 23:13:05 $";
#endif

/************************************************************************
 *									*
 *			Copyright (c) 1991, 1993 by			*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

#include <sys/param.h>
#include <hal/cpuconf.h>
#include <machine/cpu.h>
#include <mach/vm_param.h>
#include <sys/types.h>

/*
 * Config-generated headers
 */
/* [DEC Include BEGIN] */
/* [DEC-PXG Include BEGIN] */
#include "px.h"
/* [DEC-PXG Include END] */
/* [DEC-PV Include BEGIN */
#include "pv.h"
/* [DEC-PV Include END] */
/* [DEC-TE Include BEGIN */
#include "te.h"
#include <io/dec/netif/if_tereg.h>
/* [DEC-TE Include END] */
/* [DEC Include END] */

/*
 * External definitions
 */
/* [DEC ExternalDeclarations BEGIN] */
/* [DEC ExternalDeclarations END] */

/*
 * Common
 */
/* [DEC ExternalDefinitions BEGIN] */
vm_offset_t ws_eventQueue;
vm_offset_t ws_heap_start = (vm_offset_t) NULL;
vm_offset_t ws_heap_end = (vm_offset_t) NULL;
vm_offset_t ws_heap_at = (vm_offset_t) NULL;
size_t ws_heap_size = (size_t) 0;
/* [DEC ExternalDefinitions END] */

/*
 * Versioning information
 */
/* [DEC Version BEGIN] */
/* [DEC-PXG Version 1.0.0] */
/* [DEC-PV Version 1.0.0] */
/* [DEC Version END] */

/*
 * Various data definitions
 */
/* [DEC DataDefinition BEGIN] */

/* [DEC-PXG DataDefinition BEGIN] */
/*
 * Define the PX physical memory descriptors.
 */
#if NPX > 0
vm_offset_t px_mem_top;
vm_offset_t px_mem_bot;
vm_offset_t px_soft_tlb;
vm_offset_t px_map_pkt;
vm_offset_t px_map_inf;
#endif /* NPX > 0 */
/* [DEC-PXG DataDefinition END] */

/* [DEC-PV DataDefinition BEGIN] */
/*
 * PV memory descriptors
 */
#if NPV > 0
vm_offset_t pv_mem_bot;
vm_offset_t pv_mem_top;
vm_offset_t pv_page_table_0;		/* for PV device */
vm_offset_t pv_page_table_1;		/* for GCP device */
vm_offset_t pv_buffers;			/* ring buffs, infos, misc */
size_t pv_buffer_size;
size_t pv_page_table_0_size;
size_t pv_page_table_1_size;
#endif /* NPV */
/* [DEC-PV DataDefinition END] */

/* [DEC-TE DataDefinition BEGIN] */
#if NTE > 0
vm_offset_t te_bufptr[NTE];
#endif /* NTE */
/* [DEC-TE DataDefinition END] */

/* [DEC DataDefiniition END] */

extern	int	cpu;
extern	vm_offset_t	avail_start, avail_end;
extern	int	maxmem;

int
steal_mem()
{
    /*
     * Binary logger needs a page aligned page to map shared 
     * screen control info.
     * And it's needed now.  
     * A cnprobe is expected to be done after returning.
     */
    ws_eventQueue = avail_start;
    avail_start += PAGE_SIZE;

    /*
     * Grab an additional page which will be used for various info
     * areas.  These will often be mapped into a server process.
     */
    ws_heap_start = ws_heap_at = avail_start;
    avail_start += PAGE_SIZE;
    ws_heap_end = avail_start;
    ws_heap_size = PAGE_SIZE;

/* [DEC CodeDefinition BEGIN] */
/* [DEC-PXG CodeDefinition BEGIN] */
#if NPX > 0

    /*
     * Now check to see if we need to grab packet/ring-buffer
     * memory for PixelStamp accelerators, in particular, the
     * PX option.  It must be within the first 8MB of physical
     * memory, contiguous, unpaged, and all that good stuff...
     */
    if (cpu != DS_3100) {
	vm_offset_t aligned;
	unsigned skipped;
	/*
	 * Constrain to more stringent requirements of PX, where
	 * the image buffer and SRVCom must to be on the same 32KB
	 * chunk.  (16KB-aligned would be sufficient, because we only need
	 * to ensure that the image_buf lie within a single 16Kbyte
	 * chunk.  As a result, the server need not worry about the gaps
	 * in Pixelstamp address space).  Up to 3 PX option boards are 
	 * supported here.
	 */
#define     _uCODE_HDR (12)
#define	    _16KB  (1 << 14)
#define     _96KB  (6*_16KB)
#define	    _QSIZE (NPX*(1<<17))
#define     _INFO  ((NPX*32)<<2)
#define	    _PT    (NPX*(1<<15))
#define     _8MB   (0x800000)
	skipped = _16KB - ((unsigned)avail_start & (_16KB-1));
	if (skipped >= _uCODE_HDR) {
	    skipped -= _uCODE_HDR;
	}
	else {
	    skipped += _16KB - _uCODE_HDR;	/* more space, keep aligned */
	}
	aligned = avail_start + skipped;
	if ( ( aligned + _QSIZE ) <= avail_end ) {
	    px_mem_bot = avail_start;
	    px_map_pkt = aligned;
	    avail_start = round_page(aligned + _QSIZE);
	    /*
	     * if we can't salvage the skipped memory for pxInfo
	     * structures, then grab more memory for it...
	     */
	    if (skipped < _INFO) {
		px_map_inf = avail_start;
		avail_start += _INFO;
	    }
	    else {
		px_map_inf = px_mem_bot;
	    }
	    px_soft_tlb = avail_start = round_page(avail_start);

	    /*
	     * Get some pages for the PX software TLB
	     */
	    avail_start = round_page(avail_start + _PT);
	    px_mem_top = avail_start;
	}
	else {
	    px_map_pkt = (vm_offset_t) 0;		/* hmmm... */
	    dprintf("steal_mem: Couldn't allocate memory for PX[G] support.  ");
	    dprintf("steal_mem: PX[G] options will not be enabled.\n");
	    dprintf("             avail_start=0x%x aligned=0x%x\n",
				avail_start, aligned);
	}
#undef      _uCODE_HDR
#undef      _16KB
#undef      _96KB
#undef      _QSIZE
#undef      _INFO
#undef	    _PT
#undef      _8MB
    }

#endif /* NPX > 0 */
/* [DEC-PXG CodeDefinition END] */

/* [DEC-PV CodeDefinition BEGIN] */
#if NPV > 0

    /*
     * PV memory allocation
     *
     * Needed:
     *
     * 128KB ring buffer per module
     * 4K entry shadow page table for PV per module
     * 8K entry shadow page table for GCP per module
     * 1 page clip data buffer per module
     * 1 page private packet area
     * 1 page for overrun and dead data
     * 1 page for info structs
     *
     */

    {
	unsigned long skipped;

	/*
	 * For PVA, shadow page table must be aligned to a quantity
	 * equivalent to the page table size.
	 */
#define _128KB		(1<<17)
#define	_4KEntry	((1<<12) * sizeof(int *))
#define	_4KTotal	(NPV*_4KEntry)
#define	_8KEntry	((1<<13) * sizeof(int *))
#define	_8KTotal	(NPV*_8KEntry)
#define _PVBuffs	( _128KB * NPV + (3 + NPV) * NBPG )

	pv_mem_bot = avail_start;
	skipped = _8KEntry - ((unsigned)avail_start & (_8KEntry-1));
	if ( skipped >= _4KTotal ) {
	    pv_page_table_0 = avail_start + skipped - _4KTotal;
	    pv_page_table_1 = avail_start + skipped;
	    avail_start += skipped + _8KTotal;
	    skipped -= _4KTotal;
	}
	else {
	    pv_page_table_0 = avail_start + skipped + _8KTotal;
	    pv_page_table_1 = avail_start + skipped;
	    avail_start += skipped + _8KTotal + _4KTotal;
	}
	if ( skipped >= _PVBuffs ) {
	    pv_buffers = pv_mem_bot;
	}
	else {
	    pv_buffers = avail_start;
	    avail_start += _PVBuffs;
	}
	pv_buffer_size = _PVBuffs;
	pv_page_table_0_size = _4KEntry;
	pv_page_table_1_size = _8KEntry;
	avail_start = round_page(avail_start);
	pv_mem_top = avail_start;
#undef      _128KB
#undef      _4KEntry
#undef	    _4KTotal
#undef      _8KEntry
#undef	    _8KTotal
#undef      _PVBuffs
    }

#endif /* NPV > 0 */
/* [DEC-PV CodeDefinition END] */

    /* 
     * TGEC (Cobra Ethernet device) driver buffer allocations: the requirement 
     * here is that all addresses specified to the device be within the first 
     * Gbyte (30-bits) of the physical address space. Individual buffers are
     * carved up in the driver; all we need to do here is get a big chunck 
     * that's at least octaword-aligned. Since we are doing this stealing
     * rather early, and since we are already page-aligned, both requirements
     * can be met easily.
     */
#if NTE > 0

    {
    register u_int bytes_required = TE_BUFFER_REQ; /* if_tereg.h */
    register u_int pages_required = (bytes_required / PAGE_SIZE);
    register vm_offset_t paddr, saved_avail_start = avail_start;
    register u_int i=0;

    /* Page round-off */
    if (bytes_required % PAGE_SIZE)
	pages_required++;

    /* Skip over bad pages, if any */
    while (i != pages_required) {
    	for (i=0,paddr=avail_start; i<pages_required; i++,paddr+=PAGE_SIZE)
		if (pmap_valid_page(paddr) == FALSE) {
			avail_start = paddr+PAGE_SIZE;
			break;
		}
    }

    /* Ensure our last page is still within a Gbyte before we grab it. */
    if (((paddr-PAGE_SIZE) & 0xFFFFFFFFC0000000) == 0)
    	for (i=0; i<NTE; i++) {
    		te_bufptr[i] = PHYS_TO_KSEG(avail_start);
    		avail_start += (pages_required*PAGE_SIZE);
    	}
    else 
	/* Shouldn't get here, but if we do, we'll gripe at probe time. */
	avail_start = saved_avail_start;
    }

#endif
/* [DEC CodeDefinition END] */

    /* 
     * done
     */
    return (0);
}

