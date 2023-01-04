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
/* $XConsortium: t1snap.c,v 1.3 91/10/10 11:19:47 rws Exp $ */
/* Copyright International Business Machines,Corp. 1991
 * All Rights Reserved
 *
 * License to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is
 * hereby granted, provided that the above copyright notice
 * appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation,
 * and that the name of IBM not be used in advertising or
 * publicity pertaining to distribution of the software without
 * specific, written prior permission.
 *
 * IBM PROVIDES THIS SOFTWARE "AS IS", WITHOUT ANY WARRANTIES
 * OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT
 * LIMITED TO ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS.  THE ENTIRE RISK AS TO THE QUALITY AND
 * PERFORMANCE OF THE SOFTWARE, INCLUDING ANY DUTY TO SUPPORT
 * OR MAINTAIN, BELONGS TO THE LICENSEE.  SHOULD ANY PORTION OF
 * THE SOFTWARE PROVE DEFECTIVE, THE LICENSEE (NOT IBM) ASSUMES
 * THE ENTIRE COST OF ALL SERVICING, REPAIR AND CORRECTION.  IN
 * NO EVENT SHALL IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
 
#include "objects.h"
#include "spaces.h"
#include "paths.h"
 
/*
:h2.Handle Functions
 
:h3.Phantom() - Returns a Move Segment Equivalent to Handles
 
This is a user operator.  Its new name is QueryHandle.
*/
 
struct segment *t1_Phantom(obj)
       register struct xobject *obj;  /* object to take the Phantom of       */
{
       struct fractpoint pt; /* handle size will built here                  */
 
       if (obj == NULL)
               pt.x = pt.y = 0;
       else
               PathDelta(obj, &pt);
 
       return(PathSegment(MOVETYPE, pt.x, pt.y));
}
 
/*
:h3.Snap() - Force Ending Handle of Object to Origin
 
This is a user operator.
*/
 
struct xobject *t1_Snap(p)
       register struct segment *p;  /* path to snap                          */
{
       struct fractpoint pt; /* for finding length of path                   */
 
       if (p == NULL)
               return(NULL);
       p = UniquePath(p);
 
       PathDelta(p, &pt);
       if (p->last->type == MOVETYPE) {
               p->last->dest.x -= pt.x;
               p->last->dest.y -= pt.y;
       }
       else
               p = JoinSegment(p, MOVETYPE, -pt.x, -pt.y, NULL);
       return((struct xobject *)p);
}
