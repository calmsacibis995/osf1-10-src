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
static char *rcsid = "@(#)$RCSfile: tcds_adapt.c,v $ $Revision: 1.1.10.7 $ (DEC) $Date: 1993/11/19 21:27:14 $";
#endif

/*
Facility:

     Ultrix and OSF Scsi CAM Subsystem

Abstract:


	This module handles the asic specific pieces for the dual scsi
	options.  The tcdsconfl* routines are called from the turbochannel
	configuration code when a tcds bus is found in the configuration.
	These routines are responsible for probing the bus to see what
	is attached to them.  The tcdsconfl1 routine knows that upto 2
	asc chips can be attached to the tcds bus.  This routine calls
	the tscsiconf routine to probe each particular asc to see if
	it exists.  If it does exist, then tscsiconf will call the
	sim94_probe.  The sim94_probe code examines the scsi bus for any
	devices that may be connected.  The probe code is responsible for
	enabling interrupts when the chip and data structures are in a
	prepared state.  The return from the probe call, then returns from
	tscsiconf back to tcdsconfl1, then back to the tc.c probe code.
	The entry to tcdsconfl2 is later (and currently not used - see
	tc.c for any desired additional details).

	The tcdsintr routine is the entry for all interrupts from the tcds
	module.  This code checks for any errors in the tcds asic itself,
	and then determines if the C94 interrupt routines needs to be
	called.  If so, then ascintr is called.

	The tcds_enable routine will set the appropriate interrupt enable
	bits in the tcds asic, so that the enabled asc ports can cause
	interrupts.  Note, this enables interrupts only on the tcds asic.

	The tcds_get_speed routine is called from the DME code to determine
	what the maximum clock rate is for this particular tcds asic.  There
	are currently 2 versions available (25 MHz, and 40MHz).

Author:

     John A. Gallant
	Original author to allow the tcds asic to be treated seperate from
	the 53c94 on the other side of the tcds.

     Fred Knight
	Added code to handle tcds bus structures and probing.

*/

/*LINTLIBRARY*/

/* ---------------------------------------------------------------------- */
/* Include files. */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/buf.h>
#include <sys/proc.h>

#include <io/common/iotypes.h>
#include <io/common/devdriver.h>
#include <hal/cpuconf.h>

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
#include <io/cam/sim.h>			/* SIM specific structs and consts */
#include <io/cam/sim_94.h>		/* N53C94 specific */
#include <io/cam/dme_common.h>		/* DME common definitions. */
#include <io/cam/dme_tcds_94_dma.h>
#include <io/cam/tcds_adapt.h>

/* ---------------------------------------------------------------------- */
/* Defines and includes for Error logging.  */

#define CAMERRLOG		/* Turn on the error logging code */

#include <dec/binlog/errlog.h>		/* UERF errlog defines */
#include <io/cam/cam_errlog.h>		/* CAM error logging macro */
#include <io/cam/cam_logger.h>		/* CAM error logging definess */

/* ---------------------------------------------------------------------- */
/* Function Prototypes: */

void tcdsintr();
static int tcds_get_fast();
static int get_idbyte();

/* ---------------------------------------------------------------------- */
/* External declarations: */

extern SIM_SOFTC *softc_directory[];	/* for accessing via "controller" */
extern U32 tcds_rahead;			/* JAG-FUTZ */
extern TCDS_BUS tcds_bus_table[];	/* TCDS bus mapping data */
extern U32 sim94_fast_enable;		/* enable fast flag */
extern U32 tcds_inter_loop_limit;	/* interrupt loop counter */

extern int bcopy();			/* System routine to copy buffer  */
extern int bzero();			/* System routine to clear buffer */
extern int tc_option_control();
extern char *tc_slot_to_name();
extern void tcds_dma_disable();
extern void dme_tcds_errlog();
extern void *asic_pointer_fetch();

extern vm_offset_t pmap_extract();	/* magic VM code to get a phy addr */

/* ---------------------------------------------------------------------- */
/* Local Type Definitions and Constants */ 


/* ---------------------------------------------------------------------- */
/* Initialized and uninitialized data: */

static void (*local_errlog)() = dme_tcds_errlog;	/* logging handler */

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

tcdsconfl1(bustype, binfo, bus)
int bustype;
struct tc_info *binfo;
struct bus *bus;
{
    int	index = (int)bus->tcindx;
    TCDS_ASIC *tcds;
    int	i;				/* loop counter */

    printf("%s%d at %s%d slot %d\n",bus->bus_name, bus->bus_num,
	bus->connect_bus, bus->connect_num, binfo->slot);

    /* Prepare for interrupts from the tcds card (just in case). */

    tcds = (TCDS_ASIC *)CAM_PHYS_TO_KVA(binfo->physaddr);

    /* This also will reset both C94 chips on the tcds */
    tcds->dma_cir = 0;			/* start with it in a known state */
    WBFLUSH();
    DELAY( 25 );			/* give it time to init itself	  */
    *binfo->intr = *bus->intr;		/* set the vector from the config */

    /* For baseboard options, we need to see if the fast bit is there.
     * We don't need to do this for real tc modules, because they will
     * already have the correct name in them.
     */
    if (strncmp(tc_slot_to_name(binfo->slot), "PMAZ-", 5) == 0)
	if (!(tcds->dma_cir & INP2_FAST))	/* clear = fast */
	    bcopy ("PMAZ-FS ", tc_slot[index].modulename, 8);
	    /* The CF94/96 is a little different than the plain C94/96 chip */

    /* Configure both controllers on the tcds card.  If one has a
     * problem or is not built into the config, then mark it as dead (-1).
     */
    for (i=0; i<2; i++) {
	tcds_bus_table[bus->bus_num].asc_alive[i] = -1;
    }				/* start dead */
    for (i=0; i<2; i++) {
	tcds_bus_table[bus->bus_num].asc_alive[i] =
		tscsiconf( bus, i, tcds );
		/* try to make it alive */
    }

    /* Now that we've configured both of the asc's on the tcds, did
     * we end up with both of them disabled?  If so, then return a
     * failure to our caller - to leave the whole tcds disabled
     */
    if ((tcds_bus_table[bus->bus_num].asc_alive[0] == -1) &&
	    	(tcds_bus_table[bus->bus_num].asc_alive[1] == -1)) {
	tcds->dma_cir = 0;		/* leave it in a known state */
	WBFLUSH();			/* make sure its real */
	*binfo->intr = 0;		/* No interrupts from this puppy */
	return( CAM_FALSE );
    }
    return( CAM_TRUE );
}

int
tcdsconfl2(bustype, binfo, bus)
int bustype;
struct tc_info *binfo;
struct bus *bus;
{
return(1);
}

void 
tcdsintr(int controller)
{
    SIM_SOFTC *sc;			/* The control struct for subsystem */
    volatile TCDS_ASIC *tcds_addr;
    U32 cir_c;				/* copy of the contents of the CIR */
    U32 port;				/* which scsi it was? */
    U32 resetval, err_resetval;		/* What was/will-be in CIR */
    char *pa;				/* physical address */
    unsigned map_offset, offset;
    tcmap_t *mapaddr;
    MEMERR_LOG ml;			/* incase of a memory error report */
    struct tc_memerr_status status;	/* memory error status struct */
    volatile TCDS_DMA_COMMON *tcds;
    int asc_controller;			/* which ASC owns the interrupt */
    U32 bit_in_imer;			/* the ports bit in the imer reg */
    U32 tcds_inter_loop=0;		/* interrupt loop counter */
    SIM_MODULE(tcdsintr);

    PRINTD( controller, NOBTL, NOBTL, CAMD_INOUT,
       ("[%d/t/l] (tcdsintr) Entry: slot %d\n", controller));

    if ( (tcds_addr = tcds_bus_table[controller].addr) == 0 ) {
	CAM_ERROR(module, "Spurious interrupt - no tcds_addr.",
		  CAM_ERR_LOW, NULL, NULL, NULL);
	return;
    }

    do {

	cir_c = tcds_addr->dma_cir;

	/*
	 * These bits are read and write with 0 to clear.  Therefore, to keep
	 * any other bits set until we can service them, we must write back
	 * a 1 until we have serviced it.  The low 16 bits are fully read/write.
	 */
	resetval = 0xffff0000 | cir_c;	/* preserve control bit settings */

	    /* figure out which half of the TCDS got us here */
	if ( cir_c & (SCSI_0_ERRMSK | SCSI0_C94)) {
	    port = 0;
	    bit_in_imer = SCSI0_C94_IE;
	} else
	if ( cir_c & (SCSI_1_ERRMSK | SCSI1_C94)) {
	    port = 1;
	    bit_in_imer = SCSI1_C94_IE;
	} else {
	    CAM_ERROR(module, "Spurious interrupt - no port.",
		  CAM_ERR_LOW, NULL, NULL, NULL);
	    return;
	}

	/* now make sure it's OK to use that part of the tcds */
	if ( tcds_bus_table[controller].asc_alive[port] == -1 ) {
	    CAM_ERROR( module, "ASIC - disabled port interrupt.",
		(CAM_ERR_HIGH), NULL, (DME_DESCRIPTOR *)NULL, NULL );

	    /*
	     * Now, I don't how it got turned on, but try to turn off
	     * the port that interrupted
	     */
	    tcds_addr->dma_imer &= ~( port ? (SCSI1_ERRMSK_INT | SCSI1_INT) :
		(SCSI0_ERRMSK_INT | SCSI0_INT) );
	    tcds_addr->dma_cir = resetval & ~( port ?
		(SCSI1_C94ENB | CIR_DMA1ENB) : (SCSI0_C94ENB | CIR_DMA0ENB) );
	}

	asc_controller = tcds_bus_table[controller].bus_asc_no[port];

	sc = (SIM_SOFTC *)SC_GET_SOFTC( asc_controller );

	/*
	 * Check the error bits first. Report the error and take what
	 * corrections may be necessary.
	 */
	if (cir_c & SCSI_ERRMASK) {
	    err_resetval = resetval;
	    /*
	     * Something bad happened we can reset things to recover.
	     * First, reset the DMA engine and the C94/C96 chip.  Then,
	     * reset the DMA address pointer so the DMA engine thinks
	     * that its buffers are empty.  All this reseting is overkill
	     * but its the easiest way to retry the operation.
	     */
	    tcds_addr->dma_cir = resetval & ~(port ?
		(SCSI1_C94ENB | CIR_DMA1ENB) : (SCSI0_C94ENB | CIR_DMA0ENB) );
	    WBFLUSH();
	    DELAY( 1 );            /* 500nsec minimum, so 1usec should do */

	    *TCDS_DMA_ADDR_PTR(sc) = 0;

	    if (cir_c & (SCSI0_EDMA | SCSI1_EDMA)) {	/* memory read error */
		/* JAG -mapping/non-mapping issue */
		ml.controller = asc_controller;
		ml.pa = asic_pointer_fetch( sc );	/* get the phy addr */

		CAM_ERROR( module, "ASIC DMA Memory read error\n",
		    (CAM_ERR_HIGH), (DME_ERR_MEMERR),
		    (DME_DESCRIPTOR *)NULL, &ml );

		/* JAG -notyet w/mapping support 
		if (build_frag_table == build_flam_map_frag_table) {
		    map_offset = (unsigned long)pa >> PGSHIFT - 2;
		    offset = (unsigned long)pa & PGOFSET;
		    mapaddr = (tcmap_t *)(TC_MAP_PHYSADDR + map_offset);
		    pa = (char *)((*mapaddr | offset) & 0x1fffff);
		}
		*/
		status.pa = (caddr_t)ml.pa;
		status.va = 0;
		status.log = TC_LOG_MEMERR;
		status.blocksize = 4;
		tc_isolate_memerr(&status);
		err_resetval &= ~(port ? SCSI1_EDMA : SCSI0_EDMA);
	    }

	    if (cir_c & (SCSI0_PAR | SCSI1_PAR)) {
		CAM_ERROR(module, "TCDS DB bus parity error",
		    CAM_ERR_HIGH, NULL,
		    NULL, NULL);
		err_resetval &= ~(port ? SCSI1_PAR : SCSI0_PAR);
	    }

	    if (cir_c & (SCSI0_DMAPAR | SCSI1_DMAPAR)) {
		CAM_ERROR(module, "TCDS DMA buffer parity error",
		    CAM_ERR_HIGH, NULL,
		    NULL, NULL);
		err_resetval &= ~(port ? SCSI1_DMAPAR : SCSI0_DMAPAR);
	    }

	    if (cir_c & (SCSI0_TCPAR | SCSI1_TCPAR)) {
		CAM_ERROR(module, "TC DMA read data parity error",
		    CAM_ERR_HIGH, NULL,
		    NULL, NULL);
		err_resetval &= ~(port ? SCSI1_TCPAR : SCSI0_TCPAR);
	    }

	    if (cir_c & SCSITCWDPAR) {
		CAM_ERROR(module, "TC IO write data parity error",
		    CAM_ERR_HIGH, NULL,
		    NULL, NULL);
		err_resetval &= ~SCSITCWDPAR;
	    }

	    if (cir_c & SCSITCADPAR) {
		CAM_ERROR(module, "TC IO address parity error",
		    CAM_ERR_HIGH, NULL,
		    NULL, NULL);
		err_resetval &= ~SCSITCADPAR;
	    }

	    /*
	     * Now that the error has been reported, and any specific
	     * actions have been taken, we can do the generic handling.
	     * Clear out the error bits, and get the C94 going again.
	     */

	    tcds_addr->dma_cir = err_resetval;
	    WBFLUSH();			/* now turn it back on */

	    (sc->hba_chip_reset)(sc);	/* set it up again */

	    /*
	     * A real nasty happened, so start it over from scratch.
	     * Again, this is overkill, but it's the easiest way to
	     * retry the operation for now.
	     */
	    SC_HBA_BUS_RESET(sc);
	}

	if (cir_c & (SCSI0_PREF | SCSI1_PREF))
	{
            printf("tcds%d: Unexpected prefetch interrupt, channel %d\n",
		controller, port);
	    resetval &= ~(port ? SCSI1_PREF : SCSI0_PREF);
	}

	if (cir_c & (SCSI0_C94 | SCSI1_C94)) {

	    /* Disable the interrupt enable in the IO ASIC from the C94 chip
	     * (because the C94 intr line is still set).  If we don't clear
	     * it, then we'll get another interrupt (after ascintr has cleared
	     * the C94 intr condition) and it will be spurious.
	     */
	    tcds_addr->dma_imer &= ~bit_in_imer;
	    WBFLUSH();
	    /* Now, clear this interrupt from the IO ASIC. Note, you can't
	     * do this after the call to ascintr, because that call may have
	     * done something to generate a new interrupt, and if we clear
	     * it (after the ascintr call) then we'll loose that new interrupt.
	     */
	    resetval &= ~(port ? SCSI1_C94 : SCSI0_C94);
	    tcds_addr->dma_cir = resetval;
	    WBFLUSH();
	    /* Now go and process this interrupt */
	    ascintr( asc_controller );
	    /* Now we can turn interrupt enable back on.  This allows us to
	     * now get any new interrupts that have happened because of stuff
	     * that the ascintr routine did.
	     */
	    tcds_addr->dma_imer |= bit_in_imer; 
	    WBFLUSH();
	} else {
	    tcds_addr->dma_cir = resetval;
	    WBFLUSH();
	}
	tcds_inter_loop++;
    } while ( (tcds_inter_loop < tcds_inter_loop_limit) &&
		(tcds_addr->dma_cir &
		(SCSI_0_ERRMSK | SCSI0_C94 | SCSI_1_ERRMSK | SCSI1_C94)));
}

/* ---------------------------------------------------------------------- */
/* wake up the TCDS ASIC */

tcds_enable( ctlr )
struct controller *ctlr;
{
    int tcds_num=ctlr->bus_num;
    U32 i = 0;
    U32 j = 0;		/* start with 0 for cir and imer registers */
    TCDS_ASIC *tcds;
    extern int cpu;

    tcds = tcds_bus_table[tcds_num].addr;
    if (tcds_bus_table[tcds_num].asc_alive[0] != -1) {
	i |= SCSI0_C94ENB;
	j |= (SCSI0_ERRMSK_INT | SCSI0_INT );
    }
    if (tcds_bus_table[tcds_num].asc_alive[1] != -1) {
	i |= SCSI1_C94ENB;
	j |= (SCSI1_ERRMSK_INT | SCSI1_INT );
    }
    if (tc_option_control(ctlr, SLOT_PARITY) == SLOT_PARITY) {
	j |= SCSITC_ERRMSK_INT;
    }
    tcds->dma_cir = i;	/* Enable the DMA on the correct channels */
    WBFLUSH();
    tcds->dma_imer = j;	/* Get the correct interrupt lines on */
    WBFLUSH();
}

/* ---------------------------------------------------------------------- */
/* Get the appropriate clock rate for this puppy.  This will return the real
 * rate that the crystal is running at no matter what the "fast" setting.
 */

U32
tcds_get_clock( sc )
    SIM_SOFTC *sc;
{
    TCDS_ASIC *tcds;
    int controller;

    /*
     * We could ask the system the speed of the turbo channel
     * (via get_info()) but since we know that the tcds has its
     * own crystal we can't ask.  The crystal may be different
     * than the turbo channel bus speed.
     */

    controller = ((struct controller *)sc->um_probe)->bus_num;
    tcds = tcds_bus_table[controller].addr;
    if ( tcds->dma_cir & INP2_FAST ) {
	/* If the bit is set, the H/W says we have a slow crystal (25Mhz) */
	return( SIM_TCDS_CLK_SPEED );
    } else {
	/* If the bit is clear, we have a fast crystal (40Mhz) */
	return( SIM_FAST_CLK_SPEED );
    }
}

/* ---------------------------------------------------------------------- */
/* Get the appropriate period for this puppy.  This will return the minimum
 * period we should run at dependent on the state of the "fast" setting.
 */

U32
tcds_get_period( sc )
    SIM_SOFTC *sc;
{
    TCDS_ASIC *tcds;
    int controller;

    controller = ((struct controller *)sc->um_probe)->bus_num;
    tcds = tcds_bus_table[controller].addr;
    if ( tcds->dma_cir & INP2_FAST ) {
	/* The bit is set, so it is a slow crystal */
	return( SIM94_PERIOD_MIN );
    } else {
	/* The bit is clear, so the H/W has a fast crystal */
	if (sim94_fast_enable && tcds_get_fast( sc )) {
	    /* The user wants fast so use the fast minimum period */
	    return( SIMFAST_PERIOD_MIN );
	} else {
	    /* The user wants slow so use the minimum period for slow */
	    return( SIMFAST_MIN_FOR_SLOW );
	}
    }
}

/* ---------------------------------------------------------------------- */
#define Cprintf

int
tscsiconf( busp, port, tcds )
    struct bus *busp;
    U32 port;
    TCDS_ASIC *tcds;
{
    register struct controller *ctlr;
    register volatile SIM94_REG *reg;

    Cprintf ("tscsiconf: bus = %s%d\n",
		busp->bus_name, busp->bus_num);

    /* Now, turn off the reset line for this C94,
     * or, turn on the don't reset bit for this C94,
     * it just depends on how you want to look at it.
     */
    tcds->dma_cir |= ( port ? SCSI1_C94ENB : SCSI0_C94ENB );
    WBFLUSH();

    /*
     * Check for the existence of an actual chip
     * on the other side of the TSCSI
     *
     * This is done by writing two different values
     * to the 94 chip.   If the values are still present
     * when they are read, then the chip must be there.
     */
    reg = (SIM94_REG *)(port ? &tcds->c941_reg : &tcds->c940_reg);

    reg->sim94_cnf2 = 0x8a;
    reg->sim94_cnf3 = 0x05;
    WBFLUSH();
    if (((reg->sim94_cnf2 & 0xff) != 0x8a) ||
		((reg->sim94_cnf3 & 0xff) != 0x05)) {
	/* if the thing is broken or not there, exit */
	return(-1);
    }

    reg->sim94_cnf2 = 0;
    reg->sim94_cnf3 = 0;
    WBFLUSH();			/* make sure they get put back */

    Cprintf("tscsiconf: Calling get_ctlr %d\n", port);

    /* Get the controller structure.  Check in descending levels of
     * of specificity (nice word eh).
     */
    if( (ctlr = get_ctlr("asc", 		/* "asc" */
    		     port,			/* slot on tcds */
    		     busp->bus_name,		/* "tcds" */
    		     busp->bus_num)) ||		/* which tcds */
        (ctlr = get_ctlr("asc",
    		     -1,			/* wild slot on tcds */
    		     busp->bus_name,
    		     busp->bus_num)) ||
        (ctlr = get_ctlr("asc",
    		     port,
    		     busp->bus_name,
    		     -1)) ||			/* wild tcds number */
        (ctlr = get_ctlr("asc",
    		     -1,			/* both wild slot on tcds */
    		     busp->bus_name,
    		     -1)) ||			/* and which tcds */
        (ctlr = get_ctlr("asc",
    		     -1,
    		     "*",			/* finally take ANY "asc" */
    		     -99))) {

	Cprintf("tscsiconf: Found controller: %x\n", ctlr);

	ctlr->slot = port;
	ctlr->bus_name = busp->bus_name;
	ctlr->bus_num = busp->bus_num;
	ctlr->tcindx = busp->tcindx;

	/* The following tcds_bus_table structure values are
	 * setup here (rather than in tcdsconfl1) because the
	 * needed values are available here and not up in the
	 * tcdsconfl1 routine.
	 */
	tcds_bus_table[ctlr->bus_num].bus_asc_no[port]=ctlr->ctlr_num;
	/* mark it alive now so we can try to do the probe */
	tcds_bus_table[ctlr->bus_num].asc_alive[port]=0;
	if ((tcds_bus_table[ctlr->bus_num].addr) &&
			(tcds_bus_table[ctlr->bus_num].addr != tcds)) {
		printf("tcds%d port address mismatch - port %d disabled\n",
			ctlr->bus_num, port);
		return(-1);
	}
	tcds_bus_table[ctlr->bus_num].addr = tcds;
	tcds_enable(ctlr);

	Cprintf("tscsiconf: Calling config_cont\n");

	if (!(tc_config_cont(reg, tcds, port, ctlr->ctlr_name, ctlr))) {
	    printf("%s%d not probed\n", ctlr->ctlr_name, ctlr->ctlr_num);
	    return(-1);		/* If it fails, just bail out */
	} else {
	    Cprintf("tscsiconf: Calling conn_ctlr\n");
	    conn_ctlr(busp, ctlr);
	    /*
	     * Now set the flag to tell DEVIOCGET to use the parent
	     * bus structure for the slot number rather than the
	     * controller structure.
	     */
	    ctlr->boot_slot = (caddr_t)-1;
        }
    } else {
	return(-1);
    }
return( CAM_SUCCESS );
}

#ifdef __alpha

/* On Flamingo, there are only two host id's maintained by the console.
 * For the internal adaptor, the "A" channels get set to the value of
 * "SCSI_A", and the "B" channels get set to the value of "SCSI_B".
 * For dual scsi cards (PMAZB-AA or PMAZC-AA) each card holds the scsi
 * id values in their own EEPROM.  So, we examine the appropriate address
 * in the option card to determine the scsi ids for that card.  Any
 * PMAZ-AA cards will get the id from the console SCSI_A value.
 *
 * The pointer returned by prom_getenv points to the id for the "A"
 * channel (SCSI_A).  The next byte contains the id for SCSI_B.
 * The "rambuf" member of the softc structure is set to zero if
 * we're talking about the A channel, and is set to one for the
 * B channel (see the DME code).
 */
int
sim_kn15_get_scsiid( sc )
SIM_SOFTC *sc;
{
    int	id;

    SIM_MODULE(sim_kn15_get_scsiid);

    id = get_idbyte( sc );
    id &= 0x7;			/* the id is the low 3 bits */

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("end\n"));

    return( id );
}

/* Get just the fast bit from the byte that contains it.  For the 94
 * family the low 3 bits are the id, and the 4th bit is the fast flag.
 * If bit 4 is set, then run fast.  If not, then run slow.
 */
static int
tcds_get_fast( sc )
SIM_SOFTC *sc;
{
    int	fast;

    fast = get_idbyte( sc );

    if (fast & 0x8) {
	return( CAM_TRUE );
    } else {
	return( CAM_FALSE );
    }
}

/* Read the 4 bits that contain the scsiid for the requested channel.
 * These bits come from different places for baseboard and tc scsi options.
 */
static int
get_idbyte( sc )
SIM_SOFTC *sc;
{
    char *sid;
    int index, id, fast;
    extern int default_scsiid;
    extern char *prom_getenv();

    SIM_MODULE(get_idbyte);

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("begin\n"));

    index = (int)((struct controller *)sc->um_probe)->tcindx;
    id = default_scsiid;

    if (strncmp(tc_slot[index].modulename, "PMAZ-", 5) == 0) {
	if (!(sid = prom_getenv("scsiid"))) {
	    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
		("can't get initiator ID from environment\n"));
	} else {
	    if (sc->rambuf) {
		id = *++sid;
	    } else {
		id = *sid;
	    }
	}
	/* now check to see if the id we got is valid */
	if (id < 0 || id > 7) {
	    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
		("illegal initiator ID: %d\n", id));
	    id = default_scsiid;
	    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
		("new initiator ID == %d\n", id));
	}
	if (!(sid = prom_getenv("scsifast"))) {
	    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
		("can't get fast flag from environment\n"));
	    fast = CAM_TRUE;	/* fake fast flag for now */
	} else {
	    if (sc->rambuf) {
		fast = *++sid;
	    } else {
		fast = *sid;
	    }
	}
	if (fast) {
	    id |= 0x8;		/* emulate the PMAZC fast flag */
	}
    } else {
	id = ((TCDS_ASIC *)sc->csr_probe)->host_ids;
	/* A is the high nibble, B is the low nibble. */
	if (!(sc->rambuf)) id >>= 4;
    }
    id &= 0xf;			/* 4 bits per channel */

    return( id );
}

#endif
