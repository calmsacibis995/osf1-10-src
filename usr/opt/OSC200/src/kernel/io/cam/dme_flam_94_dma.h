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
 * @(#)$RCSfile: dme_flam_94_dma.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/09/21 21:51:05 $
 */
#ifndef __DME_TCDS_H__
#define __DME_TCDS_H__ 1

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
#define	BASE_IOASIC_OFFSET	0x80000	/* offset to the IO-ASIC (sparse space) */
#define SCSI1_OFFSET		0x200

#define SSR_DMADIR      0x00000080    /* Set this bit for write, 0 for read */
#define SSR_DMA0ENB	0x00000100    /* Set this bit to enable DMA */
#define SSR_DMA1ENB	0x00000200    /* Set this bit to enable DMA */

#define SSR_O           0x00002008    		/* IOASIC System support reg. */
#define	SIR_O           0x00000000		/* IOASIC Interrupt reg. */

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
#define SCSI0_TCPAR	0x10000000	/* Turbochannel DMA read data parity error */
#define SCSI1_TCPAR	0x20000000	/* Turbochannel DMA read data parity error */
#define SCSITCWDPAR	0x40000000	/* Turbochannel IO write data parity error */
#define SCSITCADPAR	0x80000000	/* Turbochannel IO address parity error */

#define SCSI_ERRMASK	(SCSI0_EDMA | SCSI1_EDMA | SCSI0_PAR | SCSI1_PAR | SCSI0_DMAPAR | SCSI1_DMAPAR | SCSI0_TCPAR | SCSI1_TCPAR | SCSITCWDPAR | SCSITCADPAR)

#define SCSI_DRDY       0x00000004
#define SCSI_C94        0x00000200
#define SCSI_MERR       0x00020000
#define SCSI_OERR       0x00040000
#define SCSI_DBPL       0x00080000

#define DME_FLAM_TABLE_INCOMPLETE	0
#define DME_FLAM_TABLE_COMPLETE	        1
#define DME_FLAM_TABLE_RETRY		2

#define IOASIC_WRITE	        0
#define IOASIC_READ             1
#define IOASIC_UNDEF	        2

#define LOAD_94_COUNTER(d,c)    { d->sim94_tcmsb = (c) & 0xff00 >> 8;     \
                                   d->sim94_tclsb = (c) & 0x00ff;         \
				   WBFLUSH(); }
#define GET_94_COUNTER(d)     ( ( ( (U32)d->sim94_tcmsb & 0xff ) << 8 ) + \
                                    ( (U32)d->sim94_tclsb & 0xff ) )
#define MAX_TABLE_ENTRIES    256

#define MAX_TABLE_SIZE	MAX_TABLE_ENTRIES*sizeof( DME_FLAM_TABLE )

#define MAX_CONTROLLER         nCAMBUS
#define MAX_TARGET             8
#define MAX_LUN                8

#define MAX_UNIT_COUNT         MAX_CONTROLLER * MAX_TARGET * MAX_LUN

/*  The following is related to the DAT table design */

typedef struct fragbuf
    {
    char        top[8];
    char        bot[8];
    } FRAGBUF;

typedef struct dme_flam_table
    {
    unsigned int length;	/* length of table entry */
    unsigned int ocount;	/* length of table entry */
    char        *addr;		/* pointer to table entry */
    char        *uadr;	        /* pointer to real buffer if local xfer */
    unsigned	iadr;		/* address for IOASIC */
    char         completed;	/* entry completion status flag */
    char         dir;           /* DMA direction */
    unsigned long base;
    } DME_FLAM_TABLE;

typedef struct dme_flam_struct
    {
    FRAGBUF            *frag_buffer;
    DME_FLAM_TABLE     *frag_table;
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
    int                channel;
    unsigned long base;

    SCATTER_ELEMENT    sg;
    } DME_FLAM_STRUCT;

static unsigned ioa_addrcvt( SIM_WS *, char * );
static void *backcvt( U32 );
static void ssr_dma_on( SIM_SOFTC *, int );
static void ssr_dma_off( SIM_SOFTC * );
extern void dumphex( char *, unsigned int );

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
#endif
