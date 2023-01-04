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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: spdbm.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/04/01 20:18:05 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	spdbm.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.6.2.2  1992/06/10  20:37:01  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/10  20:30:07  hosking]
 *
 * Revision 1.6  1991/01/07  13:05:49  devrcs
 * 	Cleanup ifdef NLS
 * 	[90/11/27  08:39:02  aster]
 * 
 * Revision 1.5  90/10/31  14:23:54  devrcs
 * 	Append newline to end of message.
 * 	[90/10/21  12:19:08  seiden]
 * 
 * Revision 1.4  90/10/07  15:42:31  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  13:05:46  gm]
 * 
 * Revision 1.3  90/07/17  11:48:49  devrcs
 * 	Merged with SecureWare osc14
 * 	[90/07/10  13:15:10  staffan]
 * 
 * $OSF_EndLog$
 */

/*
 * Copyright (c), 1988-90, SecureWare, Inc.
 *   All Rights Reserved.
 */

/* #ident "@(#)spdbm.c	3.1 10:02:30 6/7/90 SecureWare" */
/*
 * Based on:
 *   "@(#)spdbm.c	2.2 10:44:44 9/22/89 SecureWare"
 */

/*
	Security Policy Database Manager (SPDM)

	The Database Manager is provided to support Security Policy
	Daemons and Modules in the administration of security tags and
	attribute internal representations. Each subject/object will
	contain a number of abstract tags that represent a security
	attribute. This tag is mapped by the database to the internal
	representation of that attribute.

	The database is wholly replicated with a partition for both the
	tag to internal representation and internal representation to
	tag mappings. In this manner, the database is considered to be
	multi-keyed with the tags being the primary key and the internal
	representations being the alternate key. The database is wholly
	replicated since the alternate index entries, which typically
	only contain pointers to the actual records on the primary key
	partition, contain full records themselves since the amount of
	additional storage is inconsequential. The database records only
	contain the two keys along with the reference count for the
	tag.

	Reference count maintenance is used to support re-use of security
	tags and to eliminate unnecessary records from the database. This
	tends to reduce storage requirements and also serves to increase
	performance since search time can be reduced. When tags are either
	created or fetched, a flag is passed to the routine indicating
	whether the tag is associated with a permanent object or not. All
	file creations result in permanent tags whereas subject tags are
	considered volatile and therefore temporary. On a lookup, the type
	is specified as a temporary access since the tag is not being
	assigned to another object. On delete operations, the reference
	count is checked to insure it is 0 before deletion occurs. Deletion
	also requires the object type only if it is a permanent object (file).

	Entry Points:

		spdbm_open(database name,mode,buffers)
		spdbm_tag_allocate(ir_address,ir_length,tag_buffer,type)
		spdbm_tag_deallocate(tag_address)
		spdbm_get_ir(tag_address,ir_buffer,ir_length)
		spdbm_get_stats(buffer)
		spdbm_close()

	Entry points used by dbck(1M) only:

		spdbm_lookup(type,attribute,size,object type)
		spdbm_insert(type,attribute,size,object type)

*/

#include <sys/types.h>
#include <sys/security.h>
#include <sys/secpolicy.h>
#include "spdbm.h"

#include <stdio.h>
#include <fcntl.h>
#ifndef _OSF_SOURCE
#include <malloc.h>
#else
extern char *malloc();
#endif

#include <locale.h>
#include "secpolicy_msg.h"

nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SECPOLICY,n,s) 

/* to align on power-of-two boundaries */

#include <sys/param.h>
#ifndef roundup
#define roundup(x,y)	(((x)+((y)-1)) & ~(y-1))
#endif

/* Flush Masks for Critical Structure Updates */

#define DBM_FLUSH_DBHDR		1
#define DBM_FLUSH_TAG_L1	2
#define DBM_FLUSH_IR_L1		3
#define DBM_FLUSH_SFREE		4
#define DBM_FLUSH_MFREE		5
#define DBM_FLUSH_L2_MASK	6


#define freepage(x)	(((x)->pgsize.extent * dbase_hdr.pagesize) - \
			(x)->pgsize.pgsize)

/* Random Number Generator Functions for Tag Allocation */

long srand48();
long lrand48();

/* These symbols are required by the dbck(1M) program */

int dbm_fd = 0;
int dbm_l2_indices = 0;

/* All others are static */

static int dbm_debug = 0;
static int dbm_buffers = 0;
static int dbm_sbuf_index = 0;
static int dbm_tag_cache = 0;
static int dbm_page_cache = 0;
static int dbm_buf_pool_size = 0;
static int dbm_l2_mask_size = 0;
static int dbm_l1_slot = 0;
static int dbm_l2_slot = 0;

static int *dbm_buf_pool;
static struct tag_cache *tcache;
static struct page_cache *pcache;
static daddr_t *tag_level2;
static daddr_t *ir_level2;
static struct buf_cblock *dbm_single;

/* Buffer Control Structure for Multi-Extent Pages */

static struct buf_cblock dbm_multi[DBM_MULTI_EXTENTS];

/* Database Control Header Buffer */

struct database_header dbase_hdr;

/* Primary Index Pages for Tag and IR Partitions */

daddr_t tag_level1 [DBM_IPAGESIZE / sizeof(daddr_t)];
daddr_t  ir_level1 [DBM_IPAGESIZE / sizeof(daddr_t)];

/* Single and Multi-Extent Free Page Blocks from Database */

static daddr_t slevel_free [DBM_PAGESIZE / sizeof(daddr_t)];
static struct mlevel_free mlevel_free[DBM_PAGESIZE / sizeof(struct mlevel_free)];

#define DBM_MAX_SLEVEL	(DBM_PAGESIZE / sizeof(daddr_t))
#define DBM_MAX_MLEVEL	(DBM_PAGESIZE / sizeof(struct mlevel_free))

/* Static Storage for Level 1 Free Page Bit Mask */

static ulong *l2_mask_page;

/* Static Storage for Internal Representation Copy During Delete */

static char *ir_delete;

/* Static Tag Generation Storage and Null Tag Declaration */

static tag_t tag[SEC_TAG_SIZE] = { 0 };
static tag_t null_tag[SEC_TAG_SIZE] = { 0 };

/* Routine Declarations */

caddr_t spdbm_page_locate();
caddr_t page_read();
caddr_t page_allocate();
caddr_t page_extend();
caddr_t page_buffer_alloc();
caddr_t pcache_lookup();
daddr_t tcache_lookup();
daddr_t page_find_daddr();
daddr_t page_block_alloc();
struct dbase_rechdr *spdbm_lookup();

#ifdef DEBUG_MALLOC

/* Debug Support for Malloc/Free Operations */

#define MPOOL_COUNT	16

#define ALLOC	1
#define FREE	2

struct malloc_stat {
	char	*addr;		/* malloc address */
	long	link;		/* link from malloc block */
	int	type;		/* allocate or free operation */
} mpool[MPOOL_COUNT];

int mcount = 0;

#define mcount_incr()	if(++mcount == MPOOL_COUNT) \
				mcount = 0; \

#endif
/*
	spdbm_debug_set()-used to dynamically alter the debug state.
*/

spdbm_debug_set(state)
int state;
{
	if(state)
		dbm_debug = 1;
	else dbm_debug = 0;
}

/*
	spdbm_open()-open the specified database file and initialize the
	database manager for tag cache entries, page cache entries, and
	page buffers.
*/

spdbm_open(dbase,mode,buffers)
char *dbase;
int mode, buffers;
{
	long seed;
	int i;
	char *round;

	/* Open the database file */

	mode = mode & (DBM_RDONLY | DBM_RDWR);
	if(mode & DBM_RDONLY)
		dbm_fd = open(dbase,O_RDONLY);
	else
		dbm_fd = open(dbase,O_RDWR);

	if(dbm_fd == -1) {
		perror(MSGSTR(SPDBM_1, "spdbm: error on database open"));
		return(-1);
	}

	/* Read in the Header Page for the Database */

	if(read(dbm_fd,&dbase_hdr,sizeof(struct database_header)) == -1) {
		perror(MSGSTR(SPDBM_2, "spdbm: error on database header read"));
		return(-1);
	}

	/* Zero the statistics for the current open session */

	page_clear(&dbase_hdr.stats.curr,sizeof(struct session_stats),0);

	/* Read in Level 1 Index Pages for Both Tag and IR Partitions */

	lseek(dbm_fd,(long) DBM_PKEY_OFFSET, 0);
	if(read(dbm_fd,tag_level1,DBM_IPAGESIZE) == -1) {
		perror(MSGSTR(SPDBM_3, "spdbm: error on database tag level1 read"));
		return(-1);
	}

	lseek(dbm_fd,(long) DBM_AKEY_OFFSET, 0);
	if(read(dbm_fd,ir_level1,DBM_IPAGESIZE) == -1) {
		perror(MSGSTR(SPDBM_4, "spdbm: error on database ir level1 read"));
		return(-1);
	}

	/* Read in the Single and Multi-Extent Free Page Lists */

	lseek(dbm_fd,(long) DBM_SLEVEL_OFFSET, 0);
	if(read(dbm_fd,slevel_free,DBM_PAGESIZE) == -1) {
		perror(MSGSTR(SPDBM_5, "spdbm: error on database single level free read"));
		return(-1);
	}

	lseek(dbm_fd,(long) DBM_MLEVEL_OFFSET, 0);
	if(read(dbm_fd,mlevel_free,DBM_PAGESIZE) == -1) {
		perror(MSGSTR(SPDBM_6, "spdbm: error on database multi-level free read"));
		return(-1);
	}

	/* Allocate the Level 2 Free Page Bit Mask Buffer */

	if((l2_mask_page = (ulong *) malloc(dbase_hdr.l2size)) ==
	   (ulong *) NULL) {
		printf(MSGSTR(SPDBM_7, "spdbm: error on level2 page mask malloc\n"));
		return(-1);
	}

	dbm_l2_indices = dbase_hdr.pagesize / sizeof(daddr_t);
	dbm_l2_mask_size = dbm_l2_indices / DBM_NBPL;

	lseek(dbm_fd,(long) DBM_L2_MASK_OFFSET, 0);
	if(read(dbm_fd,l2_mask_page,dbase_hdr.l2size) == -1) {
		perror(MSGSTR(SPDBM_8, "spdbm: error on database level2 mask page read"));
		return(-1);
	}

	/* Establish the number of buffers to be allocated */

	if(buffers == 0)
		dbm_buffers = DBM_DEF_BUFFERS / dbase_hdr.pagesize;
	else dbm_buffers = buffers;

	/* Allocate the Tag Cache based on Level 1 Index Count */

	dbm_tag_cache = dbase_hdr.tag_cache_entries;
	if((tcache = (struct tag_cache *) malloc(DBM_TCACHE_SIZE))
	   == (struct tag_cache *) NULL) {
		printf(MSGSTR(SPDBM_9, "spdbm: error on tag cache malloc\n"));
		return(-1);
	}

	tcache_clear();

	/* Allocate the Page Cache based on Command Block Parameters */

	dbm_page_cache = dbm_buffers;
	if((pcache = (struct page_cache *) malloc(dbm_page_cache * 
	   sizeof(struct page_cache))) == (struct page_cache *) NULL) {
		printf(MSGSTR(SPDBM_10, "spdbm: error on page cache malloc\n"));
		return(-1);
	}

	pcache_clear();

	/* Allocate the I/O Page Buffers for Index/Data Page Operations */

	if((dbm_single = (struct buf_cblock *)
	   malloc(dbm_buffers * sizeof(struct buf_cblock))) == 
	   ((struct buf_cblock *) 0)) {
		printf(MSGSTR(SPDBM_11, "spdbm: error on malloc of buffer control block\n"));
		return(-1);
	}

	dbm_buf_pool_size = dbm_buffers * dbase_hdr.pagesize + 
		(dbase_hdr.pagesize - 1);

	if((dbm_buf_pool = 
	   (int *) malloc(dbm_buf_pool_size)) == (int *) NULL) {
		printf(MSGSTR(SPDBM_12, "spdbm: error on buffer pool malloc\n"));
		return(-1);
	}

	/* Round the first buffer address to even multiple of dbase_hdr.pagesize
	   and make address entry for each buffer in memory pool array.  */

	round = (char *) (((int)dbm_buf_pool + 
		   (dbase_hdr.pagesize - 1)) & ~(dbase_hdr.pagesize - 1));

	for(i=0; i < dbm_buffers; i++) {
		dbm_single[i].addr = (caddr_t) round;
		dbm_single[i].daddr = 0L;
		dbm_single[i].time = 0L;
		dbm_single[i].flags.allocated = 0;

		round = (char *) ((int) round + dbase_hdr.pagesize);
	}

	/* Prime the random number generator for tag allocation */

	seed = time((long *) 0);
	srand48(seed);

	dbase_hdr.flags |= (DBM_OPEN | mode);
	return(0);
}

/*
	spdbm_close()-close the database and release all buffer and cache
	memory. The database header block is flushed to disk with the
	updated statistics.
*/

spdbm_close()
{


	/* Flush the primary and alternate master index pages */

	if((dbase_hdr.flags & DBM_OPEN) == 0)
		return(0);
	else dbase_hdr.flags &= ~DBM_OPEN;

	/* Only flush pages if open mode is RDWR */

	if((dbase_hdr.flags & DBM_RDONLY) == 0) {
		if((spdbm_flush(DBM_FLUSH_TAG_L1) == -1) ||
		   (spdbm_flush(DBM_FLUSH_IR_L1) == -1))
			return(-1);

		dbase_hdr.flags &= ~(DBM_IPAGE_FLUSH);

		/* Flush the single and multi-level free block lists */

		if((spdbm_flush(DBM_FLUSH_SFREE) == -1) ||
		   (spdbm_flush(DBM_FLUSH_MFREE) == -1))
			return(-1);

		/* Flush the database header to disk */

		dbase_hdr.flags &= ~(DBM_RDONLY | DBM_RDWR);
		if(spdbm_flush(DBM_FLUSH_DBHDR) == -1)
			return(-1);

		/* Re-write the Level 2 Free Page Bit Mask */

		if(spdbm_flush(DBM_FLUSH_L2_MASK) == -1)
			return(-1);
	}

	/* Free all dynamically allocated memory and null the pointers */

	free(dbm_buf_pool);
	free(dbm_single);
	free(l2_mask_page);

	/* Check to see if allocated and if so free */

	if(ir_delete)
	    free(ir_delete);

	if(tcache)
	    free(tcache);

	if(pcache)
	    free(pcache);

	close(dbm_fd);

	dbm_buffers = dbm_fd = 0;
	tcache = (struct tag_cache *) NULL;
	pcache = (struct page_cache *) NULL;
	dbm_buf_pool = (int *) NULL;
	ir_delete = (char *) NULL;
	dbm_single = (struct buf_cblock *) NULL;
	l2_mask_page = (ulong *) NULL;
	return(0);
}

/*
	spdbm_reopen()-repoen the database in read only mode for daemon
	shutdown. All lookups can be done but no database writes will
	take place. Therefore, decisions are possible as are mapping
	tags for those that already IRs in the database. No new mappings
	or inserts can be done.
*/

spdbm_reopen(dbase,mode)
char *dbase;
int mode;
{
	/* If the new mode is current mode just return */

	if(((dbase_hdr.flags & DBM_OPEN) == 0) || (mode & dbase_hdr.flags))
		return(0);

	/* Do the required processing for each mode */

	switch(mode) {

	   case DBM_RDONLY:

		if((dbase_hdr.flags & DBM_RDWR) == 0)
			return(0);

		if((spdbm_flush(DBM_FLUSH_TAG_L1) == -1) ||
		   (spdbm_flush(DBM_FLUSH_IR_L1) == -1))
			return(-1);

		dbase_hdr.flags &= ~(DBM_IPAGE_FLUSH);

		/* Flush the single and multi-level free block lists */

		if((spdbm_flush(DBM_FLUSH_SFREE) == -1) || 
		   (spdbm_flush(DBM_FLUSH_MFREE) == -1))
			return(-1);

		/* Flush the database header to disk */

		if(spdbm_flush(DBM_FLUSH_DBHDR) == -1)
			return(-1);

		/* Re-write the Level 2 Free Page Bit Mask */

		if(spdbm_flush(DBM_FLUSH_L2_MASK) == -1)
			return(-1);

		close(dbm_fd);
		if((dbm_fd = open(dbase,O_RDONLY)) == -1) {
			perror(MSGSTR(SPDBM_13, "spdbm: error on database reopen"));
			return(-1);
		}

		dbase_hdr.flags &= ~(DBM_RDONLY | DBM_RDWR);
		dbase_hdr.flags |= DBM_RDONLY;
		break;

	   case DBM_RDWR:

		if((dbase_hdr.flags & DBM_RDONLY) == 0)
			return(0);

		close(dbm_fd);
		if((dbm_fd = open(dbase,O_RDWR)) == -1) {
			perror(MSGSTR(SPDBM_13, "spdbm: error on database reopen"));
			return(-1);
		}

		dbase_hdr.flags &= ~(DBM_RDONLY | DBM_RDWR);
		dbase_hdr.flags |= DBM_RDWR;
		break;

	}

	return(0);
}

/*
	spdbm_tag_allocate()-this function is designed to return a tag to
	the caller for the specified security attribute. The attribute
	may already exist in the database in which case the existing tag
	is returned. Otherwise, a new tag is allocated and the attribute
	is stored in the database with the new tag value.
*/

spdbm_tag_allocate(ir_addr, ir_length, tag_buffer, obj_type)
char *ir_addr;
int ir_length;
tag_t *tag_buffer;
int obj_type;
{
	struct dbase_rechdr *recp;
	int i, j, index;

	if((dbase_hdr.flags & DBM_OPEN) == 0)
		return(-1);

	/* See if the IR already has a Tag assigned, if so share it */

	if((recp = spdbm_lookup(DBM_IR,ir_addr,ir_length,obj_type)) != NULL_RECP) {
		for(i=0; i < SEC_TAG_SIZE; i++)
			*(tag_buffer + i) = recp->tag[i];

		return(0);
	}

	/* Return error if the database is open readonly */

	if(dbase_hdr.flags & DBM_RDONLY)
		return(-1);

	/* The IR was not in the database so allocate a tag in a cluster
	   which is below threshold if a free page is available.	*/

	/* Get the Level 1 Slot for Next Free Page */

start:
	for(i=0; i < DBM_L1_MASKSIZE; i++) {
		for(j=DBM_NBPL - 1; j >= 0; j--) {
			if((dbase_hdr.l1_mask[i] & (1L << j)) == 0)
				goto found1;
		}
	}

	goto random;

	/* NOTE: Dependency here on number of longs in tag */

found1:
	dbm_l1_slot = (i * DBM_NBPL) + (DBM_NBPL - 1 - j);

	/* Search the Level 2 Page Masks for the Selected Level 1 Slot */

	for(i=0; i < dbm_l2_mask_size; i++) {
		for(j=DBM_NBPL - 1; j >= 0; j--) {
			index = (dbm_l1_slot * dbm_l2_mask_size) + i;
			if((l2_mask_page[index] & (1L << j)) == 0)
				goto found2;
		}
	}

	/* No free pages for that slot, mark it and move to next */

	dbase_hdr.l1_mask[dbm_l1_slot / DBM_NBPL] |= 
	   1L << ((DBM_NBPL - 1) - (dbm_l1_slot % DBM_NBPL));
	goto start;
	
found2:
	dbm_l2_slot = (i * DBM_NBPL) + (DBM_NBPL - 1 - j);

	/* Randomly allocate the level 2 tag forcing the hash relevant
	   portion to be the value of the selected level 2 slot.  */

	do {
		tag[0] = (lrand48() & ~(dbm_l2_indices - 1)) | dbm_l2_slot;

	/* Modify the high order level 1 slot component to hash to L1 slot */

		tag[0] = (tag[0] & ~DBM_L1_SLOTMASK) 
			| (dbm_l1_slot << DBM_L1_SHIFT);

		if(tag[0] < DBM_MIN_TAG)
			continue;

		if(spdbm_lookup(DBM_TAG,tag,SEC_TAG_SIZE,obj_type) != NULL_RECP) {
			dbase_hdr.stats.curr.tag_collisions++;
			dbase_hdr.stats.perm.tag_collisions++;
			continue;
		}

		for(i=0; i < SEC_TAG_SIZE; i++)
			*(tag_buffer + i) = tag[i];

		return(spdbm_insert(DBM_BOTH,tag,ir_addr,ir_length,obj_type));

	} while(1);

	/* Allocate a random tag and make sure it is not already allocated */

random:
	do {
		for(i=0; i < SEC_TAG_SIZE; i++)
			tag[i] = lrand48();

		if(spdbm_lookup(DBM_TAG,tag,SEC_TAG_SIZE,obj_type) != NULL_RECP)
			continue;

		for(i=0; i < SEC_TAG_SIZE; i++)
			*(tag_buffer + i) = tag[i];

		return(spdbm_insert(DBM_BOTH,tag,ir_addr,ir_length,obj_type));

	} while(1);
}

/*
	spdbm_tag_deallocate()-deallocate the specified tag by deleting the
	tag and the accompanying security attribute records from the
	database. The tag may subsequently be reused.
*/

spdbm_tag_deallocate(tag_addr,obj_type)
tag_t *tag_addr;
int obj_type;
{

	if(((dbase_hdr.flags & DBM_OPEN) == 0) || (dbase_hdr.flags & DBM_RDONLY))
		return(-1);

	return(spdbm_delete(DBM_BOTH,tag_addr,0,0,obj_type));
}

/*
	spdbm_get_ir()-retrieve the internal representation of the security
	attribute from the database given the record Tag.
*/

spdbm_get_ir(tag_addr,ir_buffer,ir_length)
tag_t *tag_addr;
char *ir_buffer;
int *ir_length;
{
	struct dbase_rechdr *recp;

	if((dbase_hdr.flags & DBM_OPEN) == 0)
		return(-1);

	if((recp = spdbm_lookup(DBM_TAG,tag_addr,SEC_TAG_SIZE,DBM_TEMPORARY)) 
	   == NULL_RECP)
		return(-1);

	*ir_length = recp->recsize - RECSIZE;
	spdbm_reccopy((int)recp + RECSIZE,ir_buffer,*ir_length);
	return(0);
}

/*
	spdbm_get_stats()-retrieve the database statistics for the current
	session as well as the aggragate statistic counts for all
	cumulative sessions.
*/

spdbm_get_stats(stats_buffer)
struct dbm_stats *stats_buffer;
{
	spdbm_reccopy(&dbase_hdr.stats,stats_buffer,sizeof(struct dbm_stats));
}

/*-------------------------------------------------------------------------*/
/*									   *
 *   The Remaining Routines are NOT Security Policy Visible Entry Points   *
 *									   *
 *-------------------------------------------------------------------------*/

/*
	spdbm_lookup()-lookup the record specified by the key in either
	the Tag or IR Partition of the database. The page containing
	the record is first located. If this locate returns a buffer
	address of 0 then the page is not in the database. Otherwise,
	the buffer is searched for a match on the key. The record
	offset or -1 is returned to indicate success.
*/

struct dbase_rechdr *
spdbm_lookup(func,attr,length,obj_type)
int func;
char *attr;
int length, obj_type;
{
	struct tag_header *thdr;
	struct ir_header *irhdr;
	struct dbase_rechdr *recp;
	int compare, pagesize;
	int space_occupied;

	/* Locate the page where the record should reside */

	if((thdr = (struct tag_header *) spdbm_page_locate(func,attr,length,
	    DBM_NO_CREATE)) == 0)
		return(NULL_RECP);

	/* Database Lookup with Tag Key */

	if(func == DBM_TAG) {
		dbase_hdr.stats.curr.pkey_lookups++;
		dbase_hdr.stats.perm.pkey_lookups++;

		if((spdbm_tag_compare(attr,thdr->low_tag,length) == 
		    DBM_SECOND_KEY_GREATER) ||
		   (spdbm_tag_compare(attr,thdr->high_tag,length) == 
		    DBM_FIRST_KEY_GREATER)) {
			page_release(thdr);
			return(NULL_RECP);
		}

		recp = (struct dbase_rechdr *) ((int)thdr + THDR_SIZE);
		pagesize = thdr->pgsize.pgsize - THDR_SIZE;

		while(pagesize > 0) {
			compare = spdbm_tag_compare(attr,recp->tag,length);

			if(compare == DBM_KEYS_EQUAL) {
				if(obj_type == DBM_PERMANENT)
					recp->ref_count++;
				page_release(thdr);
				return(recp);
			}
			else if(compare == DBM_SECOND_KEY_GREATER) {
				page_release(thdr);
				return(NULL_RECP);
			}

			space_occupied = roundup(recp->recsize,4);
			pagesize -= space_occupied;
			recp = (struct dbase_rechdr *) ((int)recp + space_occupied);
		}

	/* No match on Key */

		page_release(thdr);
		return(NULL_RECP);
	}

	/* Database Lookup with IR Key */

	else {
		dbase_hdr.stats.curr.akey_lookups++;
		dbase_hdr.stats.perm.akey_lookups++;

		irhdr = (struct ir_header *)thdr;
		recp = (struct dbase_rechdr *) ((int)irhdr + IRHDR_SIZE);
		pagesize = irhdr->pgsize.pgsize - IRHDR_SIZE;

		while(pagesize > 0) {
			compare = spdbm_key_compare(attr,(int)recp + RECSIZE,
			   length,recp->recsize - RECSIZE);

			if(compare == DBM_KEYS_EQUAL) {
				page_release(irhdr);
				return(recp);
			}
			else if(compare == DBM_SECOND_KEY_GREATER) {
				page_release(irhdr);
				return(NULL_RECP);
			}

			space_occupied = roundup(recp->recsize,4);
			pagesize -= space_occupied;
			recp = (struct dbase_rechdr *) ((int)recp + space_occupied);
		}

	/* No match on Key */

		page_release(irhdr);
		return(NULL_RECP);
	}
}

/*
	spdbm_insert()-insert the Tag/IR pair into the database. The
	page for the record is first located, creating any newly
	required pages along the way. The page is searched to find
	the location for the new record including duplicate record
	detection. The record is then inserted. If a page is not
	large enough to support the new record the page is extended.
*/

spdbm_insert(func,tag_addr,ir_addr,ir_length,obj_type)
int func;
tag_t *tag_addr;
char *ir_addr;
int ir_length, obj_type;
{
	struct tag_header *thdr;
	struct ir_header *irhdr;
	struct dbase_rechdr *recp;
	int i, compare, pagesize, index;
	int space_occupied;

	if((func == DBM_TAG) || (func == DBM_BOTH)) {
#ifdef DEBUG
	if(dbm_debug)
	   printf(MSGSTR(SPDBM_14, "***** TAG INSERT: %x *****\n"),*tag_addr);
#endif
	   thdr = (struct tag_header *) spdbm_page_locate(DBM_TAG,tag_addr,0,
		DBM_CREATE);

retry1:
	space_occupied = roundup(RECSIZE + ir_length, 4);
	if((freepage(thdr) - THDR_SIZE) > space_occupied) {

	/* Search the page until the insertion location is found. */

		recp = (struct dbase_rechdr *) ((int)thdr + THDR_SIZE);
		pagesize = thdr->pgsize.pgsize - THDR_SIZE;

		while(pagesize > 0) {
			compare = spdbm_tag_compare(tag_addr,recp->tag,SEC_TAG_SIZE);

			if(compare == DBM_KEYS_EQUAL) {
				printf(MSGSTR(SPDBM_15, "spdbm: duplicate key on insert\n"));
				page_release(thdr);
				return(-1);
			}
			else if(compare == DBM_SECOND_KEY_GREATER)
				break;

			space_occupied = roundup(recp->recsize, 4);
			pagesize -= space_occupied;
			recp = (struct dbase_rechdr *) ((int)recp + space_occupied);

		}

	/* Move the remainder of the page down to make room */

		if(pagesize > 0) {
		   space_occupied = roundup(RECSIZE+ir_length, 4);
		   spdbm_backcopy(recp,(int)recp + space_occupied,pagesize);
		}
		recp->recsize = RECSIZE + ir_length;

	/* Set the record count to 1 if Permanent else set to 0 */

		if(obj_type == DBM_PERMANENT)
			recp->ref_count = 1;
		else
			recp->ref_count = 0;

	/* Update the new record tag field */

		for(i=0; i < SEC_TAG_SIZE; i++)
			recp->tag[i] = *(tag_addr + i);

	/* Insert the internal representation into the new record */

		spdbm_reccopy(ir_addr,(int)recp + RECSIZE,ir_length);
		space_occupied = roundup(recp->recsize,4);
		thdr->pgsize.pgsize += space_occupied;

	/* If page exceeds free space threshold indicate in page mask */

		if((thdr->pgsize.pgsize > dbase_hdr.threshold) &&
		   (thdr->pgsize.mask_set == 0)) {
			thdr->pgsize.mask_set = 1;
			spdbm_set_tag_slots(tag_addr);
			index = (dbm_l1_slot * dbm_l2_mask_size) + 
			   (dbm_l2_slot / DBM_NBPL);
			l2_mask_page[index] |= 
			   (1L << ((DBM_NBPL - 1) - (dbm_l2_slot % DBM_NBPL)));
			if(spdbm_flush(DBM_FLUSH_L2_MASK) == -1) {
				perror(MSGSTR(SPDBM_16, "spdbm: error on l2 mask flush"));
				return(-1);
			}
		}

	/* If key is lower than current low key, update page header */

		if((spdbm_tag_compare(tag_addr,thdr->low_tag,
		   SEC_TAG_SIZE) == DBM_SECOND_KEY_GREATER) ||
		   (spdbm_tag_compare(null_tag,thdr->low_tag,
		   SEC_TAG_SIZE) == DBM_KEYS_EQUAL)) {

			for(i=0; i < SEC_TAG_SIZE; i++)
				thdr->low_tag[i] = *(tag_addr + i);
		}

	/* If key is higher than current high key, update page header */

		if((spdbm_tag_compare(tag_addr,thdr->high_tag,
		   SEC_TAG_SIZE) == DBM_FIRST_KEY_GREATER) ||
		   (spdbm_tag_compare(null_tag,thdr->low_tag,
		   SEC_TAG_SIZE) == DBM_KEYS_EQUAL)) {

			for(i=0; i < SEC_TAG_SIZE; i++)
				thdr->high_tag[i] = *(tag_addr + i);
		}

	/* Flush the modified page to the database */

		page_write(thdr,DBM_TAG_PAGE);

		dbase_hdr.stats.pkey_entries++;
		dbase_hdr.stats.curr.pkey_inserts++;
		dbase_hdr.stats.perm.pkey_inserts++;

	   }

	/* Insufficient room, extend the page, invalidate the tag cache
	   entry for the tag since the page has changed and retry */

	   else {
		thdr = (struct tag_header *) page_extend(thdr,DBM_TAG_PAGE,
			tag_addr,SEC_TAG_SIZE);
		tcache_invalidate(tag_addr);
		goto retry1;
	   }

	}

	/* Insert the new record into the IR partition if specified */

	if((func == DBM_IR) || (func == DBM_BOTH)) {
#ifdef DEBUG
	if(dbm_debug)
	   printf(MSGSTR(SPDBM_17, "***** IR INSERT *****\n"));
#endif
	   irhdr = (struct ir_header *) spdbm_page_locate(DBM_IR,ir_addr,
		ir_length,DBM_CREATE);

retry2:
	space_occupied = roundup(RECSIZE+ir_length,4);
	if((freepage(irhdr) - IRHDR_SIZE) > space_occupied) {

	/* Search the page until the insertion location is found. */

		recp = (struct dbase_rechdr *) ((int)irhdr + IRHDR_SIZE);
		pagesize = irhdr->pgsize.pgsize - IRHDR_SIZE;

		while(pagesize > 0) {
			compare = spdbm_key_compare(ir_addr,(int)recp + RECSIZE,
			   ir_length,recp->recsize - RECSIZE);

			if(compare == DBM_KEYS_EQUAL) {
				printf(MSGSTR(SPDBM_15, "spdbm: duplicate key on insert\n"));
				page_release(irhdr);
				return(-1);
			}
			else if(compare == DBM_SECOND_KEY_GREATER)
				break;

			space_occupied = roundup(recp->recsize,4);
			pagesize -= space_occupied;
			recp = (struct dbase_rechdr *) ((int)recp + space_occupied);

		}

	/* Move the remainder of the page down to make room */

		space_occupied = roundup(RECSIZE+ir_length,4);

		if(pagesize > 0)
		   spdbm_backcopy(recp,(int)recp + space_occupied,pagesize);
		recp->recsize = RECSIZE + ir_length;

	/* Update the new record tag field */

		for(i=0; i < SEC_TAG_SIZE; i++)
			recp->tag[i] = *(tag_addr + i);

	/* Insert the internal representation into the new record */

		spdbm_reccopy(ir_addr,(int)recp + RECSIZE,ir_length);
		irhdr->pgsize.pgsize += space_occupied;

	/* Flush the page to the database */

		page_write(irhdr,DBM_IR_PAGE);

		dbase_hdr.stats.akey_entries++;
		dbase_hdr.stats.curr.akey_inserts++;
		dbase_hdr.stats.perm.akey_inserts++;

	   }

	/* Insufficient room, extend the page and retry */

	   else {
		irhdr = (struct ir_header *) page_extend(irhdr,DBM_IR_PAGE,
			ir_addr,ir_length);
		goto retry2;
	   }
	}

	return(0);

}

/*
	spdbm_delete()-delete the record specified by the key from
	the database. The data page is located and then is searched
	until the record is found. When found, the page is compresed
	to remove the record. If a tag page, the low or high key field
	is updated if necessary.
*/

spdbm_delete(func,tag_addr,ir_addr,ir_length,obj_type)
int func;
tag_t *tag_addr;
char *ir_addr;
int ir_length, obj_type;
{
	struct tag_header *thdr;
	struct ir_header *irhdr;
	struct dbase_rechdr *recp, *lastrecp;
	daddr_t daddr;
	int i, compare, pagesize, index;
	int space_occupied;

	if((func == DBM_TAG) || (func == DBM_BOTH)) {
#ifdef DEBUG
	if(dbm_debug)
		printf(MSGSTR(SPDBM_18, "***** TAG DELETE *****\n"));
#endif
		thdr = (struct tag_header *) spdbm_page_locate(DBM_TAG,tag_addr,
			0,DBM_NO_CREATE);

	/* Check the desired Tag against the low and high key on the
	   page. If out of range, then the record does not exist. */

		if((spdbm_tag_compare(tag_addr,thdr->low_tag,
		   SEC_TAG_SIZE) == DBM_SECOND_KEY_GREATER) ||
		   (spdbm_tag_compare(tag_addr,thdr->high_tag,
		   SEC_TAG_SIZE) == DBM_FIRST_KEY_GREATER)) {
			page_release(thdr);
			return(-1);
		}

	/* Search the page to locate the record to be deleted. */

		recp = (struct dbase_rechdr *) ((int)thdr + THDR_SIZE);
		pagesize = thdr->pgsize.pgsize - THDR_SIZE;

		while(pagesize > 0) {
			compare = spdbm_tag_compare(tag_addr,recp->tag,
			   SEC_TAG_SIZE);

			if(compare == DBM_KEYS_EQUAL)
				break;
			else if(compare == DBM_SECOND_KEY_GREATER) {
				page_release(thdr);
				return(-1);
			}

			space_occupied = roundup(recp->recsize,4);
			pagesize -= space_occupied;
			lastrecp = recp;
			recp = (struct dbase_rechdr *) ((int)recp + space_occupied);

		}

		if(compare != DBM_KEYS_EQUAL) {
			page_release(thdr);
			return(-1);
		}

	/* Only delete the record if the reference count has reached zero */

		if(obj_type == DBM_PERMANENT)  {
			if(--(recp->ref_count) != 0) {
				page_release(thdr);
				return(0);
			}
		}
		else if(recp->ref_count != 0) {
			page_release(thdr);
			return(0);
		     }
			
	/* If a delete for the IR as well, move the IR to delete buffer
	   since the internal representation is not passed on the
	   tag deallocate request. It must therefore be extracted from
	   the record and saved for the second half of the delete.  */

		if(func == DBM_BOTH) {
			ir_length = recp->recsize - RECSIZE;
			if((ir_delete = malloc(ir_length)) == (char *) NULL) {
				printf(MSGSTR(SPDBM_19, "spdbm: unable to malloc for ir delete\n"));
				page_release(thdr);
				return(-1);
			}

			ir_addr = ir_delete;

			spdbm_reccopy((int)recp + RECSIZE,ir_delete,ir_length);
		}

	/* Update pagesize to reflect the deleted record. The page is empty
	   and can be deallocated if the size is equal to the size of the
	   page header structure.					*/

		space_occupied = roundup(recp->recsize,4);
		thdr->pgsize.pgsize -= space_occupied;

	/* If page less than free space threshold indicate in page mask */

		if((thdr->pgsize.pgsize < dbase_hdr.threshold) &&
		   (thdr->pgsize.mask_set == 1)) {
			thdr->pgsize.mask_set = 0;
			spdbm_set_tag_slots(tag_addr);

	/* Indicate the Level 2 Page Has Free Space */

			index = (dbm_l1_slot * dbm_l2_mask_size) + 
			   (dbm_l2_slot / DBM_NBPL);
			l2_mask_page[index] &= 
			   ~(1L << ((DBM_NBPL - 1) - (dbm_l2_slot % DBM_NBPL)));

	/* Reset the Level 1 Page Mask to Indicate Free Page Space */

			dbase_hdr.l1_mask[dbm_l1_slot / DBM_NBPL] &= 
			   ~(1L << ((DBM_NBPL - 1) - (dbm_l1_slot % DBM_NBPL)));

			if((spdbm_flush(DBM_FLUSH_L2_MASK) == -1) ||
			   (spdbm_flush(DBM_FLUSH_DBHDR) == -1)) {
				perror(MSGSTR(SPDBM_20, "spdbm: error on flush from delete"));
				return(-1);
			}
		}

	/* If the page is empty, free the associated disk block and the
	   page buffer. Also update the level2 index page to indicate
	   that the hash slot no longer has a valid page entry.      */

		if(thdr->pgsize.pgsize == THDR_SIZE) {
			daddr = page_find_daddr(thdr);
			page_block_free(daddr,thdr->pgsize.extent);
			page_buffer_free(thdr,thdr->pgsize.pgtype);
			spdbm_level2_update(DBM_TAG_PAGE,tag_addr,SEC_TAG_SIZE,0L);
			tcache_invalidate(tag_addr);
			goto stats1;
		}

	/* Compress the remainder of the page to delete the record */

		spdbm_reccopy((int)recp + space_occupied,recp,
		   pagesize - space_occupied);

	/* If the record was the low key on page update low key */

		if(spdbm_tag_compare(tag_addr,thdr->low_tag,
		   SEC_TAG_SIZE) == DBM_KEYS_EQUAL) {
			if(thdr->pgsize.pgsize > THDR_SIZE)
				for(i=0; i < SEC_TAG_SIZE; i++)
					thdr->low_tag[i] = recp->tag[i];
			else
				for(i=0; i < SEC_TAG_SIZE; i++)
					thdr->low_tag[i] = null_tag[i];
		}

	/* If the record was the high key on page update high key */

		if(spdbm_tag_compare(tag_addr,thdr->high_tag,
		   SEC_TAG_SIZE) == DBM_KEYS_EQUAL) {
			if(thdr->pgsize.pgsize > THDR_SIZE)
				for(i=0; i < SEC_TAG_SIZE; i++)
					thdr->high_tag[i] = recp->tag[i];
			else
				for(i=0; i < SEC_TAG_SIZE; i++)
					thdr->high_tag[i] = null_tag[i];
		}

	/* Flush the updated page copy to disk */

		page_write(thdr,DBM_TAG_PAGE);

	/* Update statistics and insure that cache is invalidated */

stats1:
		dbase_hdr.stats.pkey_entries--;
		dbase_hdr.stats.curr.pkey_deletes++;
		dbase_hdr.stats.perm.pkey_deletes++;

		tcache_invalidate(tag_addr);

	}

	/* Delete the record from the IR partition if specified */

	if((func == DBM_IR) || (func == DBM_BOTH)) {
#ifdef DEBUG
	if(dbm_debug)
	   printf(MSGSTR(SPDBM_21, "***** IR DELETE *****\n"));
#endif
	   irhdr = (struct ir_header *) spdbm_page_locate(DBM_IR,ir_addr,
		ir_length,DBM_NO_CREATE);

	/* Search the page until the record is found. */

		recp = (struct dbase_rechdr *) ((int)irhdr + IRHDR_SIZE);
		pagesize = irhdr->pgsize.pgsize - IRHDR_SIZE;

		while(pagesize > 0) {
			compare = spdbm_key_compare(ir_addr,(int)recp + RECSIZE,
			   ir_length,recp->recsize - RECSIZE);

			if(compare == DBM_KEYS_EQUAL)
				break;
			else if(compare == DBM_SECOND_KEY_GREATER) {
				page_release(irhdr);
				return(-1);
			}

			space_occupied = roundup(recp->recsize,4);
			pagesize -= space_occupied;
			recp = (struct dbase_rechdr *) ((int)recp + space_occupied);

		}

		if(compare != DBM_KEYS_EQUAL) {
			page_release(irhdr);
			return(-1);
		}

	/* Update pagesize, free the buffer and the disk block if empty page */

		space_occupied = roundup(recp->recsize,4);
		irhdr->pgsize.pgsize -= space_occupied;
		if(irhdr->pgsize.pgsize == IRHDR_SIZE) {
			daddr = page_find_daddr(irhdr);
			page_block_free(daddr,irhdr->pgsize.extent);
			page_buffer_free(irhdr,irhdr->pgsize.pgtype);
			spdbm_level2_update(DBM_IR_PAGE,ir_addr,ir_length,0L);
			goto stats2;
		}

	/* Compress the remainder of the page to delete the record */

		spdbm_reccopy((int)recp + space_occupied,recp,
		   pagesize - space_occupied);

		page_write(irhdr,DBM_IR_PAGE);

stats2:
		dbase_hdr.stats.akey_entries--;
		dbase_hdr.stats.curr.akey_deletes++;
		dbase_hdr.stats.perm.akey_deletes++;

		free(ir_delete);
	}

	return(0);

}

/*
	spdbm_page_locate()-locate the database page using the specified
	record key (either Tag or IR). If the locate is done using the
	Tag, the tag cache is checked to see if already in core. If
	not a tag, the IR is hashed to locate the index page offset.
	If the tag entry is not in cache, it also is hashed to locate
	the index page offset. As page offsets are computed, the page
	cache is checked to see if in core. If so, the memory address
	is used. Otherwise, the page is read in using the read page
	routine. If an index or data page is missing for the record key,
	the locate fails.

	The create option, if 1, specifies that index and data pages
	are to be created as needed to suport the record key. It is
	used by spdbm_insert() to return the buffer address for a page
	into which a record can be inserted.

	A by-product of the routine is to lock the actual datapage if
	a page address is found or created. This prevents page stealing
	from occurring until the page is explicitly freed, written, or
	released.
*/

char *
spdbm_page_locate(func,attr,length,create)
int func;
char *attr;
int length, create;
{
	caddr_t addr, l2_addr;
	daddr_t daddr;
	int slot;

	/* Perform a lookup using the TAG-use tag cache */

	if(func == DBM_TAG) {
		if((daddr = tcache_lookup(attr)) != (daddr_t) 0) {
#ifdef DEBUG
		if(dbm_debug)
			printf(MSGSTR(SPDBM_22, "TCACHE Hit: daddr: %x\n"),daddr);
#endif
			if((addr = pcache_lookup(daddr)) != (caddr_t) 0) {
#ifdef DEBUG
			if(dbm_debug)
				printf(MSGSTR(SPDBM_23, "PCACHE Datapage Hit: addr: %x\n"),addr);
#endif
				return(addr);
			}
			else {
				addr = page_read(daddr,DBM_TAG_PAGE);
#ifdef DEBUG
			if(dbm_debug)
				printf(MSGSTR(SPDBM_24, "PCACHE Miss 1: slot: %x addr: %x daddr: %x\n"),slot,addr,daddr);
#endif
				return(addr);
			}
		}

	/* Calculate the level 1 index page slot and retrieve the in
	   core address for the block from cache or read the page in. */

		slot = spdbm_level1_tag_hash(attr,length);
		dbm_l1_slot = slot;

		if((daddr = tag_level1[slot]) == (daddr_t) 0) {
			if(create == DBM_CREATE) {
				l2_addr = page_allocate(&daddr,DBM_INDEX_PAGE);
#ifdef DEBUG
			if(dbm_debug)
				printf(MSGSTR(SPDBM_25, "LEVEL1 allocate: slot: %x addr: %x daddr: %x\n"),slot,l2_addr,daddr);
#endif
				tag_level1[slot] = daddr;
				dbase_hdr.flags |= DBM_IPAGE_FLUSH;
				if(spdbm_flush(DBM_FLUSH_TAG_L1) == -1) {
					perror(MSGSTR(SPDBM_26, "spdbm: error on hdr flush"));
					return((caddr_t) 0);
				}
			}
			else return((caddr_t) 0);
		}
		else if((l2_addr = pcache_lookup(daddr)) == (caddr_t) 0) {
			  l2_addr = page_read(daddr,DBM_INDEX_PAGE);
#ifdef DEBUG
			if(dbm_debug)
			  printf(MSGSTR(SPDBM_27, "PCACHE Miss 2: slot: %x addr: %x daddr: %x\n"),
				slot,l2_addr,daddr);
#endif
		      }

	/* Calculate the level 2 index page slot and retrieve the in
	   core address for the block from cache or read the page in.
	   Flush the level2 index page to disk if newly created only
	   after the data page has been allocated and filled in.  */

		slot = spdbm_level2_tag_hash(attr,length);
		dbm_l2_slot = slot;
#ifdef DEBUG
		if(dbm_debug)
		printf(MSGSTR(SPDBM_28, "LEVEL2 Slot: %x\n"),slot);
#endif
		tag_level2 = (daddr_t *) l2_addr;

		daddr = tag_level2[slot];
		if(daddr == (daddr_t) 0) {
			if(create == DBM_CREATE) {
				addr = page_allocate(&daddr,DBM_TAG_PAGE);
#ifdef DEBUG
			if(dbm_debug)
				printf(MSGSTR(SPDBM_29, "LEVEL2 allocate: slot: %x addr: %x daddr: %x\n"),slot,addr,daddr);
#endif
				tag_level2[slot] = daddr;
				page_write(tag_level2,DBM_INDEX_PAGE);
			}
			else return((caddr_t) 0);
		}
		else if((addr = pcache_lookup(daddr)) == (caddr_t) 0) {
			addr = page_read(daddr,DBM_TAG_PAGE);
#ifdef DEBUG
		if(dbm_debug)
			printf(MSGSTR(SPDBM_30, "PCACHE Miss 3: slot: %x addr: %x daddr: %x\n"),
				slot,addr,daddr);
#endif
		}

	/* Release the lock on the level 2 index page now */

		page_release(l2_addr);

	/* Cache the tag and disk block offset */

#ifdef DEBUG
		if(dbm_debug)
		printf(MSGSTR(SPDBM_31, "LOCATE OK: addr: %x daddr: %x\n"),addr,daddr);
#endif
		tcache_insert(attr,daddr);

	/* Return the result of the page search */

		return(addr);

	}

	/* Perform a search on the IR database partition */

	if(func == DBM_IR) {

	/* Calculate the level 1 index page slot and retrieve the in
	   core address for the block from cache or read the page in. */

		slot = spd_level1_hash(attr,length);

		if((daddr = ir_level1[slot]) == 0)
			if(create == DBM_CREATE) {
				l2_addr = page_allocate(&daddr,DBM_INDEX_PAGE);
#ifdef DEBUG
			if(dbm_debug)
				printf(MSGSTR(SPDBM_32, "IR LEVEL1 allocate: slot: %x addr: %x daddr: %x\n"),slot,l2_addr,daddr);
#endif
				ir_level1[slot] = daddr;
				dbase_hdr.flags |= DBM_IPAGE_FLUSH;
				if(spdbm_flush(DBM_FLUSH_IR_L1) == -1) {
					perror(MSGSTR(SPDBM_26, "spdbm: error on hdr flush"));
					return((caddr_t) 0);
				}
			}
			else return((caddr_t) 0);
		else if((l2_addr = pcache_lookup(daddr)) == 0) {
			l2_addr = page_read(daddr,DBM_INDEX_PAGE);
#ifdef DEBUG
		if(dbm_debug)
			printf(MSGSTR(SPDBM_33, "IR PCACHE Miss 2: slot: %x addr: %x daddr: %x\n"),slot,l2_addr,daddr);
#endif
		      }

	/* Calculate the level 2 index page slot and retrieve the in
	   core address for the block from cache or read the page in. */

		slot = spd_level2_hash(attr,length);
#ifdef DEBUG
		if(dbm_debug)
		printf(MSGSTR(SPDBM_34, "IR LEVEL2 Slot: %x\n"),slot);
#endif
		ir_level2 = (daddr_t *) l2_addr;

		daddr = ir_level2[slot];
		if(daddr == (daddr_t) 0)
			if(create == DBM_CREATE) {
				addr = page_allocate(&daddr,DBM_IR_PAGE);
#ifdef DEBUG
			if(dbm_debug)
				printf(MSGSTR(SPDBM_35, "IR LEVEL2 allocate: slot: %x addr: %x daddr: %x\n"),slot,addr,daddr);
#endif
				ir_level2[slot] = daddr;
				page_write(ir_level2,DBM_INDEX_PAGE);
			}
			else return((caddr_t) 0);
		else if((addr = pcache_lookup(daddr)) == (caddr_t) 0) {
			addr = page_read(daddr,DBM_IR_PAGE);
#ifdef DEBUG
		if(dbm_debug)
			printf(MSGSTR(SPDBM_36, "IR PCACHE Miss 3: slot: %x addr: %x daddr: %x\n"),slot,addr,daddr);
#endif
		}

	/* Release the level2 index page now */

#ifdef DEBUG
		if(dbm_debug)
		printf(MSGSTR(SPDBM_37, "IR LOCATE OK: addr: %x daddr: %x\n"),addr,daddr);
#endif
		page_release(l2_addr);

	/* Return the result of the page search */

		return(addr);

	}
}

/*
	spdbm_flush()-flush selected portions of the database control
	structure based on the low-level actions performed in the
	database manager.
*/

spdbm_flush(function)
int function;
{

	switch(function) {

	   case DBM_FLUSH_DBHDR:	/* database header */

		dbase_hdr.flags &= ~(DBM_RDONLY | DBM_RDWR);
		lseek(dbm_fd,(long) DBM_HEADER_OFFSET, 0);
		if(write(dbm_fd,&dbase_hdr,sizeof(struct database_header)) == -1)
			return(-1);
		break;

	   case DBM_FLUSH_TAG_L1:	/* tag level 1 index */

		lseek(dbm_fd,(long) DBM_PKEY_OFFSET, 0);
		if(write(dbm_fd,tag_level1,DBM_IPAGESIZE) == -1)
			return(-1);
		break;

	   case DBM_FLUSH_IR_L1:	/* ir level 1 index */

		lseek(dbm_fd,(long) DBM_AKEY_OFFSET, 0);
		if(write(dbm_fd,ir_level1,DBM_IPAGESIZE) == -1)
			return(-1);
		break;

	   case DBM_FLUSH_SFREE:	/* single free list */

		lseek(dbm_fd,(long) DBM_SLEVEL_OFFSET, 0);
		if(write(dbm_fd,slevel_free,DBM_PAGESIZE) == -1)
			return(-1);
		break;

	   case DBM_FLUSH_MFREE:	/* multi block free list */

		lseek(dbm_fd,(long) DBM_MLEVEL_OFFSET, 0);
		if(write(dbm_fd,mlevel_free,DBM_PAGESIZE) == -1)
			return(-1);
		break;

	   case DBM_FLUSH_L2_MASK:	/* level 2 page mask */

		lseek(dbm_fd,(long) DBM_L2_MASK_OFFSET, 0);
		if(write(dbm_fd,l2_mask_page,dbase_hdr.l2size) == -1)
			return(-1);
		break;

	   default:
		return(-1);

	}

	return(0);
}

/*
	spdbm_level2_update()-Update the level 2 index page that maps
	the specified Tag or IR attribute since the page containing
	the records for hashed attributes has been changed as the
	result of a page extend during an insert.
*/

spdbm_level2_update(type,attr,length,daddr)
int type, length;
caddr_t attr;
daddr_t daddr;
{
	caddr_t addr;
	daddr_t diskaddr;
	int slot;

	/* Locate and update the level 2 index page for the given Tag */

	if(type == DBM_TAG_PAGE) {

	/* Calculate the level 1 index page slot and retrieve the in
	   core address for the block from cache or read the page in. */

		slot = spdbm_level1_tag_hash(attr,length);

		if((diskaddr = tag_level1[slot]) == (daddr_t) 0) {
			printf(MSGSTR(SPDBM_38, "spdbm: no level 2 page located for extend\n"));
			return(-1);
		}
		else if((addr = pcache_lookup(diskaddr)) == (caddr_t) 0)
			addr = page_read(diskaddr,DBM_INDEX_PAGE);

	/* Hash the level 2 portion of the Tag and update index slot.
	   The current level 2 index page address is also invalidated
	   in the cache and the new address pair is inserted.	    */

		slot = spdbm_level2_tag_hash(attr,length);

		tag_level2 = (daddr_t *) addr;

		pcache_invalidate(tag_level2[slot]);
		pcache_insert(addr,diskaddr);

		tag_level2[slot] = daddr;
		page_write(tag_level2,DBM_INDEX_PAGE);
	}

	/* Locate the level2 page for the specified IR and update the index */

	else {

	/* Calculate the level 1 index page slot and retrieve the in
	   core address for the block from cache or read the page in. */

		slot = spd_level1_hash(attr,length);

		if((diskaddr = ir_level1[slot]) == (daddr_t) 0) {
			printf(MSGSTR(SPDBM_38, "spdbm: no level 2 page located for extend\n"));
			return(-1);
		}
		else if((addr = pcache_lookup(diskaddr)) == (caddr_t) 0)
			addr = page_read(diskaddr,DBM_INDEX_PAGE);

	/* Hash the level 2 portion of the IR and update index slot.
	   The current level 2 index page address is also invalidated
	   in the cahce and the new address pair is inserted.	    */

		slot = spd_level2_hash(attr,length);

		ir_level2 = (daddr_t *) addr;

		pcache_invalidate(ir_level2[slot]);
		pcache_insert(addr,diskaddr);

		ir_level2[slot] = daddr;
		page_write(ir_level2,DBM_INDEX_PAGE);
	}
}

/*
	spdbm_key_compare()-generic key comparison routine that will
	return an integer reflecting the outcome of a byte by byte
	comparison between the keys (internal representations only).

	Returns:
		-1 (DBM_FIRST_KEY_GREATER)   if Key1 > Key2
		 0 (DBM_KEYS_EQUAL)          if Key1 = Key2
		+1 (DBM_SECOND_KEY_GREATER)  if Key1 < Key2
*/

spdbm_key_compare(key1,key2,key1_length,key2_length)
unsigned char *key1, *key2;
int key1_length, key2_length;
{

	/* Compare until mismatch or one key length is exhausted */

	while((key1_length > 0) && (key2_length > 0)) {
		if(*key1 == *key2) {
			key1++;
			key2++;
			key1_length--;
			key2_length--;
			continue;
		}
		else break;
	}

	/* Determine if equal or which key is greater */

	if((key1_length == 0) && (key2_length == 0))
		return(DBM_KEYS_EQUAL);

	if((key1_length == 0) ||
	   ((key2_length > 0) && (*key1 < *key2)))
		return(DBM_SECOND_KEY_GREATER);

	if((key2_length == 0) ||
	   ((key1_length > 0) && (*key2 < *key1)))
		return(DBM_FIRST_KEY_GREATER);

}

/*
	spdbm_tag_compare()-generic tag comparison routine that will
	return an integer reflecting the outcome of a comparison 
	between the tag longwords. This routine is used so that the
	actual number of longs in the tag can be made independent
	of the routines that use the tags.

	Returns:
		-1 (DBM_FIRST_KEY_GREATER)   if Tag1 > Tag2
		 0 (DBM_KEYS_EQUAL)          if Tag1 = Tag2
		+1 (DBM_SECOND_KEY_GREATER)  if Tag1 < Tag2
*/

spdbm_tag_compare(tag1,tag2,tag_length)
tag_t *tag1, *tag2;
int tag_length;
{

	/* Compare until mismatch or tag length is exhausted */

	while(tag_length > 0) {
		if(*tag1 == *tag2) {
			tag1++;
			tag2++;
			tag_length--;
			continue;
		}
		else break;
	}

	/* Determine if equal or which tag is greater */

	if(tag_length == 0)
		return(DBM_KEYS_EQUAL);

	if(*tag1 < *tag2)
		return(DBM_SECOND_KEY_GREATER);

	if(*tag2 < *tag1)
		return(DBM_FIRST_KEY_GREATER);

}

/*
	spdbm_backcopy()-copy the page contents further down in the page
	to make room for an inserted record. The copy must be done in
	reverse to avoid a move overlap problem.
*/

spdbm_backcopy(src,dest,length)
char *src, *dest;
int length;
{
	src = (char *) (int) src + length - 1;
	dest = (char *) (int) dest + length - 1;

	while(length-- > 0)
		*dest-- = *src--;
}

/*
	spdbm_reccopy()-forward copy used like bcopy() to move attributes
	around.
*/

spdbm_reccopy(src,dest,length)
char *src, *dest;
int length;
{
	while(length-- > 0)
		*dest++ = *src++;
}

/*
	spdbm_level1_tag_hash()-used to hash the tag value at the specified
	address to a level1 index table slot.
*/

spdbm_level1_tag_hash(tag_addr)
tag_t *tag_addr;
{
	return((*tag_addr >> DBM_L1_SHIFT) % DBM_LEVEL1_INDICES);
}

/*
	spdbm_level2_tag_hash()-used to hash the tag value at the specified
	address to a level2 index table slot.
*/

spdbm_level2_tag_hash(tag_addr)
tag_t *tag_addr;
{
	return((*tag_addr & ~DBM_L1_SLOTMASK) 
		% (dbase_hdr.pagesize / sizeof(daddr_t)));
}

/*
	spdbm_tag_hash()-used to hash the specified tag into a tag cache
	slot. The function is a simple cache based on the number of 
	actual tag cache entries.
*/

spdbm_tag_hash(tag_addr)
tag_t *tag_addr;
{
	return(*tag_addr % dbm_tag_cache);
}

/*
	spdbm_page_hash()-used to hash the specified disk block offset into
	a page cache slot. The function is a simple cache based on the
	number of actual page cache buffers.
*/

spdbm_page_hash(daddr)
daddr_t daddr;
{
	return(daddr % dbm_page_cache);
}

/*
	TAG Cache Routines

	These routines provide cache management functions for the TAG Cache
	which provides a quick mapping from Tag values to Disk offsets for
	the page on which the tag resides. This can avoid the necessity of
	having to perform I/O on index pages to locate the data page offset
	of the tag. In this manner, index I/O is reduced which can result in
	much more effective utilization of the I/O page cache.
*/

/*
	tcache_insert()-insert an entry into the tag cache-the size of
	the tag is configurable.
*/

tcache_insert(tag_addr,offset)
tag_t *tag_addr;
daddr_t offset;
{
	int i, slot, bucket;

#ifdef DEBUG
	if(dbm_debug)
	printf(MSGSTR(SPDBM_39, "tcache_insert addr: %x daddr: %x\n"),tag_addr,offset);
#endif
	slot = spdbm_tag_hash(tag_addr);

	/* Avoid replicating valid entries into multiple buckets */

	for(i=0; i < TCACHE_BUCKETS; i++) {
		if((tcache[slot].bucket[i].tag[0] == *tag) &&
		   (tcache[slot].bucket[i].tag[1] == *(tag + 1)) &&
		   (tcache[slot].bucket[i].flags.valid == 1))
			return(0);
	}

	/* Determine the bucket number in the slot and increment */

	bucket = tcache[slot].lru_bucket;

	tcache[slot].lru_bucket++;
	tcache[slot].lru_bucket = tcache[slot].lru_bucket % TCACHE_BUCKETS;

	/* Insert the entry into the cache bucket and make it valid */

	tcache[slot].bucket[bucket].flags.valid = 1;
	tcache[slot].bucket[bucket].daddr = offset;

	for(i=0; i < SEC_TAG_SIZE; i++)
		tcache[slot].bucket[bucket].tag[i] = *(tag_addr + i);

	return(0);
}

/*
	tcache_lookup()-lookup an entry for a tag to see if its is
	already in the cache. Return 0 or the disk offset.
*/

daddr_t
tcache_lookup(tag_addr)
tag_t *tag_addr;
{
	int i, j, slot, match;

	slot = spdbm_tag_hash(tag_addr);

	for(i=0; i < TCACHE_BUCKETS; i++) {

	   match = 0;		/* Set if tags entries match */

	   if(tcache[slot].bucket[i].flags.valid == 1) {
		for(j=0; j < SEC_TAG_SIZE; j++ ) {
			if(tcache[slot].bucket[i].tag[j] == *(tag_addr + j))
				match = 1;
			else {
				match = 0;
				break;
			}
		}

	/* Tag cache hit */

		if(match == 1) {
			dbase_hdr.stats.curr.tag_cache_hits++;
			dbase_hdr.stats.perm.tag_cache_hits++;
			return(tcache[slot].bucket[i].daddr);
		}

	   }
	}

	/* The specified Tag was not in the cache */

	dbase_hdr.stats.curr.tag_cache_misses++;
	dbase_hdr.stats.perm.tag_cache_misses++;
	return((daddr_t) 0);
}

/*
	tcache_invalidate()-invalidate the tag cache entry associated
	with the specified tag value.
*/

tcache_invalidate(tag_addr)
tag_t *tag_addr;
{
	int i, j, slot, match;

	slot = spdbm_tag_hash(tag_addr);

	for(i=0; i < TCACHE_BUCKETS; i++) {

	   match = 0;

	   if(tcache[slot].bucket[i].flags.valid == 1) {
		for(j=0; j < SEC_TAG_SIZE; j++) {
			if(tcache[slot].bucket[i].tag[j] == *(tag_addr + j))
				match = 1;
			else {
				match = 0;
				break;
			}
		}

		if(match == 1) {
			tcache[slot].bucket[i].flags.valid = 0;
			return(0);
		}
	   }
	}
	return(0);
}

/*
	tcache_clear()-clear the tag cache of all entries-make invalid
*/

tcache_clear()
{
	int i, j;

	for(i=0; i < dbm_tag_cache; i++) {

		tcache[i].lru_bucket = 0;
		for(j=0; j < TCACHE_BUCKETS; j++) {
			tcache[i].bucket[j].flags.valid = 0;
			tcache[i].bucket[j].flags.rfu = 0;
			tcache[i].bucket[j].tag[0] = 0;
			tcache[i].bucket[j].tag[1] = 0;
		}
	}

	return(0);
}

/*
	tcache_dump()-dump the state of the tag cache
*/

tcache_dump()
{
	int i, j;

	printf(MSGSTR(SPDBM_40, "\n\t*** Tag Cache Dump ***\n\n"));

	for(i=0; i < dbm_tag_cache; i++) {
	   for(j=0; j < TCACHE_BUCKETS; j++) {
	     if(tcache[i].bucket[j].flags.valid == 1) {
		printf(MSGSTR(SPDBM_41, "Slot: %d Bucket: %d Daddr: %x Tag1: %x Tag2: %x\n"),
		   i,j,tcache[i].bucket[j].daddr,tcache[i].bucket[j].tag[0],
		   tcache[i].bucket[j].tag[1]);
	     }
	   }
	}
}

/*
	PAGE Cache Routines

	These routines are designed to support an I/O page cache that
	keeps track of disk blocks that have been read from the database
	and the memory address of the buffer that contains the block.
	The cache is organized as an N bucket cache where each slot in
	the cache has N distinct buckets each of which can cache a disk
	block. The buckets for a slot are used in a round robin fashion.
*/

/*
	pcache_insert()-make a new page cache entry using the disk block
	address to hash to a bucket which will point to the memory addr
	of the buffer.
*/

pcache_insert(caddr,daddr)
caddr_t caddr;
daddr_t daddr;
{
	int i, slot, bucket;

#ifdef DEBUG
	if(dbm_debug)
	printf(MSGSTR(SPDBM_42, "pcache_insert addr: %x daddr: %x\n"),caddr,daddr);
#endif
	slot = spdbm_page_hash(daddr);

	/* Avoid replicating valid entries into multiple slot buckets */

	for(i=0; i < PCACHE_BUCKETS; i++) {
		if((pcache[slot].bucket[i].daddr == daddr) &&
		   (pcache[slot].bucket[i].flags.valid == 1))
			return(0);
	}

	/* Determine the bucket number in the slot and increment */

	bucket = pcache[slot].lru_bucket;

	pcache[slot].lru_bucket++;
	pcache[slot].lru_bucket = pcache[slot].lru_bucket % PCACHE_BUCKETS;

	/* Insert the entry into the cache bucket and make it valid */

	pcache[slot].bucket[bucket].flags.valid = 1;
	pcache[slot].bucket[bucket].caddr = caddr;
	pcache[slot].bucket[bucket].daddr = daddr;
	return(0);
}

/*
	pcache_lookup()-lookup for match on the disk block address in the
	page cache. If a match, then return the memory buffer address.
*/

caddr_t
pcache_lookup(daddr)
daddr_t daddr;
{
	int slot, i;

	slot = spdbm_page_hash(daddr);
	
	/* Search the buckets in the determined slot for a valid cache
	   entry with a matching page disk block address. If found,
	   lock the page in memory, update the LRU time, and return
	   the buffer address.					   */

	for(i=0; i < PCACHE_BUCKETS; i++) {
		if(pcache[slot].bucket[i].flags.valid == 1) {
			if(pcache[slot].bucket[i].daddr == daddr) {
				dbase_hdr.stats.curr.page_cache_hits++;
				dbase_hdr.stats.perm.page_cache_hits++;
				page_lock(pcache[slot].bucket[i].caddr);
				pcache_update_lru(pcache[slot].bucket[i].caddr,
					daddr);
				return(pcache[slot].bucket[i].caddr);
			}
		}
	}

	dbase_hdr.stats.curr.page_cache_misses++;
	dbase_hdr.stats.perm.page_cache_misses++;
	return((caddr_t) 0);
}

/*
	pcache_invalidate()-invalidate the cache entry that corresponds
	to the disk block address passed as an argument.
*/

pcache_invalidate(daddr)
daddr_t daddr;
{
	int slot, i;

	slot = spdbm_page_hash(daddr);
	for(i=0; i < PCACHE_BUCKETS; i++) {
		if(pcache[slot].bucket[i].flags.valid == 1) {
			if(pcache[slot].bucket[i].daddr == daddr) {
				pcache[slot].bucket[i].flags.valid = 0;
				break;
			}
		}
	}
	return(0);
}

/*
	pcache_clear()-clear all entries in the cache
*/

pcache_clear()
{
	int i, j;

	for(i=0; i < dbm_page_cache; i++) {

		pcache[i].lru_bucket = 0;
		for(j=0; j < PCACHE_BUCKETS; j++) {
			pcache[i].bucket[j].flags.valid = 0;
			pcache[i].bucket[j].flags.rfu = 0;
			pcache[i].bucket[j].caddr = 0;
			pcache[i].bucket[j].daddr = 0;
		}
	}

	return(0);
}

/*
	pcache_update_lru()-Cycle through the buffer allocation table to
	locate the entry that a pcache_lookup() just succeeded on and
	update the LRU time on that buffer. In this manner, it is possible
	to delay it from being stolen since it is proably a high use
	buffer.
*/

pcache_update_lru(addr,daddr)
caddr_t addr;
daddr_t daddr;
{
	int i;

	/* Locate the buffer address in the single extent list and
	   update the time field in the allocation entry.	*/

	for(i=0; i < dbm_buffers; i++) {
		if((dbm_single[i].addr == addr) &&
		   (dbm_single[i].daddr == daddr)) {
			dbm_single[i].time = time((long *) 0);
			return(0);
		}
	}

	/* Buffer must be a multi-extent so search this list also */

	for(i=0; i < DBM_MULTI_EXTENTS; i++) {
		if((dbm_multi[i].addr == addr) &&
		   (dbm_multi[i].daddr == daddr)) {
			dbm_multi[i].time = time((long *) 0);
			return(0);
		}
	}
}

/*
	pcache_dump()-dump the state of the page cache
*/

pcache_dump()
{
	int i, j;

	printf(MSGSTR(SPDBM_43, "\n\t*** Page Cache Dump ***\n\n"));

	for(i=0; i < dbm_page_cache; i++) {
	   for(j=0; j < PCACHE_BUCKETS; j++) {
		   if(pcache[i].bucket[j].flags.valid == 1) {
			printf(MSGSTR(SPDBM_44, "Slot: %d Bucket: %d Daddr: %x Addr: %x\n"),
			   i,j,pcache[i].bucket[j].daddr,pcache[i].bucket[j].caddr);
		   }
	   }
	}
}

/*
	Page I/O Routines

	These routines perform the low level I/O for reading and writing
	database pages as well as functions like page clearing, allocation,
	and deallocation. Allocation and deallocation involves maintaining
	the local memory buffer map as well as the disk free block map in
	the database header for both single and multi extent pages.
*/

/*
	page_allocate()-allocate a new disk block page and a new
	memory buffer to map the disk block. The pages are allocated
	and associated and the memory page is locked until it is
	released with the page_release() function. The entry is
	also cached into the page cache. The routine returns the
	address of the memory buffer but also returns the disk
	block address using the daddr pointer passed to the routine.
	The disk block is required by routines building index
	pages during traversal.
*/

caddr_t
page_allocate(dblock,type)
daddr_t *dblock;
int type;
{
	struct datapage_header *dphdr;
	daddr_t daddr;
	caddr_t addr;

	/* Allocate a disk block and a buffer to map it */

	daddr = page_block_alloc(1);
	addr = page_buffer_alloc(daddr,1,type);
	*dblock = daddr;
	pcache_insert(addr,daddr);

	/* Build a page header unless the page is for indices */

	if(type == DBM_INDEX_PAGE)
		return(addr);
	else {
		dphdr = (struct datapage_header *) addr;

	/* Set the pagesize in the header according to page type */

		if(type == DBM_TAG_PAGE)
			dphdr->pgsize.pgsize = THDR_SIZE;
		else
			dphdr->pgsize.pgsize = IRHDR_SIZE;

		dphdr->pgsize.pgtype = type;
		dphdr->pgsize.extent = 1;
		dphdr->pgsize.mask_set = 0;
		dphdr->pgsize.rfu = 0;
	}
	return(addr);
}

/*
	page_extend()-extend the current page to make room for more
	records. A larger page is allocated and the contents of the
	old page are copied to it. The old page and disk block are
	then discarded. The new buffer address of the larger page
	is returned.
*/

caddr_t
page_extend(addr,type,attr,length)
caddr_t addr, attr;
int type, length;
{
	struct datapage_header *dphdr;
	struct tag_header *thdr;
	struct dbase_rechdr *recp;
	daddr_t daddr;
	caddr_t newaddr;
	int pagesize, extent_need = 0;
	int space_occupied;

	dphdr = (struct datapage_header *) addr;
	
	/* Invalidate all tag cache entries on the page since the page
	   disk block will change causing stale cache entries.      */

	if(dphdr->pgsize.pgtype == DBM_TAG_PAGE) {
		thdr = (struct tag_header *) dphdr;
		recp = (struct dbase_rechdr *) ((int)thdr + THDR_SIZE);
		pagesize = thdr->pgsize.pgsize - THDR_SIZE;

		while(pagesize > 0) {

			tcache_invalidate(recp->tag);

			space_occupied = roundup(recp->recsize,4);
			pagesize -= space_occupied;
			recp = (struct dbase_rechdr *) ((int)recp + 
				space_occupied);
		}
	}

	/* Compute needed extent count and lock current page in memory */

	extent_need = dphdr->pgsize.extent + 1;
	page_lock(addr);

	/* Get new buffer and disk block allocations */

	daddr = page_block_alloc(extent_need);
	newaddr = page_buffer_alloc(daddr,extent_need,type);

	/* Update the level 2 index page pointer to reflect the newly
	   allocated page.					   */

	spdbm_level2_update(type,attr,length,daddr);

	/* Copy the old page(s) to the new buffer allocation, flush
	   the new page copy to the database, and insert an entry
	   into the page cache for the new mapping.		 */

	spdbm_reccopy(addr,newaddr,(extent_need - 1) * dbase_hdr.pagesize);

	pcache_insert(newaddr,daddr);

	/* Update the datapage header in the new page to reflect extension */

	((struct datapage_header *) newaddr)->pgsize.extent = 
		dphdr->pgsize.extent + 1;

	/* Obtain the current disk block address for the page and use
	   it to deallocate the disk block and free the current buffer
	   mapping the disk block.				     */

	daddr = page_find_daddr(addr);
	page_block_free(daddr,extent_need - 1);
	page_buffer_free(addr,type);

	if((extent_need - 1) == 1)
		pcache_invalidate(daddr);

	return(newaddr);
}

/*
	page_read()-read the page at the specified disk offset.
	The routine will first allocate a single extent page and
	read the block into it. If the type is a Data Page, it is
	checked to determine if there are additional extents are
	to read in. If so, a larger buffer is allocated and the
	remainder of the page is read. The address of the buffer
	containing the disk block is returned.
*/

caddr_t
page_read(daddr,type)
daddr_t daddr;
int type;
{
	struct datapage_header *dphdr;
	caddr_t addr;
	int extent_count;

	dbase_hdr.stats.perm.page_io++;
	dbase_hdr.stats.curr.page_io++;

	addr = page_buffer_alloc(daddr,1,type);
#ifdef DEBUG
	if(dbm_debug)
	printf(MSGSTR(SPDBM_45, "READ: on page: %x into addr: %x\n"),daddr,addr);
#endif

	/* Lseek to the disk block and read the page from the database */

	lseek(dbm_fd,(long) daddr, 0);
	if(read(dbm_fd,addr,dbase_hdr.pagesize) == -1) {
		perror(MSGSTR(SPDBM_46, "spdbm: error on database read"));
		return((caddr_t) 0);
	}
	
	/* If an index page (single extent) cache it and return */

	if(type == DBM_INDEX_PAGE) {
		pcache_insert(addr,daddr);
		return(addr);
	}

	/* Check the datapage header for the extent count */

	dphdr = (struct datapage_header *) addr;

	if(dphdr->pgsize.extent == 1) {
		pcache_insert(addr,daddr);
		return(addr);
	}
	else {		/* Multi-extent, must re-read */

	/* Set the extent count to 1 for single page free but save the
	   extent count for the actual reallocation of the new page. */

		extent_count = dphdr->pgsize.extent;
		dphdr->pgsize.extent = 1;

		page_buffer_free(addr,type);
		addr = page_buffer_alloc(daddr,extent_count,type);

	/* Re-read the disk block into the new buffer and return it */

#ifdef DEBUG
		if(dbm_debug)
		printf(MSGSTR(SPDBM_47, "MULTI READ: on page: %x into addr: %x for %d\n"),
		daddr,addr,extent_count);
#endif
		lseek(dbm_fd,(long) daddr, 0);
		if(read(dbm_fd,addr,extent_count * dbase_hdr.pagesize) == -1) {
			perror(MSGSTR(SPDBM_46, "spdbm: error on database read"));
			return((caddr_t) 0);
		}

		pcache_insert(addr,daddr);
		return(addr);
	}
}

/*
	page_write()-the buffer specified by addr is written to the
	database. The disk address is located from the buffer control
	structures since the page is locked into memory. As a
	byproduct of the write, the page is also unlocked.
*/

page_write(addr,type)
caddr_t addr;
int type;
{
	daddr_t daddr = (daddr_t) 0;
	int i, length;

	dbase_hdr.stats.perm.page_io++;
	dbase_hdr.stats.curr.page_io++;

	/* Locate the buffer from the single extent list first */

	for(i=0; i < dbm_buffers; i++) {
		if(dbm_single[i].addr == addr) {
			daddr = dbm_single[i].daddr;
			length = dbase_hdr.pagesize;
			dbm_single[i].flags.locked = 0;
			goto write_page;
		}
	}

	/* Try the multi-extent list if not found previously */

	for(i=0; i < DBM_MAX_MLEVEL; i++) {
		if(dbm_multi[i].addr == addr) {
			daddr = dbm_multi[i].daddr;
			length = dbm_multi[i].flags.extent * dbase_hdr.pagesize;
			dbm_multi[i].flags.locked = 0;
			break;
		}
	}

write_page:

	if(daddr == (daddr_t) 0)
		return(-1);

	/* Write the page to the database */

#ifdef DEBUG
	if(dbm_debug)
	printf(MSGSTR(SPDBM_48, "WRITE: page: %x from addr: %x for %d\n"),daddr,addr,length);
#endif
	lseek(dbm_fd,(long) daddr, 0);
	if(write(dbm_fd,addr,length) == -1) {
		perror(MSGSTR(SPDBM_49, "spdbm: I/O error on page write"));
		return(-1);
	}
	else return(0);
}

/*
	page_lock()-lock the specified page in memory since the
	page must be retained while other memory allocation
	operations will follow. This prevents the page from
	being stolen before it can be used and released.

	Two cases are notable:
		1. This prevents the stealing of an index page
		   by the retrieval of a datapage when the index
		   page must be retained for updating.
		2. A page must also be locked when an extend is
		   being performed since the old page must not be
		   stolen before the new page is allocated and the
		   contents copied.
*/

page_lock(addr)
caddr_t addr;
{
	int i;

	/* Locate the buffer from the single extent list first */

#ifdef DEBUG
	if(dbm_debug)
	printf(MSGSTR(SPDBM_50, "page_lock: addr %x\n"),addr);
#endif
	for(i=0; i < dbm_buffers; i++) {
		if(dbm_single[i].addr == addr) {
			dbm_single[i].flags.locked = 1;
			return(0);
		}
	}

	/* Try the multi-extent list if not found previously */

	for(i=0; i < DBM_MAX_MLEVEL; i++) {
		if(dbm_multi[i].addr == addr) {
			dbm_multi[i].flags.locked = 1;
			return(0);
		}
	}

	printf(MSGSTR(SPDBM_51, "spdbm: cannot locate page to lock\n"));
	return(-1);
}

/*
	page_release()-release a lock on the specified page which
	was previously read or allocated but not written.
*/

page_release(addr)
caddr_t addr;
{
	int i;

	/* Locate the buffer from the single extent list first */

#ifdef DEBUG
	if(dbm_debug)
	printf(MSGSTR(SPDBM_52, "page_release: addr %x\n"),addr);
#endif
	for(i=0; i < dbm_buffers; i++) {
		if(dbm_single[i].addr == addr) {
			dbm_single[i].flags.locked = 0;
			return(0);
		}
	}

	/* Try the multi-extent list if not found previously */

	for(i=0; i < DBM_MAX_MLEVEL; i++) {
		if(dbm_multi[i].addr == addr) {
			dbm_multi[i].flags.locked = 0;
			return(0);
		}
	}

	return(-1);
}

/*
	page_buffer_alloc()-allocate a block of memory for page
	operations according to the specified number of extents
	required. If only a single extent is required, the allocation
	queue is consulted. If no blocks are available, the least
	recently used page is stolen from the cache. If the request
	is for a multi-extent block, it is malloc'd and recorded in
	the multi-extent queue. It will be freed upon release.

	Always resume the single extent buffer search from the place
	where the previous allocation was made to reduce page cache
	buffer thrashing by allocating the most recently released
	pages on new alloc requests (these are most likely to still
	be valid in the cache).
*/

caddr_t
page_buffer_alloc(daddr,extent,type)
daddr_t daddr;
int extent, type;
{
	caddr_t addr, maddr;
	long ctime = 0;
	int i, count, ctime_index, pgsize;

	dbase_hdr.stats.pages_allocated++;
	dbase_hdr.stats.pages_allocated++;

	if(extent == 1) {
		ctime = time((long *) 0);
		ctime_index = -1;
		for(i=dbm_sbuf_index, count=0; count < dbm_buffers; 
			i = (i + 1) % dbm_buffers, count++) {

	/* Allocate the free block and set LRU time value */

			if(dbm_single[i].flags.allocated == 0) {

			/* Invalidate cache entry for disk block */

				if(dbm_single[i].daddr != 0)
					pcache_invalidate(dbm_single[i].daddr);

				addr = dbm_single[i].addr;
				dbm_sbuf_index = i;
				dbm_single[i].flags.allocated = 1;
				dbm_single[i].flags.locked = 1;
				dbm_single[i].flags.pgtype = type;
				dbm_single[i].daddr = daddr;
				dbm_single[i].time = time((long *) 0);
				page_clear(addr,1,1);
				return(addr);
			}
			else {

	/* Record the LRU page to be stolen if none free */

				if(((ctime_index == -1) &&
				   (dbm_single[i].time <= ctime) &&
				   (dbm_single[i].flags.locked == 0)) ||
				   ((dbm_single[i].time < ctime) &&
				   (dbm_single[i].flags.locked == 0))) {
					ctime = dbm_single[i].time;
					ctime_index = i;
				}
			}
		}

	/* No pages free-steal one in use if possible */

		if(ctime_index == -1) {
			printf(MSGSTR(SPDBM_53, "spdbm: all pages locked on allocate\n"));
			return((caddr_t) 0);
		}

	/* Steal the LRU buffer from the Page Cache */

		pcache_invalidate(dbm_single[ctime_index].daddr);

	/* Update the buffer control block entry */

		addr = dbm_single[ctime_index].addr;
		dbm_single[ctime_index].flags.allocated = 1;
		dbm_single[ctime_index].flags.locked = 1;
		dbm_single[ctime_index].flags.pgtype = type;
		dbm_single[ctime_index].daddr = daddr;
		dbm_single[ctime_index].time = time((long *) 0);
		dbm_single[ctime_index].flags.extent = extent;
		page_clear(addr,1,1);
		return(addr);
	}
	else {		/* Multi-extent request */

		dbase_hdr.stats.page_overflows++;
		dbase_hdr.stats.page_overflows++;

		ctime = time((long *) 0);
		ctime_index = -1;

		for(i=0; i < DBM_MULTI_EXTENTS; i++) {
			if(dbm_multi[i].flags.allocated == 0) {

	/* If not allocated, the slot may be reused for new allocation */

				pgsize = (extent * dbase_hdr.pagesize) +
					(dbase_hdr.pagesize - 1);
				maddr = (char *) malloc(pgsize);
#ifdef DEBUG_MALLOC
				mpool[mcount].type = ALLOC;
				mpool[mcount].addr = maddr;
				mpool[mcount].link = *(maddr - 4);
				mcount_incr();
#endif
				if(maddr == (char *) NULL) {
					printf(MSGSTR(SPDBM_54, "spdbm: no memory on malloc\n"));
					return((char *) NULL);
				}

				addr = (char *) (((int) maddr + 
				   dbase_hdr.pagesize - 1)
				   & ~(dbase_hdr.pagesize - 1));

	/* Invalidate the page cache entry if a valid disk block address */

				if(dbm_multi[i].daddr != 0)
					pcache_invalidate(dbm_multi[i].daddr);

	/* Update the control block for the buffer entry */

				dbm_multi[i].addr = addr;
				dbm_multi[i].maddr = maddr;
				dbm_multi[i].daddr = daddr;
				dbm_multi[i].flags.allocated = 1;
				dbm_multi[i].flags.locked = 1;
				dbm_multi[i].flags.pgtype = type;
				dbm_multi[i].flags.extent = extent;
				page_clear(addr,extent,1);
				return(addr);
			}
			else {

	/* Record the LRU page to be stolen if none free */

				if(((ctime_index == -1) &&
				   (dbm_multi[i].time <= ctime) &&
				   (dbm_multi[i].flags.locked == 0)) ||
				   ((dbm_multi[i].time < ctime) &&
				   (dbm_multi[i].flags.locked == 0))) {
					ctime = dbm_multi[i].time;
					ctime_index = i;
				}
			}
		}

	/* Steal a multi-extent allocation slot for new page-release
	   the current memory allocation for the extent before malloc. */

		if(ctime_index == -1) {
			printf(MSGSTR(SPDBM_55, "spdbm: all multi-extent pages locked on alloc\n"));
			return((caddr_t) 0);
		}

	/* Steal the LRU buffer from the Page Cache and free memory page */

#ifdef DEBUG_MALLOC
		mpool[mcount].type = FREE;
		mpool[mcount].addr = dbm_multi[ctime_index].maddr;
		mpool[mcount].link = *(dbm_multi[ctime_index].maddr - 4);
		mcount_incr();
#endif
		free(dbm_multi[ctime_index].maddr);
		pcache_invalidate(dbm_multi[ctime_index].daddr);

	/* Compute new page size and malloc(), round up to mod pagesize */

		pgsize = (extent * dbase_hdr.pagesize) +
			(dbase_hdr.pagesize - 1);
		maddr = (char *) malloc(pgsize);
#ifdef DEBUG_MALLOC
		mpool[mcount].type = ALLOC;
		mpool[mcount].addr = maddr;
		mpool[mcount].link = *(maddr - 4);
		mcount_incr();
#endif
		if(maddr == (char *) NULL) {
			printf(MSGSTR(SPDBM_54, "spdbm: no memory on malloc\n"));
			return((char *) NULL);
		}

		addr = (char *) (((int) maddr + dbase_hdr.pagesize - 1)
			   & ~(dbase_hdr.pagesize - 1));

	/* Update the control block for the buffer entry */

		dbm_multi[ctime_index].addr = addr;
		dbm_multi[ctime_index].maddr = maddr;
		dbm_multi[ctime_index].daddr = daddr;
		dbm_multi[ctime_index].flags.allocated = 1;
		dbm_multi[ctime_index].flags.locked = 1;
		dbm_multi[ctime_index].flags.pgtype = type;
		dbm_multi[ctime_index].flags.extent = extent;
		dbm_multi[ctime_index].time = time((long *) 0);

	/* Clear the new allocation */

		page_clear(addr,extent,1);

		return(addr);

	}
}

/*
	page_buffer_free()-free the specified buffer back to the single
	extent buffer list or use free() to return multi-extent malloc
	space. Mark the control block structure entry for the allocation
	as freed.
*/

page_buffer_free(addr,type)
char *addr;
int type;
{
	struct datapage_header *dphdr;
	int i, single = 0;

	dbase_hdr.stats.pages_deallocated++;
	dbase_hdr.stats.pages_deallocated++;

	if((type == DBM_TAG_PAGE) || (type == DBM_IR_PAGE)) {
		dphdr = (struct datapage_header *) addr;
		if(dphdr->pgsize.extent == 1)
			single = 1;
	}

	/* Determine if a single extent buffer or multi-extent */

	if((single == 1) || (type == DBM_INDEX_PAGE)) {
		for(i=0; i < dbm_buffers; i++) {
			if(dbm_single[i].addr == addr) {
				dbm_single[i].flags.allocated = 0;
				dbm_single[i].flags.locked = 0;
				return(0);
			}
		}
	}
	else {
		for(i=0; i < DBM_MULTI_EXTENTS; i++) {
			if(dbm_multi[i].addr == addr) {
				dbm_multi[i].flags.allocated = 0;
				dbm_multi[i].flags.locked = 0;
				free(dbm_multi[i].maddr);
				return(0);
			}
		}
	}

	printf(MSGSTR(SPDBM_56, "spdbm: unable to locate control block entry on buffer free\n"));
	return(-1);
}

/*
	page_block_alloc()-allocate a single or multi-extent disk
	block address that the a page will map to in the database.
	Use the single and multi-extent free page lists to satisfy
	the requests if possible. Otherwise, the high offset of
	the file is extended to alocate the required block.
*/

daddr_t
page_block_alloc(extent)
int extent;
{
	daddr_t daddr;
	int i, count = 0;

	/* Single extent-use the free list else extend database */

	if(extent == 1) {
		if(dbase_hdr.slevel_count > 0) {
			daddr = slevel_free[--dbase_hdr.slevel_count];
			slevel_free[dbase_hdr.slevel_count] = 0;
			if(spdbm_flush(DBM_FLUSH_SFREE) == -1) {
				perror(MSGSTR(SPDBM_57, "spdbm: error on sfree flush"));
				return(-1);
			}
			return(daddr);
		}

	/* Extend the high offset of the file for the new page */

		daddr = dbase_hdr.high_offset;
		dbase_hdr.high_offset += dbase_hdr.pagesize;
		if(spdbm_flush(DBM_FLUSH_DBHDR) == -1) {
			perror(MSGSTR(SPDBM_58, "spdbm: error on dbase hdr flush for sfree"));
			return(-1);
		}
	return(daddr);
}

/* Multi-extent allocation-try free list for extent match */

else {
   for(i=0; i < DBM_MAX_MLEVEL && count < dbase_hdr.mlevel_count; i++) {
	if(mlevel_free[i].flags.valid == 1) {
		if(mlevel_free[i].flags.extent == extent) {
			daddr = mlevel_free[i].daddr;
			mlevel_free[i].daddr = 0;
			mlevel_free[i].flags.extent = 0;
			mlevel_free[i].flags.valid = 0;
			dbase_hdr.mlevel_count--;
			if(spdbm_flush(DBM_FLUSH_MFREE) == -1) {
				perror(MSGSTR(SPDBM_59, "spdbm: error on mfree flush"));
				return(-1);
			}
			return(daddr);
		}
		count++;
	}
   }

/* Extend the database file for a multi-extent page */

   daddr = dbase_hdr.high_offset;
   dbase_hdr.high_offset += (extent * dbase_hdr.pagesize);
   if(spdbm_flush(DBM_FLUSH_DBHDR) == -1) {
	perror(MSGSTR(SPDBM_60, "spdbm: error on dbase hdr flush for mfree"));
	return(-1);
   }
   return(daddr);

}
}

/*
	page_block_free()-free the specified disk block back to the
	database. If it is a single extent block then put it on the
	single extent free list. If multi-extent, put it on the
	multi-extent list with the extent count. If either list is
	full, an overflow occurs.
*/

page_block_free(daddr,extent)
daddr_t daddr;
int extent;
{
	struct datapage_header *dphdr;
	int i;

	if(extent == 1) {
		if(dbase_hdr.slevel_count >= DBM_MAX_SLEVEL)
			printf(MSGSTR(SPDBM_61, "spdbm: slevel free list overflow\n"));
		else 
			slevel_free[dbase_hdr.slevel_count++] = daddr;
	}
	else {
		if(dbase_hdr.mlevel_count >= DBM_MAX_MLEVEL)
			printf(MSGSTR(SPDBM_62, "spdbm: mlevel free list overflow\n"));
		else {
			for(i=0; i < DBM_MAX_MLEVEL; i++) {
				if(mlevel_free[i].flags.valid == 0)
					break;
			}

			if(i >= DBM_MAX_MLEVEL)
				printf(MSGSTR(SPDBM_62, "spdbm: mlevel free list overflow\n"));
			else {
				mlevel_free[i].daddr = daddr;
				mlevel_free[i].flags.extent = extent;
				mlevel_free[i].flags.valid = 1;
				dbase_hdr.mlevel_count++;
			}
		}
	}

	pcache_invalidate(daddr);

	/* Flush either the single or multi extent list */

	if(extent == 1) {
		if(spdbm_flush(DBM_FLUSH_SFREE) == -1) {
			perror(MSGSTR(SPDBM_63, "spdbm: error on sfree flush in block free"));
			return(-1);
		}
	}
	else {
		if(spdbm_flush(DBM_FLUSH_MFREE) == -1) {
			perror(MSGSTR(SPDBM_64, "spdbm: error on mfree flush in block free"));
			return(-1);
		}
	}

	return(0);
}

/*
	page_find_daddr()-locate the disk block address given the
	memory buffer address of a single extent page locked into
	memory.
*/

daddr_t
page_find_daddr(addr)
char *addr;
{
	int i;

	/* Since the buffer is locked, find the matching entry */

	for(i=0; i < dbm_buffers; i++) {
		if(dbm_single[i].addr == addr)
			return(dbm_single[i].daddr);
	}

	for(i=0; i < DBM_MULTI_EXTENTS; i++) {
		if(dbm_multi[i].addr == addr)
			return(dbm_multi[i].daddr);
	}

	printf(MSGSTR(SPDBM_65, "spdbm: unable to map buffer into disk block address\n"));
	return((daddr_t) 0);
}

/*
	page_clear()-clear the page at the specified address for the
	number of bytes indicated. If the extents field is non-zero,
	a longword zero is performed the number of extents else a byte
	zero operation is performed for general purpose bzero() usage.
*/

page_clear(lsrc,size,extent)
long *lsrc;
int size, extent;
{
	char *bsrc;
	int length;

	if(extent != 0) {
		length = (size * dbase_hdr.pagesize) / sizeof(long);

		while(length-- > 0)
			*lsrc++ = 0L;
	}
	else {
		bsrc = (char *) lsrc;

		while(size-- > 0)
			*bsrc++ = 0;
	}
}

/* Hash functions designed to work with Security Policy Attribute Labels */

spd_level1_hash(attr,length)
char *attr;
int length;
{
	unsigned long *hash_attr = (unsigned long *) attr;
	unsigned long hash = 0;
	int i, long_count = length / sizeof(long);

	for(i=0; i < long_count; i++) {
		hash = hash + *(hash_attr + i);
		if(*(hash_attr + i) != 0)
			hash++;
	}

	return(hash % DBM_LEVEL1_INDICES);
}

spd_level2_hash(attr,length)
char *attr;
int length;
{
	unsigned long *hash_attr = (unsigned long *) attr;
	unsigned long hash = 0;
	int i, long_count = length / sizeof(long);

	for(i=0; i < long_count; i++) {
		hash = hash + *(hash_attr + i);
		if(*(hash_attr + i) != 0)
			hash++;
	}

	return(hash % (dbase_hdr.pagesize / sizeof(daddr_t)));
}
