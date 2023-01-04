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
static char *rcsid = "@(#)$RCSfile: sim_kn01.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/09/21 21:53:09 $";
#endif

/* ---------------------------------------------------------------------- */

/* sim_kn01.c		Version 1.03			Oct. 22, 1991 

	This file contains DS3100 specific SIM functions.
	These functions will be called by sim_sii.c.

Modification History:

	Version	Date		Who	Reason
	1.00	4/19/91		rln	Created this file.
	1.01	8/08/91		rln	Add in synchronous support
	1.02	09/09/91	rps	Changed reference from kn02 to kn01.
	1.03	10/22/91	janet
	o Include "scsi_all.h" and "sim_target.h"
	o Added SIM_MODULE() to every function.
	o Replaced all PRINTD's with SIM_PRINTD's


*/

/* ---------------------------------------------------------------------- */
/* Include files.
 */
#include <io/common/iotypes.h>
#include <io/common/devdriver.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>
#include <io/cam/dec_cam.h>
#include <mach/vm_param.h>
#include <io/cam/cam.h>
#include <io/cam/scsi_all.h>
#include <io/cam/cam_debug.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/dme.h>
#include <io/cam/sim.h>
#include <io/cam/sim_common.h>
#include <io/cam/sim_sii.h>

/* ---------------------------------------------------------------------- */
/* Local defines.
 */

    
/* ---------------------------------------------------------------------- */
/* External declarations.
 */

/*
 * Routine Name : sim_kn01_chip_reset
 *
 * Functional Description :
 *	Sim_kn01_chip_reset will reset the DEC SII. 
 *
 * Call Syntax :
 *	sim_kn01_chip_reset(sc)
 *
 * Arguments:
 *	SIM_SOFTC *sc		-- SIM_SOFTC pointer
 *
 * Return Value :  None
 */
void
sim_kn01_chip_reset(sc)
SIM_SOFTC *sc;
{
    int s;
    SIMSII_REG *siireg = (SIMSII_REG *)sc->reg;
    SIM_MODULE(sim_kn01_chip_reset);

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("begin\n"));

    /*
     * This function does NOT perform a SCSI bus reset.
     */
    

    /*
     * Reset the DEC SII SCSI chip.
     */
    SC_LOCK(s, sc);
    CMD_PENDING(sc->hba_sc,SII_COMM_CHIP_RST,siireg);

    /*
     * Set the host's SCSI ID.
     */
    siireg->id = SII_ID_IO | sc->scsiid;

    /*
     * Enable interrupts from the SII.
     */
    siireg->csr |= SII_CSR_IE;
    WBFLUSH();

    sc->sync_offset = SIM_KN01_SYNC_OFFSET;
    sc->sync_period = SIM_KN01_SYNC_PERIOD;
  
    SC_UNLOCK(s, sc);
    DELAY(25);

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("end\n"));
}
