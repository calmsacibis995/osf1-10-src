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
static char *rcsid = "@(#)$RCSfile: ffbdma.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:08:44 $";
#endif
/*
 */

#include "ffb.h"
#include "ffbdma.h"
#include <sys/types.h>
#ifndef VMS
#include <sys/mman.h>
#endif

#if defined(alpha__) && !defined(VMS)
#include <mach/kern_return.h>
extern kern_return_t lw_wire();
extern kern_return_t lw_unwire();
#else
typedef int kern_return_t;
#define KERN_SUCCESS 0
#endif


int		virtualPageMask;
int		virtualPageShift;
Pixel8		**virtualPageMap;
unsigned long   virtualPageBase;
unsigned long   virtualPageMapLength;


#if defined(SOFTWARE_MODEL)
kern_return_t   fake_wire(virtualBaseAddr, sizeInPages, returnBufferAddr)
    unsigned long   virtualBaseAddr;
    unsigned long   sizeInPages;
    unsigned long   returnBufferAddr;
{
    /* Simulate the wiring interface.  In returnBufferAddr, put the magic
       handle in [0], and then map each page to itself starting at [1]. */

    int i;

    for (i = 0; i < sizeInPages; i++) {
	((unsigned long *)returnBufferAddr)[i+1] =
	    virtualBaseAddr + i * (virtualPageMask+1);
    }
    return KERN_SUCCESS;
} /* fake_wire */

kern_return_t   fake_unwire(magicHandle)
    unsigned long magicHandle;
{
    return KERN_SUCCESS;
} /* fake_unwire */

#   if defined(__alpha)
#       define do_wire(base, size, map) \
		lw_wire(base, size, map) & fake_wire(base, size, map)
#       define do_unwire(magicHandle) \
		lw_unwire(magicHandle) & fake_unwire(magicHandle)
#   else /* no lw_wire in kernel */    
#	define do_wire(base, size, map)     fake_wire(base, size, map)
#	define do_unwire(magicHandle)	    fake_unwire(magicHandle)
#   endif /* if alpha ... else ... */

#else /* real hardware */

#   define do_wire(base, size, map)     lw_wire(base, size, map)
#   define do_unwire(magicHandle)	lw_unwire(magicHandle)

#endif


Bool ffbWirePages(startAddr, byteLength)
    Pixel8  *startAddr;
    long    byteLength;
{
    long	    align;
    kern_return_t   result;

#ifndef VMS
    if (virtualPageMapLength != 0) {
	ErrorF("Trying to wire when already have pages wired\n");
	return FALSE;
    }
    align = (long)startAddr & virtualPageMask;
    virtualPageBase = (long)startAddr - align;
    virtualPageMapLength =
	(align + byteLength + virtualPageMask) >> virtualPageShift;
    result = do_wire((unsigned long)virtualPageBase,
		(unsigned long)virtualPageMapLength,
		(unsigned long)virtualPageMap);
    if (result != KERN_SUCCESS) {
	ErrorF("Couldn't lw_wire pages for DMA, returned %d\n", (long)result);
	virtualPageMapLength = 0;
	return FALSE;
    }
#endif
    return TRUE;
}

Bool ffbUnwirePages()
{
    kern_return_t   result;

#ifndef VMS
    if (virtualPageMapLength == 0) {
	ErrorF("Trying to unwire when no pages wired");
	return FALSE;
    }
    virtualPageMapLength = 0;
    result = do_unwire((unsigned long) ((virtualPageMap)[0]));
    if (result != KERN_SUCCESS) {
	ErrorF("Couldn't lw_unwire pages, returned %d\n", (long)result);
	return FALSE;
    }
#endif
    return TRUE;
}


static Bool ffbInitedDMA = FALSE;

/* Initialize data structures associated with DMA page locking */
Bool ffbInitDMA()
{
    int virtualPageBytes;
    
#ifndef VMS
    if (ffbInitedDMA) return TRUE;
    ffbInitedDMA = TRUE;

    /* Get # of bytes in a virtual pages */
    virtualPageBytes = getpagesize();
    virtualPageMask = virtualPageBytes - 1;
    virtualPageShift = FFSS(virtualPageBytes) - 1;
    
    /* Get a buffer shared with the kernel big enough for the lw_wire call */
#ifdef mips
#   ifdef SOFTWARE_MODEL
    virtualPageMap = (Pixel8 **) xalloc((FFBMAXDMAPAGES+1) * sizeof(Pixel8 *));
    if (virtualPageMap == NULL) {
	ErrorF("Could not allocate fake virtual page map buffer\n");
	return FALSE;
    }
#   endif
#else
    virtualPageMap = (Pixel8 **) mmap(0, (FFBMAXDMAPAGES+1) * sizeof(Pixel8 *),
	PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_VARIABLE | MAP_SHARED,
	-1, 0);
    if ((long)virtualPageMap == -1) {
	extern int errno;
	ErrorF("Could not get virtual page map buffer from kernel\n");
	ErrorF("     errno = %d\n", errno);
	return FALSE;
    }
#endif

#endif /* VMS */
    return TRUE;
} /* ffbInitDMA */

/*
 * HISTORY
 */
