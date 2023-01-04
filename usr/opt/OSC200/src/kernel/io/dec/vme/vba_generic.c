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
static char *rcsid = "@(#)$RCSfile: vba_generic.c,v $ $Revision: 1.1.7.3 $ (DEC) $Date: 1993/09/15 21:41:52 $";
#endif

/*
 *
 * Abstract:
 *	This module contains the routines for the generic VMEbus interface.
 *
 * Revision History
 *
 *	24-Jan-92	stuarth -- Stuart Hollander
 *		general revisions, editing
 *
 *	14-Nov-91	stuarth -- Stuart Hollander
 *		Original version
 *************************************************************************/
/* contains:
	vme_map_csr(vhp, addr, size, atype, print_flag) 
	vme_unmap_csr(vhp, addr, size, atype)
	vbasetup(ctlr, bp, flags, addr)
	vballoc(ctlr, addr, bcnt, flags, vmeaddr)
	vbarelse(ctlr, mr)
	log_vme_device_error(text,vhp, devptr)
	log_vme_ctlr_error(text,vhp, devptr)
	vme_rmw(ctlr,address_ptr,data,mask)
	vba_get_vmeaddr(ctlr, addr)
	vbaconfl1(bustype, binfo, bus)
	vbaconfl2(bustype, binfo, bus)
	vme_display_addr_type(atype)
	vbaerrors(vbanumber)
	vme_is_ivec_valid(vec)
	vme_ba_ins(vhp)
	vme_ba_get(bus_adapt_number) 
	vme_ba_config_bus(bus)
	vme_ba_vatophys(bp, addr)

	vme_read_byte(addr, ctlr)
	vme_read_word(addr, ctlr)
	vme_read_long(addr, ctlr)
	vme_write_byte(addr, data, ctlr)
	vme_write_word(addr, data, ctlr)
	vme_write_long(addr, data, ctlr)
	vme_badaddr(addr, len, ptr)
	vme_intrsetvec(vhp, vec, handler, param)
 */

#include <io/dec/vme/gen_bus_adapt.h>

#ifdef DEBUG
/* this is in xvia_data.c */
extern int vmedebug;
#define Cprintf if(vmedebug)printf
#define Dprintf if(vmedebug >= 2 )printf
#else
#define Cprintf ;
#define Dprintf ;
#endif

struct	gen_bus_adapt	*head_vba = 0;

caddr_t
vme_map_csr(vhp, addr, size, atype, print_flag) 
	struct	gen_bus_adapt	*vhp;
	caddr_t			 addr;	/* match type of  controller.addr
	int			 size;	/* match type of  driver.size */
	int			 atype;	/* match type of  driver.atype */
	int			 print_flag;
{
	Cprintf("vme_map_csr\n");
	return(gen_ba_map_csr(vhp, (u_long)addr, size, atype, print_flag));
}

void
vme_unmap_csr(vhp, addr, size, atype)
	struct	gen_bus_adapt *vhp;
	caddr_t  addr; /* the virtual addr (from map_csr), not the vme addr  */
	int size;      /* match type of  driver.size */
	int atype;     /* match type of  driver.atype */
{
	Cprintf("vme_unmap_csr\n");
	gen_ba_unmap_csr(vhp, addr, size, atype);
}

unsigned int
vbasetup(ctlr, bp, flags, addr)
	struct controller *ctlr;
	struct buf	*bp;
	long		 flags;
	u_long		 addr;
{
	Cprintf("vbasetup: flags 0x%lx addr 0x%lx\n", flags, addr);
	return(gen_ba_setup(ctlr, bp, flags, addr));
}

unsigned int
vballoc(ctlr, addr, bcnt, flags, vmeaddr)
	struct controller *ctlr;
	caddr_t		addr;
	int		bcnt;
	long		flags;
	u_long		vmeaddr;
{
	struct buf vbabuf;

	Cprintf("vballoc\n");
	vbabuf.b_un.b_addr = addr;
	vbabuf.b_flags = B_BUSY;
	vbabuf.b_bcount = bcnt;
	vbabuf.b_proc = 0;
	/* that's all the fields vbasetup() needs */
	return(vbasetup(ctlr, &vbabuf, flags, vmeaddr));
}
 
void
vbarelse(ctlr, mr)
	struct controller *ctlr;
	u_int		mr;
{
	Cprintf("vbarelse\n");
	gen_ba_release(ctlr, mr);
}

void
log_vme_device_error(text, vhp, devptr)
	char			*text;
	struct	gen_bus_adapt	*vhp;
	struct  device		*devptr;
{
	Cprintf("log_vme_device_error\n");
	gen_ba_log_dev_err(text, vhp, devptr);
}

void
log_vme_ctlr_error(text, vhp, devptr)
	char			*text;
	struct	gen_bus_adapt	*vhp;
	struct  controller	*devptr;
{
	Cprintf("log_vme_ctlr_error\n");
	gen_ba_log_ctlr_err(text, vhp, devptr);
}

int
vme_rmw(ctlr,address_ptr,data,mask)
	struct controller *ctlr; /* pointer to controller structure */
	u_int		*address_ptr;   /* points to old data */
	u_int		 data;           /* new data */
	u_int		 mask;           /* lock bit(s) mask */
{
	Cprintf("vme_rmw\n");
	return(gen_ba_rmw(ctlr,address_ptr,data,mask));
}

u_long
vba_get_vmeaddr(ctlr, addr)
	struct controller	*ctlr;
	u_long			 addr;
{
	Cprintf("vba_get_vmeaddr\n");
	return(gen_ba_get_bus_addr(ctlr, addr));
}

int
vbaconfl1(bustype, binfo, bus)
	int		 bustype;
	caddr_t		 binfo;
	struct bus	*bus;
{
	Cprintf("vbaconfl1\n");
#ifdef JAA_notsure
	switch( cpu ) {
	      case DEC_3000_300:
	      case DEC_3000_500:
		break;

	      default:
		printf("vba%d: VMEbus Adapter not supported on this machine\n",
		       vhp->xbanum);
		return(0);
	}
#endif /* JAA_notsure */

	switch (bustype) {
	      case BUS_TC:
		{
			caddr_t addr, physaddr;
			int slot, unit;
			int (**intr)();
			int vbatype;
			int (*confrtn)();
			struct bus *bus_hd;

			struct tc_info *tinfo = (struct tc_info *)binfo;

			addr = tinfo->addr;
			physaddr = tinfo->physaddr;
			slot = tinfo->slot;
			unit = tinfo->unit;
			intr = tinfo->intr;
			bus_hd = tinfo->bus_hd;
			Cprintf("vbaconfl1: index %d\n",(int)bus->tcindx); 
			confrtn = tc_slot[(int)bus->tcindx].adpt_config;
			Cprintf("vbaconfl1: addr 0x%x physaddr 0x%x slot %d unit %d confrtn 0x%x\n",
				addr, physaddr, slot, unit, confrtn);
			Cprintf("vbaconfl1: Call configuration routine\n");
			return((*confrtn)(addr, physaddr, slot, unit, intr, bus, bus_hd));
			break;
		}
	      default:
		panic("vbaconfl1: Unsupported bus type\n");
		break;
	}
}

int		
vbaconfl2(bustype, binfo, bus)
	int	bustype;
	caddr_t binfo;
	struct bus *bus;
{
	Dprintf("vbaconfl2\n");
	return(1);
}

void
vme_display_addr_type(atype)
	int	atype;
{
	int aspace, dsize;
	static char	*vme_spaces[3][3] = {
		{"vmea16d08", "vmea24d08", "vmea32d08"},
		{"vmea16d16", "vmea24d16", "vmea32d16"},
		{"vmea16d32", "vmea24d32", "vmea32d32"}
	};

	Cprintf("vme_display_addr_type\n");
	/* adjust this code, maybe, for adjusted flag values */
	/* allow for customer specified function */
	if( atype & VME_AM ) {
		printf(" Address Modifier %x", (atype & VME_AM)>>VME_AM_SHIFT);
	} else {
		aspace = atype & VME_ASPACE_MASK;
		dsize = (atype & VME_ASIZE_MASK) >> VME_ASIZE_SHIFT;
		printf(" %s", vme_spaces[dsize][aspace]);
		if( atype & VME_USER ) {
			printf(" user");
		} else {
			printf(" supervisory");
		}
		if( atype & VME_DATA ) {
			printf(" program");
		} else {
			printf(" data");
		}
		printf(" mode");
	}
}

void
vbaerrors(vbanumber)
	int vbanumber;
{
	register struct gen_bus_adapt *vhp;

	Cprintf("vbaerrors: Entered  vbanumber=%d\n", vbanumber);
	vhp = gen_ba_get(&head_vba, vbanumber);
	Dprintf("vbaerrors: After gen_ba_get\n");

	if( (*vhp->errors)(vhp) == TRUE)
		panic("VMEbus adapter error");

}

/* Is the interrupt vector valid? */
int
vme_is_ivec_valid(vec)
	int vec;
{
	Dprintf("vme_is_ivec_valid\n");
	return((vec >= VME_MIN_VEC && vec <= VME_MAX_VEC));
}

int
vme_ba_ins(vhp)
	struct gen_bus_adapt	*vhp;
{
	Cprintf("vme_ba_ins\n");
	return(gen_ba_ins(&head_vba, vhp));
}

struct gen_bus_adapt *
vme_ba_get(bus_adapt_number) 
	int bus_adapt_number;
{
	Cprintf("vme_ba_get\n");
	return(gen_ba_get(&head_vba, bus_adapt_number));
}

int
vme_ba_config_bus(bus)
	struct bus *bus;
{
	Cprintf("vme_ba_config_bus\n");
	return(gen_ba_config_bus(bus));
}

u_long
vme_ba_vatophys(bp, addr)
	struct buf *bp;
	u_long addr;
{
	Cprintf("vme_ba_vatophys\n");
	return(gen_ba_vatophys(bp, addr));
}

#ifdef __alpha

char
vme_read_byte(addr, ctlr)
	u_long			 addr;
	struct controller	*ctlr;
{
	Cprintf("vme_read_byte\n");
	return(gen_ba_read_byte(addr, ctlr));
}

short
vme_read_word(addr, ctlr)
	u_long			 addr;
	struct controller	*ctlr;
{
	Cprintf("vme_read_word\n");
	return(gen_ba_read_word(addr, ctlr));
}

int
vme_read_long(addr, ctlr)
	u_long			 addr;
	struct controller	*ctlr;
{
	Cprintf("vme_read_long\n");
	return(gen_ba_read_long(addr, ctlr));
}

void
vme_write_byte(addr, data, ctlr)
	u_long			 addr;
	u_char			 data;
	struct controller	*ctlr;
{
	Cprintf("vme_write_byte\n");
	gen_ba_write_byte(addr, data, ctlr);
}

void
vme_write_word(addr, data, ctlr)
	u_long			addr;
	u_short                 data;
	struct controller	*ctlr;
{
	Cprintf("vme_write_word\n");
	gen_ba_write_word(addr, data, ctlr);
}

void
vme_write_long(addr, data, ctlr)
	u_long			addr;
	u_int                   data;
	struct controller	*ctlr;
{
	Cprintf("vme_write_long\n");
	gen_ba_write_long(addr, data, ctlr);
}

int
vme_badaddr(addr, len, ptr)
	vm_offset_t addr;
	int len;
	struct bus_ctlr_common *ptr;
{
	volatile long l;
	volatile int i, bum_addr;
	volatile short s;
	volatile char c;
	/* flag used to communicate between badaddr and Mcheck handling */
	int a = splextreme();
	extern mcheck_expected;

	Cprintf("vme_badaddr(0x%lx 0x%lx 0x%lx)\n", addr, len, ptr);

	if (mcheck_expected)
		panic("vme_badaddr: mcheck_expected set in when unexpected");
	mcheck_expected = 1;		/* enter the mcheck_ok region.	*/
	mb();				/* insure mcheck_expected is written */

	switch(len) {
		case 1:
			c = vme_read_byte((u_char *)addr, ptr);
			break;
		case 2:
			s = vme_read_word((unsigned short *)addr, ptr);
			break;
		case 4:
			i = vme_read_long((unsigned int *)addr, ptr);
			break;
		case 8:
			/* TODO read_quad */
		default:
			panic("vme_badaddr: bad length");
			break;
	}
	bum_addr = ! mcheck_expected;
	mcheck_expected = 0;
	mb();

	Cprintf("vme_badaddr: addr 0x%lx is %s there\n", 
		addr, (bum_addr ? "*not*" : "really"));
	splx(a);
	return(bum_addr);
}

#include <machine/scb.h>

void
vme_intrsetvec(vhp, vec, handler, param)
	struct gen_bus_adapt *vhp;
	u_short vec;
        u_long (*handler)();    /* isr in vector table */
        u_long param;           /* usually this is the unit numer */
{
        struct scbentry *scbp;

        scbp = (struct scbentry *)((u_long)(vhp->intr_vec + vec));
	Cprintf("vme_intrsetvec: vhp 0x%lx vec 0x%x handler 0x%lx param 0x%x scbp 0x%lx\n",
		vhp, vec, handler, param, scbp);
        scbp->scb_vector = handler;
        scbp->scb_param = param;
}

#endif /* __alpha */
