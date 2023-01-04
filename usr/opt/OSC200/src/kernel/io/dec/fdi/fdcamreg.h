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
/*
static char *rcsid = "@(#)$RCSfile: fdcamreg.h,v $ $Revision: 1.1.9.2 $ (DEC) $Date: 1993/06/24 22:40:02 $";
*/
#endif

/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/************************************************************************
 *
 * Modification history:
 *
 * 12-Aug-91	Roger S. Morris
 *	New file for FDI (Floppy Disk Interconnect) driver.
 * 09-Oct-91	Roger S. Morris
 *	Added fix for moter enable.
 *
 ************************************************************************/

#ifndef FDCAMREG_INCLUDE
#define FDCAMREG_INCLUDE   1

#define FDCAMREG_H_VER "1.5"

/* fdcamreg.h */

typedef unsigned long UNS32;
typedef unsigned char UNS8;
typedef unsigned short UNS16;
typedef short S16;

/* ************************************************************************* */
/*                                                       struct fdcam_82077a */
/* ************************************************************************* */

struct fdcam_82077aa
    {
    /* private: */
    unsigned char sra;                 /* Status Register A                  */
    unsigned char srb;                 /* Status Register B                  */
    unsigned char dor;                 /* Digital Output Register            */
    unsigned char tdr;                 /* Tape Drive Register                */
    unsigned char msr;                 /* Main status Register               */
    unsigned char dsr;                 /* Data Rate Select Register          */
    unsigned char dat;                 /* Data (FIFO)                        */
    unsigned char res;                 /* Reserved                           */
    unsigned char dir;                 /* Digital Input Register             */
    unsigned char ccr;                 /* Configuration Control Register     */
    };

#define SRA (0x000)             /* Offset to Status Register A              */
#define SRB (0x001)             /* Offset to Status Register B              */
#define DOR (0x002)             /* Offset to Digital Output Register        */
#define TDR (0x003)             /* Offset to Tape Drive Register            */
#define MSR (0x004)             /* Offset to Main status Register           */
#define DSR (0x004)             /* Offset to Data Rate Select Register      */
#define DAT (0x005)             /* Offset to Data (FIFO)                    */
#define RES (0x006)             /* Offset to Reserved                       */
#define DIR (0x007)             /* Offset to Digital Input Register         */
#define CCR (0x007)             /* Offset to Configuration Control Register */

/* ************************************************************************* */
/*                                                       Chip-register bits  */
/* ************************************************************************* */

                                       /* Status Register A                  */
#define SRA_INT_PENDING     0x80
#define SRA_DRQ             0x40
#define SRA_STEP_FF         0x20
#define SRA_TRK0            0x10
#define SRA_HDSEL_i         0x08
#define SRA_INDEX           0x04
#define SRA_WP              0x02
#define SRA_DIR_i           0x01
                                       /* Status Register B                  */
#define SRB_DRV2_i          0x80
#define SRB_DS1_i           0x40
#define SRB_DS0_i           0x20
#define SRB_WRDATA_FF       0x10
#define SRB_RDDATA_FF       0x08
#define SRB_WE_FF           0x04
#define SRB_DS3_i           0x02
#define SRB_DS2_i           0x01
                                       /* Digital Output Register            */

    /* fcp->r[DOR] = fcp->rc.dor = fcp->rc.dor & ~0x40  */
    /*    | DOR_DMA_GATE_i | 0x20;                      */

#define DOR_MOT_EN3         0x80
#define DOR_MOT_EN2         0x40
#define DOR_MOT_EN1         0x20
#define DOR_MOT_EN0         0x10
#define DOR_DMA_GATE_i      0x08
#define DOR_RESET_i         0x04
#define DOR_DRIVE_SEL1      0x02
#define DOR_DRIVE_SEL0      0x01
#define DOR_NORMAL          (DOR_RESET_i|DOR_DMA_GATE_i)

#define DOR_DRIVE_SEL_MASK  (DOR_DRIVE_SEL0|DOR_DRIVE_SEL1)
#define LUN_TO_DOR_DRIVE_SEL(L) (L&DOR_DRIVE_SEL_MASK)

#define DOR_SPECIAL_DENSEL0 DOR_MOT_EN3
#define DOR_SPECIAL_DENSEL1 DOR_MOT_EN2
#define DOR_SPECIAL (DOR_SPECIAL_DENSEL0|DOR_SPECIAL_DENSEL1)
                               /* The following are all bits actually used */
                               /*  for motor enable.                       */
#define DOR_MOT_EN          (DOR_MOT_EN0|DOR_MOT_EN1)

                                       /* Tape Drive Register                */
#define TDR_TAPE_SEL1       0x02
#define TDR_TAPE_SEL0       0x01
                                       /* Main status Register               */
#define MSR_RQM             0x80
#define MSR_DIO             0x40
#define MSR_NON_DMA         0x20
#define MSR_CMD_BSY         0x10
#define MSR_DRV3_BUSY       0x08
#define MSR_DRV2_BUSY       0x04
#define MSR_DRV1_BUSY       0x02
#define MSR_DRV0_BUSY       0x01


#if 0 /* the old way... */


/* The following bits indicate the state of the chip.                        */
#define MSR_STATE               (MSR_RQM|MSR_DIO|MSR_NON_DMA|MSR_CMD_BSY)

/* ************************************************************************* */
/* The following macros may be applied to values read from the MSR register. */

/* The following non-zero if chip is in EXECUTION PHASE.                     */
#define MSR_EXEC_PHASE(X)       ((X)&MSR_NON_DMA)

/* The following non-zero if chip is in EXECUTION PHASE and is ready to have */
/*  a data byte READ from it.                                                */
#define MSR_EXEC_PHASE_R_RDY(X) (((X)&MSR_STATE)== \
				 (MSR_RQM|MSR_DIO|MSR_NON_DMA|MSR_CMD_BSY))

/* The following non-zero if chip is in EXECUTION PHASE and is ready to have */
/*  a data byte WRITTEN to it.                                               */
#define MSR_EXEC_PHASE_W_RDY(X) (((X)&MSR_STATE)== \
				 (MSR_RQM        |MSR_NON_DMA|MSR_CMD_BSY))


/* The following non-zero if chip is in COMMAND PHASE and is ready to have   */
/*  a (either first byte or next byte) command byte written to it.           */
#define MSR_CMD_PHASE_W_RDY(X)  (((X)&(MSR_RQM|MSR_DIO|MSR_NON_DMA))==MSR_RQM)

/* The following non-zero if chip is in COMMAND PHASE and is ready to have   */
/*  the first command byte written to it.                                    */
#define MSR_CMD_PHASE_W1_RDY(X)  (((X)&MSR_STATE)==MSR_RQM)

/* The following non-zero if chip is possibly in COMMAND PHASE but is NOT    */
/*  ready to have a command byte written to it.                              */
#define MSR_CMD_PHASE_W_NRDY(X) (((X)&(MSR_RQM|MSR_NON_DMA))==0)

/* The following non-zero if chip might be in COMMAND PHASE.                 */
/* WARNING: PARAMETER POSSIBLY EXECUTED TWICE.                               */
#define MSR_CMD_PHASE(X)        (MSR_CMD_PHASE_W_RDY(X) || \
				 MSR_CMD_PHASE_W_NRDY(X) )

/* The following non-zero if chip is in RESULT PHASE and is ready to have    */
/*  a result byte written to it.                                             */
#define MSR_RSLT_PHASE_R_RDY(X) (((X)&MSR_STATE)== \
				 (MSR_RQM|MSR_DIO|MSR_CMD_BSY))

/* The following non-zero if chip might be in RESULT PHASE but is NOT ready  */
/* to have a result byte written to it.                                      */
#define MSR_RSLT_PHASE_R_NRDY(X) (((X)&(MSR_RQM|MSR_CMD_BSY|MSR_NON_DMA))==\
				  (MSR_CMD_BSY))

/* The following non-zero if chip might be in RESULT PHASE.                  */
/* WARNING: PARAMETER POSSIBLY EXECUTED TWICE.                               */
#define MSR_RSLT_PHASE(X)        (MSR_RSLT_PHASE_R_RDY(X) ||\
				  MSR_RSLT_PHASE_R_NRDY(X) )


#else



/* The following bits indicate the state of the chip.                        */
#define MSR_STATE               (MSR_RQM|MSR_DIO|MSR_NON_DMA|MSR_CMD_BSY)

/* ************************************************************************* */
/* The following macros may be applied to values read from the MSR register. */

/* The following non-zero if chip is possibly in EXECUTION PHASE.            */
#define MSR_EXEC_PHASE(X)       (((X)&MSR_RQM)==0||(X)&MSR_NON_DMA)

/* The following non-zero if chip is in EXECUTION PHASE and is ready to have */
/*  a data byte READ from it.                                                */
#define MSR_EXEC_PHASE_R_RDY(X) (((X)&MSR_STATE)== \
				 (MSR_RQM|MSR_DIO|MSR_NON_DMA|MSR_CMD_BSY))

/* The following non-zero if chip is in EXECUTION PHASE and is ready to have */
/*  a data byte WRITTEN to it.                                               */
#define MSR_EXEC_PHASE_W_RDY(X) (((X)&MSR_STATE)== \
				 (MSR_RQM        |MSR_NON_DMA|MSR_CMD_BSY))

/* The following non-zero if chip is in EXECUTION PHASE and is ready to have */
/*  a data byte READ from or WRITTEN to it.                                  */
#define MSR_EXEC_PHASE_RDY(X) (((X)&(MSR_RQM|MSR_NON_DMA|MSR_CMD_BSY))== \
				 (MSR_RQM|MSR_NON_DMA|MSR_CMD_BSY))


/* The following non-zero if chip is in COMMAND PHASE and is ready to have   */
/*  a (either first byte or next byte) command byte written to it.           */
#define MSR_CMD_PHASE_W_RDY(X)  (((X)&(MSR_RQM|MSR_DIO|MSR_NON_DMA))==MSR_RQM)

/* The following non-zero if chip is in COMMAND PHASE and is ready to have   */
/*  the first command byte written to it.                                    */
#define MSR_CMD_PHASE_W1_RDY(X)  (((X)&MSR_STATE)==MSR_RQM)

/* The following non-zero if chip is possibly in COMMAND PHASE but is NOT    */
/*  ready to have a command byte written to it.                              */
#define MSR_CMD_PHASE_W_NRDY(X) (((X)&MSR_RQM)==0)

/* The following non-zero if chip might be in COMMAND PHASE.                 */
/* WARNING: PARAMETER POSSIBLY EXECUTED TWICE.                               */
#define MSR_CMD_PHASE(X)        (MSR_CMD_PHASE_W_RDY(X) || \
				 MSR_CMD_PHASE_W_NRDY(X) )

/* The following non-zero if chip is in RESULT PHASE and is ready to have    */
/*  a result byte written to it.                                             */
#define MSR_RSLT_PHASE_R_RDY(X) (((X)&MSR_STATE)== \
				 (MSR_RQM|MSR_DIO|MSR_CMD_BSY))

/* The following non-zero if chip might be in RESULT PHASE but is NOT ready  */
/* to have a result byte written to it.                                      */
#define MSR_RSLT_PHASE_R_NRDY(X) (((X)&MSR_RQM)==0)

/* The following non-zero if chip might be in RESULT PHASE.                  */
/* WARNING: PARAMETER POSSIBLY EXECUTED TWICE.                               */
#define MSR_RSLT_PHASE(X)        (MSR_RSLT_PHASE_R_RDY(X) ||\
				  MSR_RSLT_PHASE_R_NRDY(X) )


#endif



                                       /* Data Rate Select Register          */
#define DSR_SW_RESET        0x80
#define DSR_POWER_DOWN      0x40
#define DSR_unused          0x20
#define DSR_PRECOMP_2       0x10
#define DSR_PRECOMP_1       0x08
#define DSR_PRECOMP_0       0x04
#define DSR_DRATE_SEL1      0x02
#define DSR_DRATE_SEL0      0x01

#define DSR_NORMAL          0x00

#define DSR_VAL(RESET,PD,PRECOMP,DRATE) ( ((RESET)?DSR_SW_RESET:0) \
	| ((PD)?DSR_POWER_DOWN:0) | ((PRECOMP)<<2&0x1C) \
	| ((DRATE)&0x03) )

                                       /* Data (FIFO)                        */
                                       /* Reserved                           */
                                       /* Digital Input Register             */
#define DIR_DSK_CHG         0x80
#define DIR_DMA_GATE_i      0x08
#define DIR_NOPREC          0x04
#define DIR_DRATE_SEL1      0x02
#define DIR_DRATE_SEL0      0x01
                                       /* Configuration Control Register     */
#define CCR_NOPREC          0x04
#define CCR_DRATE_SEL1      0x02
#define CCR_DRATE_SEL0      0x01


/* ************************************************************************* */
/*                                     Various status register bits.         */
/* ************************************************************************* */

#define ST0_IC_MASK         0xC0
#define ST0_IC_NORMAL       0x00
#define ST0_IC_ABNORMAL     0x40
#define ST0_IC_INVALID_CMD  0x80
#define ST0_IC_PABNORMAL    0xC0

#define ST0_SE              0x20
#define ST0_EC              0x10
#define ST0_H               0x04
#define ST0_DS              0x03

#define ST1_EN              0x80
#define ST1_DE              0x20
#define ST1_OR              0x10
#define ST1_ND              0x04
#define ST1_NW              0x02
#define ST1_MA              0x01

#define ST2_CM              0x40
#define ST2_DD              0x20
#define ST2_WC              0x10
#define ST2_BC              0x02
#define ST2_MD              0x01

#define ST3_WP              0x40
#define ST3_T0              0x10
#define ST3_HD              0x04
#define ST3_DS1             0x02
#define ST3_DS0             0x01

/* ************************************************************************* */
/*                                      Various command and result bytes.    */
/* ************************************************************************* */

#define CBN_RW_DATA (9)                /* Numbers common to READ, WRITE, and */
                                       /*  VERIFY.                           */
#define RBN_RW_DATA (7)                /*  "                                 */

#define CBN_READ_DATA CBN_RW_DATA
#define CB0_READ_DATA(MT,MFM,SK) ((MT?0x80:0)|(MFM?0x40:0)|(SK?0x20:0)|0x06)
#define CB1_READ_DATA(HDS,DS)    ((HDS?0x04:0)|(DS&0x03)|0x00)
#define CB2_READ_DATA(C)   (C&0xFF)
#define CB3_READ_DATA(H)   (H&0xFF)
#define CB4_READ_DATA(R)   (R&0xFF)
#define CB5_READ_DATA(N)   (N&0xFF)
#define CB6_READ_DATA(EOT) (EOT&0xFF)
#define CB7_READ_DATA(GPL) (GPL&0xFF)
#define CB8_READ_DATA(DTL) (DTL&0xFF)

#define RBN_READ_DATA RBN_RW_DATA

#define CBN_READ_DELETED_DATA (9)
#define CB0_READ_DELETED_DATA(MT,MFM,SK) \
	((MT?0x80:0)|(MFM?0x40:0)|(SK?0x20:0)|0x0C)
#define CB1_READ_DELETED_DATA(HDS,DS)    ((HDS?0x04:0)|(DS&0x03)|0x00)
#define CB2_READ_DELETED_DATA(C)   (C&0xFF)
#define CB3_READ_DELETED_DATA(H)   (H&0xFF)
#define CB4_READ_DELETED_DATA(R)   (R&0xFF)
#define CB5_READ_DELETED_DATA(N)   (N&0xFF)
#define CB6_READ_DELETED_DATA(EOT) (EOT&0xFF)
#define CB7_READ_DELETED_DATA(GPL) (GPL&0xFF)
#define CB8_READ_DELETED_DATA(DTL) (DTL&0xFF)

#define RBN_READ_DELETED_DATA (7)

#define CBN_WRITE_DATA CBN_RW_DATA
#define CB0_WRITE_DATA(MT,MFM) ((MT?0x80:0)|(MFM?0x40:0)|0x05)
#define CB1_WRITE_DATA(HDS,DS)    ((HDS?0x04:0)|(DS&0x03)|0x00)
#define CB2_WRITE_DATA(C)   (C&0xFF)
#define CB3_WRITE_DATA(H)   (H&0xFF)
#define CB4_WRITE_DATA(R)   (R&0xFF)
#define CB5_WRITE_DATA(N)   (N&0xFF)
#define CB6_WRITE_DATA(EOT) (EOT&0xFF)
#define CB7_WRITE_DATA(GPL) (GPL&0xFF)
#define CB8_WRITE_DATA(DTL) (DTL&0xFF)

#define RBN_WRITE_DATA RBN_RW_DATA

#define CBN_WRITE_DELETED_DATA (9)
#define CB0_WRITE_DELETED_DATA(MT,MFM) ((MT?0x80:0)|(MFM?0x40:0)|0x09)
#define CB1_WRITE_DELETED_DATA(HDS,DS)    ((HDS?0x04:0)|(DS&0x03)|0x00)
#define CB2_WRITE_DELETED_DATA(C)   (C&0xFF)
#define CB3_WRITE_DELETED_DATA(H)   (H&0xFF)
#define CB4_WRITE_DELETED_DATA(R)   (R&0xFF)
#define CB5_WRITE_DELETED_DATA(N)   (N&0xFF)
#define CB6_WRITE_DELETED_DATA(EOT) (EOT&0xFF)
#define CB7_WRITE_DELETED_DATA(GPL) (GPL&0xFF)
#define CB8_WRITE_DELETED_DATA(DTL) (DTL&0xFF)

#define RBN_WRITE_DELETED_DATA (0)

#define CBN_READ_TRACK (9)
#define CB0_READ_TRACK(MFM)      (((MFM)?0x40:0)|0x02)
#define CB1_READ_TRACK(HDS,DS)    (((HDS)?0x04:0)|((DS)&0x03)|0x00)
#define CB2_READ_TRACK(C)   ((C)&0xFF)
#define CB3_READ_TRACK(H)   ((H)&0xFF)
#define CB4_READ_TRACK(R)   ((R)&0xFF)
#define CB5_READ_TRACK(N)   ((N)&0xFF)
#define CB6_READ_TRACK(EOT) ((EOT)&0xFF)
#define CB7_READ_TRACK(GPL) ((GPL)&0xFF)
#define CB8_READ_TRACK(DTL) ((DTL)&0xFF)

#define RBN_READ_TRACK (7)

#define CBN_VERIFY CBN_RW_DATA
#define CB0_VERIFY(MT,MFM,SK) (((MT)?0x80:0)|((MFM)?0x40:0)|((SK)?0x20:0)|0x16)
#define CB1_VERIFY(EC,HDS,DS) (((EC)?0x80:0)|((HDS)?0x04:0)|((DS)&0x03)|0x00)
#define CB2_VERIFY(C)   ((C)&0xFF)
#define CB3_VERIFY(H)   ((H)&0xFF)
#define CB4_VERIFY(R)   ((R)&0xFF)
#define CB5_VERIFY(N)   ((N)&0xFF)
#define CB6_VERIFY(EOT) ((EOT)&0xFF)
#define CB7_VERIFY(GPL) ((GPL)&0xFF)
#define CB8_VERIFY(DTL_SC) ((DTL_SC)&0xFF)

#define RBN_VERIFY RBN_RW_DATA

#define CBN_VERSION (1)
#define CB0_VERSION (0x10)

#define RBN_VERSION (1)

#define CBN_FORMAT_TRACK (6)
#define CB0_FORMAT_TRACK(MFM)      (((MFM)?0x40:0)|0x0D)
#define CB1_FORMAT_TRACK(HDS,DS)    (((HDS)?0x04:0)|((DS)&0x03)|0x00)
#define CB2_FORMAT_TRACK(N)   ((N)&0xFF)
#define CB3_FORMAT_TRACK(SC)  ((SC)&0xFF)
#define CB4_FORMAT_TRACK(GPL) ((GPL)&0xFF)
#define CB5_FORMAT_TRACK(D)   ((D)&0xFF)

#define RBN_FORMAT_TRACK (7)

#define CBN_RECALIBRATE (2)
#define CB0_RECALIBRATE      (0x07)
#define CB1_RECALIBRATE(DS)  (((DS)&0x03)|0x00)

#define RBN_RECALIBRATE (0)

#define CBN_SENSE_INTR_STAT (1)
#define CB0_SENSE_INTR_STAT (0x08)

#define RBN_SENSE_INTR_STAT (2)
#define RBI_SIS_ST0 (0)                /* Return-byte-index, sense-intr-stat */
                                       /*  command, ST0 byte.                */
#define RBI_SIS_PCN (1)                /* Return-byte-index, sense-intr-stat */
                                       /*  command, PCN byte.                */

#define CBN_SPECIFY      (3)
#define CB0_SPECIFY      (0x03)
#define CB1_SPECIFY(SRT,HUT)  ((((SRT)<<4)&0xF0)|((HUT)&0x0F))
#define CB2_SPECIFY(HLT,ND)   ((((HLT)<<1)&0xFE)|((ND)?0x01:0)) 
#define GET_ND_FROM_CB2_SPECIFY(VAL) ((VAL)&0x01)

#define RBN_SPECIFY (0)

#define CBN_SENSE_DRV_STAT  (2)
#define CB0_SENSE_DRV_STAT  (0x04)
#define CB1_SENSE_DRV_STAT(HDS,DS)    (((HDS)?0x04:0)|((DS)&0x03)|0x00)

#define RBN_SENSE_DRV_STAT (1)

#define CBN_SEEK  (3)
#define CB0_SEEK  (0x0F)
#define CB1_SEEK(HDS,DS)    (((HDS)?0x04:0)|((DS)&0x03)|0x00)
#define CB2_SEEK(NCN)       ((NCN)&0xFF)

#define RBN_SEEK (0)

#define CBN_CONFIGURE  (4)
#define CB0_CONFIGURE  (0x13)
#define CB1_CONFIGURE  (0x00)
#define CB2_CONFIGURE(EIS,EFIFO,POLL,FIFOTHR) \
	(((EIS)?0x40:0)|((EFIFO)?0x20:0)|((POLL)?0x10:0)|((FIFOTHR)&0x0F)|0x00)
#define CB3_CONFIGURE(PRETRK)    ((PRETRK)&0xFF)

#define RBN_CONFIGURE (0)

#define CBN_RELATIVE_SEEK  (3)
/* 1 == greater track number, 0 == lesser track number. */
#define CB0_RELATIVE_SEEK(DIR)     (((DIR)?0x40:0)|0x8F)
#define CB1_RELATIVE_SEEK(HDS,DS)  (((HDS)?0x04:0)|((DS)&0x03)|0x00)
#define CB2_RELATIVE_SEEK(RCN)     ((RCN)&0xFF)

#define RBN_RELATIVE_SEEK (0)

#define CBN_DUMPREG    (1)
#define CB0_DUMPREG    (0x0E)

#define RBN_DUMPREG (10)

#define CBN_READ_ID    (2)
#define CB0_READ_ID(MFM)     (((MFM)?0x40:0)|0x0A)
#define CB1_READ_IN(HDS,DS)  (((HDS)?0x04:0)|((DS)&0x03)|0x00)

#define RBN_READ_ID (7)

#define CBN_PERPEND_MODE    (2)
#define CB0_PERPEND_MODE          (0x12)
#define CB1_PERPEND_MODE(OW,D,GAP,WGATE) \
	(((OW)?0x80:0)|(((D)<<2)&0x3C)|((GAP)?0x02:0)|((WGATE)?0x01:0)|0x00)

#define GET_D_FROM_CB1_PERPEND_MODE(VAL) ((VAL)>>2&0x0F)

#define RBN_PERPEND_MODE (0)

#define CBN_LOCK        (1)
#define CB0_LOCK(LOCK)  (((LOCK)?0x80:0)|0x14)

#define RBN_LOCK (1)

#endif /* FDCAMREG_INCLUDE */
