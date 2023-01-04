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
static char rcsid[] = "$Header: /alphabits/u3/x11/ode/rcs/x11/src/extensions/ip/ipcs.c,v 1.1.2.3 92/06/11 19:01:06 Jim_Ludwig Exp $";
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
/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/extensions/ip/ipcs.c,v 1.1.2.3 92/06/11 19:01:06 Jim_Ludwig Exp $
 *  cs.c:
 *	Critical Section routines:
 *	This is based on Dekker's solution to the Critical Section
 *	problem (just like you learned in school); see "The Logical
 *	Design of Operating Systems, Allen Shaw, p. 64" for an
 *	example.  This algorithm is not good in the general case,
 *	but it works fine here.
 */

#include "ip.h"

#ifdef IP_STATS
extern int ipnEnter;
extern int ipnLeave;
extern int ipnConflict;
extern int ipnMaxWait1;
extern int ipnMaxWait2;
#endif IP_STATS


/******************************
 * ipEnterSection():
 ******************************/
int
ipEnterSection(chan, buf)
ChannelPtr chan;
BufDescPtr buf;
{
    int me;
    int them;
    int i = 0;
#ifdef IP_STATS
    int conflict = 0;
    int wait1 = 0;
    int wait2 = 0;
#endif IP_STATS

    if (chan == NULL) return IP_BADCHAN;
    if (!CHANNEL_OK(chan)) return IP_SHUTDOWN;
    if (buf == NULL) buf = &chan->base->bd0;

#ifdef IP_STATS
    ipnEnter++;
#endif IP_STATS
    me = chan->whoami;
    them = THEM(me);

A1:
    buf->usingBuf[me] = TRUE;
L1:
    if (!CHANNEL_OK(chan)) return IP_SHUTDOWN;
    if ( buf->usingBuf[them] ) {
#ifdef IP_STATS
	if (!conflict) {
	    conflict = 1;
	    ipnConflict++;
	    wait1++;
	} else {
	    wait1++;
	}
#endif IP_STATS
	if (buf->turn == me) goto L1;
	buf->usingBuf[me] = FALSE;
#ifdef IP_STATS
	wait2 = 0;
#endif IP_STATS
	while (buf->turn == them) {
#ifdef IP_STATS
	    wait2++;
#endif IP_STATS
	    WAIT(++i, buf);
	};
#ifdef IP_STATS
	if (wait2 > ipnMaxWait2) ipnMaxWait2 = wait2;
#endif IP_STATS
	goto A1;
    } /* end if */
#ifdef IP_STATS
	if (wait1 > ipnMaxWait1) ipnMaxWait1 = wait1;
#endif IP_STATS
    return IP_SUCCESS;

} /* end ipEnterSection() */


/******************************
 * ipLeaveSection():
 ******************************/
int
ipLeaveSection(chan, buf)
ChannelPtr chan;
BufDescPtr buf;
{
    int me;
    int them;

    if (chan == NULL) return IP_BADCHAN;
    if (!CHANNEL_OK(chan)) return IP_SHUTDOWN;
    if (buf == NULL) buf = &chan->base->bd0;

#ifdef IP_STATS
    ipnLeave++;
#endif IP_STATS

    me = chan->whoami;
    them = THEM(me);

    buf->turn = them;
    buf->usingBuf[me] = FALSE;

    if (CHANNEL_OK(chan))
	return IP_SUCCESS;
    else
	return IP_SHUTDOWN;


} /* end ipLeaveSection() */
