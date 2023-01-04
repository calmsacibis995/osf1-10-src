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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Id: spraycan.c,v 1.1.2.3 92/12/11 08:36:04 devrcs Exp $";
#endif		/* BuildSystemHeader */
/* jj-port
  #ifndef ULTRIX
  #module SPRAYCAN
  #endif
*/
/*
****************************************************************************
**                                                                          *
**  Copyright (c) 1988                                                      *
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
**   John Hainsworth, May 1988
**
**  ABSTRACT:
**
**   Spray operation:  simulate the function of a spray can.
**
**  ENVIRONMENT:
**
**   User mode, executable image.
**
**  MODIFICATION HISTORY:
**	May 1988 John Hainsworth - created.
**
**      dl      10/6/88
**      don't spray if fill pattern is null
**--
**/           
/*  */

#include "paintrefs.h"
#include <stdlib.h>

#define ONE_PLUS_EPS 1.0000001

typedef struct {
    short dx;
    short dy;
} SprayDot;

static int DotCount = 10;	/* number of dots per screen location:*/
				/*   set equal to SprayRadius */
static XSegment *Dots;		/* array of dots to be output */
static double SprayPower = 2.0;	/* power of density decay from center */
static int SprayRadius = 10;	/* radius of spray area */
static int SprayTableSize;	/* number of SprayDot's in SprayTable */
static SprayDot *SprayTable;	/* table of random points */
static int SprayTableIndex = 0;	/* next value to use in SprayTable */
static short SprayTableSet = 0;	/* flag indicating if SprayTable exists */

/*  */

int rand();		/* returns a number from 0 to RAND_MAX-1 */

/* macro to generate a random integer in the closed interval [0, N-1] */
#define RANDOM(N) ((int)(((double)(N)) * ((double)rand()) / ((double)RAND_MAX)))

/* static void PrintSprayHistogram (
    char *s,
    int R,
    double *histogram)
{
    int i;
    int j;
    int index;
    double histogram_sum;

    printf ("<<<<< %s >>>>>\n", s);

    index = 0;
    histogram_sum = 0.0;
    for (i=(-R); i<=R; i++) {
	for (j=(-R); j <=R; j++) {
	    printf ("%7.1f", histogram[index]);
	    histogram_sum += histogram[index++];
	}
	printf ("\n");
    }
    printf ("<<<<< SUM = %f, max value = %f >>>>>\n", 
		    histogram_sum, histogram[2 * R * (R + 1)]);
} */

/* static void PrintSprayTable (
    SprayDot *t,
    int nt)
{
    int i;
    for (i=0; i<nt; i++) {
	printf ("[%3d,%3d] ", (int) (t[i].dx), (int)(t[i].dy));
    }
} */

void DestroySprayTable ()
    /* static SprayDot *SprayTable; */
    /* static int SprayTableSet; */
    /* static XSegment *Dots; */
{
    if (SprayTableSet != 0)
    {
	XtFree ((char *) Dots);
	XtFree ((char *) SprayTable);
	SprayTableSet = 0;
    }
}

static void CreateSprayTable ()
    /* static double SprayPower; */
    /* static int SprayRadius; */
    /* static int SprayTableSize; */
    /* static SprayDot *SprayTable; */
{
    register double *histogram;
    register double histogram_sum;
    register double histogram_sum2;
    register int histogram_size;

    /*** if there's already a SprayTable, get rid of it */
    DestroySprayTable ();
    SprayTableSet++;

    /*** allocate memory for the 2-D histogram */
    histogram_size = (2 * SprayRadius + 1) * (2 * SprayRadius + 1);
    histogram = (double *) XtMalloc (sizeof(double) * histogram_size);

    /*** fill the histogram and compute its sum */
    histogram_sum = 0.0;
    {
	register int ih = 0;
	register int ix;
	register int iy;
	register double weight;
	register double RR = SprayRadius * SprayRadius;
	for (iy=(-SprayRadius); iy<=SprayRadius; iy++) {
	    for (ix=(-SprayRadius); ix<=SprayRadius; ix++) {
		if ((weight = ((double) (ix*ix + iy*iy)) / RR) < ONE_PLUS_EPS) {
		    histogram_sum += (histogram[ih++] = exp (-SprayPower * weight));
		} else {
		    histogram[ih++] = 0.0;
		}
	    }
	}
    }
    /* PrintSprayHistogram ("unscaled", SprayRadius, histogram); */

    /*** Set SprayTableSize, and then rescale all histogram entries so that */
    /*** the histogram sum is (approximately) equal to SprayTableSize. */
    SprayTableSize = 5 * histogram_size;
    {
	register int ih2;
	register double rescale_factor = SprayTableSize / histogram_sum;
	histogram_sum2 = 0.0;
	for (ih2=0; ih2<histogram_size; ih2++) {
	    histogram_sum2 += (histogram[ih2] = 
			       (int) (histogram[ih2] * rescale_factor + 0.5));
	}
    }
    /* PrintSprayHistogram ("scaled, but unadjusted", SprayRadius, histogram);*/

    /*** randomly increment or decrement histogram entries until */
    /*** the histogram sum is exactly equal to SprayTableSize. */
    {
	register int id = (int) (histogram_sum2 + 0.5) - SprayTableSize;
	register int ihr;
	while (id != 0) {
	    if (histogram[ihr = RANDOM (histogram_size)] > 0.5) {
		if (id > 0) {
		    histogram[ihr] -= 1.0;
		    id--;
		} else {
		    histogram[ihr] += 1.0;
		    id++;
		}
	    }
	}
    }
    /* PrintSprayHistogram ("scaled and adjusted", SprayRadius, histogram); */

    /*** Allocate space (never freed) for the lookup table */
    SprayTable = (SprayDot *) XtMalloc (sizeof (SprayDot) * SprayTableSize);

    {
	register int it = 0;
	register int ih3 = 0;
	register int ix2;
	register int iy2;
	register int frequency;
	for (iy2=(-SprayRadius); iy2<=SprayRadius; iy2++) {
	    for (ix2=(-SprayRadius); ix2<=SprayRadius; ix2++) {
		frequency = (int) (histogram[ih3++] + 0.5);
		while (frequency--) {
		    SprayTable[it].dx = ix2;
		    SprayTable[it++].dy = iy2;
		}
	    }
	}
    }

    {
	register int it2;
	register int itr;
	register short temp_short;
	for (it2=0; it2<SprayTableSize; it2++) {
	    if (it2 != (itr = RANDOM (SprayTableSize))) {
		temp_short = SprayTable[it2].dx;
		SprayTable[it2].dx = SprayTable[itr].dx;
		SprayTable[itr].dx = temp_short;
		temp_short = SprayTable[it2].dy;
		SprayTable[it2].dy = SprayTable[itr].dy;
		SprayTable[itr].dy = temp_short;
	    }
	}
    }

    /*** free the memory used by the histogram */
    if (histogram != NULL)
    {
	XtFree ((char *) histogram);
	histogram = NULL;
    }

    /* PrintSprayTable (SprayTable, SprayTableSize); */
}

/* generate a spraycan spray at the current position. */
void Rband_Spray()
    /* globalref Display *disp; */
    /* globalref cur_x; */
    /* globalref cur_y; */
    /* static int SprayTableSet; */
    /* static double SprayPower; */
    /* static XSegment *Dots; */
    /* static int DotCount; */
    /* static int SprayRadius; */
    /* static int SprayTableSize; */
    /* static SprayDot *SprayTable; */
    /* static int SprayTableIndex = 0; */
{
    register int i;
    register int delta_x;
    register int delta_y;
    int xmin, ymin, xmax, ymax;

    if (!SprayTableSet) {
	char instr[80];

	CreateSprayTable ();

	DotCount = SprayRadius;
	Dots = (XSegment *) XtMalloc ((sizeof (XSegment)) * DotCount); /* jj-port */
    }

    Save_Point (cur_x, cur_y);
    rband_x = points[numpts-2].x;
    rband_y = points[numpts-2].y;
    delta_x = points[numpts-1].x - rband_x;
    delta_y = points[numpts-1].y - rband_y;
      
    ymax = xmax = -1;
    ymin = picture_ht;
    xmin = picture_wd;

    for (i=0; i<DotCount; i++) {
	if (++SprayTableIndex >= SprayTableSize) SprayTableIndex = 0;
	Dots[i].x2 = (Dots[i].x1 = SprayTable[SprayTableIndex].dx + 
		      rband_x + (delta_x * i) / DotCount)
		     + 1;
	Dots[i].y2 = Dots[i].y1 = SprayTable[SprayTableIndex].dy +
	             rband_y + (delta_y * i) / DotCount;

	if (xmin > Dots[i].x1) xmin = Dots[i].x1;
	if (xmax <= Dots[i].x1) xmax = Dots[i].x2; /* .x1 + 1 */
	if (ymin > Dots[i].y1) ymin = Dots[i].y1;
	if (ymax <= Dots[i].y1) ymax = Dots[i].y1 + 1;
    }

    Continue_Save_Picture_State (xmin, ymin, xmax - xmin, ymax - ymin);

/* if fill pattern is none, don't spray dl - 10/6/88 */
    if( outline_stipple )   /* jj - 3/27/89 */
/*  if( fill_stipple ) */
	XDrawSegments (disp, picture, Get_GC(GC_PD_SPRAY), Dots, DotCount);

    Refresh_Picture(xmin, ymin, xmax - xmin, ymax - ymin);

/* Refresh Zoom window if necessary */
    if( ifocus == zoom_widget )
	Refresh_Zoom_View( xmin, ymin, xmax - xmin, ymax - ymin);

}

/*
 *
 * ROUTINE:  Draw_Spray
 *
 * ABSTRACT: Draw a spraycan stroke into the picture
 *
 */
void Draw_Spray()
{
    int x, y, wd, ht, xmax, ymax;
    int px, py, delta_x, delta_y;
    int i, j, k;
    int xoff, yoff;
    XPoint pts[2];
    Pixmap tmp_pix;

    xmax = shape_xmax + SprayRadius + 1;
    ymax = shape_ymax + SprayRadius + 1;
    x = MAX (-picture_x, shape_xmin - SprayRadius);
    y = MAX (-picture_y, shape_ymin - SprayRadius);
    wd = MIN (xmax, IX_TO_PMX (pimage_wd)) - x;
    ht = MIN (ymax, IY_TO_PMY (pimage_ht)) - y;

    i = UG_num_used;
    Find_More_Affected_Rect (x, y, wd, ht, &UG_num_used, UG_used);

    if (i == UG_num_used)
	return;

    Set_Cursor_Watch (pwindow);

    while (i < UG_num_used) {
        j = UG_used[i];
        UG_image[j] = XSubImage (picture_image, ui_x (j), ui_y (j),
                                 ui_wd (j), ui_ht (j));
        i++;
    }

    for (i = 1; i <= 4; i++) {
        if (UG_image[UG_last + i] != 0) {
	    XSetTSOrigin (disp, Get_GC (GC_PD_SPRAY), -UG_extra[i].x,
			  -UG_extra[i].y);
            tmp_pix = XCreatePixmap (disp, DefaultRootWindow(disp),
                                     UG_extra[i].wd, UG_extra[i].ht, pdepth);
            XPutImage (disp, tmp_pix, Get_GC (GC_PD_COPY),
                       UG_image[UG_last + i], 0, 0, 0, 0,
                       UG_extra[i].wd, UG_extra[i].ht);

            xoff = picture_x - UG_extra[i].x;
            yoff = picture_y - UG_extra[i].y;

            for (j = 1; j < numpts; j++) {
		x = MIN(points[j].x, points[j-1].x) - SprayRadius + xoff;
		y = MIN(points[j].y, points[j-1].y) - SprayRadius + yoff;
		wd = abs (points[j].x - points[j-1].x) + (2 * SprayRadius) + 1;
		ht = abs (points[j].y - points[j-1].y) + (2 * SprayRadius) + 1;
		if (Intersecting_Rectangles (0, 0, UG_extra[i].wd,
                                             UG_extra[i].ht, x, y, wd, ht)) {
		    px = points[j-1].x + xoff;
		    py = points[j-1].y + yoff;
		    delta_x = points[j].x - points[j-1].x;
		    delta_y = points[j].y - points[j-1].y;
		    for (k=0; k<DotCount; k++) {
			if (++SprayTableIndex >= SprayTableSize) 
			    SprayTableIndex = 0;
			Dots[k].x1 = SprayTable[SprayTableIndex].dx + px + 
				     (delta_x * k) / DotCount;
			Dots[k].x2 = Dots[k].x1 + 1;
			Dots[k].y1 = SprayTable[SprayTableIndex].dy + py + 
				     (delta_y * k) / DotCount;
			Dots[k].y2 = Dots[k].y1;
		    }
		    XDrawSegments (disp, tmp_pix, Get_GC(GC_PD_SPRAY), Dots,
				   DotCount);
		}
	    }
            MY_XGetSubImage (disp, tmp_pix, 0, 0, UG_extra[i].wd,
                             UG_extra[i].ht, bit_plane, img_format,
                             picture_image,  UG_extra[i].x,
                             UG_extra[i].y, ImgK_Src);

            XFreePixmap (disp, tmp_pix);
        }
    }
    XSetTSOrigin (disp, Get_GC (GC_PD_SPRAY), -picture_x, -picture_y);
    Set_Cursor (pwindow, current_action);
}
