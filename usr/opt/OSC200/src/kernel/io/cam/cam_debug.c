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
static char *rcsid = "@(#)$RCSfile: cam_debug.c,v $ $Revision: 1.1.8.3 $ (DEC) $Date: 1993/11/23 23:07:29 $";

/************************************************************************
 *									*
 * File:	cam_debug.c						*
 * Date:	April 18, 1991						*
 * Author:	Robin Miller						*
 *									*
 * Description:								*
 *	Functions for CAM debugging.					*
 *									*
 * Modification History:						*
 *									*
 * March 7, 1992 by Robin Miller.					*
 *	Properly decode and print the sense specific bytes of request	*
 *	sense data.  Previously, these bytes were not being displayed.	*
 *									*
 * February 26, 1992 by Robin Miller.					*
 *	Added cdbg_DumpLimit variable to control the number of bytes	*
 *	dumped by the cdbg_DumpBuffer() function.			*
 *									*
 * December 10, 1991 by Robin Miller.					*
 *	Modified text in the SenseKeyTable[] to match the SCSI-2 Spec.	*
 *									*
 * December 9, 1991 by Robin Miller.					*
 *	Added decoding of CAM status code and flags in CAM header.	*
 *	Added decoding of CAM_SIM_QFRZDIS is cam flags field.		*
 *									*
 * November 19, 1991 by Robin Miller.					*
 *	Added forward declarations for all debug functions since these	*
 *	extern's have been removed from cam_debug.h.			*
 *									*
 * Nov. 15, 1991 by Janet Schank					*
 *      Removed check for "CAMDEBUG."  All functions in this file are	*
 *      now compiled in independent of CAMDEBUG.			*
 *									*
 * Oct. 22, 1991 by Janet Schank					*
 *      Replaced all 'register name' with register long name'		*
 *									*
 * October 5, 1991 by Robin Miller.					*
 *	o Conditionalize code using KERNEL for user level applications.	*
 *	  These debugging routines are usee by the SCSI Utility (scu).	*
 *	o Added routines to dump additional CAM CCB's.			*
 *	o Added displaying hex value of flags being decoded.		*
 *									*
 * September 10, 1991 by Robin Miller.					*
 *	Ensure dump buffer routine terminates output with a newline.	*
 *									*
 * Sept. 3, 1991 by Maria Vella.					*
 *	Removed reference to pws_sense_ptr.				*
 *									*
 * Aug. 21, 1991 by Bill Dallas						*
 *	Deleted static reference to SenseKeyTable[] so device drivers	*
 *	can use it.							*
 *									*
 * July 23, 1991 by John Gallant					*
 *	Added the debug output for the Autosense residual field in the  *
 *	SCSI I/O CCB.							*
 *									*
 * June 27, 1991 by Robin Miller.					*
 *	Add code to decode additional sense code & qualifier of sense	*
 *	data and display the appropriate message.			*
 *									*
 ************************************************************************/

#ifdef _KERNEL

#include <sys/errno.h>
#include <sys/types.h>
#include <kern/lock.h>
#include <io/common/iotypes.h>
#include <io/cam/dec_cam.h>
#include <mach/vm_param.h>
#include <io/cam/cam.h>
#include <io/cam/pdrv.h>
#include <io/cam/scsi_cdbs.h>
#include <io/cam/scsi_all.h>
#include <io/cam/scsi_status.h>
#include <io/cam/cam_debug.h>

#else /* _KERNEL */

#include <sys/errno.h>
#include <sys/types.h>
#include <io/common/iotypes.h>
#include <io/cam/dec_cam.h>
#include <io/cam/cam.h>
#include <io/cam/pdrv.h>
#include <io/cam/scsi_cdbs.h>
#include <io/cam/scsi_all.h>
#include <io/cam/scsi_status.h>
#include <io/cam/cam_debug.h>

#endif _KERNEL


#if defined(_KERNEL)

/*
 * Available OSF Printing Facilities.
 */
extern int dprintf(), printf(), uprintf(), xprintf();

void (*cdbg_cprintf)() = (void (*)()) printf;

#include "xpr_debug.h"

#if XPR_DEBUG

void (*cdbg_printf)() = (void (*)()) xprintf;
void (*cdbg_xprintf)() = (void (*)()) xprintf;

#else /* XPR_DEBUG */

void (*cdbg_printf)() = (void (*)()) printf;

#endif /* XPR_DEBUG */

#else /* !defined(_KERNEL) */

U32 camdbg_flag = 0L;
U32 camdbg_id = 0L;

extern int printf();

void (*cdbg_printf)() = (void (*)()) printf;

#endif /* defined(_KERNEL) */

#define dp	(*cdbg_printf)		/* Function to print facility. */

/*
 * Forward References:
 */
#if defined(CAMDEBUG)

static void dump_character_array();
caddr_t cdbg_SystemStatus(), cdbg_GetDeviceName();
void cdbg_DumpCCBHeader(), cdbg_DumpCCBHeaderFlags();
void cdbg_DumpSCSIIO(), cdbg_DumpABORT(), cdbg_DumpTERMIO();
void cdbg_DumpGETDEV(), cdbg_DumpPATHINQ(), cdbg_DumpSETDEV();
void cdbg_DumpPDRVws(), cdbg_DumpBuffer(), cdbg_DumpSenseData();
void cdbg_DumpInquiryData(), cdbg_DumpHBACapabilityFlags();
void cdbg_DumpHBATargetSupportFlags(), cdbg_DumpHBAMiscFlags();
void cdbg_DumpHBAAsyncEventFlags();
char *cdbg_SenseMessage();

#endif /* defined(CAMDEBUG) */

#if defined(CAMDEBUG)

/*
 * CAM XPT Function Code Table.
 */
struct CAM_FunctionTable {
	u_char	cam_function;
	caddr_t	status_msg_brief;
	caddr_t	status_msg_full;
} cam_FunctionTable[] = {
    {	XPT_NOOP,		"XPT_NOOP",
				"Execute Nothing"			},
    {	XPT_SCSI_IO,		"XPT_SCSI_IO",
				"Execute the requested SCSI IO"		},
    {	XPT_GDEV_TYPE,		"XPT_GDEV_TYPE",
				"Get the device type information"	},
    {	XPT_PATH_INQ,		"XPT_PATH_INQ",
				"Path Inquiry"				},
    {	XPT_REL_SIMQ,		"XPT_REL_SIMQ",
				"Release the SIM queue that is frozen"	},
    {	XPT_SASYNC_CB,		"XPT_SASYNC_CB",
				"Set Async callback parameters"		},
    {	XPT_SDEV_TYPE,		"XPT_SDEV_TYPE",
				"Set the device type information"	},
    /*
     * XPT SCSI control functions, 0x10 - 0x1F.
     */
    {	XPT_ABORT,		"XPT_ABORT",
				"Abort the selected CCB"		},
    {	XPT_RESET_BUS,		"XPT_RESET_BUS",
				"Reset the SCSI bus"			},
    {	XPT_RESET_DEV,		"XPT_RESET_DEV",
				"Reset the SCSI device, BDR"		},
    {	XPT_TERM_IO,		"XPT_TERM_IO",
				"Terminate the I/O process"		},
    /*
     * HBA engine commands, 0x20 - 0x2F.
     */
    {	XPT_ENG_INQ,		"XPT_ENG_INQ",
				"HBA engine inquiry"			},
    {	XPT_ENG_EXEC,		"XPT_ENG_EXEC",
				"HBA execute engine request"		},
    /*
     * Target mode commands, 0x30 - 0x3F.
     */
    {	XPT_EN_LUN,		"XPT_EN_LUN",
				"Enable LUN, Target mode support"	},
    {	XPT_TARGET_IO,		"XPT_TARGET_IO",
				"Execute the Target IO request"		},

    {	XPT_FUNC,		"XPT_FUNC",
				"Template"				},
    {	XPT_VUNIQUE,		"XPT_VUNIQUE",
				"Vendor unique commands"		}
};

static int cam_FunctionEntrys =
		sizeof(cam_FunctionTable) / sizeof(cam_FunctionTable[0]);

caddr_t
cdbg_CamFunction (cam_function, report_format)
register u_char cam_function;
int report_format;
{
	register struct CAM_FunctionTable *cst = cam_FunctionTable;
	register I32 entrys;

	for (entrys = 0; entrys < cam_FunctionEntrys; cst++) {
	    if (cst->cam_function == cam_function) {
		if (report_format == CDBG_BRIEF) {
		    return (cst->status_msg_brief);
		} else {
		    return (cst->status_msg_full);
		}
	    }
	}
	return ((report_format) ? "Unknown Function Code" : "Unknown");
}

#endif /* defined(CAMDEBUG) */

/*
 * CAM Status Code Table.
 */
struct CAM_StatusTable {
	u_char	cam_status;
	caddr_t	status_msg_brief;
	caddr_t	status_msg_full;
} cam_StatusTable[] = {
    {	CAM_REQ_INPROG,		"CAM_REQ_INPROG",
				"CCB request is in progress"		},
    {	CAM_REQ_CMP,		"CAM_REQ_CMP",
				"CCB request completed w/out error"	},
    {	CAM_REQ_ABORTED,	"CAM_REQ_ABORTED",
				"CCB request aborted by the host"	},
    {	CAM_UA_ABORT,		"CAM_UA_ABORT",
				"Unable to Abort CCB request"		},
    {	CAM_REQ_CMP_ERR,	"CAM_REQ_CMP_ERR",
				"CCB request completed with an error"	},
    {	CAM_BUSY,		"CAM_BUSY",
				"CAM subsystem is busy"			},
    {	CAM_REQ_INVALID,	"CAM_REQ_INVALID",
				"CCB request is invalid"		},
    {	CAM_PATH_INVALID,	"CAM_PATH_INVALID",
				"ID supplied is invalid"		},
    {	CAM_DEV_NOT_THERE,	"CAM_DEV_NOT_THERE",
				"Device not installed/there"		},
    {	CAM_UA_TERMIO,		"CAM_UA_TERMIO",
				"Unable to Terminate I/O CCB request"	},
    {	CAM_SEL_TIMEOUT,	"CAM_SEL_TIMEOUT",
				"Target selection timeout"		},
    {	CAM_CMD_TIMEOUT,	"CAM_CMD_TIMEOUT",
				"Command timed out"			},
    {	CAM_MSG_REJECT_REC,	"CAM_MSG_REJECT_REC",
				"Reject received"			},
    {	CAM_SCSI_BUS_RESET,	"CAM_SCSI_BUS_RESET",
				"Bus reset sent/received"		},
    {	CAM_UNCOR_PARITY,	"CAM_UNCOR_PARITY",
				"Parity error occured"			},
    {	CAM_AUTOSENSE_FAIL,	"CAM_AUTOSENSE_FAIL",
				"Request sense commad failed"		},
    {	CAM_NO_HBA,		"CAM_NO_HBA",
				"No HBA detected Error"			},
    {	CAM_DATA_RUN_ERR,	"CAM_DATA_RUN_ERR",
				"Overrun/underrun error"		},
    {	CAM_UNEXP_BUSFREE,	"CAM_UNEXP_BUSFREE",
				"BUS free"				},
    {	CAM_SEQUENCE_FAIL,	"CAM_SEQUENCE_FAIL",
				"Bus phase sequence failure"		},
    {	CAM_CCB_LEN_ERR,	"CAM_CCB_LEN_ERR",
				"CCB length supplied is inadaquate"	},
    {	CAM_PROVIDE_FAIL,	"CAM_PROVIDE_FAIL",
				"To provide requ. capability"		},
    {	CAM_BDR_SENT,		"CAM_BDR_SENT",
				"A SCSI BDR msg was sent to target"	},
    {	CAM_REQ_TERMIO,		"CAM_REQ_TERMIO",
				"CCB request terminated by the host"	},
    {	CAM_LUN_INVALID,	"CAM_LUN_INVALID",
				"LUN supplied is invalid"		},
    {	CAM_TID_INVALID,	"CAM_TID_INVALID",
				"Target ID supplied is invalid"		},
    {	CAM_FUNC_NOTAVAIL,	"CAM_FUNC_NOTAVAIL",
				"Requested function is not available"	},
    {	CAM_NO_NEXUS,		"CAM_NO_NEXUS",
				"Nexus is not established"		},
    {	CAM_IID_INVALID,	"CAM_IID_INVALID",
				"The initiator ID is invalid"		},
    {	CAM_CDB_RECVD,		"CAM_CDB_RECVD",
				"The SCSI CDB has been received"	},
    {	CAM_SCSI_BUSY,		"CAM_SCSI_BUSY",
				"SCSI bus busy"				}
};
static int cam_StatusEntrys =
		sizeof(cam_StatusTable) / sizeof(cam_StatusTable[0]);

caddr_t
cdbg_CamStatus (cam_status, report_format)
register u_char cam_status;
int report_format;
{
	register struct CAM_StatusTable *cst = cam_StatusTable;
	register I32 entrys;

	for (entrys = 0; entrys < cam_StatusEntrys; cst++) {
	    if (cst->cam_status == (cam_status&CAM_STATUS_MASK)) {
		if (report_format == CDBG_BRIEF) {
		    return (cst->status_msg_brief);
		} else {
		    return (cst->status_msg_full);
		}
	    }
	}
	return ((report_format) ? "Unknown CAM Status" : "Unknown");
}

/*
 * CAM Status Code Table.
 */
struct SCSI_StatusTable {
	u_char	scsi_status;
	caddr_t	status_msg_brief;
	caddr_t	status_msg_full;
} scsi_StatusTable[] = {
    {	SCSI_STAT_GOOD,			"SCSI_STAT_GOOD",
		"Command successfully completed"		},
    {	SCSI_STAT_CHECK_CONDITION,	"SCSI_STAT_CHECK_CONDITION",
		"Error, exception, or abnormal condition"	},
    {	SCSI_STAT_CONDITION_MET,	"SCSI_STAT_CONDITION_MET",
		"Requested operation satisifed"			},
    {	SCSI_STAT_BUSY,			"SCSI_STAT_BUSY",
		"Target is BUSY"				},
    {	SCSI_STAT_INTERMEDIATE,		"SCSI_STAT_INTERMEDIATE",
		"Linked commands successfully completed"	},
    {	SCSI_STAT_INTER_COND_MET,	"SCSI_STAT_INTER_COND_MET",
		"Intermediate-Condition met"			},
    {	SCSI_STAT_RESERVATION_CONFLICT,	"SCSI_STAT_RESERVATION_CONFLICT",
		"Target reservation conflict"			},
    {	SCSI_STAT_COMMAND_TERMINATED,	"SCSI_STAT_COMMAND_TERMINATED",
		"Command terminated by Terminate I/O Message"	},
    {	SCSI_STAT_QUEUE_FULL,		"SCSI_STAT_QUEUE_FULL",
		"Command tag queue is full"			}
};
static int scsi_StatusEntrys =
		sizeof(scsi_StatusTable) / sizeof(scsi_StatusTable[0]);

caddr_t
cdbg_ScsiStatus (scsi_status, report_format)
register u_char scsi_status;
int report_format;
{
	register struct SCSI_StatusTable *cst = scsi_StatusTable;
	register I32 entrys;

	for (entrys = 0; entrys < scsi_StatusEntrys; cst++) {
	    if (cst->scsi_status == scsi_status) {
		if (report_format == CDBG_BRIEF) {
		    return (cst->status_msg_brief);
		} else {
		    return (cst->status_msg_full);
		}
	    }
	}
	return ((report_format) ? "Unknown SCSI Status" : "Unknown");
}

#if defined(CAMDEBUG)

/*
 * Function to Dump Common CCB Header.
 */
void
cdbg_DumpCCBHeader (ccb)
register CCB_HEADER *ccb;
{
    dp ("\nCCB Header:\n");
    dp ("    Address of this CCB .................... *my_addr: 0x%lx\n",
							ccb->my_addr);
    dp ("    Length of the entire CCB ............ cam_ccb_len: 0x%x\n",
							ccb->cam_ccb_len);
    dp ("    The XPT function code ............. cam_func_code: 0x%x (%s)\n",
	ccb->cam_func_code, cdbg_CamFunction(ccb->cam_func_code, CDBG_BRIEF));
    dp ("    CAM subsystem status ................. cam_status: 0x%x (%s)\n",
	ccb->cam_status, cdbg_CamStatus(ccb->cam_status, CDBG_BRIEF));
    /*
     * Display full status if other flags are set.
     */
    if (ccb->cam_status & (CAM_SIM_QFRZN | CAM_AUTOSNS_VALID)) {
        dp ("    CAM Status Code/Flags Set:\n");
        dp ("\t0x%x = %s = %s\n",
		(ccb->cam_status & CAM_STATUS_MASK),
		cdbg_CamStatus(ccb->cam_status, CDBG_BRIEF),
		cdbg_CamStatus(ccb->cam_status, CDBG_FULL));
	if (ccb->cam_status & CAM_SIM_QFRZN) {
	    dp ("\t0x%x = CAM_SIM_QFRZN = The SIM queue is frozen w/this error.\n",
							CAM_SIM_QFRZN);
	}
	if (ccb->cam_status & CAM_AUTOSNS_VALID) {
	    dp ("\t0x%x = CAM_AUTOSNS_VALID = Autosense data is valid for target.\n",
							CAM_AUTOSNS_VALID);
	}
    }
    dp ("    Path ID for the request ............. cam_path_id: 0x%x\n",
							ccb->cam_path_id);
    dp ("    Target device ID .................. cam_target_id: 0x%x\n",
							ccb->cam_target_id);
    dp ("    Target LUN number ................ cam_target_lun: 0x%x\n",
							ccb->cam_target_lun);
    dp ("    Operation flags for subsystem ......... cam_flags: 0x%x\n",
							ccb->cam_flags);
    cdbg_DumpCCBHeaderFlags (ccb->cam_flags);
    dp ("\n");
}

/*
 * Function to Dump the CCB Header Flags.
 */
void
cdbg_DumpCCBHeaderFlags (cam_flags)
register U32 cam_flags;
{
    if (cam_flags == 0) return;

    dp ("    CAM Flags Set:\n");

    if (cam_flags & CAM_CDB_POINTER) {
	dp ("\t0x%x = CAM_CDB_POINTER = The CDB field contains a pointer.\n",
							CAM_CDB_POINTER);
    }
    if (cam_flags & CAM_QUEUE_ENABLE) {
	dp ("\t0x%x = CAM_QUEUE_ENABLE = SIM queue actions are enabled.\n",
							CAM_QUEUE_ENABLE);
    }
    if (cam_flags & CAM_CDB_LINKED) {
	dp ("\t0x%x = CAM_QUEUE_ENABLE = The CCB contains a linked CDB.\n",
							CAM_QUEUE_ENABLE);
    }
    if (cam_flags & CAM_DIS_CALLBACK) {
	dp ("\t0x%x = CAM_DIS_CALLBACK = Disable callback feature.\n",
							CAM_DIS_CALLBACK);
    }
    if (cam_flags & CAM_SCATTER_VALID) {
	dp ("\t0x%x = CAM_SCATTER_VALID = Scatter/gather list is valid.\n",
							CAM_SCATTER_VALID);
    }
    if (cam_flags & CAM_DIS_AUTOSENSE) {
	dp ("\t0x%x = CAM_DIS_AUTOSENSE = Disable auto-sense feature.\n",
							CAM_DIS_AUTOSENSE);
    }

    if ((cam_flags & CAM_DIR_NONE) == CAM_DIR_RESV) {
	dp ("\t0x%x = CAM_DIR_RESV = Data direction is 'RESERVED'.\n",
							CAM_DIR_RESV);
    } else if ((cam_flags & CAM_DIR_NONE) == CAM_DIR_NONE) {
	dp ("\t0x%x = CAM_DIR_NONE = Data direction is 'NO DATA'.\n",
							CAM_DIR_NONE);
    } else if (cam_flags & CAM_DIR_IN) {
	dp ("\t0x%x = CAM_DIR_IN = Data direction is 'DATA IN'.\n",
							CAM_DIR_IN);
    } else if (cam_flags & CAM_DIR_OUT) {
	dp ("\t0x%x = CAM_DIR_OUT = Data direction is 'DATA OUT'.\n",
							CAM_DIR_OUT);
    }

    if (cam_flags & CAM_ENG_SYNC) {
	dp ("\t0x%x = CAM_ENG_SYNC = Flush residual bytes before completing.\n",
							CAM_ENG_SYNC);
    }
    if (cam_flags & CAM_SIM_QFRZDIS) {
	dp ("\t0x%x = CAM_SIM_QFRZDIS = Disable SIM Q freeze on errors.\n",
							CAM_SIM_QFRZDIS);
    }
    if (cam_flags & CAM_SIM_QFREEZE) {
	dp ("\t0x%x = CAM_SIM_QFREEZE = Return the SIM Q to frozen state.\n",
							CAM_SIM_QFREEZE);
    }
    if (cam_flags & CAM_SIM_QHEAD) {
	dp ("\t0x%x = CAM_SIM_QHEAD = Place CCB at the head of SIM Q.\n",
							CAM_SIM_QHEAD);
    }
    if (cam_flags & CAM_DIS_SYNC) {
	dp ("\t0x%x = CAM_DIS_SYNC = Disable sync, go to async mode.\n",
							CAM_DIS_SYNC);
    }
    if (cam_flags & CAM_INITIATE_SYNC) {
	dp ("\t0x%x = CAM_INITIATE_SYNC = Attempt Sync data xfer, and SDTR.\n",
							CAM_INITIATE_SYNC);
    }
    if (cam_flags & CAM_DIS_DISCONNECT) {
	dp ("\t0x%x = CAM_DIS_DISCONNECT = Disable disconnect.\n",
							CAM_DIS_DISCONNECT);
    }

    if (cam_flags & CAM_CALLBCK_PHYS) {
	dp ("\t0x%x = CAM_CALLBCK_PHYS = Callback function pointer is physical.\n",
							CAM_CALLBCK_PHYS);
    }
    if (cam_flags & CAM_NXT_CCB_PHYS) {
	dp ("\t0x%x = CAM_NXT_CCB_PHYS = Next CCB pointer is physical.\n",
							CAM_NXT_CCB_PHYS);
    }
    if (cam_flags & CAM_MSG_BUF_PHYS) {
	dp ("\t0x%x = CAM_MSG_BUF_PHYS = Message buffer pointer is physical.\n",
							CAM_MSG_BUF_PHYS);
    }
    if (cam_flags & CAM_SNS_BUF_PHYS) {
	dp ("\t0x%x = CAM_SNS_BUF_PHYS = Auto-sense data pointer is physical.\n",
							CAM_SNS_BUF_PHYS);
    }
    if (cam_flags & CAM_DATA_PHYS) {
	dp ("\t0x%x = CAM_DATA_PHYS = SG/Buffer data pointers are physical.\n",
							CAM_DATA_PHYS);
    }
    if (cam_flags & CAM_CDB_PHYS) {
	dp ("\t0x%x = CAM_CDB_PHYS = The CDB pointer is physical.\n",
							CAM_CDB_PHYS);
    }
    if (cam_flags & CAM_ENG_SGLIST) {
	dp ("\t0x%x = CAM_CDB_PHYS = The SG list is for the HBA engine.\n",
							CAM_CDB_PHYS);
    }

    if (cam_flags & CAM_DIS_AUTOSRP) {
	dp ("\t0x%x = CAM_DIS_AUTOSRP = Disable auto-save/restore pointers.\n",
							CAM_DIS_AUTOSRP);
    }
    if (cam_flags & CAM_DIS_AUTODISC) {
	dp ("\t0x%x = CAM_DIS_AUTODISC = Disable auto-disconnect.\n",
							CAM_DIS_AUTODISC);
    }
    if (cam_flags & CAM_TGT_CCB_AVAIL) {
	dp ("\t0x%x = CAM_TGT_CCB_AVAIL = Target CCB is available.\n",
							CAM_TGT_CCB_AVAIL);
    }
    if (cam_flags & CAM_TGT_PHASE_MODE) {
	dp ("\t0x%x = CAM_TGT_PHASE_MODE = The SIM will run in phase mode.\n",
							CAM_TGT_PHASE_MODE);
    }
    if (cam_flags & CAM_MSGB_VALID) {
	dp ("\t0x%x = CAM_MSGB_VALID = Message buffer is valid.\n",
							CAM_MSGB_VALID);
    }
    if (cam_flags & CAM_STATUS_VALID) {
	dp ("\t0x%x = CAM_STATUS_VALID = Status buffer is valid.\n",
							CAM_STATUS_VALID);
    }
    if (cam_flags & CAM_DATAB_VALID) {
	dp ("\t0x%x = CAM_DATAB_VALID = Data buffer is valid.\n",
							CAM_DATAB_VALID);
    }
}

/*
 * Function to Dump SCSI I/O CCB.
 */
void
cdbg_DumpSCSIIO (ccb)
register CCB_SCSIIO *ccb;
{
    register CCB_HEADER *ccbh = (CCB_HEADER *)ccb;
    register u_char *cdbp;
    register caddr_t strp;
    register int i;

    dp ("\nDumping SCSI I/O CCB at 0x%lx:\n", ccb);

    cdbg_DumpCCBHeader (ccbh);

    dp ("    Peripheral driver pointer ......... *cam_pdrv_ptr: 0x%lx\n",
							ccb->cam_pdrv_ptr);
    if (ccb->cam_pdrv_ptr) {
	cdbg_DumpPDRVws ((PDRV_WS *)ccb->cam_pdrv_ptr);
    }
    dp ("    Next CCB pointer .................. *cam_next_ccb: 0x%lx\n",
							ccb->cam_next_ccb);
    dp ("    Request mapping info pointer ....... *cam_req_map: 0x%lx\n",
							ccb->cam_req_map);
    dp ("    Callback completion function ........ *cam_cbfcnp: 0x%lx\n",
							ccb->cam_cbfcnp);
    dp ("    Data buffer/SG list pointer ....... *cam_data_ptr: 0x%lx\n",
							ccb->cam_data_ptr);
    dp ("    Data transfer length .............. cam_dxfer_len: %d (0x%x)\n",
				    ccb->cam_dxfer_len, ccb->cam_dxfer_len);
    dp ("    Sense data buffer pointer ........ *cam_sense_ptr: 0x%lx\n",
							ccb->cam_sense_ptr);
    dp ("    Sense data buffer length .......... cam_sense_len: %d\n",
							ccb->cam_sense_len);
    dp ("    Number of bytes in this CDB ......... cam_cdb_len: %d\n",
							ccb->cam_cdb_len);
    dp ("    Scatter gather list entries ...... cam_sglist_cnt: %d\n",
							ccb->cam_sglist_cnt);
    dp ("    SCSI device status code ......... cam_scsi_status: 0x%x (%s)\n",
	ccb->cam_scsi_status, cdbg_ScsiStatus(ccb->cam_scsi_status, CDBG_BRIEF));
    dp ("    Autosense residual length ....... cam_sense_resid: %d\n",
							ccb->cam_sense_resid);
    dp ("    Transfer residual length .............. cam_resid: %d\n",
							ccb->cam_resid);
    if (ccbh->cam_flags & CAM_CDB_POINTER) {
	cdbp = ccb->cam_cdb_io.cam_cdb_ptr;
        dp ("    Pointer to command descriptor block...cam_cdb_ptr: 0x%lx\n",
								cdbp);
    } else {
	cdbp = (u_char *) ccb->cam_cdb_io.cam_cdb_bytes;
        dp ("    Command descriptor block in CCB at..cam_cdb_bytes: 0x%lx\n",
								cdbp);
    }
    dp ("    Command descriptor block bytes ....... cam_cdb_io:");
    for (i = 0; i < ccb->cam_cdb_len; i++) {
	dp (" %x", (u_char) cdbp[i]);
    }
    dp ("\n");

    if (ccb->cam_timeout == CAM_TIME_DEFAULT) {
	strp = "(Default)";
    } else if (ccb->cam_timeout == CAM_TIME_INFINITY) {
	strp = "(Infinite)";
    } else {
	strp = "seconds";
    }
    dp ("    Command timeout value ............... cam_timeout: %d %s\n",
						ccb->cam_timeout, strp);
    dp ("    Message buffer pointer ............. *cam_msg_ptr: 0x%lx\n",
							ccb->cam_msg_ptr);
    dp ("    Message buffer length .............. cam_msgb_len: %d\n",
							ccb->cam_msgb_len);
    dp ("    Vendor unique flags ................ cam_vu_flags: 0x%x\n",
							ccb->cam_vu_flags);
    dp ("    Tag queuing action ............... cam_tag_action: 0x%x\n\n",
							ccb->cam_tag_action);
}

/*
 * Function to Dump SCSI I/O CCB.
 */
void
cdbg_DumpPDRVws (pws)
register PDRV_WS *pws;
{
    dp ("    Dumping Peripheral Working Set at 0x%lx:\n", pws);

    dp ("        Working set forward link ......... *pws_flink: 0x%lx\n",
							pws->pws_flink);
    dp ("        Working set backward link ........ *pws_flink: 0x%lx\n",
							pws->pws_blink);
    dp ("        Pointer to this CCB ................ *pws_ccb: 0x%lx\n",
							pws->pws_ccb);
    dp ("        Working set flags ................. pws_flags: 0x%x\n",
							pws->pws_flags);
    dp ("        Retry count for request ....... pws_retry_cnt: 0x%x\n",
							pws->pws_retry_cnt);
    dp ("        Peripheral driver structure ptr ... *pws_pdrv: 0x%lx\n",
							pws->pws_pdrv);
    dp ("        Auto-sense buffer in WS at .... pws_sense_buf: 0x%lx\n",
							pws->pws_sense_buf);
}

/*
 * Function to Dump Abort CCB.
 */
void
cdbg_DumpABORT (ccb)
register CCB_ABORT *ccb;
{
    dp ("\nDumping Abort CCB at 0x%lx:\n", ccb);

    cdbg_DumpCCBHeader (ccb);

    dp ("    Pointer to CCB being aborted ....... cam_abort_ch: 0x%lx\n\n",
							ccb->cam_abort_ch);
}

/*
 * Function to Dump Get Device Type CCB.
 */
void
cdbg_DumpGETDEV (ccb)
register CCB_GETDEV *ccb;
{
    dp ("\nDumping Get Device Type CCB at 0x%lx:\n", ccb);

    cdbg_DumpCCBHeader (ccb);

    dp ("    Pointer to inquiry data buffer ..... cam_inq_data: 0x%lx\n",
							ccb->cam_inq_data);
    dp ("    Peripheral device type from EDT ....  cam_pd_type: 0x%x (%s)\n\n",
		ccb->cam_pd_type, cdbg_GetDeviceName (ccb->cam_pd_type));
}

/*
 * Function to Dump Set Device Type CCB.
 */
void
cdbg_DumpSETDEV (ccb)
register CCB_SETDEV *ccb;
{
    dp ("\nDumping Set Device Type CCB at 0x%lx:\n", ccb);

    cdbg_DumpCCBHeader (ccb);

    dp ("    Peripheral device type being set ... cam_dev_type: 0x%x (%s)\n",
		ccb->cam_dev_type, cdbg_GetDeviceName (ccb->cam_dev_type));
}

/*
 * Function to Dump Terminate I/O CCB.
 */
void
cdbg_DumpTERMIO (ccb)
register CCB_TERMIO *ccb;
{
    dp ("\nDumping Terminate I/O CCB at 0x%lx:\n", ccb);

    cdbg_DumpCCBHeader (ccb);

    dp ("    Pointer to CCB being terminated ... cam_termio_ch: 0x%lx\n\n",
							ccb->cam_termio_ch);
}

/*
 * Function to Path Inquiry CCB.
 */
void
cdbg_DumpPATHINQ (ccb)
register CCB_PATHINQ *ccb;
{
    register int i;

    dp ("\nDumping Path Inquiry CCB at 0x%lx:\n", ccb);

    cdbg_DumpCCBHeader (ccb);

    dp ("    Version number for the SIM/HBA .. cam_version_num: %d.%d\n",
					(ccb->cam_version_num >> 4) & 0x0f,
					(ccb->cam_version_num & 0x0f) );
    dp ("    SCSI HBA capabilities flags ..... cam_hba_inquiry: 0x%x\n",
							ccb->cam_hba_inquiry);
    cdbg_DumpHBACapabilityFlags (ccb->cam_hba_inquiry);

    dp ("    Target mode support flags ....... cam_target_sprt: 0x%x\n",
							ccb->cam_target_sprt);
    cdbg_DumpHBATargetSupportFlags (ccb->cam_target_sprt);

    dp ("    Miscellaneous HBA feature flags .... cam_hba_misc: 0x%x\n",
							ccb->cam_hba_misc);
    cdbg_DumpHBAMiscFlags (ccb->cam_hba_misc);

    dp ("    The HBA engine count ............ cam_hba_eng_cnt: 0x%x\n",
							ccb->cam_hba_eng_cnt);
    dp ("    Vendor unique capabilities .....  cam_vuhba_flags:");
    for (i = 0; i < VUHBA; i++) {
	dp (" 0x%x", ccb->cam_vuhba_flags[i]);
    }
    dp ("\n");
    dp ("    Size of SIM private data area ...... cam_sim_priv: 0x%x\n",
							ccb->cam_sim_priv);
    dp ("    Async callback capabilities ..... cam_async_flags: 0x%x\n",
							ccb->cam_async_flags);
    cdbg_DumpHBAAsyncEventFlags (ccb->cam_async_flags);

    dp ("    Highest HBA path ID assigned ....... cam_hpath_id: 0x%x\n",
							ccb->cam_hpath_id);
    dp ("    SCSI device ID of initiator .... cam_initiator_id: 0x%x\n",
							ccb->cam_initiator_id);
    dp ("    Reserved fields ........... cam_prsvd0/cam_prsvd1: 0x%x 0x%x\n",
					ccb->cam_prsvd0, ccb->cam_prsvd1);
    dp ("    The Vendor ID of the SIM ............ cam_sim_vid: ");
    dump_character_array (ccb->cam_sim_vid, SIM_ID);

    dp ("    The Vendor ID of the HBA ............ cam_hba_vid: ");
    dump_character_array (ccb->cam_hba_vid, HBA_ID);

    dp ("    The OSD usage pointer ............ *cam_osd_usage: 0x%x\n\n",
							ccb->cam_osd_usage);
}

/*
 * Function to Dump the Path Inquiry CCB SCSI HBA Capabilities Flags.
 */
void
cdbg_DumpHBACapabilityFlags (hba_flags)
register u_char hba_flags;
{
    if (hba_flags == 0) return;

    dp ("    Capabilities Flags Set:\n");

    if (hba_flags & PI_MDP_ABLE) {
	dp ("\t0x%x = PI_MDP_ABLE = Supports MDP message.\n", PI_MDP_ABLE);
    }
    if (hba_flags & PI_WIDE_32) {
	dp ("\t0x%x = PI_WIDE_32 = Supports 32 bit wide SCSI.\n", PI_WIDE_32);
    }
    if (hba_flags & PI_WIDE_16) {
	dp ("\t0x%x = PI_WIDE_16 = Supports 16 bit wide SCSI.\n", PI_WIDE_16);
    }
    if (hba_flags & PI_SDTR_ABLE) {
	dp ("\t0x%x = PI_SDTR_ABLE = Supports SDTR message.\n", PI_SDTR_ABLE);
    }
    if (hba_flags & PI_LINKED_CDB) {
	dp ("\t0x%x = PI_LINKED_CDB = Supports linked CDBs.\n", PI_LINKED_CDB);
    }
    if (hba_flags & PI_TAG_ABLE) {
	dp ("\t0x%x = PI_TAG_ABLE = Supports tag queue message.\n", PI_TAG_ABLE);
    }
    if (hba_flags & PI_SOFT_RST) {
	dp ("\t0x%x = PI_SOFT_RST = Supports soft reset.\n", PI_SOFT_RST);
    }
}

/*
 * Function to Dump the Path Inquiry CCB SCSI HBA Capabilities Flags.
 */
void
cdbg_DumpHBATargetSupportFlags (target_support_flags)
register u_char target_support_flags;
{
    if (target_support_flags == 0) return;

    dp ("    Target Mode Support Flags Set:\n");

    if (target_support_flags & PIT_PROCESSOR) {
	dp ("\t0x%x = PIT_PROCESSOR = Target mode processor mode.\n",
							PIT_PROCESSOR);
    }

    if (target_support_flags & PIT_PHASE) {
	dp ("\t0x%x = PIT_PHASE = Target mode phase cognizant mode.\n",
							PIT_PHASE);
    }
}

/*
 * Function to Dump the Path Inquiry CCB SCSI HBA Miscellaneous Flags.
 */
void
cdbg_DumpHBAMiscFlags (hba_misc_flags)
register u_char hba_misc_flags;
{
    if (hba_misc_flags == 0) return;

    dp ("    Miscellaneous Flags Set:\n");

    if (hba_misc_flags & PIM_SCANHILO) {
	dp ("\t0x%x = PIM_SCANHILO = Bus scans from ID 7 to ID 0.\n",
							PIM_SCANHILO);
    }
    if (hba_misc_flags & PIM_NOREMOVE) {
	dp ("\t0x%x = PIM_NOREMOVE = Removable device not included in scan.\n",
							PIM_NOREMOVE);
    }

    if (hba_misc_flags & PIM_NOINQUIRY) {
	dp ("\t0x%x = PIM_NOINQUIRY = Inquiry data not kept by XPT.\n",
							PIM_NOINQUIRY);
    }
}

/*
 * Function to Dump the Path Inquiry CCB Asyn Event Capabilities Flags.
 */
void
cdbg_DumpHBAAsyncEventFlags (async_flags)
register u_char async_flags;
{
    if (async_flags == 0) return;

    dp ("    Asynchronous Event Capabilities Flags Set:\n");

    if (async_flags & AC_FOUND_DEVICES) {
	dp ("\t0x%x = AC_FOUND_DEVICES = During a rescan new device found.\n",
							AC_FOUND_DEVICES);
    }
    if (async_flags & AC_SIM_DEREGISTER) {
	dp ("\t0x%x = AC_SIM_DEREGISTER = A loaded SIM has de-registered.\n",
							AC_SIM_DEREGISTER);
    }
    if (async_flags & AC_SIM_REGISTER) {
	dp ("\t0x%x = AC_SIM_REGISTER = A loaded SIM has registered.\n",
							AC_SIM_REGISTER);
    }
    if (async_flags & AC_SENT_BDR) {
	dp ("\t0x%x = AC_SENT_BDR = A BDR message was sent to target.\n",
							AC_SENT_BDR);
    }
    if (async_flags & AC_SCSI_AEN) {
	dp ("\t0x%x = AC_SCSI_AEN = A SCSI AEN has been received.\n",
							AC_SCSI_AEN);
    }
    if (async_flags & AC_UNSOL_RESEL) {
	dp ("\t0x%x = AC_UNSOL_RESEL = A unsolicited reselection occurred.\n",
							AC_UNSOL_RESEL);
    }
    if (async_flags & AC_BUS_RESET) {
	dp ("\t0x%x = AC_BUS_RESET = A SCSI bus RESET occurred.\n",
							AC_BUS_RESET);
    }
}

/*
 * Function to Dump Data Buffer in Hex Bytes.
 */
u_long cdbg_DumpLimit = 512;		/* Patchable dump buffer limit.	*/

void
cdbg_DumpBuffer (buffer, size)
char *buffer;
int size;
{
	register int count;
	register int limit = 
	     size < (int)cdbg_DumpLimit ? size : (int)cdbg_DumpLimit;
	register int field_entrys = 16;
	register u_char *bptr = (u_char *)buffer;

	/*
	 * Displaying a header is left to the caller.
	 */
	for (count = 0; count < limit; count++) {
	    if ((count % field_entrys) == 0) {
		dp ("\n0x%lx ", bptr);
	    }
	    dp (" %02x", *bptr++);
	}
	if (count) dp ("\n");
}

/*
 * cdbg_GetDeviceName() - Get Device Type String.
 */
char *
cdbg_GetDeviceName (device_type)
register int device_type;
{
	static struct {
		char *name;
		char typ;
	} dnames[] = {
		"Direct Access",		ALL_DTYPE_DIRECT,
		"Sequential Access",		ALL_DTYPE_SEQUENTIAL,
		"Printer",			ALL_DTYPE_PRINTER,
		"Processor",			ALL_DTYPE_PROCESSOR,
		"Write-Once/Read-Many",		ALL_DTYPE_WORM,
		"Read-Only Direct Access",	ALL_DTYPE_RODIRECT,
		"Scanner",			ALL_DTYPE_SCANNER,
		"Optical",			ALL_DTYPE_OPTICAL,
		"Changer",			ALL_DTYPE_CHANGER,
		"Communications Device",	ALL_DTYPE_COMM,
		"(not present)",		ALL_DTYPE_NONE,
		0
	};
	register I32 i;

	for (i = 0; dnames[i].name; i++) {
		if (dnames[i].typ == device_type) {
			return (dnames[i].name);
		}
	}
	return ("Unknown Device");
}

static void
dump_character_array (fptr, field_length)
register char *fptr;
register int field_length;
{
	register unsigned c;

	while (field_length--) {
	    if ((c = *fptr++) == '\0' || c >= 0177) {
		break;
	    } else {
		dp ("%c", c);
	    }
	}
	dp ("\n");
}

/*
 * Yes/no Table.
 */
static char *yesno_table[] = {
	"No",
	"Yes"
};
static char *pqual_table[] = {
	"Peripheral Device Connected",				/* 0x0 */
	"Peripheral Device NOT Connected",			/* 0x1 */
	"Reserved",						/* 0x2 */
	"No Physical Device Support"				/* 0x3 */
};

static char *ansi_table[] = {
	"Level 0",						/* 0x0 */
	"SCSI-1 Compliant",					/* 0x1 */
	"SCSI-2 Compliant",					/* 0x2 */
	"Reserved"					  /* 0x3 - 0x7 */
};

static char *rdf_table[] = {
	"ANSI SCSI-1",						/* 0x00 */
	"CCS",							/* 0x01 */
	"ANSI SCSI-2",						/* 0x02 */
	"Reserved"					 /* 0x03 - 0x1F */
};

/*
 * Function to Dump Inquiry Data Buffer.
 */
void
cdbg_DumpInquiryData (inquiry)
register ALL_INQ_DATA *inquiry;
{
    dp ("\nDumping Inquiry Data at 0x%lx:\n", inquiry);

    dp ("    Peripheral Device Type .......... : %d (%s)\n",
		inquiry->dtype, cdbg_GetDeviceName(inquiry->dtype));

    dp ("    Peripheral Qualifier ............ : 0x%x (%s)\n",
		inquiry->pqual,
		(inquiry->pqual & ALL_PQUAL_VENDOR_SPEC) ?
			"Vendor Specific" : pqual_table[inquiry->pqual]);

    dp ("    Device Type Qualifier ........... : 0x%x\n", inquiry->dmodify);

    dp ("    Removable Media ................. : %s\n",
					yesno_table[inquiry->rmb]);

    dp ("    ANSI Version .................... : %d (%s)\n",
			inquiry->ansi, ansi_table[inquiry->ansi&0x03]);

    dp ("    ECMA Version .................... : %d\n", inquiry->ecma);

    dp ("    ISO Version ..................... : %d\n", inquiry->iso);

    dp ("    Response Data Format ............ : %d (%s)\n",
				inquiry->rdf, rdf_table[inquiry->rdf&0x03]);

    if (inquiry->rdf == ALL_RDF_SCSI2) {
        dp ("    Terminate I/O Process ........... : %d\n", inquiry->trmiop);
        dp ("    Asynchronous Notification ....... : %d\n", inquiry->aenc);
    }

    dp ("    Additional Length ............... : %d\n", inquiry->addlen);

    if (inquiry->rdf == ALL_RDF_SCSI2) {
        dp ("    Soft Reset Support .............. : %d\n", inquiry->sftre);
        dp ("    Command Queuing Support ......... : %d\n", inquiry->cmdque);
	dp ("    Linked Command Support .......... : %d\n", inquiry->linked);
	dp ("    Synchronous Data Transfers ...... : %d\n", inquiry->sync);
	dp ("    Support for 16 Bit Transfers .... : %d\n", inquiry->wbus16);
	dp ("    Support for 32 Bit Transfers .... : %d\n", inquiry->wbus32);
	dp ("    Relative Addressing Support ..... : %d\n", inquiry->reladdr);
    }

    dp ("    Vendor Identification ........... : ");
    dump_character_array (inquiry->vid, sizeof(inquiry->vid));

    dp ("    Product Identification .......... : ");
    dump_character_array (inquiry->pid, sizeof(inquiry->pid));

    dp ("    Firmware Revision Level ......... : ");
    dump_character_array (inquiry->revlevel, sizeof(inquiry->revlevel));
}
#endif /* defined(CAMDEBUG) */

/*
 * Sense Key Error Table.
 */
char *cdbg_SenseKeyTable[] = {
	"NO SENSE - No error or no sense information",		/* 0x00 */
	"RECOVERED ERROR - Recovery action performed",		/* 0x01 */
	"NOT READY - Logical unit is NOT ready",		/* 0x02 */
	"MEDIUM ERROR - Nonrecoverable medium error",		/* 0x03 */
	"HARDWARE ERROR - Nonrecoverable hardware error",	/* 0x04 */
	"ILLEGAL REQUEST - Illegal request or CDB parameter",	/* 0x05 */
	"UNIT ATTENTION - Medium changed or target reset",	/* 0x06 */
	"DATA PROTECT - Data protected from this operation",	/* 0x07 */
	"BLANK CHECK - No-data condition occured",		/* 0x08 */
	"Vendor Specific",					/* 0x09 */
	"COPY ABORTED - Copy command was aborted",		/* 0x0a */
	"ABORTED COMMAND - Target aborted command",		/* 0x0b */
	"EQUAL - Search satisfied with equal comparison",	/* 0x0c */
	"VOLUME OVERFLOW - Physical end of media detected",	/* 0x0d */
	"MISCOMPARE - Source and medium data differ",		/* 0x0e */
	"RESERVED",						/* 0x0f */
};

#if defined(CAMDEBUG)

/*
 * Sense Code/Qualifier Table:
 */
struct sense_entry SenseCodeTable[] = {
/*
                     Table 7-41: ASC and ASCQ Assignments
--------------------------------------------------------------------------

                 ASC AND ASCQ ASSIGNMENTS

      D          = Direct Access Device
       T         = Sequential Access Device
        L        = Printer Device
         P       = Processor Device
          W      = Write Once Read Multiple Device
           R     = Read Only (CD-ROM) Device
            S    = Scanner Device
             O   = Optical Memory Device
              M  = Media Changer Device
               C = Communication Device
     BYTE
   12    13     DTLPWRSOMC    DESCRIPTION
   --    --                   -------------------------------------------- */
{ 0x13, 0x00, /*D   W  O  */ "Address mark not found for data field" },
{ 0x12, 0x00, /*D   W  O  */ "Address mark not found for ID field" },
{ 0x00, 0x11, /*     R    */ "Audio play operation in progress" },
{ 0x00, 0x12, /*     R    */ "Audio play operation paused" },
{ 0x00, 0x14, /*     R    */ "Audio play operation stopped due to error" },
{ 0x00, 0x13, /*     R    */ "Audio play operation successfully completed" },
{ 0x00, 0x04, /* T    S   */ "Beginning-of-partition/medium detected" },
{ 0x14, 0x04, /* T        */ "Block sequence error" },
{ 0x30, 0x02, /*DT  WR O  */ "Cannot read medium - incompatible format" },
{ 0x30, 0x01, /*DT  WR O  */ "Cannot read medium - unknown format" },
{ 0x52, 0x00, /* T        */ "Cartridge fault" },
{ 0x3F, 0x02, /*DTLPWRSOMC*/ "Changed operating definition" },
{ 0x11, 0x06, /*    WR O  */ "Circ unrecovered error" },
{ 0x30, 0x03, /*DT        */ "Cleaning cartridge installed" },
{ 0x4A, 0x00, /*DTLPWRSOMC*/ "Command phase error" },
{ 0x2C, 0x00, /*DTLPWRSOMC*/ "Command sequence error" },
{ 0x2F, 0x00, /*DTLPWRSOMC*/ "Commands cleared by another initiator" },
{ 0x2B, 0x00, /*DTLPWRSO C*/ "Copy cannot execute since host cannot disconnect" },
{ 0x41, 0x00, /*D         */ "Data path failure (should use 40 nn)" },
{ 0x4B, 0x00, /*DTLPWRSOMC*/ "Data phase error" },
{ 0x11, 0x07, /*    W  O  */ "Data resychronization error" },
{ 0x16, 0x00, /*D   W  O  */ "Data synchronization mark error" },
{ 0x19, 0x00, /*D      O  */ "Defect list error" },
{ 0x19, 0x03, /*D      O  */ "Defect list error in grown list" },
{ 0x19, 0x02, /*D      O  */ "Defect list error in primary list" },
{ 0x19, 0x01, /*D      O  */ "Defect list not available" },
{ 0x1C, 0x00, /*D      O  */ "Defect list not found" },
{ 0x32, 0x01, /*D   W  O  */ "Defect list update failure" },
#ifdef notdef
{ 0x40, 0xNN, /*DTLPWRSOMC*/ "Diagnostic failure on component nn (80h-ffh)" },
#endif notdef
{ 0x40, 0x00, /*DTLPWRSOMC*/ "Diagnostic failure on component 0 (80h-ffh)" },
{ 0x63, 0x00, /*     R    */ "End of user area encountered on this track" },
{ 0x00, 0x05, /* T    S   */ "End-of-data detected" },
{ 0x14, 0x03, /* T        */ "End-of-data not found" },
{ 0x00, 0x02, /* T    S   */ "End-of-partition/medium detected" },
{ 0x51, 0x00, /* T     O  */ "Erase failure" },
{ 0x0A, 0x00, /*DTLPWRSOMC*/ "Error log overflow" },
{ 0x11, 0x02, /*DT  W SO  */ "Error too long to correct" },
{ 0x03, 0x02, /* T        */ "Excessive write errors" },
{ 0x3B, 0x07, /*  L       */ "Failed to sense bottom-of-form" },
{ 0x3B, 0x06, /*  L       */ "Failed to sense top-of-form" },
{ 0x00, 0x01, /* T        */ "Filemark detected" },
{ 0x14, 0x02, /* T        */ "Filemark or setmark not found" },
{ 0x09, 0x02, /*    WR O  */ "Focus servo failure" },
{ 0x31, 0x01, /*D L    O  */ "Format command failed" },
{ 0x58, 0x00, /*       O  */ "Generation does not exist" },
{ 0x1C, 0x02, /*D      O  */ "Grown defect list not found" },
{ 0x00, 0x06, /*DTLPWRSOMC*/ "I/o process terminated" },
{ 0x10, 0x00, /*D   W  O  */ "Id crc or ecc error" },
{ 0x22, 0x00, /*D         */ "Illegal function (should use 20 00, 24 00, or 26 00)" },
{ 0x64, 0x00, /*     R    */ "Illegal mode for this track" },
{ 0x28, 0x01, /*        M */ "Import or export element accessed" },
{ 0x30, 0x00, /*DT  WR OM */ "Incompatible medium installed" },
{ 0x11, 0x08, /* T        */ "Incomplete block read" },
{ 0x48, 0x00, /*DTLPWRSOMC*/ "Initiator detected error message received" },
{ 0x3F, 0x03, /*DTLPWRSOMC*/ "Inquiry data has changed" },
{ 0x44, 0x00, /*DTLPWRSOMC*/ "Internal target failure" },
{ 0x3D, 0x00, /*DTLPWRSOMC*/ "Invalid bits in identify message" },
{ 0x2C, 0x02, /*      S   */ "Invalid combination of windows specified" },
{ 0x20, 0x00, /*DTLPWRSOMC*/ "Invalid command operation code" },
{ 0x21, 0x01, /*        M */ "Invalid element address" },
{ 0x24, 0x00, /*DTLPWRSOMC*/ "Invalid field in cdb" },
{ 0x26, 0x00, /*DTLPWRSOMC*/ "Invalid field in parameter list" },
{ 0x49, 0x00, /*DTLPWRSOMC*/ "Invalid message error" },
{ 0x11, 0x05, /*    WR O  */ "L-ec uncorrectable error" },
{ 0x60, 0x00, /*      S   */ "Lamp failure" },
{ 0x5B, 0x02, /*DTLPWRSOM */ "Log counter at maximum" },
{ 0x5B, 0x00, /*DTLPWRSOM */ "Log exception" },
{ 0x5B, 0x03, /*DTLPWRSOM */ "Log list codes exhausted" },
{ 0x2A, 0x02, /*DTL WRSOMC*/ "Log parameters changed" },
{ 0x21, 0x00, /*DT  WR OM */ "Logical block address out of range" },
{ 0x08, 0x00, /*DTL WRSOMC*/ "Logical unit communication failure" },
{ 0x08, 0x02, /*DTL WRSOMC*/ "Logical unit communication parity error" },
{ 0x08, 0x01, /*DTL WRSOMC*/ "Logical unit communication time-out" },
{ 0x05, 0x00, /*DTL WRSOMC*/ "Logical unit does not respond to selection" },
{ 0x4C, 0x00, /*DTLPWRSOMC*/ "Logical unit failed self-configuration" },
{ 0x3E, 0x00, /*DTLPWRSOMC*/ "Logical unit has not self-configured yet" },
{ 0x04, 0x01, /*DTLPWRSOMC*/ "Logical unit is in process of becoming ready" },
{ 0x04, 0x00, /*DTLPWRSOMC*/ "Logical unit not ready, cause not reportable" },
{ 0x04, 0x04, /*DTL    O  */ "Logical unit not ready, format in progress" },
{ 0x04, 0x02, /*DTLPWRSOMC*/ "Logical unit not ready, initializing command required" },
{ 0x04, 0x03, /*DTLPWRSOMC*/ "Logical unit not ready, manual intervention required" },
{ 0x25, 0x00, /*DTLPWRSOMC*/ "Logical unit not supported" },
{ 0x15, 0x01, /*DTL WRSOM */ "Mechanical positioning error" },
{ 0x53, 0x00, /*DTL WRSOM */ "Media load or eject failed" },
{ 0x3B, 0x0D, /*        M */ "Medium destination element full" },
{ 0x31, 0x00, /*DT  W  O  */ "Medium format corrupted" },
{ 0x3A, 0x00, /*DTL WRSOM */ "Medium not present" },
{ 0x53, 0x02, /*DT  WR OM */ "Medium removal prevented" },
{ 0x3B, 0x0E, /*        M */ "Medium source element empty" },
{ 0x43, 0x00, /*DTLPWRSOMC*/ "Message error" },
{ 0x3F, 0x01, /*DTLPWRSOMC*/ "Microcode has been changed" },
{ 0x1D, 0x00, /*D   W  O  */ "Miscompare during verify operation" },
{ 0x11, 0x0A, /*DT     O  */ "Miscorrected error" },
{ 0x2A, 0x01, /*DTL WRSOMC*/ "Mode parameters changed" },
{ 0x07, 0x00, /*DTL WRSOM */ "Multiple peripheral devices selected" },
{ 0x11, 0x03, /*DT  W SO  */ "Multiple read errors" },
{ 0x00, 0x00, /*DTLPWRSOMC*/ "No additional sense information" },
{ 0x00, 0x15, /*     R    */ "No current audio status to return" },
{ 0x32, 0x00, /*D   W  O  */ "No defect spare location available" },
{ 0x11, 0x09, /* T        */ "No gap found" },
{ 0x01, 0x00, /*D   W  O  */ "No index/sector signal" },
{ 0x06, 0x00, /*D   WR OM */ "No reference position found" },
{ 0x02, 0x00, /*D   WR OM */ "No seek complete" },
{ 0x03, 0x01, /* T        */ "No write current" },
{ 0x28, 0x00, /*DTLPWRSOMC*/ "Not ready to ready transition (medium may have changed)" },
{ 0x5A, 0x01, /*DT  WR OM */ "Operator medium removal request" },
{ 0x5A, 0x00, /*DTLPWRSOM */ "Operator request or state change input (unspecified)" },
{ 0x5A, 0x03, /*DT  W  O  */ "Operator selected write permit" },
{ 0x5A, 0x02, /*DT  W  O  */ "Operator selected write protect" },
{ 0x61, 0x02, /*      S   */ "Out of focus" },
{ 0x4E, 0x00, /*DTLPWRSOMC*/ "Overlapped commands attempted" },
{ 0x2D, 0x00, /* T        */ "Overwrite error on update in place" },
{ 0x3B, 0x05, /*  L       */ "Paper jam" },
{ 0x1A, 0x00, /*DTLPWRSOMC*/ "Parameter list length error" },
{ 0x26, 0x01, /*DTLPWRSOMC*/ "Parameter not supported" },
{ 0x26, 0x02, /*DTLPWRSOMC*/ "Parameter value invalid" },
{ 0x2A, 0x00, /*DTL WRSOMC*/ "Parameters changed" },
{ 0x03, 0x00, /*DTL W SO  */ "Peripheral device write fault" },
{ 0x50, 0x02, /* T        */ "Position error related to timing" },
{ 0x3B, 0x0C, /*      S   */ "Position past beginning of medium" },
{ 0x3B, 0x0B, /*      S   */ "Position past end of medium" },
{ 0x15, 0x02, /*DT  WR O  */ "Positioning error detected by read of medium" },
{ 0x29, 0x00, /*DTLPWRSOMC*/ "Power on, reset, or bus device reset occurred" },
{ 0x42, 0x00, /*D         */ "Power-on or self-test failure (should use 40 nn)" },
{ 0x1C, 0x01, /*D      O  */ "Primary defect list not found" },
{ 0x40, 0x00, /*D         */ "Ram failure (should use 40 nn)" },
{ 0x15, 0x00, /*DTL WRSOM */ "Random positioning error" },
{ 0x3B, 0x0A, /*      S   */ "Read past beginning of medium" },
{ 0x3B, 0x09, /*      S   */ "Read past end of medium" },
{ 0x11, 0x01, /*DT  W SO  */ "Read retries exhausted" },
{ 0x14, 0x01, /*DT  WR O  */ "Record not found" },
{ 0x14, 0x00, /*DTL WRSO  */ "Recorded entity not found" },
{ 0x18, 0x02, /*D   WR O  */ "Recovered data - data auto-reallocated" },
{ 0x18, 0x05, /*D   WR O  */ "Recovered data - recommend reassignment" },
{ 0x17, 0x05, /*D   WR O  */ "Recovered data using previous sector ID" },
{ 0x18, 0x03, /*     R    */ "Recovered data with CIRC" },
{ 0x18, 0x01, /*D   WR O  */ "Recovered data with error correction and retries applied" },
{ 0x18, 0x00, /*DT  WR O  */ "Recovered data with error correction applied" },
{ 0x18, 0x04, /*     R    */ "Recovered data with LEC" },
{ 0x17, 0x03, /*DT  WR O  */ "Recovered data with negative head offset" },
{ 0x17, 0x00, /*DT  WRSO  */ "Recovered data with no error correction applied" },
{ 0x17, 0x02, /*DT  WR O  */ "Recovered data with positive head offset" },
{ 0x17, 0x01, /*DT  WRSO  */ "Recovered data with retries" },
{ 0x17, 0x04, /*    WR O  */ "Recovered data with retries and/or CIRC applied" },
{ 0x17, 0x06, /*D   W  O  */ "Recovered data without ECC - data auto-reallocated" },
{ 0x17, 0x07, /*D   W  O  */ "Recovered data without ECC - recommend reassignment" },
{ 0x1E, 0x00, /*D   W  O  */ "Recovered id with ECC correction" },
{ 0x3B, 0x08, /* T        */ "Reposition error" },
{ 0x36, 0x00, /*  L       */ "Ribbon, ink, or toner failure" },
{ 0x37, 0x00, /*DTL WRSOMC*/ "Rounded parameter" },
{ 0x5C, 0x00, /*D      O  */ "Rpl status change" },
{ 0x39, 0x00, /*DTL WRSOMC*/ "Saving parameters not supported" },
{ 0x62, 0x00, /*      S   */ "Scan head positioning error" },
{ 0x47, 0x00, /*DTLPWRSOMC*/ "Scsi parity error" },
{ 0x54, 0x00, /*   P      */ "Scsi to host system interface failure" },
{ 0x45, 0x00, /*DTLPWRSOMC*/ "Select or reselect failure" },
{ 0x3B, 0x00, /* TL       */ "Sequential positioning error" },
{ 0x00, 0x03, /* T        */ "Setmark detected" },
{ 0x3B, 0x04, /*  L       */ "Slew failure" },
{ 0x09, 0x03, /*    WR O  */ "Spindle servo failure" },
{ 0x5C, 0x02, /*D      O  */ "Spindles not synchronized" },
{ 0x5C, 0x01, /*D      O  */ "Spindles synchronized" },
{ 0x1B, 0x00, /*DTLPWRSOMC*/ "Synchronous data transfer error" },
{ 0x55, 0x00, /*   P      */ "System resource failure" },
{ 0x33, 0x00, /* T        */ "Tape length error" },
{ 0x3B, 0x03, /*  L       */ "Tape or electronic vertical forms unit not ready" },
{ 0x3B, 0x01, /* T        */ "Tape position error at beginning-of-medium" },
{ 0x3B, 0x02, /* T        */ "Tape position error at end-of-medium" },
{ 0x3F, 0x00, /*DTLPWRSOMC*/ "Target operating conditions have changed" },
{ 0x5B, 0x01, /*DTLPWRSOM */ "Threshold condition met" },
{ 0x26, 0x03, /*DTLPWRSOMC*/ "Threshold parameters not supported" },
{ 0x2C, 0x01, /*      S   */ "Too many windows specified" },
{ 0x09, 0x00, /*DT  WR O  */ "Track following error" },
{ 0x09, 0x01, /*    WR O  */ "Tracking servo failure" },
{ 0x61, 0x01, /*      S   */ "Unable to acquire video" },
{ 0x57, 0x00, /*     R    */ "Unable to recover table-of-contents" },
{ 0x53, 0x01, /* T        */ "Unload tape failure" },
{ 0x11, 0x00, /*DT  WRSO  */ "Unrecovered read error" },
{ 0x11, 0x04, /*D   W  O  */ "Unrecovered read error - auto reallocate failed" },
{ 0x11, 0x0B, /*D   W  O  */ "Unrecovered read error - recommend reassignment" },
{ 0x11, 0x0C, /*D   W  O  */ "Unrecovered read error - recommend rewrite the data" },
{ 0x46, 0x00, /*DTLPWRSOMC*/ "Unsuccessful soft reset" },
{ 0x59, 0x00, /*       O  */ "Updated block read" },
{ 0x61, 0x00, /*      S   */ "Video acquisition error" },
{ 0x50, 0x00, /* T        */ "Write append error" },
{ 0x50, 0x01, /* T        */ "Write append position error" },
{ 0x0C, 0x00, /* T    S   */ "Write error" },
{ 0x0C, 0x02, /*D   W  O  */ "Write error - auto reallocation failed" },
{ 0x0C, 0x01, /*D   W  O  */ "Write error recovered with auto reallocation" },
{ 0x27, 0x00, /*DT  W  O  */ "Write protected" },
{ 0xDE, 0x00, /*DT  WR O  */ "Drive may need to be spun down" }
};
int SenseCodeEntrys = sizeof(SenseCodeTable) / sizeof(struct sense_entry);

/*
 * Function to Dump Sense Data.
 */
void
cdbg_DumpSenseData (sdp)
register struct all_req_sns_data *sdp;
{
    register int sense_length = (int) sdp->addition_len + 7;

    dp ("\nDumping Request Sense Data at 0x%lx:\n", sdp);
    dp ("    Error code ...................... : 0x%x\n",
						(sdp->error_code & 0xf));
    dp ("    Error class ..................... : 0x%x\n",
						(sdp->error_code >> 4) & 0x7);
    dp ("    Information fields valid ........ : %d\n", sdp->valid);
    dp ("    Segment number .................. : 0x%x\n", sdp->segment);
    dp ("    Sense Key ....................... : 0x%x (%s)\n",
			sdp->sns_key, cdbg_SenseKeyTable[sdp->sns_key]);
    dp ("    Illegal length indicator ........ : %d\n", sdp->ili);
    dp ("    End of medium ................... : %d\n", sdp->eom);
    dp ("    Tape file mark detected ......... : %d\n", sdp->filemark);
    dp ("    Information byte 3 .............. : 0x%x\n", sdp->info_byte3);
    dp ("    Information byte 2 .............. : 0x%x\n", sdp->info_byte2);
    dp ("    Information byte 1 .............. : 0x%x\n", sdp->info_byte1);
    dp ("    Information byte 0 .............. : 0x%x\n", sdp->info_byte0);
    dp ("    Additional sense length ......... : 0x%x\n",
						     sdp->addition_len);
    if ( (sense_length -= 8) > 0) {
      dp ("    Command information byte 3 ...... : 0x%x\n",
							sdp->cmd_specific3);
      dp ("    Command information byte 2 ...... : 0x%x\n",
							sdp->cmd_specific2);
      dp ("    Command information byte 1 ...... : 0x%x\n",
							sdp->cmd_specific1);
      dp ("    Command information byte 0 ...... : 0x%x\n",
							sdp->cmd_specific0);
      sense_length -= 4;
    }
    if (sense_length > 0) {
      char *ascq_msg;

      dp ("    Additional sense code ........... : 0x%x\n",
							sdp->asc);
      dp ("    Additional sense qualifier ...... : 0x%x\n",
							sdp->asq);
      if ( (ascq_msg = cdbg_SenseMessage(sdp)) != (char *) 0) {
          dp ("    Sense Code/Qualifier Message .... : %s\n", ascq_msg);
      }
      sense_length -=2;
    }
    dp ("    Field replaceable unit code ..... : 0x%x\n", sdp->fru);
    /*
     * Sense specific sense data.
     */
    if (sense_length > 0) {
      struct all_sks_ill_req *sksi;
      sksi = (struct all_sks_ill_req *)&sdp->sense_specific.sks_ill_req;
      if (sksi->sksv && (sdp->sns_key == ALL_ILLEGAL_REQ) ) {
	dp ("    Bit pointer to field in error ... : 0x%x\n",
							sksi->bit_pointer);
	dp ("    Bit pointer field valid (BPV) ... : 0x%x\n", sksi->bpv);
	dp ("    Error field command/data (C/D) .. : 0x%x\n", sksi->c_or_d);
	dp ("    Field pointer field (MSB) ....... : 0x%x\n", sksi->field_ptr1);
	dp ("    Field pointer field (LSB) ....... : 0x%x\n", sksi->field_ptr0);
	dp ("    Sense-key specific field valid .. : 0x%x\n", sksi->sksv);
      } else if (sksi->sksv && ( (sdp->sns_key == ALL_RECOVER_ERR) ||
				 (sdp->sns_key == ALL_HARDWARE_ERR) ||
				 (sdp->sns_key == ALL_MEDIUM_ERR) ) ) {
	struct all_sks_retry_cnt *sksr;
	sksr = (struct all_sks_retry_cnt *) sksi;
	dp ("    Sense-key specific field valid .. : 0x%x\n", sksr->sksv);
	dp ("    Actual retry count (MSB) ........ : 0x%x\n", sksr->retry_cnt1);
	dp ("    Actual retry count (LSB) ........ : 0x%x\n", sksr->retry_cnt0);
      } else {
	int sks_length = sizeof(*sksi);
	u_char *ssbp = (u_char *) sksi;
	dp ("    Sense specific bytes ............ :");
	do {
	    dp (" 0x%x", *ssbp++);
	} while (--sks_length);
	dp ("\n");
      }
      sense_length -= 3;
    }
    /*
     * Additional sense bytes (if any);
     */
    if (sense_length > 0) {
	u_char *asbp = (u_char *)sdp->additional_sense.other_sns;
	dp ("    Additional sense bytes .......... :");
	do {
	    dp (" 0x%x", *asbp++);
  	} while (--sense_length);
    }
    dp ("\n");
}

/*
 * Function to Find Additional Sense Code/Qualifier Message.
 */
char *
cdbg_SenseMessage (sdp)
register struct all_req_sns_data *sdp;
{
      register struct sense_entry *se;
      int entrys = 0;

      for (se = SenseCodeTable; entrys < SenseCodeEntrys; se++, entrys++) {
	if ( (se->sense_code == sdp->asc) &&
	     (se->sense_qualifier == sdp->asq) ) {
	    return (se->sense_message);
	}
      }
      return ((char *) 0);
}

/*
 * System Error Code Table.
 */
struct System_ErrorTable {
	u_char	errno;
	caddr_t	errno_msg;
} system_ErrorTable[] = {
	{ ESUCCESS,	"SUCCESS"	},		/* 0 */
	{ EPERM,	"EPERM"		},		/* 1 */
	{ ENOENT,	"ENOENT"	},		/* 2 */
	{ ESRCH,	"ESRCH"		},		/* 3 */
	{ EINTR,	"EINTR"		},		/* 4 */
	{ EIO,		"EIO"		},		/* 5 */
	{ ENXIO,	"ENXIO"		},		/* 6 */
	{ E2BIG,	"E2BIG"		},		/* 7 */
	{ ENOEXEC,	"ENOEXEC"	},		/* 8 */
	{ EBADF,	"EBADF"		},		/* 9 */
	{ ECHILD,	"ECHILD"	},		/* 10 */
	{ EDEADLK,	"EDEADLK"	},		/* 11 */
	{ ENOMEM,	"ENOMEM"	},		/* 12 */
	{ EACCES,	"EACCES"	},		/* 13 */
	{ EFAULT,	"EFAULT"	},		/* 14 */
	{ ENOTBLK,	"ENOTBLK"	},		/* 15 */
	{ EBUSY,	"EBUSY"		},		/* 16 */
	{ EEXIST,	"EEXIST"	},		/* 17 */
	{ EXDEV,	"EXDEV"		},		/* 18 */
	{ ENODEV,	"ENODEV"	},		/* 19 */
	{ ENOTDIR,	"ENOTDIR"	},		/* 20 */
	{ EISDIR,	"EISDIR"	},		/* 21 */
	{ EINVAL,	"EINVAL"	},		/* 22 */
	{ ENFILE,	"ENFILE"	},		/* 23 */
	{ EMFILE,	"EMFILE"	},		/* 24 */
	{ ENOTTY,	"ENOTTY"	},		/* 25 */
	{ ETXTBSY,	"ETXTBSY"	},		/* 26 */
	{ EFBIG,	"EFBIG"		},		/* 27 */
	{ ENOSPC,	"ENOSPC"	},		/* 28 */
	{ ESPIPE,	"ESPIPE"	},		/* 29 */
	{ EROFS,	"EROFS"		},		/* 30 */
	{ EMLINK,	"EMLINK"	},		/* 31 */
	{ EPIPE,	"EPIPE"		},		/* 32 */
	{ EDOM,		"EDOM"		},		/* 33 */
	{ ERANGE,	"ERANGE"	},		/* 34 */
	{ EWOULDBLOCK,	"EWOULDBLOCK"	},		/* 35 */
	{ EINPROGRESS,	"EINPROGRESS"	},		/* 36 */
	{ EALREADY,	"EALREADY"	},		/* 37 */
	{ ENOTSOCK,	"ENOTSOCK"	},		/* 38 */
	{ EDESTADDRREQ,	"EDESTADDRREQ"	},		/* 39 */
	{ EMSGSIZE,	"EMSGSIZE"	},		/* 40 */
	{ EPROTOTYPE,	"EPROTOTYPE"	},		/* 41 */
	{ ENOPROTOOPT,	"ENOPROTOOPT"	},		/* 42 */
	{ EPROTONOSUPPORT,"EPROTONOSUPPORT"},		/* 43 */
	{ ESOCKTNOSUPPORT,"ESOCKTNOSUPPORT"},		/* 44 */
	{ EOPNOTSUPP,	"EOPNOTSUPP"	},		/* 45 */
	{ EPFNOSUPPORT,	"EPFNOSUPPORT"	},		/* 46 */
	{ EAFNOSUPPORT,	"EAFNOSUPPORT"	},		/* 47 */
	{ EADDRINUSE,	"EADDRINUSE"	},		/* 48 */
	{ EADDRNOTAVAIL,"EADDRNOTAVAIL"	},		/* 49 */
	{ ENETDOWN,	"ENETDOWN"	},		/* 50 */
	{ ENETUNREACH,	"ENETUNREACH"	},		/* 51 */
	{ ENETRESET,	"ENETRESET"	},		/* 52 */
	{ ECONNABORTED,	"ECONNABORTED"	},		/* 53 */
	{ ECONNRESET,	"ECONNRESET"	},		/* 54 */
	{ ENOBUFS,	"ENOBUFS"	},		/* 55 */
	{ EISCONN,	"EISCONN"	},		/* 56 */
	{ ENOTCONN,	"ENOTCONN"	},		/* 57 */
	{ ESHUTDOWN,	"ESHUTDOWN"	},		/* 58 */
	{ ETOOMANYREFS,	"ETOOMANYREFS"	},		/* 59 */
	{ ETIMEDOUT,	"ETIMEDOUT"	},		/* 60 */
 	{ ECONNREFUSED,	"ECONNREFUSED"	},		/* 61 */
	{ ELOOP,	"ELOOP"		},		/* 62 */
	{ ENAMETOOLONG,	"ENAMETOOLONG"	},		/* 63 */
	{ EHOSTDOWN,	"EHOSTDOWN"	},		/* 64 */
	{ EHOSTUNREACH,	"EHOSTUNREACH"	},		/* 65 */
	{ ENOTEMPTY,	"ENOTEMPTY"	},		/* 66 */
	{ EPROCLIM,	"EPROCLIM"	},		/* 67 */
	{ EUSERS,	"EUSERS"	},		/* 68 */
	{ EDQUOT,	"EDQUOT"	},		/* 69 */
	{ ESTALE,	"ESTALE"	},		/* 70 */
	{ EREMOTE,	"EREMOTE"	},		/* 71 */
	{ EBADRPC,	"EBADRPC"	},		/* 72 */
	{ ERPCMISMATCH,	"ERPCMISMATCH"	},		/* 73 */
	{ EPROGUNAVAIL,	"EPROGUNAVAIL"	},		/* 74 */
	{ EPROGMISMATCH,"EPROGMISMATCH"	},		/* 75 */
	{ EPROCUNAVAIL,	"EPROCUNAVAIL"	},		/* 76 */
	{ ENOLCK,	"ENOLCK"	},		/* 77 */
	{ ENOSYS,	"ENOSYS"	},		/* 78 */
	{ 79,		"UNKNOWN"	},		/* 79 */
	{ ENOMSG,	"ENOMSG"	},		/* 80 */
	{ EIDRM,	"EIDRM"		},		/* 81 */
	{ ENOSR,	"ENOSR"		},		/* 82 */
	{ ETIME,	"ETIME"		},		/* 83 */
	{ EBADMSG,	"EBADMSG"	},		/* 84 */
	{ EPROTO,	"EPROTO"	},		/* 85 */
	{ ENODATA,	"ENODATA"	},		/* 86 */
	{ ENOSTR,	"ENOSTR"	},		/* 87 */
	{ ECLONEME,	"ECLONEME"	},		/* 88 */
	{ EDIRTY,	"EDIRTY"	},		/* 89 */
	{ EDUPPKG,	"EDUPPKG"	},		/* 90 */
	{ EVERSION,	"EVERSION"	},		/* 91 */
	{ ENOPKG,	"ENOPKG"	},		/* 92 */
	{ ENOSYM,	"ENOSYM"	},		/* 93 */
	{ ECANCELED,	"ECANCELED"	},		/* 94 */
	{ EFAIL,	"EFAIL"		},		/* 95 */
	{ EFTYPE,	"EFTYPE"	},		/* 96 */
	{ EINPROG,	"EINPROG"	},		/* 97 */
	{ EMTIMERS,	"EMTIMERS"	},		/* 98 */
	{ ENOTSUP,	"ENOTSUP"	},		/* 99 */
};

static int system_ErrorEntrys =
		sizeof(system_ErrorTable) / sizeof(system_ErrorTable[0]);

caddr_t
cdbg_SystemStatus (errno)
register int errno;
{
	if (errno < system_ErrorEntrys) {
		return (system_ErrorTable[errno].errno_msg);
	} else {
		return ("Unknown");
	}
}

#endif /* defined(CAMDEBUG) */
