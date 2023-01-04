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
 * @(#)$RCSfile: dme_3min_94_dma.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/09/21 21:50:43 $
 */
#ifndef __DME_IOASIC_H__
#define __DME_IOASIC_H__ 1

#include <io/common/devdriver.h>

/* ---------------------------------------------------------------------- */

/* dme_3min_94_dma.h	Version 1.03			Nov. 06, 1991 */

/*  This file contains the definitions and data structures needed by
    the 3MIN DME related files.

Modification History

	Version	  Date		Who	Reason

	1.00    12/31/90        rps     Created this file.
	1.01	07/20/91	rps	Added mapping related entry to
					DMA_3MIN_TABLE.
	1.02    08/17/91        rps     Added 3MAX+/BIGMAX address data.
	1.03	11/06/91	jag	Removed the IOASIC address magic #'s
					now use the offset from the 94's csr.

*/
#define	BASE_IOASIC_OFFSET	0x2C0000	/* offset to the IO-ASIC */

#define SSR_MASK        0x00020200    /* DMA read, enable, C94 /RESET */
#define SSR_DMADIR      0x00040000    /* Set this bit for write, 0 for read */
#define SSR_DMAENB	0x00020000    /* Set this bit to enable DMA */

#define	SIR_MASK	0x000e0204
#define SCSI_SLOT_DATA	0x0000000e		/* must write upon init */

#define	DB0_O		0x000000c0		/* offset to data buffer 0 */
#define	DB1_O		0x000000d0		/* offset to data buffer 1 */
#define	DB2_O		0x000000e0		/* offset to data buffer 2 */
#define	DB3_O		0x000000f0		/* offset to data buffer 3 */
#define SSR_O           0x00000100    		/* IOASIC System support reg. */
#define	SIR_O           0x00000110		/* IOASIC Interrupt reg. */
#define SIMR_O          0x00000120		/* IOASIC Int. mask reg. */
#define SCSI_DMASLOT_O	0x00000170		/* IOASIC SCSI DMA slot reg. */
#define	SCSI_CTRL_O	0x000001b0		/* new SCSI control register */
#define SCSI_DATA0_O	0x000001c0		/* new data register-1 of 2 */
#define SCSI_DATA1_O	0x000001d0		/* 2 of 2 */

#define AND_O		0x00001000		/* and with TC data */
#define OR_O		0x00002000		/* or with TC data */

#define IOA_ADDRMASK	0x1ffffffc
#define CREG_BUSG_M	0x00000003		/* Byte usage mask (SCSI_CTRL */
#define CREG_DMA_M	0x00000004		/* Direction mask (1=write) */

#define IOA_S_DMAP_O	0x00000000
#define	IOA_S_DMABP_O	0x00000010

/*#define PMAZ_BA_CFG3 ASC_C3_T8 |  ASC_C3_ALTDMA  old ioasic */
#define PMAZ_BA_CFG3 	0

#define	SCSI_DRDY	0x00000004
#define SCSI_C94	0x00000200
#define SCSI_MERR	0x00020000
#define SCSI_OERR	0x00040000
#define SCSI_DBPL	0x00080000

#define DME_3MIN_TABLE_INCOMPLETE	0
#define DME_3MIN_TABLE_COMPLETE	        1
#define DME_3MIN_TABLE_RETRY		2

#define IOASIC_WRITE	        0
#define IOASIC_READ             1
#define IOASIC_UNDEF	        2

#define CTRL_UNDEFINED          -1
#define TARGET_UNDEFINED        -1
#define LUN_UNDEFINED           -1

#define LOAD_94_COUNTER(d,c)    { d->sim94_tcmsb = (c) & 0xff00 >> 8;     \
                                   d->sim94_tclsb = (c) & 0x00ff;         \
				   WBFLUSH(); }
#define GET_94_COUNTER(d)     ( ( ( (U32)d->sim94_tcmsb & 0xff ) << 8 ) + \
                                    ( (U32)d->sim94_tclsb & 0xff ) )
#define MAX_TABLE_ENTRIES    256

#define MAX_TABLE_SIZE	MAX_TABLE_ENTRIES*sizeof( DME_3MIN_TABLE )

#define MAX_CONTROLLER         4
#define MAX_TARGET             8
#define MAX_LUN                8

#define MAX_UNIT_COUNT         MAX_CONTROLLER * MAX_TARGET * MAX_LUN

/*  The following is related to the DAT table design */

typedef struct fragbuf
    {
    char        top[8];
    char        bot[8];
    } FRAGBUF;

typedef struct dme_3min_table
    {
    unsigned int length;	/* length of table entry */
    char        *addr;		/* pointer to table entry */
    char        *uadr;	        /* pointer to real buffer if local xfer */
    unsigned	*iadr;		/* address for IOASIC */
    char         completed;	/* entry completion status flag */
    char         dir;           /* DMA direction */
    } DME_3MIN_TABLE;

typedef struct dme_3min_struct
    {
    FRAGBUF            *frag_buffer;
    DME_3MIN_TABLE     *frag_table;
    int                frag_index;

    SIM_WS             *sim_ws;

    DME_DESCRIPTOR     *dme_desc;

    U32                xfer_size;             /* total xfer size */
    U32                xfer_current_count;    /* total xfer'ed so far */
    void               *user_buffer;          /* passed in pointer */
    unsigned int       direction;

    U32                init_xfer_size;        /* initial segment xfer size */
    void               *init_user_buffer;     /* and user segment buf */

    U32	               state;

    struct pte         *svapte;	              /* Sys Virt Address of1st PTE*/
    U32	               num_pte;	              /* # of PTE's allocated ent  */
    void               *sva;		      /* K0SEG Address of mapped buf*/
    int                ismapped;

    SCATTER_ELEMENT    sg;
    } DME_3MIN_STRUCT;

unsigned *ioa_addrcvt( SIM_WS *, char * );
void *backcvt( void * );
void ssr_dma_on( SIM_SOFTC *, int );
void ssr_dma_off( SIM_SOFTC * );
void set_ioasic_control( SIM_SOFTC *, int );
void dumphex( char *, unsigned int );

#endif














