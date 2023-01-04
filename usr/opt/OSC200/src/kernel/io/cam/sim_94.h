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
 * @(#)$RCSfile: sim_94.h,v $ $Revision: 1.1.10.5 $ (DEC) $Date: 1993/11/23 21:51:23 $
 */
#ifndef _SIM_94_
#define _SIM_94_

#include <io/common/devdriver.h>

/* ---------------------------------------------------------------------- */

/* sim_94.h		Version 1.04			Nov. 13, 1991 */

/*  This file contains the definitions and data structures needed by the
    NCR53C94 SIM module.

Modification History

	1.04	11/13/91	janet
	Added clock conversion macros.  Maded registers volatile.
        Added fields to SIM94_INTR. Added VERS.

	1.03	10/28/91	janet
	Added clock conversion define and transfer period defines.

	1.02	10/22/91	janet
	o Added SIM94_READ_MESSAGE_OUT() macro for target mode.
	o Added SIM94_SSTEP_TARG_MSGO define.

	1.01	03/26/91	janet
	Updated after code review.

	1.00	11/09/90	janet
	Created this file.
*/

/* ---------------------------------------------------------------------- */

/*
 * Macro Name :  SIM94_LOAD_FIFO_BYTE
 *
 * Macro Description :
 *	SIM94_LOAD_FIFO() will load the specified number of bytes into
 *	the 94's FIFO.  No error checking is done in this macro.
 */
#define SIM94_LOAD_FIFO_BYTE(reg, byte) (reg)->sim94_fifo = (byte)

/*
 * Macro Name : SIM94_FLUSH_FIFO
 *
 * Macro Description :
 *      Flush the FIFO.  A NOP command is also performed.  There may be
 *      a problem with the 94 taking too long to perform the flush.
 *      The NOP is just in case.
 */
#define SIM94_FLUSH_FIFO(reg) {					\
   (reg)->sim94_cmd = SIM94_CMD_FFIFO;				\
   WBFLUSH();							\
   (reg)->sim94_cmd = SIM94_CMD_NOP;				\
   WBFLUSH();							\
}

/*
 * Macro Name : SIM94_GET_INTR
 *
 * Macro Description :
 *	SIM94_GET_INTR() will get a buffer from the interrupt queue
 *	kept for the 94.  This function will increment the current
 *	index so that the next call to this macro will return the
 *	next buffer.
 */
#define SIM94_GET_INTR(ssc, intr);					\
{									\
	(intr) = &(CIRQ_GET_CURR((ssc)->intrq, (ssc)->intrq_buf));	\
	(ssc)->intrq.curr = CIRQ_INC((ssc)->intrq, (ssc)->intrq.curr);	\
}

/*
 * Macros to store and retrieve the last command.
 */
#define SIM94_STORE_CMD(sws, cmd); (sws)->hba_data[0] = (U_WORD)(cmd);
#define SIM94_LAST_CMD(sws) (sws)->hba_data[0]

/*
 * Get the HBA specific interrupt data from the SIM State Machine's
 * structure.
 */
#define SIM94_GET_SSR(hba_intr) (hba_intr)->ssr
#define SIM94_GET_SR(hba_intr) (hba_intr)->sr
#define SIM94_GET_IR(hba_intr) (hba_intr)->ir

/*
 * Get the HBA specific DMA address, RAM buffer address, and CSR.
 */
#define SIM94_DMAADDR_OFFSET	0x40000
#define SIM94_RAM_BUF_OFFSET	0x80000

/* for PMAZ-AA support on Alpha: the ram buffer is accessed through
 * dense space on Flamingo, but the chip and DMA registers are
 * still accessed though sparse.
 */
#ifdef __alpha

/* convert a sparse slot base address into its dense equivilent.
 * caution: this works only for slot *base* addresses
 */
#ifndef DENSE
#define DENSE(x)	((x) - 0x10000000)
#endif

#define SIM94_GET_DMAADDR(softc) \
    (void *)(DENSE((unsigned long)(softc)->reg) + SIM94_DMAADDR_OFFSET)
#define SIM94_GET_RAM_BUF(softc) \
    (void *)(DENSE((unsigned long)(softc)->reg) + SIM94_RAM_BUF_OFFSET)
#else
#define SIM94_GET_DMAADDR(softc) \
    (void *)((unsigned long)(softc)->reg + SIM94_DMAADDR_OFFSET)
#define SIM94_GET_RAM_BUF(softc) \
    (void *)((unsigned long)(softc)->reg + SIM94_RAM_BUF_OFFSET)
#endif

#define SIM94_GET_CSR(sc) (SIM94_REG *)(sc)->reg

/*
 * 94 FIFO access macros
 */
#define SIM94_GET_FIFO_LN(reg) ((reg)->sim94_ffss & 0x1f)
#define SIM94_READ_FIFO(reg) (reg)->sim94_fifo

/*
 * Macro Name : SIM94_SEND_CMD
 *
 * Macro Description :
 *	SIM94_SEND_CMD will store the specified command in the
 *	SIM94_SOFTC "command" field.  It will then load the chip's
 *	"cmd" register with this command.  This macro should be
 *	called with high IPL.
 */
#define SIM94_SEND_CMD(sws, reg, action, poll);				\
{									\
    SIM94_STORE_CMD(sws, action);					\
    (reg)->sim94_cmd = (action);					\
    WBFLUSH();								\
    if (poll) {								\
         while(!((reg)->sim94_stat & SIM94_STAT_INT));			\
	 ascintr((short)sws->cntlr);					\
    }									\
}

/*
 * Macro Name : SIM94_READ_MESSAGE
 *
 * Macro Description :
 *	SIM94_READ_MESSAGE will read a message byte from the FIFO
 *	and put it in the message in queue.
 */
#define SIM94_READ_MESSAGE(sws, reg);					\
{									\
    if (SIM94_GET_FIFO_LN(reg) == 0) {					\
    	printf("(SIM94_READ_MESSAGE) Zero FIFO length!\n");		\
    }									\
    SC_ADD_MSGIN(sws, (reg)->sim94_fifo);				\
}

/*
 * Macro Name : SIM94_READ_MESSAGE_OUT
 *
 * Macro Description :
 *	SIM94_READ_MESSAGE_OUT will read a message byte from the FIFO
 *	and put it in the message out queue.  For target mode.
 */
#define SIM94_READ_MESSAGE_OUT(sws, reg);				\
{									\
    if (SIM94_GET_FIFO_LN(reg) == 0) {					\
    	printf("(SIM94_READ_MESSAGE_OUT) Zero FIFO length!\n");		\
	sim_break(); 							\
    }									\
    SC_ADD_MSGOUT(sws, (reg)->sim94_fifo);				\
}

/*
 * Macro Name : SIM94_READ_STATUS
 *
 * Macro Description :
 *	SIM94_READ_STATUS will read a status byte from the FIFO
 *	and save it in the SIM_WS "scsi_status" field.
 */
#define SIM94_READ_STATUS(sws, reg);					\
{									\
    if (SIM94_GET_FIFO_LN(reg) == 0)					\
    	printf("(SIM94_READ_MESSAGE) Zero FIFO length!\n");		\
    (sws)->scsi_status = (reg)->sim94_fifo;				\
}

/*
 * NCR53C94 Command set.  Used with the asc_cmd register below.
 */
#define SIM94_CMD_NOP		0x00 /* No Operation			*/
#define SIM94_CMD_FFIFO		0x01 /* Flush the FIFO			*/
#define SIM94_CMD_RSTCHIP	0x02 /* Reset the Chip			*/
#define SIM94_CMD_RSTBUS	0x03 /* Reset the SCSI Bus		*/
#define SIM94_CMD_RESEL		0x40 /* Reselect sequence		*/
#define SIM94_CMD_SELWOATN	0x41 /* Select without ATN sequence	*/
#define SIM94_CMD_SELECT	0x42 /* Select with ATN sequence	*/
#define SIM94_CMD_SELSTOP	0x43 /* Select with ATN and stop seq.	*/
#define SIM94_CMD_ENSEL		0x44 /* Enable selection/reselection	*/
#define SIM94_CMD_DISSEL	0x45 /* Disable selection/reselection	*/
#define SIM94_CMD_SELATN3	0x46 /* Select with ATN3		*/
#define SIM94_CMD_SNDMSG	0x20 /* Send message			*/
#define SIM94_CMD_SNDSTAT	0x21 /* Send status			*/
#define SIM94_CMD_SNDDATA	0x22 /* Send data			*/
#define SIM94_CMD_DISCSEQ	0x23 /* Disconnect sequence		*/
#define SIM94_CMD_TERMSEQ	0x24 /* Terminate sequence		*/
#define SIM94_CMD_TARGCMP	0x25 /* Target command complete sequence*/
#define SIM94_CMD_DISCON	0x27 /* Disconnect			*/
#define SIM94_CMD_RECMSGSEQ	0x28 /* Receive message sequence	*/
#define SIM94_CMD_RECCMD	0x29 /* Receive command			*/
#define SIM94_CMD_RECDATA	0x2a /* Receive data			*/
#define SIM94_CMD_RECCMDSEQ	0x2b /* Receive command sequence	*/
#define SIM94_CMD_TARGABORT	0x04 /* Target abort DMA		*/

#define SIM94_CMD_XFERINFO	0x10 /* Transfer information		*/
#define SIM94_CMD_CMDCMPLT	0x11 /* Initiator command complete seq.	*/
#define SIM94_CMD_MSGACPT	0x12 /* Message accepted		*/
#define SIM94_CMD_XFERPAD	0x18 /* Transfer pad			*/
#define SIM94_CMD_SETATN	0x1a /* Set ATN				*/
#define SIM94_CMD_CLRATN	0x1b /* Clear ATN			*/
#define SIM94_CMD_DMANOP	0x80 /* No Operation			*/
#define SIM94_CMD_DMAXFER	0x90 /* DMA Transfer information	*/
#define SIM94_CMD_DMA		0x80 /* Enable DMA on a given cmd	*/

/*
 * Sequence step defines.
 */
#define SIM94_SSTEP_TARG_MSGO	0x00
#define SIM94_SSTEP_TO		0x00 /* Selection time out		*/
#define SIM94_SSTEP_MSGO	0x01 /* Cmplt msgo phase, 1 byte sent	*/
#define SIM94_SSTEP_NOCMD	0x02 /* Target didn't go to cmd phase	*/
#define SIM94_SSTEP_CMDFAIL	0x03 /* Stopped during command transfer	*/
#define SIM94_SSTEP_CMPLT	0x04 /* Sequence has completed		*/
#define SIM94_SSTEP_MASK	0x07 /* Bit mask for sequence step	*/

/*
 * FIFO flags register.
 */
#define SIM94_FIFO_SIZE		0x0f /* Depth of the FIFO.		*/
#define SIM94_FIFO_MSK		0x1f /* Mask off sequence step info	*/

/*
 * Status Register bit defines.
 */
#define SIM94_STAT_INT		0x80
#define SIM94_STAT_GE		0x40
#define SIM94_STAT_PE		0x20
#define SIM94_STAT_TC		0x10
#define SIM94_STAT_VGC		0x08
#define SIM94_STAT_MSG		0x04
#define SIM94_STAT_CD		0x02
#define SIM94_STAT_IO		0x01
#define SIM94_PHASE_MASK	0x07

/*
 * Interrupt Register bit defines.
 */
#define SIM94_INTR_RST		0x80
#define SIM94_INTR_ILLCMD	0x40
#define SIM94_INTR_DIS		0x20
#define SIM94_INTR_BS		0x10
#define SIM94_INTR_FC		0x08
#define SIM94_INTR_RESEL	0x04
#define SIM94_INTR_SELATN	0x02
#define SIM94_INTR_SEL		0x01

/*
 * Configuration 1 register bit defines.
 */
#define SIM94_CNF1_EPC		0x10	/* Enable Parity Checking	*/

/*
 * Configuration 2 register bit defines.
 */
#define SIM94_CNF2_DPE		0x01	/* Enable DMA parity checking	*/
#define SIM94_CNF2_RPE		0x02	/* Enable Register parity check */

/*
 * Configuration 3 register bit defines.
 */
#define SIMFAST_CNF3_FCLK	0x08	/* tell CF96 fast crystal	*/
#define SIMFAST_CNF3_FSCSI	0x10	/* use fast scsi timing		*/

/*
 * Configuration 4 register bit defines.
 */
#define SIMFAST_CNF4_EAN	0x04	/* enable active negation */

/*
 * Clock conversion factor.  Clock speed in MHz divided by this
 * number is the correct number to load into the clock conversion
 * register.
 */
#define SIM94_CLKCNV		5
#define SIM94_CLKMSK		7		/* only 3 bits */
#define SIM94_CONVERT_CLOCK(speed) \
    ( (speed) % SIM94_CLKCNV ? ((speed) / SIM94_CLKCNV) + 1 : \
                               ((speed) / SIM94_CLKCNV) ) & SIM94_CLKMSK

/*
 * Synchronous Transfer Period
 *
 *	The old formulas assumed the period always divided evenly by the
 *	clock rate.  This caused the transfer period value to be to low
 *	(and therefore the transfer rate to fast) for those devices that
 *	negotiated a rate that was not an even multiple of the clock rate.
 *	So, if the period/clockrate has a remainder, then use the next
 *	slower rate (val+1), else if no remainder, use the calculated val.
 */
#define SIM94_PERIOD_MIN	5
#define SIM94_PERIOD_MASK	0x1f
#define SIM94_PERIOD_CONVERT(val,period)			\
    (period) % (10000L/(U32)ssc->clock) ?			\
	((val)+1 < SIM94_PERIOD_MIN ? SIM94_PERIOD_MIN :	\
		(val)+1 & SIM94_PERIOD_MASK ) :			\
	((val) < SIM94_PERIOD_MIN ? SIM94_PERIOD_MIN :		\
		(val) & SIM94_PERIOD_MASK )			\

/*
 * The CF96 chip has two modes (with minimums for each).  In slow mode
 * the minimum is 8.  In fast mode, the minimum is 4 (see sim_94_fast.c).
 */
#define SIMFAST_MIN_FOR_SLOW	0x08	/* minimum period for slow */
#define SIMFAST_PERIOD_MIN	4
#define SIMFAST_PERIOD_CONVERT(val,period)			\
    (period) % (10000L/(U32)ssc->clock) ?			\
	((val)+1 < SIMFAST_PERIOD_MIN ? SIMFAST_PERIOD_MIN :	\
		(val)+1 & SIM94_PERIOD_MASK ) :			\
	((val) < SIMFAST_PERIOD_MIN ? SIMFAST_PERIOD_MIN :	\
		(val) & SIM94_PERIOD_MASK )

/*
 * The NCR53C94's register definitions.
 */
typedef struct {
    PAD_U32 r_xcntrl;	/* transfer counter LSB				*/
    PAD_U32 r_xcntrh;	/* transfer counter MSB				*/
    PAD_U32 r_fifo;	/* FIFO access register, 16x9 deep		*/
    PAD_U32 r_cmd;	/* command register				*/
    PAD_U32 r_status;	/* status register				*/
    PAD_U32 r_intr;	/* interrupt register				*/
    PAD_U32 r_sequ;	/* sequence step register			*/
    PAD_U32 r_fflag;	/* FIFO flags/sequ step register		*/
    PAD_U32 r_conf1;	/* Configuration 1 register			*/
    PAD_U32 r_rsvd1;
    PAD_U32 r_rsvd2;
    PAD_U32 r_conf2;	/* Configuration 2 register			*/
    PAD_U32 r_conf3;	/* Configuration 3 register			*/
    PAD_U32 r_conf4;	/* Configuration 4 register                     */
    PAD_U32 r_xcntvh;	/* transfer count HSB low, medium, high - CF94  */
    PAD_U32 r_rsvd3;
} RO_SIM94_REG;

typedef struct {
    PAD_U32 w_xcntl;	/* transfer count LSB				*/
    PAD_U32 w_xcnth;	/* transfer count MSB				*/
    PAD_U32 w_fifo;	/* FIFO access register, 16x9 deep		*/
    PAD_U32 w_cmd;	/* command register				*/
    PAD_U32 w_dest;	/* destination bus ID register			*/
    PAD_U32 w_timeout;	/* select/reselect timeout register		*/
    PAD_U32 w_period;	/* synchronous period register			*/
    PAD_U32 w_offset;	/* synchronous offset register			*/
    PAD_U32 w_conf1;	/* Configuration 1 register			*/
    PAD_U32 w_clkcnv;	/* clock conversion register			*/
    PAD_U32 w_testm;	/* test mode register				*/
    PAD_U32 w_conf2;	/* Configuration 2 register			*/
    PAD_U32 w_conf3;	/* Configuration 3 register			*/
    PAD_U32 w_conf4;	/* Configuration 4 register                     */
    PAD_U32 w_xcntvh;	/* transfer count HSB low, medium, high - CF94  */
} WO_SIM94_REG;

/*
 * This union definition is for accessing the registers with a
 * single pointer type.  The following defines are for accessing
 * the registers with a simple name.
 */
typedef union {
    volatile RO_SIM94_REG r;
    volatile WO_SIM94_REG w;
} SIM94_REG;

#define sim94_tclsb	w.w_xcntl.reg	/* transfer count LSB		*/
#define sim94_tcmsb	w.w_xcnth.reg	/* transfer count MSB		*/
#define sim94_fifo	r.r_fifo.reg	/* FIFO acc reg, 16x9 deep	*/
#define sim94_cmd	w.w_cmd.reg	/* command register		*/
#define sim94_stat	r.r_status.reg	/* status register		*/
#define sim94_dbid	w.w_dest.reg	/* dest bus ID reg		*/
#define sim94_ir	r.r_intr.reg	/* interrupt register		*/
#define sim94_srto	w.w_timeout.reg	/* sel/resel timeout reg	*/
#define sim94_ss	r.r_sequ.reg	/* sequence stepreg		*/
#define sim94_sp	w.w_period.reg	/* sync period reg		*/
#define sim94_ffss	r.r_fflag.reg	/* FIFO flags/sequ reg		*/
#define sim94_so	w.w_offset.reg	/* sync offset reg		*/
#define sim94_cnf1	w.w_conf1.reg	/* Configuration 1 register	*/
#define sim94_ccf	w.w_clkcnv.reg	/* clock conversion reg		*/
#define sim94_tm	w.w_testm.reg	/* test mode register		*/
#define sim94_cnf2	w.w_conf2.reg	/* Configuration 2 register	*/
#define sim94_cnf3	w.w_conf3.reg	/* Configuration 3 register	*/
#define sim94_cnf4	w.w_conf4.reg	/* Configuration 4 register     */

/*
 * The SIM94_INTR structure is used in conjunction with the State Machine's
 * structure, SIM_SM.  SIM94_INTR contains fields for all interrupt 
 * information which should be read at interrupt time.  This will allow
 * the interrupt to be handled at a lower IPL via the State Machine.
 *
 * If this structure is modified, increment the SIM94_INTR_VERS number.
 */
typedef struct sim94_intr {
#define SIM94_INTR_VERS 2
    U32 cntl;		/* transfer count LSB				*/
    U32 cnth;		/* transfer count MSB				*/
    U32 cmd;		/* command register				*/
    U32 sr;		/* Status Register				*/
    U32 ir;		/* Interrupt Register				*/
    U32 ssr;		/* Sequence Step Register			*/
    U32 fflag;		/* FIFO flags/sequ reg				*/
    U32 cnf1;		/* Configuration 1 register			*/
    U32 cnf2;		/* Configuration 2 register			*/
    U32 cnf3;		/* Configuration 3 register			*/
} SIM94_INTR;

/*
 * HBA specific SOFTC structure.  A pointer is provided in SIM_SOFTC
 * to point to this structure (hba_sc).
 *
 * If this structure is modified, increment the SIM94_SOFTC_VERS number.
 */
typedef struct sim94_softc {
#define SIM94_SOFTC_VERS 1
    SIM_WS *sws;	/* SIM_WS which the command was related to	*/
    U32 target;		/* Target command was issued to			*/
    U32 lun;		/* LUN command was issued to			*/
    U32 phase;		/* Phase last interrupt was at			*/
    CIR_Q intrq;	/* Q for keeping interrupt data			*/
    SIM94_INTR intrq_buf[SM_QUEUE_SZ]; /* Buffer for interrupt data	*/
    SIM94_INTR *active_intr; /* Current active interrupt data		*/
    U32 clock;		/* Clock speed in MHz * 10 (this is done to 	*
			 * allow for a fraction of a MHz		*/
} SIM94_SOFTC;

#endif /* _SIM_94_ */

