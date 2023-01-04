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
static char *rcsid = "@(#)$RCSfile: sim_kztsa.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1993/08/13 21:10:16 $";
#endif
/*
 *  sim_kztsa.c
 */
#ifdef  OSF
#include <io/common/iotypes.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>
#include <net/netisr.h>
#include <sys/buf.h>
#include <sys/proc.h>
#include <io/cam/dec_cam.h>
#include <mach/vm_param.h>
#include <machine/machparam.h>
#include <machine/pmap.h>
#include <io/dec/tc/tc.h>
#include <io/cam/cam.h>
#include <io/cam/scsi_phases.h>
#include <io/cam/scsi_status.h>
#include <dec/binlog/errlog.h>  /* UERF errlog defines */
#include <io/cam/cam_logger.h>
#include <io/cam/scsi_all.h>
#include <kern/kalloc.h>
#include <kern/thread.h>
#include <kern/sched_prim.h>
#include <machine/cpu.h>
#include <io/common/devdriver.h>
#include <io/dec/uba/ubavar.h>
#include <io/cam/cam_debug.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/sim_target.h>
#include <io/cam/dme.h>
#include <io/cam/dme_common.h>
#include <io/cam/sim.h>
#include <io/cam/sim_common.h>
#include <io/cam/spo/simport.h>
#include <io/cam/cam_errlog.h>
#else   /* OSF */
#include "../h/types.h"
#include "../h/time.h"
#include "../machine/mips/pte.h"
#include "../machine/cpu.h"
#include "../net/net/netisr.h"
#include "../h/param.h"
#include "../h/buf.h"
#include "../io/uba/ubavar.h"
#include "../h/cam_bop.h"
#include "../h/cam.h"
#include "../h/scsi_phases.h"
#include "../h/scsi_status.h"
#include "../h/dec_cam.h"
#include "../h/scsi_all.h"
#include "../h/smp_lock.h"
#include "../h/kmalloc.h"
#include "../h/errlog.h"
#include "../h/cam_logger.h"
#include "cam_debug.h"
#include "sim_cirq.h"
#include "sim_target.h"
#include "dme.h"
#include "sim.h"
#include "sim_common.h"
#include "sim_kzq.h"
#include "cam_errlog.h"
#endif  /* OSF */
#include "sim_kztsa.h"

/* ---------------------------------------------------------------------- */
/* External declarations.
 */
extern SIM_SOFTC* softc_directory[];
extern CAM_SIM_ENTRY simport_entry;
extern task_t first_task;

extern int sim_poll_mode;

/* ---------------------------------------------------------------------- */
/* Function prototypes.
 */

U32 kztsa_probe();	       		/* Driver probe routine */
U32 kztsa_init();              		/* Called during config to init the HBA */
extern DMA_CTRL *dcmn_alloc_ctrl();	/* DME common routine */
extern DME_STRUCT *dcmn_alloc_dme();   	/* DME common routine */ 
extern U32 spo_go();       		/* Start an I/O in the SIM */
extern U32 spo_bus_reset();	/* HBA specific routine to reset the SCSI bus*/
extern void spo_isr_thread(); 	/* SIMport soft interrupt service thread */ 
extern U32 spo_thread_init(caddr_t);
extern void spo_misc_errors(SIM_SOFTC *, U32); 

/* ---------------------------------------------------------------------- */
/* Local declarations.
 */
U32 kztsa_attach();
U32 kztsa_adap_reset();
U32 kztsa_set_abbr();		/* SIMport Set Adapter Block Register */
U32 kztsa_read_asr();		/* SIMport Read Adapter Status Register */
U32 kztsa_pathinq();		/* Hardware Dependent Path Inquiry Routine */
U32 kztsa_ioccb_setup();	
U32 kztsa_dme_attach(SIM_SOFTC *);
U32 kztsa_dme_unload();
U32 kztsa_dme_init(SIM_SOFTC *);
vm_offset_t dcmn_ioccb_vtop(CCB_SCSIIO *, vm_offset_t);
void kztsa_sel_timeout();      	/* Routine called to handle selection TMO */
void kztsa_intr(short);      	/* Routine called to handle selection TMO */
void kztsa_logger();           	/* HSR to log HBA unique information */
void kztsa_error_recov();      	/* Error recovery function */
caddr_t kztsa_get_reg_addr(SIM_SOFTC*, u_char); 	
			/* Get the SIMport base register address */ 

extern SPQCAR* spo_remove_queue_entry(SPQHDR *);
extern SPQCAR* spo_poll_mode(SIM_SOFTC *);
extern SPOBSM_HEADER* spo_buf_to_linked_bsm(SIM_SOFTC *, CCB_SCSIIO *, 
			DMA_WSET *, U32);
extern U32 spo_sg_to_linked_bsm(SIM_SOFTC *, CCB_SCSIIO *, U32);
extern DMA_WSET *spo_get_dma_wset(SIM_SOFTC *);

/*
 * These arrays contain the Vendor ID strings used in the Path Inquiry CCB.
 * Their length must not be any greater than SIM_ID , which
 * is 16, from the cam.h include file.
 */

/*  string position         "0000000001111111" */
/*  counter                 "1234567890123456" */
static char *spo_hba_vid =  "DEC KZTSA OSF/V1";

/*
 * For now, put stub routines in this module.
 */

U32
kztsa_probe(caddr_t csr, struct controller *prb)
{
	return( CAM_FALSE );       /* Signal that it is not there */
}

U32
kztsa_attach(SIM_SOFTC* sc)
{
}

U32
kztsa_adap_reset(register SIM_SOFTC* sc)
{
}

U32
kztsa_init(register SIM_SOFTC* sc, U32 resetflag)
{
}

U32
kztsa_dme_attach(SIM_SOFTC *sc)
{
}                  

U32
kztsa_dme_init(SIM_SOFTC *sc)
{
}

U32
kztsa_set_abbr(register SIM_SOFTC* sc, vm_offset_t phys)
{
}


caddr_t 
kztsa_get_reg_addr(SIM_SOFTC *sc, u_char reg_code)
{
}	
	     

U32
kztsa_read_asr(SIM_SOFTC* sc)
{
}

void
kztsa_intr(short controller)
{
}

U32
kztsa_pathinq(register CCB_PATHINQ* cpi)
{
}

kztsa_go(SIM_WS* sws)
{
}

U32
kztsa_ioccb_setup(CCB_HEADER* ccb_hdr)
{
}

U32 kztsa_unload()
{
}

U32
kztsa_dme_unload()
{
}


vm_offset_t
dcmn_ioccb_vtop(CCB_SCSIIO* ioccb, vm_offset_t vaddr)
{
}


void
kztsa_logger(u_char *func, u_char *msg, U32 flags, SIM_SOFTC *sc)
{    
}


