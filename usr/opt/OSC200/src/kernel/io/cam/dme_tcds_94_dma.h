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
 * @(#)$RCSfile: dme_tcds_94_dma.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/11/19 17:16:26 $
 */

#ifndef __DME_TCDS_H__
#define __DME_TCDS_H__ 1

/* ---------------------------------------------------------------------- */

/* dme_tcds_94_dma.h	Version 1.00			Oct. 15, 1992 */

/*  This file contains the definitions and data structures needed by
    the TCDS DME related files.

Modification History

	Version	  Date		Who	Reason

	1.00	10/15/92	jag	Created this file from the
					two include files for flamingo + 3min.

*/

/* ---------------------------------------------------------------------- */

#define	BASE_IOASIC_OFFSET	0x80000	/* sparce space offset to the ASIC */
#define SCSI1_OFFSET		0x200

#define SSR_DMADIR      0x00000080    /* Set this bit for write, 0 for read */
#define SSR_DMA0ENB	0x00000100    /* Set this bit to enable DMA */
#define SSR_DMA1ENB	0x00000200    /* Set this bit to enable DMA */

#define SSR_O           0x00002008    	/* IOASIC System support reg. */
#define	SIR_O           0x00000000	/* IOASIC Interrupt reg. */

#define IOA_S_DMAP_O	0x00002000

#define	SCSI0_DRDY	0x00010000	/* First C94 DREQ */
#define	SCSI1_DRDY	0x00020000
#define SCSI0_C94	0x00040000	/* C94 interrupt */
#define SCSI1_C94	0x00080000
#define SCSI0_PREF	0x00100000	/* C94 prefetched interrupt */
#define SCSI1_PREF	0x00200000
#define SCSI0_EDMA	0x00400000	/* ~ERR asserted during DMA */
#define SCSI1_EDMA	0x00800000
#define SCSI0_PAR	0x01000000	/* DB parity error */
#define SCSI1_PAR	0x02000000
#define SCSI0_DMAPAR	0x04000000	/* DMA buffer parity error */
#define SCSI1_DMAPAR	0x08000000	/* DMA buffer parity error */
#define SCSI0_TCPAR	0x10000000	/* TC DMA read data parity error */
#define SCSI1_TCPAR	0x20000000	/* TC DMA read data parity error */
#define SCSITCWDPAR	0x40000000	/* TC IO write data parity error */
#define SCSITCADPAR	0x80000000	/* TC IO address parity error */

#define SCSI_DRDY       0x00000004
#define SCSI_C94        0x00000200
#define SCSI_MERR       0x00020000
#define SCSI_OERR       0x00040000
#define SCSI_DBPL       0x00080000

#define IOA_ADDRMASK	0xfffffffc	/* Mask for the ASIC address format */

#define TCDS_ZONE_SIZE	4		/* Magic DMA/DME boundry number */

/* ---------------------------------------------------------------------- */

/* These two defines are used with the '94 counter registers. */

#define LOAD_94_COUNTER( d, c )			\
{						\
    (d)->sim94_tcmsb = ((c) & 0xFF00) >> 8;	\
    (d)->sim94_tclsb = ((c) & 0x00FF);		\
}

#define GET_94_COUNTER( d )	((((U32)(d)->sim94_tcmsb & 0xFF ) << 8 ) + \
                                    ((U32)(d)->sim94_tclsb & 0xFF ))

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* These defines are used in the error logging code. */

#define DME_TCDS_ERR_CNT	7	/* How many possible error entries */
#define DME_ERR_LOGDAT	0x00000002	/* log the DAT struct for the I/O */

/* ---------------------------------------------------------------------- */

/* JAG ADD COMMENTS ON THE COMMON USAGE */
#define DAT_TCDS_MAP	0x00010000	/* for signaling a TC map reg inuse */

/* JAG ADD COMMENTS ON THE OVERLAY OF THE GENERIC POINTERS */

#define dme_XXXX	(DME_DESCRIPTOR *)dme_ptr0
#define dme_mapbase	(void *)dme_ptr1

/* JAG - what about base to the mapping area ? */


#ifdef JAG_FOR_AWHILE
int
build_flam_map_frag_table( SIM_WS *sws, DME_FLAM_STRUCT *ldme );

int
build_flam_frag_table( SIM_WS *sws, DME_FLAM_STRUCT *ldme );

static int
flam_calc_table_entry( char *addr, U32 count );
static int
flam_calc_map_table_entry( char *addr, U32 count );
static int
build_flam_map_table_entry( SIM_WS *sws, DME_FLAM_TABLE *tent, U32 ecount,
		  char *addr, char *uadr, int dir );
static int
build_flam_table_entry( SIM_WS *sws, DME_FLAM_TABLE *tent, U32 ecount,
		  char *addr, char *uadr, int dir );
#endif /* JAG_FOR_AWHILE */
#endif
