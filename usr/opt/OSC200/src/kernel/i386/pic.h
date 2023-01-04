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
 *	@(#)$RCSfile: pic.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:19:30 $
 */ 
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


#define NINTR	0x10
#define	NPICS	0x02

/*
** The following are definitions used to locate the PICs in the system
*/

#if	AT386 || PS2
#define ADDR_PIC_BASE		0x20
#define OFF_ICW			0x00
#define OFF_OCW			0x01
#define SIZE_PIC		0x80
#else	AT386
#define ADDR_PIC_BASE		0xC0
#define OFF_ICW			0x00
#define OFF_OCW			0x02
#define SIZE_PIC		0x04
#endif	AT386


/*
** The following banks of definitions ICW1, ICW2, ICW3, and ICW4 are used
** to define the fields of the various ICWs for initialisation of the PICs 
*/

/*
**	ICW1				
*/

#define ICW_TEMPLATE		0x10

#define LEVL_TRIGGER		0x08
#define EDGE_TRIGGER		0x00
#define ADDR_INTRVL4		0x04
#define ADDR_INTRVL8		0x00
#define SINGLE__MODE		0x02
#define CASCADE_MODE		0x00
#define ICW4__NEEDED		0x01
#define NO_ICW4_NEED		0x00

/*
**	ICW2
*/

#if	AT386 || PS2
#define	PICM_VECTBASE		0x40 
#define PICS_VECTBASE		PICM_VECTBASE + 0x08
#else	AT386
#define	PICM_VECTBASE		0x20 
#define PICS_VECTBASE		PICM_VECTBASE + 0x40
#endif	AT386

/*
**	ICW3				
*/

#define SLAVE_ON_IR0		0x01
#define SLAVE_ON_IR1		0x02
#define SLAVE_ON_IR2		0x04
#define SLAVE_ON_IR3		0x08
#define SLAVE_ON_IR4		0x10
#define SLAVE_ON_IR5		0x20
#define SLAVE_ON_IR6		0x40
#define SLAVE_ON_IR7		0x80

#define I_AM_SLAVE_0		0x00
#define I_AM_SLAVE_1		0x01
#define I_AM_SLAVE_2		0x02
#define I_AM_SLAVE_3		0x03
#define I_AM_SLAVE_4		0x04
#define I_AM_SLAVE_5		0x05
#define I_AM_SLAVE_6		0x06
#define I_AM_SLAVE_7		0x07

/*
**	ICW4				
*/

#define SNF_MODE_ENA		0x10
#define SNF_MODE_DIS		0x00
#define BUFFERD_MODE		0x08
#define NONBUFD_MODE		0x00
#define I_AM_A_SLAVE		0x04
#define I_AM_A_MASTR		0x00
#define AUTO_EOI_MOD		0x02
#define NRML_EOI_MOD		0x00
#define I8086_EMM_MOD		0x01
#define SET_MCS_MODE		0x00

/*
**	OCW1				
*/
#define PICM_MASK		0xFF
#define	PICS_MASK		0xFF
/*
**	OCW2				
*/

#define NON_SPEC_EOI		0x20
#define SPECIFIC_EOI		0x60
#define ROT_NON_SPEC		0xa0
#define SET_ROT_AEOI		0x80
#define RSET_ROTAEOI		0x00
#define ROT_SPEC_EOI		0xe0
#define SET_PRIORITY		0xc0
#define NO_OPERATION		0x40

#define SEND_EOI_IR0		0x00
#define SEND_EOI_IR1		0x01
#define SEND_EOI_IR2		0x02
#define SEND_EOI_IR3		0x03
#define SEND_EOI_IR4		0x04
#define SEND_EOI_IR5		0x05
#define SEND_EOI_IR6		0x06
#define SEND_EOI_IR7		0x07
 
/*
**	OCW3				
*/

#define OCW_TEMPLATE		0x08
#define SPECIAL_MASK		0x40
#define MASK_MDE_SET		0x20
#define MASK_MDE_RST		0x00
#define POLL_COMMAND		0x04
#define NO_POLL_CMND		0x00
#define READ_NEXT_RD		0x02
#define READ_IR_ONRD		0x00
#define READ_IS_ONRD		0x01

