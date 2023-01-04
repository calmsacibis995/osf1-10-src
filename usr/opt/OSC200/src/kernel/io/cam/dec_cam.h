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
/*
 * @(#)$RCSfile: dec_cam.h,v $ $Revision: 1.1.17.4 $ (DEC) $Date: 1993/08/13 20:54:20 $
 */
#ifndef _DEC_CAM_
#define _DEC_CAM_

/* ---------------------------------------------------------------------- */

/* dec_cam.h		Version 1.03			Dec. 11, 1991 */

/*  This file contains the definitions and data structures needed by
    the DEC CAM source files.

Modification History

	Version	Date		Who	Reason

	1.00	03/21/91	janet	Created this file.

	1.01	05/24/91	maria	Added define for DEC_AUTO_SENSE_SIZE

	1.02	05/24/91	maria	Added define for DEFAULT_SCSIID

	1.03	12/11/91	jag	Added the CDrv wait define.
*/

/* ---------------------------------------------------------------------- */

#define NDPS			8	/* Number of Devices Per SCSI bus */
#define NLPT			8	/* Number of Luns Per Target	*/
#define MAX_LUNS		7	/* Maximum LUN value		*/
#define MAX_TARGETS		7	/* Maximum target id		*/

#define DEFAULT_SCSIID          7       /* Default system scsi id	*/

#define DEC_AUTO_SENSE_SIZE	164

/*
 * Default command timeout value.
 */
#define SIM_DEFAULT_TIMEOUT	30	/* Thirty seconds			*/

/*
 * Wait loop count define for the CDrv during EDT probing.
 * NOTE:  This count is the number of milliseconds to wait before determining
 * that an abort or bus reset is needed to get the original INQUIRY CCB back.
 */

#define CCFG_WAIT_DELAY_LOOP	2000		/* loop # for 2 seconds */

/* ---------------------------------------------------------------------- */

/* Defines for the CAM cam_vu_flags field in the SCSIIO_CCB. */

#define DEC_CAM_HIGH_PRIOR	0xc000		/* This CCB will be given
						 * high priority by the SIM.
						 */
#define DEC_CAM_MED_PRIOR	0x8000		/* This CCB will be given
						 * medium priority by the SIM.
						 */
#define DEC_CAM_LOW_PRIOR	0x4000		/* This CCB will be given
						 * low priority by the SIM.
						 */
#define DEC_CAM_ZERO_PRIOR	0x0000		/* This CCB will not be
						 * assigned a priority.
						 */
#define DEC_CAM_PRIORITY_MASK	0xc000		/* Bits of cam_vu_flags
						 * which are used by the SIM
						 * to specify a priority level.
						 */

/*
 * DEC specific macros
 */
#ifdef __alpha
#define CAM_IS_KUSEG(addr)	(!IS_SYS_VA((vm_offset_t)(addr)))
#define CAM_PHYS_TO_KVA(addr)	(PHYS_TO_KSEG(addr))
#else /* __alpha */
#define CAM_IS_KUSEG(addr)	(IS_KUSEG((addr)))
#define CAM_PHYS_TO_KVA(addr)	(PHYS_TO_K1(addr))
#endif /* __alpha */

#if defined(B_HWRELOC)
#define CAM_IS_HWRELOC_SET(flags) ((flags) & B_HWRELOC)
#else
#define CAM_IS_HWRELOC_SET(flags) (0)
#endif /* B_HWRELOC */

/*
 * VM memory page lock down for IO
 */
#define CAM_VM_USERACC(addr, len, prot )				\
        (vm_map_check_protection( current_task()->map,			\
                trunc_page((addr)), round_page((addr)+(len)),			\
                prot == B_READ ? VM_PROT_READ : VM_PROT_WRITE)) 	\

#define CAM_VM_LOCK( addr, len ) 					\
        vm_map_pageable(current_task()->map, trunc_page((addr)),	\
                  round_page((addr)+(len)), VM_PROT_READ|VM_PROT_WRITE);	


#define CAM_VM_UNLOCK( addr, len, prot ) 				\
        vm_map_pageable(current_task()->map, trunc_page((addr)),		\
                  round_page((addr)+(len)), VM_PROT_NONE);			\


/*
 * CAM locking macros.
 */
/* NOT YET...
#define CAM_LOCK_IT(lk, flags) lock_write((lk))
#define CAM_UNLOCK_IT(lk) lock_done((lk))
#define CAM_SLEEP_UNLOCK_IT(chan, pri, lk) lock_done( (lk))
*/
#define CAM_LOCK_IT(lk, flags)
#define CAM_UNLOCK_IT(lk)
#define CAM_SLEEP_UNLOCK_IT(chan, pri, lk) \
    mpsleep(chan, pri, "Zzzzzz", 0, (void *)0, 0)

#endif /* _DEC_CAM_ */

