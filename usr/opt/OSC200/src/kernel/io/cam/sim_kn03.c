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
static char *rcsid = "@(#)$RCSfile: sim_kn03.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/06/25 17:51:18 $";
#endif

/* ---------------------------------------------------------------------- */

/* sim_kn03.c		Version 1.01			91/10/04 

	This file contains 3MAX+ specific SIM functions.
	These functions will be called by sim_94.c.

Modification History:

	Version	Date		Who	Reason
	1.00	10/04/91	rps	Created this file.
*/

/* ---------------------------------------------------------------------- */
/* Include files.
 */
#include <io/common/iotypes.h>
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
#define SIM_KN03_SYNC_OFFSET	0x0f
#define SIM_KN03_SYNC_PERIOD	0x46
#define SIM_KN03_CLK_CONVERSION	0x05	/* 25 MHz */
    
/* ---------------------------------------------------------------------- */
/* External declarations.
 */

/*
 * Routine Name : sim_kn03_chip_reset
 *
 * Functional Description :
 *	Sim_kn03_chip_reset will reset the NCR53C94.  It will
 *	then setup the select/reselect values, the clock conversion
 *	factor, the host SCSI id, the configuration registers, the
 *	sync offset, and the sync period.
 *
 * Call Syntax :
 *	sim_kn03_chip_reset(sc)
 *
 * Arguments:
 *	SIM_SOFTC *sc		-- SIM_SOFTC pointer
 *
 * Return Value :  None
 */
void
sim_kn03_chip_reset(sc)
SIM_SOFTC *sc;
{
    SIM94_REG *reg = (SIM94_REG *)sc->reg;

    PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("(sim_kn03_chip_reset) begin\n"));

    sim_kn02_chip_reset( sc );
}
