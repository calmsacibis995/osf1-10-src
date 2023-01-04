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
static char *rcsid = "@(#)$RCSfile: rld_malloc.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1993/05/28 19:13:52 $";
#endif

#include <sys/types.h>
#include <sys/param.h>
#include <stdlib.h>
#include <errno.h>

/*
 * c h k m a l l o c
 *
 * Wrapper routine for malloc.  Tests for a successful malloc call,
 * and calls fatal() for unsuccessful malloc attempts.
 */

void *
chkmalloc(int size)
{
    char *p;

    if (size > 0) {
	p = (char *)malloc(size);
	if (!p) {
	    fatal("cannot malloc");
	}
	return(p);
    }
    return((void *)NULL);
}

/*
 * c h k r e a l l o c
 *
 * Wrapper routine for realloc.  Tests for a successful realloc call,
 * and calls fatal() for unsuccessful realloc attempts.
 */

void *
chkrealloc(void *p, int size)
{
    char *r;
    if (size > 0) {
	r = (char *)realloc(p,size);
	if (!r) {
	    fatal("cannot realloc");
	}
	return(r);
    }
    return((void *)NULL);
}

/*
 * TEMPORARY INCLUSION OF malloc.c for V1.3.  These changes will be
 * folded back into libc's malloc.c for V1.4.
 */


/*
 * This is a very fast storage allocator.  It allocates blocks of a small 
 * number of different sizes, and keeps free lists of each size.  Blocks that
 * don't exactly fit are passed up to the next larger size.  In this 
 * implementation, the available sizes are 2^n-8 (or 2^n-18) bytes long.
 * This is designed for use in a virtual memory environment.
 */


/*
 * The overhead on a block is at least 8 bytes.  When free, this space
 * contains a pointer to the next free block, and the bottom two bits must
 * be zero.  When in use, the first byte is set to MAGIC, and the second
 * byte is the size index.  The remaining bytes are for alignment.
 * If range checking is enabled then a second word holds the size of the
 * requested block, less 1, rounded up to a multiple of sizeof(RMAGIC).
 * The order of elements is critical: ov_magic must overlay the low order
 * bits of ov_next, and ov_magic can not be a valid ov_next bit pattern.
 *
 * The overhead at the beginning of a block is a multiple of 8 bytes. 
 * This is done to insure the address returned to the user is on an 8 byte
 * boundary.  This helps performance on machines which enforce an 8 byte
 * boundary alignment on certain data types if the user's data structure
 * maps these types to an 8 byte boundary.  When range checking is enabled
 * there are 2 bytes of overhead at the end of a block.
 */

union	overhead {
	union	overhead *ov_next;	/* when free */
	struct {
		u_long	ovu_magic;	/* magic number */
		u_long	ovu_index;	/* bucket # */
#ifdef RCHECK
		u_short	ovu_ralign;	/* just for alignment */
		u_short	ovu_rmagic;	/* range magic number */
		size_t	ovu_size;	/* actual block size */
#endif
	} ovu;
#define	ov_magic	ovu.ovu_magic
#define	ov_index	ovu.ovu_index
#define	ov_rmagic	ovu.ovu_rmagic
#define	ov_size		ovu.ovu_size
};

#define	MAGIC		0xef		/* magic # on accounting info */
#define RMAGIC		0x5555		/* magic # on range info */

#ifdef RCHECK
#define	RSLOP		sizeof (u_short)
#else
#define	RSLOP		0
#endif

/*
 * nextf[i] is the pointer to the next free block of size 2^(i+BUCKET_SHIFT).  The
 * smallest allocatable block is 8 bytes on 32 bit machines and 16 on 64 bit
 * machines.  The overhead information
 * precedes the data area returned to the user.
 */
#ifdef __alpha
#define BUCKET_SHIFT 4
#define MIN_BUCKET_AMT 16
#define FIRST_BUCKET 0
#define	NBUCKETS 62
#else
#define BUCKET_SHIFT 3
#define MIN_BUCKET_AMT 8
#define FIRST_BUCKET 0
#define	NBUCKETS 30
#endif
static	union overhead *nextf[NBUCKETS];

extern	void *sbrk(ssize_t);
extern	int getpagesize(void);
static void morecore(int);
static void fracture(union overhead *, int, size_t);
static void freeblock(union overhead *, int);
static int findbucket(union overhead *, int);
static int whatbucket(size_t);
extern void bcopy(char *, char *, int);

static	int pagesz;			/* page size */
static	int pagebucket;			/* page size bucket */
static	int sbrkadjust;			/* adjustment if sbrk rounds up */

/* 
 *	Variables, defines for mallopt simulation and mallinfo.
 */
#define ALIGNSZ	  sizeof(double)	/* aligns anything (smallest grain) */
#define NUMLBLKS  100	/* default number of small blocks per holding block */
#define MAXFAST   0	/* default maximum size block for fast allocation */
static int maxfast = MAXFAST;	/* maximum size block for fast allocation */
static int numlblks = NUMLBLKS;	/* number of small blocks per holding block */
static int grain = ALIGNSZ;	/* size of small block */
static int fastmallocs = 0;	/* != 0, once small malloc after mallopt */


/*
 * nmalloc[i] is the difference between the number of mallocs and frees
 * for a given block size.
 */
static	u_int nmalloc[NBUCKETS];
#ifdef MSTATS
#include <stdio.h>
#endif

#if defined(DEBUG) || defined(RCHECK)
#define	ASSERT(p,m,d)   if (!(p)) { botch(m,d); return; }
#include <stdio.h>
static void
botch(char *s, int d)
{
	fprintf(stderr, "\r\n");
	fprintf(stderr, s, d);
	fprintf(stderr, "\r\n");
 	(void) fflush(stderr);		/* just in case user buffered it */
}
#else
#define	ASSERT(p,m,d)
#endif

/*
 * NAME:	malloc
 *
 * FUNCTION:	malloc - memory allocator
 *
 * NOTES:	Malloc allocates and returns a pointer to at least
 *		'nbytes' of memory.
 *
 * RETURN VALUE DESCRIPTION:	NULL if the there is no more available
 *		memory or if the memory arena was corrupted.
 */

void *
malloc(size_t nbytes)
{
  	register union overhead *op;
  	register int bucket;
	register size_t amt, n;

	/* When zero bytes are requested NULL is returned (SVID behavior) */
	if (nbytes == 0) {
		errno = EINVAL;
		return (NULL);
	}

	/*
	 * First time malloc is called, setup page size and
	 * align break pointer so all data will be page aligned.
	 * Sbrk can round up our request to a higher boundary
	 * so check what comes back to assure page alignment.
	 */
	if (pagesz == 0) {
		pagesz = n = getpagesize();
		op = (union overhead *) sbrk(0L);
  		n = n - sizeof (*op) - ((unsigned long)op & (n - 1));
		if ((int)n < 0)
			n += pagesz;
  		if (n) {
  			if (sbrk((ssize_t)n) == (void *) -1)
				return (NULL);
		}
		if (sbrk(0L) != (void *) (n + op))
			sbrkadjust = 1;
		bucket = 0;
		amt = MIN_BUCKET_AMT;
		while (pagesz > amt) {
			amt <<= 1;
			bucket++;
		}
		pagebucket = bucket;
	}
	/*
	 * Convert amount of memory requested into closest block size
	 * stored in hash buckets which satisfies request.
	 * Account for space used per block for accounting.
	 */
	if ((bucket = whatbucket(nbytes)) < 0)
		return (NULL);
	/*
	 * If nothing in hash bucket right now,
	 * request more memory from the system.
	 */
  	if ((op = nextf[bucket]) == NULL) {
  		morecore(bucket);
  		if ((op = nextf[bucket]) == NULL)
  			return (NULL);
	}
	/* remove from linked list */
  	nextf[bucket] = op->ov_next;
	op->ov_magic = MAGIC;
	op->ov_index = bucket;
	if (nbytes <= maxfast)
		fastmallocs++;
  	nmalloc[bucket]++;
#ifdef RCHECK
	/*
	 * Record allocated size of block and
	 * bound space with magic numbers.
	 */
	op->ov_size = (nbytes + RSLOP - 1) & ~(RSLOP - 1);
	op->ov_rmagic = RMAGIC;
  	*(u_short *)((caddr_t)(op + 1) + op->ov_size) = RMAGIC;
#endif
  	return (op + 1);
}

/*
 * Allocate more memory to the indicated bucket.
 */

#define LOADER_HEAP_CEILING 0x3ffc0080000L
static int use_ovfl = 0;                     /* Use loader's overflow heap */

static void
morecore(int bucket)
{
  	register union overhead *op;
	register size_t sz;		/* size of desired block */
  	size_t amt;			/* amount to allocate */
  	int nblks;			/* how many blocks we get */
	int sbucket;			/* bucket to steal memory from */

	/*
	 * sbrk_size <= 0 only for big, FLUFFY, requests
	 * or for a negative arg.
	 */
	sz = 1L << (bucket + BUCKET_SHIFT);
#ifdef DEBUG
	ASSERT(sz > 0, "malloc: internal error, bucket = %d", bucket);
#else
	if (sz <= 0L)
		return;
#endif
	if (sz < pagesz) {
		amt = pagesz;
  		nblks = amt / sz;
	} else {
		amt = sz + pagesz;
		nblks = 1;
	}

	/* If sbrk fails or returns space extending beyond the end of
	 * the loader heap, use the overflow heap for more space.
	 */

	if (!use_ovfl) {
	    op = (union overhead *) sbrk((ssize_t)amt);
	    if (((long)op + amt) > LOADER_HEAP_CEILING || (long)op == -1) {
		use_ovfl = 1;
	    }
	}
	if (use_ovfl) {
	    op = (union overhead *) ovfl_sbrk((ssize_t)amt);
	}

	/* no more room! */
  	if ((long)op == -1L) {
		/* try to steal room from a larger bucket */
		amt = ((sz < pagesz) ? sz : amt) - sizeof (*op) - RSLOP;
		for (sbucket = bucket + 1; sbucket < NBUCKETS; sbucket++)
			if ((op = nextf[sbucket]) != NULL) {
				fracture(op, sbucket, amt);
				nextf[sbucket] = op->ov_next;
				op->ov_next = nextf[bucket];
				nextf[bucket] = op;
				break;
			}
  		return;
	}
#ifdef	ASSUME_NOBODY_ELSE_CALLS_SBRK
	op = op - sbrkadjust;		/* align data to page boundary */
#endif
	/*
	 * Add new memory allocated to that on
	 * free list for this hash bucket.
	 */
  	nextf[bucket] = op;
  	while (--nblks > 0) {
		op->ov_next = (union overhead *)((caddr_t)op + sz);
		op = (union overhead *)((caddr_t)op + sz);
  	}
	op->ov_next = NULL;
}

/*
 * NAME:	free
 *
 * FUNCTION:	free - deallocate memory
 *
 * NOTES:	Free frees up the previously allocated block of memory
 *		'cp', making it available for later allocation.
 *
 * RETURN VALUE DESCRIPTION:	none
 */

void
free(void *cp)
{   
  	register int size;
	register union overhead *op;

  	if (cp == NULL)
  		return;

	op = (union overhead *)((caddr_t)cp - sizeof (union overhead));
#ifdef DEBUG
  	ASSERT(op->ov_magic == MAGIC,		/* make sure it was in use */
		"free: block at %#x not allocated by malloc, or malloc header clobbered",
		(int)cp);
#else
	if (op->ov_magic != MAGIC)
		return;				/* sanity */
#endif
#ifdef RCHECK
  	ASSERT(op->ov_rmagic == RMAGIC,
		"free: block at %#x has clobbered malloc header", (int)cp);
	ASSERT(*(u_short *)((caddr_t)(op + 1) + op->ov_size) == RMAGIC,
		"free: user program wrote data past end of block at %#x",
		(int)cp);
#endif
  	size = op->ov_index;
  	ASSERT(size < NBUCKETS,
		"free: block at %#x has clobbered malloc header", (int)cp);
	op->ov_next = nextf[size];	/* also clobbers ov_magic */
  	nextf[size] = op;
  	nmalloc[size]--;
}

/*
 * When a program attempts "storage compaction" as mentioned in the
 * old malloc man page, it realloc's an already freed block.  Usually
 * this is the last block it freed; occasionally it might be farther
 * back.  We have to search all the free lists for the block in order
 * to determine its bucket: 1st we make one pass thru the lists
 * checking only the first block in each; if that fails we search
 * ``realloc_srchlen'' blocks in each list for a match (the variable
 * is extern so the caller can modify it).  If that fails we just copy
 * however many bytes was given to realloc() and hope it's not huge.
 */
int realloc_srchlen = 4;	/* 4 should be plenty, -1 =>'s whole list */

/*
 * NAME:	realloc
 *
 * FUNCTION:	realloc - change size of previously allocated block
 *
 * NOTES:	Realloc changes the size of a previously allocated
 *		block of memory 'cp' to 'nbytes'.
 *
 * RETURN VALUE DESCRIPTION:	NULL if there is no available memory
 *		or if the memory memory has been corrupted.
 */

void *
realloc(void *cp, size_t nbytes)
{   
  	register size_t onb, i;
	union overhead *op;
  	char *res;
	int was_alloced = 0;
	int obucket;

  	if (cp == NULL)
  		return (malloc(nbytes));

	op = (union overhead *)((caddr_t)cp - sizeof (union overhead));
	if (op->ov_magic == MAGIC) {
		was_alloced++;
		i = obucket = op->ov_index;
	} else {
		/*
		 * Already free, doing "compaction".
		 *
		 * Search for the old block of memory on the
		 * free list.  First, check the most common
		 * case (last element free'd), then (this failing)
		 * the last ``realloc_srchlen'' items free'd.
		 * If all lookups fail, then assume the size of
		 * the memory block being realloc'd is the
		 * largest possible (so that all "nbytes" of new
		 * memory are copied into).  Note that this could cause
		 * a memory fault if the old area was tiny, and the moon
		 * is gibbous.  However, that is very unlikely.
		 */
		if ((int)(i = findbucket(op, 1)) < 0 &&
		    (int)(i = findbucket(op, realloc_srchlen)) < 0)
			i = NBUCKETS;
	}
	/* if new size is zero, the block pointed to is freed */
	if (nbytes == 0) {
		if (was_alloced)
			free(cp);
		return (NULL);
	}
	onb = 1L << (i + BUCKET_SHIFT);		/* old number of bytes */
	if (onb < pagesz)
		onb -= sizeof (*op) + RSLOP;
	else
		onb += pagesz - sizeof (*op) - RSLOP;
	/* avoid the copy if same size block */
	if (was_alloced) {
		if (i) {
			i = 1L << (i + 2);	/* next smallest block */
			if (i < pagesz)
				i -= sizeof (*op) + RSLOP;
			else
				i += pagesz - sizeof (*op) - RSLOP;
		}
		if (nbytes <= onb && nbytes > i) {
#ifdef RCHECK
			op->ov_size = (nbytes + RSLOP - 1) & ~(RSLOP - 1);
			*(u_short *)((caddr_t)(op + 1) + op->ov_size) = RMAGIC;
#endif
			return(cp);
		}
		/* old block is not freed yet in case malloc fails */
	}
  	if ((res = malloc(nbytes)) == NULL)
  		return (NULL);
  	if (cp != res)		/* common optimization if "compacting" */
 		bcopy((char*)cp, res, (int)((nbytes < onb) ? nbytes : onb));
	if (was_alloced)	/* safe to free old block */
		free(cp);
  	return (res);
}

/*
 * Search ``srchlen'' elements of each free list for a block whose
 * header starts at ``freep''.  If srchlen is -1 search the whole list.
 * Return bucket number, or -1 if not found.
 */
static int
findbucket(union overhead *freep, int srchlen)
{
	register union overhead *p;
	register int i, j;

	for (i = 0; i < NBUCKETS; i++) {
		j = 0;
		for (p = nextf[i]; p && j != srchlen; p = p->ov_next) {
			if (p == freep)
				return (i);
			j++;
		}
	}
	return (-1);
}

#ifdef MSTATS
/*
 * mstats - print out statistics about malloc
 * 
 * Prints two lines of numbers, one showing the length of the free list
 * for each size category, the second showing the number of mallocs -
 * frees for each size category.
 */
void
mstats(char *s)
{
  	register int i, j;
  	register union overhead *p;
  	int totfree = 0,
  	totused = 0;

  	fprintf(stderr, "Memory allocation statistics %s\nfree:\t", s);
  	for (i = 0; i < NBUCKETS; i++) {
  		for (j = 0, p = nextf[i]; p; p = p->ov_next, j++)
  			;
  		fprintf(stderr, " %d", j);
  		totfree += j * ((1L << (i + BUCKET_SHIFT)) + (i < pagebucket ? 0 : pagesz));
  	}
  	fprintf(stderr, "\nused:\t");
  	for (i = 0; i < NBUCKETS; i++) {
  		fprintf(stderr, " %d", nmalloc[i]);
  		totused += nmalloc[i] * ((1L << (i + BUCKET_SHIFT))
				       + (i < pagebucket ? 0 : pagesz));
  	}
  	fprintf(stderr, "\n\tTotal in use: %d, total free: %d\n",
	    totused, totfree);
}
#endif

#include <malloc.h>

/*
 * NAME:	mallopt
 *
 * FUNCTION:	mallopt - tune small block allocation algorithm
 *
 * NOTES:	Mallopt is included for SysV compatibility only.
 *		It has NO real function in this implementation.
 *		The malloc above should be faster in almost all
 *		cases.  Maybe someday the two will be merged.
 *
 * RETURN VALUE DESCRIPTION:	0, 1 on error
 */
/*      Mallopt - set options for allocation

	Mallopt provides for   control over the allocation algorithm.
	The cmds available are:

	M_MXFAST Set maxfast to value.  Maxfast is the size of the
		 largest small, quickly allocated block.  Maxfast
		 may be set to 0 to disable fast allocation entirely.

	M_NLBLKS Set numlblks   to value.  Numlblks is the number of
		 small blocks per holding block.  Value must be
		 greater than 0.

	M_GRAIN  Set grain to value.  The sizes of all blocks
		 smaller than maxfast are considered to be rounded
		 up to the nearest multiple of grain.    The default
		 value of grain is the smallest number of bytes
		 which will allow alignment of any data type.    Grain
		 will   be rounded up to a multiple of its default,
		 and maxsize will be rounded up to a multiple   of
		 grain.  Value must be greater than 0.

	M_KEEP   Retain data in freed   block until the next malloc,
		 realloc, or calloc.  Value is ignored.
		 This option is provided only for compatibility with
		 the old version of malloc, and is not recommended.

	returns - 0, upon successful completion
		  1, if malloc has previously been called or
		     if value or cmd have illegal values
*/

/* ARGSUSED */
int
mallopt(int cmd, int value)
{
	/* disallow changes once a small block is allocated */
	if (fastmallocs)  {
		return 1;
	}
	switch (cmd)  {
	    case M_MXFAST:
		if (value < 0)  {
			return 1;
		}
		/* round up to a multiple of grain size */
		maxfast = (value + grain - 1)/grain;
		break;
	    case M_NLBLKS:
		if (value <= 1)  {
			return 1;
		}
		numlblks = value;
		break;
	    case M_GRAIN:
		if (value <= 0)  {
			return 1;
		}
		/* round grain up to a multiple of ALIGNSZ */
		grain = (value + ALIGNSZ - 1)/ALIGNSZ*ALIGNSZ;
		break;
	    case M_KEEP:
		break;
	    default:
		return 1;
	}
	return 0;
}

/*
 * NAME:	mallinfo
 *
 * FUNCTION:	mallinfo - return information describing current
 *		malloc arena.
 *
 * NOTES:	Mallinfo in its original state returned a
 *		structure describing the current state of
 *		the malloc family.  Here it is only included
 *		for compatibility with SysV.
 *
 * RETURN VALUE DESCRIPTION:	struct mallinfo
 */

struct mallinfo
mallinfo(void)
{
	static struct mallinfo info;
  	register int i, j, k;
  	register union overhead *p;
	int fastbucket = whatbucket(maxfast);

	info.arena = 0;			/* total space in arena */
	info.ordblks = 0;		/* number of ordinary blocks */
	info.fordblks = 0;		/* space in free ordinary blocks */
	info.uordblks = 0;		/* space in ordinary blocks in use */
	info.smblks = 0;		/* number of small blocks */
	info.fsmblks = 0;		/* space in free small blocks */
	info.usmblks = 0;		/* space in small blocks in use */

  	for (i = 0; i < NBUCKETS; i++) {
  		for (j = 0, p = nextf[i]; p; p = p->ov_next, j++)
  			;
		/* free space in this bucket */
  		k = j * ((1L << (i + BUCKET_SHIFT)) + (i < pagebucket ? 0 : pagesz));
		info.arena += k;
		if (i > fastbucket) {
			info.ordblks += j;
			info.fordblks += k;
		} else {
			info.smblks += j;
			info.fsmblks += k;
		}
  	}
  	for (i = 0; i < NBUCKETS; i++) {
		/* used space in this bucket */
  		k = nmalloc[i] * ((1L << (i + BUCKET_SHIFT))
				+ (i < pagebucket ? 0 : pagesz));
		if (i > fastbucket) {
		info.arena += k;
			info.ordblks += nmalloc[i];
			info.uordblks += k;
		} else {
			info.smblks += nmalloc[i];
			info.usmblks += k;
		}
  	}

	return (info);
}

/*
 * Break up an old block of memory.  A new block is cut off the front
 * to satisfy a malloc request.  The remainder is put on the
 * free list as one or more blocks.
 */
static void
fracture(union overhead *op, int obucket, size_t nbytes)
{
	register size_t oamt, namt;	/* old and new block sizes */
	register size_t sz;		/* size of a memory block */
	register int nbucket, bucket;	/* new and work bucket numbers */

	oamt = 1L << (obucket + BUCKET_SHIFT);
	if (oamt >= pagesz)
		oamt += pagesz;
	nbucket = whatbucket(nbytes);
	op->ov_index = nbucket;
	namt = 1L << (nbucket + BUCKET_SHIFT);
  	nmalloc[obucket]--;
  	nmalloc[nbucket]++;
	/*
	 * Cut the new block off the front of the old block.  There
	 * will be a remainder to deal with here only when the new
	 * block is smaller than a page.  How much remainder depends on
	 * the old block size, but it will always be less than a page.
	 */
	if (namt < pagesz) {
		/* decide how much remainder gets handled here */
		if (oamt < pagesz) {
			sz = oamt - namt;
			oamt = 0;
		} else {
			sz = pagesz - namt;
			oamt -= pagesz;
		}
		op = (union overhead *)((caddr_t)op + namt);
		/* put namt sized blocks on the free list */
		do {
			freeblock(op, nbucket);
			op = (union overhead *)((caddr_t)op + namt);
		} while ((sz -= namt) > 0);
	} else {
		namt += pagesz;
		oamt -= namt;
		op = (union overhead *)((caddr_t)op + namt);
	}
	/*
	 * There may be no remainder left at this point, but if there
	 * is it will be in multiples of pagesz.  When multiple pages
	 * are left, take as large a block as will fit the remainder
	 * and put it on the free list.  Sometimes just one page is
	 * left, chop it up into smaller pieces and put them on the
	 * free list.  Keep doing this until there is nothing left.
	 */
	while (oamt > 0) {
		if (oamt > pagesz) {
			/* fit the largest block we can in the remainder */
			bucket = whatbucket(oamt - sizeof (*op) - RSLOP);
			sz = (1L << (bucket + BUCKET_SHIFT)) + pagesz;
			if (sz > oamt) {	/* wasn't an exact fit */
				bucket -= 1;
				sz = (1L << (bucket + BUCKET_SHIFT)) + pagesz;
			}
			/* put our best fit on the free list */
			op->ov_next = nextf[bucket];
			nextf[bucket] = op;
			op = (union overhead *)((caddr_t)op + sz);
			oamt -= sz;
		} else {
			/* just one page left, decide how to chop it up */
			bucket = (namt < pagesz) ? nbucket : pagebucket - 1;
			sz = 1L << (bucket + BUCKET_SHIFT);
			/* put sz sized blocks on the free list */
			do {
				freeblock(op, bucket);
				op = (union overhead *)((caddr_t)op + sz);
			} while ((oamt -= sz) > 0);
		}
	}
}

/*
 * Puts a block on the free list.
 */
static void
freeblock(union overhead *op, int bucket)
{
	op->ov_next = nextf[bucket];
	nextf[bucket] = op;
}

/*
 * Convert an amount of memory size into the closest block size
 * stored in hash buckets which will hold that amount.
 * Account for space used per block for accounting.
 * Return the bucket number, or -1 if request is too large.
 */
static int
whatbucket(size_t nbytes)
{
	register int bucket;
	register size_t amt, n;

	if (nbytes <= (n = pagesz - sizeof (union overhead) - RSLOP)) {
#ifndef RCHECK
                amt = MIN_BUCKET_AMT; /* size of first bucket */
                bucket = FIRST_BUCKET;

#else
		amt = 32;	/* size of first bucket */
		bucket = 2;
#endif
		n = -(sizeof (union overhead) + RSLOP);
	} else {
		amt = pagesz;
		bucket = pagebucket;
	}
	while (nbytes > amt + n) {
		amt <<= 1;
		if (amt <= 0)
			return (-1);
		bucket++;
	}
	return (bucket);
}
