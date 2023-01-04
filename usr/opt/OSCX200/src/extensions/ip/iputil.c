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
static char rcsid[] = "$Header: /alphabits/u3/x11/ode/rcs/x11/src/extensions/ip/iputil.c,v 1.1.2.3 92/06/11 19:01:15 Jim_Ludwig Exp $";
#endif

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
/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/extensions/ip/iputil.c,v 1.1.2.3 92/06/11 19:01:15 Jim_Ludwig Exp $
 *  util.c:
 *	Shared-Memory IPC package utilities
 */

#include "ip.h"

#define WAIT_THRESH	7
#define WAIT_TICKS	800

/* must be a power of 2 */
#define TABSIZE 8
#define HIGHTAB (TABSIZE-1)
#define TABMASK (TABSIZE-1)

typedef struct _HashElement {
    Card used;
    Card key;
    Card count;
    Card sum;
} HashElement, *HashElementPtr;

static HashElement table[TABSIZE];

void ipInitHash()
{
    int i;

    for (i=0; i<TABSIZE; i++) {
	table[i].used = FALSE;
	table[i].key = 0;
	table[i].count = 0;
	table[i].sum = 0;
    }
} /* end ipInitHash() */

static int
Hash(str)
char *str;
{
    return (str[0] ^ str[1] ^ str[2] ^ str[3]) & TABMASK;
} /* end Hash() */

static int
Enter(key, value)
Card key;
Card value;
{
    int index = Hash(&key);
    int i = index;

    do {
	if (table[i].used == FALSE) {
	    table[i].used = TRUE;
	    table[i].key = key;
	    table[i].count++;
	    table[i].sum += value;
	    return TRUE;
	} else if (table[i].key == key) {
	    table[i].count++;
	    table[i].sum += value;
	    return TRUE;
	}
	i = (i == HIGHTAB) ? 0 : i+1;
    } while (i == index);
    return FALSE;
}


/******************************
 * ipWait():
 *	wait for something to happen, and keep track of who is
 *	doing a lot of waiting.
 ******************************/
ipWait(n, param)
Card n;
Card param;
{
   register int i;

   Enter(param, n);		/* for performance analysis */
   if (n < WAIT_THRESH)
	for (i=0; i<(1<<(n<<2)); i++) {
	   ipNoOp(i); /* defeat -O */
	}
   else
	sleep(0);		/* force context switch */
} /* end ipWait() */
