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
static char	*sccsid = "@(#)$RCSfile: uba.c,v $ $Revision: 1.2.16.3 $ (DEC) $Date: 1993/09/21 21:55:43 $";
#endif /* lint */
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
 * derived from uba.c	2.7      (ULTRIX)        3/6/90";
 */

/* Modification History:
 *
 * 23-Feb-90 -- sekhar
 * 	Merged Joe Martin's fix for 3.1 cld. When copying user PTEs, 
 *	check for page crossing and reevaluate vtopte.
 *
 * 15-Oct-89	Robin Lewis
 *	patched the qbus map register assignment to set the High bit on vax's
 *
 * 20-Jul-89	Mark Parenti
 *	Add code for reset of KDM70.
 *
 * 15-Apr-89	Kong
 *	Fixed bug in qbasetup so that the magic cookie returned is
 *	in the format defined in the routine block comment.
 *	
 * 05-Nov-88	Robin Lewis
 *	Changed the Q22-Bus map register allocation to return registers
 *	in clicks of 8.  This is necessary because there are insuficient
 *	bits in the int to pass back the offset in the page, the starting
 *	map register and the number of map registers allocated.  Nine
 *	bit are needed to have a 512 offset value and that can not change.
 *	The remainint bits return the starting map register with a max
 *	of 8K and the number of registers which can also be 8K ..?
 *	Passing back clicks of 8 registers allows this to work but
 *	does waste registers (but only until they are released.)
 *
 * 12-11-87	Robin L. and Larry C.
 *	Added portclass support to the system.
 *
 * 15-Sep-87  -- darrell
 *	Removed unnecessary consistency checks in vs_bufctl.
 *
 * 19-May-87  -- darrell
 *	Fixed a hole in vs_bufctl that was causing a panic during the
 *	installation on VAXstation/MicroVAX 2000. 
 *	
 * 12-May-87  -- darrell
 *	Added a temporary variable to vs_bufctl to keep track of a 
 * 	pointer to the vsdev structure for the driver that is active.
 *	This fixes several panics.
 *
 * 23-Apr-87  -- darrell
 *	vs_bufctl has been changed to accpet a pointer to structure
 *	that contains the the ID of the device calling it, the action
 *	to be performed, and a pointer to the routine to call back
 *	into to driver. (stc.c or sdc.c)
 *
 * 29-Sep-86  -- darrell
 *	Space for vsbuf is now allocated here instead of in 
 *	../data/sdc_data.c.
 *
 * 26-Sep-86  -- darrell
 *	Fixed a bug in vs_bufctl that caused the vaxstar tape/disk
 *	buffer to be allocated incorrectly.
 *
 * 05-Sep-86  -- darrell
 *	Added a panic. vs_bufctl will now panic if called by the
 *	owner of the buffer.
 *
 * 30-Aug-86  -- darrell (Darrell Dunnuck)
 *	Fix bugs in VAXstar data buffer interlock code, which
 *	allows TZK50 and disk to share a common DMA data buffer.
 *
 *  5-Aug-86   -- gmm (George Mathew) and darrell (Darrell Dunnuck)
 *	Added routines to allow sharing the common disk data buffer
 *	between the VAXstar disk and TZK50 drivers.
 *
 * 13-Jun-86   -- jaw 	fix to uba reset and drivers.
 *
 * 14-May-86 -- bjg
 *	Move uba# field in subid when logging uba errors
 *
 * 16-Apr-86 -- afd
 *	Changed UMEMmap to QMEMmap and umem to qmem for QBUS reset.
 *
 * 19-feb-86 -- bjg  add uba error logging
 *
 * 04-feb-86 -- jaw  get rid of biic.h.
 *
 * 15-jul-85 -- jaw
 *	VAX8800 support
 *
 * 11 Nov 85   depp
 *	Removed System V conditional compiles.
 *
 * 08-Aug-85	darrell
 *	Zero vectors are now timed.  If we get too many, a message is 
 *	printed into the message buffer reporting the rate at which they
 *	are accuring.  The routine "ubatimer" was added.
 *
 * 11-jul-85 -- jaw
 *	fix bua/bda map registers.
 *
 * 19-Jun-85 -- jaw
 *	VAX8200 name change.
 *
 * 06 Jun 85 -- jaw
 *	add in the BDA support.
 *
 *  7 May 85 -- rjl
 *	Turned on Q-bus map as part of bus init. 
 * 
 * 22 Mar 85 -- depp
 *	Added Sys V Shared memory support
 *
 * 13-MAR-85 -jaw
 *	Changes for support of the VAX8200 were merged in.
 *
 * 27-Feb-85 -tresvik
 *	Changes for support of the VAX8600 were merged in.
 *
 * 12 Nov 84 -- rjl
 *	Added support for MicroVAX-II notion of a q-bus adapter.
 */


#include <sys/param.h>
#include <sys/systm.h>
#include <sys/map.h>
#include <sys/vmmac.h>
#include <sys/buf.h>
#include <sys/vm.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/conf.h>
#include <sys/dk.h>
#include <sys/kernel.h>
#include <sys/clist.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <dec/binlog/errlog.h>

#include <machine/cpu.h>

#ifdef __vax
#include <machine/mtpr.h>
#endif /* __vax */
 
#ifdef __mips
#include <machine/ssc.h>
#endif	/* __mips */

#include <machine/nexus.h>
#include <io/dec/uba/ubareg.h>
#include <io/dec/uba/ubavar.h>

#include <io/dec/bi/buareg.h>

#include <io/common/devdriver.h>
#include <machine/scb.h>

#define VAX_PGOFSET 0x1ff
#define VAX_NBPG 0x200
#define VAX_PGSHIFT  9

#define QIPCR 0x1f40			/* Q-bus Inter-processor csr	*/
#define LMEA 0x20			/* Local memory access enable	*/
#define QMCIA 0x4000			/* Invalidate all cached map regs*/

extern	struct device		device_list[];
extern	struct controller	controller_list[];

/*
 *  For zero vector timer -- should really be in a data.c file, but
 *  but there isn't one for uba.c
 */
int	ubatimer();
int	zvintvl = ZVINTVL;	/* zero vector timer interval in seconds */
int	zvthresh = ZVTHRESH;	/* zero vector timer threhsold for reporting */

char	ubasr_bits[] = UBASR_BITS;

#define ubacsrcheck(vubp, uhp) NULL

#ifndef __alpha
ubaconfl1(btype, binfo, bus)
	int btype;
	struct ub_info *binfo;
	struct bus *bus;
{
	if(btype != BUS_UNIBUS && btype != BUS_QBUS)
		panic("ubaconfl1: Unsupported bus type \n");
	unifind(btype, binfo, bus);
	return(1);
}
		
ubaconfl2(btype, binfo, bus)
	int btype;
	caddr_t binfo;
	struct bus *bus;
{
	return(1);
}

/*
 * Find devices on a UNIBUS.
 * Uses per-driver routine to set <br, cvec> 
 * and then fills in the tables, with help from a per-driver
 * slave initialization routine.
 */
unifind(btype, binfo, bus)
	int btype;
	struct ub_info *binfo;
	struct bus *bus;
{
	register struct controller *ctlr;
	register struct uba_hd *uhp;
	int i, (**ivec)();
	caddr_t ualloc;
	caddr_t tempio;
	int ubinfo;
	extern int stray();	/* Stray interrupts catcher for mips */
	extern nNUBA;

	/*
	 * Initialize the UNIBUS, by freeing the map
	 * registers and the buffered data path registers
	 */
	uhp = &uba_hd[numuba];
	bus->uba_hdp = ( caddr_t )uhp;

	uhp->uh_map = (struct map *) kalloc(UAMSIZ*sizeof(struct map));
	rminit(uhp->uh_map, (long)NUBMREG, (long)1, "uba", UAMSIZ);

	if(uhp->uba_type & UBABUA)
		uhp->uh_bdpfree = (1 << NBDP_BUA) - 1;

	uhp->uq_map = (struct map *) kalloc(QAMSIZ*sizeof(struct map));
	rminit(uhp->uq_map, (long)QBMREG - 1024, (long)1, "qba", QAMSIZ);

	/*
	 * Save virtual and physical addresses
	 * of adaptor, and allocate and initialize
	 * the UNIBUS interrupt vector.
	 */
	uhp->uh_uba = (struct uba_regs *) binfo->vubp;
	uhp->uh_physuba = (struct uba_regs *)binfo->pubp;

	/* set up vector if one hasn't been setup yet */
	if(numuba < nNUBA) {
		uhp->uh_vec = SCB_UNIBUS_PAGE(numuba);

		/*
		 * Set last free interrupt vector for devices with
		 * programmable interrupt vectors.  Use is to decrement
		 * this number and use result as interrupt vector.
		 */
		uhp->uh_lastiv = 0x200;

		/*
		 * Fill in the page of SCB for unibus interrupt
		 * vectors.
		 */
		for(i = 0; i < (uhp->uh_lastiv/4); i++) 
			uhp->uh_vec[i] = scbentry(stray, 0);
	}

	/*
	 * Map the adapter memory and the i/o space. For unibuses the io space
	 * is the last 8k of the adapter memory. On q-bus it's totally 
	 * disjoint.
	 * We map the i/o space right after the adapter memory space so that 
	 * its easy to compute the virtual addresses.
	 */
	/* BDA does not have dev space! */
/*
 * NOTE: On a BDA, you must allocate and map the fake virtual address space
 */
	if((uhp->uba_type & UBABDA) == 0) {
		kern_return_t ret;
		vm_offset_t kva;
		extern unsigned int qmem;

		/* allocate some kernel virt address space */
		if((kva = vm_alloc_kva(binfo->umemsize+DEVSPACESIZE)) == NULL) 
			panic("unifind: no space to map adapter memory");

		/* now map the phys to the virt */
		if(pmap_map_io(binfo->pumem, binfo->umemsize, kva, 
			       (VM_PROT_READ|VM_PROT_WRITE), TB_SYNC_ALL) !=
		   KERN_SUCCESS)
			panic("unifind: cannot map adapter memory");
		
		if(pmap_map_io(binfo->pdevaddr, DEVSPACESIZE, 
			       (kva + binfo->umemsize), 
			       (VM_PROT_READ|VM_PROT_WRITE), TB_SYNC_ALL) !=
		   KERN_SUCCESS)
			panic("unifind: cannot map adapter I/O");

		qmem = binfo->vumem = kva;
	}

	/* clear uba csr */
	if(binfo->haveubasr) 
		ubacsrcheck(binfo->vubp, uhp);

	/*
	 * Grab some memory to record the umem address space we allocate,
	 * so we can be sure not to place two devices at the same address.
	 *
	 * We could use just 1/8 of this (we only want a 1 bit flag) but
	 * we are going to give it back anyway, and that would make the
	 * code here bigger (which we can't give back), so ...
	 *
	 * One day, someone will make a unibus with something other than
	 * an 8K i/o address space, & screw this totally.
	 * When that happens we should add a new field to cpusw for it.
	 */
	if((ualloc = (caddr_t) kalloc(DEVSPACESIZE)) == (caddr_t)0)
		panic("unifind: no mem for unifind");

	/*
	 * Map the first page of BUS i/o space to the first page of memory
	 * for devices which will need to dma output to produce an interrupt.
	 */
	if((tempio = (caddr_t) kalloc(1024)) == (caddr_t)0 )
		panic("unifind: no mem for probe i/o");

	if((ubinfo = uballoc(numuba, tempio, 1024, 0)) & 0x3ffff ) 
		panic("unifind: probe i/o space not at bus virtual address 0");

#define	ubaoff(off)	((off) & 0x1fff)
#define	ubaddr(off)	(u_short *)((int)binfo->vumem + binfo->umemsize +(ubaoff(off)))

	/*
	 * Check each unibus mass storage controller.
	 * For each one which is potentially on this uba,
	 * see if it is really there, and if it is record it and
	 * then go looking for slaves.
	 */
	for(ctlr = controller_list; ctlr->ctlr_name; ctlr++) {
		register struct driver *drp;
		u_long *ap;
		u_short addr;

		if(((ctlr->bus_num != bus->bus_num) &&
		    (ctlr->bus_num != -1) && (ctlr->bus_num != -99)) ||
		   (strcmp(ctlr->bus_name, bus->bus_name) ) ||
		   (ctlr->alive & ALV_ALIVE) ) {
			continue;
		}

		addr = (u_short)ctlr->addr;
		if(uhp->uba_type & UBABDA && addr > 0x0ff) 
			continue;		
		
		drp = ctlr->driver;
		/*
		 * use the particular address specified first,
		 * or if it is given as "0", of there is no device
		 * at that address, try all the standard addresses
		 * in the driver till we find it
		 */
		for(ap = ( u_long *)drp->addr_list;
			addr || (addr = (u_short)*ap++); addr = 0 ) {
			register struct device *dev;
			u_short *reg;

			if((uhp->uba_type & (UBABDA|UBAXMI)) && (addr > 0x0ff))
				continue;		
			
			/* clear uba csr */
			if(binfo->haveubasr) 
				ubacsrcheck(binfo->vubp, uhp);
			
			if(ualloc[ubaoff(addr)])
				continue;
			reg = ubaddr(addr);
			if(BADADDR((caddr_t)reg, 2, bus)) 
				continue;
			
			if(binfo->haveubasr && ubacsrcheck(binfo->vubp, uhp))
					continue;

			cvec = 0x200;
			i = (*drp->probe)(reg, ctlr->ctlr_num);
			if(binfo->haveubasr && ubacsrcheck(binfo->vubp, uhp)) 
					continue;
			if(i == 0) 
				continue;

			ctlr->bus_num = numuba;
			config_fillin(ctlr);
			printf(" csr %o ",addr);
			
			if(cvec == 0) {
				printf("zero vector\n");
				continue;
			}
			if(cvec == 0x200) {
				printf("didn't interrupt\n");
				continue;
			}
			while(--i >= 0)
				ualloc[ubaoff(addr+i)] = 1;
			printf("vec %o\n", cvec);
			ctlr->alive |= ALV_ALIVE;
			ctlr->addr = (caddr_t)reg;
			if(svatophys(ctlr->addr,&ctlr->physaddr)!=KERN_SUCCESS)
				panic("unifind: svatophys");

			drp->ctlr_list[ctlr->ctlr_num] = ctlr;
			conn_ctlr(bus, ctlr);

			for(ivec = ctlr->intr; *ivec; ivec++) {
				uhp->uh_vec[cvec/4]=scbentry(*ivec,SCB_ISTACK);
				cvec += 4;
			}

			/* call the controller attach rtn now */
			if(drp->cattach)
				(*drp->cattach)(ctlr);

			for(dev = device_list; dev->dev_name; dev++) {
				int savectlr;
				char *savectname;

				if(((dev->ctlr_num != ctlr->ctlr_num) &&
				    (dev->ctlr_num !=-1)) ||
				   ((strcmp(dev->ctlr_name, ctlr->ctlr_name))&&
				    (strcmp(dev->ctlr_name, "*"))) ||
				   (dev->alive & ALV_ALIVE) ) 
					continue;

				savectlr = dev->ctlr_num;
				savectname = dev->ctlr_name;
				dev->ctlr_num = ctlr->ctlr_num;
				dev->ctlr_name = ctlr->ctlr_name;

				if((*drp->slave)(dev, reg)) {
					dev->alive |= ALV_ALIVE;
					conn_device(ctlr, dev);
					drp->dev_list[dev->logunit] = dev;
					perf_init( dev );
					/* dev_type comes from driver */
					printf("%s%d at %s%d slave %d",
					       dev->dev_name, dev->logunit,
					       drp->ctlr_name, ctlr->ctlr_num, 
					       dev->unit);
					if(drp->dattach)
						(*drp->dattach)(dev);
					printf("\n");
				} else {
					dev->ctlr_num = savectlr;
					dev->ctlr_name = savectname;
				}
			}
			break;
		}
	}

#ifdef	AUTO_DEBUG
	printf("Unibus allocation map");
	for(i = 0; i < 8*1024; ) {
		register n, m;

		if((i % 128) == 0) {
			printf("\n%6o:", i);
			for(n = 0; n < 128; n++)
				if(ualloc[i+n])
					break;
			if(n == 128) {
				i += 128;
				continue;
			}
		}

		for(n = m = 0; n < 16; n++) {
			m <<= 1;
			m |= ualloc[i++];
		}

		printf(" %4x", m);
	}
	printf("\n");
#endif /* AUTO_DEBUG */

	/*
	 * Free resources.  We free the bus map register but it's unlikely
	 * that it will ever be used again due to the fact that it only 
	 * maps two pages.
	 */
	kfree(ualloc, DEVSPACESIZE);
	kfree(tempio, 1024);
	ubarelse(numuba, &ubinfo);
}
#endif /* !__alpha */

/*
 * Do transfer on device argument.  The controller
 * and uba involved are implied by the device.
 * We queue for resource wait in the uba code if necessary.
 * We return 1 if the transfer was started, 0 if it was not.
 * If you call this routine with the head of the queue for a
 * UBA, it will automatically remove the device from the UBA
 * queue before it returns.  If some other device is given
 * as argument, it will be added to the request queue if the
 * request cannot be started immediately.  This means that
 * passing a device which is on the queue but not at the head
 * of the request queue is likely to be a disaster.
 */
ubago(devp)
	register struct device *devp;
{
	register struct controller *ctlrp = devp->ctlr_hd;
	register struct uba_hd *uh;
	register struct buf *bp;
	register int s, unit, uba_hd_index;

	if(ctlrp->bus_hd->bus_type == BUS_XMI) 
		uba_hd_index = ctlrp->ctlr_num;
	else
		uba_hd_index = ctlrp->bus_num;
	uh = &uba_hd[uba_hd_index];

	s = splbio();
	if(ctlrp->driver->xclu && uh->uh_users > 0 || uh->uh_xclu)
		goto rwait;
	bp = ( struct buf * )ctlrp->um_tab;
	ctlrp->um_ubinfo = (void *)ubasetup(ctlrp->bus_num, bp->b_actf->b_actf,
					 UBA_NEEDBDP | UBA_CANTWAIT);
	if((int)ctlrp->um_ubinfo == 0)
		goto rwait;
	uh->uh_users++;
	if(ctlrp->driver->xclu)
		uh->uh_xclu = 1;
	splx(s);
	if((short)devp->perf >= 0) {
		unit = (short)devp->perf;
		dk_busy |= 1 << unit;
		dk_xfer[unit]++;
		dk_wds[unit] += bp->b_actf->b_actf->b_bcount>>6;
	}
	if(uh->uh_actf == devp)
		uh->uh_actf = devp->nxt_dev;
	(*ctlrp->driver->go)(ctlrp);
	return (1);
rwait:
	if(uh->uh_actf != devp) {
		devp->nxt_dev = NULL;
		if(uh->uh_actf == NULL)
			uh->uh_actf = devp;
		else
			uh->uh_actl->nxt_dev = devp;
		uh->uh_actl = devp;
	}
	splx(s);
	return (0);
}

ubadone(ctlrp)
	register struct controller *ctlrp;
{
	register int uba_hd_index;
	register struct uba_hd *uh;

	if(ctlrp->bus_hd->bus_type == BUS_XMI) 
		uba_hd_index = ctlrp->ctlr_num;
	else
		uba_hd_index = ctlrp->bus_num;
	uh = &uba_hd[uba_hd_index];

	if(ctlrp->driver->xclu)
		uh->uh_xclu = 0;
	uh->uh_users--;
	ubarelse(uba_hd_index, &ctlrp->um_ubinfo);
}

/*
 * Allocate and setup UBA map registers, and bdp's
 * Flags says whether bdp is needed, whether the caller can't
 * wait (e.g. if the caller is at interrupt level).
 *
 * Return value:
 *	Bits 0-8	Byte offset
 *	Bits 9-17	Start map reg. no.
 *	Bits 18-27	No. mapping reg's
 *	Bits 28-31	BDP no.
 */

ubasetup(uban, bp, flags)
	int uban;
	struct buf *bp;
	int flags;
{
	register struct uba_hd *uh = &uba_hd[uban];
	register unsigned long v, ev, ep, ep_start;
#ifdef __alpha
	register int *io;
#else
	register struct pt_entry *io;
#endif /* __alpha */
	int bdp, a, ubinfo;
	long reg;
	int vax_o, vax_npf, o;
	vm_offset_t pfn;

	if(uh->uba_type & UBAUVII)
		flags &= ~UBA_NEEDBDP;

	/* 
	 * UNIBUS mapping registers map the natural machine pages to 
	 * VAX pages and back DO NOT replace VAX_PGOFSET with PGOFFSET 
	 */
	vax_o = (u_int)bp->b_un.b_addr & VAX_PGOFSET;
	/* 
	 * Number of VAX pages spanned by the buffer + 1 (hence number of
	 * map registers needed since we need one guard page)
	 */
	vax_npf = (((bp->b_bcount + vax_o) + VAX_PGOFSET) >> VAX_PGSHIFT) + 1; 

	a = splbio();
	while((reg = rmalloc(uh->uh_map, (long)vax_npf)) == 0) {
		if(flags & UBA_CANTWAIT) {
			splx(a);
			return (0);
		}
		uh->uh_mrwant++;
		sleep((caddr_t)&uh->uh_mrwant, PSWP);
	}
	bdp = 0;

	if(flags & UBA_NEEDBDP) {
		while((bdp = ffs(uh->uh_bdpfree)) == 0) {
			if(flags & UBA_CANTWAIT) {
				rmfree(uh->uh_map, (long)vax_npf, (long)reg);
				splx(a);
				return(0);
			}
			uh->uh_bdpwant++;
			sleep((caddr_t)&uh->uh_bdpwant, PSWP);
		}
		uh->uh_bdpfree &= ~(1 << (bdp - 1));
	} else if(flags & UBA_HAVEBDP)
		bdp = (flags >> 28) & 0xf;
	splx(a);
	reg--;
	ubinfo = (bdp << 28) | (vax_npf << 18) | (reg << 9) | vax_o;
	/* get address of starting UNIBUS map register */
	io = &uh->uh_uba->uba_map[reg];

	/*
	 * outer loop is page control
	 * inner loop handles mapping between natural machine pages 
	 * and the VAX pages 
	 */
	ev = (v = (u_long)bp->b_un.b_addr) + bp->b_bcount;
	while(v < ev) {
		/* this handles setup and crossing page boundaries */
#ifdef __alpha
		if(!IS_SYS_VA(v)) 
#else
		if(IS_KUSEG(v)) 
#endif /* __alpha */
			pfn = pmap_extract(bp->b_proc->task->map->vm_pmap, v);
		 else if(svatophys(v, &pfn) != KERN_SUCCESS) 
			panic("ubasetup: bad virtual address");
		if(pfn == 0)
			panic("ubasetup: zero pfn");
		pfn = (btop(pfn) << (PGSHIFT - VAX_PGSHIFT));
		o = v & PGOFSET;
#ifdef __alpha
		/* make sure this happens with 64-bit LONGS!!! */
		ep_start = (v + NBPG) & ~((long)(NBPG - 1));
		if (ep_start > ev)
		    ep_start = ev;
#else
 		ep_start = min((v + NBPG) & ~((long)(NBPG - 1)), ev);
#endif /* __alpha */
		for(ep = ep_start; v < ep; 
		    v += (VAX_NBPG - (o % VAX_NBPG)), 
		    o += (VAX_NBPG - (o % VAX_NBPG))) {
			*(int *)io++ = (pfn + (o >> VAX_PGSHIFT)) | UBAMR_MRV;
		}
	}
	/* Set up fire wall */
	*(int *)io++ = 0;	
	WBFLUSH();
	return (ubinfo);
}

/*
 * Non buffer setup interface... set up a buffer and call ubasetup.
 */
qballoc(uban, addr, bcnt, flags)
	int uban;
	caddr_t addr;
	int bcnt, flags;
{
	struct buf qbbuf;

	qbbuf.b_un.b_addr = addr;
	qbbuf.b_flags = B_BUSY;
	qbbuf.b_bcount = bcnt;
	/* that's all the fields qbasetup() needs */
	return (qbasetup(uban, &qbbuf, flags));
}
 
/*
 * Non buffer setup interface... set up a buffer and call ubasetup.
 */
uballoc(uban, addr, bcnt, flags)
	int uban;
	caddr_t addr;
	int bcnt, flags;
{
	struct buf ubabuf;

	ubabuf.b_un.b_addr = addr;
	ubabuf.b_flags = B_BUSY;
	ubabuf.b_bcount = bcnt;
	/* that's all the fields ubasetup() needs */
	return (ubasetup(uban, &ubabuf, flags));
}
 
/*
 * Release resources on uba uban, and then unblock resource waiters.
 * The map register parameter is by value since we need to block
 * against uba resets on 11/780's.
 */
ubarelse(uban, amr)
	int uban;
	int *amr;
{
	register struct uba_hd *uh = &uba_hd[uban];
	register int bdp, reg, npf, s;
	int mr;
 
	/*
	 * Carefully see if we should release the space, since
	 * it may be released asynchronously at uba reset time.
	 */
	s = splbio();
	mr = *amr;
	if(mr == 0) {
		/*
		 * A ubareset() occurred before we got around
		 * to releasing the space... no need to bother.
		 */
		splx(s);
		return;
	}
	*amr = 0;
	splx(s);		/* let interrupts in, we're safe for a while */
	bdp = (mr >> 28) & 0x0f;

	if(bdp) {
		if(uh->uba_type & UBABUA){
			bdp = bdp & 0x07;
			((struct bua_regs *)uh->uh_uba)->bua_dpr[bdp] |= 
				BUADPR_PURGE;
		}

		uh->uh_bdpfree |= 1 << (bdp-1);		/* atomic */
		if(uh->uh_bdpwant) {
			uh->uh_bdpwant = 0;
			wakeup((caddr_t)&uh->uh_bdpwant);
		}
	}
	/*
	 * Put back the registers in the resource map.
	 * The map code must not be reentered, so we do this
	 * at high ipl.
	 */
	npf = (mr >> 18) & 0x3ff;
	reg = ((mr >> 9) & 0x1ff) + 1;
	s = splbio();
	rmfree(uh->uh_map, (long)npf, (long)reg);
	splx(s);

	/*
	 * Wakeup sleepers for map registers,
	 * and also, if there are processes blocked in dgo(),
	 * give them a chance at the UNIBUS.
	 */
	if(uh->uh_mrwant) {
		uh->uh_mrwant = 0;
		wakeup((caddr_t)&uh->uh_mrwant);
	}
	while(uh->uh_actf && ubago(uh->uh_actf)) { ; }
}

ubapurge(ctlrp)
	register struct controller *ctlrp;
{
	register int bdp = ((int)ctlrp->um_ubinfo >> 28) & 0x0f;
	register struct uba_hd *uh;

	if(ctlrp->bus_hd->bus_type == BUS_XMI) 
		uh = &uba_hd[ctlrp->ctlr_num];
	else
		uh = &uba_hd[ctlrp->bus_num];

	if(uh->uba_type & UBABUA) {
		bdp &= 0x07;
		((struct bua_regs *)uh->uh_uba)->bua_dpr[bdp] |= BUADPR_PURGE;
	}
}

/*
 * Generate a reset on uba number uban.  Then
 * call each device in the character device table,
 * giving it a chance to clean up so as to be able to continue.
 */
ubareset(uban)
	int uban;
{
	register struct cdevsw *cdp;
	register struct uba_hd *uh = &uba_hd[uban];
	int s;

	s = splbio();
	uh->uh_users = 0;
	uh->uh_zvcnt = 0;
	uh->uh_xclu = 0;
	uh->uh_actf = uh->uh_actl = 0;
	uh->uh_bdpwant = 0;
	uh->uh_mrwant = 0;

	rminit(uh->uh_map, (long)NUBMREG, (long)1, "uba", UAMSIZ);
	if(uh->uba_type & UBABUA)
		uh->uh_bdpfree = (1 << NBDP_BUA) - 1;

	wakeup((caddr_t)&uh->uh_bdpwant);
	wakeup((caddr_t)&uh->uh_mrwant);

	if(uh->uba_type & UBABDA) 
		printf("bda%d: reset",uban);
	else if(uh->uba_type & UBAXMI) 
		printf("kdm%d: reset",uban);
	else {
		printf("uba%d: reset", uban);
		ubainit(uh->uh_uba, uh->uba_type);
	}

	/* reallocate global unibus space for tty drivers */
/* commented out tty_ubinfo to eliminate unresolved reference */
#ifdef notdef
	if(tty_ubinfo[uban] != 0)
		tty_ubinfo[uban] = uballoc(uban, (caddr_t)cfree,
	    		nclist*sizeof (struct cblock), 0);
#endif /* notdef */

	for(cdp = cdevsw; cdp < cdevsw + nchrdev; cdp++)
		(*cdp->d_reset)(uban);
#ifdef INET
	ifubareset(uban);
#endif /* INET */
	printf("\n");
	splx(s);
}

/*
 * Init a uba.  This is called with a pointer
 * rather than a virtual address since it is called
 * by code which runs with memory mapping disabled.
 * In these cases we really don't need the interrupts
 * enabled, but since we run with ipl high, we don't care
 * if they are, they will never happen anyways.
 */
ubainit(uba, ubatype)
	register int ubatype;
	register struct uba_regs *uba;

{
#ifndef __alpha
/* not sure I like this heavy-handedness, but... */
	extern qmem;
	extern struct ssc_regs *ssc_ptr;

#ifdef notdef
/* comented out buainit call to remove unresolved reference */

	if(ubatype & UBABUA) 
		buainit(uba);
	else
#endif /* notdef */
	if(ubatype & UBAUVII) {
#define LMEA 0x20			/* Local memory access enable	*/
#define QIPCR 0x1f40			/* Q-bus Inter-processor csr	*/
		/*
		 * Reset the bus and wait for the devices to
		 * settle down
		 */
		ssc_ptr->ssc_ioreset = 0;  /* equiv to mtpr(IUR, 0) */

		DELAY(500000);
		/*
		 * The bus reset turns off the q-bus map (unfortunately)
		 * The problem is further agravated by the fact that the
		 * enable bit is in the IPC register which is in I/O space
		 * instead of local register space.  Because of this we
		 * have to figure out if we're virtual or physical.
		 */

		/*
		 * We are on a Qbus mips machine.  The bus reset we just
		 * did turned off the Qbus map.  We need to allow
		 * external access to Qbus memory space via the Qbus map.
		 * Since on a mips we always have memory management,
		 * we access the IPCR (implemented in the CQBIC chip) 
		 * through Kseg 1 space
		 *
		 * IPCR is a 16-bit register located in offset
		 * 0x1f40 from the Q bus I/O space.  
		 */
		*(u_short *)((char *)qmem+QMEMSIZEUVI+QIPCR) = LMEA;
	}
#endif /* !__alpha */
}


int	ubawedgecnt = 10;
int	ubacrazy = 500;
/*
 * This routine is called by the locore code to
 * process a UBA error on an 11/780.  The arguments are passed
 * on the stack, and value-result (through some trickery).
 * In particular, the uvec argument is used for further
 * uba processing so the result aspect of it is very important.
 * It must not be declared register.
 */
/*ARGSUSED*/
ubaerror(uban, uh, xx, uvec, uba, ubapc)
	register int uban;
	register struct uba_hd *uh;
	int uvec;
	register struct uba_regs *uba;
	int *ubapc;
{
	register sr, s;
 	struct el_rec *elrp;

	/*
	 *	Start a timer to time the rate of zero vectors.
	 *	The counting is done in locore.s.
	 */

	if(uvec == 0) {
		if(uh->uh_zvflg)
			printf("ubaerror: zero vector flag shouldn't be set\n");
		else {
			uh->uh_zvcnt++;
			uh->uh_zvflg++;
			timeout(ubatimer, uban, hz * zvintvl);
		}
		return;
	}
	if(uh->uba_type & UBABUA) {
		sr = ((struct bua_regs *)uh->uh_uba)->bua_ctrl;
		s = spl7();
		printf("bua%d: bua error ctrl=%x", uban, sr); 
		splx(s);
		((struct bua_regs *)uh->uh_uba)->bua_ctrl = sr;
		ubareset(uban);
	} else if((uh->uba_type & UBABDA) == 0) {	

		if(uba->uba_cnfgr & NEX_CFGFLT) {
#if	UERF
			elrp = ealloc(EL_UBASIZE,EL_PRILOW);
			if(elrp != NULL) {
			    LSUBID(elrp,ELCT_ADPTR,ELADP_UBA,EL_UNDEF,EL_UNDEF,uban,EL_UNDEF);
		 	    elrp->el_body.eluba780.uba_cf = uba->uba_cnfgr;
		 	    elrp->el_body.eluba780.uba_cr = uba->uba_cr;
			    elrp->el_body.eluba780.uba_sr = uba->uba_sr;
		 	    elrp->el_body.eluba780.uba_dcr = uba->uba_dcr;
		 	    elrp->el_body.eluba780.uba_fmer = uba->uba_fmer;
			    elrp->el_body.eluba780.uba_fubar = uba->uba_fubar;
		 	    elrp->el_body.eluba780.uba_pc = *ubapc;
		 	    elrp->el_body.eluba780.uba_psl = *++ubapc;
			    EVALID(elrp);
			} else {
#endif /* UERF */
			    printf("uba%d: sbi fault sr=%b cnfgr=%b\n",
				    uban, uba->uba_sr, ubasr_bits,
				    uba->uba_cnfgr, NEXFLT_BITS);
#if	UERF
			}
#endif /* UERF */
			ubareset(uban);
			uvec = 0;
			return;
		}
		sr = uba->uba_sr;
		s = spl7();
#if	UERF
		elrp = ealloc(EL_UBASIZE,EL_PRILOW);
		if(elrp != NULL) {
		    LSUBID(elrp,ELCT_ADPTR,ELADP_UBA,EL_UNDEF,EL_UNDEF,uban,EL_UNDEF);
	 	    elrp->el_body.eluba780.uba_cf = uba->uba_cnfgr;
	 	    elrp->el_body.eluba780.uba_cr = uba->uba_cr;
		    elrp->el_body.eluba780.uba_sr = uba->uba_sr;
	 	    elrp->el_body.eluba780.uba_dcr = uba->uba_dcr;
	 	    elrp->el_body.eluba780.uba_fmer = uba->uba_fmer;
		    elrp->el_body.eluba780.uba_fubar = uba->uba_fubar;
	 	    elrp->el_body.eluba780.uba_pc = *ubapc;
	 	    elrp->el_body.eluba780.uba_psl = *++ubapc;
		    EVALID(elrp);
		} else {
#endif /* UERF */
		    printf("uba%d: uba error sr=%b fmer=%x fubar=%o\n",
			     uban, uba->uba_sr, ubasr_bits, 
			     uba->uba_fmer, 4*uba->uba_fubar);
#if	UERF
		}
#endif / * UERF */
		splx(s);
		uba->uba_sr = sr;
		uvec &= UBABRRVR_DIV;
	}
	if(++uh->uh_errcnt % ubawedgecnt == 0) {
		if(uh->uh_errcnt > ubacrazy)
			panic("ubaerror: uba crazy");
		printf("ERROR LIMIT ");
		ubareset(uban);
		uvec = 0;
		return;
	}
	return;
}

/*
 * Allocate UNIBUS memory.  Allocates and initializes
 * sufficient mapping registers for access.  On a 780,
 * the configuration register is setup to disable UBA
 * response on DMA transfers to addresses controlled
 * by the disabled mapping registers.
 */
ubamem(uban, addr, npg, doalloc)
	int uban, addr, npg, doalloc;
{
	register struct uba_hd *uh = &uba_hd[uban];
	register int a;

	if(doalloc) {
		int s = splbio();
		a = rmget(uh->uh_map, npg, (addr >> 9) + 1);
		splx(s);
	} else
		a = (addr >> 9) + 1;
	if(a) {
		register int i, *m;

		m = (int *)&uh->uh_uba->uba_map[a - 1];

		for(i = 0; i < npg; i++)
			*m++ = 0;	/* All off, especially 'valid' */
	}
	return (a);
}

/*
 *  Check the number of zero vectors and report if we get too many of them.
 *  Always reset the zero vector count and the zero vector timer flag.
 */

ubatimer(uban)
	int	uban;
{
	struct uba_hd *uh;

	uh = &uba_hd[uban];
	if(uh->uh_zvcnt > zvthresh)
		printf("ubatimer: uba%d -- %d zero vectors in %d minutes\n",
			uban, uh->uh_zvcnt, zvintvl/60);
	uh->uh_zvcnt = 0;
	uh->uh_zvflg = 0;
}

/*
 * Allocate and setup Q-BUS map registers, and bdp's
 * Flags says whether bdp is needed, whether the caller can't
 * wait (e.g. if the caller is at interrupt level).
 *
 * Return value:
 *	Bits 0-8	Byte offset 		512  number of bytes
 *	Bits 9-21	Start map reg. no.	all the 8192 regs
 *	Bits 22-31	No. mapping reg's allocated, divided by 8
 */
qbasetup(uban, bp, flags)
	int uban;
	struct buf *bp;
	int flags;
{
	register struct uba_hd *uh = &uba_hd[uban];
#ifdef __alpha
	register int *io;
#else
	register struct pt_entry *io;
#endif /* __alpha */
	int vax_npf, fake_npf;
	long reg;
	int a, vax_o, o, ubinfo;
	unsigned long v, ev, ep, ep_start;
	static int first_time = 0;
	vm_offset_t pfn;

	flags &= ~UBA_NEEDBDP;

	/* 
	 * The first time through to get map registers allocate 512 of them
	 * and never use them or give them back.  This will allow any calls
	 * from a users driver to uballoc to get these without conflicts with 
	 * the ones this routine controls.
	 * This is done for backward compatability.
	 */
	if(first_time == 0) {
		first_time = 1;
		rmalloc(uh->uq_map, (long)(btoc(QBNOTUB * NBPG) + 1));
	}
	/* 
	 * Find the page, offset into the page and the number of vax
	 * page frames that will be needed.  This code will work for mips
	 * also because we will simulate vax pfn's when infact one mips
	 * pfn is equal to eight (8) vax pfn's.
	 */
	vax_o = (u_int)bp->b_un.b_addr & VAX_PGOFSET;
	/* 
	 * Number of VAX pages spanned by the buffer + 1 (hence number of
	 * map registers needed since we need one guard page)
	 */
	vax_npf = (((bp->b_bcount + vax_o) + VAX_PGOFSET) >> VAX_PGSHIFT) + 1; 
	/* 
	 * get regs in groups of 8 to allow the ubinfo word to
	 * hold all the information.  This may be a waste of maps but
	 * it gets us closer to using all 8K maps on a Q22 bus, we need
	 * to do this because we can't get all the information into
	 * the returned integer. Sigh!
	 *
	 * Also MIPS uses 1 map where VAX looks for 8 so if its on
	 * a MIPS system fake_npf is really the vax maps needed.
	 */
	fake_npf = (vax_npf + ((NBPG/VAX_NBPG) - (vax_npf % (NBPG/VAX_NBPG))));
	/* Allocate the map registers for use by calling rmalloc */
	a = splbio();
	while((reg = rmalloc(uh->uq_map, (long)fake_npf)) == 0) {
		if(flags & UBA_CANTWAIT) {
			splx(a);
			return (0);
		}
		uh->uh_mrwant++;
		sleep((caddr_t)&uh->uh_mrwant, PSWP);
	}
	splx(a);

	/*
	 * Setup the return value which holds the map register number
	 * and related info.
	 */
	reg--;
	ubinfo = ((reg & 0x1fff) << 9) | vax_o;
	ubinfo |= ((fake_npf >> 3) << 22);
	/* get address of starting UNIBUS map register */
	io = &uh->uh_uba->uba_map[reg];

	/*
	 * outer loop is page control
	 * inner loop handles mapping between natural machine pages 
	 * and the VAX pages 
	 */
	ev = (v = (u_long)bp->b_un.b_addr) + bp->b_bcount;
	while(v < ev) {
#ifdef __alpha
		if(!IS_SYS_VA(v)) 
#else
		if(IS_KUSEG(v)) 
#endif /* __alpha */
			pfn = pmap_extract(bp->b_proc->task->map->vm_pmap, v);
		 else if(svatophys(v, &pfn) != KERN_SUCCESS) 
			panic("qbasetup: bad virtual address");
		if(pfn == 0)
			panic("qbasetup: zero pfn");
		pfn = (btop(pfn) << (PGSHIFT - VAX_PGSHIFT));
		o = v & PGOFSET;
#ifdef __alpha
		/* make sure this happens with 64-bit LONGS!!! */
		ep_start = (v + NBPG) & ~((long)(NBPG - 1));
		if (ep_start > ev)
		    ep_start = ev;
#else
		ep_start = min((v + NBPG) & ~(NBPG - 1), ev);
#endif /* __alpha */
		for(ep = ep_start; v < ep; 
		    v += (VAX_NBPG - (o % VAX_NBPG)), 
		    o += (VAX_NBPG - (o % VAX_NBPG))) {
			*(int *)io++ = (pfn + (o >> VAX_PGSHIFT)) | UBAMR_MRV;
		}
	}
	/* Set up fire wall */
	*(int *)io++ = 0;
	WBFLUSH();
	return(ubinfo);
}

 
/*
 * Release resources on uba uban, and then unblock resource waiters.
 * The map register parameter is by value since we need to block
 * against uba resets on 11/780's.
 */
qbarelse(uban, amr)
	unsigned *amr;
{
	register struct uba_hd *uh = &uba_hd[uban];
	register int  reg, npf, s;
	unsigned mr;
 
	/*
	 * Carefully see if we should release the space, since
	 * it may be released asynchronously at uba reset time.
	 */
	s = splbio();
	mr = *amr;
	if(mr == 0) {
		/*
		 * A ubareset() occurred before we got around
		 * to releasing the space... no need to bother.
		 */
		splx(s);
		return;
	}
	*amr = 0;
	splx(s);		/* let interrupts in, we're safe for a while */

	/*
	 * Put back the registers in the resource map.
	 * The map code must not be reentered, so we do this
	 * at high ipl.
	 */
	npf = (mr >> 22) * 8;
	reg = (((mr >> 9) & QBREGMASK) + 1);
	s = splbio();
	rmfree(uh->uq_map, (long)npf , (long)reg);
	splx(s);

	/*
	 * Wakeup sleepers for map registers,
	 * and also, if there are processes blocked in dgo(),
	 * give them a chance at the UNIBUS.
	 */
	if(uh->uh_mrwant) {
		uh->uh_mrwant = 0;
		wakeup((caddr_t)&uh->uh_mrwant);
	}
	while(uh->uh_actf && ubago(uh->uh_actf))
		;
}
