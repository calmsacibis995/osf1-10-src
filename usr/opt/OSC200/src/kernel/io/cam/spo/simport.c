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
static char *rcsid = "@(#)$RCSfile: simport.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/08/13 21:11:02 $";
#endif
/*
 * FACILITY:
 *
 *      ULTRIX SCSI CAM SUBSYSTEM
 *
 * ABSTRACT:
 *
 * The SIMport host driver is the component of the SIM which the XPT calls to
 * initiate I/O on behalf  of peripheral device drivers (PDrv).
 * The peripheral device driver allocates a CCB from the XPT
 * and fills CCB fields with SCSI command bytes and other information.
 * The PDrv then hands this CCB to the XPT which calls the selected SIM's,
 * spo_action routine to begin processing of the I/O request. The SIMport
 * host driver sends this request to SIMport adapter command queue.
 * The SIMport host adapter processes the command and notify the
 * SIMport host driver the completion of the command through the
 * SIMport driver adapter response queue.  When the SIMport host driver
 * receives a completed I/O request it will callback to  the peripheral device
 * driver and release any SIM resources allocated for the request.
 */

/*
 * simport.c
 * This module contains the common routines to simport architecture.
 */


/*
 * SIMport Command and Response Processing
 */
/* ----------------------------------------------------------------------
 *
 * Include files.
 *
 */
#ifdef OSF
#include <io/common/iotypes.h>
#include <sys/types.h>
#include <sys/time.h>
#include <mach/vm_param.h>
#include <vm/vm_kern.h>
#include <kern/thread.h>
#include <io/cam/dec_cam.h>
#include <io/cam/cam.h>
#include <io/cam/scsi_all.h>
#include <io/cam/cam_debug.h>
#include <io/cam/cam_errlog.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/dme.h>
#include <io/cam/sim.h>
#include <io/cam/sim_common.h>
#include <io/cam/scsi_status.h>
#include <io/cam/sim_xpt.h>
#include <io/cam/dme_common.h>
#include <io/cam/spo/simport.h>
#include <kern/queue.h>

#else /* OSF */
#include "../h/types.h"
#include "../h/time.h"
#include "../h/smp_lock.h"
#include "../h/kmalloc.h"
#include "../h/cam_bop.h"
#include "../h/cam.h"
#include "../h/dec_cam.h"
#include "../h/scsi_all.h"
#include "cam_debug.h"
#include "sim_target.h"
#include "sim_cirq.h"
#include "dme.h"
#include "sim.h"
#include "sim_common.h"
#include "../h/scsi_status.h"
#include "sim_xpt.h"

#endif /* OSF */

/* ----------------------------------------------------------------------
 *
 * extern declaration:
 *
 */

/*
 * There is one SOFTC_DIRECTORY for the entire system. For each HBA channel
 * present there is one entry in the SOFTC_DIRECTORY. The SOFTC_DIRECTORY
 * is indexed * by the pathid and contains the address of the softc for a
 * particular HBA channel. This structure is statically allocated and
 * filled in when the kernel is built.
 */
extern SIM_SOFTC* softc_directory[];    /* Address of HBA's sim_softc */
extern I32 spo_init();
extern I32 spo_action();
extern task_t first_task;
extern int hz;

/*
 * This structure is used by the XPT to find the address of the SIM's
 * entry points. The cam_conftbl[] contains pointers to this type of
 * structures.
 */
CAM_SIM_ENTRY simport_entry =           /* One per SIM, used by the XPT */
{
    spo_init,                   /* to initialize the SIMport */
    spo_action                  /* to "do" a CCB */
};

/*
 * These arrays contain the Vendor ID strings used in the Path Inquiry CCB.
 * Their length must not be any greater than SIM_ID, which
 * is 16, from the cam.h include file.
 */

/*  string position     "0000000001111111" */
/*  counter             "1234567890123456" */
static char *spo_vid =  "DEC SIMPORT/VA.7";

/*----------------------------------------------------------------------
 *
 * Function Prototypes:
 *
 */
static SIM_WS* sx_setup_ws();
static U32 spo_immediate(register CCB_HEADER *);
U32 spo_bsm_alloc(SIMPORT_SOFTC *);
U32 spo_qcar_init(SIMPORT_SOFTC *, caddr_t, U32);
/*
U32 spo_get_free_qcar_entry(SIMPORT_SOFTC*, SPQCAR*);
*/
U32 spo_add_queue_entry(SIM_SOFTC*, SPQHDR *, SPQCAR*, CCB_HEADER *, u_char);
SPQCAR* spo_remove_queue_entry(SPQHDR *);
SPQCAR* spo_poll_cmd(SIM_SOFTC *, U32);
SPQCAR* spo_poll_mode(SIM_SOFTC *);
U32 spo_go(SIM_SOFTC *, SPQCAR *,  CCB_HEADER *);
U32 spo_ab_init(SIM_SOFTC *);
U32 spo_set_adap_state_cmd(SIM_SOFTC *, u_char);
U32 spo_set_channel_state(SIM_SOFTC *, u_char, u_char, u_char);
U32 spo_pathinq_cmd(CCB_HEADER *);
U32 spo_immd_cmd(SIM_SOFTC *, CCB_HEADER *);
U32 spo_ccb_dup(CCB_HEADER *, CCB_HEADER *);
U32 spo_bus_reset(SIM_SOFTC *);
U32 spo_set_dev_state_cmd(SIM_SOFTC *, U32, U32, U32, u_char);
void spo_bus_reset_rspn(CCB_HEADER *);
void spo_process_adap_state_set_msg(SIMPORT_SOFTC *, CCB_SPVU_STATE_SET *);
void spo_verify_adap_sanity(U32);
void spo_read_asr(SIM_SOFTC *);
void spo_misc_errors(SIM_SOFTC *, U32);
U32 spo_thread_init(caddr_t);
void spo_isr_thread();
#ifdef SPODEBUG
void spo_assert(CCB_HEADER *);
#endif

/*
 *  Common SIM XPT Routines:
 */
extern void sx_done_device_reset();
extern void sx_queue_async();

/*----------------------------------------------------------------------
 *
 * Macros:
 *
 * See simport.h include file.
 */

/******************************************************************************
 *
 * Start of SIMport.C routines.
 *
 *****************************************************************************/

/*
 * For now, put stub routines in this module.
 */

I32
spo_action(register CCB_HEADER* ccb_hdr)
{
}                       /* spo_action */


I32
spo_init(U32 pathid)
{
}                               /* spo_init */

U32
spo_ab_init(SIM_SOFTC* sc)
{
}

U32
spo_qcar_init(SIMPORT_SOFTC *spsc, caddr_t start_addr, U32 size)
{
}

U32
spo_bsm_alloc(SIMPORT_SOFTC* spsc)
{
}

U32
spo_unload( SIM_SOFTC *sim_softc )
{
}

static U32
spo_immediate (register CCB_HEADER* ccb_hdr)
{
}

U32
spo_go(SIM_SOFTC* sc, SPQCAR* qcarp, CCB_HEADER* ccb_hdr)
{
}

U32
spo_immd_cmd(SIM_SOFTC *sc, CCB_HEADER *ccb)
{
}

SPQCAR*
spo_poll_cmd(SIM_SOFTC *sc, U32 code)
{
}

SPQCAR *
spo_poll_mode(SIM_SOFTC *sc)
{
}

void
spo_iocmd_cmplt(SIM_SOFTC *sc, register CCB_HEADER* ccb_hdr)
{
}

U32
spo_get_free_qcar_entry(SIMPORT_SOFTC* spsc, SPQCAR* qcarp)
{
}

U32
spo_put_free_qcar(SIMPORT_SOFTC* spsc, SPQCAR* qcarp)
{
}

U32
spo_add_queue_entry(SIM_SOFTC* sc, SPQHDR* qhdr, SPQCAR* qcarp,
                CCB_HEADER* ccb_hdr, u_char reg_code)
{
}

SPQCAR*
spo_remove_queue_entry(SPQHDR *qhdr)
{
}

U32
spo_reset_detected(U32 pathid)
{
}

U32
spo_set_adap_state_cmd(SIM_SOFTC* sc, u_char state)
{
}

U32
spo_set_param_cmd(SIM_SOFTC* sc, u_char flags, U32 sys_time, U32 int_holdoff,
        U32 rp_timer, u_char n_host_sg, SPOBSD host_sg_bsd)
{
}

U32
spo_set_channel_state(SIM_SOFTC* sc, u_char state, u_char chan_id,
                        u_char node_id)
{
}

U32
spo_set_dev_state_cmd(SIM_SOFTC *sc, U32 path_id, U32 target_id,
        U32 target_lun, u_char state)
{
}

U32
spo_veri_adap_sanity_cmd(SIM_SOFTC* sc)
{
}

U32
spo_read_counters(SIM_SOFTC* sc, u_char flags)
{
}

U32
spo_bsd_rspn_cmd(SIM_SOFTC* sc, CCB_HEADER* ccb_hdr)
{
}

U32
spo_pathinq_cmd(register CCB_HEADER *ccb_hdr)
{
}

U32
spo_bus_reset(SIM_SOFTC* sc)
{
}

void
spo_verify_adap_sanity(U32 pathid)
{
}

U32
spo_ccb_dup(CCB_HEADER* occb, CCB_HEADER* nccb)
{
}

void
spo_softintr(SIM_SOFTC* sc)
{
}

spo_enable_interrupts(SIM_SOFTC *sc)
{
}

U32
spo_thread_init(caddr_t sc)
{
}

void spo_isr_thread()
{
}

void
spo_process_adap_state_set_msg
(SIMPORT_SOFTC* spsc, CCB_SPVU_STATE_SET* spccb)
{
}

void
spo_bus_reset_rspn(CCB_HEADER* ccb_hdr)
{
}

SPQCAR*
spo_get_queue_carrier(SPQHDR *fqh)
{
}

void
spo_read_asr(SIM_SOFTC* sc)
{
}

void spo_misc_errors(SIM_SOFTC *sc, U32 asr_reg)
{
}

#ifdef SPODEBUG
void spo_assert(CCB_HEADER *ccb_hdr)
{
}
#endif



