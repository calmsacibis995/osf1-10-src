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
 *	@(#)scamachmac.h	4.2	(ULTRIX)	9/1/90
 */

/*
 *
 *   Facility:	Systems Communication Architecture
 *
 *   Abstract: This module contents hardware platform dependent macro's
 *	       for VAX and MIPS used by the SCA subsystem.
 *
 *   Creator:	Pete Keilty 	Date:	01-Jul-1989
 *
 *   Modification History:
 *
 *   08-Dec-1989	Pete Keilty
 *	Modified  MtoVspt() & MptetoVbdt() macros now use NBPG as a
 *	divisor.
 */
/************************************************************************/

#ifndef _SCAMACHMAC_H_
#define _SCAMACHMAC_H_

#ifdef	__vax

#define Sid 		(mfpr( SID ))
#define Tbis( vaddr )	(( void )mtpr( TBIS, vaddr ))
#define VPQB_BASE   	((GVPPQB *)&pccb->Pqb)
#define BDT_BASE   	((GVPBD *)gvp_bddb->bdt)
#define SPT_BASE   	((struct pte *)mfpr( SBR ))
#define SPT_LEN   	(mfpr( SLR ))
#define GPT_BASE   	((struct pte *)(&Sysmap[ 0 ]))
#define CI_DFREEQ 	(&pccb->Dfreeq)
#define CI_MFREEQ 	(&pccb->Mfreeq)
#define BVP_DFREEQ 	(&Pccb.dfreeq)
#define BVP_MFREEQ 	(&Pccb.mfreeq)
#define Dm_pccbifISIS	;
#define Dm_bddbifISIS	;
#define Dm_buffer( bdp, index, addr ) 	;
#define Dm_msg_dg( gvpbp, size ) 	;
#define BHOLE_MASK	0x7fffffff
#define MSI_RPROTOPTE	(( u_long )( PG_V | PG_KW ))
#define MSI_XPROTOPTE   (( u_long )( PG_V | PG_KR ))
#define VAX_PGOFSET	0x1ff

#endif /*	__vax */

/************************************************************************/

#ifdef	__mips

struct vaxpte {
    unsigned int  pg_pfnum :21;
    unsigned int	   :10;
    unsigned int  pg_v	   :1;
};
#define Maxvax_dbpte 136 	/* 64KB of pte's + 8 round up for alignment */
struct vax_dbpte {
    struct vaxpte pte[Maxvax_dbpte];
};

extern struct vaxpte *Vaxmap;
extern int Vaxmap_size;
extern struct vaxpte *VSysmap;
extern struct pte *Vaxdbpte; 
extern struct vaxpte *Dbptemap;

#define VAX_SVMASK	0xbfffffff
#define VAX_PG_V	0x80000000
#define VAX_SYSVA	0x80000000
#define VAX_PGOFSET	0x1ff
#define VAX_PGSIZE	512
#define VAX_PGSHIFT	9
#define VAX_PAGE_INDEX_BITS	0x00000e00
#define VAX_PFN_OFFSET(addr)	(((u_long)(addr) & VAX_PAGE_INDEX_BITS) >> VAX_PGSHIFT)


/* 
 * Map Mips ptes in Vaxmap 
 * mpte = mips pte, cnt = number of ptes
 */
#define MtoVspt( mpte, size ) { 				\
    u_int i, pfn, cnt;						\
    struct vaxpte *vpte = (struct vaxpte *)(Vaxmap + (((mpte) - Sysmap) << 3));\
    for( cnt = ((size) + PGOFSET)/NBPG ; cnt > 0 ; --cnt , (mpte)++ ) { \
	for( i = 0 , pfn = ((mpte)->pg_pfnum << 3) ; i < 8 ;	\
	     vpte++, pfn++, i++ ) {				\
		*(u_int *)vpte = pfn | VAX_PG_V;		\
	}							\
    }								\
}

/*
 * Map Mips data buffer ptes into Vaxmap & setup bdt entry
 */
#define MptetoVbdt( bdp, index, addr ) { 			\
    u_int i, pfn, cnt;						\
    struct vaxpte *vpte;					\
    struct pte *vptesave;					\
    vptesave = Vaxdbpte + ((index) * Maxvax_dbpte);		\
    vpte = (struct vaxpte *)					\
		((struct vax_dbpte *)Dbptemap + (index)); 	\
    for( cnt = ((bdp)->bsize + ((u_long)addr & PGOFSET) + PGOFSET)/NBPG ; \
		 cnt > 0 ; --cnt , (bdp)->bpte++ ) { 		\
	for( i = 0 , pfn = ((bdp)->bpte->pg_pfnum << 3) ; i < 8 ; \
	     vpte++, pfn++, i++ ) {				\
		*(u_int *)vpte = pfn | VAX_PG_V;		\
	}							\
    }								\
    (bdp)->bpte = (struct pte *)((u_long)( vptesave + 		\
		VAX_PFN_OFFSET( addr ) )); 			\
}

/* 
 * Map mips system virt. addr. and size into Vaxmap
 * addr = Mips sva; size = bytes
 */
#define MaptoVmap( addr, size ) {				\
    struct pte *mpte = svtopte( addr );				\
    MtoVspt( mpte, ( size + ((u_long)addr & PGOFSET )))		\
}

#define Dm_pccbifISIS {						\
    if( cpu == DS_5800 ) {					\
        MaptoVmap( pccb, pccb->size )				\
        pccb->lpinfo.flags.dm = 1;				\
    }								\
}

#define Dm_bddbifISIS {						\
    if( cpu == DS_5800 ) {					\
        MaptoVmap( gvp_bddb, size )				\
    }								\
}

#define Dm_buffer( bdp, index, addr ) {				\
    if( pccb->lpinfo.flags.dm ) {				\
        MptetoVbdt( bdp, index, addr )				\
    }								\
}

#define Dm_msg_dg( gvpbp, size ) {				\
    if( pccb->lpinfo.flags.dm ) {				\
        MaptoVmap( gvpbp, size )				\
    }								\
}

#define Sid 		(cpu_systype)
#define Tbis(vaddr)	(unmaptlb( 0, btop( vaddr )))
#define CI_DFREEQ  	((struct _gvpbq *)((u_long)&pccb->Dfreeq & VAX_SVMASK))
#define CI_MFREEQ  	((struct _gvpbq *)((u_long)&pccb->Mfreeq & VAX_SVMASK))
#define BVP_DFREEQ  	((struct _gvpbq *)((u_long)&Pccb.dfreeq & VAX_SVMASK))
#define BVP_MFREEQ  	((struct _gvpbq *)((u_long)&Pccb.mfreeq & VAX_SVMASK))
#define VPQB_BASE   	((GVPPQB *)(( u_long )&pccb->Pqb & VAX_SVMASK))
#define BDT_BASE   	((GVPBD *)(( u_long )gvp_bddb->bdt & VAX_SVMASK))
#define SPT_BASE   	((struct pte *)(K0_TO_PHYS( Vaxmap )))
#define SPT_LEN   	(Vaxmap_size)
#define GPT_BASE   	((struct pte *) VSysmap)
#define BHOLE_MASK	0x1fffffff
/*
#define MSI_RPROTOPTE   (( u_long )( PG_V | PG_KW | PG_G | PG_M ))
#define MSI_XPROTOPTE   (( u_long )( PG_V | PG_KR | PG_G | PG_M ))
*/
extern unsigned cpu_systype;
#endif /*	__mips */

#endif
