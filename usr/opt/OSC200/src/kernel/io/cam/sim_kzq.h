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
 * @(#)$RCSfile: sim_kzq.h,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/09/21 21:53:45 $
 */
#ifndef _SIM_KZQ_
#define _SIM_KZQ_

#include <io/common/devdriver.h>

/* ---------------------------------------------------------------------- */

/* sim_kzq.h		Version 1.04			Dec 3, 1991    */

/*  This file contains the definitions and data structures needed by the
    DEC KZQ SIM module.

Modification History

	Version	  Date		Who	

	1.04	12/03/91	janet
	Added SIMKZQ_REG_VERS define.
	
	1.03    11/15/91	rln
	Add SIMKZQ_SOFTC_VERS symbols.

	1.02	10/22/91	rln	
	PreEFT changes: 1) Make registers volatile

	1.01	8/12/91		rln	
	Added new symbols

	1.00	3/14/91		rln
	Created this file.
*/

/* ---------------------------------------------------------------------- */

/*
 * KZQ specific constants:
 *
 */

#define SIM_KZQ_SYNC_OFFSET 0x3	/* Maximum sdtr offset for this chip */
#define SIM_KZQ_SYNC_PERIOD 63		/* Transfer period from old driver   */
  

/*
 * Macro Name : SIMKZQ_GET_INTR
 *
 * Macro Description :
 *	SIMKZQ_GET_INTR() will get a buffer from the interrupt queue
 *	kept for the KZQ.  This function will increment the current
 *	index so that the next call to this macro will return the
 *	next buffer.
 */
#define SIMKZQ_GET_INTR(ssc, intr);					\
{									\
	intr = &(CIRQ_GET_CURR((ssc)->intrq, (ssc)->intrq_buf));	\
	(ssc)->intrq.curr = CIRQ_INC((ssc)->intrq, (ssc)->intrq.curr);	\
}

/*
 * Macros to store and retrieve the last command.
 */
#define SIMKZQ_STORE_CMD(sws, cmd); (sws)->hba_data[0] = (U_WORD) cmd;
#define SIMKZQ_LAST_CMD(sws) (sws)->hba_data[0]

/*
 * Get the HBA specific interrupt data from the SIM State Machine's
 * structure.
 */
/*#define SIMKZQ_GET_CSR(hba_intr) (hba_intr)->csr*/
#define SIMKZQ_GET_CSAT(hba_intr) (hba_intr)->cstat
#define SIMKZQ_GET_DSTAT(hba_intr) (hba_intr)->dstat

/*
 * Get the HBA specific DMA address, RAM buffer address, and CSR.
 */
#define SIMKZQ_GET_RAM_BUF(softc) \
	(void *)((U32)((DME_KZQ_STRUCT *)(softc)->dme->extension)->SVA)

#define SIMKZQ_GET_CSR(sc) ((SIMKZQ_REG *)(sc)->reg)

/*
 * Macro Name : SIMKZQ_GET/PUT_BYTE
 *
 * Macro Description :
 *	SIMKZQ_GET/PUT_BYTE will get or put a message byte into or out of
 * the KZQ data register.
 */
#define SIMKZQ_GET_BYTE(byte,reg);					\
{								        \
  (byte) = (u_char) (reg)->sdb;						\
}

#define SIMKZQ_PUT_BYTE(byte,reg);					\
{							        	\
 (reg)->data = (u_short) (byte);		                        \
 WBFLUSH();								\
}

/*
 * Macro Name : SIMKZQ_READ_MESSAGE
 *
 * Macro Description :
 *	SIMKZQ_READ_MESSAGE will read a message byte from the FIFO
 *	and put it in the message in queue.
 */
#define SIMKZQ_READ_MESSAGE(sws, reg);					\
{									\
    if (SIMKZQ_GET_FIFO_LN(reg) == 0) {					\
    	printf("(SIMKZQ_READ_MESSAGE) Zero FIFO length!\n");		\
    }									\
    SC_ADD_MSGIN(sws, (reg)->data);					\
}

/*
 * Macro Name : SIMKZQ_READ_STATUS
 *
 * Macro Description :
 *	SIMKZQ_READ_STATUS will read a status byte from the FIFO
 *	and save it in the SIM_WS "scsi_status" field.
 */
#define SIMKZQ_READ_STATUS(sws, reg);					\
{									\
(reg)->data);								\
}


/*
 * This macro is executed, before returning from a code segment that is request
 * ing an interrupt from the KZQ. Here we will record that we are indeed 
 * waiting  for an interrupt and which command was issued to the KZQ.
 */

#define CMD_PENDING(softc,cmd,reg) {((SIMKZQ_SOFTC*)softc)->pend_int = 1;\
    ((SIMKZQ_SOFTC*)softc)->which_int=(cmd);\
    (reg)->comm = (cmd);}



#define CLEAR_PENDING(softc,cmd) {((SIMKZQ_SOFTC*)softc)->pend_int = (cmd);\
				      ((SIMKZQ_SOFTC*)softc)->which_int=(cmd);}


/*
 * Locking macros for the SIM KZQ.
 */

/*#define SC_LOCK(s,lock_handle) {cprintf("Locked %d\n");s = splbio();}*/
#define SC_LOCK(s,lock_handle)	(s) = splbio()

/*#define SC_UNLOCK(s,lock_handle) {cprintf("Unlocked\n");splx(s);}*/
#define SC_UNLOCK(s,lock_handle)	splx(s)

/*
 * KZQ register set.
 */
#define SIMKZQ_REG_VERS 1
typedef struct simkzq_reg {
    volatile u_short sdb;		/* SCSI Data Bus and Parity */
    volatile u_short sc1;		/* SCSI Control Signals One */
    volatile u_short sc2;		/* SCSI Control Signals Two */
    volatile u_short csr;		/* Control/Status register */
    volatile u_short id;			/* Bus ID register */
    volatile u_short slcsr;		/* Select Control and Status Register */
    volatile u_short destat;		/* Selection Detector Status Register */
    volatile u_short dstmo;		/* DSSI Timeout Register */
    volatile u_short data;		/* Data Register */
    volatile u_short dmctrl;		/* DMA Control Register */
    volatile u_short dmlotc;		/* DMA Length of Transfer Counter */
    volatile u_short dmaddrl;		/* DMA Address Register Low */
    volatile u_short dmaddrh;		/* DMA Address Register High */
    volatile u_short dmabyte;		/* DMA Initial Byte Register */
    volatile u_short stlp;		/* DSSI Short Target List Pointer */
    volatile u_short ltlp;		/* DSSI Long Target List Pointer */
    volatile u_short ilp;		/* DSSI Initiator List Pointer */
    volatile u_short dsctrl;		/* DSSI Control Register */
    volatile u_short cstat;		/* Connection Status Register */
    volatile u_short dstat;		/* Data Transfer Status Register */
    volatile u_short comm;		/* Command Register */
    volatile u_short dictrl;		/* Diagnostic Control Register */
    volatile u_short clock;		/* Diagnostic Clock Register */
    volatile u_short bhdiag;		/* Bus Handler Diagnostic Register */
    volatile u_short sidiag;		/* SCSI IO Diagnostic Register */
    volatile u_short dmdiag;		/* Data Mover Diagnostic Register */
    volatile u_short mcdiag;		/* Main Control Diagnostic Register */
    volatile u_short dmacsr;		/* DMA Control/Status Reg. for KZQSA	*/
    volatile u_short qbar;		/* Qbus Address (high 5 bits in dmacsr) */
    volatile u_short lbar;		/* Local Address for Qbus DMA.		*/
    volatile u_short wc;		/* Word Count for DMA.			*/
    volatile u_short vector;		/* Intr vector & 128k memory base.	*/
} SIMKZQ_REG;

/*
 * KZQ phase defines.
 */
#define SIMKZQ_PHASE_DATAOUT	0x0000
#define SIMKZQ_PHASE_DATAIN	0x0001
#define SIMKZQ_PHASE_COMMAND	0x0002
#define SIMKZQ_PHASE_STATUS	0x0003
#define SIMKZQ_PHASE_MSGOUT	0x0006
#define SIMKZQ_PHASE_MSGIN	0x0007

/*
 * KZQ SCSI Data Bus register bit defines.
 */
#define KZQ_SDB_PTY		0x0100
#define KZQ_SDB_DATA		0x00ff

/*
 * KZQ SCSI Control Signals One
 */
#define KZQ_SC1_BSY		0x0100
#define KZQ_SC1_SEL		0x0080
#define KZQ_SC1_RST		0x0040
#define KZQ_SC1_ACK		0x0020
#define KZQ_SC1_REQ		0x0010
#define KZQ_SC1_ATN		0x0008
#define KZQ_SC1_MSG		0x0004
#define KZQ_SC1_CD		0x0002
#define KZQ_SC1_IO		0x0001

/*
 * KZQ SCSI Control Signals Two
 */
#define KZQ_SC2_IGS		0x0008
#define KZQ_SC2_TGS		0x0004
#define KZQ_SC2_ARB		0x0002
#define KZQ_SC2_SBE		0x0001

/*
 * KZQ Control/Status Register
 */
#define KZQ_CSR_HPM		0x0010
#define KZQ_CSR_RSE		0x0008
#define KZQ_CSR_SLE		0x0004
#define KZQ_CSR_PCE		0x0002
#define KZQ_CSR_IE		0x0001

/*
 * KZQ Bus ID Register
 */
#define KZQ_ID_BUSID		0x0007
#define KZQ_ID_IO		0x8000

/*
 * KZQ Selector Control and Status Register
 */
#define KZQ_SLCSR_BUSID		0x0007

/*
 * KZQ Selection Detector Status Register
 */
#define KZQ_DESTAT_BUSID	0x0007

/*
 * KZQ DSSI Timeout Register
 */
#define KZQ_DSTMO_ENA		0x8000
#define KZQ_DSTMO_TARGTO	0x00f0
#define KZQ_DSTMO_INITTO	0x000f

/*
 * KZQ Data Register
 */
#define KZQ_DATA_BYTE		0x00ff

/*
 * KZQ DMA Control Register
 */
#define KZQ_DMCTRL_REQ_ACK	0x0003

/*
 * KZQ DMA Length of Transfer Counter
 */
#define KZQ_DMLOTC_COUNT	0x1fff

/*
 * KZQ DMA Address Registers
 */
#define KZQ_DMADDRL		0xffff
#define KZQ_DMADDRH		0x0003

/*
 * KZQ DMA Initial Byte Register
 */
#define KZQ_DMABYTE		0x00ff

/*
 * KZQ Short Target List Pointer
 */
#define KZQ_STLP		0xfffe

/*
 * KZQ Long Target List Pointer
 */
#define KZQ_LTLP		0xfffe

/*
 * KZQ Initiator List Pointer
 */
#define KZQ_ILP			0xfffe

/*
 * KZQ DSSI Control Register
 */
#define KZQ_DSCTRL_DSI		0x8000
#define KZQ_DSCTRL_OUT		0x4000
#define KZQ_DSCTRL_CH7		0x0080
#define KZQ_DSCTRL_CH6		0x0040
#define KZQ_DSCTRL_CH5		0x0020
#define KZQ_DSCTRL_CH4		0x0010
#define KZQ_DSCTRL_CH3		0x0008
#define KZQ_DSCTRL_CH2		0x0004
#define KZQ_DSCTRL_CH1		0x0002
#define KZQ_DSCTRL_CH0		0x0001

/*
 * KZQ Conection Status Register
 */
#define KZQ_CSTAT_CI		0x8000
#define KZQ_CSTAT_DI		0x4000
#define KZQ_CSTAT_RST		0x2000
#define KZQ_CSTAT_BER		0x1000
#define KZQ_CSTAT_OBC		0x0800
#define KZQ_CSTAT_TZ		0x0400
#define KZQ_CSTAT_BUF		0x0200
#define KZQ_CSTAT_LDN		0x0100
#define KZQ_CSTAT_SCH		0x0080
#define KZQ_CSTAT_CON		0x0040
#define KZQ_CSTAT_DST		0x0020
#define KZQ_CSTAT_TGT		0x0010
#define KZQ_CSTAT_SWA		0x0008
#define KZQ_CSTAT_SIP		0x0004
#define KZQ_CSTAT_LST		0x0002
#define KZQ_CSTAT_STATE		0x0078

/*
 * KZQ Data Transfer Status Register
 */
#define KZQ_DSTAT_CI		0x8000
#define KZQ_DSTAT_DI		0x4000
#define KZQ_DSTAT_DNE		0x2000
#define KZQ_DSTAT_TCZ		0x1000
#define KZQ_DSTAT_TBE		0x0800
#define KZQ_DSTAT_IBF		0x0400
#define KZQ_DSTAT_IPE		0x0200
#define KZQ_DSTAT_OBB		0x0100
#define KZQ_DSTAT_MIS		0x0010
#define KZQ_DSTAT_ATN		0x0008
#define KZQ_DSTAT_MSG		0x0004
#define KZQ_DSTAT_CD		0x0002
#define KZQ_DSTAT_IO		0x0001
#define KZQ_DSTAT_PHASE		0x0007

/*
 * KZQ Command Register
 */
#define KZQ_COMM_DMA		0x8000
#define KZQ_COMM_RST		0x4000
#define KZQ_COMM_RSL		0x1000
#define KZQ_COMM_CMD		0x0f80
#define KZQ_COMM_CHIP_RST	0x0080
#define KZQ_COMM_DISCON		0x0100
#define KZQ_COMM_REQ_DATA	0x0200
#define KZQ_COMM_SELECT		0x0400
#define KZQ_COMM_INFO_XFER	0x0800
#define KZQ_COMM_CON		0x0040
#define KZQ_COMM_ORI		0x0020
#define KZQ_COMM_TGT		0x0010
#define KZQ_COMM_ATN		0x0008
#define KZQ_COMM_MSG		0x0004
#define KZQ_COMM_CD		0x0002
#define KZQ_COMM_IO		0x0001
#define KZQ_COMM_SELECT_ATN     0x0408 

/*
 * KZQ Diagnostic Control Register
 */
#define KZQ_DICTRL_LPB		0x0008
#define KZQ_DICTRL_PRE		0x0004
#define KZQ_DICTRL_DIA		0x0002
#define KZQ_DICTRL_TST		0x0001

/*
 * KZQ Bus Handler Diagnostic Register
 */
#define KZQ_BHDIAG_SDT		0x8000
#define KZQ_BHDIAG_ENX		0x4000
#define KZQ_BHDIAG_MAT		0x2000
#define KZQ_BHDIAG_PHS		0x1000
#define KZQ_BHDIAG_200		0x0800
#define KZQ_BHDIAG_T25		0x0400
#define KZQ_BHDIAG_Q9		0x0200
#define KZQ_BHDIAG_Q4		0x0100
#define KZQ_BHDIAG_DTO		0x0080
#define KZQ_BHDIAG_TCR		0x0040
#define KZQ_BHDIAG_DCR		0x0020
#define KZQ_BHDIAG_ABT		0x0010
#define KZQ_BHDIAG_SMR		0x0008
#define KZQ_BHDIAG_RST		0x0004
#define KZQ_BHDIAG_BM		0x0003

/*
 * KZQ SCSI IO Diagnostic Register
 */
#define KZQ_SIDIAG_DIR		0x8000
#define KZQ_SIDIAG_DPH		0x4000
#define KZQ_SIDIAG_ISS		0x2000
#define KZQ_SIDIAG_TAK		0x1000
#define KZQ_SIDIAG_ODR		0x0800
#define KZQ_SIDIAG_SNT		0x0400
#define KZQ_SIDIAG_REQ		0x0200
#define KZQ_SIDIAG_IDR		0x0100
#define KZQ_SIDIAG_MAT		0x0080
#define KZQ_SIDIAG_WON		0x0040
#define KZQ_SIDIAG_IDL		0x0020
#define KZQ_SIDIAG_ERR		0x0010
#define KZQ_SIDIAG_OF		0x000c
#define KZQ_SIDIAG_FI		0x0003

/*
 * KZQ Data Mover Diagnostic Register
 */
#define KZQ_DMDIAG_CLR		0x0040
#define KZQ_DMDIAG_LD6		0x0020
#define KZQ_DMDIAG_LTC		0x0010
#define KZQ_DMDIAG_CHI		0x0008
#define KZQ_DMDIAG_CLO		0x0004
#define KZQ_DMDIAG_ENH		0x0002
#define KZQ_DMDIAG_RDY		0x0001

/*
 * KZQ Main Control Diagnostic Register
 */
#define KZQ_MCDIAG_LISTSM	0x3c00
#define KZQ_MCDIAG_MAIN		0x0380
#define KZQ_MCDIAG_XF		0x0030
#define KZQ_MCDIAG_LCTRL	0x000f

/*
 * The SIMKZQINTR structure is used in conjunction with the State Machine's
 * structure, SIM_SM.  SIMKZQINTR contains fields for all interrupt 
 * information which should be read at interrupt time.  This will allow
 * the interrupt to be handled at a lower IPL via the State Machine.
 */
#define  KZQ_INTR_VERS 0x1
typedef struct KZQ_INTR_TYPE {
    u_short csr;	/* Control Status Register       		*/
    u_short cstat;	/* Connection State Register     		*/
    u_short dstat;	/* Data Transfer Status Register 		*/
    u_short orig_dstat;	/* Data Transfer Status Register 		*/
    u_short comm;	/* Command Register		 		*/
    u_short sc1;	/* SCSI Control Signal 1 Register		*/
    u_short sdb;	/* Data bus register				*/
    u_short new_state;	/* Whether this interrupt signals a new state 	*/
} KZQ_INTR;

/*
 * HBA specific SOFTC structure.  A pointer is provided in SIM_SOFTC
 * to point to this structure (hba_sc).
 */
#define KZQ_MULTI_STEP 0x00000001	/* Multi-step operation in progress */
#define SIMKZQ_SOFTC_VERS 0x1

/*
 * SIMKZQ_SOFTC flags defines.
 */
#define SIMKZQ_CLR_ATN	0x0000001L
#define SIMKZQ_SET_ATN	0x0000002L

typedef struct SIMKZQ_SOFTC_TYPE {
    U32 flags;	/* HBA specific softc flags			*/
    SIM_WS *sws;	/* SIM_WS which the command was related to	*/
    U32 target;	/* Target command was issued to			*/
    U32 lun;		/* LUN command was issued to			*/
    U32 phase;	/* Phase last interrupt was at			*/
    CIR_Q intrq;	/* Q for keeping interrupt data			*/
    U32 pend_int;	/* Set to 1 if HBA interrupt pending		*/
    U32 which_int;   /* The KZQ command associated with an interrupt */
    U32 last_int_phase; /* DSTAT phase bits, synch with softclk   	*/
    struct KZQ_INTR_TYPE *last_kzq_intr;
    			/* Ptr to interrupt context of last inter 	*/
    struct KZQ_INTR_TYPE *last_dismissed_kzq_intr;
    KZQ_INTR intrq_buf[SM_QUEUE_SZ]; /* Buffer for interrupt data	*/
    KZQ_INTR *active_intr; /* Current active interrupt data		*/
    U32 cnt_resched_sel;	/* Count of selections rescheduled	*/
    U32 cnt_sel_tmo;		/* Count of timedout reselections	*/
    U32 cnt_bus_resets;	/* Count of iniated bus resets		*/
    U32 cnt_resets_seen;	/* Count of bus resets detected		*/
    void (*chip_reset)(); /* Reset the DEC KZQ, don't do a bus reset.	*/
} SIMKZQ_SOFTC;



#endif /* _SIM_KZQ_ */


