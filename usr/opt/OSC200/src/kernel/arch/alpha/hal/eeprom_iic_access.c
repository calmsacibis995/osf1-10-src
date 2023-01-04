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
static char *rcsid = "@(#)$RCSfile: eeprom_iic_access.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1992/08/31 12:14:59 $";
#endif
/*
 * eeprom_iic_access.c  -  This module contains the EEPROM read and write
 *                         routines for the IIC bus adapter used on Cobra
 *                         (PCD8584).  It uses the LBus mailbox registers
 *                         to access the chip, and therefore uses the mailbox
 *                         CSR read and write macros to access the IIC chip's
 *                         registers.
 *
 *******************NOTES: This code is not "SMP safe".  It makes no
 *                         assumptions about current IPL and probably should
 *                         not be used at splextreme.  For example, a single
 *                         byte write to the EEPROM requires about 20 ms
 *                         (that milliseconds!) of recovery time so a memory
 *                         correctable error log of four bytes will take about
 *                         80 ms.  Use this code without adult supervision
 *                         is not recommended.
 *
 *                         The struct *Iic_ctlr MUST be alloc-ed and init-ed
 *                         prior to any calls to this function.  (This is
 *                         done in the function lbusconfl1 in ka_cobra.c).
 *
 *                         These functions are mid-level primitives that allow
 *                         us to go to and from the EEPROMs on the other
 *                         modules.  They use the low level primitives in
 *                         general_iic_routines.c, but are called by
 *                         the function log_iic_error which is the only
 *                         function that should be called from the OS when
 *                         it needs to get to the EEPROMs.
 *
 *                         The registers of the PCD8584 are accessed via an
 *                         external control and data address.  The internal
 *                         chip register address must be written to the control
 *                         register.  Then that register can be accessed via
 *                         the external data register.
 */

#include <arch/alpha/hal/iic.h>
#include <arch/alpha/machparam.h>

extern struct controller    *Iic_ctlr;

int                         write_iic_eeprom();
unsigned char               *read_iic_eeprom();
unsigned char               *read_entire_iic_eeprom();
void                        wait_for_iic_free();
void                        wait_for_pin();
int                         check_lrb_on_read();
int                         check_lrb_on_write();


int write_iic_eeprom(device, addr, data, size)
unsigned char    device;
unsigned char    addr;
unsigned char    *data;
unsigned char    size;
{
  unsigned char words;            /* number of 2 byte transfer possible      */
  unsigned char odd = size % 2;   /* if 1, a single byte transfer must occur */
  unsigned char bytes;            /* # of bytes transferred each loop        */

  /* size of 0 indicates a 256 byte write (write the entire EEPROM) */
  words = (size == 0) ? 128 : (size / 2);

  do
    {
      wait_for_iic_free();
      
      /* write slave address into S0, keep the R/!W bit clear */
      WR_IIC_DATA(Iic_ctlr, device | IIC_WRITE_DIR);

      /* tell controller to start transferring the byte */
      WR_IIC_CNTL(Iic_ctlr, IIC_START);

      wait_for_pin();
      if (check_lrb_on_write(device, addr) == 0)
	return(0);

      /* write the EEPROM address to be written into the IIC controller chip */
      WR_IIC_DATA(Iic_ctlr, addr);

      wait_for_pin();
      if (check_lrb_on_write(device, addr) == 0)
	return(0);

      /* write the EEPROM data to be written into the IIC controller chip */
      WR_IIC_DATA(Iic_ctlr, *data++);

      wait_for_pin();
      if (check_lrb_on_write(device, addr) == 0)
	return(0);

      if (!odd)
	{
	  /* write the next byte of data for the EEPROM also */
	  WR_IIC_DATA(Iic_ctlr, *data++);
	  
	  wait_for_pin();
	  if (check_lrb_on_write(device, addr + 1) == 0)
	    return(0);

	  addr += 2;
	  bytes = 2;
	}
      else
	{
	  /*
	   * indicate that the odd transfer has happened and increment words
	   * so that the decrement at the end of the loop has no net effect
	   */
	  odd = 0;
	  ++words;
	  ++addr;
	  bytes = 1;
	}
      
      /* tell controller to stop sending data */
      WR_IIC_CNTL(Iic_ctlr, IIC_STOP);
      
      /*
       * Delay to allow the write to happen. Otherwise if we tried to access
       * the same EEPROM, it wouldn't acknowledge its address while the
       * write was taking place.  The time needed to do the write depends
       * on how many bytes must be written (20ms/byte).
       */
      DELAY (bytes * 20000);
    }
  while (--words != 0);

  return(1);

}



unsigned char *read_iic_eeprom(device, addr, size)
unsigned char    device;
unsigned char    addr;
unsigned char    size;
{
  unsigned char   slave_address;
  unsigned char   eeprom_byte[256];
  int             i;
#ifdef IICDEBUG
  unsigned char   num_left;
  int             j;
#endif IICDEBUG

  /*
   * Note: this routine will only work if the read done is at least 3 bytes
   *       in size so if the caller has passed a value to the size argument
   *       that is less than 3, we must still read 4 bytes.
   */
  if (size < 3)
    size = 3;

  wait_for_iic_free();

  /* write slave address into S0, keep the R/!W bit clear */
  WR_IIC_DATA(Iic_ctlr, device | IIC_WRITE_DIR);

  /* tell controller to start transferring the byte */
  WR_IIC_CNTL(Iic_ctlr, IIC_START);

  wait_for_pin();
  if (check_lrb_on_write(device, addr) == 0)
    return ((unsigned char *)0);

  /* write the EEPROM address to be read into the IIC controller chip */
  WR_IIC_DATA(Iic_ctlr, addr);

  wait_for_pin();
  if (check_lrb_on_write(device, addr) == 0)
    return ((unsigned char *)0);

  /* give controller the repeated start condition */
  WR_IIC_CNTL(Iic_ctlr, IIC_RESTART);

  /* write slave address into S0 again, this time keep the R/!W bit set */
  WR_IIC_DATA(Iic_ctlr, device | IIC_READ_DIR);

  wait_for_pin();
  if (check_lrb_on_read(device, addr) == 0)
    return ((unsigned char *)0);

  /* first read is the device address */
  slave_address = (unsigned char)RD_IIC_DATA(Iic_ctlr);
  if ((slave_address & 0xFE) != device)
    printf ("Incorrect IIC device responded!!! FATAL ERROR!!!\n");
#ifdef IICDEBUG
  printf ("Data is coming from device 0x%x\n", slave_address & 0xFE);
#endif IICDEBUG

  wait_for_pin();

  /* the rest of the reads will be the data */
  for (i = 0; i < size - 2; i++)
    {
      eeprom_byte[(addr + i) & 0xFF] = (unsigned char)RD_IIC_DATA(Iic_ctlr);

      wait_for_pin();
    }

  /* turn off ACKs before the 2nd to last byte of data is read in */
  WR_IIC_CNTL(Iic_ctlr, IIC_NAK);

  eeprom_byte[(addr + size - 2) & 0xFF] = (unsigned char)RD_IIC_DATA(Iic_ctlr);
  
  wait_for_pin();

  /* tell controller to stop receiving data */
  WR_IIC_CNTL(Iic_ctlr, IIC_STOP);

  /* get the last byte from the data reg */
  eeprom_byte[(addr + size - 1) & 0xFF] = (unsigned char)RD_IIC_DATA(Iic_ctlr);

#ifdef IICDEBUG
  /* print out the data */
  printf ("The data coming in starting at address 0x%x:\n", addr);
  num_left = size;
  i = (addr / 16) * 16;
  for (j = 15; j >= 0; j--)
    {
      if ((j >= num_left + addr % 16) || (j < addr % 16))
	printf("    ");
      else
	{
	  printf("%2x ", eeprom_byte[i + j]);
	  num_left--;
	}
    }
  printf("      [%2x]\n", i);
  while (num_left > 0)
    {
      i += 16;
      if (num_left >= 16)
	{
	  for (j = 15; j >= 0; j--)
	    {
	      printf("%2x ", eeprom_byte[i + j]);
	    }
	  num_left -= 16;
	}
      else
	{
	  for (j = 15; j >= 0; j--)
	    {
	      if (j >= num_left)
		printf("    ");
	      else
		printf("%2x ", eeprom_byte[i + j]);
	    }
	  num_left = 0;
	}
      printf("      [%2x]\n", i);
    }
  printf("\n");
#endif IICDEBUG
  
  return(eeprom_byte + addr);  
}



/* this is essentially a dump EEPROM function */
unsigned char *read_entire_iic_eeprom(device)
unsigned char    device;
{
  unsigned char   slave_address;
  unsigned char   eeprom_byte[256];
  int             i;
#define IICDEBUG
#ifdef IICDEBUG
  int             j;
#endif IICDEBUG

  wait_for_iic_free();

  /* write slave address into S0, keep the R/!W bit clear */
  WR_IIC_DATA(Iic_ctlr, device | IIC_WRITE_DIR);

  /* tell controller to start transferring the byte */
  WR_IIC_CNTL(Iic_ctlr, IIC_START);

  wait_for_pin();
  if (check_lrb_on_write(device, 0) == 0)
    return ((unsigned char *)0);

  /* write the EEPROM base address into the IIC controller chip */
  WR_IIC_DATA(Iic_ctlr, 0x00);

  wait_for_pin();
  if (check_lrb_on_write(device, 0) == 0)
    return ((unsigned char *)0);

  /* give controller the repeated start condition */
  WR_IIC_CNTL(Iic_ctlr, IIC_RESTART);

  /* write slave address into S0, keep the R/!W bit set */
  WR_IIC_DATA(Iic_ctlr, device | IIC_READ_DIR);

  wait_for_pin();
  if (check_lrb_on_read(device, 0) == 0)
    return ((unsigned char *)0);

  /* first read is the device address */
  slave_address = (unsigned char)RD_IIC_DATA(Iic_ctlr);
  if ((slave_address & 0xFE) != device)
    printf ("Incorrect IIC device responded!!! FATAL ERROR!!!\n");
#ifdef IICDEBUG
  printf ("Data is coming from device 0x%x\n", slave_address & 0xFE);
#endif IICDEBUG

  wait_for_pin();

  /* the rest of the reads will be the data */
  for (i = 0; i < 254; i++)
    {
      eeprom_byte[i] = (unsigned char)RD_IIC_DATA(Iic_ctlr);

      wait_for_pin();
    }

  /* turn off ACKs before the 2nd to last byte of data is read in */
  WR_IIC_CNTL(Iic_ctlr, IIC_NAK);

  /* get the second to last byte from the data reg */
  eeprom_byte[254] = (unsigned char)RD_IIC_DATA(Iic_ctlr);
  
  wait_for_pin();

  /* tell controller to stop receiving data */
  WR_IIC_CNTL(Iic_ctlr, IIC_STOP);

  /* get the last byte from the data reg */
  eeprom_byte[255] = (unsigned char)RD_IIC_DATA(Iic_ctlr);

#ifdef IICDEBUG
  /* print out the data */
  printf ("The data coming in is:\n");
  for (i = 0; i < 256 ; i += 16)
    {
      for (j = 15; j >= 0; j--)
	{
	  printf("%2x ", eeprom_byte[i + j]);
	}
      printf("      [%2x]\n", i);
    }
  printf("\n");
#endif IICDEBUG

  return(eeprom_byte);
}
