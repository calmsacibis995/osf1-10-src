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
static char *rcsid = "@(#)$RCSfile: general_iic_routines.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1992/08/31 12:15:09 $";
#endif
/*
 * general_iic_routines.c  -  This module contains the IIC routines that
 *                            get reused over and over again. These are
 *                            routines for the IIC bus adapter used on Cobra
 *                            (PCD8584).  It uses the LBus mailbox registers
 *                            to access the chip, and therefore uses the
 *                            mailbox CSR read and write macros to access
 *                            the IIC chip's registers.
 *
 ****************NOTES: This code is not "SMP safe".  It makes no
 *                      assumptions about current IPL and probably should
 *                      not be used at splextreme.  For example, a single
 *                      byte write to the EEPROM requires about 20 ms
 *                      (that milliseconds!) of recovery time so a memory
 *                      correctable error log of four bytes will take about
 *                      80 ms.  Use this code without adult supervision
 *                      is not recommended.
 *
 *                      The struct *Iic_ctlr MUST be alloc-ed and init-ed
 *                      prior to any calls to this function.  (This is
 *                      done in the function lbusconfl1 in ka_cobra.c).
 *
 *                      The registers of the PCD8584 are accessed via an
 *                      external control and data address.  The internal
 *                      chip register address must be written to the control
 *                      register.  Then that register can be accessed via
 *                      the external data register.
 *
 *                      These functions are the lowest level function
 *                      which do the bit twiddling and checking of the
 *                      8584 IIC controller chip.  These functions are
 *                      often-used 8584 sequences that are called from the
 *                      functions in eeprom_iic_access.c, ocp_iic_access.c,
 *                      and send_psc_cmd.c.
 *
 *                      These functions can be made into macros if
 *                      you really want to squeeze out some performance
 *                      but it seems silly because of how slow the IIC
 *                      is so I ain't gonna do it.
 */

#include <arch/alpha/hal/iic.h>

extern struct controller    *Iic_ctlr;

void                        wait_for_iic_free();
void                        wait_to_be_addressed();
void                        wait_for_pin();
int                         check_lrb_on_read();
int                         check_lrb_on_write();

union IIC_STATUS_UNION      iic_status_reg;


void wait_for_iic_free()
{
  /* wait for the IIC bus to be available (check bus busy not bit for 1) */
  do
    iic_status_reg.word = (unsigned char)RD_IIC_CNTL(Iic_ctlr);
  while (!iic_status_reg.bits.bb);
}

void wait_to_be_addressed()
{
  /*
   * wait for someone to issue your address as a slave address for a transfer
   * (check the Addressed As Slave bit for 1)
   */
  do
    iic_status_reg.word = (unsigned char)RD_IIC_CNTL(Iic_ctlr);
  while (!iic_status_reg.bits.aas);
}

void wait_for_pin()
{
  /* wait for pending interrupt not bit to set (=0) */
  do
    iic_status_reg.word = (unsigned char)RD_IIC_CNTL(Iic_ctlr);
  while (iic_status_reg.bits.pin);
}

int check_lrb_on_read(dev, addr)
unsigned int dev;
unsigned int addr;
{
  unsigned char             bad_data;

  /* check for positive acknowledgement */
  if (iic_status_reg.bits.lrb)
    {
      printf ("Error: Device 0x%x did not respond (it said NAK)\n", dev);
      printf ("       when you tried to read address %d.\n", addr);

      /* tell controller to stop receiving data */
      WR_IIC_CNTL(Iic_ctlr, IIC_STOP);

      /* get the last byte from the data reg */
      bad_data = (unsigned char)RD_IIC_DATA(Iic_ctlr);

      return(0);
    }
  else
    return (1);
}

int check_lrb_on_write(dev, addr)
unsigned int dev;
unsigned int addr;
{
  /* check for positive acknowledgement */
  if (iic_status_reg.bits.lrb)
    {
      printf ("Error: Device 0x%x did not respond (it said NAK)\n", dev);
      printf ("       when you tried to write address %d.\n", addr);
      
      /* tell controller to stop sending the data */
      WR_IIC_CNTL(Iic_ctlr, IIC_STOP);
      
      return(0);
    }
  else
    return(1);
}





