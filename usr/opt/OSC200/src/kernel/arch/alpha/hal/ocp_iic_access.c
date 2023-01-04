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
static char *rcsid = "@(#)$RCSfile: ocp_iic_access.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1992/08/31 12:16:00 $";
#endif
/*
 * ocp_iic_access.c  -  This module contains code that allows us to turn the
 *                      LEDs on the OCP (operator Control Panel) on or off and
 *                      check the status of the halt switch or baud rate switch
 *                      on the OCP.  This is done through the PCD8584 adapter
 *                      on Cobra.  The routine uses the LBus mailbox registers
 *                      to access the chip, and therefore uses the mailbox CSR
 *                      read and write macros to access the IIC chip's
 *                      registers.
 *
 ****************NOTE: This code is not "SMP safe".  It makes no
 *                     assumptions about current IPL and probably should
 *                     not be used at splextreme.  For example, a single
 *                     byte write to the EEPROM requires about 20 ms
 *                     (that milliseconds!) of recovery time so a memory
 *                     correctable error log of four bytes will take about
 *                     80 ms.  Use this code without adult supervision
 *                     is not recommended.
 *
 *                     The struct *Iic_ctlr MUST be alloc-ed and init-ed
 *                     prior to any calls to this function.  (This is
 *                     done in the function lbusconfl1 in ka_cobra.c).
 *
 *                     The registers of the PCD8584 are accessed via an
 *                     external control and data address.  The internal
 *                     chip register address must be written to the control
 *                     register.  Then that register can be accessed via
 *                     the external data register.
 */

#include <arch/alpha/hal/iic.h>

extern struct controller    *Iic_ctlr;

int                         ocp_led_off();
int                         ocp_led_on();
unsigned char               ocp_read();
unsigned char               ocp_get_state();
void                        wait_for_iic_free();
void                        wait_for_pin();
int                         check_lrb_on_read();
int                         check_lrb_on_write();


int ocp_led_off(led)
enum ocp_led  led;  /* the LSByte of led is the bit mask; the next higher
                     * order byte is the IIC address; the rest are zero */
{
  unsigned char curr_state;

  /* read in the current state of the OCP IIC register */
  curr_state = ocp_get_state((unsigned char)(led >> 8));

  /* now write back the data read in plus the bit of the LED to turn off */

  wait_for_iic_free();

  /* write slave address into S0, keep the R/!W bit clear */
  WR_IIC_DATA(Iic_ctlr, (unsigned char)((led >> 8) | IIC_WRITE_DIR));

  /* tell controller to start transferring the byte */
  WR_IIC_CNTL(Iic_ctlr, IIC_START);

  wait_for_pin();
  if (check_lrb_on_write((unsigned char)(led >> 8), 0) == 0)
    return(0);

  /*
   * make sure the input pins of the register that has them are always
   * written with ones
   */
  if (led > 0x4200)
    curr_state |= 0xE8;

  /* write the data to be written to the IIC chip that will clear the led */
  if ((led != cpu1_led_mbz) && (led != cpu2_led_mbz))
    {
      WR_IIC_DATA(Iic_ctlr, (curr_state | (unsigned char)(led & 0x00FF)));
    }
  else
    {
      /* should only write zeros to the mbz bits */
      WR_IIC_DATA(Iic_ctlr, (curr_state & ~((unsigned char)(led & 0x00FF))));
    }

  wait_for_pin();
  if (check_lrb_on_write((unsigned char)(led >> 8), 0) == 0)
    return(0);
  
  /* tell controller to stop sending data */
  WR_IIC_CNTL(Iic_ctlr, IIC_STOP);

  return(1);
}

int ocp_led_on(led)
enum ocp_led  led;  /* the LSByte of led is the bit mask; the next higher
                     * order byte is the IIC address; the rest are zero */
{
  unsigned char curr_state;

  /* read in the current state of the OCP IIC register */
  curr_state = ocp_get_state((unsigned char)(led >> 8));

  /* now write back the data read and mask the bit of the LED to turn on */

  wait_for_iic_free();

  /* write slave address into S0, keep the R/!W bit clear */
  WR_IIC_DATA(Iic_ctlr, (unsigned char)((led >> 8) | IIC_WRITE_DIR));

  /* tell controller to start transferring the byte */
  WR_IIC_CNTL(Iic_ctlr, IIC_START);

  wait_for_pin();
  if (check_lrb_on_write((unsigned char)(led >> 8), 0) == 0)
    return(0);

  /*
   * make sure the input pins of the register that has them are always
   * written with ones
   */
  if (led > 0x4200)
    curr_state |= 0xE8;

  /* write the data to be written to the IIC chip that will set the led */
  WR_IIC_DATA(Iic_ctlr, (curr_state & ~((unsigned char)(led & 0x00FF))));

  wait_for_pin();
  if (check_lrb_on_write((unsigned char)(led >> 8), 0) == 0)
    return(0);
  
  /* tell controller to stop sending data */
  WR_IIC_CNTL(Iic_ctlr, IIC_STOP);

  return(1);
}

unsigned char ocp_read(device)
enum ocp_rd_dev  device;  /* the LSByte of device is the bit mask; the next
			   * higher order byte is the IIC address; the rest
			   * are zero */
{
  unsigned char curr_state;

  /* read in the current state of the OCP IIC register */
  curr_state = ocp_get_state((unsigned char)(device >> 8));

  /* return the value of just the bits requested */
  curr_state = curr_state & (unsigned char)(device & 0x00FF);
  if (device & 0x00FF == 0x0008)
    {
      /* the halt status was requested */
      return (curr_state >> 3);
    }
  else if (device & 0xFF == 0xE0)
    {
      /* the bit rate status was requested */
      return (curr_state >> 5);
    }
  else
    printf ("ERROR: Invalid OCP read request\n");
}

/* routine to read in the current state of the OCP IIC register */
unsigned char ocp_get_state(address)
unsigned char   address;
{
  unsigned char slave_address;

  wait_for_iic_free();

  /* write slave address into S0, keep the R/!W bit set */
  WR_IIC_DATA(Iic_ctlr, address | IIC_READ_DIR);

  /* tell controller to start transferring the byte */
  WR_IIC_CNTL(Iic_ctlr, IIC_START);

  wait_for_pin();
  if (check_lrb_on_read(address, 0) == 0)
    return((unsigned char)0);

  /* turn off ACKs before the 2nd to last byte of data is read in */
  WR_IIC_CNTL(Iic_ctlr, IIC_NAK);

  /* first read is the device address */
  slave_address = (unsigned char)RD_IIC_DATA(Iic_ctlr);
  if ((slave_address & 0xFE) != address)
    printf ("Incorrect IIC device responded!!! FATAL ERROR!!!\n");
#ifdef IICDEBUG
  printf ("Data is coming from device 0x%x\n", slave_address & 0xFE);
#endif IICDEBUG

  wait_for_pin();

  /* tell controller to stop receiving data */
  WR_IIC_CNTL(Iic_ctlr, IIC_STOP);

  return ((unsigned char)RD_IIC_DATA(Iic_ctlr));
}














