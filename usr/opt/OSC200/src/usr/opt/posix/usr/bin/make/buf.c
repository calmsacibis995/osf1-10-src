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
static char *rcsid = "@(#)$RCSfile: buf.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/29 22:01:22 $";
#endif

/*
 * Copyright (c) 1988, 1989, 1990 The Regents of the University of California.
 * Copyright (c) 1988, 1989 by Adam de Boor
 * Copyright (c) 1989 by Berkeley Softworks
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Adam de Boor.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*-
 * buf.c --
 *	Functions for automatically-expanded buffers.
 */

#include    <stdlib.h>
#include    "alphaosf.h"
#include    "buf.h"

void bcopy();
void *emalloc();

typedef struct {
    int	    size; 	/* Current size of the buffer */
    Byte    *buffer;	/* The buffer itself */
    Byte    *inPtr;	/* Place to write to */
    Byte    *outPtr;	/* Place to read from */
} Buf, *BufPtr;

#ifndef max
#define max(a,b)  ((a) > (b) ? (a) : (b))
#endif

/*
 * BufExpand --
 * 	Expand the given buffer to hold the given number of additional
 *	bytes.
 *	Makes sure there's room for an extra NULL byte at the end of the
 *	buffer in case it holds a string.
 */
#define BufExpand(bp,nb) \
 	if (((bp)->size - ((bp)->inPtr - (bp)->buffer)) < (nb)+1) {\
	    int newSize = (bp)->size + max((nb)+1,BUF_ADD_INC); \
	    Byte  *newBuf = (Byte *) realloc((bp)->buffer, newSize); \
	    \
	    (bp)->inPtr = newBuf + ((bp)->inPtr - (bp)->buffer); \
	    (bp)->outPtr = newBuf + ((bp)->outPtr - (bp)->buffer);\
	    (bp)->buffer = newBuf;\
	    (bp)->size = newSize;\
	}

#define BUF_DEF_SIZE	256 	/* Default buffer size */
#define BUF_ADD_INC	256 	/* Expansion increment when Adding */

/*-
 *-----------------------------------------------------------------------
 * Buf_AddByte --
 *	Add a single byte to the buffer.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The buffer may be expanded.
 *
 *-----------------------------------------------------------------------
 */
void
Buf_AddByte (Buffer buf, Byte byte)
{
    register BufPtr  bp = (BufPtr) buf;

    BufExpand (bp, 1);

    *bp->inPtr = byte;
    bp->inPtr += 1;

    /*
     * Null-terminate
     */
    *bp->inPtr = 0;
}

/*-
 *-----------------------------------------------------------------------
 * Buf_AddBytes --
 *	Add a number of bytes to the buffer.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Guess what?
 *
 *-----------------------------------------------------------------------
 */
void
Buf_AddBytes (Buffer buf, int numBytes, Byte *bytesPtr)
{
  /* Buffer  buf; */
  /* int     numBytes; */
  /* Byte    *bytesPtr; */

    register BufPtr  bp = (BufPtr) buf;

    BufExpand (bp, numBytes);

    bcopy (bytesPtr, bp->inPtr, numBytes);
    bp->inPtr += numBytes;

    /*
     * Null-terminate
     */
    *bp->inPtr = 0;
}

/*-
 *-----------------------------------------------------------------------
 * Buf_GetAll --
 *	Get all the available data at once.
 *
 * Results:
 *	A pointer to the data and the number of bytes available.
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
Byte *
Buf_GetAll (Buffer buf, int *numBytesPtr)
{
    /* Buffer  buf; */
    /* int     *numBytesPtr; */

    BufPtr  bp = (BufPtr)buf;

    if (numBytesPtr != (int *)NULL) {
	*numBytesPtr = bp->inPtr - bp->outPtr;
    }
    
    return (bp->outPtr);
}

/*-
 *-----------------------------------------------------------------------
 * Buf_Discard --
 *	Throw away bytes in a buffer.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The bytes are discarded. 
 *
 *-----------------------------------------------------------------------
 */
void
Buf_Discard (Buffer buf, int numBytes)
{
    /* Buffer  buf; */
    /* int    numBytes; */

    register BufPtr	bp = (BufPtr) buf;

    if (bp->inPtr - bp->outPtr <= numBytes) {
	bp->inPtr = bp->outPtr = bp->buffer;
	*bp->inPtr = 0;
    } else {
	bp->outPtr += numBytes;
    }
}

/*-
 *-----------------------------------------------------------------------
 * Buf_Size --
 *	Returns the number of bytes in the given buffer. Doesn't include
 *	the null-terminating byte.
 *
 * Results:
 *	The number of bytes.
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
int
Buf_Size (Buffer buf)
{
    /* Buffer  buf; */

    return (((BufPtr)buf)->inPtr - ((BufPtr)buf)->outPtr);
}

/*-
 *-----------------------------------------------------------------------
 * Buf_Init --
 *	Initialize a buffer. If no initial size is given, a reasonable
 *	default is used.
 *
 * Results:
 *	A buffer to be given to other functions in this library.
 *
 * Side Effects:
 *	The buffer is created, the space allocated and pointers
 *	initialized.
 *
 *-----------------------------------------------------------------------
 */
Buffer
Buf_Init (int size)
{
    /* int	    size;      Initial size for the buffer */

    BufPtr  bp;	  	/* New Buffer */

    bp = (Buf *)emalloc(sizeof(Buf));

    if (size <= 0) {
	size = BUF_DEF_SIZE;
    }
    bp->size = size;
    bp->buffer = (Byte *)emalloc (size);
    bp->inPtr = bp->outPtr = bp->buffer;
    *bp->inPtr = 0;

    return ((Buffer) bp);
}

/*-
 *-----------------------------------------------------------------------
 * Buf_Destroy --
 *	Nuke a buffer and all its resources.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The buffer is freed.
 *
 *-----------------------------------------------------------------------
 */
void
Buf_Destroy (Buffer buf, Boolean freeData)
{
    /* Buffer  buf;  	   Buffer to destroy */
    /* Boolean freeData;   TRUE if the data should be destroyed as well */

    BufPtr  bp = (BufPtr) buf;
    
    if (freeData) {
	free ((Address)bp->buffer);
    }
    free ((Address)bp);
}
