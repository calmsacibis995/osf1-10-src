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
 * test_iic.c  -  This module contains the code to test IIC bus functionality
 *                on Cobra (using the PCD8584).  It uses the LBus
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
 *                This code tests out the functionality of all the IIC
 *                code written for use in the kernel, but in normal
 *                operation, it's actually never used.  This code
 *                is destructive to both the EEPROMs and the OCP
 *                and it will probably cause some information from the 
 *                PSC to be lost as well so it should not be used
 *                when normal operation of the COBRA is intended.  The calls
 *                to the functions in send_psc_cmd will only work with
 *                COBRA hardware from Group 1 builds or later AND
 *                PSC firmware ROM Version T2.1 or later
 */

#include <arch/alpha/hal/iic.h>
#include <arch/alpha/machparam.h>
#include <arch/alpha/hal/kn430.h>
#include <arch/alpha/hal/dc21064.h>
#include <dec/binlog/errlog.h>

void                      init_iic();
int                       test_iic_write();
unsigned char             *test_iic_entire_eeprom_read();
unsigned char             *test_iic_read();
union IIC_STATUS_UNION    iic_status_reg;
int                       ocp_led_on();
int                       ocp_led_off();

extern struct controller  *Iic_ctlr;

void test_iic()
{
  unsigned char d[6] = { 0x37, 0x56, 0xA2, 0xC5, 0xFD, 0x05 };
  unsigned char e[6] = { 0x13, 0x17, 0x1C, 0x24, 0x28, 0x2A };
  unsigned char count[2] = {0x1C, 0x4};  /* 28, 4 */
  unsigned char z[256];
  struct el_cobra_data_mcheck cpulog1, cpulog2, cpulog3, cpulog4;
  struct el_cobra_data_mcheck cpulog5, cpulog6, cpulog7, cpulog8;
  struct el_cobra_data_memory memlog1, memlog2;
  int i;


  /* more initialization */
  for (i = 0; i < 256; i++)
    z[i] = 0;

  cpulog1.elcmc_biu_stat = (2L << 12L);
  cpulog1.elcmc_fill_syndrome = (0x73L << FILL_L_SHIFT);
  cpulog1.elcmc_bc_tag = 0x35ABCDL;
  cpulog1.elcmc_bcce = (0x37L << BCCE_EDCSYN0_SHIFT);
  cpulog1.elcmc_bccea = 0x35ABCDL;

  cpulog2.elcmc_biu_stat = (1L << 12L);
  cpulog2.elcmc_fill_syndrome = (0x52L << FILL_H_SHIFT);
  cpulog2.elcmc_bc_tag = 0xDCBA98L;
  cpulog2.elcmc_bcce = (0x2BL << BCCE_EDCSYN1_SHIFT);
  cpulog2.elcmc_bccea = 0x00DCBA9800DCBA98L;

  cpulog3.elcmc_biu_stat = (0L << 12L);
  cpulog3.elcmc_fill_syndrome = (0x26L << FILL_H_SHIFT);
  cpulog3.elcmc_bc_tag = 0xFEDCBAL;
  cpulog3.elcmc_bcce = (0x62L << BCCE_EDCSYN2_SHIFT);
  cpulog3.elcmc_bccea = 0xFEDCBAL;

  cpulog4.elcmc_biu_stat = (3L << 12L);
  cpulog4.elcmc_fill_syndrome = (0x71L << FILL_H_SHIFT);
  cpulog4.elcmc_bc_tag = 0x123456L;
  cpulog4.elcmc_bcce = (0x17L << BCCE_EDCSYN3_SHIFT);
  cpulog4.elcmc_bccea = 0x0012345600654321L;

  cpulog5.elcmc_biu_stat = (2L << 12L);
  cpulog5.elcmc_fill_syndrome = (0x1AL << FILL_L_SHIFT);
  cpulog5.elcmc_bc_tag = 0x345678L;
  cpulog5.elcmc_bcce = (0x21L << BCCE_EDCSYN0_SHIFT);
  cpulog5.elcmc_bccea = 0x876548L;

  cpulog6.elcmc_biu_stat = (1L << 12L);
  cpulog6.elcmc_fill_syndrome = ((0x04L << FILL_H_SHIFT) |
				 (0x29L << FILL_L_SHIFT));
  cpulog6.elcmc_bc_tag = 0x989898L;
  cpulog6.elcmc_bcce = ((0x40L << BCCE_EDCSYN1_SHIFT) |
			(0x5FL << BCCE_EDCSYN0_SHIFT));
  cpulog6.elcmc_bccea = 0x00ACACAC00747474L;

  cpulog7.elcmc_biu_stat = (0L << 12L);
  cpulog7.elcmc_fill_syndrome = (0x26L << FILL_H_SHIFT);
  cpulog7.elcmc_bc_tag = 0x111444L;
  cpulog7.elcmc_bcce = (0x62L << BCCE_EDCSYN2_SHIFT);
  cpulog7.elcmc_bccea = 0x444111L;

  cpulog8.elcmc_biu_stat = (3L << 12L);
  cpulog8.elcmc_fill_syndrome = (0x33L << FILL_H_SHIFT);
  cpulog8.elcmc_bc_tag = 0x7C7C7CL;
  cpulog8.elcmc_bcce = (0x32L << BCCE_EDCSYN3_SHIFT);
  cpulog8.elcmc_bccea = 0x00B4B4B400000000L;

  memlog1.elcm_merr = (MERR_EDCCEL | MERR_CAPE0 | MERR_MCAPEL |
		       MERR_DPE6 | MERR_MWDPEL);
  memlog1.elcm_mcmd1 = 0x1234567812345678L;
  /* 1Mb, 4 banks, no interleave */
  memlog1.elcm_mconf = (MCONF_BANKL | MCONF_BANKH); 
  /* bit 22 bad */
  memlog1.elcm_medc2 = (0x11DL << EDCSTAT2_SYNL_SHIFT);


  memlog2.elcm_merr = (MERR_EDCCEH | MERR_CAPE3 | MERR_MCAPEH |
		       MERR_DPE3 | MERR_MWDPEH);
  memlog2.elcm_mcmd1 = 0xFEDCBA98FEDCBA98L;
  /* 4Mb, 2 banks, 4-way interleave */
  memlog2.elcm_mconf = ((1L << MCONF_SIZEL_SHIFT) | (1L << MCONF_SIZEH_SHIFT) |
			(2L << MCONF_ILVML_SHIFT) | (2L << MCONF_ILVMH_SHIFT));
  /* bits 30 and 31 bad */
  memlog2.elcm_medc2 = (0xC3AL << EDCSTAT2_SYNH_SHIFT);


  printf ("Call init_iic from test_iic\n");
  init_iic();
/*
 * commented out these lower level EEPROM calls once the higher level calls
 * to log_iic_error were tested and working
  read_entire_iic_eeprom(MEM4ROM);
  read_iic_eeprom(MEM4ROM, 0x30, 0x20);
  write_iic_eeprom(MEM4ROM, 0x40, &d[2], 1);
  read_iic_eeprom(MEM4ROM, 0x30, 0x20);
  write_iic_eeprom(MEM4ROM, 0x47, &d[4], 2);
  read_iic_eeprom(MEM4ROM, 0x33, 0x21);
  read_entire_iic_eeprom(MEM4ROM);
  read_entire_iic_eeprom(MEM4ROM);

  write_iic_eeprom(MEM4ROM, 0x10, &e[0], 5);
  write_iic_eeprom(MEM4ROM, 0x15, &e[2], 4);
  write_iic_eeprom(MEM4ROM, 0x24, &e[4], 1);
  write_iic_eeprom(MEM4ROM, 0x22, &e[4], 2);
  write_iic_eeprom(MEM4ROM, 0x19, &e[0], 3);
  read_entire_iic_eeprom(MEM4ROM);

  read_iic_eeprom(MEM4ROM, 0x10, 0x10);
  read_iic_eeprom(MEM4ROM, 0x00, 0x20);
  read_iic_eeprom(MEM4ROM, 0x10, 0x10);
  read_iic_eeprom(MEM4ROM, 0x10, 0x08);
  read_iic_eeprom(MEM4ROM, 0x40, 0x10);
  read_entire_iic_eeprom(MEM4ROM);
*/
  DELAY (12000000);
  printf ("Turn off all LEDs\n");
  ocp_led_off(cpu1_led_mbz);
  printf ("1..");
  ocp_led_off(cpu1_led);
  printf ("2..");
  ocp_led_off(cpu2_led_mbz);
  printf ("3..");
  ocp_led_off(cpu2_led);
  printf ("4..");
  ocp_led_off(mem1_led);
  printf ("5..");
  ocp_led_off(mem2_led);
  printf ("6..");
  ocp_led_off(mem3_led);
  printf ("7..");
  ocp_led_off(mem4_led);
  printf ("8..");
  ocp_led_off(fbus_led);
  printf ("9..");
  ocp_led_off(io_led);
  printf ("10..");
  ocp_led_off(halt_led); 
  ocp_led_off(reset_led);
  printf ("11..\n");
  DELAY (1000000);

  printf ("Turn on CPU 1 LED\n");
  ocp_led_on(cpu1_led);
  DELAY (1000000);

  printf ("Turn on CPU 2 LED\n");
  ocp_led_on(cpu2_led);
  DELAY (1000000);

  printf ("Turn on Memory board 2 LED\n");
  ocp_led_on(mem2_led);
  DELAY (1000000);

  printf ("Turn on Fbus LED\n");
  ocp_led_on(fbus_led);
  DELAY (1000000);

  printf ("Turn off CPU 2 LED\n");
  ocp_led_off(cpu2_led);
  DELAY (1000000);

  printf ("Turn on Halt LED\n");
  ocp_led_on(halt_led);
  DELAY (1000000);

  printf ("Find out psc status\n");
  get_psc_status();
  printf ("Find out psc update\n");
  get_psc_update();

  printf ("Zero out Memory ROM SDD fields\n");
  write_iic_eeprom(MEM4ROM, 0x38, &z[0], 0x98);
  printf ("Initialize Memory ROM counter\n");
  write_iic_eeprom(MEM4ROM, 0x48, &count[1], 1);
  read_entire_iic_eeprom(MEM4ROM);
  printf ("Log some Memory Errors\n");
  log_iic_error(CMIC1_ERR,      MEM4ROM,  1, 0);
  log_iic_error(SYNC_ERROR_E,   MEM4ROM,  7, 0);
  log_iic_error(CMIC2_ERR,      MEM4ROM, 13, 0);
  log_iic_error(SYNC_ERROR_O,   MEM4ROM,  1, 0);
  log_iic_error(CMIC1_ERR,      MEM4ROM, 14, 0);
  log_iic_error(MEM_CRD,        MEM4ROM,  1, memlog1);
  log_iic_error(MEM_HARD_FAIL,  MEM4ROM,  6, memlog2);
  log_iic_error(MEM_HARD_FAIL,  MEM4ROM, 22, memlog1);
  log_iic_error(MEM_CRD,        MEM4ROM, 15, memlog1);
  log_iic_error(MEM_CRD,        MEM4ROM, 65, memlog2);
  log_iic_error(CA_PAR_ERR_E,   MEM4ROM, 87, memlog2);
  log_iic_error(CA_PAR_ERR_O,   MEM4ROM, 43, memlog2);
  log_iic_error(DATA_PAR_ERR_O, MEM4ROM, 27, memlog2);
  log_iic_error(DATA_PAR_ERR_E, MEM4ROM, 50, memlog1);
  read_entire_iic_eeprom(MEM4ROM);
  printf ("Zero out Memory ROM SDD fields\n");
  write_iic_eeprom(MEM4ROM, 0x38, &z[0], 0x98);
  printf ("Initialize Memory ROM counter\n");
  write_iic_eeprom(MEM4ROM, 0x48, &count[1], 1);

  printf ("Zero out CPU ROM SDD fields\n");
  write_iic_eeprom(CPU1ROM, 0x38, &z[0], 0x68);
  printf ("Initialize CPU ROM counter\n");
  write_iic_eeprom(CPU1ROM, 0x48, &count[0], 1);
  read_entire_iic_eeprom(CPU1ROM);
  printf ("Log some CPU Errors\n");
  log_iic_error(C3_WD_NOACK_E,  CPU1ROM, 0x08, 0);
  log_iic_error(C3_T_PAR_O,     CPU1ROM, 0x2D, 0);
  log_iic_error(C3_RD_PAR_O,    CPU1ROM, 0x13, 0);
  log_iic_error(C3_DT_PAR_E,    CPU1ROM, 0x01, 0);
  log_iic_error(C3_T_PAR_O,     CPU1ROM, 0xE4, 0);
  log_iic_error(C3_RD_PAR_O,    CPU1ROM, 0x05, 0);
  log_iic_error(EV_BCACHE_ERR,  CPU1ROM, 0x08, cpulog1);
  log_iic_error(C3_BCACHE_ERR,  CPU1ROM, 0x02, cpulog1);
  log_iic_error(EV_BCACHE_ERR,  CPU1ROM, 0x03, cpulog2);
  log_iic_error(EV_BCACHE_ERR,  CPU1ROM, 0x01, cpulog1);
  log_iic_error(C3_BCACHE_ERR,  CPU1ROM, 0x18, cpulog2);
  log_iic_error(C3_BCACHE_ERR,  CPU1ROM, 0x30, cpulog4);
  log_iic_error(C3_BCACHE_ERR,  CPU1ROM, 0x4B, cpulog3);
  log_iic_error(C3_BCACHE_ERR,  CPU1ROM, 0xEE, cpulog4);
  log_iic_error(EV_BCACHE_ERR,  CPU1ROM, 0x78, cpulog3);
  log_iic_error(EV_BCACHE_ERR,  CPU1ROM, 0x87, cpulog4);
  log_iic_error(EV_BCACHE_ERR,  CPU1ROM, 0xD2, cpulog5);
  log_iic_error(EV_BCACHE_ERR,  CPU1ROM, 0xC3, cpulog6);
  log_iic_error(EV_BCACHE_ERR,  CPU1ROM, 0xB4, cpulog7);
  log_iic_error(C3_BCACHE_ERR,  CPU1ROM, 0xA5, cpulog5);
  log_iic_error(C3_BCACHE_ERR,  CPU1ROM, 0x96, cpulog6);
  log_iic_error(C3_BCACHE_ERR,  CPU1ROM, 0x87, cpulog7);

  read_entire_iic_eeprom(CPU1ROM);
  printf ("Zero out CPU ROM SDD fields\n");
  write_iic_eeprom(CPU1ROM, 0x38, &z[0], 0x68);
  printf ("Initialize CPU ROM counter\n");
  write_iic_eeprom(CPU1ROM, 0x48, &count[0], 1);

  printf ("Zero out IO ROM SDD fields\n");
  write_iic_eeprom(IOROM, 0x38, &z[0], 0x68);
  printf ("Initialize IO ROM counter\n");
  write_iic_eeprom(IOROM, 0x48, &count[0], 1);
  read_entire_iic_eeprom(IOROM);
  printf ("Log some IO Errors\n");
  log_iic_error(IO_FB_DMA_PAR_E, IOROM, 0x44, 0);
  log_iic_error(IO_BUSSSYNC,     IOROM, 0x5C, 0);
  log_iic_error(IO_NCR2_A,       IOROM, 0x09, 0);
  log_iic_error(IO_CA_NOACK_O,   IOROM, 0x31, 0);
  log_iic_error(IO_NCR4_B,       IOROM, 0x01, 0);
  log_iic_error(IO_NCR4_A,       IOROM, 0xF4, 0);
  log_iic_error(IO_NCR2_A,       IOROM, 0x07, 0);
  log_iic_error(IO_NCR4_A,       IOROM, 0x16, 0);
  read_entire_iic_eeprom(IOROM);
  printf ("Zero out IO ROM SDD fields\n");
  write_iic_eeprom(IOROM, 0x38, &z[0], 0x68);
  printf ("Initialize IO ROM counter\n");
  write_iic_eeprom(IOROM, 0x48, &count[0], 1);

  printf ("finished IIC tests\n");  
}











