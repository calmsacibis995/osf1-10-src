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
static char	*sccsid = "@(#)$RCSfile: pic.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:19:27 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/* 
 * OSF/1 Release 1.0
 */

/*
**
**  Copyright (c) 1988 Prime Computer, Inc.  Natick, MA 01760
**  All Rights Reserved
**
**  This is UNPUBLISHED PROPRIETARY SOURCE CODE of Prime Computer, Inc.
**  Inclusion of the above copyright notice does not evidence any actual
**  or intended publication of such source code.
*/


#include <sys/types.h>
#include <i386/ipl.h>
#include <i386/pic.h>

/*
 * PS2 references pic_mask[0] ... pic_mask[SPLHI] while other ports only reference
 * up to SPLHI-1.
 */
ushort 		pic_mask[SPLHI+1] = {
/* spl0 */ 0,
/* spl1 */ 0,
/* spl2 */ 0,
/* spl3 */ 0,
/* spl4 */ 0,
/* spl5 */ 0,
/* spl6 */ 0,
/* spl7 */ 0,
/* SPLHI */ 0,
};

int		curr_ipl;
ushort		curr_pic_mask;

int 		nintr = NINTR;
int		npics = NPICS;

char		*master_icw, *master_ocw, *slaves_icw, *slaves_ocw;

ushort PICM_ICW1, PICM_OCW1, PICS_ICW1, PICS_OCW1 ;
ushort PICM_ICW2, PICM_OCW2, PICS_ICW2, PICS_OCW2 ;
ushort PICM_ICW3, PICM_OCW3, PICS_ICW3, PICS_OCW3 ;
ushort PICM_ICW4, PICS_ICW4 ;

/*
** picinit() - This routine 
**		* Establishes a table of interrupt vectors
**		* Establishes a table of interrupt priority levels
**		* Establishes a table of interrupt masks to be put
**			in the PICs.
**		* Establishes location of PICs in the system 
**		* Initialises them
**
**	At this stage the interrupt functionality of this system should be 
**	coplete.
**
*/


/*
** 1. First we form a table of PIC masks - rather then calling form_pic_mask()
**	each time there is a change of interrupt level - we will form a table
**	of pic masks, as there are only 7 interrupt priority levels.
**
** 2. The next thing we must do is to determine which of the PIC interrupt
**	request lines have to be masked out, this is done by calling 
**	form_pic_mask() with a (int_lev) of zero, this will find all the 
**	interrupt lines that have priority 0, (ie to be ignored).
**	Then we split this up for the master/slave PICs.
**
** 2. Initialise the PICs , master first, then the slave.
**	All the register field definitions are described in pic_jh.h, also
**	the settings of these fields for the various registers are selected.
**
*/

picinit()
{

	ushort i;

#if PS2
	inline_cli();
#else
	asm("cli");
#endif

	/*
	** 1. Form pic mask table for startup from static info.
	**
	** note: pic_mask[0]...pic_mask[SPLHI+1] are used on PS2 while other
	** ports may only use pic_mask[0]...pic_mask[SPLHI].
	**/
#if PS2
	for (i=SPL0; i <= SPLHI; i++)
		pic_mask[i] = 0xffff;	/* force all interrupts to be masked */
#else
	form_pic_mask();
#endif

	/*
	** 1a. Select current SPL.
	*/

	curr_ipl = SPLHI;
	curr_pic_mask = pic_mask[SPLHI];

	/*
	** 2. Generate addresses to each PIC port.
	*/

	master_icw = (char *)(ADDR_PIC_BASE + OFF_ICW);
	master_ocw = (char *)(ADDR_PIC_BASE + OFF_OCW);
	slaves_icw = (char *)(ADDR_PIC_BASE + OFF_ICW + SIZE_PIC);
	slaves_ocw = (char *)(ADDR_PIC_BASE + OFF_OCW + SIZE_PIC);

#ifndef PS2		 /* PS/2 initializes these at boot time using BIOS */
	/*
	** 3. Select options for each ICW and each OCW for each PIC.
	*/

	PICM_ICW1 = 
 	(ICW_TEMPLATE | EDGE_TRIGGER | ADDR_INTRVL8 | CASCADE_MODE | ICW4__NEEDED);

	PICS_ICW1 = 
 	(ICW_TEMPLATE | EDGE_TRIGGER | ADDR_INTRVL8 | CASCADE_MODE | ICW4__NEEDED);

	PICM_ICW2 = PICM_VECTBASE;
	PICS_ICW2 = PICS_VECTBASE;

#ifdef	AT386
	PICM_ICW3 = ( SLAVE_ON_IR2 );
	PICS_ICW3 = ( I_AM_SLAVE_2 );
#endif
#ifdef	iPC2
	PICM_ICW3 = ( SLAVE_ON_IR7 );
	PICS_ICW3 = ( I_AM_SLAVE_7 );
#endif

#ifdef  iPSC2
	/* Use Buffered mode for iPSC2 */
        PICM_ICW4 = (SNF_MODE_DIS | BUFFERD_MODE | I_AM_A_MASTR |
                     NRML_EOI_MOD | I8086_EMM_MOD);
        PICS_ICW4 = (SNF_MODE_DIS | BUFFERD_MODE | I_AM_A_SLAVE |
                     NRML_EOI_MOD | I8086_EMM_MOD);
#else
	PICM_ICW4 = 
 	(SNF_MODE_DIS | NONBUFD_MODE | NRML_EOI_MOD | I8086_EMM_MOD);
	PICS_ICW4 = 
 	(SNF_MODE_DIS | NONBUFD_MODE | NRML_EOI_MOD | I8086_EMM_MOD);
#endif

	PICM_OCW1 = (curr_pic_mask & 0x00FF);
	PICS_OCW1 = ((curr_pic_mask & 0xFF00)>>0x08);

	PICM_OCW2 = SPECIFIC_EOI;
	PICS_OCW2 = SPECIFIC_EOI;

	PICM_OCW3 = (OCW_TEMPLATE | READ_NEXT_RD | READ_IR_ONRD );
	PICS_OCW3 = (OCW_TEMPLATE | READ_NEXT_RD | READ_IR_ONRD );


	/* 
	** 4.	Initialise master - send commands to master PIC
	*/ 

	outb ( master_icw, PICM_ICW1 );
	outb ( master_ocw, PICM_ICW2 );
	outb ( master_ocw, PICM_ICW3 );
	outb ( master_ocw, PICM_ICW4 );

	outb ( master_ocw, PICM_MASK );
	outb ( master_icw, PICM_OCW3 );

	/*
	** 5.	Initialise slave - send commands to slave PIC
	*/

	outb ( slaves_icw, PICS_ICW1 );
	outb ( slaves_ocw, PICS_ICW2 );
	outb ( slaves_ocw, PICS_ICW3 );
	outb ( slaves_ocw, PICS_ICW4 );


	outb ( slaves_ocw, PICS_OCW1 );
	outb ( slaves_icw, PICS_OCW3 );

	/*
	** 6. Initialise interrupts
	*/
	outb ( master_ocw, PICM_OCW1 );
#endif /* ! PS2 */

#if 0
	printf(" spl set to %x pic_mask set to %x \n", curr_ipl, curr_pic_mask);
#endif

}


/*
** form_pic_mask() 
**
** Thie routine sets up the masks for each spl#() to set the pics. At
** initialization, a driver places its SPL# in the intpri array at its
** pic line index, then calls form_pic_mask(), which updates the pic_masks.
**
*/

#if	AT386 || PS2
#define SLAVEMASK       (0xFFFF ^ SLAVE_ON_IR2)
#endif
#ifdef  iPSC2
#define SLAVEMASK       (0xFFFF ^ SLAVE_ON_IR7)
#endif

#define SLAVEACTV       0xFF00

extern u_char intpri[];

form_pic_mask()
{
        int i, j, bit, mask;

        for (i = SPL0; i < sizeof pic_mask / sizeof pic_mask[0]; i++) {
		bit = 0x0001;
		for (j = mask = 0; j < NINTR; ++j) {
                        if (intpri[j] <= i)
                                mask |= bit;
			bit <<= 1;
		}
                if ((mask & SLAVEACTV) != SLAVEACTV )
                        mask &= SLAVEMASK;

                pic_mask[i] = mask;
        }
}

/*
 * Stubs for unitialized interrupt vectors. The interrupt
 * dispatcher will likely determine spurious on return.
 */
intnull(unit_dev)
{
	return 0;
}

int prtnull_count = 0;
prtnull(unit)
{
        ++prtnull_count;
	return 0;
}


#if PS2
/*
 * warning: only works for 16 interrupt levels!
 */
int_enable(int_number,int_lvl)
	unsigned short int_number,int_lvl;
{
	unsigned short i;
	unsigned short mask = 1 << int_number;

	if (int_number > 7) {
		mask |= 4;
	}
	for (i=SPL0; i < int_lvl; i++) {
		pic_mask[i] &= ~mask;
	}
}

int_disable(int_number)
{
	unsigned short i;
	unsigned short bit = 1 << int_number;

	for (i=SPL0; i <= SPLHI; i++) {
		pic_mask[i] |= bit;
 		if ((pic_mask[i] & SLAVEACTV) == SLAVEACTV ) {
			pic_mask[i] |= 0x4;
		}
	}
}

#endif /* PS2 */
