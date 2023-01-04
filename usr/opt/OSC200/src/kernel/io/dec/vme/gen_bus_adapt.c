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
static char *rcsid = "@(#)$RCSfile: gen_bus_adapt.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/07/14 18:50:47 $";
#endif

/*
 * Abstract:
 *	This module contains routines to support the general bus adapter interface.
 *
 * Revision History
 *
 *	24-Jan-92	stuarth -- Stuart Hollander
 *		rename some functions
 *
 *	07-Jan-92	stuarth -- Stuart Hollander
 *		Original version
 *************************************************************************/
#include <sys/buf.h>
#include <sys/proc.h>
#include <io/dec/vme/gen_bus_adapt.h>

extern	struct device device_list[];
extern	struct controller controller_list[];

#ifdef ORIGINALCODE
extern	int	splm[];
#endif

#ifdef DEBUG
/* this is in xvia_data.c */
extern int gen_ba_debug;
struct gen_bus_adapt *debug_vhp;
#define Cprintf if(gen_ba_debug)printf
#define Dprintf if(gen_ba_debug >= 2 )printf
#else
#define Cprintf ;
#define Dprintf ;
#endif

#ifdef __alpha
void vme_intrsetvec();
#endif /* __alpha */

int
gen_ba_ins(head_p, vhp)
	struct gen_bus_adapt	**head_p, *vhp;
{
	struct	gen_bus_adapt	*vhp_ptr;

	Cprintf("gen_ba_ins: entry head_p=%x, vhp=%x\n", head_p, vhp);
	vhp_ptr = *head_p;

	while(vhp_ptr) {
		if(vhp_ptr->next == (struct gen_bus_adapt *)0)
			break;
		vhp_ptr = vhp_ptr->next;
	}
        if(vhp_ptr) {
		vhp_ptr->next = vhp;
	}
        else {
		*head_p = vhp;
	}
	vhp->next = (struct gen_bus_adapt *)0;
	Cprintf("gen_ba_ins: exit head_p=%x, vhp=%x\n", head_p, vhp);
}

struct gen_bus_adapt *
gen_ba_get(head_p, bus_adapt_number) 
	struct gen_bus_adapt	**head_p;
	int bus_adapt_number;
{
	register struct gen_bus_adapt *vhp;

	Cprintf("gen_ba_get: entry head_p=%x, bus_adapt_number=%d\n", head_p, bus_adapt_number);
	vhp = *head_p;

	while(vhp) {
		if(vhp->xbanum == bus_adapt_number) {
			Cprintf("gen_ba_get: exit vhp=%x\n", vhp);
			return(vhp);
		}
		vhp = vhp->next;
	}
	panic("gen_ba_get: no gen_bus_adapt");
	/*NOTREACHED*/
}

caddr_t
gen_ba_map_csr(vhp, addr, size, atype, print_flag) 
	struct	gen_bus_adapt		*vhp;
	u_long		addr;
	int		size;
	int		atype;
	int		print_flag;
{
	Cprintf("gen_ba_map_csr\n");
	if(vhp->map_csr)
		return((*vhp->map_csr)(vhp, addr, size, atype, print_flag));
	else
		panic("gen_ba_map_csr: no function");
}

void
gen_ba_unmap_csr(vhp, addr, size, atype)
	struct gen_bus_adapt *vhp;
	caddr_t addr; /* the virtual addr (from map_csr), not the vme addr  */
	int	size;
	int	atype;
{
	Cprintf("gen_ba_unmap_csr\n");
	if(vhp->unmap_csr)
		(*vhp->unmap_csr)(vhp, addr, size, atype);
	else
		panic("gen_ba_unmap_csr: no function");
}

unsigned int
gen_ba_setup(ctlr, bp, flags, addr)
	struct controller *ctlr;
	struct buf *bp;
	long flags;
	u_long addr;
{
	struct gen_bus_adapt *vhp = (struct gen_bus_adapt *)ctlr->bus_hd->xbahd;

	Cprintf("gen_ba_setup: flags 0x%lx addr 0x%lx\n", flags, addr);
	if( vhp->setup )
		return((*vhp->setup)(ctlr, bp, flags, addr));
	else
		panic("gen_ba_setup: No setup function");
}

/*
 * Non buffer setup interface... set up a buffer and call gen_ba_setup.
 */
unsigned int
gen_ba_alloc(ctlr, addr, bcnt, flags, busaddr)
	struct controller *ctlr;
	caddr_t addr;
	int bcnt;
	long flags;
	u_long busaddr;
{
	struct buf xbabuf;

	Cprintf("gen_ba_alloc\n");
	xbabuf.b_un.b_addr = addr;
	xbabuf.b_flags = B_BUSY;
	xbabuf.b_bcount = bcnt;
	xbabuf.b_proc = 0;
	/* that's all the fields gen_ba_setup() needs */
	return(gen_ba_setup(ctlr, &xbabuf, flags, busaddr));
}
 
void
gen_ba_release(ctlr, mr)
	struct controller *ctlr;
	u_int mr;
{
	struct gen_bus_adapt *vhp = (struct gen_bus_adapt *)ctlr->bus_hd->xbahd;

	Cprintf("gen_ba_release\n");
	if( vhp->release )
		(*vhp->release)(ctlr, mr);
	else
		panic("gen_ba_release: No release function");
}

void
gen_ba_log_ctlr_err(text, vhp, devptr)
	char	*text;
	struct	gen_bus_adapt	*vhp;
	struct  controller	*devptr;
{
	Cprintf("gen_ba_log_ctlr_err\n");
	if (*vhp->log_ctlr_err)
		(*vhp->log_ctlr_err)(text,vhp, devptr);
/* no action -- perhaps a warning?? */
}

void
gen_ba_log_dev_err(text,vhp, devptr)
	char	*text;
	struct	gen_bus_adapt	*vhp;
	struct  device *devptr;

{
	Cprintf("gen_ba_log_dev_err\n");
	gen_ba_log_ctlr_err(text, vhp, devptr);
	if (*vhp->log_dev_err) 
		(*vhp->log_dev_err)(text,vhp, devptr);
/* no action -- perhaps a warning?? */
}

int
gen_ba_rmw(ctlr,address_ptr,data,mask)
	struct controller *ctlr; /* pointer to controller structure */
	u_int  *address_ptr;   /* points to old data */
	u_int  data;           /* new data */
	u_int  mask;           /* lock bit(s) mask */
{
	struct gen_bus_adapt *vhp = (struct gen_bus_adapt *)ctlr->bus_hd->xbahd;

	Cprintf("gen_ba_rmw\n");
	if( vhp->rmw )
		return((*vhp->rmw)(ctlr,address_ptr,data,mask));
	else
		panic("gen_ba_rmw: No rmw function");
}

unsigned long
gen_ba_get_bus_addr(ctlr, addr)
	struct controller *ctlr;
	u_long addr;
{
	struct gen_bus_adapt *vhp = (struct gen_bus_adapt *)ctlr->bus_hd->xbahd;

	Cprintf("gen_ba_get_bus_addr\n");
	if(vhp->get_bus_addr)
		return((*vhp->get_bus_addr)(ctlr, addr));
	else
		panic("gen_ba_get_bus_addr: no function");
}

int
gen_ba_config_bus(bus)
	struct bus *bus;
{
	struct controller *ctlr;
	caddr_t map_addr, map_addr2;
	struct driver *drp;
	int vec, i;
	int savectlr;
	char *savectname;
	struct device *device;
	int (**ivec)(), (**intr_dispatch)();

	struct gen_bus_adapt *vhp = (struct gen_bus_adapt *)bus->xbahd;
#ifdef DEBUG
	debug_vhp = vhp;
#endif 

	Cprintf("gen_ba_config_bus\n");
	/*
	 * Search controller table for xxxbus controller devices,
	 * see if it is really there, and if it is record it and
	 * then go looking for slaves.
	 */
	for (ctlr = controller_list; ctlr->ctlr_name; ctlr++) {
		if (((ctlr->bus_num != bus->bus_num) &&
		     (ctlr->bus_num !=-1) && (ctlr->bus_num != -99)) ||
		    (strcmp(ctlr->bus_name, bus->bus_name) ) ||
		    (ctlr->alive & ALV_ALIVE) ) {
			continue;
		}

		map_addr = 0;
		map_addr2 = 0;
	        drp = ctlr->driver;
	        vec = ctlr->ivnum;

		if(!(*vhp->is_ivec_valid)(vec)) {
			printf("%s%d not configured: Invalid vector 0x%x\n",
				ctlr->ctlr_name, ctlr->ctlr_num, vec);
			continue;
		}
		Cprintf("gen_ba_config_bus: Found device\n");
		Cprintf("gen_ba_config_bus: name %s%d Addr1 0x%lx Addr2 0x%lx\n",
			ctlr->ctlr_name, ctlr->ctlr_num, 
			ctlr->addr, ctlr->addr2);

		/* Map csr space(s)			*/
	        /* Map csr1 address space if present	*/
		if(ctlr->addr) {
			if ( (map_addr = gen_ba_map_csr(vhp, 
							(u_long)ctlr->addr, 
							drp->addr1_size,
							drp->addr1_atype,
							TRUE)) == 0){
				printf("  %s%d not configured.\n", 
				       ctlr->ctlr_name, ctlr->ctlr_num);
				continue;
			}
		}

		/* Map csr2 address space if present	*/
		if(ctlr->addr2) {
			if( (map_addr2 = gen_ba_map_csr(vhp, 
							(u_long)ctlr->addr2, 
							drp->addr2_size,
							drp->addr2_atype,
							TRUE)) == 0) {
				printf("  %s%d not configured.\n", 
				       ctlr->ctlr_name, ctlr->ctlr_num);
				if(ctlr->addr) {
					gen_ba_unmap_csr(vhp, map_addr, 
							 drp->addr1_size,
							 drp->addr1_atype);
				}
				continue;
			}
		}

	        Cprintf("gen_ba_config_bus: Calling probe for %s%d\n",
			ctlr->ctlr_name, ctlr->ctlr_num); 
		/* 
		 * XXX
		 * we need to connect the ctlr to the bus
		 * before we probe because probe will use 
		 * routines (i.e. read_byte)
		 * that can only be found via the bus structure
		 */
		ctlr->bus_hd = bus;
		i = (*drp->probe)(ctlr, map_addr, map_addr2);
		ctlr->bus_hd = (struct bus *)0;
		if (i == 0) {
			/* Probe failed.  Unmap and check next entry. */
			/* 
			 * In case TC timed out first, 
			 * allow other bus to timeout 
			 */
			DELAY(1000); 
			if(ctlr->addr) {
				gen_ba_unmap_csr(vhp, map_addr, 
						 drp->addr1_size,
						 drp->addr1_atype);
			}
			if(ctlr->addr2) {
				gen_ba_unmap_csr(vhp, map_addr2, 
						 drp->addr2_size,
						 drp->addr2_atype);
			}
			continue;
		}
		
		config_fillin(ctlr);
		printf(" csr 0x%x", ctlr->addr);
		(*vhp->display_addr_type)(drp->addr1_atype);
		if(map_addr2) {
			printf(" csr2 0x%x", ctlr->addr2);
			(*vhp->display_addr_type)(drp->addr2_atype);
		}
		printf(" vec 0x%x", ctlr->ivnum);
		printf(" priority %d\n", ctlr->bus_priority);
		ctlr->alive |= ALV_ALIVE;
		ctlr->addr = map_addr;
		ctlr->addr2 = map_addr2;
		/* mbox addrs are already physical */
		if(ctlr->ctlr_mbox == 0) {
			if(map_addr)
				(void)svatophys(map_addr, &ctlr->physaddr);
			if(map_addr2)
				(void)svatophys(map_addr2, &ctlr->physaddr2);
		}
		drp->ctlr_list[ctlr->ctlr_num] = ctlr;
	        conn_ctlr(bus, ctlr);
		
		vhp = (struct gen_bus_adapt *)bus->xbahd;

#ifdef __alpha
		for(ivec = ctlr->intr; *ivec; ivec++, vec++) 
			vme_intrsetvec(vhp, vec, *ivec, ctlr->ctlr_num);
#else /* __alpha */
		intr_dispatch = (int (**)())
			((((struct gen_bus_adapt *)vhp)->intr_vec) + (vec));

		Cprintf("gen_ba_config_bus: intr_dispatch 0x%x\n", 
			intr_dispatch);
		for (ivec = ctlr->intr; *ivec; ivec++) {
			*intr_dispatch = *ivec;
			intr_dispatch++;
		}
#endif  /* __alpha */

		ctlr->priority = ctlr->bus_priority;

		if (drp->cattach)
			(*drp->cattach)(ctlr);

		for (device = device_list; device->dev_name; device++) {
			if (((device->ctlr_num != ctlr->ctlr_num) &&
			     (device->ctlr_num !=-1)) ||
			    ((strcmp(device->ctlr_name, ctlr->ctlr_name)) &&
			     (strcmp(device->ctlr_name, "*"))) ||
			    (device->alive & ALV_ALIVE) ) {
				continue;
			}

			Cprintf("gen_ba_config_bus: Found matching device entry\n");
			savectlr = device->ctlr_num;
			savectname = device->ctlr_name;
			device->ctlr_num = ctlr->ctlr_num;
			device->ctlr_name = ctlr->ctlr_name;
			device->ctlr_hd = ctlr;
			
			if((drp->slave) && 
			   ((*drp->slave)(device, map_addr, map_addr2))) {
				device->alive |= ALV_ALIVE;
				device->ctlr_num = ctlr->ctlr_num;
				conn_device(ctlr, device);
				drp->dev_list[device->logunit] = device;
#ifdef JAA_notdef
				printf("%s%d at %s%d slave %d\n",
				       device->dev_name, device->logunit,
				       drp->ctlr_name, ctlr->ctlr_num,
				       device->unit);
#else /* JAA_notdef */

				if(device->unit >= 0) {
				/* print bus target lun info for SCSI devs */
				    if(!(strncmp(device->dev_name,"rz",2)) || 
				       (!strncmp(device->dev_name, "tz", 2))){
					    printf("%s%d at %s%d bus %d target %d lun %d",
						   device->dev_name, 
						   device->logunit,
						   drp->ctlr_name, 
						   ctlr->ctlr_num, 
						   ((device->unit & 0xFC0)>>6),
						   ((device->unit & 0x38)>>3),
						   (device->unit & 0x7) );
				    } else {
					    printf("%s%d at %s%d unit %d",
						   device->dev_name, 
						   device->logunit,
						   drp->ctlr_name, 
						   ctlr->ctlr_num, 
						   device->unit);
				    }
			    }
#endif /* JAA_notdef */
				Cprintf("gen_ba_config_bus: calling attach\n");
				if(drp->dattach)
					(*drp->dattach)(device);
				printf("\n");
			} else {
				device->ctlr_num = savectlr;
				device->ctlr_name = savectname;
			}
		}
	}
	return(1);
}

/* Convert virtual address to physical address. */
u_long
gen_ba_vatophys(bp, addr)
	struct buf *bp;
	u_long addr;
{
	u_long phys = 0;
	struct pmap *pmap;

	Cprintf("gen_ba_vatophys: addr 0x%l016x\n", addr);
#ifdef mips
		/* mips KSEG Addressing */
        if(IS_KSEG0(addr)){ 
			/* unmapped KSEG areas - no ptes */
               	phys = K0_TO_PHYS(addr);
        } else if(IS_KSEG1(addr)){
			/* unmapped KSEG areas - no ptes */
               	phys = K1_TO_PHYS(addr);
        } else  {
		/* must be mapped */
		Cprintf("gen_ba_vatophys: bp->b_proc=%x\n", bp->b_proc);
		if( bp->b_proc ) {
			pmap = bp->b_proc->task->map->vm_pmap;
		} else {
			pmap = pmap_kernel();
		}
		phys = pmap_extract(pmap, addr);
	}
#endif
#ifdef __alpha
	if(IS_KSEG_VA(addr)){
			/* kernel space, unmapped */
		phys = KSEG_TO_PHYS(addr);
		Cprintf("gen_ba_vatophys: kernel space unmapped\n");
	} else if(IS_SEG0_VA(addr)){
			/* user space, mapped */
		Cprintf("gen_ba_vatophys: bp->b_proc=%x\n", bp->b_proc);
		if( bp->b_proc ) {
			pmap = bp->b_proc->task->map->vm_pmap;
		} else {
		  /* 
		   * SEG0 means user space, so there should be a process
		   * (b_proc) associated with the buffer 
		   */
			pmap = pmap_kernel();
			/* It may be preferable/necessary to panic */
			/*panic("no user pmap for buf (in gen_ba_vatophys)");*/
		}
		phys = pmap_extract(pmap, addr);
	} else if(IS_SEG1_VA(addr)){
			/* kernel space, mapped */
		pmap = pmap_kernel();
		phys = pmap_extract(pmap, addr);
		Cprintf("gen_ba_vatophys: kernel space mapped\n");
	} else {
		printf("gen_ba_vatophys: bad addr 0x%l016x\n", addr);
		panic("gen_ba_vatophys: bad address");
      }
#endif
	Dprintf("gen_ba_vatophys: phys 0x%l016x\n", phys);
	return(phys);
}

char
gen_ba_read_byte(addr, ctlr)
	u_long addr;
	struct controller *ctlr;
{
	struct gen_bus_adapt *vhp = (struct gen_bus_adapt *)ctlr->bus_hd->xbahd;

	Cprintf("gen_ba_read_byte\n");
	if(vhp->read_byte)
		return((*vhp->read_byte)(addr, ctlr));
	panic("gen_ba_read_byte: no function");
}

short
gen_ba_read_word(addr, ctlr)
	u_long addr;
	struct controller *ctlr;
{
	struct gen_bus_adapt *vhp = (struct gen_bus_adapt *)ctlr->bus_hd->xbahd;

	Cprintf("gen_ba_read_word\n");
	if(vhp->read_word)
		return((*vhp->read_word)(addr, ctlr));
	panic("gen_ba_read_word: no function");
}

int
gen_ba_read_long(addr, ctlr)
	u_long addr;
	struct controller *ctlr;
{
	struct gen_bus_adapt *vhp = (struct gen_bus_adapt *)ctlr->bus_hd->xbahd;

	Cprintf("gen_ba_read_long\n");
	if(vhp->read_long)
		return((*vhp->read_long)(addr, ctlr));
	panic("gen_ba_read_long: no function");
}

void
gen_ba_write_byte(addr, data, ctlr)
	u_long addr;
	char data;
	struct controller *ctlr;
{
	struct gen_bus_adapt *vhp = (struct gen_bus_adapt *)ctlr->bus_hd->xbahd;

	Cprintf("gen_ba_write_byte\n");
	if(vhp->write_byte)
		(*vhp->write_byte)(addr, data, ctlr);
	else
		panic("gen_ba_write_byte: no function");
}

void
gen_ba_write_word(addr, data, ctlr)
	u_long addr;
	short data;
	struct controller *ctlr;
{
	struct gen_bus_adapt *vhp = (struct gen_bus_adapt *)ctlr->bus_hd->xbahd;

	Cprintf("gen_ba_write_word\n");
	if(vhp->write_word)
		(*vhp->write_word)(addr, data, ctlr);
	else
		panic("gen_ba_write_word: no function");
}

void
gen_ba_write_long(addr, data, ctlr)
	u_long addr;
	int data;
	struct controller *ctlr;
{
	struct gen_bus_adapt *vhp = (struct gen_bus_adapt *)ctlr->bus_hd->xbahd;

	Cprintf("gen_ba_write_long\n");
	if(vhp->write_long)
		(*vhp->write_long)(addr, data, ctlr);
	else
		panic("gen_ba_write_long: no function");
}

