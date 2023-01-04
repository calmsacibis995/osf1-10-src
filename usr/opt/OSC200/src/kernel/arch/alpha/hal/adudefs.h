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
 *	"@(#)adudefs.h	9.1	(ULTRIX/OSF)	10/21/91"
 */

/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * Definitions for ADU I/O ports
 *
 * Modification History: /sys/machine/alpha/adudefs.h
 *
 * 16-Jan-90 -- map (Mark Parenti)
 *	Update Ethernet constants to match reality.
 *
 * 30-Nov-90 -- Tim Burke
 *	Added IO register map & configuration data structure defs.
 *
 * 04-Oct-90 -- map
 *	Created this file for Alpha support.
 */

/*
 * Protect the file from multiple includes.
 */
#ifndef _ADUDEFS_HDR_
#define _ADUDEFS_HDR_

/* Network Definitions */

/* FLAG codes	*/
#define	ETHER_EMPTY		0
#define	ETHER_TRANSMIT_PACKET	1
#define	ETHER_INITIALIZE	2
#define	ETHER_RECEIVE_PACKET	3

/* STATUS codes	*/
#define	ETHER_OK		0
#define	ETHER_DEFER		1
#define	ETHER_REJECT		2
#define	ETHER_COLLISION		3
#define	ETHER_FRAMING		4
#define	ETHER_CRC		5
#define	ETHER_FIRMWARE		6
#define	ETHER_ERROR		0x00000010	/* All interesting errors */
                                                /* have this bit set      */
                                                /* ETHER_REJECT		  */
                                                /* ETHER_COLLISION	  */
                                                /* ETHER_FIRMWARE	  */
/* RXMODE codes	*/
#define	ETHER_NORMAL		0
#define	ETHER_PROMISCUOUS	1

/* Interrupt Register Definitions */
#define	ETHER_RE		0x00000001
#define	ETHER_TE		0x00000002

/* SCSI Definitions	 see io/scsi/alpha/adureg.h */

/* Serial Line Definitions */

/* TXMODE codes */
#define	SERIAL_NORMAL		0
#define	SERIAL_PAUSE		1
#define	SERIAL_FLUSH		2

/* Interrupt Register Definitions */
#define	SERIAL_RE		0x00000001
#define	SERIAL_TE		0x00000002

/*
 * ADU Register IO mapping
 */

/* 	SCSI Registers */

struct adu_scsiregs {
	u_long	*base;		/* Base of rings pointer	*/
	u_long	*icr;		/* Interrupt control register	*/
	u_long	*db;		/* Doorbell register		*/
};

/* 	Network Interface Registers */

struct adu_niregs {
	u_long	*base;		/* Base of rings pointer	*/
	u_long	*icr;		/* Interrupt control register	*/
	u_long	*db;		/* Doorbell register		*/
};

/* 	Serial line interface (console) Registers */

struct adu_consregs {
					/* Registers for line number 1  */
	u_long	*base1;		/* Base of rings pointer	*/
	u_long	*icr1;		/* Interrupt control register	*/
	u_long	*db1;		/* Doorbell register		*/
					/* Registers for line number 2  */
	u_long	*base2;		/* Base of rings pointer	*/
	u_long	*icr2;		/* Interrupt control register	*/
	u_long	*db2;		/* Doorbell register		*/
};

/*
 * IO Register map address offsets from the base (BB)
 */

#define ADU_NI_BASE		0x2100
#define ADU_SCSI_BASE		0x2120
#define ADU_CONS1_BASE		0x2140
#define ADU_CONS2_BASE		0x21A0
#define ADU_NI_ICR		0x2200
#define ADU_SCSI_ICR		0x2220
#define ADU_CONS1_ICR		0x2240
#define ADU_CONS2_ICR		0x22A0
#define ADU_NI_DB		0x6000
#define ADU_SCSI_DB		0x6020
#define ADU_CONS1_DB		0x6040
#define ADU_CONS2_DB		0x60A0


#define DELAY_FLAG 15

/*
 * ADU Configuration data structure definitions and associated defines.
 */
/*
 * The console processor builds an in memory configuration data structure.
 * The physical address of this data structure is stored in the HWRPB.
 */
struct adu_config {
	u_int   type;		/* Type of module in the TVBus slot */
	u_int   flags;		/* Status of the individual module  */
	union {			/* Type specific fields		    */
	    struct io_module_type {
		long	enet_addr; /* Ethernet Station Address */
		int     scsi_id;   /* SCSI id of the IO module */
	    } io;
	    struct storage_module_type {
		u_long	interleave_ba;  /* Base address of interleave group */
		u_long	interleave_pos; /* Position in interleave group     */
	    } storage;
	    char    padbytes[504];	/* Make total struct size 512	    */
	} type_specific;
};
/*
 * Configuration type codes
 */
#define ADU_TYPE_EMPTY		0	/* Empty			*/
#define ADU_TYPE_IO		1	/* IO Module			*/
#define ADU_TYPE_STORAGE	2	/* 64MB Storage Module		*/
#define ADU_TYPE_DC227		3	/* CPU DC227			*/
#define ADU_TYPE_DC228		4	/* CPU DC228			*/
/*
 * Configuration flag codes
 */
#define	ADU_FLAG_BROKEN		0x1	/* Broken			*/
#define	ADU_FLAG_CPU		0x2	/* Primary CPU (type 3 or 4)	*/
#define	ADU_FLAG_IO		0x4	/* Console (type 1)		*/

/*
 * Calls to adu_vector_alloc returns the interrupt request node and channel.
 * The low 4 bits are the interrupt request node and the next 5 bits are the
 * interrupt request channel.  These macros are used to extract the two fields.
 */
#define ADU_GETIRQNODE(cookie) (cookie & 0xf)
#define ADU_GETIRQCHAN(cookie) ((cookie >> 4) & 0x1f)

#endif /* _ADUDEFS_HDR_ */
