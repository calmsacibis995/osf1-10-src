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
static char *rcsid = "@(#)$RCSfile: skz_error.c,v $ $Revision: 1.1.2.19 $ (DEC) $Date: 93/02/05 15:35:02 $";
#endif

/************************************************************************
 *									*
 * File:	skz_error.c						*
 * Date:	January 15, 1992					*
 * Author:	Ron Hegli                                               *
 *									*
 * Description:								*
 *	This file contains the XZA error handling code.			*
 *	Routines:							*
 *		1) skz_error						*
 *		2) skz_error_th						*
 *		3) skz_process_error					*
 *		4) skz_logger						*
 *		5) skz_error_init					*
 *									*
 *	All printed text is in *this* file				*
 *									*
 *									*
 ************************************************************************/

/* #define SKZ_DEBUG */

/*
** Include Files
*/
#include <sys/types.h>
#include <sys/time.h>
#include <sys/lock.h>
#include <sys/syslog.h>
#include <kern/sched_prim.h>
#include <kern/thread.h>
#include <kern/lock.h>

#include <vm/vm_kern.h>
#include <mach/vm_param.h>
#include <mach/kern_return.h>

#include <io/common/devdriver.h>
#include <io/common/iotypes.h>

#include <dec/binlog/errlog.h>
#include <io/cam/cam.h>

#include <io/cam/dec_cam.h>
#include <io/cam/scsi_all.h>
#include <io/cam/cam_logger.h>

#include <io/cam/cam_debug.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/dme.h>
#include <io/cam/sim_common.h>
#include <io/cam/sim.h>

#include <io/cam/xza/skz_params.h>
#include <io/cam/xza/skz_error.h>
#include <io/cam/xza/skz_xza.h>
#include <io/cam/xza/skz_nport.h>
#include <io/cam/xza/skz_xza_qb.h>
#include <io/cam/xza/skz.h>

void skz_process_error ( XZA_SOFTC* );
static void skz_logger ( SKZ_ERR_BUF* );

extern int	shutting_down;

/*
** Error Descriptor Definition
*/

/*
** Error Descriptor - contains static information about each error
** 		      that can occur in the XZA or the skz driver code.
*/
typedef struct skz_error_dsc {
    u_long		error_code;
    char*		error_symbol;
    char*		error_text;
    u_long		error_flags;
    u_int		reg_mask;
    u_long		cam_status;
} SKZ_ERROR_DSC;

/*
** XZA Error Table
*/

SKZ_ERROR_DSC	skz_acc_errors[] = {

    {0,0,0,0,0,0},				/* index == 0 */
    
    {	XZA_ACC_ILL_STATE_TRANS_ERROR,
	"XZA_ACC_ILL_STATE_TRANS_ERROR",
	"Illegal State Transition Error",
	SKZ_LOG_ERROR | SKZ_RESET_ADAPTER,
	XZA_ASR,
	0 	},


    {	XZA_ACC_EEPROM_UPDATE_ERROR,
	"XZA_ACC_EEPROM_UPDATE_ERROR",
	"EEPROM Update Error - Embedded Checksum Invalid",
	SKZ_RESET_ADAPTER | SKZ_LOG_ERROR,
	XZA_ASR,
	0	},
	
    {	XZA_ACC_BUS_RESET_RECEIVED,
	"XZA_ACC_BUS_RESET_RECEIVED",
	"SCSI Bus Reset Received/Completed",
	SKZ_ENABLE_CHAN | SKZ_LOG_ERROR,
	XZA_ASR,
	0	},

    {	XZA_ACC_UNSOL_RESEL_BUS_WEDGED,
	"XZA_ACC_UNSOL_RESEL_BUS_WEDGED",
	"Unsolicited Reselection, SCSI Bus Wedged",
	SKZ_ENABLE_CHAN | SKZ_LOG_ERROR,
	XZA_ASR,
	0	},

    {	XZA_ACC_ILL_CARRIER_ADDRESS,
	"XZA_ACC_ILL_CARRIER_ADDRESS",
	"Illegal Carrier Address",
	SKZ_LOG_ERROR | SKZ_ENABLE_CHAN,
	XZA_AFAR0 | XZA_AFAR1,
	0	}

};

static int	skz_acc_error_entries = (sizeof ( skz_acc_errors ) /
					 sizeof ( SKZ_ERROR_DSC )) - 1;

SKZ_ERROR_DSC	skz_ame_errors[] = {

    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},	/* 0-f */

    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},	/* 10-1f */

    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},	/* 20-2f */

    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},	/* 30-3f */

    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},	/* 40-4f */

    {0,0,0,0,0,0},{0,0,0,0,0,0},				/* 50-51 */

    {	XZA_AME_FATAL_SIOP_ERROR,
	"XZA_AME_FATAL_SIOP_ERROR",
	"Fatal SIOP Error Detected",
	SKZ_RESET_ADAPTER | SKZ_LOG_ERROR,
	XZA_AFAR0 | XZA_ASR,
	0	},

    {	XZA_AME_FW_UPDATE_CMPL,	
	"XZA_AME_FW_UPDATE_CMPL",
	"Firmware Update Completed",
	SKZ_RESET_ADAPTER | SKZ_LOG_ERROR,
	0,
	0	},

    {	XZA_AME_FW_KEEPALIVE_TMO,
	"XZA_AME_FW_KEEPALIVE_TMO",
	"Firmware Keepalive Timeout",
	SKZ_LOG_ERROR | SKZ_RESET_ADAPTER,
	XZA_ASR,
	0 	},

    {	XZA_AME_POWERFAIL_DETECT,
	"XZA_AME_POWERFAIL_DETECT",
	"Power Failure Detected",
	SKZ_LOG_ERROR | SKZ_RESET_ADAPTER,
	XZA_ASR,
	0	},

    {	XZA_AME_EEPROM_INVALID,
	"XZA_AME_EEPROM_INVALID",
	"EEPROM Contents Invalid",
	SKZ_LOG_ERROR | SKZ_RESET_ADAPTER,
	XZA_AFAR0,
	0	},

    {	XZA_AME_FW_ENTRY_ERROR,
	"XZA_AME_FW_ENTRY_ERROR",
	"Firmware Entry Error",
	SKZ_LOG_ERROR | SKZ_RESET_ADAPTER,
	0,
	0	},

    {	XZA_AME_INIT_ERROR,
	"XZA_AME_INIT_ERROR",
	"Initialization Error, Lamb XMI Node ID Invalid",
	SKZ_LOG_ERROR | SKZ_RESET_ADAPTER,
	0,
	0	},

    {	XZA_AME_INT_HARD_ERROR,
	"XZA_AME_INT_HARD_ERROR",
	"Internal Hard Error, Firmware Halted",
	SKZ_LOG_ERROR | SKZ_RESET_ADAPTER,
	0,
	0	},

    {	XZA_AME_XBE_ERROR,
	"XZA_AME_XBE_ERROR",
	"XBE Error",
	SKZ_LOG_ERROR | SKZ_RESET_ADAPTER,
	0,
	0	},

    {	XZA_AME_HOST_INT_ERROR,
	"XZA_AME_HOST_INT_ERROR",
	"Host Interrupt Error",
	SKZ_LOG_ERROR | SKZ_RESET_ADAPTER,
	0,
	0	},

    {	XZA_AME_DATAMOVE_ERROR,
	"XZA_AME_DATAMOVE_ERROR",
	"Datamove Error",
	SKZ_LOG_ERROR | SKZ_RESET_ADAPTER,
	XZA_AFAR0 | XZA_AFAR1 | XZA_XFADR | XZA_XFAER | XZA_ASR,
	0	},

    {	XZA_AME_PEEK_TRANS_ERROR,
	"XZA_AME_PEEK_TRANS_ERROR",
	"Peek Transition Error",
	SKZ_LOG_ERROR | SKZ_RESET_ADAPTER,
	0,
	0	},

    {	XZA_AME_NODE_HALTED,
	"XZA_AME_NODE_HALTED",
	"Node Halted",
	SKZ_LOG_ERROR | SKZ_RESET_ADAPTER,
	0,
	0	},

    {	XZA_AME_MACHINE_CHECK,
	"XZA_AME_MACHINE_CHECK",
	"Machine Check or Unexpected Firmware Exception",
	SKZ_LOG_ERROR | SKZ_RESET_ADAPTER,
	0,
	0	},

    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},	/* 60-6f */

    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},	/* 60-6f */

    {	XZA_AME_ILL_AB_ADDRESS,
	"XZA_AME_ILL_AB_ADDRESS",
	"Illegal Adapter Block Address",
	SKZ_LOG_ERROR | SKZ_EXIT,
	0,
	0	},

    {	XZA_AME_APB_CPB_WRITE_ERROR,
	"XZA_AME_APB_CPB_WRITE_ERROR",
	"APB/CPB Hard Write Error",
	SKZ_LOG_ERROR | SKZ_RESET_ADAPTER,
	0,
	0	},

    {	XZA_AME_MISC_INTR_COPY_ERROR,
	"XZA_AME_MISC_INTR_COPY_ERROR",
	"Miscellaneous Interrupt AB Field Hard Copy Error",
	SKZ_LOG_ERROR | SKZ_EXIT,
	0,
	0	},

    {	XZA_AME_SELF_TEST_FAILED,
	"XZA_AME_SELF_TEST_FAILED",
	"Adapter Failed Self-Test",
	SKZ_LOG_ERROR,
	0,
	0	}
};

static int	skz_ame_error_entries = ( sizeof ( skz_ame_errors ) /
					  sizeof ( SKZ_ERROR_DSC ) ) - 1;

SKZ_ERROR_DSC	skz_dse_errors[] = {

    {0,0,0,0,0,0},

    {	XZA_DSE_NONEXIST_MEMORY,
	"XZA_DSE_NONEXIST_MEMORY",
	"Non-Existent Memory Access Error",
	SKZ_LOG_ERROR | SKZ_RESET_ADAPTER,
	0,
	0	},

    {	XZA_DSE_ILL_BUF_PTR,
	"XZA_DSE_ILL_BUF_PTR",
	"Illegal Buffer Pointer Format, MBZ Field Non-Zero",
	SKZ_LOG_ERROR | SKZ_EXIT,
	0,
	0	},

    {	XZA_DSE_UNSUPP_BUF_PTR_TYPE,
	"XZA_DSE_UNSUPP_BUF_PTR_TYPE",
	"Unsupported Buffer Pointer Type",
	SKZ_LOG_ERROR | SKZ_EXIT,
	0,
	0	},

    {	XZA_DSE_ILL_AB_FORMAT,
	"XZA_DSE_ILL_AB_FORMAT",
	"Illegal Adapter Block Format",
	SKZ_LOG_ERROR | SKZ_EXIT,
	XZA_AFAR0 | XZA_AFAR1 | XZA_ASR,
	0	},

    {	XZA_DSE_REG_PROTOCOL_VIOL,
	"XZA_DSE_REG_PROTOCOL_VIOL",
	"Register Protocol Violation",
	SKZ_LOG_ERROR | SKZ_EXIT,
	0,
	0	},

    {	XZA_DSE_INSUF_AMPB_LEN,
	"XZA_DSE_INSUF_AMPB_LEN",
	"Allocated AMPB Size Insufficient",
	SKZ_LOG_ERROR | SKZ_RESET_ADAPTER,
	0,
	0	},

    {	XZA_DSE_INSUF_QE_LEN,
	"XZA_DSE_INSUF_QE_LEN",
	"Queue Element Size Insufficient",
	SKZ_LOG_ERROR | SKZ_EXIT,
	0,
	0	},

    {	XZA_DSE_ILL_QB_ALIGN,
	"XZA_DSE_ILL_QB_ALIGN",
	"Illegal Queue Buffer Alignment",
	SKZ_LOG_ERROR | SKZ_EXIT,
	0,
	0	}
};

static int	skz_dse_error_entries = ( sizeof ( skz_dse_errors ) /
					  sizeof ( SKZ_ERROR_DSC ) ) - 1;

/*
** These errors must NOT have the RESET_ADAPTER bit set
*/
SKZ_ERROR_DSC	skz_qb_errors[] = {

    {	XZA_OK,
	"XZA_OK",
	"Queue Buffer Process Successfully",
	SKZ_CAM_STATUS,
	0,
	CAM_REQ_CMP	},

    {	XZA_PACKET_SIZE_VIOLATION,
	"XZA_PACKET_SIZE_VIOLATION",
	"Packet Size Violation",
	SKZ_LOG_ERROR | SKZ_CAM_STATUS,
	0,
	CAM_REQ_CMP_ERR	},

    {0,0,0,0,0,0},

    {	XZA_BUFFER_LENGTH_VIOLATION,
	"XZA_BUFFER_LENGTH_VIOLATION",
	"Buffer Length Violation",
	SKZ_LOG_ERROR | SKZ_CAM_STATUS,
	0,
	CAM_REQ_CMP_ERR	},

    {	XZA_SCSI_STATUS_RET,
	"XZA_SCSI_STATUS_RET",
	"SCSI Status Byte Returned",
	SKZ_CAM_STATUS,
	0,
	CAM_REQ_CMP	},

    {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0},
    {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0},
    {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0,0,0},

    {	XZA_UNRECOGNIZED_CMD,
	"XZA_UNRECOGNIZED_CMD",
	"Unrecognized Command",
	SKZ_LOG_ERROR | SKZ_CAM_STATUS,
	0,
	CAM_REQ_INVALID	},

    {	XZA_INV_DEST_XPORT,
	"XZA_INV_DEST_XPORT",
	"Invalid Destination Xport",
	SKZ_CAM_STATUS,
	0,
	CAM_TID_INVALID	},

    {0,0,0,0,0,0}, {0,0,0,0,0,0},

    {	XZA_NO_PATH,
	"XZA_NO_PATH",
	"No Path",
	SKZ_LOG_ERROR | SKZ_CAM_STATUS,
	0,
	CAM_NO_NEXUS	},

    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},

    {	XZA_INSUFF_NEXT_RESRC,
	"XZA_INSUFF_NEXT_RESRC",
	"Insufficient NEXT Resources",
	SKZ_LOG_ERROR | SKZ_CAM_STATUS,
	0,
	CAM_REQ_CMP_ERR	},

    {	XZA_CMD_ABORT_IN_PROG,
	"XZA_CMD_ABORT_IN_PROG",
	"Command Aborted In Progress",
	SKZ_CAM_STATUS,
	0,
	CAM_REQ_ABORTED	},

    {	XZA_CMD_ABORT_PRIOR_EXEC,
	"XZA_CMD_ABORT_PRIOR_EXEC",
	"Command Abort Prior to Execution",
	SKZ_CAM_STATUS,
	0,
	CAM_REQ_ABORTED	},

    {	XZA_CMD_NOT_FOUND,
	"XZA_CMD_NOT_FOUND",
	"Command Not Found",
	SKZ_CAM_STATUS,
	0,
	CAM_UA_ABORT	},

    {	XZA_DEVICE_WAS_RESET,
	"XZA_DEVICE_WAS_RESET",
	"Device Was Reset",
	SKZ_CAM_STATUS,
	0,
	CAM_BDR_SENT	},  /* CHECK!! */

    {	XZA_BUS_WAS_RESET,
	"XZA_BUS_WAS_RESET",
	"Bus Was Reset",
	SKZ_CAM_STATUS,
	0,
	CAM_SCSI_BUS_RESET	},

    {	XZA_AUTOSENSE_DATA_TRUNC,
	"XZA_AUTOSENSE_DATA_TRUNC",
	"Autosense Data Truncated",
	SKZ_LOG_ERROR | SKZ_CAM_STATUS,
	0,
	CAM_AUTOSENSE_FAIL	}, /* CHECK!! */

    {	XZA_CMD_TIMEOUT,
	"XZA_CMD_TIMEOUT",
	"Command Timeout/ Phase Change Timeout Occurred",
	SKZ_LOG_ERROR | SKZ_CAM_STATUS,
	0,
	CAM_CMD_TIMEOUT	},

    {	XZA_SCSI_BUS_WEDGED,
	"XZA_SCSI_BUS_WEDGED",
	"SCSI Bus Wedged",
	SKZ_LOG_ERROR | SKZ_RESET_BUS | SKZ_CAM_STATUS,
	0,
	CAM_REQ_CMP_ERR	},

    {	XZA_CMD_TIMEOUT_BUS_WEDGED,
	"XZA_CMD_TIMEOUT_BUS_WEDGED",
	"Command Timeout Occurred, SCSI Bus Wedged",
	SKZ_LOG_ERROR | SKZ_RESET_BUS | SKZ_CAM_STATUS,
	0,
	CAM_REQ_CMP_ERR	},

    {	XZA_SIOP_PARITY_ERROR,
	"XZA_SIOP_PARITY_ERROR",
	"SIOP Parity Error Detected",
	SKZ_LOG_ERROR | SKZ_CAM_STATUS,
	0,
	CAM_UNCOR_PARITY	},

    {	XZA_ILL_PHASE_ERROR,
	"XZA_ILL_PHASE_ERROR",
	"Illegal SCSI Phase Error",
	SKZ_LOG_ERROR | SKZ_RESET_BUS | SKZ_CAM_STATUS,
	0,
	CAM_SEQUENCE_FAIL	},

    {	XZA_ILL_PHASE_BUS_WEDGED,
	"XZA_ILL_PHASE_BUS_WEDGED",
	"Illegal SCSI Phase Error, SCSI Bus Wedged",
	SKZ_LOG_ERROR | SKZ_RESET_BUS | SKZ_CAM_STATUS,
	0,
	CAM_REQ_CMP_ERR	},

    {	XZA_SELECTION_TIMEOUT,
	"XZA_SELECTION_TIMEOUT",
	"Selection Timeout",
	SKZ_CAM_STATUS,
	0,
	CAM_SEL_TIMEOUT	},

    {	XZA_NONSETDIAG_IN_DIAG,
	"XZA_NONSETDIAG_IN_DIAG",
	"Non-SETDIAG Command Received in Adapter Diagnostic State",
	SKZ_LOG_ERROR | SKZ_CAM_STATUS,
	0,
	CAM_REQ_CMP_ERR	},

    {	XZA_SDTR_FAILED,
	"XZA_SDTR_FAILED",
	"SDTR Failed During Sync SCSI Command",
	SKZ_LOG_ERROR | SKZ_CAM_STATUS,
	0,
	CAM_REQ_CMP	},

    {	XZA_ILL_BUF_PTR,
	"XZA_ILL_BUF_PTR",
	"Illegal Buffer Pointer",
	SKZ_LOG_ERROR | SKZ_CAM_STATUS,
	0,
	CAM_REQ_CMP_ERR	},

    {	XZA_UNEXP_DISCONNECT,
	"XZA_UNEXP_DISCONNECT",
	"Unexpected SCSI Target Disconnect",
	SKZ_LOG_ERROR | SKZ_CAM_STATUS,
	0,
	CAM_UNEXP_BUSFREE	}, /* CHECK!! */

    {	XZA_MSG_REJECT_BY_TARGET,
	"XZA_MSG_REJECT_BY_TARGET",
	"Message Rejected By Target",
	SKZ_CAM_STATUS,
	0,
	CAM_MSG_REJECT_REC	}
};

static int	skz_qb_error_entries = ( sizeof ( skz_qb_errors ) /
				     	 sizeof ( SKZ_ERROR_DSC ) ) - 1;

SKZ_ERROR_DSC	skz_errors[] = {


    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},	/* 0-f */

    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},	/* 10-1f */

    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},	/* 20-2f */

    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
    {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},	/* 30-3f */

    {	SKZ_INVALID_QUEUE_TYPE,
	"SKZ_INVALID_QUEUE_TYPE",
	"Remove/Insert from/to and Invalid Queue",
	SKZ_LOG_ERROR | SKZ_RESET_ADAPTER,
	0,
	0	},

    {	SKZ_QUEUE_INSERT_ERROR,
	"SKZ_QUEUE_INSERT_ERROR",
	"Queue Insertion Error",
	SKZ_LOG_ERROR | SKZ_RESET_ADAPTER,
	0,
	0	},

    {	SKZ_QUEUE_REMOVE_ERROR,
	"SKZ_QUEUE_REMOVE_ERROR",
	"Unable to allocate Queue Buffer from adapter free queue",
	SKZ_LOG_ERROR,
	0,
	0	},

    {	SKZ_STATE_CHANGE_TIMEOUT,
	"SKZ_STATE_CHANGE_TIMEOUT",
	"State Change Timeout",
	SKZ_LOG_ERROR,
	0,
	0	},

    {	SKZ_WRONGSTATE,
	"SKZ_WRONGSTATE",
	"Invalid Adapter/Channel State for Operation",
	SKZ_LOG_ERROR | SKZ_RESET_ADAPTER,
	0,
	0	},

    {	SKZ_ADAPTER_RESET_TIMEOUT,
	"SKZ_ADAPTER_RESET_TIMEOUT",
	"Timeout on Adapter Reset",
	SKZ_LOG_ERROR,
	0,
	0	},

    {	SKZ_ADAPTER_HALT_TIMEOUT,
	"SKZ_ADAPTER_HALT_TIMEOUT",
	"Timeout on Adapter Halt",
	SKZ_LOG_ERROR,
	0,
	0	},

    {	SKZ_DIAG_TIMEOUT,
	"SKZ_DIAG_TIMEOUT",
	"Timeout on Adapter Diagnostic Command",
	SKZ_LOG_ERROR,
	0,
	0	},

    {	SKZ_WRONG_DEV,
	"SKZ_WRONG_DEV",
	"Incorrect Device Type From XDEV Register",
	SKZ_LOG_ERROR,
	XZA_XDEV,
	0	},

    {	SKZ_SIMATTACH_ERR,
	"SKZ_SIMATTACH_ERR",
	"SIM Attach to CAM failed",
	SKZ_LOG_ERROR,
	0,
	0	},

    {	SKZ_PROBE_FAILURE,
	"SKZ_PROBE_FAILURE",
	"XZA Probe Failed",
	SKZ_LOG_ERROR,
	0,
	0	},

    {	SKZ_READ_REG_ERR,
	"SKZ_READ_REG_ERR",
	"Error Reading XZA Adapter Register",
	SKZ_LOG_ERROR,
	0,
	0	},

    {	SKZ_INVALID_REGISTER,
	"SKZ_INVALID_REGISTER",
	"Invalid Register ID",
	SKZ_LOG_ERROR,
	0,
	0	},

    {	SKZ_BDRST_TIMEOUT,
	"SKZ_BDRST_TIMEOUT",
	"Bus Device Reset Timeout",
	SKZ_LOG_ERROR,
	0,
	0	},

    {	SKZ_SELF_TEST_FAILED,
	"SKZ_SELF_TEST_FAILED",
	"XZA Adapter Self-Test Failed",
	SKZ_LOG_ERROR,
	XZA_XBE | XZA_ASR,
	0	},

    {	SKZ_NO_CONTROLLERS,
	"SKZ_NO_CONTROLLERS",
	"No SCSI (skz) controllers configured for adapter",
	SKZ_LOG_ERROR,
	0,
	0	},

    {	SKZ_INIT_ERROR,
	"SKZ_INIT_ERROR",
	"Error initializing adapter",
	SKZ_LOG_ERROR,
	0,
	0	},

    {	SKZ_MEM_ALLOC_ERR,
	"SKZ_MEM_ALLOC_ERR",
	"Error allocating kernel memory",
	SKZ_LOG_ERROR | SKZ_EXIT,
	0,
	0	},

    {	SKZ_PHYS_CONV_ERR,
	"SKZ_PHYS_CONV_ERR",
	"Error converting virtual address to physical",
	SKZ_LOG_ERROR,
	0,
	0	},

    {	SKZ_VEC_ALLOC_ERR,
	"SKZ_VEC_ALLOC_ERR",
	"Error allocating adapter interrupt vector",
	SKZ_LOG_ERROR | SKZ_EXIT,
	0,
	0	},

    {	SKZ_MAX_TRANSFER_ERR,
	"SKZ_MAX_TRANSFER_ERR",
	"Transfer size exceeds XZA max transfer of 8MB",
	SKZ_LOG_ERROR,
	0,
	0	},

    {	SKZ_SG_ALLOC_ERR,
	"SKZ_SG_ALLOC_ERR",
	"Unable to allocate scatter-gather array",
	0,
	0,
	0	},

    {	SKZ_EMUL_TIMEOUT_ERR,
	"SKZ_EMUL_TIMEOUT_ERR",
	"XZA Register Emulation failed to complete",
	SKZ_LOG_ERROR,
	XZA_XPD1,
	0	},

    {	SKZ_XZA_ADAPT_INIT_ERR,
	"SKZ_XZA_ADAPT_INIT_ERR",
	"XZA Initialization Failure, adapter not ready",
	SKZ_LOG_ERROR,
	XZA_XBE | XZA_XPUD,
	0	},

    {	SKZ_BUS_RESET_TIMEOUT,
	"SKZ_BUS_RESET_TIMEOUT",
	"SCSI Bus reset timeout",
	SKZ_LOG_ERROR | SKZ_RESET_ADAPTER,
	0,
	0	},

    {	SKZ_STCHGTMO_ENABLE,
	"SKZ_STCHGTMO_ENABLE",
	"State change timeout on adapter/channel Enable",
	SKZ_LOG_ERROR | SKZ_RESET_ADAPTER,
	XZA_ASR,
	0	},

    {	SKZ_STCHGTMO_DISABLE,
	"SKZ_STCHGTMO_DISABLE",
	"State change timeout on adapter/channel Disable",
	SKZ_LOG_ERROR | SKZ_RESET_ADAPTER,
	XZA_ASR,
	0	},

    {	SKZ_EMUL_ERR,
	"SKZ_EMUL_ERR",
	"Error encountered waiting for register emulation to complete",
	SKZ_LOG_ERROR,
	XZA_ASR | XZA_XPD1 | XZA_XPD2,
	0	},

    {	SKZ_REG_WRITE_ERR,
	"SKZ_REG_WRITE_ERR",
	"Error on register write",
	SKZ_LOG_ERROR,
	XZA_ASR | XZA_XPD1 | XZA_XPD2,
	0	},

    {	SKZ_DIAG_COMPL,
	"SKZ_DIAG_COMPL",
	"XZA diagnostic command complete",
	SKZ_RESET_ADAPTER,
	0,
	0	}
};

static int	skz_error_entries = ( sizeof ( skz_errors ) /
				      sizeof ( SKZ_ERROR_DSC ) ) - 1;





/************************************************************************
 *
 *  ROUTINE NAME:  skz_error()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine looks up error information based on an error type
 *	and an error code.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

void
skz_error ( char* 	error_module,
	    u_int 	error_type,
	    u_int 	error_code,
	    u_int* 	cam_status,
	    vm_offset_t struct_ptr )

{
    u_int		skz_status;
    u_int		s;

    SKZ_ERROR_DSC	error_dsc;

    XZA_SOFTC*		xza_softc = 0;
    SIM_SOFTC*		sim_softc = 0;
    XZA_CHANNEL*	xza_chan = 0;
    QB*			qb_ptr = 0;

    SKZ_ERR_BUF*	eb_ptr;
    CARRIER*		car_ptr;

    SIM_WS*		sws;
    u_long		cam_flags;

    SIM_MODULE(skz_error);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nEntering %s", module) );


    /*
    ** Find the error descriptor based on error type and error code
    */
    switch ( error_type ) {

	case SKZ_ACC_ERROR:

	    /*
	    ** Get error descriptor
	    */
	    if ( error_code > skz_acc_error_entries )
	    {
#ifdef SKZ_DEBUG
		printf ( "Invalid ACC Error Code: %d\n", error_code );
#endif
		return;
	    }
	    else
	        error_dsc = skz_acc_errors[error_code];

	    /*
	    ** Interpret struct pointer based on error_type
	    */
	    xza_chan = (XZA_CHANNEL *) struct_ptr;
	    xza_softc = (XZA_SOFTC *) xza_chan -> xza_softc;
	    sim_softc = xza_chan -> sim_softc;

	    cam_flags = SIM_LOG_HBA_CSR | SIM_LOG_HBA_SOFTC;   
	    cam_flags |= SIM_LOG_PRISEVERE;

	    break;

	case SKZ_AME_ERROR:

	    if ( error_code > skz_ame_error_entries )
	    {
#ifdef SKZ_DEBUG
		printf ( "Invalid AME Error Code: %d\n", error_code );
#endif
		return;
	    }
	    else
		error_dsc = skz_ame_errors[error_code];

	    /*
	    ** Interpret struct pointer based on error_type
	    */
	    xza_chan = (XZA_CHANNEL *) struct_ptr;
	    xza_softc = (XZA_SOFTC *) xza_chan -> xza_softc;
	    sim_softc = xza_chan -> sim_softc;

	    cam_flags = SIM_LOG_HBA_CSR | SIM_LOG_HBA_SOFTC;   
	    cam_flags |= SIM_LOG_PRISEVERE;

	    break;

	case SKZ_DSE_ERROR:

	    if ( error_code > skz_dse_error_entries )
	    {
#ifdef SKZ_DEBUG
		printf ( "Invalid DSE Error Code: %d\n", error_code );
#endif
		return;
	    }
	    else
	        error_dsc = skz_dse_errors[error_code];

	    /*
	    ** Interpret struct pointer based on error_type
	    */
	    xza_chan = (XZA_CHANNEL *) struct_ptr;
	    xza_softc = (XZA_SOFTC *) xza_chan -> xza_softc;
	    sim_softc = xza_chan -> sim_softc;

	    cam_flags = SIM_LOG_HBA_CSR | SIM_LOG_HBA_SOFTC;   
	    cam_flags |= SIM_LOG_PRISEVERE;

	    break;

	case SKZ_QB_ERROR:

	    if ( error_code > skz_qb_error_entries )
	    {
#ifdef SKZ_DEBUG
		printf ( "Invalid QB Error Code: %d\n", error_code );
#endif
		return;
	    }
	    else
	        error_dsc = skz_qb_errors[error_code];

	    /*
	    ** Interpret struct_ptr
	    */
	    qb_ptr = ( QB *) struct_ptr;
	    xza_chan = (XZA_CHANNEL *) qb_ptr -> qb_driver.xza_chan_ptr;
	    xza_softc = (XZA_SOFTC *) xza_chan -> xza_softc;
	    sim_softc = xza_chan -> sim_softc;

	    cam_flags = SIM_LOG_SIM_WS | SIM_LOG_HBA_SOFTC;
	    cam_flags |= SIM_LOG_PRIHIGH;

	    break;

	case SKZ_ERROR:

	    if ( error_code > skz_error_entries )
	    {
#ifdef SKZ_DEBUG
		printf ( "Invalid SKZ Error Code: %d\n", error_code );
#endif
		return;
	    }
	    else
	        error_dsc = skz_errors[error_code];

	    xza_softc = ( XZA_SOFTC * ) struct_ptr;

	    cam_flags = SIM_LOG_HBA_SOFTC;
	    cam_flags |= SIM_LOG_PRIHIGH;

	    break;

	default:

#ifdef SKZ_DEBUG
	    printf ( "\nInvalid Error Type: %d\n", error_type );
#endif
	    return;

    } /* switch */


    /**********************************************************************
    **
    ** Get a free err_buf entry from the free queue and fill in with
    ** error descriptor information
    **
    ************************************************************************/
    skz_status = skz_get_queue ( DDEFQ, &eb_ptr, &car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
#ifdef SKZ_DEBUG
	printf ( "Unable to get free error buffer" );
#endif
	return;
    }

    eb_ptr -> error_module = error_module;
    eb_ptr -> error_code = error_dsc.error_code;
    eb_ptr -> error_text = error_dsc.error_text;
    eb_ptr -> error_symbol = error_dsc.error_symbol;
    eb_ptr -> error_flags = error_dsc.error_flags;
    eb_ptr -> reg_mask = error_dsc.reg_mask;
    eb_ptr -> cam_status = error_dsc.cam_status;

    eb_ptr -> xza_softc = (vm_offset_t) xza_softc;
    eb_ptr -> xza_chan = (vm_offset_t) xza_chan;
    eb_ptr -> sim_softc = (vm_offset_t) sim_softc;
    eb_ptr -> qb_ptr = (vm_offset_t) qb_ptr;

    if ( qb_ptr ) 
    {
	sws = (SIM_WS *) qb_ptr -> qb_driver.sws;

	eb_ptr -> sws = (vm_offset_t) sws;
	eb_ptr -> bus_id = sws -> cntlr;
	eb_ptr -> targid = sws -> targid;
	eb_ptr -> lun = sws -> lun;

	eb_ptr -> error_flags |= SKZ_BTL_VALID;
    }
    else
    {
	eb_ptr -> bus_id = -1;
	eb_ptr -> targid = -1;
	eb_ptr -> lun = -1;
    }

    if ( xza_chan )
    {
	eb_ptr -> channel = xza_chan -> chan_num;
	eb_ptr -> error_flags |= SKZ_CHAN_VALID;
    }

    eb_ptr -> adapter = ((struct bus *) xza_softc -> xza_bus) -> bus_num;

    eb_ptr -> cam_flags = cam_flags;


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_FLOW,
		 ("\n%s: Initialized the eb fields", module) );

    /***********************************************************************
    ** Get Error Entry Information
    ************************************************************************/

    SKZ_LOCK ( xza_softc -> lock, s );

    /***********************************************************************
    ** 
    ** xza_softc information
    **
    ************************************************************************/
    eb_ptr -> softc_entry.xza_base = xza_softc -> xza_base;
    eb_ptr -> softc_entry.xza_xmi_node = xza_softc -> xza_xmi_node;
    eb_ptr -> softc_entry.xmi_bus_num = xza_softc -> xza_bus -> connect_num;
    eb_ptr -> softc_entry.flags = xza_softc -> flags;
    if ( ( error_type == SKZ_ACC_ERROR ) ||
	 ( error_type == SKZ_AME_ERROR ) ||
	 ( error_type == SKZ_DSE_ERROR ) ||
	 ( error_type == SKZ_QB_ERROR ) )
        eb_ptr -> softc_entry.error_code = eb_ptr -> error_code;

    /***********************************************************************
    ** 
    ** xza_chan information
    **
    ************************************************************************/
    if ( xza_chan )
    {
	eb_ptr -> chan_entry.chan_num = xza_chan -> chan_num;
	eb_ptr -> chan_entry.scsi_id = (u_int) xza_chan -> scsi_id.targid;
	eb_ptr -> chan_entry.flags = xza_chan -> flags;
	eb_ptr -> chan_entry.commands_sent = xza_chan -> commands_sent;
	eb_ptr -> chan_entry.state = xza_chan -> state;
    }

    SKZ_UNLOCK ( xza_softc -> lock, s );

    /************************************************************************
    **
    ** Gather Register Information
    **
    *************************************************************************/

    /*
    ** XDEV
    */
    skz_status = skz_read_reg ( XDEV, &(eb_ptr -> reg_entry.xdev), xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	printf ( "\nError reading XDEV register\n" );

    /*
    ** ASR
    */
    if ( error_dsc.reg_mask & XZA_ASR )
    {
	skz_status = skz_read_reg ( ASR, &(eb_ptr -> reg_entry.asr), xza_softc );
	if ( skz_status != CAM_REQ_CMP )
	    printf ( "\nError reading ASR register\n" );
    }
    
    /*
    ** XFADR
    */
    if ( error_dsc.reg_mask & XZA_XFADR )
    {
	skz_status = skz_read_reg ( XFADR, &(eb_ptr -> reg_entry.xfadr), xza_softc );
	if ( skz_status != CAM_REQ_CMP )
	    printf ( "\nError reading XFADR register\n" );
    }
    
    /*
    ** XFAER
    */
    if ( error_dsc.reg_mask & XZA_XFAER )
    {
	skz_status = skz_read_reg ( XFAER, &(eb_ptr -> reg_entry.xfaer), xza_softc );
	if ( skz_status != CAM_REQ_CMP )
	    printf ( "\nError reading XFAER register\n" );
    }
    
    /*
    ** AFAR0
    */
    if ( error_dsc.reg_mask & XZA_AFAR0 )
    {
	skz_status = skz_read_reg ( AFAR0, &(eb_ptr -> reg_entry.afar0), xza_softc );
	if ( skz_status != CAM_REQ_CMP )
	    printf ( "\nError reading AFAR0 register\n" );
    }
    
    /*
    ** AFAR1
    */
    if ( error_dsc.reg_mask & XZA_AFAR1 )
    {
	skz_status = skz_read_reg ( AFAR1, &(eb_ptr -> reg_entry.afar1), xza_softc );
	if ( skz_status != CAM_REQ_CMP )
	    printf ( "\nError reading AFAR1 register\n" );
    }
    
    /*
    ** XBE
    */
    if ( error_dsc.reg_mask & XZA_XBE )
    {
	skz_status = skz_read_reg ( XBE, &(eb_ptr -> reg_entry.xbe), xza_softc );
	if ( skz_status != CAM_REQ_CMP )
	    printf ( "\nError reading XBE register\n" );
    }
    

    /***********************************************************************
    **
    ** CAM Status 
    **
    ************************************************************************/

    /*
    ** If valid, set the CAM status
    */
    if ( error_dsc.error_flags & SKZ_CAM_STATUS )
	*cam_status = (unsigned int) error_dsc.cam_status;

    /*
    ** Right now, at least indicate bus (and adapter) resets
    */
    SKZ_LOCK ( xza_softc -> lock, s );

    if ( error_dsc.error_flags & SKZ_RESET_BUS )
	xza_chan -> flags |= SKZ_RESET_NEEDED;
    else if ( error_dsc.error_flags & SKZ_RESET_ADAPTER )
	xza_softc -> flags |= SKZ_RESET_NEEDED;

    SKZ_UNLOCK ( xza_softc -> lock, s );

    /*
    ** Queue the error buffer and wake up the error thread
    */
    skz_status = skz_put_queue ( DDEQ, eb_ptr, car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	printf ( "\n%s: error inserting eb onto DDEQ", module );

    /*
    ** If we're just booting up, the error thread doesn't exist yet.
    ** If so, just call skz_process_error directly, else queue the
    ** eb to the error thread and wake it up.
    */
    if ( xza_softc -> flags & SKZ_ERROR_TH )
    {
	skz_clear_wait ( xza_softc -> error_th, THREAD_AWAKENED );
    }
    else if ( cam_at_boottime() || shutting_down )
    {
	skz_process_error ( xza_softc );
    }
    else /* we're in an interrupt */
    {
	timeout ( skz_process_error, xza_softc, hz );
    }


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nLeaving %s", module) );

    return;

}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_error_th()
 *
 *  FUNCTIONAL DESCRIPTION:  If threads are active, 
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

skz_error_th ()

{
    u_int	 	skz_status;
    int			s;

    thread_t		thread;

    XZA_SOFTC*		xza_softc;


    SIM_MODULE(skz_error_th);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nEntering %s", module) );

    thread = current_thread();
    thread_swappable(thread, FALSE);

    /* Collect argument left by kernel_thread_w_arg() */
    xza_softc = (XZA_SOFTC *) thread -> reply_port;
    thread -> reply_port = PORT_NULL;

    /*
    ** If the response thread and the error thread have both initialized
    ** then set the THREADS_ACTIVE flag
    */
    SKZ_LOCK ( xza_softc -> lock, s );

    xza_softc -> flags |= SKZ_ERROR_TH; 	

    SKZ_UNLOCK ( xza_softc -> lock, s );

    /*
    ** after we're created, wait for an actual error to get queued
    */
    assert_wait((vm_offset_t)&(xza_softc -> err_buf_pool), TRUE );

    /*
    ** Loop, processing error blocks as they come in
    */
    for ( ;; )
    {
	/*
	** Block until we're awakened 
	*/
	thread_block();

        SIM_PRINTD ( NOBTL,
 		     NOBTL,
		     NOBTL,
		     CAMD_FLOW,
		     ("\n%s: Back from thread_block", 
			module) );
	/*
	** Process the error
	*/
	skz_process_error ( xza_softc );

	SKZ_LOCK ( xza_softc -> lock, s );

        if ( (np_drv_q_entry_present ( &(xza_softc -> ddeq) )
                        != NP_SUCCESS) )
            assert_wait((vm_offset_t)&(xza_softc -> err_buf_pool), TRUE );

	SKZ_UNLOCK ( xza_softc -> lock, s );


        SIM_PRINTD ( NOBTL,
 		     NOBTL,
		     NOBTL,
		     CAMD_FLOW,
		     ("\n%s: Completed Error Processing, about to block", 
			module) );
    } /* for */

}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_process_error()
 *
 *  FUNCTIONAL DESCRIPTION:
 * 	This function takes an error buffer and logs the error.
 *	If the flags field in the error buffer indicate any 
 *	corrective action, this routine attempts to perform it.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

void 
skz_process_error ( XZA_SOFTC* xza_softc )

{
    u_int		skz_status = CAM_REQ_CMP;
    u_int		s;

    XZA_CHANNEL*	xza_chan;

    SIM_SOFTC*		sim_softc;

    SKZ_ERR_BUF*	eb_ptr;
    CARRIER*		car_ptr;

    SIM_MODULE(skz_process_error);


    SIM_PRINTD (NOBTL,
		NOBTL,
		NOBTL,
		CAMD_INOUT,
		("\nEntering %s\n", module) );


    while ( skz_get_queue ( DDEQ, &eb_ptr, &car_ptr, xza_softc )
			== CAM_REQ_CMP )
    {

    /***************************************************************
    **
    ** Log the error
    **
    ****************************************************************/

#ifdef CAMDEBUG
    skz_dump_eb ( eb_ptr );
#endif

    SIM_PRINTD ( NOBTL,
	         NOBTL,
	         NOBTL,
	         CAMD_FLOW,
	         ("\n%s: About to log the error...", 
		 module) );

    /*
    ** Log error if SKZ_LOG_ERROR flag is set for this error
    **
    ** A Kludge is added so that errors reported for lun > 0 are not
    ** logged.  The XZA returns errors, particularly "SIOP Parity Errors",
    ** for luns > 1, which only occurs during bus scan at bootup.  I added
    ** this here so that the boot looks better, and no luns > 0 exist anyway.
    */
    if ( ( (eb_ptr -> error_flags) & SKZ_LOG_ERROR ) &&
	 ( eb_ptr -> lun == 0 ) ||
	 ( eb_ptr -> lun == -1) )  /* don't log for luns > 0 */
    {
#ifdef SKZ_DEBUG
	printf ( "\n%s - %s", 
		eb_ptr -> error_symbol, 
		eb_ptr -> error_text );
	printf ( ", xza%d", eb_ptr -> adapter );
	if ( eb_ptr -> error_flags & SKZ_CHAN_VALID )
	    printf ( ", chan = %d", eb_ptr -> channel );
	if ( eb_ptr -> error_flags & SKZ_BTL_VALID )
	    printf ( ", %d/%d/%d", eb_ptr -> bus_id,
				   eb_ptr -> targid,
				   eb_ptr -> lun );
	printf ( "\n" );

	if ( eb_ptr -> reg_mask & XZA_ASR )
	    printf ( "ASR: %x\n", eb_ptr -> reg_entry.asr );
	if ( eb_ptr -> reg_mask & XZA_XBE )
	    printf ( "XBE: %x\n", eb_ptr -> reg_entry.xbe );
	if ( eb_ptr -> reg_mask & XZA_XFADR )
	    printf ( "XFADR: %x\n", eb_ptr -> reg_entry.xfadr );
	if ( eb_ptr -> reg_mask & XZA_XFAER )
	    printf ( "XFAER: %x\n", eb_ptr -> reg_entry.xfaer );
	if ( eb_ptr -> reg_mask & XZA_AFAR0 )
	    printf ( "AFAR0: %x\n", eb_ptr -> reg_entry.afar0 );
	if ( eb_ptr -> reg_mask & XZA_AFAR1 )
	    printf ( "AFAR1: %x\n", eb_ptr -> reg_entry.afar1 );
#endif
/*
	log ( LOG_ERR, "%s - %s, xza%d", 
		eb_ptr -> error_symbol, 
		eb_ptr -> error_text,
		eb_ptr -> adapter );
	if ( eb_ptr -> error_flags & SKZ_CHAN_VALID )
	    log ( LOG_ERR, ", chan = %d", eb_ptr -> channel );
	if ( eb_ptr -> error_flags & SKZ_BTL_VALID )
	    log ( LOG_ERR, ", bus %d target %d lun %d", 
			eb_ptr -> bus_id,
			eb_ptr -> targid,
			eb_ptr -> lun );

	if ( eb_ptr -> reg_mask & XZA_ASR )
	    log ( LOG_ERR, "ASR: %x\n", eb_ptr -> reg_entry.asr );
	if ( eb_ptr -> reg_mask & XZA_XBE )
	    log ( LOG_ERR, "XBE: %x\n", eb_ptr -> reg_entry.xbe );
	if ( eb_ptr -> reg_mask & XZA_XFADR )
	    log ( LOG_ERR, "XFADR: %x\n", eb_ptr -> reg_entry.xfadr );
	if ( eb_ptr -> reg_mask & XZA_XFAER )
	    log ( LOG_ERR, "XFAER: %x\n", eb_ptr -> reg_entry.xfaer );
	if ( eb_ptr -> reg_mask & XZA_AFAR0 )
	    log ( LOG_ERR, "AFAR0: %x\n", eb_ptr -> reg_entry.afar0 );
	if ( eb_ptr -> reg_mask & XZA_AFAR1 )
	    log ( LOG_ERR, "AFAR1: %x\n", eb_ptr -> reg_entry.afar1 );
*/

	skz_logger ( eb_ptr );

    }


    /***************************************************************
    **
    ** ACTIONS, to be done last
    **
    ****************************************************************/

    /*
    ** Reset the bus?
    */
    if ( eb_ptr -> error_flags & SKZ_RESET_BUS )
    {
	if ( eb_ptr -> sim_softc ) 
	{
	    skz_status = skz_reset_bus ( eb_ptr -> sim_softc );
	    if ( skz_status != CAM_REQ_CMP )
	    {
            	skz_error ( 	module,
				SKZ_ERROR,
				skz_status,
				NULL,
				eb_ptr -> xza_softc );
		return;
	    }
	} /* if sim_softc */
    } /* if reset_bus */

    /*
    ** Reset the Adapter?
    */
    if ( eb_ptr -> error_flags & SKZ_RESET_ADAPTER )
    {

	/*
	** Clear out outstanding commands
	*/

	/* channel 0 */
	xza_chan = (XZA_CHANNEL *) 
			&(((XZA_SOFTC *) eb_ptr -> xza_softc) -> channel[0]);

	if ( xza_chan -> flags & SKZ_ALIVE )
	    skz_clear_command_queues ( xza_chan );

	/* channel 1 */
	xza_chan = (XZA_CHANNEL *) 
			&(((XZA_SOFTC *) eb_ptr -> xza_softc) -> channel[1]);

	if ( xza_chan -> flags & SKZ_ALIVE )
	    skz_clear_command_queues ( xza_chan );

	/*
	** Do the re-init
	*/
    	skz_status = skz_init ( eb_ptr -> xza_softc );
	if ( skz_status != CAM_REQ_CMP )
	{
	    skz_error ( 	module,
				SKZ_ERROR,
				skz_status,
				NULL,
				eb_ptr -> xza_softc );
	    return;
	}

	/*
	** Turn I/O back on 
	*/
	/* channel 0 */
	xza_chan = (XZA_CHANNEL *) 
			&(((XZA_SOFTC *) eb_ptr -> xza_softc) -> channel[0]);

	if ( xza_chan -> flags & SKZ_ALIVE )
	    SC_SCHED_RUN_SM ( xza_chan -> sim_softc );

	/* channel 1 */
	xza_chan = (XZA_CHANNEL *) 
			&(((XZA_SOFTC *) eb_ptr -> xza_softc) -> channel[1]);

	if ( xza_chan -> flags & SKZ_ALIVE )
	    SC_SCHED_RUN_SM ( xza_chan -> sim_softc );


    } /* if reset_adapter */

    /*
    ** Enable Channel?
    */
    if ( eb_ptr -> error_flags & SKZ_ENABLE_CHAN )
	skz_bus_reset_complete ( eb_ptr -> xza_chan );


    /*
    ** Should we die now?
    */
    if ( eb_ptr -> error_flags & SKZ_EXIT )
    {
	printf ( "\nXZA Adapter no longer operational...\n" );
#ifdef XZA_PANIC
	panic ( "XZA: fatal error" );
#endif
    } 


    /*
    ** We're done with this error, return eb to free queue
    */
    skz_status = skz_put_queue ( DDEFQ, eb_ptr, car_ptr, xza_softc );

    }


    SIM_PRINTD (NOBTL,
		NOBTL,
		NOBTL,
		CAMD_INOUT,
		("\nLeaving %s\n", module) );

    return;
}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_logger()
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

void skz_logger ( SKZ_ERR_BUF* eb_ptr )

{
    u_int	skz_status;

    register CAM_ERR_HDR hdr;
    static CAM_ERR_ENTRY entrys[SIM_LOG_SIZE];
    register CAM_ERR_ENTRY *entry;

    SIM_MODULE(skz_logger);

    hdr.hdr_type = CAM_ERR_PKT;
    hdr.hdr_class = CLASS_XZA;
    hdr.hdr_subsystem = SUBSYS_XZA;
    hdr.hdr_size = 0;
    hdr.hdr_entries = 0;
    hdr.hdr_list = entrys;

    if (eb_ptr -> cam_flags & SIM_LOG_PRISEVERE)
	hdr.hdr_pri = EL_PRISEVERE;
    else if (eb_ptr -> cam_flags & SIM_LOG_PRIHIGH)
	hdr.hdr_pri = EL_PRIHIGH;
    else
	hdr.hdr_pri = EL_PRILOW;

    /*
    ** Log the module name.
    */
    if (eb_ptr -> error_module != (char *)NULL) {
	entry = &hdr.hdr_list[hdr.hdr_entries++];
	entry->ent_type = ENT_STR_MODULE;
	entry->ent_size = strlen(eb_ptr -> error_module) + 1;
	entry->ent_vers = 0;
	entry->ent_data = (u_char *) eb_ptr -> error_module;
	entry->ent_pri = PRI_BRIEF_REPORT;
    }

    /*
    ** Log the message.
    */
    if (eb_ptr -> error_text != (char *)NULL) {
	entry = &hdr.hdr_list[hdr.hdr_entries++];
	entry->ent_type = ENT_STRING;
	entry->ent_size = strlen(eb_ptr -> error_text) + 1;
	entry->ent_vers = 0;
	entry->ent_data = (u_char *) eb_ptr -> error_text;
	entry->ent_pri = PRI_BRIEF_REPORT;
    }

    /*
    ** Log the XZA_SOFTC
    */
    if (eb_ptr -> cam_flags & SIM_LOG_HBA_SOFTC) {
	if (eb_ptr -> xza_softc != (vm_offset_t)NULL) {
	    entry = &hdr.hdr_list[hdr.hdr_entries++];
	    entry->ent_type = ENT_XZA_SOFTC;
	    entry->ent_size = sizeof(struct xza_softc_entry);
	    entry->ent_vers = 0;
	    entry->ent_data = (u_char *) &(eb_ptr -> softc_entry);
	    entry->ent_pri = PRI_FULL_REPORT;
	}
	if (eb_ptr -> xza_chan != (vm_offset_t)NULL) {
	    entry = &hdr.hdr_list[hdr.hdr_entries++];
	    entry->ent_type = ENT_XZA_CHAN;
	    entry->ent_size = sizeof(struct xza_chan_entry);
	    entry->ent_vers = 0;
	    entry->ent_data = (u_char *) &(eb_ptr -> chan_entry);
	    entry->ent_pri = PRI_FULL_REPORT;
	}
    }

    /*
    ** Log the XZA Registers
    */
    if (eb_ptr -> cam_flags & SIM_LOG_HBA_CSR) {
	if (eb_ptr -> xza_softc != (vm_offset_t)NULL) {
	    entry = &hdr.hdr_list[hdr.hdr_entries++];
	    entry->ent_type = ENT_XZA_REG;
	    entry->ent_size = sizeof(struct xza_reg_entry);
	    entry->ent_vers = 0;
	    entry->ent_data = (u_char *) &(eb_ptr -> reg_entry);
	    entry->ent_pri = PRI_FULL_REPORT;
	}
    }

    /*
    ** Log the SIM_SOFTC
    */
    if (eb_ptr -> cam_flags & SIM_LOG_SIM_SOFTC) {
	if (eb_ptr -> sim_softc != (vm_offset_t)NULL) {
	    entry = &hdr.hdr_list[hdr.hdr_entries++];
	    entry->ent_type = ENT_SIM_SOFTC;
	    entry->ent_size = sizeof(SIM_SOFTC);
	    entry->ent_vers = SIM_SOFTC_VERS;
	    entry->ent_data = (u_char *) eb_ptr -> sim_softc;
	    entry->ent_pri = PRI_FULL_REPORT;
	}
    }

    /*
    ** Log the SIM_WS
    */
    if (eb_ptr -> cam_flags & SIM_LOG_SIM_WS) {
	if (eb_ptr -> sws != (vm_offset_t)NULL) {
	    entry = &hdr.hdr_list[hdr.hdr_entries++];
	    entry->ent_type = ENT_SIM_WS;
	    entry->ent_size = sizeof(SIM_WS);
	    entry->ent_vers = SIM_WS_VERS;
	    entry->ent_data = (u_char *) eb_ptr -> sws;
	    entry->ent_pri = PRI_FULL_REPORT;
	}
    }

    /*
    ** Call sc_logger to log the common structures.
    */
    cam_logger ( &hdr, eb_ptr -> bus_id, eb_ptr -> targid, eb_ptr -> lun );

}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_error_init()
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

skz_error_init ( XZA_SOFTC* xza_softc )

{
    u_int		skz_status;
    u_int		i;

    SKZ_ERR_BUF*	eb_ptr;
    CARRIER*		car_ptr;

    SIM_MODULE(skz_error_init);


    SIM_PRINTD (NOBTL,
		NOBTL,
		NOBTL,
		CAMD_INOUT,
		("\nEntering %s\n", module) );
    /*
    ** Allocate memory areas and bzero them 
    */
    xza_softc -> err_buf_pool = (SKZ_ERR_BUF *) kmem_alloc
	( kernel_map, SKZ_ERR_BUF_POOL_ENTRIES * sizeof ( SKZ_ERR_BUF ) );
    if ( !(xza_softc -> err_buf_pool) )
	return ( SKZ_MEM_ALLOC_ERR );

    bzero ( xza_softc -> err_buf_pool, 
	    SKZ_ERR_BUF_POOL_ENTRIES * sizeof ( SKZ_ERR_BUF ) );

    xza_softc -> err_buf_car_pool = (CARRIER *) kmem_alloc
	( kernel_map, SKZ_ERR_BUF_CAR_POOL_ENTRIES * sizeof ( CARRIER ) );
    if ( !(xza_softc -> err_buf_car_pool) )
	return ( SKZ_MEM_ALLOC_ERR );

    bzero ( xza_softc -> err_buf_car_pool,
	    SKZ_ERR_BUF_CAR_POOL_ENTRIES * sizeof ( CARRIER ) );
    /*
    ** Initialize the error buffer queues
    */
    SIM_PRINTD (NOBTL,
		NOBTL,
		NOBTL,
		CAMD_FLOW,
		("\n%s: Initializing error buffer queues\n", module) );

    car_ptr = xza_softc -> err_buf_car_pool;

    np_init_queue ( 	DRIVER_DRIVER,
			&(xza_softc -> ddeq),
			car_ptr++ );

    np_init_queue ( 	DRIVER_DRIVER,
			&(xza_softc -> ddefq),
			car_ptr++ );

    /*
    ** Insert entries onto the free queue
    */
    eb_ptr = xza_softc -> err_buf_pool;

    /* driver-driver error free quue */
    for ( i = 0; i < SKZ_ERR_BUF_POOL_ENTRIES; i++ )
    {
	np_insert_dd_q ( &(xza_softc -> ddefq.tail_ptr),
			 car_ptr++,
			 eb_ptr++ );
    }

    SIM_PRINTD (NOBTL,
		NOBTL,
		NOBTL,
		CAMD_INOUT,
		("\nLeaving %s\n", module) );

    return ( CAM_REQ_CMP );
}
