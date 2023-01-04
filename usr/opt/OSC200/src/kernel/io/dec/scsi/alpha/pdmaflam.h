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
 * OSF/1 Release 1.0
 */
/*
 * Derived from pdma3min.h	4.2	(ULTRIX)	2/25/91
 */

 /***********************************************************************
  * pdma3min.h	
  *
  * Modification History
  *
  * 10-May-91	Paul Grist
  *	Added base address of IOASIC on KN03 systems (3max+/bigmax).
  *
  ***********************************************************************/
#ifndef _PDMAFLAM_H_
#define _PDMAFLAM_H_

#define BASE_IOASIC     0x1c040000              /* 3min IOASIC Base Address */
#define BASE_IOASIC_K1  0xbc040000

#define KN03_BASE_IOASIC 0x1f840000             /* 3max+ IOASIC Base address */

#define CIR_CFG         0x00000c00	/* C94's out of reset, interrupt enabled for 0 */
#define CIR_DMA0ENB	0x00000100	/* Set this bit to enable DMA */
#define CIR_DMA1ENB	0x00000200	/* Set this bit to enable DMA */
#define IMER_CFG	0x0000000c	/* don't latch anything (yet) */
#define IMER0_INTR_ON	0x0004000c	/* turn interrupts on */
#define IMER0_INTR_OFF	0x00040000	/* turn interrupts on */
#define IMER_INTR_ON	0x00040004	/* turn interrupts on */
#define IMER_INTR_OFF	0x00000004	/* turn interrupts on */
#define IMER1_INTR_ON	0x0008000c	/* turn interrupts on */
#define IMER1_INTR_OFF	0x00080000	/* turn interrupts on */

#define DMIC_DMADIR     0x00000080	/* Set this bit for write, zero for read */

#define	IMER_MASK	0x00450000	/* listen to ~err, 'C94, and DREQ int's */

#define	CIR_O           0x00000000	/* IOASIC Interrupt reg. */
#define IMER_O          0x00000008	/* IOASIC Int. mask reg. */
#define IOA_S_DMAP_O	0x00002000	/* DMA address offset */
#define DMIC_O		0x00002008	/* DMA interrupt ctl offset */
#define DMA_UNALN0_O	0x00002010	/* DMA unaligned 0 offset */
#define DMA_UNALN1_O	0x00002018	/* DMA unaligned 1 offset */

#define	SCSI0_DRDY	0x00010000
#define	SCSI1_DRDY	0x00020000
#define SCSI0_C94	0x00040000
#define SCSI1_C94	0x00080000

#define DATEP_INCOMPLETE	0
#define DATEP_COMPLETE		1
#define DATEP_RETRY		2

/* dual SCSI ASIC interrupt control and status register
 */
typedef union scsi_cir {
    unsigned int cir_reg;
    struct {

    /* first 16 bits for status: read, write zero to clear */

    unsigned tc_io_addr_parity:1;	/* TURBOchannel IO address parity error */
    unsigned tc_io_write_parity:1;	/* TURBOchannel IO write data parity error */
    unsigned sc1_tc_readdata_parity:1;	/* scsi[1] TURBOchannel DMA read data parity error */
    unsigned sc0_tc_readdata_parity:1;	/* scsi[0] TURBOchannel DMA read data parity error */
    unsigned sc1_dmabuf_parity:1;	/* scsi[1] DMA buffer parity error */
    unsigned sc0_dmabuf_parity:1;	/* scsi[0] DMA buffer parity error */
    unsigned sc1_db_parity:1;		/* scsi[1] DB parity error */
    unsigned sc0_db_parity:1;		/* scsi[0] DB parity error */
    unsigned sc1_err:1;			/* scsi[1] ~err asserted during DMA*/
    unsigned sc0_err:1;			/* scsi[0] ~err asserted during DMA*/
    unsigned sc1_prefetch:1;		/* scsi[1] prefetch interrupt */
    unsigned sc0_prefetch:1;		/* scsi[0] prefetch interrupt */
    unsigned sc1_interrupt:1;		/* scsi[1] 53C94 interrupt */
    unsigned sc0_interrupt:1;		/* scsi[0] 53C94 interrupt */
    unsigned sc1_dreq:1;		/* scsi[1] 53C94 DMA request */
    unsigned sc0_dreq:1;		/* scsi[0] 53C94 DMA request */

    /* next 8 control the DMA engine and reset the C94's: r/w */

    unsigned tc_parity_test:1;		/* TURBOchannel parity test mode */
    unsigned db_parity_test:1;		/* DB parity test mode */
    unsigned sc1_dmabuf_par_test:1;	/* scsi[1] DMA buffer parity test mode */
    unsigned sc0_dmabuf_par_test:1;	/* scsi[0] DMA buffer parity test mode */

    /* write ZERO to assert reset, write ONE to de-assert */
    unsigned sc1_reset:1;		/* scsi[1] reset 53C94 */
    unsigned sc0_reset:1;		/* scsi[0] reset 53C94 */

    /* write zero to disable DMA, write one to enable */
    unsigned sc1_dma_enable:1;		/* scsi[1] DMA enable */
    unsigned sc0_dma_enable:1;		/* scsi[0] DMA enable */

    /* These are unused at this time: inputs are r/o, outputs r/w */

    unsigned gp_input_3:1;		/* general purpose input #3 */
    unsigned gp_input_2:1;		/* general purpose input #2 */
    unsigned gp_input_1:1;		/* general purpose input #1 */
    unsigned gp_input_0:1;		/* general purpose input #0 */
    unsigned gp_output_0:1;		/* general purpose output #3 */
    unsigned gp_output_1:1;		/* general purpose output #2 */
    unsigned gp_output_2:1;		/* general purpose output #1 */
    unsigned gp_output_3:1;		/* general purpose output #0 */
    } _bits;
} scsi_cir;

/* dual SCSI ASIC interrupt mask and enable register
 * these bits correspond one-to-one with those in the CIR above.
 * Two sections are provided: the first 16 bits are the "mask".
 * They determine which of the bits in the CIR may assert the
 * TURBOchannel ~int line.  The "enable" bits allow the various
 * conditions to be propogated to the CIR
 */
typedef union scsi_imer {
    unsigned int imer_reg;
    struct {
    unsigned m_tc_io_addr_parity:1;	/* TURBOchannel IO address parity error */
    unsigned m_tc_io_write_parity:1;	/* TURBOchannel IO write data parity error */
    unsigned m_sc1_tc_readdata_parity:1;	/* scsi[1] TURBOchannel DMA read data parity error */
    unsigned m_sc0_tc_readdata_parity:1;	/* scsi[0] TURBOchannel DMA read data parity error */
    unsigned m_sc1_dmabuf_parity:1;	/* scsi[1] DMA buffer parity error */
    unsigned m_sc0_dmabuf_parity:1;	/* scsi[0] DMA buffer parity error */
    unsigned m_sc1_db_parity:1;		/* scsi[1] DB parity error */
    unsigned m_sc0_db_parity:1;		/* scsi[0] DB parity error */
    unsigned m_sc1_err:1;			/* scsi[1] ~err asserted during DMA*/
    unsigned m_sc0_err:1;			/* scsi[0] ~err asserted during DMA*/
    unsigned m_sc1_prefetch:1;		/* scsi[1] prefetch interrupt */
    unsigned m_sc0_prefetch:1;		/* scsi[0] prefetch interrupt */
    unsigned m_sc1_interrupt:1;		/* scsi[1] 53C94 interrupt */
    unsigned m_sc0_interrupt:1;		/* scsi[0] 53C94 interrupt */
    unsigned m_sc1_dreq:1;		/* scsi[1] 53C94 DMA request */
    unsigned m_sc0_dreq:1;		/* scsi[0] 53C94 DMA request */

    unsigned tc_io_addr_parity:1;	/* TURBOchannel IO address parity error */
    unsigned tc_io_write_parity:1;	/* TURBOchannel IO write data parity error */
    unsigned sc1_tc_readdata_parity:1;	/* scsi[1] TURBOchannel DMA read data parity error */
    unsigned sc0_tc_readdata_parity:1;	/* scsi[0] TURBOchannel DMA read data parity error */
    unsigned sc1_dmabuf_parity:1;	/* scsi[1] DMA buffer parity error */
    unsigned sc0_dmabuf_parity:1;	/* scsi[0] DMA buffer parity error */
    unsigned sc1_db_parity:1;		/* scsi[1] DB parity error */
    unsigned sc0_db_parity:1;		/* scsi[0] DB parity error */
    unsigned sc1_err:1;			/* scsi[1] ~err asserted during DMA*/
    unsigned sc0_err:1;			/* scsi[0] ~err asserted during DMA*/
    unsigned sc1_prefetch:1;		/* scsi[1] prefetch interrupt */
    unsigned sc0_prefetch:1;		/* scsi[0] prefetch interrupt */
    unsigned sc1_interrupt:1;		/* scsi[1] 53C94 interrupt */
    unsigned sc0_interrupt:1;		/* scsi[0] 53C94 interrupt */
    unsigned sc1_dreq:1;		/* scsi[1] 53C94 DMA request */
    unsigned sc0_dreq:1;		/* scsi[0] 53C94 DMA request */
    } _bits;
} scsi_imer;

#define sc0_err _bits.sc0_err
#define sc0_dreq _bits.sc0_dreq
#define sc0_interrupt _bits.sc0_interrupt


/* dual SCSI ASIC DMA interrupt control regitser
 */
typedef union scsi_dmic {
    unsigned int dmic_bits;
    struct {
    unsigned char d_asc_intr;		/* 53C94 interrupt register */
    unsigned char d_asc_fifoflags;	/* 53C94 FIFO flags register */
    unsigned char d_asc_status;		/* 53C94 status register */
    unsigned dm_direction:1;		/* DMA direction: 0 = read, 1 = write */
    unsigned d_prefetch_enb:1;		/* DMA read prefetch enable */
    unsigned d_reserved:4;
    unsigned dm_dma_addr_10:2;		/* low-order two bits of DMA addr */
    } _bits;
} scsi_dmic;
#define d_direction _bits.dm_direction
#define d_dma_addr _bits.dm_dma_addr

/* for non-longword aligned reads (writes to memory), the
 * start of an unaligned xfer appears in the unaligned data zero
 * register, in proper byte order.  The mask contains reflects
 * which bytes are valid, so state does not have to be kept.
 * The trailing part of an unaligned xfer is in the unaligned
 * data 1 register
 */
typedef struct scsi_unalgn_0 {
    unsigned char byte11;		/* byte [11] */
    unsigned char byte10;		/* byte [10] */
    unsigned char byte01;		/* byte [01] */
    unsigned zero:4;
    unsigned mask:3;			/* valid bits */
}scsi_unaln_0;

typedef struct scsi_unalgn_1 {
    unsigned zero:4;
    unsigned mask:3;			/* valid bits */
    unsigned char byte01;		/* byte [01] */
    unsigned char byte10;		/* byte [10] */
    unsigned char byte11;		/* byte [11] */
}scsi_unaln_1;

typedef struct ioa_s_dmap {
    unsigned            addr;
    } IOA_S_DMAP ;

typedef struct ioa_s_dmabp
    {
    unsigned            addr;
    } IOA_S_DMABP ;

/*  The following is related to the DAT table design */

typedef struct fragbuf
    {
    char        top[8];
    char        bot[8]; 
    }FRAGBUF;

typedef struct dtent
    {
    unsigned int length;	/* length of table entry */
    char        *addr;		/* pointer to table entry */
    char        *uadr;		/* pointer to real buffer if local xfer */
    unsigned	iadr;		/* address for IOASIC */
    char         completed;	/* entry completion status flag */
    char         dir;		/* DMA direction */
    unsigned	odd:2;		/* bits [1-0] */
    } DTENT;

#define IOASIC_WRITE	0
#define IOASIC_READ	1
#define IOASIC_UNDEF	2

#define MAX_DATTBL_ENT    256

#define DATTBL_SIZ	MAX_DATTBL_ENT*sizeof(DTENT)

extern int printstate;

extern int pmaz_ba_init();
extern int pmaz_ba_setup();
extern int pmaz_ba_start();
extern int pmaz_ba_cont();
extern int pmaz_ba_end();
extern int pmaz_ba_flush();
extern int bcopy();
extern int bzero();
unsigned ioa_addrcvt ( char * );
unsigned long backcvt( unsigned int );
int frag_buf_load( PDMA * );
flushfifo( struct sz_softc * );
ioasicint( struct sz_softc *, int, int );
int is_local_xfer( PDMA * );
dumptbl( DTENT * );
dumpent( DTENT * );
blddatent( DTENT *, unsigned long, char *, char *, int );
blddattbl( int, PDMA *, unsigned long, char *, int );
int caldatent( char *, long );
flush_fragbuf( PDMA *, int );
dmapload( struct sz_softc *, int, DTENT *);
ssrpload( struct sz_softc *, int );
dumphex( char *, unsigned );
int getdbuffer( struct sz_softc *, void * );
int getdbcount( struct sz_softc *, int );
int flushdb( struct sz_softc *, DTENT *, int );

#endif
