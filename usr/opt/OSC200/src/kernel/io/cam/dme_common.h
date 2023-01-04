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
 * @(#)$RCSfile: dme_common.h,v $ $Revision: 1.1.12.2 $ (DEC) $Date: 1993/09/07 20:38:33 $
 */

#ifndef __DME_COMMON_H__
#define __DME_COMMON_H__ 1

/* ---------------------------------------------------------------------- */

/* dme_common.h		Version 1.01			Nov. 05, 1992  */

/*  This file contains the definitions and data structures needed by
    the DME related files.

Modification History

	Version	  Date		Who	Reason

	1.00	10/14/92	jag	Created this file from the 3min include
					file, dme_3min_94_dma.h.
	1.01	11/05/92	jag	Changed the def of the working set.
	1.02	03/23/93	jag	Added the "concat" flag for signaling
					physically contig pages/DATs.  Added
					the dme_bp element to the working set.
*/

/* ---------------------------------------------------------------------- */
/* These two system page defines are here to help the Alpha-RISC/OSF port. */

#define DME_PAGE_SIZE	NBPG		/* from machparam.h */
#define DME_PAGE_MASK	PGOFSET		/* from machparam.h */

/* ---------------------------------------------------------------------- */
/* Definition of a DAT entry that all the I/Os are built out of.  Each 
DAT entry will be used to describe the information that is needed for all
of the individual DMA, IOASIC, TZA, TCDS, and 94, requests. */

typedef struct dat_elem
{
    U32 dat_flags;		/* flags for this element */
    U32 dat_count;		/* the number of bytes desc by this element */
    void *dat_vaddr;		/* kernel/user virtual address for the data */
    void *dat_paddr;		/* physical address for the data */
} DAT_ELEM;
#define DME_DAT_VERS  1       /* please incr if DAT_ELEM changes */

/* Defines for the flags field in the DAT_ELEM structure. */

#define DAT_FREE	0x00000000	/* This DAT is free for I/O allocs */

/* These zone flags are also used for setting the DME_WSET flags. */
#define DAT_ZONEA	0x00000001	/* contains safe zone A data info */
#define DAT_ZONEB	0x00000002	/* contains safe zone B data info */

#define DAT_ALLOCED	0x00000010	/* available for DAT calc use */
#define DAT_USED	0x00000020	/* used and abused :-) */
#define DAT_VALID	0x00000040	/* contains valid DAT I/O data */
#define DAT_FINAL	0x00000080	/* the final DAT entry for the I/O */
#define DAT_INPROGRESS	0x00000100	/* the ASIC is working on this one */
#define DAT_CONCAT	0x00000200	/* this entry has been concat-ed */
#define DAT_INCOMPLETE	0x80000000	/* more I/O calcs need to be done */

/* The following Bits in the DAT flags field, 23:16 are available for all
the various DMEs to make use of.  It is expected that there will be 
"overlays" with in this sub-field in the individual DME include files. */

#define DAT_DME_MASK	0x00FF0000	/* for masking in/out the DME field */

/* ---------------------------------------------------------------------- */

/* The DAT attachment structure is used to "attach" a DAT group to the 
control structure in a single linked list.  The DAT attachment structure
actually resides on the same "page allocation" as the DAT group and is the
first strcture on the allocation.  This will allow DAT groups to be "freed"
as demand drops.

NOTE:  It is a VERY IMPORTANT GOAL that this structure be the SAME SIZE and
a DAT_ELEM.  This will allow for minimal storage loss and misalignment. */

typedef struct dat_attch
{
    U32 dat_attch_flags;	/* misc flags for this DAT group */
    U32 dat_elem_free;		/* the number of DAT elements available */
    struct dat_attch *next;	/* pointer to the next DAT group */
    DAT_ELEM *dat_ring;		/* pointer to the actual DAT group */
} DAT_ATTCH;
#define DME_ATTCH_VERS  1       /* please incr if DAT_ATTCH changes */

/* ---------------------------------------------------------------------- */

/* This structure defines the local storage for the transfer information. */

typedef struct xfer_info
{
    U32 count;			/* the number of bytes remaining */
    void *vaddr;		/* kernel/user virtual address */
    U32 index;			/* index for the valid DAT element */
    U32 bxfer;			/* running count of transfered bytes */
} XFER_INFO;

/* ---------------------------------------------------------------------- */

/* This is the DME working set for the I/O.  It, along with the SIM_WS
and DME_DESCRIPTOR, contian all there is to know about the data
transfer for the I/O.  The attachment to the DAT elements and the
pointers to the DMA alignment safe zones are also contained in here. */

typedef struct dma_wset
{
    struct dma_wset *flink;		/* forward pointer */
    struct dma_wset *blink;		/* backward pointer */
    U32 dme_flags;			/* flags for this DME I/O */
    U32 wset_state;			/* DME data state for the I/O */
    U32 dir;				/* DME data xfer direction */
    U32 hba_cmd;                        /* HBA specific instr to start DMA */
    DME_DESCRIPTOR *dme_desc;		/* Pointer for the DME descriptor */
    void *dme_ptr0;			/* pointer for the DME use */
    void *dme_ptr1;			/* pointer for the DME use */
    void *dme_bp;			/* for the I/O mapping info */
    struct dma_ctrl *dma_ctrl;		/* back pointer to the ctrl struct */
    DAT_ATTCH *dat_grp;			/* pointer to the DAT group */
    DAT_ELEM  *de_base;			/* Base addr of the DAT group */
    U32        de_count;		/* number of DAT ELEM in group */

    XFER_INFO current;			/* current pointers and cnts */
    XFER_INFO ahead;			/* ahead pointers and cnts */
    XFER_INFO saved;			/* saved pointers and cnts */

    SCATTER_ELEMENT sg_current;		/* for scatter/gather tracking */
    SCATTER_ELEMENT sg_saved;		/* for scatter/gather tracking */

    caddr_t asz_paddr;			/* physical addr of A safe zone */
    caddr_t asz_vaddr;			/* virtual addr of A safe zone */
    caddr_t bsz_paddr;			/* physical addr of B safe zone */
    caddr_t bsz_vaddr;			/* virtual addr of B safe zone */
} DMA_WSET;
#define DMA_WSET_VERS  3       /* please incr if DMA_WSET changes */

/* The following defines are for the flags field. */

#define DWS_ALLOC_BASE	0x80000000	/* hint for the alloc/free code */

/* The following MACROs are used to access/change the DAT index values in
the working set.  The DATs are treated as a ring, and as the index is 
incremented or decremented the end conditions of the ring have to be
taken into account. */

/* Return the increment of the passed index for the "next" DAT. */
#define NEXT_DAT(d, i)	((((i) + 1) == (d)->de_count) ? (0) : ((i) + 1))

/* Return the decrement of the passed index for the "previous" DAT. */
#define PREV_DAT(d, i)	(((i) == 0) ? ((d)->de_count - 1) : ((i) - 1))

/* Return the address of the indexed DAT array. */
#define GET_DAT_PTR(d, i)	((DAT_ELEM *)&(d)->de_base[(i)])

/* This MACRO will return the DME control pointer from the working set. */
#define	GET_DMA_CTRL( d )	((DMA_CTRL *)(d)->dma_ctrl)

/* The following MACROs deal with the safe zones in the working sets. */

/* Get the zone "id" from the DAT element. */
#define GET_DAT_ZONE( d )	((d)->dat_flags & (DAT_ZONEA | DAT_ZONEB))

/* Make all the zones available in the working set. */
#define AVAILABLE_DAT_ZONES( d )	\
	((d)->dme_flags |= ( DAT_ZONEA | DAT_ZONEB ));	/* A & B are availble */

/* Free up the zone in the working set, by setting the zone flag. */
#define FREE_DAT_ZONE( d, z )	 ((d)->dme_flags |= (z));	

/* Return the kernel address of the request zone. */
#define ZONE_KADDR( d, z )	\
    (((z) == DAT_ZONEA) ? ((d)->asz_vaddr) : ((d)->bsz_vaddr))

/* Return the physical address of the request zone. */
#define ZONE_PADDR( d, z )	\
    (((z) == DAT_ZONEA) ? ((d)->asz_paddr) : ((d)->bsz_paddr))

/* ---------------------------------------------------------------------- */

/* The DME DMA control structure is used for attachment of the working 
sets and the DAT groups.  It is the locking point for alloc/free of all
the DME DMA resources. */

typedef struct dma_ctrl
{
    DMA_WSET *flink;		/* forward pointer for "free" working sets */
    DMA_WSET *blink;		/* backward pointerfor "busy" working sets */
    U32 dma_ctrl_flags;		/* flags for the control struct */
    DAT_ATTCH *dat_group;	/* where the DAT rings are stored */
    U32 nfree;			/* number of DMA_WSETs on the free side */
    U32 nbusy;			/* number of DMA_WSETs on the busy side */
    U32 dat_per_ring;		/* for the ring searching */
    U32 dme_zone_size;		/* for the safe zone size used in DAT calcs */
    lock_data_t d3_lk_ctrl;	/* for locking on the control struct */
} DMA_CTRL;
#define DMA_CTRL_VERS  2       /* please incr if DMA_CTRL changes */

/* This define is used to signal any DME working set scanners that this
DME working set is actually the control struct. */

#define DME_DMA_CTRL	0x0C000000	/* C for control */

/* The IPL/SMP locking Macros for the control structure. */

#define D3CTRL_INIT_LOCK( d3p )                          \
{                                                        \
    lock_init( &((d3p)->d3_lk_ctrl), TRUE );             \
}

#define D3CTRL_IPLSMP_LOCK( saveipl, d3p )               \
{                                                        \
    (saveipl) = splbio();                                \
    CAM_LOCK_IT( &((d3p)->d3_lk_ctrl), LK_RETRY );       \
}

#define D3CTRL_IPLSMP_UNLOCK( saveipl, d3p )             \
{                                                        \
    CAM_UNLOCK_IT( &((d3p)->d3_lk_ctrl) );               \
    (void)splx((saveipl));                               \
}

#define D3CTRL_SMP_SLEEPUNLOCK( chan, pri, d3p )           \
{                                                          \
    CAM_SLEEP_UNLOCK_IT( chan, pri, &((d3p)->d3_lk_ctrl) );\
}

#define D3CTRL_SMP_LOCK( d3p )                           \
{                                                        \
    CAM_LOCK_IT( &((d3p)->d3_lk_ctrl), LK_RETRY );       \
}

/* ---------------------------------------------------------------------- */

/* These transmit/receive defines are used to match the ASIC manual.  They
are use to remove any confusion with memory read/write vs I/O read/write. */

#define ASIC_TRANSMIT			0	/* For SCSI DATA OUT phase */
#define ASIC_RECEIVE			1	/* For SCSI DATA IN  phase */
#define ASIC_UNDEF			2	/* For CAM NO DATA DIR */

/* ---------------------------------------------------------------------- */
/* This code will have to change for the ULTRIX/BSD to ULTRIX/OSF port !! */
/* ---------------------------------------------------------------------- */

/* These two defines are used to "do the right thing" when it comes to 
manipulating the system data cache.  The DO_CACHE_BEFORE_DMA() macro is
used to prepare the buffer and cache, in DME_SETUP(), before any data
xfers take place, ie R4000.  The DO_CACHE_AFTER_DMA() macro is used
to take care of the buffer and cache following a data DMA xfer, ie MIPS,
in DME_END() and the flushing of the safe zones.  The two argument for
both the macros are a KO seg address and a count number. */

#ifdef __alpha

#   define DO_CACHE_BEFORE_DMA( k0, n )		/* Alpha has cache coherentcy */
#   define DO_CACHE_AFTER_DMA( k0, n )		/* there is no need for this */

#else			/* MIPS */

#   define DO_CACHE_BEFORE_DMA( k0, n )		/* 09/18/92 do nothing yet */

#   define DO_CACHE_AFTER_DMA( k0, n )					\
    {									\
	(void)clean_dcache( (k0), (n) )	/* call the system support */	\
    }

#endif	/* __alpha */

#endif	/* __DME_COMMON_H__ */



