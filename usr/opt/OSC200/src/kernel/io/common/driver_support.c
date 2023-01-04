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
static char	*sccsid = "@(#)$RCSfile: driver_support.c,v $ $Revision: 1.2.8.10 $ (DEC) $Date: 1993/12/17 20:55:57 $";
#endif 
/*
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * New in OSF
 */

/* Abstract:
 *	This module contains the general support routines 
 *	for system device  configuration.
 *
 * Modification History:
 *
 *  25-Jan-91 -- Mark Parenti (map)
 *	Original Version
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
#include <sys/config.h>
#include <sys/dk.h>	

/* DMA support includes */
#include <mach/vm_param.h>
#include <io/common/devdriver.h>

/* External declarations	*/

extern	struct bus bus_list[];
extern	struct controller controller_list[];
extern	struct device	device_list[];
extern	int	cold;	/* for bootstrap status for dma zalloc code */
/*
 * Prototypes
 */
u_long no_dma_map_alloc();
u_long no_dma_map_load();
int no_dma_map_unload();
int no_dma_map_dealloc();
int no_dma_min_bound();

u_long null_dma_map_alloc();
u_long null_dma_map_load();
int null_dma_map_unload();
int null_dma_map_dealloc();
int null_dma_min_bound();

int confdebug = 0;	/* Configuration debug routine */

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

/* get_bus()
 *
 *	The get_bus() routine searches the static bus_list array for a 
 *	bus structure with values matching the passed parameters.
 *	A parameter value of -1 is used to indicate that a particular
 *	member need not match.
 *	The members checked are the bus name, the bus node
 *	or slot number, and the bus number.  This routine is used with 
 *	busses which have the capability to automatically configure 
 *	themselves.
 */
struct bus *
get_bus(name, slot, bus_name, num)
char	*name;	/* device name	*/
int	slot;	/* device node or slot number */
char 	*bus_name; /* bus name connected to */
int	num;	/* bus number connected to */
{
	struct bus 	*bus;

	for (bus = bus_list; bus->bus_name != 0; bus++) {
		if ((!strcmp(bus->bus_name, name)) &&
		    ((bus->slot == slot)) &&
		    ((bus->connect_num == num)) &&
		    ((!strcmp(bus->connect_bus, bus_name))) &&
		    (bus->alive == 0))
			break;
	}
	if (bus->bus_name)
		return(bus);
	else
		return(0);
}

/* get_sys_bus()
 *
 *	The get_sysbus() routine returns a pointer to a system bus structure.
 *	A system bus is connected to bus -1. Config will generate this value
 *	for any bus connected to nexus. This routine looks for a system bus
 *	structure with a bus name matching  the passed parameter.
 */
struct bus *
get_sys_bus(name)
char *name;
{
	struct bus *bus;

	for(bus = bus_list; bus->bus_name; bus++) {
		if ( (bus->connect_num == -1) &&
		    !(strcmp(bus->bus_name, name))) 
			return(bus);
	}
	panic("No system bus structure");
}

/* get_ctlr()
 *
 *	The get_ctlr() routine searches the static ctlr_list array for a 
 *	controller structure with values matching the passed parameters.
 *	The members checked are the controller name and the controller node
 *	or slot number.  This routine is used with busses which have
 *	the capability to automatically configure the bus.
 */

struct controller *
get_ctlr(ctlr_name, slot, bus_name, num)
char	*ctlr_name;	/* ctlr name to search for */
int	slot;		/* node or slot number */
char 	*bus_name; 	/* bus name connected to */
int num;		/* bus number connected to */
{
	struct controller *ctlr;

	for (ctlr = controller_list; ctlr->driver != 0; ctlr++) {
		if (ctlr->slot == slot &&
		    !(strcmp(ctlr->ctlr_name, ctlr_name)) &&
		    !(strcmp(ctlr->bus_name, bus_name)) &&
		    ctlr->bus_num == num &&
		    ctlr->alive == 0) {
			break;
		}
	}
	if (ctlr->driver) 
		return (ctlr);
	else
		return (0);
}


/* get_ctlr_num()
 *
 *	The get_ctlr() routine searches the static ctlr_list array for a 
 *	controller structure with values matching the passed parameters.
 *	The members checked are the controller name and the controller number.
 *	This routine is used by graphics drivers to gain access to a specific
 *	controller data structure (eg "fb0") at a time when these structures
 *	have not been connected to devices (ie during console initialization).
 */

struct controller *
get_ctlr_num(ctlr_name, ctlr_num)
char	*ctlr_name;	/* ctlr name to search for */
int	ctlr_num;	/* ctlr number */
{
	struct controller *ctlr;

	for (ctlr = controller_list; ctlr->driver != 0; ctlr++) {
		if (ctlr->ctlr_num == ctlr_num &&
		    !(strcmp(ctlr->ctlr_name, ctlr_name))) {
			break;
		}
	}
	if (ctlr->driver) 
		return (ctlr);
	else
		return (0);
}


/* ctlr_search()
 *
 *	The ctlr_search() routine looks for a controller structure connected
 *	to a specific bus.  This routine is used by buses without auto
 *	configuration capabilities.  This routine would be called repeatedly
 *	until all controllers connected to the specified bus have been found.
 */

struct controller *
ctlr_search(num, name)
int	num;	/* bus number to search for */
char	*name;	/* bus name to search for */
{
	struct controller *ctlr;

	for (ctlr = controller_list; ctlr->ctlr_name != 0; ctlr++) {
		if (ctlr->bus_num == num &&
		    bcmp(ctlr->bus_name, name) ) {
			break;
		}
	}
	if (ctlr->ctlr_name) 
		return (ctlr);
	else
		return (0);
}

/* bus_search()
 *
 *	The bus_search() routine searches the static bus_list array for a 
 *	bus structure with values matching the passed parameters.
 *	The members checked are the bus name and the bus node
 *	or slot number.  This routine is used with busses which have
 *	the capability to automatically configure themselves.
 */
struct bus *
bus_search(busname, slot)
char	*busname;	/* bus name to search for */
int	slot;		/* node or slot to check */
{
	struct bus *bus;

	for (bus = bus_list; bus->bus_name != 0; bus++) {
		if (bcmp(bus->connect_bus, busname) &&
		    (bus->slot == slot) &&
		    (bus->alive == 0)) {
			break;
		}
	}
	if (bus->bus_name) 
		return (bus);
	else
		return (0);
}

/* conn_ctlr
 *
 *	This routine will connect the passed controller structure to the
 *	passed bus structure.
 */
void
conn_ctlr(bus, ctlr)
struct bus *bus;
struct controller *ctlr;
{
	struct controller *nxtctlr;

	ctlr->bus_hd = bus;
	for(nxtctlr = bus->ctlr_list; nxtctlr;
	    nxtctlr = nxtctlr->nxt_ctlr) {
		if(nxtctlr->nxt_ctlr == 0)
			break;
	}
	if(nxtctlr == 0)
		bus->ctlr_list = ctlr;
	else
		nxtctlr->nxt_ctlr = ctlr;
	ctlr->nxt_ctlr = 0;
}

/* conn_bus
 *
 *	This routine will connect the 2nd passed bus structure to the
 *	1st passed bus structure.
 */
void
conn_bus(hdbus, bus)
struct bus *hdbus;
struct bus *bus;
{
	struct bus *nxtbus;

	bus->bus_hd = hdbus;
	for(nxtbus = hdbus->bus_list; nxtbus;
	    nxtbus = nxtbus->nxt_bus) {
		if(nxtbus->nxt_bus == 0)
			break;
	}
	if(nxtbus == 0)
		hdbus->bus_list = bus;
	else
		nxtbus->nxt_bus = bus;
	bus->nxt_bus = 0;
}


/* conn_device
 *
 *	This routine will connect the passed device structure to the passed
 *	controller structure.
 */
void
conn_device(ctlr, device)
struct controller *ctlr;
struct device *device;
{
	struct device *nxtdev;

	device->ctlr_hd = ctlr;
	for(nxtdev = ctlr->dev_list; nxtdev;
	    nxtdev = nxtdev->nxt_dev) {
		if(nxtdev->nxt_dev == 0)
			break;
	}
	if(nxtdev == 0)
		ctlr->dev_list = device;
	else
		nxtdev->nxt_dev = device;
	device->nxt_dev = 0;
}


/* get_device()
 *
 *	This routine will return the device structure associated with the
 *	next device which was connected to the controller in the system 
 *	configuration file.
 */
struct device *
get_device(ctlr)
struct controller	*ctlr;	
{
	struct device *device;

	for (device = device_list; 
	     device->dev_name; 
	     device++) {
		if ((device->ctlr_num == ctlr->ctlr_num) &&
		    !(strcmp(device->ctlr_name, ctlr->ctlr_name)) &&
		    (device->alive == 0) ) {
			break;
		}
	}
	if (device->dev_name) 
		return (device);
	else
		return (0);
}

/* perf_init()
 *
 *	This routine will initialize the performance structure for 
 *	a disk device.
 */
int	dkn;		/* number of iostat dk numbers assigned so far */


perf_init(device)
struct device	*device;
{
	if ((!strcmp(device->dev_type, "disk")) && (dkn < DK_NDRIVE))
		device->perf = (caddr_t)dkn++;
	else
		device->perf = (caddr_t)-1;

}




/*
 *  Name:		
 *
 *  Abstract:
 *
 *  Inputs:
 *
 *	dev	device name (major/minor number)
 *  Outputs:
 *
 *
 *  Return
 *  Values:
 *
 */
asyncsel(dev, events, revents, scanning)
	dev_t dev;
	short *events, *revents;
        int scanning;
{
#if	0
	/*
	 * XXX - This code needs work - conversion to BUF_LOCKS - before
	 * it can be activated. - XXX
	 */
	register struct buf *bp;
	register struct abuf *dp;
	register int s;
	register int doown = 0;
	
	BOP_SPLBIO(s);
	XPR(XPR_BIO, ("enter asyncsel",0,0,0,0));
	if(async_bp == (struct abuf *)0)
		return(EINVAL);	
	if (scanning) {
		for(dp = async_bp->b_forw; dp != async_bp; dp = dp->b_forw) {
			bp = &dp->bp;
			if(bp->b_dev != dev)
				continue;
			if(bp->b_proc != u.u_procp)
				continue;
			doown++;
			if(bp->b_flags&B_BUSY)
				continue;
			BOP_SPLX(s);
			*revents = *events; /* Found a non-busy buffer */
			return(0);
		}
		if(bp->b_proc->p_wchan == (caddr_t) &selwait)
			if (*events & POLLNORM)
				bp->b_selflags |= ASYNC_RCOLL;
		if (*events & POLLOUT)
			bp->b_selflags |= ASYNC_WCOLL;
		if(!doown) /* None owned so must be ok */
			*revents = *events;
	}
	BOP_SPLX(s);
#endif
	return(0);	
}


/*
 * define jump/vector/function table for dma map functions
 * and initialize it to safe or catch-something-wrong functions.
 */
struct dma_callsw	dma_callsw = { 
				no_dma_map_alloc,
				no_dma_map_load,
				no_dma_map_unload,
				no_dma_map_dealloc,
				null_dma_min_bound 	/* default to 1 */
			};
/*
 *  This routine will fill in the dma_callsw table, 
 *  which is platform-specific.  
 */
dma_callsw_init(alloc_fcn,load_fcn,unload_fcn,dealloc_fcn,min_bound_fcn)
	u_long (*alloc_fcn)();    /* ptr. to HAL's dma_map_alloc funct.   */
	u_long (*load_fcn)();     /* ptr. to HAL's dma_map_load funct.    */
	int	(*unload_fcn)();    /* ptr. to HAL's dma_map_unload funct.  */
	int	(*dealloc_fcn)();   /* ptr. to HAL's dma_map_dealloc funct. */
	int	(*min_bound_fcn)(); /* ptr. to HAL's dma_min_boundary funct */
{
	dma_callsw.hal_dma_map_alloc = alloc_fcn;
	dma_callsw.hal_dma_map_load  = load_fcn;
	dma_callsw.hal_dma_map_unload = unload_fcn;
	dma_callsw.hal_dma_map_dealloc = dealloc_fcn;
	dma_callsw.hal_dma_min_bound = min_bound_fcn;
}

/*
 * init-stubs to catch when system does not have dma_map'ing
 * functions setup before a driver tries to use them
 */
u_long
no_dma_map_alloc()
{
	panic("dma_map_alloc unimplemented");
}

u_long
no_dma_map_load()
{
	panic("dma_map_load unimplemented");
}

int
no_dma_map_unload()
{
	panic("dma_map_unload unimplemented");
}

int
no_dma_map_dealloc()
{
	panic("dma_map_unload unimplemented");
}

int
no_dma_min_bound()
{
	panic("dma_min_boundary unimplemented");
}

/*
 * For the record, ought to put null_dma_map_*()
 * routines here for systems that require a successful
 * call that does nothing.
 */

u_long
null_dma_map_alloc()
{
	return(0L);
}

u_long
null_dma_map_load()
{
	return(0L);
}

int
null_dma_map_unload()
{
        return(0);
}

int
null_dma_map_dealloc()
{
        return(0);
}

int
null_dma_min_bound()
{
        return(1);
}


/************************************************************************/
/*									*/
/* DMA memory zone code							*/
/* 									*/
/************************************************************************/

/*
 * zone indices
 */
#define SGLIST_ZONE 0
#define SUPER_SGLIST_ZONE 1

#define SGLIST_ZONE_SIZE (sizeof(struct sglist))
#define SUPER_SGLIST_ZONE_SIZE (sizeof(struct sglist)+sizeof(struct ovrhd))

#define DMA_ZONE_MAX 4

struct zone *dma_zone[DMA_ZONE_MAX];

/*
 * Some system-level params driving the following zone size choices:
 *	Max. enet packet: approx. 1.5K
 *	Max. fddi packet: approx. 4.0K
 *	Floppy max xfer : 	   .5K
 *	Typical disk transfers:	  4.0K
 *				  8.0K
 *				 64.0K
 *
 * So, anything less than or equal to 8K fits in two ba-bc pairs (assume
 * non-page-aligned).  Add 2 guard pages worse case, and these transfers
 * fit in 4 ba-bc pairs, 16 bytes per ba-bc pair, or 64-bytes. In V2.0
 * this translates into an element 2X sizeof(struct sglist); 
 * sizeof(struct ovrhd) = sizeof(struct sglist) in V2.0
 */
static char *dma_zone_name[DMA_ZONE_MAX] = {
	"dmazone.sglist",   /* common for linked sglist structs, DMA > 4MB */
			    /* -- also babc lists for transfers <= 8K     */
	"dmazone.ssglist",  /* common for:  1st, larger/super sglist, one/DMA */
	"dmazone.64k",      /* common for babc lists for 64K transfers */
	"dmazone.big",      /* common for babc lists for > 64K transfers */
};
unsigned int dma_zone_size[DMA_ZONE_MAX] = {
      SGLIST_ZONE_SIZE,		/*     32 Byte  */
      SUPER_SGLIST_ZONE_SIZE,	/*     64 Byte  */
      256,			/*    246 Byte  */
      8192,			/*   8192 Byte  */
};
/*
 * CAM scaled all zones to 4K entries each, so scale these relative
 * to CAM zones (4K for 64K transfers, others scaled up & down based
 * on use in dma mem. allocators).
 */
/* 32-byte zone used for large io's (linked sglist struct) & <= 8K xfers
 * 64-byte zone used for super sglist structs & xfers >8K, <= 24K xfers.
 * 256-byte zone used for >24K <=120K transfers; most common for 64K disk (CAM) io.
 * 8192-byte zone geared for larger io's (up to 4MBytes/zone element).
 */
unsigned long dma_zone_max[DMA_ZONE_MAX] = {
      134217728,	/* 128M 32 Byte elements max */
      268435456,	/* 256M 64 Byte elements max */
      134217728,	/* 128M 256 Byte elements max */
       67108864,	/*  64M 8192 Byte elements max */
};

/*
 * Size (in bytes) of memory allocated when a zone is empty 
 * (or all used up) and needs to expand.
 */
unsigned long dma_zone_alloc[DMA_ZONE_MAX] = {
	32768,		/*  4 pages, 32768/32    = 1024 elements */
	131072,		/* 16 pages, 131072/64   = 2048 elements */
	262144,		/* 32 pages, 262144/256  = 1024 elements */
	131072,		/* 16 pages, 131072/8192 =   16 elements */
};


int dma_zones_initialized = 0;

/************************************************************************/
/*									*/
/* dma_zones_init  -  initialize memory zones for allocation by dma     */
/*		      map support subsystem.				*/
/*
/*									*/
/************************************************************************/
void
dma_zones_init()
{
    int 		i, size;
    vm_offset_t 	dma_zone_addr;

    if (dma_zones_initialized)
	return;

    dma_zones_initialized++;

    for( i = 0; i < DMA_ZONE_MAX; ++i ) {
	dma_zone[i] = zinit( dma_zone_size[i], 
			     dma_zone_max[i] * dma_zone_size[i],
			     dma_zone_alloc[i], 
			     dma_zone_name[i]);
	/*
	 * panic if memory not available; drivers depending on dma_map_*()
	 *	to work in some minimal fashion in order to do any io.
	 *	w/o io, we have no system!
	 */
	if (dma_zone[i] == (vm_offset_t)NULL)
		panic("No zone mem. avail. for dma subsystem support");

	/* format: zchange(zone,pageable,sleepable,exhaustible,collectable); */
	zchange( dma_zone[i],FALSE,FALSE,TRUE,TRUE); 
    }
}

/************************************************************************/
/*									*/
/* dma_zalloc  -  babc (scatter-gather) list memory allocator		*/
/*									*/
/* SYNOPSIS                                                             */
/*	vm_offset_t dma_zalloc(unsigned int size, int flags)		*/
/*									*/
/* PARAMETERS                                                           */
/*	size	Number of bytes to allocate to store ba-bc pairs.	*/
/*	flags	DMA_* flags to indicate whether SLEEP-ing on mem.	*/
/*		allocation ok/allowed if zone memory all used up.	*/
/*									*/
/* DESCRIPTION                                                          */
/*	This function calls the OSF/1 zone allocation interfaces	*/
/*	to provide requested memory size. Zones are used for fixed-	*/
/*	size, fast memory allocation.  If DMA_SLEEP is set, then zalloc */
/*	called, since this function may go to sleep if out of zone	*/
/*	memory.  If DMA_SLEEP not set, then do a zget() which will not  */
/*	SLEEP when memory resources are used up, and more mem. needs 	*/
/*	to be allocated.						*/
/*									*/
/*	V2.0 DMA zone memory allocator.					*/
/*									*/
/* RETURN VALUES                                                        */
/*	Either a pointer to the beginning of the requested-size 	*/
/*	(contiguous) memory is returned, or if out-of/can't-get memory  */
/*	0 is returned.							*/
/*									*/
/************************************************************************/

vm_offset_t
dma_zalloc(unsigned int size, int flags)
{
    	int	zindex;
	vm_offset_t addr;
	sg_entry_t	sg_entryp;

    /* compute the size of the block that we will actually allocate */
    /*
     * Note: if size of babc list > last zone size, get last (max) zone 
     */
    for( zindex = 0; zindex < DMA_ZONE_MAX; ++zindex ) {
        if( size <= dma_zone_size[zindex] ) {
            break;
        }
    }
	
    /* Try fast zget 1st; if fail, check cold & sleep flags for zalloc */
    if ((addr = (vm_offset_t) zget(dma_zone[zindex])) == (vm_offset_t)NULL) {
	if (cold || (flags & DMA_SLEEP)) {
		addr = (vm_offset_t) zalloc(dma_zone[zindex]);
	}
    }

    /* store no. of babc/sg_entry entries in 1st longword of alloc'd zone */
    if (addr != (vm_offset_t)NULL) {
	sg_entryp = (sg_entry_t)addr;
	(unsigned int)sg_entryp->ba = 
		(unsigned int)dma_zone_size[zindex]/sizeof(struct sg_entry);
    }

    /* note: memory is not bzero'd: caller will setup/init this space */
    return(addr);
}


/************************************************************************/
/*									*/
/* dma_zalloc_super_sglist -  super/first sglist struct mem. allocator	*/
/*									*/
/* SYNOPSIS                                                             */
/*	vm_offset_t dma_zalloc_super_sglist(int flags)			*/
/*									*/
/* PARAMETERS                                                           */
/*	flags	DMA_* flags to indicate whether SLEEP-ing on mem.	*/
/*		allocation ok/allowed if zone memory all used up.	*/
/*									*/
/* DESCRIPTION                                                          */
/*	This function calls the OSF/1 zone allocation interfaces	*/
/*	to provide requested memory size. Zones are used for fixed-	*/
/*	size, fast memory allocation.  If DMA_SLEEP is set, then zalloc */
/*	called, since this function may go to sleep if out of zone	*/
/*	memory.  If DMA_SLEEP not set, then do a zget() which will not  */
/*	SLEEP when memory resources are used up, and more mem. needs 	*/
/*	to be allocated.						*/
/*	This function is provided/used instead of dma_zalloc() for      */
/*	speed reasons (no looping through zone-size table to get a size */
/*	Allocated memory is bzero'd. 					*/
/*									*/
/*	V2.0 DMA zone memory allocator.					*/
/*									*/
/* RETURN VALUES                                                        */
/*	Either a pointer to the beginning of the requested-size 	*/
/*	(contiguous) memory is returned, or if out-of/can't-get memory  */
/*	0 is returned.							*/
/*									*/
/************************************************************************/

vm_offset_t
dma_zalloc_super_sglist(int flags)
{
	vm_offset_t	addr;

    /* Try fast zget 1st; if fail, check cold & sleep flags for zalloc */
    if ((addr = (vm_offset_t) zget(dma_zone[SUPER_SGLIST_ZONE])) == (vm_offset_t)NULL) {
	if (cold || (flags & DMA_SLEEP)) {
		addr = (vm_offset_t) zalloc(dma_zone[SUPER_SGLIST_ZONE]);
	}
    }

    if (addr != (vm_offset_t)NULL)
	bzero(addr, SUPER_SGLIST_ZONE_SIZE);

    return(addr);
}


/************************************************************************/
/*									*/
/* dma_zalloc_sglist -  (linked) sglist struct mem. allocator		*/
/*									*/
/* SYNOPSIS                                                             */
/*	vm_offset_t dma_zalloc_sglist(int flags)			*/
/*									*/
/* PARAMETERS                                                           */
/*	flags	DMA_* flags to indicate whether SLEEP-ing on mem.	*/
/*		allocation ok/allowed if zone memory all used up.	*/
/*									*/
/* DESCRIPTION                                                          */
/*	This function calls the OSF/1 zone allocation interfaces	*/
/*	to provide requested memory size. Zones are used for fixed-	*/
/*	size, fast memory allocation.  If DMA_SLEEP is set, then zalloc */
/*	called, since this function may go to sleep if out of zone	*/
/*	memory.  If DMA_SLEEP not set, then do a zget() which will not  */
/*	SLEEP when memory resources are used up, and more mem. needs 	*/
/*	to be allocated.						*/
/*	This function is provided/used instead of dma_zalloc() for      */
/*	speed reasons (no looping through zone-size table to get a size */
/*	Allocated memory is bzero'd. 					*/
/*									*/
/*	V2.0 DMA zone memory allocator.					*/
/*									*/
/* RETURN VALUES                                                        */
/*	Either a pointer to the beginning of the requested-size 	*/
/*	(contiguous) memory is returned, or if out-of/can't-get memory  */
/*	0 is returned.							*/
/*									*/
/************************************************************************/

vm_offset_t
dma_zalloc_sglist(int flags)
{
	vm_offset_t	addr;

    /* Try fast zget 1st; if fail, check cold & sleep flags for zalloc */
    if ((addr = (vm_offset_t) zget(dma_zone[SGLIST_ZONE])) == (vm_offset_t)NULL) {
	if (cold || (flags & DMA_SLEEP)) {
		addr = (vm_offset_t) zalloc(dma_zone[SGLIST_ZONE]);
	}
    }

    if (addr != (vm_offset_t)NULL)
	bzero(addr, SGLIST_ZONE_SIZE);

    return(addr);
}


/************************************************************************/
/*									*/
/* dma_zfree  -  dma zone memory deallocator 				*/
/*									*/
/* SYNOPSIS                                                             */
/*	dma_zfree(unsigned int size, vm_offset_t addr)			*/
/*									*/
/* PARAMETERS                                                           */
/*	size	Number of bytes to deallocate. Assumption is that 	*/
/*		size to be deallocated will fit into respective alloc.  */
/*		zone.							*/
/*	addr	Address/pointer to contiguous (zone) memory to dealloc. */
/*									*/
/* DESCRIPTION                                                          */
/*	This function calls the OSF/1 zone free interface to free	*/
/*	up/return the zone-block of memory. Zones are used for fixed-	*/
/*	size, fast memory allocation.  zfree does a spin-lock to	*/
/*	return the memory, but does *not* (and assumptions are made     */
/*	that the interface will not) go to SLEEP while freeing memory.	*/
/*									*/
/*	V2.0 DMA zone memory allocator.					*/
/*									*/
/* RETURN VALUES                                                        */
/*	None.								*/
/*									*/
/************************************************************************/

void
dma_zfree(unsigned int size, vm_offset_t addr)
{
    int 		zindex;

    for( zindex = 0; zindex < DMA_ZONE_MAX; zindex++ ) {
	if( size <= dma_zone_size[zindex] ) {
	    break;
	}
    }
        zfree(dma_zone[zindex], addr);
}


/************************************************************************/
/*									*/
/* dma_zfree_super_sglist  -  deallocate super sglist zone element	*/
/*									*/
/* SYNOPSIS                                                             */
/*	dma_zfree_super_sglist(vm_offset_t addr)			*/
/*									*/
/* PARAMETERS                                                           */
/*	addr	Address/pointer to super sglist to deallocate. 		*/
/*									*/
/* DESCRIPTION                                                          */
/*	This function calls the OSF/1 zone free interface to free	*/
/*	up/return the zone-block of memory. Zones are used for fixed-	*/
/*	size, fast memory allocation.  zfree does a spin-lock to	*/
/*	return the memory, but does *not* (and assumptions are made     */
/*	that the interface will not) go to SLEEP while freeing memory.	*/
/*									*/
/*	V2.0 DMA zone memory allocator.					*/
/*									*/
/* RETURN VALUES                                                        */
/*	None.								*/
/*									*/
/************************************************************************/

void
dma_zfree_super_sglist(vm_offset_t addr)
{
    zfree(dma_zone[SUPER_SGLIST_ZONE], addr);
}


/************************************************************************/
/*									*/
/* dma_zfree_sglist  -  deallocate sglist structure/zone element	*/
/*									*/
/* SYNOPSIS                                                             */
/*	dma_zfree_sglist(vm_offset_t addr)				*/
/*									*/
/* PARAMETERS                                                           */
/*	addr	Address/pointer to sglist struct to deallocate. 	*/
/*									*/
/* DESCRIPTION                                                          */
/*	This function calls the OSF/1 zone free interface to free	*/
/*	up/return the zone-block of memory. Zones are used for fixed-	*/
/*	size, fast memory allocation.  zfree does a spin-lock to	*/
/*	return the memory, but does *not* (and assumptions are made     */
/*	that the interface will not) go to SLEEP while freeing memory.	*/
/*									*/
/*	V2.0 DMA zone memory allocator.					*/
/*									*/
/* RETURN VALUES                                                        */
/*	None.								*/
/*									*/
/************************************************************************/

void
dma_zfree_sglist(vm_offset_t addr)
{
    zfree(dma_zone[SGLIST_ZONE], addr);
}

/************************************************************************/
/*									*/
/* DMA mapping support functions used by drivers 			*/
/*									*/
/************************************************************************/

/************************************************************************/
/*									*/
/* dma_get_next_sgentry  -  gets a pointer to the next sg_entry struct 	*/
/*			    in a babc/scatter-gather list.		*/
/*									*/
/* SYNOPSIS                                                             */
/*	sg_entry_t	dma_get_next_sgentry(struct sglist *sglistp)	*/
/*									*/
/* PARAMETERS                                                           */
/*	sglistp	Address/pointer to an sglist struct assoc. w/this DMA. 	*/
/*									*/
/* DESCRIPTION                                                          */
/*	This function returns a pointer to a bus_address/byte_count	*/
/*	(ba-bc) pair/structure that drivers and lower-level bus  	*/
/*	code can use to access sequential ba-bc pairs w/o knowing the	*/
/*	the exact layout of a scatter-gather list.			*/
/*									*/
/* RETURN VALUES                                                        */
/*	Pointer to an sg_entry structure (ba-bc pair) or		*/
/*	0 for no more valid entries.					*/
/*									*/
/************************************************************************/

sg_entry_t
dma_get_next_sgentry(sglistp)
	struct	sglist	*sglistp;
{
	sg_entry_t	sgentryp = (sg_entry_t)(0);
	int		index = sglistp->index;;

	/* assume index points to desired ba-bc pair; update after fetch */
	if (index < sglistp->val_ents) { 
	     sgentryp = &sglistp->sgp[index];
	     sglistp->index++;
	/* when index >= val_ents, all ba-bc sets been read in this list */
	} else	{
	     /* rtn 0 by def. if index > no. valid ba-bc sets, no more lists */
	     if (sglistp->next != (struct sglist *)0)
		sgentryp = dma_get_next_sgentry(sglistp->next);
	}	
	
	return(sgentryp);
}

/*
 * dma_get_curr_sgentry() returns a pointer to a ba-bc 
 * (pair) structure that drivers and lower-level bus
 * support code can use to access "ba" and "bc"
 * elements in a sg_entry structure.
 *
 * Return values:
 *	pointer to sg_entry structure (ba-bc pair)
 *  or  0 for no more valid entries.
 */
sg_entry_t
dma_get_curr_sgentry(sglistp)
	struct	sglist	*sglistp;
{
	sg_entry_t	sgentryp = (sg_entry_t)(0);
	int		index = sglistp->index;;

	/* assume index points to desired ba-bc pair; update after fetch */
	if (index < sglistp->val_ents) { 
		sgentryp = &sglistp->sgp[index];
	/* when index >= val_ents, all ba-bc sets been read in this list */
	} else	{
		/* rtn 0 if index > no. valid ba-bc sets, no more lists */
		if (sglistp->next != (struct sglist *)0)
			sgentryp = dma_get_curr_sgentry(sglistp->next);
	}	
	return(sgentryp);
}

/*
 * dma_put_curr_sgentry() puts a ba-bc (pair) structure 
 * into an existing/pre-allocated & pre-loaded babc list.
 * This function is used in DMA "error handling" paths
 * where DMA transfers are not completed to their initial
 * "bc" length (due to some sort of transfer interruption)
 * and the driver needs to push (on writes to main memory)
 * the data into a "safe" buffer that can be merged with
 * existing (already transferred) data.
 * For example: the C94 on TC breaks in the middle of a
 * DMA due to a SCSI disconnect by a SCSI device. Since
 * TC dma_min_boundary is > 1 (not byte-divisible), need
 * to patch up the s-g table and restart the io from the
 * breakpt. in order to complete the io transfer.
 *
 * Return values:
 *	0 if sgentry pointer is invalid.
 *	1 if successful.
 */
int
dma_put_curr_sgentry(sglistp, new_sg_entryp)
	sglist_t	sglistp;
	sg_entry_t	new_sg_entryp;
{
	sg_entry_t	sgentryp = (sg_entry_t)(0);
	int		index = sglistp->index;;

	/* assume index points to desired ba-bc pair; update after fetch */
	if (index < sglistp->val_ents) { 
		sgentryp = &sglistp->sgp[index];
		sgentryp->ba = new_sg_entryp->ba;
		sgentryp->bc = new_sg_entryp->bc;
		return(1);
	} else { /* when index >= val_ents, all ba-bc sets been read in this list */
	        /* rtn 0 by default if index > no. valid ba-bc sets, no more lists */
		if (sglistp->next != (struct sglist *)0)
			return(dma_put_curr_sgentry(sglistp->next, new_sg_entryp));
		else
			return(0);	/* end of all babc's; can't do a put */
	}	
}

/*
 * dma_put_prev_sgentry() puts a ba-bc (pair) structure 
 * into an existing/pre-allocated & pre-loaded babc list.
 * This function is used in DMA "error handling" paths
 * where DMA transfers are not completed to their initial
 * "bc" length (due to some sort of transfer interruption)
 * and the driver needs to push (on writes to main memory)
 * the data into a "safe" buffer that can be merged with
 * existing (already transferred) data.
 * For example: the C94 on TC breaks in the middle of a
 * DMA due to a SCSI disconnect by a SCSI device. Since
 * TC dma_min_boundary is > 1 (not byte-divisible), need
 * to patch up the s-g table and restart the io from the
 * breakpt. in order to complete the io transfer.
 *
 * Return values:
 *	0 if structure &/or sgentry's such that put cannot be done.
 *	1 if successful.
 */
int
dma_put_prev_sgentry(sglistp, new_sg_entryp)
	sglist_t	sglistp;
	sg_entry_t	new_sg_entryp;
{
	int		index = sglistp->index;
	int		put_last_sgentry = 0;


	/* Case 0: sgentry in this sglist struct avail for mod. */
	/*	 : High-freq. case -- so, check 1st!		*/
	/*	 : Middle of sgentry list			*/
	if ((index > 0) && (index < sglistp->val_ents)) {
		put_last_sgentry++;
	}

	/* Case 1: "previous" is end of current sglist's sgentry's */
	if (index >= sglistp->val_ents)
		if (sglistp->next != (sglist_t)0) { 
			/* above if avoids mem. flt. in next line 
			 * if no next sglist exists 		*/
			/* Beginning of new sglistp; go back 1 sglist */
			if (sglistp->next->index == 0)
				put_last_sgentry++;
		} else { /* At end of current sglist's sgentry's */
			put_last_sgentry++;
		}

	if (put_last_sgentry) {
		index = --(sglistp->index);
		sglistp->sgp[index].ba = new_sg_entryp->ba;
		sglistp->sgp[index].bc = new_sg_entryp->bc;
		return(1);
	}

	/* Case 2: top of all sglist's, index at 1st sgentry */
	if (index == 0)
		return(0);	/* No previous sgentry to modify! */

	/* Case 3: "put" farther down in chain/link of sglist struct's 
	 *	 : Note: Above cases must be done before this one
	 *	   to avoid if-check's on index & sglist->next values.
	 *	 : Note: Case 1 above ensures "index" value of "next"
	 *		 sglist will not be 0, so Case 2 never occurs
	 *		 in recursive call.
	 */
	return(dma_put_prev_sgentry(sglistp->next, new_sg_entryp));

}

/************************************************************************/
/*									*/
/* dma_kmap_buffer  -  gets a kernel virtual address (kseg) of the      */
/*		       memory location involved in a DMA transfer.	*/
/*									*/
/* SYNOPSIS                                                             */
/*	vm_offset_t	dma_kmap_buffer(sglist_t sglistp, long offset)	*/
/*									*/
/* PARAMETERS                                                           */
/*	sglistp	Address/pointer to an sglist struct assoc. w/this DMA. 	*/
/*	offset	Byte offset from beginning of DMA buffer pointed to 	*/
/*		by original procp-va associated with the sglistp var.   */
/*									*/
/* DESCRIPTION                                                          */
/*	This function returns a kseg address of the memory location	*/
/*	pointed to by va+offset.  The function gets the proc struct	*/
/*	pointer from the sglist structure's overhead area, does a 	*/
/*	physical address fetch of the associated va+offset (in context	*/
/*	of assoc. proc), and then coerces the address to a kseg		*/
/*	address.							*/
/*									*/
/* RETURN VALUES                                                        */
/*	Kseg/kernel virtual address of va+bc, if valid. If address 	*/
/*	translation returns invalid, then 0 is returned.		*/
/*									*/
/************************************************************************/

vm_offset_t
dma_kmap_buffer(sglistp, offset)
	sglist_t	sglistp;
	u_long		offset;
{
	struct  ovrhd   *sgl_ovrhdp = (struct ovrhd *)(&sglistp[1]);
	vm_offset_t     phys_addr;
	vm_offset_t	va_to_map;
	struct	proc 	*procp;

	va_to_map = sgl_ovrhdp->va + (vm_offset_t)offset;
	procp = sgl_ovrhdp->procp;

	if (procp) {
		phys_addr = pmap_extract(procp->task->map->vm_pmap,va_to_map);
	} else {       /* 0 == kernel */
		svatophys(va_to_map,&phys_addr);
	}

	return(PHYS_TO_KSEG(phys_addr));
}

/************************************************************************/
/*									*/
/* drvr_register_shutdown						*/
/*		       register calback routine to be called when	*/
/*			system performs shutdown.			*/
/*									*/
/* SYNOPSIS                                                             */
/*	void drvr_register_shutdown(callback, param, flags)		*/
/*									*/
/* PARAMETERS                                                           */
/*	callback Driver routine to be called at system shutdown		*/
/*	param    Parameter passed to driver shutdown routine		*/
/*	flags	 flags field						*/
/*									*/
/* DESCRIPTION                                                          */
/*	This function registers or de-registers a shutdown routine	*/
/*	for the calling driver. This routine is called just prior	*/
/*	to system halt. This allows drivers to gracefully quiesce	*/
/*	its hardware.							*/
/*									*/
/* RETURN VALUES                                                        */
/*	None								*/
/*									*/
/************************************************************************/

struct drvr_shut *drvr_shut_head = 0;

void
drvr_register_shutdown(callback, param, flags)
	void		(*callback)();
	caddr_t		param;
	int		flags;
{
	struct drvr_shut *drvr_shut, *cur_dvr, *prev_dvr;

/*
 *	Driver shutdown registration
 *
 *	Allocate data structure and put onto linked list.
 *	This list is processed at shutdown time.
 */
	if (flags & DRVR_REGISTER) {
		drvr_shut = (struct drvr_shut *)kalloc(sizeof(struct drvr_shut));
		if (drvr_shut == NULL)
			panic("Unable to register driver shutdown routine");
		drvr_shut->callback = callback;
		drvr_shut->param = param;
		drvr_shut->next = drvr_shut_head;
		drvr_shut_head = drvr_shut;
	}
/*
 *	Driver shutdown unregister
 *
 *	Remove entry from linked list that matches the paramater and routine
 *	in this call.
 */
	else if (flags & DRVR_UNREGISTER) {
		prev_dvr = 0;
		cur_dvr = drvr_shut_head;
		while (cur_dvr) {
			if ((cur_dvr->callback == callback) &&
			    (cur_dvr->param == param)) {
				if (prev_dvr == 0)
					drvr_shut_head = cur_dvr->next;
				else
					prev_dvr->next = cur_dvr->next;
				kfree(cur_dvr, sizeof(struct drvr_shut));
				break;
			}
			prev_dvr = cur_dvr;
			cur_dvr = cur_dvr->next;
		}
	}
	else {
		panic("Invalid flag parameter for drvr_register_shutdown");
	}
}



/************************************************************************/
/*									*/
/* drvr_shutdown							*/
/*		       call driver shutdown routines			*/
/*									*/
/* SYNOPSIS                                                             */
/*	drvr_shutdown()							*/
/*									*/
/* PARAMETERS                                                           */
/*									*/
/* DESCRIPTION                                                          */
/*	This routine calls the driver shutdown routines that have been	*/
/*	registered by calls to drvr_register_shutdown. The global	*/
/*	drvr_shutdown_head points to a linked list of driver routines	*/
/*	to call.							*/
/*									*/
/* RETURN VALUES                                                        */
/*									*/
/************************************************************************/

void
drvr_shutdown()
{
	struct drvr_shut *cur_dvr, *prev_dvr;

	cur_dvr = drvr_shut_head;
	while (cur_dvr) {
		(*cur_dvr->callback)(cur_dvr->param);
		prev_dvr = cur_dvr;
		cur_dvr = cur_dvr->next;
		kfree(prev_dvr, sizeof(struct drvr_shut));
	}
}


/************************************************************************/
/*									*/
/* dma_put_private  -  puts data in the dma handle's			*/
/*		       	      private storage space.			*/
/*									*/
/* SYNOPSIS                                                             */
/*	int	dma_put_private(struct sglist sglistp, int index, 	*/
/*							u_long data)	*/
/*									*/
/* PARAMETERS                                                           */
/*	sglistp	Address/pointer to an sglist struct assoc. w/this DMA. 	*/
/*	index  	   Index to point to which private data area/entry.	*/
/*	data	   64-bit data to store in private storage area.	*/
/*									*/
/* DESCRIPTION                                                          */
/*	This function stores the passed in private data into the sglist */
/*	structure that was allocated by a driver in a previous		*/
/*	dma_map_alloc() call (or indirectly through a dma_map_load() 	*/
/*	call).								*/
/*									*/
/* RETURN VALUES                                                        */
/*	If index is a valid one, 0 is returned as successful.		*/
/*	If the index is out-of-range/invalid, 1 is returned as failure. */
/*									*/
/* NOTE: V1.0 of this interface only accepts an index value of zero/0.	*/
/*                                                                      */
/************************************************************************/

int
dma_put_private(sglistp, index, data)
	sglist_t	sglistp;
	int		index;
	u_long		data;
{
	struct  ovrhd   *sgl_ovrhdp = (struct ovrhd *)(&sglistp[1]);

	if (index == 0) { /* V1.0 -- only index of 0 is valid */
		sgl_ovrhdp->private = data;
		return(1);
	} else {
		return(0); /* failure */
	}
}

/************************************************************************/
/*									*/
/* dma_get_private  -  gets the data in the dma_handle 			*/
/*		       	      private storage space.			*/
/*									*/
/* SYNOPSIS                                                             */
/*	int	dma_get_private(sglist_t sglistp, int index, 		*/
/*							u_long *data)	*/
/*									*/
/* PARAMETERS                                                           */
/*	sglistp	Address/pointer to an sglist struct assoc. w/this DMA. 	*/
/*	index	Index to point to which private data area/entry.	*/
/*	data	Address/pointer to location to store the read data.	*/
/*									*/
/* DESCRIPTION                                                          */
/*	This function reads/gets the private data out of the sglist	*/
/*	structure that a driver previously stored in the structure	*/
/*	with a dma_put_sglist_private() call. The data is placed in 	*/
/*	the location pointed to by *data.				*/
/*									*/
/* RETURN VALUES                                                        */
/*	If index is a valid one, 0 is returned as successful.		*/
/*	If the index is out-of-range/invalid, 1 is returned as failure. */
/*									*/
/* NOTE: V1.0 of this interface only accepts an index value of zero/0.	*/
/*                                                                      */
/************************************************************************/

int
dma_get_private(sglistp, index, data)
	sglist_t	sglistp;
	int		index;
	u_long		*data;
{
	struct  ovrhd   *sgl_ovrhdp = (struct ovrhd *)(&sglistp[1]);

	if (index == 0) { /* V1.0 -- only index of 0 is valid */
		*data = sgl_ovrhdp->private;
		return(1);
	} else {
		return(0); /* failure */
	}
}



/************************************************************************/
/*                                                                      */
/* NAME									*/
/*   do_config - Initializes board to its assinged configuration.	*/
/*									*/
/* SYNOPSIS								*/
/*   void  do_config (struct controller *ctlr_p)			*/
/*									*/
/* PARAMETERS								*/
/*   ctlr_p	Pointer to controller structure for device to be	*/
/*		initialized. 						*/
/*									*/
/* DESCRIPTION								*/
/*   Initializes the device based on its power up resource		*/
/*   assingments. If device uses either an interrupt or a DMA		*/
/*   channel then any setup required is performed at this time also.	*/
/*									*/
/************************************************************************/

void
do_config (struct controller *ctlr_p)

{  /* Begin do_config */
   
   if ( ctlr_p->bus_hd->busfuncs )
      (*((struct bus_funcs
	 *)(ctlr_p->bus_hd->busfuncs))->do_config)(ctlr_p);
      
}  /* End do_config */



/************************************************************************/
/* NAME									*/
/*   get_config - Gets assigned configuration data for a device.	*/
/*	   								*/
/* SYNOPSIS								*/
/*   int  get_config (struct controller *ctlr_p,			*/
/*			 uint_t config_item, char *func_type,		*/
/*			 void *data_p, int handle)			*/
/*				 					*/
/* PARAMETERS								*/
/*   ctlr_p	Pointer to controller structure for device.		*/
/*   	   								*/
/*   config_item	Item of configuration data desired. Legal values*/
/*		are RES_MEM, RES_IRQ, RES_DMA and RES_PORT which	*/
/*			are defined in io/common/devdriver.h.		*/
/*	   								*/
/*   func_type	Function type for which data is desired. For		*/
/*		example for EISA devices this is the function type	*/
/*		string that appears in the device's EISA config		*/
/*		file. Not all busses will require this			*/
/*		information.						*/
/*	   								*/
/*   data_p	Pointer to a structure appropriate for the data		*/
/*		requested. Structures are defined in 			*/
/*		io/common/devdriver.h. They are bus_mem, irq, dma and	*/
/*		io_port.						*/
/*	   								*/
/*   handle	Handle returned if there is more config data of		*/
/*		the type requested. ON INITIAL CALL THIS SHOULD BE	*/
/*		SET TO ZERO. On subsequent calls it should be set	*/
/*		to the value returned by the prior call.		*/
/*	   								*/
/* DESCRIPTION								*/
/*   get_config returns information about the requested resource.	*/
/*									*/
/* RETURN VALUE								*/
/*   If option has only one resource of the type requested its value is */
/*   placed in the data_p parameter and the function returns a value of */
/*   0. If option has multiple resources of the type requested the 	*/
/*   value at the head of the list is placed in the data_p parameter    */
/*   and the function returns a handle that points to the next element  */
/*   in the list. To get the next element call eisa_get_config again	*/
/*   with the returned handle passed in as the handle parameter. When   */
/*   all elements have been read eisa_get_config will return a value of */
/*   0. If option does not have a resource of the type requested a 	*/ 
/*   value of -1 is returned.					   	*/
/************************************************************************/

int
get_config (struct controller *ctlr_p, uint_t config_item, char *func_type,
	    void *data_p, int handle)

{  /* Begin get_config. */

   if ( ctlr_p->bus_hd->busfuncs )
      return ((*((struct bus_funcs
		 *)(ctlr_p->bus_hd->busfuncs))->get_config)(ctlr_p,
							   config_item,
							   func_type,
							   data_p, handle));


}  /* End get_config. */



/************************************************************************/
/*                                                                      */
/* NAME									*/
/* enable_option  -  Enables interrupts from specified option.	   	*/
/*                                                                      */
/* SYNOPSIS                                                             */
/*	void  enable_option (struct  controller  ctlr_p)		*/
/*                                                                      */
/* PARAMETERS								*/
/*	ctlr_p	Pointer to controller structure for option for which to */
/*		enable interrupts.					*/
/*                                                                      */
/* DESCRIPTION 								*/
/* 	Enables interrupts from option represented by the passed in 	*/
/* 	controler structure. 						*/
/************************************************************************/

void
enable_option (struct  controller  *ctlr_p)

{  /* Begin enable_option. */
   
   if ( ctlr_p->bus_hd->busfuncs )
      (*((struct bus_funcs
	 *)(ctlr_p->bus_hd->busfuncs))->enable_option)(ctlr_p);
      
}  /* End enable_option. */



/************************************************************************/
/*                                                                      */
/* NAME									*/
/* disable_option  -  Disables interrupts from specified option.	*/
/*                                                                      */
/* SYNOPSIS                                                             */
/*	void  disable_option (struct  controller  ctlr_p)		*/
/*                                                                      */
/* PARAMETERS								*/
/*	ctlr_p	Pointer to controller structure for option for which to */
/*		disable interrupts.					*/
/*                                                                      */
/* DESCRIPTION 								*/
/* 	Disables interrupts from option represented by the passed in 	*/
/* 	controler structure. 						*/
/************************************************************************/

void
disable_option (struct  controller  *ctlr_p)

{  /* Begin disable_option. */
   
   if ( ctlr_p->bus_hd->busfuncs )
      (*((struct bus_funcs
	 *)(ctlr_p->bus_hd->busfuncs))->disable_option)(ctlr_p);
      
}  /* End disable_option. */



/************************************************************************/
/*                                                                      */
/* NAME									*/
/*   busphys_to_iohandle - returns io_handle for bus physical address	*/
/*									*/
/* SYNOPSIS								*/
/*   io_handle_t  busphys_to_iohandle (u_long addr,                     */
/*                                      int flags,                      */
/*                                      struct controller *ctlr_p)	*/
/*									*/
/* PARAMETERS								*/
/*   addr	Bus physical address as seen from bus device            */
/*   flags      Address space to return handle for (BUS_IO|BUS_MEMORY|  */
/*                                                  DENSE_MEMORY)       */
/*   ctlr_p	Pointer to controller structure for device on the bus	*/
/*              the handle will be generated for.                       */
/*									*/
/* DESCRIPTION								*/
/*   Initializes the device based on its power up resource		*/
/*   assingments. If device uses either an interrupt or a DMA		*/
/*   channel then any setup required is performed at this time also.	*/
/*									*/
/************************************************************************/

io_handle_t
busphys_to_iohandle (u_long addr, int flags, struct controller *ctlr_p)

{  /* Begin busphys_to_iohandle */
   
   if ( ctlr_p->bus_hd->busfuncs )
      (*((struct bus_funcs
	 *)(ctlr_p->bus_hd->busfuncs))->busphys_to_iohandle)(addr,flags,ctlr_p);
      
}  /* End busphys_to_iohandle */
