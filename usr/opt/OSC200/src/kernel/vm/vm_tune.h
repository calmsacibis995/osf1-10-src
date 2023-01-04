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
 * @(#)$RCSfile: vm_tune.h,v $ $Revision: 1.1.15.3 $ (DEC) $Date: 1993/08/20 19:28:33 $
 */
#ifndef	__VM_TUNE__
#define	__VM_TUNE__

struct vm_tune {
	unsigned char	vt_cowfaults;		/* Copy point */
	unsigned short	vt_mapentries;		/* Maximum map entries */
	vm_offset_t	vt_maxvas;		/* Maximum VAS for user map */
	vm_offset_t	vt_maxwire;		/* Maximum wired memory */
	unsigned char	vt_heappercent;		/* kva percentage for heap */
	unsigned short	vt_anonklshift;		/* anon page shift */
	unsigned short	vt_anonklpages;		/* pages to fetch in kluster */
	unsigned long	vt_vpagemax;		/* Maximum vpage for umap */
	unsigned char	vt_segmentation;	/* Segmentation on or off */ 
	unsigned short	vt_ubcpagesteal;	/* Steal vnode clean list */
	unsigned char	vt_ubcdirtypercent;	/* Percent dirty push value */
	unsigned char	vt_ubcseqstartpercent;	/* Start ubcseqpercent */
	unsigned char	vt_ubcseqpercent;	/* Sequential allocation */
	vm_size_t 	vt_csubmapsize;		/* Size of kernel copy map */
	unsigned short	vt_ubcbuffers;		/* ubc buffers */
	unsigned short	vt_syncswapbuffers;	/* synchronous swap buffers */
	unsigned long	vt_clustermap;		/* ckluster dup map size */
	unsigned long	vt_clustersize;		/* max cluster bp size */
	vm_size_t       vt_zone_size;           /* Size of zone_map */
	vm_size_t       vt_kentry_zone_size;    /* Size of kentry_map */
	unsigned char	vt_syswiredpercent;	/* max pct of wired memory system-wide) */
	unsigned short  vt_asyncswapbuffers;	/* asynchronous swap buffers */
	unsigned short  vt_inswappedmin;	/* minimum inswapped ticks */	
};

#define	vm_tune_compare(VALUE,FIELD,ROP) ((VALUE) ROP vm_tune.vt_/**/FIELD)
#define	vm_tune_value(FIELD)	vm_tune.vt_/**/FIELD

extern struct vm_tune vm_tune;

#ifndef COWFAULTS
#define COWFAULTS (4)		/*Copy point */
#endif

#ifndef MAPENTRIES
#define MAPENTRIES (200)       	/*Maximum map entries */
#endif

#ifndef MAXVAS
#define MAXVAS  (1L<<30)	/*Maximum VAS for user maps */
#endif

#ifndef MAXWIRE
#define MAXWIRE (1L<<24)	/*Maximum wired memory*/
#endif

#ifndef HEAPPERCENT
#define HEAPPERCENT (7)		/*Percent of memory for heap*/
#endif

#ifndef ANONKLSHIFT
#define ANONKLSHIFT (17)       	/*Anon page shift*/
#endif

#ifndef ANONKLPAGES
#define ANONKLPAGES (1)		/*Pages to fetch in kluster*/
#endif

#ifndef VPAGEMAX
#define VPAGEMAX (16*1024)	/*Maximum vpage for umap*/
#endif

#ifndef SEGMENTATION 
#define SEGMENTATION (1)	/*Segmentation on or off*/
#endif

#ifndef UBCPAGESTEAL
#define UBCPAGESTEAL (24)	/*Steal vnode clean list*/
#endif

#ifndef UBCDIRTYPERCENT
#define UBCDIRTYPERCENT (10)	/*Percent dirty push value*/
#endif

#ifndef UBCSEQSTARTPERCENT
#define UBCSEQSTARTPERCENT (50)	/*Start ubcseqpercent */
#endif

#ifndef UBCSEQPERCENT
#define UBCSEQPERCENT (10)	/*Sequential allocation*/
#endif

#ifndef CSUBMAPSIZE
#define CSUBMAPSIZE (1024*1024)	/*Size of kernel copy map*/
#endif

#ifndef UBCBUFFERS
#define UBCBUFFERS (256)	/*ubc buffers*/
#endif

#ifndef SYNCSWAPBUFFERS
#define SYNCSWAPBUFFERS (128)	/* synchronous swap buffers*/
#endif

#ifndef CLUSTERMAP
#define CLUSTERMAP (1024*1024*1) /*Cluster dup map size*/
#endif

#ifndef CLUSTERSIZE
#define CLUSTERSIZE (1024*64)	/*Max cluster dup per bp */
#endif

#ifndef ZONE_SIZE
#define ZONE_SIZE (64*1024*1024)        /*size of zone_map */
#endif

#ifndef KENTRY_ZONE_SIZE
#define KENTRY_ZONE_SIZE (16*1024*1024) /*size of kentry_map */
#endif

#ifndef SYSWIREDPERCENT
#define SYSWIREDPERCENT (80) /*max pct of wired memory system-wide*/
#endif

#ifndef ASYNCSWAPBUFFERS
#define ASYNCSWAPBUFFERS (4)   /* asynchronous swap buffers*/
#endif

#ifndef INSWAPPEDMIN
#define INSWAPPEDMIN	(1)   /* minimum inswapped ticks */
#endif

#endif	/* !__VM_TUNE__ */
