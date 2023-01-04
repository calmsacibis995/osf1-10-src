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
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
/* BuildSystemHeader added automatically */
/* $Header: /usr/sde/x11/rcs/x11/src/./motif/clients/paint/icons.h,v 1.1.2.2 92/12/11 08:34:59 devrcs Exp $ */
/****************************************************************************
**                                                                          *
**  Copyright (c) 1987                                                      *
**  By DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                        *
**  All Rights Reserved                                                     *
**                                                                          *
**  This software is furnished under a license and may be used and  copied  *
**  only  in  accordance  with  the  terms  of  such  license and with the  *
**  inclusion of the above copyright notice.  This software or  any  other  *
**  copies  thereof may not be provided or otherwise made available to any  *
**  other person.  No title to and ownership of  the  software  is  hereby  *
**  transferred.                                                            *
**                                                                          *
**  The information in this software is subject to change  without  notice  *
**  and  should  not  be  construed  as  a commitment by DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL assumes no responsibility for the use or  reliability  of  its  *
**  software on equipment which is not supplied by DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**   DECpaint - VMS DECwindows paint program
**
**  AUTHOR
**
**   Daniel Latham, October 1987
**
**  ABSTRACT:
**
**   containts bitmap definitions for icons
**
**  ENVIRONMENT:
**
**   User mode, executable image.
**
**  MODIFICATION HISTORY:
**
**
**--
**/           
#define NUM_ICONS 17

/** IMPORTANT!!! - order must correspond to the order of the icon data
  * array - each entry is the action for the corresponding icon.
 */
static char *icon_names[NUM_ICONS] = {
/* column 1 */
    "select_icon_36",
    "spraycan_icon_36",
    "eraser_icon_36",
    "text_icon_36",
    "line_icon_36",
    "rectangle_icon_36",
    "ellipse_icon_36",
    "stroke_icon_36",
    "dropper_icon_36",
/* column 2 */
    "scissors_icon_36",
    "pencil_icon_36",
    "brush_icon_36",
    "bucket_icon_36",
    "arc_icon_36",
    "square_icon_36",
    "circle_icon_36",
    "polygon_icon_36"
};

static int icon_action[NUM_ICONS] = {
/* column 1 */
    SELECT_RECT,
    SPRAYCAN,
    ERASE,
    TEXT,
    LINE,
    RECTANGLE,
    ELLIPSE,
    STROKE,
    DROPPER,
/* column 2 */
    SELECT_AREA,
    PENCIL,
    BRUSH,
    FLOOD,
    ARC,
    SQUARE,
    CIRCLE,
    POLYGON
};

