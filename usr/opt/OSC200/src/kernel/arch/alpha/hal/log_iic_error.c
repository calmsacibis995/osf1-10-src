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
static char *rcsid = "@(#)$RCSfile: log_iic_error.c,v $ $Revision: 1.1.2.8 $ (DEC) $Date: 1992/11/09 18:47:27 $";
#endif
/*
 * log_iic_error.c  -  This module contains the EEPROM error logger for the
 *                     IIC bus.  Logging is done through the PCD8584 adapter
 *                     on Cobra.  The routine uses the LBus mailbox registers
 *                     to access the chip, and therefore uses the mailbox CSR
 *                     read and write macros to access the IIC chip's
 *                     registers.
 *
 ***************NOTES: This code is not "SMP safe".  It makes no
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
 *                     This function is the high level means to access
 *                     the EEPROMs on the other modules.  It uses the
 *                     lower level primitives in eeprom_iic_access.c
 *                     This is the only function that should be called
 *                     from the OS when it needs to get to the EEPROMs.
 */

#include <arch/alpha/hal/iic.h>
#include <arch/alpha/hal/kn430.h>
#include <arch/alpha/hal/dc21064.h>
#include <dec/binlog/errlog.h>

int              log_iic_error();
void             write_iic_eeprom();
unsigned char    *read_iic_eeprom();

int log_iic_error(descriptor, device, num_errors, errorlog)
unsigned char                               descriptor;
unsigned char                               device;
unsigned char                               num_errors;
union {
  struct el_cobra_data_mcheck mcheck_log;
  struct el_cobra_data_memory memory_log;
  int    bogus;                           } errorlog;

/*
 * errorlog is a union of to structs defined in ka_cobra.h,
 * it expects a memory_log for some memory errors (MEM_HARD_FAILs,
 * MEM_UNCORR_BITs, and MEM_CRDs).  It expects a mcheck_log for
 * some CPU errors (EV_BCACHE_ERRs and C3_BCACHE_ERRs).  For all
 * other errors, no log is expected and a 0 can be passed to this
 * argument.  That is why bogus is in the union.
 */

{
  unsigned char               *read_data;
  unsigned char               rdata;
  unsigned char               address;
  unsigned char               symloglength;
  unsigned char               already_there;       /* a flag */
  unsigned char               i, j, k;             /* counters */
  unsigned char               error_types = 0;     /* number of uniquely 
						      loggable errors */
  unsigned int                bad_syndrome;
  unsigned int                corr_bits;
  unsigned int                bad_cmic_shift;
  union MEM_BAD_BIT_DESC      mem_error_entry[3];
  union CPU_BAD_BIT_DESC      cpu_error_entry[5];
  union {
    unsigned int              word;
    unsigned char             bytes[4];
  } full_desc;

  unsigned int                syn2bit();
  unsigned int                compute_offset_field();


  switch (device)
    {
    case CPU1ROM:
    case CPU2ROM:
      if ((descriptor < C3_CA_NOACK) || (descriptor > C3_DT_PAR_O))
	{
	  switch (descriptor)
	    {
	    case EV_BCACHE_ERR:
	      /* 
	       * CPU cycle detected correctable error in the bcache,
	       * log where it is;
	       * determine the syndrome and offset from the values in the
	       * error registers
	       */

	      /* check the CPU error registers for the needed info */
	      if ((errorlog.mcheck_log.elcmc_fill_syndrome & FILL_L) != 0)
		{
		  /* error in the low longword */
		  error_types++;
		  
		  cpu_error_entry[error_types].bits.syndrome =
		    errorlog.mcheck_log.elcmc_fill_syndrome & FILL_L;
		  
		  /*
		   * bit 0 of the offset should be 0 since this is the
		   * low longword; BIU_STAT<13:12> must go in bits <2:1>
		   * of the offset; BC_TAG<21:14> must go in bits <15:8>
		   * of the offset
		   */
		  cpu_error_entry[error_types].bits.offset =
		    (((errorlog.mcheck_log.elcmc_biu_stat & BIU_FILL_QW) >>
		      11) |
		     ((errorlog.mcheck_log.elcmc_bc_tag & 0x3FC000) >> 6));
		}
	      if ((errorlog.mcheck_log.elcmc_fill_syndrome & FILL_H) != 0)
		{
		  /* error in the high longword */
		  error_types++;
		  
		  cpu_error_entry[error_types].bits.syndrome =
		    (errorlog.mcheck_log.elcmc_fill_syndrome & FILL_H) >>
		      FILL_H_SHIFT;
		  
		  /*
		   * bit 0 of the offset should be 1 since this is the
		   * high longword; BIU_STAT<13:12> must go in bits <2:1>
		   * of the offset; BC_TAG<21:14> must go in bits <15:8>
		   * of the offset
		   */
		  cpu_error_entry[error_types].bits.offset = 0x1 |
		    (((errorlog.mcheck_log.elcmc_biu_stat & BIU_FILL_QW) >>
		      11) |
		     ((errorlog.mcheck_log.elcmc_bc_tag & 0x3FC000) >> 6));
		}
	      break;

	    case C3_BCACHE_ERR:
	      /* 
	       * C-Bus cycle detected correctable error in the bcache,
	       * log where it is;
	       * determine the syndrome and offset from the values in the
	       * error registers
	       */

	      /* check the C-bus error registers for the needed info */
	      if ((errorlog.mcheck_log.elcmc_bcce & BCCE_EDCSYN0) != 0)
		{
		  /* error in LW-0 */
		  error_types++;
		  
		  cpu_error_entry[error_types].bits.syndrome =
		    ((errorlog.mcheck_log.elcmc_bcce & BCCE_EDCSYN0) >>
		     BCCE_EDCSYN0_SHIFT);
		  
		  /*
		   * bits <1:0> of the offset should be 00 to indicate
		   * LW-0; BCCEA<16:9> must go in bits <15:8>
		   * of the offset
		   */
		  cpu_error_entry[error_types].bits.offset =
		    ((errorlog.mcheck_log.elcmc_bccea & 0x1FE00) >> 1);
		}
	      if ((errorlog.mcheck_log.elcmc_bcce & BCCE_EDCSYN1) != 0)
		{
		  /* error in LW-1 */
		  error_types++;
		  
		  cpu_error_entry[error_types].bits.syndrome =
		    ((errorlog.mcheck_log.elcmc_bcce & BCCE_EDCSYN1) >>
		     BCCE_EDCSYN1_SHIFT);
		  
		  /*
		   * bits <1:0> of the offset should be 01 to indicate
		   * LW-1; BCCEA<48:41> must go in bits <15:8>
		   * of the offset
		   */
		  cpu_error_entry[error_types].bits.offset = 0x1 |
		    ((errorlog.mcheck_log.elcmc_bccea & (0x1FE00L << 32))
		     >> 33);
		}
	      if ((errorlog.mcheck_log.elcmc_bcce & BCCE_EDCSYN2) != 0)
		{
		  /* error in LW-2 */
		  error_types++;
		  
		  cpu_error_entry[error_types].bits.syndrome =
		    ((errorlog.mcheck_log.elcmc_bcce & BCCE_EDCSYN2) >>
		     BCCE_EDCSYN2_SHIFT);
		  
		  /*
		   * bits <1:0> of the offset should be 10 to indicate
		   * LW-2; BCCEA<16:9> must go in bits <15:8>
		   * of the offset
		   */
		  cpu_error_entry[error_types].bits.offset = 0x2 |
		    ((errorlog.mcheck_log.elcmc_bccea & 0x1FE00) >> 1);
		}
	      if ((errorlog.mcheck_log.elcmc_bcce & BCCE_EDCSYN3) != 0)
		{
		  /* error in LW-3 */
		  error_types++;
		  
		  cpu_error_entry[error_types].bits.syndrome =
		    ((errorlog.mcheck_log.elcmc_bcce & BCCE_EDCSYN3) >>
		     BCCE_EDCSYN3_SHIFT);
		  
		  /*
		   * bits <1:0> of the offset should be 11 to indicate
		   * LW-3; BCCEA<48:41> must go in bits <15:8>
		   * of the offset
		   */
		  cpu_error_entry[error_types].bits.offset = 0x3 |
		    ((errorlog.mcheck_log.elcmc_bccea & (0x1FE00L << 32))
		     >> 33);
		}
	      break;

	    default:
	      printf ("Error:%d is an invalid error type for the CPU Module\n",
		      descriptor);
	      return (0);
	    }           /* end of switch (descriptor) */

	  /*
	   * see if each unique error entry is already in the EEPROM and
	   * if it is,  update the counter for the error
	   */
	  for (k = 1; k <= error_types; k++)
	    {
	      /* grab the new error count */
	      cpu_error_entry[k].bits.count = num_errors;
	      
	      /* check all the entries */
	      already_there = FALSE;
	      full_desc.word = 1;
	      for (i = 0;
		   (i < CPU_CORRERR_FIELDS) && (!already_there) &&
		   (full_desc.word != 0);
		   i++) 
		{
		  read_data =
		    read_iic_eeprom(device,
				    cpu_iic_eeprom.cpu_sdd.bad_bit_desc[i], 4);
		  
		  for (j = 0; j < 4; j++)
		    full_desc.bytes[j] = *(read_data + j);
		  
		  /* don't compare the count fields */
		  if ((cpu_error_entry[k].word & 0x00FFFFFF) ==
		      (full_desc.word & 0x00FFFFFF))
		    already_there = TRUE;
		}

	      /*
	       * if the error wasn't already there, write it out as
	       * long as there's room in the EEPROM
	       */
	      if (full_desc.word == 0)
		{
		  i--;
		  /* save the descriptor in this location */
		  write_iic_eeprom(device,
				   cpu_iic_eeprom.cpu_sdd.bad_bit_desc[i],
				   &cpu_error_entry[k].bytes[0], 4);
		  
		  /* and update the symloglength field */
		  symloglength = CPU_SYMLL_INIT + 4 * (i + 1);
		  write_iic_eeprom(device,
				   cpu_iic_eeprom.cpu_sdd.sym_log_len,
				   &symloglength, 1);
		}
	      else if (already_there)
		{
		  /* don't allow counter to wrap-around */
		  if (((full_desc.bytes[3] + cpu_error_entry[k].bits.count)
		       & 0x0FF) < full_desc.bytes[3])
		    full_desc.bytes[3] = 0xFF;
		  else
		    full_desc.bytes[3] += cpu_error_entry[k].bits.count;
		  /* save the new counter */
		  i--;
		  write_iic_eeprom(device,
				   cpu_iic_eeprom.cpu_sdd.bad_bit_desc[i] + 3,
				   &full_desc.bytes[3], 1);
		}
	      else
		{
		  printf ("There are more CPU errors than there is room for in the EEPROM\n");
		  return (0);
		}
	    }
	}
      else
	{
	  /*
	   * uncorrectable error, update the appropriate counter for the
	   * address that corresponds to this error descriptor
	   */
	  address = descriptor;
	  
	  /* first read the old counter */
	  read_data = read_iic_eeprom(device, address, 1);
	  
	  /*
	   * see if we're going to max out the counter with the additional
	   * numbers of errors being logged
	   */
	  if (((*read_data + num_errors) & 0x0FF) < *read_data)
	    rdata = 0xFF;
	  else
	    rdata = *read_data + num_errors;
	  
	  /* write the counter back out */
	  write_iic_eeprom(device, address, &rdata, 1);
	}
      break;
      
    case IOROM:
      /*
       * all I/O module errors are uncorrectable so simply update the
       * appropriate counter for the error
       */
      if ((descriptor < IO_CA_NOACK_E) || (descriptor >= IO_ERRTYPE34))
	{
	  printf ("Error: %d is an invalid error type for the IO Module\n",
		  descriptor);
	  return (0);
	}
      else
	{
	  address = descriptor;
	  
	  /* first read the old counter */
	  read_data = read_iic_eeprom(device, address, 1);
	  
	  /*
	   * see if we're going to max out the counter with the additional
	   * numbers of errors being logged
	   */
	  if (((*read_data + num_errors) & 0x0FF) < *read_data)
	    rdata = 0xFF;
	  else
	    rdata = *read_data + num_errors;

	  /* write the counter back out */
	  write_iic_eeprom(device, address, &rdata, 1);
	}
      break;
      
    case MEM1ROM:
    case MEM2ROM:
    case MEM3ROM:
    case MEM4ROM:
      switch (descriptor)
	{
	case MEM_HARD_FAIL:
	case MEM_UNCORR_BIT:
	  /* uncorrectable errors should be logged as best as possible */
	  error_types++;
	  mem_error_entry[error_types].bits.err_typ = descriptor;
	  if (num_errors > 1)
	    mem_error_entry[error_types].bits.multi = 1;
	  else
	    mem_error_entry[error_types].bits.multi = 0;

	  /*
	   * it's impossible to determine the failing ram_num and bits
	   * within the ram (bitmask)
	   */
	  mem_error_entry[error_types].bits.ram_num = 0;
	  mem_error_entry[error_types].bits.bitmask = 0;

	  mem_error_entry[error_types].bits.offset =
	    compute_offset_field(errorlog.memory_log);

	  break;

	case MEM_CRD:
	  /*
	   * find out which CMIC got the correctable error and grab the
	   * syndrome of the correctable error
	   */
	  if (errorlog.memory_log.elcm_merr & MERR_EDCCEL)
	    {
	      /* CMIC1 got a correctable error */
	      error_types++;
	      bad_syndrome = errorlog.memory_log.elcm_medc2 & EDCSTAT2_SYNL;

	      /* find the corrected bit(s) that correspond to the syndrome */
	      corr_bits = syn2bit (bad_syndrome);
	      
	      /* determine the bitmask and ram_num from corr_bits */
	      /* CMIC1 bits are in the low half of the x4 DRAMs */
	      if (corr_bits <= 139)
		{
		  /* single bit error: is it an odd or even bit? */
		  if (corr_bits % 2 == 0)
		    mem_error_entry[error_types].bits.bitmask = 0x1 << 0;
		  else
		    mem_error_entry[error_types].bits.bitmask = 0x2 << 0;
		  
		  mem_error_entry[error_types].bits.ram_num =
		    ((corr_bits / 2) % 70) + 1;
		}
	      else if (corr_bits != 300)
		{
		  /* double bit error */
		  mem_error_entry[error_types].bits.bitmask = 0x3 << 0;
		  
		  mem_error_entry[error_types].bits.ram_num = corr_bits - 139;
		}
	      else
		{
		  printf("Fatal error - corr. memory error not logged \n");
		  return(0);
		}
	      
	      mem_error_entry[error_types].bits.offset =
		compute_offset_field(errorlog.memory_log);	      
	    }

	  if (errorlog.memory_log.elcm_merr & MERR_EDCCEH)
	    {
	      /* CMIC2 got a correctable error */
	      error_types++;
	      bad_syndrome = ((errorlog.memory_log.elcm_medc2 & EDCSTAT2_SYNH)
			      >> EDCSTAT2_SYNH_SHIFT);

	      /* find the corrected bit(s) that correspond to the syndrome */
	      corr_bits = syn2bit (bad_syndrome);
	      
	      /* determine the bitmask and ram_num from corr_bits */
	      /* CMIC2 bits are in the high half of the x4 DRAMs */
	      if (corr_bits <= 139)
		{
		  /* single bit error: is it an odd or even bit? */
		  if (corr_bits % 2 == 0)
		    mem_error_entry[error_types].bits.bitmask = 0x1 << 2;
		  else
		    mem_error_entry[error_types].bits.bitmask = 0x2 << 2;
		  
		  mem_error_entry[error_types].bits.ram_num =
		    ((corr_bits / 2) % 70) + 1;
		}
	      else if (corr_bits != 300)
		{
		  /* double bit error */
		  mem_error_entry[error_types].bits.bitmask = 0x3 << 2;
		  
		  mem_error_entry[error_types].bits.ram_num = corr_bits - 139;
		}
	      else
		{
		  printf("Fatal error - corr. memory error not logged \n");
		  return(0);
		}
	      
	      mem_error_entry[error_types].bits.offset =
		compute_offset_field(errorlog.memory_log);
	    }

	  for (k = 1; k <= error_types; k++)
	    {
	      /* correctable error locations must be fully logged */
	      mem_error_entry[k].bits.err_typ = descriptor;
	      if (num_errors > 1)
		mem_error_entry[k].bits.multi = 1;
	      else
		mem_error_entry[k].bits.multi = 0;
	    }

	  break;

	case CMIC1_ERR:
	case CMIC2_ERR:
	case MEM_ADDR_ERR:
	case SYNC_ERROR_E:
	case SYNC_ERROR_O:

	  error_types++;

	  /* log the code type for other types of memory errors */
	  mem_error_entry[error_types].bits.err_typ = 0;

	  /* entry not made by diagnostics */
	  mem_error_entry[error_types].bits.multi = 0;

	  /* bitmask field is unused (must be 0x1) */
	  mem_error_entry[error_types].bits.bitmask = 0x1;

	  /* ram_num identifies	the error type for these */
	  mem_error_entry[error_types].bits.ram_num = descriptor;

	  /* offset field is unused (MBZ) */
	  mem_error_entry[error_types].bits.offset = 0;

	  break;

	case CA_PAR_ERR_E:
	case CA_PAR_ERR_O:

	  error_types++;

	  /* log the code type for other types of memory errors */
	  mem_error_entry[error_types].bits.err_typ = 0;

	  /* entry not made by diagnostics */
	  mem_error_entry[error_types].bits.multi = 0;

	  /* bitmask field is unused (must be 0x1) */
	  mem_error_entry[error_types].bits.bitmask = 0x1;

	  /* ram_num identifies	the error type for these */
	  mem_error_entry[error_types].bits.ram_num = descriptor;

	  /* determine the offset field bits from the mem board's error reg */
	  if (descriptor == CA_PAR_ERR_E)
	    {
	      /*
	       * check for cape0, cape2, and mcapel; these are in 
	       * bits 8, 9, and 3 of CSR0 respectively and they should
	       * go into bits 0, 2, and 12 of the the error log word
	       * respectively
	       */
	      mem_error_entry[error_types].bits.offset =
		(((errorlog.memory_log.elcm_merr & MERR_CAPE0)  >> 8) |
		 ((errorlog.memory_log.elcm_merr & MERR_CAPE2)  >> 7) |
		 ((errorlog.memory_log.elcm_merr & MERR_MCAPEL) << 9));
	    }
	  else
	    {
	      /*
	       * check for cape1, cape3, and mcapeh; these are in 
	       * bits 40, 41, and 35 of CSR0 respectively and they should
	       * go into bits 1, 3, and 13 of the the error log word
	       * respectively
	       */
	      mem_error_entry[error_types].bits.offset =
		(((errorlog.memory_log.elcm_merr & MERR_CAPE1)  >> 39) |
		 ((errorlog.memory_log.elcm_merr & MERR_CAPE3)  >> 38) |
		 ((errorlog.memory_log.elcm_merr & MERR_MCAPEH) >> 22));
	    }

	  break;

	case DATA_PAR_ERR_E:
	case DATA_PAR_ERR_O:

	  error_types++;

	  /* log the code type for other types of memory errors */
	  mem_error_entry[error_types].bits.err_typ = 0;

	  /* entry not made by diagnostics */
	  mem_error_entry[error_types].bits.multi = 0;

	  /* bitmask field is unused (must be 0x1) */
	  mem_error_entry[error_types].bits.bitmask = 0x1;

	  /* ram_num identifies	the error type for these */
	  mem_error_entry[error_types].bits.ram_num = descriptor;

	  /* determine the offset field bits from the mem board's error reg */
	  if (descriptor == DATA_PAR_ERR_E)
	    {
	      /*
	       * check for dpe0, dpe2, dpe4, dpe6, and mwdpel; these are in 
	       * bits 10, 11, 12, 13, and 5 of CSR0 respectively and they
	       * should go into bits 4, 6, 8, 10, and 14 of the the error
	       * log word respectively
	       */
	      mem_error_entry[error_types].bits.offset =
		(((errorlog.memory_log.elcm_merr & MERR_DPE0)   >> 6) |
		 ((errorlog.memory_log.elcm_merr & MERR_DPE2)   >> 5) |
		 ((errorlog.memory_log.elcm_merr & MERR_DPE4)   >> 4) |
		 ((errorlog.memory_log.elcm_merr & MERR_DPE6)   >> 3) |
		 ((errorlog.memory_log.elcm_merr & MERR_MWDPEL) << 9));
	    }
	  else
	    {
	      /*
	       * check for dpe1, dpe3, dpe5, dpe7, and mwdpeh; these are in 
	       * bits 42, 43, 44, 45, and 37 of CSR0 respectively and they
	       * should go into bits 5, 7, 9, 11, and 15 of the the error
	       * log word respectively
	       */
	      mem_error_entry[error_types].bits.offset =
		(((errorlog.memory_log.elcm_merr & MERR_DPE1)   >> 37) |
		 ((errorlog.memory_log.elcm_merr & MERR_DPE3)   >> 36) |
		 ((errorlog.memory_log.elcm_merr & MERR_DPE5)   >> 35) |
		 ((errorlog.memory_log.elcm_merr & MERR_DPE7)   >> 34) |
		 ((errorlog.memory_log.elcm_merr & MERR_MWDPEH) >> 22));
	    }

	  break;

	default:
	  printf ("Error: %d is an invalid error type for the Memory Module\n",
		  descriptor);
	  return (0);
	  break;
	}       /* end of switch (descriptor) */

      /*
       * for each uniquely loggable entry, see if the entry is already
       * in the EEPROM and only write it out when it's not already there
       */
      for (k = 1; k <= error_types; k++)
	{
	  already_there = FALSE;
	  full_desc.word = 1;
	  for (i = 0;
	       (i < MEM_CORRERR_FIELDS) && (!already_there) &&
	       (full_desc.word != 0);
	       i++)
	    {
	      read_data =
		read_iic_eeprom(device,
				mem_iic_eeprom.mem_sdd.bad_bit_desc[i], 4);
	      for (j = 0; j < 4; j++)
		full_desc.bytes[j] = *(read_data + j);
	      
	      if (mem_error_entry[k].bits.err_typ != 0)
		{
		  /* mask out the multi bit */
		  if ((mem_error_entry[k].word & 0xDFFFFFFF) ==
		      (full_desc.word & 0xDFFFFFFF))
		    already_there = TRUE;
		}
	      else
		{
		  /* check the whole word */
		  if (mem_error_entry[k].word == full_desc.word)
		    already_there = TRUE;
		}
	    }
	  if (full_desc.word == 0)
	    {
	      i--;
	      /* save the descriptor in this location */
	      write_iic_eeprom(device, mem_iic_eeprom.mem_sdd.bad_bit_desc[i],
			       &mem_error_entry[k].bytes[0], 4);
	      
	      /* and update the symloglength field */
	      symloglength = MEM_SYMLL_INIT + 4 * (i + 1);
	      write_iic_eeprom(device, mem_iic_eeprom.mem_sdd.sym_log_len,
			       &symloglength, 1);
	    }
	  else if (already_there)
	    {
	      if (mem_error_entry[k].bits.err_typ != 0)
		{
		  /* set the multi-bit */
		  full_desc.bytes[3] |= 0x20;
		  i--;
		  write_iic_eeprom(device,
				   mem_iic_eeprom.mem_sdd.bad_bit_desc[i] + 3,
				   &full_desc.bytes[3], 1);
		}
	    }
	  else
	    {
	      printf ("There are more memory errors than there is room for in the EEPROM\n");
	      return (0);
	    }      
	}
      break;
      
    default:
      printf ("Error: Invalid device type. Can't log to 0x%x.\n", device);
      break;
    }           /* end of switch (device) */
  
}


unsigned int compute_offset_field(memlog)
struct el_cobra_data_memory   memlog;
{
  unsigned int offset;
  /*
   * Determine the offset from the values in the mem error registers.
   * The bits that go into the offset will depend on the memory
   * configuration. For all cases physical address bits <6:5> will
   * go into offset <1:0>. Then physical address bit 13 will go
   * into offset 2 and this will continue (i.e. physical address bit
   * 13 + i will go into offset bit 2 + i) up to a configuration
   * dependent value. Above this value, the offset bits will be
   * filled with zeros up to and including offset bit 17. The
   * highest valid address value put in the offset field is given
   * below for every possible configuration.  REMEMBER, that the
   * physical address is shifted right 2 bits before it is stored in
   * the error register (CSR1).  For example, bit 14 of the address is
   * actually in bit 12 of CSR1.
   *
   *
   * DRAM size |   Banks   | Interleave |  Highest address bit to
   *           | on board  |            | offset field translation
   * ----------+-----------+------------+-------------------------
   *   1 Mbit  |     2     |   1-way    |  23 -> 12   (<17:13> = 0)
   *           |           |            |
   *   1 Mbit  |     2     |   2-way    |  24 -> 13   (<17:14> = 0)
   *           |           |            |
   *   1 Mbit  |     2     |   4-way    |  25 -> 14   (<17:15> = 0)
   *           |           |            |
   *   1 Mbit  |     4     |   1-way    |  24 -> 13   (<17:14> = 0)
   *           |           |            |
   *   1 Mbit  |     4     |   2-way    |  25 -> 14   (<17:15> = 0)
   *           |           |            |
   *   1 Mbit  |     4     |   4-way    |  26 -> 15   (<17:16> = 0)
   *           |           |            |
   *   4 Mbit  |     2     |   1-way    |  25 -> 14   (<17:15> = 0)
   *           |           |            |
   *   4 Mbit  |     2     |   2-way    |  26 -> 15   (<17:16> = 0)
   *           |           |            |
   *   4 Mbit  |     2     |   4-way    |  27 -> 16   (<17> = 0)
   *           |           |            |
   *   4 Mbit  |     4     |   1-way    |  26 -> 15   (<17:16> = 0)
   *           |           |            |
   *   4 Mbit  |     4     |   2-way    |  27 -> 16   (<17> = 0)
   *           |           |            |
   *   4 Mbit  |     4     |   4-way    |  28 -> 17
   *           |           |            |
   */
  if (((memlog.elcm_mconf & MCONF_SIZEL) >> MCONF_SIZEL_SHIFT) == 0)
    {
      /* 1 Mbit DRAMs */
      if ((memlog.elcm_mconf & MCONF_BANK) == 0)
	{
	  /* 2 banks */
	  if (((memlog.elcm_mconf & MCONF_ILVML) >> MCONF_ILVML_SHIFT) == 0)
	    {
	      /* no interleave (1-way) */
	      offset = (((memlog.elcm_mcmd1 & MCMD1_ADR6_5)
			 >> MCMD1_ADR6_5_SHIFT) |
			((memlog.elcm_mcmd1 & MCMD1_ADR23TO13)
			 >> ADR13_TO_OFFSET2_SHIFT));
	    }
	  else if (((memlog.elcm_mconf & MCONF_ILVML) >> MCONF_ILVML_SHIFT) ==
		   1)
	    {
	      /* 2-way interleave */
	      offset = (((memlog.elcm_mcmd1 & MCMD1_ADR6_5)
			 >> MCMD1_ADR6_5_SHIFT) |
			((memlog.elcm_mcmd1 & MCMD1_ADR24TO13)
			 >> ADR13_TO_OFFSET2_SHIFT));
	    }
	  else if (((memlog.elcm_mconf & MCONF_ILVML) >> MCONF_ILVML_SHIFT) ==
		   2)
	    {
	      /* 4-way interleave */
	      offset = (((memlog.elcm_mcmd1 & MCMD1_ADR6_5)
			 >> MCMD1_ADR6_5_SHIFT) |
			((memlog.elcm_mcmd1 & MCMD1_ADR25TO13)
			 >> ADR13_TO_OFFSET2_SHIFT));
	    }
	  else
	    {
	      printf("Error: Undefined interleave field\n");
	      return (0);
	    }
	}
      else
	{
	  /* 4 banks */
	  if (((memlog.elcm_mconf & MCONF_ILVML) >> MCONF_ILVML_SHIFT) == 0)
	    {
	      /* no interleave (1-way) */
	      offset = (((memlog.elcm_mcmd1 & MCMD1_ADR6_5)
			 >> MCMD1_ADR6_5_SHIFT) |
			((memlog.elcm_mcmd1 & MCMD1_ADR24TO13)
			 >> ADR13_TO_OFFSET2_SHIFT));
	    }
	  else if (((memlog.elcm_mconf & MCONF_ILVML) >> MCONF_ILVML_SHIFT) ==
		   1)
	    {
	      /* 2-way interleave */
	      offset = (((memlog.elcm_mcmd1 & MCMD1_ADR6_5)
			 >> MCMD1_ADR6_5_SHIFT) |
			((memlog.elcm_mcmd1 & MCMD1_ADR25TO13)
			 >> ADR13_TO_OFFSET2_SHIFT));
	    }
	  else if (((memlog.elcm_mconf & MCONF_ILVML) >> MCONF_ILVML_SHIFT) ==
		   2)
	    {
	      /* 4-way interleave */
	      offset = (((memlog.elcm_mcmd1 & MCMD1_ADR6_5)
			 >> MCMD1_ADR6_5_SHIFT) |
			((memlog.elcm_mcmd1 & MCMD1_ADR26TO13)
			 >> ADR13_TO_OFFSET2_SHIFT));
	    }
	  else
	    {
	      printf("Error: Undefined interleave field\n");
	      return (0);
	    }
	}
    }
  else if (((memlog.elcm_mconf & MCONF_SIZEL) >> MCONF_SIZEL_SHIFT) == 1)
    {
      /* 4 Mbit DRAMs */
      if ((memlog.elcm_mconf & MCONF_BANK) == 0)
	{
	  /* 2 banks */
	  if (((memlog.elcm_mconf & MCONF_ILVML) >> MCONF_ILVML_SHIFT) == 0)
	    {
	      /* no interleave (1-way) */
	      offset = (((memlog.elcm_mcmd1 & MCMD1_ADR6_5)
			 >> MCMD1_ADR6_5_SHIFT) |
			((memlog.elcm_mcmd1 & MCMD1_ADR25TO13)
			 >> ADR13_TO_OFFSET2_SHIFT));
	    }
	  else if (((memlog.elcm_mconf & MCONF_ILVML) >> MCONF_ILVML_SHIFT) ==
		   1)
	    {
	      /* 2-way interleave */
	      offset = (((memlog.elcm_mcmd1 & MCMD1_ADR6_5)
			 >> MCMD1_ADR6_5_SHIFT) |
			((memlog.elcm_mcmd1 & MCMD1_ADR26TO13)
			 >> ADR13_TO_OFFSET2_SHIFT));
	    }
	  else if (((memlog.elcm_mconf & MCONF_ILVML) >> MCONF_ILVML_SHIFT) ==
		   2)
	    {
	      /* 4-way interleave */
	      offset = (((memlog.elcm_mcmd1 & MCMD1_ADR6_5)
			 >> MCMD1_ADR6_5_SHIFT) |
			((memlog.elcm_mcmd1 & MCMD1_ADR27TO13)
			 >> ADR13_TO_OFFSET2_SHIFT));
	    }
	  else
	    {
	      printf("Error: Undefined interleave field\n");
	      return (0);
	    }
	}
      else
	{
	  /* 4 banks */
	  if (((memlog.elcm_mconf & MCONF_ILVML) >> MCONF_ILVML_SHIFT) == 0)
	    {
	      /* no interleave (1-way) */
	      offset = (((memlog.elcm_mcmd1 & MCMD1_ADR6_5)
			 >> MCMD1_ADR6_5_SHIFT) |
			((memlog.elcm_mcmd1 & MCMD1_ADR26TO13)
			 >> ADR13_TO_OFFSET2_SHIFT));
	    }
	  else if (((memlog.elcm_mconf & MCONF_ILVML) >> MCONF_ILVML_SHIFT) ==
		   1)
	    {
	      /* 2-way interleave */
	      offset = (((memlog.elcm_mcmd1 & MCMD1_ADR6_5)
			 >> MCMD1_ADR6_5_SHIFT) |
			((memlog.elcm_mcmd1 & MCMD1_ADR27TO13)
			 >> ADR13_TO_OFFSET2_SHIFT));
	    }
	  else if (((memlog.elcm_mconf & MCONF_ILVML) >> MCONF_ILVML_SHIFT) ==
		   2)
	    {
	      /* 4-way interleave */
	      offset = (((memlog.elcm_mcmd1 & MCMD1_ADR6_5)
			 >> MCMD1_ADR6_5_SHIFT) |
			((memlog.elcm_mcmd1 & MCMD1_ADR28TO13)
			 >> ADR13_TO_OFFSET2_SHIFT));
	    }
	  else
	    {
	      printf("Error: Undefined interleave field\n");
	      return (0);
	    }
	}
    }
  else
    {
      printf("Error: Invalid DRAM size\n");
      return (0);
    }
  return (offset);
}


unsigned int syn2bit (syn)
unsigned int syn;
{
  switch (syn)
    {
    case BAD_BIT_0:
      return (0);
    case BAD_BIT_1:
      return (1);
    case BAD_BIT_2:
      return (2);
    case BAD_BIT_3:
      return (3);
    case BAD_BIT_4:
      return (4);
    case BAD_BIT_5:
      return (5);
    case BAD_BIT_6:
      return (6);
    case BAD_BIT_7:
      return (7);
    case BAD_BIT_8:
      return (8);
    case BAD_BIT_9:
      return (9);
    case BAD_BIT_10:
      return (10);
    case BAD_BIT_11:
      return (11);
    case BAD_BIT_12:
      return (12);
    case BAD_BIT_13:
      return (13);
    case BAD_BIT_14:
      return (14);
    case BAD_BIT_15:
      return (15);
    case BAD_BIT_16:
      return (16);
    case BAD_BIT_17:
      return (17);
    case BAD_BIT_18:
      return (18);
    case BAD_BIT_19:
      return (19);
    case BAD_BIT_20:
      return (20);
    case BAD_BIT_21:
      return (21);
    case BAD_BIT_22:
      return (22);
    case BAD_BIT_23:
      return (23);
    case BAD_BIT_24:
      return (24);
    case BAD_BIT_25:
      return (25);
    case BAD_BIT_26:
      return (26);
    case BAD_BIT_27:
      return (27);
    case BAD_BIT_28:
      return (28);
    case BAD_BIT_29:
      return (29);
    case BAD_BIT_30:
      return (30);
    case BAD_BIT_31:
      return (31);
    case BAD_BIT_32:
      return (32);
    case BAD_BIT_33:
      return (33);
    case BAD_BIT_34:
      return (34);
    case BAD_BIT_35:
      return (35);
    case BAD_BIT_36:
      return (36);
    case BAD_BIT_37:
      return (37);
    case BAD_BIT_38:
      return (38);
    case BAD_BIT_39:
      return (39);
    case BAD_BIT_40:
      return (40);
    case BAD_BIT_41:
      return (41);
    case BAD_BIT_42:
      return (42);
    case BAD_BIT_43:
      return (43);
    case BAD_BIT_44:
      return (44);
    case BAD_BIT_45:
      return (45);
    case BAD_BIT_46:
      return (46);
    case BAD_BIT_47:
      return (47);
    case BAD_BIT_48:
      return (48);
    case BAD_BIT_49:
      return (49);
    case BAD_BIT_50:
      return (50);
    case BAD_BIT_51:
      return (51);
    case BAD_BIT_52:
      return (52);
    case BAD_BIT_53:
      return (53);
    case BAD_BIT_54:
      return (54);
    case BAD_BIT_55:
      return (55);
    case BAD_BIT_56:
      return (56);
    case BAD_BIT_57:
      return (57);
    case BAD_BIT_58:
      return (58);
    case BAD_BIT_59:
      return (59);
    case BAD_BIT_60:
      return (60);
    case BAD_BIT_61:
      return (61);
    case BAD_BIT_62:
      return (62);
    case BAD_BIT_63:
      return (63);
    case BAD_BIT_64:
      return (64);
    case BAD_BIT_65:
      return (65);
    case BAD_BIT_66:
      return (66);
    case BAD_BIT_67:
      return (67);
    case BAD_BIT_68:
      return (68);
    case BAD_BIT_69:
      return (69);
    case BAD_BIT_70:
      return (70);
    case BAD_BIT_71:
      return (71);
    case BAD_BIT_72:
      return (72);
    case BAD_BIT_73:
      return (73);
    case BAD_BIT_74:
      return (74);
    case BAD_BIT_75:
      return (75);
    case BAD_BIT_76:
      return (76);
    case BAD_BIT_77:
      return (77);
    case BAD_BIT_78:
      return (78);
    case BAD_BIT_79:
      return (79);
    case BAD_BIT_80:
      return (80);
    case BAD_BIT_81:
      return (81);
    case BAD_BIT_82:
      return (82);
    case BAD_BIT_83:
      return (83);
    case BAD_BIT_84:
      return (84);
    case BAD_BIT_85:
      return (85);
    case BAD_BIT_86:
      return (86);
    case BAD_BIT_87:
      return (87);
    case BAD_BIT_88:
      return (88);
    case BAD_BIT_89:
      return (89);
    case BAD_BIT_90:
      return (90);
    case BAD_BIT_91:
      return (91);
    case BAD_BIT_92:
      return (92);
    case BAD_BIT_93:
      return (93);
    case BAD_BIT_94:
      return (94);
    case BAD_BIT_95:
      return (95);
    case BAD_BIT_96:
      return (96);
    case BAD_BIT_97:
      return (97);
    case BAD_BIT_98:
      return (98);
    case BAD_BIT_99:
      return (99);
    case BAD_BIT_100:
      return (100);
    case BAD_BIT_101:
      return (101);
    case BAD_BIT_102:
      return (102);
    case BAD_BIT_103:
      return (103);
    case BAD_BIT_104:
      return (104);
    case BAD_BIT_105:
      return (105);
    case BAD_BIT_106:
      return (106);
    case BAD_BIT_107:
      return (107);
    case BAD_BIT_108:
      return (108);
    case BAD_BIT_109:
      return (109);
    case BAD_BIT_110:
      return (110);
    case BAD_BIT_111:
      return (111);
    case BAD_BIT_112:
      return (112);
    case BAD_BIT_113:
      return (113);
    case BAD_BIT_114:
      return (114);
    case BAD_BIT_115:
      return (115);
    case BAD_BIT_116:
      return (116);
    case BAD_BIT_117:
      return (117);
    case BAD_BIT_118:
      return (118);
    case BAD_BIT_119:
      return (119);
    case BAD_BIT_120:
      return (120);
    case BAD_BIT_121:
      return (121);
    case BAD_BIT_122:
      return (122);
    case BAD_BIT_123:
      return (123);
    case BAD_BIT_124:
      return (124);
    case BAD_BIT_125:
      return (125);
    case BAD_BIT_126:
      return (126);
    case BAD_BIT_127:
      return (127);
    case BAD_BIT_C0:
      return (128);
    case BAD_BIT_C1:
      return (129);
    case BAD_BIT_C2:
      return (130);
    case BAD_BIT_C3:
      return (131);
    case BAD_BIT_C4:
      return (132);
    case BAD_BIT_C5:
      return (133);
    case BAD_BIT_C6:
      return (134);
    case BAD_BIT_C7:
      return (135);
    case BAD_BIT_C8:
      return (136);
    case BAD_BIT_C9:
      return (137);
    case BAD_BIT_C10:
      return (138);
    case BAD_BIT_C11:
      return (139);
    case BAD_BITS_0_1:
      return (140);
    case BAD_BITS_2_3:
      return (141);
    case BAD_BITS_4_5:
      return (142);
    case BAD_BITS_6_7:
      return (143);
    case BAD_BITS_8_9:
      return (144);
    case BAD_BITS_10_11:
      return (145);
    case BAD_BITS_12_13:
      return (146);
    case BAD_BITS_14_15:
      return (147);
    case BAD_BITS_16_17:
      return (148);
    case BAD_BITS_18_19:
      return (149);
    case BAD_BITS_20_21:
      return (150);
    case BAD_BITS_22_23:
      return (151);
    case BAD_BITS_24_25:
      return (152);
    case BAD_BITS_26_27:
      return (153);
    case BAD_BITS_28_29:
      return (154);
    case BAD_BITS_30_31:
      return (155);
    case BAD_BITS_32_33:
      return (156);
    case BAD_BITS_34_35:
      return (157);
    case BAD_BITS_36_37:
      return (158);
    case BAD_BITS_38_39:
      return (159);
    case BAD_BITS_40_41:
      return (160);
    case BAD_BITS_42_43:
      return (161);
    case BAD_BITS_44_45:
      return (162);
    case BAD_BITS_46_47:
      return (163);
    case BAD_BITS_48_49:
      return (164);
    case BAD_BITS_50_51:
      return (165);
    case BAD_BITS_52_53:
      return (166);
    case BAD_BITS_54_55:
      return (167);
    case BAD_BITS_56_57:
      return (168);
    case BAD_BITS_58_59:
      return (169);
    case BAD_BITS_60_61:
      return (170);
    case BAD_BITS_62_63:
      return (171);
    case BAD_BITS_64_65:
      return (172);
    case BAD_BITS_66_67:
      return (173);
    case BAD_BITS_68_69:
      return (174);
    case BAD_BITS_70_71:
      return (175);
    case BAD_BITS_72_73:
      return (176);
    case BAD_BITS_74_75:
      return (177);
    case BAD_BITS_76_77:
      return (178);
    case BAD_BITS_78_79:
      return (179);
    case BAD_BITS_80_81:
      return (180);
    case BAD_BITS_82_83:
      return (181);
    case BAD_BITS_84_85:
      return (182);
    case BAD_BITS_86_87:
      return (183);
    case BAD_BITS_88_89:
      return (184);
    case BAD_BITS_90_91:
      return (185);
    case BAD_BITS_92_93:
      return (186);
    case BAD_BITS_94_95:
      return (187);
    case BAD_BITS_96_97:
      return (188);
    case BAD_BITS_98_99:
      return (189);
    case BAD_BITS_100_101:
      return (190);
    case BAD_BITS_102_103:
      return (191);
    case BAD_BITS_104_105:
      return (192);
    case BAD_BITS_106_107:
      return (193);
    case BAD_BITS_108_109:
      return (194);
    case BAD_BITS_110_111:
      return (195);
    case BAD_BITS_112_113:
      return (196);
    case BAD_BITS_114_115:
      return (197);
    case BAD_BITS_116_117:
      return (198);
    case BAD_BITS_118_119:
      return (199);
    case BAD_BITS_120_121:
      return (200);
    case BAD_BITS_122_123:
      return (201);
    case BAD_BITS_124_125:
      return (202);
    case BAD_BITS_126_127:
      return (203);
    case BAD_BITS_C0_C1:
      return (204);
    case BAD_BITS_C2_C3:
      return (205);
    case BAD_BITS_C4_C5:
      return (206);
    case BAD_BITS_C6_C7:
      return (207);
    case BAD_BITS_C8_C9:
      return (208);
    case BAD_BITS_C10_C11:
      return (209);
    default:
      printf("The syndrome 0x%x does not correspond to a correctable error\n",
	     syn);
      return (300);
    }
}
