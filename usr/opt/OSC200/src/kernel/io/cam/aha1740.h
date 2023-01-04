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
 * @(#)$RCSfile: aha1740.h,v $ $Revision: 1.1.6.4 $ (DEC) $Date: 1993/07/30 18:31:19 $
 */


/* ---------------------------------------------------------------------- */

/* aha1740.h        Version 1.00		April 2, 1993		  */

/*
   This file contains the definitions and data structures needed by the      
   AHA 1740A/1742A SIM module.  This file is based on Jeff Wong's       
   original aha.h with modification to data structures.                 

   Modification History

   1.00		04/02/93		Theresa Chin
   Created this file.

		04/20/93		Theresa Chin
   Added structure for host adapter inquiry data and the pointer to it
   in the aha_softc structure.

		04/26/93		Theresa Chin
   Moved the next/prev queue pointer fields in AHA_JOB structure to
   the top of the structure instead of in the middle of it.

		05/18/93		Theresa Chin
   Renamed file to aha1740.h from aha174x.h.  
   Added defines for IO access units, bytes, word, longword etc since they
   are no longer defined in devdriver.h.

   Changed valid minimum firmware rev. number to H since that should be
   the case once it is available from Adaptec.
*/

/* ---------------------------------------------------------------------- */
 									     															 	
#ifndef _AHA1740_H_ 
#define _AHA1740_H_ 

/*
 * Bit mask 
 */
#define BIT0 0x0001
#define BIT1 0x0002
#define BIT2 0x0004
#define BIT3 0x0008
#define BIT4 0x0010
#define BIT5 0x0020
#define BIT6 0x0040
#define BIT7 0x0080
#define BIT8 0x0100
#define BIT9 0x0200
#define BITa 0x0400
#define BITb 0x0800
#define BITc 0x1000
#define BITd 0x2000
#define BITe 0x4000
#define BITf 0x8000

/*
 * 	Defines for accessing IO space 
 */
#define IO_BYTE 1
#define IO_WORD 2
#define IO_LONG_WORD 4

/*
 *	Some general defines
 */

#define AHA_MAX_ADDRESS 0x00000000ffffffff
					/* Max. physical address  */

#define AHA_1740A_HID 	0x01009004	/* AHA 1740A Host ID reg. */
#define AHA_1742A_HID	0x02009004	/* AHA 1742A Host ID reg. */
#define AHA_ID_MASK 	0xffffffff	/* AHA 174xA Host ID mask */
#define AHA_MAXSLOT 	6		/* Max. # of controllers  */


/*
 *	 Expansion Board ID Registers address
 */ 
#define AHA_HID      	0xC80	/* Hardware ID 			*/
#define AHA_EBCTRL   	0xC84	/* Expansion board control	*/
#define AHA_PORTADDR 	0xCC0	/* Port address 		*/
#define AHA_BIOSADDR 	0xCC1	/* BIOS address 		*/
#define AHA_INTDEF   	0xCC2	/* Interrupt definition 	*/
#define AHA_SCSIDEF  	0xCC3	/* SCSI definition 		*/
#define AHA_BUSDEF   	0xCC4	/* Bus definition 		*/

/*
 *	Mailbox registers address
 */
#define AHA_MBOXOUT	0xCD0	/* Mailbox out registers used as*
				 * one longword register 	*/
#define AHA_MBOXOUT0 	0xCD0	/* mailbox out 0 byte 		*/
#define AHA_MBOXOUT1 	0xCD1	/* mailbox out 1 byte		*/
#define AHA_MBOXOUT2 	0xCD2	/* mailbox out 2 byte		*/
#define AHA_MBOXOUT3 	0xCD3	/* mailbox out 3 byte		*/

#define AHA_MBOXIN	0xCD8	/* Mailbox in registers used as *
				 * one longword register        */
#define AHA_MBOXIN0  	0xCD8	/* mailbox in 0 byte		*/
#define AHA_MBOXIN1  	0xCD9	/* mailbox in 1 byte		*/
#define AHA_MBOXIN2  	0xCDA	/* mailbox in 2 byte		*/
#define AHA_MBOXIN3  	0xCDB	/* mailbox in 3 byte		*/
 
/* 	
 *	Enhanced Mode (Group 2) control registers address
 */
#define AHA_ATTN     	0xCD4	/* Attention 			*/
#define AHA_G2CNTRL  	0xCD5	/* Control register		*/
#define AHA_G2INTST  	0xCD6	/* Interrupt Status		*/
#define AHA_G2STAT   	0xCD7	/* Status 			*/
#define AHA_G2STAT2  	0xCDC	/* Status 2 			*/

/*
 *	registers bit definitions
 */
/*
 *	Expansion Board Control Register, (EBCTL, ZC84, W/R)
 */
#define EBCTL_ERRST   	BIT2	/* reset HAERR and CDEN, WO	*/
#define EBCTL_HAERR   	BIT1	/* host adapter ERROR, read only*/
#define EBCTL_CDEN   	BIT0	/* 0=host adapter disabled R/W	*/

/*
 *	I/O Port Address, (PORTADDR, ZCC0, W/R)
 *
 *	Bits 5-3 are reserved and 2-0 have meaning only in
 *	Standard Mode.
 */
#define PORTADDR_EI   	BIT7	/* 1 = Enhanced interface	*/
#define PORTADDR_CF   	BIT6	/* 1 = Configure EEPROM		*/
#define PORTADDR_DS	0x0	/* I/O Port address - disable 	*/
#define PORTADDR_130	0x2	/* I/O Port address - 130 	*/
#define PORTADDR_134	0x3	/* I/O Port address - 134 	*/
#define PORTADDR_230	0x4	/* I/O Port address - 230 	*/
#define PORTADDR_234	0x5	/* I/O Port address - 234 	*/
#define PORTADDR_330	0x6	/* I/O Port address - 330 	*/
#define PORTADDR_334	0x7	/* I/O Port address - 334 	*/

/*
 *	Interupt Definition (INTDEF, ZCC2, W/R)
 */
#define INTDEF_INTEN  		BIT4	/* Enable Interrupt		*/
#define INTDEF_INTHIGH 		BIT3	/* Interrupt high true state	*/
#define INTDEF_INTSEL_9 	0	/* interrupt channel 9 in EISA	*/
#define INTDEF_INTSEL_10	1	/* interrupt channel 10 in EISA	*/
#define INTDEF_INTSEL_11 	2	/* interrupt channel 11 in EISA	*/
#define INTDEF_INTSEL_12 	3	/* interrupt channel 12 in EISA	*/
#define INTDEF_INTSEL_14 	5	/* interrupt channel 14 in EISA	*/
#define INTDEF_INTSEL_15 	6	/* interrupt channel 15 in EISA	*/

/*
 *	EISA interrupt level
 */
#define EISA9_INTR       	0x9	/* EISA interrupt level	9 	*/
#define EISA10_INTR      	0xa	/* EISA interrupt level 10	*/
#define EISA11_INTR      	0xb	/* EISA interrupt level 11	*/
#define EISA12_INTR      	0xc	/* EISA interrupt level 12	*/
#define EISA14_INTR      	0xe	/* EISA interrupt level 14	*/
#define EISA15_INTR      	0xf	/* EISA interrupt level 15	*/

/*
 *	SCSI Definition (SCSIDEF, ZCC3, W/R)
 */
#define SCSIDEF_RSTPWR		BIT4	/* Reset on power up and HRESET	*/
#define SCSIDEF_HSCSIID6	0x6	/* Host SCSI id 6		*/
#define SCSIDEF_HSCSIID7	0x7	/* Host SCSI id 7		*/

/*
 *	Bus Definition (BUSDEF, ZCC4, W/R)
 *	
 *	This register is used for configuration of EISA bus features.
 *	DMA channel has meaning only in Standard Mode.  
 *	Actually this register is a NOP in Enhanced Mode.
 */
#define BUSDEF_DMA0	0x0	/* DMA 0 channel Returned		*/
#define BUSDEF_DMA5	0x4	/* DMA 5 channel Returned		*/
#define BUSDEF_DMA6	0x8	/* DMA 6 channel Returned		*/
#define BUSDEF_DMA7	0xc	/* DMA 7 channel Returned		*/
#define BUSDEF_0US	0x0	/* 0 usecs before ending transfer	*/
#define BUSDEF_4US	0x1	/* 4 usecs before ending transfer	*/
#define BUSDEF_8US	0x2	/* 8 usecs before ending transfer	*/


/*
 *	Attention (ATTN, zCD4, W/R), 
 *	bit 7-4 is OP code, 
 *	bit 3-0 is Target ID
 */
#define ATTN_OP_ICMD 	0x10		/* Immediate command		*/
#define ATTN_OP_START	0x40		/* Start CCB			*/
#define ATTN_OP_ABORT	0x50		/* Abort CCB			*/

#define ICMD_DEV_RESET 	0x80		/* Immediate command - reset 	*/
#define ICMD_DEV_ABORT 	0x40080		/* Device Reset Option flag = 1	*/
#define ICMD_ADP_RESET 	0x80		/* Immediate command - reset   	*/
#define ICMD_ADP_ABORT 	0x80080		/* Adapter Reset Option flag = 1*/

#define ICMD_RESUME 	0x90		/* Immediate command - resume  	*/
#define ICMD_RESUME_DI 	0x200090 	/* resume - interrupt disable  	*/


/*
 *	Enhanced Mode (Group 2) Control (G2CNTRL, zCD5, W/R)
 */
#define G2CNTRL_HRESET	BIT7	/* Hard Reset - min 10 micro sec 	*/
#define G2CNTRL_CLRINT	BIT6	/* Clear EISA Interrupt	       		*/
#define G2CNTRL_SETRDY 	BIT5	/* Set Host Ready			*/

/*
 *	Enhanced Mode (Group 2) Interrupt Status (G2INTST, zDC6, R)
 */
#define G2INTST_IMSK 	0xf0	/* Interrupt status mask	*/
#define G2INTST_TMSK 	0x0f	/* Interrupt target id mask	*/
#define G2INTST_CMP 	0x10	/* CCB completed with success	*/
#define G2INTST_CAR 	0x50	/* CCB cmp success after retry  */
#define G2INTST_AHF 	0x70	/* Adapter hardware failure	*/
#define G2INTST_ICMP 	0xa0	/* Imd completed with success	*/
#define G2INTST_CWE 	0xc0	/* CCB completed with error	*/
#define G2INTST_AEN 	0xd0	/* Asyn event notification	*/
#define G2INTST_ICE 	0xe0	/* Immediate cmp with error	*/

/*
 *	Enhanced Mode (Group 2) Status (G2STAT, zCD7, R)
 */
#define G2STAT_MOE	BIT2	/* Mailbox out empty		*/
#define G2STAT_IP	BIT1	/* Interrupt pending		*/
#define G2STAT_BUSY	BIT0	/* Busy				*/

/*
 *	Enhanced Mode (Group 2) Status 2 (G2STAT2, zCDC, R)
 */
#define G2STAT2_HRDY   	BIT0 	/* Host ready			*/

/*
 *	MBOXIN0 Error code
 */
#define MBOXIN0_NE	0x00	/* No Error			*/
#define MBOXIN0_EC_ROM	0x01	/* ROM test failure		*/
#define MBOXIN0_EC_RAM	0x02	/* RAM test failure		*/
#define MBOXIN0_EC_PPD	0x03	/* Power protection dev error	*/
#define MBOXIN0_EC_IPF	0x04	/* Internal peripheral failure	*/
#define MBOXIN0_EC_BCC	0x05	/* Buffer control chip failure	*/
#define MBOXIN0_EC_SIC	0x08	/* SCSI Interface chip failure	*/
#define MBOXIN0_EC_HD	0x07	/* Hardware failure		*/

/*
 *	Host Adapter Status - This field is valid when the Command Done
 *	Flag is set to zero and one of the following flags is set to one:
 *	Specification Check, Initialization Required, or Major Error/
 *	Expception
 */
/*
 *	Major Error/Exception
 */
#define HS_NS		0x00	/* No Host Status Available	*/
#define HS_CMD_AB_HT	0x04	/* Command Aborted by Host	*/
#define HS_CMD_AB_HA	0x05	/* Command Aborted by Host Adp	*/
#define HS_SEL_TO	0x11	/* Selection Timeout		*/
#define HS_DA_OR	0x12	/* Data Overrun or Underrun	*/
#define HS_UE_BF	0x13	/* Unexpected Bus Free		*/
#define HS_IV_BPD	0x14	/* Invalid bus phase detected	*/
#define HS_IV_SLO	0x17	/* Invalid SCSI linking Oper.	*/
#define HS_RSC_F	0x1b	/* Request Sense Command Failed	*/
#define HS_TQ_MR	0x1c	/* Tag Q msg rejected by target	*/
#define HS_HA_HE	0x20	/* Host Adapter Hardware Error	*/
#define HS_TA_NRA	0x21	/* Target not respond to attn	*/
#define HS_SB_RSTH	0x22	/* SCSIbus reset by host adapter*/
#define HS_SB_RSTD	0x23	/* SCSIbUS reset by other device*/
#define HS_PG_CF	0x80	/* Program checksum failure	*/


/*
 *	Specification Check
 */
#define HS_TA_NA	0x0a	/* Target Not Assigned to SCSI	*/
#define HS_IV_OP	0x16	/* Invalid Operation Code	*/
#define HS_IV_CBP	0x18	/* Invalid Control Block Para.	*/
#define HS_DUP_TCB	0x19	/* Duplicate Target Control blk	*/
#define HS_IV_SG	0x1a	/* Invalid Scatter/Gather list	*/

/*
 *	Initialization Required
 */
#define HS_FW_NDL	0x08	/* Firmware Not Downloaded	*/

/*
 *	Target Status
 */
#define TS_GD		0x00	/* Good or No Status available 	*/
#define TS_CC		0x02	/* Check Condition	 	*/
#define TS_CM		0x04	/* Condition Met 		*/
#define TS_TB		0x08	/* Target Busy 			*/
#define TS_IN		0x10	/* Intermediate			*/
#define TS_ICM		0x14	/* Intermediate Condition Met	*/
#define TS_RC		0x18	/* Reservation Conflict		*/

/*
 *	Maximum Synchronous Transfer Rate
 */
#define MSTR_10		0x0	/* 10.0 Mbytes/second		*/
#define MSTR_6		0x1	/* 6.67 Mbytes/second		*/
#define MSTR_5		0x2	/* 5.00 Mbytes/second		*/
#define MSTR_4		0x3	/* 4.00 Mbytes/second		*/
#define MSTR_3		0x4	/* 3.33 Mbytes/second		*/

/*
 * 	Command Word Operations Code
 */
#define AHA_OP_NOP	0x00 	/* No Operation			*/
#define AHA_OP_ISC	0x01 	/* Initiator SCSI Command	*/
#define AHA_OP_RDT	0x05 	/* Run Diagnostic Test		*/
#define AHA_OP_ISS	0x06 	/* Initialize SCSI Subsystem	*/
#define AHA_OP_RSI	0x08 	/* Read Sense Information	*/
#define AHA_OP_DF	0x09 	/* Download Firmware		*/
#define AHA_OP_RHAID	0x0a 	/* Read HA Inquiry Data		*/
#define AHA_OP_TSC	0x10 	/* Target SCSI Command		*/

/*
 *	Flags Mask
 */
#define AHA_F1_MASK	0xd481 	/* flag1 mask word		*/
#define AHA_F2_MASK	0xcf7f 	/* flag2 mask word		*/

/*
 *	Flags bits
 */
#define AHA_F1_CNE	BIT0 	/* Chain No Error		*/
#define AHA_F1_DI	BIT7 	/* Disable Interrupt		*/
#define AHA_F1_SES	BITa 	/* Suppress Error on Underrun	*/
#define AHA_F1_SG	BITc 	/* ScatterGather		*/
#define AHA_F1_DSB	BITe 	/* Disable Status Block		*/
#define AHA_F1_ARS	BITf 	/* Automatic Request Sense	*/

#define AHA_F2_LUNMSK	0x0007	/* LUN number mask              */
#define AHA_F2_TAG	BIT3	/* Tagged Queuing		*/
#define AHA_F2_ND	BIT6	/* No Disconnect		*/
#define AHA_F2_CD	BIT8	/* Check Direction		*/
#define AHA_F2_DIR	BIT9	/* Direction of Transfer	*/
#define AHA_F2_ST	BITa	/* Suppress Xfer to Host Memory	*/
#define AHA_F2_CHK	BITb	/* Calculate checksum on data	*/
#define AHA_F2_REC	BITe	/* Error Recovery		*/
#define AHA_F2_NRB	BITf	/* No Retry on Busy Status	*/

/*
 *	Status Word
 *
#define AHA_SW_MASK	0x5bfb	/* Status Word mask		 */
#define AHA_SW_DON	BIT0	/* Command Done - No Error	 */
#define AHA_SW_DU	BIT1	/* Data Underrun		 */
#define AHA_SW_QF	BIT3	/* Host Adapter Queue Full	 */
#define AHA_SW_SC	BIT4	/* Specification Check		 */
#define AHA_SW_DO	BIT5	/* Data Overrun			 */
#define AHA_SW_CH	BIT6	/* Chaining Halted		 */
#define AHA_SW_INT	BIT7	/* Interrupt Issued for SCB	 */
#define AHA_SW_ASA	BIT8	/* Additional Status Available	 */
#define AHA_SW_SNS	BIT9	/* Sense Information Stored	 */
#define AHA_SW_INI	BITb	/* Initialization Required	 */
#define AHA_SW_ME	BITc	/* Major Error or Exception 	 */
#define AHA_SW_ECA	BITe	/* Extended Contingent Allegiance*/



/*
 * The control block is a 48-byte structure created and maintained in
 * shared memory by software in the system unit. It is used to convey
 * requests to the host adapter.  
 */
typedef struct aha_cb{
  U16	command;		/* Command word 			*/
  U16 	flag1;			/* flag bits word 1 			*/
  U16 	flag2;			/* flag bits word 2 			*/
  U16 	res0;			/* reserved 0 - SB ZERO			*/
  U32 	dlptr;			/* data scatter/gather list 		*/ 	
  U32 	dllen;			/* pointer and length 			*/
  U32 	stat_ptr;		/* status block physical addr 		*/
  U32 	chnaddr;		/* chain address 		 	*/
  U32 	res1;			/* reserved 1 - SB ZERO			*/
  U32 	snsptr;			/* sense infor physical addr 		*/
  U8 	snslen;			/* sense length 			*/
  U8 	cdblen;			/* cdb length				*/
  U16 	data_cksum;		/* Data Checksum			*/
  U8 	cdb[12];		/* cdb commands 			*/
} AHA_CB;

#define AHA_MAXACB      64      /* max active control block     */

#define AHA_MAX_CDB_LENGTH 12   /* max. CDB length supported by adapter */


/*
 *	The Status Block is a 32-byte structure
 */
typedef struct aha_sb{
  U16 	stat;			/* status word 				*/
  U8	hstat;			/* host adapter status 			*/
  U8 	tstat;			/* target status 			*/
  U32 	rescnt;			/* residual byte count 			*/
  U32 	resaddr;		/* residual buffer address 		*/
  U16 	addstlen;		/* additional status length 		*/
  U8 	snslen;			/* sense length 			*/
  U8 	res0;			/* reserved - 1 byte			*/
  U32 	res1;			/* reserved - 4 bytes 			*/
  U32 	res2;			/* reserved - 4 bytes 			*/
  U8 	cdb[6];			/* target mode CDB 			*/
} AHA_SB;


/*
 * 	The Host Adapter Inquiry Data structure
 */
typedef struct aha_inq_data{
  U16	dev_type;		/* SCSI device type of the adapter 	*/
  U16	support_level;		/* SCSI support level			*/
  U8	residual_length;	/* Additional bytes of data available	*/
  U8	num_lun;		/* Number of LUNs the adapter has 	*
				 * enabled for Target Mode		*/
  U8	num_cb;			/* Max. number of control blocks the    *
				 * adapter can store internally         */
  U8	flags;			/* SCSI features flag                   */
  U8	vendor_name[8];		/* 8 bytes of vendor ID in ASCII,       *
				 * "Adaptec "				*/
  U8	product_id[8];		/* 8 bytes of this adapter product name *
				 * in ASCII                             */
  U8	firmware_type[8];	/* 8 bytes of ASCII firmware type,      *
				 * 'standard' or 'enhanced'		*/
  U32	firmware_rev;		/* 4 bytes of ASCII firmware revision   *
				 * level, starts at "A   "		*/
  U8	release_date[8];	/* 8 bytes of ASCII firmware release    *
				 * date                                 */
  U8	release_time[8];	/* 8 bytes of ASCII firmware release    *
				 * time value				*/
  U16	firmware_checksum;	/* Firmware checksum 			*/
  U8	reserved[202];		/* Reserved fields			*/

} AHA_INQ_DATA;


#define AHA_MIN_FIRMREV    'H   '	/* Minimum acceptable firmware revision *
				 	 * number for this adapter              */

/*
 * struct of each entry in the scatter/gather list.
 */
typedef struct sg_segment {
  U32   dataptr;                /* physical data pointer        */
  U32   count;                  /* transfer byte count          */
} SG_SEGMENT;


/*
 * Forward structure declarations.  This will allow the use of the
 * structure as a pointer before the structure is defined.
 */
struct aha_job;



/* 
 * structure for queue of aha_job.  
 */
typedef struct ahaq {
  struct aha_job 	*ahaq_head;
  struct aha_job 	*ahaq_tail;
  simple_lock_data_t 	ahaq_lock;	/* Lock for the queue */
} AHA_Q;



/*
 * HBA specific SOFTC structure.  A pointer is provided in SIM_SOFTC
 * to point to this structure (hba_sc).
 *
 * If this structure is modified, increment the AHA_SOFTC_VERSION
 * number below.
 */

#define AHA_NDPS	8		/* Number of devices per SCSI bus   *
					 * for this adapter     	    */
#define AHA_NLPT	8		/* Number of LUNs per target for    *
					 * this adapter			    */


typedef struct aha_softc {

#define AHA_SOFTC_VERSION 2

  I32	aha_init_called;		/* # of times aha_init() was called*/
  I32	aha_reset_num;			/* # of times hard reset was done  */
  I32   aha_checked_fw;			/* checked firmware and enabled    *
					 * interrupts for this controller  */
  U32	aha_state;			/* controller state info	   */
  U32	aha_total_acb;			/* total active control block	   */
  U32	aha_cb_pa;			/* control block physical addr	   *
				 	 * for the lastest one sent    	   */
  AHA_CB *aha_cb_va;			/* pointer to control block	   *
					 * just sent to adapter            */
  SIM_SOFTC *aha_sim_softc;		/* SIM softc struct 		   */
  struct controller *aha_ctlr;		/* controller reference		   */
  struct aha_job    *aha_bdr_job;	/* ptr. to job block for bus	   *
                               		 * device reset request        	   */
  AHA_INQ_DATA *aha_inquiry_data;	/* ptr. to the adapter inquiry     *
					 * data block			   */
  U32	aha_inq_data_length;		/* Actual number of inquiry data   *
					 * bytes returned from adapter     */
  AHA_Q	aha_activeq[AHA_NDPS][AHA_NLPT];
					/* array of queue of active jobs   *
				 	 * sorted by target ID             */
  AHA_Q aha_freeq;			/* queue of free job blocks	   */
  simple_lock_data_t aha_lock;		/* lock for this controller 	   */

} AHA_SOFTC;


#define AHA_NUM_JOBS 	80		/* number of job blocks */

/*
 * Adapter softc state flags
 */
#define AHA_ALIVE      		  0x01		/* controller is alive 	*/
#define AHA_RESET_IN_PROGRESS  	  0x02		/* controller is being  *
					 	 * reset/reinitialized  */
#define AHA_BUS_RESET_IN_PROGRESS 0x04		/* bus reset detected   */


/* 
 * the maximum number of bytes that can be mapped in one scatter/gather
 * entry.
 */
#define AHA_DMA_MAX_COUNT	0x400000	/* 4 M max per entry	   */
#define AHA_DMA_MAX_SIZE	0x1000000	/* 16 M max S/G DMA	   */
#define AHA_DMA_MAX_SGLENGTH	0x400		/* 1 K max S/G list length */
#define AHA_DMA_MAX_SEGMENTS	0x80		/* 128 max. S/G segments   */
	

#define AHA_SNS_BUF_LEN		0x100		/* Sense buffer length 	   */

/*
 * aha_job
 */
typedef struct aha_job {

#define AHA_JOB_VERSION 1

  struct aha_job  *aha_next;	  	/* host virtual for linked lists  */
  struct aha_job  *aha_prev;	  	/* host virtual for linked lists  */

  AHA_CB	aha_cb;			/* 48-byte I/O control block    */ 

  AHA_SB	aha_sb;			/* 32-byte I/O status block     */

  SG_SEGMENT	aha_sglist[AHA_DMA_MAX_SEGMENTS];
					/* Scatter/gather list for DMA  */

  U8		aha_sensebuf[AHA_SNS_BUF_LEN];  
					/* Sense buffer                 */

  /*     
   * NB: If the AHA_JOB structure is changed and the order of the
   *      structures, AHA_CB, AHA_SB, SG_SEGMENT array, and sense
   *      buffer is modified, the calculation for these fields' physical
   *	  addresses must also be changed in the source module.
   *
   */

  /*
   * The area below is accessed only by the aha driver code.
   */

  /* non-static fields */

  U32		aha_cbcmdtype;		/* saved control block command    */
  U32		aha_sgcount;	  	/* number of sg segments used	  */

  U32		aha_senseflag;	  	/* If non-zero, sense buffer in   *
				 	 * this block is being used for   *
				   	 * this I/O request		  */


  SIM_WS 	*aha_sws;	  	/* SIM_WS - SIM Working Set 	  */
  CCB_SCSIIO    *aha_ccb;		/* Pointer to the CCB		  */
  U32		aha_jobtype;	  	/* Non-tagged I/O, tagged I/O, 	  *
				 	 * or special job block		  */
  U32		aha_state;	  	/* State of this job block	  */

  /* static fields */
  vm_offset_t	aha_job_va;		/* Self pointer to this job block */
  U32           aha_cb_pa;              /* control block physical addr    */
  U32		aha_sb_pa;		/* status block physical address  */
  U32		aha_sglist_pa;		/* scatter/gather list phys. addr */
  U32		aha_snsbuf_pa;		/* sense buffer physical address  */
  AHA_SOFTC     *aha_softc;             /* Adapter softc pointer          */

} AHA_JOB;


#define AHA_NON_TAGGED	0		/* Non-tagged normal I/O job	*/
#define AHA_TAGGED	1		/* Tagged normal I/O job	*/
#define AHA_SPECIAL	2		/* Special job block for bus    *
				 	 * device reset			*/


/*
 * Locking macros - handle IPL and SMP locks
 */

#define AHA_LOCK_INIT(lock) simple_lock_init ( &lock )

#define AHA_IPL_LOCK(lock, s) {         \
	s = splbio();                   \
	simple_lock ( &lock );          \
}

#define AHA_IPL_UNLOCK(lock, s) {	\
	simple_unlock ( &lock );	\
	splx(s);                        \
}

#define AHA_LOCK(lock)	simple_lock ( &lock ) 

#define AHA_UNLOCK(lock)  simple_unlock ( &lock )

/* 
 * Macros for inserting and removing queue elements
 */
#define AHA_INSERT(que_elem, queue) insque((void *)(que_elem), (void *)(queue))

#define AHA_REMOVE(que_elem) remque((void *)(que_elem))

#define AHA_INSERT_TAIL(queue, que_elem) enqueue_tail((void *)(queue), (void *)(que_elem))

#define AHA_REMOVE_HEAD(queue) dequeue_head((void *)(queue))

/*
 * Macro for getting the pointer to the job block from the pointer
 * to the AHA_CB 
 */
#define AHA_JOB_PTR(cbptr) 	\
	( (vm_offset_t)((unsigned long)cbptr - ( (sizeof(AHA_JOB *)) * 2 )) )

#endif
