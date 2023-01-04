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
static char	*sccsid = "@(#)$RCSfile: autoconf.c,v $ $Revision: 1.2.3.6 $ (DEC) $Date: 1992/07/08 08:37:08 $";
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from autoconf.c	2.7	(ULTRIX) 3/30/90";
 */

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */

/*
 * Modification History: autoconf.c
 *
 * 7-SEP-91  -- harrigan
 *      Removed changes of 30-sep-91, as changes .h file fixed problems.
 *
 * 06-Mar-91 -- map (Mark Parenti)
 *	Modify to use new I/O data structures and new configuration mechanism.
 *
 * 05-Oct-90 -- burns
 *	Move to OSF/1.
 *
 * 10-Aug-89 -- afd
 *	In ib_config_dev, added "ui" (ptr to uba_device struct) as a parameter
 *	to probe routine.  This is for systems that allow more than one
 *	of a given device type.
 *
 *	In ib_config_conf, added "um" (ptr to uba_ctlr struct) as a parameter
 *	to probe routine.  This is for systems that allow more than one
 *	of a given controller type.
 *
 * 20-Feb-89 -- Kong
 *	Changed ib_config_dev to set up the phyaddr field.
 *	Added Unibus, Qbus support here.
 *
 * 01-Feb-89 -- Kong
 *	Moved over VAX-like configuration routines.
 *
 * 13-Jan-89 -- Kong
 *	Moved pmax specific configuration routines to kn01.c.  Configuration
 *	is called through the system switch table.
 *
 * 09-Nov-88 -- afd
 *	Examine the "systype" from the PROM & log what we are (in configure()).
 *
 * 06-Sep-88 -- afd
 *	Changed autoconfiguration messages from "cprintf" to "printf".
 *
 */


#include <cpus.h>
#include <machine/pmap.h>
#include <sys/param.h>
#include <sys/systm.h>

#include <machine/cpu.h>
#include <machine/fpu.h>
#include <machine/hwconf.h>
#include <hal/scb.h>
#include <hal/cpuconf.h>
#include <io/common/devdriver.h>

extern int cpu;
extern struct device	device_list[];

struct bus *sysbus;

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
 * The following several variables are related to
 * the configuration process, and are used in initializing
 * the machine.
 */
int	cold;		/* if 1, still working on cold-start */
int	autoconf_cvec;	/* global for interrupt vector from probe routines */
int	autoconf_br;	/* global for IPL from probe routines */
int	(*autoconf_intr)();	/* interrupt handler */
u_int	autoconf_csr;	/* csr base address in k1seg */

/*
 * cache sizes
 */
unsigned icache_size;
unsigned dcache_size;

/*
 * coprocessor revision identifiers
 */
unsigned cputype_word;		/* PRId word */
unsigned fptype_word;
extern unsigned cpu_systype;	/* systype word in boot PROM */


struct imp_tbl cpu_imp_tbl[] = {
	{ "MIPS R2000 Processor Chip",			0 },
	{ "MIPS R2000 Processor Chip",			1 },
	{ "MIPS R2000A Processor Chip",			2 },
	{ 0,						0 }
};

struct imp_tbl fp_imp_tbl[] = {
	{ "MIPS R2360 Floating Point Board",		1 },
	{ "MIPS R2010 VLSI Floating Point Chip",	2 },
	{ 0,						0 }
};

coproc_find()
{
	char *imp_name();
	union rev_id ri;
	extern char mfc0_start[], mfc0_end[];

	hwconf.cpu_processor.ri_uint = ri.ri_uint = cputype_word = 
								get_cpu_irr();
	if (ri.ri_imp == 0) {
		ri.ri_majrev = 1;
		ri.ri_minrev = 5;
	}
	printf("cpu0 ( version %d.%d, implementation %d )\n",
	    ri.ri_majrev, ri.ri_minrev,ri.ri_imp);
#ifdef oldmips
#ifndef SABLE
	/*
	 * Make sure the PROBE_BUG option and the mfc0 assembler option
	 * are turned on for the old 1.5 chips.
	 */
	if (ri.ri_majrev < 2) {
#ifndef PROBE_BUG
		panic("Kernel must be compiled with -DPROBE_BUG for 1.5 revision cpu chip");
#endif /* !PROBE_BUG */
		if ((mfc0_end - mfc0_start) <= 4)
			panic("Kernel must be assembled with -Wb,-mfc0 for 1.5 revision cpu chip");
	}
#endif /* !SABLE */
	
	/*
	 * TODO:
	 *	check cpu_config register
	 *	print a message about vme or local memory
	 */
#ifdef SABLE
	hwconf.cpubd_config = CONFIG_NOCP1|CONFIG_NOCP2|CONFIG_POWERUP;
#else /* !SABLE */
	hwconf.cpubd_config = *(char *)PHYS_TO_K1(CPU_CONFIG);
#endif /* SABLE */
#endif /* oldmips */

#ifdef oldmips
/*	This is out....if you don't have floating point you may hang */
	if (hwconf.cpubd_config & CONFIG_NOCP1) {
		fptype_word = 0;
		hwconf.fpu_processor.ri_uint = 0;
		printf("No floating point processor\n");
	} else {
		hwconf.fpu_processor.ri_uint =
#endif /* oldmips */
		ri.ri_uint = fptype_word = get_fpc_irr();
		fptype_word &= IRR_IMP_MASK;
		printf("fpu0 ( version %d.%d, implementation %d )\n",
	    		ri.ri_majrev, ri.ri_minrev,ri.ri_imp);
		fp_init();
#ifdef oldmips
	}
#endif /* oldmips */
}

char *
imp_name(ri, itp)
union rev_id ri;
struct imp_tbl *itp;
{
	for (; itp->it_name; itp++)
		if (itp->it_imp == ri.ri_imp)
			return(itp->it_name);
	return("Unknown implementation");
}

int fpu_inited;

fp_init()
{
	int led;

	wbflush();
	DELAY(10);
	wbflush();
	DELAY(10);
	set_fpc_csr(0);
	fpu_inited++;
}

/*
 * Cpu board serial number
 */
machineid()
{
#ifdef TODO
	fill this in based on cpu board id ??
#endif /* TODO */
	return (123);
}

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
int bustype;
caddr_t binfo;
struct bus *bus;
{
    /* Currently only support ibus as system bus */
    if(bustype != -1)
	    panic("ibus not system bus");
    bus->bus_type = BUS_IBUS;
    bus->alive = 1;
    printf("%s%d at nexus\n", 
	   bus->bus_name, bus->bus_num);

    /*
     * Lower IPL.
     */
    splnone();

    /* Call machine specific configuration routine as ibus is */
    /* used by several different machines.		      */
    switch(cpu) {
	  case DS_3100:
	    kn01config_devices(bus);
	    break;

	  case DS_5100:
	    kn230_config_devices(bus);
	    break;

          case DS_5500:
            kn220config_devices(bus);
            break;

	  default:
	    printf("Unknown bus type in ibusconfl1\n");
	    break;
    }
}

ibusconfl2(bustype, binfo, bus)
int bustype;
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
int scb_vec_addr;
{
	register struct device *device;
	register struct controller *ctlr;
	register struct driver *drp;
	int savectlr, savebus;
	char *savectname, *savebusname;
	int (**ivec)();
	int i;
	int found = 0;

	if ((ctlr = get_ctlr(name, slot, bus->bus_name, bus->bus_num)) ||
	    (ctlr = get_ctlr(name, -1, bus->bus_name, -1)) ||
	    (ctlr = get_ctlr(name, -1, "*", -99))  )
		found++;

	if (found == 0) {
	    return(0);
	}
	 

	savebus = ctlr->bus_num;
	savebusname = ctlr->bus_name;
	ctlr->bus_name = bus->bus_name;
	ctlr->bus_num = bus->bus_num;

	drp = ctlr->driver;
/* DAD - do this here for now.  Need to set up the vector since
 * sz_siiprobe goes and looks for devices
 */
	if (scb_vec_addr)
	    ibcon_vec(scb_vec_addr, ctlr);

	i = (*drp->probe)(nxv, ctlr);
	if (i == 0) {
	    ctlr->bus_num = savebus;
	    ctlr->bus_name = savebusname;
	    return(0);
        }
	ctlr->slot = slot;
	ctlr->alive = 1;
	ctlr->addr = (char *)nxv;
	(void)svatophys(ctlr->addr, &ctlr->physaddr);
	drp->ctlr_list[ctlr->ctlr_num] = ctlr;
	config_fillin(ctlr);
	printf("\n");
	if (drp->cattach)
		(*drp->cattach)(ctlr);
	Cprintf("ib_config_cont: Calling conn_ctlr\n");
	conn_ctlr(bus, ctlr);
	

	Cprintf("ib_config_cont: Searching device list\n");
	for (device = device_list;
	     device->dev_name;
	     device++) {


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
		device->alive = 1;
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
		else {
		    printf("%s%d at %s%d",
			device->dev_name, device->logunit,
			drp->ctlr_name, ctlr->ctlr_num);
		}
		Cprintf("tc_config_cont: calling attach\n");
		if (drp->dattach)
			(*drp->dattach)(device);
		printf("\n");
	    }
	    else {
		    device->ctlr_num = savectlr;
		    device->ctlr_name = savectname;
	    }
	}
	return (found);
}

ibcon_vec(scb_vec_addr, ctlr)
int scb_vec_addr;
struct controller *ctlr;
{
    register int (**ivec)();
    register int (**addr)();	/* double indirection neccessary to keep
			   	   the C compiler happy */
    ivec = ctlr->intr;
    addr = (int (**)())scb_vec_addr;
    *addr = scbentry(*ivec,SCB_ISTACK);
    return;
}

config_fillin(ctlr) 
register struct controller *ctlr;
{
	register int bitype = 0;
	register int xmitype = 0;
	register int tctype = 0;

	if (strcmp(ctlr->bus_name,"vaxbi") == 0) {
		bitype = 1;
	}
	else if (strcmp(ctlr->bus_name,"xmi") == 0) {
		xmitype = 1;
	}
	else if (strcmp(ctlr->bus_name,"tc") == 0) {
		tctype = 1;
	}
	printf("%s%d at %s%d", 
			ctlr->ctlr_name, ctlr->ctlr_num,
			ctlr->bus_name, ctlr->bus_num);
	if (bitype|xmitype)
		printf(" node %d",ctlr->slot);
	if (tctype)
		printf(" slot %d",ctlr->slot);
}

/*
 * Configure swap space and related parameters.
 */
swapconf()
{
	if (!cold)			/* in case called for TODO device */
		return;
	if (dumpdev == NODEV)
		return;
#ifdef	mips
	dumplo = 0;
#else
	if (dumplo == 0 && bdevsw[major(dumpdev)].d_psize) {
		BDEVSW_PSIZE(major(dumpdev), dumpdev, dumplo);
		dumplo = dumplo - ctod(physmem) - btodb(BLKDEV_IOSIZE);
	}
	if (dumplo < 0)
		dumplo = 0;
#endif
}
