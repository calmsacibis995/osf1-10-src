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
static char *rcsid = "@(#)$RCSfile: sim_94_fast.c,v $ $Revision: 1.1.4.6 $ (DEC) $Date: 1993/11/23 21:51:42 $";
#endif

/* ---------------------------------------------------------------------- */

/* sim_94_fast.c	Version 1.00			Apr. 02, 1993 

	This file contains C94 Fast specific SIM functions.
	These functions will be called by sim_94.c.

Modification History:

	.00	03/18/93	Fred	Created this file.
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
 
/* ---------------------------------------------------------------------- */
/* External declarations.
 */
extern void sim_kn02ba_chip_reset();


/* ---------------------------------------------------------------------- */
/*
 * Routine Name : sim_fast_chip_reset
 *
 * Functional Description :
 *	Sim_fast_chip_reset will reset the CF94/96 chip.  It will
 *	then setup the select/reselect values, the clock conversion
 *	factor, the host SCSI id, the configuration registers, the
 *	sync offset, and the sync period.  This is done by calling
 *	the standard 94 chip setup code, then doing the special setup
 *	that the fast chip needs.
 *
 * Call Syntax :
 *	sim_fast_chip_reset(sc)
 *
 * Arguments:
 *	SIM_SOFTC *sc		-- SIM_SOFTC pointer
 *
 * Return Value :  None
 */
void
sim_fast_chip_reset(sc)
SIM_SOFTC *sc;
{
    register volatile SIM94_REG *reg = (SIM94_REG *)sc->reg;
    register SIM94_SOFTC *ssc = (SIM94_SOFTC *)sc->hba_sc;
    SIM_MODULE(sim_fast_chip_reset);

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("begin\n"));

    /*
     * This function does NOT perform a SCSI bus reset.
     */

    /*
     * Reset the NCR 53CF94/96 chip.
     */
    sim_kn02ba_chip_reset( sc );

    /*
     * Reset the ccf to 0 (since the clock is always 40MHz).
     */
    reg->sim94_ccf = 0x0;
    WBFLUSH();

    /*
     * Initialize other NCR53CF96 registers.
     */
    reg->sim94_cnf3 = SIMFAST_CNF3_FCLK;	/* fast clock, slow bus */
    WBFLUSH();
    reg->sim94_cnf4 = SIMFAST_CNF4_EAN;		/* enable active negation */
    WBFLUSH();

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("end\n"));
}

U32
simfast_attach(sc)
SIM_SOFTC *sc;
    {
    extern U32 sim94_init(), sim94_go(), sim94_sm(), sim94_bus_reset(),
                  sim94_send_msg(), sim94_xfer_info(), sim94_sel_stop(),
                  sim94_req_msgout(), sim94_msg_accept(), simfast_setup_sync(),
                  sim94_discard_data(), sim94_nop(), sim94_clr_atn(),
		  sim94_targ_cmd_cmplt(), sim94_targ_recv_cmd(),
		  sim94_targ_send_msg(), sim94_targ_disconnect(),
		  sim94_targ_recv_msg();
    SIM_MODULE(simfast_attach);

    sc->hba_init = sim94_init;
    sc->hba_go = sim94_go;
    sc->hba_sm = sim94_sm;
    sc->hba_bus_reset = sim94_bus_reset;
    sc->hba_send_msg = sim94_send_msg;
    sc->hba_xfer_info = sim94_xfer_info;
    sc->hba_sel_msgout = sim94_sel_stop;
    sc->hba_msgout_pend = sim94_req_msgout;
    sc->hba_msgout_clear = sim94_clr_atn;
    sc->hba_msg_accept = sim94_msg_accept;
    sc->hba_setup_sync = simfast_setup_sync;
    sc->hba_discard_data = sim94_discard_data;

    sc->hba_targ_cmd_cmplt = sim94_targ_cmd_cmplt;
    sc->hba_targ_recv_cmd = sim94_targ_recv_cmd;
    sc->hba_targ_send_msg = sim94_targ_send_msg;
    sc->hba_targ_disconnect = sim94_targ_disconnect;
    sc->hba_targ_recv_msg = sim94_targ_recv_msg;

    return(CAM_REQ_CMP);
}

/*
 * Routine Name : simfast_setup_sync
 *
 * Functional Description :
 *      Simfast_setup_sync will set the sync offset and period
 *      of the NCR53CF96 to the values stored in the IT_NEXUS
 *      "sync_period" and "sync_offset" fields.
 *
 * Call Syntax :
 *      simfast_setup_sync(sws)
 *
 * Arguments:
 *      SIM_WS *sws;
 *
 * Return Value :  CAM_TRUE
 */
U32
simfast_setup_sync(sws)
register SIM_WS *sws;
{
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    register SIM94_SOFTC *ssc = (SIM94_SOFTC *)sc->hba_sc;
    volatile SIM94_REG *reg = SIM94_GET_CSR(sc);
    int temp, s;
    SIM_MODULE(simfast_setup_sync);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
               ("begin, period 0x%x, offset 0x%x\n",
                sws->it_nexus->sync_period, sws->it_nexus->sync_offset ));

    SIM_REG_LOCK(s, sc);

    SC_ADD_FUNC(sws, module);

    /*
     * Set the NCR53CF96's synchronous offset and period.
     * Use "sync_offset" and "sync_period" from the IT_NEXUS.
     *
     * The period in the IT_NEXUS must be multiplied by four
     * to determine the value in nanoseconds.  Convert this value
     * to clocks per byte by dividing by the clock speed of the chip.
     * Convert the clock speed from MHz to nanoseconds by dividing
     * 10000 by the clock speed (the clock speed was stored as
     * MHz * 10).  Finally call SIM94_PERIOD_CONVERT() to come up with
     * the correct value to load into the chip.
     */
    temp = SIMFAST_PERIOD_CONVERT(
	((U32)sws->it_nexus->sync_period * 4L) / (10000L / (U32)ssc->clock),
	((U32)sws->it_nexus->sync_period * 4L));

    /* Reference NCR notice #844 Unique Features of the 53C94/96-2
     * Rev. 2.0, 5/93.  For sync periods of 5, 6, or 7, the chip
     * uses 1 less clock cycle than we ask for, so we must compensate
     * for the chip.  So, for 5, 6, or 7, add 1 to the number of clocks
     * per byte.  This also means that the chip can NOT do 7 clock
     * cycles per byte.  This clock rate tweeking is also only necessary
     * for DATAO operations (writes).
     */
    if ( sws->cam_flags & CAM_DIR_OUT ) {
	if ( temp > 4 && temp < 8 ) {
            temp += 1;
	}
    }

    /* If the synchronous transfer period is less than 200ns per byte,
     * then we must set fast scsi (0x32*4 = 200ns).  If not, then
     * its just a regular (with a fast clock).
     *
     * Remember that the sync_period is stored just as the device
     * negotiated it with us (#ns per byte / 4).  Therefore, 0x32 = 200ns.
     */
    reg->sim94_cnf3 = ((U32)sws->it_nexus->sync_period < 0x32 ?
	(SIMFAST_CNF3_FCLK | SIMFAST_CNF3_FSCSI) : (SIMFAST_CNF3_FCLK));
    WBFLUSH();

    reg->sim94_sp = temp;
    WBFLUSH();

    reg->sim94_so = sws->it_nexus->sync_offset;
    WBFLUSH();

    SIM_REG_UNLOCK(s, sc);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
               ("end\n"));

    return(CAM_TRUE);
}

