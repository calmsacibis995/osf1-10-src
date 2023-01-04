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
#if !defined(SKZ_ERROR_INCLUDE)
#define SKZ_ERROR_INCLUDE


/************************************************************************
 *									*
 * File:	skz_error.h						*
 * Date:	January 15, 1992					*
 * Author:	Ron Hegli                                               *
 *									*
 * Description:  This file contains error code definitions and other    *
 *	defines used by the error reporting system.			*
 *									*
 ************************************************************************/

/*
** XZA Driver Error Types
*/
#define SKZ_ACC_ERROR	1
#define SKZ_AME_ERROR	2
#define SKZ_DSE_ERROR	3
#define SKZ_QB_ERROR	4
#define SKZ_ERROR	5

/*
** XZA Register Masks
*/
#define XZA_XDEV	0x00000001L	/* XDEV Register */
#define XZA_XBE 	0x00000002L	/* XBE Register */
#define XZA_XFADR	0x00000004L	/* XFADR Register */
#define XZA_XCOMM	0x00000008L	/* XCOMM Register */
#define XZA_XFAER	0x00000010L	/* XFAER Register */
#define XZA_XPD1	0x00000020L	/* XPD1 Register */
#define XZA_XPD2	0x00000040L	/* XPD2 Register */
#define XZA_AFAR1	0x00000080L	/* AFAR1 Register */
#define XZA_NRE 	0x00000100L	/* NRE Register */
#define XZA_AFAR0	0x00000200L	/* AFAR0 Register */
#define XZA_XPUD	0x00000400L	/* XPUD Register */
#define XZA_ASR 	0x00000800L	/* ASR Register */
#define XZA_GCQ0IR	0x00001000L	/* GCQ0IR Register */
#define XZA_GCQ1IR	0x00002000L	/* GCQ1IR Register */
#define XZA_QIR 	0x00004000L	/* QIR Register */

/*
** XZA Error Flags
*/
#define	SKZ_LOG_ERROR	0x00000001L	/* Log the error */
#define SKZ_CAM_STATUS	0x00000002L	/* CAM staus field valid */
#define SKZ_RESET_BUS	0x00000004L	/* Set if error calls for bus reset */
#define SKZ_RESET_ADAPTER	0x00000008L 	/* Set for adapter reset */
#define SKZ_LOG_SIM_SOFTC	0x00000010L	/* For CAM_ERROR */
#define SKZ_DISABLE_CHAN	0x00000020L	/* */
#define SKZ_ENABLE_CHAN		0x00000040L	/* */
#define SKZ_EXIT		0x00000080L
#define SKZ_ENABLE_ADAPTER	0x00000100L
#define SKZ_RESET_CMPL		0x00000200L
#define SKZ_REGISTER_DATA	0x00000400L
#define SKZ_BTL_VALID		0x00000800L
#define SKZ_CHAN_VALID		0x00001000L


/************************************************************************
**
**  Error Codes
**
*************************************************************************/

/*
** Adapter Abnormal Condition Codes, ASR ACC values
*/
#define	XZA_ACC_ILL_STATE_TRANS_ERROR	0x1
#define XZA_ACC_EEPROM_UPDATE_ERROR	0x2
#define XZA_ACC_BUS_RESET_RECEIVED	0x3
#define XZA_ACC_UNSOL_RESEL_BUS_WEDGED	0x4
#define XZA_ACC_ILL_CARRIER_ADDRESS	0x5


/*
** Adapter Maintenance Error Codes, ASR AMEC values
*/

/* Adapter-detected AME errors */

#define XZA_AME_FATAL_SIOP_ERROR	0x52
#define XZA_AME_FW_UPDATE_CMPL		0x53
#define XZA_AME_FW_KEEPALIVE_TMO	0x54
#define XZA_AME_POWERFAIL_DETECT	0x55
#define XZA_AME_EEPROM_INVALID		0x56
#define XZA_AME_FW_ENTRY_ERROR		0x57
#define XZA_AME_INIT_ERROR		0x58
#define XZA_AME_INT_HARD_ERROR		0x59
#define XZA_AME_XBE_ERROR		0x5a
#define XZA_AME_HOST_INT_ERROR		0x5b
#define XZA_AME_DATAMOVE_ERROR		0x5c
#define XZA_AME_PEEK_TRANS_ERROR	0x5d
#define XZA_AME_NODE_HALTED		0x5e
#define XZA_AME_MACHINE_CHECK		0x5f

/* AECR Write AME Error Codes */
   /* 60-6f, not currently used */

/* AICR Write AME Error Codes */
#define XZA_AME_ILL_AB_ADDRESS		0x7c
#define XZA_AME_APB_CPB_WRITE_ERROR	0x7d
#define XZA_AME_MISC_INTR_COPY_ERROR	0x7e
#define XZA_AME_SELF_TEST_FAILED	0x7f


/*
** Adapter Data Structure Error Codes, ASR ADSEC values
*/
#define XZA_DSE_NONEXIST_MEMORY		0x1
#define XZA_DSE_ILL_BUF_PTR		0x2
#define XZA_DSE_UNSUPP_BUF_PTR_TYPE	0x3
#define XZA_DSE_ILL_AB_FORMAT		0x4
#define XZA_DSE_REG_PROTOCOL_VIOL	0x5
#define XZA_DSE_INSUF_AMPB_LEN		0x6
#define XZA_DSE_INSUF_QE_LEN		0x7
#define XZA_DSE_ILL_QB_ALIGN		0x8


/*
** Queue Buffer Return Status Codes
*/
#define	XZA_OK				0
#define	XZA_PACKET_SIZE_VIOLATION	1
#define	XZA_BUFFER_LENGTH_VIOLATION	3
#define	XZA_SCSI_STATUS_RET		4
#define XZA_UNRECOGNIZED_CMD		16
#define	XZA_INV_DEST_XPORT		17
#define	XZA_NO_PATH			20
#define	XZA_INSUFF_NEXT_RESRC		64
#define	XZA_CMD_ABORT_IN_PROG		65
#define	XZA_CMD_ABORT_PRIOR_EXEC	66
#define	XZA_CMD_NOT_FOUND		67
#define	XZA_DEVICE_WAS_RESET		68
#define	XZA_BUS_WAS_RESET		69
#define	XZA_AUTOSENSE_DATA_TRUNC	70
#define	XZA_CMD_TIMEOUT			71
#define	XZA_SCSI_BUS_WEDGED		72
#define	XZA_CMD_TIMEOUT_BUS_WEDGED	73
#define	XZA_SIOP_PARITY_ERROR		74
#define	XZA_ILL_PHASE_ERROR		75
#define	XZA_ILL_PHASE_BUS_WEDGED	76
#define	XZA_SELECTION_TIMEOUT		77
#define	XZA_NONSETDIAG_IN_DIAG		78
#define	XZA_SDTR_FAILED			79
#define	XZA_ILL_BUF_PTR			80
#define	XZA_UNEXP_DISCONNECT		81
#define	XZA_MSG_REJECT_BY_TARGET	82


/*
** XZA Port Driver Error Codes
*/
#define SKZ_INVALID_QUEUE_TYPE		0x40
#define SKZ_QUEUE_INSERT_ERROR		0x41
#define SKZ_QUEUE_REMOVE_ERROR		0x42
#define SKZ_STATE_CHANGE_TIMEOUT	0x43
#define SKZ_WRONGSTATE			0x44
#define SKZ_ADAPTER_RESET_TIMEOUT	0x45
#define SKZ_ADAPTER_HALT_TIMEOUT	0x46
#define SKZ_DIAG_TIMEOUT		0x47
#define SKZ_WRONG_DEV			0x48
#define SKZ_SIMATTACH_ERR		0x49
#define SKZ_PROBE_FAILURE		0x4a
#define SKZ_READ_REG_ERR		0x4b
#define SKZ_INVALID_REGISTER		0x4c
#define SKZ_BDRST_TIMEOUT		0x4d
#define SKZ_SELF_TEST_FAILED		0x4e
#define SKZ_NO_CONTROLLERS		0x4f
#define SKZ_INIT_ERROR			0x50
#define SKZ_MEM_ALLOC_ERR		0x51
#define SKZ_PHYS_CONV_ERR		0x52
#define SKZ_VEC_ALLOC_ERR		0x53
#define SKZ_MAX_TRANSFER_ERR		0x54
#define SKZ_SG_ALLOC_ERR		0x55
#define SKZ_EMUL_TIMEOUT_ERR		0x56
#define SKZ_XZA_ADAPT_INIT_ERR		0x57
#define SKZ_BUS_RESET_TIMEOUT		0x58
#define SKZ_STCHGTMO_ENABLE		0x59
#define SKZ_STCHGTMO_DISABLE		0x5a
#define SKZ_EMUL_ERR			0x5b
#define SKZ_REG_WRITE_ERR		0x5c
#define SKZ_DIAG_COMPL			0x5d

/*
** Binary error logger error entries
*/
struct xza_softc_entry {
	vm_offset_t	xza_base;
	u_short		xza_xmi_node;
	u_short		xmi_bus_num;
	u_int		flags;
	u_short		error_code;
};

struct xza_chan_entry {
	u_int		chan_num;
	u_int		scsi_id;
	u_int		state;
	u_int		flags;
	u_long		commands_sent;
};

struct xza_reg_entry {
    u_int		xdev;
    u_int		xbe;
    u_int		xfadr;
    u_int		xfaer;
    u_int		afar0;
    u_int		afar1;
    u_int		asr;
};

/*
** Error Buffer - contains the error descriptor and other dynamic
** 		  information that is put onto the DDEQ for processing
**		  by the error thread
*/

typedef struct skz_err_buf {
    u_long		token;		/* so this can be put on np queues */

    /*
    ** XZA Error Information
    */
    char*		error_module;
    u_long		error_code;
    char*		error_symbol;
    char*		error_text;
    u_long		error_flags;
    u_int		reg_mask;
    u_long		cam_status;
    u_long		cam_flags;

    /*
    ** Saved for error processing
    */
    vm_offset_t		xza_softc;
    vm_offset_t		xza_chan;
    vm_offset_t		sim_softc;
    vm_offset_t		qb_ptr;
    vm_offset_t		sws;

    u_int		adapter;
    u_int		channel;
    u_int		bus_id;
    u_int		targid;
    u_int		lun;

    /*
    ** Error Entry Information
    */
    struct xza_softc_entry	softc_entry;
    struct xza_chan_entry	chan_entry;
    struct xza_reg_entry	reg_entry;

} SKZ_ERR_BUF;

#endif
