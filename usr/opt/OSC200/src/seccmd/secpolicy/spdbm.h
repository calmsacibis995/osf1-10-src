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
 * @(#)$RCSfile: spdbm.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/04/01 20:18:11 $
 */
#ifndef __SPDBM__
#define __SPDBM__

#if SEC_ARCH

/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	spdbm.h,v $
 * Revision 1.1.1.1  92/03/07  00:44:00  devrcs
 * *** OSF1_1B23 version ***
 * 
 * Revision 1.4  1990/10/07  15:42:38  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  13:06:03  gm]
 *
 * Revision 1.3  90/07/17  11:48:58  devrcs
 * 	Merged with SecureWare osc14
 * 	[90/07/10  13:19:06  staffan]
 * 
 * $OSF_EndLog$
 */
/*
 * Copyright (c) 1988-1990, SecureWare, Inc.  All Rights Reserved.
 *
 * @(#)spdbm.h	3.1 10:02:39 6/7/90 SecureWare
 * Based on:
 *	@(#)spdbm.h	2.1 12:32:08 1/25/89
 *	
 * Security Policy Database Manager Header File
 */

/* Define the size of a tag */

#define	SEC_TAG_SIZE	1

/* Security Policy Entry Points */

int spdbm_open();
int spdbm_close();
int spdbm_tag_allocate();
int spdbm_tag_deallocate();
int spdbm_get_ir();

/* Minimum Tag Value that can be generated by Database Manager */

#define DBM_MIN_TAG		20L	/* lowest permissible tag value */

/* Miscellaneous Defines */

#define	DBM_NO_CREATE	0		/* no page creation on lookup */
#define DBM_CREATE	1		/* create page if null on lookup */

#define DBM_TEMPORARY	0		/* no ref count for volatile object */
#define DBM_PERMANENT	1		/* permanent object for ref count */

#define DBM_PAGE_FREE	0		/* page count under threshold */
#define DBM_PAGE_FULL	1		/* page exceeds threhold, skip it */

/* Database Key Comparison Results */

#define DBM_FIRST_KEY_GREATER	-1	/* A > B */
#define DBM_KEYS_EQUAL		0	/* A = B */
#define DBM_SECOND_KEY_GREATER	1	/* A < B */

/* Database Manager Insert/Delete/Lookup Function Codes */

#define DBM_TAG			1	/* Only Tag partition */
#define DBM_IR			2	/* Only IR partition */
#define DBM_BOTH		3	/* Both partitions */
#define DBM_BOTH_FROM_TAG	4	/* Both partitions from Tag */
#define DBM_BOTH_FROM_IR	5	/* Both partitions from IR */

/* Routine Definition for Level 1 and 2 Slot Calculation for Tags */

#define	spdbm_set_tag_slots(x)	dbm_l1_slot = spdbm_level1_tag_hash(x);\
				dbm_l2_slot = spdbm_level2_tag_hash(x);

/* Database File Statistics */

struct session_stats {
	long	pkey_inserts;		/* primary key insertions */
	long	pkey_deletes;		/* primary key deletions */
	long	pkey_lookups;		/* primary key lookups */
	long	akey_inserts;		/* alternate key insertions */
	long	akey_deletes;		/* alternate key deletions */
	long	akey_lookups;		/* alternate key lookups */
	long	tag_collisions;		/* collision on tag creation */
	long	tag_cache_hits;		/* tag cache hit total */
	long	tag_cache_misses;	/* tag cache miss total */
	long	page_cache_hits;	/* I/O page cache hit total */
	long	page_cache_misses;	/* I/O page cache miss total */
	long	page_io;		/* page I/O total */
};					/* session statistics */

struct dbm_stats {
	long	pages_allocated;	/* total pages for data/index */
	long	pages_deallocated;	/* total freed pages */
	long	page_overflows;		/* total overflow pages */
	long	page_shrinks;		/* total page shrinks */
	long	pkey_entries;		/* primary key records */
	long	akey_entries;		/* alternate key records */
	struct session_stats curr;	/* current open session */
	struct session_stats perm;	/* permanent statistics */
};

/* Database Key Locator Information */

struct key_info {
	caddr_t	caddr;			/* memory location of buffer */
	daddr_t	daddr;			/* disk block offset */
};

/* File Control Header Flags */

#define DBM_INITIALIZED		0x01	/* database initialized */
#define DBM_PARTITION		0x02	/* database on raw partition */
#define DBM_IPAGE_FLUSH		0x04	/* level 1 index pages need flushing */
#define DBM_OPEN		0x08	/* database is open */
#define DBM_RDONLY		0x10	/* open for reading only */
#define DBM_RDWR		0x20	/* open for reading/writing */

/* Database Page Sizes and Entry Counts for Control Block Pages */

#define DBM_PAGESIZE		4096	/* control page sizes */
#define DBM_IPAGESIZE		128	/* level 1 index page size */
#define DBM_TCACHE_SIZE		8192	/* size of tag cache block */
#define DBM_NBPL		32	/* bit flags per long word */

#define DBM_TCACHE_ENTRIES	((DBM_TCACHE_SIZE / sizeof(struct tag_cache)))
#define DBM_L1_MASKSIZE		((DBM_IPAGESIZE / sizeof(daddr_t)) / DBM_NBPL)
#define DBM_L1_ENTRIES		(DBM_IPAGESIZE / sizeof(daddr_t))

/* Dependency: If DBM_IPAGESIZE changes, modify this mask */

#define DBM_L1_SHIFT	27		/* shift to high order 5 bits */
#define DBM_L1_SLOTMASK	((DBM_L1_ENTRIES - 1) << DBM_L1_SHIFT)	/* bit mask */

/* Database File Offsets for Special Control Block Pages */

#define	DBM_HEADER_OFFSET   (0 * DBM_PAGESIZE)   /* control header block */
#define DBM_SLEVEL_OFFSET   (1 * DBM_PAGESIZE)   /* single level free block */
#define DBM_MLEVEL_OFFSET   (2 * DBM_PAGESIZE)   /* multi-level free block */
#define DBM_TCACHE_OFFSET   (3 * DBM_PAGESIZE)   /* permanent tag cache block */

/* Primary and Alternate Key Master Index Page Offsets */

#define DBM_PKEY_OFFSET	    (DBM_TCACHE_OFFSET + DBM_TCACHE_SIZE)
#define DBM_AKEY_OFFSET     (DBM_PKEY_OFFSET + DBM_IPAGESIZE)

/* Level 2 Tag Allocation Cluster Page Map Offset */

#define DBM_L2_MASK_OFFSET  (DBM_AKEY_OFFSET + DBM_IPAGESIZE)

#define DBM_LEVEL1_INDICES	(DBM_IPAGESIZE / sizeof(daddr_t))

#define DBM_DEF_BUFFERS	(100 * 1024)	/* default buffer allocation */
#define DBM_MULTI_EXTENTS	5	/* active multi-extent table slots */

/* Database File Control Header-database parameters */

struct database_header {
	struct key_info	pkey;		/* primary key first level */
	struct key_info akey;		/* alternate key first level */
	short	flags;			/* flag word */
	short	thresh_percent;		/* free space threshold percentage */
	short	tag_cache_entries;	/* number of tag cache slots */
	short	slevel_count;		/* number of single level free pages */
	short	slevel_block;		/* block offset to slevel free list */
	short	mlevel_count;		/* number of multi level free pages */
	short	mlevel_block;		/* block offset to mlevel free list */
	long	max_filesize;		/* maximum file extent */
	long	primary_pagesize;	/* primary index pagesize */
	long	pagesize;		/* secondary index, data pagesizes */
	long	threshold;		/* level 2 free space threshold */
	long	l2size;			/* size of level 2 bit mask page */
	long	high_offset;		/* highest seek address for allocate */
	ulong	l1_mask[DBM_L1_MASKSIZE];	/* level1 mask page */
	struct dbm_stats stats;		/* database statistics */
};

/* Database Record Header Format */

struct dbase_rechdr {
	long	recsize;		/* size of internal representation */
	long	ref_count;		/* non-volatile reference count */
	tag_t	tag[SEC_TAG_SIZE];	/* security attribute tag */
	/* internal representation follows */
};

#define RECSIZE		(sizeof(struct dbase_rechdr))
#define NULL_RECP	((struct dbase_rechdr *) 0)

/* Database Data Page Types */

#define DBM_TAG_PAGE	0		/* tag page type */
#define DBM_IR_PAGE	1		/* ir page type */
#define DBM_INDEX_PAGE	2		/* first or second level index */

/* Pagesize Structure to Include Extent Count */

struct pagesize {
	uint	mask_set:1;		/* set free page mask on delete */
	uint	rfu:15;			/* reserved */
	uint	pgtype:2;		/* page type */
	uint	extent:14;		/* extent count */
	ulong	pgsize;			/* data page size */
};

/* Generic Datapage Header with Pagesize Structure */

struct datapage_header {
	struct pagesize pgsize;		/* data page size */
};

/* Security Attribute TAG Database Page Header */

struct tag_header {
	struct pagesize	pgsize;		/* data page size */
	tag_t	low_tag[SEC_TAG_SIZE];	/* low tag on page */
	tag_t	high_tag[SEC_TAG_SIZE];	/* high tag on page */
};

#define THDR_SIZE	(sizeof(struct tag_header))

/* Security Attribute Internal Representation Database Page Header */

struct ir_header {
	struct pagesize	pgsize;		/* data page size */
};

#define IRHDR_SIZE	(sizeof(struct ir_header))

/* Tag Cache Structure Format */

#define TCACHE_BUCKETS	8		/* Eight buckets per Slot */

struct tag_cache {
	short lru_bucket;			/* next bucket to use */
	struct {
		struct {
			uint	valid:1;	/* cache entry valid */
			uint	rfu:15;		/* reserved */
		} flags;
		tag_t	tag[SEC_TAG_SIZE];	/* tag value */
		daddr_t	daddr;			/* disk offset */
	} bucket [TCACHE_BUCKETS];
};

/* Data/Index Page I/O Cache */

#define PCACHE_BUCKETS	4		/* Four buckets per Slot */

struct page_cache {
	short	lru_bucket;			/* next bucket to use */
	struct {
		struct {
			uint	valid:1;	/* cache entry valid */
			uint	rfu:15;		/* reserved */
		} flags;
		caddr_t	caddr;			/* memory buffer address */
		daddr_t	daddr;			/* disk offset */
	} bucket [PCACHE_BUCKETS];		/* number of buckets per slot */
};

/* Database Memory/Disk Block Control Structure */

struct buf_cblock {
	struct {
		uint	allocated:1;		/* buffer is allocated */
		uint	pgtype:2;		/* type of page using it */
		uint	locked:1;		/* do NOT steal this page */
		uint	extent:4;		/* page extent count */
		uint	rfu:24;			/* reserved */
	} flags;
	time_t	time;				/* LRU management */
	caddr_t	addr;				/* rounded memory address */
	caddr_t	maddr;				/* malloc'd memory address */
	daddr_t	daddr;				/* disk block address */
};

/* Multi-Level Free Disk Block Control Structure */

struct mlevel_free {
	daddr_t	daddr;				/* disk block address */
	struct {
		uint	valid:1;		/* valid free entry */
		uint	rfu:15;			/* reserved */
		uint	extent:16;		/* number of extents */
	} flags;
};

#endif /* SEC_ARCH */
#endif /* __SPDBM__ */
