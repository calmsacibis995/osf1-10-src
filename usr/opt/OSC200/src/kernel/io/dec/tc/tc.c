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
static char *rcsid = "@(#)$RCSfile: tc.c,v $ $Revision: 1.2.21.7 $ (DEC) $Date: 1994/01/24 22:54:42 $";
#endif 

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
 * Modification History: tc.c
 *
 * 16-Sep-91	Andrew Duane
 *	Modified for support of ALPHA FLAMINGO
 *		and a bunch of portability fixes.
 *
 * 20-Jun-91	Mark Parenti
 *	Add TC devices which are not configured to the configuration
 *	tree and mark them as ALV_PRES. This allows sizer to add them
 *	to the config file.
 *	Fix a problem where we try to config empty slots.
 *	
 * 11-Apr-91	Mark Parenti
 *	Use new alive bit definitions.
 *	Fix bus support.
 *
 * 12-Mar-91	Fred Canter
 *	MIPS C 2.20+
 *
 * 01-Mar-91	Mark Parenti
 *	Modified to use the new I/O data structures and configuration.
 *
 * 21-Jan-91	Randall Brown
 *	Changed interface to routine tc_isolate_memerr(), now has
 *	a tc_memerr_status struct passed to it.
 *
 * 06-Dec-90	Randall Brown
 *	Added the interface routine tc_isolate_memerr().
 *
 * 15-Oct-90	Randall Brown
 *	Changed the tc_probe() routine so that it looks at the ROM header
 *	correctly for 2-byte and 4-byte wide ROMs.
 *
 *  6-Sep-90	Randall Brown
 *	Changed slot_order to be config_order. 
 *
 *  5-Jul-90	Randall Brown
 *	Added the routine tc_module_name() for drivers to determine
 *	the name of the specific option module.
 *
 * 13-Mar-90	Randall Brown
 *	Changed the probe routine to look for the ROM at both 
 *	offset 0 and 0x3c0000.
 *
 * 22-Jan-90	Randall Brown
 *	Created this file for TURBOchannel support
 *
 */
#include <machine/cpu.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/map.h>
#include <sys/buf.h>
#include <sys/dk.h>
#include <sys/user.h>			/* gets time.h and debug.h too */
#include <sys/proc.h>
#include <sys/vmmac.h>
#include <sys/conf.h>
#include <dec/binlog/errlog.h>
#include <sys/config.h>
#include <kern/kalloc.h>
#include <hal/cpuconf.h>

#include <io/dec/tc/tc.h>

#include <io/common/handler.h>
#include <io/common/devdriver.h>

static tc_probed = 0;
struct tc_slot 	tc_slot[TC_IOSLOTS];	/* table with IO device info */
vm_offset_t	tc_slotaddr[TC_IOSLOTS];/* table with slot address info */

/* 
 * Define the loadable framework extension.  Routines live in
 * tc_loadable.c
 */

extern struct bus_framework TC_loadable_framework;

/* offsets to look for rom in address space (0 is a valid offset) */
vm_offset_t	tc_romoffset[] = { TC_ROMOFFSET, 0 };
#define	NROMOFFSETS	(sizeof(tc_romoffset)/sizeof(tc_romoffset[0]))

struct tc_sw tc_sw;
struct	bus_funcs	tc_funcs;

extern	struct device	device_list[];

extern int confdebug;		/* debug variable for configuration code */


/* Define debugging stuff. */
#define DEBUG
#ifdef DEBUG
#define Cprintf if(confdebug)printf
#define Dprintf if( confdebug >= 2 )printf
#else
#define Cprintf ;
#define Dprintf ;
#endif


extern stray();
static void tc_map_init();

tc_init()
{
    int i;

    /*
     * Clear the tc_slot table to "safe" initialized values
     */
    for (i = 0; i < TC_IOSLOTS; i++) {
	strcpy(tc_slot[i].modulename,"");
	strcpy(tc_slot[i].devname,"");
	tc_slot[i].slot = -1;
	tc_slot[i].module_width = 0;
	tc_slot[i].intr = stray;
	tc_slot[i].unit = -1;
	tc_slot[i].physaddr = 0;
	tc_slot[i].class = 0;
	tc_slot[i].intr_b4_probe = 0;
	tc_slot[i].intr_aft_attach = 0;
	tc_slot[i].adpt_config = 0;
    }
}

/*
 * tc_probe()
 *
 * Determine what is really on the system by doing a badaddr on
 * each of the variable IO option slots and reading the module ROM.
 */
tc_probe(verbose)
    int verbose;			/* work silently if not set */
{
    register int i, j, k, l;
    int s;
    int found;
    u_long rom_width;
    u_long rom_stride;
    u_long module_width;
    vm_offset_t romoffset;
    int good_rom, bad_format;
    char curr_module_version[TC_ROMNAMLEN + 1];	/* module version from the ROM */
    char curr_module_vendor[TC_ROMNAMLEN + 1];	/* module vendor from the ROM */
    char curr_module_name[TC_ROMNAMLEN + 1];	/* module name from the ROM */
#ifdef __alpha
    u_int *romptr;	/* read a longword at a time, rom_stride = 2 */
#else
    u_char *romptr;	/* read a byte at a time, rom_stride = 4 */
#endif	/* __alpha */
    u_int   testp;	/* test pattern character */
    u_long curr_module_id;		/* module id from the ROM */
    extern struct tc_option tc_option[];

    tc_probed = 1;

    tc_probed = 1;

/* Virtual address of rom byte at slot <i>, adjusted by offsets off1 */
/* (tc_romoffset[]) and off2 (an arbitrary additional field offset). */
/* Convert with PHYS_TO_K1 on MIPS, or map to scratch page on ALPHA. */
/* Also set MINWIDTH; mips: byte-addressable, alpha: longword-addressable */

#ifdef __alpha
#define	MINWIDTH 4		/* must read a longword */
#define ROMADDR(i,off1,off2) \
		(u_int *)PHYS_TO_KSEG(tc_slotaddr[i]+(off1*2)+(off2*2))
#endif	/* __alpha */

#ifdef mips
#define	MINWIDTH 1		/* can read a single byte */
#define ROMADDR(i,off1,off2) \
		(u_int *)PHYS_TO_K1(tc_slotaddr[i]+(off1)+(off2))
#endif	/* mips */

    for (i = TC_IOSLOTS-1; i >= TC_OPTION_SLOTS; i--) {

	    /*
	     * Do badaddr probe on addr tc_slotaddr[i] to see if the
	     * slot still has the device in it. This are normally part
	     * of the system board, but my have been removed to create
	     * a lower cost/size version (try to read 1 byte/longword).
	     */
	    if (BADADDR(ROMADDR(i,0,0), MINWIDTH, 0)) {
		module_width = 1;
		(*tc_sw.clear_errors)();
		wbflush();
		strcpy(tc_slot[i].modulename,"");
		strcpy(tc_slot[i].devname,"");
		tc_slot[i].slot = i;
		tc_slot[i].module_width = 0;
		tc_slot[i].intr = stray;
		tc_slot[i].unit = -1;
		tc_slot[i].physaddr = 0;
		tc_slot[i].class = 0;
		tc_slot[i].intr_b4_probe = 0;
		tc_slot[i].intr_aft_attach = 0;
		tc_slot[i].adpt_config = 0;
		continue;
	    }	
    }

    /*
     * Clear the optional tc_slot table entries to "safe"
     * initialized values.
     */
    for (i = TC_OPTION_SLOTS-1; i >= 0; i--) {
	strcpy(tc_slot[i].modulename,"");
	strcpy(tc_slot[i].devname,"");
	tc_slot[i].slot = -1;
	tc_slot[i].module_width = 0;
	tc_slot[i].intr = stray;
	tc_slot[i].unit = -1;
	tc_slot[i].physaddr = 0;
	tc_slot[i].class = 0;
	tc_slot[i].intr_b4_probe = 0;
	tc_slot[i].intr_aft_attach = 0;
	tc_slot[i].adpt_config = 0;
    }

    /*
     * Start at the TOP of the variable IO option slots and work down.
     * This is done for the sake of options that can take up more than
     * 1 slot (the ROM space will be at the highest slot's address space).
     */
    for (i = TC_OPTION_SLOTS-1; i >= 0; i = i - 1) {

	good_rom = 0;
	bad_format = 0;


	for (j = 0; (j < NROMOFFSETS) && (!good_rom); j++) {

	    romoffset = tc_romoffset[j];

#ifdef __alpha
	Cprintf ("Probing slot %d @ %lx, offset %x\n\r",
		i, tc_slotaddr[i], romoffset);
#endif

	    /*
	     * Do badaddr probe on addr tc_slotaddr[i] to see if the
	     * slot has a module in it (try to read 1 byte/longword).
	     */
	    if (BADADDR(ROMADDR(i,romoffset,0), MINWIDTH, 0)) {
		module_width = 1;
		(*tc_sw.clear_errors)();
		wbflush();
		continue;
	    }

	    /*
	     * Found a module, now read the ROM:
	     * First read the ROM width, stride and module width out
	     * of the "first layer".  Also sanity check the ROM by reading
	     * the test patterns.  If its ok, then from the ROM width &
	     * stride we know how to read the "second" layer", with the
	     * module name and module id.
	     * NOTE: rom_stride is always "4" in the ROM.
	     */
	    rom_width = *ROMADDR(i, romoffset, 0x3e0);
	    rom_width &= 0xff;
	    rom_stride = *ROMADDR(i, romoffset, 0x3e4);
	    rom_stride &= 0xff;
	    module_width = *ROMADDR(i, romoffset, 0x3ec);
	    module_width &= 0xff;
	    testp = *ROMADDR(i, romoffset, 0x3f0);
	    if ((testp & 0xff) != 0x55) {
		module_width = 1;
		bad_format = 1;
		continue;
	    }
	    testp = *ROMADDR(i, romoffset, 0x3f4);
	    if ((testp & 0xff) != 0x00) {
		module_width = 1;
		bad_format = 1;
		continue;
	    }
	    testp = *ROMADDR(i, romoffset, 0x3f8);
	    if ((testp & 0xff) != 0xaa ) {
		module_width = 1;
		bad_format = 1;
		continue;
	    }
	    testp = *ROMADDR(i, romoffset, 0x3fc);
	    if ((testp & 0xff) != 0xff) {
		module_width = 1;
		bad_format = 1;
		continue;
	    }


	    /* Since we have made it through all of the tests, we are  	*/
	    /* dealing with a good rom.					*/
	    good_rom = 1;

	}

	if (good_rom == 0) {
	    if (verbose && bad_format)
		printf ("Module in slot %d has bad ROM format, can't configure it\n", i);
	    continue;
	}

	    
#ifdef __alpha
	/* Since ROM is mapped via "sparse" space, must access as longwords, */
	/* but quadword spaced. Romptr is (u_int *), so make rom_stride 2. */
	rom_stride = 2;

	/* Read in the version from the ROM */
	romptr = ROMADDR(i,romoffset,TC_VERSION_OFFSET);
	for (k = 0, l = 0; k < TC_ROMNAMLEN; k++, l += rom_stride)
	    curr_module_version[k] = (char)romptr[l];
	curr_module_version[TC_ROMNAMLEN] = '\0';

	/* Read in the vendor from the ROM */
	romptr = ROMADDR(i,romoffset,TC_VENDOR_OFFSET);
	for (k = 0, l = 0; k < TC_ROMNAMLEN; k++, l += rom_stride)
	    curr_module_vendor[k] = (char)romptr[l];
	curr_module_vendor[TC_ROMNAMLEN] = '\0';

	/* Read in the name from the ROM */
	romptr = ROMADDR(i,romoffset,TC_NAME_OFFSET);
	for (k = 0, l = 0; k < TC_ROMNAMLEN; k++, l += rom_stride)
	    curr_module_name[k] = (char)romptr[l];
	curr_module_name[TC_ROMNAMLEN] = '\0';
#else
	romptr = (u_char *)ROMADDR(i,romoffset,0);
	/* rom_stride is already 4 */

	/* Read in the version from the ROM */
	for (k = 0, l = TC_VERSION_OFFSET; k < TC_ROMNAMLEN; k++, l += rom_stride)
	    curr_module_version[k] = (char)romptr[l];
	curr_module_version[TC_ROMNAMLEN] = '\0';

	/* Read in the vendor from the ROM */
	for (k = 0, l = TC_VENDOR_OFFSET; k < TC_ROMNAMLEN; k++, l += rom_stride)
	    curr_module_vendor[k] = (char)romptr[l];
	curr_module_vendor[TC_ROMNAMLEN] = '\0';

	/* Read in the name from the ROM */
	for (k = 0, l = TC_NAME_OFFSET; k < TC_ROMNAMLEN; k++, l += rom_stride)
	    curr_module_name[k] = (char)romptr[l];
	curr_module_name[TC_ROMNAMLEN] = '\0';
#endif	/* __alpha */

	Cprintf ("In this slot: name = '%s', vendor = '%s', version = '%s'\n\r",
		curr_module_name, curr_module_vendor, curr_module_version);

	/*
	 * Since the module exists, we will always fill in some of the tc_slot
	 * table (esp. the module name and slot number).  The reason
	 * we do this is to remember what modules are where even if there is no
	 * static driver available to control them.  Later, when a dynamically 
	 * loaded driver is loaded, the tc_slot table can be scanned for
	 * modules it controls.  This is important since 3rd party modules may
	 * not be in the tc_option table and, hence, will not have any bus or
	 * controller structures created during static configuration.
	 */
	strcpy(tc_slot[i].modulename, curr_module_name);
	strcpy(tc_slot[i].version, curr_module_version);
	strcpy(tc_slot[i].vendor, curr_module_vendor);
	tc_slot[i].module_width = module_width;
#ifdef __alpha
	if ((!(strcmp(curr_module_name, "PMAGB-BA"))) ||
	    (!(strcmp(curr_module_name, "PMAG-RO "))) ||
	    (!(strcmp(curr_module_name, "PMAG-JA ")))) {
	    tc_slot[i].physaddr = ((vm_offset_t)(tc_slotaddr[i]) - 0x10000000);
	} else {
	    tc_slot[i].physaddr = tc_slotaddr[i];
	}
#endif	/* __alpha */
#ifdef mips
	tc_slot[i].physaddr = tc_slotaddr[i];
#endif	/* mips */

#ifdef __alpha
	if (kn15aa_is_sandpiper())
		tc_slot[i].slot = i-3;
	else
#endif	/* __alpha */
		tc_slot[i].slot = i;
	tc_slot[i].class = TC_UNKNOWN;

	/*
	 * Look for the module in the tc_option data table to get
	 * the config file name for the controller/device.
	 */
	found = 0;
	for (j = 0; tc_option[j].modname[0] != '\0'; j++) {
	    if (!(strcmp (curr_module_name, tc_option[j].modname))) {
		/*
		 * Found it, fill in the tc_slot table
		 */
		strcpy(tc_slot[i].devname, tc_option[j].confname);
		tc_slot[i].intr_b4_probe = tc_option[j].intr_b4_probe;
		tc_slot[i].intr_aft_attach = tc_option[j].intr_aft_attach;
		if (tc_option[j].type == 'A') {
		    tc_slot[i].adpt_config = tc_option[j].adpt_config;
		    tc_slot[i].class = TC_ADPT;
		}
		else
		    tc_slot[i].class = TC_CTLR;
		found = 1;
		break;
	    }
	}
	if ((found == 0) && verbose) {
	    printf ("Module %s not in tc_option data table, can't configure it\n", 
		    curr_module_name);
	    module_width = 1;
	}

    }

    /* turn off parity checking, scatter-gather, and block mode for
     * any option present.  Must do this *after* tc_slot[i].slot has
     * been set since the function we're jumping to may need it.
     */
    for (i = 0; i < TC_IOSLOTS; ++i)
	(void)(*tc_sw.option_control)(i, SLOT_CLEARFLAGS);


}

#undef ROMADDR
#undef MINWIDTH

/*
 *	tc_option_control()
 *	
 *	Takes a pointer to a controller structure and a flags arg to indicate option selection
 *
 */
int
tc_option_control(ctlr,flags)
    struct controller *ctlr;
    int flags;
{
    register int index = (int)ctlr->tcindx;
    int temp = (int)(*tc_sw.option_control)(index,flags);

    return( temp );
}

/*
 *	tc_enable_option()
 *	
 *	Takes a pointer to a controller structure.
 *
 *	This function enables an option's interrupt on the TURBOchannel
 *	to interrupt the system at the I/O interrupt level.  
 *	This is done calling the system specific routine to allow the
 *	option's slot to interrupt the system.
 */
tc_enable_option(ctlr)
    struct controller *ctlr;
{
    register int index = (int)ctlr->tcindx;
    (*tc_sw.enable_option)(index);
}

/*
 *	tc_disable_option()
 *	
 *	Takes a pointer to a controller structure.
 *
 *	This function disables an option's interrupt on the TURBOchannel from
 *	interrupting the system at the I/O interrupt level.  
 *	This is done by calling the system specific routine to reset the 
 *	option's slot from interrupting the system.
 */	
tc_disable_option(ctlr)
    struct controller *ctlr;
{
    register int index = (int)ctlr->tcindx;
    (*tc_sw.disable_option)(index);
}

/*
 *	tc_module_name(ctlr, cp)
 *		struct controller *ctlr;
 *		char cp[TC_ROMNAMLEN];
 *
 *	Takes a pointer to a uba_device struct or a uba_controller struct,
 *  	since the beginning of these structs are identical it will take
 *	a pointer to either one.
 *
 *	This function will fill in the character array 'cp' with the ascii
 *	string of the TURBOchannel option's module name refered to by the
 *	 'ui' pointer.
 *
 *	The function will return a (-1) if it was unable to use the 'cp'
 *	pointer that it was given.
 */

tc_module_name(ctlr, cp)
	struct controller *ctlr;
	char *cp;
{
    register int index = (int)ctlr->tcindx;
    register int i;
    register char *tcp;

#ifdef mips
    /* sanity check the buffer cp[] to verify that it 	*/
    /* can be written into.				*/
    for (i = 0, tcp = cp; i < TC_ROMNAMLEN + 1; i++, tcp++) 
	if (BADADDR(tcp, 1, ctlr))
	    return (-1);
#endif	/* mips */

    bcopy(tc_slot[index].modulename, cp, TC_ROMNAMLEN + 1);
    return (0);
}

tc_addr_to_name(addr, cp)
	vm_offset_t addr;
	char *cp;
{
    register int i;
    register char *tcp;
    vm_offset_t phys;

#ifdef __alpha
    for (i = 0; i < TC_IOSLOTS; i++) {
	if (tc_slotaddr[i] == addr || PHYS_TO_KSEG(tc_slotaddr[i]) == addr) {
		bcopy(tc_slot[i].modulename, cp, TC_ROMNAMLEN + 1);
		return (0);
	}
    }
#else	/* __alpha */
    /* sanity check the buffer cp[] to verify that it 	*/
    /* can be written into.				*/
    for (i = 0, tcp = cp; i < TC_ROMNAMLEN + 1; i++, tcp++) 
	if (BADADDR(tcp, 1, 0))
	    return (-1);

    for (i = 0; i < TC_IOSLOTS; i++) {
	if(svatophys(addr, &phys) == KERN_SUCCESS) {
		if (phys == tc_slotaddr[i]) {
			bcopy(tc_slot[i].modulename, cp, TC_ROMNAMLEN + 1);
			return (0);
		}
	}
    }
#endif	/* __alpha */

    return (-1);
}

char *
tc_slot_to_name(slot)
	int slot;
{
    register int i;

    if (!tc_probed)
	tc_probe(0);
    for (i = 0; i < TC_IOSLOTS; i++) {
	if (tc_slot[i].slot == slot)
	    return (tc_slot[i].modulename);
    }

    return ((char *)-1);
}

char *
tc_slot_to_ctlr_name(slot)
	int slot;
{
    register int i;

    if (!tc_probed)
	tc_probe(0);
    for (i = 0; i < TC_IOSLOTS; i++) {
	if (tc_slot[i].slot == slot)
	    return (tc_slot[i].devname);
    }

    return ((char *)-1);
}

caddr_t
tc_slot_to_addr(slot)
	int slot;
{
    register int i;

    if (!tc_probed)
	tc_probe(0);
    for (i = 0; i < TC_IOSLOTS; i++) {
	if (tc_slot[i].slot == slot)
	    return ((caddr_t)tc_slot[i].physaddr);
    }

    return ((caddr_t) -1);
}

int
tc_get_unit_num(slot)
	int slot;
{
    register int i;

    for (i = 0; i < TC_IOSLOTS; i++) {
	if (tc_slot[i].slot == slot)
	    return (tc_slot[i].unit);
    }

    return (-1);
}
/*
 * Look for a specific option on the system, if its found return its
 *     physical address, else return 0.
 * In particular, this routine is used at "consinit" time, to determine
 *     if there is a graphics module in the system and if so, where it is.
 */
caddr_t
tc_where_option(str)
	char *str;			/* the device name to look for */
{
	register int i;
	register int index;
#ifdef mips
	extern VEC_nofault(), VEC_trap();
	extern  (*causevec[])();
#endif	/* mips */

	extern int cold;

	cold = 1;
#ifdef mips
	causevec[EXC_DBE>>CAUSE_EXCSHIFT] = VEC_nofault;
	tc_probe(0);
	causevec[EXC_DBE>>CAUSE_EXCSHIFT] = VEC_trap;
#else
	if (!tc_probed)
		tc_probe(0);
#endif	/* mips */
	cold = 0;

	/*
	 * Go thru the tc_slot table and look for device in "str".
	 * If found, return its address, else return 0.
	 */
	for (i = 0; tc_sw.config_order[i] != -1; i++) {
	    index = tc_sw.config_order[i];

#ifdef __alpha
	    if (!(strcmp (str, tc_slot[index].devname)))
			return ((caddr_t) PHYS_TO_KSEG(tc_slot[index].physaddr));
#else
	    if (!(strcmp (str, tc_slot[index].devname)))
			return (tc_slot[index].physaddr);
#endif
	}

	return( (caddr_t) 0);
}

/* Stub for tc_do_config. */
void
tc_do_config (struct controller *ctlr_p)

{
   return;
}



/* Stub for tc_get_config. */
int
tc_get_config (struct controller *ctlr_p, uint_t config_item, char *func_type,
	       void *data_p, int handle)

{
   return (0);
}



/*
 * define and initialize the DMA call switch table
 */
struct  bus_bridge_dma_callsw tc_dma_callswitch  = { 0,0,0,0 };
tc_dma_callsw_init(map_alloc_fcn, loadmap_fcn,
                   unloadmap_fcn,map_free_fcn)
unsigned long (*map_alloc_fcn)();
unsigned long (*loadmap_fcn)();
int           (*unloadmap_fcn)();
int           (*map_free_fcn)();
{
     tc_dma_callswitch.dma_alloc = map_alloc_fcn;
     tc_dma_callswitch.dma_load = loadmap_fcn;
     tc_dma_callswitch.dma_unload = unloadmap_fcn;
     tc_dma_callswitch.dma_dealloc = map_free_fcn;
}

extern cpu;


/*	tcconfl1()
 *
 *	TURBOchannel level 1 configuration routine:
 *
 *	This routine probes the option slots and fills in the tc_slot
 *	table with the devices that are found on the system.  It then
 *	turns on interrupts and calls the probe routine for each device.
 *
 *	NOTE: 	Interrupts are enabled after return from this routine.
 */
tcconfl1(bustype, binfo, bus)
int bustype;
caddr_t binfo;
struct bus *bus;
{
    int found;
    register struct controller *ctlr, *ctrl_final;
    register struct bus *busptr;
    register int i, j;
    register int index;		/* current index into tc_slot to config */
    int savebus;
    char *savebusname;
    struct bus *savebushd; 
    struct tc_info info;
    struct bus *firstbus;
    struct bus_bridge_dma_callsw  *current_hw_dma_bus_mapping;


    tc_map_init();
    /* Currently only support TURBOchannel as system bus */
    if(bustype != -1)
	    panic("TURBOchannel not system bus");

/*---------------------------*/
/* Load bus function  table. */
/*---------------------------*/
   tc_funcs.do_config = tc_do_config;
   tc_funcs.get_config = tc_get_config;
   tc_funcs.enable_option = (void(*))tc_enable_option;
   tc_funcs.disable_option = (void(*))tc_disable_option;
   bus->busfuncs = &tc_funcs;

    /* Probe the option slots with printing turned on. */
    tc_probe(1);

    bus->bus_type = BUS_TC;
    bus->alive |= ALV_ALIVE;
    printf("%s%d at nexus\n",
	   bus->bus_name, bus->bus_num);  
   
    /*
     * Indicate in the tc bus structure field "bus_bridge_dma" wheather this
     * tc bus machine supports DMA sg mapping.
     */
    firstbus = bus;
    ctrl_final = (struct controller *)firstbus;
    if (cpu != DEC_3000_300)
      {
      /* Initialize the TC DMA bus bridge mapping structure */
      tc_sg_map_setup();
      /* Init bus pointer to TC DMA bus bridge structure */
      /* This line is disabled for the first submit so that TC sg mapping
       * is not enabled(by default). Turning this on is TBD, KFE after
       * more DMA testing is done.  
       */
      /*  ****** bus->bus_bridge_dma = (&tc_dma_callswitch);  ************* */ 
      bus->bus_bridge_dma = 0x00;
      /* printf(" &dma_callswitch = %lx  %lx \n",&tc_dma_callswitch,tc_dma_callswitch);*/ 
      }
    else
      bus->bus_bridge_dma = 0x00;


    /*
     * Provide support for loadable controllers (and sub-buses) by
     * connected the TURBOchannel loadable framework extension to the
     * current TURBOchannel bus structure.
     */
    if (bus->framework == NULL) {
	bus->framework = &TC_loadable_framework;
    }
    else {
	printf ("Warning: TURBOchannel bus already has loadable framework extension.\n");
    }

    /*
     * Safe to take interrupts now (first clear any old bits in ERR reg).
     */
    (*tc_sw.clear_errors)();
    splnone();

    /*
     * For device found on the system, locate it's controller or bus
     * structure (built from the config file).
     * If it is found, then fill in the tc_slot table.
     *
     * If in the config file a device was assigned to a specific
     * slot #, then we will only match if we find that piece
     * of hardware in the corresponding slot on the system.
     *
     * The default case is that we will assign unit numbers to like devices
     * starting with unit 0 and going thru the devices according to the
     * "config_order" list.
     */
    for (i = 0; tc_sw.config_order[i] != -1; i++) {
	index = tc_sw.config_order[i];
	found = 0;
	/*
	 * If the class = 0 and it's an option slot then there is nothing
	 * present in the slot (onboard options are always present).  If
	 * the class = TC_UNKNOWN then there is a module present but no
	 * entry was found in the tc_options table.  This means that no
	 * static driver exists to handle it so we might as well punt and
	 * go to the next slot.
	 */
	if (((tc_slot[index].class == 0) && (index < TC_OPTION_SLOTS)) ||
	     (tc_slot[index].class == TC_UNKNOWN))
		continue;

	if (tc_slot[index].devname[0] == '\0')
		continue;	/* ALD: this seems reasonable */

	Cprintf("tcconfl1: Calling get_ctlr\n");
	Cprintf("tcconfl1: name = '%s', slot = %d\n",
		tc_slot[index].devname, tc_slot[index].slot);
	/* Get the controller structure.  Check in descending levels of
	 * of specificity (nice word eh).
	 */
	if( (ctlr = get_ctlr(tc_slot[index].devname, /* no wildcards */
			     tc_slot[index].slot,
			     bus->bus_name,
			     bus->bus_num)) ||
	    (ctlr = get_ctlr(tc_slot[index].devname, /* bus num only */
			     tc_slot[index].slot,
			     bus->bus_name,
			     -1)) ||
	    (ctlr = get_ctlr(tc_slot[index].devname, /* bus name & num */
			     tc_slot[index].slot,
			     "*",
			     -99)) ||
	    (ctlr = get_ctlr(tc_slot[index].devname, /* slot num only */
			     -1,
			     bus->bus_name,
			     bus->bus_num)) ||
	    (ctlr = get_ctlr(tc_slot[index].devname, /* slot num & bus num */
			     -1,
			     bus->bus_name,
			     -1)) ||
	    (ctlr = get_ctlr(tc_slot[index].devname, /* slot & bus name/num */
			     -1,
			     "*",
			     -99)) ) {
			
		Cprintf("tcconfl1: Found controller\n");
		if((tc_func_used(ctlr->intr))) /* If already matched */
			continue;
		tc_slot[index].intr = *ctlr->intr;
		tc_slot[index].unit = ctlr->ctlr_num;
		tc_slot[index].param = (caddr_t)ctlr->ctlr_num;
		ctlr->tcindx = (caddr_t)index;
		if (tc_slot[index].class == 0)
			tc_slot[index].class = TC_CTLR;
		tc_slot[index].dev_str = (caddr_t)ctlr;
		if (tc_slot[index].intr_b4_probe)
		    (*tc_sw.enable_option)(index);

		savebus = ctlr->bus_num;
                savebushd = ctlr->bus_hd; 
		savebusname = ctlr->bus_name;
                ctlr->bus_hd = bus; 
		ctlr->bus_name = bus->bus_name;
		ctlr->bus_num = bus->bus_num;

		/* If an adapter, call adpt_config, and skip the rest */
		if (tc_slot[index].class == TC_ADPT) {
			if (tc_slot[index].adpt_config) {
				Cprintf ("tcconfl1: Calling adpt_config: %lx\n", tc_slot[index].adpt_config);
				(*tc_slot[index].adpt_config)(&tc_slot[index],bus,ctlr);
			}
			else {
				printf ("tcconfl1: Warning: adapter %s has no config routine\n",
					tc_slot[index].modulename);
				/* Should this drop through and call tc_config_cont ??? */
			}
			continue;
		}

		/*
		 * Call the tc_config_cont routine for each controller found
		 * The tc_config_cont routine will in turn call the probe
		 * routine for each controller and will call the slave and 
		 * attach routines for each device.
		 */
		Cprintf("tcconfl1: Calling config_cont\n");

#ifdef mips
		if (!(tc_config_cont(PHYS_TO_K1(tc_slot[index].physaddr),
				     tc_slot[index].physaddr, 
				     tc_slot[index].slot, 
				     tc_slot[index].devname, 
				     tc_slot[index].dev_str)))
#else
		if (!(tc_config_cont(PHYS_TO_KSEG(tc_slot[index].physaddr),
				     PHYS_TO_KSEG(tc_slot[index].physaddr),
				     tc_slot[index].slot, 
				     tc_slot[index].devname, 
				     tc_slot[index].dev_str)))
#endif
		{
		    ctlr->bus_num = savebus;
		    ctlr->bus_name = savebusname;
                    ctlr->bus_hd = savebushd; 
		    printf("%s%d not probed\n",
			   tc_slot[index].devname, tc_slot[index].unit);
		    (*tc_sw.disable_option)(index);
		} else {
		    Cprintf("tcconfl1: Calling conn_ctlr\n");
		    conn_ctlr(bus, ctlr);
		    tc_slot[index].flags |= TC_ACTIVE;
		    if (tc_slot[index].intr_aft_attach) {
			(*tc_sw.enable_option)(index);
		    } else {
			(*tc_sw.disable_option)(index);
		    }
	        }
	}
	else {
	    Cprintf("tcconfl1: Calling get_bus\n");
	    if((busptr = get_bus(tc_slot[index].devname,
			        tc_slot[index].slot,
				 bus->bus_name,
				 bus->bus_num)) ||
	       (busptr = get_bus(tc_slot[index].devname,
				tc_slot[index].slot,
				 bus->bus_name,
				 -1)) ||
	       (busptr = get_bus(tc_slot[index].devname,
				-1,
				 bus->bus_name,
				 bus->bus_num)) ||
	       (busptr = get_bus(tc_slot[index].devname,
				-1,
				 bus->bus_name,
				 -1)) ||
	       (busptr = get_bus(tc_slot[index].devname,
				 -1,
				 "*",
				 -99)))
	       {
		    Cprintf("tcconfl1: Found bus %s%d\n", busptr->bus_name,
							busptr->bus_num);
		    busptr->connect_bus = bus->bus_name;
		    busptr->connect_num = bus->bus_num;
		    /* 
		     * Note: bus confl1 routine fills in the
		     * 	interrupt routine
		     */
		    tc_slot[index].unit = busptr->bus_num;
		    tc_slot[index].param = (caddr_t)busptr->bus_num;
		    tc_slot[index].class = TC_ADPT;
		    tc_slot[index].dev_str = (caddr_t)busptr;
		    if (tc_slot[index].intr_b4_probe)
			(*tc_sw.enable_option)(index);
#ifdef mips
		    info.addr = (caddr_t)PHYS_TO_K1(tc_slot[index].physaddr);
#endif
#ifdef __alpha
		    info.addr = (caddr_t)PHYS_TO_KSEG(tc_slot[index].physaddr);
#endif
		    info.physaddr = (caddr_t)tc_slot[index].physaddr; 
		    info.slot = tc_slot[index].slot;
		    info.unit = tc_slot[index].unit;
		    info.intr = &tc_slot[index].intr;
		    info.bus_hd = bus;
		    busptr->tcindx = (caddr_t)index;
		    Cprintf("tcconfl1: Calling bus confl1\n");
		    if (!(*busptr->confl1)(bus->bus_type, &info, busptr)) {
			    printf("%s%d not probed\n",
				  tc_slot[index].devname, tc_slot[index].unit);
			    (*tc_sw.disable_option)(index);
		    } else {
			    busptr->alive |= ALV_ALIVE;
			    busptr->slot = tc_slot[index].slot;
			    Cprintf("tcconfl1: Calling conn_bus\n");
			    conn_bus(bus, busptr);
			    tc_slot[index].flags |= TC_ACTIVE;
			    if (tc_slot[index].intr_aft_attach) {
				    (*tc_sw.enable_option)(index);
			    } else {
				    (*tc_sw.disable_option)(index);
			    }
		    }
	       }
	       else {
		    /*
		     * No entry was found in the static bus and controller
		     * lists created by config.  Since we know a bit about the
		     * module, we can create the appropriate structures and
		     * hook them into the config.
		     */
		    Cprintf("tcconfl1: Not Configured ");
		    switch (tc_slot[index].class) {
			case TC_ADPT : {
			   Cprintf("Adapter\n");
			       busptr = (struct bus *)kalloc(sizeof(struct bus));
			       if(busptr == NULL)
				    continue;
			       busptr->bus_name = tc_slot[index].devname;
			       busptr->alive = ALV_PRES;
			       busptr->slot = tc_slot[index].slot;
			       busptr->bus_num = -99;
			       busptr->connect_bus = bus->bus_name;
			       busptr->connect_num = bus->bus_num;
			       conn_bus(bus, busptr);
			       break;
			}
			case TC_CTLR : {
			   Cprintf("Controller\n");
			       ctlr = (struct controller *)kalloc(sizeof(struct controller));
			       if(ctlr == NULL)
				    continue;
			       ctlr->ctlr_name = tc_slot[index].devname;
			       ctlr->ctlr_num = -99;
			       ctlr->alive = ALV_PRES;
			       ctlr->bus_name = bus->bus_name;
			       ctlr->bus_num = bus->bus_num;
			       conn_ctlr(bus, ctlr);
			       break;
			}
			default : {
			   Cprintf("\n");
			}
		    }
	       }
        }		     
    }
#ifdef mips
    config_delay();
#endif	/* mips */
}

/* TURBOchannel level 2 configuration routine:
 *
 *	Simply call the level 2 routine for each connected bus
 *
 *	TO DO: investigate removing the completely unused "binfo" argument.
 */
tcconfl2(bustype, binfo, bus)
int bustype;
caddr_t binfo;
struct bus *bus;
{
	register struct bus *busptr;
	register struct tc_info info;
	register int index;

	Cprintf("tcconfl2: Entered\n");
	/* Currently only support TURBOchannel as system bus */
	if(bustype != -1)
	    panic("TURBOchannel not system bus");

	for(busptr = bus->bus_list; busptr; busptr = busptr->nxt_bus) {
		/* If bus is alive then level 1 config was successful */
		if(busptr->alive & ALV_ALIVE) {
		    index = (int)busptr->tcindx;
#ifdef mips
		    info.addr = (caddr_t)PHYS_TO_K1(tc_slot[index].physaddr);
#endif
#ifdef __alpha
		    /* Alpha doesn't really use this routine ... */
		    info.addr = (caddr_t)0x00DEAD0000BEEF00;
#endif
		    info.physaddr = (caddr_t)tc_slot[index].physaddr; 
		    info.slot = tc_slot[index].slot;
		    info.unit = tc_slot[index].unit;
		    info.intr = &tc_slot[index].intr;
		    Cprintf("tcconfl2: Calling bus confl2\n");
		    if (!(*busptr->confl2)(bus->bus_type, &info, busptr))
			busptr->alive = 0;
		}
	}
	Cprintf("tcconfl2: Exiting\n");
}

/*
 * Search thru the tc_slot table to see if the given func address has
 *   already been used (already in the table).  Return 1 if used, 0 if not.
 * This is for devices & controllers that have the ibus number wildcarded.
 */
tc_func_used(intr)
	int (*intr)();		/* the interrupt routine for this device */
{
	register int i;

	for (i = 0; i < TC_IOSLOTS; i++) {
		if (tc_slot[i].intr == intr)
			return(1);
	}
	return (0);
}

tc_isolate_memerr(memerr_status)
	struct tc_memerr_status *memerr_status;
{
#ifdef mips
    if (memerr_status->va == 0) 
	memerr_status->va = (caddr_t)PHYS_TO_K1(memerr_status->pa);
#endif	/* mips */

#ifdef __alpha
    if (memerr_status->va == 0) {
	memerr_status->va = (caddr_t)PHYS_TO_KSEG(memerr_status->pa);
    }
#endif	/* __alpha */

    if (tc_sw.isolate_memerr)
	return ((*tc_sw.isolate_memerr)(memerr_status));
    else
	return (-1);
}

tc_config_cont(nxv, nxp, slot, name, ctlr)
char *nxv;
char *nxp;
u_long slot;
char *name;			/* never used */
struct controller *ctlr;
{
	register struct driver *drp;
	register struct device *device;
	int savectlr;
	char *savectname;
	int (**ivec)();
	int i;
	int found = 0;


	if (ctlr->alive & ALV_ALIVE)
		return(0);

	drp = ctlr->driver;

	i = (*drp->probe)(nxv, ctlr);
	if (i == 0)
	    return(0);
	ctlr->slot = slot;
	ctlr->alive |= ALV_ALIVE;
	ctlr->addr = (char *)nxv;
	(void)svatophys(ctlr->addr, &ctlr->physaddr);
	drp->ctlr_list[ctlr->ctlr_num] = ctlr;
	config_fillin(ctlr);
	printf("\n");
	if (drp->cattach)
		(*drp->cattach)(ctlr);

	for (device = device_list;
	     device->dev_name;
	     device++) {
	    if (((device->ctlr_num != ctlr->ctlr_num) &&
		     (device->ctlr_num !=-1) && (device->ctlr_num != -99)) ||
		    ((strcmp(device->ctlr_name, ctlr->ctlr_name)) &&
		     (strcmp(device->ctlr_name, "*"))) ||
		    (device->alive & ALV_ALIVE) ||
		    (device->alive & ALV_NOCNFG) ) {
		    continue;
	    }

	    savectlr = device->ctlr_num;
	    savectname = device->ctlr_name;
	    device->ctlr_num = ctlr->ctlr_num;
	    device->ctlr_name = ctlr->ctlr_name;

	    if ((drp->slave) && (*drp->slave)(device, nxv)) {
		device->alive |= ALV_ALIVE;
		conn_device(ctlr, device);
		drp->dev_list[device->logunit] = device;
		if(device->unit >= 0) {
		    /* print bus target lun info for SCSI devices */
		    if( (strncmp(device->dev_name, "rz", 2) == 0) |
		        (strncmp(device->dev_name, "tz", 2) == 0) ) {
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
		if (drp->dattach)
			(*drp->dattach)(device);
		printf("\n");
	}
	else {
	    device->ctlr_num = savectlr;
	    device->ctlr_name = savectname;
	}
    }
    return(1);
}


#ifdef __alpha

static struct {
	struct map tcmap;
	struct mapent tcmapent[TC_NMAP];
} tc_vdma_map;

static long tc_vdma_addr = {
	TC_VDMA_ADDR,	    /* shdb 0 but rmap can't handle that */
};			    /* TC_FIND_MAP subtracts this out, and */
			    /* ioa_mapaddr will shift this into */
			    /* oblivion */

static tcmap_t *tc_to_mi = {          /* location of TC-to-Physical maps */
        (tcmap_t *)TC_MAP_PHYSADDR,   /* in sparse space */
};


static u_int tc_stamp;

/* note: this should be NBPG
 */
static char tc_scratch_page[8192] = {0};
static unsigned long tc_scratch_phys;

extern cpu;

/* allocate Turbochannel map registers
 *
 * count - the number of bytes to be mapped.  The caller
 *	   gets one map per page, plus one guard page,
 *	   and possibly one scratch page.
 *
 * flags - TC_SCRATCH: allocate a scratch page at the end
 *		       of the mapped area as a barrier against
 *		       sloppy DMA hardware.  Names are witheld
 *		       to protect the guilty.
 *
 * 	   TC_SLEEP:   sleep if the requested maps aren't available
 *		       at the time the request is made.  Normally
 *		       tc_map_malloc will return -1 if it can't
 *		       honor the request.
 *
 * returns - -1 (failure) for a negative byte count, or if the
 *	     request can't be met immediately and TC_SLEEP is not set.
 *	     Otherwise, a non-zero value is returned.  This value is
 *	     intended to be passed in to a later call to tc_maddr or
 *	     tc_mvaddr
 *
 */
unsigned long
tc_map_alloc(count, flags)
int count;
int flags;
{
	register int s;
	register long nseg;
	register long base;
	register struct map *mp;
	uint stamp;

	if (count <= 0)
		return -1;	/* negative byte count */

	if (cpu != DEC_3000_500)
		return (-1);

	if (flags & TC_MAP_SCRATCH)
		nseg = btoc(count) + 3;	/* +1 for alignment, +1 for scratch */
					/* +1 for guard */
	else
		nseg = btoc(count) + 2;	/* +1 for alignment, +1 for guard */

	mp = &tc_vdma_map.tcmap;
	stamp = tc_stamp;

	while (1) {
		s = spldevhigh();
		base = rmalloc(mp, nseg);
		if (base != NULL) {
			splx(s);
			return base;
		}
		if (!(flags & TC_MAP_SLEEP)) {
			splx(s);
			return -1;
		}

		printf("tc_map_alloc sleeping: count=0x%x\n", count);
		while (stamp == tc_stamp)
			sleep(&tc_stamp, PRIBIO);
		stamp = tc_stamp;
		splx(s);
	}
}

/* free resources obtained through tc_map_alloc
 */
tc_map_free(base, count, flags)
int count, flags;
long base;
{
	register tcmap_t *up;
	register int map, nseg, saveseg;
	register struct map *mp;
	int s;

	if (count <= 0)	/* sanity check */
		return -1;

	if (cpu != DEC_3000_500)
		return (-1);

	if (flags & TC_MAP_SCRATCH)	/* alignment, scratch, guard */
		nseg = btoc(count) + 2;
	else				/* alignment and guard */
		nseg = btoc(count) + 1;
	saveseg = nseg + 1;

	TC_FIND_MAP(base, mp, map, up);

	if (flags & TC_MAP_INVAL)
		for ( ; --nseg >= 0; ) {
			*(int *)up++ = 0;
			mb();
		}

	rmfree(mp, (long)saveseg, base);
	s = spldevhigh();
	tc_stamp++;
	splx(s);
	wakeup(&tc_stamp);

	return 0;
}

/* initialize the Turbochannel map structures
 * get physical address of scratch page for later use
 */
static void
tc_map_init()
{

	if (cpu != DEC_3000_500)
		return ;

	tc_to_mi = (tcmap_t *)PHYS_TO_KSEG(TC_MAP_PHYSADDR);

	rminit(&tc_vdma_map.tcmap, (long)TC_VDMA_ENTRIES,
		tc_vdma_addr, "TC Map Registers", (long)TC_NMAP);
	svatophys(tc_scratch_page, &tc_scratch_phys);
}

/*
 * Fill in map rams and return an "address" to load into
 * a DMA engine
 *
 * This will also allocate map registers if none were
 * reserved (*base == 0).
 */
unsigned long
tc_loadmap(proc, base, addr, count, flags)
struct proc *proc;
long *base;
vm_offset_t addr;
{
	int offset = addr & PGOFSET;
	int npf, map;
	tcmap_t *up;
	struct map *mp;
	unsigned long paddr;

	if (cpu != DEC_3000_500)
		return (-1);

	if ((!*base) && ((*base = tc_map_alloc(count, flags)) == -1) ) {
		if (flags & TC_MAP_FAILSOFT)
			return ((paddr_t)-1);
		panic("tc_mapaddr: tc_malloc failed");
	}

	count += offset;	/* for alignment */
	npf = btoc(count);

	TC_FIND_MAP(*base, mp, map, up);

	/* for each page, get its physical page number (via pmap_extract for
	 * lowly users, or svatophys for kernel) and fill in the map registers
	 * If pmap_extract should fail, stop and return an error.
	 */
	for (; --npf >= 0; addr += NBPG, ++up) {
		if (IS_SEG0_VA(addr)) {
			if ((paddr = pmap_extract(proc->task->map->vm_pmap, addr)) == 0)
				return (-1);
		}
		else
			svatophys(addr, &paddr);
		*(int *)up = TC_MAP_MTE(paddr);
		mb();
#if 0
printf("mapaddr: base 0x%lx, addr 0x%lx, count %d, paddr 0x%lx, up 0x%lx, *up 0x%x\n", *base, addr, count - offset, paddr, up, *up);
#endif
	}
	if (flags & TC_MAP_SCRATCH) {	/* map the scratch page */
		*(int *)up++ = TC_MAP_MTE(tc_scratch_phys);
		mb();
	}
	*(int *)up = 0;		/* mark the guard page as invalid */
	mb();

	return((*base << PGSHIFT) + offset);
}

/* Unload  physical TC sg map pages
 */
long
tc_unload_map(base, count, flags)
int count, flags;
long base;
{
        register tcmap_t *up;
        register int map, nseg, saveseg;
        register struct map *mp;

        if (count <= 0) /* sanity check */
                return -1;

        if (cpu != DEC_3000_500)
                return (-1);

        if (flags & TC_MAP_SCRATCH)     /* alignment, scratch, guard */
                nseg = btoc(count) + 2;
        else                            /* alignment and guard */
                nseg = btoc(count) + 1;
        saveseg = nseg + 1;

        /* TC_FIND_MAP(base, mp, map, up); */

        if (flags & TC_MAP_INVAL)
                for ( ; --nseg >= 0; ) {
                        *(int *)up++ = 0;
                        mb();
                }
}



/* Since the TC DMA scatter gather mapping registers are documented in the 
 * Guide to Writing Device Drivers they cannot be modified. Instead the
 * following jacket routines will be used to create a calling interface
 * for the bus bridge DMA support. This interface will be used across
 * all bus bridge mapping support. 
 *
 * The final definition of these interfaces are under development. 
 */ 
u_long
tc_bus_bridge_map_alloc(count, flags)
  int count;
  int flags;
{
u_long base;
int jflags;

    /* Flags must be decoded and implemented, current tc_map_alloc flags
     * have very different meanings.
     *
     */ 

     /*
      * add one page_size to bc in case buffer crosses a page boundary;
      * add one page_size per guard page requested
      * add one page_size if bc less than page_size, since div = 0;
      *
      * This code is not required on this platform until discontinous resource
      *  allocation is supported...
      *
      *    if (flags & DMA_GUARD_UPPER)  || (flags & DMA_GUARD_LOWER) 
      * 
      *   TC_SLEEP 
      *   TC_SCRATCH 
      */


    /* Before executing this routine must determine if this tc slots mapping
     * is enabled. If it is not then this call must return failure. If it is
     * the mapping is allowed to continue. It is up to the confl1 routine of
     * a particular bus/adapter(vme) to configure that particular itc bus slot
     * for Hardware mapping. 
     */ 

     base = tc_map_alloc(count, jflags); 

    /* base = -1 error: invalid count,machine doe not have mapping capability,
     * address of sg register base
     * 
     * Look at the "tc_map_alloc" flag definitions and translate them
     * into dma_direct_map definitions. This call returns the base address
     * of the hardware registers allocated for this request.
     */

}

unsigned long
tc_bus_bridge_map_load(proc, base, addr, count, flags)
  struct proc *proc;
  long *base;
  vm_offset_t addr;
{
int jflags;

    /* Flags must be decoded and implemented, current tc_loadmap flags
     * have very different meanings.
     *
     *   Guard pages
     *   Sleep
     *   DMA_IN and DMA_OUT
     */ 
    tc_loadmap(proc, base, addr, count, jflags); 

    /* Look at the "tc_load_map" flag definitions and translate them
     * into dma_direct_map definitions. 
     */ 
}

int
tc_bus_bridge_map_unload(base, count, flags)
  int count, flags;
  long base;
{
int jflags;

    /* Flags must be decoded and implemented, current tc_unload_map flags
     * have very different meanings.
     *
     *   Guard pages
     *   Sleep
     */ 
    tc_unload_map(base, count, jflags); 

    /* Look at the "tc_unload_map" flag definitions and translate them
     * into dma_direct_map definitions.
     */ 
}

int
tc_bus_bridge_map_dealloc(base, count, flags)
int count, flags;
long base;
{
int jflags;

    /* Flags must be decoded and implemented, current tc_map_free flags
     * have very different meanings.
     *
     *   Guard pages
     *   Sleep
     */ 
    tc_map_free(base, count, jflags); 

    /* Look at the "tc_map_free" flag definitions and translate them
     * into dma_direct_map definitions.
     */ 

}

/* This function is responsible for initing the "TC" bus bridge
 * structure with the appropriate DMA bus support routines...
 */ 
tc_sg_map_setup()
{
/***** Documented "Guide to Writing Device Drivers" definition of the interface 

      tc_dma_callsw_init(tc_map_alloc,
                         tc_loadmap,
                         tc_unload_map,
                         tc_map_free );
*/ 

/* Jacketed version of sg hardware resource calls... to work with Alpha DMA
 * support.
 */ 
      tc_dma_callsw_init(tc_bus_bridge_map_alloc,
                         tc_bus_bridge_map_load,
                         tc_bus_bridge_map_unload,
                         tc_bus_bridge_map_dealloc);
}


#else /* __alpha */

/* stubs for machines that don't have Turbochannel virtual DMA
 */
tc_map_alloc(count, flags)
int count, flags;
{
	return (-1);
}

tc_map_free(base, count, flags)
unsigned long base;
int count, flags;
{
	return (0);
}

unsigned long
tc_loadmap(proc, base, addr, count, flags)
struct proc *proc;
long *base;
vm_offset_t addr;
{
	return (-1);
}

static void
tc_map_init() {}
#endif /* __alpha */
