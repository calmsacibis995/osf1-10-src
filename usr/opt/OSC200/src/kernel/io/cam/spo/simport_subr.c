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
static char *rcsid = "@(#)$RCSfile: simport_subr.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1993/08/13 21:11:29 $";
#endif
/**
 *   SIMport IO Common Routines
 *  
 *   Abstract:
 *   
 *
 **/
/* ----------------------------------------------------------------------
 *
 * Include files.
 *
 */
#ifdef OSF
#include <io/common/iotypes.h>
#include <sys/types.h>
#include <sys/time.h>
#include <io/cam/dec_cam.h>
#include <mach/vm_param.h>
#include <io/cam/cam.h>
#include <io/cam/scsi_all.h>
#include <vm/vm_kern.h>
#include <io/cam/cam_debug.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/dme.h>
#include <io/cam/sim.h>
#include <io/cam/sim_common.h>
#include <io/cam/scsi_status.h>
#include <io/cam/sim_xpt.h>
#include <io/cam/dme_common.h>
#include <io/cam/dme_common.h>
#include <io/cam/spo/simport.h>
#endif

/*
 * For now, put in stub routines for this module.
 */

extern SIM_SOFTC *softc_directory[];
SPOBSM_HEADER *
spo_buf_to_linked_bsm(SIM_SOFTC *, CCB_SCSIIO *, DMA_WSET *, U32);
U32 spo_phys_cont_page(CCB_SCSIIO *, vm_offset_t, U32, U32);

DMA_WSET *
spo_get_dma_wset(SIM_SOFTC *sc)
{
}


U32
spo_sg_to_linked_bsm(SIM_SOFTC *sc, CCB_SCSIIO *ioccb, u_char align)
{
}

SPOBSM_HEADER *
spo_buf_to_linked_bsm(SIM_SOFTC *sc, CCB_SCSIIO *ioccb, 
		DMA_WSET *dws, U32 xfer_align_code)
{
}

U32
spo_phys_cont_page(CCB_SCSIIO *ioccb, vm_offset_t start_addr, 
	U32 buflen, U32 max_len)
{
}
