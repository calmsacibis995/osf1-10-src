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
/****************************************************************************
**                                                                          *
**            Copyright (c) Digital Equipment Corporation, 1990.            *
**            All Rights Reserved.  Unpublished rights reserved             *
**            under the copyright laws of the United States.                *
**                                                                          *
**            The software contained on this media is proprietary           *
**            to and embodies the confidential technology of                *
**            Digital Equipment Corporation.  Possession, use,              *
**            duplication or dissemination of the software and              *
**            media is authorized only pursuant to a valid written          *
**            license from Digital Equipment Corporation.                   *
**                                                                          *
**            RESTRICTED RIGHTS LEGEND   Use, duplication, or               *
**            disclosure by the U.S. Government is subject to               *
**            restrictions as set forth in Subparagraph (c)(1)(ii)          *
**            of DFARS 252.227-7013, or in FAR 52.227-19, as                *
**            applicable.                                                   *
**                                                                          *
****************************************************************************/

/*
 * This is a combination of a fixed sized preallocated bucket
 * allocator and a variation on the "buddy system" allocator theme.
 * See Knuth's "Fundamental Algorithms", pp. 442.
 *
 * salient features of the fixed sized preallocated bucket system:
 *	- very fast
 *	- size of buckets are variable (tuned to minimize space
 *	  wastage. The overhead is one word
 *	- all blocks are multiples of four to make it word aligned
 *	- There is a minimum block size of two words (one usable)
 *	- never grows in size, or changes distribution of blocks into the
 *	  buckets. Will thus, be unable to alloc memory at times, for 
 *	  we use the buddy allocator
 *
 * salient features of the buddy system:
 *	- all blocks are powers of two in size, and contain
 *	  one word of unusable (by the client) overhead.
 *	- all blocks are aligned on boundaries natural to their size.
 *	- there is a minimum block size of four words (three usable).
 *
 *	- blocks may be coalesced and split, subject to alignment constraints.
 */
#include <sys/types.h>
#include <stdio.h>
#include <signal.h>

#if 0
#include "misc.h"	/* declares Xalloc() to return (long *) */
#else
#define FALSE		0x0
#define TRUE		0x1

#define NO_MORE_MEMORY	-1

#define USED		0x0
#define NOT_USED	0x1
#define IN_Q		0x2

#define MAXACTUALQ	30

#define max(a,b)	((a)>(b)?(a):(b))
#define			LOG2_MIN_SBRK 14

typedef int		Bool;
#endif


#define BtoW( B)	((B)>>2)
#define WtoB( W)	((W)<<2)
#define BUD_MIN_BUC	4	/* log2( sizeof( BudBlkFree)) */
#define BUD_MAX_BUC	27	/*
				 * implies max block size of 
				 * 2^(BUD_MAX_BUC-1) bytes 
				 */
#define MIN_SIZEFIXBUC	8	/* minimum size of a fixed bucket */
#define MAX_FIXBUC	17	/* number of fixed buckets */
#define SIG		0xf00d
#define	CACHE_LINE_SIZE	16

/*
 * This macro adds a slot to a list of usable slots
 */
#define ADDSLOTTOFREESLOTQ( a )						\
				{				  	\
					if( (a)->prevSlotPtr )	  	\
					{			  	\
					  (a)->prevSlotPtr->nextSlotPtr =\
						(a)->nextSlotPtr; 	\
					}			  	\
					if( (a)->nextSlotPtr )	  	\
					{			  	\
					  (a)->nextSlotPtr->prevSlotPtr =\
						(a)->prevSlotPtr; 	\
					}			  	\
					if( (a) == lazyFreeQ )	  	\
					{			  	\
					  lazyFreeQ = (a)->nextSlotPtr;	\
					}			  	\
					(a)->nextSlotPtr = freeSlotQ; 	\
					freeSlotQ = (a);		\
				}

/*
 * This macro removes a slot from the list of available slot
 */
#define REMOVESLOTFROMFREESLOTQ( a )					\
				{				  	\
					register BudLZQSlot *tq;  	\
								  	\
					(a)->slotlist = tq = freeSlotQ;	\
					tq->freePtr = (a);	  	\
					freeSlotQ = tq->nextSlotPtr;  	\
					tq->nextSlotPtr = lazyFreeQ;	\
					if( lazyFreeQ )		  	\
					{			  	\
					  lazyFreeQ->prevSlotPtr = tq;	\
					}			  	\
					tq->prevSlotPtr = NULL;   	\
					lazyFreeQ = tq;		  	\
				}
#   define CHECK_FATAL_ADDR_OVERFLOW( pb )	{			\
		    if( (char *)pb > heapStop )				\
		    {							\
			FatalError( 					\
			"BADREALLOC: Address overflow\n");		\
			return;						\
		    }							\
		}
#   define CHECK_FATAL_ADDR_UNDERFLOW( pb )	{			\
		    if( (char *)pb < fixHeapStart )			\
		    {							\
			FatalError( 					\
			"BADREALLOC: Address underflow\n");		\
		        return;						\
		    }							\
		}
#   define CHECK_ADDR_OVERFLOW( pb, MSG )	{			\
		    if( (char *)pb > heapStop )				\
		    {							\
			fprintf( stderr, 				\
			"MSG: Address overflow\n");			\
			return;						\
		    }							\
		}
#   define CHECK_ADDR_UNDERFLOW( pb, MSG )	{			\
		    if( (char *)pb < fixHeapStart )			\
		    {							\
		        fprintf( stderr, 				\
			"MSG: Address underflow\n");		\
		        return;						\
		    }							\
		}


/*
 * GATHER_STATS : enables gathering of statistics of the buddy allocator
 *		  Upon receiving a SIGUSR2 signal, all blocks are scanned
 *		  and statistics are printed
 * DM_RESOURCE_DUMP :
 *		  enables gathering statistics of the resource usage
 *		  by the server. 
 * DUMPSTAT :	  enables gathering data of approximately how many blocks
 *		  in each bucket size has been allocated and not yet freed
 *
 * NOTE: GATHER_STATS prevails over DM_RESOURCE_DUMP and DUMPSTAT
 */
#ifndef GATHER_STATS
#   ifdef DM_RESOURCE_DUMP
#       include <sys/time.h>
#       include <sys/resource.h>
#       define DMRD_INIT_RESOURCE_DUMP()	signal( SIGUSR2, resource_use )
        static void resource_use();
#   else
#       define DMRD_INIT_RESOURCE_DUMP()
#   endif
#   ifdef DUMPSTAT
#       define DS_INC_FREE( buc ) {				\
				    stats_free[buc]++;		\
				    dump_stats(1<<buc);		\
				  }
#       define DS_DEC_FREE( buc ) {				\
				    stats_free[buc]--;		\
				    dump_stats(1<<buc);		\
				  }
	static int	dump_stats();
        static int	stats_free[BUD_MAX_BUC];
#   else
#       define DS_INC_FREE( buc ) 
#       define DS_DEC_FREE( buc ) 
#   endif
#   define GS_INIT_GATHER_STATS()
#else
#   define GS_INIT_GATHER_STATS()	signal( SIGUSR2, gather_stats);
#   define DMRD_INIT_RESOURCE_DUMP()
#   define DS_INC_FREE( buc ) 
#   define DS_DEC_FREE( buc ) 
    static void	gather_stats();
    static int	stats_inuse[BUD_MAX_BUC];
    static int	stats_free[BUD_MAX_BUC];
    static int	stats_total;
#endif


/*
 * DM_SIZE_TRACE : this variable allows the tracing of memory calls
 *
 * maxSizeCall[MAX_SIZE_TRACE] is an array giving the maximum number
 * of calls that were outstanding at any given time during the life of
 * the server. This is to be used to tune (determining size and number of
 * buckets) the fixed bucket allocator
 *
 * numInSize[MAX_SIZE_TRACE] is an array of the present number of blocks 
 * still being used in the respective sizes.
 *
 * total_memory gives the total sbrk-ed memory
 *
 * free_memory and used_memory is the currently free and used memory
 *
 * num_actually_asked is the actual number of bytes requested in the course
 * of the life of the server
 *
 * num_actually_given is the actual number of bytes returned (measure of the
 * internal fragmentation
 *
 * num_q_full is the number of times that the lazy Q got full
 */
#ifdef DM_SIZE_TRACE
#   define  SBRK( n )	mysbrk( n )
#   define  DMST_ALLOC( n )					\
	{							\
		free_memory += total_memory - tmem - (n);	\
		used_memory += (n);				\
	}
#   define  DMST_FREE( n )					\
	{							\
		free_memory += (n);				\
		used_memory -= (n);				\
	}
#   define DMST_INIT_DATA()  {					\
				for( ib=0; ib<MAX_SIZE_TRACE; 	\
						ib++ )		\
				{				\
				    maxSizeCall[ib] = 0;	\
				    numInSize[ib] = 0;		\
				}				\
			     }
#   define  MEM_USED( nb )					\
	{							\
		num_actually_asked += nb;			\
		num_actually_given += used_memory - tused;	\
	}
#   define DMST_INIT_Q_FULL()	{ num_q_full = 0; }
#   define DMST_INC_Q_FULL()	{ num_q_full++; }
#   define DMST_INIT_TMEM()	{ tmem = total_memory; }
#   define MAX_SIZE_TRACE	1024
#   define MYMALLOC		myalloc
#   define MYFREE		myfree
#   define SCOPE		static
    /*
     * writing the head data
     * 3max-specific hack: shifting and OR'ing is much faster than
     * partial word writes.
     */
#   define OR_HEAD( status, buc)	(((status)<<8) | (buc))
#   define OR_FIXHEAD( buc)		(buc)
#   define DMNST_MEM_CHECK( pb, MSG )

    static char *MYMALLOC();
    static void	MYFREE();
    static char *mysbrk();
    static int	maxSizeCall[MAX_SIZE_TRACE];
    static int	numInSize[MAX_SIZE_TRACE];
    static int	total_memory = 0;
    static int	free_memory = 0;
    static int	used_memory = 0;
    static long num_actually_asked = 0;
    static long num_actually_given = 0;
    static int  tmem = 0;
    static int	num_q_full;
    static int  tused = 0;

#else
#   define SBRK( n )		sbrk( n )
#   define DMST_ALLOC( n )
#   define DMST_FREE( n )
#   define DMST_INIT_TMEM()
#   define DMST_INIT_DATA()
#   define DMST_INIT_Q_FULL()
#   define DMST_INC_Q_FULL()
#   define MYMALLOC		MALLOC
#   define MYFREE		FREE
#   define SCOPE
    /*
     * writing the head data
     * 3max-specific hack: shifting and OR'ing is much faster than
     * partial word writes.
     *
     * POLICY DECISION:
     * Memory will not always be checked (for overwites) at free time
     * Only when MEMCHECK is enabled will it be checked
     */
#   ifdef MEMCHECK
#       define OR_HEAD( status, buc)	((SIG<<16) | ((status)<<8) | (buc))
#       define OR_FIXHEAD( buc)	((SIG<<16) | (buc))
#	define DMNST_MEM_CHECK( pb, MSG )				\
		    if ( pb->head.s.signature != SIG )			\
		    {							\
			fprintf( stderr, "MSG: Data may be corrupted\n");\
			pb->head.w = OR_HEAD( USED, 0xff );		\
			return;						\
		    }
#   else
#       define OR_HEAD( status, buc)	(((status)<<8) | (buc))
#       define OR_FIXHEAD( buc)		(buc)
#	define DMNST_MEM_CHECK( pb, MSG )
        /*
#       define OR_HEAD( status, buc)	((SIG<<16) | ((status)<<8) | (buc))
#       define OR_FIXHEAD( buc)	((SIG<<16) | (buc))
         */
#   endif

#endif

/*
 * DM_DEBUG :	enables address checking to test for errors in the buddy
 * 		system allocator.
 * buc_in_q gives the number of blocks in the lazyq for each bucket
 * numInBuc gives the number of blocks given out in each bucket
 * maxCall gives the maximum number of blocks given out in each bucket
 */
#if DM_DEBUG
#   define DMDB_INIT_BUC_IN_Q()		{ buc_in_q[ib] = 0; }
#   define DMDB_INC_BUC_IN_Q( buc )	{ buc_in_q[buc]++; }
#   define DMDB_DEC_BUC_IN_Q( buc )	{ buc_in_q[buc]--; }
#   define DMDB_CHECK_ADDR_ALIGN( pb, buc )				\
		    {							\
			if( (int)(pb) & ( (1<<(buc)) - 1) )		\
			  fprintf( stderr, "error in block address\n" );\
		    }
    /*
     * Macros used to find the max. number of blocks in a bucket in use
     * at any time.
     */
#   define DMDB_UPDATENUMINBUC( buc )	{			  	     \
					numInBuc[buc]++;	  	     \
					if( numInBuc[buc] > maxCall[buc] )   \
						maxCall[buc] = numInBuc[buc];\
				}

#   define DMDB_DOWNDATENUMINBUC( buc )	{ numInBuc[buc]--; }
#   define DMDB_CHECK_ADDR()						     \
		    if( (int)heapStop + minRequired + moreRequired != newtop)\
			printf("error in new_chunk\n");
    static int	maxCall[BUD_MAX_BUC];
    static int	numInBuc[BUD_MAX_BUC];
    static int	buc_in_q[BUD_MAX_BUC];
#else
#   define DMDB_INIT_BUC_IN_Q()
#   define DMDB_INC_BUC_IN_Q( buc )
#   define DMDB_DEC_BUC_IN_Q( buc )
#   define DMDB_CHECK_ADDR_ALIGN( pb, buc )
#   define DMDB_UPDATENUMINBUC( buc )
#   define DMDB_DOWNDATENUMINBUC( buc )
#   define DMDB_CHECK_ADDR()
#endif


/*
 * The head for the buddy scheme has an additional free field to
 * allow it to known if the block is free or not. We do not need
 * two types of heads if we will not use more that 255 blocks in
 * the fixed bucket scheme.
 */
typedef union {
    struct {
	unsigned char	buc;	/* bucket # => block size = 2 exp buc*/
	unsigned char	status;	/* NOT_USED, USED or IN_Q */
	unsigned short	signature;	/* for post-mortem debugging */
    } s;
    unsigned int	w;
} BudBlkHead;

typedef union {
	struct {
        unsigned short	buc;
        unsigned short  signature;      /* for post-mortem debugging */
    } s;
    unsigned int        w;
} FixBlkHead;

/*
 * The buddy system needs to maintain a doubly linked list. It also
 * needs a doubly linked list for the lazy free queue. However since
 * the lazy free queue is of a limited size, we can avoid the 
 * inconvenience of allocating two words for every block. We use an
 * list of slots to keep a double link of the lazy free queue, and
 * we use a word from the BudBlkFree header to point to this slot
 *
 * The fixed bucket system blocks are not guaranteed to be cacheline aligned.
 */
typedef struct BudBlkFree {
    BudBlkHead		head;
    struct BudBlkFree *	blink;		/* doubly linked list */
    struct BudBlkFree *	flink;		/* doubly linked list */
    struct BudLZQSlot *	slotlist;	/* pointer to slot in */
					/* slots	      */
} BudBlkFree;

/*
 * This is the lazy queue element structure. 
 */
typedef struct BudLZQSlot {
    BudBlkFree	  *	freePtr;	/* points to the free memory	*/
    struct BudLZQSlot *	nextSlotPtr;	/* points to next slot		*/
    struct BudLZQSlot *	prevSlotPtr;	/* points to previous slot	*/
} BudLZQSlot;

/*
 * The fixed bucket allocator only requires 2 words of overhead
 * if the block is free and 1 word if it is use.
 */
typedef struct FixBlkFree{
    FixBlkHead           head;	/* header info	*/
    struct FixBlkFree *	flink;	/* next block	*/
} FixBlkFree;

typedef struct FixSizeNumber {
    unsigned short	size;	/* size of the block			*/
    int	 		number;	/* number of blocks in the bucket	*/
} FixSizeNumber;

/*
 * Buckets 0..3  are not used.
 * Bucket 4 holds blocks of the minimum size, 2^4 = 16 bytes.
 *
 * Note that:
 *	fls( 1<<n) == ffs( 1<<n) == n+1
 */
BudBlkFree 		budBuckets[BUD_MAX_BUC];
/*
 * A buddy system allocator could be a bottle neck for a repeated sequence
 * of allocs and frees of small sizes (splitting a large size block, and
 * coaleasing upon a free). So a fixed amount of memory is preassigned
 * in an alternate set of buckets. Blocks from these buckets cannot be 
 *  consolidated
 */

/*
 * Fix bucket allocator's buckets
 */
FixBlkFree	fixBuckets[MAX_FIXBUC];

static FixSizeNumber fixSizeNumber[MAX_FIXBUC] =
	{
		{    0,     0 },
		{   16, 22360 },
		{   24, 23544 },
		{   28,  3852 },
		{   52,  4760 },
		{   60,  1252 },
		{   80,   700 },
		{  132,   224 },
		{  156,   328 },
		{  184,  3660 },
		{  232,   292 },
		{  264,   128 },
		{  520,   364 },
		{ 1024,   360 },
		{ 2048,    48 },
		{ 4096,    16 },
		{ 8192,    16 }
	};



/*
 * slots is a list of slots which are used to keep track of blocks
 * freed by the client, but not yet coaleased in the buddy allocator
 * When a client gives up a block to free, the server does a lazy
 * free. The server puts the block into the appropriate bucket, and
 * places the address of this block into a Q of partially (unconsol-
 * idated) freed block. The server consolidates memory only when 
 * (a) there are no more slots to grow the Q 
 * (b) an sbrk is the only other alternative to providing the
 *     requested memory.
 *
 * Each element of slots points to the block that underwent a lazy
 * free but was not actually consolidated. The freelink field of
 * the block points back to this slots element
 * freeSlotQ is a link list of slots which are not yet used
 * lazyFreeQ is a link list of the "lazy free" queue
 */
static BudLZQSlot	slots[MAXACTUALQ], *freeSlotQ, *lazyFreeQ;
static int	maxq = MAXACTUALQ;

/*
 * The memory is partioned as follows
 *
 *   +---------------------+-----------------------+------------------+--->
 * fixHeapStart          budHeapStart           heapStop           sbrkTop
 *   <---fixed bucket------><----buddy allocator---><--buffered sbrk-->
 * 
 */
static Bool	Initialized = FALSE;
static char *   fixHeapStart; 	/* don't reference before this address */
static char *	budHeapStart;	/*
				 * don't reference before this address
				 * for the buddy system allocator
				 */
static char *	heapStop;	/* don't reference beyond this address */
static char *	sbrkTop = (char *)NULL;	


/*
 * fast lookup table to get the right bucket number for the buddy 
 * system allocator
 */
#define		LOG2_SMALL_TABLE_SIZE	10
char		small_bytes_to_buc[ 1<<LOG2_SMALL_TABLE_SIZE];

/*
 *  * fast lookup table to get the right bucket number for the fixed
 * bucket allocator
 */
#define		LOG2_SMALL_FIXTABLE_SIZE	10
char		small_bytes_to_fixbuc[ 1<<LOG2_SMALL_FIXTABLE_SIZE];

/*
 * define routines
 */
static int		get_mem();
static void		out_of_memory();
static int		fixbuchash();
static void		init_lzq();
static void		init_buckets();
static void		init_lookups();
static void		preallocate_fixed_buckets();
static void		initialize();
static void		bud_blk_free();
static BudBlkFree	*bud_blk_unfree_by_addr();
static BudBlkFree	*bud_blk_unfree();
static BudBlkFree	*split1();
static BudBlkFree	*split_to_size();
static void		free_recurs();
static void		put_q();
static void		free_q();
static int		calc_excess_required();
static int		new_chunk();
static unsigned int	newfls();
static unsigned int	newffs();
static int		bytes_to_buc();
char 			*Xalloc();
char			*Xrealloc();
void			Xfree();
extern char		*sbrk();


/*
 * error function to be used when the allocator runs out of memory 
 */
static void (*xErrF)();

/*
 * must abort if we run out of memory when this flag is set .. defined in
 * server/os/4.2/os/utils.c
 */
extern int Must_have_memory; 

/*
 * This routine allows you to change which error function will be
 * called when Xalloc runs out of memory (ie: sbrk does not return
 * enough space).  By default, the error function is set to out_of_memory,
 * which is defined below.  XSetErrorFunc returns the previously set 
 * error function.  By convention, the calling function, when done,
 * should restore the error function back to its previously defined
 * value.  
 */
void (*XSetErrorFunc( new_erfn ))()
void (*new_erfn)();
{
	void (*old_erfn)();

	old_erfn = xErrF;
	xErrF = new_erfn;
	return old_erfn;
}

/*
 * This is the default routine which is called when we run out of memory 
 * If Must_have_memory is set then we must abort because the calling routine
 * cannot handle an out of memory condition.  If Must_have_memory is not
 * set then just print an error and return. 
 */
static void out_of_memory( s)
char *s;
{
    if (Must_have_memory)
        FatalError( s );
    ErrorF( s );
}


/*
 * This routine does a binary search on the fixed bucket allocator's
 * buckets to find the correct bucket.
 */
static fixbuchash( nb )
register int nb;
{
    register int top = MAX_FIXBUC - 1;
    register int bottom = MAX_FIXBUC >> 1;
    register int temp = 0;

    while( 1 )
    {
	if( fixSizeNumber[bottom].size >= nb )
	{
	    top = bottom;
	    bottom = (temp + bottom) >> 1;
	}
	else if( (top - bottom) == 1 )
	{
	    return top;
	}
	else
	{
	    temp = bottom;
	    bottom = (top + bottom) >> 1;
	}
    }
}

/*
 * initalize the slots , the available slots freeSlotQ, and 
 * "lazy queue" lazyFreeQ
 */
static void init_lzq()
{
    register int	ib;

    for ( ib=1; ib<maxq; ib++ )
	slots[ib-1].nextSlotPtr = &slots[ib];
    slots[ib].nextSlotPtr = (BudLZQSlot *)NULL;
    freeSlotQ = &slots[0];
    lazyFreeQ = (BudLZQSlot *)NULL;
}

/*
 * initialize the lookup tables to get the bucket number
 * for small amounts of memory 
 */
static void init_lookups()
{
    register int	ib;

    for ( ib=0; ib < 1<<LOG2_SMALL_TABLE_SIZE; ib++)
	small_bytes_to_buc[ ib] = 
		max( BUD_MIN_BUC, newfls( ib-1+sizeof(BudBlkHead)/*4*/));

    for ( ib=0; ib < 1<<LOG2_SMALL_FIXTABLE_SIZE; ib++)
	small_bytes_to_fixbuc[ ib] = fixbuchash( ib+sizeof(FixBlkHead)/*4*/);
}

/*
 * initialize the buckets for both the fixed bucket and buddy
 * allocators.
 */
static void init_buckets()
{
    register int	ib;

    for ( ib=0; ib < MAX_FIXBUC; ib++)
    {
	fixBuckets[ib].flink = (FixBlkFree *) NULL;
    }
    for ( ib=BUD_MIN_BUC; ib < BUD_MAX_BUC; ib++)
    {
	budBuckets[ib].flink = budBuckets[ib].blink = &budBuckets[ib];

	DMDB_INIT_BUC_IN_Q();
    }
}

/*
 * preallocate memory for the fixed bucket allocator
 */
static void preallocate_fixed_buckets()
{
    register int	ib;
    char		*chunk;
    register int	nb;
    register int	i;

    nb = 0;
    for( ib=0; ib<MAX_FIXBUC; ib++ )
    {
	nb +=  fixSizeNumber[ib].size * fixSizeNumber[ib].number;
    }
    sbrkTop = budHeapStart = heapStop = (chunk = SBRK( nb )) + nb;
    for( ib=0; ib<MAX_FIXBUC; ib++ )
    {
	for( i=0; i<fixSizeNumber[ib].number; i++ )
	{
	    ((FixBlkFree *)chunk)->head.w = OR_FIXHEAD( ib );
	    ((FixBlkFree *)chunk)->flink = fixBuckets[ib].flink;
	    fixBuckets[ib].flink = ((FixBlkFree *)chunk);
	    chunk += fixSizeNumber[ib].size;
	}
    }
}

/*
 * initialize the memory alloctors
 */
static void
initialize()
{
    register int	addrlowbits;
    register int	nb;
/*
    extern   void	FatalError();
*/

    GS_INIT_GATHER_STATS();

    DMRD_INIT_RESOURCE_DUMP();

    DMST_INIT_Q_FULL();

    DMST_INIT_DATA();

    /*
     * initalize the slots , the available slots freeSlotQ, and 
     * "lazy queue" lazyFreeQ
     */
    init_lzq();

    /*
     * initialize the lookup tables to get the bucket number
     * for small amounts of memory 
     */
    init_lookups();

    /*
     * initialize the buckets for both the fixed bucket and buddy
     * allocators.
     */
    init_buckets();

    /*
     * align sbrk such that the first word of the of a
     * block starts a 3max cache line.
     * fixHeapStart to budHeapStart is the fixed bucket allocator's space
     * budHeapStart to heapStop is the buddy allocator's space
     */
    addrlowbits = ((int) sbrk(0)) & (CACHE_LINE_SIZE-1);
    nb = CACHE_LINE_SIZE-addrlowbits;	/* cache align sbrk */
    fixHeapStart = sbrk( nb) + nb;
    nb = 0;

    /*
     * preallocate memory for the fixed bucket allocator
     */
    preallocate_fixed_buckets();

    /*
     * Assume fatal error when we run out of memory
     */
    xErrF = out_of_memory;

    Initialized = TRUE;
}


/*
 * insert pb into the appropriate free list in the
 * buddy allocator
 */
static void
bud_blk_free( pb, status, buc)
    BudBlkFree		*pb;
    register int	status;
    register int	buc;
{
    DS_INC_FREE( buc );

    DMDB_CHECK_ADDR_ALIGN( pb, buc );

    pb->head.w = OR_HEAD( status, buc);
    pb->flink = budBuckets[buc].flink;
    pb->blink = &budBuckets[buc];

    budBuckets[buc].flink->blink = pb;	/* preserve locality of reference */
    budBuckets[buc].flink = pb;
}

/*
 * remove a block from it's bucket in the buddy allocator
 */
static BudBlkFree *
bud_blk_unfree_by_addr( pb)
    BudBlkFree      *pb;
{
    int		buc = pb->head.s.buc;

    DS_DEC_FREE( buc );

    DMDB_CHECK_ADDR_ALIGN( pb, buc );
    if( pb->head.s.status == IN_Q )
    {
	DMDB_DEC_BUC_IN_Q( pb->head.s.buc );
	ADDSLOTTOFREESLOTQ( pb->slotlist );
    }

    pb->flink->blink = pb->blink;
    pb->blink->flink = pb->flink;

    pb->head.w = OR_HEAD( USED, buc);
    return pb;
}

/*
 * Take a block off buc's free list in the buddy allocator
 */
static BudBlkFree *
bud_blk_unfree( buc)
    register int	buc;
{
    return bud_blk_unfree_by_addr( budBuckets[buc].flink);
}

/*
 * split a block pb in bucket buc into two equal blocks of size
 * of bucket (buc-1).
 */
static BudBlkFree *
split1( pb, buc)
    BudBlkFree		*pb;	/* split into two, put one in buc-1 */
    register int	buc;
{
    bud_blk_free( pb, NOT_USED, buc-1);

    return (BudBlkFree *)((char *)pb + (1<<buc-1)); /* return the second block */
}

/*
 * split a block pb in bucket buc into two equal blocks of size
 * of bucket (buc-1).
 * Return the first half
 */
static BudBlkFree *
split2( pb, buc)
    BudBlkFree		*pb;	/* split into two, put one in buc-1 */
    register int	buc;
{
    bud_blk_free( ((char *)pb + (1 << (buc - 1))), NOT_USED, buc-1);

    return pb; /* return the first block*/
}

/*
 * split_to_size could have been implemented recursively, but iteration
 * seems just as clean.
 */
static BudBlkFree *
split_to_size( buc)
    register int	buc; 
		    /*split larger blocks, if any, to yield one of size buc */
{
    register int	ic;
    BudBlkFree		*pb;

    for ( ic=buc+1; ic<BUD_MAX_BUC; ic++)
    {
	if ( budBuckets[ic].flink != &budBuckets[ic])
	    break;
    }
    if ( ic == BUD_MAX_BUC)
	return NULL;

    for ( pb = bud_blk_unfree( ic); ic>buc; ic--)
	pb = split1( pb, ic);

    pb->head.w = OR_HEAD( USED, buc);
    return pb;
}

/*
 * recursively coalease a block of memory which has been given
 * up for freeing, in the buddy allocator
 */
static void
free_recurs( pb, buc)
    BudBlkFree      	*pb;
    register int	buc;
{
    BudBlkFree	*pbuddy = (BudBlkFree *) ((int)pb ^ (1<<buc));

    /*
     * The termination conditions for the recursion are
     * (a) buddy is not in the valid address space
     * (b) buddy is in use
     * (c) buddy does not belong to the same bucket
     * (d) buddy is in the lazy Q. In which case the consolidation
     *     is done when the buddy is freed recursively
     */
    if ( pbuddy < (BudBlkFree *)budHeapStart
      || pbuddy >= (BudBlkFree *)heapStop
      || pbuddy->head.w != OR_HEAD( NOT_USED, buc))
    {
	/* XXX is the following test neccessary anymore??? */
	if( (pb->head.s.status == USED) || (pb->head.s.buc != buc) )
	{
		bud_blk_free( pb, NOT_USED, buc );
	}
	return;
    }

    (void)bud_blk_unfree_by_addr( pbuddy );
    
    free_recurs( (BudBlkFree *) ((int)pb & ~(1<<buc)), buc+1);
}

/*
 * add a block to be freed to the lazy queue, in the buddy
 * allocator
 */
static void
put_q( pb, buc )
    BudBlkFree      	*pb;
    register int	buc;
{
    DMDB_INC_BUC_IN_Q( buc );

    bud_blk_free( pb, IN_Q, buc);
    REMOVESLOTFROMFREESLOTQ( pb );
    return;
}

/*
 * frees all elements in the lazy q
 */
static void free_q()
{
    BudBlkFree *tpb;

    while( lazyFreeQ )
    {
	tpb = lazyFreeQ->freePtr;
	(void)bud_blk_unfree_by_addr( tpb );
	free_recurs( tpb, tpb->head.s.buc );
    }
}

/*
 * This routine finds the excess amount that should be allocated
 * if an sbrk is about to be performed.
 * This routine should only be called when LOG2_MIN_SBRK >= buc
 */
static int calc_excess_required( minRequired, buc )
register int minRequired;
register int buc;
{
    register int newtop;
    register int moreRequired;
    register int m;

    /*
     * allocate more memory such that there is atleast one
     * block in all buckets from buc+1 to LOG2_MIN_SBRK
     * and the final address is aligned
     * to bucket LOG2_MIN_SBRK. This size is stored in 
     * moreRequired
     */
    moreRequired = (1<<(LOG2_MIN_SBRK + 1)) - (1<<(buc + 1));
    newtop = ((int)heapStop) + minRequired + moreRequired;
    m = (int)newtop & ((1<<LOG2_MIN_SBRK) - 1);
    if( m )
    {
        moreRequired += (1<<LOG2_MIN_SBRK) - m;
    }

    DMDB_CHECK_ADDR();
    return moreRequired;
}

/*
 * if the memory is not available, then return FALSE.
 * if the memory is not contiguous, then abort due to fatal error.
 * if memory is available, then bump sbrkTop and return TRUE.
 */
static int get_mem( total )
int total;
{
        register int m;
 
        m = (int) SBRK( total );

	/*
	 * not enough memory available
	 */
        if (m == NO_MORE_MEMORY)
                return( FALSE);

	/*
	 * buddy system will fail, memory not continuous
	 */
        if (m != (int) sbrkTop)
	    FatalError("BADALLOC: sbrk returned non-contiguous storage\n");

	/*
	 * success!!
	 */
        sbrkTop += total;
        return( TRUE);
}

/*
 * get memory from the kernel. Get more memory than what is
 * actually needed (if possible) to delay calling sbrk again
 *
 * Buffer the sbrks. If xalloc sees the entire chunk which is sbrk-ed
 * it causes fragmentation. On the other hand if we sbrk only what
 * is required, we would have to do many more sbrks. So we buffer the
 * sbrk, and ask for much more than what is needed (if buc < LOG2_MIN_SBRK )
 * but only let xalloc know about the memory that was actually needed.
 * Thus fragmentation and too many sbrks are avoided
 */
static int 
new_chunk( buc )
register int	buc;
{
    BudBlkHead	*pb;
    register int	total;
    register int	ib;
    register int	minRequired;

    int bufAmount;	/* bufAmount=num_byte(sbrkTop - heapStop)*/

    bufAmount = (int)(sbrkTop - heapStop);

    /*
     * allocate all memory from the end of the existing
     * heap to the next memory address that is aligned to 
     * the requested bucket size. This size is minRequired
     */
    minRequired = ((int)heapStop) & ((1<<buc) - 1);
    if( minRequired )
    {
	minRequired = (1<<(buc + 1)) - minRequired;
    }
    else
    {
	minRequired = (1<<buc);
    }

    if( minRequired > bufAmount )
    {
	register int	moreRequired = 0;

	if( LOG2_MIN_SBRK >= buc )
	    moreRequired = calc_excess_required( minRequired, buc );

	/*
	 * get more memory .. if we can not get the amount requested then 
	 * try to back off and ask for less.
	 */
	total = moreRequired + minRequired - bufAmount;
	if ( !get_mem( total ) )
	{
	    if ( !moreRequired || !get_mem(total - moreRequired) )
	    {
		(*xErrF)( "BADALLOC: Out of virtual memory\n");
		return FALSE;
	    }
	}
    }

    /* else
     * {
     *     do not perform an sbrk, enough memory has been preallocated
     *     just bump heapStop
     *     bufAmount -= minRequired;
     * }
     */

    pb = (BudBlkHead *)heapStop;
    heapStop += minRequired;
    while( ib = newffs( minRequired ) )
    {
	bud_blk_free( (BudBlkFree *)pb, NOT_USED, --ib);
	pb  = (BudBlkHead *)((char *)pb + (1 << ib));
	minRequired = minRequired ^ (1 << ib);
    }

    return TRUE;
}


/*
 * returns bit number 1..32, or 0 if none were set.
 * Note that this is
 *      |_ log2( n) _| + 1, for n != 0
 *
 * Also, _          _
 *      |  log2( n)  | == |_ log2( n+1) _| + 1
 * so    _          _
 *      |  log2( n)  | == newfls( n+1)
 *
 * The integer ceiling of the base 2 logarithm is the same as "rounding up
 * to the next higher power of 2", and returning the exponent.
 */
static unsigned int
newfls( n)
    register unsigned int	n;
{
    register unsigned int	i=0;

    if ( n & 0xffff0000)
    {
	n >>= 0x10;
	i = 16;
    }
    if ( n & 0xff00)
    {
	n >>= 0x8;
	i += 8;
    }
    if ( n & 0xf0)
    {
	n >>= 0x4;
	i += 4;
    }
    if ( n & 0xc)
    {
	n >>= 0x2;
	i += 2;
    }
    if ( n & 0x2)
    {
	n >>= 0x1;
	i += 1;
    }
    return i+n;		/* n is either 0 or 1 */
}

/*
 * newffs() compiles into 35 MIPS instructions, about 29 of which are executed
 * in the case of a single bit set in a random location.
 *
 * The C shell times for 2^20 iterations are:
 *			user time	normalized
 *	ffs		5.1		1
 *	newffs		2.4		0.47
 *
 * ffs() runs in time proportional to the position of the set bit.
 * newffs() runs in not-quite-constant time (depends on number of set bits
 * in the *position*).
 *
 * The cryptic expression "((n-1)^n)&n" works as follow:
 *	-1	clear bit, trash below
 *	^n	clear above, set bit
 *	&n	clear below
 */ 
static unsigned int
newffs( n)
    register unsigned int	n;
{
    register unsigned int	i = 0;

    if ( n == 0)
	return 0;
    n = ((n-1)^n)&n;		/* clear higher bits */
    if ((n & 0xffff) == 0x0000)
    {
	n >>= 0x10;
	i |= 0x10;
    }
    if ( n & 0xff00)
	i |= 0x8;
    if ( n & 0xf0f0)
	i |= 0x4;
    if ( n & 0xcccc)
	i |= 0x2;
    if ( n & 0xaaaa)
	i |= 0x1;
    return i+1;
}

/*
 * hashing function to convert the numbr of bytes to the bucket
 * number for the buddy allocator
 */
static int
bytes_to_buc( nb)
    register int	nb;
{
    if ( nb >> LOG2_SMALL_TABLE_SIZE == 0)	/* performance hack */
	return small_bytes_to_buc[ nb];
    else /* can't use table, compute instead */
	return newfls( nb-1+sizeof(BudBlkHead)/*3*/);
}


#ifdef ATDALLOC
/*
 * These are called by standard library routines
 */
char *malloc(nbytes)		/* compatibility */
    register unsigned nbytes;
{
    char    *MALLOC();
    return MALLOC(nbytes);
}
char *realloc(chp, nbytes)
    char     *chp;
    unsigned nbytes;
{
    char    *REALLOC();
    return REALLOC(chp, nbytes);
}
void free(chp)
    char    *chp;
{
    void FREE();
    FREE(chp);
}

/* Xcalloc
*/
char *
Xcalloc(amount)
register unsigned amount;
{
    register char *ret;

    if (ret = MALLOC(amount))
	bzero(ret, (int) amount);
    return ret;
}

#endif

/*
 * The strategy is:
 *	- if a block of the proper size is free, return it
 *	- if a larger block exists, break it up and return the appropriate
 *		fragment
 *	- call sbrk() to get more virtual space
 */
SCOPE char *MYMALLOC/*Xalloc*/( nb)
register int	nb;
{
    register int	buc;
    BudBlkFree		*pb;

    DMST_INIT_TMEM();

    if ( !Initialized)
	initialize();

    /*
     * first try to allocate memory from the fixed block allocator
     */
    {
	register int		fbuc;
	register FixBlkFree	*pb;

        if ( nb <= (1 << LOG2_SMALL_FIXTABLE_SIZE ) )
        {
	    fbuc = small_bytes_to_fixbuc[nb];
            if( pb = fixBuckets[fbuc].flink )
	    {
		DMST_ALLOC( fixSizeNumber[fbuc].size );
		fixBuckets[fbuc].flink = pb->flink;
		return (char *)((FixBlkHead *)pb + 1);
	    }
        }
        else
        {
            register int nbt;

            if ( (nbt = nb + sizeof( FixBlkHead )) 
			<= fixSizeNumber[MAX_FIXBUC-1].size )
            {
	        fbuc = fixbuchash( nbt );
                if( pb = fixBuckets[fbuc].flink )
	        {
		    DMST_ALLOC( fixSizeNumber[fbuc].size );
		    fixBuckets[fbuc].flink = pb->flink;
		    return (char *)((FixBlkHead *)pb + 1);
	        }
	    }
	}
    }

    /*
     * the fixed size allocator did not have the required size
     * block, try to get it from the buddy allocator
     * If the required bucket has a block, return it. Otherwise
     * split up a larger block to the required size. If no bigger 
     * block exists get more memory from the kernel
     */
    buc = bytes_to_buc( nb); /* effects round-up to block size */

    DMDB_UPDATENUMINBUC( buc );

    if ( budBuckets[buc].flink != &budBuckets[buc])
    {
	DMST_ALLOC( 1 << buc );
	return (char *)((BudBlkHead *)bud_blk_unfree( buc)+1);
    } 

    if ( (pb = split_to_size( buc)) != NULL)
    {
	DMST_ALLOC( 1 << buc );
        return (char *)((BudBlkHead *)pb+1);
    }

    free_q();
    if ( budBuckets[buc].flink != &budBuckets[buc])
    {
	DMST_ALLOC( 1 << buc );
        return (char *)((BudBlkHead *)bud_blk_unfree( buc)+1);
    }
    if ( (pb = split_to_size( buc)) != NULL)
    {
	DMST_ALLOC( 1 << buc );
        return (char *)((BudBlkHead *)pb+1);
    }

    /* call sbrk() to get more virtual memory */
    {
	if ( !new_chunk( buc ) )
	    return ((char *)NULL);
	DMST_ALLOC( 1 << buc );
        return (char *)((BudBlkHead *)bud_blk_unfree( buc)+1);
    }
}

/*
 * Coalesce virtually-contiguous blocks iff they are of equal size and the
 * result will be naturally-aligned.
 */
SCOPE void MYFREE/*Xfree*/( pc)
    char	*pc;
{
    BudBlkFree	*pb = (BudBlkFree *)(pc-sizeof(BudBlkHead));

    if ( pc == NULL)	/* why does malloc allow this? */
	return;

    CHECK_ADDR_UNDERFLOW( pb, BADFREE );

    DMNST_MEM_CHECK( pb, BADFREE );

    /*
     * if the returned memory address is less than budHeapStart,
     * then it is to be handled by the fixed bucket allocator.
     * Otherwise it is to be handled by the buddy allocator
     */
    if( (char *)pb < budHeapStart )
    {
	((FixBlkFree *)pb)->flink = fixBuckets[pb->head.s.buc].flink;
	fixBuckets[pb->head.s.buc].flink = (FixBlkFree *)pb;
	DMST_FREE( fixSizeNumber[pb->head.s.buc].size );
	return;
    }

    CHECK_ADDR_OVERFLOW( pb, BADFREE );

    DMDB_DOWNDATENUMINBUC( pb->head.s.buc );

    /*
     * buddy allocator block. If there are more slots to
     * put another block in a "lazy free" queue, do it.
     * Otherwise flush the q
     */
    DMST_FREE( 1 << pb->head.s.buc );
    if ( freeSlotQ == NULL )
    {
	    DMST_INC_Q_FULL();

	    free_q();
	    free_recurs( pb, pb->head.s.buc);
    }
    else
    {
	    put_q( pb, pb->head.s.buc );
    }
    return;
}

/*
 * policy decision:  never copy to a smaller block if the caller asks to shrink
 * a block, just leave the space unused.
 */
char *REALLOC/*Xrealloc*/( pc, nb)
    char		*pc;
    register int	nb;
{
    BudBlkFree	*pb = (BudBlkFree *)(pc-sizeof(BudBlkHead));
    register int size;

    if ( pc == NULL)
	return Xalloc( nb);

    CHECK_FATAL_ADDR_UNDERFLOW( pb );

    CHECK_FATAL_ADDR_OVERFLOW( pb );

    /*
     * The old version allowed a free block to be realloced, in which
     * case there was no check for size agreement. This facility is no
     * longer allowed. i.e. the block SHOULD be in use
     */
    if ( pb->head.s.status !=USED )
    {
	FatalError( "BADREALLOC: Re-allocating a free block not supported\n" );
    }

    if( (char *)pb < budHeapStart )
    {
	if( nb + sizeof( FixBlkHead ) <= 
		(size = fixSizeNumber[((FixBlkFree *)pb)->head.s.buc].size) )
	{
	    return pc;
	}
    }
    else
    {
	if ( (size = (1<<pb->head.s.buc) - sizeof( BudBlkHead)) > nb )
        {
	    return pc;
        }
    }
    {
	char	*pnew = Xalloc( nb);

	if( pnew )
	{
	    wcpy( pnew, pc, BtoW( size ));
	    Xfree( pc);
	}
	return pnew;
    }
}

/*
 * Returns the size of a block that was allocated in this allocator
 * This function is useful for buffer management routines that handle
 * buffers that may grow. The main advantage of this function is to increase
 * efficiency (a major consideration in critical sections of the code).
 */
int
XGetBlkSize( pc )
char *pc;
{
    BudBlkHead	*pb = ((BudBlkHead *)pc) - 1;

    /*
     * if the block was from the fixed size allocator, then the
     * size is got from the "fixSizeNumber[]" array
     */
    if( (char *)pb < budHeapStart )
    {
	if( (char *)pb < fixHeapStart )
	{
	    return 0;
	}
	return( fixSizeNumber[((FixBlkHead *)pb)->s.buc].size 
		 - sizeof( FixBlkHead ) );
    }

    /*
     * if the block was from the buddy allocator, then the
     * size is 2 raised to the power of the bucket
     */
    if( (char *)pb >= heapStop )
    {
	return 0;
    }
    return( (1 << (pb->s.buc)) - sizeof( BudBlkHead ) );
}

/*
 * Copies the data into a new block if there is too much wastage and it
 * is possible to copy it.
 * This routine differs from Xrealloc since if nb < the size of the data
 * block and if the block is from the fixed allocator portion, then
 * xrealloc will not do anything. And if the block is from the buddy 
 * allocator, then it would follow the rules used by Xalloc (which is
 * not the best thing to do for shrinking a block).
 * If nb > the size of the block, nothing is changed.
 */
char * 
XShrinkBlock( data, nb )
char	*data;
int	nb;
{
    BudBlkFree *pb = (BudBlkFree *)(data - sizeof(BudBlkHead));

    CHECK_ADDR_UNDERFLOW( pb, BADSHRINK );

    DMNST_MEM_CHECK( pb, BADSHRINK );

    DMST_INIT_TMEM();

    /*
     * Only when the shrunk size bucket is less than the current bucket
     * number is an actual shrink done
     */
    if( (char *)pb < budHeapStart )
    {
	FixBlkFree *tfpb;
	register int fbuc;

	fbuc = (nb <= (1 << LOG2_SMALL_FIXTABLE_SIZE))?
		small_bytes_to_fixbuc[nb]:
		fixbuchash( nb + sizeof( FixBlkHead ) );
	for( ; fbuc < ((FixBlkFree *)pb)->head.s.buc; fbuc++ )
	{
	    if( tfpb = fixBuckets[fbuc].flink )
	    {
		DMST_ALLOC( fixSizeNumber[fbuc].size );
		fixBuckets[fbuc].flink = tfpb->flink;
		wcpy( (FixBlkHead *)tfpb + 1, data, nb >> 2 );

		((FixBlkFree *)pb)->flink =
			fixBuckets[((FixBlkFree *)pb)->head.s.buc].flink;
		fixBuckets[((FixBlkFree *)pb)->head.s.buc].flink =
			(FixBlkFree *)pb;
		DMST_FREE( fixSizeNumber[((FixBlkFree *)pb)->head.s.buc].size );
		
		return (char *)((FixBlkHead *)tfpb + 1);
	    }
	}
	return data;
    }

    CHECK_ADDR_OVERFLOW( pb, BADSHRINK );

    /*
     * A block of the shrunk size having a lower address than the
     * existing block is sought. Only if such a block is found is the
     * the recopying to the smaller block done. Otherwise the current
     * block is broken up, and the unused parts are put into the
     * appropriate free buckets. Trying to copy to lower addressed
     * blocks is good, as it lowers fragmentation.
     */
    {
	register int buc, tbuc;
	BudBlkHead *tpb;

	tbuc = buc = bytes_to_buc( nb );
	/*
	 * POLICY DECISION:
	 *	We would rather copy data to a slightly larger block
	 *	than required if we can move the data to lower addressed
	 *	memory (to prevent fragmentation
	 */
	for( ; buc < pb->head.s.buc; buc ++ )
	{
	    if( budBuckets[buc].flink != &budBuckets[buc] &&
		budBuckets[buc].flink < pb )
	    {
		DMST_ALLOC( 1 >> buc );
		DMDB_UPDATENUMINBUC( buc );
		DMDB_DOWNDATENUMINBUC( pb->head.s.buc );
		DMST_FREE( 1 << pb->head.s.buc );
		tpb = (BudBlkHead *)bud_blk_unfree( buc );
		wcpy( tpb + 1, data, nb >> 2 );

		if ( freeSlotQ == NULL )
		{
		    DMST_INC_Q_FULL();

		    free_q();
		    free_recurs( pb, pb->head.s.buc);
		}
		else
		{
		    put_q( pb, pb->head.s.buc );
		}

		return (char *)(tpb + 1);
	    }
	}

	if( tbuc < (buc = pb->head.s.buc) )
	{
	     DMDB_UPDATENUMINBUC( tbuc );
	     DMDB_DOWNDATENUMINBUC( buc );
	     for( ; buc > tbuc; buc-- )
	     {
		pb = split2( pb, buc );
		DMST_FREE( 1 << (buc - 1) );
	     }
	     pb->head.s.buc = tbuc;
	}
	return data;
    }
}


/*
 * debugging routines follow
 */
#ifdef DM_SIZE_TRACE
    static char *mysbrk( n )
    int n;
    {
	char *i;

	if( (i = sbrk( n )) != (char *)NO_MORE_MEMORY )
	{
	    total_memory += n;
	}
	return i;
    }

    char *
    MALLOC/*Xalloc*/( nb)
    int         nb;
    {
        char *MYMALLOC();
        char *temp;
        int tused = used_memory;

        temp = MYMALLOC( nb );

        if( nb < MAX_SIZE_TRACE )
        {
	    ((BudBlkHead *)(temp - sizeof( BudBlkHead )))->s.signature = 
						(short)nb;
	    numInSize[nb]++;
	    if( numInSize[nb] > maxSizeCall[nb] )
	        maxSizeCall[nb]++;
	    MEM_USED( nb );
	    return temp;
        }
        else
        {
	    ((BudBlkHead *)(temp - sizeof( BudBlkHead )))->s.signature = 
						(short)0;
	    MEM_USED( nb );
	    return temp;
        }
    }

    void FREE/*Xfree*/( pc)
    char *pc;
    {
        register int i;
        void myfree();

        if( pc < fixHeapStart || pc > heapStop )
	    return;
        if( (i = ((BudBlkHead *)(pc - sizeof( BudBlkHead )))->s.signature) 
						< MAX_SIZE_TRACE )
        {
	    numInSize[i]--;
        }
        MYFREE( pc );
    }

#endif

/*
 * The following function is useful and may be used at a later time
 */
#if USEFUL_FUNCTION
    void
    check_free_lists()
    {
        int		ib;
        BudBlkFree	*pb;

        for ( ib=0; ib<BUD_MAX_BUC; ib++)
        {
	    for ( pb = budBuckets[ib].flink; 
		  pb->flink != &budBuckets[ib]; 
		  pb = pb->flink)
	        if ( pb->head.s.buc != ib)
		    FatalError( "check_free_lists\n");
        }
    }
#endif

#ifdef GATHER_STATS
    static void
    gather_stats()
    {
        BudBlkHead	*pb;
        int		i;

        for ( i=0; i<BUD_MAX_BUC; i++)
	    stats_inuse[i] = stats_free[i] = 0;
        stats_total = 0;

        for (   pb = (BudBlkHead *)budHeapStart;
	    pb < (BudBlkHead *)heapStop;
	    pb = (BudBlkHead *)((char *)pb + (1<<pb->s.buc)))
        {
	    if ( pb->s.signature != SIG)
	        FatalError( "Xalloc: signature overwritten\n");

	    if ( pb->s.status)
	        stats_free[pb->s.buc]++;
	    else
	        stats_inuse[pb->s.buc]++;
	    stats_total += 1<<pb->s.buc;
        }
    }
#else
#   ifdef DM_RESOURCE_DUMP
	static void resource_use()
	{
	    struct rusage ruse;

	    getrusage( 0, &ruse );
	    printf("page reclaims = %d\n", ruse.ru_minflt);
	    printf("page faults = %d\n", ruse.ru_majflt);
	    printf("swaps = %d\n", ruse.ru_nswap);
	    printf("block input operations = %d\n", ruse.ru_inblock);
	    printf("block output operations = %d\n", ruse.ru_oublock);
	    printf("voluntary context switches = %d\n", ruse.ru_nvcsw);
	    printf("involuntary context switches = %d\n", ruse.ru_nivcsw);
	}
#   endif
#   ifdef DUMPSTAT
	static int dump_stats( size)
	int size;
	{
	    int	buc;
	    static int	stats_changes;

	    stats_changes+=size;
	    if ( (stats_changes > 1000))
	    {
		stats_changes = 0;
		for ( buc=BUD_MIN_BUC; buc<BUD_MAX_BUC; buc++)
	    	    ErrorF( "%d\t%d\n", buc, newfls( stats_free[buc]-1));
	    }
	}
#   endif
#endif
