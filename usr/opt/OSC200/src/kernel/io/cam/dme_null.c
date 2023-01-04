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
static char *rcsid = "@(#)$ $ (DEC) $";
#endif

/* CAM DME for HBA's which don't need an associated DME.  This module
 * just provided stubs to make CAM happy.
 */
#ifdef OSF
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
#include <io/cam/sim.h>
#include <io/cam/sim_common.h>
#include <io/dec/tc/tc.h>

#else /* OSF */

#ifdef mips
#include "../machine/mips/pte.h"
#else
#include "../machine/vax/pte.h"
#endif
#include "../machine/common/cpuconf.h"          /* CPU specific defines */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/dir.h"
#include "../h/conf.h"
#include "../h/kmalloc.h"
#include "../h/user.h"
#include "../h/file.h"
#include "../h/map.h"
#include "../h/vm.h"
#include "../h/dk.h"
#include "../h/proc.h"
#include "../h/ioctl.h"
#include "../h/mtio.h"
#include "../h/cmap.h"
#include "../h/uio.h"

#include "../h/devio.h"
#include "../h/ipc.h"
#include "../h/shm.h"
#include "../h/types.h"


#include "../h/cam_bop.h"
#include "../h/cam.h"
#include "../h/dec_cam.h"
#include "../h/scsi_all.h"
#include "cam_debug.h"
#include "sim_cirq.h"
#include "sim_target.h"
#include "dme.h"                        /* DME specific structs and consts */
#include "sim.h"                        /* SIM specific structs and consts */
#include "../h/scsi_status.h"
#include "sim_common.h"
#include "../io/tc/tc.h"

#endif OSF

U32 dme_null_null();

U32
dme_null_attach(SIM_SOFTC *scp)
{
    DME_STRUCT *dme;

    dme = (DME_STRUCT *)sc_alloc(sizeof( DME_STRUCT ));

    if(!dme)
    {
	 printf("[b/t/l] dme_null_attach:  Unable to allocate memory for dme structure.\n" );
	 return CAM_REQ_CMP_ERR;
    }

    scp->dme = dme;

    dme->vector.dme_init = dme_null_null;
    dme->vector.dme_setup = dme_null_null;
    dme->vector.dme_start = dme_null_null;
    dme->vector.dme_end = dme_null_null;
    dme->vector.dme_pause = dme_null_null;
    dme->vector.dme_resume = dme_null_null;
    dme->vector.dme_save = dme_null_null;
    dme->vector.dme_restore = dme_null_null;
    dme->vector.dme_modify = dme_null_null;
    dme->vector.dme_copyin = dme_null_null;
    dme->vector.dme_copyout = dme_null_null;
    dme->vector.dme_clear = dme_null_null;
    dme->vector.dme_interrupt = dme_null_null;

    return CAM_REQ_CMP;
}

U32
dme_null_null()
{
    return CAM_REQ_CMP;
}
