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
 * @(#)$RCSfile: sim_sii.h,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/06/25 17:52:22 $
 */
#ifndef _SIM_SII_
#define _SIM_SII_

/* ---------------------------------------------------------------------- */

/* sim_sii.h		Version 1.04			Dec 3, 1991    */

/*  This file contains the definitions and data structures needed by the
    DEC SII SIM module.

Modification History

	Version	  Date		Who	

	1.04	12/03/91	janet
	Added SIMSII_REG_VERS define.
	
	1.03    11/15/91	rln
	Add SIMSII_SOFTC_VERS symbols.

	1.02	10/22/91	rln	
	PreEFT changes: 1) Make registers volatile

	1.01	8/12/91		rln	
	Added new symbols

	1.00	3/14/91		rln
	Created this file.
*/

/* ---------------------------------------------------------------------- */

/*
 * SII specific constants:
 *
 */

#define SIM_KN01_SYNC_OFFSET 0x3	/* Maximum sdtr offset for this chip */
#define SIM_KN01_SYNC_PERIOD 63		/* Transfer period from old driver   */
  

/*
 * Macro Name : SIMSII_GET_INTR
 *
 * Macro Description :
 *	SIMSII_GET_INTR() will get a buffer from the interrupt queue
 *	kept for the SII.  This function will increment the current
 *	index so that the next call to this macro will return the
 *	next buffer.
 */
#define SIMSII_GET_INTR(ssc, intr);					\
{									\
	intr = &(CIRQ_GET_CURR((ssc)->intrq, (ssc)->intrq_buf));	\
	(ssc)->intrq.curr = CIRQ_INC((ssc)->intrq, (ssc)->intrq.curr);	\
}

/*
 * Macros to store and retrieve the last command.
 */
#define SIMSII_STORE_CMD(sws, cmd); (sws)->hba_data[0] = (U_WORD) cmd;
#define SIMSII_LAST_CMD(sws) (sws)->hba_data[0]

/*
 * Get the HBA specific interrupt data from the SIM State Machine's
 * structure.
 */
/*#define SIMSII_GET_CSR(hba_intr) (hba_intr)->csr*/
#define SIMSII_GET_CSAT(hba_intr) (hba_intr)->cstat
#define SIMSII_GET_DSTAT(hba_intr) (hba_intr)->dstat

/*
 * Get the HBA specific DMA address, RAM buffer address, and CSR.
 */
#define SIMSII_GET_RAM_BUF(softc) (void *)((int)(softc)->reg + 0x1000000)
#define SIMSII_GET_CSR(sc) ((SIMSII_REG *)(sc)->reg)

/*
 * Macro Name : SIMSII_GET/PUT_BYTE
 *
 * Macro Description :
 *	SIMSII_GET/PUT_BYTE will get or put a message byte into or out of
 * the SII data register.
 */
#define SIMSII_GET_BYTE(byte,reg);					\
{								        \
  (byte) = (u_char) (reg)->sdb;\
}

#define SIMSII_PUT_BYTE(byte,reg);					\
{							        	\
 (reg)->data = (u_short) (byte);		                        \
}

/*
 * Macro Name : SIMSII_READ_MESSAGE
 *
 * Macro Description :
 *	SIMSII_READ_MESSAGE will read a message byte from the FIFO
 *	and put it in the message in queue.
 */
#define SIMSII_READ_MESSAGE(sws, reg);					\
{									\
    if (SIMSII_GET_FIFO_LN(reg) == 0) {					\
    	printf("(SIMSII_READ_MESSAGE) Zero FIFO length!\n");		\
    }									\
    SC_ADD_MSGIN(sws, (reg)->data);				\
}

/*
 * Macro Name : SIMSII_READ_STATUS
 *
 * Macro Description :
 *	SIMSII_READ_STATUS will read a status byte from the FIFO
 *	and save it in the SIM_WS "scsi_status" field.
 */
#define SIMSII_READ_STATUS(sws, reg);					\
{									\
(reg)->data);				\
}


/*
 * This macro is executed, before returning from a code segment that is request
 * ing an interrupt from the SII. Here we will record that we are indeed 
 * waiting  for an interrupt and which command was issued to the SII.
 */

#define CMD_PENDING(softc,cmd,reg) {((SIMSII_SOFTC*)softc)->pend_int = 1;\
    ((SIMSII_SOFTC*)softc)->which_int=(cmd);\
    (reg)->comm = (cmd);}



#define CLEAR_PENDING(softc,cmd) {((SIMSII_SOFTC*)softc)->pend_int = (cmd);\
				      ((SIMSII_SOFTC*)softc)->which_int=(cmd);}


/*
 * Locking macros for the SIM SII.
 */

/*#define SC_LOCK(s,lock_handle)   {printf("Locked %d\n");s = splbio();}*/
#define SC_LOCK(s,lock_handle) 		(s) = splbio()

/*#define SC_UNLOCK(s,lock_handle) {printf("Unlocked\n");splx(s);}*/
#define SC_UNLOCK(s,lock_handle) 	splx(s)

/*
 * SII register set.
 */
#define SIMSII_REG_VERS 1
typedef struct {
    volatile u_short sdb;		/* SCSI Data Bus and Parity */
    volatile u_short pad0;
    volatile u_short sc1;		/* SCSI Control Signals One */
    volatile u_short pad1;
    volatile u_short sc2;		/* SCSI Control Signals Two */
    volatile u_short pad2;
    volatile u_short csr;		/* Control/Status register */
    volatile u_short pad3;
    volatile u_short id;			/* Bus ID register */
    volatile u_short pad4;
    volatile u_short slcsr;		/* Select Control and Status Register */
    volatile u_short pad5;
    volatile u_short destat;		/* Selection Detector Status Register */
    volatile u_short pad6;
    volatile u_short dstmo;		/* DSSI Timeout Register */
    volatile u_short pad7;
    volatile u_short data;		/* Data Register */
    volatile u_short pad8;
    volatile u_short dmctrl;		/* DMA Control Register */
    volatile u_short pad9;
    volatile u_short dmlotc;		/* DMA Length of Transfer Counter */
    volatile u_short pad10;
    volatile u_short dmaddrl;		/* DMA Address Register Low */
    volatile u_short pad11;
    volatile u_short dmaddrh;		/* DMA Address Register High */
    volatile u_short pad12;
    volatile u_short dmabyte;		/* DMA Initial Byte Register */
    volatile u_short pad13;
    volatile u_short stlp;		/* DSSI Short Target List Pointer */
    volatile u_short pad14;
    volatile u_short ltlp;		/* DSSI Long Target List Pointer */
    volatile u_short pad15;
    volatile u_short ilp;		/* DSSI Initiator List Pointer */
    volatile u_short pad16;
    volatile u_short dsctrl;		/* DSSI Control Register */
    volatile u_short pad17;
    volatile u_short cstat;		/* Connection Status Register */
    volatile u_short pad18;
    volatile u_short dstat;		/* Data Transfer Status Register */
    volatile u_short pad19;
    volatile u_short comm;		/* Command Register */
    volatile u_short pad20;
    volatile u_short dictrl;		/* Diagnostic Control Register */
    volatile u_short pad21;
    volatile u_short clock;		/* Diagnostic Clock Register */
    volatile u_short pad22;
    volatile u_short bhdiag;		/* Bus Handler Diagnostic Register */
    volatile u_short pad23;
    volatile u_short sidiag;		/* SCSI IO Diagnostic Register */
    volatile u_short pad24;
    volatile u_short dmdiag;		/* Data Mover Diagnostic Register */
    volatile u_short pad25;
    volatile u_short mcdiag;		/* Main Control Diagnostic Register */
    volatile u_short pad26;
} SIMSII_REG;

/*
 * SII phase defines.
 */
#define SIMSII_PHASE_DATAOUT	0x0000
#define SIMSII_PHASE_DATAIN	0x0001
#define SIMSII_PHASE_COMMAND	0x0002
#define SIMSII_PHASE_STATUS	0x0003
#define SIMSII_PHASE_MSGOUT	0x0006
#define SIMSII_PHASE_MSGIN	0x0007

/*
 * SII SCSI Data Bus register bit defines.
 */
#define SII_SDB_PTY		0x0100
#define SII_SDB_DATA		0x00ff

/*
 * SII SCSI Control Signals One
 */
#define SII_SC1_BSY		0x0100
#define SII_SC1_SEL		0x0080
#define SII_SC1_RST		0x0040
#define SII_SC1_ACK		0x0020
#define SII_SC1_REQ		0x0010
#define SII_SC1_ATN		0x0008
#define SII_SC1_MSG		0x0004
#define SII_SC1_CD		0x0002
#define SII_SC1_IO		0x0001

/*
 * SII SCSI Control Signals Two
 */
#define SII_SC2_IGS		0x0008
#define SII_SC2_TGS		0x0004
#define SII_SC2_ARB		0x0002
#define SII_SC2_SBE		0x0001

/*
 * SII Control/Status Register
 */
#define SII_CSR_HPM		0x0010
#define SII_CSR_RSE		0x0008
#define SII_CSR_SLE		0x0004
#define SII_CSR_PCE		0x0002
#define SII_CSR_IE		0x0001

/*
 * SII Bus ID Register
 */
#define SII_ID_BUSID		0x0007
#define SII_ID_IO		0x8000

/*
 * SII Selector Control and Status Register
 */
#define SII_SLCSR_BUSID		0x0007

/*
 * SII Selection Detector Status Register
 */
#define SII_DESTAT_BUSID	0x0007

/*
 * SII DSSI Timeout Register
 */
#define SII_DSTMO_ENA		0x8000
#define SII_DSTMO_TARGTO	0x00f0
#define SII_DSTMO_INITTO	0x000f

/*
 * SII Data Register
 */
#define SII_DATA_BYTE		0x00ff

/*
 * SII DMA Control Register
 */
#define SII_DMCTRL_REQ_ACK	0x0003

/*
 * SII DMA Length of Transfer Counter
 */
#define SII_DMLOTC_COUNT	0x1fff

/*
 * SII DMA Address Registers
 */
#define SII_DMADDRL		0xffff
#define SII_DMADDRH		0x0003

/*
 * SII DMA Initial Byte Register
 */
#define SII_DMABYTE		0x00ff

/*
 * SII Short Target List Pointer
 */
#define SII_STLP		0xfffe

/*
 * SII Long Target List Pointer
 */
#define SII_LTLP		0xfffe

/*
 * SII Initiator List Pointer
 */
#define SII_ILP			0xfffe

/*
 * SII DSSI Control Register
 */
#define SII_DSCTRL_DSI		0x8000
#define SII_DSCTRL_OUT		0x4000
#define SII_DSCTRL_CH7		0x0080
#define SII_DSCTRL_CH6		0x0040
#define SII_DSCTRL_CH5		0x0020
#define SII_DSCTRL_CH4		0x0010
#define SII_DSCTRL_CH3		0x0008
#define SII_DSCTRL_CH2		0x0004
#define SII_DSCTRL_CH1		0x0002
#define SII_DSCTRL_CH0		0x0001

/*
 * SII Conection Status Register
 */
#define SII_CSTAT_CI		0x8000
#define SII_CSTAT_DI		0x4000
#define SII_CSTAT_RST		0x2000
#define SII_CSTAT_BER		0x1000
#define SII_CSTAT_OBC		0x0800
#define SII_CSTAT_TZ		0x0400
#define SII_CSTAT_BUF		0x0200
#define SII_CSTAT_LDN		0x0100
#define SII_CSTAT_SCH		0x0080
#define SII_CSTAT_CON		0x0040
#define SII_CSTAT_DST		0x0020
#define SII_CSTAT_TGT		0x0010
#define SII_CSTAT_SWA		0x0008
#define SII_CSTAT_SIP		0x0004
#define SII_CSTAT_LST		0x0002
#define SII_CSTAT_STATE		0x0078

/*
 * SII Data Transfer Status Register
 */
#define SII_DSTAT_CI		0x8000
#define SII_DSTAT_DI		0x4000
#define SII_DSTAT_DNE		0x2000
#define SII_DSTAT_TCZ		0x1000
#define SII_DSTAT_TBE		0x0800
#define SII_DSTAT_IBF		0x0400
#define SII_DSTAT_IPE		0x0200
#define SII_DSTAT_OBB		0x0100
#define SII_DSTAT_MIS		0x0010
#define SII_DSTAT_ATN		0x0008
#define SII_DSTAT_MSG		0x0004
#define SII_DSTAT_CD		0x0002
#define SII_DSTAT_IO		0x0001
#define SII_DSTAT_PHASE		0x0007

/*
 * SII Command Register
 */
#define SII_COMM_DMA		0x8000
#define SII_COMM_RST		0x4000
#define SII_COMM_RSL		0x1000
#define SII_COMM_CMD		0x0f80
#define SII_COMM_CHIP_RST	0x0080
#define SII_COMM_DISCON		0x0100
#define SII_COMM_REQ_DATA	0x0200
#define SII_COMM_SELECT		0x0400
#define SII_COMM_INFO_XFER	0x0800
#define SII_COMM_CON		0x0040
#define SII_COMM_ORI		0x0020
#define SII_COMM_TGT		0x0010
#define SII_COMM_ATN		0x0008
#define SII_COMM_MSG		0x0004
#define SII_COMM_CD		0x0002
#define SII_COMM_IO		0x0001
#define SII_COMM_CHIP_RST	0x0080
#define SII_COMM_SELECT_ATN     0x0408 

/*
 * SII Diagnostic Control Register
 */
#define SII_DICTRL_LPB		0x0008
#define SII_DICTRL_PRE		0x0004
#define SII_DICTRL_DIA		0x0002
#define SII_DICTRL_TST		0x0001

/*
 * SII Bus Handler Diagnostic Register
 */
#define SII_BHDIAG_SDT		0x8000
#define SII_BHDIAG_ENX		0x4000
#define SII_BHDIAG_MAT		0x2000
#define SII_BHDIAG_PHS		0x1000
#define SII_BHDIAG_200		0x0800
#define SII_BHDIAG_T25		0x0400
#define SII_BHDIAG_Q9		0x0200
#define SII_BHDIAG_Q4		0x0100
#define SII_BHDIAG_DTO		0x0080
#define SII_BHDIAG_TCR		0x0040
#define SII_BHDIAG_DCR		0x0020
#define SII_BHDIAG_ABT		0x0010
#define SII_BHDIAG_SMR		0x0008
#define SII_BHDIAG_RST		0x0004
#define SII_BHDIAG_BM		0x0003

/*
 * SII SCSI IO Diagnostic Register
 */
#define SII_SIDIAG_DIR		0x8000
#define SII_SIDIAG_DPH		0x4000
#define SII_SIDIAG_ISS		0x2000
#define SII_SIDIAG_TAK		0x1000
#define SII_SIDIAG_ODR		0x0800
#define SII_SIDIAG_SNT		0x0400
#define SII_SIDIAG_REQ		0x0200
#define SII_SIDIAG_IDR		0x0100
#define SII_SIDIAG_MAT		0x0080
#define SII_SIDIAG_WON		0x0040
#define SII_SIDIAG_IDL		0x0020
#define SII_SIDIAG_ERR		0x0010
#define SII_SIDIAG_OF		0x000c
#define SII_SIDIAG_FI		0x0003

/*
 * SII Data Mover Diagnostic Register
 */
#define SII_DMDIAG_CLR		0x0040
#define SII_DMDIAG_LD6		0x0020
#define SII_DMDIAG_LTC		0x0010
#define SII_DMDIAG_CHI		0x0008
#define SII_DMDIAG_CLO		0x0004
#define SII_DMDIAG_ENH		0x0002
#define SII_DMDIAG_RDY		0x0001

/*
 * SII Main Control Diagnostic Register
 */
#define SII_MCDIAG_LISTSM	0x3c00
#define SII_MCDIAG_MAIN		0x0380
#define SII_MCDIAG_XF		0x0030
#define SII_MCDIAG_LCTRL	0x000f

/*
 * The SIMSIIINTR structure is used in conjunction with the State Machine's
 * structure, SIM_SM.  SIMSIIINTR contains fields for all interrupt 
 * information which should be read at interrupt time.  This will allow
 * the interrupt to be handled at a lower IPL via the State Machine.
 */
#define  SII_INTR_VERS 0x1
typedef struct SII_INTR_TYPE {
    u_short csr;	/* Control Status Register       		*/
    u_short cstat;	/* Connection State Register     		*/
    u_short dstat;	/* Data Transfer Status Register 		*/
    u_short orig_dstat;	/* Data Transfer Status Register 		*/
    u_short comm;	/* Command Register		 		*/
    u_short sc1;	/* SCSI Control Signal 1 Register		*/
    u_short sdb;	/* Data bus register				*/
    u_short new_state;	/* Whether this interrupt signals a new state 	*/
} SII_INTR;

/*
 * HBA specific SOFTC structure.  A pointer is provided in SIM_SOFTC
 * to point to this structure (hba_sc).
 */
#define SII_MULTI_STEP 0x00000001	/* Multi-step operation in progress */
#define SIMSII_SOFTC_VERS 0x1

/*
 * SIMSII_SOFTC flags defines.
 */
#define SIMSII_CLR_ATN	0x0000001L
#define SIMSII_SET_ATN	0x0000002L

typedef struct SIMSII_SOFTC_TYPE {
    U32 flags;		/* HBA specific softc flags			*/
    SIM_WS *sws;	/* SIM_WS which the command was related to	*/
    U32 target;		/* Target command was issued to			*/
    U32 lun;		/* LUN command was issued to			*/
    U32 phase;		/* Phase last interrupt was at			*/
    CIR_Q intrq;	/* Q for keeping interrupt data			*/
    U32 pend_int;	/* Set to 1 if HBA interrupt pending		*/
    U32 which_int;   	/* The SII command associated with an interrupt */
    U32 last_int_phase; /* DSTAT phase bits, synch with softclk   	*/
    struct SII_INTR_TYPE *last_sii_intr;
    			/* Ptr to interrupt context of last inter 	*/
    struct SII_INTR_TYPE *last_dismissed_sii_intr;
    SII_INTR intrq_buf[SM_QUEUE_SZ]; /* Buffer for interrupt data	*/
    SII_INTR *active_intr; 	/* Current active interrupt data	*/
    U32 cnt_resched_sel;	/* Count of selections rescheduled	*/
    U32 cnt_sel_tmo;		/* Count of timedout reselections	*/
    U32 cnt_bus_resets;		/* Count of iniated bus resets		*/
    U32 cnt_resets_seen;	/* Count of bus resets detected		*/
    void (*chip_reset)(); /* Reset the DEC SII, don't do a bus reset.	*/
} SIMSII_SOFTC;



#endif /* _SIM_SII_ */

