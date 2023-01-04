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
 * @(#)$RCSfile: tcds_adapt.h,v $ $Revision: 1.1.10.2 $ (DEC) $Date: 1993/11/19 21:27:12 $
 */

#ifndef __TCDS_ADPT_H__
#define __TCDS_ADPT_H__ 1

/* ---------------------------------------------------------------------- */

/* tcds_adapt.h		Version 1.00			Nov. 06, 1991 */

/*  This file contains the definitions and data structures needed by
    the TurboChannel Dual SCSI adapter routines.

Modification History

	Version	  Date		Who	Reason

	1.00    10/21/92        jag     Created this file.

*/
/* ---------------------------------------------------------------------- */
/*              CIR bit definitions                                       */
/* ---------------------------------------------------------------------- */

#define	BASE_IOASIC_OFFSET	0x80000	/* offset to the ASIC (sparse space) */
#define SCSI1_OFFSET		0x200

#define INP0_DUAL	0x00000010	/* General purpose input bits ... */
#define INP1_XXXX	0x00000020
#define INP2_FAST	0x00000040
#define INP3_XXXX	0x00000080	/* ... 0-3 */

#define SDIC_DMADIR	0x00000080	/* Set this bit for write, 0 for read */

#define CIR_DMA0ENB	0x00000100	/* Set this bit to enable DMA */
#define CIR_DMA1ENB	0x00000200	/* Set this bit to enable DMA */

#define SCSI0_C94ENB	0x00000400	/* Set to enable C94 0 */
#define SCSI1_C94ENB	0x00000800	/* Set to enable C94 1 */

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

#define SCSI_0_ERRMSK	(SCSI0_EDMA | SCSI0_PAR | SCSI0_DMAPAR | SCSI0_TCPAR)

#define SCSI_1_ERRMSK	(SCSI1_EDMA | SCSI1_PAR | SCSI1_DMAPAR | SCSI1_TCPAR)

#define SCSI_ERRMASK	(SCSI_0_ERRMSK | SCSI_1_ERRMSK |		\
				SCSITCWDPAR | SCSITCADPAR)

#define SDIC_ABITS_MASK		0x03	/* SCSI DME Ctrl Addr bits 01:00 */
#define UNALIGN_VALID_MASK	0x07	/* for the valid byte bits */

/* ---------------------------------------------------------------------- */
/*              IMER bit definitions                                      */
/* ---------------------------------------------------------------------- */

#define SCSI0_DRDY_IE	0x00000001	/* DREQ interrupt enable */
#define SCSI1_DRDY_IE	0x00000002
#define SCSI0_C94_IE	0x00000004	/* C94 interrupt enable */
#define SCSI1_C94_IE	0x00000008
#define SCSI0_PREF_IE	0x00000010	/* prefetch interrupt enable */
#define SCSI1_PREF_IE	0x00000020
#define SCSI0_EDMA_IE	0x00000040	/* ~ERR asserted during DMA IE */
#define SCSI1_EDMA_IE	0x00000080
#define SCSI0_PAR_IE	0x00000100	/* DB parity error IE */
#define SCSI1_PAR_IE	0x00000200
#define SCSI0_DMAPAR_IE	0x00000400	/* DMA buffer parity error IE */
#define SCSI1_DMAPAR_IE	0x00000800
#define SCSI0_TCPAR_IE	0x00001000	/* TC DMA read data parity error IE */
#define SCSI1_TCPAR_IE	0x00002000
#define SCSITCWDPAR_IE	0x00004000	/* TC write data parity error IE */
#define SCSITCADPAR_IE	0x00008000	/* TC address parity error IE */
#define SCSI0_DRDY_IM	0x00010000	/* DREQ interrupt mask */
#define SCSI1_DRDY_IM	0x00020000
#define SCSI0_C94_IM	0x00040000	/* C94 interrupt mask */
#define SCSI1_C94_IM	0x00080000
#define SCSI0_PREF_IM	0x00100000	/* prefetch interrupt mask */
#define SCSI1_PREF_IM	0x00200000
#define SCSI0_EDMA_IM	0x00400000	/* ~ERR asserted during DMA IM */
#define SCSI1_EDMA_IM	0x00800000
#define SCSI0_PAR_IM	0x01000000	/* DB parity error interrupt mask */
#define SCSI1_PAR_IM	0x02000000
#define SCSI0_DMAPAR_IM	0x04000000	/* DMA buffer parity error IM */
#define SCSI1_DMAPAR_IM	0x08000000
#define SCSI0_TCPAR_IM	0x10000000	/* TC DMA read data parity error IM */
#define SCSI1_TCPAR_IM	0x20000000
#define SCSITCWDPAR_IM	0x40000000	/* TC write data parity error IM */
#define SCSITCADPAR_IM	0x80000000	/* TC address parity error IM */

#define SCSI0_ERRMSK_INT	(SCSI0_EDMA_IE | SCSI0_PAR_IE |		\
				SCSI0_EDMA_IM | SCSI0_PAR_IM)

#define SCSI1_ERRMSK_INT	(SCSI1_EDMA_IE | SCSI1_PAR_IE |		\
				SCSI1_EDMA_IM | SCSI1_PAR_IM)

#define SCSI0_INT		(SCSI0_C94_IE | SCSI0_C94_IM)
#define SCSI1_INT		(SCSI1_C94_IE | SCSI1_C94_IM)

#define SCSITC_ERRMSK_INT	(SCSITCWDPAR_IE | SCSITCADPAR_IE |	\
				SCSITCWDPAR_IM | SCSITCADPAR_IM |	\
				SCSI0_DMAPAR_IE | SCSI1_DMAPAR_IE |	\
				SCSI0_DMAPAR_IM | SCSI1_DMAPAR_IM |	\
				SCSI0_TCPAR_IE | SCSI1_TCPAR_IE |	\
				SCSI0_TCPAR_IM | SCSI1_TCPAR_IM)

/* ---------------------------------------------------------------------- */

#define SIM_TCDS_CLK_SPEED	250	/* speed in MHz */
#define SIM_FAST_CLK_SPEED	400	/* speed in MHz */

/* ---------------------------------------------------------------------- */
/* This struct is used to pass the Memory read error information from the
interrupt handler to the local error log handler. */

typedef struct memerr_log
{
    U32 controller;		/* the controller number from the handler */
    void *pa;			/* the physical address */
} MEMERR_LOG;
#define	DME_MEMERR_VERS		1	/* please update if this changes */

#define DME_ERR_MEMERR	0x00000001	/* IOASIC detected a memory error */

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* The following Defines are for the DMA etc register layout for the TCDS
I/O ASIC controller. */

/* The TCDS_DMA_COMMON structure is the common register set for either of
SCSI 94 chips. */

typedef struct tcds_dma_common {
    PAD_U32 dma_addr;		/* SCSI DMA Phys address */
    PAD_U32 dma_sdic;		/* SCSI DMA/Interrupt control */
    PAD_U32 dma_unaln0;		/* SCSI Unaligned Data Buf 0 */
    PAD_U32 dma_unaln1;		/* SCSI Unaligned Data Buf 1 */
    PAD_U32 reserved[0x3c];	/* Reserved Filler */
} TCDS_DMA_COMMON;

/* The TCDS_DMA structure describes the shared DMA control and interrupt
registers.  The common DMA structurs are also contained in here. */

typedef struct tcds_dma {
    PAD_U32 cir;		/* Control interrupt register */
    PAD_U32 imer;		/* Interrupt mask/enable register */
    PAD_U32 reserved[0x3fe];	/* Reserved Filler */
    TCDS_DMA_COMMON dmaregs[2];	/* The two sets of DMA common registers */
} TCDS_DMA;

/* This is the TCDS layout for all the ASIC I/O space.  */

typedef struct tcds_asic {
    PAD_U32 flash_0[0x2];	/* the start of Flash EEPROM */
    PAD_U32 ids;
    PAD_U32 flash_1[0x10000 - 0x3];	/* The rest of Flash EEPROM */
    TCDS_DMA dma;
    PAD_U32 xyzzy[0x10000 - (sizeof (TCDS_DMA) / sizeof (PAD_U32))];
    SIM94_REG c940_reg;		/* C94 0 registers */
    PAD_U32 xc[0x40 - (sizeof (SIM94_REG) / sizeof (PAD_U32))];
    SIM94_REG c941_reg;		/* C94 1 registers */
    PAD_U32 xd[0x40 - (sizeof (SIM94_REG) / sizeof (PAD_U32))];
    PAD_U32 dmabufs[0x10000];	/* DMA buffers */
} TCDS_ASIC;

#define dma_cir		dma.cir.reg
#define dma_imer	dma.imer.reg
#define host_ids	ids.reg

/* ---------------------------------------------------------------------- */
/* The following Defines are for the tcds bus descriptor                  */

typedef struct tcds_bus {
    U32 bus_asc_no[2];		/* asc number for bus a/b */
    int asc_alive[2];		/* asc active for bus a/b */
    TCDS_ASIC *addr;		/* the tcds register set pointer */
} TCDS_BUS;

/* ---------------------------------------------------------------------- */
/* The following Macros are for accessing the TCDS DMA and control registers
via the information stored in the SIM softc structure. */

/* Get the address for the shared DMA registers. */
#define TCDS_DMA_PTR( sc )	((TCDS_DMA *)(sc)->dmaeng)

/* Get the address for the common DMA registers */
#define	TCDS_DMA_COMMON_PTR( sc )			\
		    (&(((TCDS_DMA *)(sc)->dmaeng)->dmaregs[(int)sc->rambuf]))

/*((TCDS_DMA_COMMON *)&(((TCDS_DMA *)((sc)->dmaeng))->dmaregs[(int)sc->rambuf])) */
/* ((TCDS_DMA_COMMON *)&(TCDS_DMA_PTR((sc)))->dmaregs[(int)sc->rambuf]) */

/* Get the address for the overall ASIC I/O register space. */
#define TCDS_ASIC_PTR( sc )	( (TCDS_ASIC *)(sc)->ifchip1 )

/* ---------------------------------------------------------------------- */
/* The following Macros are for accessing the various registers via the
TCDS structures.  The PAD_U32 define introduces a level of indirection
with the sparce space mapping of the padding and reg fields. */

/* Get the address of the SCSI Common DMA Registers. */
#define	TCDS_DMA_ADDR_PTR(sc)	(&(TCDS_DMA_COMMON_PTR((sc))->dma_addr.reg))
#define	TCDS_DMA_SDIC_PTR(sc)	(&(TCDS_DMA_COMMON_PTR((sc))->dma_sdic.reg))
#define	TCDS_DMA_UNALN0_PTR(sc)	(&(TCDS_DMA_COMMON_PTR((sc))->dma_unaln0.reg))
#define	TCDS_DMA_UNALN1_PTR(sc)	(&(TCDS_DMA_COMMON_PTR((sc))->dma_unaln1.reg))

/* Get the address of the SCSI Shared DMA Registers. */
#define	TCDS_DMA_CIR_PTR(sc)	(&(TCDS_DMA_PTR((sc))->cir.reg))
#define	TCDS_DMA_IMER_PTR(sc)	(&(TCDS_DMA_PTR((sc))->imer.reg))

/* ---------------------------------------------------------------------- */
/* These TCDS control interrupt macros are used to help hide the fact that
there are shared bits between the two SCSI controllers.  The SCSI controller
number for the ASIC is stored in the rambuffer field in the SIM_SOFTC. */

/* Turn on the DMA enable bit in the Control Interrupt register */
#define TCDS_CIR_DMA_ENABLE( sc )				\
{								\
    *(TCDS_DMA_CIR_PTR((sc))) |= 0xffff0000 |			\
	(((sc)->rambuf == 0)?(CIR_DMA0ENB):(CIR_DMA1ENB));	\
}

/* Turn off the DMA enable bit in the Control Interrupt register */
#define TCDS_CIR_DMA_DISABLE( sc )				\
{								\
    *(TCDS_DMA_CIR_PTR((sc))) = (*(TCDS_DMA_CIR_PTR((sc))) &	\
	~(((sc)->rambuf == 0)?(CIR_DMA0ENB):(CIR_DMA1ENB))) |	\
	0xffff0000;						\
}

/* ---------------------------------------------------------------------- */
/* Misc Macros for accessing the TCDS registers. */

/* MAPPING FUTURES: This macro is being created for now to work with the
mapping register changes. */

/* Turn on or off the TCDS prefetch feature. */
#define TCDS_PREFETCH( sc, dws )				\
{								\
    /* Compile into a NOOP */					\
}


/* ---------------------------------------------------------------------- */


#endif
