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
static char *rcsid = "@(#)$ $ (DEC) $";
#endif

#include <cpus.h>
#include <machine/pmap.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/map.h>
#include <sys/vmmac.h>
#include <sys/buf.h>
#include <sys/conf.h>
#include <sys/config.h>

#include <io/dec/uba/ubareg.h>
#include <io/dec/uba/ubavar.h>
#include <kern/xpr.h>
#include <machine/cpu.h>
#include <machine/scb.h>
#include <hal/cpuconf.h>
#include <io/common/devdriver.h>
#include <vm/vm_kern.h>
#include <io/dec/mbox/mbox.h>

extern int cpu;
extern struct device	device_list[];

extern int confdebug;		/* debug variable for configuration code */

/* Define debugging stuff.
 */
#define DEBUG
#ifdef DEBUG
#define Cprintf if(confdebug)printf
#define Dprintf if( confdebug >= 2 )printf
#else
#define Cprintf ;
#define Dprintf ;
#endif

/*
 * Name:	ibusconfl1(bustype, binfo, bus)
 *
 * Args:	bustype	- Type of the calling bus. -1 is we're the system bus.
 *
 *		binfo	- Bus info structure for the calling bus. 
 *
 *		bus	- Pointer to bus our bus structure.
 *
 */
ibusconfl1(bustype, binfo, bus)
	long bustype;
	caddr_t binfo;
	struct bus *bus;
{
    /* Currently only support ibus as system bus */
    if(bustype != -1)
	    panic("ibus not system bus");
    bus->bus_type = BUS_IBUS;
    bus->alive = ALV_ALIVE;
    printf("%s%d at nexus\n", bus->bus_name, bus->bus_num);

    /* Call machine specific configuration routine as ibus is */
    /* used by several different machines.		      */
    switch(cpu) {
#ifdef __alpha

	  case DEC_4000:
	    cobra_config_cbus(bus);
	    break;

	  case DEC_2000_300:
	    kn121_configure_io(bus);
	    break;

#endif /* __alpha */	    

	  default:
	    printf("Unknown bus type in ibusconfl1\n");
	    break;
    }
    return(0);
}

ibusconfl2(bustype, binfo, bus)
	long bustype;
	caddr_t binfo;
	struct bus *bus;
{
	return(1);
}

/*
 * Name:	ib_config_cont (nxv, nxp, slot, name, scb_vec_addr);
 *
 * Args:	nxv	- The virtual address of the "controller"
 *
 *		nxp	- The physicall address of the "controller"
 *
 *		slot	- The mbus slot number containing the "controller"
 *
 *		name	- The name of the "controller" to match with in the
 *			  ubminit and ubdinit structures
 *
 *		bus	- ptr to the bus structure
 *
 *		scb_vec_addr - The offset from the begining of scb block
 *				 zero that you want the address of the 
 *				 interrupt routine specified in the um
 *				 structure inserted.  If the value equals
 *				 zero, do not insert the the address of the
 *				 interrupt routine into the scb.
 *
 * Returns:	1 - if the "controller" was found
 *		0 - if the "controller" wasn't found
 */
ib_config_cont(nxv, nxp, slot, name, bus, scb_vec_addr)
	char *nxv;
	char *nxp;
	u_long slot;
	char *name;
	struct bus *bus;
	long scb_vec_addr;
{
	register struct controller *ctlr;
	register struct device *device;
	register struct driver *drp;
	int savectlr, savebus, saveslot;
	char *savectname, *savebusname;

	if( ((ctlr = get_ctlr(name, slot, bus->bus_name, bus->bus_num)) == 0) &&
	    ((ctlr = get_ctlr(name, -1, bus->bus_name, bus->bus_num)) == 0) &&
	    ((ctlr = get_ctlr(name, -1, bus->bus_name, -1)) == 0) &&
	    ((ctlr = get_ctlr(name, -1, "*", -99)) == 0) )
		    return(0);

	savebus = ctlr->bus_num;
	savebusname = ctlr->bus_name;
	saveslot = ctlr->slot;
	ctlr->bus_name = bus->bus_name;
	ctlr->bus_num = bus->bus_num;
	ctlr->slot = slot;

	MBOX_GET(bus, ctlr);

	drp = ctlr->driver;
	if (scb_vec_addr)
	    ibcon_vec(scb_vec_addr, ctlr);

	if((*drp->probe)(nxv, ctlr) == 0) {
	    ctlr->bus_num = savebus;
	    ctlr->bus_name = savebusname;
	    ctlr->slot = saveslot;
	    return(0);
        }
	ctlr->alive = ALV_ALIVE;
	ctlr->addr = (char *)nxv;

	/* mbox addrs are already physical */
	if(ctlr->ctlr_mbox == 0)
		(void)svatophys(ctlr->addr, &ctlr->physaddr);

	drp->ctlr_list[ctlr->ctlr_num] = ctlr;
	config_fillin(ctlr);
	printf("\n");
	if (drp->cattach)
		(*drp->cattach)(ctlr);
	Cprintf("ib_config_cont: Calling conn_ctlr\n");
	conn_ctlr(bus, ctlr);
	
	Cprintf("ib_config_cont: Searching device list\n");
	for (device = device_list; device->dev_name; device++) {

		if (((device->ctlr_num != ctlr->ctlr_num) &&
		     (device->ctlr_num !=-1) && (device->ctlr_num != -99)) ||
		    ((strcmp(device->ctlr_name, ctlr->ctlr_name)) &&
		     (strcmp(device->ctlr_name, "*"))) ||
		    (device->alive) ) {
			continue;
		}

		savectlr = device->ctlr_num;
		savectname = device->ctlr_name;
		device->ctlr_num = ctlr->ctlr_num;
		device->ctlr_name = ctlr->ctlr_name;
		Cprintf("ib_config_cont: Found device: calling slave\n");
		if ((drp->slave) && (*drp->slave)(device, nxv)) {
			device->alive = ALV_ALIVE;
			device->ctlr_num = ctlr->ctlr_num;
			Cprintf("tc_config_cont: calling conn_device\n");
			conn_device(ctlr, device);
			drp->dev_list[device->logunit] = device;
		if(device->unit >= 0) {
		    /* print bus target lun info for SCSI devices */
		    if( (strncmp(device->dev_name, "rz", 2) == 0) || 
		        (strncmp(device->dev_name, "tz", 2) == 0) )   {
		    	printf("%s%d at %s%d bus %d target %d lun %d",
			    device->dev_name, device->logunit,
			    drp->ctlr_name, ctlr->ctlr_num, 
			    ((device->unit & 0xFC0) >> 6),
			    ((device->unit & 0x38) >> 3),
			    (device->unit & 0x7) );
		    } else {
		    	printf("%s%d at %s%d unit %d",
			    device->dev_name, device->logunit,
			    drp->ctlr_name, ctlr->ctlr_num, device->unit);
		    }
		  }
			Cprintf("tc_config_cont: calling attach\n");
			if (drp->dattach)
				(*drp->dattach)(device);
			printf("\n");
		} else {
			device->ctlr_num = savectlr;
			device->ctlr_name = savectname;
		}
	}
	return (1);
}

#ifndef __alpha
ibcon_vec(scb_vec_addr, ctlr)
	long scb_vec_addr;
	struct controller *ctlr;
{
	register int (**ivec)();
	register int (**addr)();

	ivec = ctlr->intr;
	addr = (int (**)())scb_vec_addr;
	*addr = scbentry(*ivec, SCB_ISTACK);
	return;

}

#else /* __alpha */

ibcon_vec(scb_vec, ctlr)
	u_short scb_vec;
	struct controller *ctlr;
{
	intrsetvec(scb_vec, *ctlr->intr, ctlr);
	return;
}
#endif /* __alpha */

