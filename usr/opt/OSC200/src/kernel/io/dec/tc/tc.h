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
/*	
 *	@(#)$RCSfile: tc.h,v $ $Revision: 1.2.8.4 $ (DEC) $Date: 1993/10/20 00:16:59 $
 */ 
/*
 * Modification History: tc.h
 *
 * 16-Sep-91	Andrew Duane
 *	Modified for support of ALPHA FLAMINGO
 *		and a bunch of portability fixes.
 *
 * 06-Mar-91	Mark Parenti
 *	Modify to use new I/O data structures.
 *
 * 21-Jan-91	Randall Brown
 *	Added tc_memerr_status struct for use in tc_isolate_memerr()
 *	routine.
 *
 * 15-Oct-90	Randall Brown
 *	Added defines for ROM header offsets.
 *
 *  6-Sep-90	Randall Brown
 *	Changed slot_order to be config_order. 
 *
 *  5-Jul-90	Randall Brown
 *	Added the routine tc_ui_to_name() for drivers to determine
 *	the name of the specific option module.
 *
 * 27-Mar-90	Randall Brown
 *	Added #defines for TC_OPTION_SLOT_*
 *
 * 22-Jan-90	Randall Brown
 *	Created this file for TURBOchannel support
 *
 */

#ifndef _TC_TC_H_
#define _TC_TC_H_

#include <sys/types.h>

/* Are these the same for FLAMINGO ??? */
#ifdef mips
/* 3MAX has 8 slots. 0-2 are user option slots, 3-4 are unused, */
/* 5-7 are CPU card fixed: 5 = SCSI, 6 = LANCE, 7 = SERIAL (SCC) */
/* MAXINE has 11 slots, to up TC_IOSLOTS to the bigger of the two */
#define TC_IOSLOTS		11	/* number of TURBOchannel slots */
#define TC_OPTION_SLOTS		3	/* number of optional tc slots */
#define TC_OPTION_SLOT_0	0
#define TC_OPTION_SLOT_1	1
#define TC_OPTION_SLOT_2	2
#endif

#ifdef __alpha
/* FLAMINGO has 8 "slots". 0-5 are option slots, 6-7 are CPU card fixed: */
/* 6 = SCSI and TC regs, 7 = Core I/O ASIC (SCC, CFB, Junk I/O, TOY, ISDN) */
/* But, we pretend there are separate slots for each mappable entity on */
/* the Core I/O ASIC, which brings us up to a lot more than 8. */
/* All fixed entities and option slots are mapped in via pmap_data.c */
/* The option slots make a somewhat wild guess at how much to map in now */

#define TC_IOSLOTS		15	/* number of TURBOchannel slots */
#define TC_OPTION_SLOTS		6	/* number of optional tc slots */
#define TC_OPTION_SLOT_0	0
#define TC_OPTION_SLOT_1	1
#define TC_OPTION_SLOT_2	2
#define TC_OPTION_SLOT_3	3
#define TC_OPTION_SLOT_4	4
#define TC_OPTION_SLOT_5	5
#define TC_SCC0_SLOT		6
#define TC_SCC1_SLOT		7
#define TC_SCSI_SLOT		8
#define TC_CFB_SLOT		9
#define TC_ROM_SLOT		10
#define TC_REG_SLOT		11
#define TC_ISDN_SLOT		12
#define TC_TOY_SLOT		13
#define TC_LANCE_SLOT		14

#endif


#define TC_ROMOFFSET	0x003c0000	/* offset in slot to IO ROM */

#define TC_VERSION_OFFSET	0x400
#define TC_VENDOR_OFFSET	0x420
#define TC_NAME_OFFSET		0x440

#define TC_ROMNAMLEN 8			/* length of module name in rom */

struct tc_option {
	char modname[TC_ROMNAMLEN+1];	/* the module name */
	char confname[TC_ROMNAMLEN+1];	/* device or ctlr name (config file) */
	int  intr_b4_probe;		/* enable intr before probe routine */
	int  intr_aft_attach;		/* enable intr after attach routine */
	char type;			/* D = dev, C = ctrlr, A = adpt */
	int (*adpt_config)();		/* adpater config routine to call */
};

/* Define the tc_info structure used to contain tc-specific bus information */
/* This is passed to the configuration routine of any bus connected to the  */
/* TURBOchannel.							    */

struct	tc_info {
	caddr_t	addr;			/* virtual address of slot	*/
	caddr_t	physaddr;		/* physical address of slot	*/
	int	slot;			/* slot number			*/
	int	unit;			/* logical unit number		*/
	int (**intr)();			/* intr routine for device (from config file)*/
	struct bus *bus_hd;		/* pointer to bus head structure */
};


#define	tcindx		conn_priv[1]
#define	boot_slot	conn_priv[2]

/* Define the tc interrupt handler structure for use by loadable device
 * drivers.
 */

struct tc_intr_info {
	caddr_t configuration_st;	/* Pointer to bus/controller struct */
	int (*intr)();			/* Interrupt handler */
	caddr_t param;			/* param for interrupt handler */
	unsigned int config_type;	/* Driver type, TC_ADPT or TC_CTLR */
};

/*
 *	The following are used to describe the class attribute of the
 *	module.  These will be filled in during auto-configuration.
 *	These are also used to specify driver type in the config_type field
 *	of the tc_intr_info data structure.
 */
#define TC_CTLR 	1
#define TC_DEV 		2
#define TC_ADPT 	3
#define TC_UNKNOWN	4	/* Module exists but we don't know what kind */
				/* it is.  Provided for loadable support.    */

/* Definitions for flags field in tc_slot structure */

#define	TC_INTR_ADD	0x1	/* Handler add performed */
#define	TC_INTR_ENAB	0x2	/* Handler enable performed */
#define TC_ACTIVE	0x4	/* Slot is controlled by a driver */

struct tc_slot {
	char version[TC_ROMNAMLEN+1];	/* the version in the ROM */
	char vendor[TC_ROMNAMLEN+1];	/* the vendor in the ROM */
	char modulename[TC_ROMNAMLEN+1];	/* the module name in the ROM */
	char devname[TC_ROMNAMLEN+1];	/* the controller or device name */
	int slot;		/* the TURBOchannel IO slot number */
	int module_width;	/* how many slots the IO module takes */
	int (*intr)();		/* intr routine for device (from config file)*/
	int unit;		/* device unit number (from config file) */
	vm_offset_t physaddr;	/* the physical addr of the device */
	int class;		/* ctlr or dev: to call right config routine */
	int intr_b4_probe;	/* enable intr before probe routine */
	int intr_aft_attach;	/* enable intr after attach routine */
	int (*adpt_config)();	/* config routine for adapters
				   (from tc_option_data table) */
	caddr_t dev_str;	/* controller structure	*/
	caddr_t param;		/* interrupt parameter */
	int flags;		/* various flags	*/
};

extern struct tc_slot tc_slot[];	/* table with IO device info */
extern vm_offset_t tc_slotaddr[];	/* table with slot address info */

struct tc_sw {
        int *config_order;	/* order to probe slots of TURBOchannel */
        int (*enable_option)();	/* routine to enable interrupt */
	int (*disable_option)();/* routine to disable interrupt */
	int (*clear_errors)();	/* routine to clear errors caused by probe */
	int (*isolate_memerr)();/* routine to isolate memory errors */
	int (*option_control)();/* routine to enable/disable tc operating modes */
};

extern struct tc_sw tc_sw;
/*
 *	tc_enable_option(ctlr)
 *		struct controller *ctlr;
 *
 *	Takes controller structure for this controller.
 *
 *	This function enables an option's interrupt on the TURBOchannel
 *	to interrupt the system at the I/O interrupt level.  
 *	This is done calling the system specific routine to allow the
 *	option's slot to interrupt the system.
 */

int tc_enable_option();

/*
 *	tc_disable_option(ctlr)
 *		struct controller *ctlr;
 *
 *	Takes controller structure for this controller.
 *
 *	This function disables an option's interrupt on the TURBOchannel from
 *	interrupting the system at the I/O interrupt level.  
 *	This is done by calling the system specific routine to reset the 
 *	option's slot from interrupting the system.
 */	

int tc_disable_option();

/*
 *	tc_module_name(ctlr, cp)
 *		struct controller *ctlr;
 *		char *cp; ( cp[TC_ROMNAMLEN + 1] )
 *
 *	Takes a pointer to a controller struct.
 *
 *	This function will fill in the character array 'cp' with the ascii
 *	string of the TURBOchannel option's module name refered to by the
 *	 'ctlr' pointer. The array 'cp' must be declared by the caller
 *	of this routine to be the size defined above.
 *
 *	The function will return a (-1) if it was unable to use the 'cp'
 *	pointer that it was given.
#ifdef __alpha
 *	Alpha would have a much harder time walking the name array, so skip it.
#endif
 */


int	tc_option_control();

/*
 *	tc_option_control(ctlr,flags)
 *		struct controller *ctlr;
 *		int	flags;	        	 Flags which indicate whats
 *						 options to enable(1)/disable(0) 
 * 
 *	Take the flags argument and enables all functions indicated by a 1 and
 *	disables all options indicated by a 0.
 *	The return status matches bit for bit with the flags and returns which
 *	options are currently enabled and which ones
 *	are disabled. A flags param with a -1 a request for currently enabled
 *	functions for this slot.
 *
 */

/* Define the flags unsigned longword that is retured and passed in. The bit
 * locations are the same but the flags  passed in are a 
 * request to enable, while the return value is the options that enabled on return
 * from the call for that slot/bus.
 */

#define SLOT_CLEARFLAGS 0
#define SLOT_MAPREGS	1
#define SLOT_BLOCKMODE	2
#define SLOT_PARITY	4
#define SLOT_STATUS     8
#define BAD_REQUEST	-1

int	tc_module_name();

/*
 *	tc_addr_to_name(addr, cp)
 *		vm_offset_t	addr;
 *		char		*cp; ( cp[TC_ROMNAMLEN + 1] )
 *
 *	Takes the address passed to the device driver's probe routine
 *	which is the base address of the device.
 *
 *	This function will fill in the character array 'cp' with the ascii
 *	string of the TURBOchannel option's module name refered to by the
 *	base address 'addr'.  The array 'cp' must be declared by the caller
 *	of the routine to be the size defined above.
 *
 *	This function would be used in a driver's probe routine, since the
 *	'ctlr' pointer required by the above routine 'tc_module_name' is
 *	not valid in the probe routine.
 *
 *	The function will return a (-1) if it was unable to use the 'cp'
 *	pointer that it was given.
#ifdef __alpha
 *	Alpha would have a much harder time walking the name array, so skip it.
#endif
 */

int	tc_addr_to_name();

struct tc_memerr_status {
    caddr_t	pa;		/* physical address of error */
    caddr_t	va;		/* virtual address, 0 if not know */
    int		log;		/* flag whether to log error */
    int		blocksize;	/* size of DMA block */
    u_int	errtype;	/* error type status */
};

/*
 *	tc_isolate_memerr(memerr_status)
 *	    struct tc_memerr_status *memerr_status;
 *
 *	Takes a pointer to a tc_memerr_status struct.
 *
 *	Takes the physical address (pa) of the error, the virtual
 *	address of the error (va), flag for logging, and a pointer
 *	to a status int.  If the va is 0, a K1 address is formed from
 *	the physical address and is used as the virtual address.
#ifdef __alpha
 *	Alpha cannot easily convert a physical address to a va,
 *	so this step will not be done.
#endif
 *	
 *	This function will fill in the u_int 'errtype' with
 *	the information about the memory error of 'pa' as defined below.
 *	This is done by calling a system specific routine to determine the 
 *	exact error based on the physical address and virtual address.  If
 *	the parameter 'log' is set to TC_LOG_MEMERR, the system 
 *	specific routine will log the error in the same manner a memory
 *	error is logged as if it came directly into the CPU.
 *
 *	The function will return a (-1) if it the physical address supplied
 *	is bad, or if the system specific routine does not exist.
 */

/*
 * 	The following defines are used for the 'log' parameter in the
 * 	tc_memerr_status struct passed to tc_isolate_memerr().
 */
#define	TC_NOLOG_MEMERR		0	/* do not log error info */
#define	TC_LOG_MEMERR		1	/* log error information */

/* 
 *	The following defines are used for the 'errtype' return status
 *	in the tc_memerr_status struct passed to tc_isolate_memerr().
 *
 *	NOTE: As other systems are produced that have different
 *	      memory subsystems, additional defines will be added to the
 *	      list below.
 */
#define TC_MEMERR_NOERROR	0	/* no error found */
#define TC_MEMERR_TRANS		1	/* transient parity error */
#define TC_MEMERR_SOFT		2	/* soft parity error */
#define TC_MEMERR_HARD		3	/* hard parity error */

int	tc_isolate_memerr();

#ifdef __alpha
/* The MIPS writebuffer flush is the same as our memory barrier */
#define	wbflush()	mb()
#endif	/* __alpha */



/* definitions for Turbochannel map registers
 */

/*
 * Fills in mp and map (TC-to-Physical map entry to use)
 * and up (address of map register entry) depending on the value of base.
 */
#define TC_FIND_MAP(base, mp, map, up)		\
	mp = &tc_vdma_map.tcmap;		\
	map = base;				\
	up = &tc_to_mi[(map - TC_VDMA_ADDR)];

#define TC_MAP_SLEEP 1		/* calls to ioa_malloc and ioa_mvaddr and
				 * ioa_maddr can sleep
				 */

#define TC_MAP_FAILSOFT 2	/* calls to ioa_maddr and ioa_mvaddr will
				 * not panic if ioa_malloc fails
				 */

#define TC_MAP_INVAL 4		/* invalidate map entries as they're freed */

#define TC_MAP_SCRATCH 8	/* map the guard page to a scratch page */
				/* instead of marking it invalid */

#define EPMAP 1			/* set if pmap_extract fails
				 */

#define TC_MAP_PHYSADDR 0x1d5000000

#ifdef __alpha

/* parametric goodies for map registers on a Flamingo
 *
 * These will need to be changed for other machines that
 * have a virtual DMA capability
 */

typedef volatile unsigned long tcmap_t;

#define TC_MAP_VALID 0x800000	/* This is the "valid" bit in the map
				 * register
				 */

#define TC_VDMA_ADDR 0x8000000000000000

#define TC_NMAP	50   		/* Number of map fragments in TC-to-MI map */

#define TC_VDMA_ENTRIES 32767	/* TC-to-Physical entries in map */
				/* last map entry is used by the TC */
				/* slot interrupt mask register */

				/* bytemask for writing all four bytes */
				/* in sparse space */
#define MASK_LW	0xf000000000000000L

/*
 * Calculate and return 32-bit value to be stuffed into
 * Map Table Entry (mte) register.  Each register contains
 * a 17-bit physical page number up-shifted by the page
 * offset (9 bits for Alpha/OSF).  The parity bit is
 * calculated by the hardware, the funny bit is always
 * turned off, and the valid bit must be turned on.
 * high-order bits are or-ed in to ensure that the entire
 * longword is written by the hardware.
 */
#define TC_MAP_MTE(addr) (unsigned int)((((addr) & 0x3fffe000) >> 9) | TC_MAP_VALID)

/* end of parametric goodies
 */

#else	/* __alpha */

typedef int tcmap_t;

#define TC_VDMA_ADDR 0
#define TC_NMAP 1
#define TC_VDMA_ENTRIES 0
#define TC_MAP_VALID 0

#endif /* __alpha */

#endif
