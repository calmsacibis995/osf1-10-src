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
static char *rcsid = "@(#)$RCSfile: isp.c,v $ $Revision: 1.1.5.5 $ (DEC) $Date: 1993/12/09 20:24:48 $";
#endif

/***************************************************************************/
/*                                                                         */
/* MODULE NAME: isp.c							   */
/* 									   */ 
/* LOCATION:	.../src/kernel/arch/alpha/hal				   */
/* 									   */ 
/* DESCRIPTION:								   */
/* 									   */ 
/*	Contains functions specific to the Intel 82357 Integrated System   */
/*	Peripheral (ISP). See the Intel 82350DT EISA Chip Set reference    */
/*	manual for specifications.                                         */
/* 									   */ 
/* FUNCTIONS:								   */
/* 									   */ 
/*	isp_dma_init		Initialize dma engine.			   */
/*	isp_dma_config		Configure a DMA channel.		   */
/*	isp_dma_alloc		Allocate resources required for DMA.	   */
/*	isp_dma_load		Setup registers and data structs for DMA.  */
/*	isp_dma_unload		Unload resources used for DMA.		   */
/*	isp_dma_dealloc		Free resources allocated for DMA.	   */
/*	isp_dma_intr		DMA interrupt handler.			   */
/***************************************************************************/

#include	<sys/types.h>

#include	<io/dec/eisa/eisa.h>
#include	<io/common/devdriver.h>
#include	<hal/isp.h>


struct  dma_softc	dma_softc[NDMA_CHAN];

/*
 * knxx files must fill in isp_base with io_handle_t to use isp code
 */
io_handle_t isp_base;

#define WRITE_ISP_D8(a,d)    WRITE_BUS_D8 (isp_base+a,d)
#define READ_ISP_D8(a)       READ_BUS_D8(isp_base +a)

/***************************************************************************/
/*                                                                         */
/* isp_dma_init  -  Initialize dma engine.				   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	int  isp_dma_init ()					           */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	None required.							   */
/*                                                                         */
/*                                                                         */
/* DESCRIPTION								   */
/*	This function does a Master Clear of the DMA controllers. The      */
/*	Master Clear ends up enabling all DMA channels. isp_dma_init then  */
/*	sets the priority to rotate mode, masks all channels except 4,	   */
/*	sets the DREQ sense to active high and sets the DACK sense to 	   */
/*	active low. Channel 4 is not masked because channels 0-3 are 	   */
/*	cascaded through it. To mask channel 4 would effectively mask	   */
/*	channels 0-3.
/***************************************************************************/

int
isp_dma_init ()

{  /* Begin isp_dma_init */

   extern  struct  eisa_sw   eisa_sw;	/* Eisa function switch table. */

   int	data = 0;
   
   /* First do master clear. */
   WRITE_ISP_D8 ( EISA_DMA_CT1_MCLR, EISA_BYTE_MASK);
   mb();
   WRITE_ISP_D8 ( EISA_DMA_CT2_MCLR, EISA_BYTE_MASK);
   mb();

   /* Set up priority and DREQ and DACK levels */
   WRITE_ISP_D8 ( EISA_DMA_CT1_CMD, ISP_PRI_ROTATE | ISP_DREQ_HIGH |
		   ISP_DACK_LOW); 
   mb();
   WRITE_ISP_D8 ( EISA_DMA_CT2_CMD, ISP_PRI_ROTATE | ISP_DREQ_HIGH |
		   ISP_DACK_LOW); 
   mb();

#ifdef	DMA_DEBUG
	data = READ_ISP_D8 (EISA_DMA_CT1_STATUS);
	printf ("DMA CT1 status reg = 0x%x \n", data & EISA_BYTE_MASK);
	data = READ_ISP_D8 (EISA_DMA_CT1_AMASK);
	printf ("DMA CT1 mask reg = 0x%x \n", data & EISA_BYTE_MASK);
	data = READ_ISP_D8 (EISA_DMA_CT2_STATUS);
	printf ("DMA CT2 status reg = 0x%x \n", data & EISA_BYTE_MASK);
	data = READ_ISP_D8 (EISA_DMA_CT2_AMASK);
	printf ("DMA CT2 mask reg = 0x%x \n", data & EISA_BYTE_MASK);
#endif  /* DMA_DEBUG */

   /* Mask off all but channel 4 */
   WRITE_ISP_D8 ( EISA_DMA_CT1_AMASK, ISP_CH_MASK_ALL);
   mb();
   WRITE_ISP_D8 ( EISA_DMA_CT2_AMASK, ISP_CH5_MASK | ISP_CH6_MASK |
		   ISP_CH7_MASK);
   mb();
   
#ifdef	DMA_DEBUG
	data = READ_ISP_D8 (EISA_DMA_CT1_AMASK);
	printf ("DMA CT1 mask reg = 0x%x \n", data & EISA_BYTE_MASK);
	data = READ_ISP_D8 (EISA_DMA_CT2_AMASK);
	printf ("DMA CT2 mask reg = 0x%x \n", data & EISA_BYTE_MASK);
#endif  /* DMA_DEBUG */

   /* Finally enable the DMA interrupt. */
   (*eisa_sw.enable_option) (EISA_DMA_IRQ);

   return (0);

}  /* End isp_dma_init */



/***************************************************************************/
/*                                                                         */
/* isp_dma_config  -  Configure a DMA channel.				   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	int  isp_dma_config (union u_dma  *dma_info_p)		           */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	dma_info_p	Pointer to structure containing configuration 	   */
/*			information for the DMA channel of interest.	   */
/*                                                                         */
/*                                                                         */
/* DESCRIPTION								   */
/*	For the DMA channel specified in dma_info_p this function disables */
/*	auto initialize, sets the address step to increment, sets the 	   */
/*	transfer mode to demand, set the transfer size to that specified   */
/*	in dma_info_p, sets the timing to that specified in dma_info_p,    */
/*	sets EOP to be an output and disables the use of stop registers.   */
/***************************************************************************/

int
isp_dma_config (union u_dma  *dma_info_p)

{  /* Begin isp_dma_config */

   
   uint_t	channel;
   uint_t	size;
   
   channel = dma_info_p->dma.channel;
   size = dma_info_p->dma.xfer_size;
   if (size == 1)
      size = 3;
   
   dma_softc[channel].mode_reg = channel & EISA_B2_MASK;
   dma_softc[channel].emode_reg = ((dma_info_p->dma.timing << EISA_B4) |
				   size << EISA_B2 | (channel & EISA_B2_MASK));
#ifdef	DMA_DEBUG
   printf ("\t *****In isp_dma_config***** \n");
   printf ("\t channel = %d \n", channel);
   printf ("\t mode_reg = 0x%x \n", dma_softc[channel].mode_reg);
   printf ("\t emode_reg = 0x%x \n", dma_softc[channel].emode_reg);
#endif  /* DMA_DEBUG */
   /*---------------------------------------------------------------------*/
   /* First set the mode register which controls auto initialize, address */
   /* step direction and transfer mode. This register also controls 	  */
   /* transfer direction. We will set the direction later as part of the  */
   /* dma load function. For now we will set it to VERIFY. To do this we  */
   /* have to write a register of all zeros except the channel bits which */
   /* specify which channel to program.					  */
   /*---------------------------------------------------------------------*/
   if (channel < DMA_CH4)
      {  /* Channels 0-3, controller 1. */
      WRITE_ISP_D8 (EISA_DMA_CT1_MODE, channel & EISA_B2_MASK);
      WRITE_ISP_D8 (EISA_DMA_CT1_EMODE, ((dma_info_p->dma.timing << EISA_B4) |
		       size << EISA_B2 | (channel & EISA_B2_MASK)));
      }
   else
      {  /* Channels 4-7, controller 2. */
      WRITE_ISP_D8 (EISA_DMA_CT2_MODE, channel & EISA_B2_MASK);
      WRITE_ISP_D8 (EISA_DMA_CT2_EMODE, ((dma_info_p->dma.timing << EISA_B4) |
		       (size << EISA_B2) | (channel & EISA_B2_MASK)));
      }
   mb();
   
   return (0);

}  /* End isp_dma_config */



/***************************************************************************/
/*                                                                         */
/* isp_dma_alloc  -  Allocate resources required for DMA.		   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	int  isp_ma_alloc (struct controller  *ctlr_p, int flags)	   */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	ctlr_p	Pointer to controller structure for the driver that is     */
/*		requesting use of the DMA channel.			   */
/*	flags	Flag word indicating if caller wants to sleep if channel   */
/*		is shared and busy.					   */
/*                                                                         */
/*                                                                         */
/* DESCRIPTION								   */
/*	This function   */
/***************************************************************************/

int
isp_dma_alloc (struct controller  *ctlr_p, int flags)

{  /* Begin isp_dma_alloc */

   struct  dma_softc	*scp;
   int			 chan;
   struct dma            dma_p;

#ifdef	DMA_DEBUG
   printf("\n ENTERED isp_dma_alloc(). calling params: \n");
   printf("\t cntrlrp = 0x%lx \n", ctlr_p);
   printf("\t flags   = 0x%x  \n", flags);
#endif  /* DMA_DEBUG */

   /* Find out which channel this requester wants to use. */
   get_config(ctlr_p, EISA_DMA, "",&dma_p,0);
   chan = dma_p.channel;
   
#ifdef	DMA_DEBUG
   printf("\t channel   = %d  \n", chan);
#endif  /* DMA_DEBUG */

   /* Get the softc structure for this channel. */
   scp = &dma_softc[chan];
   
#ifdef	DMA_DEBUG
   printf(" scp(&dma_softc[chan]) = 0x%x  \n", scp);
#endif  /* DMA_DEBUG */

   /*----------------------------------------------------------------------*/
   /* If channel is free  mark it allocated, set owner  and return. If 	   */
   /* already allocated see if the caller is the current owner. If so do   */
   /* nothing and return. If not see if caller wants to wait. If so and    */
   /* channel is a shared channel sleep until channel is free.		   */
   /*----------------------------------------------------------------------*/
   if (!scp->allocated)
      {
      scp->allocated = TRUE;
      scp->ownerp = ctlr_p;
      }
   else
      {
      if (!(ctlr_p == scp->ownerp))
	 {
	 if (flags & DMA_SLEEP && dma_p.is_shared)
	    {
	    /* sleep until channel is free */
	    /* For now we just return an error and let the user */
	    /* decide if they want to retry later.		*/
	    return (-1);
	    }
	 else
	    return (-1);
	 }
      }   
   return (0);

}  /* End isp_dma_alloc */



/***************************************************************************/
/*                                                                         */
/* isp_dma_load  -  Setup registers and data structs for DMA.		   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	int  isp_dma_load (struct controller  *ctlr_p,		           */
/*			   sglist_t  sglist_p, int flags)		   */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	ctlr_p		Pointer to controller structure for the driver 	   */
/*			that is requesting use of the DMA channel.	   */
/*	sglist_p	Pointer to list of address, byte count pairs to    */
/*			use for DMA transfer.				   */
/*	flags		Flag word indicating if caller wants to sleep if   */
/*			channel is shared and busy. Also indicates 	   */
/*			direction of transfer; DMA_IN into system memory   */
/*			and DMA_OUT out of system memory.		   */
/*                                                                         */
/*                                                                         */
/* DESCRIPTION								   */
/*	This function   */
/***************************************************************************/

int
isp_dma_load (struct controller  *ctlr_p, sglist_t  sglist_p, int flags)

{  /* Begin isp_dma_load */

   int			 chan;	/* Dma channel to be used. */
   int			 dir;	/* Direction of transfer. Read from system */
				/* memory or write to system memory. */
   int			 chaining = FALSE;
   int			 value;
   int			 i;
   int			 more;
   long			 count;
   vm_offset_t		 address;
   struct dma            dma_p;   
   struct  dma_softc	*scp;	/* Pointer to softc structure for channel. */
   sg_entry_t		 sgp;	/* Pointer to current address, count pair. */
   
   int		s = splbio();

#ifdef	DMA_DEBUG
   printf("\n ENTERED isp_dma_load(). some params... \n");
   printf("\t sglistp = 0x%lx \n", sglist_p);
   printf("\t cntrlrp = 0x%lx \n", ctlr_p);
#endif  /* DMA_DEBUG */

   /* Find out which channel this requester wants to use. */
   get_config(ctlr_p, EISA_DMA, "",&dma_p,0);
   chan = dma_p.channel;
   
   /* Get the softc structure for this channel. */
   scp = &dma_softc[chan];
   
   /* Get direction from flags. */
   if (flags & DMA_IN) 
      {
      dir = ISP_WRITE_XFER;
#ifdef	DMA_DEBUG
      printf(" ---- A write to memory ---- \n");
#endif  /* DMA_DEBUG */
      }
   else  if (flags & DMA_OUT) 
      {
      dir = ISP_READ_XFER;
#ifdef	DMA_DEBUG
      printf(" ---- A read from memory ---- \n");
#endif  /* DMA_DEBUG */
      }
   else
      {
      /* gjdfix: What do we do here. */
      }
   
#ifdef	DMA_DEBUG
   printf("sglistp->sgp      = 0x%lx \n", sglist_p->sgp);
   printf("sglistp->val_ents = 0x%x \n", sglist_p->val_ents);
#endif  /* DMA_DEBUG */
   /* See if we need chaining. */
   if (sglist_p->val_ents > 1)
      {
#ifdef	DMA_DEBUG
      printf ("isp_dma_load:   chaining required. \n");
#endif  /* DMA_DEBUG */
      chaining = scp->has_chain = TRUE;
      }   

   /* Store sglist in dma_softc. */
   scp->sglistp = sglist_p;
   
   /* Tell controller direction of transfer. */
   value = scp->mode_reg | dir;
   if (chan < DMA_CH4)
      WRITE_ISP_D8 (EISA_DMA_CT1_MODE, value);
   else
      WRITE_ISP_D8 (EISA_DMA_CT2_MODE, value);

   /* Now load address and count. */
   sgp = dma_get_next_sgentry (sglist_p);
#ifdef	DMA_DEBUG
   printf("\n ---- IN isp_dma_load ---- \n");
   printf("\t First ba-bc load into eisa dma engine \n");
   printf("\t Address of ba-bc pair from get_next = 0x%lx \n", sgp);
#endif  /* DMA_DEBUG */
   address = (vm_offset_t)sgp->ba;
   count = sgp->bc - 1;	/* DMA transfers programmed count + 1 */
#ifdef	DMA_DEBUG
   printf("\t ba = 0x%lx \n", address);
   printf("\t bc = 0x%lx \n", count);
#endif  /* DMA_DEBUG */
   
   for (i=0, more=chaining; i<2; i++)
      {  /* Begin address and count programming. */
      /* Clear the byte pointer so we point to the correct one. */
      if (chan < DMA_CH4)
	 WRITE_ISP_D8 (EISA_DMA_CT1_CLR_BYTE_P, EISA_BYTE_MASK);
      else
	 WRITE_ISP_D8 (EISA_DMA_CT2_CLR_BYTE_P, EISA_BYTE_MASK);
      mb();
#ifdef	DMA_DEBUG
      printf ("\t channel = %d \n", chan);
#endif  /* DMA_DEBUG */
      /* Load the address. */
      switch (chan)
	 {
         case   DMA_CH0:
            WRITE_ISP_D8 (EISA_DMA_CH0_BASE_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    address >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH0_BASE_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    address >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH0_LOW_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    address >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH0_HIGH_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    break;
	 
	 case   DMA_CH1:
	    WRITE_ISP_D8 (EISA_DMA_CH1_BASE_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    address >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH1_BASE_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    address >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH1_LOW_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    address >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH1_HIGH_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    break;
	 
	 case   DMA_CH2:
	    {

#ifdef	DMA_DEBUG
	    int		data;
	    
	    printf("\t address = 0x%lx \n", address);
#endif  /* DMA_DEBUG */
	    WRITE_ISP_D8 (EISA_DMA_CH2_BASE_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    address >>= EISA_B8;
#ifdef	DMA_DEBUG
	    printf("\t address = 0x%lx \n", address);
#endif  /* DMA_DEBUG */
	    WRITE_ISP_D8 (EISA_DMA_CH2_BASE_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    address >>= EISA_B8;
#ifdef	DMA_DEBUG
	    printf("\t address = 0x%lx \n", address);
#endif  /* DMA_DEBUG */
	    WRITE_ISP_D8 (EISA_DMA_CH2_LOW_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    address >>= EISA_B8;
#ifdef	DMA_DEBUG
	    printf("\t address = 0x%lx \n", address);
#endif  /* DMA_DEBUG */
	    WRITE_ISP_D8 (EISA_DMA_CH2_HIGH_ADDR, address&EISA_BYTE_MASK);
	    mb();
#ifdef	DMA_DEBUG
	WRITE_ISP_D8 (EISA_DMA_CT1_CLR_BYTE_P, EISA_BYTE_MASK);
	mb();
	data = READ_ISP_D8 (EISA_DMA_CH2_BASE_ADDR);
	printf ("DMA ch2 base addr 1 = 0x%x \n", data & EISA_BYTE_MASK);
	data = READ_ISP_D8 (EISA_DMA_CH2_BASE_ADDR);
	printf ("DMA ch2 base addr 2 = 0x%x \n", data & EISA_BYTE_MASK);
	data = READ_ISP_D8 (EISA_DMA_CH2_LOW_ADDR);
	printf ("DMA ch2 base addr 3 = 0x%x \n", data & EISA_BYTE_MASK);
	data = READ_ISP_D8 (EISA_DMA_CH2_HIGH_ADDR);
	printf ("DMA ch2 base addr 4 = 0x%x \n", data & EISA_BYTE_MASK);
#endif  /* DMA_DEBUG */
	    break;
	    }
	 
	 case   DMA_CH3:
	    WRITE_ISP_D8 (EISA_DMA_CH3_BASE_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    address >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH3_BASE_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    address >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH3_LOW_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    address >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH3_HIGH_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    break;
	 
	 case   DMA_CH5:
	    WRITE_ISP_D8 (EISA_DMA_CH5_BASE_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    address >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH5_BASE_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    address >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH5_LOW_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    address >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH5_HIGH_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    break;
	 
	 case   DMA_CH6:
	    WRITE_ISP_D8 (EISA_DMA_CH6_BASE_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    address >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH6_BASE_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    address >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH6_LOW_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    address >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH6_HIGH_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    break;
	 
	 case   DMA_CH7:
	    WRITE_ISP_D8 (EISA_DMA_CH7_BASE_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    address >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH7_BASE_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    address >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH7_LOW_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    address >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH7_HIGH_ADDR, address&EISA_BYTE_MASK);
	    mb();
	    break;
	 
	 default:
	    panic ("Invalid DMA channel");/* Should never get here. */
	 }  /* End switch on channel. */

      /* Clear the byte pointer so we point to the correct one. */
      if (chan < DMA_CH4)
	 WRITE_ISP_D8 (EISA_DMA_CT1_CLR_BYTE_P, EISA_BYTE_MASK);
      else
	 WRITE_ISP_D8 (EISA_DMA_CT2_CLR_BYTE_P, EISA_BYTE_MASK);
      mb();

      /* Load the count registers. */
      switch (chan)
	 {
         case   DMA_CH0:
            WRITE_ISP_D8 (EISA_DMA_CH0_BASE_CNT, count&EISA_BYTE_MASK);
	    mb();
	    count >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH0_BASE_CNT, count&EISA_BYTE_MASK);
	    mb();
	    count >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH0_HIGH_CNT, count&EISA_BYTE_MASK);
	    mb();
	    break;
	 
	 case   DMA_CH1:
	    WRITE_ISP_D8 (EISA_DMA_CH1_BASE_CNT, count&EISA_BYTE_MASK);
	    mb();
	    count >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH1_BASE_CNT, count&EISA_BYTE_MASK);
	    mb();
	    count >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH1_HIGH_CNT, count&EISA_BYTE_MASK);
	    mb();
	    break;
	 
	 case   DMA_CH2:
	    {
	    
	    int		data;
	    
#ifdef	DMA_DEBUG
	    printf("\t count = 0x%lx \n", count);
#endif  /* DMA_DEBUG */
	    WRITE_ISP_D8 (EISA_DMA_CH2_BASE_CNT, count&EISA_BYTE_MASK);
	    mb();
	    count >>= EISA_B8;
#ifdef	DMA_DEBUG
	    printf("\t count = 0x%lx \n", count);
#endif  /* DMA_DEBUG */
	    WRITE_ISP_D8 (EISA_DMA_CH2_BASE_CNT, count&EISA_BYTE_MASK);
	    mb();
	    count >>= EISA_B8;
#ifdef	DMA_DEBUG
	    printf("\t count = 0x%lx \n", count);
#endif  /* DMA_DEBUG */
	    WRITE_ISP_D8 (EISA_DMA_CH2_HIGH_CNT, count&EISA_BYTE_MASK);
	    mb();
	    WRITE_ISP_D8 (EISA_DMA_CT1_CLR_BYTE_P, EISA_BYTE_MASK);
	    mb();
#ifdef	DMA_DEBUG
	data = READ_ISP_D8 (EISA_DMA_CH2_BASE_CNT);
	printf ("DMA ch2 count 1 = 0x%x \n", data & EISA_BYTE_MASK);
	data = READ_ISP_D8 (EISA_DMA_CH2_BASE_CNT);
	printf ("DMA ch2 count 2 = 0x%x \n", data & EISA_BYTE_MASK);
	data = READ_ISP_D8 (EISA_DMA_CH2_HIGH_CNT);
	printf ("DMA ch2 count 3 = 0x%x \n", data & EISA_BYTE_MASK);
#endif  /* DMA_DEBUG */

	    break;
	    }
	    
	 
	 case   DMA_CH3:
	    WRITE_ISP_D8 (EISA_DMA_CH3_BASE_CNT, count&EISA_BYTE_MASK);
	    mb();
	    count >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH3_BASE_CNT, count&EISA_BYTE_MASK);
	    mb();
	    count >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH3_HIGH_CNT, count&EISA_BYTE_MASK);
	    mb();
	    break;
	 
	 case   DMA_CH5:
	    WRITE_ISP_D8 (EISA_DMA_CH5_BASE_CNT, count&EISA_BYTE_MASK);
	    mb();
	    count >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH5_BASE_CNT, count&EISA_BYTE_MASK);
	    mb();
	    count >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH5_HIGH_CNT, count&EISA_BYTE_MASK);
	    mb();
	    break;
	 
	 case   DMA_CH6:
	    WRITE_ISP_D8 (EISA_DMA_CH6_BASE_CNT, count&EISA_BYTE_MASK);
	    mb();
	    count >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH6_BASE_CNT, count&EISA_BYTE_MASK);
	    mb();
	    count >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH6_HIGH_CNT, count&EISA_BYTE_MASK);
	    mb();
	    break;
	 
	 case   DMA_CH7:
	    WRITE_ISP_D8 (EISA_DMA_CH7_BASE_CNT, count & EISA_BYTE_MASK);
	    mb();
	    count >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH7_BASE_CNT, count & EISA_BYTE_MASK);
	    mb();
	    count >>= EISA_B8;
	    WRITE_ISP_D8 (EISA_DMA_CH7_HIGH_CNT, count & EISA_BYTE_MASK);
	    mb();
	    break;
	 
	 default:
	    panic ("Invalid DMA channel");/* Should never get here. */
	 }  /* End switch on channel. */

      /*------------------------------------------------------------*/
      /* If we have more than one buffer we will enable chaining    */
      /* then go back and load address and count for second buffer. */
      /*------------------------------------------------------------*/
      if (more)
	 {
	 more = FALSE;
	 if (chan < DMA_CH4)
	    WRITE_ISP_D8 (EISA_DMA_CT1_CHAIN, ISP_CHAIN_IRQ |
			    ISP_CHAIN_NRDY | ISP_CHAIN_ON | chan);
	 else
	    WRITE_ISP_D8 (EISA_DMA_CT2_CHAIN, ISP_CHAIN_IRQ |
			    ISP_CHAIN_NRDY | ISP_CHAIN_ON | chan);
	 /* Load new address and count. */
	 mb();
	 sgp = dma_get_next_sgentry (sglist_p);
#ifdef	DMA_DEBUG
         printf("\t Second ba-bc load into eisa dma engine \n");
   	 printf("\t Address of ba-bc pair from get_next = 0x%lx \n", sgp);
#endif  /* DMA_DEBUG */
	 address = (vm_offset_t)sgp->ba;
	 count = sgp->bc;
#ifdef	DMA_DEBUG
         printf("\t ba = 0x%lx \n", address);
         printf("\t bc = 0x%lx \n", count);
#endif  /* DMA_DEBUG */
	 }
      else
	 break;

      }  /* End address and count programming. */
   
   /* If we are using chaining turn it on. */
   if (chaining)
      if (chan < DMA_CH4)
	 WRITE_ISP_D8 (EISA_DMA_CT1_CHAIN, ISP_CHAIN_IRQ | ISP_CHAIN_RDY
			 | ISP_CHAIN_ON | chan); 
      else
	 WRITE_ISP_D8 (EISA_DMA_CT2_CHAIN, ISP_CHAIN_IRQ | ISP_CHAIN_RDY |
			 ISP_CHAIN_ON | (chan & EISA_B2_MASK)); 

   /* Everything is set up so unmask the channel and return. */
   if (chan < DMA_CH4)
      WRITE_ISP_D8 (EISA_DMA_CT1_SMASK, ISP_CLR_MASK | chan);
   else
      WRITE_ISP_D8 (EISA_DMA_CT2_SMASK, ISP_CLR_MASK |( chan & EISA_B2_MASK));
   mb();
   
   splx(s);

   return (0);

}  /* End isp_dma_load */



/***************************************************************************/
/*                                                                         */
/* isp_dma_unload  -  Unload resources used for DMA..			   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	int  isp_dma_unload (struct controller  *ctlr_p,	           */
/*			     sglist_t  sglist_p)			   */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	ctlr_p		Pointer to controller structure for the driver 	   */
/*			that is requesting use of the DMA channel.	   */
/*	sglist_p	Pointer to list of address, byte count pairs used  */
/*			for DMA transfer.				   */
/*                                                                         */
/*                                                                         */
/* DESCRIPTION								   */
/*	This function   */
/***************************************************************************/

int
isp_dma_unload (struct controller  *ctlr_p,  sglist_t  sglist_p)

{  /* Begin isp_dma_ */

   int			 chan;	/* Dma channel to be used. */
   struct dma            dma_p;

   get_config(ctlr_p, EISA_DMA, "",&dma_p,0);
   chan = dma_p.channel;
   
   /* Mask off channel */
   /* Everything is set up so unmask the channel and return. */

   if (chan < DMA_CH4)
      WRITE_ISP_D8 (EISA_DMA_CT1_SMASK, ISP_SET_MASK | chan);
   else
      WRITE_ISP_D8 (EISA_DMA_CT2_SMASK, ISP_SET_MASK |( chan & EISA_B2_MASK));
   mb();

   return(0);

}  /* End isp_dma_ */



/***************************************************************************/
/*                                                                         */
/* isp_dma_dealloc  -  Free resources allocated for DMA.		   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	int  isp_dma_dealloc (struct controller  *ctlr_p)	           */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	ctlr_p		Pointer to controller structure for the driver 	   */
/*			that is requesting use of the DMA channel.	   */
/*                                                                         */
/*                                                                         */
/* DESCRIPTION								   */
/*	This function   */
/***************************************************************************/

int
isp_dma_dealloc (struct controller  *ctlr_p)

{  /* Begin isp_dma_dealloc */

   struct  dma_softc	*scp;
   int			 chan;

   struct dma            dma_p;

   /* Find out which channel this requester wants to use. */
   get_config(ctlr_p, EISA_DMA, "",&dma_p,0);
   chan = dma_p.channel;
   
   /* Get the softc structure for this channel. */
   scp = &dma_softc[chan];
   
   /* Clear owner and mark free. */
   scp->allocated = FALSE;
   scp->ownerp = (struct controller  *)(0);


   return(0);

}  /* End isp_dma_dealloc */



/***************************************************************************/
/*                                                                         */
/* isp_dma_intr  -  DMA interrupt handler.				   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	int  isp_ma_intr (struct controller  *ctlr_p)		           */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	ctlr_p		Pointer to controller structure for the driver 	   */
/*			that is requesting use of the DMA channel.	   */
/*                                                                         */
/*                                                                         */
/* DESCRIPTION								   */
/*	This function   */
/***************************************************************************/

void
isp_dma_intr (struct controller  *ctlr_p)

{  /* Begin isp_dma_intr */

   int			 chan;	/* Dma channel to be used. */
   int			 value;
   int			 i;
   int			 more;
   long			 count;
   vm_offset_t		 address;
   
   struct  dma_softc	*scp;	/* Pointer to softc structure for channel. */
   sg_entry_t		 sgp;	/* Pointer to current address, count pair. */
   

#ifdef	DMA_DEBUG
   printf ("isp_dma_intr called. \n");
#endif  /* DMA_DEBUG */
   
   /* Find out which channel interrupted. */
   value = READ_ISP_D8(EISA_DMA_INTR_STATUS);
   for (i=0; i<=DMA_CH7; i++)
      if (value & 1<<i)
	 break;
   if (i<=DMA_CH7)
      {  /* Have an interrupt. */
      chan = i;
      scp = &dma_softc[chan];

      /* See if we have more. */
      sgp = dma_get_next_sgentry (scp->sglistp);
      if (sgp)
	 {  /* More buffers. */
	 address = (vm_offset_t)sgp->ba;
	 count = sgp->bc;
	 /* Setup for programming chaining */
	 if (chan < DMA_CH4)
	    WRITE_ISP_D8 (EISA_DMA_CT1_CHAIN, ISP_CHAIN_IRQ |
			    ISP_CHAIN_NRDY | ISP_CHAIN_ON | chan);
	 else
	    WRITE_ISP_D8 (EISA_DMA_CT2_CHAIN, ISP_CHAIN_IRQ |
			    ISP_CHAIN_NRDY | ISP_CHAIN_ON | chan);
	 mb();
	 /* Load new address and count. */
	 /* Clear the byte pointer so we point to the correct one. */
	 if (chan < DMA_CH4)
	    WRITE_ISP_D8 (EISA_DMA_CT1_CLR_BYTE_P, EISA_BYTE_MASK);
	 else
	    WRITE_ISP_D8 (EISA_DMA_CT2_CLR_BYTE_P, EISA_BYTE_MASK);
	 mb();
      
	 /* Load the address. */
	 switch (chan)
	    {
	    case   DMA_CH0:
               WRITE_ISP_D8 (EISA_DMA_CH0_BASE_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       address >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH0_BASE_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       address >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH0_LOW_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       address >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH0_HIGH_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       break;
	 
	    case   DMA_CH1:
	       WRITE_ISP_D8 (EISA_DMA_CH1_BASE_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       address >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH1_BASE_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       address >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH1_LOW_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       address >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH1_HIGH_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       break;
	 
	    case   DMA_CH2:
	       WRITE_ISP_D8 (EISA_DMA_CH2_BASE_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       address >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH2_BASE_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       address >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH2_LOW_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       address >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH2_HIGH_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       break;
	 
	    case   DMA_CH3:
	       WRITE_ISP_D8 (EISA_DMA_CH3_BASE_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       address >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH3_BASE_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       address >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH3_LOW_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       address >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH3_HIGH_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       break;
	 
	    case   DMA_CH5:
	       WRITE_ISP_D8 (EISA_DMA_CH5_BASE_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       address >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH5_BASE_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       address >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH5_LOW_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       address >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH5_HIGH_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       break;
	 
	    case   DMA_CH6:
	       WRITE_ISP_D8 (EISA_DMA_CH6_BASE_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       address >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH6_BASE_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       address >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH6_LOW_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       address >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH6_HIGH_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       break;
	 
	    case   DMA_CH7:
	       WRITE_ISP_D8 (EISA_DMA_CH7_BASE_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       address >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH7_BASE_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       address >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH7_LOW_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       address >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH7_HIGH_ADDR, address&EISA_BYTE_MASK);
	       mb();
	       break;
	 
	    default:
	       panic ("Invalid DMA channel");/* Should never get here. */
	    }  /* End switch on channel. */

	 /* Clear the byte pointer so we point to the correct one. */
	 if (chan < DMA_CH4)
	    WRITE_ISP_D8 (EISA_DMA_CT1_CLR_BYTE_P, EISA_BYTE_MASK);
	 else
	    WRITE_ISP_D8 (EISA_DMA_CT2_CLR_BYTE_P, EISA_BYTE_MASK);
	 mb();

	 /* Load the count registers. */
	 switch (chan)
	    {
	    case   DMA_CH0:
               WRITE_ISP_D8 (EISA_DMA_CH0_BASE_CNT, count&EISA_BYTE_MASK);
	       mb();
	       count >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH0_BASE_CNT, count&EISA_BYTE_MASK);
	       mb();
	       count >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH0_HIGH_CNT, count&EISA_BYTE_MASK);
	       mb();
	       break;
	 
	    case   DMA_CH1:
	       WRITE_ISP_D8 (EISA_DMA_CH1_BASE_CNT, count&EISA_BYTE_MASK);
	       mb();
	       count >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH1_BASE_CNT, count&EISA_BYTE_MASK);
	       mb();
	       count >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH1_HIGH_CNT, count&EISA_BYTE_MASK);
	       mb();
	       break;
	 
	    case   DMA_CH2:
	       WRITE_ISP_D8 (EISA_DMA_CH2_BASE_CNT, count&EISA_BYTE_MASK);
	       mb();
	       count >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH2_BASE_CNT, count&EISA_BYTE_MASK);
	       mb();
	       count >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH2_HIGH_CNT, count&EISA_BYTE_MASK);
	       mb();
	       break;
	 
	    case   DMA_CH3:
	       WRITE_ISP_D8 (EISA_DMA_CH3_BASE_CNT, count&EISA_BYTE_MASK);
	       mb();
	       count >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH3_BASE_CNT, count&EISA_BYTE_MASK);
	       mb();
	       count >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH3_HIGH_CNT, count&EISA_BYTE_MASK);
	       mb();
	       break;
	 
	    case   DMA_CH5:
	       WRITE_ISP_D8 (EISA_DMA_CH5_BASE_CNT, count&EISA_BYTE_MASK);
	       mb();
	       count >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH5_BASE_CNT, count&EISA_BYTE_MASK);
	       mb();
	       count >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH5_HIGH_CNT, count&EISA_BYTE_MASK);
	       mb();
	       break;
	 
	    case   DMA_CH6:
	       WRITE_ISP_D8 (EISA_DMA_CH6_BASE_CNT, count&EISA_BYTE_MASK);
	       mb();
	       count >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH6_BASE_CNT, count&EISA_BYTE_MASK);
	       mb();
	       count >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH6_HIGH_CNT, count&EISA_BYTE_MASK);
	       mb();
	       break;
	 
	    case   DMA_CH7:
	       WRITE_ISP_D8 (EISA_DMA_CH7_BASE_CNT, count & EISA_BYTE_MASK);
	       mb();
	       count >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH7_BASE_CNT, count & EISA_BYTE_MASK);
	       mb();
	       count >>= EISA_B8;
	       WRITE_ISP_D8 (EISA_DMA_CH7_HIGH_CNT, count & EISA_BYTE_MASK);
	       mb();
	       break;
	 
	    default:
	       panic ("Invalid DMA channel");/* Should never get here. */
	    }  /* End switch on channel. */
	 /* Indicate that everything is set up. */
	 if (chan < DMA_CH4)
	    WRITE_ISP_D8 (EISA_DMA_CT1_CHAIN, ISP_CHAIN_IRQ | ISP_CHAIN_NRDY
			    | ISP_CHAIN_OFF | chan); 
	 else
	    WRITE_ISP_D8 (EISA_DMA_CT2_CHAIN, ISP_CHAIN_IRQ | ISP_CHAIN_NRDY |
			    ISP_CHAIN_OFF | chan); 
	 mb();
	 } /* End more buffers. */
      else
	 { /* No more buffers turn chaining off. */
	 if (chan < DMA_CH4)
	    WRITE_ISP_D8 (EISA_DMA_CT1_CHAIN, ISP_CHAIN_IRQ | ISP_CHAIN_RDY
			    | ISP_CHAIN_ON | chan); 
	 else
	    WRITE_ISP_D8 (EISA_DMA_CT2_CHAIN, ISP_CHAIN_IRQ | ISP_CHAIN_RDY |
			    ISP_CHAIN_ON | chan); 
	 mb();
	 } /* End no more buffers. */
      
      }  /* End have an interrupt. */

   
   return;

}  /* End isp_dma_intr */




