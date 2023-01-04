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
static char *rcsid = "@(#)$RCSfile: sim_kn02.c,v $ $Revision: 1.1.12.3 $ (DEC) $Date: 1993/11/23 21:51:33 $";
#endif

/* ---------------------------------------------------------------------- */

/* sim_kn02.c		Version 1.05			Jan. 15, 1992 

	This file contains 3MAX specific SIM functions.
	These functions will be called by sim_94.c.

Modification History:

        1.05	01/15/92	janet
	If the simd_init field has been set, load the sim94_ccf
        register based on the stored system clock value.

	1.04	11/13/91	janet
	Changed where the clock speed is setup.

	1.03	10/28/91	janet
	Store the clock speed in the SIM94_SOFTC struct and perform
	structure initializations based on this value for both 3MAX
	and 3MIN.

	1.02	10/22/91	janet
	o Added SIM_MODULE() to every function.
	o Made registers volatile.
	o Replaced all PRINTD's with SIM_PRINTD's.

	1.01	03/26/91	janet	Updated after code review

	1.00	11/30/90	janet	Created this file.
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
#include <io/cam/sim_94.h>

/* ---------------------------------------------------------------------- */
/* Local defines.
 */
#define SIM_KN02_SYNC_OFFSET	0x0f
    
/* ---------------------------------------------------------------------- */
/* External declarations.
 */

/*
 * Routine Name : sim_kn02_chip_reset
 *
 * Functional Description :
 *	Sim_kn02_chip_reset will reset the NCR53C94.  It will
 *	then setup the select/reselect values, the clock conversion
 *	factor, the host SCSI id, the configuration registers, the
 *	sync offset, and the sync period.
 *
 * Call Syntax :
 *	sim_kn02_chip_reset(sc)
 *
 * Arguments:
 *	SIM_SOFTC *sc		-- SIM_SOFTC pointer
 *
 * Return Value :  None
 */
void
sim_kn02_chip_reset(sc)
SIM_SOFTC *sc;
{
    register volatile SIM94_REG *reg = (SIM94_REG *)sc->reg;
    register SIM94_SOFTC *ssc = (SIM94_SOFTC *)sc->hba_sc;
    SIM_MODULE(sim_kn02_chip_reset);

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("begin\n"));

    /*
     * This function does NOT perform a SCSI bus reset.
     */

    /*
     * Reset the NCR53C94.
     */
    reg->sim94_cmd = SIM94_CMD_RSTCHIP;
    WBFLUSH();
    DELAY(25);

    reg->sim94_cmd = SIM94_CMD_NOP;
    WBFLUSH();
    DELAY(25);

    /*
     * Set select/reselect timeout to max needed.
     */
    reg->sim94_srto = 0xa0;
    WBFLUSH();

    reg->sim94_so = 0x0;
    WBFLUSH();
    reg->sim94_ffss = 0x0;
    WBFLUSH();

    /*
     * If the "DME" layer has already be initialized, load the
     * sim94_ccf register with the stored system clock value.
     * The "dme_attach" function will load this register.
     */ 
    if (sc->simd_init) {
        reg->sim94_ccf = SIM94_CONVERT_CLOCK((ssc->clock/10));
	WBFLUSH();
    }

    /*
     * Set the SCSI id and indicate the use of parity.
     */
    reg->sim94_cnf1 = SIM94_CNF1_EPC | sc->scsiid;
    WBFLUSH();

    /*
     * Initialize other NCR53C94 registers.
     */
    reg->sim94_cnf2 = 0x0;
    WBFLUSH();
    reg->sim94_cnf3 = 0x0;
    WBFLUSH();

    /*
     * Save maximum offset.
     */
    sc->sync_offset = SIM_KN02_SYNC_OFFSET;

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("end\n"));
}

void sim_kn02ba_chip_reset( SIM_SOFTC *sc )
{
    register volatile SIM94_REG *reg = (SIM94_REG *)sc->reg;

    SIM_MODULE(sim_kn02ba_chip_reset);
    sim_kn02_chip_reset( sc );

    /* These systems/chips can do additional parity checking */

    reg->sim94_cnf2 = SIM94_CNF2_DPE | SIM94_CNF2_RPE;
    WBFLUSH();

}

