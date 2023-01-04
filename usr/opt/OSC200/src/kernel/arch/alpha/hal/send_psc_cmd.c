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
static char *rcsid = "@(#)$RCSfile: send_psc_cmd.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1992/08/31 12:16:09 $";
#endif
/*
 * send_psc_cmd.c - This module contains the code that allows communication 
 *                  with the IIC controller on the PSC (power supply).
 *                  Communication witch the PSC begins when we send it
 *                  a command.  Then we wait for a response to our command.
 *                  In addition, this code contains specific function calls
 *                  for the only two commands I think the OS will have any
 *                  use for: the SEND UPDATE command (an inquiry to be used
 *                  when the SYS_EVENT signal is driven by the PSC) and the
 *                  SEND STATUS command, an inquiry which can be used any other
 *                  time.  It also contains a pair of functions calles
 *                  that can be used to turn on/off the LDCs once warm
 *                  swap is supported.
 *                  IIC communication is done through the PCD8584 adapter on
 *                  the I/O module of Cobra to another PCD8584 on the power
 *                  supply. The routine uses the LBus mailbox registers to
 *                  access the chip, and therefore uses the mailbox read and
 *                  write macros to access the IIC chip's registers.
 *                  There arre two forms of communication we must handle.
 *                  The first type is when we send the PSC a command and then
 *                  wait for a response to our command.  The second is the
 *                  routine that should be called whenever the SYS_EVENT
 *                  signal is driven by the PSC.
 *
 ************NOTES: This code is not "SMP safe".  It makes no
 *                  assumptions about current IPL and probably should
 *                  not be used at splextreme.  For example, a single
 *                  byte write to the EEPROM requires about 20 ms
 *                  (that milliseconds!) of recovery time so a memory
 *                  correctable error log of four bytes will take about
 *                  80 ms.  Use this code without adult supervision
 *                  is not recommended.
 *
 *                  The struct *Iic_ctlr MUST be alloc-ed and init-ed
 *                  prior to any calls to this function.  (This is
 *                  done in the function lbusconfl1 in ka_cobra.c).
 *
 *                  The routines that access the PSC will only work in
 *                  COBRA systems built in group 1 or later which contain
 *                  PSC firmware ROMs of Version T2.1 or later.
 *
 *                  The PSC commands and responses are defined in the
 *                  "Power System Messaging Protocol" document written
 *                  by Robert White.  This code is based on Revision X11
 *                  of that document.
 *
 *                  The registers of the PCD8584 are accessed via an
 *                  external control and data address.  The internal
 *                  chip register address must be written to the control
 *                  register.  Then that register can be accessed via
 *                  the external data register.
 *
 *                  Warm swap was not supported when these functions were
 *                  written so the functions that turn on/off the LDCs are
 *                  untested code.
 */

#include <arch/alpha/hal/iic.h>

unsigned char *send_psc_cmd(cmd_ptr)
unsigned char *cmd_ptr;
{
  extern struct controller  *Iic_ctlr;
  
  int                       write_psc();
  unsigned char             *read_psc();
  void                      wait_for_iic_free();
  void                      wait_to_be_addressed();
  void                      wait_for_pin();
  int                       check_lrb_on_write();

  unsigned char             slave_address;
  unsigned char             psc_pkt[256];
  int                       i;
#ifdef IICDEBUG
  unsigned char             num_left;
  int                       j;
#endif IICDEBUG

  unsigned char  numbytes = *cmd_ptr;  /* The number of bytes in the transmit
					  packet appears in the first byte of
					  the packet. NOTE: The count includes
					  this byte */
  /*
   * The format for packets sent to the PSC is assumed to be the format defined
   * in the IIC Bus Spec Rev. X11 for the power supply written by Robert White.
   * The format appears in Section 3.1 of that document, and below:
   *       - Number of bytes in the packet (including this byte)
   *       - Bus address to which responses should be sent (always 0xB6 for us)
   *       - Command
   *       - Qualifier
   *       - Parameter (as needed)
   *       - Data (as needed)
   * Note that the first field of the packet described in the document (PSC bus
   * address) has been omitted above. It is actually part of the IIC protocol
   * and not part of the PSC packet so it is handled by this function though
   * not included as part of the packet.
   */

  wait_for_iic_free();

  /* write slave address into S0, keep the R/!W bit clear */
  WR_IIC_DATA(Iic_ctlr, PSC | IIC_WRITE_DIR);

  /* tell controller to start transferring the byte */
  WR_IIC_CNTL(Iic_ctlr, IIC_START);

  wait_for_pin();
  if (check_lrb_on_write(PSC, 0) == 0)
    return ((unsigned char *)0);

#ifdef IICDEBUG
  printf ("start transfer of %d bytes\n", numbytes);
#endif IICDEBUG

  i = 0;
  while (numbytes--)
    {
      /* write a byte of the command to the PSC */
      WR_IIC_DATA(Iic_ctlr, *cmd_ptr++);

      wait_for_pin();
      if (check_lrb_on_write(PSC, 0) == 0)
	return ((unsigned char *)0);
    }

  /* tell the controller to stop sending data */
  WR_IIC_CNTL(Iic_ctlr, IIC_STOP);

  /*
   * Now that the command has been sent, we must switch the IIC controller to
   * become a slave and wait for a response from the PSC.
   *
   * The format for packets received from the PSC is assumed to be the format
   * defined in the IIC Bus Spec Rev. X11 for the power supply written by
   * Robert White. The format appears in Section 3.2 of that document,
   * and below:
   *       - Number of bytes in the packet (including this byte)
   *       - PSC bus address (always 0xB8 for us)
   *       - Command echo
   *       - Qualifier echo
   *       - Parameter echo (as needed)
   *       - Command completion status (OK/Error)
   *       - Data (as needed)
   * Note that the first field of the packet described in the document
   * (Destination address) has been omitted above. It is actually part of
   * the IIC protocol and not part of the PSC packet so it is handled by
   * this function though not included as part of the packet.
   */

  wait_to_be_addressed();

#ifdef IICDEBUG
  printf("I'm being addressed\n");
#endif IICDEBUG

  wait_for_pin();

  /* first read is the slave address (our's)*/
  slave_address = (unsigned char)RD_IIC_DATA(Iic_ctlr);

#ifdef IICDEBUG
  printf ("Data is coming in for device 0x%x\n", slave_address & 0xFE);
#endif IICDEBUG

  if (slave_address != IOCNTL)
    printf ("ERROR: This isn't our address, yet our AAS bit is set\n");

  wait_for_pin();

  /* second read is the packet size */
  numbytes = (unsigned char)RD_IIC_DATA(Iic_ctlr);
  psc_pkt[0] = numbytes;

  wait_for_pin();

  for (i = 1; i < numbytes - 1; i++)
    {
      psc_pkt[i] = (unsigned char)RD_IIC_DATA(Iic_ctlr);

      wait_for_pin();
    }

  /* get the last byte from the data register */
  psc_pkt[numbytes - 1] = (unsigned char)RD_IIC_DATA(Iic_ctlr);

#ifdef IICDEBUG
  /* print out the data */
  printf ("The data packet from device 0x%x (PSC = 0xb8) is:\n", psc_pkt[1]);
  i = 0;
  while (i + 16 <= numbytes)
    {
      for (j = 15; j >= 0; j--)
	{
	  printf ("%2x, ", psc_pkt[i + j]);
	}
      printf ("      [%2x]\n",i);
      i += 16;
    }
  if (i != numbytes)
    {
      for (j = 15; j >= 0; j--)
	{
	  if (j >= numbytes - i)
	    printf ("    ");
	  else
	    printf ("%2x, ", psc_pkt[i + j]);
	}
      printf ("      [%2x]\n",i);
    }
  printf("\n");
#endif IICDEBUG

  return(psc_pkt);
}


/*
 * Above is a generic function that will allow you to write any command you
 * wish to the PSC.  However, I believe that the only commands the operating
 * system will need to send to the PSC are the "SEND STATUS" and
 * "SEND UPDATE" commands, so I have provided these functions to do them
 * without requiring the higher level code to create the appropriate packet
 * for these commands.  Since these call send_psc_cmd, they test the
 * functionality of that function as well.
 */
       /* commands */
#define SEND     0x01
#define SET      0x03
#define CLEAR    0x04
       /* qualifiers */
#define UPDATE   0x01
#define STATUS   0x02
#define LDC      0x08


unsigned char *get_psc_status()
{
  unsigned char  cmd_ptr[4];

  cmd_ptr[0] = 0x04;     /* number of bytes in the packet */
  cmd_ptr[1] = IOCNTL;   /* bus addr to which response should be sent (0xB6) */
  cmd_ptr[2] = SEND;     /* command */
  cmd_ptr[3] = STATUS;   /* qualifier */
  
  return (send_psc_cmd(cmd_ptr));
}


unsigned char *get_psc_update()
{
  unsigned char  cmd_ptr[4];

  cmd_ptr[0] = 0x04;     /* number of bytes in the packet */
  cmd_ptr[1] = IOCNTL;   /* bus addr to which response should be sent (0xB6) */
  cmd_ptr[2] = SEND;     /* command */
  cmd_ptr[3] = UPDATE;   /* qualifier */
  
  return (send_psc_cmd(cmd_ptr));  
}


/*
 * the following functions can be used to set and clear LDCs for the disk
 * drives in case warm swap is ever supported. Note that warm swap was not
 * supported when these functions were written so THIS IS UNTESTED CODE.
 * However, since the functions above work, it's highly probable that these
 * do as well.
 */

unsigned char *set_psc_ldc(param)
unsigned char param;
{
  unsigned char  cmd_ptr[5];

  cmd_ptr[0] = 0x05;     /* number of bytes in the packet */
  cmd_ptr[1] = IOCNTL;   /* bus addr to which response should be sent (0xB6) */
  cmd_ptr[2] = SET;      /* command */
  cmd_ptr[3] = LDC;      /* qualifier */
  cmd_ptr[4] = param;    /* parameter */
  
  return (send_psc_cmd(cmd_ptr));  
}


unsigned char *clear_psc_ldc(param)
unsigned char param;
{
  unsigned char  cmd_ptr[5];

  cmd_ptr[0] = 0x05;     /* number of bytes in the packet */
  cmd_ptr[1] = IOCNTL;   /* bus addr to which response should be sent (0xB6) */
  cmd_ptr[2] = CLEAR;      /* command */
  cmd_ptr[3] = LDC;      /* qualifier */
  cmd_ptr[4] = param;    /* parameter */
  
  return (send_psc_cmd(cmd_ptr));  
}


