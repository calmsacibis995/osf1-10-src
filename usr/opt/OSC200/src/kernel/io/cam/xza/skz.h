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
#if !defined(SKZ_INCLUDE)
#define SKZ_INCLUDE

/************************************************************************
 *									*
 * File:	skz.h							*
 * Date:	November 6, 1991					*
 * Author:	Ron Hegli						*
 *									*
 * Description:								*
 *	This file contains skz driver structures and definitions for    *
 *	the XZA/SCSI							*
 *									*
 ************************************************************************/

/*
** Basic masks
*/
#define XZA_CHAN_0		0x00000001
#define XZA_CHAN_1		0x00000002
#define XZA_BOTH_CHANNELS	0x00000003

#define	SKZ_ALL_LUNS		0xff

/*
** SKZ Flags
*/
#define SKZ_EEPROM_DATA_UPDATED		0x00000001L /* EEPROM up-to-date */
#define SKZ_RESET_NEEDED		0x00000002L /* bus reset required */
#define SKZ_BUS_RESET_IN_PROGRESS	0x00000004L /* bus reset in progress */
#define SKZ_PROBE			0x00000010L /* Probe flag */
#define SKZ_THREADS_ACTIVE		0x00000020L /* Threads Initialized */
#define SKZ_DUP_VA			0x00000040L /* Kernel mapped uva's */
#define SKZ_SG				0x00000080L /* SG array alloc'ed */
#define SKZ_ADAPT_RESET_IN_PROGRESS	0x00000100L /* adapter reset */
#define SKZ_RESP_TH			0x00000200L /* Resp thread init */
#define SKZ_ERROR_TH			0x00000400L /* Error thread init */
#define SKZ_ALIVE			0x00000800L /* Chan alive flag */



/*
** XZA SCSI Target Structure
*/
typedef struct xza_scsi_target {
	unsigned long		purgqs;		/* outstanding PURGQ cmds */

	simple_lock_data_t	lock;
} XZA_SCSI_TARGET;


/*
** XZA Channel Specific Structure
*/
typedef struct xza_channel {
	XZA_SCSI_TARGET		target[NDPS];
	XZA_STATE		state;
	
	SCSI_ID			scsi_id;
	u_char			chan_num;

	vm_offset_t		xza_softc;
	SIM_SOFTC*		sim_softc;

	thread_t		misc_th;

	u_long			flags;

	ASR_REG			asr;

	u_long			commands_sent;

	simple_lock_data_t	lock;
} XZA_CHANNEL;


/*
** XZA Softc structure
*/
typedef struct xza_softc {

	caddr_t			csr;			/* XZA base_addr */
        vm_offset_t		xza_base;		/* ditto */

	XDEV_REG		xdev;			/* saved XDEV reg */

	XZA_REG_ADDRS		xza_reg_addrs;		/* Reg addresses */

	XZA_CHANNEL		channel[XZA_CHANNELS];

	AB*			ab;			/* adapter block */
	QB*			np_qb_pool;		/* q_buffer pool */
	CARRIER*		np_carrier_pool;	/* carrier pool  */

	SKZ_DME_SG_DSC*		dme_sg_dsc_pool;
	CARRIER*		dme_sg_dsc_car_pool;

	SKZ_ERR_BUF*		err_buf_pool;		/* error buffer pool */
	CARRIER*		err_buf_car_pool;	/* err buf carriers */

 	vm_offset_t		ampb_ptr;		/* virtual ampb ptr */
	CQIB*			cqib;			/* que insert block */
	u_int			ampb_size;		/* requested pages */

	NP_Q			ddfq;			/* driver free queue */
	NP_Q			ddsgq;	
	NP_Q			ddeq;			/* driver error que */
	NP_Q			ddefq;			/* error free que */
	
	unsigned short		xza_xmi_node;
	unsigned short		lamb_xmi_node;
	struct bus*		xza_bus;	/* CHECK!! ( controller?? */

	unsigned short		c0_misc_intr_vector;
	unsigned short		c1_misc_intr_vector;
	unsigned short		resp_intr_vector;

    	/*
	** Threads
	*/
	thread_t		resp_th;
	thread_t		error_th;

	NEXUS_ENTRY		setnex_mask;	/* mask field for SETNEX */
	EEPROM_FCN		eeprm_fcn;	/* EEPROM param settings */

	unsigned long		flags;

	simple_lock_data_t	lock;
} XZA_SOFTC;

/*
** Directories of controlling data structures, used for later lookup
** based on controller/bus number
*/
extern SIM_SOFTC* softc_directory[];

#define PAGE_MASK page_mask
#define THIS_PAGE_ADDR( addr ) trunc_page ( addr )
#define NEXT_PAGE_ADDR( addr ) round_page ( addr + 1 )	/* always *next* page */

/*
** virtual to physical translations
*/
#define kvtop(kva,pa) svatophys(kva,pa)


/*
** Timing!!
*/
extern int hz;

/*
** Macros to help deal with threads
*/
#define skz_thread_wakeup(event,result) \
	thread_wakeup_prim((event), TRUE, (result))
#define skz_clear_wait(thread,result) \
	clear_wait((thread), (result), TRUE)

#define SKZ_IDLE_THREAD	(current_thread()->state & TH_IDLE)
#define SKZ_CHAN_THREADS_ACTIVE(xza_chan) (xza_chan->flags & SKZ_THREADS_ACTIVE)

#define SKZ_ALREADY_WAITING (current_thread()->wait_event != 0)

#define SKZ_SAVE_WAIT {					\
	wait_event = current_thread()->wait_event; 	\
	if ( wait_event != 0 )				\
		clear_wait(current_thread(), THREAD_INTERRUPTED, TRUE);	\
}

#define SKZ_RESTORE_WAIT {				\
	if ( wait_event != 0 )				\
		assert_wait((vm_offset_t)wait_event, TRUE );	\
}


/*
** Locking macros - handle IPL and SMP locks
*/
#define SPLMISC	SPLBIO
#define SPLRESP SPLBIO

#define SKZ_LOCK_INIT(lock) simple_lock_init ( &lock )

#if SKZ_THREADED == 0
#define SKZ_LOCK(lock, s) { 		\
	s = splbio();			\
	simple_lock ( &lock );		\
}
#else
#define SKZ_LOCK(lock, s) { 		\
	simple_lock ( &lock );		\
}
#endif

#if SKZ_THREADED == 0
#define SKZ_UNLOCK(lock, s) { 		\
	simple_unlock ( &lock );	\
	splx(s);			\
}
#else
#define SKZ_UNLOCK(lock, s) { 		\
	simple_unlock ( &lock );	\
}
#endif


#endif /* SKZ_INCLUDE */
