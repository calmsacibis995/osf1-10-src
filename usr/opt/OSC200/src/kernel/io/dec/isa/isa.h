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
 * @(#)$RCSfile: isa.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/24 22:40:51 $
 */

/* isa.h
 *
 *  definitions for the isa bus
 *
 *
 *  Modification History
 *
 *   04-Sep-91   Joe Notarangelo
 *               created this file
 */


#ifndef __ISA_H__
#define __ISA_H__

/* interrupt levels for isa bus */


#define ISA_L15    15
#define ISA_L14    14
#define ISA_L12    12
#define ISA_L11    11
#define ISA_L10    10
#define ISA_L9      9
#define ISA_L7      7
#define ISA_L6      6
#define ISA_L5      5
#define ISA_L4      4
#define ISA_L3      3

struct isa_config_struct{
  char *isa_name;
  int   isa_unit;
  int   isa_level;
  int   isa_channel;
  int   isa_csr;
};

/* Macros for I/O. */

#define MGL(x)  ( (((long) (x) & 1) << 30) | \
    (((long) (x) & 0xe) << 26) | \
    (((long) (x) & 0xffff0) << 1) ) 

#define MGLW(x)  ( (((long) (x) & 0xe) << 26) | (((long) (x) & 0xffff0) << 1)) 

/* pc interrupt channel definitions */

/* combo chip interrupts */
#define PC_UART_INTR   0x0
#define PC_LPR_INTR    0x1
#define PC_KB_MS_INTR  0x2
/* dma interrupts */
#define PC_ICW2_INTR        0x5
#define PC_DMA_CHAIN_INTR   0x4
#define PC_DMA_TCOUNT_INTR  0x7
/* isa interrupts */
#define PC_ISA3_INTR       0xe
#define PC_ISA4_INTR       0xf
#define PC_ISA5_INTR       0x10
#define PC_ISA6_INTR       0x11
#define PC_ISA7_INTR       0x12
#define PC_ISA9_INTR       0x14
#define PC_ISA10_INTR      0x15
#define PC_ISA11_INTR      0x16
#define PC_ISA12_INTR      0x17
#define PC_ISA14_INTR      0x19
#define PC_ISA15_INTR      0x1a

#define PC_IRQMAX   PC_ISA15_INTR

#endif /*__ISA_H__*/
