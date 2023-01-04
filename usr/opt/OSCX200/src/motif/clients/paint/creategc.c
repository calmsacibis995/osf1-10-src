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
/* jj-port
  #ifndef ULTRIX
  #module CREATEGC "V1-000"
  #endif
*/
/*
****************************************************************************
**                                                                          *
**  Copyright (c) 1987                                                      *
**  By DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                        *
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
**   This module contains code to create GC's and pass pointers back
**
**  ENVIRONMENT:
**
**   User mode, executable image.
**
**  MODIFICATION HISTORY:
**
**      dl      10/6/88
**      Change spraycan and brush to use the fill pattern
**
**--       
**/           

/*  */

#include "paintrefs.h"
static char gray_bitmap_data[] =
    {0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55};

GC Create_GC( gc_id )
int gc_id;
{
    XGCValues values;     
    int value_mask;
    GC new_gc;
    Pixmap gray_pix;

    switch (gc_id) {
    	case GC_PD_SOLID :
	    values.foreground = picture_fg;
	    values.background = picture_bg;
	    values.fill_style = FillSolid;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | GCFillStyle | 
			 GCGraphicsExposures;
	    new_gc = XCreateGC (disp, pxmap, value_mask, &values);
	    break;

	case  GC_PD_ERASER :
	    values.foreground = picture_bg;
	    values.background = picture_fg;
	    values.fill_style = FillSolid;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | GCFillStyle | 
			 GCGraphicsExposures;
	    new_gc = XCreateGC (disp, pxmap, value_mask, &values);
	    break;

	case GC_PD_SQUARE_BRUSH :
	    values.foreground = picture_fg;
	    values.background = picture_bg;
	    values.fill_style = FillOpaqueStippled;
	    values.stipple = outline_stipple; /* jj - 3/27/89 */
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | GCFillStyle | 
			 GCGraphicsExposures;
	    if (values.stipple != 0)
		value_mask |= GCStipple;
	    new_gc = XCreateGC (disp, pxmap, value_mask, &values);
	    break;

	case  GC_PD_ROUND_BRUSH :
	    values.foreground = picture_fg;
	    values.background = picture_bg;
	    values.fill_style = FillOpaqueStippled;
	    values.stipple = outline_stipple; /* jj - 3/27/89 */
	    values.join_style = JoinRound;
	    values.line_width = cur_line_wd;
	    values.cap_style = CapRound;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | GCFillStyle | 
			 GCJoinStyle | GCLineWidth | GCCapStyle | 
			 GCGraphicsExposures;
	    if (values.stipple != 0)
		value_mask |= GCStipple;
	    new_gc = XCreateGC (disp, pxmap, value_mask, &values);
	    break;

	case GC_PD_OUTLINE :
	    values.foreground = picture_fg;
	    values.background = picture_bg;
	    values.fill_style = FillOpaqueStippled;
	    values.stipple = outline_stipple;
	    values.line_width = cur_line_wd;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | GCFillStyle | 
			 GCLineWidth | GCGraphicsExposures;
	    if (values.stipple != 0)
		value_mask |= GCStipple;
	    new_gc = XCreateGC (disp, pxmap, value_mask, &values);
	    break;

	case  GC_PD_FILL :
	    values.foreground = picture_fg;
	    values.background = picture_bg;
	    values.fill_style = FillOpaqueStippled;
	    values.stipple = fill_stipple;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | GCFillStyle | 
			 GCGraphicsExposures;
	    if (values.stipple != 0)
		value_mask |= GCStipple;
	    new_gc = XCreateGC (disp, pxmap, value_mask, &values);
	    break;

	case  GC_PD_FLOOD :
	    values.foreground = picture_fg;
	    values.background = picture_bg;
	    values.fill_style = FillOpaqueStippled;
	    values.stipple = fill_stipple;
	    values.graphics_exposures = FALSE;
	    values.cap_style = CapNotLast;
	    values.line_width = 1;
	    value_mask = GCForeground | GCBackground | GCFillStyle | 
			 GCCapStyle | GCGraphicsExposures | GCLineWidth;
	    if (values.stipple != 0)
		value_mask |= GCStipple;
	    new_gc = XCreateGC (disp, pxmap, value_mask, &values);
	    break;

	case GC_PD_STROKE :
	    values.foreground = picture_fg;
	    values.background = picture_bg;
	    values.fill_style = FillOpaqueStippled;
	    values.stipple = outline_stipple;
	    values.line_width = cur_line_wd;
	    values.join_style = JoinRound;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | GCFillStyle | 
			 GCLineWidth | GCJoinStyle | GCGraphicsExposures;
	    if (values.stipple != 0)
		value_mask |= GCStipple;
	    new_gc = XCreateGC (disp, pxmap, value_mask, &values);
	    break;

	case  GC_PD_COPY :
	    values.foreground = picture_fg;
	    values.background = picture_bg;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | GCGraphicsExposures;
	    new_gc = XCreateGC( disp, pxmap, value_mask, &values);
	    break;

	case GC_PD_FUNCTION :
	    values.foreground = picture_fg;
	    values.background = picture_bg;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | GCGraphicsExposures;
	    new_gc = XCreateGC (disp, pxmap, value_mask, &values);
	    break;

	case GC_PD_INVERT :
	    values.function = GXxor;
	    values.foreground = colormap[BLACK].pixel ^ colormap[WHITE].pixel;
	    values.background = colormap[BLACK].pixel;
	    values.plane_mask = AllPlanes;
	    values.graphics_exposures = FALSE;
	    value_mask = GCFunction | GCForeground | GCPlaneMask |
			 GCBackground | GCGraphicsExposures;
	    new_gc = XCreateGC (disp, pxmap, value_mask, &values);
	    break;

	case  GC_PD_MASK :
	    values.foreground = picture_fg;
	    values.background = picture_bg;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | GCGraphicsExposures;
	    new_gc = XCreateGC (disp, pxmap, value_mask, &values);
	    break;

	case GC_PD_SPRAY :
	    values.function = GXcopy;
	    values.fill_style = FillOpaqueStippled;
	    values.stipple = outline_stipple; /* jj - 3/27/89 */
	    values.cap_style = CapNotLast;
	    values.foreground = picture_fg;
	    values.background = picture_bg;
	    values.graphics_exposures = FALSE;
	    values.line_width = 1;
	    value_mask = GCFunction | GCForeground | GCBackground | 
			 GCGraphicsExposures | GCFillStyle | GCCapStyle |
			 GCLineWidth;
	    if (values.stipple != 0)
		value_mask |= GCStipple;
	    new_gc = XCreateGC (disp, pxmap, value_mask, &values);
	    break;

	case GC_SD_SOLID :
	    values.foreground = colormap[paint_color].pixel;
	    values.background = colormap[paint_bg_color].pixel;
	    values.fill_style = FillSolid;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | GCFillStyle | 
			 GCGraphicsExposures;
	    new_gc = XCreateGC (disp, pxmap, value_mask, &values);
	    break;

	case GC_SD_ERASER :
	    values.foreground = colormap[paint_bg_color].pixel;
	    values.background = colormap[paint_color].pixel;
	    values.fill_style = FillSolid;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | GCFillStyle |
			 GCGraphicsExposures;
	    new_gc = XCreateGC (disp, pxmap, value_mask, &values);
	    break;

	case GC_SD_SQUARE_BRUSH :
	    values.foreground = colormap[paint_color].pixel;
	    values.background = colormap[paint_bg_color].pixel;
	    values.fill_style = FillOpaqueStippled;
	    values.stipple = outline_stipple; /* jj - 3/27/89 */
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | GCFillStyle |
			 GCGraphicsExposures;
	    if (values.stipple != 0)
		value_mask |= GCStipple;
	    new_gc = XCreateGC (disp, pxmap, value_mask, &values);
	    break;

	case GC_SD_ROUND_BRUSH :
	    values.foreground = colormap[paint_color].pixel;
	    values.background = colormap[paint_bg_color].pixel;
	    values.fill_style = FillOpaqueStippled;
	    values.stipple = outline_stipple; /* jj - 3/27/89 */
	    values.join_style = JoinRound;
	    values.line_width = cur_line_wd;
	    values.cap_style = CapRound;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | GCFillStyle |
			 GCJoinStyle | GCLineWidth | GCCapStyle | 
			 GCGraphicsExposures;
	    if (values.stipple != 0)
		value_mask |= GCStipple;
	    new_gc = XCreateGC (disp, pxmap, value_mask, &values);
	    break;

	case GC_SD_STROKE :
	    values.foreground = colormap[paint_color].pixel;
	    values.background = colormap[paint_bg_color].pixel;
	    values.fill_style = FillOpaqueStippled;
	    values.stipple = outline_stipple;
	    values.line_width = cur_line_wd;
	    values.join_style = JoinRound;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | GCFillStyle |
			 GCLineWidth | GCJoinStyle | GCGraphicsExposures;
	    if (values.stipple != 0)
		value_mask |= GCStipple;
	    new_gc = XCreateGC (disp, pxmap, value_mask, &values);
	    break;

	case GC_SD_OUTLINE :
	    values.foreground = colormap[paint_color].pixel;
	    values.background = colormap[paint_bg_color].pixel;
	    values.fill_style = FillOpaqueStippled;
	    values.stipple = outline_stipple;
	    values.line_width = cur_line_wd;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | GCFillStyle |
			 GCLineWidth | GCGraphicsExposures;
	    if (values.stipple != 0)
		value_mask |= GCStipple;
	    new_gc = XCreateGC (disp, pxmap, value_mask, &values);
	    break;

	case  GC_SD_FILL :
	    values.foreground = colormap[paint_color].pixel;
	    values.background = colormap[paint_bg_color].pixel;
	    values.fill_style = FillOpaqueStippled;
	    values.stipple = fill_stipple;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | GCFillStyle |
			 GCGraphicsExposures;
	    if (values.stipple != 0)
		value_mask |= GCStipple;
	    new_gc = XCreateGC (disp, pxmap, value_mask, &values);
	    break;

	case GC_SD_COPY :
	    values.foreground = colormap[paint_color].pixel;
	    values.background = colormap[paint_bg_color].pixel;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | GCGraphicsExposures;
	    new_gc = XCreateGC (disp, pxmap, value_mask, &values);
	    break;

	case GC_RUBBERBAND :
/*
	    values.function = GXinvert;
	    values.foreground = colormap[paint_color].pixel;
	    values.background = colormap[paint_bg_color].pixel;
	    values.graphics_exposures = FALSE;
	    value_mask = GCFunction | GCForeground | GCBackground | 
			 GCGraphicsExposures;
	    new_gc = XCreateGC (disp, pxmap, value_mask, &values);
	    break;
*/
	    values.function = GXxor;
	    values.foreground = colormap[BLACK].pixel ^ colormap[WHITE].pixel;
	    values.background = colormap[BLACK].pixel;
	    values.plane_mask = AllPlanes;
	    values.graphics_exposures = FALSE;
	    value_mask = GCFunction | GCForeground | GCPlaneMask |
			 GCBackground | GCGraphicsExposures;
	    new_gc = XCreateGC (disp, pxmap, value_mask, &values);
	    break;

	case GC_D1_SOLID :
	    values.foreground = 1;
	    values.background = 0;
	    values.fill_style = FillSolid;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | GCFillStyle |
			 GCGraphicsExposures;
	    new_gc = XCreateGC (disp, btmap, value_mask, &values);
	    break;

	case GC_D1_COPY :
	    values.foreground = 1;
	    values.background = 0;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | GCGraphicsExposures;
	    new_gc = XCreateGC (disp, btmap, value_mask, &values);
	    break;

	case GC_D1_INVERT :
	    values.function = GXinvert;
	    values.foreground = 1;
	    values.background = 0;
	    values.graphics_exposures = FALSE;
	    value_mask = GCFunction | GCForeground | GCBackground |
			 GCGraphicsExposures;
	    new_gc = XCreateGC (disp, btmap, value_mask, &values);
	    break;

	case GC_D1_FUNCTION :
	    values.foreground = 1;
	    values.background = 0;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | GCGraphicsExposures;
	    new_gc = XCreateGC (disp, btmap, value_mask, &values);
	    break;

	case GC_D1_GRAY :
	    gray_pix = XCreateBitmapFromData (disp, DefaultRootWindow (disp),
					      gray_bitmap_data, 8, 8);
	    values.foreground = 1;
	    values.background = 0;
	    values.fill_style = FillOpaqueStippled;
	    values.stipple = gray_pix;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | GCFillStyle | 
			 GCGraphicsExposures;
	    if (values.stipple != 0)
		value_mask |= GCStipple;
	    new_gc = XCreateGC (disp, btmap, value_mask, &values);
	    break;

	case GC_MD_COPY :
            values.foreground = window_fg;
            values.background = window_bg;
	    values.line_width = 2;
            values.graphics_exposures = FALSE;
            value_mask = GCForeground | GCBackground | GCLineWidth |
			 GCGraphicsExposures;
            new_gc = XCreateGC (disp, DefaultRootWindow(disp), value_mask,
				&values);
            break;

	case GC_MD_SOLID :
	    values.foreground = window_fg;
	    values.background = window_bg;
	    values.fill_style = FillSolid;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | GCFillStyle | 
			 GCGraphicsExposures;
            new_gc = XCreateGC (disp, DefaultRootWindow(disp), value_mask,
				&values);
	    break;

	case GC_MD_OUTLINE :
	    values.foreground = window_fg;
	    values.background = window_bg;
	    values.fill_style = FillOpaqueStippled;
	    values.stipple = outline_stipple;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | GCFillStyle |
			 GCGraphicsExposures;
	    if (values.stipple != 0)
		value_mask |= GCStipple;
            new_gc = XCreateGC (disp, DefaultRootWindow(disp), value_mask,
				&values);
	    break;

	case  GC_MD_FILL :
	    values.foreground = window_fg;
	    values.background = window_bg;
	    values.fill_style = FillOpaqueStippled;
	    values.stipple = fill_stipple;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | GCFillStyle |
			 GCGraphicsExposures;
	    if (values.stipple != 0)
		value_mask |= GCStipple;
            new_gc = XCreateGC (disp, DefaultRootWindow(disp), value_mask,
				&values);
	    break;

    }
    return( new_gc );
}
