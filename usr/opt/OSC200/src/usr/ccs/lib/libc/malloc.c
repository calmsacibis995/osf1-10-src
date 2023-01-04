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
static const char	*sccsid = "@(#)$RCSfile: malloc.c,v $ $Revision: 4.2.9.12 $ (DEC) $Date: 1994/01/13 16:20:55 $";
#endif 

#if !defined(lint) && !defined(_NOIDENT)

#endif

#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak mallopt = __mallopt
#pragma weak mallinfo = __mallinfo
#endif
#define valloc __valloc
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include <sys/mman.h>

/* cartesian.h */

static void *cartesian_alloc(size_t n,
		      size_t mod,
		      size_t residue,
		      unsigned long /* boolean */ exact);
static void cartesian_free(void *xx);
static void cartesian_noshrink(void);

struct cartesian_stats {
  unsigned long freeobjects;
  size_t totalbytes;
};

static struct cartesian_stats cartesian_stats();

/* malloc.c */

#undef NULL
#define NULL ((void*)0)

/* malloc and free, using binary-buddy for small nodes and falling
   back on Cartesian trees for large nodes.  Memory for small nodes is
   allocated in 6KB, 2KB, or 10KB "chunks" from the Cartesian heap. */

typedef unsigned long Bool;
#define TRUE 1
#define FALSE 0

/* In the comments below, a "word" is 64 bits on an Alpha and 32 bits
   on a R3000. */

/* All arithmetic is performed using longs, since this leads to
   slightly better code than using ints when possible. */

/* An ADDRESS is a numeric type that can hold an address. */

typedef unsigned long ADDRESS;

#ifndef __alpha
typedef signed long ssize_t;
#endif

#define WORDSIZE ((ssize_t)(sizeof(size_t)))

/* INC and DEC increment or decrement a pointer by a specified number
   of bytes. */

#define INC(p, n) ((void*)((char*)(p)+(n)))
#define DEC(p, n) ((void*)((char*)(p)-(n)))

/* All nodes are small or large, and each node is preceded by a signed
   one-word "bucket" field.  The following values are possibilities:

   0 < bucket <= MAXSMALLBUCKET    This is an allocated small node.
				   "bucket" holds the size index.

   0 < -bucket <= MAXBUCKET        This is a small node that has
				   recently been freed.  -bucket holds
				   the previous size index, or a larger
				   index.  Alternatively, it is a small
				   node that has been poisoned by a call
				   to mallopt(M_KEEP).

   MAXSMALL + WORDSIZE < -bucket   This is an allocated large node.
				   -bucket holds the size in bytes.

   MAXSMALL + WORDSIZE < bucket    This is a large node that has recently
				   been freed, or a small node heading a
				   chunk that has recently been freed.
				   bucket holds the previous size of the
				   large node or chunk, in bytes.

   bucket == 0			   This is a "keep" node, with a special
				   four-word header.  The first word
				   holds the size (if recently freed) or
				   the negative of the size (if allocated)
				   and the last word is the dummy zero
				   bucket number.

   When buggy programs call free or realloc with arguments that have
   already been freed, this redundant information can often be used to
   proceed gracefully. */

typedef signed long Bucket;
typedef unsigned long BucketI;

/* Small nodes have size [1 .. MAXSMALL] bytes, including header. */

#define MAXSMALL 640

/* There are small buckets from 3 * WORDSIZE up to 10KB.  Bucket 0 is
   never used; the following buckets are for 3 words, 4 words, 5 words,
   6, 8, 10, 12, 16, 20, and so on.  The size of bucket i is about
   2^(1/3) times the size of bucket i-1, and exactly 2 times the size of
   bucket i-3.  The three largest bucket sizes used--the "chunk"
   sizes--are 6KB, 2KB, and 10KB, each of which requires 2KB alignment.

   To accomodate future Alpha architecture changes, the minimum alignment
   of an object is 2 * WORDSIZE.  For this reason, buckets 1 and 3 are
   never used, since they have odd lengths. */

#ifdef __alpha
#define MAXSMALLBUCKET 15
#else
#define MAXSMALLBUCKET 18
#endif
#define MAXBUCKET (MAXSMALLBUCKET + 12)

/* Each small node (640 bytes or less, including header) contains a
   one-word header holding the bucket number: positive for allocated
   nodes, negative for free nodes.

   Free nodes belong to a doubly-linked list of free nodes of its size.
   Each first element has previous == 0; the last element has next == 0.

   The one-word "bucket" header precedes the block allocated for the
   client.  The block's address is a multiple of 2 * WORDSIZE, so the
   one-word header must lie on an odd word address. */

/* A SmallBlock is a small block, following the bucket.  If the block
   is allocated, these fields are overlaid with user data. */

typedef struct SmallBlock {
  struct SmallBlock *next, *previous;
} *SmallBlock;

#define BUCKET(ptr) (*(Bucket *)DEC((ptr), WORDSIZE))
#define BUCKETTOBLOCK(bucketptr) INC((bucketptr), WORDSIZE)

/* Each large node contains a one-word header: the size of the node in
   bytes, including header.

   The bytesize field is the same as for Cartesian nodes, allocated by
   cartesian_alloc and accepted by cartesian_free.  However, while a
   node is allocated, we store the negative of the size in this field.
   This restricts the largest large node to less than half the address
   space. */

typedef struct LargeNode {
  ssize_t bytesize;
} *LargeNode;

#define LARGENODETOBLOCK(large) INC(large, WORDSIZE)
#define BLOCKTOLARGENODE(block) ((LargeNode)DEC(block, WORDSIZE))

/* A "keep node" has a four-word header, and is allocated from the
   Cartesian heap.  When a keep block is freed, user data is not
   overwritten. */

typedef struct KeepNode {
  ssize_t bytesize;
  size_t requestedsize;
  Bool issmall;
  Bucket dummybucket;
} *KeepNode;

#define KEEPNODETOBLOCK(keep) INC(keep, 4 * WORDSIZE)
#define BLOCKTOKEEPNODE(block) ((KeepNode)DEC(block, 4 * WORDSIZE))

/* The freelists and the _bucket and _tobucket structures are combined
   into a giant top-level structure, to speed accesses.

   data.freelist contains a freelist pointer per bucket.

   data._bucket[i].bytesize is the size of nodes in bucket i, in bytes, and
   data._bucket[i].align is their necessary byte alignment.

   data._tobucket maps from a small allocation request (0 to
   640-WORDSIZE) to the appropriate bucket number.  Bucket 0 is always
   empty, so we map requests for 0 bytes there in order to postpone the
   test for that special case until the slow path.

   data.last is the most recent argument to free; it has not yet been freed,
   to cater to buggy client programs. */

#ifdef __alpha
static struct data {
  SmallBlock freelist[MAXBUCKET + 1];
  signed int _bytesize[MAXBUCKET + 1];
  signed int _align[MAXBUCKET + 1];
  unsigned char _tobucket[MAXSMALL - WORDSIZE + 1];
  void *last;
} data = {
  {NULL},
  {0, 24, 32, 40, 48, 64, 80, 96, 128, 160, 192, 256, 320, 384, 512, 640,
   768, 1024, 1280, 1536, 2048, 2560, 3072, 4096, 5120, 6144, 8192, 10240},
  {0,  8, 32,  8, 16, 64, 16, 32, 128,  32,  64, 256,  64, 128, 512, 128,
   256, 1024,  256,  512, 2048,  512, 1024, 4096, 1024, 2048, 8192, 2048},

   {0,
   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
   2, 2, 2, 2, 2, 2, 2, 2,
   4, 4, 4, 4, 4, 4, 4, 4,
   4, 4, 4, 4, 4, 4, 4, 4,
   5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
   6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
   7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
   8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
   8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
   9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
   9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
   10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
   10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
   11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
   11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
   11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
   11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
   12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
   12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
   12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
   12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
   13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
   13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
   13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
   13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
   14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
   14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
   14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
   14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
   14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
   14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
   14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
   14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
   15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
   15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
   15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
   15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
   15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
   15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
   15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
   15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15},
  NULL
};
#else
static struct data {
  SmallBlock freelist[MAXBUCKET + 1];
  size_t _bytesize[MAXBUCKET + 1];
  unsigned int _align[MAXBUCKET + 1];
  char _tobucket[MAXSMALL - WORDSIZE + 1];
  void *last;
} data = {
  {NULL},
  {0, 12, 16, 20, 24, 32, 40, 48, 64, 80, 96, 128, 160, 192, 256, 320,
   384, 512, 640, 768, 1024, 1280, 1536, 2048, 2560, 3072, 4096, 5120,
   6144, 8192, 10240},
  {0,  4, 16,  4,  8, 32,  8, 16, 64, 16, 32, 128,  32,  64, 256,  64,
   128, 512, 128, 256, 1024,  256,  512, 2048,  512, 1024, 4096, 1024,
   2048, 8192, 2048},
  {0,
   2, 2, 2, 2, 2, 2, 2, 2,
   2, 2, 2, 2,
   4, 4, 4, 4,
   4, 4, 4, 4,
   5, 5, 5, 5, 5, 5, 5, 5,
   6, 6, 6, 6, 6, 6, 6, 6,
   7, 7, 7, 7, 7, 7, 7, 7,
   8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
   9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
   10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
   11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
   11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
   12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
   12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
   13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
   13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
   14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
   14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
   14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
   14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
   15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
   15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
   15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
   15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
   16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
   16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
   16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
   16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
   18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
   18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
   18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
   18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
   18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
   18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
   18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18},
  NULL
};
#endif

/* Chunks are 8KB, 2KB, or 10KB, with the first block aligned on a
   CHUNKALIGN boundary. */

#define CHUNKALIGN 2048

/* Small nodes are allocated from 6KB, 2KB, or 10KB chunks, aligned on
   2KB boundaries.  We keep a hash table of chunk descriptors on the heap.
   */

typedef unsigned long HashKey;

#define MAKEKEY(address, bytes) ((ADDRESS)(address) | ((bytes) / CHUNKALIGN))
#define KEYADDRESS(key) (void*)((key) & ~(CHUNKALIGN - 1))
#define KEYBYTES(key) (((key) & (CHUNKALIGN - 1)) * CHUNKALIGN)

static HashKey *hash_table = NULL;    /* the hash table */
static unsigned long hash_n = 0;      /* current number of elements */
static unsigned long hash_N = 0;      /* number of slots */

/* The following data support the System V mallopt call. */

static struct {
  unsigned long maxfast;
  unsigned long numblks;
  unsigned long grain;
  Bool keep;
} sysv = {0, 100, 2 * WORDSIZE, FALSE};

/* statistical info, for System V support */

static struct {
  struct {
    unsigned long large;
    unsigned long poisoned;
    struct {
      unsigned long large;
      unsigned long small;
    } keep;
  } objects, bytes;
} info = {{0, 0, {0, 0}}, {0, 0, {0, 0}}};

/* An array mapping System V grains to current and maximum number of
   objects. */

static struct {
  unsigned long current;
  unsigned long max;
} *sysv_block_info = NULL;

/*
 * A flag so mallopt() can return an error if called after the
 * first small block has been allocated and mallopt was called to
 * use the small block algorithm.
 *
 * Keeping track of whether malloc() is called during the .init
 * phase is not directly necessary since the sysv.maxfast value
 * functions as a "better flag."  This is because mallopt() is the
 * only thing that sets sysv.maxfast, and this will also work
 * should an .init routine call mallopt().
 */
static int the_first_small_block_is_allocated_after_mxfast = FALSE;

int __crt0_doing_init;  /* should be removed in GOLD source pool
			   (only here to correct libc.so checksum) */

/* the machine's page size, initially unknown */

static size_t pagesize = 0;

/* function prototypes */

static void *expand_and_malloc(BucketI i, size_t bytes);
static void *keep_malloc(size_t bytes, unsigned long align);
static void *large_malloc(size_t bytes, unsigned long align);
static void large_free(void *block, Bucket i);
static void keep_free(void *block);
static void *bad_realloc(register void *old,
			 register size_t bytesize,
			 register size_t oldbytesize);
static Bool hash_insert(register HashKey key);
static void hash_delete(register HashKey key);
static void mallopt_keep();

/* malloc using binary buddy for small elements, falling back on
   Cartesian trees as necessary.

   This routine is on the critical path, and it has been highly
   optimized, sacrificing readability for speed.  Changing it risks
   making it slower. */

void *
malloc(register size_t bytes)
{
#ifdef __alpha
  register unsigned char *c = &(data._tobucket[bytes]);
#else
  register signed char *c = &(data._tobucket[bytes]);
#endif
  /*
   *  Set the small block allocated flag if mallopt(M_MXFAST, ...) was
   *  called, and this value is less than the fictitious small blocks.
   *
   *  (bit-wise 'or' for this case is cheaper than logical 'or').
   */
  the_first_small_block_is_allocated_after_mxfast |= (bytes && bytes <= sysv.maxfast);

  if (bytes <= MAXSMALL - WORDSIZE) {
    /* small node */
    register BucketI i = *c;
    register SmallBlock small = data.freelist[i];
    register SmallBlock next;

    if (small) {
      /* got a node */
      next = small->next;
      data.freelist[i] = next;
      if (next) next->previous = NULL;
      BUCKET(small) = i;
      return small;
    } else {
      if (i) {
	/* no node of correct size; split a larger node */
	register BucketI ii = i + 3;
	register size_t bytesize;
	while (TRUE) {
	  small = data.freelist[ii];
	  if (small) break;
	  ii += 3;
	  if (ii > MAXBUCKET) {
	    /* no larger node; add one of right size */
	    return expand_and_malloc(ii - 3, bytes);
	  }
	}
	/* pop the too-large node */
	next = small->next;
	data.freelist[ii] = next;
	if (next) next->previous = NULL;
	/* split the node into pieces */
	bytesize = data._bytesize[ii];
	do {
	  register SmallBlock buddy;
	  ii -= 3;
	  bytesize = bytesize / 2;
	  buddy = INC(small, bytesize);
	  BUCKET(buddy) = -ii;
	  buddy->next = NULL;
	  buddy->previous = NULL;
	  data.freelist[ii] = buddy;
	} while (ii > i);
	/* return the correctly-sized node */
	BUCKET(small) = i;
	return small;
      } else {
	/* malloc(0) */
	errno = EINVAL;
	return NULL;
      }
    }
  } else {
    return large_malloc(bytes, 2 * WORDSIZE);
  }
}

/* Allocate a large node (out of line), aligned on a multiple of
   "align" bytes, which must be a power of two. There's a one-word
   header, and the total length is forced to a multiple of 4 * WORDSIZE
   to slightly reduce fragmentation. */

static void *
large_malloc(register size_t bytes, unsigned long align)
{
  if (data.last) free(NULL);            /* free extra storage if possible */
  if (!sysv.keep) {
    register size_t totalbytes =
      (WORDSIZE + bytes + (4 * WORDSIZE - 1)) & ~(4 * WORDSIZE - 1);
    if (totalbytes > bytes) {
      register LargeNode large =
	cartesian_alloc(totalbytes, align, -WORDSIZE, FALSE);
      if (large) {
	register ssize_t bytesize = large->bytesize;
	if (bytesize < 0) {
	  /* too big */
	  cartesian_free(large);
	  return NULL;
	}
	large->bytesize = -bytesize;
	++info.objects.large;
	info.bytes.large += bytesize;
	return LARGENODETOBLOCK(large);
      } else {
	/* out of memory */
	return NULL;
      }
    } else {
      /* too big */
      return NULL;
    }
  } else {
    return keep_malloc(bytes, align);
  }
}

/* expand_and_malloc (out of line) expands the appropriate free list
   by one 6KB, 2KB, or 10KB chunk, then retries the allocation request.
   The freelist is empty upon entry. */

static void *
expand_and_malloc(register BucketI i, register size_t bytes)
{
  if (!sysv.keep) {
    register size_t chunkbytes;
    register void *block;
    while (data._align[i] > CHUNKALIGN) i -= 3;
    chunkbytes = data._bytesize[i];
    block = cartesian_alloc(chunkbytes, CHUNKALIGN, -WORDSIZE, TRUE);
    if (block) {
      register SmallBlock small = BUCKETTOBLOCK(block);
      /* format chunk as small node */
      if (hash_insert(MAKEKEY(small, chunkbytes))) {
	BUCKET(small) = -i;
	small->next = NULL;
	small->previous = NULL;
	data.freelist[i] = small;
	return malloc(bytes);
      } else {
	/* out of memory */
	cartesian_free(block);
	return NULL;
      }
    } else {
      /* out of space */
      return NULL;
    }
  } else {
  /* If we're in keep mode, we fall through to here because the
     freelists stay empty. */
    return keep_malloc(bytes, 2 * WORDSIZE);
  }
}

/* keep_malloc allocates a "keep" node */

static void *
keep_malloc(register size_t n, register unsigned long align)
{
  register size_t bytesize =
    (n + 4 * WORDSIZE + (WORDSIZE - 1)) & ~(WORDSIZE - 1);
  if (bytesize > n) {
    register KeepNode keep =
      cartesian_alloc(bytesize, align, -4 * WORDSIZE, FALSE);
    if (keep) {
      register ssize_t bytesize = keep->bytesize;
      if (bytesize < 0) {
	/* too big */
	cartesian_free(keep);
	return NULL;
      }
      keep->bytesize = -bytesize;
      keep->requestedsize = n;
      keep->dummybucket = 0;
      if (n < sysv.maxfast) {
	if (sysv_block_info == 0) {
	  register unsigned long max =
	    ((sysv.maxfast - 1 + sysv.grain - 1) / sysv.grain);
	  sysv_block_info =
	    cartesian_alloc((max + 1) * sizeof(sysv_block_info[0]),
			    2 * WORDSIZE, -WORDSIZE, FALSE);
	  if (sysv_block_info) {
	    register unsigned long i;
	    for (i = 1; i <= max; i++) {
	      sysv_block_info[i].current = 0;
	      sysv_block_info[i].max = 0;
	    }
	  } else {
	    /* out of memory */
	    keep->bytesize = bytesize;
	    cartesian_free(keep);
	    return NULL;
	  }
	}
	keep->issmall = TRUE;
	++info.objects.keep.small;
	info.bytes.keep.small += bytesize;
	{ register unsigned long i = (n + sysv.grain - 1) / sysv.grain;
	  if (++sysv_block_info[i].current > sysv_block_info[i].max) {
	    ++sysv_block_info[i].max;
	  }
	}
      } else {
	keep->issmall = FALSE;
	++info.objects.keep.large;
	info.bytes.keep.large += bytesize;
      }
      return KEEPNODETOBLOCK(keep);
    } else {
      /* out of memory */
      return NULL;
    }
  } else {
    /* too big */
    return NULL;
  }
}

/* valloc is like malloc, but page-aligns the returned object. */

#pragma weak valloc = __valloc

void *
valloc(register size_t bytes)
{
  if (pagesize == 0) pagesize = getpagesize();
  return large_malloc(bytes, pagesize);
}

/* free frees a node, corresponding to malloc; it delays freeing its
   most recent argument in case of a buggy realloc.

   This routine is on the critical path, and it has been highly
   optimized, sacrificing readability for speed.  Changing it risks
   making it slower. */

void
free(register void *block)
{
  register void *prev = data.last;
  if (prev) {
    register Bucket i = BUCKET(prev);
    data.last = block;
    if (i) {
	  /* this test will catch the case of a user trying to
       * free a buffer after it has been freed.
       */
      if((i < 0 && -i <= MAXBUCKET) ||
				(i > 0 && i > MAXBUCKET)) {
		return;
	  }
      if ((unsigned long)i <= MAXSMALLBUCKET) {
	/* small allocated node */
	register SmallBlock small = prev;
	register SmallBlock next = data.freelist[i];
	register size_t bytes = data._bytesize[i];
	register SmallBlock buddy = INC(small, bytes);
	register SmallBlock otherbuddy = DEC(small, bytes);
	if ((ADDRESS)small & data._align[i]) buddy = otherbuddy;
	/* change the sign of the node being freed, so that duplicate
	   frees can be detected. */
	BUCKET(small) = -i;
	if (BUCKET(buddy) != -i) {
	  /* fast path: can't coalesce buddy */
	  small->next = next;
	  small->previous = NULL;
	  if (next) next->previous = small;
	  data.freelist[i] = small;
	  return;
	} else {
	  register unsigned long align = data._align[i];
	  /* coalesce buddy; continue up sizes trying to coalesce more */
	  while (TRUE) {
	    register SmallBlock previous = buddy->previous;
	    next = buddy->next;
	    if (next) next->previous = previous;
	    if (previous) {
	      previous->next = next;
	    } else {
	      data.freelist[i] = next;
	    }
	    if (buddy < small) small = buddy;
	    i += 3;
	    bytes *= 2;
	    align *= 2;
	    if (align < CHUNKALIGN) {
	      buddy = INC(small, bytes);
	      otherbuddy = DEC(small, bytes);
	      if ((ADDRESS)small & align) buddy = otherbuddy;
	      if (BUCKET(buddy) == -i) continue;
	    } else {
	      if (data.freelist[i]) {
		/* keep one chunk free; return others to Cartesian heap */
		register LargeNode large = BLOCKTOLARGENODE(small);
		large->bytesize = bytes;
		cartesian_free(large);
		hash_delete(MAKEKEY(small, bytes));
		return;
	      }
	    }
	    /* new buddy isn't free; free this node */
	    BUCKET(small) = -i;
	    next = data.freelist[i];
	    small->next = next;
	    if (next) next->previous = small;
	    small->previous = NULL;
	    data.freelist[i] = small;
	    return;
	  }
	}
      } else {
	/* free the large block or other */
	large_free(prev, i);
      }
    } else {
      /* Free a "keep" node. */
      keep_free(prev);
    }
  } else {
    data.last = block;
  }
}

/* Free a large node (out of line) */

static void
large_free(register void *block, register Bucket i)
{
  register ssize_t bytesize = -i;

  if (bytesize > MAXSMALL + WORDSIZE) {
    /* free the large block */
    register LargeNode large = BLOCKTOLARGENODE(block);
    large->bytesize = bytesize;
    info.bytes.large -= bytesize;
    if (!sysv.keep) {
      --info.objects.large;
      cartesian_free(large);
	  if(bytesize > pagesize) {
        register size_t a =
				 		((size_t)large + pagesize - 1) & ~(pagesize - 1);

        bytesize = (bytesize - (a - (size_t)large)) & ~(pagesize - 1);
        if(bytesize > pagesize)
          madvise(a, bytesize, MADV_DONTNEED);
      }
    }
  } else if ((unsigned long)(-i) <= MAXBUCKET) {
    /* This is either a small free node, or a poisoned small node.
       In either case, we do nothing. */
    info.bytes.poisoned -= data._bytesize[-i];
  } else {
    /* client error: ignore */
  }
}

/* Free a keep node (out of line) */

static void
keep_free(register void *block)
{
  register KeepNode keep = BLOCKTOKEEPNODE(block);
  register ssize_t bytesize = -keep->bytesize;
  if (bytesize > 4 * WORDSIZE) {
    keep->bytesize = bytesize;
    if (keep->issmall) {
      --info.objects.keep.small;
      info.bytes.keep.small -= bytesize;
      { register unsigned long i =
	  (keep->requestedsize + sysv.grain - 1) / sysv.grain;
	--sysv_block_info[i].current;
      }
    } else {
      --info.objects.keep.large;
      info.bytes.keep.large -= bytesize;
    }
    cartesian_free(keep);
  } else {
    /* ignore attempt to free a free keep node, or a client error */
  }
}

/* A simple realloc.  For efficiency, objects that need to grow are
   grown by at least 25%.  Objects that can shrink are not copied unless
   they shrink by more than 50%. */

void *
realloc(register void *old, register size_t bytesize)
{
  if (old) {
    if (old == data.last) data.last = NULL;
    if (bytesize) {
      /* compute old size */
      register size_t oldbytesize;
      register SmallBlock small = old;
      register Bucket i = BUCKET(small);
      if (i) {
	if ((unsigned long)i <= MAXSMALLBUCKET) {
	  oldbytesize = data._bytesize[i] - WORDSIZE;
	} else if ((unsigned long)(-i) <= MAXBUCKET) {
	  /* This is either a small free node, or a poisoned small
	     node.  In case it's the latter, we proceed. */
	  return bad_realloc(old, bytesize, data._bytesize[-i] - WORDSIZE);
	} else {
	  /* large node */
	  register ssize_t bytes = -i;
	  if (bytes > MAXSMALL + WORDSIZE) {
	    oldbytesize = bytes - WORDSIZE;
	  } else {
	    /* ignore attempt to realloc a large free node */
	    errno = EINVAL;
	    return NULL;
	  }
	}
      } else {
	/* keep node */
	register KeepNode keep = BLOCKTOKEEPNODE(old);
	register ssize_t bytes = -keep->bytesize;
	if (bytes > 4 * WORDSIZE) {
	  oldbytesize = bytes - 4 * WORDSIZE;
	} else if (-bytes > 4 * WORDSIZE) {
	  /* freed keep node; proceed anyway */
	  return bad_realloc(old, bytesize, -bytes - 4 * WORDSIZE);
	} else {
	  /* ignore client error */
	  errno = EINVAL;
	  return NULL;
	}
      }
      if (bytesize > oldbytesize) {
	/* need to grow: try to grow by at least 25% */
	register size_t goodbytesize = oldbytesize + oldbytesize / 4;
	if (bytesize > goodbytesize) goodbytesize = bytesize;
	{ register void *new = malloc(goodbytesize);
	  if (new) {
	    (void)memcpy(new, old, oldbytesize);
	    free(old);
	    return new;
	  } else {
	    /* try to allocate just what was asked for */
	    new = malloc(bytesize);
	    if (new) {
	      (void)memcpy(new, old, oldbytesize);
	      free(old);
	      return new;
	    } else {
	      /* out of memory */
	      return NULL;
	    }
	  }
	}
      } else {
	/* can shrink, but do so only if shrink by 50% */
	if (bytesize < oldbytesize / 2) {
	  /* shrink */
	  register void *new = malloc(bytesize);
	  if (new) {
	    (void)memcpy(new, old, bytesize);
	    free(old);
	    return new;
	  } else {
	    /* out of memory; stay in place */
	    return old;
	  }
	} else {
	  /* stay in place */
	  return old;
	}
      }
    } else {
      /* realloc(old, 0) */
      free(old);
      return NULL;
    }
  } else {
    if (bytesize) {
      /* realloc(0, bytesize) */
      return malloc(bytesize);
    } else {
      /* realloc(0, 0) */
      return NULL;
    }
  }
}

/* bad_realloc does a realloc from a node that has already been freed
   but that has not been or may not have been overwritten. */

static void *
bad_realloc(register void *old,
	    register size_t bytesize,
	    register size_t oldbytesize)
{
  register void *new = malloc(bytesize);
  if (new) {
    (void)memcpy(new, old, bytesize < oldbytesize ? bytesize : oldbytesize);
    return new;
  } else {
    /* out of memory */
    return NULL;
  }
}

/* hash_insert inserts a chunk descriptor into the hash table */

static Bool
hash_insert(register HashKey key)
{
  if (2 * hash_n >= hash_N) {
    register unsigned long old_N = hash_N;
    register HashKey *old_table = hash_table;
    register unsigned long i;
    hash_N = 2 * hash_N;
    if (hash_N == 0) hash_N = 64;
    hash_table =
      cartesian_alloc(hash_N * sizeof(HashKey), 2 * WORDSIZE, -WORDSIZE, TRUE);
    if (!hash_table) {
      /* out of memory */
      hash_N = old_N;
      hash_table = old_table;
      return FALSE;
    }
    hash_n = 0;
    for (i = 0; i < hash_N; i++) hash_table[i] = 0;
    if (old_table) {
      for (i = 0; i < old_N; i++) {
	register HashKey oldkey = old_table[i];
	if (oldkey) (void)hash_insert(oldkey);
      }
      *(unsigned long *)old_table = old_N * sizeof(HashKey);
      cartesian_free(old_table);
    }
  }
  { register unsigned long i = key / CHUNKALIGN;
    register unsigned long n;
    i = (i ^ i / 4);
    n = i | 1;
    while (1) {
      register HashKey oldkey;
      i = i & (hash_N - 1);
      oldkey = hash_table[i];
      if (!oldkey) {
	hash_table[i] = key;
	++hash_n;
	return TRUE;
      }
      i += n;
    }
  }
}

/* hash_delete deletes a chunk descriptor from the hash table */

static void
hash_delete(register HashKey key)
{
  register unsigned long i = key / CHUNKALIGN;
  register unsigned long n;
  i = (i ^ i / 4);
  n = i | 1;

  while (1) {
    register HashKey oldkey;
    i = i & (hash_N - 1);
    oldkey = hash_table[i];
    if (oldkey == key) {
      hash_table[i] = 0;
      --hash_n;
      return;
    }
    i += n;
  }
}

/* mallopt implements the System V mallopt function. */

int
mallopt(register signed int command, register signed int value)
{
  if (sysv_block_info || the_first_small_block_is_allocated_after_mxfast) {
    errno = EINVAL;
    return -1;
  }
  switch (command) {
  case M_MXFAST:
    if (value > 0) {
      sysv.maxfast = value;
	} else if(value < 0){
      errno = EINVAL;
      return -1;
    } else {
      sysv.maxfast = 0;
    }
    if (value) mallopt_keep();
    return 0;
  case M_NLBLKS:
    if (value > 1) {
      sysv.numblks = value;
      return 0;
    } else {
      errno = EINVAL;
      return -1;
    }
  case M_GRAIN:
    if (value > 0) {
      sysv.grain = (value + 2 * WORDSIZE - 1) & ~(2 * WORDSIZE - 1);
      return 0;
    } else {
      errno = EINVAL;
      return -1;
    }
  case M_KEEP:
    mallopt_keep();
    return 0;
  default:
    errno = EINVAL;
    return -1;
  }
}

/* mallopt_keep turns on keep mode; in this implementation, this makes
   malloc and free take much more time and space. */

static void
mallopt_keep()
{
  if (sysv.keep) return;

  if (hash_table) {
    register unsigned long i;
    /* Walk through all allocated small nodes.  Return the free ones
       piecemeal to the Cartesian heap; poison the allocated ones. */
    for (i = 0; i < hash_N; i++) {
      register HashKey key = hash_table[i];
      if (key) {
	register SmallBlock small = KEYADDRESS(key);
	register SmallBlock end = INC(small, KEYBYTES(key));
	do {
	  register Bucket bucket = BUCKET(small);
	  register size_t bytesize;
	  if (bucket > 0) {
	    /* allocated; poison it */
	    bytesize = data._bytesize[bucket];
	    BUCKET(small) = -bucket;
	    ++info.objects.poisoned;
	    info.bytes.poisoned += bytesize;
	  } else {
	    /* free; return it to the Cartesian heap */
	    bytesize = data._bytesize[-bucket];
	    BUCKET(small) = bytesize;
	    cartesian_free(&BUCKET(small));
	  }
	  small = INC(small, bytesize);
	} while (small < end);
      }
    }
    /* free the hash table, no longer needed */
    *(unsigned long*)hash_table = hash_N * sizeof(HashKey);
    cartesian_free(hash_table);
    hash_n = 0;
    hash_N = 0;
    hash_table = NULL;
    /* clear the free lists; they will remain clear */
    for (i = 0; i < MAXBUCKET; i++) data.freelist[i] = NULL;
  }

  sysv.keep = TRUE;
}

struct mallinfo
mallinfo()
{
  register struct mallinfo mi;
  register struct cartesian_stats cartesian;

  cartesian = cartesian_stats();

  mi.arena = cartesian.totalbytes;

  mi.ordblks =
    cartesian.freeobjects +
      info.objects.large + info.objects.poisoned + info.objects.keep.large;
  mi.uordblks =
    info.bytes.large + info.bytes.poisoned + info.bytes.keep.large;

  /* and count all ordinary small nodes */
  { register unsigned long i;
    for (i = 0; i < hash_N; i++) {
      register HashKey key = hash_table[i];
      if (key) {
	register SmallBlock small = KEYADDRESS(key);
	register SmallBlock end = INC(small, KEYBYTES(key));
	do {
	  register Bucket bucket = BUCKET(small);
	  register size_t bytesize;
	  ++mi.ordblks;
	  if (bucket > 0) {
	    /* allocated */
	    bytesize = data._bytesize[bucket];
	    mi.uordblks += bytesize;
	  } else {
	    /* free */
	    bytesize = data._bytesize[-bucket];
	  }
	  small = INC(small, bytesize);
	} while (small < end);
      }
    }
  }

  mi.smblks = 0;
  mi.hblks = 0;
  mi.fsmblks = 0;
  if (sysv_block_info) {
    register unsigned long n =
      ((sysv.maxfast - 1 + sysv.grain - 1) / sysv.grain + 1);
    register unsigned long i;
    for (i = 1; i <= n; i++) {
      register unsigned long blks =
	(sysv_block_info[i].max + sysv.numblks - 1) / sysv.numblks;
      mi.smblks += blks * sysv.numblks;
      mi.hblks += blks;
      mi.fsmblks += blks * sysv.numblks * (i * sysv.grain + 4 * WORDSIZE);
    }
  }

  mi.hblkhd = 0;

  mi.usmblks = info.bytes.keep.small;

  mi.fsmblks -= info.bytes.keep.small;

  mi.fordblks = mi.arena - mi.usmblks - mi.fsmblks - mi.uordblks;

  if (sysv.keep) {
    mi.keepcost = ((4 - 1) * WORDSIZE) * (info.objects.keep.large + mi.smblks);
  } else {
    mi.keepcost = 0;
  }

  return mi;
}

/*
#define NOSHRINK
*/

/* cartesian.c implements variants of malloc and free using Cartesian
   trees to implement a variant of first-fit; this gives good space
   efficiency at a moderate cost in time.

   Each node contains a one-word header that holds the total number of
   bytes allocated, including the header.  The number of bytes requested
   includes this header; the pointer returned points to the header.

   (This is different from standard malloc, where the number of bytes
   does not include the header, and where the pointer returned follows
   the header; this different formulation simplified writing a
   higher-level allocator that uses these routines.)

   When a node is free, it contains an additional two pointers to
   children.  Therefore, allocated nodes must be at least 3 words (or
   they could not be freed).  All node addresses and node are a multiple
   of WORDSIZE. */

#define NODESIZE (3 * WORDSIZE)

/* A node. */

typedef struct Node {
  size_t bytesize;
  struct Node *left, *right;
} *Node;

#define Nil ((Node)0)

/* The root of the Cartesian tree, initially empty */

static Node root = Nil;

/* When the heap grows, it will do so by at least 50K, and at least
   10% */

#define MINGROW  24576
#define MINGROWFACTOR 0.00025

/* If the heap shrinks, it will do so by at least 64K, and at least
   10% */

#define MINSHRINK 16384
#define MINSHRINKFACTOR 0.001

/* If NOSHRINK is defined or noshrink becomes true, the heap will not
   shrink */

#ifndef NOSHRINK
static Bool noshrink = FALSE;
#endif

/* other operational information */

static struct {
  size_t size;                     /* bytes in the heap */
  void *end;                       /* end of heap */
  Bool atend;                      /* whether end == sbrk(0) */
  unsigned long freeobjects;       /* free objects */
} cartesian_info = {0, (void*)0, FALSE, 0};

/* function prototypes */

static void cartesian_insert(Node x, Node *r);
static Node cartesian_merge(Node a, Node b);
static void cartesian_growheap(size_t n);
static Bool cartesian_growheap2(size_t n);
static void cartesian_split(Node a, Node *l, Node *r, Node x);

extern void *sbrk(ssize_t incr);

/* DTM(p, modulus, residue) is the distance in bytes from the pointer
   p to an address that equals "residue" mod "modulus", which must be a
   power of two. */

#define DTM(p, modulus, residue) (((residue)-(ADDRESS)(p))&((modulus)-1))

/* FITS(node, bytes, modulus, residue, exact, excess) is true iff
   "node" can fit a block of size "bytes" at an address equal to
   "residue" mod "modulus"; as a side-effect, it sets the size_t lvalue
   "excess" to the number of bytes between the beginning of "node" and
   the block to be returned.  "residue" must be a multiple of WORDSIZE.
   "modulus" must be a power of two, and at least WORDSIZE.  If "exact"
   is true, the fit must be exact, with no bytes left at the end. */

#define FITS(node, bytes, modulus, residue, exact, excess) \
  (((excess) = DTM((node), (modulus), (residue))), \
   (0 < (excess) && (excess) < NODESIZE && \
    ((((excess) += (modulus)) < NODESIZE && \
      ((excess) += (modulus))))), \
   ((node)->bytesize >= (excess) && \
    ((exact) \
     ? ((node)->bytesize - (excess) == (bytes) || \
	(node)->bytesize - (excess) >= (bytes) + NODESIZE) \
     : ((node)->bytesize - (excess) >= (bytes)))))

/* cartesian_alloc(bytes, modulus, residue, exact) returns a node of
   size "bytes" whose address mod "modulus" is "residue".  "bytes" and
   "residue" must be a multiple of WORDSIZE, and "modulus" must be a
   power of two, and at least WORDSIZE.  If "exact" is true, the fit must
   be exact, with no bytes left at the end.  The actual number of bytes
   allocated is returned stored in the first word of the block. */

static void *
cartesian_alloc(register size_t bytes,
		register size_t modulus,
		register size_t residue,
		register Bool exact)
{
  register Node *r = &root;
  register Node node = *r;
  register size_t excess, e;
  register Node *lastr;

  /* adjust to multiple of word size */
  if (exact) {
    if (bytes < NODESIZE || (bytes & (WORDSIZE - 1)) != 0) {
      /* illegal request */
      return 0;
    }
  } else {
    if (bytes > NODESIZE) {
      bytes = (bytes + (WORDSIZE - 1)) & ~(WORDSIZE - 1);
      if (bytes) {
      } else {
	/* too big */
	return 0;
      }
    } else {
      bytes = NODESIZE;
    }
  }

  /* if not enough memory in top node, grow heap to be safe */
  if (node == Nil || !FITS(node, bytes, modulus, residue, exact, excess)) {
    cartesian_growheap(bytes + modulus - WORDSIZE);
    node = *r;
    if (node == Nil || !FITS(node, bytes, modulus, residue, exact, excess)) {
      /* probably can't grow heap */
      return 0;
    }
  }
  /* The property FITS is not monotonic at we descend the tree.  We
     therefore keep descending even if FITS becomes false, but remember the
     last time it was true. */
  lastr = r;
  while (TRUE) {
    register Node left = node->left;
    if (left != Nil && left->bytesize >= bytes) {
      r = &node->left;
      node = left;
      if (FITS(node, bytes, modulus, residue, exact, e)) {
	lastr = r;
	excess = e;
      }
    } else {
      /* found the node */
      register size_t size;
      r = lastr;
      node = *r;
      *r = cartesian_merge(node->left, node->right);
      --cartesian_info.freeobjects;
      size = node->bytesize;
      if (excess > 0) {
	/* reinsert the remainder on the left */
	register Node fragment = node;
	fragment->bytesize = excess;
	cartesian_insert(fragment, r);
	++cartesian_info.freeobjects;
	node = INC(node, excess);
	size -= excess;
      }
      if (size > bytes && size - bytes >= NODESIZE) {
	/* reinsert the remainder on the right */
	register Node fragment = INC(node, bytes);
	fragment->bytesize = size - bytes;
	cartesian_insert(fragment, r);
	++cartesian_info.freeobjects;
	size = bytes;
      }
      node->bytesize = size;
if (exact && size != bytes) abort();
      return node;
    }
  }
}

#undef DTM
#undef FITS

/* free using a Cartesian tree.  The first word of the block contains
   the block size in bytes. This code also handles the buggy case where
   free nodes overlap. */

static void
cartesian_free(register void *addr)
{
  register Node x = addr;
  register size_t xsize = x->bytesize;
  register Node x1 = INC(x, xsize);
  register Node *r = &root;
  register Node node = *r;

  /* look for adjacent free nodes to coalesce */
  while (TRUE) {
    register size_t nodesize;
    register Node node1;
    if (node == Nil) break;
    if (x1 < node) {
      r = &(node->left);
      node = *r;
    } else {
      nodesize = node->bytesize;
      node1 = INC(node, nodesize);
      if (node1 < x) {
	r = &(node->right);
	node = *r;
      } else {
	if (node < x) x = node;
	if (node1 > x1) x1 = node1;
	if (x >= x1) abort();
	x->bytesize = xsize = (ADDRESS)x1 - (ADDRESS)x;
	*r = node = cartesian_merge(node->left, node->right);
	--cartesian_info.freeobjects;
      }
    }
  }
#ifndef NOSHRINK
  /* maybe shrink heap */
  if (x1 == cartesian_info.end && cartesian_info.atend && !noshrink) {
    register unsigned long p = pagesize;
    if (p == 0) pagesize = p = getpagesize();
    { register unsigned long remainder = (-(ADDRESS)x) & (p - 1);
      if (0 < remainder && remainder < NODESIZE) remainder += p;
      if (x->bytesize > remainder) {
	register unsigned long excess = x->bytesize - remainder;
	if (excess >= MINSHRINK && excess >= MINSHRINKFACTOR * cartesian_info.size) {
	  if (sbrk(0) == cartesian_info.end) {
	    if ((ssize_t)excess < 0) {
	      do {
		excess -= (unsigned long)(-1) / 4 + 1;
		remainder += (unsigned long)(-1) / 4 + 1;
	      } while ((ssize_t)excess <= 0);
	    }
	    (void) sbrk(-(ssize_t)excess);
	    cartesian_info.end = DEC(cartesian_info.end, excess);
	    cartesian_info.size -= excess;
	    if (remainder) {
	      x->bytesize = remainder;
	      cartesian_insert(x, &root);
	      ++cartesian_info.freeobjects;
	    }
	    return;
	  } else {
	    cartesian_info.atend = FALSE;
	  }
	}
      }
    }
  }
#endif
  /* insert free node into tree */
  cartesian_insert(x, &root);
  ++cartesian_info.freeobjects;
}

/* GROWING THE HEAP */

/* Try to grow the heap to contain a free node of at least "n" bytes.
   Try to grow the heap by at least 64K, or 25% of its current size;
   retry if that fails. */

static void
cartesian_growheap(register size_t n)
{
  register size_t size = n;

  /* grow some miniumum amount */
  if (size < cartesian_info.size * MINGROWFACTOR) {
    size = cartesian_info.size * MINGROWFACTOR;
    if (size < n) size = n;
  }
  if (size < MINGROW) size = MINGROW;

  if (cartesian_growheap2(size)) return;

  /* can't grow heap that much: try to grow a little less */
  while (size > n) {
    size = (size + n) / 2;
    if (size < n) size = n;
    if (cartesian_growheap2(size)) return;
  }
  /* couldn't grow by as much as requested */
}

/* try to grow the heap by at least "n" WORDSIZE-aligned bytes */

static Bool
cartesian_growheap2(register size_t n)
{
  register char *mem;
  register size_t size;
  register Node node;

  /* adjust for possible poor alignment. */
  size = (n + 2 * (WORDSIZE - 1)) & ~(WORDSIZE - 1);
  if (size < n) return FALSE;

#ifdef __alpha
  /* adjust size upward to pagesize multiple */
  if (pagesize == 0) pagesize = getpagesize();
  size = (size + (pagesize - 1)) & ~(pagesize - 1);
  if (size < n) return FALSE;
#endif

  /* get the memory */
  mem = sbrk(size);
  if (mem == (char*)-1) {
    return FALSE;
  }

  if ((ADDRESS)mem % WORDSIZE != 0) {
    /* allocation not WORDSIZE-aligned; adjust */
    register unsigned long excess = (-(unsigned long)mem) % WORDSIZE;
    mem += excess;
    size -= excess;
    { register char *extra = sbrk(excess);
      if (extra == mem + size) {
	size += excess;
      } else {
	size -= WORDSIZE - excess;
      }
    }
  }

  /* construct free node; add to tree */
  node = (Node)mem;
  node->bytesize = size;
  cartesian_free(node);

  cartesian_info.size += size;
  cartesian_info.end = mem + size;
  cartesian_info.atend = TRUE;

  return TRUE;
}

static void
cartesian_noshrink()
{
#ifndef NOSHRINK
  noshrink = TRUE;
#endif
}

static struct cartesian_stats
cartesian_stats()
{
  register struct cartesian_stats stats;

  stats.totalbytes = cartesian_info.size;
  stats.freeobjects = cartesian_info.freeobjects;
  return stats;
}

/* CARTESIAN TREES */

/* cartesian_merge(a, b) merges the Cartesian trees a and b and
   returns the result.  All nodes in a must be to the left of all nodes
   in b.

   This code is extremely heavily expanded to avoid unnecessary
   operations.  The original version was:

    static Node
    cartesian_merge(a, b)
    Node a, b;
    {
      if (a == Nil) return b;
      if (b == Nil) return a;
      if (a->bytesize >= b->bytesize) {
        a->right = cartesian_merge(a->right, b);
        return a;
      } else {
        b->left = cartesian_merge(a, b->left);
        return b;
      }
    }
*/

static Node
cartesian_merge(register Node a, register Node b)
{
  if (a == Nil) return b;
  if (b == Nil) return a;
  { register size_t asize = a->bytesize;
    register size_t bsize = b->bytesize;
    register Node result, *r;
    if (asize >= bsize) {
      result = a;
      do {
        r = &(a->right);
        a = *r;
        if (a == Nil) {
          *r = b;
          return result;
        }
        asize = a->bytesize;
      } while (asize >= bsize);
      while (TRUE) {
        *r = b;
        do {
          /* *r == b */
          r = &(b->left);
          b = *r;
          if (b == Nil) {
            *r = a;
            return result;
          }
          bsize = b->bytesize;
        } while (asize < bsize);
        *r = a;
        do {
          /* *r == a */
          r = &(a->right);
          a = *r;
          if (a == Nil) {
            *r = b;
            return result;
          }
          asize = a->bytesize;
        } while (asize >= bsize);
      }
    } else {
      result = b;
      do {
        r = &(b->left);
        b = *r;
        if (b == Nil) {
          *r = a;
          return result;
        }
        bsize = b->bytesize;
      } while (asize < bsize);
      while (TRUE) {
        *r = a;
        do {
          /* *r == a */
          r = &(a->right);
          a = *r;
          if (a == Nil) {
            *r = b;
            return result;
          }
          asize = a->bytesize;
        } while (asize >= bsize);
        *r = b;
        do {
          /* *r == b */
          r = &(b->left);
          b = *r;
          if (b == Nil) {
            *r = a;
            return result;
          }
          bsize = b->bytesize;
        } while (asize < bsize);
      }
    }
  }
}

/* cartesian_insert(x, r) inserts the node x into the Cartesian tree
   *r.  x's left and right fields are ignored upon entry.  x must not
   already be in the Cartesian tree.

   The original version was:

    static Node
    cartesian_insert(x, a)
    Node x, a;
    {
      if (a == Nil) {
        x->left = Nil;
        x->right = Nil;
        return x;
      }
      if (x->bytesize > a->bytesize || (x->bytesize == a->bytesize && x < a)) {
        cartesian_split(a, &(x->left), &(x->right), x);
        return x;
      } else {
        if (x < a) {
          a->left = cartesian_insert(x, a->left);
        } else {
          a->right = cartesian_insert(x, a->right);
        }
        return a;
      }
    }

*/

static void
cartesian_insert(register Node x, register Node *r)
{
  register size_t xsize = x->bytesize;
  register Node a;
  while (TRUE) {
    a = *r;
    if (a) {
      register size_t asize = a->bytesize;
      if (x < a) {
	if (xsize >= asize) break;
	r = &(a->left);
      } else {
	if (xsize > asize) break;
	r = &(a->right);
      }
    } else {
      x->left = Nil;
      x->right = Nil;
      *r = x;
      return;
    }
  }
  *r = x;
  cartesian_split(a, &(x->left), &(x->right), x);
}

/* cartesian_split(a, l, r, x) splits the Cartesian tree a in two: the
   new tree containing all nodes to the left of node x is stored in *l,
   and the new tree of nodes at or to the right of x is stored in *r.  r
   must not point into the Cartesian tree.

   This code is very heavily expanded to avoid unnecessary operations.
   The original version was:

    static void
    cartesian_split(a, l, r, x)
    Node a, *l, *r, x;
    {
      if (a == Nil) {
        *l = Nil;
        *r = Nil;
      } else if (x < a) {
        *r = a;
        cartesian_split(a->left, l, &(a->left), x);
      } else {
        *l = a;
        cartesian_split(a->right, &(a->right), r, x);
      }
    }
*/

static void
cartesian_split(register Node a,
		register Node *l,
		register Node *r,
		register Node x)
{
  if (a == Nil) {
    *l = Nil;
    *r = Nil;
    return;
  }
  if (x < a) {
    while (TRUE) {
      *r = a;
      do {
        /* *r == a */
        r = &(a->left);
        a = *r;
        if (a == Nil) {
          *l = Nil;
          *r = Nil;
          return;
        }
      } while (x < a);
      *l = a;
      do {
        /* *l == a */
        l = &(a->right);
        a = *l;
        if (a == Nil) {
          *l = Nil;
          *r = Nil;
          return;
        }
      } while (x >= a);
    }
  } else {
    while (TRUE) {
      *l = a;
      do {
        /* *l == a */
        l = &(a->right);
        a = *l;
        if (a == Nil) {
          *l = Nil;
          *r = Nil;
          return;
        }
      } while (x >= a);
      *r = a;
      do {
        /* *r == a */
        r = &(a->left);
        a = *r;
        if (a == Nil) {
          *l = Nil;
          *r = Nil;
          return;
        }
      } while (x < a);
    }
  }
}
