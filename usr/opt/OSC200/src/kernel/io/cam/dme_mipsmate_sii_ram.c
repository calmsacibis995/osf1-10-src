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
static char *rcsid = "@(#)$RCSfile: dme_mipsmate_sii_ram.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/06/25 17:48:50 $";
#endif

/* ---------------------------------------------------------------------- */

/* dme_mipsmate_sii_ram.c	Version 1.00			Nov. 1, 1991 

	The source file dme_mipsmate_sii_ram.c contains functions which
	are specific to DME on the Mipsmate.

Modification History:

	1.00	11/01/91	janet
	Create this file.
*/

/* ---------------------------------------------------------------------- 
 *
 * Include files.
 *
 */
#include <io/common/iotypes.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/conf.h>
#include <sys/user.h>
#include <sys/file.h>
#include <sys/map.h>
#include <sys/vm.h>
#include <sys/dk.h>
#include <sys/proc.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include <sys/cmap.h>
#include <sys/uio.h>

#include <io/common/devio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#include <io/cam/dec_cam.h>
#include <io/cam/cam.h>
#include <mach/vm_param.h>
#include <io/cam/scsi_all.h>
#include <io/cam/cam_debug.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/sim_target.h>
#include <io/cam/dme.h>			/* DME specific structs and consts */
#include <io/cam/sim.h>			/* SIM specific structs and consts */
#include <io/cam/scsi_status.h>
#include <io/cam/sim_sii.h>		/* DEC SII specific definitions */
#include <io/cam/sim_common.h>
#include <io/cam/dme_pmax_sii_ram.h>

/* ---------------------------------------------------------------------- 
 *
 * External declarations:
 *
 */
extern U32 ramsii_dme_init();
extern U32 ramsii_dme_setup();
extern U32 ramsii_dme_end();
extern U32 ramsii_dme_pause();
extern U32 ramsii_dme_resume();
extern U32 ramsii_dme_save();
extern U32 ramsii_dme_restore();
extern U32 ramsii_dme_modify();
extern U32 kn230_bzero();
extern U32 kn230_wbcopy();
extern U32 kn230_rbcopy();

/* ---------------------------------------------------------------------- 
 *
 * Function Prototypes:
 */
U32 ds5100_dme_attach();
U32 ds5100_dme_unload();
U32 cam_wmbcopy_kn230();
U32 cam_rmbcopy_kn230();
U32 cam_wmbzero_kn230();

/*
 * Transform the passed address from a monotonically increasing address space
 * to whatever implementation specific addressing scheme is used for this 
 * DMA RAM buffer. This macro will need to be implementation specific.
 *
 * For PMAX subtract off the base address of the RAM buffer shift left (*2)
 * one position and then add back in base address.
 */
#define DME_MOD_ADDRESS(addr,softc) (u_short*) (((U32) (addr) - \
		       (U32)SIMSII_GET_RAM_BUF(sim_softc)\
     		       <<1) + (U32)SIMSII_GET_RAM_BUF(sim_softc))

/**
 * ds5100_dme_attach -
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * DME attach for the Mipsmate (DS5100).
 *
 * This function is called during the initialization sequence in the SIM
 * to configure the DME by loading the appropriate routines into the the
 * HBA_DME_CONTROL structure in the sim_softc. This function is provided 
 * to allow the Data Mover Engine to be configured based on the capabilities
 * of the underlying HBA. 
 *
 * FORMAL PARAMETERS:  		SIM_SOFTC* sim_softc - Addr of SIM cntrl strc
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		sim_softc->dme initialized.
 *
 * RETURN VALUE:        	CAM_REQ_CMP - All is well.
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
ds5100_dme_attach(sim_softc)
    
    SIM_SOFTC* sim_softc;
    
{
    U32 retval=CAM_REQ_CMP;			/* Return value */
    SIM_MODULE(ds5100_dme_attach);
    
    /*  
     * Implement some for of scan for the appropriate vectors to load
     * into the dme_control structure.
     */
    SIM_PRINTD(NOBTL, NOBTL, NOBTL, CAMD_DMA_FLOW,
	       ("begin\n"));

    if ( CAM_REQ_CMP == ramsii_dme_init(sim_softc))
    {
	/* initialize the data mover engine	*/
	sim_softc->dme->vector.dme_init = ramsii_dme_init;
	
	/* setup for a transfer			*/
	sim_softc->dme->vector.dme_setup = ramsii_dme_setup;
	
	/* end a data transfer			*/
	sim_softc->dme->vector.dme_end  = ramsii_dme_end;
	
	/* pause the data mover engine		*/
	sim_softc->dme->vector.dme_pause= ramsii_dme_pause;
	
	 /* continue a transfer			*/
	sim_softc->dme->vector.dme_resume = ramsii_dme_resume;
	
	/* save the pointers			*/
	sim_softc->dme->vector.dme_save = ramsii_dme_save;
	
	/* pause the data mover engine		*/
	sim_softc->dme->vector.dme_restore = ramsii_dme_restore;
	
	/* modify the active data pointers	*/
	sim_softc->dme->vector.dme_modify  = ramsii_dme_modify;
	
	/* data in xfer with buffer */
	sim_softc->dme->vector.dme_copyin  = cam_wmbcopy_kn230;

	/* data out xfer with buffer */
	sim_softc->dme->vector.dme_copyout = cam_rmbcopy_kn230;
	
	/* clear the buffer */
	sim_softc->dme->vector.dme_clear   = cam_wmbzero_kn230; 
	
	return(retval);
    }
    else
	return(CAM_FAILURE);
    
};			/* ramsii_dme_attach */

U32
ds5100_dme_unload()
{
  return(1);
};

/**
 /**
  * cam_wmbcopy_kn230
  *
  * FUNCTIONAL DESCRIPTION: 
  * Copies data from the users buffer to the ram buffer of the Mipsmate.
  *
  * FORMAL PARAMETERS:
  * u_char	*src		Pointer to the user data
  * u_char	*dst		Pointer to the RAM buffer
  * u_int	len		Number of bytes to transfer
  * SIM_SOFTC	*sim_softc	Pointer to the SIM_SOFTC
  *
  * IMPLICIT INPUTS:     	NONE
  *
  * IMPLICIT OUTPUTS:		NONE
  *
  * RETURN VALUE:        	NONE
  *
  * SIDE EFFECTS:        	NONE
  *
  * ADDITIONAL INFORMATION:	NONE
  *
  **/
U32
cam_wmbcopy_kn230(src, dst, len, sim_softc)
register u_char *src;	/* pointer to user data */
register u_short *dst;	/* pointer to the RAM buffer */
register u_int len;	/* count of bytes to xfer */
SIM_SOFTC *sim_softc;	/* Softc control structure  */ 
{
    dst = DME_MOD_ADDRESS(dst,sim_softc);
    kn230_wbcopy(src, dst, len);
}

 /**
  * cam_rmbcopy_kn230
  *
  * FUNCTIONAL DESCRIPTION: 
  * Copies data from the ram buffer to the users buffer on the Mipsmate.
  *
  * FORMAL PARAMETERS:
  * u_char	*src		Pointer to the RAM buffer
  * u_char	*dst		Pointer to the users buffer
  * u_int	len		Number of bytes to transfer
  * SIM_SOFTC	*sim_softc	Pointer to the SIM_SOFTC
  *
  * IMPLICIT INPUTS:     	NONE
  *
  * IMPLICIT OUTPUTS:		NONE
  *
  * RETURN VALUE:        	NONE
  *
  * SIDE EFFECTS:        	NONE
  *
  * ADDITIONAL INFORMATION:	NONE
  *
  **/
U32
cam_rmbcopy_kn230(src, dst, len, sim_softc)
register u_short *src;	/* pointer to the RAM buffer */
register u_char *dst;	/* pointer to user data      */
register u_int len;	/* count of bytes to xfer    */
SIM_SOFTC *sim_softc;	/* Softc control structure  */ 
{
    src = DME_MOD_ADDRESS(src,sim_softc);
    kn230_rbcopy(src, dst, len);
}

 /**
  * cam_wmbzero_kn230
  *
  * FUNCTIONAL DESCRIPTION: 
  * Zeroes out the ram buffer of the MIPSMATE.
  *
  * FORMAL PARAMETERS:
  * u_char	*dst		Pointer to the ram buffer
  * u_int	len		Number of bytes to zero
  * SIM_SOFTC	*sim_softc	Pointer to the SIM_SOFTC
  *
  * IMPLICIT INPUTS:     	NONE
  *
  * IMPLICIT OUTPUTS:		NONE
  *
  * RETURN VALUE:        	NONE
  *
  * SIDE EFFECTS:        	NONE
  *
  * ADDITIONAL INFORMATION:	NONE
  *
  **/
U32 
cam_wmbzero_kn230(dst, len, sim_softc)
register u_short *dst;
register u_int len;
SIM_SOFTC *sim_softc;
{
    dst = DME_MOD_ADDRESS(dst,sim_softc);
    kn230_bzero(dst, len);
}

/* ---------------------------------------------------------------------- */
