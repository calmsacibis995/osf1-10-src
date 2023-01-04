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
/*  DEC/CMS REPLACEMENT HISTORY, Element DECWMHINTS.H */
/*  *3    27-FEB-1988 18:18:48 GEORGE "Add copyright" */
/*  *2     5-JAN-1988 15:37:12 TREGGIARI "Replace with completely new version" */
/*  *1    18-DEC-1987 14:06:16 TREGGIARI "Initial Entry" */
/*  DEC/CMS REPLACEMENT HISTORY, Element DECWMHINTS.H */
/*
* $Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/clients/dxmail/DECWmHints.h,v 1.1.2.2 92/06/26 11:42:01 Dave_Hill Exp $
*/
/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1988 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**	MASSACHUSSETTS INSTITUTE OF TECHNOLOGY, CAMBRIDGE, MASS.	    *
**                         ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION OR MIT.                                                     *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**      < to be supplied >
**
**  ABSTRACT:
**
**      < to be supplied >
**
**  ENVIRONMENT:
**
**      < to be supplied >
**
**  MODIFICATION HISTORY:
**
**      < to be supplied >
**
**--
**/
#ifndef _DECWmHints_h_
#define _DECWmHints_h_

extern void WmSetDecorationGeometry();
extern void WmSetDECWmHints();
extern void WmSetIconBoxName();
extern Status WmGetDecorationGeoemtery();
extern Status WmGetDECWmHints();
extern Status WmGetIconBoxName();

typedef struct {
	Font title_font;
	Font icon_font;
	int border_width;
	int title_height;
	int non_title_width;
	int icon_name_width;
	int iconify_width;
	int iconify_height;
} WmDecorationGeometryRec, *WmDecorationGeometry;


#define DECWmIconifyPixmapMask		(1L<<0)
#define DECWmIconBoxXMask 		(1L<<1)
#define DECWmIconBoxYMask		(1L<<2)
#define DECWmTiledMask			(1L<<3)
#define DECWmStickyMask			(1L<<4)
#define DECWmNoIconifyButtonMask	(1L<<5)
#define DECWmNoLowerButtonMask		(1L<<6)
#define DECWmNoResizeButtonMask		(1L<<7)

typedef struct {
	unsigned long value_mask;
	Pixmap iconify_pixmap;
	int icon_box_x;
	int icon_box_y;
	Bool tiled;
	Bool sticky;
	Bool no_iconify_button;
	Bool no_lower_button;
	Bool no_resize_button;
} DECWmHintsRec, *DECWmHints;

#define WmNormalState 0
#define WmIconicState 1

typedef struct {
	int state; /* normal or iconic (or zoomed?) */
} WmIconStateRec, *WmIconState;

#endif /* _DECWmHints_h_ */

