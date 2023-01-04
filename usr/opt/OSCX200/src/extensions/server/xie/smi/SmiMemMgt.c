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

/***********************************************************
Copyright 1989 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/*
**++
**  FACILITY:
**
**      X Imaging Extension
**	Sample Machine Independant DDX
**
**  ABSTRACT:
**
**      This module contains SMI jacket routines which call either client
**	or server memory management functions depending on how its compiled.
**	This allows us to use common code for both client and server.
**
**  ENVIRONMENT:
**
**	VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHORS:
**
**      John Weber
**
**
**  CREATION DATE:      August 30, 1989
**
**  MODIFICATION HISTORY:
**
**--
*/

/*
**  Include files
*/
#include <stdio.h>
#ifdef SERVER
#include <X.h>
#include <XieDdx.h>
#include <os.h>
#endif
#ifdef CLIENT
#include <X11/Xlibint.h>
#endif

/* #define COUNT_SMIMEMMGT 1 */
/* #define TRACE_SMIMEMMGT 1 */

#ifdef TRACE_SMIMEMMGT

/*
** If COUNT_SMIMEMMGT is turned on, the trace will report the actual (modified)
** size and address of the allocation.
*/

#define TraceAlloc_(ptr, size, name) \
		ErrorF("Allocating %7d bytes at 0x%08x from %s\n", \
			(size),(unsigned)(ptr), name)
#define TraceRealloc_(ptr, size, newptr, name) \
		ErrorF("Realloc to %7d bytes at 0x%08x --> 0x%08x from %s\n", \
			(size), (unsigned)(ptr+4), (unsigned)(newptr), name)
#define TraceFree_(ptr, name) \
		ErrorF("Freeing                  at 0x%08x from %s\n", \
			(unsigned)(ptr), name)
#else
#define TraceAlloc_(ptr, size, name)
#define TraceRealloc_(ptr, size, newptr, name)
#define TraceFree_(ptr, name)
#endif


#ifdef COUNT_SMIMEMMGT
static int OtherMemInUse = 0;
static int BitsMemInUse = 0;
static int TotalMemInUse = 0;
#define CountAdjSize_(nbytes) ((nbytes)+=4)
#define CountAlloc_(ptr, nbytes) (*(int *)(ptr) = nbytes, \
				  (ptr)+=4, \
				  OtherMemInUse+=nbytes, \
				  TotalMemInUse+=nbytes)
#define CountFree_(ptr) ((ptr)-=4, \
			 OtherMemInUse-=(*(int *)(ptr)), \
			 TotalMemInUse-=(*(int *)(ptr)))
			 
#define CountAllocBits_(ptr, nbytes) (*(int *)(ptr) = nbytes, \
				  (ptr)+=4, \
				  BitsMemInUse+=nbytes, \
				  TotalMemInUse+=nbytes)
#define CountFreeBits_(ptr) ((ptr)-=4, \
			     BitsMemInUse-=(*(int *)(ptr)), \
			 TotalMemInUse-=(*(int *)(ptr)))

void _ReportCurrentMemoryCount()
{
    printf("%8d : bytes allocated by calls to Bits routines\n", BitsMemInUse);
    printf("%8d : bytes allocated by calls to regular routines\n", OtherMemInUse);
    printf("%8d : total bytes allocated\n", TotalMemInUse );
}
#else
#define CountAdjSize_(nbytes)
#define CountAlloc_(ptr, nbytes)
#define CountFree_(ptr)
#define CountAllocBits_(ptr, nbytes)
#define CountFreeBits_(ptr)
#endif


void *SmiCalloc(number,size)
 unsigned number;
 unsigned size;
{
#ifdef SERVER
    char *memptr;

    CountAdjSize_(size);
    memptr = (char *)Xalloc((number)*(size));
    if (memptr != NULL)
       memset(memptr,0,(number)*(size));
    CountAlloc_(memptr, size);
    TraceAlloc_(memptr, size*number, "SmiCalloc");
    return (void *)(memptr);
#endif
#ifdef CLIENT
    return (void *)Xcalloc((number),(size));
#endif
}

unsigned char *SmiCallocBits(bits)
 unsigned bits;
{
#ifdef SERVER
    unsigned char *memptr;
    int bytes = (bits)+39>>3;

    CountAdjSize_(bytes);
    memptr = (unsigned char *) Xalloc(bytes);

    if (memptr != NULL)
       memset(memptr,0,bytes);
    CountAllocBits_( memptr, bytes );
    TraceAlloc_(memptr, bytes, "SmiCallocBits");
    return (memptr);
#endif
#ifdef CLIENT
    return (unsigned char *)Xcalloc(1,(bits)+39>>3);
#endif
}

void  *SmiCfree(ptr)
 char *ptr;
{
#ifdef SERVER
    if(IsPointer_(ptr)) {
	TraceFree_(ptr, "SmiCfree");
        CountFree_(ptr);
	Xfree(ptr);   
	}
#endif
#ifdef CLIENT
    if(ptr)
	Xfree(ptr);   
#endif
    return(0);
}

void  *SmiFree(ptr)
 char *ptr;
{
#ifdef SERVER
    if(IsPointer_(ptr))
	{
	TraceFree_(ptr, "SmiFree");
        CountFree_(ptr);
	Xfree(ptr);   
	}
#endif
#ifdef CLIENT
    if(ptr)
	Xfree(ptr);   
#endif
    return(0);
}

unsigned char *SmiFreeBits(ptr)
 unsigned char *ptr;
{
#ifdef SERVER
    if(IsPointer_(ptr)) {
	TraceFree_(ptr, "SmiFreeBits");
	CountFreeBits_(ptr);
	Xfree(ptr);   
	}
#endif
#ifdef CLIENT
    if(ptr)
	Xfree(ptr);   
#endif
    return(0);
}

void *SmiMalloc(size)
 unsigned size;
{
#ifdef SERVER
    char *memptr;

    CountAdjSize_(size);
    memptr = (char *)Xalloc(size);

    CountAlloc_(memptr, size);
    TraceAlloc_(memptr, size, "SmiMalloc");
    return (void *)(memptr);
#endif
#ifdef CLIENT
    return (void *)Xmalloc(size);
#endif
}

unsigned char *SmiMallocBits(bits)
 unsigned bits;
{
#ifdef SERVER
    unsigned char *memptr;
    int bytes = (bits)+39>>3;

    CountAdjSize_(bytes);
    memptr = (unsigned char *) Xalloc(bytes);

    CountAllocBits_(memptr, bytes );
    TraceAlloc_(memptr, bytes, "SmiMallocBits");
    return (memptr);
#endif
#ifdef CLIENT
    return (unsigned char *)Xmalloc((bits)+39>>3);
#endif
}

void *SmiRealloc(ptr,size)
 char *ptr;
 unsigned size;
{
#ifdef SERVER
    char *memptr;

    CountFree_(ptr);
    CountAdjSize_(size);
    memptr = (char *)Xrealloc(ptr,size);
    CountAlloc_(memptr, size);

    TraceRealloc_(ptr, size, memptr, "SmiRealloc");
    return (void *)(memptr);
#endif
#ifdef CLIENT
    return (void *)Xrealloc(ptr,size);
#endif
}

unsigned char *SmiReallocBits(ptr,bits)
 unsigned char *ptr;
 unsigned bits;
{
#ifdef SERVER
    unsigned char *memptr;
    int bytes = (bits)+39>>3;

    CountFreeBits_(ptr);
    CountAdjSize_(bytes);
    memptr = (unsigned char *)Xrealloc(ptr,bytes);
    CountAllocBits_(memptr, bytes );

    TraceRealloc_(ptr, bytes, memptr, "SmiReallocBits");
    return (memptr);
#endif
#ifdef CLIENT
    return (unsigned char *)Xrealloc(ptr,(bits)+39>>3);
#endif
}

