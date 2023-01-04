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
static char *rcsid = "@(#)$RCSfile: dme_common.c,v $ $Revision: 1.1.10.3 $ (DEC) $Date: 1993/09/22 13:30:23 $";
#endif

/* ---------------------------------------------------------------------- */

/* dme_common.c		Version 1.02			Mar. 25, 1993  */

/*  This file contains the definitions and data structures needed by
    the DME related files.

Modification History

	Version	  Date		Who	Reason

	1.00	10/14/92	jag	Created this file from the 3min DME
					file, dme_3min_94_dma.c.
	1.01	11/05/92	jag	Changed the Working set allocation code
					to work with the dynaminc zone sizes.
	1.02	03/25/93	jag	Added the "concatinate" DATs routine.
					The SIM WS is not used to access the
					CCBs.  General comment updating.
*/

/* ---------------------------------------------------------------------- */
/*
    This file contains common support modules for the CAM Data Mover
Engine (DME) components within the SIM layers.  The DME component is an
evolution of the PDMA method of data transfer used in the previous
ULTRIX SCSI sub-system.  The DME's intent is to hide the details of
data movement in different HBA's, behind a well defined sequence of
higher level functions.  The DME common routines are equivilent to a
library component for user level programs.

    Currently the DME support in this file works with a data structure
called the DME working set, DMA_WSET, like the SIMs and PDrvs, the
DME common code works with there being a DMA_WSET structure for each
I/O.  Through the DMA_WSET structure are all the DME resources that
are needed for an I/O accessed.  The DME descriptor, DME_DESCRIPTOR,
is refereced via the working set, and through the DME_DESCRIPTOR the
SIM working set etal.

    The DME common code uses a DME control structure, DMA_CTRL, on a 
per-bus basis.  The DMA_CTRL structure is used to hold, and be a
locking point, for all the allocated DME resource pools.  For example
the working set and DAT pools.

    The DME code uses another data structure to go along with the
DMA_WSET, called a Data Address Translation, DAT, table.  An element in
the DAT table consists of addresses, flags, and data counts.  In
practice a unique DAT table is created for every I/O.  The "user"s
buffer and data count is "de-composed" into a DAT table of physical
address and the count of contigious data bytes contained at that
address.

    In this file are all the necessary support and manipulation routine
for using DAT tables for user I/O.  The data structures, DAT tables and
the DME working sets, are initialized, allocated, and freed using
routines in this file.

    The code that builds up the DAT table from the users Address/count
information, has an added ability to watch and handle hardware DMA
boundry constraints.  For example the IOASIC, on the 5000/100+ series
can only support DMA on 8 byte boundries, also the TC channel can only
support memory access on 4 byte boundries.  The DAT build routine uses
boundry limit information stored in the DME control structure.  The DAT
build routine uses the System "PAGE" size and the DMA boundry limit to
fill in the DAT elements with valid address and count information for
the buffer in main memory.

The DAT table consists of elements, and each element contains a two
address pointers, a byte count, and flags.  Two pointers are used, one
the physical address of this part of the buffer and the other is the
Kernel virtual address for the same location.  The count, System
physical page size, or zone, is how many contigious bytes are located
at the pointers.  The flags field is used for signaling and control
information, ie the DAT element is for one of the zones.  The code
tries to build a complete DAT to describe the entire I/O buffer,
however as demand increases it is possible that there are not enough
available DAT elements.  The code is capabable of using the allocated
DAT elements as a ring, reusing what is nolonger needed.  In the DAT
calculation code all address go through two manipulations, the
"working" address is converted to it's physical address and the
physical address is converted to a Kernel virtual address.  The Kernel
virtual address is there to allow the CPU to manupulate the user buffer
when necessary.

Any I/O transfer that contains starting address or ending addresses
that do not end on a Systems DMA boundary, has to have special DMA
pre-processing to allow DMA to occur.  The DME code uses "safe zones"
that are contained in the DME working sets.  These safe zones are X
bytes large and start on an X byte boundary.  All data bytes that are
not on a proper boundary in main memory are transfered via these zones,
with the DMA engines using the physical address and the CPU using
Kernel Virtual addresses.

This DME file can be used by DMA systems.  The setup for the allocation
and attachment for the DME resources is straight forward for example:

softc->dctrl = dcmn_alloc_ctrl();		/* alloc the control struct *
(void)dcmn_alloc_wsets( softc->dctrl );		/* set working set pool *
(void)dcmn_alloc_dats( softc->dctrl );		/* set DAT pool *

With the working set and DAT pool setup the resources are ready and available
to be used.

*/

/* ---------------------------------------------------------------------- */
/* Include files. */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/buf.h>
#include <sys/proc.h>

#include <kern/lock.h>
#include <mach/vm_param.h>
#include <machine/machparam.h>
#include <machine/pmap.h>

#include <io/dec/tc/tc.h>

#include <io/common/iotypes.h>
#include <io/cam/cam.h>
#include <io/cam/dec_cam.h>
#include <io/cam/scsi_all.h>
#include <io/cam/cam_debug.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim_common.h>		/* SIM common definitions. */
#include <io/cam/dme.h>			/* DME specific structs and consts */
#include <io/cam/dme_common.h>		/* DME common definitions. */
#include <io/cam/sim.h>			/* SIM specific structs and consts */
#include <io/cam/sim_94.h>		/* N53C94 specific */

/* ---------------------------------------------------------------------- */
/* Defines and includes for Error logging.  */

#define CAMERRLOG		/* Turn on the error logging code */

#include <dec/binlog/errlog.h>		/* UERF errlog defines */
#include <io/cam/cam_errlog.h>		/* CAM error logging macro */
#include <io/cam/cam_logger.h>		/* CAM error logging definess */

/* ---------------------------------------------------------------------- */
/* Function Prototypes: */

U32 dumpent();
void dcmn_dumptbl();
U32 dcmn_dat_alloc();
void dcmn_dat_free();
U32 dcmn_dat_build();
U32 dcmn_dat_calc();
U32 dcmn_dat_adj();
U32 dcmn_dat_concat();
U32 dcmn_get_avail_zone();
U32 dcmn_flush_safe_zone();
U32 dcmn_rtn_resources();
void * dcmn_avtophy();
DMA_CTRL * dcmn_alloc_ctrl();
void dcmn_free_ctrl();
U32 dcmn_alloc_wsets();
void dcmn_free_wsets();
U32 dcmn_alloc_dats();
void dcmn_free_dats();
DME_STRUCT * dcmn_alloc_dme();

/* ---------------------------------------------------------------------- */
/* External declarations: */

extern SIM_SOFTC *softc_directory[];	/* for accessing via "controller" */

extern int bcopy();			/* System routine to copy buffer  */
extern int bzero();			/* System routine to clear buffer */

extern char *sc_alloc();		/* SIM Common memory allocator */
extern void sc_free();			/* SIM Common memory deallocator */
extern int  sim_poll_mode;		/* Signal for the '94 cmd MACRO */

extern vm_offset_t pmap_extract();	/* magic VM code to get a phy addr */

/* ---------------------------------------------------------------------- */
/* Local Type Definitions and Constants */ 

/* ---------------------------------------------------------------------- */
/* Initialized and uninitialized data: */

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

/* Some General purpose DAT ring output routines. */

#ifdef CAMDEBUG

static U32
dumpent( d )
    DAT_ELEM *d;
{
    printf( "  flags=0x%x len=0x%x vaddr=0x%x paddr=0x%x\n",
	d->dat_flags, d->dat_count, d->dat_vaddr, d->dat_paddr);

    return( d->dat_count );
}
 
void
dcmn_dumptbl( dws )
    DMA_WSET *dws;
{
    int k;

    printf( "DAT dump, current DAT index %d\n", dws->current.index );
    for( k = 0; k < dws->de_count; k++ )
    {
        printf( " index: %d %c", k,
	    ((k == dws->current.index)?('>'):(' ')) );
        (void)dumpent( &(dws->de_base[k]) );
    }
}
#endif

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/*
Routine Name: dcmn_dat_alloc

Functional Description:
    This routine will scan the DAT rings attached to the DME control
    structure looking for the requested number of "free" DAT elements to
    assign to the DME working set.  If the requested number of DAT
    elements are not available the largest "group" of free DATs in the
    Rings will be returned.

Formal Parameters:
    The DME working set for the I/O
    The number of requested DATs to use for the I/O.

Implicit Inputs:
    The DME control structure and the attached DAT rings.

Implicit Outputs:
    The "found" number of DATs stored in the DME working set.

Return Value:
    The number of free DATs in the group allocated.

Side Effects:
    This routine does a series of linear searches though the DAT rings.
    This routine must be called SMP/IPL locked.  There may be a more 
    performance enhanced method of alloc-ing and freeing the DATs.  This
    will be left as "futures".

Additional Information:
    None
*/

U32
dcmn_dat_alloc( dws, needed )
    DMA_WSET *dws;			/* the I/Os working set */
    U32 needed;				/* the number of DATs needed */
{
    DMA_CTRL *dc;			/* for the control struct */
    register U32 ndpr;			/* loop cntr : Num Dats Per Ring */
    DAT_ATTCH *da;			/* for accessing the attach struct */
    register DAT_ELEM *de;		/* the working element pointer */

    register DAT_ELEM *alloc_de;	/* holder for current alloc-ed grp */
    register U32 alloc_cnt;		/* holder for current alloc-ed cnt */

    DAT_ATTCH *last_alloc_da;		/* holder for last alloc-ed attach */
    DAT_ELEM *last_alloc_de;		/* holder for last alloc-ed grp */
    U32 last_alloc_cnt;			/* holder for last alloc-ed cnt */

    /* Setup for the DAT Ring scans. */

    dc = GET_DMA_CTRL( dws );		/* get the DME control struct */

    last_alloc_cnt = 0;			/* havn't found any yet */
    last_alloc_de = (DAT_ELEM *)NULL;	/* same thing */
    last_alloc_da = (DAT_ATTCH *)NULL;	/* same thing */

    /* Do a reality check to see if this working set already has a 
    group of DATs attached.  If there are just return what was attached
    from before. */

    if( dws->de_base != (DAT_ELEM *)NULL )	/* Already some */
    {
	return( dws->de_count );		/* how many there are */
    }

    /* There are two while loops, one for the DAT attach list and the other
    for scanning the DAT rings. */

    da = dc->dat_group;			/* start at the head of the list */

    while( da != (DAT_ATTCH *)NULL )	/* at the end ? */
    {
	/* Setup the ring scanning parameters. */

	alloc_cnt = 0;			/* start w/new ring */
	alloc_de = (DAT_ELEM *)NULL;	/* same */

	de = da->dat_ring;			/* start at the ring top */
	ndpr = dc->dat_per_ring;		/* how many to search */

	while( ndpr != 0 )			/* search the whole ring */
	{
	    /* If a DAT is found that is free, add it to the current
	    list of free DATs. */

	    if( de->dat_flags == DAT_FREE )	/* find one */
	    {
		alloc_cnt++;		/* incr current count */

		/* Did this free one get all that were needed ? */

		if( alloc_cnt == needed )	/* got them all ! */
		{
		    last_alloc_cnt = alloc_cnt;	/* save the counts */
		    last_alloc_de = alloc_de;
		    last_alloc_da = da;
		    
		    break;				/* stop scanning */
		}

		/* Look at alloc_de if this is a new group save it. */

		if( alloc_de == (DAT_ELEM *)NULL )	/* nothing yet */
		{
		    alloc_de = de;			/* save it */
		}
	    }
	    else	/* "dat_flags != DAT_FREE" a DAT already alloc-ed */
	    {
		/* look at alloc_de to see if this terminates a group
		of free DATs. */

		if( alloc_de != (DAT_ELEM *)NULL )  /* were keeping track */
		{
		    /* If this group is larger than the last group then
		    save the info in the last_* variables. */

		    if( alloc_cnt > last_alloc_cnt )	/* update ! */
		    {
			last_alloc_cnt = alloc_cnt;	/* save the counts */
			last_alloc_de = alloc_de;
			last_alloc_da = da;
		    }

		    /* Clear out the current DAT counters to prepare for
		    the next free group. */

		    alloc_cnt = 0;
		    alloc_de = (DAT_ELEM *)NULL;
		}
	    }
	    de++;				/* next DAT element to check */
	    ndpr--;				/* loop again */
	}	/* end of Ring search while() */

	/* Check to see if the Ring scan loop did find enough free
	DATs to satisify the needed request.  If there were then
	simply break out of the outer while() loop. */

	if( last_alloc_cnt == needed )	/* find enough */
	{
	    break;				/* don't search anymore */
	}

	/* Check to see if the end of the ring was reached during a 
	group of free DATs.  Update the last_* variables if needed. */

	if( alloc_de != (DAT_ELEM *)NULL )	/* were keeping track */
	{
	    if( alloc_cnt > last_alloc_cnt )	/* need to update */
	    {
		last_alloc_cnt = alloc_cnt;		/* save the counts */
		last_alloc_de = alloc_de;
		last_alloc_da = da;
	    }
	}
	/* Now go on to the next DAT ring. */
	da = da->next;			/* next attach on the list */

    }	/* end of DAT attach list while() */

    /* If there were any free DATs found, using the last_* values set the
    DAT flags to be available for the I/Os use */

    if( last_alloc_de != (DAT_ELEM *)NULL )
    {
	da = last_alloc_da;			/* the lucky DAT ring */

	de = last_alloc_de;			/* the starting DAT */
	for( ndpr = 0; ndpr < last_alloc_cnt; ndpr++ )
	{
	    de->dat_flags = DAT_ALLOCED;
	    de++;				/* next DAT element */
	}

	/* Subtract the number of alloc-ed DAts in the Ring attach struct */

	da->dat_elem_free -= last_alloc_cnt;	/* keep up to date */
    }

    /* Put the alloc-ed DATs in to the working set for the I/O. */

    dws->de_base = last_alloc_de;	/* where they start */
    dws->de_count = last_alloc_cnt;	/* How many there are */
    dws->dat_grp = last_alloc_da;	/* where the ring is */

    /* Return to the caller the number of DATs that were found. */

    return( last_alloc_cnt );
}

/* ---------------------------------------------------------------------- */
/*
Routine Name: dcmn_dat_free

Functional Description:
    This routine will return a previously allocated group of DATs to
    the DAT ring.  The group of DATs will be set to "free" and the number
    of free DATs in the attach structure will be updated. 

Formal Parameters:
    The DME working set for the I/O

Implicit Inputs:
    The DME control structure and the attached DAT rings.

Implicit Outputs:
    The DME control structure and the attached DAT rings.

Return Value:
    None

Side Effects:
    None

Additional Information:
    This routine must be called SMP/IPL locked.
*/

void
dcmn_dat_free( dws )
    DMA_WSET *dws;			/* the I/Os working set */
{
    DAT_ATTCH *da;			/* for accessing the attach struct */
    register DAT_ELEM *de;		/* the working element pointer */
    register U32 i;			/* loop counter */

    /* From the I/O working set get the DAT ring information. */

    da = dws->dat_grp;
    de = dws->de_base;

    /* Loop through the DATs setting them to the DAT_FREE state. */

    for( i = 0; i < dws->de_count; i++ )
    {
	de->dat_flags = DAT_FREE;	/* free it up for future use */
	de++;				/* next DAT */
    }

    /* Add the returned number of DATs to the attach free list. */

    da->dat_elem_free += dws->de_count;	/* some more free ones */

    /* Clear out the working set DAT ring parameters. */

    dws->dat_grp = (DAT_ATTCH *)NULL;
    dws->de_base = (DAT_ELEM *)NULL;
    dws->de_count = 0;

    return;				/* all done */
}

/* ---------------------------------------------------------------------- */
/*
Routine Name: dcmn_dat_build

Functional Description:
    This routine will setup the "ahead" XFER_INFO data structre and
    prepare the working set zones.  The XFER_INFO current struct is used
    to setup the XFER_INFO ahead  struct.  The dcmn_dat_calc() routine
    is called to do the actual work using the XFER_INFO ahead struct to
    calculate the addresses and counts and place them in the DATs for the
    starting or restarting of an I/O.

Formal Parameters:
    The DME working set for the I/O

Implicit Inputs:
    None

Implicit Outputs:
    The DATs indicated in the DME working set.

Return Value:
    CAM_REQ_CMP		: all went well
    CAM_REQ_CMP_ERR		: all did not go well

Side Effects:
    None

Additional Information:
    None
*/

U32
dcmn_dat_build( dws )
    DMA_WSET *dws;                      /* pointer to the I/O working set */
{
    /* Make the safe zones available in the working set. */

    AVAILABLE_DAT_ZONES( dws );		/* Make all zones available */

    /* Now with the new xfer info data the DATs will have to be
    recalculated.  Any "ahead" DAT information is nolonger valid. Copy
    the current info into the ahead info and call the DAT calc code.
    All the DATs are used with keeping index 0 available for a
    possible disconnect on an aligned DAT. */

    dws->ahead.count = dws->current.count;
    dws->ahead.vaddr = dws->current.vaddr;

    dws->ahead.index = dws->current.index = 1;	/* restart the index */

    if( dcmn_dat_calc( dws, (dws->de_count - 1) ) != CAM_REQ_CMP)
    {
	return( CAM_REQ_CMP_ERR );
    }

    return( CAM_REQ_CMP );		/* all done */
}

/* ---------------------------------------------------------------------- */
/*
Routine Name: dcmn_dat_calc

Functional Description:
    This routine will setup and calculate the addresses and counts in
    the DATs for the I/O.  The XFER_INFO ahead struct is used to
    fill in the allowed number of DATs.  This routine is setup to be
    called by any of the formal routines, setup/resume/restore/modify.  

Formal Parameters:
    The DME working set for the I/O
    The number of available DATs to use in the calc.

Implicit Inputs:
    The DME DESC and SIM working set, and the I/O bp.

Implicit Outputs:
    The DATs indicated in the DME working set.

Return Value:
    CAM_REQ_CMP		: all went well
    CAM_REQ_CMP_ERR		: all did not go well

Side Effects:
    None

Additional Information:
    This routine has system dependent knowledge of pages and their size.
*/

U32
dcmn_dat_calc( dws, avail_dats )
    DMA_WSET *dws;                      /* pointer to the I/O working set */
    U32 avail_dats;			/* how many DATs can be used */
{
    /* Local variables */

    U32 loop;				/* loop counter variable */
    register U32 resid;			/* holds the masking results */
    U32 zone;				/* the current working zone flag */
    register U32 wrk_count;		/* the loops working down count */
    void *wrk_vaddr;			/* the loops working addr pointer */
    U32 dat_index;			/* the index to working DAT */
    register DAT_ELEM *d;		/* pointer to the working DAT */
    register U32 zonesize;		/* for holding the safe zone size */
    register U32 zonemask;		/* for holding the zone align mask */

    void *phy_addr;			/* physical addr for the DAT */
    void *vk0_addr;			/* Virtual KVA seg addr for the DAT */

    /* Pull the starting working values from the XFER_INFO ahead struct.
    These values are used in the loop. */

    dat_index = dws->ahead.index;	/* last valid DAT from before */
    wrk_vaddr = dws->ahead.vaddr;	/* last valid address */
    wrk_count = dws->ahead.count;	/* number of bytes left */

    /* Get the zone size and mask information from the DMA control structure
    for this bus. */

    zonesize = dws->dma_ctrl->dme_zone_size;	/* zone size for the bus */
    zonemask = zonesize - 1;			/* the mask for this size */

    /* In the odd case where this code may be called when there is
    no data to transfer, clear out the DAT flags, set DAT_FINAL and
    return. */

    if( wrk_count == 0 )
    {
	d = GET_DAT_PTR( dws, dat_index );	/* point to first avail DAT */
	d->dat_flags = (DAT_ALLOCED | DAT_FINAL);	/* signal last one */
	d->dat_count = 0;				/* just in case */
	return( CAM_REQ_CMP );				/* all done w/nothing */
    }

    /* Make sure that the DAT flags of all the available DAT elements are
    set/reset to the default, DAT_ALLOCED.  If this call is from a recalc
    or DAT_INCOMPLETE, the code has to make sure that there are no left
    over flags hanging around in the DATs that may be reused. */

    for( loop = 0; loop < avail_dats; loop++, dat_index =
						    NEXT_DAT(dws, dat_index) )
    {
	/* Set the pointer to the DAT to use. */

	d = GET_DAT_PTR( dws, dat_index );		/* point to it */

	/* Set/reset the DAT flags to the default, DAT_ALLOCED. */

	d->dat_flags = DAT_ALLOCED;		/* clear out all others */
    }

    /* This while loop will run until all the available DAT elements
    have been used up or a failure to alloc a safe zone for the xfer.
    The loop is setup to be a one pass loop this way only one DAT
    element is used for each pass.  Rather than having a series of
    cascading if()s, there is a continue statement following each of
    the if() address checks.  At each if() statement the address is
    "more" aligned.  There are a number of "duplicated" lines of code
    and decidision points that could be taken out.  This method was
    choosen for readability and simplicity.  This loop should not be
    called to "re-calc" the DATs all over again too often.
    In a nut shell the ASIC has the following boundaries, page and
    8 byte (octaword) alignment. */

    dat_index = dws->ahead.index;	/* reset back to the first available */
    for( loop = 0; loop < avail_dats; loop++, dat_index =
						    NEXT_DAT(dws, dat_index) )
    {
	/* Check to see if the loop has exhausted the data count there is
	no more need to run the decode loop. */

	if( wrk_count == 0 )		/* no more data bytes ? */
	{
	    break;			/* terminate the loop */
	}

	/* Set the pointer to the DAT to use. */

	d = GET_DAT_PTR( dws, dat_index );		/* point to it */

	/* Calc the KO and phys addresses for this working buffer address. */

	phy_addr = dcmn_avtophy( dws, wrk_vaddr );	/* get the phys addr */
	vk0_addr = (void *)CAM_PHYS_TO_KVA( phy_addr );	/* get the KVA addr */

	/* This first if() condition and code are checking for the ASIC
	boundary and will align the count and address to the next one.  If
	the count or the boundary does not start on a boundary then one of
	the safe zones will be used. */

	if( (resid = ((vm_offset_t)wrk_vaddr & zonemask)) != 0 ) /* octaword */
	{
	    /* The value in resid must contain the number of bytes that
	    are left to get to the next boundary.  The above if() produces
	    the address offset, to get the byte count the offset is
	    subtracted from the size. */

	    resid = zonesize - resid;		/* from offset to bytes */

	    /* Check to make sure that there are enough bytes left to 
	    transfer. */

	    if( wrk_count < resid )		/* only a few left */
	    {
		resid = wrk_count;		/* all the bytes left */
	    }

	    /* There has to be a safe zone allocated for this DAT entry.
	    If there is not one available, terminate the loop. */

	    if( (zone = dcmn_get_avail_zone( dws )) == 0 )
	    {
		break;					/* don't do any more */
	    }

	    /* If the data direction is a ASIC_TRANSMIT then the
	    selected zone has to have the data pre-loaded into it. */

	    if( dws->dir == ASIC_TRANSMIT )	/* data doesouta */
	    {
		bcopy( vk0_addr, ZONE_KADDR( dws, zone ), resid );   /* fill */
	    }

	    /* Load the DAT with the information for this part of the I/O */

	    d->dat_count = resid;		/* how many in the zone */
	    d->dat_vaddr = vk0_addr;		/* where in KVA space */
	    d->dat_paddr = ZONE_PADDR( dws, zone );	/* where in phys */
	    d->dat_flags |= (zone | DAT_VALID );	/* whats in it */

	    /* Correct the working address and counts with the resid. */

	    wrk_vaddr = (void *)((vm_offset_t)wrk_vaddr + resid); /* new addr */
	    wrk_count -= resid;				  /* down the counter */

	    continue;				/* back to the top */
	}

	/* This second if() condition and code are checking for the PAGE
	boundary and will align the count and address to the next one.  There
	also has to be a check for the ASIC boundaries.  If there are not
	enough bytes to go to the next PAGE boundary then the transfer can
	only go up to the next ASIC boundary, and the next iteration of
	the loop will take care of the zone alloc.
	NOTE:  The code flow made it past the above ASIC bountry check
	therefore this working address is on a ASIC boundary. */

	if( (resid = ((vm_offset_t)wrk_vaddr & DME_PAGE_MASK)) != 0 )/* page */
	{
	    /* The value in resid must contain the number of bytes that
	    are left to get to the next boundary.  The above if() produces
	    the address offset, to get the byte count the offset is
	    subtracted from the size. */

	    resid = DME_PAGE_SIZE - resid;	/* from offset to bytes */

	    /* With the address being on an ASIC boundary the count has
	    to be checked to determine if only a safe zone is needed. */

	    if( wrk_count < zonesize )		/* only a few left */
	    {
		resid = wrk_count;		/* all the bytes left */

		/* There has to be a safe zone allocated for this DAT entry.
		If there is not one available, terminate the loop. */

		if( (zone = dcmn_get_avail_zone( dws )) == 0 )
		{
		    break;			/* don't do any more */
		}

		/* If the data direction is a ASIC_TRANSMIT then the
		selected zone has to have the data pre-loaded into it. */

		if( dws->dir == ASIC_TRANSMIT )	/* data doesouta */
		{
		    bcopy( vk0_addr, ZONE_KADDR( dws, zone ), resid );/* fill */
		}

		/* Load the DAT with the information for this part of
		the I/O contained in a safe zone.  */

		d->dat_count = resid;			/* count in the zone */
		d->dat_vaddr = vk0_addr;		/* where in KVA space */
		d->dat_paddr = ZONE_PADDR( dws, zone );	/* where in phys */
		d->dat_flags |= (zone | DAT_VALID );	/* whats in it */
	    }
	    else	/* wrk_count >= zonesize */
	    {
		/* Check the working count to determine if there are 
		enough bytes to go the the next page boundary. */

		if( wrk_count < resid )
		{
		    /* Adjust the resid value to contain only the number
		    of bytes that will take the address and count to the
		    next ASIC boundary. */

		    resid = (wrk_count & ~(zonemask));	/* only to next bdry */
		}

		/* The current working count and address can be loaded
		into a DAT element and a normal DMA can be done.  The
		next pass of the loop will take care of the ASIC
		boundary if necessary. */

		d->dat_count = resid;		/* how many in the zone */
		d->dat_vaddr = vk0_addr;	/* where in KVA space */
		d->dat_paddr = phy_addr;	/* where in phys */
		d->dat_flags |= DAT_VALID;	/* whats in it */
	    }

	    /* Correct the working address and counts with the resid. */

	    wrk_vaddr = (void *)((vm_offset_t)wrk_vaddr + resid);/* new addr */
	    wrk_count -= resid;				/* down counter */

	    continue;				/* back to the top */
	}

	/* Having reached this point in the loop, the address is both
	page aligned and ASIC boundary aligned.  There also has to be
	a check for the ASIC boundaries.  If there are not enough
	bytes to go to the next PAGE boundary then the transfer can only
	go up to the next ASIC boundary, and the next iteration of the
	loop will take care of the zone alloc. */

	resid = DME_PAGE_SIZE;				/* assume a PAGE */

	/* With the address being on a PAGE and an ASIC boundary the
	count has to be checked to determine if only a safe zone is
	needed. */

	if( wrk_count < zonesize )		/* only a few left */
	{
	    resid = wrk_count;			/* all the bytes left */

	    /* There has to be a safe zone allocated for this DAT entry.
	    If there is not one available, terminate the loop. */

	    if( (zone = dcmn_get_avail_zone( dws )) == 0 )
	    {
		break;					/* don't do any more */
	    }

	    /* If the data direction is a ASIC_TRANSMIT then the
	    selected zone has to have the data pre-loaded into it. */

	    if( dws->dir == ASIC_TRANSMIT )	/* data doesouta */
	    {
		bcopy( vk0_addr, ZONE_KADDR( dws, zone ), resid );   /* fill */
	    }

	    /* Load the DAT with the information for this part of the
	    I/O contained in a safe zone.  */

	    d->dat_count = resid;		/* how many in the zone */
	    d->dat_vaddr = vk0_addr;		/* where in KVA space */
	    d->dat_paddr = ZONE_PADDR( dws, zone );	/* where in phys */
	    d->dat_flags |= (zone | DAT_VALID );	/* whats in it */
	}
	else		/* wrk_count >= zonesize */
	{

	    /* Check the working count to determine if there are 
	    enough bytes to go the the next PAGE boundary. */

	    if( wrk_count < resid )
	    {
		/* Adjust the resid value to contain only the number of
		bytes that will take the address and count to the next
		ASIC boundary. */

		resid = (wrk_count & ~(zonemask));	/* only to next bdry */
	    }

	    /* The current working count and address can be loaded
	    into a DAT element and a normal DMA can be done.  The
	    next pass of the loop will take care of the ASIC
	    boundary if necessary. */

	    d->dat_count = resid;		/* how many in the zone */
	    d->dat_vaddr = vk0_addr;		/* where in KVA space */
	    d->dat_paddr = phy_addr;			/* where in phys */
	    d->dat_flags |= DAT_VALID;			/* whats in it */
	}

	/* Correct the working address and counts with the resid. */

	wrk_vaddr = (void *)((vm_offset_t)wrk_vaddr + resid);	/* new addr */
	wrk_count -= resid;			/* down counter */
    }

    /* Now that the while loop has terminated, re-get the pointer to
    the *last* filled in DAT.  When the loop terminates, via count or
    exhaused DATs, the "working" dat_index will point to the next
    available DAT.  In the last filled DAT, the working count will
    determine weather to set the flags to be either finished or
    incomplete. */

    d = GET_DAT_PTR( dws, PREV_DAT( dws, dat_index ) ); /* last valid DAT */

    if( wrk_count == 0 )			/* normal termination ? */
    {
	d->dat_flags |= DAT_FINAL;		/* last one */
    }
    else					/* not normal */
    {
	d->dat_flags |= DAT_INCOMPLETE;	/* more to do */
    }

    /* Update the XFER_INFO ahead structure with the ending counts and
    addresses.  If necessary they will be used again the next time it is
    needed to setup/calc more DATs. */

    dws->ahead.count = wrk_count;		/* whats left to do */
    dws->ahead.vaddr = wrk_vaddr;		/* where it left off */
    dws->ahead.index = dat_index;		/* last valid DAT */

    /* Signal all went well in the calc/setup loop. */

    return( CAM_REQ_CMP );
}

/* ---------------------------------------------------------------------- */
/*
Routine Name: dcmn_dat_adj

Functional Description:
    This routine will make the necessary adjustment to a DAT to handle
    an incomplete transfer.  If the new address forces the data to be
    moved via one of the safe zones the previous DAT to the indexed
    one will also be used.

Formal Parameters:
    The DME working set for the I/O
    The index of the current DAT that needs to be adjusted.

Implicit Inputs:
    The DME Descriptor, direction, and DME working set safe zones.
    The current DAT still has the VALID flag set.

Implicit Outputs:
    The DAT and possibly the previous one in the list.

Return Value:
    CAM_REQ_CMP		: all went well
    CAM_REQ_CMP_ERR		: all did not go well

Side Effects:
    None

Additional Information:
    This routine works with the assumption that the physical and KVA
    address in the DAT are contigious within the adjustment area.
*/

U32
dcmn_dat_adj( dws, dat_index )
    DMA_WSET *dws;                      /* pointer to the I/O working set */
    U32 dat_index;			/* the current one */
{
    /* Local variables */

    register U32 resid;			/* holds the masking results */
    U32 zone;				/* the current working zone flag */
    register DAT_ELEM *d;		/* pointer to the working DAT */
    register U32 zonesize;		/* for holding the safe zone size */
    register U32 zonemask;		/* for holding the zone align mask */

    void *vk0_addr;			/* Virtual KVA seg addr for the DAT */

    /* Get the zone size and mask information from the DMA control structure
    for this bus. */

    zonesize = dws->dma_ctrl->dme_zone_size;	/* zone size for the bus */
    zonemask = zonesize - 1;			/* the mask for this size */

    /* Using the index get the pointer to the DAT. */

    d = GET_DAT_PTR( dws, dat_index );

    /* Check out the physical address in the DAT if it is not aligned
    on the ASIC octaword boundary then it has to be adjusted. */

    if( (resid = ((vm_offset_t)d->dat_paddr & zonemask)) == 0)	/* aligned */
    {
	return( CAM_REQ_CMP );				/* all done */
    }

    /* The value in resid must contain the number of bytes that are
    left to get to the next ASIC boundary.  The above if() produces the
    address offset, to get the byte count the offset is subtracted from
    the ZONE size. */

    resid = zonesize - resid;		/* from offset to bytes */

    /* Check to see if the DAT already pointed to a safe zone.  If it was
    then just update the counts and restore the zone address */

    if( (zone = GET_DAT_ZONE( d )) != 0 )
    {
	/* If the data direction is a ASIC_TRANSMIT then the selected
	zone has to have the data re-loaded into it. */

	if( dws->dir == ASIC_TRANSMIT )	/* data doesouta */
	{
	    bcopy( d->dat_vaddr, ZONE_KADDR( dws, zone ), resid );   /* fill */
	}
	d->dat_paddr = ZONE_PADDR( dws, zone );	/* where in phys */

	return( CAM_REQ_CMP );		/* all done */
    }

    /* At this point there has to be some DAT element adjusting to be
    done.  As the code in DME_RESUME()/DME_PAUSE() use the DAT element
    ring they will always leave the "previous" DAT element available
    for use.  This rest of this routine will make use of this
    knowledge.  The end of the data transfer has left the current data
    pointer on a non-ASIC boundary.  The current DAT element will be
    adjusted to point to the next ASIC boundary, and one of the safe
    zones will be allocated to take care of the data that is now
    "unaligned".  Note:  The allocation of one of the safe zones,
    *MUST* always succede, there is no easy way to "back off" the
    latest DAT calculations, that might have used up both zones.  This
    is the primary reason for Scatter/Gather lists being taken care of
    in the DME_PAUSE() code.  */

    /* Save the KVA address for the new safe zone DAT.  It will be loaded
    in for the buffer's address. */

    vk0_addr = d->dat_vaddr; 

    /* Update the current DAT with the corrected counts and addresses now
    that the beginning data is going into a safe zone. */

    d->dat_count -= resid;			/* not as many to xfer */
    d->dat_vaddr = (void *)((vm_offset_t)d->dat_vaddr + resid);	/* new start */
    d->dat_paddr = (void *)((vm_offset_t)d->dat_paddr + resid);	/* new phy */

    /* Now that the current DAT has been adjusted, work on the previous
    one that uses the zone.  The index is decremented and the flags
    field is set to DAT_ALLOCED for work. */

    dat_index = PREV_DAT( dws, dat_index );

    d = GET_DAT_PTR( dws, dat_index );

    d->dat_flags = DAT_ALLOCED;		/* clear out all others */

    /* Alloc an available safe zone, and fill it in if necessary */

    zone = dcmn_get_avail_zone( dws );		/* get a zone */

    /* If the data direction is a ASIC_TRANSMIT then the selected zone
    has to have the data pre-loaded into it. */

    if( dws->dir == ASIC_TRANSMIT )	/* data doesouta */
    {
	bcopy( vk0_addr, ZONE_KADDR( dws, zone ), resid );   /* fill */
    }

    /* Load the DAT with the information for this part of the I/O */

    d->dat_count = resid;		/* how many in the zone */
    d->dat_vaddr = vk0_addr;		/* where in KVA space */
    d->dat_paddr = ZONE_PADDR( dws, zone );	/* where in phys */
    d->dat_flags |= (zone | DAT_VALID );	/* whats in it */

    /* Set in the current XFER_INFO structure what the new DAT index is
    for the next I/O request via DME_RESUME(). */

    dws->current.index = dat_index;

    /* Return back to the caller, the adjustment is done. */

    return( CAM_REQ_CMP );
}

/* ---------------------------------------------------------------------- */
/*
Routine Name: dcmn_dat_concat

Functional Description:
    This routine will make the necessary adjustment to a series of DAT
    entries to "concat"inate entries for physically contigious PAGEs.  This
    will allow a single DAT entry to contain a physical address and count
    for multiple PAGEs.  There will be no re-ordering of the DAT entries
    that have been concat-ed.  The following entries will have their
    count value set to 0, and the DAT_CONCAT flag set.  The caller has
    to be prepared to deal with valid/zero count entries.

Formal Parameters:
    The DME working set for the I/O
    The number of valid DAT entries to concatinate.
    The byte limit for successive DAT concatinations.

Implicit Inputs:
    The "ring" of DAT entries pointed to by the working set.

Implicit Outputs:
    The "ring" of DAT entries pointed to by the working set.
    The current DAT still has the VALID flag set.

Return Value:
    CAM_REQ_CMP			: all went well
    CAM_REQ_CMP_ERR		: all did not go well

Side Effects:
    None

Additional Information:
    This routine works with the assumption that the physical 
    address in the DAT are contigious based on the PAGE sizes.
*/

U32
dcmn_dat_concat( dws, avail_dats, cat_limit )
    DMA_WSET *dws;                      /* pointer to the I/O working set */
    U32 avail_dats;			/* how many DATs to be concat-ed */
    U32 cat_limit;			/* limit of how many bytes to cat */
{
    U32 loop;				/* loop counter variable */
    U32 w_index;			/* the index to working DAT */
    register DAT_ELEM *wd;		/* pointer to the working DAT */
    register DAT_ELEM *cd;		/* pointer to the concat-ing DAT */

    /* Do a quick check to make sure that the number of available DATs is
    worth the effort. */

    if( avail_dats < 3 )		/* need 3 min to concat */
    {
	/* Return back to the caller there is nothing to do. */

	return( CAM_REQ_CMP );
    }

    /* Set the concat and working DAT pointers/index to the start of the
    available DAT elements in the ring.  In the for() loop the working
    values are pre-incremented to the second DAT for the first pass.  */

    w_index = dws->current.index;			/* DAT index */
    wd = cd = GET_DAT_PTR( dws, dws->current.index );	/* for ref fields */

    /* In the loop, the "next" DAT is compared to the current DAT.  The
    loop should only have to search through the all the entries except 
    the first one.  The loop counter, with the search limitation, is 
    the number of DATs minus 1. */

    loop = avail_dats - 1;		/* don't bother w/first one */

    for( ; loop != 0; loop-- )
    {
	/* After we get to the final entry, then check to see if we concat'ed
	   it.  If we did, then set the FINAL entry bit in the one we put the
	   real transfer into.  Then, it's time to quite looking. */

	if( wd->dat_flags & DAT_FINAL ){
	    if( wd->dat_flags & DAT_CONCAT ){
		cd->dat_flags |= DAT_FINAL;
	    }
	    break;
	}

	/* If we got to the end of an incomplete transfer, then just
	   get out and start what we have.  Note that we can't move
	   the INCOMPLETE flag (like we do the FINAL flag), since the
	   dat_calc code will simply start where it left off (in the
	   ring) when it continues with whats left of the transfer.  */

	if( wd->dat_flags & DAT_INCOMPLETE) {
	    break;
	}

	/* If its not the FINAL or the last part of an INCOMPLETE then,
	   take the working values and go to the next DAT entry. */

	w_index = NEXT_DAT( dws, w_index );	/* next index */
	wd = GET_DAT_PTR( dws, w_index );	/* next entry */

	/* Make sure that this working entry does contain valid data.  If
	a non-valid entry is found, return to the caller. */

	if( (wd->dat_flags & DAT_VALID) == NULL )
	{
	    return( CAM_REQ_CMP_ERR );		/* signal a problem */
	}

	/* Compare the *physical* address in the working DAT entry against
	the concat address plus the concat count. */

	if( wd->dat_paddr == 
	    (void *)((vm_offset_t)(cd->dat_paddr) + cd->dat_count) )
	{
	    /* These two DAT entries describe two physically contigious
	    regions.  The count value in the working entry can be added
	    to the concat-ing one and the working entry can be
	    "skipped".  The concat byte limit has to be checked to make
	    sure that adding these two together does not exceed the
	    callers limit. */

	    if( (cd->dat_count + wd->dat_count) <= cat_limit )
	    {
		/* It is ok for the concat-ing entry to contain both of the
		regions byte count. */

		cd->dat_count += wd->dat_count;	/* add the # of bytes */
		wd->dat_count = 0;		/* clear out the working one */
		wd->dat_flags |= DAT_CONCAT;	/* signal concatination */

		continue;			/* go to the next iteration */
	    }
	}

	/* The previous checks for contigious and byte limit have
	failed, at this point in the loop you can start over at the
	working entry.  The concat pointer is now moved to the working
	entry and the loop iterates again. */

	cd = wd;			/* move concat to the working DAT */
    }

    /* Return back to the caller, the concatination has finished. */

    return( CAM_REQ_CMP );
}

/* ---------------------------------------------------------------------- */
/*
Routine Name: dcmn_get_avail_zone

Functional Description:
    This routine will return the first availble zone, via the flag
    setting, that is contained in the working set.  The zone is then
    marked as unavailable to be used in the DME working set.

Formal Parameters:
    A DME working set containing the safe zone.

Implicit Inputs:
    None

Implicit Outputs:
    None

Return Value:
    A flag value for ZONE A or ZONE B or None zone.

Side Effects:

Additional Information:
    This code has the specific knowledge of what/where the zones are.
    To competely hide the zones from the "common" code this routine
    will have to change.
*/

U32
dcmn_get_avail_zone( dws )
    DMA_WSET *dws;
{
    /* If there are no zones available then return 0.  Otherwise return
    A or B. */

    if( dws->dme_flags & (DAT_ZONEA | DAT_ZONEB) == 0 )
    {
	return( 0 );			/* no zones available */
    }

    if( dws->dme_flags & DAT_ZONEA != 0 )
    {
	dws->dme_flags &= ~(DAT_ZONEA);	/* clear A flag, it is now used */
	return( DAT_ZONEA );		/* return which one to use */
    }
    
    dws->dme_flags &= ~(DAT_ZONEB);	/* clear B flag, it is now used */
    return( DAT_ZONEB );		/* return which one to use */
}

/* ---------------------------------------------------------------------- */
/*
Routine Name: dcmn_flush_safe_zone

Functional Description:
    This routine will move the users data from the selected safe zone
    into the buffer.  The zone is then marked as available to be reused
    in the DME working set.

Formal Parameters:
    A DME working set containing the safe zone.
    A DAT pointer that has the address and zone flag.
    A count of valid bytes to copy.

Implicit Inputs:
    None

Implicit Outputs:
    The users data buffer.

Return Value:
    CAM_REQ_CMP		All went well

Side Effects:
    The data is move into the users buffer.  JAG - Question on what to do
    about the systems data cache.

Additional Information:
    It is expected that this routine will ONLY be called to move data
    FROM the safe zone INTO the users buffer.
*/

U32
dcmn_flush_safe_zone( dws, d, xfer_cnt )
    DMA_WSET *dws;
    DAT_ELEM *d;
    U32 xfer_cnt;
{
    U32 zone;				/* which zone is used */
    
    /* Setup required local variables. */

    zone = GET_DAT_ZONE( d );

    /* Prior to moving the zone data, via a bcopy(), the systems data
    cache has to be prepared.  */

    DO_CACHE_AFTER_DMA( ZONE_KADDR( dws, zone ), d->dat_count );

    /* When this routine is called you know that the data has to be
    moved from the safe zone to the users buffer.  The KVA address for
    the buffer is contained in the DAT and the kernel address for the
    selected zone is in the DME working set.  Using the address
    information and the count bcopy the data. */

    bcopy( ZONE_KADDR( dws, zone ), d->dat_vaddr, xfer_cnt );

    /* Return the "zone" to the available state in the DME working set by
    setting the zone flags. */

    FREE_DAT_ZONE( dws, zone );		/* allow the zone to be used */
    dws->dme_flags |= zone;		/* allow the zone to be used */

    return( CAM_REQ_CMP );		/* all done */
}

/* ---------------------------------------------------------------------- */
/*
Routine Name: dcmn_rtn_resources

Functional Description:
    This routine will return DME working sets and DATs to the pools.
    It will check to see if there are DATs that need to be returned.

Formal Parameters:
    A DME working set containing DATs

Implicit Inputs:
    The DME control struct

Implicit Outputs:
    The DME working set and DAT pools

Return Value:
    CAM_REQ_CMP		All went well

Side Effects:
    The pools are updated.

Additional Information:
    This routine MUST be called with the control structure locked.
*/

U32
dcmn_rtn_resources( dws )
    DMA_WSET *dws;
{
    DMA_CTRL *dctrl;

    /* Return the allocated DATs back to the groups, for the next
    setup call.  They are nolonger needed. */

    if( dws->dat_grp != (DAT_ATTCH *)NULL )	/* Are there DATs ? */
    {
	dcmn_dat_free( dws );			/* put them back */
    }

    /* Return the allocated DMA_WSET back on the free side. */

    dctrl = GET_DMA_CTRL( dws );	/* get the control struct */
    dws->flink = dctrl->flink;	/* attach to old front */
    if( dws->flink != NULL )	/* incase the FREE side is empty */
    {
	(dws->flink)->blink = dws;	/* attach the old front back ptr */
    }
    dctrl->flink = dws;		/* bump up the free side */
    dctrl->nfree++;			/* incr # of free */
    dctrl->nbusy--;			/* decr # of busy */

    return( CAM_REQ_CMP );
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* This code will have to change for the ULTRIX/BSD to ULTRIX/OSF port !! */
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/*
Routine Name: dcmn_avtophy

Functional Description:
    This routine will return a physical address from the passed virtual
    address and data buffer bp struct.

Formal Parameters:
    The address to get the physical address for
    The I/O DME working structure

Implicit Inputs:
    All kinds of stuff WRT system VM
    The I/O struct buf pointer bp, for accessing the proc tables for user
    addresses.

Implicit Outputs:
    None

Return Value:
    A physical address

Side Effects:
    None - that I will admit to.

Additional Information:
    This routine will be one of the Ultrix/BSD -> Ultrix/OSF port problems.
    This routine is *pure* magic.  
*/

void *
dcmn_avtophy( dws, addr )
    DMA_WSET *dws;
    register char *addr;
{
    struct buf *bp;			/* Pointer to the users buf sturct */
    void *p;				/* the holder of the phys addr */

    /* Do a quick check to see if the address is a Kernel Virtual, KVA or
    User, KUSEG.  If the address is a kernel there is no need to 
    go through the pmap_extract call. There is also the chance that the bp
    is NULL anyway. */

    if( !CAM_IS_KUSEG( addr ) )
    {
        /* This is a kernel address, let svatophys() deal with the segments. */
	svatophys( addr, &p );
    }
    else
    {
	/* Working with a user address and bp convert the address to a
	physical one. */

	bp = (struct buf *)dws->dme_bp;		/* get the bp */
	if( bp == (struct buf *)NULL )
	{
	    /* JAG - not sure what else to do ! */
	    panic("dcmn_avtophy: DME scsi NULL value for bp.");
	}

	/* Get the physical address for the virtual address, using this magical
	call to pmax_extract(). */

	p = (void *)pmap_extract( bp->b_proc->task->map->vm_pmap, addr );
    }

    return( p );				/* return the physical addr */
}

/* ---------------------------------------------------------------------- */
/*
Routine Name: dcmn_alloc_ctrl
 
Functional Description:
    This routine will dynamicly allocate the necessary memory for the
    DME control structure.

Formal Parameters:
    None

Implicit Inputs:
    None

Implicit Outputs:
    None

Return Value:
    A DMA_CTRL address	: the memory was allocated
    NULL			: no memory was allocated

Side Effects:
    None

Additional Information:
    This routine will be one of the Ultrix/BSD -> Ultrix/OSF port problems.
*/

/* ARGSUSED */
DMA_CTRL *
dcmn_alloc_ctrl()
{
    DMA_CTRL *c;

    c = (DMA_CTRL *)sc_alloc( sizeof( DMA_CTRL ) );

    return( c );
}

/* ---------------------------------------------------------------------- */
/*
Routine Name: dcmn_free_ctrl

Functional Description:
    This routine will free previously allocate memory from a nolonger used
    DME control structure.
Formal Parameters:
    A DMA_CTRL address from a previous allocation.	

Implicit Inputs:
    None

Implicit Outputs:
    None

Return Value:
    None

Side Effects:
    None

Additional Information:
    This routine will be one of the Ultrix/BSD -> Ultrix/OSF port problems.
*/

/* ARGSUSED */
void
dcmn_free_ctrl( c )
    DMA_CTRL *c;
{
    sc_free( c, sizeof( DMA_CTRL ) );
}

/* ---------------------------------------------------------------------- */
/*
Routine Name: dcmn_alloc_wsets

Functional Description:
    This routine will dynamicly allocate the necessary memory for a
    bunch of DME Working Sets.  A system page(s) of uncached memory
    will be allocated and then be carved up into DME working sets.  It
    is critical that the initial system allocation is uncached.  The
    working set will have the "safe zones" that the ASIC will DMA into
    and out of "appended" to them.  The safe zones are used for parts
    of the users buffer that is not aligned on an ASIC boundary.

Formal Parameters:
    The control structure that will hold the working sets.

Implicit Inputs:
    The initial number of working sets and the size of a system "page".
    The number of working sets is stored in the cam_data.c file.  JAG TODO

Implicit Outputs:
    None

Return Value:
    The number of DME working sets that were successfully allocated.

Side Effects:
    There may be problems with attempting to free up any excess
    DMA_WSETs.  The allocated page is carved up into many DMA_WSETs and
    the beginning is lost.  For now as part of the allocation the first
    DMA_WSET that is carved up from the page will have an extra bit set
    in it's flag field.  This bit may be used to at least signal an
    address that can be freed.

Additional Information:
    A working set is build up from the initial definition of the
    structure in the dme_common.h file.  The additional safe zones are
    "attached" to the end of the working set.  A "pad" area is added
    immediatly following the address pointers to have the safe zones
    properly aligned for the DMA hardware.  The zone size information is
    contained in the control structure.

    To make matters even more confusing, another pad area has to
    be calculated for the end of the working set.  This code is "hand
    calculating" the size of a data structure and creating an "array" of
    them.  There could be a problem with "pointer alignment" for the
    following working set.  Depending on the zone size and the value of
    sizeof( void * ) in this system, it is possible for the next
    working set to *not* begin on a valid pointer boundary.

    NOTE: There is the assumption that the zone values are powers of 2.

    This routine will be one of the Risc/OSF <-> Alpha/OSF port problems.
*/

/* ARGSUSED */
U32
dcmn_alloc_wsets( dctrl )
    DMA_CTRL *dctrl;
{
    DMA_WSET *dws;			/* working pointer for working sets */
    char *base;				/* base pointer for the PAGE */
    DMA_WSET t_qhead;			/* for holding the linked list */
    U32 total_wsets;			/* total working sets alloc-ed here */
    U32 wset_zpad;			/* the size of the zone pad area */
    U32 wset_size;			/* the accumulated size of a wset */
    U32 wset_ppad;			/* the size of the pointer pad area */
    U32 i;				/* loop counter */

    int s;				/* for locking on the control struct */

    /* Alloc a system PAGE and attach it to the base pointer */

    base = sc_alloc( DME_PAGE_SIZE );

    /* Make sure that the memory was able to be allocated. */

    if( base == (char *)NULL )
    {
	return( 0 );			/* no working sets allocated */
    }

    /* Using the defined size of the working set structure, and the zone
    size/alignment requirements calc the size needed for the pad and then
    the overall size of a working set. */

    if(( wset_zpad = (sizeof(DMA_WSET) % dctrl->dme_zone_size)) != 0 )
    {
	/* If there is a residual, then that amount has to be subtracted
	from the zone size to get the number of bytes needed to get to the
	next zone boundary. */

	wset_zpad = dctrl->dme_zone_size - wset_zpad;	/* for the boundary */
    }

    wset_size = sizeof(DMA_WSET);		/* initialy whats defined */
    wset_size += wset_zpad;			/* add the zone bndry pad */
    wset_size += dctrl->dme_zone_size;		/* add zone A size */
    wset_size += dctrl->dme_zone_size;		/* add zone B size */

    /* Now that the overall size of the working set is known, determine
    what is needed for the ending pad for the pointer alignment.  See the
    "Additional Information" section in this routine header. */

    if(( wset_ppad = (wset_size % sizeof(void *))) != 0 )
    {
	/* If there is a residual, then that amount has to be
	subtracted from the pointer size to get the number of bytes
	needed to get to the next pointer boundary. */

	wset_ppad = sizeof(void *) - wset_ppad;	/* for the boundary */

	/* Add the pointer pad to the overall working set size. */

	wset_size += wset_ppad;			/* add the ptr bndry pad */
    }

    /* Figure out how many working sets will fit into the PAGE. */

    total_wsets = DME_PAGE_SIZE / wset_size;

    t_qhead.flink = (DMA_WSET *)NULL;

    /* The following loop will increment through the PAGE carving out
    DME working sets and setting them up. */

    dws = (DMA_WSET *)base;		/* start at the PAGE base */
    for( i = 0; i < total_wsets; i++ )
    {
	/* Initialize the working set, and place it on the temp Q. */

	dws->dme_flags = 0;			/* clear out the flags */
	dws->flink = (DMA_WSET *)NULL;		/* clear the pointer */
	dws->blink = (DMA_WSET *)NULL;		/* clear the pointer */
	dws->wset_state = DME_STATE_UNDEFINED;	/* no set yet */

	dws->dme_desc = (DME_DESCRIPTOR *)NULL;	/* clear the pointer */
	dws->dme_ptr0 = (void *)NULL;		/* clear the pointer */
	dws->dme_ptr1 = (void *)NULL;		/* clear the pointer */
	dws->dma_ctrl = (DMA_CTRL *)NULL;	/* clear the pointer */
	dws->dat_grp = (DAT_ATTCH *)NULL;	/* clear the pointer */
	dws->de_count = 0;			/* clear the count */
	dws->de_base = (DAT_ELEM *)NULL;	/* clear the pointer */
	dws->dir = 0;				/* clear the direction */

	/* The next game has to do with the safe zones and forcing
	their alignment onto a zone boundary.  As part of the
	allocation there is a pad area in "front", at a lower address,
	of the zones.  This pad area is used to make sure that the
	following safe zones are starting on a zone boundary.  Once the
	zones are "attached" to the end of the working set inorder to
	access the zones only the address pointers in the working set
	can be used NOT the offset address based on the structure
	declaration. */

	/* NOTE: The "B" zone addresses are simply the "A" addresses
	plus the zone size.  The virtual address is calc-ed by adding
	the working set size and pad size to the working loop pointer.
	Next the physical address is calc-ed from the virtual address.
	And finally a KVA seg address is calc-ed from the physical
	address.  This way all "virtual" address used in memory
	accesses are either physical or KVA seg. */

	/* Add the working set size and the pad to get to A. */
	dws->asz_vaddr =
	    (caddr_t)((vm_offset_t)dws + sizeof( DMA_WSET ) + wset_zpad);
	dws->asz_paddr = (caddr_t)dcmn_avtophy( dws, dws->asz_vaddr );
	dws->asz_vaddr = (caddr_t)CAM_PHYS_TO_KVA(dws->asz_paddr);

	dws->bsz_paddr =
	    (caddr_t)((vm_offset_t)dws->asz_paddr + dctrl->dme_zone_size);
	dws->bsz_vaddr = (caddr_t)CAM_PHYS_TO_KVA(dws->bsz_paddr);

	AVAILABLE_DAT_ZONES( dws );

	/* Put the setup working set on the local list. */

        if( t_qhead.flink != ((DMA_WSET *)NULL ) )/* check for not empty */
        {
            (t_qhead.flink)->blink = dws;	/* back point to new front */
        }
        else					/* empty Q special case */
        {
            t_qhead.blink = dws;		/* keep a pointer to the end */
        }
        dws->flink = t_qhead.flink;		/* bump up the free side */
        t_qhead.flink = dws;			/* point to the new front */

	/* Now add the total size for the working set to the pointer to
	get to the start of the next working set. */

	dws = (DMA_WSET *)((vm_offset_t)dws + wset_size); /* next DMA_WSET */
    }

    /* The special flag, DWS_ALLOC_BASE, needs to be added to the first
    working set on the page.  This hint can be used when the list of
    working sets needs to be returned to the system as PAGEs. */

    dws = (DMA_WSET *)base;
    dws->dme_flags |= DWS_ALLOC_BASE;		/* signal the special one */

    /* Add the new working sets from the temporary control structure to
    the passed control structure, and update the number of free ones. */

    D3CTRL_IPLSMP_LOCK( s, dctrl );		/* lock on control */

    if( dctrl->flink != (DMA_WSET *)NULL)	/* in case empty free list */
    {
	(dctrl->flink)->blink = t_qhead.blink;	/* set to temp back */
    }
    (t_qhead.blink)->flink = dctrl->flink;	/* set to pool front */
    dctrl->flink = t_qhead.flink;		/* set to temp front */

    dctrl->nfree += total_wsets;		/* incr # of free */

    D3CTRL_IPLSMP_UNLOCK( s, dctrl );		/* unlock control */

    /* Return to the caller the number of working sets that were added to
    the control structure. */

    return( total_wsets );
}

/* ---------------------------------------------------------------------- */
/*
Routine Name: dcmn_free_wsets

Functional Description:
    This routine will free all the previously allocated DMA_WSETs contained
    in the DMA_CTRL structure.  The free list will be scanned for any
    DMA_WSETs that have the DWS_ALLOC_BASE bit set in the flags field.
    All the DMA_WSETs with this bit set will be set aside and freed up
    once the free list has been completely scanned.

    It is assumed that the caller wants *ALL* of the DMA_WSETs released.

Formal Parameters:
    The DME control structure that contain all the working sets.

Implicit Inputs:
    None

Implicit Outputs:
    All the memory is returned to the system.

Return Value:
    None

Side Effects:
    None

Additional Information:
    There is NO checking to see that all the working sets allocated on the
    page from the initial allocation are there.

    This routine will be one of the Ultrix/BSD -> Ultrix/OSF port problems.
*/

/* ARGSUSED */
void
dcmn_free_wsets( dctrl )
    DMA_CTRL *dctrl;			/* the DME control struct */
{
    DMA_WSET tmp_d;			/* for holding the DME working sets */
    DMA_WSET *d;

    int s;				/* for locking on the control struct */

    tmp_d.flink = tmp_d.blink = (DMA_WSET *)NULL;

    /* Lock on the DME control struct and take off all the free ones.  Set
    the count to 0 and release the lock. */

    D3CTRL_IPLSMP_LOCK( s, dctrl );		/* lock on control */

    tmp_d.flink = dctrl->flink;		/* save the front */

    dctrl->flink = (DMA_WSET *)NULL;	/* no more free ones */
    dctrl->nfree = 0;				/* all gone */

    D3CTRL_IPLSMP_UNLOCK( s, dctrl );		/* unlock control */

    /* Scan the saved list and find all of the working sets that have
    the DWS_ALLOC_BASE bit set.  These are saved on the blink side.
    The special working sets have to be saved, and freed later, the
    lists may have become scattered and you really should not scan
    along a list that may suddenly have holes. */

    d = tmp_d.flink;
    while( d != (DMA_WSET *)NULL )
    {
	/* Found one, use the blink pointer to track it. */

	if( (d->dme_flags & DWS_ALLOC_BASE) != 0 )
	{
	    d->blink = tmp_d.blink;		/* rip up the back */
	    tmp_d.blink = d;			/* put in the front */
	}
	d = d->flink;				/* next one */
    }

    /* Now that all the "base" ones are found, free them up.  They are
    on the blink side of the temp working set. */

    d = tmp_d.blink;
    while( d != (DMA_WSET *)NULL )
    {
	tmp_d.flink = d->blink;			/* tmp storage */
	sc_free( d, DME_PAGE_SIZE );		/* return it */
	d = tmp_d.flink;			/* next one */
    }
}

/* ---------------------------------------------------------------------- */
/*
Routine Name: dcmn_alloc_dats

Functional Description:
    This routine will dynamicly allocate the necessary memory for the
    DME DAT groups.  A DAT group is contained within a system page.
    A DAT group will consist of a DAT_ATTCH structure and a "array"
    DAT_ELEMs.  The DAT_ATTCH structure is the first one in the page
    and the rest of the page is DAT_ELEMs.  The array of DAT_ELEMs
    will be treated as a ring.

Formal Parameters:
    A DME control struct.  The DAT group will be placed on the front
    of the linked list on the control struct.

Implicit Inputs:
    A system page is the minimum size that will be alloc-ed and
    transformed into a DAT group

Implicit Outputs:
    None

Return Value:
    The number of DAT_ELEMs that were added to the control structure.

Side Effects:
    None

Additional Information:
    This routine will be one of the Ultrix/BSD -> Ultrix/OSF port problems.
*/

/* ARGSUSED */
U32
dcmn_alloc_dats( dctrl )
    DMA_CTRL *dctrl;
{
    DAT_ELEM *de;			/* working pointer for DAT elements */
    char *base;				/* base pointer for the PAGE */
    DAT_ATTCH *d_attch;			/* for the attach structure */
    U32 total_dats;			/* total DATs in the ring alloc-ed */
    U32 i;				/* loop counter */

    int s;				/* for locking on the control struct */

    /* Alloc a system PAGE and attach it to the base pointer */

    base = sc_alloc( DME_PAGE_SIZE );

    /* Make sure that the memory was able to be allocated. */

    if( base == (char *)NULL )
    {
	return( 0 );			/* no working sets allocated */
    }

    /* Figure out how many DATs will fit into the PAGE after the 
    DAT attach structure is taken out. */

    total_dats = ((DME_PAGE_SIZE - sizeof( DAT_ATTCH))
	/ sizeof( DAT_ELEM ));

    /* Put the attach structure at the base of the alloc-ed PAGE.  Setup
    up the attach values. */

    d_attch = (DAT_ATTCH *)base;	/* set the attach struct */

    d_attch->dat_attch_flags = DWS_ALLOC_BASE;		/* just for hints */
    d_attch->next = (DAT_ATTCH *)NULL;			/* None yet */
    d_attch->dat_elem_free = total_dats;		/* all are free */

/* JAG - Alignment concerns ? */
    d_attch->dat_ring = (DAT_ELEM *)((vm_offset_t)base + sizeof( DAT_ATTCH ));
							/* the ring follows */

    /* Now loop through the DAT ring setting the DAT flags to DAT_FREE. */

    de = d_attch->dat_ring;
    for( i = 0; i < total_dats; i++ )
    {
	de->dat_flags = DAT_FREE;		/* make it so */
	de++;					/* next DAT element */
    }

    /* Add the new DAT ring to the control structure. */

    D3CTRL_IPLSMP_LOCK( s, dctrl );		/* lock on control */

    d_attch->next = dctrl->dat_group;		/* point to current front */
    dctrl->dat_group = d_attch;			/* put it on the front */

    dctrl->dat_per_ring = total_dats;		/* should ALWAYS be the same */

    D3CTRL_IPLSMP_UNLOCK( s, dctrl );		/* unlock control */

    return( total_dats );
}

/* ---------------------------------------------------------------------- */
/*
Routine Name: dcmn_free_dat

Functional Description:
    This routine will free previously allocate memory from a nolonger used
    DME DAT group.

Formal Parameters:
    A DMA_CTRL address from a previous allocation.	

Implicit Inputs:
    None

Implicit Outputs:
    None

Return Value:
    None

Side Effects:
    None

Additional Information:
    There is NO checking to see if there are any DAT's still in use.
    This routine will be one of the Ultrix/BSD -> Ultrix/OSF port problems.
*/

/* ARGSUSED */
void
dcmn_free_dat( da )
    DAT_ATTCH *da;
{
    sc_free( da, DME_PAGE_SIZE );		/* return it */
}

/* ---------------------------------------------------------------------- */
/*
Routine Name: dcmn_alloc_dme

Functional Description:
    This routine will dynamicly allocate the necessary memory for the
    DME attachment structure as part of the DME_ATTACH() call.

Formal Parameters:
    None

Implicit Inputs:
    None

Implicit Outputs:
    None

Return Value:
    A DME_STRUCT address	: the memory was allocated
    NULL			: no memory was allocated

Side Effects:
    None

Additional Information:
    This routine will be one of the Ultrix/BSD -> Ultrix/OSF port problems.
*/

/* ARGSUSED */
DME_STRUCT *
dcmn_alloc_dme()
{
    DME_STRUCT *dme;

    dme = (DME_STRUCT *)sc_alloc( sizeof( DME_STRUCT ) );

    return( dme );
}

/* ---------------------------------------------------------------------- */


