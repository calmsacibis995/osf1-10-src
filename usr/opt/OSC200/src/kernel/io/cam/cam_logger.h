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
 * @(#)$RCSfile: cam_logger.h,v $ $Revision: 1.1.10.3 $ (DEC) $Date: 1993/08/13 20:46:54 $
 */
#ifndef _CAM_LOGGER_
#define _CAM_LOGGER_

/* ---------------------------------------------------------------------- */

/* cam_logger.h		Version 1.05			Nov. 15, 1991

This file contains all the defines that the CAM sub-system uses to log
errors. 

Modification History

	Version		Date		Who 	Reason
	1.00		06/30/91	dallas	Created this module
	1.01		08/01/91	dallas  New defines- CCB's
						Now follow the ccb type.
	1.02		08/21/91	dallas	New entry defines( for
						rln and janet ).
	1.03		11/13/91	janet	Added defines for CLASS_SIM94,
						CLASS_DEC_SIM, SUBSYS_SIM94,
						SUBSYS_DEC_SIM
	1.04		11/13/91	jag	Added header include check.
	1.05		11/15/91	jag	Added the XPT, CCFG and no
						error value defines.

*/

/* ---------------------------------------------------------------------- */

/* Include Files	*/
/*	None		*/






/* ---------------------------------------------------------------------- */


/* Structure definitions	*/

/*
 * The error entry struct describes a data type (string, sii regs
 * sim working set struct etc). There can be multiple error entries 
 * per packet.
 */
typedef struct cam_err_entry {
    U32		ent_type;	/* String, TAPE_SPECIFIC, CCB, etc */
    U32		ent_size; 	/* Size of the data (CCB, TAPE_SEPC)*/
    U32		ent_total_size; /* To preserve alignment (uerf) */
    U32		ent_vers;	/* Version number of type.	*/
    u_char		*ent_data;	/* Pointer to whatever string etc */
    U32		ent_pri;	/* FULL or Brief uerf output	*/
}CAM_ERR_ENTRY;


/*
 * The cam error header contains all the data need by uerf to determine
 * that this is a cam error log packet.
 */

typedef struct cam_err_hdr {
    u_short		hdr_type;	/* Packet type - CAM_ERR_PKT	*/
    U32		hdr_size;	/* Filled in by cam_logger 	*/
    u_char		hdr_class;	/* Sub system class Tape, disk,
					 * sii_dme , etc..
					 */
    U32		hdr_subsystem;	/* 
					 * Mostly for controller type
					 * But the current errloger uses
					 * disk tape etc if no controller
					 * is known.. So what we will do
					 * is dup the disk and tape types
					 * in the lower number 0 - 1f and
					 * the controllers asc sii 5380
					 * etc can use the uppers.
					 */
    U32		hdr_entries;	/* Number of error entries in list*/
    CAM_ERR_ENTRY	*hdr_list;	/* Pointer to list of error entries*/
    U32		hdr_pri;	/* Error logger priority.	*/
}CAM_ERR_HDR;


/* ---------------------------------------------------------------------- */

/*
 * General purpose define for no Error logger value.
 */

#define NO_ERRVAL	-1


/* Defines for CAM_ERR_HDR	*/

/* 
 * CAM_ERR_HDR.hdr_type defines	
 * There is only 1 type CAM_ERR_PKT. This tells uerf that a CAM
 * error or informational packet is in the log.
 */
#define CAM_ERR_PKT	199	/* Based on what has been defined already */

/*
 * CAM_ERR_HDR.hdr_class defines.
 * This field id's which part of cam detected the error...
 * Tape, disk, dme, sim_sii etc. Please note that 0 - 1F  are
 * reserved for SCSI devices..........
 */

#define CLASS_DISK ALL_DTYPE_DIRECT		/* 0x00			*/
#define CLASS_TAPE ALL_DTYPE_SEQUENTIAL		/* 0x01			*/
#define CLASS_PRINTER ALL_DTYPE_PRINTER		/* 0x02			*/
#define CLASS_PROCESSOR ALL_DTYPE_PROCESSOR	/* 0x03			*/
#define CLASS_WORM ALL_DTYPE_WORM      		/* 0x04			*/
#define CLASS_RODIRECT ALL_DTYPE_RODIRECT 	/* 0x05			*/
#define CLASS_SCANNER ALL_DTYPE_SCANNER 	/* 0x06			*/
#define CLASS_OPTICAL ALL_DTYPE_OPTICAL		/* 0x07			*/
#define CLASS_MEDIA_CHANGER ALL_DTYPE_CHANGER  	/* 0x08			*/
#define CLASS_COMM ALL_DTYPE_COMM    		/* 0x09			*/
#define CLASS_UNKNOWN ALL_DTYPE_NONE   		/* 0x1f			*/
#define CLASS_SIM94			0x20	/* ASC (94) controller	*/
#define CLASS_SII			0x21	/* SII Chip		*/
#define CLASS_DEC_SIM			0x22	/* DEC SIM error	*/
#define CLASS_XPT			0x23	/* CAM Transport layer  */
#define CLASS_CCFG			0x24	/* CAM Config layer     */
#define CLASS_KZQ			0x25    /* KZQ controller       */
#define CLASS_DME			0x26	/* CAM DME layer	*/
/* ADD your classes here	*/
#define CLASS_SIOP			0x30	/* Cobra SIOP controllers */
#define CLASS_XZA			0x31	/* LASER/Ruby XZA/SCSI 	*/
#define CLASS_AHA			0x32	/* AHA 1740A controllers */
#define CLASS_TZA                       0x33    /* TurboChannel TZA/SCSI */


/*
 * CAM_ERR_HDR.hdr_subsystem
 */
#define SUBSYS_DISK ALL_DTYPE_DIRECT		/* 0x00			*/
#define SUBSYS_TAPE ALL_DTYPE_SEQUENTIAL	/* 0x01			*/
#define SUBSYS_PRINTER ALL_DTYPE_PRINTER	/* 0x02			*/
#define SUBSYS_PROCESSOR ALL_DTYPE_PROCESSOR	/* 0x03			*/
#define SUBSYS_WORM ALL_DTYPE_WORM      	/* 0x04			*/
#define SUBSYS_RODIRECT ALL_DTYPE_RODIRECT 	/* 0x05			*/
#define SUBSYS_SCANNER ALL_DTYPE_SCANNER 	/* 0x06			*/
#define SUBSYS_OPTICAL ALL_DTYPE_OPTICAL	/* 0x07			*/
#define SUBSYS_MEDIA_CHANGER ALL_DTYPE_CHANGER  /* 0x08			*/
#define SUBSYS_COMM ALL_DTYPE_COMM    		/* 0x09			*/
#define SUBSYS_UNKNOWN ALL_DTYPE_NONE   	/* 0x1f			*/
#define SUBSYS_SIM94			0x20	/* ASC (94) controller	*/
#define SUBSYS_SII			0x21	/* SII Chip		*/
#define SUBSYS_DEC_SIM			0x22	/* DEC SIM error	*/
#define SUBSYS_XPT			0x23	/* CAM Transport layer  */
#define SUBSYS_CCFG			0x24	/* CAM Config layer     */
#define SUBSYS_KZQ			0x25    /* KZQ controller       */
#define SUBSYS_3MIN_DME			0x26    /* CAM IOASIC/DMA DME	*/
#define SUBSYS_TCDS_DME			0x27    /* CAM TCDS/DMA DME	*/
/* ADD  5380 HERE */
#define SUBSYS_SIOP                     0x30    /* SIOP controllers     */
#define SUBSYS_XZA			0x31	/* LASER/Ruby XZA/SCSI 	*/
#define SUBSYS_AHA                      0x32    /* AHA 1740A controller */
#define SUBSYS_TZA                      0x33    /* TurboChannel TZA/SCSI */


/* 
 * CAM_ERR_HDR.hdr_pri defines....
 * The Priority define is how we allocate and report the error event.
 * CAM_ERR_SEVERE - nasty panics, controller parity errors, no recovery.
 * CAM_ERR_HIGH - Hard errors, but recovery
 * CAM_ERR_LOW - Informational messages.
 */

#define CAM_ERR_SEVERE	EL_PRISEVERE
#define CAM_ERR_HIGH	EL_PRIHIGH
#define CAM_ERR_LOW	EL_PRILOW

/*
 * CAM_ERR_ENTRY defines
 */

/*
 * CAM_ERR_ENTRY.ent_type defines
 */
/* CCB types */
#define ENT_CCB_NOOP		XPT_NOOP	/* 0x00			*/
#define ENT_CCB_SCSIIO		XPT_SCSI_IO	/* 0x01			*/
#define ENT_CCB_GETDEV  	XPT_GDEV_TYPE	/* 0x02			*/
#define ENT_CCB_PATHINQ 	XPT_PATH_INQ	/* 0x03			*/
#define ENT_CCB_RELSIM		XPT_REL_SIMQ	/* 0x04			*/
#define ENT_CCB_SETASYNC	XPT_SASYNC_CB	/* 0x05			*/
#define ENT_CCB_SETDEV		XPT_SDEV_TYPE	/* 0x06			*/
#define ENT_CCB_ABORT		XPT_ABORT	/* 0x10 		*/
#define ENT_CCB_RESETBUS	XPT_RESET_BUS	/* 0x11			*/
#define ENT_CCB_RESETDEV	XPT_RESET_DEV	/* 0x12			*/
#define ENT_CCB_TERMIO		XPT_TERM_IO	/* 0x13			*/
#define ENT_CCB_ENG_INQ		XPT_ENG_INQ	/* 0x20			*/
#define ENT_CCB_ENG_EXEC	XPT_ENG_EXEC	/* 0x21			*/
#define ENT_CCB_EN_LUN		XPT_EN_LUN	/* 0x30			*/
#define ENT_CCB_TARGET_IO	XPT_TARGET_IO	/* 0x31			*/
/* 0x00 - 0xff for defined and future of ccb's */



/* 
 * There can be multiple types of strings.... This will allow the error
 * log formatter to make the output look nice.
 */
#define STR_START		0x00000100	/* Start string range	*/
#define ENT_STRING		0x00000100	/* Generic string	*/
#define ENT_STR_DEV		0x00000101	/* Device name string	*/
#define ENT_STR_MODULE		0x00000102	/* Module name string	*/
/*
 * ERROR type string hard soft software etc.
 */
#define ENT_STR_SOFTWARE_ERROR	0x00000103	/* Software error	*/
#define ENT_STR_HARD_ERROR	0x00000104	/* Hardware error	*/
#define ENT_STR_SOFT_ERROR	0x00000105	/* Soft error (recovered)*/
#define ENT_STR_INFOR_ERROR	0x00000106	/* Informational error	*/
#define ENT_STR_UNKNOWN_ERROR	0x00000107	/* Unknown error type	*/

#define STR_END			0x000001ff	/* END string range	*/
/* 108 - 1ff reserved for further expansion.	*/

#define ENT_SIMSII_REG		0x00000200	/* SII registers	*/
#define ENT_SIM94_REG		0x00000201	/* ASC registers	*/
#define ENT_SIMKZQ_REG		0x00000202      /* struct SIMKZQ_REGS   */
#define ENT_XZA_REG		0x00000203	/* XZA error-related regs */
#define ENT_DME_PHYADDR		0x00000204	/* IOASIC DME phys addr	*/
/* 205 - 2ff reserved for further expansion	*/

#define ENT_SENSE_DATA		0x00000300	/* Sense Data		*/
#define ENT_SCSI_STAT		0x00000301	/* SCSI Status		*/
/* 302 - 3ff reserved for further expansion	*/

/* Structure defines........ */
#define ENT_PDRV_WS		0x00000400	/* struct PDRV_WS 	*/
#define ENT_TAPE_SPECIFIC	0x00000401	/* TAPE_SPECIFIC struct */
#define ENT_DISK_SPECIFIC	0x00000402	/* DISK_SPECIFIC struct */
#define ENT_PDRV_DEVICE		0x00000403	/* PDRV_DEVICE struct	*/
#define ENT_BUF_BP		0x00000404	/* struct buf (buf.h)	*/
#define ENT_SII_INTR		0x00000405	/* struct SII_INTR	*/
#define ENT_SIMSII_SOFTC	0x00000406	/* struct SIMSII_SOFTC	*/
#define ENT_SCATTER_ELEMENT	0x00000407	/* struct SIMSII_SOFTC	*/
#define ENT_SEGMENT_BUFFER	0x00000408	/* struct SEGMENT_BUFFER*/
#define ENT_SEGMENT_ELEMENT	0x00000409	/* struct SEGMENT_ELEMENT*/
#define ENT_DME_DESCRIPTOR	0x0000040a	/* struct DME_DESCRIPTOR */
#define ENT_DME_STRUCT		0x0000040b	/* struct DME_STRUCT	*/
#define ENT_DME_PMAX_STRUCT	0x0000040c	/* struct DME_PMAX_STRUCT*/
#define ENT_IT_NEXUS		0x0000040d	/* struct IT_NEXUS	*/
#define ENT_SIM_WS		0x0000040e	/* struct SIM_WS	*/
#define ENT_NEXUS		0x0000040f	/* struct nexus		*/
#define ENT_SIM_SOFTC		0x00000410	/* struct SIM_SOFTC	*/
#define ENT_SIM_SM_DATA		0x00000411	/* struct SIM_SM_DATA	*/
#define ENT_SIM_SM		0x00000412	/* struct SIM_SM	*/
#define ENT_TAG_ELEMENT		0x00000413	/* struct TAG_ELEMENT	*/
#define ENT_SIM94_INTR		0x00000414	/* struct SIM94_INTR	*/
#define ENT_SIM94_SOFTC		0x00000415	/* struct SIM94_SOFTC	*/
#define ENT_CIR_Q		0x00000416	/* struct CIR_Q 	*/
#define ENT_HBA_DME_CONTROL	0x00000417	/* struct HBA_DME_CONTROL*/
#define ENT_XPT_WS		0x00000418	/* struct XPT_WS	*/
#define ENT_XPT_CTRL		0x00000419	/* struct XPT_CTRL	*/
#define ENT_DEC_CAM_PKT		0x0000041a	/* struct DEC_CAM_PKT	*/
#define ENT_CCFG_CTRL		0x0000041b	/* struct CCFG_CTRL	*/
#define ENT_CCFG_QHEAD		0x0000041c	/* struct CCFG_QHEAD	*/
#define ENT_EDT			0x0000041d	/* struct EDT		*/
#define ENT_XPT_QHEAD		0x0000041e	/* struct XPT_QHEAD	*/
#define ENT_CCFG_CONFTBL	0x0000041f	/* struct CAM CONF_TBL  */
#define ENT_KZQ_INTR		0x00000420      /* struct KZQ_INTR      */
#define ENT_SIMKZQ_SOFTC	0x00000421      /* struct SIMKZQ_SOFTC  */
#define ENT_DMA_WSET		0x00000422	/* struct DME DMA_WSET	*/
#define ENT_DME_DAT		0x00000423	/* struct DME DAT_ELEM	*/
#define ENT_DME_ATTCH		0x00000424	/* struct DME DAT_ATTCH	*/
/* Add your own here....... */
#define ENT_SIOP_SCRIPTHOST     0x00000430      /* SIOP scripthost      */
#define ENT_SIOP_SIOPJOB        0x00000431      /* SIOP siopjob         */
#define ENT_XZA_SOFTC		0x00000432	/* XZA softc 		*/
#define ENT_XZA_CHAN		0x00000433	/* XZA chan		*/
#define ENT_AHA_SOFTC           0x00000434      /* AHA softc            */
#define ENT_AHA_JOB             0x00000435      /* AHA JOB              */
#define ENT_SIMPORT_SOFTC       0x00000436      /* SIMPORT softc        */


#define ENT_STRUCT_UNKNOWN	0xffffffff	/* Unknown structure 	*/



/*
 * CAM_ERR_ENTRY.ent_pri
 * Defines whether you always want to see when you run uerf or
 * only when you do a full error report
 */

#define PRI_BRIEF_REPORT	0x00000001
#define PRI_FULL_REPORT		0x00000002


/*
 * The peripherial drivers error types...
 */
#define CAM_INFORMATIONAL	0x00000001	/* Informational message */
#define CAM_SOFTERR		0x00000002	/* Soft error		 */
#define CAM_HARDERR		0x00000004	/* Hard error		 */
#define CAM_SOFTWARE		0x00000008	/* Software detected 	 */
#define CAM_DUMP_ALL		0x00000010	/* Dump all structures	 */

#endif /* _CAM_LOGGER_ */
