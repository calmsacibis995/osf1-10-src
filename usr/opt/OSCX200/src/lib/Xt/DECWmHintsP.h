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
#ifndef _DECWMHintsP_h_
#define  _DECWMHintsP_h_
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif
typedef struct {
	INT32 title_font;
	INT32 icon_font;
	INT32 border_width;
	INT32 title_height;
	INT32 non_title_width;
	INT32 icon_name_width;
	INT32 iconify_width;
	INT32 iconify_height;
} internalDecorationGeometryRec, *internalDecorationGeometry;

#define WmNumDecorationGeometryElements (sizeof(internalDecorationGeometryRec)/4)

typedef struct {
	INT32 value_mask;
	INT32 iconify_pixmap;
	INT32 icon_box_x;
	INT32 icon_box_y;
	INT32 tiled;
	INT32 sticky;
	INT32 no_iconify_button;
	INT32 no_lower_button;
	INT32 no_resize_button;
} internalDECWmHintsRec, *internalDECWmHints;

#define WmNumDECWmHintsElements (sizeof(internalDECWmHintsRec)/4)

typedef struct {
	INT32 state;
} internalWmIconStateRec, *internalWmIconState;

#define WmNumIconStateElements (sizeof(internalWmIconStateRec)/4)

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _DECWmHintsP_h_ */
