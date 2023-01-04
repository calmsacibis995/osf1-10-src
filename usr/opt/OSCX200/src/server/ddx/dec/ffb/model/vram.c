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
static char *rcsid = "@(#)$RCSfile: vram.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/11/20 19:16:44 $";
#endif


#include <stdio.h>

#include "defs.h"
#include "types.h"
#include "vars.h"

#include "vram.h"

extern FILE *vid;
extern int   cmdLine;
extern int  vramLine;
FILE *vramOut;
FILE *vramFile;


#define NBUF 512
char    vrambuf[NBUF];

int noHaltOnError = 0;	/*from gram.y   don't halt on static miscompare*/
int diffCompare = 0;	/*               output sfb+ mem writes onstdout*/
int vramCompareFlag = 0;
int silent = 0;		/*               silent sfbtgen/sfb+ mem compare*/
#define VERBOSE 1       /*   verbose mode, for dynamic mismatching      */

unsigned *vram;

VramPointType pointBuf[MAXPOINTS];
int    npoints;

void NextLine();

static void DoCompare(unsigned addr, 
		      unsigned dataHi,
		      unsigned dataLo,
		      unsigned planemask,
		      unsigned bytemask);
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
	SFBDepth depth
	)
{
  int i, row, col, offset;
  unsigned pixel;
  int mask;
  PIXELS d, m;

  /*
   * mask off the offset into the RAM array.
   * it's 2MB (256K 64-bit words).
   */
  addr &= 0x3ffff;
  offset = addr % ((RAM_WIDTH/8) * RAM_DEPTH);
  row = offset / (RAM_WIDTH/8);
  col = 8 * (offset % (RAM_WIDTH/8));

  if (row > RAM_DEPTH-1) {
    fprintf(stderr, "do_rams: row value (%d) out of range, address = %08x\n",
	    row, addr);
    d.l.lo = d.l.hi = 0;
    return (d.l);
  }

  if (depth == DEPTH32 && BitsSet(byteMask) > 2) {
    fprintf (stderr, "do_rams: CAS conflict; depth=%d, mask=%02x\n",
	     depth, byteMask);
    exit (1);
  }

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

    d.l = data;
    m.l.lo = m.l.hi = planeMask;
    for (i=0; i<8; ++i) {
      switch (depth) {
      case DEPTH8:	pixel = d.p8[i];     mask = ((int)m.p8[i]<<24) >> 24;  break;
      case DEPTH32:	pixel = d.p32[i%2];  mask = m.p32[i%2];   break;
      default:
	fprintf (stderr, "**** unrecognized depth: %-.8x\n", depth);
	exit (1);
      }

      if (byteMask & (1 << i)) {
/*
	printf ("*** do_rams: write a=%x d=%x pm=%x\n",
		row*RAM_WIDTH+col+i, pixel, mask);*/
	vram[row * RAM_WIDTH + col+i] &= (~mask);
	vram[row * RAM_WIDTH + col+i] |= (pixel & mask);
	pixel = vram[row * RAM_WIDTH + col+i];

	if (vid) Paint (col+i, row, pixel);
      }
    }
  }

  d.l.lo = d.l.hi = 0;
  for (i=0; i<8; ++i) {
    if (byteMask & (1 << i)) {
      switch (depth) {
      case DEPTH8:	d.p8[i] = vram[row * RAM_WIDTH + col+i];	break;
      case DEPTH32:	d.p32[i%2] = vram[row * RAM_WIDTH + col+i];	break;
      default:
	fprintf (stderr, "**** unrecognized depth: %-.8x\n", depth);
	exit (1);
      }
    }
  }

  return (d.l);
}
 
SetScale(int zoom)
{
  if (vid == NULL) OpenVid();
  if (vid) fprintf( vid, "s %d\n", zoom);
}


/*
 * initialize the simulated VRAM with a pseudo-random pattern.
 */
InitVRAM()
{
  int i,j;
  extern char *malloc();

  vram = (unsigned *) malloc( RAM_DEPTH * RAM_WIDTH * sizeof( unsigned ));
  if (vram == NULL) {
    fprintf( stderr, "PANIC! InitVRAM.malloc returned NULL!\n" );
    exit( 1 );
  }

  srandom (1);

  for (i=0; i<=RAM_DEPTH-1; ++i)
    for (j=0; j<=RAM_WIDTH-1; ++j) {
      vram[i * RAM_WIDTH + j] = random();
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

static void DoCompare(unsigned addr,
		      unsigned dataHi,
		      unsigned dataLo,
		      unsigned planemask,
		      unsigned bytemask)
{
  unsigned a, dlo, dhi, plmsk, cas;
  ULONG64 data;

  if (bytemask == 0) return;

  data.hi = dataHi;
  data.lo = dataLo;

  do {
    NextLine ();
    if (sscanf(vrambuf, "%x %x %x %x %x", &a, &dlo, &dhi, &plmsk, &cas) != 5) {
      fprintf (stderr, "can't parse vram data '%s'\n", vrambuf);
      fprintf (stderr, "**** cmdLine = %d, vramLine = %d\n",
	       cmdLine, vramLine);
      exit (1);
    }
  } while (cas == 0);

  if(silent) { 
/* no data miscompares... Checks are done external on memory stream... */
  }
  else {    
    if (((a ^ addr)&0x3ffff)
	|| DataMisCompare (dlo, data.lo, bytemask & 0xf)
	|| DataMisCompare (dhi, data.hi, (bytemask>>4) & 0xf)
	|| plmsk != planemask || cas != bytemask) {
    fprintf (stderr, "**** incorrect writedata: cmdLine = %d, vramLine = %d\n",
	     cmdLine, vramLine);
    fprintf (stderr, "     expected %x %x %x %x %x\n",
	     (a & 0x3ffff), dlo, dhi, plmsk, cas);
    fprintf (stderr, "     received %x %x %x %x %x\n",
	     addr, data.lo, data.hi, planemask, bytemask); 
    
      if((!diffCompare)&&(!noHaltOnError)) exit(1);
      if(diffCompare)
	printf ("%x %x %x %x %x\n",addr,data.lo,data.hi,planemask,bytemask); 
  } }
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

/*
 * get the next line from the file of vram trace data.
 */
void NextLine()
{
  if (! fgets(vrambuf, NBUF, vramFile)) {
    fprintf (stderr, "eof on vramFile\n");
    fprintf (stderr, "**** cmdLine = %d, vramLine = %d\n",
	     cmdLine, vramLine);
    exit (1);
  }
  ++vramLine;
}

Paint (x, y, v)
{
  pointBuf[npoints].x = x;
  pointBuf[npoints].y = y;
  if (++npoints >= MAXPOINTS)
    npoints = MAXPOINTS - 1;

  fprintf (vid, "p %d %d %d\n", x, y, v);
/*  fprintf (stderr, "p %d %d %d\n", x, y, v);*/
}

