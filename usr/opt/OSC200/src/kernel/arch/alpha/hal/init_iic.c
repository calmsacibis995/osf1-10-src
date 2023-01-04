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
static char *rcsid = "@(#)$RCSfile: init_iic.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1992/08/31 12:15:16 $";
#endif
/*
 * init_iic.c  -  This module contains the initialization code for the IIC
 *                bus controller chip (PCD8584) on Cobra.  It uses the LBus
 *                mailbox registers to access the chip, and therefore
 *                uses the mailbox CSR read and write macros to access
 *                the IIC chip's registers.
 *
 **********NOTES: This code is not "SMP safe".  It makes no
 *                assumptions about current IPL and probably should
 *                not be used at splextreme.  For example, a single
 *                byte write to the EEPROM requires about 20 ms
 *                (that milliseconds!) of recovery time so a memory
 *                correctable error log of four bytes will take about
 *                80 ms.  Use this code without adult supervision
 *                is not recommended.
 *
 *                The struct *Iic_ctlr MUST be alloc-ed and init-ed
 *                prior to any calls to this function.  (This is
 *                done in the function lbusconfl1 in ka_cobra.c).
 *
 *                The registers of the PCD8584 are accessed via an
 *                external control and data address.  The internal
 *                chip register address must be written to the control
 *                register.  Then that register can be accessed via the
 *                external data register.
 */

#include <arch/alpha/hal/iic.h>

struct controller   *Iic_ctlr;

void init_iic()
{
#ifdef IICDEBUG
  printf ("start of init\n");
#endif

  /* reset chip and get ready to access register S0' */
  WR_IIC_CNTL(Iic_ctlr, IIC_INIT);

  /* 
   * write own address (0xAC) to S0' --- It must be shifted to conform to
   * the bit definitions in this register
   */
  WR_IIC_DATA(Iic_ctlr, IOCNTL >> 1);

  /* next access must be to register S2 */
  WR_IIC_CNTL(Iic_ctlr, IIC_PIN | IIC_S2);

  /* write clock bits to S2 */
  WR_IIC_DATA(Iic_ctlr, IIC_CLOCK_6 | IIC_SCL_90);

  /* enable serial mode on the controller chip */
  WR_IIC_CNTL(Iic_ctlr, IIC_AK);

#ifdef IICDEBUG
  printf ("through init\n");
#endif
}







