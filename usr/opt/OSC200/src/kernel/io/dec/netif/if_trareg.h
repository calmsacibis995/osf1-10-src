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
 * @(#)$RCSfile: if_trareg.h,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/09/23 17:34:29 $
 */

#ifndef _IF_TRAREG_H_
#define _IF_TRAREG_H_
/****************************************************************
 *								*	
 * SIFCMD and SIFSTS register definitions.			*
 *								*
 ****************************************************************
 * Write-bit and Read-bit functions
 */
#define CMDSTS_M_INTR_ADAPTER	0x8000	
#define CMDSTS_M_ADAPTER_RESET	0x4000
#define CMDSTS_M_SSB_CLEAR	0x2000
#define CMDSTS_M_EXECUTE	0x1000
#define CMDSTS_M_SCB_REQUEST	0x0800
#define CMDSTS_M_RCV_CONTINUE	0x0400
#define CMDSTS_M_RCV_VALID	0x0200
#define CMDSTS_M_XMIT_VALID	0x0100
#define CMDSTS_M_SYS_INTERRUPT	0x0080
#define CMDSTS_M_INTERRUPT_MASK	0xff80
#define CMDSTS_M_INTR_TYPE_MASK	0x000f
#define CMDSTS_M_ERROR_REASON	0x000f
#define CMDSTS_M_SOFTWARE_RESET	0xff00


/* 
 * Read-bit functions 
 */
#define CMDSTS_M_INITIALIZE	0x0040
#define CMDSTS_M_TEST		0x0020
#define CMDSTS_M_ERROR		0x0010 
#define CMDSTS_M_READ_MASK	0x00f0
#define CMDSTS_M_INIT_TEST_ERR  0x0070
#define CMDSTS_M_INTR_CODE_MASK	0x000f

/* 
 * Interrupt Reason
 */
#define CMDSTS_INTR_ADAPTER_CHECK	0x0000	
#define CMDSTS_INTR_RING_STATUS		0x0004
#define CMDSTS_INTR_LLC_STATUS		0x0005
#define CMDSTS_INTR_SCB_CLEAR		0x0006
#define CMDSTS_INTR_TIMER		0x0007
#define CMDSTS_INTR_COMMAND_STATUS	0x0008
#define CMDSTS_INTR_RECV_STATUS		0x000a
#define CMDSTS_INTR_XMIT_STATUS		0x000c
#define CMDSTS_INTR_RCV_PENDING		0x000e

/*
 * Error code.
 */
#define BUD_INITIAL_TEST_ERROR		0x0000
#define BUD_ADAP_SOFT_CKSUM_ERROR	0x0001
#define BUD_ADAP_RAM_ERROR		0x0002
#define	BUD_ADAP_INSTR_ERROR		0x0003
#define	BUD_CTXT_INTR_TEST_ERROR	0x0004
#define	BUD_PH_HARDWARE_ERROR		0x0005
#define	BUD_SYS_INTF_REG_ERROR		0x0006	

/********************************************************
 *							*
 * SIFACL Adapter control registers definition		*
 *							*
 ********************************************************/
#define SIFACL_RESERVED_0	0xf000	/* Read as zero */
#define SIFACL_SWHLDA		0x0800	/* Relevant when using PSDMA */
#define SIFACL_SWDDIR		0x0400	/* DMA xfer direction */
#define SIFACL_SWHRQ		0x0200	
#define SIFACL_PSDMAEN		0x0100	/* PSDMA enable */
#define SIFACL_ARESET		0x0080	/* Adapter reset */
#define SIFACL_CPHALT		0x0040	/* Halts the Comm. Processor */
#define SIFACL_BOOT		0x0020	
#define SIFACL_RESERVED_1	0x0010	/* Must be set to zero */
#define SIFACL_SINTEN		0x0008
#define SIFACL_PEN		0x0004
#define SIFACL_INPUT0		0x0002	/* Reserved; Read only */
#define SIFACL_RESERVED_2	0x0001	/* Read as zero */

/*
 * DMA Direction and read from TRA_SIFACL_SWDDIR register.
 */
#define HOST_TO_ADAP_DMA	0x0000
#define ADAP_TO_HOST_DMA	0x0001


/*
 * System Command Block definition
 */
typedef struct {
	u_short scb_cmd;
	u_short scb_parm_0;
	u_short scb_parm_1;
} SYSTEM_COMMAND_BLOCK;

/*
 * System Status Block definition
 */
typedef struct {
	u_short ssb_cmd;
	u_short ssb_parm_0;
	u_short ssb_parm_1;
	u_short ssb_parm_2;
} SYSTEM_STATUS_BLOCK;
	


/*
 * Initialization Parameter Block definition
 */
typedef struct {
	u_short		init_options;
 	u_char  	cmd_status_vector;
	u_char  	xmit_vector;
	u_char  	recv_vector;
	u_char  	ring_status_vector;
	u_char  	scb_clear_vector;
	u_char  	adap_chk_vector;
	u_short		recv_burst_size;
	u_short		xmit_burst_size;	
	u_short		dma_abort_threshold;
	u_short   	scb_address_hi;
	u_short   	scb_address_lo;
	u_short   	ssb_address_hi;
	u_short   	ssb_address_lo;
} INIT_PARM_BLOCK ;



/*
 *  INIT_OPTIONS field definition
 */
#define INITOP_RESERVED_0		0x0001	/* Must be set to one */
#define INITOP_PARITY_ENABLE		0x0006
#define INITOP_BURST_SCB_SSB		0x0008
#define INITOP_BURST_LIST		0x0010
#define INITOP_BURST_LST_STATUS 	0x0020
#define INITOP_BURST_RCV_DATA		0x0040
#define INITOP_BURST_XMIT_DATA		0x0080
#define INITOP_RESERVED_1		0x0100	/* Must be reset to zero */
#define INITOP_LLC_ENABLE		0x0200
#define INITOP_RESERVED_2  		0xfc00	/* Must be reset to zero */

/*
 * Adapter intialization errors.
 */
#define INITOP_INVALID_BLOCK_ERR	0x0001
#define INITOP_INVALID_OPTION_ERR	0x0002
#define INITOP_INVALID_RCV_BURST_SZ_ERR	0x0003
#define INITOP_INVALID_XMT_BURST_SZ_ERR	0x0004
#define INITOP_INVALID_DMA_THRESH_ERR	0x0005
#define INITOP_INVALID_SCB_ERR		0x0006
#define INITOP_INVALID_SSB_ERR		0x0007
#define INITOP_DIO_PARITY_ERR		0x0008
#define INITOP_DMA_TIMEOUT_ERR		0x0009
#define INITOP_DMA_PARITY_ERR		0x000A
#define INITOP_DMA_BUS_ERR		0x000B
#define INITOP_DMA_DATA_ERR		0x000C
#define INITOP_ADAPTER_CHECK_ERR	0x000D

/*
 * MAC OPEN Parameter Block definitions
 */
typedef struct {
	u_short open_options;
	u_char  node_address[6];
	u_char	group_address[4];
	u_char	functional_address[4];
	u_short rcv_list_size;	
	u_short xmit_list_size;
	u_short buffer_size;
	u_short	reserved1;
	u_short	reserved2;
	u_short xmit_buf_min_max;
	u_short	product_id_addr_lo;
	u_short product_id_addr_hi;
} OPEN_PARM_BLOCK;


/*
 * OPEN_OPTIONS field definitions.
 */
#define OPEN_WRAP_INTERFACE		0x8000
#define OPEN_DISABLE_HARD_ERROR		0x4000
#define OPEN_DISABLE_SOFT_ERROR		0x2000
#define OPEN_PASS_ADPT_MAC_FRAMES	0x1000
#define OPEN_PASS_ATTN_MAC_FRAMES	0x0800
#define OPEN_PAD_ROUT_FIELD		0x0400
#define OPEN_FRAME_HOLD			0x0200
#define OPEN_CONTENDER			0x0100
#define OPEN_PASS_BEACON_MAC_FRAMES	0x0080
#define OPEN_RESERVED_0			0x0060	/* Must be reset to zero */
#define OPEN_EARLY_TOKEN_RELEASE	0x0010
#define OPEN_RESERVED_1			0x0008	/* Must be set to zero */
#define OPEN_COPY_ALL_MAC_FRAMES	0x0004
#define OPEN_COPY_ALL_NONMAC_FRAMES	0x0002
#define OPEN_PASS_FIRST_BUF_ONLY	0x0001


/*
 * Upon completion of MAC.OPEN SSB_PARM_0 of SSB will contain the 
 * OPEN_COMPLETION code. These are the bit definitions of OPEN_COMPLETION. 
 */
#define OPENC_OPEN_SUCCESS		0x8000
#define OPENC_NODE_ADDR_ERROR		0x4000
#define OPENC_LIST_SIZE_ERROR		0x2000
#define OPENC_BUFF_SIZE_ERROR		0x1000
#define OPENC_RESERVED_0		0x0800
#define OPENC_XMIT_BUFF_COUNT_ERROR	0x0400
#define OPENC_OPEN_ERROR		0x0200
#define OPENC_INV_OPEN_OPTION		0x0100

/*
 * OPEN Phases bit definition of OPEN_COMPLETION field shifted
 * right by 4.
 */
#define OPENC_LOBE_MEDIA_TEST		0x0001
#define OPENC_PHYS_INSERTION		0x0002
#define OPENC_ADDR_VERIFY		0x0003
#define OPENC_PART_POLL_RING		0x0004
#define OPENC_REQ_INIT			0x0005
#define OPENC_PHASE_MASK		0x00f0

/*
 * OPEN Error Codes bit definition of OPEN_COMPLETION field
 */
#define OPENC_FUNCT_FAILURE		0x0001
#define OPENC_SIGNAL_LOSS		0x0002
#define OPENC_TIMEOUT			0x0005
#define OPENC_RING_FAIL			0x0006
#define OPENC_RING_BEACON		0x0007
#define OPENC_DUP_NODE_ADDR		0x0008
#define OPENC_REQ_PARMS			0x0009
#define OPENC_REMOVE_RCVD		0x000a
#define OPENC_ERROR_MASK		0x000f

/*
 * MAC TRANSMIT Parameter List definition
 * NOTE: For transmit we use only one field of the data address/data count
 *	 pair in the XMIT_PARM_DATA structure. But, for receive we use
 *	 all the 3 pairs.
 *	 The padding put in xmit parm list makes it quad word aligned. 
 *	 This is not required for the xmit_parm_list but is need for
 *	 the rcv_parm_list because of a hardware bug.
 */
struct xmit_parm_list {
	u_short forw_ptr_hi;
	u_short forw_ptr_lo;
	u_short xmit_cstat;
	u_short frame_size;
	u_short data_count;
	u_short	data_addr_hi;	
	u_short	data_addr_lo;	
	u_short data_count1;
	u_short	data_addr1_hi;	
	u_short	data_addr1_lo;	
	u_short data_count2;
	u_short	data_addr2_hi;	
	u_short	data_addr2_lo;	
	u_short padding1;
	u_short padding2;
	u_short padding3;
};
#define XMIT_PARM_LIST struct xmit_parm_list


/*
 * TRANSMIT_CSTAT_REQUEST field defintions
 */
#define XMITCSTAT_REQ_VALID		0x8000
#define XMITCSTAT_REQ_FRAME_COMPLETE	0x4000
#define XMITCSTAT_REQ_START_FRAME	0x2000
#define XMITCSTAT_REQ_END_FRAME		0x1000/*Set to 1 for end of frame list*/
#define XMITCSTAT_REQ_FRAME_INTR	0x0800
#define XMITCSTAT_REQ_RESERVED_0	0x0400	/* Must be reset to zero */
#define XMITCSTAT_REQ_PASS_CRC		0x0200
#define XMITCSTAT_REQ_PASS_SRC_ADDR	0x0100


/* 
 * TRANSMIT_CSTAT_COMPLETE field defintions
 */
#define XMITCSTAT_COMP_VALID		0x8000 
#define XMITCSTAT_COMP_FRAME_COMPLETE	0x4000
#define XMITCSTAT_COMP_FRAME_START	0x2000
#define XMITCSTAT_COMP_FRAME_END	0x1000
#define XMITCSTAT_COMP_FRAME_INTR	0x0800
#define XMITCSTAT_COMP_XMIT_ERROR	0x0400
#define XMITCSTAT_COMP_PASS_CRC		0x0200
#define XMITCSTAT_COMP_PASS_SRC_ADDR	0x0100
#define XMITCSTAT_COMP_STRIP_FS		0x00ff	/* 0x00ff  Copy of FS if no error */


/*
 * Bit definitions of the XMIT_STATUS field of TRANSMIT completion SSB
 */
#define XSTAT_COMMAND_COMPLETE		0x8000
#define XSTAT_FRAME_COMPLETE		0x4000
#define XSTAT_LIST_ERROR		0x2000
#define XSTAT_RESERVED_0		0x1f00	/* Reset to zero */
#define XSTAT_FRAME_SIZE_ERROR		0x0080 
#define XSTAT_XMIT_THRESHOLD		0x0040
#define XSTAT_ODD_ADDRESS		0x0020
#define XSTAT_FRAME_ERROR		0x0010
#define XSTAT_ACCESS_PRI_ERROR		0x0008
#define XSTAT_UNENBL_MAC_FRAME		0x0004
#define XSTAT_ILL_FRAME_FORMAT		0x0002 
#define XSTAT_RESERVED_1		0x0001




/*
 * MAC RECEIVE Parameter List definition
 * It is important to have the padding in the rcv_parm_list so as to
 * make the structure quad word aligned when place 2 or more of these in
 * a ring. If this is not quad word aligned then every other member of the ring
 * will have the forw_ptr_lo and the data_count_0 field corrupted after
 * a receive.
 * The corruption is caused while doing a DMA to update the rcv_cstat and 
 * frame_size fields; the forw_ptr_lo and data_count_0 get written into also.
 * Padding the structure prevents that from happening.
 */
struct rcv_parm_list {
	u_short forw_ptr_hi;
	u_short forw_ptr_lo;
        u_short rcv_cstat;
        u_short frame_size;
        u_short data_count_0;
        u_short data_addr0_hi;
        u_short data_addr0_lo;
        u_short data_count_1;
        u_short data_addr1_hi;
        u_short data_addr1_lo;
        u_short data_count_2;
        u_short data_addr2_hi;
        u_short data_addr2_lo;
	u_short padding;
};
#define RCV_PARM_LIST struct rcv_parm_list
 

/*
 * RECEIVE_CSTAT_REQUEST field definitions
 */
#define RCVCSTAT_REQ_VALID		0x8000
#define RCVCSTAT_REQ_FRAME_COMPLETE	0x4000  /* Must be reset to zero */
#define RCVCSTAT_REQ_START_FRAME	0x2000	/* Must be reset to zero */
#define RCVCSTAT_REQ_END_FRAME		0x1000	/* Must be reset to zero */
#define RCVCSTAT_REQ_FRAME_INTR		0x0800
#define RCVCSTAT_REQ_INTERFRM_WAIT	0x0400
#define RCVCSTAT_REQ_PASS_CRC		0x0200
#define RCVCSTAT_INITIALIZE		0x8800	/* intial value */
#define RCVCSTAT_REQ_RESERVED		0x01ff	/* Must be reset to zero */


/*
 * RECEIVE_CSTAT_COMPLETE field definitions
 */
#define RCVCSTAT_COMP_VALID		0x8000	/* Reset to zero */
#define RCVCSTAT_COMP_FRAME_COMPLETE	0x4000  /* Set to one */
#define RCVCSTAT_COMP_FRAME_START	0x2000  /* Set to one on the list
						     that starts the frame */	
#define RCVCSTAT_COMP_FRAME_END		0x1000	/* Set to one on the list 
						     that ends the frame */
#define RCVCSTAT_START_END_FRAME	0x3000	/* start and end of frame */
#define RCVCSTAT_COMP_FRAME_INTR	0x0800
#define RCVCSTAT_COMP_INTERFRM_WAIT	0x0400
#define RCVCSTAT_COMP_CRC_PASSED	0x0300
#define RCVCSTAT_COMP_RESERVED		0x0080	/* Reset to zero */
#define RCVCSTAT_COMP_RCV_FS		0x007d	
#define RCVCSTAT_COMP_ADDR_MATCH	0x0003

/*
 * Field definitions for the ADDRESS MATCH codes of RECEIVE_CSTAT_COMPLETE.
 */
#define ADDR_MATCH_NO_INT_EXT		0x0000
#define ADDR_MATCH_INT			0x4000
#define ADDR_MATCH_EXT			0x8000 	/* Via XMATCH/XFAIL */
#define ADDR_MATCH_INT_EXT		0xc000

/*
 * Bit definitions of RECEIVE_COMPLETE code; this is the value that the
 * adapter assigns the the RCV_STATUS field of SSB, upon a RECEIVE_STATUS
 * interrupt.
 */
#define RCVSSB_FRAME_COMPLETE	0x8000
#define RCVSSB_RCV_SUSPEND	0x4000 

/*
 * 	MAC READ.ERROR.LOG Error Log Table
 * These error counters are reset after the MAC READ.ERROR.LOG command
 * has completed.
 */
typedef struct {
	u_short line_error;
	u_short burst_arifci_error;
	u_short reserved_1;
	u_short lostframe_rcvcong_error;
	u_short frame_copied_error;
	u_short token_error;
	u_short dma_bus_parity_error;
} ERROR_LOG;	



/* 
 * 	MAC-only command definitions
 * These values will appear in both the SCB_CMD field of SCB during 
 * command initiation and in the SSB_CMD field of SSB upon completion.
 */
#define MAC_OPEN			0x0003
#define MAC_TRANSMIT			0x0004
#define MAC_TRANSMIT_HALT		0x0005 
#define MAC_RECEIVE			0x0006
#define MAC_CLOSE			0x0007
#define MAC_SET_GROUP_ADDRESS		0x0008
#define MAC_SET_FUNCT_ADDRESS		0x0009
#define MAC_READ_ERROR_LOG		0x000a
#define MAC_READ_ADAPTER		0x000b
#define MAC_MODIFY_OPEN_PARMS		0x000d
#define MAC_RESTORE_OPEN_PARMS		0x000e
#define MAC_SET_F16_GROUP_ADDRESS	0x000f
#define MAC_SET_BRIDGE_PARMS		0x0010
#define MAC_CONFIG_BRIDGE_PARMS		0x0011
/*
 * Values appearing only on the SSB_CMD field of SSB upon completion
 */
#define RING_STATUS		0x0001
#define COMMAND_REJECT		0x0002

/*
 * Completion codes for some of MAC commands
 */
#define MAC_SUCCESS_COMPLETE	0x8000
#define MAC_INV_OPEN_OPTION	0x0100


/*
 * MAC_CLOSE success.
 */
#define MAC_CLOSE_SUCCESS	0x8000

/************************************************************************
 *
 * The following are bit assignents of RING_STATUS and COMMAND_REJECT 
 * fields of SSB returned upon adapter-to-system interrupts.
 *
 ************************************************************************/

/*
 * Bit Assignment of the RING_STATUS field of SSB
 */
#define TRA_RSTAT_SIGNAL_LOSS       0x8000
#define TRA_RSTAT_HARD_ERROR        0x4000
#define TRA_RSTAT_SOFT_ERROR        0x2000
#define TRA_RSTAT_XMIT_BEACON       0x1000
#define TRA_RSTAT_LOBE_WIRE_FAULT   0x0800
#define TRA_RSTAT_AUTO_REM_ERROR    0x0400
#define TRA_RSTAT_RESERVED_0        0x0200
#define TRA_RSTAT_REMOVE_RCVD       0x0100
#define TRA_RSTAT_COUNT_OVERFLOW    0x0080
#define TRA_RSTAT_SINGLE_STATION    0x0040
#define TRA_RSTAT_RING_RECOVERY     0x0020
#define TRA_RSTAT_RESERVED_1        0x001f


/*
 * Bit Assignment of the COMMAND_REJECT field of SSB
 */
#define COMM_REJ_ILL_COMMAND	0x8000
#define COMM_REJ_ADDR_ERROR	0x4000
#define COMM_REJ_ADAPTER_OPEN	0x2000
#define COMM_REJ_ADAPTER_CLOSED	0x1000
#define COMM_REJ_SAME_COMMAND 	0x0800




/*
 * Definition of the ADAPTER_CHECK_BLOCK returned through the 
 * SIFDAT/INC register.
 */
typedef struct {
	u_short ac_status;
	u_short ac_parm_0;
	u_short ac_parm_1;
	u_short ac_parm_2;
} ADAP_CHECK_BLOCK;

/*
 * Bit Assignment of ADAPTER_CHECK_STATUS field of Adapter_Check_Block
 */
#define ADCHECK_DIO_PARITY	0x8000
#define ADCHECK_DMA_RD_ABORT	0x4000
#define ADCHECK_DMA_WR_ABORT	0x2000
#define ADCHECK_ILL_OP_CODE	0x1000
#define ADCHECK_PARITY_ERRORS	0x0800
#define ADCHECK_RESERVED_0	0x0700
#define ADCHECK_RAM_DATA_ERROR	0x0080
#define ADCHECK_RAM_PRT_ERROR	0x0040
#define ADCHECK_RING_UNDERRUN	0x0020
#define ADCHECK_RESERVED_1	0x0010
#define ADCHECK_INV_INTR	0x0008
#define ADCHECK_INV_ERROR_INTR	0x0004
#define ADCHECK_INV_XOP		0x0002
#define ADCHECK_RESERVED_2	0x0001


typedef struct {
        volatile u_short *tms_sifdat;
        volatile u_short *tms_sifdat_inc;
        volatile u_short *tms_sifadr1;
        volatile u_short *tms_sifcmd_sts;
        volatile u_short *tms_sifacl;
        volatile u_short *tms_sifadr2;
        volatile u_short *tms_sifadx;
        volatile u_short *tms_dmalen;
	volatile u_short *csr0_leds;
	volatile u_short *csr1_link_ctrl;
	volatile u_short *csr4_intr_status;
	volatile u_short *csr5_sw_board_reset;
} TRA_REGISTERS;

#define TRA_REGISTER volatile struct tra_register;

/*
 * Adapter internal pointers. These are available immediately
 * after initialization.
 * The set of pointers also contain pointers regarding LLC. But,
 * since we don't use LLC on the TMS380 we don't record them here.
 */
typedef struct {
	u_short	bia;
	u_short	software_level;
	u_short node_addr;
	u_short	grp_addr;
	u_short func_addr;
	u_short phy_drop_numb;
	u_short	upstream_neighbor;
	u_short	upstream_drop_numb;
	u_short	last_ring_poll_addr;
	u_short	xmit_access_priority;
	u_short	src_class_auth;
	u_short	last_attn_code;
	u_short	last_src_addr;
	u_short	last_beacon_type;
	u_short	last_major_vector;
	u_short	ring_status;
	u_short	soft_err_timer_val;
	u_short	ring_intf_err_cnt;
	u_short	local_ring_numb;
	u_short	monitor_error_code;
	u_short	beacon_xmit_type;
	u_short	beacon_rcv_type;
	u_short	frame_corr_sav;
	u_short	beaconing_stn_una;
	u_short	beaconing_stn_phy_drop_numb;
	u_short	mac_buf_ptr;
	u_short	speed_flag;
	u_short	total_ram;
} ADAP_INTERNAL_POINTERS;

#ifdef mips
#define TRA_TMS_SIFDAT_REG          (0X00200000)
#define TRA_TMS_SIFDAT_INC_REG      (0X00200008)
#define TRA_TMS_SIFADR1_REG         (0X00200010)
#define TRA_TMS_SIFCMD_REG          (0X00200018)
#define TRA_TMS_SIFACL_REG          (0X00200020)
#define TRA_TMS_SIFADR2_REG         (0X00200028)
#define TRA_TMS_SIFADX_REG          (0X00200030)
#define TRA_TMS_DMALEN_REG          (0X00200038)
#define TRA_CSR0		    (0x00200040)
#define TRA_CSR1	    	    (0x00200048)
#define TRA_CSR2	    	    (0x00200050)
#define TRA_CSR3	    	    (0x00200058)
#define TRA_CSR4	    	    (0x00200060)
#define TRA_CSR5	    	    (0x00200068)
#define TRA_CSR6	    	    (0x00200070)
#define TRA_CSR7	    	    (0x00200078)
#endif /* mips */


/*
 * For alpha since we use sparse space addressing everything will
 * be double that of the MIPS value.
 */
#ifdef __alpha
#define TRA_TMS_SIFDAT_REG          (0X00400000)
#define TRA_TMS_SIFDAT_INC_REG      (0X00400010)
#define TRA_TMS_SIFADR1_REG         (0X00400020)
#define TRA_TMS_SIFCMD_REG          (0X00400030)
#define TRA_TMS_SIFACL_REG          (0X00400040)
#define TRA_TMS_SIFADR2_REG         (0X00400050)
#define TRA_TMS_SIFADX_REG          (0X00400060)
#define TRA_TMS_DMALEN_REG          (0X00400070)
#define TRA_CSR0		    (0x00400080)
#define TRA_CSR1	    	    (0x00400090)
#define TRA_CSR2	    	    (0x004000a0)
#define TRA_CSR3	    	    (0x004000b0) /* software reset */
#define TRA_CSR4	    	    (0x004000c0)
#define TRA_CSR5	    	    (0x004000d0)
#define TRA_CSR6	    	    (0x004000e0)
#define TRA_CSR7	    	    (0x004000f0)
#endif /* __alpha */

#endif
