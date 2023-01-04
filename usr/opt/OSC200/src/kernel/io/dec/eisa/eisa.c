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
static char *rcsid = "@(#)$RCSfile: eisa.c,v $ $Revision: 1.1.4.8 $ (DEC) $Date: 1993/12/17 20:56:01 $";
#endif

/***************************************************************************/
/*                                                                         */
/* MODULE NAME:	eisa.c 							   */
/* 									   */ 
/* LOCATION:	.../src/kernel/io/dec/eisa				   */
/* 									   */ 
/* DESCRIPTION:								   */
/*		Contains the functions required to initialize the eisa bus */
/*		and populate the driver interface structures for support   */
/*		of device drivers.					   */
/* 									   */ 
/* FUNCTIONS:								   */
/* 									   */ 
/*     get_func_str	Gets function type string for an option. 	   */
/*     eisa_do_config	Initializes board based on ECU generated config.   */
/*     eisa_get_config	Gets config data for an option.			   */
/*     eisa_config_cont	Configure a device.				   */
/*     eisa_enable_option   Enables interrupts from specified option.	   */
/*     eisa_disable_option  Disables interrupts from specified option.	   */
/*     eisa_probe	Probe function that obtains bus configuration info.*/
/*     eisaconfl1	First level configuration function.		   */
/*     eisaconfl2	Second level configuration function.		   */
/*     eisa_slot_to_physaddr	Convert slot number to IO address base.    */
/*     eisa_slot_to_id	Convert slot number to product id.		   */
/*     eisa_slot_to_name Convert slot number to board id of option in slot.*/
/*     eisa_slot_to_ctrl_name  Convert slot number to controller name.	   */
/*     eisa_get_unit_num  Return the unit number of the option in 	   */
/*			  specified slot.				   */
/* 									   */ 
/***************************************************************************/

#include	<sys/types.h>
#include	<sys/param.h>		/* Required for KSEG support */
#include	<machine/rpb.h>		/* Required for HWRPB definition. */
#include	<hal/cpuconf.h>
#include	<io/common/handler.h>
#include	<io/common/devdriver.h>
#include	<io/dec/eisa/eisa.h>
#include	<io/dec/eisa/eisa_cnfg.h>

extern	stray();

extern	struct device	device_list[];

extern	find_einfo_entry();

static	eisa_probed = 0;

struct	eisa_slot	eisa_slot[MAX_EISA_SLOTS*2];

struct	eisa_sw		eisa_sw;
struct	bus_funcs	eisa_funcs;

/*--------------------------------------------------------------*/
/* config_hdr[0] is for the system board which we  do nothing   */
/* with at the moment.						*/
/*--------------------------------------------------------------*/
struct cnfg_hdr	config_hdr[MAX_EISA_SLOTS];

int	next_free_entry = MAX_EISA_SLOTS;	/* Index of next free slot */
						/* table entry. 	   */
int	phys_slots;	/* Number of eisa slots on system. Returned */
                        /* from e_get_config.	 		    */
int	table_slots;	/* Number of entries used in eisa slot table. */

vm_offset_t	eisa_io_base;	/* Base address for eisa bus io addresses. */
vm_offset_t	eisa_mem_base;	/* Base address for eisa bus mem addresses. */
vm_offset_t	eisa_config_base; /* Base address for configuration data. */
uint_t		eisa_config_stride; /* Stride between bytes in config space. */

/***************************************************************************/
/*                                                                         */
/* get_func_str  -  Gets function type string for an option.		   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	void  get_func_str (src_str, func_str)			           */
/*                                                                         */
/*	char	*src_str;		                                   */
/*	char	*func_str;						   */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	src_str		EISA type and sub-type string for option.	   */
/*                                                                         */
/*	func_str	Pointer to buffer to place function string in.     */
/*                                                                         */
/***************************************************************************/

void
get_func_str (src_str, func_str)

char	*src_str;
char	*func_str;

{  /* Begin get_func_str. */

   register char       *src_p;
   register char       *func_p;
   register int		count;
   
   src_p = src_str;
   func_p = func_str;
   count = 0;
   
   while (src_p  && *src_p != ';'  &&  count++ < EISA_FUNCLEN)
      *func_p++ = *src_p++;
   *func_p = '\0';

}  /* End  get_func_str. */


/***************************************************************************/
/*                                                                         */
/* eisa_do_config  -  Initializes board based on ECU generated config.	   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	void  eisa_do_config (struct controller *ctlr_p)	           */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	ctlr_p	Pointer to controller structure for device to be	   */
/*		initialized. 						   */
/*                                                                         */
/* DESCRIPTION								   */
/*	If specified option requires an IRQ the IRQ's trigger sense is set */
/*	to the sense specified in the configuration. If option requires a  */
/*	DMA channel set the timing and transfer size for the specified DMA */
/*	channel. Finally parse the initialization data and write the       */
/*	specified data to the specified IO registers.			   */
/***************************************************************************/

void
eisa_do_config (struct controller *ctlr_p)

{  /* Begin eisa_do_config. */

   struct  eisa_config_info	*cnfg_info_p;
   uchar_t	*init_p;
   int		 i;
   int		 access_size;
   int		 have_mask;
   int		 more;
   int		 value;
   int		 mask;
   int		slot;
   vm_offset_t	 port_addr;
   

   /*-----------------------------*/
   /* Get slot this option is in. */
   /*-----------------------------*/
   slot = ctlr_p->slot;
   
   /*-------------------------------------*/
   /* Get base address of slots IO space. */
   /*-------------------------------------*/
   port_addr = eisa_slot[slot].phys_io_addr;

   /*-----------------------------------------------------------------------*/
   /* Start by disabling and resetting board. In this state board will only */
   /* respond to slot specific IO. Board will be enabled again before       */
   /* probe. 								    */
   /*-----------------------------------------------------------------------*/
#ifdef	EISA_DEBUG
   printf ("Disabling option in slot %d at address 0x%lx\n", slot, port_addr);
#endif	/* EISA_DEBUG */
   WRITE_BUS_D8 (port_addr | EISA_BCR_OFFSET, EISA_BOARD_RESET);


   /*------------------------------------------------------------------*/
   /* Make sure board is ready to accept init data. Do this by reading */
   /* first byte of board id. If value returned is 0x7x (i.e. bits 4,5 */
   /* and 6 = 111) board is not ready. If board is not ready we wait   */
   /* 100 ms then try again. Continue this process until board is      */
   /* ready. Once board is ready we go ahead and initialize it.	       */
   /*------------------------------------------------------------------*/
   port_addr |=  EISA_ID_OFFSET;


   WRITE_BUS_D8 (port_addr, EISA_ID_PRECHARGE);
   mb();
   while ((((value = READ_BUS_D8(port_addr)) >> 4) & 0xf) == 7)
      {
      microdelay (100000);
#ifdef	EISA_DEBUG
      printf ("Option in slot %d not ready. \n", slot);
#endif	/* EISA_DEBUG */
      }

   
   /*--------------------------------*/
   /* Get config info for this slot. */
   /*--------------------------------*/
   cnfg_info_p = eisa_slot[slot].slot_info_p;
   
   while (cnfg_info_p)
      {  /* Process this configuration block. */
      /*-----------------------------------------------*/
      /* If we use an interrupt set its trigger sense. */
      /*-----------------------------------------------*/
      i = 0;
      if ((cnfg_info_p->irq[i].intr_cnfg & EISA_BYTE_MASK) != EISA_NO_CONFIG)
	 {  /* Have an interrupt so set the trigger sense. */
	 do
	    {
	    if (cnfg_info_p->irq[i].intr.trigger == 1)
	       (*eisa_sw.set_irq_level)(cnfg_info_p->irq[i].intr.intr_num);
	    else
	       (*eisa_sw.set_irq_edge)(cnfg_info_p->irq[i].intr.intr_num);
	    more = cnfg_info_p->irq[i++].intr_cnfg & 0x80;
	    } while (more);
	 } /* End have interrupts. */
      
      /*-------------------------------------------------------------*/
      /* If this option uses DMA set channel's timing and xfer size. */
      /*-------------------------------------------------------------*/
      i = 0;
      if ((cnfg_info_p->dma_chan[i].dma_cnfg & EISA_BYTE_MASK) !=
	  EISA_NO_CONFIG)
	 {  /* Use DMA so set it up. */
	 do
	    {
	    (*eisa_sw.dma_config)(&cnfg_info_p->dma_chan[i]);
	    more = cnfg_info_p->dma_chan[i].dma_cnfg & 0x80;
	    }  while(more);
	 }  /* End use DMA. */
      
      /*---------------------------------------------------------*/
      /* Now write initialization values to specified registers. */
      /*---------------------------------------------------------*/
      init_p = cnfg_info_p->init_data;
      if (*init_p != (uchar_t)EISA_NO_CONFIG)
	 {  /* Have initialization data so process it. */
	 do
	    {
	    access_size = (int)(*init_p) & 0x3;
	    have_mask = (int)(*init_p) & 0x4;
	    more = (int)(*init_p++) & 0x80;
	    port_addr = (int)(*init_p++);
	    port_addr = (port_addr | ((int)(*init_p++) << 8)) & EISA_WORD_MASK;
	    port_addr |= eisa_io_base;
	    
	    if (!have_mask)
	       {  /* No mask just get value and write it. */
	       switch (access_size)
		  {
		case	EISA_BYTE:
		  value = (int)(*init_p++) & EISA_BYTE_MASK;
		  break;
		case	EISA_WORD:
		  value = (int)(*init_p++);
		  value = (value |  ((int)(*init_p++) << 8)) & EISA_WORD_MASK;
		  break;
		case	EISA_DWORD:
		  value = 0;
		  for (i=0; i<4; i++)
		     value = value | ((int)(*init_p++) << (8*i));
		  value &= EISA_DWORD_MASK;
		  break;
		  }  /* End switch */
	       }  /* End no mask. */
	    else
	       {  /* Have mask, so read port, or in unmasked bits then write. */
	       switch (access_size)
		  {
		case	EISA_BYTE:
		  value = (int)(*init_p++) & EISA_BYTE_MASK;
		  mask  = (int)(*init_p++) & EISA_BYTE_MASK;
		  value = (value & ~mask) | (READ_BUS_D8(port_addr) &
					     mask);
		  break;
		case	EISA_WORD:
		  value = (int)(*init_p++);
		  value = (value |  ((int)(*init_p++) << 8)) & EISA_WORD_MASK;
		  mask = (int)(*init_p++);
		  mask = (mask |  ((int)(*init_p++) << 8)) & EISA_WORD_MASK;
		  value = (value & ~mask) | (READ_BUS_D16(port_addr) &
					     mask);
		  break;
		case	EISA_DWORD:
		  value = 0;
		  for (i=0; i<4; i++)
		     value = value | ((int)(*init_p++) << (8*i));
		  value &= EISA_DWORD_MASK;
		  mask = 0;
		  for (i=0; i<4; i++)
		     mask = mask | ((int)(*init_p++) << (8*i));
		  mask &= EISA_DWORD_MASK;
		  value = (value & ~mask) | (READ_BUS_D32(port_addr) &
					     mask);
		  break;
		  }  /* End switch */
	       }  /* End have mask. */
	    
	    write_io_port (port_addr, (access_size == 2 ? 4: access_size+1),
			   BUS_IO, value);
	    }  while (more);
	 }  /* End process init data. */
      
      cnfg_info_p = cnfg_info_p->next_p;
      }  /* End process this configuration block. */
   
   
}  /* End eisa_do_config. */



/***************************************************************************/
/*                                                                         */
/* eisa_get_config  -  Gets config data for an option.			   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	int  eisa_get_config (struct controller *ctlr_p,		   */
/*			      uint_t config_item, char *func_type,	   */
/*			      void *data_p, int handle)	           	   */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	ctlr_p		Pointer to controller structure for option.	   */
/*                                                                         */
/*	config_item	Item of config data desired. Legal values are	   */
/*			EISA_MEM, EISA_IRQ, EISA_DMA and EISA_PORT which   */
/* 			are defined in io/dec/eisa/eisa.h.		   */
/*                                                                         */
/*	func_type	Function type for which data is desired. This is   */
/*			the function type string that appears in the 	   */
/*			option's EISA config file.			   */
/*									   */
/*	data_p		Pointer to a structure appropriate for the data    */
/*			requested. Structures are defined in 		   */
/*			io/dec/eisa/eisa.h. They are bus_mem, irq, dma and */
/*			e_port.						   */
/*									   */
/*	handle		Handle returned if there is more config data of    */
/*			the type requested. ON INITIAL CALL THIS SHOULD BE */
/*			SET TO ZERO. On subsequent calls it should be set  */
/*			to the value returned by the prior call.	   */
/*									   */
/* RETURN VALUE								   */
/*	If option has only one resource of the type requested its value is */
/*	placed in the data_p parameter and the function returns a value of */
/*	0. If option has multiple resources of the type requested the 	   */
/*	value at the head of the list is placed in the data_p parameter    */
/*	and the function returns a handle that points to the next element  */
/*	in the list. To get the next element call eisa_get_config again	   */
/*	with the returned handle passed in as the handle parameter. When   */
/*	all elements have been read eisa_get_config will return a value of */
/*	0. If option does not have a resource of the type requested a 	   */
/*	value of -1 is returned.					   */
/***************************************************************************/

int
eisa_get_config (struct controller *ctlr_p, uint_t config_item,
		 char *func_type,	    void *data_p,
		 int handle)

{  /* Begin eisa_get_config. */

   int		slot;
   int		found;
   int		have_data;
   int	ret_val;

   uint_t	byte0;
   
   struct  eisa_config_info  *info_p, *entry_p;
   struct  bus_mem	*mem_p;
   struct  irq		*irq_p;
   struct  dma		*dma_p;
   struct  e_port	*port_p;
   
   char	       *type;
   
   /*********/
   /* BEGIN */
   /*********/

   ret_val = 0;
   
   /*----------------------------------------------------*/
   /* Get slot number and pointer to configuration data. */
   /*----------------------------------------------------*/
   slot = ctlr_p->slot;
   info_p = eisa_slot[slot].slot_info_p;
   
   /*-------------------------*/
   /* Set type to search for. */
   /*-------------------------*/
   if (func_type && func_type[0] != '\0')
      type = func_type;
   else
      /* No function specified so get it from eisa_slot table. */
      type = eisa_slot[slot].function;
   
   /*-------------*/
   /* Go find it. */
   /*-------------*/
   have_data = FALSE;
   found = find_einfo_entry (type, info_p, &entry_p);
   while (found==0 && !have_data)
      { /* Begin while found. */
      switch (config_item)
	 { /* Begin switch. */
         case	EISA_MEM:
	    if (entry_p->phys_mem[handle].size != 0)
	       have_data = TRUE;
	    else  if (entry_p->next_p)
	       info_p = entry_p->next_p;
	    else
	       found = -1;
	    break;

	 case	EISA_IRQ:
	    if (entry_p->irq[handle].intr_cnfg != EISA_NO_CONFIG)
	       have_data = TRUE;
	    else  if (entry_p->next_p)
	       info_p = entry_p->next_p;
	    else
	       found = -1;
	    break;

	 case	EISA_DMA:
	    if (entry_p->dma_chan[handle].dma_cnfg != EISA_NO_CONFIG)
	       have_data = TRUE;
	    else  if (entry_p->next_p)
	       info_p = entry_p->next_p;
	    else
	       found = -1;
	    break;

	 case	EISA_PORT:
	    if (entry_p->ports[handle] != EISA_NO_CONFIG)
	       have_data = TRUE;
	    else  if (entry_p->next_p)
	       info_p = entry_p->next_p;
	    else
	       found = -1;
	    break;
	 
	 default:
	    found = -1;
	    break;
	 } /* End switch. */
      /*-------------------------------------------------------*/
      /* If we have found the right one yet and there are more */
      /* entries in the list then go get the next one and loop */
      /* back again to see if it contains the data we are      */
      /* looking for.					       */
      /*-------------------------------------------------------*/
      if (!have_data && found == 0)
	 found = find_einfo_entry (type, info_p, &entry_p);
      } /* End while found. */
   

   /*--------------------------------------*/
   /* Now load data into return structure. */
   /*--------------------------------------*/
   switch (config_item)
      { /* Begin switch. */
      case	EISA_MEM:
         if (found == -1)
	    {
	    ret_val = -1;
	    ((struct bus_mem *)data_p)->size = 0;
	    }
	 else
	    {
	    mem_p = (struct bus_mem *)data_p;
	    mem_p->isram = entry_p->phys_mem[handle].isram;
	    mem_p->decode = entry_p->phys_mem[handle].decode;
	    mem_p->unit_size = entry_p->phys_mem[handle].unit_size;
	    mem_p->size = entry_p->phys_mem[handle].size;
	    mem_p->start_addr = entry_p->phys_mem[handle].start_addr |
	                        eisa_mem_base;
	    if (handle+1 < NUMB_OF_MEM_ENTRIES &&
		entry_p->phys_mem[handle+1].size != 0 )
	       ret_val = handle+1;
	    }
	 break;
	 
      case	EISA_IRQ:
	 if (found == -1)
	    {
	    ret_val = -1;
	    ((struct irq *)data_p)->channel = EISA_NO_CONFIG;
	    }
	 else
	    {
	    irq_p = (struct irq *)data_p;
	    irq_p->channel = entry_p->irq[handle].intr.intr_num;
	    irq_p->trigger = entry_p->irq[handle].intr.trigger;
	    irq_p->is_shared = entry_p->irq[handle].intr.is_shared;
	    if (handle+1 < NUMB_OF_IRQ_ENTRIES &&
		entry_p->irq[handle+1].intr_cnfg != EISA_NO_CONFIG)
	       ret_val = handle+1;
	    }
	 break;
	 
      case	EISA_DMA:
	 if (found == -1)
	    {
	    ret_val = -1;
	    ((struct dma *)data_p)->channel = EISA_NO_CONFIG;
	    }
	 else
	    {
	    dma_p = (struct dma *)data_p;
	    dma_p->channel = entry_p->dma_chan[handle].dma.channel;
	    dma_p->is_shared = entry_p->dma_chan[handle].dma.is_shared;
	    dma_p->xfer_size = entry_p->dma_chan[handle].dma.xfer_size;
	    dma_p->timing = entry_p->dma_chan[handle].dma.timing;
	    if (handle+1 < NUMB_OF_DMA_ENTRIES &&
		entry_p->dma_chan[handle+1].dma_cnfg != EISA_NO_CONFIG)
	       ret_val = handle+1;
	    }
	 break;

      case	EISA_PORT:
	 if (found == -1)
	    {
	    ret_val = -1;
	    ((struct e_port *)data_p)->numb_of_ports = 0;
	    }
	 else
	    {
	    port_p = (struct e_port *)data_p;
	    byte0 = entry_p->ports[handle] & EISA_BYTE_MASK;
	    
	    port_p->base_address = ((entry_p->ports[handle] >> EISA_B8) &
	                           EISA_WORD_MASK) | eisa_io_base;
	    port_p->numb_of_ports = (byte0 & EISA_B5_MASK) + 1;
	    port_p->is_shared = byte0 & EISA_B6;
	    
	    if (handle+1 < NUMB_OF_PORT_ENTRIES &&
		entry_p->ports[handle+1] != EISA_NO_CONFIG)
	       ret_val = handle+1;
	    }
	 break;
	 

      default:
         ret_val = -1;
      
      } /* End   switch. */
   
   return (ret_val);
   
}  /* End eisa_get_config. */



/***************************************************************************/
/*                                                                         */
/* eisa_config_cont  -  Configure device controller.			   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	int  eisa_config_cont (phys_addr, slot, ctlr)		           */
/*                                                                         */
/*	io_handle_t	phys_addr;	                                   */
/*	uint_t		slot;						   */
/*	struct  controller	*ctlr;					   */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	phys_addr	Handle to controllers IO address space.  	   */
/*                                                                         */
/*	slot		Expansion slot number this option is in.	   */
/*                                                                         */
/*	ctlr		Pointer to controller structure for this option.   */
/*									   */
/***************************************************************************/

int
eisa_config_cont (phys_addr, slot, ctlr)

io_handle_t		phys_addr;
uint_t			slot;
struct  controller	*ctlr;

{  /* Begin eisa_config_cont. */

   register struct driver *drp;
   register struct device *device;
   int savectlr;
   char *savectname;
   int i;
   
   
   if (ctlr->alive & ALV_ALIVE)
      return(0);
   
   /*------------------------------------------*/
   /* Get driver stucture for this controller. */
   /*------------------------------------------*/
   drp = ctlr->driver;
   
   /*---------------------------------------------*/
   /* Load addresses into controller structure so */
   /* that the probe can use them if desired.     */
   /* Physical is raw eisa io address, virtual is */
   /* is first byte swizzled.			  */
   /*---------------------------------------------*/
   ctlr->physaddr = (caddr_t)phys_addr;
   ctlr->slot = slot;
   
   i = (*drp->probe)(phys_addr, ctlr);
   if (i == 0)
      return(0);
   ctlr->alive |= ALV_ALIVE;

   drp->ctlr_list[ctlr->ctlr_num] = ctlr;
   config_fillin(ctlr);
   printf("\n");

   if (drp->cattach)
      (*drp->cattach)(ctlr);
   
   for (device = device_list; device->dev_name;	device++)
      {
      if (((device->ctlr_num != ctlr->ctlr_num) &&
	   (device->ctlr_num !=-1) && (device->ctlr_num != -99)) ||
	  ((strcmp(device->ctlr_name, ctlr->ctlr_name)) &&
	   (strcmp(device->ctlr_name, "*"))) ||
	  (device->alive & ALV_ALIVE) ||
	  (device->alive & ALV_NOCNFG) ) 
	 {
	 continue;
	 }
   
      savectlr = device->ctlr_num;
      savectname = device->ctlr_name;
      device->ctlr_num = ctlr->ctlr_num;
      device->ctlr_name = ctlr->ctlr_name;
      
      if ((drp->slave) && (*drp->slave)(device, phys_addr))
	 {
	 device->alive |= ALV_ALIVE;
	 conn_device(ctlr, device);
	 drp->dev_list[device->logunit] = device;
	 if(device->unit >= 0)
	    {
	    /* print bus target lun info for SCSI devices */
	    if( (strncmp(device->dev_name, "rz", 2) == 0) |
	       (strncmp(device->dev_name, "tz", 2) == 0) )
	       {
	       printf("%s%d at %s%d bus %d target %d lun %d",
		      device->dev_name, device->logunit,
		      drp->ctlr_name, ctlr->ctlr_num, 
		      ((device->unit & 0xFC0) >> 6),
		      ((device->unit & 0x38) >> 3),
		      (device->unit & 0x7) );
	       }
	    else
	       {
	       printf("%s%d at %s%d unit %d",
		      device->dev_name, device->logunit,
		      drp->ctlr_name, ctlr->ctlr_num, device->unit);
	       }
	    }
	 else
	    {
	    printf("%s%d at %s%d",
		   device->dev_name, device->logunit,
		   drp->ctlr_name, ctlr->ctlr_num);
	    }
	 if (drp->dattach)
	    (*drp->dattach)(device);
	 printf("\n");
	 }
      else
	 {
	 device->ctlr_num = savectlr;
	 device->ctlr_name = savectname;
	 }
      }
   
   return (1);
   

}  /* End eisa_config_cont. */


/***************************************************************************/
/*                                                                         */
/* eisa_busphys_to_iohandle  -  Returns io_handle for bus address	   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	void  eisa_busphys_to_io_handle (addr,flagsctlr_p)                 */
/*                                                                         */
/*      u_long  addr;							   */
/*      int     flags;							   */
/*	struct  controller	ctlr_p;	                                   */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*      addr    bus physical addres as seen from the bus device 	   */
/*      flags   directs what type of handle to return (BUS_IO,BUS_MEMORY,  */
/*                                                     DENSE_MEMORY)       */
/*	ctlr_p	Pointer to controller structure for option for which to    */
/*		return handle. 					           */
/*                                                                         */
/* DESCRIPTION 								   */
/*      Returns an io_handle for the address space specified in flags      */
/*      to be used with the routines read_io_port, write_io_port,          */
/*      io_copyin, and io_copyout.                                         */
/***************************************************************************/

io_handle_t
eisa_busphys_to_iohandle(u_long addr, int flags, struct controller *ctlr_p)
{  

   (*eisa_sw.busphys_to_iohandle) (addr,flags,ctlr_p);

} 




/***************************************************************************/
/*                                                                         */
/* eisa_enable_option  -  Enables interrupts from specified option.	   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	void  eisa_enable_option (ctlr_p)		                   */
/*                                                                         */
/*	struct  controller	ctlr_p;	                                   */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	ctlr_p	Pointer to controller structure for option for which to    */
/*		enable interrupts.					   */
/*                                                                         */
/* DESCRIPTION 								   */
/* 	Enables interrupts from option represented by the passed in 	   */
/* 	controler structure. This is done by calling the cpu specific code */
/* 	through the eisa_sw table. The interrupt line to enable is 	   */
/*	obtained through the eisa_info element of the controller structure.*/
/***************************************************************************/

void
eisa_enable_option (ctlr_p)

struct  controller	*ctlr_p;

{  /* Begin eisa_enable_option */

   (*eisa_sw.enable_option) (eisa_slot[ctlr_p->slot].irq.intr.intr_num);
   
}  /* End eisa_enable_option */


/***************************************************************************/
/*                                                                         */
/* eisa_disable_option  -  Disables interrupts from specified option.	   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	void  eisa_disable_option (ctlr_p)		                   */
/*                                                                         */
/*	struct  controller	ctlr_p;	                                   */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	ctlr_p	Pointer to controller structure for option for which to    */
/*		disable interrupts.					   */
/*                                                                         */
/* DESCRIPTION 								   */
/* 	Disables interrupts from option represented by the passed in 	   */
/* 	controler structure. This is done by calling the cpu specific code */
/* 	through the eisa_sw table. The interrupt line to disable is 	   */
/*	obtained through the eisa_info element of the controller structure.*/
/***************************************************************************/

void
eisa_disable_option (ctlr_p)

struct  controller	*ctlr_p;

{  /* Begin eisa_disable_option */

   (*eisa_sw.disable_option) (eisa_slot[ctlr_p->slot].irq.intr.intr_num);

}  /* End eisa_disable_option */


/***************************************************************************/
/*                                                                         */
/* eisa_probe  -  Probe function that obtains bus configuration info..	   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	int  eisa_probe (verbose)		   			   */
/*                                                                         */
/*	int		verbose;	                                   */
/*                                                                         */
/* PARAMETERS								   */
/*	verbose		If not set work quietly, otherwise print status.   */
/*                                                                         */
/* DESCRIPTION								   */
/*	Determines configuration on the EISA bus by accessing the 	   */
/*	configuration supplied by the firmware (console).		   */
/*                                                                         */
/***************************************************************************/

void
eisa_probe (verbose)

int	verbose;

{  /* Begin eisa_probe */

   int		i, j, k, l;
   int		blocks;		/* Number of config blocks for a slot. */
   int		index;		/* Index into slot table. */
   int		multi_func;	/* Flag indicatiing if an option has */
                                /* multiple functions.		     */
   int		found;
   int		slot_to_option[MAX_EISA_SLOTS*2];
   int		func_list[MAX_EISA_SLOTS];
   
   ulong_t	mem_desc;
   
   struct  cnfg_hdr	*sl_chdr_p;	/* Pointer to slots config header */
   struct  cnfg_blk	*sl_cnfg_p;	/* Pointer to slots config data */
					/* block.			*/
   struct  bus_mem	*sl_mem_p;	/* Pointer to slots memory desc. */
   struct  eisa_config_info  *info_p;	/* Pointer to slots eisa info list */
   struct  eisa_config_info  *tinfo_p;	/* Pointer to slots eisa info list */
   struct  irq		slot_irq;
   struct  dma   	slot_dma;
   struct  controller	ctlr;
   
   extern struct  eisa_option	eisa_option[];
   extern	  find_einfo_entry();
   
/*-------*/
/* BEGIN */
/*-------*/

   eisa_probed = 1;
   
   /*----------------------------------------------------------------*/
   /* Start by initializing the slot table entries to "safe" values. */
   /*----------------------------------------------------------------*/
   for (i = 0; i<MAX_EISA_SLOTS*2; i++)
      {  /* Init slot table. */
      eisa_slot[i].next_func_index = -1;
      strcpy (eisa_slot[i].board_id, "");
      strcpy (eisa_slot[i].function, "");
      strcpy (eisa_slot[i].driver_name, "");
      eisa_slot[i].slot = -1;
      eisa_slot[i].intr = stray;
      eisa_slot[i].unit = -1;
      eisa_slot[i].phys_io_addr = 0;
      eisa_slot[i].ctlr_p = NULL;
      eisa_slot[i].bus_p = NULL;
      eisa_slot[i].slot_info_p = NULL;
      eisa_slot[i].phys_mem.size = 0;
      eisa_slot[i].irq.intr_cnfg = 0;
      eisa_slot[i].dma_chan.dma_cnfg = 0;
      for (j=0; j<60; j++)
	 eisa_slot[i].init_data[j]  = 0;
      eisa_slot[i].class = 0;
      eisa_slot[i].intr_b4_probe = 0;
      eisa_slot[i].intr_aft_attach = 0;
      eisa_slot[i].adpt_config = 0;
      eisa_slot[i].dev_str = 0;
      eisa_slot[i].intr_param = 0;
      }  /* End init slot table. */
   

   /*---------------------------------------------------------------------*/
   /* After this call config_hdr[] will contain the eisa configuration as */
   /* generated by the firmware. The index into config_hdr[] is the eisa  */
   /* slot number.                                                        */
   /*---------------------------------------------------------------------*/

   e_get_config (config_hdr, &phys_slots);

   table_slots = phys_slots;
   
   /*----------------------------------------------------------------------*/
   /* For each function in each slot read its configuration block and fill */
   /* in the corresponding slot table entry.                               */
   /*----------------------------------------------------------------------*/

   for (i=1; i<phys_slots; i++)
      {  /* For each slot. */
      sl_chdr_p = &config_hdr[i];
      sl_cnfg_p = sl_chdr_p->config_block_p;
      if (sl_chdr_p->sbb_offset == 0)
	 {  /* If offset == 0 there is no option in slot. Just set slot */
	    /* value then go to next slot.				*/
	 eisa_slot[i].slot = i;
	 continue;
	 }
      blocks = sl_chdr_p->cnfg_blocks;
      /* Get config data from config block. */
      if (blocks == 0)
	 {  /* If no configuration blocks we have id only. Set slot */
	    /* value, board id and slot specific io base address.   */
	 eisa_slot[i].slot = i;
	 strcpy (eisa_slot[i].board_id, sl_chdr_p->manuf_id);
	 eisa_slot[i].phys_io_addr = (i << EISA_SLOT_ADDR_SHIFT) |
	                             eisa_io_base;
	 }
      else
	 {  /* Have a configuration block so load the slot table. */
	 /* Start by building the eisa_config_info list for the slot */
	 e_build_eisa_info (sl_cnfg_p, &info_p);
	 /* Now populate the eisa slot table. */
	 eisa_slot[i].slot = i;
	 eisa_slot[i].slot_info_p =info_p;
	 strcpy (eisa_slot[i].board_id, sl_chdr_p->manuf_id);
	 eisa_slot[i].phys_io_addr = (i << EISA_SLOT_ADDR_SHIFT) |
	                             eisa_io_base;
	 }
      }  /* End for each slot. */

   /*------------------------------------------------------------------*/
   /* Now match entries in the slot table with options listed in the   */
   /* eisa_options tables. This is also where we expand multi function */
   /* options to an eisa_slot entry per function.		       */
   /*------------------------------------------------------------------*/
   for (i=1; i<phys_slots; i++)
      {  /* For each slot. */
      /* If no board id then slot is empty so just go to next one. */
      if (eisa_slot[i].board_id[0] == '\0')
	 {
	 slot_to_option[i] = -1;
	 continue;
	 }

      eisa_slot[i].class = EISA_UNKNOWN;
      /*--------------------------------------------------------------*/
      /* Now look for the option in the eisa_option data table to get */
      /* the config name for the controller.		       	      */
      /*--------------------------------------------------------------*/
      for (j=0; eisa_option[j].board_id[0] != '\0'; j++)
	 {  /* Look for this option. */
	 if (!(strcmp (eisa_slot[i].board_id, eisa_option[j].board_id)))
	    {  /* We found this id. See if it is the correct one*/
	    if (eisa_option[j].function[0] != '\0')
	       { /* Have a function string. */
	         /* See if we have multiple entries for this id. */
	       multi_func = FALSE;
	       for (k=j+1; eisa_option[k].board_id[0] != '\0' &&
		    !multi_func; k++)
		  if (!(strcmp (eisa_slot[i].board_id,
				eisa_option[k].board_id)))
		     multi_func = TRUE;
	       if (multi_func)
		  {  /* Expand to multiple eisa_slot entries. */
		  /*--------------------------------------------------*/
		  /* Build List of option entries with this board id. */
		  /*--------------------------------------------------*/
		  for (k=j,l=0; eisa_option[k].board_id[0] != '\0'; k++)
		     if (!(strcmp (eisa_slot[i].board_id,
				   eisa_option[k].board_id)))
			func_list[l++] = k;
		  func_list[l] = -1;	/* Mark the end of the list. */
		  
		  /*-----------------------------------------------------*/
		  /* Walk the list looking for function in slots config. */
		  /*-----------------------------------------------------*/
		  index = i;
		  info_p = eisa_slot[i].slot_info_p;
		  l = 0;
		  while (func_list[l] >= 0)
		     {  /* See if function is in config. */
		     k = func_list[l++];
		     found = find_einfo_entry(eisa_option[k].function,
					   info_p, &tinfo_p);
		     if (found == 0)
			{  /* We found one. */
			if (index != i)
			   {
			   table_slots++;
			   eisa_slot[index] = eisa_slot[i];
			   eisa_slot[index].slot_info_p = tinfo_p;
			   }
			slot_to_option[index] = k;
			strcpy (eisa_slot[index].function,
				eisa_option[k].function);
			index = table_slots;
			}  /* End we found one. */
		     
		     }  /* End: See if function is in config. */
		  
		  }  /* End: Expand to multiple eisa_slot entries. */
	       else
		  {  /* No multifunc. Make sure function is ours. */
		  found = find_einfo_entry(eisa_option[j].function,
					   eisa_slot[i].slot_info_p,
					   &tinfo_p);
		  if (found == 0)
		     {  /* This is ours. */
		     slot_to_option[i] = j;
		     strcpy (eisa_slot[i].function, eisa_option[j].function);
		     }
		  else
		     slot_to_option[i] = -1;
		  }  /* End: No multifunc. */
	       
	       } /* End have a function string. */
	    else
	       /* No function string so declare a match. */
	       slot_to_option[i] = j;
	    break;
	    }  /* End: We found this one so fill slot table data. */
	 else
	    slot_to_option[i] = -1;	/* No controller info for this one. */
	 }  /* End:  Look for this option. */
      }  /* End: For each slot. */
   
      
   /*--------------------------------------------------------------------*/
   /* We now have everything we need to finish filling in the slot table */
   /* entries so do it.							 */
   /*--------------------------------------------------------------------*/
   for (i=1; i<table_slots; i++)
      {  /* For each slot. */
      /* If no match was made then just go to the next entry. */
      if (slot_to_option[i] < 0)
	 continue;
      
      j = slot_to_option[i];
      tinfo_p = eisa_slot[i].slot_info_p;
      if (eisa_slot[i].function[0] == '\0')
	 strcpy (eisa_slot[i].function, tinfo_p->type);

      ctlr.slot = i;
      eisa_get_config (&ctlr, EISA_MEM, NULL,
		       (void *)(&eisa_slot[i].phys_mem), 0);
      eisa_get_config (&ctlr, EISA_IRQ, NULL, &slot_irq, 0);
      eisa_get_config (&ctlr, EISA_DMA, NULL, &slot_dma, 0);

      if (eisa_slot[i].phys_mem.size > 0)
	 eisa_slot[i].phys_mem.start_addr |= eisa_mem_base;

      if (slot_irq.channel == EISA_NO_CONFIG)
	 eisa_slot[i].irq.intr_cnfg = EISA_NO_CONFIG;
      else
	 {
	 eisa_slot[i].irq.intr.intr_num = slot_irq.channel;
	 eisa_slot[i].irq.intr.trigger = slot_irq.trigger;
	 eisa_slot[i].irq.intr.is_shared = slot_irq.is_shared;
	 }
      
      if (slot_dma.channel == EISA_NO_CONFIG)
	 eisa_slot[i].dma_chan.dma_cnfg = EISA_NO_CONFIG;
      else
	 {
	 eisa_slot[i].dma_chan.dma.channel = slot_dma.channel;
	 eisa_slot[i].dma_chan.dma.is_shared = slot_dma.is_shared;
	 eisa_slot[i].dma_chan.dma.xfer_size = slot_dma.xfer_size;
	 eisa_slot[i].dma_chan.dma.timing = slot_dma.timing;
	 }
      
      for (k=0; k<60; k++)
	 eisa_slot[i].init_data[k]  = tinfo_p->init_data[k];

      strcpy (eisa_slot[i].driver_name, eisa_option[j].driver_name);
      eisa_slot[i].intr_b4_probe = eisa_option[j].intr_b4_probe;
      eisa_slot[i].intr_aft_attach = eisa_option[j].intr_aft_attach;
      if (eisa_option[j].type == 'A')
	 {
	 eisa_slot[i].class = EISA_ADPT;
	 eisa_slot[i].adpt_config = eisa_option[j].adpt_config;
	 }
      else
	 eisa_slot[i].class = EISA_CTLR;
      }  /* End for each slot. */ 
   

}  /* End eisa_probe */



/***************************************************************************/
/*                                                                         */
/* eisaconfl1  -  First level eisa bus configuration function.		   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	int  eisaconfl1 (bustype, binfo, bus)		                   */
/*                                                                         */
/*	int		bustype;	                                   */
/*	caddr_t		binfo;						   */
/*	struct bus	*bus;						   */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	bustype		Specifies type of bus that this bus connects to.   */
/*			If this bus is the system bus this will be set to  */
/*			-1.						   */
/*                                                                         */
/*	binfo		Pointer to structure containing bus specific 	   */
/*			information for bus this bus connects to.	   */ 
/*                                                                         */
/* 	bus		Pointer to bus structure for this bus.		   */
/*                                                                         */
/* DESCRIPTION								   */
/*	This function reads the configuration generated by the ECU to 	   */
/*	determine what is in each option slot. Using this information it   */
/*	fills in the eisa_slot table. It then turns on interrupts and      */
/*	calls the probe routine for each option.			   */
/***************************************************************************/

int
eisaconfl1(bustype, binfo, bus)

int 		bustype;
caddr_t 	*binfo;
struct bus 	*bus;

{   /* Begin eisaconfl1 */

   struct controller	*ctlr;
   struct bus		*busptr;
   struct eisa_info	*info_p;
   struct bus  		*savebushd;
   struct common_bus_info  *bus_infop;
   struct irq   	slot_irq;
   struct dma   	slot_dma;
   
   int		i, j, index;
   int		multi_func;
   int		scb_vec;
   int		savebus;
   int		reg_value;
   
   char*	savebusname;

/*-------*/
/* BEGIN */
/*-------*/
   
/*--------------------------------------------------------------------------*/
/* If bustype is -1 we are the system bus and therefore the head of the bus */
/* list. If bustype is not -1 then we are connected to some other bus. 	    */
/*--------------------------------------------------------------------------*/

/*------------------------------------------------------------*/
/* Initialize config space base address and stride from the   */
/* switch table for this bus. Then initialize the bus io and  */
/* memory io handles from the bus_info structure handed to us */
/* by our parent.					      */
/*------------------------------------------------------------*/
   eisa_config_base = eisa_sw.config_base_addr;
   eisa_config_stride = eisa_sw.config_stride;

   bus_infop = ((struct bus_info_hdr *)binfo)->common_infop;
   eisa_io_base = bus_infop->sparse_io_base;
   eisa_mem_base = bus_infop->sparse_mem_base;

/*---------------------------*/
/* Load bus function  table. */
/*---------------------------*/
   eisa_funcs.do_config = eisa_do_config;
   eisa_funcs.get_config = eisa_get_config;
   eisa_funcs.enable_option = eisa_enable_option;
   eisa_funcs.disable_option = eisa_disable_option;
   eisa_funcs.busphys_to_iohandle = eisa_busphys_to_iohandle;
   bus->busfuncs = &eisa_funcs;

/*----------------------------------------------------------*/
/* Go determine bus configuration, with printing turned on. */
/*----------------------------------------------------------*/
   eisa_probe (VERBOSE);
   
   
/*----------------------------------------------------------*/
/* Fill in our bus type and the fact that we are now alive. */
/*----------------------------------------------------------*/
   bus->bus_type = BUS_EISA;
   bus->alive |= ALV_ALIVE;

   if (bustype == -1)
      printf("%s%d at nexus\n", bus->bus_name, bus->bus_num);
   else
      printf("%s%d at %s%d\n", bus->bus_name, bus->bus_num, 
	     bus->bus_hd->bus_name, bus->bus_hd->bus_num);

/*-------------------------------------------------------------------*/
/* gjd Loadable driver support has to go in here. We have to connect */
/*     loadable framework to bus structure.			     */
/*-------------------------------------------------------------------*/


/*------------------------------*/
/* Safe to take interrupts now. */
/*------------------------------*/

   splnone();

/*----------------------------------------------------------------------*/
/* For devices found on the system try to get thier controller or bus   */
/* structure. If found finish filling in the slot table, configure the  */
/* board per the ECU, connect the controller to our bus structure, and  */
/* call the options probe and attach functions.				*/
/*----------------------------------------------------------------------*/


   /*----------------------------------------------------------*/
   /* First we fill in the slot table and configure the board. */
   /*----------------------------------------------------------*/
   for (i = 1; i<=table_slots; i++)
      {  /* For each slot. */
      /*-----------------------------------------------------------*/
      /* If the class = 0 there is nothing in the slot. If class = */
      /* EISA_UNKNOWN there is an option in the slot but no driver */
      /* was found in the eisa_options_data table. In either case  */
      /* just go on to the next slot.				   */
      /*-----------------------------------------------------------*/
      if (eisa_slot[i].class == 0  || eisa_slot[i].class == EISA_UNKNOWN)
	 continue;

      index = i;

      /*-----------------------------------------------------------*/
      /* Look for the option in the controller list generated from */
      /* the config file. Start with a fully qualified search and  */
      /* proceed with less qualification.			   */
      /*-----------------------------------------------------------*/
      if( (ctlr = get_ctlr(eisa_slot[index].driver_name,
			   eisa_slot[index].slot, bus->bus_name,
			   bus->bus_num)) || 
	 (ctlr = get_ctlr(eisa_slot[index].driver_name,
			  eisa_slot[index].slot, 
			  bus->bus_name, -1)) ||
	 (ctlr = get_ctlr(eisa_slot[index].driver_name,
			  eisa_slot[index].slot, "*", 
			  -99)) ||
	 (ctlr = get_ctlr(eisa_slot[index].driver_name, -1,
			  bus->bus_name, 
			  bus->bus_num)) ||
	 (ctlr = get_ctlr(eisa_slot[index].driver_name, -1,
			  bus->bus_name, -1)) || 
	 (ctlr = get_ctlr(eisa_slot[index].driver_name, -1, "*", -99)) )
	 {  /* Found a controller entry. */
	 /*-----------------------------------------------------------------*/
	 /* Set alive bit in controller structure to "ALV_PRES" to indicate */
	 /* that this structure is currently connected to an option. We     */
	 /* have to undo this before we call eisa_config_cont.		    */
	 /*-----------------------------------------------------------------*/
	 ctlr->alive |= ALV_PRES;
	 
	 eisa_slot[index].ctlr_p = ctlr;
	 eisa_slot[index].intr = *ctlr->intr;
	 eisa_slot[index].unit = ctlr->ctlr_num;
	 eisa_slot[index].intr_param = (caddr_t)ctlr->ctlr_num;
	 eisa_slot[index].dev_str = (caddr_t)ctlr;
	 /*-----------------------------------------------------------*/
	 /* Build eisa_info structure and attach it to the controller */
	 /* structure. Eisa_info contains resource assingment 	      */
	 /* information for this option. Allocate the space then      */
	 /* populate it. 					      */
	 /*-----------------------------------------------------------*/
	 info_p = (struct eisa_info *)kalloc (sizeof (struct eisa_info));
	 ctlr->eisainfo = (caddr_t)info_p;
	 ctlr->slot = index;
	 eisa_get_config (ctlr, EISA_MEM, NULL, (void *)(&info_p->phys_mem), 0);
	 eisa_get_config (ctlr, EISA_IRQ, NULL, &slot_irq, 0);
	 eisa_get_config (ctlr, EISA_DMA, NULL, &slot_dma, 0);
	 info_p->irq.intr.intr_num = slot_irq.channel;
	 info_p->irq.intr.trigger = slot_irq.trigger;
	 info_p->irq.intr.is_shared = slot_irq.is_shared;

	 info_p->dma_chan.dma.channel = slot_dma.channel;
	 info_p->dma_chan.dma.is_shared = slot_dma.is_shared;
	 info_p->dma_chan.dma.xfer_size = slot_dma.xfer_size;
	 info_p->dma_chan.dma.timing = slot_dma.timing;
	 for (j=0; j<60; j++)
	    info_p->init_data[j] = eisa_slot[index].slot_info_p->init_data[j];
	 /*------------------*/
	 /* Set up intr vec. */
	 /*------------------*/
	 if ((eisa_slot[index].irq.intr_cnfg & EISA_BYTE_MASK) !=
	     EISA_NO_CONFIG)
	    {  /* Set up interrupts for option. */
	    scb_vec = (*eisa_sw.irq_to_scb_vector)
	       (eisa_slot[index].irq.intr.intr_num);
	    intrsetvec(scb_vec, eisa_sw.intr_dispatch, index);

	    }  /* End set up interrupts. */
	 /*-----------------------------------------------*/
	 /* Configure the board as specified by ECU data. */
	 /*						  */
	 /* We only do the configuration on the physical  */
	 /* slots because the init data for a board is not*/
	 /* segregated by function.			  */
	 /*-----------------------------------------------*/
	 if (index < phys_slots)
	    eisa_do_config (ctlr);
	 }  /* End found a controller entry. */
      else
	 {  /* No Controller entry see if there is a bus entry. */
	 if((busptr = get_bus(eisa_slot[index].driver_name,
			      eisa_slot[index].slot,
			      bus->bus_name,  bus->bus_num)) ||
	    (busptr = get_bus(eisa_slot[index].driver_name,
			      eisa_slot[index].slot,
			      bus->bus_name,  -1)) ||
	    (busptr = get_bus(eisa_slot[index].driver_name, -1,
			      bus->bus_name,
			      bus->bus_num)) ||
	    (busptr = get_bus(eisa_slot[index].driver_name,	-1,
			      bus->bus_name,
			      -1)) || 
	    (busptr = get_bus(eisa_slot[index].driver_name,	-1, "*",
			      -99)))
	    {  /* Found a bus entry. */
	    }  /* End found a bus entry. */
	 else
	    {  /* No bus or controller entry. */
	    }  /* End no bus or controller entry. */
	 }  /* End see if bus entry. */
      }  /* End for each slot. */
   
   /*--------------------------------------------------------------*/
   /* Now we connect the controller to our bus, enable the board's */
   /* interrupt if requested and call the options probe function.  */
   /*--------------------------------------------------------------*/
   for (i = 1; i<=table_slots; i++)
      {  /* For each slot. */
      /*-------------------------------------------------------*/
      /* If we have a controller structure for the slot then   */
      /* continue with configuration process. If no controller */
      /* structure go on to the next slot.		       */
      /*-------------------------------------------------------*/
      if (!eisa_slot[i].ctlr_p && !eisa_slot[i].bus_p )
	 continue;

      index = i;
      if (eisa_slot[i].ctlr_p)
	 { 
	 /*---------------------------------------*/
	 /* Have a controller structure so do it. */
	 /*---------------------------------------*/
	 ctlr = eisa_slot[i].ctlr_p;
	 
	 /*------------------------------------------------------*/
	 /* Enable the board before we call it's probe function. */
	 /* 							 */
	 /* Enable per board not per function.			 */
	 /*------------------------------------------------------*/
	 if (i < phys_slots)
	    WRITE_BUS_D8 (eisa_slot[i].phys_io_addr + EISA_BCR_OFFSET,
			    EISA_BOARD_ENABLE);
	 
	 /*----------------------------------------*/
	 /* Enable the slots interrupt if desired. */
	 /*----------------------------------------*/
	 if (((eisa_slot[index].irq.intr_cnfg & EISA_BYTE_MASK) !=
	      EISA_NO_CONFIG) && eisa_slot[index].intr_b4_probe)
	    (*eisa_sw.enable_option)(eisa_slot[index].irq.intr.intr_num);
	 
	 savebushd = ctlr->bus_hd;
	 savebus = ctlr->bus_num;
	 savebusname = ctlr->bus_name;
	 ctlr->bus_hd = bus;		/* Set so controller's probe can */
					/* get to bus structure. 	 */
	 ctlr->bus_num = bus->bus_num;
	 ctlr->bus_name = bus->bus_name;
	 
	 /*----------------------------------------------------*/
	 /* If an adapter, call adpt_config, and skip the rest */
	 /*----------------------------------------------------*/
	 if (eisa_slot[index].class == EISA_ADPT) 
	    {
	    if (eisa_slot[index].adpt_config) 
	       {
	       (*eisa_slot[index].adpt_config)(&eisa_slot[index],bus,ctlr);
	       }
	    else 
	       {
	       printf ("eisaconfl1: Warning: adapter %s has no config routine\n",
		       eisa_slot[index].board_id);
	       }
	    continue;
	    }
	 /*---------------------------------------------------*/
	 /* If option is a controller call config controller. */
	 /*---------------------------------------------------*/
	 if (!(eisa_config_cont (eisa_slot[index].phys_io_addr,
				 eisa_slot[index].slot,
				 eisa_slot[index].dev_str)))
	    { /* Controller not configured undo stuff. */
	    ctlr->bus_hd = savebushd;
	    ctlr->bus_num = savebus;
	    ctlr->bus_name = savebusname;
	    printf ("%s%d not probed\n", eisa_slot[index].driver_name,
		    eisa_slot[index].unit);
	    if (((eisa_slot[index].irq.intr_cnfg & EISA_BYTE_MASK) !=
		 EISA_NO_CONFIG) && eisa_slot[index].intr_b4_probe)
	       (*eisa_sw.disable_option)(eisa_slot[index].irq.intr.intr_num);
	    } /* End controller not configured. */
	 else
	    { /* Controller configured. Connect to our bus and enable */
	      /* interrupts if desired.				      */
	    conn_ctlr(bus, ctlr);
	    if (eisa_slot[index].intr_aft_attach)
	       {
	       if (((eisa_slot[index].irq.intr_cnfg & EISA_BYTE_MASK) !=
		    EISA_NO_CONFIG)) 
		  (*eisa_sw.enable_option)
		     (eisa_slot[index].irq.intr.intr_num);
	       }
	    else 
	       {
	       if (((eisa_slot[index].irq.intr_cnfg & EISA_BYTE_MASK) !=
		    EISA_NO_CONFIG)) 
		  (*eisa_sw.disable_option)
		     (eisa_slot[index].irq.intr.intr_num);
	       }
	    }
	 }  /* End have a controller entry. */
      else
	 {  /* No controller entry therefore have a bus entry. */
	 }  /* End have a bus entry. */
      
      }  /* End for each slot. */


   return(1);

}   /* End   eisaconfl1 */


int
eisaconfl2 (bustype, binfo, bus)

int 		bustype;
caddr_t 	binfo;
struct bus 	*bus;
{
	return(1);
}


/***************************************************************************/
/*                                                                         */
/* eisa_slot_to_physaddr  -  Given a slot number return the base physical  */
/*			     address of the slot's IO address space.	   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	vm_offset_t	eisa_slot_to_physaddr (slot)			   */
/*                                                                         */
/*	uint_t		slot;		                                   */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	slot		EISA slot number for which the IO address is 	   */
/*                      desired.                                           */
/*                                                                         */
/*                                                                         */
/* RETURNS 								   */
/* 	Returns the base physical address of the specified slot's IO       */
/*	address space. For example for slot = 3, eisa_slot_to_physaddr	   */
/*	would return 0x3000.						   */
/*									   */
/***************************************************************************/

vm_offset_t
eisa_slot_to_physaddr (slot)

uint_t	slot;

{  /* Begin eisa_slot_to_physaddr */

   return (eisa_slot[slot].phys_io_addr);

}  /* End eisa_slot_to_physaddr */


/***************************************************************************/
/*                                                                         */
/* eisa_slot_to_id  -  Convert slot number to product id.		   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	void  eisa_slot_to_id (slot, id_p)		                   */
/*                                                                         */
/*	uint_t		slot;		                                   */
/*	char	       *id_p;						   */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	slot	EISA slot number for which the option ID is desired.	   */
/*                                                                         */
/*	id_p	Pointer to a buffer large enough to hold the product id of */
/*              option at the specified slot. Length is EISA_IDNAMELEN+1.  */
/***************************************************************************/

void
eisa_slot_to_id (slot, id_p)

uint_t		slot;
char	       *id_p;

{  /* Begin eisa_slot_to_id */

   bcopy (eisa_slot[slot].board_id, id_p, EISA_IDNAMELEN+1);
   

}  /* End eisa_slot_to_id */


/***************************************************************************/
/*                                                                         */
/* eisa_slot_to_name  -  Convert slot number to board id of option in slot.*/
/*                                                                         */
/* SYNOPSIS                                                                */
/*	char *  eisa_slot_to_name (slot)		                   */
/*                                                                         */
/*	uint_t		slot;		                                   */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	slot	EISA slot number for which the option ID is desired.	   */
/*                                                                         */
/* RETURN								   */
/*	If slot is a legal slot number then a pointer to 		   */
/*	eisa_slot[slot].board_id is returned. Otherwise a pointer of value */
/*	-1 is returned.							   */
/***************************************************************************/

char *
eisa_slot_to_name (uint_t slot)

{  /* Begin eisa_slot_to_name */

   if (!eisa_probed)
      eisa_probe(QUIET);
   
   if (slot >= 0 && slot <= table_slots)
      return (eisa_slot[slot].board_id);
   else
      return ((char *)-1);

}  /* End eisa_slot_to_name */


/***************************************************************************/
/*                                                                         */
/* eisa_slot_to_ctrl_name  -  Convert slot number to controller name.	   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	char *  eisa_slot_to_ctlr_name (uint_t	slot)	                   */
/*                                                                         */
/* PARAMETERS								   */
/*	slot	EISA slot number for which the option controller name is   */
/*		desired.						   */
/*                                                                         */
/* RETURN								   */
/*	If slot is a legal slot number then a pointer to 		   */
/*	eisa_slot[slot].driver_name is returned. Otherwise a pointer of    */
/*	value -1 is returned.						   */
/***************************************************************************/

char *
eisa_slot_to_ctlr_name (uint_t  slot)

{  /* Begin eisa_slot_to_ctlr_name */

   if (!eisa_probed)
      eisa_probe(QUIET);
   
   if (slot >= 0 && slot <= table_slots)
      return (eisa_slot[slot].driver_name);
   else
      return ((char *)-1);
   

}  /* End eisa_slot_to_ctlr_name */


/***************************************************************************/
/*                                                                         */
/* eisa_get_unit_num  -  Return the unit number of the option in 	   */
/*			 specified slot.				   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	int  eisa_get_unit_num (uint_t	slot)		                   */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	slot	EISA slot number for which option unit number is desired.  */
/*                                                                         */
/* RETURN								   */
/*	If slot is a legal slot number then the value of 		   */
/*	eisa_slot[slot].unit is returned. Otherwise a value of -1 is 	   */
/*	returned.							   */
/***************************************************************************/

int
eisa_get_unit_num (uint_t  slot)

{  /* Begin eisa_get_unit_num */

   if (slot >= 0 && slot <= table_slots)
      return (eisa_slot[slot].unit);
   else
      return (-1);
   

}  /* End eisa_get_unit_num */
