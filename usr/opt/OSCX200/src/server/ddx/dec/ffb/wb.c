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
static char *rcsid = "@(#)$RCSfile: wb.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:49:23 $";
#endif
/*
 */
/* wb.c -- simulate EV-4 write buffer for accesses to ffb+
 *
 * Simulates worst-case behavior of wb.  Nothing gets written until the wb is
 * full.  Writes don't want to be merged into previous wb lines.  Writes within
 * a line should be in order.  Lines flushes will write in order.  No reads
 * until wb has flushed.
 */
#include "ffbcpu.h"


/*
#undef CPU_WB_WORDS
#define CPU_WB_WORDS  0
*/

#if CPU_WB_WORDS != 0

#define RA()		(~0)
#if (defined(ALPHA) || defined(__alpha)) && !defined(VMS)
#  undef  RA
#  include <c_asm.h>
#  define RA()	asm("addq %ra,0,%v0")
#endif

typedef struct _wbrec {
    long base;
    int  dirty;
    int  data[CPU_WB_WORDS];
    long ra[CPU_WB_WORDS];
} wbRec, *wbPtr;

static wbRec wb[CPU_WB_LINES];

static unsigned long wbline = 0;


#ifndef SOFTWARE_MODEL
#define BusWrite(pdst,data,mask)  *((volatile unsigned int *)(pdst)) = (data)
#define BusRead(psrc)             *((unsigned int *)(psrc))
#endif

static void wbFlushLine(wb, mask)
    wbPtr wb;
    unsigned mask;
{
    register i;

    if (wb->dirty) {
	for( i=0; i < CPU_WB_WORDS; i++ ) {
	    if (wb->dirty & (1 << i)) {
		BusWrite(wb->base | (i * sizeof(int)), wb->data[i], mask);
	    }
	}
	wb->dirty = 0;
	wb->base = 0;   /* Make sure no match */
    }
}
#endif

wbBusWrite(addr, data, mask)
    unsigned long addr;
    unsigned int  data;
    unsigned      mask;
{
#if CPU_WB_WORDS != 0
    long ra = RA();
    long word, found, lim, l, line, i;
#endif

   if ((long)(addr) & 3) {
	ErrorF("addr 0x%lx is not 32-bit aligned\n", addr);
	abort();
    }
#if CPU_WB_WORDS != 0
    if (mask != 0xf) {
	ErrorF("mask 0x%lx is not 0xf\n", mask);
	abort();
    }
    word  = (addr &  CPU_WB_LINEMASK) / sizeof(int);
    addr &= (addr &~ CPU_WB_LINEMASK);

    found = 0;
    /*
     * look for a match.  only valid results are: no match or found in
     * current wbline.
     */
    if (wbline > CPU_WB_LINES)
	lim = wbline - (CPU_WB_LINES-1);
    else
	lim = 0;

    for ( l = wbline; l >= lim; l-- ) {
	long line;
	line = l % CPU_WB_LINES;
	if (wb[line].base == addr) {
	    if (wb[line].dirty & (1 << word)) {
		/* no real merge allowed */
		ErrorF("addr 0x%lx @ 0x%lx conflict 0x%lx\n",
			addr | (word * sizeof(int)), wb[line].ra[word],
			ra);
		abort();
	    }
	    if (l != wbline) {
		/* no "reaching back" in the write buffer, either */
		ErrorF("addr 0x%lx @ 0x%lx not in current wbline\n",
			addr | (word * sizeof(int)), ra);
		abort();
	    }
	    if (wb[line].dirty &  ~((1 << word) | ((1 << word)-1))) {
		ErrorF("addr 0x%lx @ 0x%lx out of order\n",
		       addr | (word * sizeof(int)), ra);
		abort();
	    }
	    wb[line].data[word] = data;
	    wb[line].dirty |= 1 << word;
	    wb[line].ra[word] = ra;
	    return line;
	}
    }
    /*
     * not found; can't merge into any line; move to new line and flush
     * if necessary.
     */
    line = wbline % CPU_WB_LINES;
    if (wb[line].dirty) {
	register i;
	wbline++;
	line = wbline % CPU_WB_LINES;
	wbFlushLine(&wb[line], mask);
    }
    wb[line].base = addr;
    wb[line].data[word] = data;
    wb[line].dirty = 1 << word;
    wb[line].ra[word] = ra;

    return line;
#else
    /* basically, we're running the model on mips and not simulating alpha behavior,
       so disable any wb'ing, since mips wb's don't merge... or, we just don't want
       to use this wb stimulator, because.  So there. */
    BusWrite(addr, data, mask);
#endif
}


wbBusRead(addr)
    unsigned long addr;
{
    if ((long)(addr) & 3) {
	ErrorF("addr 0x%lx is not 32-bit aligned\n", addr);
	abort();
    }
#if CPU_WB_WORDS != 0
    /*
     * wb must have been flushed...
     */
    if (wbline || wb[0].dirty) {
	ErrorF("wbBusRead: wb not flushed\n");
	abort();
    }
#endif
    return BusRead(addr);
}


void wbMB(mask)
{
#if CPU_WB_WORDS != 0
    long lim, line, l;

    if (wbline >= CPU_WB_LINES)
	lim = wbline - (CPU_WB_LINES-1);
    else
	lim = 0;

    for( l=lim; l <= wbline; l++ ) {
	line = l % CPU_WB_LINES;
	wbFlushLine(&wb[line], mask);
    }
    wbline = 0;
#endif
#ifdef WB_MODEL         /* we've flushed our model now make sure the real alpha is flushed */
#ifdef VMS
   __MB();
#else
   (void)asm("mb");
#endif
#endif
}


#ifdef WBTEST
#include <stdio.h>

abort() {}

ErrorF(s,a,b,c,d)
    long s,a,b,c,d;
{
    fprintf(stderr, s,a,b,c,d);
}

BusWrite(addr, data, mask)
    long addr;
    int data, mask;
{
    fprintf(stderr, "0x%x @ 0x%lx\n", data, addr);
}

BusRead(addr)
    long addr;
{
    fprintf(stderr, "0x%lx -> 0x%x\n", addr, *(int *)addr);
}

main(argc, argv)
    int argc;
    char **argv;
{
    int byte, i;

    fprintf(stderr, "INIT AND ORDER TESTS\n");
    for ( i=0; i < CPU_WB_LINES; i++ ) {
	byte = i * CPU_WB_WORDS * sizeof(int);
	fprintf(stderr, "wbline %d: 0x%4lx ", i, byte);
	wbBusWrite((long)byte, byte, 0xf);
	byte += (CPU_WB_WORDS-1) * sizeof(int);
	fprintf(stderr, "0x%4lx ", byte);
	wbBusWrite((long)byte, byte, 0xf);
	byte -= 2*sizeof(int);
	fprintf(stderr, "0x%4lx\n", byte);
	wbBusWrite((long)byte, byte, 0xf);
    }

    fprintf(stderr, "MERGE TEST\n");
    for ( i=0; i < CPU_WB_LINES; i++ ) {
	byte = (i * CPU_WB_WORDS * sizeof(int)) + sizeof(int);
	fprintf(stderr, "wbline %d: 0x%4lx ", i, byte);
	wbBusWrite((long)byte, byte, 0xf);
	byte += (CPU_WB_WORDS-3) * sizeof(int);
	fprintf(stderr, "0x%4lx\n", byte);
	wbBusWrite((long)byte, byte, 0xf);
    }

    fprintf(stderr, "OVER-WRITE TEST\n");
    for ( i=0; i < CPU_WB_LINES; i++ ) {
	byte = i * CPU_WB_WORDS * sizeof(int);
	fprintf(stderr, "wbline %d: 0x%4lx ", i, byte);
	wbBusWrite((long)byte, -byte, 0xf);
	byte += (CPU_WB_WORDS-1) * sizeof(int);
	fprintf(stderr, "0x%4lx\n", byte);
	wbBusWrite((long)byte, -byte, 0xf);
    }

    fprintf(stderr, "PUSH-OUT TEST\n");
    for ( i=CPU_WB_LINES; i < (CPU_WB_LINES+CPU_WB_LINES); i++ ) {
	byte = i * CPU_WB_WORDS * sizeof(int);
	fprintf(stderr, "wbline %d: 0x%4lx ", i, byte);
	wbBusWrite((long)byte, byte, 0xf);
	byte += (CPU_WB_WORDS-1) * sizeof(int);
	fprintf(stderr, "0x%4lx\n", byte);
	wbBusWrite((long)byte, byte, 0xf);
    }

    fprintf(stderr, "BUSREAD 0x%x AND MB TESTS\n", byte);
    wbBusRead(&byte);
    wbMB(0xf);
    wbBusRead(&byte);
}
#endif

/*
 * HISTORY
 */
