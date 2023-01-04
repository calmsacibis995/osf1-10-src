
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
static char *rcsid = "@(#)$RCSfile: 82357_pic.c,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/12/09 20:24:45 $";
#endif


/*****************************************************************************
 *
 *	82357_pic.c
 *
 * 	82357 Programmable Interrupt Controller code.
 *	To be used by systems that use the 82357
 *	PIC chip for I/O interrupts or other
 *	purposes.
 *
 *
 ****************************************************************************/

#include <sys/types.h>
#include <io/common/devdriver.h>
#include <hal/82357_pic.h>

#define WRITE_PIC(a,d) WRITE_BUS_D8((pic_base+a),d)
#define READ_PIC(a)    READ_BUS_D8((pic_base+a))

/*
 *  knxx files must initialize pic_base to use 82357 code
 */

io_handle_t pic_base;


/***********************************
 *
 *  Intel 82357-compatible PIC code
 *
 ***********************************/

/**************************************************************************
 *
 * pic_eoi()
 *
 * Jensen 82357 PIC eoi sends a Specific-EOI interrupt command 
 * to the PIC, which enable this irq level for another interrupt.
 *
 *	psgfix: This is moving to PALcode
 *
 **************************************************************************/
int pic_eoi( int irq )
{
    switch( irq){
      case PIC_IRQ15:
      case PIC_IRQ14:
      case PIC_IRQ13:   		/* DMA chaining interrupts */
      case PIC_IRQ12:
      case PIC_IRQ11:
      case PIC_IRQ10:
      case PIC_IRQ9:
	/* eoi to cntrl 2 */
	WRITE_PIC(CTRL2_OCW2,SPEC_EOI | (irq&0x7));  
      case PIC_IRQ7:
      case PIC_IRQ6:
      case PIC_IRQ5:
      case PIC_IRQ4:
      case PIC_IRQ3:
      case PIC_IRQ1:
      case PIC_IRQ0:
	/* eoi to cntrl 1 */
	WRITE_PIC(CTRL1_OCW2,SPEC_EOI|
			    ((irq>PIC_IRQ7) ? 2:irq));
	break;
      default:
	printf("eoi: unrecognized IRQ = %d\n", irq );
    }
}

/************************************************************************ 
 *
 * irq_to_scb_vector()
 *
 * Jensen irq-channel to scb vector
 * routine.
 * 
 * returns the jensen-specific scb vector for the IRQ level.
 * Available for bus code to load SCB vector for EISA slot IRQs.
 *
 *************************************************************************/
int irq_to_scb_vector( int irq )
{
    switch( irq ){
      case PIC_IRQ0:
	return( PIC_SCB_IRQ0 );
      case PIC_IRQ1:
	return( PIC_SCB_IRQ1 );
      case PIC_IRQ2:
	return( PIC_SCB_IRQ2 );
      case PIC_IRQ3:
	return( PIC_SCB_IRQ3 );
      case PIC_IRQ4:
	return( PIC_SCB_IRQ4 );
      case PIC_IRQ5:
	return( PIC_SCB_IRQ5 );
      case PIC_IRQ6:
	return( PIC_SCB_IRQ6 );
      case PIC_IRQ7:
	return( PIC_SCB_IRQ7 );
      case PIC_IRQ8:
	return( PIC_SCB_IRQ8 );
      case PIC_IRQ9:
	return( PIC_SCB_IRQ9 );
      case PIC_IRQ10:
	return( PIC_SCB_IRQ10 );
      case PIC_IRQ11:
	return( PIC_SCB_IRQ11 );
      case PIC_IRQ12:
	return( PIC_SCB_IRQ12 );
      case PIC_IRQ13:
	return( PIC_SCB_IRQ13 );
      case PIC_IRQ14:
	return( PIC_SCB_IRQ14 );
      case PIC_IRQ15:
	return( PIC_SCB_IRQ15 );

      default:
	printf("82357 pic: unsupported IRQ assignment = %d\n", irq);
    }

}

#ifdef PSG_DEBUG
check_pic()
{
    int c1_irr, c2_irr;

    WRITE_PIC(CTRL2_OCW3,READ_IR);   /*CTLR 2*/ 
    mb();
    c2_irr = READ_PIC(CTRL2_OCW3); 

    WRITE_PIC(CTRL1_OCW3,READ_IR);   /*CTLR 1*/ 
    mb();
    c1_irr = READ_PIC(CTRL1_OCW3); 

    printf("PIC IRR status, CTRL2 = 0x%x\n\n", c2_irr);
    printf("\nPIC IRR status, CTRL1 = 0x%x\n", c1_irr);
}
#endif PSG_DEBUG


/***************************************************************************
 *
 * pic_enable_irq
 *
 * Jensen 82357-specific interrupt enable
 * enable interrupts in 82357 PIC for this IRQ
 *
 *
 ***************************************************************************/
pic_enable_irq(int irq)
{
    int ctrl1_ocw1, ctrl2_ocw1, ipl;

    ipl = splextreme();

    ctrl1_ocw1 = READ_PIC(CTRL1_OCW1); 
    ctrl2_ocw1 = READ_PIC(CTRL2_OCW1); 

    switch( irq ){
      case PIC_IRQ15:
      case PIC_IRQ14:
      case PIC_IRQ13:		/* DMA interrupts */
      case PIC_IRQ12:
      case PIC_IRQ11:
      case PIC_IRQ10:
      case PIC_IRQ9:
	ctrl2_ocw1 &= ~(1<<(irq-8));   
      case PIC_IRQ7:
      case PIC_IRQ6:
      case PIC_IRQ5:
      case PIC_IRQ4:
      case PIC_IRQ3:
      case PIC_IRQ1:
      case PIC_IRQ0:
	ctrl1_ocw1 &= ~( 1 << ((irq>7)?2:irq));
	break;
      default:
	printf( "undefined IRQ  = %d\n", irq );
    }

    ctrl2_ocw1 &= 0xff;
    ctrl1_ocw1 &= 0xff;
   
    WRITE_PIC(CTRL2_OCW1,ctrl2_ocw1 ); /*CTLR 2*/ 
    WRITE_PIC(CTRL1_OCW1,ctrl1_ocw1 ); /*CTLR 1*/
    
    splx(ipl);
}

/***************************************************************************
 *
 * pic_disable_irq
 *
 * Jensen/Theta2 82357-specific interrupt disable
 * disable interrupts in 82357 PIC for this IRQ
 *
 *
 ***************************************************************************/
pic_disable_irq(int irq)
{
    int ctrl1_ocw1, ctrl2_ocw1, ipl;

    ipl = splextreme();

    ctrl1_ocw1 = READ_PIC(CTRL1_OCW1); 
    ctrl2_ocw1 = READ_PIC(CTRL2_OCW1); 

    switch( irq )
	{
	  case PIC_IRQ15:
	  case PIC_IRQ14:
	  case PIC_IRQ13:		/* DMA interrupts */
	  case PIC_IRQ12:
	  case PIC_IRQ11:
	  case PIC_IRQ10:
	  case PIC_IRQ9:
	    ctrl2_ocw1 |= (1<<(irq-8));   
	    break;

	  case PIC_IRQ7:
	  case PIC_IRQ6:
	  case PIC_IRQ5:
	  case PIC_IRQ4:
	  case PIC_IRQ3:
	  case PIC_IRQ1:
	  case PIC_IRQ0:
	    ctrl1_ocw1 |= (1 << irq);
	    break;

	  default:
	    printf("pic_disable_irq: No disable support for IRQ%d\n", 
		   irq );
    }

    ctrl2_ocw1 &= 0xff;
    ctrl1_ocw1 &= 0xff;
   
    WRITE_PIC(CTRL2_OCW1,ctrl2_ocw1 ); /*CTLR 2*/ 
    WRITE_PIC(CTRL1_OCW1,ctrl1_ocw1 ); /*CTLR 1*/
    
    splx(ipl);
}

/****************************************************************************
 *
 * pic_set_irq_edge
 *
 * Jensen/Theta2 82357-specific ELR routine to set interrupt 
 * to be edge-triggered for the irq passed as parameter.
 *
 * The following must always be Edge-triggered: 13,8,2,1,0
 * 
 *****************************************************************************/

pic_set_irq_edge(int irq)
{
    int elr1, elr2, ipl;

    ipl = splextreme();

    elr2 = READ_PIC(PIC_ELR2); 
    elr1 = READ_PIC(PIC_ELR1); 

    switch( irq )
	{
	  case PIC_IRQ15:
	  case PIC_IRQ14:
	  case PIC_IRQ13:
	  case PIC_IRQ12:
	  case PIC_IRQ11:
	  case PIC_IRQ10:
	  case PIC_IRQ9:
	    elr2 &= ~(1<<(irq-8));
	    break;

	  case PIC_IRQ7:
	  case PIC_IRQ6:
	  case PIC_IRQ5:
	  case PIC_IRQ4:
	  case PIC_IRQ3:
	  case PIC_IRQ2:
	  case PIC_IRQ1:
	  case PIC_IRQ0:
	    elr1 &= ~(1 << irq);
	    break;

	  default:
	    printf( "Set IRQ%d to edge-trigger not supported\n",irq );
    }

    elr2 &= 0xff;
    elr1 &= 0xff;
   
    WRITE_PIC(PIC_ELR2,elr2 ); /*ELR 2*/ 
    WRITE_PIC(PIC_ELR1,elr1 ); /*ELR 1*/
    
    splx(ipl);
}


/*****************************************************************************
 *
 * pic_set_irq_level
 *
 * Jensen/Theta2 82357-specific ELR routine to set PIC interrupt 
 * to be level-triggered for the irq passed as parameter.
 *
 * The following must always be Edge-triggered: 13,8,2,1,0
 *
 *
 *****************************************************************************/


pic_set_irq_level(int irq)
{
    int elr1, elr2, ipl;

    ipl = splextreme();

    elr2 = READ_PIC(PIC_ELR2); 
    elr1 = READ_PIC(PIC_ELR1); 

    switch( irq )
	{
	  case PIC_IRQ15:
	  case PIC_IRQ14:
	  case PIC_IRQ12:
	  case PIC_IRQ11:
	  case PIC_IRQ10:
	  case PIC_IRQ9:
	    elr2 |= (1<<(irq-8));   
	    break;

	  case PIC_IRQ7:
	  case PIC_IRQ6:
	  case PIC_IRQ5:
	  case PIC_IRQ4:
	  case PIC_IRQ3:
	    elr1 |= (1 << irq);
	    break;

	  default:
	    printf("level-trigger not supported for IRQ%d\n", irq );
    }

    elr2 &= 0xff;
    elr1 &= 0xff;
   
    WRITE_PIC(PIC_ELR2,elr2 ); /*CTLR 2*/ 
    WRITE_PIC(PIC_ELR1,elr1 ); /*CTLR 1*/
    
    splx(ipl);
}

/********************************************************
 *
 *   End of 82357 PIC routines
 *
 ********************************************************/

