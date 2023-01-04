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
static char *rcsid = "@(#)$RCSfile: newvram.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:41:14 $";
#endif
/*
 under RCS

 $Author: Robert_Lembree $
 $Date: 1993/11/19 21:41:14 $, $Revision: 1.1.2.2 $
 $Header: /usr/sde/osf1/rcs/x11/src/server/ddx/dec/ffb/model/newvram.c,v 1.1.2.2 1993/11/19 21:41:14 Robert_Lembree Exp $
 $Locker:  $
 $Log: newvram.c,v $
 * Revision 1.1.2.2  1993/11/19  21:41:14  Robert_Lembree
 * 	SFB+ Initial version
 * 	[1993/11/19  16:34:14  Robert_Lembree]
 *
 * Revision 1.1.1.2  1993/11/19  16:34:14  Robert_Lembree
 * 	SFB+ Initial version
 *
 * Revision 2.33  1993/06/08  15:18:38  rsm
 * don't print out gobs of useless data when MAXCELLS is exceeded
 * if lastDinotTick has been set, don't exit on a compare failure.
 *
 * Revision 2.32  1993/06/07  16:45:39  rsm
 * modified address out of range messages so they get disabled after
 * MAXWARN messages are printed.
 *
 * Revision 2.31  1993/06/03  14:59:54  rsm
 * added an ExitSfb(0) after printing address out of range message
 *
 * Revision 2.30  1993/05/28  16:25:46  rsm
 * limit number of entries that get printed out from lists
 *
 * Revision 2.29  1993/05/27  15:46:19  pcharles
 * made 32-bit block byte-lane checking (formerly PLANES32) the default
 * must #define PLANES8 to get 8-bit checking.
 *
 * Revision 2.28  1993/05/26  20:42:04  rsm
 * change to prevent out of range addresses from writing off the
 * end of the vram array.
 *
 * Revision 2.27  1993/05/25  18:54:37  rsm
 * modified InitVRAM to initialize with a different pseudo-random
 *
 * Revision 2.26  1993/05/25  18:19:03  rsm
 * use simulated memory more efficiently for 8-plane.
 * make parameter to InitVRAM specify amount of memory to use.
 *
 * Revision 2.25  1993/05/24  13:32:55  chris
 * do not abort on out-of-range addresses
 *
 * Revision 2.24  1993/05/21  16:10:50  rsm
 * changes for better reporting with truncated vram files
 *
 * Revision 2.23  1993/05/17  18:33:12  rsm
 * added heuristics for detecting truncated vram files
 *
 * Revision 2.22  1993/05/15  16:46:40  pcharles
 * DoCompareByte() - added loop to scan writeList[] entries whenever
 * match fails, and exit if all entries are NULL;
 * (prevents truncated or nonexistent vram file compares from
 *  continuing indefinitely)
 *
 * Revision 2.21  1993/05/13  19:22:33  rsm
 * changed Paint() to use an externally defined variable initialized to RAM_WIDTH rather than RAM_WIDTH directly
 *
 * Revision 2.20  1993/05/12  16:09:33  chris
 * Added fifo for dma entries so the search for fb writes would not be stopped
 *
 * Revision 2.19  1993/05/11  15:50:45  rsm
 * changed MAXCELLS to be 128*NLISTS (was being exceeded in 32 plane case)
 *
 * Revision 2.18  1993/05/06  20:29:22  rsm
 * changed MINHARD to be 2*NLISTS*8 and NREAD to be MINHARD/4
 *
 * Revision 2.17  1993/05/05  21:38:47  rsm
 * changed MINHARD to 32
 *
 * Revision 2.16  1993/05/05  17:22:12  rsm
 * update lastMatchLine *every* time a match is detected
 *
 * Revision 2.15  1993/05/04  14:14:38  rsm
 * modified data comparison to AND against the planemask
 * fixed reporting of mismatches to index properly into writeList based on NLISTS
 *
 * Revision 2.14  1993/04/30  14:23:57  rsm
 * changed MINHARD from 8 to 16
 *
 * Revision 2.13  1993/04/15  19:15:03  rsm
 * fixed bug which caused too many cells to be allocated
 * conditionally extended number of lists to 32 for 32-plane testing
 *
 * Revision 2.12  1993/03/30  18:43:52  rsm
 * changed some printf's to fprintf's (to stderr)
 *
 * Revision 2.11  1993/03/29  14:21:43  pcharles
 * moved WRLIST struct to top for function MarkSoft()
 * prototype
 *
 * Revision 2.10  1993/03/26  19:16:32  chris
 * added prototypes for FindMatch and MarkSoft
 *
 * Revision 2.9  1993/03/25  23:51:18  rsm
 * added a limit on number of cells that can be allocated for holding
 * unmatched entries in the vram log file
 *
 * Revision 2.8  1993/03/17  00:17:27  rsm
 * init all of memory to pseudo-random pattern
 *
 * Revision 2.7  1993/03/15  22:43:32  rsm
 * moved computation of x, y into Paint()
 * print warning but don't quit when a script terminates with a
 * premature EOF on the vram file.
 *
 * Revision 2.6  1993/03/09  23:20:57  rsm
 * change to initialization of memory, so 1280x1024 can be modeled backward
 * compatible with 1024x864.
 *
 * Revision 2.5  1993/03/09  18:25:54  rsm
 * 32 plane changes
 *
 * Revision 2.4  1993/03/03  20:55:59  rsm
 * print most recently match vramLine on error
 *
 * Revision 2.3  1993/02/25  19:39:39  rsm
 * changed address mask after working out the addressing with Chris
 *
 * Revision 2.2  1993/02/22  21:24:06  rsm
 * changes to support dma checking in scripts
 *
 * Revision 2.1  1993/02/22  15:17:52  rsm
 * modified to unget any lines with dma read/write data
 *
 * Revision 2.0  1993/02/15  20:11:32  rsm
 * rev 2.0
 *
 * Revision 1.3  1993/02/12  17:38:13  rsm
 * fixed byte address in error reporting
 *
 * Revision 1.2  1993/02/10  19:28:13  rsm
 * writes need to be done immediately, else a read/mod/write will fail
 *
 * Revision 1.1  1993/02/06  15:15:31  rsm
 * Initial revision
 *
 * Revision 1.11  1993/02/01  15:50:04  rsm
 * modified planemask code for 8 plane
 *
 * Revision 1.10  1993/01/21  14:40:07  rsm
 * fixed planemasking logic
 *
 * Revision 1.9  1992/12/03  20:35:48  rsm
 * moved around data storage definitions again
 *
 * Revision 1.8  1992/12/01  16:57:22  rsm
 * moved storage definition for some global variables to simplify
 * building an X-server with our model.
 *
 * Revision 1.7  1992/11/16  19:05:22  pcharles
 * Moved dynamic comparison routines external (diffram.c)
 * and added -silent option for use with said routines.
 *
 * Revision 1.6  1992/11/16  13:50:17  rsm
 * changes for 32bpp
 *
 * Revision 1.5  1992/11/13  02:31:07  rsm
 * changes for generating traces
 *
 * Revision 1.3  1992/11/09  19:04:30  pcharles
 * fixed malloc/calloc typecast.
 *
 * Revision 1.2  1992/10/27  15:12:53  pcharles
 * Added support for -nolog, -difflog, -dynamic vram comparison,
 * DoCompare(), options... funcs Reconcile() and DeleteMismatch().
 *

*/

#include <stdio.h>

#include "defs.h"
#include "types.h"
#include "vars.h"

#include "vram.h"

extern FILE *vid;
extern int  ticks;
extern int  cmdLine;
extern int  vramLine;

FILE *vramOut;
FILE *vramFile;

#define MAXWARN		4	/* maximum number of warnings to print */
#define TRUNCATEDMAX	1000	/* number of entries to print past vramFile eof before giving up */
int truncated = 0;	/* set when truncated vram file is detected */
int noHaltOnError = 0;	/*from gram.y   don't halt on static miscompare*/
int vramCompareFlag = 0;
int silent = 0;		/*               silent sfbtgen/sfb+ mem compare*/

PIXELS *vram;

VramPointType pointBuf[MAXPOINTS];
int    npoints;

typedef struct WRLIST {
  unsigned addr;
  unsigned data;
  unsigned planemask;
  unsigned soft;
  int vramLine;
  struct WRLIST *next;
} WRLIST;

#define MAXDMA   16 /* max DMA entries to read ahead */

typedef struct DMLIST {
  int  head;
  int  tail;
  char *ptr[MAXDMA];
} DMLIST;

static void DoCompare(unsigned addr, 
		      unsigned dataHi,
		      unsigned dataLo,
		      unsigned planemask,
		      unsigned bytemask);

static int FindMatch(unsigned addr,
		     unsigned data,
		     unsigned planemask,
		     unsigned bytePosition);

static int MarkSoft(unsigned addr,
		    unsigned data,
		    unsigned planemask,
		    WRLIST *p);

static int ramTotal;
/*
 * 
 */
ULONG64
do_rams(
	unsigned addr,
	ULONG64 data,
	unsigned planeMask,
	unsigned byteMask,
	unsigned we_L,
	unsigned depth32
	)
{
  static int outOfRange = 0;
  int i, offset;
  unsigned pixel;
  int mask;
  PIXELS d, m;
  int numPixels;	/* number of pixels to operate on in a loop */

  /*
   * addr is a 64-bit word address.
   * mask the unused upper bits.
   */
  if (depth32) {
    /* 32 bits per pixel */
    addr &= 0x1fffff;	/* 21-bits: 2M * 8 bytes = 16MB (4M pixels) */
    numPixels = 2;
  }
  else {
    /* 8 bits per pixel */
    addr &= 0xfffff;	/* 20-bits: 1M * 8 bytes = 8MB (8M pixels) */
    numPixels = 8;
  }

  /* check for address out of range */
  if (addr >= ramTotal/8) {
    if (outOfRange < MAXWARN) {
      fprintf(stderr, "do_rams: address out of range = %-.8x (%x)\n", addr, addr<<3);
      fprintf(stderr, "cmdLine = %d, vramLine = %d\n", cmdLine, vramLine);

      if (outOfRange++ == MAXWARN) {
	fprintf(stderr, "address out of range warnings disabled\n");
      }
    }
    addr = addr % (ramTotal/8);
  }

  /* make out of range addresses wrap around */
  offset = addr % (ramTotal/8);

  if (we_L == 0 && byteMask != 0) {   /** asserted low, so write it **/
    extern FILE *vramOut;
    extern int logFlag;

    if (logFlag)
      fprintf (vramOut, "%x %x %x %x %x\n", addr, data.lo, data.hi, planeMask, byteMask);
/*
    printf ("*** do_rams: a=%x d=%x.%x pm=%x bm=%x\n",
	    addr, data.hi, data.lo, planeMask, byteMask);
*/

    if (noHaltOnError)
      printf ("%x %x %x %x %x\n",addr,data.lo,data.hi,planeMask,byteMask);

    /*
     * compare to log file if comparing enabled.
     */
    if (vramCompareFlag)
      DoCompare(addr, data.hi, data.lo, planeMask, byteMask);

    /* merge planeMask and byteMask into one value */
    m.l.lo = m.l.hi = planeMask;
    for (i=0; i<8; ++i)
      m.p8[i] &= (byteMask & (1 << i) ? 0xff : 0);

    d.l = data;

    for (i=0; i<numPixels; ++i) {
      if (depth32) {
	pixel = d.p32[i];
	mask = m.p32[i];

	if (mask) {
	  /*printf ("*** do_rams: write a=%x d=%x pm=%x\n",
	    offset*numPixels+i, pixel, mask);*/
	  vram[offset].p32[i] &= (~mask);
	  vram[offset].p32[i] |= (pixel & mask);
	  pixel = vram[offset].p32[i];

	  if (vid) Paint (offset * numPixels + i, pixel);
	}
      } else {
	pixel = d.p8[i];
	mask = m.p8[i] | (m.p8[i] ? 0xffffff00 : 0);

	if (mask) {
	  /*printf ("*** do_rams: write a=%x d=%x pm=%x\n",
	    offset*numPixels+i, pixel, mask);*/
	  vram[offset].p8[i] &= (~mask);
	  vram[offset].p8[i] |= (pixel & mask);
	  pixel = vram[offset].p8[i];
	  
	  if (vid) Paint (offset * numPixels + i, pixel);
	}
      }
    }
  }

  return vram[offset].l;
}
 
SetScale(int zoom)
{
  if (vid == NULL) OpenVid();
  if (vid) fprintf( vid, "s %d\n", zoom);
}


/*
 * initialize the simulated VRAM with a pseudo-random pattern.
 *
 * there's an awful kludge here to get around the fact that
 * some of our traces were made with one sequence of initialization
 * and others were made with another sequence.  we differentiate
 * the two cases by whether or not InitVRAM has already been
 * called once (and hence, vram is non-nil).
 *
 * blecchhh!
 */
InitVRAM(int totalBytes)
{
  int i, j;
  extern char *malloc();
  int new = 0;

  /*
   * free the memory if it's already been malloc'd
   */
  if (vram) {
    free (vram);
    ++new;
  }

  /*
   * We use malloc here to get storage for vram because the c89 compiler
   * wouldn't generate a bss section, resulting in huge executable images.
   * How stupid...
   */
  vram = (PIXELS *) malloc( totalBytes );
  if (vram == NULL) {
    fprintf( stderr, "PANIC! InitVRAM.malloc returned NULL!\n" );
    exit( 1 );
  }
  ramTotal = totalBytes;

  /*
   * initialize memory to a pseudo-random pattern.
   */
  srandom (1);

  if (new) {
    for (i=0; i<totalBytes/sizeof(PIXELS); ++i)
      for (j=0; j<2; ++j)
	vram[i].p32[j] = random();
  } else {
    for (i=0; i<totalBytes/sizeof(PIXELS); ++i)
      for (j=0; j<8; ++j)
	vram[i].p8[j] = random();
  }
}

/*
 * count the number of bits set.
 */
BitsSet (unsigned n)
{
  unsigned i;
  int count = 0;

  for (i=0x80000000; i!=0; i>>=1)
    if (n & i) ++count;

  return (count);
}


/************************/
/* general malloc/check */
/************************/
void *genalloc(long nsize, long nelems, char *abort_string, int flag)
{
void *memptr;
  if(flag==0)
    if((memptr=(void *)malloc(nsize*nelems))==NULL) {
      fprintf(stderr,"Error on memory allocation on ");
      fprintf(stderr,"%s.",abort_string); exit(1); }
  else 
    if((memptr=(void *)calloc(nsize,nelems))==NULL) {
      fprintf(stderr,"Error on clear memory allocation on ");
      fprintf(stderr,"%s.",abort_string); exit(1); }
  return(memptr); 
}

#ifdef PLANES8
#define NLISTS 8
#else
#define NLISTS 32
#endif

WRLIST  *writeList [NLISTS];
WRLIST  *freeList;
DMLIST  dmaList = {0,0, {NULL}};

int ncells;
int lastMatchLine;

#define MAXCELLS 128*NLISTS	/* maximum number of cells we'll
			   	allocate before PANIC'ing */
int howMany;

WRLIST *
NewWrList ()
{
  WRLIST *p;

  if (freeList) {
    /* remove an entry from the free list, then pass it back. */
    p = freeList;
    freeList = p->next;
  } else {
    /* allocate storage for a new entry, then pass it back. */
    ++ncells;	/* count how many we allocate */
    if (ncells > MAXCELLS) {
      /* something must be wrong...give up now */
      extern int nhard;
      int i;
      fprintf (stderr, "PANIC! MAXCELLS exceeded in NewWrList\n");
      fprintf (stderr, "cmdLine = %d, near vramLine = %d\n",
	       cmdLine, lastMatchLine);
      fprintf (stderr, "**** clock tick %d\n", ticks);
      fprintf (stderr, "     howMany = %d, nhard = %d\n", howMany, nhard);
      for (i=0; i<NLISTS; ++i)
	(void) PrintList(i, 2);
      exit (1);
    }
    p = (WRLIST *) malloc(sizeof(WRLIST));
    if (p == NULL) {
      fprintf(stderr, "PANIC! malloc returned NULL in NewWrList\n");
      exit (1);
    }
  }

  return (p);
}

int ungotFlag;
/*
 * get the next line from the file of vram trace data.
 */
char *GetVramLine ()
{
#define NBUF 512
  static char    vrambuf[NBUF];

  if (ungotFlag) {
    ungotFlag = 0;
    return vrambuf;
  }
  else if (fgets(vrambuf, NBUF, vramFile)) {
    ++vramLine;
    return vrambuf;
  }

  return NULL;
}

/*
 * get the next line of dma commands from the file of vram trace data.
 */
char *GetDmaLine ()
{
  char *p;
/*
 * Return any entries that may have been queued up before reading more
 */
  while ((dmaList.head == dmaList.tail) && (ReadMore() != 0)) ;

  if (dmaList.head == dmaList.tail)
    p = NULL;
  else {
    p = dmaList.ptr[dmaList.head];
    dmaList.head = (dmaList.head+1)%MAXDMA;
  }

  return p;
}

#define MINHARD	(2*NLISTS*8)	/* minimum number of hard entries to keep in lists */
#define NREAD	MINHARD/4	/* number of entries to read from vram log file */
int nhard;

int
ReadMore()
{
  unsigned a, dlo, dhi, plmsk, cas;
  int i, j, nlines;
  PIXELS d, m;
  WRLIST *tail [NLISTS];
  char *p;


  /* find the tail of each list. */
  for (i=0; i<NLISTS; ++i) {
    tail [i] = writeList [i];
    if (tail [i]) {
      while (tail [i]->next)
	tail [i] = tail [i]->next;
    }
  }

  /* read a few lines and parcel the data into the lists for
     each of the NLISTS byte positions. */
  nlines = 0;
  for (i=0; i<NREAD; ++i) {
    do {
      if ((p = GetVramLine ()) == NULL)
	return nlines;

      if ((p[0] == 'w') || (p[0] == 'r')) { /* it's a dma read or write comm */
	if ((dmaList.tail+1)%MAXDMA == dmaList.head) {
	  ungotFlag = 1;
	  return nlines;
	}

	if (dmaList.ptr[dmaList.tail] != NULL)
	  free ((void *) dmaList.ptr[dmaList.tail]);

	dmaList.ptr[dmaList.tail] = (char *) malloc(strlen(p)+1);
	strcpy(dmaList.ptr[dmaList.tail], p);
	dmaList.tail = (dmaList.tail+1)%MAXDMA;

	cas = 1;
      }

      else if (sscanf(p, "%x %x %x %x %x", &a, &dlo, &dhi, &plmsk, &cas) != 5) {
	if (p [strlen (p) - 1] != '\n') {
	  if (feof (vramFile)) {
	    fprintf (stderr, "**** vram file appears truncated: '%s'.  Continuing...\n", p);
	    ++truncated;
	  } else {
	    fprintf (stderr, "**** vram file appears corrupted: '%s'.  Exiting...\n", p);
	    exit (1);
	  }
	} else {
	  fprintf (stderr, "can't parse vram data '%s'\n", p);
	  fprintf (stderr, "**** cmdLine = %d, vramLine = %d\n",
		   cmdLine, vramLine);
	  exit (1);
	}
      }
    } while (cas == 0);
    
    if ((p[0] != 'w') && (p[0] != 'r')) {
      d.l.lo = dlo;
      d.l.hi = dhi;
      m.l.lo = m.l.hi = plmsk;
      /* pick off the data for each byte which is being written. */
      for (j=0; j<8; ++j) {
	if (cas & (1 << j)) {
	  int index = j;
	  
	if (NLISTS == 32)
	  index += 8*(a%4);
	  
	  /* place this entry onto the tail of the list for this byte */
	  if (tail [index] == NULL) {
	    tail [index] = NewWrList();
	    writeList [index] = tail [index];
	  } else {
	    /* get a new entry and tack it onto the tail */
	    tail [index]->next = NewWrList();
	    tail [index] = tail [index]->next;
	  }
	  
	  /* copy the data into this new entry */
	  tail [index]->addr = a;
	  tail [index]->data = d.p8[j];
	  tail [index]->planemask = m.p8[j];
	  tail [index]->soft = 0;
	  tail [index]->vramLine = vramLine;
	  tail [index]->next = NULL;
	  ++nhard;
	}
      }
    }
    ++nlines;    
  }
  return (1);
}

PrintList(unsigned i, int maxHard)
{
  WRLIST *p = writeList [i];
  int n = 0;

  fprintf (stderr, "**** list for byte position %d\n", i);
  if (p==NULL) fprintf (stderr, "     (nil)\n");
  while (p) {
    ++n;
    fprintf (stderr, "     vramLine=%d a=%-.5x d=%-.2x m=%-.2x %s\n",
	    p->vramLine, p->addr, p->data, p->planemask, p->soft ? "soft" : "hard");
    if (p->soft == 0 && --maxHard == 0)
      /* we've printed enough lines; quit now */
      break;
    p = p->next;
  }

  return n;
}

static void DoCompareByte(unsigned addr,
			  unsigned data,
			  unsigned planemask,
			  unsigned bytePosition)
{
  WRLIST *p, *pnext;
  int i;

  /*
   * step thru list of writes
   *	if entry doesn't match and entry is a "hard" write
   *		it's an error (lost or incorrect write)
   *
   *	if match found
   *		remove earliest match,
   *		any earlier writes to same address,
   *		and any earlier "soft" writes.
   *
   *		if any earlier "hard" writes are still present,
   *		it's an error (lost write).
   *
   *		if any later exact matches exist,
   *		mark them and any preceding writes
   *		to the same address as "soft".
   *
   *	else (if match not found)
   *		it's an error (lost or incorrect write)
   */

  if (FindMatch(addr, data, planemask, bytePosition)) {
    /* step thru list, remove first match, any earlier writes
       to the same address, and any earlier "soft" writes. */
    int index = bytePosition;

    if (NLISTS == 32)
      index += 8*(addr%4);

    p = writeList [index];
    while (p) {
      /* remove this entry by placing it on the freeList */
      if (p->soft == 0) --nhard;
      pnext = p->next;
      p->next = freeList;
      freeList = p;
      /* this entry must either be a "soft" write,
	 a write to the same address,
	 or an exact match. */
      if (p->addr == addr && ((p->data ^ data) & planemask) == 0 && p->planemask == planemask) {
	/* it's an exact match...we're done here. */
	if (p->vramLine > lastMatchLine)
	  lastMatchLine = p->vramLine;
	break;
      } else if (p->addr != addr)
	/* must be a "soft" write. */
	if (p->soft == 0) {
	  fprintf(stderr, "PANIC! Consistency error in DoCompareByte\n");
	  exit(1);
	}
      p = pnext;
    }
    /* the writeList now begins after all the removed entries */
    writeList [index] = pnext;

    /* if any later exact matches exist,
       mark them and any preceding writes
       to the same address as "soft". */
    /* first, make sure there are enough entries for us to scan */
    while (nhard < MINHARD) {
      howMany = nhard;
      if (! ReadMore()) break;
    }

    (void) MarkSoft (addr, data, planemask, writeList [index]);
  } else if (truncated) {
    unsigned index = bytePosition % 4;

    if (truncated < TRUNCATEDMAX) {
      ++truncated;
      fprintf (stderr, "%x %x %x %x %x\n",
	       addr,
	       bytePosition < 4 ? data << (8*index) : 0,
	       bytePosition < 4 ? 0 : data << (8*index),
	       planemask << (8*index),
	       1 << bytePosition);
    } else {
      fprintf (stderr, "**** PANIC!  too many entries past end-of-file\n");
      exit (1);
    }
  } else {
    extern int ticks, lastDinoTick;
    unsigned index = bytePosition % 4;
    char *zeros = (bytePosition >= 4 ? ".00000000" : "");
    int emptyFlag = TRUE;

    /* it's an error.  give up now or we'll get hopelessly lost */
    fprintf (stderr, "**** incorrect writedata: ");
    fprintf (stderr, "cmdLine = %d, near vramLine = %d\n",
	     cmdLine, lastMatchLine);
    fprintf (stderr, "**** clock tick %d\n", ticks);

    for (i=0; i<NLISTS; ++i) 
      if (writeList [i]) emptyFlag = FALSE;
    if (emptyFlag) {
      fprintf (stderr, "**** All lists empty: ");
      if (feof (vramFile)) {
	fprintf (stderr, "vram file appears truncated: EOF. Continuing...\n");
	++truncated;
	fprintf (stderr, "%x %x %x %x %x\n",
	     addr,
	     bytePosition < 4 ? data << (8*index) : 0,
	     bytePosition < 4 ? 0 : data << (8*index),
	     planemask << (8*index),
	     1 << bytePosition);
	return;
      } else {
	fprintf (stderr, "eof on vram file has not occurred. Exiting...\n");
	exit(1);
      }
    }
    if (NLISTS == 32)
      p = writeList [8*(addr%4)+bytePosition];
    else
      p = writeList [bytePosition];

    while (p && p->soft) {
      fprintf (stderr, "        maybe %x %x%s %x (%x)\n",
	       p->addr, 
	       p->data << (8*index),
	       zeros,
	       p->planemask << (8*index),
	       (p->addr << 3) + bytePosition);
      p = p->next;
    }
    if (p && !p->soft) {
      fprintf (stderr, "     expected %x %x%s %x (%x)\n",
	       p->addr,
	       p->data << (8*index),
	       zeros,
	       p->planemask << (8*index),
	       (p->addr << 3) + bytePosition);
    }
    fprintf (stderr, "     received %x %x%s %x (%x)\n",
	     addr,
	     data << (8*index),
	     zeros,
	     planemask << (8*index),
	     (addr << 3) + bytePosition);

    if (lastDinoTick < 10000000) {
      /* keep simulating until the specified 
	 clock tick, but stop the comparing. */
      vramCompareFlag = 0;
    } else {
      int i;
      int unmatched = 0;
      
      for (i=0; i<NLISTS; ++i)
	unmatched += PrintList(i, 2);
      fprintf(stderr, "\n");
      
      if (unmatched != 0)
	exit (1);
    }
  }
}


/*
 * MarkSoft recursively looks for the last exact match in the list,
 * marking it and any preceding writes to the same address as "soft".
 */
static int MarkSoft(unsigned addr,
		    unsigned data,
		    unsigned planemask,
		    WRLIST *p)
{
  int soft = 0;

  while (p) {
    if (p->addr == addr && ((p->data ^ data) & planemask) == 0 && p->planemask == planemask) {
      /* an exact match. mark it soft. */
      if (p->vramLine > lastMatchLine)
	lastMatchLine = p->vramLine;
      soft = 1;
      if (p->soft == 0) --nhard;
      p->soft = 1;
    } else if (p->addr == addr) {
      /* not an exact match, but it's the same address.
	 mark it soft if there is a later exact match.
	 since calling ourselves recursively will have
	 checked all the remaining entries, we need to
	 return in either case  */
      if (MarkSoft(addr, data, planemask, p->next)) {
	if (p->soft == 0) --nhard;
	p->soft = 1;
	return (1);
      }
      return (soft);
    } else if (! p->soft)
      /* this is a hard write to a different address.
	 don't mark any further entries. */
      return (soft);
    p = p->next;
  }
  return (soft);
}


static int FindMatch(unsigned addr,
		     unsigned data,
		     unsigned planemask,
		     unsigned bytePosition)
{
  WRLIST *p;
  int index = bytePosition;

  if (NLISTS == 32)
    index += 8*(addr%4);

  p = writeList [index];

  while (p) {
    if (p->addr == addr && ((p->data ^ data) & planemask) == 0 && p->planemask == planemask) {
      /* an exact match. we found it. */
      if (p->vramLine > lastMatchLine)
	lastMatchLine = p->vramLine;
      return (1);
    } else if (p->addr != addr && !p->soft) {
      /* a hard write, but the address doesn't match */
      return (0);
    }
    p = p->next;
  }
  /* hmmm, we'll have to read more data and try again */
  if (ReadMore())
    return (FindMatch (addr, data, planemask, bytePosition));
  else
    /* no more data to read.  just return with no match found. */
    return (0);
}

static void DoCompare(unsigned addr,
		      unsigned dataHi,
		      unsigned dataLo,
		      unsigned planemask,
		      unsigned bytemask)
{
  int i;
  PIXELS d, m;

  d.l.lo = dataLo;
  d.l.hi = dataHi;
  m.l.lo = m.l.hi = planemask;

  for (i=0; i<8; ++i) {
    if (bytemask & (1 << i))
      DoCompareByte(addr, d.p8 [i], m.p8 [i], i);
  }
}


/*
 * compare two longwords of data, masking out
 * the unused bytes as specified in the bytemask.
 * return TRUE if the data are different.
 */
DataMisCompare (dlo1, dlo2, bytemask)
unsigned dlo1, dlo2, bytemask;
{
  unsigned mask;

  mask = ((bytemask & 1 ? 0x000000ff : 0) |
	  (bytemask & 2 ? 0x0000ff00 : 0) |
	  (bytemask & 4 ? 0x00ff0000 : 0) |
	  (bytemask & 8 ? 0xff000000 : 0));

  return ((dlo1 ^ dlo2) & mask);
}

int ramWidth = RAM_WIDTH;

Paint (offset, v)
{
  int x, y;

  y = offset / ramWidth;
  x = offset % ramWidth;

  pointBuf[npoints].x = x;
  pointBuf[npoints].y = y;
  if (++npoints >= MAXPOINTS)
    npoints = MAXPOINTS - 1;

  fprintf (vid, "p %d %d %d\n", x, y, v);
}
