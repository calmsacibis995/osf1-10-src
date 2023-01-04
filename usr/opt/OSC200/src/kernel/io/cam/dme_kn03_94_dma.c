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
static char *rcsid = "@(#)$RCSfile: dme_kn03_94_dma.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/06/25 17:48:29 $";
#endif

/**
 * FACILITY:
 * 
 *      ULTRIX SCSI CAM SUBSYSTEM
 *
 * ABSTRACT:
 *
 * The Data Mover Engine (DME) component is an evolution of the
 * PDMA method of data transfer used in the existing ULTRIX SCSI support.
 * PDMA's intent was to hide the details of data movement in different
 * HBA's, behind a well defined sequence of higher level functions. The
 * extensions to PDMA which will be discussed in this document, address
 * the CAM's requirements for the support of LUN's, queued commands and
 * the use of mapping in next generation SCSI HBA's (such as the XZA).
 *
 * The DME model has broken down data transfer operations into
 * 8 distinct steps or primitives: 
 * 
 * dme_init - Initialize DME subsystem, resource queues and control 
 * structures.
 * dme_setup - Allocate required DMA resources and preload buffer on write.
 * dme_start - Transfer setup information from DME segment structures to the 
 *             HBA and start DMA activity. Points to dme_resume.
 * dme_pause - Instructs the DME to pause the DMA to be resumed later.  
 * dme_save - Instructs the DME to save the current pointers
 * dme_restore - Instructs the DME to restore the current pointers
 * dme_resume - Continue (restart) DMA after terminal count or PAUSE and fill 
 *            or clear next buffer.  
 * dme_end - Complete the DMA transaction, release any buffers, move residual
 *           data to user space, and return actual bytes sent or received.
 * 
 * There are several other utility functions supported by DME.
 * 
 * dme_copyin -Moves bytes out of the intermediate buffer to their
 * destination.
 * dme_copyout - Moves bytes into the intermediate buffer.
 * dme_clear - Utility routine used to zero buffers for security reasons.
 * dme_modify - Instructs the DME to modify the current data pointers.
 *
 *
 * AUTHOR:
 *
 *      Robert P. Scott         6-Mar-1991
 *              Based upon original 3MAX implementation by Rich Napolitano
 *
 * MODIFIED BY:
 * 
 *
 **/


/* ---------------------------------------------------------------------- 
 *
 * Include files.
 *
 */
#include <io/common/iotypes.h>
#include <sys/types.h>
#include <hal/cpuconf.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <kern/lock.h>
#include <io/cam/dec_cam.h>
#include <mach/vm_param.h>
#include <io/cam/cam.h>
#include <io/cam/scsi_all.h>
#include <machine/pmap.h>
#include <io/cam/cam_debug.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/dme.h>
#include <io/cam/scsi_status.h>
#include <io/cam/sim.h>
#include <io/cam/sim_94.h>
#include <io/cam/sim_common.h>
#include <io/cam/dme_3min_94_dma.h>

/* ---------------------------------------------------------------------- 
 *
 * Local defines:
 * 
 */
#define SIM_KN03_CLK_SPEED	24	/* 24 MHz */

/* ---------------------------------------------------------------------- 
 *
 * External declarations:
 *
 */

/* Change this later should be int not u_long */
extern U32 bcopy();                  /* System routine to copy buffer  */
extern U32 bzero();                  /* System routine to clear buffer */

/* ---------------------------------------------------------------------- 
 *
 * Function Prototypes:
 */

U32 dme_copyin();
U32 dme_copyout();
U32 dme_3min_init();
U32 dme_3min_setup();
U32 dme_3min_end();
U32 dme_3min_pause();
U32 dme_3min_resume();
U32 dme_3min_save();
U32 dme_3min_restore();
U32 dme_3min_modify();
U32 dme_3min_clear();
U32 dme_3min_bump_sglist();
U32 dme_3min_interrupt();


/* ----------------------------------------------------------------------
 * 
 * Initialized and uninitialized data: 
 * 
 */


/*---------------------------------------------------------------------- 
 *
 * Local Type Definitions and Constants
 *
 */ 

U32
dma94_kn03_dme_attach( SIM_SOFTC *sc )
{
    register volatile SIM94_REG *reg = (SIM94_REG *)sc->reg;
    register SIM94_SOFTC *ssc = (SIM94_SOFTC *)sc->hba_sc;

    DME_STRUCT *dme;

    PRINTD( NOBTL, NOBTL, NOBTL, CAMD_DMA_FLOW,
           ("[b/t/l] (dme_kn03_attach) entry - sc=0x%x\n", sc ));

    /*
     * Store the clock speed.  To be used when setting up the
     * synchronous period.  Store the clock * 10 to allow for
     * fractions.
     */
    ssc->clock = SIM_KN03_CLK_SPEED * 10;

    /*
     * Set the clock conversion register for 3Max+'s speed.
     */
    reg->sim94_ccf = SIM94_CONVERT_CLOCK(SIM_KN03_CLK_SPEED);

    /*
     * Calculate the minimum period.  The value which is used to
     * negotiate synchronous is the actual period divided by four.
     * Convert the clock speed from MHz to nanoseconds by dividing
     * 1000 by the clock speed.
     */
    sc->sync_period = ((1000 / SIM_KN03_CLK_SPEED) * SIM94_PERIOD_MIN) / 4;

    dme = (DME_STRUCT *)sc_alloc(sizeof(DME_STRUCT));

    if ( !dme )
    {
	printf(
	   "[b/t/l] dme_kn03_attach: Unable to alloc memory for dme struct\n" );
	return CAM_REQ_CMP_ERR;
    }

    sc->dme = dme;

    /*
     * Calculate the base address for the IOASIC.  To get the base address
     * the BASE_IOASIC_OFFSET has to be *SUBTRACTED* from the base address
     * of the 94 chip.  The 94 chip is stored in the SIM_SOFTC stucture.
     */

    dme->hardware = (void *)((unsigned int)sc->reg - BASE_IOASIC_OFFSET);

    dme->vector.dme_init = dme_3min_init;
    dme->vector.dme_setup = dme_3min_setup;
    dme->vector.dme_start = dme_3min_resume;
    dme->vector.dme_end = dme_3min_end;
    dme->vector.dme_pause = dme_3min_pause;
    dme->vector.dme_resume = dme_3min_resume;
    dme->vector.dme_save = dme_3min_save;
    dme->vector.dme_restore = dme_3min_restore;
    dme->vector.dme_modify = dme_3min_modify;
    dme->vector.dme_copyin = bcopy;
    dme->vector.dme_copyout = bcopy;
    dme->vector.dme_clear = bzero;
    dme->vector.dme_interrupt = dme_3min_interrupt;

    if ( dme_3min_init( sc ) == CAM_REQ_CMP )
    {
        return CAM_REQ_CMP;
    }
    else
        return CAM_REQ_CMP_ERR;
    
}                  /* dma94_kn03_dme_attach */
