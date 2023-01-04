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
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * derived from pdma3min.c	4.5.1.1      (ULTRIX)  4/3/91";
 */


/************************************************************************
 *
 * pdmaflam.c
 *
 * Pseudo DMA routines for the flamingo
 *
 * Modification history:
 *
 * 12-26-91  Farrell Woods
 * First crack at support for Flamingo
 *
 * 5 June 1991	Tom Tierney
 * Merge of ULTRIX 4.2 SCSI subsystem and OSF/1 reference port work.
 * This module is a result of work done by Fred Canter and Bill Burns
 * to merge pdma3min.c version 4.5.1.1 from ULTRIX and the OSF/1
 * reference port of the SCSI subsystem.
 *
 * Cleaned up a little and added call to kmem_alloc since KM_ALLOC
 * alias was removed. 
 *
 * 04 June 1991 Dave Gerson 
 * Removed pdma_ds5000_300 initialization. 5000_300 now uses
 * info from 5000_100.
 *
 * 10 May 1991  Paul Grist
 * Added Entry for 3max+ (DS_5000_300) which also uses IOASIC.
 *
 * 15 Aug 1990  Robert Scott
 * Created this file to implement PDMA code for 3min hardware.  Based upon
 * 3max PDMA code.
 *
 ************************************************************************/
/* ---------------------------------------------------------------------- */

#define ENTRYEXIT	0x00010000
#define REGLOAD		0x00020000
#define DETAILDAT	0x00040000
#define INTTRACE	0x00080000
#define	PADTRACK	0x00100000	
#define DUMPHEX		0x00200000
#define PASS2		0x00400000
#define FRAG		0x00800000

#define sc_ascnum	sc_siinum

#include <data/scsi_data.c>
#include <io/dec/tc/tc.h>
#include <io/dec/scsi/alpha/scsi_debug.h>
#include <io/dec/scsi/alpha/pdmaflam.h>

/* ---------------------------------------------------------------------- */
/* External functions and variables. */

extern int scsidebug;
extern PDMA_ENTRY pdma_entry[];		/* the entry array */

/* ---------------------------------------------------------------------- */
/* Local Data area. */

/* Entry structure for the pseudo DMA routines for the PMAZBA.  The external
declarations are needed for the forward reference. */

extern int pmaz_aa_init();
extern int pmaz_aa_setup();
extern int pmaz_aa_start();
extern int pmaz_aa_cont();
extern int pmaz_aa_end();
extern int pmaz_aa_flush();
extern int bcopy();
extern int bzero();

PDMA_ENTRY pdma_flamingo =
{
    ALPHA_FLAMINGO,			/* cpu type value */
    ( 0 ),				/* MISC control flags */
    pmaz_aa_init,
    pmaz_aa_setup,
    pmaz_aa_start,
    pmaz_aa_cont,
    pmaz_aa_end,
    bcopy,				/* kernel routines can be used */
    bcopy,
    bzero,
    (0),
};

int targmatch = 9;
int targtmp;

/* ---------------------------------------------------------------------- */
/*
unsigned *ioa_addrcvt( addr )

Inputs:
	char *addr;		 K2SEG address for the data 

Function:
 	Convert input virtual address to physical which, in turn, must
        be modified to meet IOASIC format requirements.
Return:
	Physical memory address in IOASIC format.
*/

unsigned ioa_addrcvt ( addr )
char *addr;
    {
    unsigned long a = (unsigned long) addr;
    unsigned long p;
    vm_offset_t phys_addr;

    if (svatophys( a , &phys_addr ) != KERN_SUCCESS) {
	printf("ioa_addrcvt: can't convert address 0x%lx\n", a);;
	panic("");
    }
    p = phys_addr;

    if ( p == 0 )
        {
/* OHMS         printstate |= PANICPRINT; */
        printf(  "ioa_addrcvt: Invalid return from svatophys().\n" );
        printf( "ioa_addrcvt: Address 0x%x maps to physical address 0.\n",a );
	return 0;
        }
    return ((unsigned int)p >> 2);
    }

/* ---------------------------------------------------------------------- */
/*
void *backcvt( addr )

Inputs:
	void *addr;		 IOASIC format physical address 

Function:
        Convert an IOASIC format physical address into normal physical
        address format.

Return:
	Physical memory address in standard format.
*/

unsigned long backcvt(unsigned int addr)
    {
    unsigned long a = (unsigned long) addr;
    unsigned long p;
    p = a << 2;
    return p;
    }

/* ---------------------------------------------------------------------- */
/*
int pdma_init( sc )

Inputs:	sz_softc pointer

Function:
	Initialize all that is necessary for the DMA engine/code to run 
	for the particular system.  For the 3min, DAT table space and
        fragment buffer space must be allocated.

	The PDMA related fields will be cleared/initialized and one of the
	pipe buffers will be set to READY.

Return:
	PDMA_SUCCESS		 all is ready and ok 
	PDMA_FAIL		 a fatal(?) problem has occured 
*/

char global_fragbuf[2][(sizeof( FRAGBUF )+DATTBL_SIZ)*8];

int pmaz_aa_init( sc )
struct sz_softc *sc;
    {
    int targid;			/* loop counter for targets */
    PDMA *pd;			/* pointer for the DMA control struct */
    static char *fbp=0;                /* Fragment buffer pool pointer */
    scsi_cir *cir;
    scsi_imer *imer;


    cir = (scsi_cir *)((unsigned long)sc->ioasicp + CIR_O);
    imer = (scsi_imer *)((unsigned long)sc->ioasicp + IMER_O);

    if (sc->sc_ascnum == 0) {
        cir->cir_reg = CIR_CFG;
    imer->imer_reg = IMER_CFG;
}

  /* For each of the target slots on a SCSI bus, this is one of the valid
    magic #'s of 8.  Setup and assign the pointers/offsets for each of the
    DMA control structures. */

  /* Allocate a chunk of memory for the fragment buffers and DAT table 
     for each target */

#ifdef WORKING_VM
    if ( !fbp ) {
        /* Note: The original code using KM_ALLOC specifies KM_NOWAIT,
         * KM_CLEAR, KM_CONTIG and KM_NOCACHE: this code may have to use
         * the zones package to get the equivalent functions.
         */
        fbp = (char *) kmem_alloc(kernel_map, (sizeof( FRAGBUF )+DATTBL_SIZ)*8);
        if (fbp == NULL)
          panic("pmaz_aa_init: could not allocate frag buffer pool\n");
        }
#else
	fbp = global_fragbuf[sc->sc_ascnum];
#endif
 
    for( targid = 0; targid < NDPS; targid++ )
        {
	pd = &sc->pdma_ctrl[ targid ];	/* assign the pointer */

      /* Clear the per call elements. */
	pd->pstate = PDMA_IDLE;		/* clear out misc flags */
	pd->iodir = 0;			/* clear direction */
	pd->count = 0;			/* clear total count */
	pd->data_addr = (char *)NULL;	/* clear data address */

     /* The DAT table and fragment buffer for a given target must be
        extracted from the large buffer kmalloc'ed above.  One might
        think of the large buffer as being an array (of length 8) of
        structures containing a DAT table followed by a fragment 
        buffer for each target.  Then again, one might not.
     */
        pd->dat_addr = (char *) ( fbp +
            targid * ( DATTBL_SIZ + sizeof( FRAGBUF ) ) );
        pd->frag_bufp = (char *) ( fbp + 
            targid * ( DATTBL_SIZ + sizeof( FRAGBUF ) ) +
            DATTBL_SIZ );

#ifdef PDMADEBUG
        PRINTD( targid, DETAILDAT, 
            ("init:  pd->dat_addr = 0x%x\n", pd->dat_addr ));
        PRINTD( targid, DETAILDAT, 
            ("init:  pd->frag_bufp = 0x%x\n", pd->frag_bufp ));
#endif

	pd->usercnt = 0;		/* user space count */
	pd->targcnt = 0;		/* target count */

	}
    return( PDMA_SUCCESS );
    }

/* ---------------------------------------------------------------------- */
/*
int pdma_setup( sc, targid, dir, count, addr )

Inputs:
	sz_softc *sc;		 pointer to the softc structure 
	int targid;		 current target ID 
	int dir;		 direction for I/O xfer 
	long count;		 number of bytes to xfer 
	char *addr;		 address for the data 

Function:
Return:
	PDMA_SUCCESS		 all is ready and ok 
	PDMA_FAIL		 a fatal(?) problem has occured 
	PDMA_RETRY(?)		 unable to setup, try again later 
*/

int pmaz_aa_setup( sc, targid, dir, count, addr )
struct sz_softc *sc;	/* pointer to the softc structure */
int targid;			/* current target ID */
int dir;			/* direction for I/O xfer */
long count;			/* number of bytes to xfer */
char *addr;			/* address for the data */
    {
    PDMA *pd;			/* pointer for the DMA control struct */
    int i;			/* loop counter */

#ifdef PDMADEBUG
    PRINTD( targid, ENTRYEXIT,
        ("pmaz_aa_setup: entry sc=0x%lx targ=%d dir=%d cnt=%d addr=0x%lx\n  bcount=%d dmaxfer=%d\n",
        sc, targid, dir, count, addr, sc->sc_b_bcount[targid], 
        sc->sc_dmaxfer[targid] ));
#endif

  /* Setup the DMA control structure for this target. */
    pd = &sc->pdma_ctrl[ targid ];	/* assign the pointer */
    targtmp = targid;			/* used for debugging purposes only */

  /* If this pipe is not IDLE return PDMA_RETRY. */
    if( pd->pstate != PDMA_IDLE )
        {
        PRINTD( targid, ENTRYEXIT, ("pmaz_aa_setup: exit - PDMA_RETRY\n"));
	return( PDMA_RETRY );
        }
    
    pd->pstate = PDMA_SETUP;		/* setup is being done */
    pd->iodir = dir;			/* load direction */

    /* must save difference between requested xfer count and real count */
    /* in order to pad out the xfer with the '94 */
    pd->count = sc->sc_b_bcount[targid] - sc->sc_dmaxfer[targid];
    /* OK, so this doesn't have anything to do with the sii, we don't 
       need another counter in the softc structure and I don't like
       using #defines to mask things.  
    */
    sc->sc_siidmacount[targid] = count - pd->count;
#ifdef PDMADEBUG
    if ( scsidebug & PADTRACK && targmatch == targid )
        {
        if ( sc->sc_siidmacount[targid] )
        cprintf("setup: Padding %d -> %d : %d\n", count, pd->count,
            sc->sc_siidmacount[targid] );
        }
#endif

    pd->data_addr = addr;		/* load data address */
    pd->usercnt = 0;			/* clear working count */
    pd->targcnt = 0;			/* clear working count */

  /* Build the DAT table for this transfer. */
    if ( blddattbl( targid, pd, pd->count, addr, dir ) == PDMA_FAIL )
        {
        printf( "setup: dat table build failure\n" );
        printf( "  target=%d dat_addr=0x%x pd->count=%d addr=0x%x dir=%d\n", 
            targid, pd->dat_addr, pd->count, addr, dir );
        dumptbl( (DTENT *)pd->dat_addr );
        return( PDMA_FAIL );
        }

  /* The DAT indexing scheme always begins with 1 as entry 0 is reserved
     in case 'backing-up' is necessary.  This could occur due to a
     disconnect which leaves a segment of less than a double word in
     length to be transferred.  In such a case, the remainder of the 
     segment interrupted would be broken into two portions:  A transfer of
     less than a double word into a local buffer followed by a normal DMA
     transfer.
  */
    pd->dat_index = 1;

#ifdef PDMADEBUG
    if ( scsidebug & DETAILDAT && ( targid==targmatch ) ) 
        dumptbl( (DTENT *)pd->dat_addr );
#endif
  /* If the setup is for a write and there the table begins with a 'special
     case' transfer from a local buffer, fill that buffer right now.
  */
    if ( ( dir == SZ_DMA_WRITE ) && is_local_xfer( pd ) )
        frag_buf_load( pd );

#ifdef PDMADEBUG
    PRINTD( targid, ENTRYEXIT, ("pmaz_aa_setup: exit\n"));
#endif
    return( PDMA_SUCCESS );		/* ready */
    }

/* ---------------------------------------------------------------------- */
/* 
int pdma_start( sc, targid, opcode )

Inputs:
	sz_softc *sc;		 pointer to the softc structure 
	int targid;		 current target ID 
	int opcode;		 value to start the dma oper in the chip 

Function:
	This routine is fairly straight forward.  It is responsible for loading
	what ever is necessary into the DMA engine and the SCSI chip.  The 
	dma control structure for the target should contain all the information.
	The SZ_DID_DMA flag in the softc structure will be set.  This informs
	the driver that data xfers are taking place.
	    NOTE: An opcode argument is passed to this routine mostly for the
	    NCR 53c94.  

Return:
	PDMA_SUCCESS		 all is ready and ok 
	PDMA_FAIL		 a fatal(?) problem has occured 
*/

int pmaz_aa_start( sc, targid, opcode )
struct sz_softc *sc;	/* pointer to the softc structure */
int targid;			/* current target ID */
int opcode;			/* value to start the dma oper in the chip */
    {
    PDMA *pd;			/* pointer for the DMA control struct */
    ASC_REG *ascaddr; 		/* pointer for the SCSI chip registers */
    DTENT *datp, *datep;	/* pointer to DAT table and individual entry */

#ifdef PDMADEBUG
    PRINTD( targid, ENTRYEXIT, 
        ("pmaz_aa_start: entry sc=0x%lx targ=%d op=0x%x, ",sc, targid, opcode ));
#endif
  /* Initialize local variables. */

    pd = &sc->pdma_ctrl[ targid ];		/* assign the pointer */
#ifdef PDMADEBUG
    PRINTD( targid, DETAILDAT, ("    dat_index=%d\n", pd->dat_index ));
#endif
    ascaddr = (ASCREG *)sc->sc_scsiaddr;	/* get the chip addr */
    pd->opcode = opcode;		/* save for later use */

  /* Load the information from the dma control structure for buffer 0 into
    the SCSI chip and DMA engine. */

    datp = (DTENT *)pd->dat_addr;
    datep = (DTENT *)&(datp[pd->dat_index]);
#ifdef PDMADEBUG
    if ( scsidebug & DETAILDAT && ( targid==targmatch ) ) 
        dumpent( datep );
#endif

    if ( datep->length==0 )	/* handle special case of padded xfer */
	{
        ASC_LOADCNTR(ascaddr, sc->sc_siidmacount[targid]);	/* load counter */
        wbflush();				/* clear write buffer */
        sc->sc_asccmd = ASC_DMA | ASC_XPAD ; 
        ascaddr->asc_cmd = sc->sc_asccmd;
        wbflush();				/* clear write buffer */
        sc->sc_szflags[ targid ] |= SZ_DID_DMA;
        return PDMA_SUCCESS;
	}

    if ( datep->addr == 0 )
        {
        printf( "start: Invalid DAT entry.\n" );
        dumpent( datep );
        }

    if ( is_local_xfer( pd ) && datep->dir == IOASIC_WRITE )
        {
        pd->pstate = PDMA_ACTIVE;
        sc->sc_szflags[ targid ] |= SZ_DID_DMA | SZ_PIO_INTR;
        asc_FIFOsenddata (sc, ASC_XINFO, datep->uadr, datep->length );
        }
    else
        {
	ssrdmaoff(sc);
        if ( dmapload( sc, targid, datep ) == PDMA_FAIL )
            {
            printf( "pdma_start: Failure return from dmapload\n" );
            return PDMA_FAIL;
            }
        ssrdmaon( sc, datep->dir );
    
#ifdef PDMADEBUG
        PRINTD( targid, REGLOAD, 
            ("    start: loading ASC counter (base 0x%x) with length (%d)\n",
            ascaddr, datep->length ));
#endif
        ASC_LOADCNTR(ascaddr, datep->length);	
    
        wbflush();

        sc->sc_asccmd = pd->opcode;   
        ascaddr->asc_cmd = pd->opcode;
        wbflush();				/* clear write buffer */
        }

  /* Set the pstate to inform the driver that DMA is in progress. */

    pd->pstate = PDMA_ACTIVE;
    sc->sc_szflags[ targid ] |= SZ_DID_DMA;
#ifdef PDMADEBUG
    PRINTD( targid, ENTRYEXIT, ("pdma_start: exit\n"));
#endif
    return PDMA_SUCCESS;
    }

/* ---------------------------------------------------------------------- */
/*
int pdma_cont( sc, targid )

Inputs:
	sz_softc *sc;		 pointer to the softc structure 
	int targid;		 current target ID 

Function:
	Handle intermediate DMA functions during the transfer.  

Return:
	The number of bytes transfered sofar for this DMA operation.
*/

int pmaz_aa_cont( sc, targid )
struct sz_softc *sc;	/* pointer to the softc structure */
int targid;			/* current target ID */
    {
    PDMA *pd;			/* pointer for the DMA control struct */
    ASC_REG *ascaddr; 		/* pointer for the SCSI chip registers */
    DTENT *datp, *datep;	/* pointer to DAT table and individual entry */

    char *newbufp;		/* buffer pointer for use in DAT table adj.  */
    int ecount;			/* used in DAT table rebuilding */
    int tidx;			/* temporary index variable in DAT rebuild */
    int	ffr;			/* value for the FIFO flags */
    int dbcount;		/* number of bytes in IOASIC data buffer */
    int	tcount;			/* corrected count for the ASC counters */
    int lcount;			/* local count from the transfers */
    int tmp1;
    vm_offset_t phys_addr;

    pd = &sc->pdma_ctrl[ targid ];	/* assign the pointer */
#ifdef PDMADEBUG
    PRINTD( targid, ENTRYEXIT, 
        ("pmaz_aa_cont: entry sc:%x targ:%d\n", sc, targid));
    PRINTD( targid, ENTRYEXIT, ("    dat_index:%d\n", pd->dat_index ));
#endif
    ascaddr = (ASCREG *)sc->sc_scsiaddr;/* get the chip addr */

  /* First pull out the DAT table entry corresponding to this xfer */
    datp = (DTENT *)pd->dat_addr;
    datep = (DTENT *)&(datp[pd->dat_index]);
#ifdef PDMADEBUG
    if ( scsidebug & DETAILDAT && ( targid==targmatch ) ) 
        dumpent( datep );
#endif

    if ( !(datep->length) )	/* handle special case of padded xfer */
	{
        sc->sc_szflags[ targid ] |= SZ_DID_DMA;
        PRINTD( targid, PADTRACK, ("    cont:  padded xfer return: %d bytes\n",
                pd->targcnt + sc->sc_siidmacount[targid] ));
        return pd->targcnt + sc->sc_siidmacount[targid];
	}

  /* From the active buffers count and the SCSI chips actual xfer counter
    register value, calc how much data was transfered.  Update the working
    counter(s) for the return xfer count.  So far the SCSI chip counters
    are all down counters, making the xfer count calc a simple subtaction.
    The counters in the ASC have to be corrected for the # of bytes
    left in the FIFO but not transferred when the target changed phase.

    It should be noted that the 3min system (or possibly the softc struct)
    exhibits a situation which should not occur; the TC bit in the status
    register is set but the dma xfer counter is non-zero.
  */
    ssrdmaoff( sc );			/* turn off DMA */
    ffr =  (int)ascaddr->asc_ffss;	/* get the fifo flags count */
    ffr &= ASC_FIFO_MSK;		/* mask off sequ step bits */

    ASC_GETCNTR(ascaddr, tcount);	/* get the counter value */
    ASC_LOADCNTR(ascaddr, 0);	             /* JAG - force TC bit off */
    wbflush();

    dbcount = 0;
/*    dbcount = getdbcount( sc, datep->dir );  /* bytes stuck in IOASIC? */
    if ((unsigned long)datep->addr & 3)
        startflush(sc, datep->addr);
    if (pd->iodir == SZ_DMA_READ && ((((unsigned long)datep->addr) + datep->length) & 3))
        endflush(sc, datep);

    if ( datep->dir==IOASIC_WRITE )  /* for RDAT bug */
        lcount = datep->length - ffr;
    else
        lcount = datep->length;

    if ( (lcount & 1) && dbcount )	/* if odd, then dbcount is off by 1 */
        dbcount--;			/* RPS 03/28/91 */

    if ( is_local_xfer( pd ) )
        newbufp = datep->uadr + lcount;
    else
        newbufp = datep->addr + lcount;

    if( pd->iodir == SZ_DMA_READ )
        {
        if ( dbcount )			/* data was left in the IOASIC     */
            flushdb( sc, datep, dbcount );
#ifdef PDMADEBUG
        if ( scsidebug & DUMPHEX && targid == targmatch )
            {
            if ( lcount > 0x40 )
                {
                dumphex(datep->addr, 0x20 );
                dumphex(datep->addr+lcount-0x20, 0x20 );
                }
            else 
                dumphex( datep->addr, lcount );
            }
#endif
      /* If the operation is a DMA_IN: check for fragment buffer usage,
         clean out that data if necessary, then determine if new DAT table
         entries must be built or modified before continuing */
        if ( is_local_xfer( pd ))	/* a local buffer was in addr? */
            {
            flush_fragbuf( pd, lcount );
            }
        else				/* completed normally */
            {
	    mb();
            datep->completed = 1;	/* tag the entry complete */
            pd->targcnt += datep->length;  /* bump transfer counter */
            pd->dat_index++;
            }

      /* Load up the IOASIC chip with the address and '94 with the count 
         for the next buffer. */

	/* Load addr reg, no need to set the write bit. */
        datep = (DTENT *)&(datp[pd->dat_index]);
#ifdef PDMADEBUG
        if ( scsidebug & DETAILDAT && ( targid==targmatch ) ) 
            {
            PRINTD( targid, DETAILDAT, 
                ("    cont: dat index=%d\n", pd->dat_index ));
            dumpent( datep );
            }
#endif
        if (datep->length != 0)
            {
	    ssrdmaoff(sc);
            if ( dmapload( sc, targid, datep ) == PDMA_FAIL )
                {
                printf( "pdma_start: Failure return from dmapload\n" );
                return PDMA_FAIL;
                }
            ssrdmaon( sc, datep->dir );
#ifdef PDMADEBUG
            PRINTD( targid, REGLOAD, 
                ("    cont: loading ASC counter (base 0x%x) with length (%d)\n",
                ascaddr, datep->length ));
#endif
            ASC_LOADCNTR(ascaddr, datep->length);	/* load counter */
            wbflush();

      /* Start the DMA operation in the ASC. */
            sc->sc_asccmd = pd->opcode;   
            ascaddr->asc_cmd = pd->opcode;
            wbflush();				/* clear write buffer */

      /* Set the flag to inform the driver that DMA is in progress. */
            sc->sc_szflags[ targid ] |= SZ_DID_DMA;
            } /* end of (datep->length != 0 ) */
        else
            {  /* time to pad out an xfer */
            ASC_LOADCNTR(ascaddr, sc->sc_siidmacount[targid]);	/* load counter */
            wbflush();				/* clear write buffer */
            sc->sc_asccmd = ASC_DMA | ASC_XPAD ; 
            ascaddr->asc_cmd = sc->sc_asccmd;
            wbflush();				/* clear write buffer */
            sc->sc_szflags[ targid ] |= SZ_DID_DMA;
            }
        } /* end of (pd->iodir == SZ_DMA_READ) */
    else
        {                              /* Write case now */
        datep->completed = 1;          /* tag the entry complete */
        pd->targcnt += datep->length;  /* bump transfer counter */
        pd->dat_index++;
        datep = (DTENT *)&(datp[pd->dat_index]);
        if ( is_local_xfer( pd ) )	/* is this a fragment? */
            {
            if ( frag_buf_load( pd ) == PDMA_FAIL )
                {
                printf( "pmaz_aa_cont(3min)(12): (WRITE(2)) frag buffer load, dat_addr=0x%x index= %d\n",
                    pd->dat_addr, pd->dat_index );
                return( PDMA_FAIL );
                }
            }

#ifdef PDMADEBUG
        if ( scsidebug & DETAILDAT && ( targid==targmatch ) ) 
            {
            PRINTD( targid, DETAILDAT, 
                ("    cont: dat index=%d\n", pd->dat_index ));
            dumpent( datep );
            }
#endif
        if ( is_local_xfer( pd ) && datep->dir == IOASIC_WRITE )
            {
            sc->sc_szflags[ targid ] |= SZ_DID_DMA | SZ_PIO_INTR;
            asc_FIFOsenddata (sc, ASC_XINFO, datep->uadr, datep->length );
            }
        else
            {
            if ( datep->length != 0 )
                {
                /* Load addr reg, set the write direction bit. */
		ssrdmaoff(sc);
                if ( dmapload( sc, targid, datep ) == PDMA_FAIL )
                    {
                    printf( "pdma_start: Failure return from dmapload\n" );
                    return PDMA_FAIL;
                    }
                ssrdmaon( sc, datep->dir );
#ifdef PDMADEBUG
                PRINTD( targid, REGLOAD, 
                    ("    cont: loading ASC counter (base 0x%x) with length (%d)\n",
                    ascaddr, datep->length ));
#endif
                ASC_LOADCNTR(ascaddr, datep->length);	/* load counter */
                wbflush();
    
          /* Start the DMA operation in the ASC. */
                sc->sc_asccmd = pd->opcode;   
                ascaddr->asc_cmd = pd->opcode;
                wbflush();				/* clear write buffer */

          /* Set the flag to inform the driver that DMA is in progress. */
    
                sc->sc_szflags[ targid ] |= SZ_DID_DMA;
                } /* end of if ( datep->length != 0 ) */
            else
                {  /* time to pad out an xfer */
                ASC_LOADCNTR(ascaddr, sc->sc_siidmacount[targid]);	/* load counter */
                wbflush();				/* clear write buffer */
                sc->sc_asccmd = ASC_DMA | ASC_XPAD ; 
                ascaddr->asc_cmd = sc->sc_asccmd;
                sc->sc_szflags[ targid ] |= SZ_DID_DMA;
                wbflush();				/* clear write buffer */
                }
            }
        } /* end of else on (pd->iodir == SZ_DMA_READ) */
#ifdef PDMADEBUG
    PRINTD( targid, ENTRYEXIT, ("pmaz_aa_cont:  exit\n" ));
#endif
    return pd->targcnt;	/* return current progress */
    }

/* ---------------------------------------------------------------------- */
/*

int pdma_end( sc, targid )

Inputs:
	sz_softc *sc;		 pointer to the softc structure 
	int targid;		 current target ID 

Function:
	Handle the completion of the DMA operation.  Free up the necessary
	DMA resources that were allocated in dma_setup().  Handle the final
	move of the data to user space if the operation was a read.

Return:
	The number of bytes transfered over the entire DMA operation.
*/

int pmaz_aa_end( sc, targid )
struct sz_softc *sc;            /* pointer to the softc structure           */
int targid;			/* current target ID                        */
    {
    PDMA *pd;			/* pointer for the DMA control struct       */
    ASC_REG *ascaddr; 		/* pointer for the SCSI chip registers      */
    DTENT *datp, *datep;	/* pointer to DAT table and individual entry */
    int	ffr;			/* value for the FIFO flags                 */
    int dbcount;		/* number of bytes in IOASIC data buffer    */
    int	tcount;			/* corrected count for the ASC counters     */
    int lcount;			/* local count from the transfers           */
    int i;			/* buffer loop counter                      */
    int tmp1;
    vm_offset_t phys_addr;

    pd = &sc->pdma_ctrl[ targid ];	/* assign the pointer */
#ifdef PDMADEBUG
    PRINTD( targid, ENTRYEXIT, 
        ("pmaz_aa_end: entry sc=0x%x targ=%d\n", sc, targid));
    PRINTD( targid, DETAILDAT, ("    dat_index=%d\n", pd->dat_index ));
#endif

    ascaddr = (ASCREG *)sc->sc_scsiaddr;/* get the chip addr */

  /* From the active buffers count and the SCSI chips actual xfer counter
    register value, calc how much data was transfered.  Update the working
    counter(s) for the return xfer count.  So far the SCSI chip counters
    are all down counters, making the xfer count calc a simple subtaction.
    The counters in the ASC have to be corrected for the # of bytes
    left in the FIFO but not transferred when the target changed phase. */

    ssrdmaoff( sc );			/* turn off DMA */
    ffr =  (int)ascaddr->asc_ffss;	/* get the fifo flags count */
    ffr &= ASC_FIFO_MSK;		/* mask off sequ step bits */

    ASC_GETCNTR(ascaddr, tcount);	/* get the counter value */

    ASC_LOADCNTR(ascaddr, 0);		/* JAG - force TC bit off */
    wbflush();

  /* Pull out the DAT table entry corresponding to this xfer */
    datp = (DTENT *)pd->dat_addr;
    datep = (DTENT *)&(datp[pd->dat_index]);

    dbcount = 0;
/*    dbcount = getdbcount( sc, datep->dir );  /* bytes stuck in IOASIC? */
    if (pd->iodir == SZ_DMA_READ && ((unsigned long)datep->addr & 3))
        startflush(sc, datep->addr);

    if (pd->iodir == SZ_DMA_READ && ((((unsigned long)datep->addr) + datep->length) & 3))
        endflush(sc, datep);
/*    dbcount = getdbcount( sc, datep->dir );  /* bytes stuck in IOASIC? */

    datep->completed = 1;	/* tag the entry complete */
    if ( !datep->length )		/* take care of empty last DAT */
        {
        sc->sc_szflags[ targid ] &= ~SZ_DID_DMA;	/* DID_DMA is done */
        sc->sc_szflags[ targid ] &= ~SZ_PIO_INTR;	/* DID_DMA is done */

        pd->pstate = PDMA_IDLE;			/* all done */
        return( pd->targcnt + sc->sc_siidmacount[targid] );
        }

#ifdef PDMADEBUG
    if ( scsidebug & DETAILDAT && ( targid==targmatch ) ) 
        dumpent( datep );
#endif

    if ( (sc->sc_asc_sr&ASC_TC) && (tcount != 0) )	/* shouldn't happen */
        {
        if ( datep->dir == IOASIC_WRITE ) 
            tcount = 0;
	}

    lcount = datep->length;		/* Assume all transferred          */

    if ( sc->sc_asc_sr & ASC_TC )	/* terminal count is not set?      */
        tcount = 0;
    else
        lcount -= tcount;		/* then subtract remainder         */

    if ( datep->dir == IOASIC_WRITE )	/* in the case of writes...        */
        lcount -= ffr;		/* don't forget the fifo flags reg */

  /* If the last operation was a SZ_DMA_READ, move the data to user space. */

    if ( (lcount & 1) && dbcount )	/* if odd, then dbcount is off by 1 */
        dbcount--;			/* RPS 03/28/91 */

    if( pd->iodir == SZ_DMA_READ )
        {
        if ( dbcount )			/* data was left in the IOASIC     */
            flushdb( sc, datep, dbcount );
#ifdef PDMADEBUG
        if ( scsidebug & DUMPHEX && targid == targmatch )
            {
            if ( lcount > 0x40 )
                {
                dumphex(datep->addr, 0x20 );
                dumphex(datep->addr+lcount-0x20, 0x20 );
                }
            else 
                dumphex( datep->addr, 0x20 );
            }
#endif
        if ( is_local_xfer( pd ))	/* a local buffer was in addr?     */
            {
	/* MOVE THE FRAGMENT BUFFER BYTES TO USER BUFFER HERE */
            if( lcount != 0 )	/* don't bother if no data */
                {	/* Change this to a flush_frag_buf() */
                flush_fragbuf( pd, lcount );
                wbflush();
                pd->targcnt += lcount;	/* update target count */
                }
            }
        else			/* else end of a normal DMA xfer? */
            {
/*            printf("pmaz_aa_end, targcn: = %d, tcount = %d\n", pd->targcnt, tcount ); */
	    mb();
            pd->targcnt += datep->length - tcount;
            }
        }			/* end of if read */
    else
        {	/* write case */
        pd->targcnt += lcount;	/* update target count */
        }
  /* Clean up after ourselves.  Clear the SZ_DID_DMA flag. */

    sc->sc_szflags[ targid ] &= ~SZ_DID_DMA;	/* DID_DMA is done */
    sc->sc_szflags[ targid ] &= ~SZ_PIO_INTR;	/* DID_DMA is done */

    pd->pstate = PDMA_IDLE;			/* all done */

  /* Return the accumulated working count to the driver.  No checking has
    been done to verify that all the data has been transfered. */
#ifdef PDMADEBUG
    PRINTD( targid, ENTRYEXIT, ("pmaz_aa_end: exit\n"));
#endif
    return( pd->targcnt );
    }

int pmaz_aa_flush( sc, targid )
struct sz_softc *sc;	/* pointer to the softc structure */
int targid;			/* current targit ID */
    {
    PDMA *pd;			/* pointer for the DMA control struct */
    DTENT *datp, *datep;	/* pointer to DAT table and individual entry */

panic("pmaz_aa_flush\n");
    pd = &sc->pdma_ctrl[ targid ];	/* assign the pointer */
#ifdef PDMADEBUG
    PRINTD( targid, ENTRYEXIT,
        ("pmaz_aa_flush: enter, target= %d, dat_index=%d\n", 
        targid, pd->dat_addr ));
#endif

  /* Pull out the DAT table entry corresponding to this xfer */
    datp = (DTENT *)pd->dat_addr;
    datep = (DTENT *)&(datp[pd->dat_index]);
    if ( datep->dir == IOASIC_READ )
        {
        (*sc->dma_rbcopy)			/* move the data */
            ( datep->addr, datep->uadr, datep->length );
        }

    return( PDMA_SUCCESS );		/* all done */ 
    }

/* ---------------------------------------------------------------------- */
/* Support routines. */

/* ---------------------------------------------------------------------- */

/* Dumps the contents of the PDMA structure to the console. */

int
dump_pdma( pd )
    PDMA *pd;
{
    int i;

    printf( "Dumping pd: 0x%x\n", pd );
    printf( "    pstate 0x%x  ", pd->pstate );
    printf( "    pflags 0x%x  ", pd->pflags );
    printf( "    iodir 0x%x\n", pd->iodir );
    printf( "    dat_addr    : 0x%x\n", pd->dat_addr );
    printf( "    dat_index   : 0x%x\n", pd->dat_index );
    printf( "    data_addr   : 0x%x\n", pd->data_addr );
    printf( "    usercnt     : 0x%x\n", pd->usercnt );
    printf( "    targcnt     : 0x%x\n", pd->targcnt );
    printf( "    opcode      : 0x%x\n", pd->opcode );
    }

int frag_buf_load( pd )
PDMA *pd;
    {
    DTENT *datp, *datep;    /* pointer to DAT table and individual entry */
    char *sadr, *dadr;

  /* Pull out the DAT table entry corresponding to this xfer */
    datp = (DTENT *)pd->dat_addr;
    datep = (DTENT *)&(datp[pd->dat_index]);
    bcopy( datep->uadr, datep->addr, datep->length );
    return PDMA_SUCCESS;
    }

flushfifo( sc )
struct sz_softc *sc;
    {
    ASC_REG *ascaddr;

    ascaddr = (ASCREG *)sc->sc_scsiaddr;	/* get the chip addr */
    ascaddr->asc_cmd = ASC_FLUSH;
    wbflush();				/* clear write buffer */
    }
 
ioasicint( sc, targid, cntlr )
struct sz_softc *sc;
int targid;
int cntlr;
    {
    unsigned int *sir;
    unsigned int sirp, sirm;
    int retval;

    sir = (unsigned int *)((unsigned long)sc->ioasicp + CIR_O );

    sirm = sirp = *sir;
    sirm |= 0xffff0000;

    retval = 1;

#ifdef NOTNEEDED
    if ( sirp & SCSI0_DRDY )
        {
        retval = 0;			
        printf("ioasicint: Unexpected 53C94 data ready interrupt.\n" );
        sirp &= ~SCSI0_DRDY;
        }
#endif

    if ( !cntlr && (sirp & SCSI0_C94 ))
        {
        retval = 0;			/* go through '94 code */
        sirp &= ~SCSI0_C94;
        }
    else if ( cntlr && (sirp & SCSI1_C94 ))
        {
        retval = 0;			/* go through '94 code */
        sirp &= ~SCSI1_C94;
        }
    else
	;
    sirm &= sirp;

    *sir = sirm;
    mb();

    return retval;
    }

int is_local_xfer( p )
PDMA *p;			/* pointer for the DMA control struct */
    {
    int i;
    DTENT *t, *e;

    t = (DTENT *) p->dat_addr; /* grab the address of a table */
    i = p->dat_index;         /* and the index into the table */
    e = &t[i];

    if ( e->uadr )            /* if user buffer address is defined, */
        {                     /* then a local fragment buffer is being */
        return 1;             /* used. */
        }
    else                      /* Yes, this could be a ?: statement but I */
        {                     /* detest those */
        return 0;
        }
    }

dumptbl( tent )
DTENT *tent;
    {
    int k;
    int so = 0;

    printf( "Table dump\n" );
    for( k = 0; k < DATTBL_SIZ; k++ )
        {
        printf( " index:%d  ", k );
  
        if ( !dumpent( &(tent[k]) ) && so )
            k = DATTBL_SIZ;
        else
            so = 1;
        }
    }

dumpent( tent )
DTENT *tent;
    {
    printf( "len=%d addr=0x%lx uadr=0x%lx iadr=0x%x, odd=0x%x ", 
    tent->length, tent->addr, tent->uadr, tent->iadr, tent->odd );
    if ( !tent->completed )
        printf( "not ");
    printf( "completed, " );
    if ( tent->dir == IOASIC_READ )
        printf( "read\n" );
    else if ( tent->dir == IOASIC_WRITE )
        printf( "write\n" );
    else 
#if 0
        cprintf( "unknown\n" );
#else
        printf( "unknown\n" );
#endif
    return( tent->length );
    }
 
blddatent( tent, ecount, addr, uadr, dir )
DTENT *tent; 
unsigned long ecount; 
char *addr, *uadr;
int dir;
    {
    if ( !tent )
	return( PDMA_FAIL );
    tent->length = ecount;
    tent->addr = addr;
    tent->uadr = uadr;
    tent->completed = 0;
    if ( dir == SZ_DMA_READ )
        dir = IOASIC_READ;
    else if ( dir == SZ_DMA_WRITE )
        dir = IOASIC_WRITE;
    else 
        dir = IOASIC_UNDEF;
    tent->dir = dir;
    if ( addr )
        {
        tent->iadr = ioa_addrcvt(addr);
        if ( tent->iadr == 0 )
            {
            dumpent( tent );
	    return( PDMA_FAIL );
            }
	tent->odd = (unsigned int)addr & 3;
        }
    else
        tent->iadr = 0;
    return( PDMA_SUCCESS );
    }

/*  blddattbl - Builds a table of DMA buffers suitable for use with
		the 3min.  Users buffer address is broken up both at
		page boundaries and, when buffer areas are less than 8
		bytes in length, fixed buffer areas allocated by the
		driver are used instead.

    Inputs:	int controller - controller number
                DTENT *table -	A pointer to the table to be filled in.

		long count -	Length of the user's buffer.

		char *addr -	Pointer to the user's buffer.

    Return:	PDMA_FAIL on failure
*/
blddattbl( controller, pd, count, addr, dir ) 
int controller; 
PDMA *pd;			/* pointer for the DMA control struct */
unsigned long count; 
char *addr;
int dir;
    {
    DTENT *table; 
    char *frag;
    int index = 0;
    unsigned ecount;	
    int rem;
    char *eaddr, *uadr;

    table = (DTENT *) pd->dat_addr;
    frag = pd->frag_bufp;

    /* build null 0th entry */
    if ( blddatent( &table[index++], NULL, NULL, NULL, 0 ) == PDMA_FAIL )
        return( PDMA_FAIL );

    while( count )
        {
        if ( !(ecount = caldatent( addr, count )) )
            {
            printf( "blddattbl: Illegal return value (0) from caldatent().\n" );
            return( PDMA_FAIL );
            }
        if ( ecount < 4 )
            {
            eaddr = frag;
            uadr = addr;
	    frag = (char *)((unsigned long)frag + sizeof (FRAGBUF) / 2);
            }
        else 
            {
            eaddr = addr;
            uadr = 0;
            }
	if ( blddatent( &(table[index++]), ecount, eaddr, uadr, dir ) ) 
	    {
            printstate |= PANICPRINT;
	    printf("eaddr=0x%x ecount=%d\n ", eaddr, ecount);
	    dumptbl( table );
            panic("blddatbl:  Invalid IOASIC physical address .\n");
	    }
        count -= ecount;
        addr += ecount;
        }
    if ( blddatent( &table[index++], NULL, NULL, NULL, NULL ) == PDMA_FAIL )
        return( PDMA_FAIL );
    return( PDMA_SUCCESS );
    }

/* rps - original 4K page params - should use other kernel defines */
#define STRPW  0xfffffffffffffffcL
#define PAGS   0x00002000
#define SEGMSK 0x00001fff
#define	FRAGSZ 0x00000004
#define FRAGM  0x00000003

int caldatent( addr, count )
char *addr;
long count;
    {
    unsigned long rem, rem2;
    unsigned long ecount;

    rem = ((unsigned long)addr) & FRAGM;
    rem2 = ((unsigned long)addr) & SEGMSK;
    if ( count < FRAGSZ )
        ecount = count;
    else if ( rem ) /* not octabyte aligned? */
        ecount = FRAGSZ - rem;
    else if ( rem2 )	/* not page aligned? */
        {
        ecount = PAGS - rem2;
        if ( count < ecount )	
            ecount = ( ( ((unsigned long)addr) + count ) & STRPW ) - ((unsigned long)addr);
        }
    else				/* a 4k page */
        {
        ecount = PAGS;
        if ( count < ecount )
            ecount = ( ( ((unsigned long)addr) + count ) & STRPW ) - ((unsigned long)addr);
        }
    return( ecount );
    }

flush_fragbuf( pd, lcount )
PDMA *pd;
int lcount;
    {
    DTENT *datp, *datep;    /* pointer to DAT table and individual entry */
    char *tp;

    if ( lcount > 4 )
        printf("flush_fragbuf: pd=0x%x, lcount=%d\n", pd, lcount );

  /* First pull out the DAT table entry corresponding to this xfer */
    datp = (DTENT *)pd->dat_addr;
    datep = (DTENT *)&(datp[pd->dat_index]);

    if( lcount > 0 )	/* don't bother if no data */
        {
        bcopy( datep->addr, datep->uadr, lcount );
        pd->targcnt += lcount;	/* update target count */
        datep->length -= lcount;	/* adjust DAT entry length */  
        datep->uadr += lcount;
        if ( datep->length == 0 )
            pd->dat_index++;
        wbflush();
        }
    }

dmapload( sc, targid, datep )
struct sz_softc *sc;
int targid;
DTENT *datep;
    {
    unsigned *dmap;         /* pointer to IOASIC DMA Ptr. reg. */
    scsi_dmic *dmic;
    unsigned int x;
    
    PDMA *pd;
#if 0
    DTENT *datp, *datep;    /* pointer to DAT table and individual entry */

    pd = &sc->pdma_ctrl[ targid ];		/* assign the pointer */
    datp = (DTENT *)pd->dat_addr;
    datep = (DTENT *)&(datp[pd->dat_index]);
#endif

    dmic = (scsi_dmic *)((unsigned long)sc->ioasicp + DMIC_O);
    dmap = (unsigned *)((unsigned long)sc->ioasicp + IOA_S_DMAP_O);

    if ( datep == 0 )
        {
        printstate |= PANICPRINT;
        dumptbl( (DTENT *)pd->dat_addr );
        printf("dat_index = %d\n", pd->dat_index );
        dumpent( datep );
        printf( "dmapload:  Null IOASIC (SCSI) DMA address pointer.\n" );
	return PDMA_FAIL;
        }
    *dmap = datep->iadr;
    mb();
    x = dmic->dmic_bits;
    x |= datep->odd;
    dmic->dmic_bits = x;
    mb();
    return PDMA_SUCCESS;
    }

ssrdmaon( sc, dir )
struct sz_softc *sc;
int dir;
    {
    volatile scsi_cir *ioasicp;	/* pointer for the SCSI DMA engine register */
    volatile scsi_dmic *dmic;
    unsigned int x;

    ioasicp = (volatile scsi_cir *)sc->ioasicp;	/* engine address */
    dmic = (volatile scsi_dmic *)((unsigned long)ioasicp + DMIC_O);
    if (sc->sc_ascnum)
	ioasicp = (scsi_cir *)((unsigned long)ioasicp - 0x200);

    x = dmic->dmic_bits;
    if ( dir )		/* read */
	x |= 0x80;
    else
	x &= ~0x80;
    dmic->dmic_bits = x;
    wbflush();

    x = ioasicp->cir_reg;
    x |= sc->sc_ascnum ? CIR_DMA1ENB : CIR_DMA0ENB;
    ioasicp->cir_reg = x;
    wbflush();
    }

ssrdmaoff( sc )
struct sz_softc *sc;
    {
    scsi_cir *ioasicp;		/* pointer for the SCSI DMA engine register */
    unsigned int x;

    ioasicp = (scsi_cir *)sc->ioasicp;	/* engine address */
    if (sc->sc_ascnum)
	ioasicp = (scsi_cir *)((unsigned long)ioasicp - 0x200);

    x = ioasicp->cir_reg;
#ifdef PDMADEBUG
    if ( scsidebug & PASS2 && targtmp == targmatch )
        {
        if ( x & CIR_DMAENB )	
    	    printf("ssrdmaoff:  DMA enabled\n");
        else
            printf("ssrdmaoff:  DMA disabled\n");
        }
#endif
    x &= sc->sc_ascnum ? ~CIR_DMA1ENB : ~CIR_DMA0ENB;
    ioasicp->cir_reg = x;
    wbflush();
    }

dumphex( ptr, len )
char *ptr;
unsigned len;
    {
    int i,j,index;

    printf("\nDump of 0x%x, length %d\n\n", ptr, len );
    for( i=0; i<len; i+=16 )
        {
        printf("  %05x: ", i );
        for( j=0; j<16; j++ )
            {
            index = i+j;
            if (index >= len )
                break;
            printf( "%2x ", ptr[index] );
            }
        printf( "\n" );
        }
    }

/* ------------------------------------------------------------------------ */
/*	getdbuffer( sc, bufp )		Copies the IOASIC data buffers into */
/*					a user buffer.  User buffer must be */
/*					at least 8 bytes long!              */
/*	Inputs:                                                             */
/*	    sc				Pointer to soft_c structure         */
/*	    bufp			Pointer to user buffer.             */
/*                                                                          */
/*	Return value:			Number of valid data bytes or -1 on */
/*					error.                              */
/* ------------------------------------------------------------------------ */

int 
getdbuffer( sc, bufp )
struct sz_softc *sc;
void *bufp;
    {
    unsigned int *dbufp, *ubufp;
    unsigned int nbytes = 0;
    scsi_unaln_0 *un0;		/* pointer for the SCSI DMA engine register */
    scsi_unaln_1 *un1;		/* pointer for the SCSI DMA engine register */
    unsigned long *dmareg;

    if ( !bufp )			/* check for invalid user address */
	return -1;

    dmareg = (unsigned long *)sc->ioasicp;
    ubufp = (unsigned long *)bufp;	/* address user buffer using 32's */

    un0 = (scsi_unaln_0 *)((unsigned long)sc->ioasicp + DMA_UNALN0_O);	/* engine address */
    un1 = (scsi_unaln_1 *)((unsigned long)sc->ioasicp + DMA_UNALN1_O);	/* engine address */

    dbufp = (unsigned int *)un0;
    *ubufp++ = *dbufp;			/* Copy word 1 */

    dbufp = (unsigned int *)un1;
    *ubufp = *dbufp;			/* Copy word 2 */

    nbytes = countbits(un0->mask & 0x7);
    nbytes += countbits((un1->mask >> 24) & 7);

#ifdef PDMADEBUG
    if ( scsidebug & ENTRYEXIT && targtmp == targmatch )
        printf( "getdbuffer:  exit(%x)\n", nbytes );
#endif
    return nbytes;			/* multiply hword's to get bytes */
    }

/* ------------------------------------------------------------------------ */
/*	getdbcount( sc, dir )		Returns the number of bytes filled  */
/*					in the IOASIC data buffers.         */
/*	Inputs:                                                             */
/*	    sc				Pointer to soft_c structure         */
/*	    dir				IO Direction (IOASIC_READ or _WRITE)*/
/*                                                                          */
/*	Return value:			Number of valid data bytes or -1 on */
/*					error.                              */
/* ------------------------------------------------------------------------ */

int 
getdbcount( sc, dir )
struct sz_softc *sc;
int dir;
    {
    volatile unsigned int *pi = (volatile unsigned int *)((unsigned long)sc->ioasicp + DMA_UNALN0_O);
      int count;

    count = countbits((*pi >> 25) & 0x7);

    pi = (volatile unsigned int *)((unsigned long)sc->ioasicp + DMA_UNALN1_O);
    count += countbits(*pi & 0x7);

    return (count);
    }

int 
flushdb( sc, db, cnt )
struct sz_softc *sc;
DTENT *db;
int cnt;
    {
    unsigned int *cregp, creg;		/* Control pointer and content.    */
    unsigned int *dmap;	                /* pointer to IOASIC DMA Ptr. reg. */
    char *padr;                         /* physical address of user buffer */
    char lbuf[32];			/* Local data buffer               */
    int bcnt;				

    dmap = (unsigned int *) ( (unsigned long) sc->ioasicp + IOA_S_DMAP_O );

    padr = (char *) backcvt( *dmap ); /* Calc. the user buf address */
    padr = (char *)((unsigned long)padr | db->odd);

    bcnt = getdbuffer( sc, lbuf );	/* grab data bytes */
    if ( cnt != bcnt )
        {
        printf("flushdb: input count (%d) doesn't match buffer count (%d)\n",
            cnt, bcnt );
        }
/*
    bcopy( lbuf, padr, cnt );
*/
    return cnt;
    }

/* count the number of bits turned on in this char
 */
int
countbits(c)
unsigned char c;
    {
    int count = 0;

    while (c)
	{
	if (c & 1)
	    ++count;
	c >>= 1;
	}
    return (count);
    }

int startflush(sc, pc)
struct sz_softc *sc;    /* pointer to the softc structure */
register char *pc;
{
    unsigned int *p = (unsigned int *)((unsigned long)sc->ioasicp + DMA_UNALN0_O);

#ifdef PDMADEBUG
printf("startflush: pc == 0x%lx, p == 0x%x\n", pc, *p);
#endif

    if (*p & 0x20000000)
	*pc++ = p[2];
    if (*p & 0x40000000)
	*pc++ = p[1];
    if (*p & 0x80000000)
	*pc = p[0];
}

/* this is called to clean up a DMA that didn't end on a 4-byte
 * boundary.  It grabs data out of the DMA unaligned 1 register
 * and stores it at the address given by the start addr in this
 * DAT entry plus its length.
 *
 * sanity checks: the low-order bits of the DMA unaligned register
 * should match the bits given by (datep->addr + datep->length) & 3.
 *
 * the DMA address register should be the sum of datep->iadr and
 * datep->length
 */
int endflush(sc, datep)
struct sz_softc *sc;    /* pointer to the softc structure */
DTENT *datep;
{
    char *pc = datep->addr;
    unsigned int *p = (unsigned int *)((unsigned long)sc->ioasicp + DMA_UNALN1_O);
    unsigned *dmap = (unsigned *)((unsigned long)sc->ioasicp + IOA_S_DMAP_O);

#ifdef PDMADEBUG
printf("endflush: unaln1 == 0x%x, addr + length == 0x%x\n", *p, datep->addr + datep->length);
printf("endflush: dmap == 0x%x, addr + length == 0x%x\n", *dmap, datep->iadr + datep->length);
#endif

    if (*p & 0x4)
	*pc++ = p[3];
    if (*p & 0x2)
	*pc++ = p[2];
    if (*p & 0x1)
	*pc = p[1];
}
