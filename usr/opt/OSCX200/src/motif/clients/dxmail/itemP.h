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
/*
	@(#)$RCSfile: itemP.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/08/02 23:58:00 $
*/

/*
 *                     Copyright (c) 1987, 1991 by
 *              Digital Equipment Corporation, Maynard, MA
 *                      All rights reserved.
 *
 *   This software is furnished under a license and may be used and
 *   copied  only  in accordance with the terms of such license and
 *   with the  inclusion  of  the  above  copyright  notice.   This
 *   software  or  any  other copies thereof may not be provided or
 *   otherwise made available to any other person.  No title to and
 *   ownership of the software is hereby transferred.
 *
 *   The information in this software is subject to change  without
 *   notice  and should not be construed as a commitment by Digital
 *   Equipment Corporation.
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 */

/*
**++
**
**  FACILITY:
**
**      DECwindows mail
**
**  ABSTRACT:
**
**      This include file contains definitions for the item
**	widget used by DECwindows mail.
**
**  AUTHORS:
**
**  CREATION DATE:     22-Aug-1989
**
**  MODIFICATION HISTORY:
**	3.0	22-Aug-1989		
**		Created from V2 source.
**	3.1	19-Nov-1989		
**		Portability changes.
*/

#define DXmSClassItem		"Item"

typedef struct
    {
    XmOffsetPtr		itemoffsets;
    caddr_t		extension;
    } ItemClassPart;

typedef struct _ItemClassRec
    {
    CoreClassPart	core_class;
    XmPrimitiveClassPart	primitive_class;
    XmLabelClassPart	label_class;
    ItemClassPart	item_class;
    } ItemClassRec, *ItemClass;

typedef struct
    {
    int			indicator_x,
			indicator_y;
    short		indicator_width;
    short		indicator_height;
    Boolean		value;
    Pixel		original_foreground;
    Pixel		original_background;
/*    GC			original_foreground_gc;
    GC			original_background_gc;*/
    XtCallbackList	single_callback;
    XtCallbackList	extend_callback;
    XtCallbackList	singleconfirm_callback;
    } ItemPart;

typedef struct _ItemRec
    {
    CorePart		core;
    XmPrimitivePart	xmprimitive;
    XmLabelPart		label;
    ItemPart		item;
    } ItemRec;
externalref ItemClassRec	itemclassrec;
