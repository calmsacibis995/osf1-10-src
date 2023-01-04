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
static char *rcsid = "@(#)$RCSfile: scu_malloc.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/04/10 16:18:57 $";
#endif
/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/* ---------------------------------------------------------------------- */

/* uagt_lib.c			Version 1.00		Apr. 25, 1991 

    This file contains support routines used by application programs that
are using the CAM User Agent Driver, UAgt - /dev/cam.

Modification History

	Version	  Date		Who	Reason

	1.00	04/25/91	jag	Creation date.
	1.01	10/04/91	rtm	Zero allocated buffers.
*/

/* ---------------------------------------------------------------------- */
/* Include files. */

#include <stdio.h>
#include <sys/types.h>
#include <malloc.h>

/* ---------------------------------------------------------------------- */
/* Local defines. */

#define MIN( X, Y )	((X) < (Y) ? (X) : (Y))
#define BUFLEN 		80

/* This structure is used by the page align malloc/free support code.  These
"working sets" will contain the malloc-ed address and the page aligned
address for the free*() call. */

typedef struct mpa_ws
{
    struct mpa_ws *next;	/* for the next on the list */
    char *palign_addr;		/* the page aligned address that's used */
    char *malloc_addr;		/* the malloc-ed address to free */
} MPA_WS;

/* ---------------------------------------------------------------------- */
/* External declarations. */

extern int getpagesize();
extern int bzero();

/* ---------------------------------------------------------------------- */
/* Initialized and uninitialized data. */

static MPA_WS mpa_qhead;	/* local Q head for the malloc stuctures */

/* ---------------------------------------------------------------------- */
/* This is a local allocation routine to alloc and return to the caller a
system page aligned buffer.  Enough space will be added, one more page, to
allow the pointers to be adjusted to the next page boundry.  A local linked
list will keep copies of the original and adjusted addresses.  This list 
will be used by free_palign() to free the correct buffer. */

char *
malloc_palign( size )
    int size;			/* what the user want page aligned */
{
    int page_size;		/* for holding the system's page size */
    MPA_WS *ws;			/* pointer for the working set */
    unsigned long tptr;		/* temp pointer for bit masking */

    /* The space for the working set structure that will go on the queue
    is allocated first. */

    ws = (MPA_WS *)malloc( sizeof(MPA_WS) );
    if( ws == (MPA_WS *)NULL )
    {
	return( (char *)NULL );		/* signal above of the failure */
    }

    /* Get the system page size, from the system, and save it. */

    page_size = getpagesize();	/* system call */

    /* Using the requested size, from the argument list, and the page size
    from the system allocate enough space to page align the requested 
    buffer.  The original request will have the space of one system page
    added to it.  The pointer will be adjusted. */

    ws->malloc_addr = (char *)malloc(size + page_size);
    if( ws->malloc_addr == (char *)NULL )
    {
	free( ws );		/* not going to be used */
	return( (char *)NULL );		/* signal the failure */
    } else {
	(void) bzero (ws->malloc_addr, (size + page_size));
    }

    /* Now using the allocated space + 1 page, adjust the pointer to 
    point to the next page boundry. */

    ws->palign_addr = ws->malloc_addr + page_size;	/* to the next page */

    /* Using the page size and subtracting 1 to get a bit mask, mask off
    the low order "page offset" bits to get the aligned address.  Now the
    aligned pointer will contain the address of the next page with enough
    space to hold the users requested size. */

    tptr = (unsigned long)ws->palign_addr;	/* copy to local int */
    tptr &= (unsigned long)(~(page_size - 1));	/* Mask addr bit to the */
    ws->palign_addr = (char *)tptr;		/* put back the address */

    /* Put the working set onto the linked list so that the original malloc-ed
    buffer can be freeed when the user program is done with it. */

    ws->next = mpa_qhead.next;		/* just put it at the head */
    mpa_qhead.next = ws;

    /* Now return the aligned address to th caller. */

    return( (char *)ws->palign_addr );
}

/* ---------------------------------------------------------------------- */

/* This is a local free routine to return to the system a previously alloc-ed
buffer.  A local linked list keeps copies of the original and adjusted
addresses.  This list is used by this routine to free the correct buffer. */

void
free_palign( pa_addr )
    char *pa_addr;		/* the page aligned address */
{
    MPA_WS *p, *q;		/* walkers for the malloc list */

    /* Walk along the malloc-ed memory linked list, watch for a match
    on the page aligned address.  If a match is found break out of the
    loop. */

    p = mpa_qhead.next;		/* set the pointers */
    q = NULL;

    while( p != NULL )
    {
	if( p->palign_addr == pa_addr )	/* found the buffer */
	    break;

	q = p;				/* save current */
	p = p->next;			/* get next */
    }

    /* After falling out of the loop the pointers are at the place where
    some work has to be done, (this could also be at the beginning).
    If a match is found call the free() routine to return the buffer, if
    the loop fell off the end just return. */

    if( p != NULL )
    {

	/* Where on the list is it, check for making it empty. */

	if( q == NULL )			/* at the front */
	{
	    mpa_qhead.next = p->next;	/* pop off front */
	}
	else				/* inside the list */
	{
	    q->next = p->next;		/* pop it */
	}

	free( p->malloc_addr );		/* free the malloc-ed addr */

	/* Now free up the working set, it is not needed any more. */

	free( p );
    }
}
