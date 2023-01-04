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
static char *rcsid = "@(#)$RCSfile: eisa_cnfg.c,v $ $Revision: 1.1.4.6 $ (DEC) $Date: 1993/11/15 23:11:25 $";
#endif

/***************************************************************************/
/*                                                                         */
/* MODULE NAME: eisa_cnfg.c						   */
/* 									   */ 
/* LOCATION:	.../src/kernel/io/dec/eisa				   */
/* 									   */ 
/* DESCRIPTION:								   */
/* 									   */ 
/* FUNCTIONS:								   */
/* 									   */ 
/*	e_read_config_int	Read an int from config.		   */
/*	e_read_config_bytes	Read specified number of bytes from config.*/
/*	e_get_config_bytes	Copy a specified number of bytes from 	   */
/*			  	config buffer to specified destination 	   */
/*				buffer.					   */
/*	uncompress_ecu_data	Uncompresses zero compressed ecu data.	   */
/*	load_new_entry		Load data into a new eisa_config_info list */
/*				entry.					   */
/*	load_existing_entry     Load data into an existing eisa_config_info*/
/*				list entry.				   */
/*	have_room		Determine if an existing eisa_config_info  */
/*				list entry has space for more data.	   */
/*	find_einfo_entry	Searches eisa_config_info list for an entry*/
/*				whose type string matches the passed in    */
/*				string.					   */
/* 	e_build_eisa_info   	Build eisa info list for a slot.	   */
/*	e_get_config		Get ECU generated EISA configuration.	   */
/* 									   */ 
/***************************************************************************/

#include	<sys/types.h>
#include	<sys/param.h>		/* Required for KSEG support */
#include	<machine/rpb.h>		/* Required for HWRPB definition. */
#include	<io/dec/eisa/eisa_cnfg.h>
#include	<io/dec/eisa/eisa.h>

/* Following two variables are set in platform specific init code. */
int		eisa_expansion_slots;

/* Config base address and config stride are set in eisaconfl1. */
extern  vm_offset_t	eisa_config_base;
extern	uint_t		config_stride;

/***************************************************************************/
/*                                                                         */
/* e_read_config_int  -  Read an integer (4 bytes) from config.		   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	int   e_read_config_int (cnfg_addr)		                   */
/*                                                                         */
/*	vm_offset_t	cnfg_addr;	                                   */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	cnfg_addr	Physical address of config area to read from.	   */
/*                                                                         */
/***************************************************************************/

int
e_read_config_int (cnfg_addr)

vm_offset_t	cnfg_addr;

{  /* Begin e_read_config_int */

   int		config_int;
   int		data;
   int		i;
   vm_offset_t	addr;
   
   addr = cnfg_addr;
   config_int = 0;
   
   for (i=0; i<4; i++)
      {
      data = (*(char *)PHYS_TO_KSEG(addr) & 0xff) << (i*8);
      config_int |= data;
      addr += FLASH_STRIDE;
      }
   return (config_int);
   
}  /* End e_read_config_int */



/***************************************************************************/
/*                                                                         */
/* e_read_config_bytes  -  Read a specified number of bytes from config.   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	int   e_read_config_bytes (cnfg_addr, buf_addr, count)             */
/*                                                                         */
/*	vm_offset_t	cnfg_addr;	                                   */
/*	char	       *buf_addr;					   */
/*	int		count;						   */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	cnfg_addr	Physical address of config area to read from.	   */
/*                                                                         */
/*	buf_addr	Address of buffer to place config bytes in.	   */
/*                                                                         */
/*	count		Number of bytes to read.			   */
/*                                                                         */
/***************************************************************************/

int
e_read_config_bytes (cnfg_addr, buf_addr, count)

vm_offset_t	cnfg_addr;
char	       *buf_addr;
int		count;


{  /* Begin e_read_config_bytes */

   char	       *buff_p;
   int		i;
   vm_offset_t	addr;
   
   addr = cnfg_addr;
   buff_p = buf_addr;
   
   for (i=0; i<count; i++)
      {
      *buff_p = *(char *)PHYS_TO_KSEG(addr);
      addr += FLASH_STRIDE;
      buff_p++;
      }
   return (i);
   
}  /* End e_read_config_bytes */


/***************************************************************************/
/*                                                                         */
/* e_get_config_bytes  -  Copy a specified number of bytes from config     */
/*			  buffer to specified destination buffer.	   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	int   e_get_config_bytes (uchar_t *dest_p, uchar_t *cnfg_buf_p,    */
/*				  int count)			           */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	dest_p		Pointer to buffer to place results in.		   */
/*                                                                         */
/*	cnfg_buf_p	Pointer to buffer containing config data.	   */
/*                                                                         */
/*	count		Number of bytes to copy.			   */
/*                                                                         */
/* RETURN								   */
/*	Returns number of bytes actually copied.			   */
/***************************************************************************/

int
e_get_config_bytes (uchar_t  *dest_p, 
		    uchar_t  *cnfg_buf_p, 
		    int       count)


{  /* Begin e_get_config_bytes */

   uchar_t      *from_p = cnfg_buf_p;
   uchar_t      *to_p = dest_p;
   int		i;
   
   for (i=0; i<count; i++)
      *to_p++ = *from_p++;
   
   return (i);
   
}  /* End e_get_config_bytes */

/***************************************************************************/
/*                                                                         */
/* uncompress_ecu_data  -  Uncompresses zero compressed ecu data.	   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	int   e_read_config_bytes (cnfg_addr, buf_addr, count)             */
/*                                                                         */
/*	vm_offset_t	cnfg_addr;	                                   */
/*	char	       *buf_addr;					   */
/*	int		count;						   */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	cnfg_addr	Physical address of config area to read from.	   */
/*                                                                         */
/*	buf_addr	Address of buffer to place config bytes in.	   */
/*                                                                         */
/*	count		Number of bytes to read.			   */
/*                                                                         */
/* RETURN VALUE								   */
/*	Returns the number of bytes consumed from the compressed buffer.   */
/*									   */
/***************************************************************************/

uint_t
uncompress_ecu_data (uchar_t *  comp_buf_p, 
		     uchar_t *  ucomp_buf_p,
		     uint_t  byte_count)

{  /* Begin uncompress_ecu_data */

   uint_t	c_byte_count = 0;
   uint_t	u_byte_count = 0;
   uint_t	zeros = 0;
   uchar_t	outchar;
   uchar_t     *cp;
   uchar_t     *up;
   
   cp = comp_buf_p;
   up = ucomp_buf_p;

   while (u_byte_count++ < byte_count)
      {  /* While we have data to compress. */
      if (zeros)
	 zeros--;
      else  if (*cp)
	 outchar = *cp++;
      else
	 {
	 outchar = *cp++;
	 zeros = (uint_t)*cp++ - 1;
	 }
      
      *up++ = outchar;
      }  /* End while we have data to compress. */

   c_byte_count = cp - comp_buf_p;
   
   return (c_byte_count);
   

}  /* End uncompress_ecu_data */


/***************************************************************************/
/*                                                                         */
/* load_new_entry   -  Load data into a new eisa_config_info list entry.   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	void   load_new_entry (struct  eisa_config_info *entry_p,	   */
/*				 struct  cnfg_blk   *config_p)		   */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	entry_p		Pointer to entry to load data into.		   */
/*                                                                         */
/*	config_p	Pointer to configuration block containing data to  */
/*			load.						   */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

void
load_new_entry (struct  eisa_config_info *entry_p,
		struct  cnfg_blk   *config_p)

{  /* Begin load_new_entry. */

   struct  bus_mem	*mem_p;
   
   int	i;
   ulong_t	mem_desc;
   
   entry_p->next_p = NULL;
   /* Load type string. */
   strcpy (entry_p->type, config_p->type_string);
   /* Load bus memory configuration. */
   mem_p = &entry_p->phys_mem[0];
   i = 0;
   mem_desc = config_p->mem_cnfg[i++];
   while (i < NUMB_OF_MEM_ENTRIES && mem_desc != EISA_NO_CONFIG)
      {
      mem_p->isram = mem_desc & 0x1L;
      mem_p->unit_size = (mem_desc >> 8) & 0x3L;
      mem_p->decode = (mem_desc >> 10) & 0x3L;
      mem_p->start_addr = ((mem_desc >> 16) & 0xffffffL) << 8;
      mem_desc = (mem_desc >> 40) & 0xffffL;
      if (mem_desc == 0)
	 mem_p->size = 0x1L << 26;
      else
	 mem_p->size = mem_desc << 10;
      mem_p++;
      mem_desc = config_p->mem_cnfg[i++];
      }
   mem_p->size = 0;
   /* Load IRQ assingments. */
   for (i=0; i<NUMB_OF_IRQ_ENTRIES; i++)
      entry_p->irq[i].intr_cnfg = config_p->irq_cnfg[i];
   /* Load DMA assingments. */
   for (i=0; i<NUMB_OF_DMA_ENTRIES; i++)
      entry_p->dma_chan[i].dma_cnfg = config_p->dma_cnfg[i];
   /* Load IO port assingments. */
   for (i=0; i<NUMB_OF_PORT_ENTRIES; i++)
      entry_p->ports[i] = config_p->port_cnfg[i];
   /* Load initialization data. */
   for (i=0; i<NUMB_OF_INIT_BYTES; i++)
      entry_p->init_data[i] = config_p->init_data[i];


}  /* End load_new_entry. */

/***************************************************************************/
/*                                                                         */
/* load_existing_entry   -  Load data into an existing eisa_config_info    */
/*			    list entry.					   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	void   load_existing_entry (struct  eisa_config_info *entry_p,	   */
/*				    struct  cnfg_blk   *config_p)	   */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	entry_p		Pointer to entry to load data into.		   */
/*                                                                         */
/*	config_p	Pointer to configuration block containing data to  */
/*			load.						   */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

void
load_existing_entry (struct  eisa_config_info *entry_p,
		struct  cnfg_blk   *config_p)

{  /* Begin load_existing_entry. */

   struct  bus_mem	*mem_p;
   
   int	i, j;
   ulong_t	mem_desc;
   

   /* Load bus memory configuration. */
   for (i=0; i<NUMB_OF_MEM_ENTRIES && entry_p->phys_mem[i].size > 0; i++)
      ;
   for (j=0, mem_desc = config_p->mem_cnfg[j], mem_p =
	&entry_p->phys_mem[i];
	i < NUMB_OF_MEM_ENTRIES && mem_desc != EISA_NO_CONFIG;
	i++, mem_desc = config_p->mem_cnfg[++j], mem_p++)
      {
      mem_p->isram = mem_desc & 0x1L;
      mem_p->unit_size = (mem_desc >> 8) & 0x3L;
      mem_p->decode = (mem_desc >> 10) & 0x3L;
      mem_p->start_addr = ((mem_desc >> 16) & 0xffffffL) << 8;
      mem_desc = (mem_desc >> 40) & 0xffffL;
      if (mem_desc == 0)
	 mem_p->size = 0x1L << 26;
      else
	 mem_p->size = mem_desc << 10;
      }
   if (i < NUMB_OF_MEM_ENTRIES)
      mem_p->size = 0;

   /* Load IRQ assingments. */
   for (i=0;
	i<NUMB_OF_IRQ_ENTRIES && entry_p->irq[i].intr_cnfg != EISA_NO_CONFIG;
	i++) 
      ;
   for (j=0; i<NUMB_OF_IRQ_ENTRIES; j++, i++)
      entry_p->irq[i].intr_cnfg = config_p->irq_cnfg[j];

   /* Load DMA assingments. */
   for (i=0;
	i<NUMB_OF_DMA_ENTRIES && entry_p->dma_chan[i].dma_cnfg !=
	EISA_NO_CONFIG; 
	i++) 
      ;
   for (j=0; i<NUMB_OF_DMA_ENTRIES; j++, i++)
      entry_p->dma_chan[i].dma_cnfg = config_p->dma_cnfg[j];

   /* Load IO port assingments. */
   for (i=0;
	i<NUMB_OF_PORT_ENTRIES && entry_p->ports[i] != EISA_NO_CONFIG; 
	i++) 
      ;
   for (j=0; i<NUMB_OF_PORT_ENTRIES; j++, i++)
      entry_p->ports[i] = config_p->port_cnfg[j];

   /* Load initialization data. */
   for (i=0;
	i<NUMB_OF_INIT_BYTES && entry_p->init_data[i] != EISA_NO_CONFIG; 
	i++) 
      ;
   for (j=0; i<NUMB_OF_INIT_BYTES; j++, i++)
      entry_p->init_data[i] = config_p->init_data[j];


}  /* End load_existing_entry. */

/***************************************************************************/
/*                                                                         */
/*	have_room		Determine if an existing eisa_config_info  */
/*				list entry has space for more data.	   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	int  have_room (struct  eisa_config_info *entry_p,		   */
/*			struct  cnfg_blk   *config_p)			   */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	entry_p		Pointer to entry to check for room.		   */
/*                                                                         */
/*	config_p	Pointer to configuration block containing data to  */
/*			load.						   */
/*									   */
/* RETURN_VALUE								   */
/*	If current entry has room for this config block's data have_room   */
/*	returns a non-zero value. If insufficient room exists a zero is    */
/*	returned.							   */
/*									   */
/* DESCRIPTION								   */
/*	For initialization data, port assingment data, memory assingment   */
/*	data, IRQ assingment data and DMA assingment data have_room 	   */
/*	determines if the entry pointed to by entry_p has sufficient enpty */
/*	space to hold the data in the configuration data block pointed to  */
/*	by config_p. If any one of the data types have insufficient space  */
/*	have_room immediately returns with a value of 0. If all data types */
/*	have sufficient space then have_room returns with a non-zero value.*/
/***************************************************************************/

int
have_room (struct  eisa_config_info *entry_p, struct  cnfg_blk   *config_p)

{  /* Begin have_room. */

   int	i, j;

   /*********/
   /* BEGIN */
   /*********/

   /*-----------------------------------------------------------------------*/
   /* Data types are processed in the following order to maxmize the chance */
   /* of finding filled blocks early on in the test process.		    */
   /*-----------------------------------------------------------------------*/
   /* Initialization Data */
   /*---------------------*/
   for (i=0;
	i<NUMB_OF_INIT_BYTES && entry_p->init_data[i] != EISA_NO_CONFIG; 
	i++) 
      ;
   if (i == NUMB_OF_INIT_BYTES)
      return (FALSE);
   for (j=0;
	j<NUMB_OF_INIT_BYTES && config_p->init_data[j] != EISA_NO_CONFIG; 
	j++) 
      ;
   if ((i + j) >= NUMB_OF_INIT_BYTES)
      return (FALSE);

   /*----------------------*/
   /* Port Assingment Data */
   /*----------------------*/
   for (i=0;
	i<NUMB_OF_PORT_ENTRIES && entry_p->ports[i] != EISA_NO_CONFIG; 
	i++) 
      ;
   if (i == NUMB_OF_PORT_ENTRIES)
      return (FALSE);
   for (j=0;
	j<NUMB_OF_PORT_ENTRIES && config_p->port_cnfg[j] != EISA_NO_CONFIG; 
	j++) 
      ;
   if ((i + j) >= NUMB_OF_PORT_ENTRIES)
      return (FALSE);

   /*------------------------*/
   /* Memory Assingment Data */
   /*------------------------*/
   for (i=0; i<NUMB_OF_MEM_ENTRIES && entry_p->phys_mem[i].size > 0; i++)
      ;
   if (i == NUMB_OF_MEM_ENTRIES)
      return (FALSE);
   for (j=0;
	j<NUMB_OF_MEM_ENTRIES && config_p->mem_cnfg[j]!= EISA_NO_CONFIG;
	j++)
      ;
   if ((i + j) >= NUMB_OF_MEM_ENTRIES)
      return (FALSE);

   /*---------------------*/
   /* IRQ Assingment Data */
   /*---------------------*/
   for (i=0;
	i<NUMB_OF_IRQ_ENTRIES && entry_p->irq[i].intr_cnfg != EISA_NO_CONFIG;
	i++) 
      ;
   if (i == NUMB_OF_IRQ_ENTRIES)
      return (FALSE);
   for (j=0;
	j<NUMB_OF_IRQ_ENTRIES && config_p->irq_cnfg[j] != EISA_NO_CONFIG;
	j++) 
      ;
   if ((i + j) >= NUMB_OF_IRQ_ENTRIES)
      return (FALSE);

   /*---------------------*/
   /* DMA Assingment Data */
   /*---------------------*/
   for (i=0;
	i<NUMB_OF_DMA_ENTRIES && entry_p->dma_chan[i].dma_cnfg !=
	EISA_NO_CONFIG; 
	i++) 
      ;
   if (i == NUMB_OF_DMA_ENTRIES)
      return (FALSE);
   for (j=0;
	j<NUMB_OF_DMA_ENTRIES && config_p->dma_cnfg[j] != EISA_NO_CONFIG; 
	j++) 
      ;
   if ((i + j) >= NUMB_OF_DMA_ENTRIES)
      return (FALSE);

   
   return (TRUE);
   

}  /* End have_room. */



/***************************************************************************/
/*                                                                         */
/*  find_einfo_entry  -  Searches eisa_config_info list for an entry whose */
/*			 type string matches the passed in string.	   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	int   find_einfo_entry (uchar_t  *type_p,			   */
/*				struct  eisa_config_info  *head_p,	   */
/*				struct  eisa_config_info  **entry_p)       */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	type_p		Pointer to type string to search for.		   */
/*									   */
/*	head_p		Pointer to beginning of list to search in.	   */
/*                                                                         */
/*	entry_p		Pointer to location to store pointer to found 	   */
/*			entry.						   */
/*                                                                         */
/* RETURNS								   */
/*	If a match is found find_einfo_entry returns a value of zero and   */
/*	*entry_p contains a pointer to the matching entry. If no match is  */
/*	found find_einfo_entry return -1 and *entry_p contains a pointer   */
/*	to the last element in the list.				   */
/*                                                                         */
/***************************************************************************/

int
find_einfo_entry (uchar_t  *type_p,
		  struct  eisa_config_info  *head_p,
		  struct  eisa_config_info  **entry_p)

{  /* Begin find_einfo_entry. */

   int	found = -1;	/* Assume not found. */
   int	stop;
   
   struct  eisa_config_info  *cur_p = head_p;
   struct  eisa_config_info  *next_p;

#ifdef ECU_DEBUG
   printf ("String to find = %s \n", type_p);
#endif /* ECU_DEBUG */
   while (found != 0 && cur_p)
      {
      if (strcmp(type_p, cur_p->type) == 0)
	 {  /* We found a match. */
#ifdef ECU_DEBUG
	 printf (" Match \n");
#endif /* ECU_DEBUG */
	 /*------------------------------------------------------*/
	 /* See if next element also matches. If so move to it.  */
	 /* Keep walking the list until we find the last element */
	 /* that matches and return a pointer to it.		 */
	 /*------------------------------------------------------*/
	 stop = FALSE;
	 while (!stop)
	    {
	    next_p = cur_p->next_p;
	    if (next_p && strcmp(type_p, next_p->type) == 0)
	       cur_p = next_p;
	    else
	       stop = TRUE;
	    }
	 
	 found = 0;
	 *entry_p = cur_p;
	 }  /* End we found a match. */
      else
	 {  /* No match found. */
	 if (!cur_p->next_p)
	    *entry_p = cur_p;	/* At end of list. */
#ifdef ECU_DEBUG
	 printf (" No Match\n");
#endif /* ECU_DEBUG */
	 cur_p = cur_p->next_p;
	 }  /* End no match found. */
      }
   
#ifdef ECU_DEBUG
   printf ("Returning with found = %d \n", found);
#endif /* ECU_DEBUG */
   
   return (found);
   
}  /* End find_einfo_entry. */



/***************************************************************************/
/*                                                                         */
/* e_build_eisa_info   -  Build eisa info list for a slot.		   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	int   e_build_eisa_info (struct  cnfg_blk   *config_p,		   */
/*				 struct  eisa_config_info **info_p)        */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	config_p	Pointer to list of configuration blocks for option */
/*			located in slot of interest.			   */
/*                                                                         */
/*	info_p		Pointer to location to store pointer to resultant  */
/*			eisa info list.					   */
/*                                                                         */
/* RETURNS								   */
/*	Returns number of list entries.					   */
/*                                                                         */
/***************************************************************************/

int
e_build_eisa_info (struct  cnfg_blk   *config_p,
		   struct  eisa_config_info **info_p)


{  /* Begin e_build_eisa_info */

   struct  eisa_config_info	*einfo_p;
   struct  eisa_config_info	*cur_p;
   struct  cnfg_blk	*cur_conf_p = config_p;
   
   int	entries = 0;
   int	i;
   
   /*------------------------------------------------------------------*/
   /* Allocate space for the first entry. We always have at least one. */
   /* Then fill it in.						       */
   /*------------------------------------------------------------------*/
   einfo_p = *info_p = (struct eisa_config_info *)
                       kalloc (sizeof (struct eisa_config_info));
   entries++;
   load_new_entry (einfo_p, cur_conf_p);
   
   /*----------------------------------------------------------------------*/
   /* Now see if there are more config blocks and load them into the list. */
   /*----------------------------------------------------------------------*/
   cur_conf_p = cur_conf_p->next_blk;
   while (cur_conf_p)
      { /* Load function blocks into the eisa config info list. */
      if ((i = find_einfo_entry (cur_conf_p->type_string, *info_p,
				 &einfo_p)) != 0)
	 {  /* No match found in list so allocate a new element and load */
	    /* it. einfo_p points to the last entry in the list.	 */
	 cur_p = einfo_p;
	 einfo_p = (struct eisa_config_info *)
	                     kalloc (sizeof (struct eisa_config_info));
	 entries++;
	 cur_p->next_p = einfo_p;
	 load_new_entry (einfo_p, cur_conf_p);
	 } /* End no match found. */
      else
	 { /*---------------------------------------------------------*/
	   /* We have a match so add data from this block to existing */
	   /* entry if there is room. Otherwise allocate a new list   */
	   /* entry and load data into it.			      */
	   /*---------------------------------------------------------*/
	 if (have_room(einfo_p, cur_conf_p))
	    load_existing_entry (einfo_p, cur_conf_p);
	 else
	    { /* Allocate new entry then load it. */
	    cur_p = einfo_p;
	    einfo_p = (struct eisa_config_info *)
	               kalloc (sizeof (struct eisa_config_info));
	    entries++;
	    einfo_p->next_p = cur_p->next_p;
	    cur_p->next_p = einfo_p;
	    load_new_entry (einfo_p, cur_conf_p);
	    } /* End allocate new entry. */
	 } /* End have a match. */
      
      cur_conf_p = cur_conf_p->next_blk;
      } /* End load function blocks. */
   
   return (entries);
   
}  /* End e_build_eisa_info */


/***************************************************************************/
/*                                                                         */
/* e_get_config  - Get ECU generated EISA configuration. 		   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	int   e_get_config (struct  cnfg_hdr  *cnfg_hdr_p,                 */
/*                          int	  *slots_p)                                */
/*						                           */
/*                                                                         */
/* PARAMETERS								   */
/*	cnfg_hdr_p	Pointer to array of configuration header   	   */
/*			structures to be filled in by e_get_config.	   */
/*	slots_p		Pointer to location for e_get_config to save the   */
/*			number of slots configured. Total slots are 	   */
/*			embedded slots + expansion slots + virtual slots,  */
/*			and bust be less than or equal to 64.		   */
/*                                                                         */
/***************************************************************************/

int
e_get_config (struct cnfg_hdr	cnfg_hdr_p[], int  *slots_p)


{  /* Start get configuration */

   struct cnfg_hdr     *conf_hdr_p;
   
   struct  cnfg_blk    *cnfg_blk_p;
   struct  cnfg_blk    *last_blk_p;
   
   struct rpb	*rpb_p;
   struct  general_slot_info	slot_info;
   
   int	i,j,k,l;
   int	num_slots;
   int	data;
   int	hdrs;
   int	offset;
   int	slots;
   int	entry;
   int	init_index;
   int	init_to_read[] = {1,2,4,0,2,4,8,0};
   int		length;
   
   uchar_t     *ecu_data_p;
   uchar_t     *config_p;
   uchar_t     *cur_p;
   uchar_t     *ucur_p;
   uchar_t      last;
   
   boolean_t	entries;
   
   vm_offset_t	config_start;
   vm_offset_t	ecu_config_start;
   vm_offset_t	config_addr;
   
   /*-------*/
   /* BEGIN */
   /*-------*/

   /*------------------------------------------*/
   /* Get location of configuration from hwrpb */
   /*------------------------------------------*/
   rpb_p = (struct rpb *)OSF_HWRPB_ADDR;
   config_start = (vm_offset_t)rpb_p + rpb_p->rpb_percpu_off;
   ecu_config_start = ((struct rpb_percpu *)config_start)->rpb_palrev_avail[3];
   config_start = rpb_p->rpb_config_off;
#ifdef ECU_DEBUG
   printf ("e_get_config: \n");
   printf ("\t ecu_config_start = 0x%lx \n", ecu_config_start);
   printf ("\t config_start     = 0x%lx \n", config_start);
#endif /* ECU_DEBUG */
   ecu_config_start = config_start;
   
/*   if (ecu_config_start == 0L || ecu_config_start & 8000000000000000L)
*/
   if (ecu_config_start == 0L || (ecu_config_start>>63 != 0)) 
      panic ("e_get_config: no ECU data.\n");
   
   /*--------------------------------------------*/
   /* Read ecu headers until we find a good one. */
   /*--------------------------------------------*/
   config_addr = ecu_config_start;
#ifdef ECU_DEBUG
   printf ("Header start address = 0x%lx \n", config_addr);
#endif /* ECU_DEBUG */
   offset = hdrs = 0;
   conf_hdr_p = cnfg_hdr_p;
   while (offset == 0)
      {
      hdrs++;
      e_read_config_bytes (config_addr, conf_hdr_p->manuf_id,
			   EISA_IDNAMELEN+1);
      config_addr += FLASH_STRIDE * (EISA_IDNAMELEN+1);
      conf_hdr_p->sbb_offset = 0;
      conf_hdr_p->sbb_offset = e_read_config_int (config_addr);
      config_addr += sizeof(int) * FLASH_STRIDE;
      offset = conf_hdr_p->sbb_offset;
      }

   /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
   /* Should put a check in to make sure we got something */
   /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
   /*----------------------------------------------------*/
   /* Now calculate number of slots then read header for */
   /* each slot that has not been read yet.		 */
   /*----------------------------------------------------*/
   /* This scheme does not work with ecu data since the offset is into a */
   /* different block of the flash. Headers in block 6 ecu data in C.    */
   /* For now the number of slots is hardcoded but should be a platform  */
   /* set global.							 */
   /*--------------------------------------------------------------------*/
   /* slots = offset/CONF_HDR_WIDTH; */
   slots = eisa_expansion_slots;
   *slots_p = slots;
   for (j=0; j<(slots-hdrs); j++)
      {
      conf_hdr_p++;
      e_read_config_bytes (config_addr, conf_hdr_p->manuf_id,
			   EISA_IDNAMELEN+1);
      config_addr += FLASH_STRIDE * (EISA_IDNAMELEN+1);
      conf_hdr_p->sbb_offset = 0;
      conf_hdr_p->sbb_offset = e_read_config_int (config_addr);
      config_addr += sizeof(int) * FLASH_STRIDE;
      }

#ifdef ECU_DEBUG
   /*-------------------------*/
   /* Now print what we have. */
   /*-------------------------*/
   for (i=0; i<slots; i++)
      {
      printf ("FOR SLOT %d:  ", i);
      printf ("Prod ID = %s  ", cnfg_hdr_p[i].manuf_id);
      printf ("Offset = 0x%x \n", cnfg_hdr_p[i].sbb_offset);
      }
#endif /* ECU_DEBUG */

   /*---------------------------------------------------*/
   /* For each occupied slot (i.e. offset > 0) read the */
   /* standard build block.				*/
   /*---------------------------------------------------*/

   /* Allocate a buffer to hold a block of ecu compressed data. */
   /* and a buffer to hold a block of uncompressed data.	*/
   ecu_data_p = (uchar_t *)kalloc(512);
   config_p = (uchar_t *)kalloc(512);
   /* Read data for each slot until 0000 is found. */
   for (i=0; i<slots; i++)
      {
      if (cnfg_hdr_p[i].sbb_offset == 0)
	 continue;
      
      config_addr = eisa_config_base + cnfg_hdr_p[i].sbb_offset;
      cur_p = ecu_data_p;
      last = 0xff;
      length = 1;
      e_read_config_bytes (config_addr, cur_p, 1);
      last = *cur_p++;
      config_addr += FLASH_STRIDE;
      while (1)
	 {
	 e_read_config_bytes (config_addr, cur_p, 1);
	 config_addr += FLASH_STRIDE;
	 length++;
	 if (!last && !(*cur_p))
	    break;
	 last = *cur_p++;
	 } ;
#ifdef ECU_DEBUG
      printf("For Slot %d  length = %d \n", i, length);
      cur_p = ecu_data_p;
      printf("Compressed data is:\n");
      for (j=0; j<length; j+=16)
	 {
	 for (k=j; k<(j+16); k++)
	    printf ("%2x  ", *cur_p++);
	 printf("\n");
	 }
#endif /* ECU_DEBUG */
      
      
      /* Now uncompress the header info. */
      cur_p = ecu_data_p;
      cur_p += uncompress_ecu_data (cur_p, config_p, SIZE_OF_EISA_HDR);

      ucur_p = config_p;
#ifdef ECU_DEBUG
      printf("Un-compressed Header data is:\n");
      for (j=0; j<SIZE_OF_EISA_HDR; j+=16)
	 {
	 for (k=j; k<(j+16); k++)
	    printf ("%2x  ", *ucur_p++);
	 printf("\n");
	 }
#endif /* ECU_DEBUG */

      ucur_p = config_p + HDR_SLOT_INFO_OFFSET;
      slot_info.slot_info = *ucur_p++;
      slot_info.ecu_major_rev = (ushort_t)*ucur_p++;
      slot_info.ecu_minor_rev = (ushort_t)*ucur_p++;
      slot_info.chk_sum = *(ushort *)ucur_p++;
      ucur_p++;
      slot_info.numb_dev_funcs = (ushort_t)*ucur_p++;
      slot_info.func_info = *ucur_p++;
      e_get_config_bytes ((uchar_t *)&slot_info.comp_id, ucur_p, sizeof(int));
      ucur_p += sizeof(int);
      
      cnfg_hdr_p[i].cnfg_blocks = slot_info.numb_dev_funcs;
      last_blk_p = NULL;
      /*------------------------------------*/
      /* Allocate some space for the block. */
      /*------------------------------------*/
      cnfg_blk_p = cnfg_hdr_p[i].config_block_p = 
	 (struct cnfg_blk *)kalloc (sizeof(struct cnfg_blk));
      cnfg_blk_p->next_blk = NULL;
      
      for (j=slot_info.numb_dev_funcs; j; j--)
	 {  /* Read each function data block. */
	 cur_p += uncompress_ecu_data (cur_p, config_p,
				       SIZE_OF_EISA_CONFIG_BLK);

      ucur_p = config_p;
#ifdef ECU_DEBUG
      printf("\nUn-compressed data is:\n");
      for (l=0; l<SIZE_OF_EISA_CONFIG_BLK; l+=16)
	 {
	 for (k=l; k<(l+16); k++)
	    printf ("%2x  ", *ucur_p++);
	 printf("\n");
	 }
#endif /* ECU_DEBUG */

	 ucur_p = config_p;
	 
	 e_get_config_bytes ((uchar_t *)&cnfg_blk_p->id, ucur_p, sizeof(int));
	 ucur_p += sizeof(int);
	 e_get_config_bytes ((uchar_t *)&cnfg_blk_p->slot_info, ucur_p,
			     sizeof(short));
	 ucur_p += sizeof(short);
	 e_get_config_bytes ((uchar_t *)&cnfg_blk_p->cfg_ext, ucur_p,
			     sizeof(short));
	 ucur_p += sizeof(short);
	 e_get_config_bytes (cnfg_blk_p->selections, ucur_p,
			     SIZE_OF_SELECTIONS);
	 ucur_p += SIZE_OF_SELECTIONS;
	 e_get_config_bytes (&cnfg_blk_p->func_info, ucur_p,
			     SIZE_OF_FUNC_INFO);
	 ucur_p += SIZE_OF_FUNC_INFO;

	 /*---------------------------------------------------*/
	 /* Make sure function is enabled. If disabled go get */
	 /* next one.					      */
	 /*---------------------------------------------------*/
	 if (cnfg_blk_p->func_info & FUNC_DISABLED)
	    continue;
	 
	 if (cnfg_blk_p->func_info & TYPE_STRING)
	    { /* Read type string */
	    e_get_config_bytes (cnfg_blk_p->type_string, ucur_p,
				SIZE_OF_TYPE_STRING);
	    } /* End read type string. */
	 else
	    cnfg_blk_p->type_string[0] = '\0';
	 ucur_p += SIZE_OF_TYPE_STRING;

	 /*----------------*/
	 /* Memory entries */
	 /*----------------*/
	 entry = 0;
	 if (cnfg_blk_p->func_info & MEM_ENTRY)
	    { /* Read memory entries */
	    entries = TRUE;
	    while (entries)
	       {
	       e_get_config_bytes ((uchar_t *)&cnfg_blk_p->mem_cnfg[entry],
				   ucur_p, SIZE_OF_MEM_ENTRY);
	       ucur_p += SIZE_OF_MEM_ENTRY;
	       if (entry == NUMB_OF_MEM_ENTRIES-1)
		  {
		  entries = FALSE;
		  entry++;
		  continue;
		  }
	       if (!(cnfg_blk_p->mem_cnfg[entry] & 0x80L))
		  entries = FALSE;
	       entry++;
	       }
	    if (entry < NUMB_OF_MEM_ENTRIES)
	       cnfg_blk_p->mem_cnfg[entry] = EISA_NO_CONFIG;
	    } /* End read memory entries */
	 else
	    cnfg_blk_p->mem_cnfg[entry] = EISA_NO_CONFIG; /* Load illegal */
							  /* value to */
							  /* indicate no */
							  /* data. */
	 ucur_p += (NUMB_OF_MEM_ENTRIES-entry) * SIZE_OF_MEM_ENTRY;

	 /*-------------*/
	 /* IRQ entries */
	 /*-------------*/
	 entry = 0;
	 if (cnfg_blk_p->func_info & IRQ_ENTRY)
	    { /* Read IRQ entries. */
	    entries = TRUE;
	    while (entries)
	       {
	       e_get_config_bytes ((uchar_t *)&cnfg_blk_p->irq_cnfg[entry],
				   ucur_p, SIZE_OF_IRQ_ENTRY); 
	       ucur_p += SIZE_OF_IRQ_ENTRY;
	       if (entry == NUMB_OF_IRQ_ENTRIES-1)
		  {
		  entries = FALSE;
		  entry++;
		  continue;
		  }
	       if (!(cnfg_blk_p->irq_cnfg[entry] & 0x80))
		  entries = FALSE;
	       entry++;
	       }
	    if (entry < NUMB_OF_IRQ_ENTRIES)
	       cnfg_blk_p->irq_cnfg[entry] = EISA_NO_CONFIG;
	    } /* End read IRQ entries. */
	 else
	    cnfg_blk_p->irq_cnfg[entry] = EISA_NO_CONFIG; /* Load illegal */
							  /* value to */
							  /* indicate no */
							  /* data. */
	 ucur_p += (NUMB_OF_IRQ_ENTRIES-entry) * SIZE_OF_IRQ_ENTRY;
 
	 /*-------------*/
	 /* DMA entries */
	 /*-------------*/
	 entry = 0;
	 if (cnfg_blk_p->func_info & DMA_ENTRY)
	    { /* Read DMA entries. */
	    entries = TRUE;
	    while (entries)
	       {
	       e_get_config_bytes ((uchar_t *)&cnfg_blk_p->dma_cnfg[entry],
				   ucur_p, SIZE_OF_DMA_ENTRY);
	       ucur_p += SIZE_OF_DMA_ENTRY;
	       if (entry == NUMB_OF_DMA_ENTRIES-1)
		  {
		  entries = FALSE;
		  entry++;
		  continue;
		  }
	       if (!(cnfg_blk_p->dma_cnfg[entry] & 0x80))
		  entries = FALSE;
	       entry++;
	       }
	    if (entry < NUMB_OF_DMA_ENTRIES)
	       cnfg_blk_p->dma_cnfg[entry] = EISA_NO_CONFIG;
	    } /* End read DMA entries. */
	 else
	    cnfg_blk_p->dma_cnfg[entry] = EISA_NO_CONFIG; /* Load illegal */
							  /* value to */
							  /* indicate no */
							  /* data. */
	 ucur_p += (NUMB_OF_DMA_ENTRIES-entry) * SIZE_OF_DMA_ENTRY;

	 /*--------------*/
	 /* Port entries */
	 /*--------------*/
	 entry = 0;
	 if (cnfg_blk_p->func_info & PORT_ENTRY)
	    { /* Read PORT entries. */
	    entries = TRUE;
	    while (entries)
	       {
	       e_get_config_bytes ((uchar_t *)&cnfg_blk_p->port_cnfg[entry],
				   ucur_p, SIZE_OF_PORT_ENTRY);
	       ucur_p += SIZE_OF_PORT_ENTRY;
	       if (entry == NUMB_OF_PORT_ENTRIES-1)
		  {
		  entries = FALSE;
		  entry++;
		  continue;
		  }
	       entry++;
	       if (!(cnfg_blk_p->port_cnfg[entry] & 0x80))
		  entries = FALSE;
	       entry++;
	       }
	    if (entry < NUMB_OF_PORT_ENTRIES)
	       cnfg_blk_p->port_cnfg[entry] = EISA_NO_CONFIG;
	    } /* End read PORT entries. */
	 else
	    cnfg_blk_p->port_cnfg[entry] = EISA_NO_CONFIG; /* Load illegal */
							   /* value to */
							   /* indicate no */
							   /* data. */
	 ucur_p += (NUMB_OF_PORT_ENTRIES-entry) * SIZE_OF_PORT_ENTRY;

	 /*--------------*/
	 /* Init entries */
	 /*--------------*/
	 entry = 0;
	 if (cnfg_blk_p->func_info & INIT_ENTRY)
	    { /* Read INIT entries. */
	    entries = TRUE;
	    while (entries)
	       {
	       e_get_config_bytes (&cnfg_blk_p->init_data[entry],
				   ucur_p,SIZE_OF_INIT_ENTRY );
	       ucur_p += SIZE_OF_INIT_ENTRY;
	       if (!((cnfg_blk_p->init_data[entry] & 0xff) & 0x80))
		  entries = FALSE;
	       init_index = cnfg_blk_p->init_data[entry] & 0x7;
	       entry += 3;
	       e_get_config_bytes (&cnfg_blk_p->init_data[entry],ucur_p , 
				   init_to_read[init_index]);
	       ucur_p += init_to_read[init_index];
	       entry += init_to_read[init_index];
	       if (entry == NUMB_OF_INIT_BYTES - 1)
		  {
		  entries = FALSE;
		  entry++;
		  }
	       }
	    if (entry < NUMB_OF_INIT_BYTES)
	       cnfg_blk_p->init_data[entry] = (uchar_t)EISA_NO_CONFIG;
	    } /* End read INIT entries. */
	 else
	    cnfg_blk_p->init_data[0] = (uchar_t)EISA_NO_CONFIG;


	 if (j > 1)
	    {  /* We have more config blocks to read so allocate some */
	       /* space. */
	    last_blk_p = cnfg_blk_p;
	    cnfg_blk_p = (struct cnfg_blk *)kalloc (sizeof(struct cnfg_blk));
	    cnfg_blk_p->next_blk = NULL;
	    last_blk_p->next_blk = cnfg_blk_p;
	    }

	 }  /* End read each function data block. */
      

      }  /* End read data for each slot. */
   
   /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
   /* This code is here to support the floppy option until we figure out */
   /* what to do about it. At the moment no ECU data is available.	 */
   /* For now we place the floppy at the end of the supported slots.	 */
   /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

   strcpy (cnfg_hdr_p[slots-1].manuf_id, "ADP0002");	/* Adaptec ID. */
   cnfg_hdr_p[slots-1].sbb_offset = 1;	/* Need a non 0 value here so that */
					/* probe does not ignore it.       */
   cnfg_hdr_p[slots-1].cnfg_blocks = 1;	/* There is only one block of data */
   cnfg_hdr_p[slots-1].config_block_p = cnfg_blk_p = (struct cnfg_blk
						      *)kalloc
							 (sizeof(struct
								 cnfg_blk));
   cnfg_blk_p->next_blk = NULL;
   cnfg_blk_p->id = 0x02009004;
   cnfg_blk_p->slot_info = 0;
   cnfg_blk_p->cfg_ext = 0x0;
   cnfg_blk_p->selections[0] = '\0';
   cnfg_blk_p->func_info = 0x0d;
   strcpy (cnfg_blk_p->type_string, "MSD,FPYCTL");
   cnfg_blk_p->mem_cnfg[0] = EISA_NO_CONFIG;
   cnfg_blk_p->irq_cnfg[0] = 0x6;
   cnfg_blk_p->irq_cnfg[1] = EISA_NO_CONFIG;
   cnfg_blk_p->dma_cnfg[0] = 0x2;
   cnfg_blk_p->dma_cnfg[1] = EISA_NO_CONFIG;
   cnfg_blk_p->port_cnfg[0] = EISA_NO_CONFIG;
   cnfg_blk_p->init_data[0] = (uchar_t)EISA_NO_CONFIG;

   
   
   
}  /* End get configuration */
