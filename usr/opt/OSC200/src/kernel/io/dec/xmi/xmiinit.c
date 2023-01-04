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
static char *rcsid = "@(#)$RCSfile: xmiinit.c,v $ $Revision: 1.2.2.16 $ (DEC) $Date: 1993/01/22 15:22:47 $";
#endif

/*
 * Revision History
 *
 * 03-Dec-90	Joe Szczypek
 *	Added return() to get_xmi() to silence LINT.
 *
 * 29-Aug-90	stuarth (Stuart Hollander)
 *	Fixed bug in xmisst when determining which xmi a node is on.
 *	Added comments explaining xmiconf structure.
 *
 * 17-Aug-90	rafiey (Ali Rafieymehr)
 *	Made the following changes for Stuart Hollander.
 *	Split xmiconf() into xmiconf_reset() and xmiconf_conf().
 *	Allows, per xmi bus, to reset all nodes, then wait as they
 *	all perform self-test in parallel, instead of resetting and
 *	waiting for each device, one-by-one.
 *	Also, handles multiple xmi s.
 *
 * 03-Aug-90	rafiey (Ali Rafieymehr)
 *	Changed xmi_io_space for VAX9000. Defined numxmi for multiple
 *	XMI support.
 *
 * 02-May-90    Joe Szczypek
 *      Modified error routines for xbi+ support.
 *
 * 13-Mar-90	rafiey (Ali Rafieymehr)
 *	Removed unnecessary check for invalid slot of xmi node from xmi_io_space().
 *
 * 11-Dec-89    Paul Grist
 *      Modified xmierrors to call panic using status returned from
 *      xbi_check_errs(). Added new routine log_xmi_bierrors() to 
 *      to log pending VAXBI errors and log_xmierrors to just log pending
 *      xbi errors. They will be used by exception handlers looking for
 *      more error information.
 *
 * 08-Dec-89 	Pete Keilty
 *	Modified nxaccess() in xmiconf() routine to use XMINODE_SIZE
 *	as node space size define in xmireg.h for VAX.
 *
 * 08-Dec-89 	jaw
 *	make printf use decimal for printing node and bus numbers.  
 *
 * 13-Nov-89	burns
 *	Made xmi_io_space consistent over vax/mips platforms. Trickery
 *      now performed in nxaccess().
 *
 * 09-Nov-89	jaw
 *	fix bug where hooking xna to wrong bus.
 *
 * 20-Jul-89	rafiey (Ali Rafieymehr)
 *	Added support for XMI devices.
 *
 * 24-May-89	darrell
 *	Changed the #include for cpuconf.h to find it in it's new home --
 *	sys/machine/common/cpuconf.h
 *
 * 24-May-89	darrell
 *	Removed the v_ prefix from all cpusw fields, removed cpup from any
 *	arguments being passed in function args.  cpup is now defined
 *	globally -- as part of the new cpusw.
 *
 *************************************************************************/

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/map.h>
#include <sys/buf.h>
#include <sys/time.h>
#include <sys/kernel.h>
#include <hal/cpuconf.h>
#include <sys/dk.h>
#include <sys/config.h>
#include <sys/vmmac.h>
#include <machine/cpu.h>

#if (vax | mips)
#include <machine/nexus.h>
#endif /* _vax | _mips */

#include <machine/scb.h>
#include <io/dec/xmi/xmireg.h>
#include <io/common/devdriver.h>
#include <io/dec/xmi/xlambreg.h>
#include <machine/machparam.h>

#include <io/dec/mbox/mbox.h>

#ifdef vax
extern int catcher[256];
extern struct xmi_reg xmi_start[];
#endif /* _vax */

#ifdef mips
extern int stray();
unsigned xmi_start = (struct xmi_reg *)PHYS_TO_K1(XMI_START_PHYS);
#endif /* _mips */

#ifdef __alpha
/* 
 * this is ruby specific since xmi's are only attached by the iop
 * and are accessed via mailboxes.
 */
volatile struct xmi_reg *xmi_start = (volatile struct xmi_reg *)XMI_START_PHYS;
#endif /* __alpha */

extern struct cpusw *cpup;	/* pointer to cpusw entry */

/* START OF COMMON CODE */
struct xmidata *
get_xmi(xminum) 
	int xminum;
{
	register struct xmidata *xmidata;
	
	for(xmidata = head_xmidata; xmidata; xmidata = xmidata->next)
		if(xmidata->xminum == xminum) 
			return(xmidata);

	panic("get_xmi: no bus data");
	/*NOTREACHED*/
}

/* 
 * Instead of being simply one large function,
 * xmiconf is structured as three subfunctions so that
 * one could easily write a function to initialize multiple
 * xmis in parallel by calling xmiconf_reset for each xmi,
 * then calling xmiconf_wait for each, and then calling
 * xmiconf_conf for each.
 * 
 * We keep xmiconf as a function that initializes
 * only one xmi so that existing code can remain unchanged.
 */
xmiconfl1(connbus, xmibus)
	struct bus *xmibus;
	struct bus *connbus;
{
#ifndef __alpha
	int s;
#endif /* ! __alpha */

	xmibus->bus_type = BUS_XMI;
	xmibus->alive |= ALV_ALIVE;
	if(connbus) 
		conn_bus(connbus, xmibus); 
		
#ifndef __alpha
	/* 
	 * I don't know why we drop IPL here unless 
	 * its to let error interrupts through
	 */
    	s = splbio();
#endif /* ! __alpha */
	xmiconf_reset(xmibus); /* Reset all nodes on the xmi */
	xmiconf_wait(xmibus);  /* Wait for all initializations to complete */
#ifndef __alpha
	splx(s);
#endif /* ! __alpha */
	xmiconf_conf(xmibus); /* Configure each node */
	return(1);
}

boolean_t
xmiconfl2(connbus, xmibus)
	struct bus *xmibus;
	struct bus *connbus;
{
	return(1);
}

boolean_t
xmi_config_con(nxv, nxp, xminum, xminode, xmidata, xmibus)
	volatile struct xmi_reg *nxv;
	char	        *nxp;
	register int    xminum;
	register int    xminode;
	register struct xmidata *xmidata;
	struct bus      *xmibus;
{
	register struct controller *ctlr;
	register struct xmisw *pxmisw = xmidata->xmierr[xminode].pxmisw;
	boolean_t stat = 0;
	
	if((ctlr = get_ctlr(pxmisw->xmi_name, xminode, 
			    xmibus->bus_name, xmibus->bus_num)) ||
	   (ctlr = get_ctlr(pxmisw->xmi_name, xminode, xmibus->bus_name,-1)) ||
	   (ctlr = get_ctlr(pxmisw->xmi_name, -1, 
			    xmibus->bus_name, xmibus->bus_num)) ||
	   (ctlr = get_ctlr(pxmisw->xmi_name, -1, xmibus->bus_name, -1)) ||
	   (ctlr = get_ctlr(pxmisw->xmi_name, -1, "*", -99))) { 
		int savebusnum;
		char *savebusname;
		int saveslot;

		if(ctlr->alive & ALV_ALIVE) {
			printf("xmi_config_con: %s%d alive\n", 
			       ctlr->ctlr_name, ctlr->ctlr_num);
			return(stat);
		}
		savebusnum = ctlr->bus_num;
		savebusname = ctlr->bus_name;
		saveslot = ctlr->slot;
		ctlr->bus_name = xmibus->bus_name;
		ctlr->bus_num = xmibus->bus_num;
		ctlr->slot = xminode;

		MBOX_GET(xmibus, ctlr);
		/* 
		 * handle any port specific initialization 
		 * (i.e. kdm, kdb)
		 */
		if(ctlr->port && ctlr->port->conf) 
			stat = (*ctlr->port->conf)(nxv, nxp, ctlr, xmidata);
		else
			stat = config_ctlr(nxv, nxp, ctlr, xmidata);
		if(stat == 0) {
			ctlr->bus_name = savebusname;
			ctlr->bus_num = savebusnum;
			ctlr->slot = saveslot;
		}
	}else if(ctlr =(struct controller *)kalloc(sizeof(struct controller))){
		ctlr->ctlr_name = pxmisw->xmi_name;
		/*** ctlr->ctlr_num = ??? ***/
		ctlr->bus_name = xmibus->bus_name;
		ctlr->bus_num = xmibus->bus_num;
		ctlr->slot = xminode;
		ctlr->alive |= ALV_PRES;
		MBOX_GET(xmibus, ctlr);
		conn_ctlr(xmibus, ctlr);
	}
	return(stat);
}

boolean_t
config_ctlr(nxv, nxp, ctlr, xmidata)
	volatile struct xmi_reg *nxv;
	char	        *nxp;
	struct controller *ctlr;
	register struct xmidata *xmidata;
{
	register struct device *dev;
	register struct driver *drp;
	extern struct device device_list[];

	drp = ctlr->driver;
	if((*drp->probe)(nxv, ctlr) == 0) 
		return(0);
	
	ctlr->alive |= ALV_ALIVE;
	ctlr->addr = (char *)nxv;
	ctlr->physaddr = (char *)nxp;
	drp->ctlr_list[ctlr->ctlr_num] = ctlr;
	conn_ctlr(xmidata->bus, ctlr);
	config_fillin(ctlr);
	printf("\n");
	
	if(drp->cattach)
		(*drp->cattach)(ctlr);
	
	for(dev = device_list; dev->dev_name; dev++) {
		int savectlr;
		char *savectname;
		
		if(((dev->ctlr_num != ctlr->ctlr_num)&&(dev->ctlr_num !=-1)) ||
		   ((strcmp(dev->ctlr_name, ctlr->ctlr_name)) &&
		    (strcmp(dev->ctlr_name, "*"))) ||
		   (dev->alive & ALV_ALIVE) ) {
			continue;
		}
		
		savectlr = dev->ctlr_num;
		savectname = dev->ctlr_name;
		dev->ctlr_num = ctlr->ctlr_num;
		dev->ctlr_name = ctlr->ctlr_name;
		
		if((drp->slave) && (*drp->slave)(dev, nxv)) {
			dev->alive |= ALV_ALIVE;
			dev->ctlr_num = ctlr->ctlr_num;
			conn_device(ctlr, dev);
			drp->dev_list[dev->logunit] = dev;
			perf_init(dev);
			printf("%s%d at %s%d", dev->dev_name, dev->logunit,
			       ctlr->ctlr_name, ctlr->ctlr_num);
			if(dev->unit >= 0)
				printf(" unit %d", dev->unit);
			if(drp->dattach)
				(*drp->dattach)(dev);
			printf("\n");
		} else {
			dev->ctlr_num = savectlr;
			dev->ctlr_name = savectname;
		}
	}
	return(1);
} 

/* TODO: This routine should use xminum .  Ali */
xmi_io_space(xminum, xminode) 
 	int xminum, xminode;
{
#ifdef vax	
	extern int cpu;			/* Ultrix internal System type */
	extern int vecbi;

	if(cpu == VAX_9000)
		return(((int)0x22000000) + (0x02000000 * vecbi)); 
	else
#endif /* _vax */
		return(((int)XMI_BASE_PHYS) + (0x02000000 * xminode));
}


xbi_check_errs()
{
	printf("xbi_check_errs\n");
}

/* END of common code */

/* START OF ALPHA CODE PORTED TO USE MAILBOXES */
#ifdef __alpha

void
xmiconf_reset(xmibus)
	struct bus *xmibus;
{
	int xminum = xmibus->bus_num;
	volatile struct xmi_reg	*nxv;	/* virtual pointer to XMI node */
	register struct xmisw *pxmisw;
	register struct xmidata *xmidata;
	register int i;
	u_int xbe;
	struct scbentry *vecaddr;

	xmidata = (struct xmidata *)xmibus->bus_xmidata;
	xmidata->xmivirt = nxv = xmi_start;
	vecaddr = (struct scbentry *)SCB_XMI_ADDR(xmidata); 
	for(i = 0; i < NXMI_VEC; i++, vecaddr++) 
		intrsetvec(vecoffset(vecaddr), stray, vecoffset(vecaddr));
	/* set bus error interrupt vector */
	xmisetvec(xmibus);

	xmidata->xminodes_alive = 0;
	for(i = 0; i < MAX_XMI_NODE; i++, nxv++) {
		short dtype;

		if(BADADDR( &nxv->xmi_dtype, sizeof(long), xmibus))
			continue;
		dtype = RDCSR( LONG_32, xmibus, &nxv->xmi_dtype);
		for(pxmisw = xmisw; pxmisw->xmi_type; pxmisw++) { 
			if(pxmisw->xmi_type == dtype ) {
				xmidata->xmierr[i].pxmisw = pxmisw;
				xmidata->xminodes_alive |= (1 << i);

				if(pxmisw->xmi_flags & XMIF_SST) {
					xbe = RDCSR( LONG_32, xmibus, 
						    &nxv->xmi_xbe);
					xbe = (xbe & ~XMI_XBAD) | XMI_NRST;
					WRTCSR( LONG_32, xmibus, 
					       &nxv->xmi_xbe, xbe);
				}
				break;
			}
		}
		if(pxmisw->xmi_type == 0)
			printf ("xmi %d node %d unsupported dev type 0x%x\n",
				xminum, i, dtype);
	}
	DELAY(10000);	/* need to give time for XMI bad line to be set */
}

/* Wait for reset of xmi nodes to take effect. */
void
xmiconf_wait(xmibus)
	struct bus *xmibus;
{
	volatile struct xmi_reg	*nxv;	/* virtual pointer to XMI node */
	volatile struct xmi_reg *cpunode = 0;
	register struct xmidata *xmidata;
	register int xminode;
	/* 
	 * Wait cumulative up to 20 seconds.
	 * For extra safety, this is double the spec value. 
	 */
	int totaldelay = 2000;

	xmidata = (struct xmidata 
*)xmibus->bus_xmidata;
	/* mailboxes use phy addrs for remote buses */
	nxv = xmidata->xmivirt;
	for(xminode = 0; xminode < MAX_XMI_NODE; xminode++, nxv++) {
		if( ! (xmidata->xminodes_alive & (1 << xminode)))
			continue;
		/* 
		 * wait here for up to remaining count time
		 * or until device is reset. 
		 */
		if(xmidata->xmierr[xminode].pxmisw->xmi_flags & XMIF_SST) {
			u_int xbe;

			cpunode = xmidata->cpu_xmi_addr;
loop1:
			xbe = RDCSR( LONG_32, xmibus, &cpunode->xmi_xbe);
			if((xbe & XMI_XBAD) && (totaldelay-- > 0)) {
				DELAY(10000);
				goto loop1;
			}
loop2:
			xbe = RDCSR( LONG_32, xmibus, &nxv->xmi_xbe);
			if( (xbe & (XMI_ETF|XMI_STF)) && (totaldelay-- > 0)) {
				DELAY(10000);
				goto loop2;
			}
		}
	}
}

/* Do config of nodes. */
void
xmiconf_conf(xmibus)
	struct bus *xmibus;
{
	volatile struct xmi_reg	*nxv;	/* virtual pointer to XMI node */
	volatile struct xmi_reg	*nxp;	/* physical pointer to XMI node */
	register int xminode;
	register struct xmisw *pxmisw;
	register struct xmidata *xmidata;
	int broke;
	int alive;
	int totaldelay;
	u_int xbe;

	xmidata = (struct xmidata *)xmibus->bus_xmidata;
	nxv = xmidata->xmivirt;
	for(xminode = 0; xminode < MAX_XMI_NODE; xminode++, nxv++) {

		if( !(xmidata->xminodes_alive & (1 << xminode)))
			continue;
		pxmisw = xmidata->xmierr[xminode].pxmisw;
		/* 
		 * the xmi is accessed by a mailbox on alpha
		 * and the remote addr in the mailbox is physical
		 */
		nxp = nxv;

		xbe = RDCSR( LONG_32, xmibus, &nxv->xmi_xbe);
		broke = (xbe & XMI_ETF) | (xbe & XMI_STF);
		alive = 0;
		if(pxmisw->xmi_flags & XMIF_CONTROLLER) {
			if(!broke)
			alive |= xmi_config_con(nxv, nxp, xmibus->bus_num,
						xminode, xmidata, xmibus);
			else {
			printf("%s at xmi%d node %d is broken, continuing!\n",
			pxmisw->xmi_name, xmibus->bus_num, xminode);
			alive = 1;	/* just dummy up xminotconf message */
			}
		} else {
			/* ADAPTERS come in here also */
			if((pxmisw->xmi_flags & XMIF_NOCONF) || broke)
				printf("%s at xmi%d node %d%s",
				       pxmisw->xmi_name, xmibus->bus_num, 
				       xminode, (broke ? 
				       " is broken, continuing!\n" : "\n"));
			if(!broke)
			(**pxmisw->probes)(nxv, nxp, xmibus->bus_num, 
					   xminode, xmidata, xmibus);
			alive = 1;
		}
		if(!alive) 
			xminotconf(nxv, nxp, xmibus->bus_num, xminode, xmidata);
	}
	
	/* 
	 * The following loop cleans up any errors that might
	 * have gotten set when we probed the XMI 
	 */
	nxv = xmidata->xmivirt;
	for(xminode = 0; xminode < MAX_XMI_NODE; xminode++, nxv++) {
		if(xmidata->xminodes_alive & (1 << xminode)) {
			xbe = RDCSR( LONG_32, xmibus, &nxv->xmi_xbe);
			xbe &= ~XMI_XBAD;
			WRTCSR( LONG_32, xmibus, &nxv->xmi_xbe, xbe);
		}
	}
}

xmierror(xmibus)
	struct bus *xmibus;
{
	int xminode, xminum;
	short xmi_dtype;
	volatile struct xmi_reg	*nxv;	/* virtual pointer to XMI node */
	register struct xmidata *xmidata;

	xminum = xmibus->bus_num;
	xmidata = (struct xmidata *)xmibus->bus_xmidata;
	nxv = xmidata->xmivirt;
	for(xminode = 0; xminode < MAX_XMI_NODE; xminode++, nxv++) {
		if(xmidata->xminodes_alive & (1 << xminode)) {
			xmi_dtype = RDCSR( LONG_32, xmibus, &nxv->xmi_dtype);
			if(xmi_dtype == XMI_XBI || xmi_dtype == XMI_XBIPLUS)
				if(!xbi_check_errs(xminum, xminode, nxv))
					panic("xbi error");
		}
	}
}

/* function: scan xmi and log any pending vaxbi errors found. */
log_xmi_bierrors(xminum)
	int xminum;
{
	int xminode;
	register int binumber;
	u_int xmi_dtype;
	volatile struct xmi_reg	*nxv;	/* virtual pointer to XMI node */
	register struct xmidata *xmidata;
	struct bus *xmibus;
	u_int err = 0;
#ifdef TODO
	register struct bidata *bid;
	extern struct bidata bidata[];
#endif /* TODO */

	xmidata = get_xmi(xminum); 
	xmibus = xmidata->bus;
	nxv = xmidata->xmivirt;

	for(xminode = 0; xminode < MAX_XMI_NODE; xminode++, nxv++) {
		if(xmidata->xminodes_alive & (1 << xminode)) {
			xmi_dtype = RDCSR( LONG_32, xmibus, &nxv->xmi_dtype);
			if(xmi_dtype == XMI_XBI || xmi_dtype == XMI_XBIPLUS) {
				binumber = xminode + (xminum << 4);
#ifdef TODO
				bid = &bidata[binumber];
				if(bid->binodes_alive) 
					err |=log_bierrors(binumber);
#endif /* TODO */
			}
		}	
	}
	/* propogate any bi errors back up */
	return(err);
}

void
xmisetvec(xmibus)
	struct bus *xmibus;
{
	register int (**ivec)();
	vm_offset_t vecaddr;

	if((int)xmibus->intr == -1)
		panic("xmisetvec: no xmi intr vector");

	for(ivec = xmibus->intr; *ivec; ivec++) {
		if(allocvec(1, &vecaddr) != KERN_SUCCESS)
			panic("xmisetvec: allocvec");
		intrsetvec(vecoffset(vecaddr), *ivec, xmibus);
	}

}

/*
 * xmidev_vec(): To set up XMI device interrupt vectors.
 * It is called with 4 parameters:
 *	xminum:	the XMI number that the device is on
 *	xminode: the XMI node number of the device
 *	level:  the offset corresponding to the interrupt priority level
 *		to start at.  See xmireg.h: LEVEL{14,15,16,17}.
 *	controller: the controller structure (for names of interrupt routines)
 */

xmidev_vec(xminum, xminode, level, ctlr)
	int xminum, xminode, level;
	struct controller *ctlr;
{
	register int (**ivec)();
	register vm_offset_t vecaddr;
	register struct xmidata *xmidata;

	xmidata = get_xmi(xminum);
	for(ivec = ctlr->intr; *ivec; ivec++) {
		vecaddr = (vm_offset_t)
			(SCB_XMI_VEC_ADDR(xmidata, xminum, xminode, level));

		intrsetvec(vecoffset(vecaddr), *ivec, ctlr);
		level += XMIVECSIZE;
	}
}

/* function: scan xmi and log any pending errors found. */
log_xmierrors(xminum)
	int xminum;
{
	int xminode;
	u_int xmi_dtype;
	int err = 0;
	volatile struct xmi_reg	*nxv;	/* virtual pointer to XMI node */
	register struct xmidata *xmidata;
	struct bus *xmibus;
	
	xmidata = get_xmi(xminum);
	xmibus = xmidata->bus;
	nxv = xmidata->xmivirt;
	for(xminode = 0; xminode < MAX_XMI_NODE; xminode++, nxv++) {
		if(xmidata->xminodes_alive & (1 << xminode)) {
			xmi_dtype = RDCSR( LONG_32, xmibus, &nxv->xmi_dtype);
			if(xmi_dtype == XMI_XBI || xmi_dtype == XMI_XBIPLUS) {
				err |= xbi_check_errs(xminum, xminode, nxv);
			}
		}
	}
	/* propogate any bi errors back up */
	return(err);
}

xmisst(nxv,bus)
	volatile struct	xmi_reg *nxv;
	struct	bus	*bus;
{
	register struct xmidata *xmidata;
	volatile struct xmi_reg *cpunode = 0;
	struct bus *xmibus;
	int totaldelay;
	int xbe;
	int s;
	int xminum;

	s = splbio();

	if (!strcmp(bus->bus_name, "ci")) 
		xminum = bus->connect_num;
	else if (!strcmp(bus->bus_name, "xmi")) 
		xminum = bus->bus_num;
	else
		panic("xmisst: invalid bus name\n");

	xmidata = get_xmi(xminum);
    	cpunode = xmidata->cpu_xmi_addr;
	if(!cpunode) 
		panic("xmisst: invalid xmi address\n");
	
	xmibus = xmidata->bus;
	xbe = RDCSR( LONG_32, xmibus, &nxv->xmi_xbe);
	xbe = (xbe & ~XMI_XBAD) | XMI_NRST;
	WRTCSR( LONG_32, xmibus, &nxv->xmi_xbe, xbe);
	/* need to give time for XMI bad line to be set */
	DELAY(10000);
	
	/* wait for XMI_XBAD line to be deasserted.  or 10 seconds.*/
	totaldelay = 1000;
loop1:	
	xbe = RDCSR( LONG_32, xmibus, &cpunode->xmi_xbe);
	if((xbe & XMI_XBAD) && (totaldelay-- > 0)) {
		DELAY(10000);
		goto loop1;
	}

	/* Wait for the self tests to finish (10 seconds) */
	totaldelay = 1000;
loop2:	
	xbe = RDCSR( LONG_32, xmibus, &nxv->xmi_xbe);
	if(((xbe & XMI_ETF) || (xbe & XMI_STF)) && (totaldelay-- > 0)) {
		--totaldelay;
		DELAY(10000);
		goto loop2;
	}

	WRTCSR( LONG_32, xmibus, &nxv->xmi_xbe, (xbe & ~(XMI_XBAD | XMI_NRST)));
	splx(s);
	return(totaldelay > 0);
}
#endif /* __alpha */
/* END OF ALPHA MAILBOX CODE */

/* START OF VAX AND MIPS CODE */
#if (__vax | __mips)
void
xmisetvec(xmibus)
	struct bus *xmibus;
{
	int i;
	struct xmidata *xmidata;

	xmidata = get_xmi(xmibus->bus_num);
	
	for(i = 0; xmierr_dispatch[i].bus_num != xmibus->bus_num ; i++) 
		if(xmierr_dispatch[i].bus_num == -1) 
			panic("xmisetvec: no vector");
	
	*(xmidata->xmivec_page + (XMIEINT_XMIVEC / 4)) =
		scbentry(xmierr_dispatch[i].bus_vec, SCB_ISTACK);
}

void
xmiconf_reset(xmibus)
	struct bus *xmibus;
{
	int xminum = xmibus->bus_num;
	volatile struct xmi_reg	*nxv;	/* virtual pointer to XMI node */
	volatile struct xmi_reg	*nxp;	/* physical pointer to XMI node */
	register int xminode;
	register struct xmisw *pxmisw;
	register struct xmidata *xmidata;
	register int i;
	u_int xbe;

	if((xmidata = get_xmi(xminum)) == (struct xmidata *)0)
		xmidata = head_xmidata;

	nxp = xmidata->xmiphys;
	
	/* see if we need to allocate pte's */
	/*
	 * Set up initial virtual address for xmi node space.
	 * This is a bit of a misnomer. On Vaxes XMI node space is mapped
	 * and thus accesses via real virtual addresses. On mips XMI node
	 * is accessible vis KSEG0 and KSEG1, so we use a KSEG1 "virtual"
	 * address which really is a direct translation of the physical.
	 */

	nxv = xmi_start;
#ifdef vax 
	nxv += (xminum * MAX_XMI_NODE);
#endif /* _vax */
	xmidata->xmivirt = nxv;

	printf("xmi %d at address 0x%x\n", xminum, nxp);
	xmidata->xminodes_alive = 0;

	/* figure out cpu node from xmi interrupt dst. reg */
	xmidata->cpu_xmi_addr = nxv + (ffs(xmidata->xmiintr_dst) - 1);

	/* If on first page of scb, do not change the first 64
	 * vectors, as these are the standard arch defined vectors.
	 * On other pages, initialize all vectors.
	 */

	/* initialize SCB */
	i = (SCB_XMI_ADDR(xmidata) == &scb.scb_stray) ? 64 : 0;
	for(; i < NXMI_VEC; i++) {
#ifdef __vax
		*(SCB_XMI_ADDR(xmidata) + i) = scbentry(&catcher[i*2], SCB_ISTACK);
#endif /* _vax */
#ifdef __mips
		*(SCB_XMI_ADDR(xmidata) + i) = scbentry(stray, 0);
#endif /* __mips */
	}

	/* set bus error interrupt vector */
	xmisetvec(xmibus);

	for(xminode = 0; xminode < MAX_XMI_NODE; xminode++, nxv++) {
		short dtype;
#ifdef __vax
		/* get physical address to map */
		nxp = (struct xmi_reg *) cpup->nexaddr(xminum, xminode);

		/* map xmi node space */
		/* 
		 * XMI node space is not mapped on mips or 
		 * mailbox based machines
		 */
		nxaccess(nxp,&Sysmap[btop((int)(nxv) & ~VA_SYS)], XMINODE_SIZE);
#endif /* _vax */
		/* xmi node alive ??? */
		if(BADADDR((caddr_t) nxv, sizeof(long), xmibus)) 
			continue;

		dtype = (short)(nxv->xmi_dtype);
		for(pxmisw = xmisw; pxmisw->xmi_type; pxmisw++) { 
			if(pxmisw->xmi_type == dtype ) {
				xmidata->xmierr[xminode].pxmisw = pxmisw;
				xmidata->xminodes_alive |= (1 << xminode);

				if(pxmisw->xmi_flags & XMIF_SST) {
					nxv->xmi_xbe = 
						(nxv->xmi_xbe & ~XMI_XBAD)|XMI_NRST;
				}
				break;
			}
		}
		if(pxmisw->xmi_type == 0)
			printf ("xmi %d node %d unsupported dev type 0x%x\n",
				xminum, xminode, dtype);
	}
	DELAY(10000);	/* need to give time for XMI bad line to be set */
}

	/* Wait for reset of xmi nodes to take effect. */

xmiconf_wait(xmibus)
	struct bus *xmibus;
{
	int xminumber = xmibus->bus_num;
	volatile struct xmi_reg	*nxv;	/* virtual pointer to XMI node */
	volatile struct xmi_reg	*nxp;	/* physical pointer to XMI node */
	register int xminode;
	register struct xmisw *pxmisw;
	register struct xmidata *xmidata;
	volatile struct xmi_reg *cpunode = 0;
	/* Wait cumulative up to 20 seconds.
	   For extra safety, this is double the spec value. */
	int totaldelay = 2000;

	xmidata = get_xmi(xminumber);
	nxv = xmidata->xmivirt;
	for(xminode = 0; xminode < MAX_XMI_NODE; xminode++, nxv++) {
		if( !(xmidata->xminodes_alive & (1 << xminode)))
			continue;
		pxmisw = xmidata->xmierr[xminode].pxmisw;
		/* wait here for up to remaining count time
		   or until device is reset.  */
		if (pxmisw->xmi_flags&XMIF_SST) {
			cpunode = xmidata->cpu_xmi_addr;
			while((cpunode->xmi_xbe & XMI_XBAD) && 
			      (totaldelay-- > 0)) {
				DELAY(10000);
			}
			while((nxv->xmi_xbe&(XMI_ETF|XMI_STF)) && 
			      (totaldelay-- > 0)) {
				DELAY(10000);
			}
			nxv->xmi_xbe = nxv->xmi_xbe & ~(XMI_XBAD | XMI_NRST);
		}
	}
}

	/* Do config of nodes. */

xmiconf_conf(xmibus)
struct bus *xmibus;
{
	int xminumber=xmibus->bus_num;
	struct xmi_reg	*nxv;	/* virtual pointer to XMI node */
	struct xmi_reg	*nxp;	/* physical pointer to XMI node */
	register int xminode;
	register struct xmisw *pxmisw;
	register struct xmidata *xmidata;
	register int i;
	int broke;
	int alive;
	int totaldelay;

	xmidata = get_xmi(xminumber);

	/* do config of devices, adapters, controllers, etc. */
	nxv = xmidata->xmivirt;
	for(xminode = 0; xminode < MAX_XMI_NODE; xminode++,nxv++) {
	    if( !(xmidata->xminodes_alive & (1 << xminode)))
		continue;
	    pxmisw = xmidata->xmierr[xminode].pxmisw;
	    nxp = (struct xmi_reg *) cpup->nexaddr(xminumber,xminode);

	    broke = (nxv->xmi_xbe & XMI_ETF) || (nxv->xmi_xbe & XMI_STF);
	    alive = 0;
	    if (pxmisw->xmi_flags & XMIF_CONTROLLER) {
		    alive |= xmi_config_con(nxv, nxp, xminumber,
					    xminode, xmidata, xmibus);
	    } else {
		    /* ADAPTERS come in here also */
		    if(pxmisw->xmi_flags & XMIF_NOCONF) 
			    printf ("%s at xmi%d node %d%s",
				    pxmisw->xmi_name, xminum, xminode, 
				    (broke ? " is broken, continuing!\n" : "\n"));
		    (**pxmisw->probes)(nxv, nxp, xminumber,
				       xminode, xmidata, xmibus);
		    alive = 1;
	    }
            if(!alive) 
		    xminotconf(nxv, nxp, xminumber, xminode, xmidata);
    }
	/* The following loop cleans up any errors that might
	   have gotten set when we probed the XMI */
	nxv = xmidata->xmivirt;
	for(xminode = 0; xminode < MAX_XMI_NODE; xminode++, nxv++) {
		if (xmidata->xminodes_alive & (1 << xminode)) 
			nxv->xmi_xbe = nxv->xmi_xbe & ~XMI_XBAD;
	}
}

/*
 * xmi errors -- currently the only device that will interrupt
 * here are the XBI and XBI+.
 */
xmierror(xminum)
	int xminum;
{
	int xminode;
	struct xmi_reg	*nxv;	/* virtual pointer to XMI node */
	short xmi_dtype;
	register struct xmidata *xmidata;
	
	xmidata = get_xmi(xminum); 
	nxv = xmidata->xmivirt;
	for(xminode = 0; xminode < MAX_XMI_NODE; xminode++, nxv++) {
		if(xmidata->xminodes_alive & (1 << xminode)) {
			if(((xmi_dtype = (short)(nxv->xmi_dtype)) == 
			   XMI_XBI) || (xmi_dtype == XMI_XBIPLUS)) {
				if(!xbi_check_errs(xminum, xminode, nxv))
					panic("xbi error");
			}
		}
	}
}

/* function: scan xmi and log any pending vaxbi errors found. */
log_xmi_bierrors(xminum)
	int xminum;
{
	int xminode;
	register int binumber;
	u_int xmi_dtype;
	struct xmi_reg	*nxv;	/* virtual pointer to XMI node */
	register struct xmidata *xmidata;
	struct bus *xmibus;
	u_int err = 0;
	register struct bidata *bid;
	extern struct bidata bidata[];

	xmidata = get_xmi(xminum); 
	nxv = xmidata->xmivirt;

	for(xminode = 0; xminode < MAX_XMI_NODE; xminode++, nxv++) {
		if(xmidata->xminodes_alive & (1 << xminode)) {
			if((xmi_dtype = (short)(nxv->xmi_dtype)) == XMI_XBI ||
			   xmi_dtype == XMI_XBIPLUS) {
				binumber = xminode + (xminum << 4);
				bid = &bidata[binumber];
				if(bid->binodes_alive) 
					err |=log_bierrors(binumber);
			}
		}	
	}
	/* propogate any bi errors back up */
	return(err);
}

/*
 * xmidev_vec(): To set up XMI device interrupt vectors.
 * It is called with 4 parameters:
 *	xminum:	the XMI number that the device is on
 *	xminode: the XMI node number of the device
 *	level:  the offset corresponding to the interrupt priority level
 *		to start at.  See xmireg.h: LEVEL{14,15,16,17}.
 *	ctlr:	the controller structure (for names of interrupt routines)
 */

xmidev_vec(xminum, xminode, level, ctlr)
	int xminum, xminode, level;
	struct controller *ctlr;
{
	register int (**ivec)();
	register int (**addr)();    /* double indirection neccessary to keep
				       the C compiler happy */
	register struct xmidata *xmidata;

	xmidata = get_xmi(xminum); 
	for(ivec = ctlr->intr; *ivec; ivec++) {
		addr = (int (**)())
			(SCB_XMI_VEC_ADDR(xmidata, xminum, xminode, level));

		*addr = scbentry(*ivec, SCB_ISTACK);
		level += XMIVECSIZE;
	}
}

xmisst(nxv)
volatile struct	xmi_reg *nxv;
{
	int totaldelay;
	int ret = 0;
	int s;
	register struct xmidata *xmidata;
	volatile struct xmi_reg *cpunode=0;

	s= splbio();
	nxv->xmi_xbe = (nxv->xmi_xbe & ~XMI_XBAD) | XMI_NRST;

	/* need to give time for XMI bad line to be set */
	DELAY(10000);
	
	xmidata = head_xmidata;

	while(xmidata) {
		if ((xmidata->xminum != -1) &&
		    (nxv >= xmidata->xmivirt) && 
		    (nxv < (xmidata->xmivirt +16)))
		    	cpunode = xmidata->cpu_xmi_addr;
		xmidata = xmidata->next;
	}

	if (!cpunode) panic("invalid xmi address");
	
	/* wait for XMI_XBAD line to be deasserted.  or 10 seconds.*/
	totaldelay = 1000;
	while((cpunode->xmi_xbe & XMI_XBAD) && (totaldelay-- > 0)) {
		DELAY(10000);
	}
/*
 * Wait for the self tests to finish (10 seconds)
 */
	totaldelay = 1000;
	while (((nxv->xmi_xbe & XMI_ETF) || (nxv->xmi_xbe & XMI_STF))
	     &&(totaldelay > 0)) {
		--totaldelay;
		DELAY(10000);
	}
	nxv->xmi_xbe = (nxv->xmi_xbe & ~(XMI_XBAD | XMI_NRST));
	if (totaldelay > 0)
		ret = 1;

	splx(s);
	return (ret);
}
#endif /* __vax | __mips */
/* END OF VAX AND MIPS CODE */
