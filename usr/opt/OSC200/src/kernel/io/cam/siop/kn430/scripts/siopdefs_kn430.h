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
/* DEFINITIONS OF ADDRESSES OF SIOP REGISTERS IN LBUS SPACE AS SEEN FROM
 * THE SIOP'S.  THIS IS USED TO MOVE DATA FROM CHIP REGISTERS TO MEMORY
 * WITH THE MEMORY MOVE INSTRUCTION.
 */

#ifndef _SIOPDEFS_KN430_H_ 
#define _SIOPDEFS_KN430_H_ 


/* The base address of the NCR53C710 registers as seen from the SIOP when
 * running SCRIPTS.
 */
#define SIOP_REG_BASE		0xc0000000

#define	SSIOP_REG_SCNTL0   	(SIOP_REG_BASE+SIOP_REG_SCNTL0)
#define	SSIOP_REG_SCNTL1   	(SIOP_REG_BASE+SIOP_REG_SCNTL1)
#define	SSIOP_REG_SDID   	(SIOP_REG_BASE+SIOP_REG_SDID)
#define	SSIOP_REG_SIEN   	(SIOP_REG_BASE+SIOP_REG_SIEN)
#define	SSIOP_REG_SCID   	(SIOP_REG_BASE+SIOP_REG_SCID)
#define	SSIOP_REG_SXFER   	(SIOP_REG_BASE+SIOP_REG_SXFER)
#define	SSIOP_REG_SODL   	(SIOP_REG_BASE+SIOP_REG_SODL)
#define	SSIOP_REG_SOCL   	(SIOP_REG_BASE+SIOP_REG_SOCL)
#define	SSIOP_REG_SFBR   	(SIOP_REG_BASE+SIOP_REG_SFBR)
#define	SSIOP_REG_SIDL   	(SIOP_REG_BASE+SIOP_REG_SIDL)
#define	SSIOP_REG_SBDL   	(SIOP_REG_BASE+SIOP_REG_SBDL)
#define	SSIOP_REG_SBCL   	(SIOP_REG_BASE+SIOP_REG_SBCL)
#define	SSIOP_REG_DSTAT   	(SIOP_REG_BASE+SIOP_REG_DSTAT)
#define	SSIOP_REG_SSTAT0   	(SIOP_REG_BASE+SIOP_REG_SSTAT0)
#define	SSIOP_REG_SSTAT1   	(SIOP_REG_BASE+SIOP_REG_SSTAT1)
#define	SSIOP_REG_SSTAT2   	(SIOP_REG_BASE+SIOP_REG_SSTAT2)
#define	SSIOP_REG_DSA0   	(SIOP_REG_BASE+SIOP_REG_DSA0)
#define	SSIOP_REG_DSA1   	(SIOP_REG_BASE+SIOP_REG_DSA1)
#define	SSIOP_REG_DSA2   	(SIOP_REG_BASE+SIOP_REG_DSA2)
#define	SSIOP_REG_DSA3   	(SIOP_REG_BASE+SIOP_REG_DSA3)
#define	SSIOP_REG_CTEST0   	(SIOP_REG_BASE+SIOP_REG_CTEST0)
#define	SSIOP_REG_CTEST1   	(SIOP_REG_BASE+SIOP_REG_CTEST1)
#define	SSIOP_REG_CTEST2   	(SIOP_REG_BASE+SIOP_REG_CTEST2)
#define	SSIOP_REG_CTEST3   	(SIOP_REG_BASE+SIOP_REG_CTEST3)
#define	SSIOP_REG_CTEST4   	(SIOP_REG_BASE+SIOP_REG_CTEST4)
#define	SSIOP_REG_CTEST5   	(SIOP_REG_BASE+SIOP_REG_CTEST5)
#define	SSIOP_REG_CTEST6   	(SIOP_REG_BASE+SIOP_REG_CTEST6)
#define	SSIOP_REG_CTEST7   	(SIOP_REG_BASE+SIOP_REG_CTEST7)
#define	SSIOP_REG_TEMP0   	(SIOP_REG_BASE+SIOP_REG_TEMP0)
#define	SSIOP_REG_TEMP1   	(SIOP_REG_BASE+SIOP_REG_TEMP1)
#define	SSIOP_REG_TEMP2   	(SIOP_REG_BASE+SIOP_REG_TEMP2)
#define	SSIOP_REG_TEMP3   	(SIOP_REG_BASE+SIOP_REG_TEMP3)
#define	SSIOP_REG_DFIFO   	(SIOP_REG_BASE+SIOP_REG_DFIFO)
#define	SSIOP_REG_ISTAT   	(SIOP_REG_BASE+SIOP_REG_ISTAT)
#define	SSIOP_REG_CTEST8   	(SIOP_REG_BASE+SIOP_REG_CTEST8)
#define	SSIOP_REG_LCRC   	(SIOP_REG_BASE+SIOP_REG_LCRC)
#define	SSIOP_REG_DBC0   	(SIOP_REG_BASE+SIOP_REG_DBC0)
#define	SSIOP_REG_DBC1   	(SIOP_REG_BASE+SIOP_REG_DBC1)
#define	SSIOP_REG_DBC2   	(SIOP_REG_BASE+SIOP_REG_DBC2)
#define	SSIOP_REG_DCMD   	(SIOP_REG_BASE+SIOP_REG_DCMD)
#define	SSIOP_REG_DNAD0   	(SIOP_REG_BASE+SIOP_REG_DNAD0)
#define	SSIOP_REG_DNAD1   	(SIOP_REG_BASE+SIOP_REG_DNAD1)
#define	SSIOP_REG_DNAD2   	(SIOP_REG_BASE+SIOP_REG_DNAD2)
#define	SSIOP_REG_DNAD3   	(SIOP_REG_BASE+SIOP_REG_DNAD3)
#define	SSIOP_REG_DSP0   	(SIOP_REG_BASE+SIOP_REG_DSP0)
#define	SSIOP_REG_DSP1   	(SIOP_REG_BASE+SIOP_REG_DSP1)
#define	SSIOP_REG_DSP2   	(SIOP_REG_BASE+SIOP_REG_DSP2)
#define	SSIOP_REG_DSP3   	(SIOP_REG_BASE+SIOP_REG_DSP3)
#define	SSIOP_REG_DSPS0   	(SIOP_REG_BASE+SIOP_REG_DSPS0)
#define	SSIOP_REG_DSPS1   	(SIOP_REG_BASE+SIOP_REG_DSPS1)
#define	SSIOP_REG_DSPS2   	(SIOP_REG_BASE+SIOP_REG_DSPS2)
#define	SSIOP_REG_DSPS3   	(SIOP_REG_BASE+SIOP_REG_DSPS3)
#define	SSIOP_REG_SCRATCH0   	(SIOP_REG_BASE+SIOP_REG_SCRATCH0)
#define	SSIOP_REG_SCRATCH1   	(SIOP_REG_BASE+SIOP_REG_SCRATCH1)
#define	SSIOP_REG_SCRATCH2   	(SIOP_REG_BASE+SIOP_REG_SCRATCH2)
#define	SSIOP_REG_SCRATCH3   	(SIOP_REG_BASE+SIOP_REG_SCRATCH3)
#define	SSIOP_REG_DMODE   	(SIOP_REG_BASE+SIOP_REG_DMODE)
#define	SSIOP_REG_DIEN   	(SIOP_REG_BASE+SIOP_REG_DIEN)
#define	SSIOP_REG_DWT   	(SIOP_REG_BASE+SIOP_REG_DWT)
#define	SSIOP_REG_DCNTL   	(SIOP_REG_BASE+SIOP_REG_DCNTL)
#define	SSIOP_REG_ADDER0   	(SIOP_REG_BASE+SIOP_REG_ADDER0)
#define	SSIOP_REG_ADDER1   	(SIOP_REG_BASE+SIOP_REG_ADDER1)
#define	SSIOP_REG_ADDER2   	(SIOP_REG_BASE+SIOP_REG_ADDER2)
#define	SSIOP_REG_ADDER3   	(SIOP_REG_BASE+SIOP_REG_ADDER3)

/* CAM error codes.  Defined here because the CAM version can't be included
 * when compiling SCRIPTS.
 */
#define CAM_REQ_INPROG          0x00    /* CCB request is in progress */
#define CAM_REQ_CMP             0x01    /* CCB request completed w/out error */
#define CAM_REQ_ABORTED         0x02    /* CCB request aborted by the host */
#define CAM_UA_ABORT            0x03    /* Unable to Abort CCB request */
#define CAM_REQ_CMP_ERR         0x04    /* CCB request completed with an err */
#define CAM_BUSY                0x05    /* CAM subsystem is busy */
#define CAM_REQ_INVALID         0x06    /* CCB request is invalid */
#define CAM_PATH_INVALID        0x07    /* Path ID supplied is invalid */
#define CAM_DEV_NOT_THERE       0x08    /* SCSI device not installed/there */
#define CAM_UA_TERMIO           0x09    /* Unable to Terminate I/O CCB req */
#define CAM_SEL_TIMEOUT         0x0A    /* Target selection timeout */
#define CAM_CMD_TIMEOUT         0x0B    /* Command timeout */
#define CAM_MSG_REJECT_REC      0x0D    /* Message reject received */
#define CAM_SCSI_BUS_RESET      0x0E    /* SCSI bus reset sent/received */
#define CAM_UNCOR_PARITY        0x0F    /* Uncorrectable parity err occurred */
#define CAM_AUTOSENSE_FAIL      0x10    /* Autosense: Request sense cmd fail */
#define CAM_NO_HBA              0x11    /* No HBA detected Error */
#define CAM_DATA_RUN_ERR        0x12    /* Data overrun/underrun error */
#define CAM_UNEXP_BUSFREE       0x13    /* Unexpected BUS free */
#define CAM_SEQUENCE_FAIL       0x14    /* Target bus phase sequence failure */
#define CAM_CCB_LEN_ERR         0x15    /* CCB length supplied is inadequate */
#define CAM_PROVIDE_FAIL        0x16    /* Unable to provide requ. capability */
#define CAM_BDR_SENT            0x17    /* A SCSI BDR msg was sent to target */
#define CAM_REQ_TERMIO          0x18    /* CCB request terminated by the host */

#define CAM_LUN_INVALID         0x38    /* LUN supplied is invalid */
#define CAM_TID_INVALID         0x39    /* Target ID supplied is invalid */
#define CAM_FUNC_NOTAVAIL       0x3A    /* The requ. func is not available */
#define CAM_NO_NEXUS            0x3B    /* Nexus is not established */
#define CAM_IID_INVALID         0x3C    /* The initiator ID is invalid */
#define CAM_CDB_RECVD           0x3E    /* The SCSI CDB has been received */
#define CAM_SCSI_BUSY           0x3F    /* SCSI bus busy */

/* sizes of various data types */
#define SIZEOF_U8	1
#define SIZEOF_U16	2
#define SIZEOF_U32	4
#define SIZEOF_U64	8

#endif 
