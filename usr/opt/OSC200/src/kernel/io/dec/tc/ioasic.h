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
 *	@(#)$RCSfile: ioasic.h,v $ $Revision: 1.2.7.2 $ (DEC) $Date: 1993/07/14 18:17:35 $
 */ 
/*
 * ioasic.h
 *
 * Modification history
 *
 * 25-Sep-1991 - Andrew Duane
 *	Modified for support of ALPHA FLAMINGO
 *		and a bunch of portability fixes.
 *
 * 10-May-1991 - Paul Grist
 * 	Made handling of IOASIC base address generic, so other systems
 *	which use the IOASIC, like 3max+, can be easily added.
 *
 * 20-Feb-1990 - pgt (Philip Gapuz Te)
 * 	created file.
 *
 */

#ifndef _IOASIC_H_ 
#define _IOASIC_H_ 

/* IOASIC locations using variable offset assigned at cons_init time */


/* These original IOC_xxxx macros were MIPS-specific (PHYS_TO_K1) */
/* They have been changed for FLAMINGO to offset from specific addresses */
/* that are mapped at boot time, or (with OSFPAL) to use PHYS_TO_KSEG. */
#ifdef __alpha
#define  IOC_COMM1_DMA_BASE      (PHYS_TO_KSEG((scc_ioasic_base) + 0x00080060))
#define  IOC_COMM2_DMA_BASE      (PHYS_TO_KSEG((scc_ioasic_base) + 0x000800A0))
#define  IOC_SSR                 (PHYS_TO_KSEG((scc_ioasic_base) + 0x00080200))
#define  IOC_SIR                 (PHYS_TO_KSEG((scc_ioasic_base) + 0x00080220))
#define  IOC_SIMR                (PHYS_TO_KSEG((scc_ioasic_base) + 0x00080240))
#else
#define  IOC_COMM1_DMA_BASE      (PHYS_TO_K1((scc_ioasic_base) + 0x00040030))
#define  IOC_COMM2_DMA_BASE      (PHYS_TO_K1((scc_ioasic_base) + 0x00040050))
#define  IOC_SSR                 (PHYS_TO_K1((scc_ioasic_base) + 0x00040100))
#define  IOC_SIR                 (PHYS_TO_K1((scc_ioasic_base) + 0x00040110))
#endif	/* __alpha */


#define  SCC_INTR (SIR_COMM1_XINT | SIR_COMM1_RINT | \
		   SIR_COMM1_XERROR | SIR_COMM1_RERROR | \
		   SIR_COMM2_XINT | SIR_COMM2_RINT | \
		   SIR_COMM2_XERROR | SIR_COMM2_RERROR | \
		   SIR_SCC0 | SIR_SCC1)

#define  LANCE_INTR (SIR_LANCE_RERROR | SIR_LANCE)

#define  ISDN_INTR  (SIR_ISDN_XINT | SIR_ISDN_RINT | SIR_ISDN_MINT | SIR_ISDN)

/* IOASIC System Interrupt Register bits */
#define  SIR_COMM1_XINT      0x80000000
#define  SIR_COMM1_XERROR    0x40000000
#define  SIR_COMM1_RINT      0x20000000
#define  SIR_COMM1_RERROR    0x10000000
#define  SIR_COMM2_XINT      0x08000000
#define  SIR_COMM2_XERROR    0x04000000
#define  SIR_COMM2_RINT      0x02000000
#define  SIR_COMM2_RERROR    0x01000000
#define  SIR_ISDN_XINT       0x00400000
#define  SIR_ISDN_RINT       0x00200000
#define  SIR_ISDN_MINT       0x00100000
#define  SIR_LANCE_RERROR    0x00010000
#define  SIR_ISDN            0x00002000
#define  SIR_LANCE           0x00000100
#define  SIR_SCC1            0x00000080
#define  SIR_SCC0            0x00000040
#define  SIR_ALT_CONSOLE     0x00000008


/* The following aren't used on MAXine */
#define  RESERVED_SIR_BITS   ( SIR_COMM2_XINT | SIR_COMM2_XERROR | \
                               SIR_COMM2_RINT | SIR_COMM2_RERROR | \
                               SIR_SCC1)

/* The following aren't used on MAXine */
#define  RESERVED_SSR_BITS   (SSR_COMM2_XEN | SSR_COMM2_REN)

/* IOASIC System Support Register bits */
#define  SSR_COMM1_XEN       0x80000000
#define  SSR_COMM1_REN       0x40000000
#define  SSR_COMM2_XEN       0x20000000
#define  SSR_COMM2_REN       0x10000000
#define  SSR_RESET           0x00000800


#ifdef __alpha
#define  IOC_RD(reg, var)        (var) = (*(u_int *)(reg))
#define  IOC_WR(reg, var)        {*(u_int *)(reg) = (u_int)(var) ; mb();}
#else	/* __alpha */
#define  IOC_RD(reg, var)        (var) = (*(u_int *)(reg))
#define  IOC_WR(reg, var)        *(u_int *)(reg) = (u_int)(var) 
#endif	/* __alpha */

#define  IOC_SET(reg, mask)      {   u_int temp; \
				     IOC_RD((reg), temp); \
				     IOC_WR((reg), temp|(mask)); \
				     }

#define  IOC_CLR(reg, mask)      {   u_int temp; \
				     IOC_RD((reg), temp); \
                                     IOC_WR((reg), temp & ~(mask)); \
				     }


#endif
