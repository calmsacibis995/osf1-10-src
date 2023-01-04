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
static char SccsId[] = "@(#)gedrawbxr.c	1.4\t8/29/89";
/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1988 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
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
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**	GEXDRAWBXR			Draws a rounded rectangle
**
**  ABSTRACT:
**
** 
**
** Pt numbering convention used in fbx and vbx vectors
**
**	      1	+ ----------------------------- + 2
**   xp1,yp1) +					  + (xp2,yp2)
** 	   0 +					   + 3
**	     |					   |
**	     |					   |
**	     |					   |
**	     |					   |
**	     |					   |
**	     |					   |
** 	   7 +					   + 4
**  (xp4,yp4) +					  + (xp3,yp3)
**	      6	+ ----------------------------- + 5
**
** Let dx, dy be the slope of any given side.  Let Len be the absolute
** distance from a corner of the box to an end point of the arc.
** And let Lside be the length of the side in question.
** Then dxp, dyp, the horizontal and vertical steps from the box corner to the
** end point of the arc is given by
**
**            dxp = dx * Len / Lside
**            dyp = dy * Len / Lside
**
** The signs of dxp and dyp are determined by the sign of the slope of the
** side on which they reside.
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 01/08/87 Created                            
**
**--
**/

#include "geGks.h"
#include <math.h>

geDrawBxr(win, gca, x1,y1, x2,y2, x3,y3, x4,y4, lw, lp, pixcol,
	  linstyle, linht, linmode,
	  filstyle, filcol, filht, filmode)
Window  	win;
GC		gca;
long    	x1,y1, x2,y2, x3,y3, x4,y4;
int		lw, lp;
unsigned long	pixcol;
int		linstyle;
short   	linht;
int     	linmode, filstyle;
unsigned long	filcol;
short		filht;
int		filmode;
{
int	alpha[4], beta[4];
long	dx, dy, xm, ym;
float	xc[4], yc[4], r[4], x_f, y_f, tf, H, W, Len, fx1, fy1, fx2, fy2,
        fx3, fy3, fx4, fy4, fdx, fdy, fbx[12], fby[12];
XPoint	vbx[12];

if (gca != geGC5)
  {if (linmode == GXxor && (pixcol == geBlack.pixel || pixcol == GEXORPIXEL))
      {GEL_FG_P_M_PM(gca, lw, lp, GEXORPIXEL, linmode, geXWPixel ^ geXBPixel);}
    else
      {GEL_FG_P_M_PM(gca, lw, lp, pixcol, linmode, AllPlanes);}
   }

if (!(dx = abs(x3 - x1)))
   {XDrawLine(geDispDev, win, gca, x2, y2, x3, y3);
    return;
   }

if (!(dy = abs(y3 - y1)))
   {XDrawLine(geDispDev, win, gca, x1, y1, x2, y2);
    return;
   }

W        = (float)sqrt((double)(x2 - x1) * (double)(x2 - x1) +
		       (double)(y2 - y1) * (double)(y2 - y1));
H        = (float)sqrt((double)(x4 - x1) * (double)(x4 - x1) +
		       (double)(y4 - y1) * (double)(y4 - y1));

if (W > H)
  Len = .20 * H;
else
  Len = .20 * W;

if (Len < 1.)
  {vbx[0].x = x1; vbx[0].y = y1;
   vbx[1].x = x2; vbx[1].y = y2;
   vbx[2].x = x3; vbx[2].y = y3;
   vbx[3].x = x4; vbx[3].y = y4;
   vbx[4]   = vbx[0];

   if (gca != geGC5 && filstyle != GETRANSPARENT)
     {GEFILSTYLE_FOS(geGC3, filstyle, filcol);
      GEFORG(geGC3, filcol);
      GEMODE(geGC3, filmode);
      GESTIPPLE(geGC3, filht);
      XFillPolygon(geDispDev, win, geGC3, vbx, 5,  Complex, CoordModeOrigin);
     }

   if (linstyle != GETRANSPARENT)
     XDrawLines(geDispDev, win, gca, vbx, 5, CoordModeOrigin);
   return;
  }

fx1      = (float)x1;        fy1      = (float)y1;
fx2      = (float)x2;        fy2      = (float)y2;
fx3      = (float)x3;        fy3      = (float)y3;
fx4      = (float)x4;        fy4      = (float)y4;

/*
 * Compute all the vertex points and the arc components' center points
 */
fbx[0]   = fx1 + Len * (fx4 - fx1) / H; GEINT(fbx[0], vbx[0].x);
fby[0]   = fy1 + Len * (fy4 - fy1) / H; GEINT(fby[0], vbx[0].y);
fbx[2]   = fx1 + Len * (fx2 - fx1) / W; GEINT(fbx[2], vbx[2].x);
fby[2]   = fy1 + Len * (fy2 - fy1) / W; GEINT(fby[2], vbx[2].y);
fbx[3]   = fx2 + Len * (fx1 - fx2) / W; GEINT(fbx[3], vbx[3].x);
fby[3]   = fy2 + Len * (fy1 - fy2) / W; GEINT(fby[3], vbx[3].y);
fbx[5]   = fx2 + Len * (fx3 - fx2) / H; GEINT(fbx[5], vbx[5].x);
fby[5]   = fy2 + Len * (fy3 - fy2) / H; GEINT(fby[5], vbx[5].y);
fbx[6]   = fx3 + Len * (fx2 - fx3) / H; GEINT(fbx[6], vbx[6].x);
fby[6]   = fy3 + Len * (fy2 - fy3) / H; GEINT(fby[6], vbx[6].y);
fbx[8]   = fx3 + Len * (fx4 - fx3) / W; GEINT(fbx[8], vbx[8].x);
fby[8]   = fy3 + Len * (fy4 - fy3) / W; GEINT(fby[8], vbx[8].y);
fbx[9]   = fx4 + Len * (fx3 - fx4) / W; GEINT(fbx[9], vbx[9].x);
fby[9]   = fy4 + Len * (fy3 - fy4) / W; GEINT(fby[9], vbx[9].y);
fbx[11]   = fx4 + Len * (fx1 - fx4) / H; GEINT(fbx[11], vbx[11].x);
fby[11]   = fy4 + Len * (fy1 - fy4) / H; GEINT(fby[11], vbx[11].y);
/*
 * Compute the 1st arc's mid-point
 */
xc[0] = fbx[0] + fbx[2] - fx1;
yc[0] = fby[0] + fby[2] - fy1;
tf    = (fx1 + (xc[0] - fx1) * .2929) * GEFSUIPP;
GEINT(tf, xm);
tf    = (fy1 + (yc[0] - fy1) * .2929) * GEFSUIPP;
GEINT(tf, ym);
vbx[1].x = GETPIX(xm);
vbx[1].y = GETPIX(ym);
geGenRC(GETSUI(vbx[0].x), GETSUI(vbx[0].y), xm, ym,
	GETSUI(vbx[2].x), GETSUI(vbx[2].y), &xc[0], &yc[0], &r[0],
	&alpha[0], &beta[0]);
/*
 * Compute the 2nd arc's mid-point
 */
xc[1] = fbx[3] + fbx[5] - fx2;
yc[1] = fby[3] + fby[5] - fy2;
tf    = (fx2 + (xc[1] - fx2) * .2929) * GEFSUIPP;
GEINT(tf, xm);
tf    = (fy2 + (yc[1] - fy2) * .2929) * GEFSUIPP;
GEINT(tf, ym);
vbx[4].x = GETPIX(xm);
vbx[4].y = GETPIX(ym);
geGenRC(GETSUI(vbx[3].x), GETSUI(vbx[3].y), xm, ym,
	GETSUI(vbx[5].x), GETSUI(vbx[5].y), &xc[1], &yc[1], &r[1],
	&alpha[1], &beta[1]);
/*
 * Compute the 3rd arc's mid-point
 */
xc[2] = fbx[6] + fbx[8] - fx3;
yc[2] = fby[6] + fby[8] - fy3;
tf    = (fx3 + (xc[2] - fx3) * .2929) * GEFSUIPP;
GEINT(tf, xm);
tf    = (fy3 + (yc[2] - fy3) * .2929) * GEFSUIPP;
GEINT(tf, ym);
vbx[7].x = GETPIX(xm);
vbx[7].y = GETPIX(ym);
geGenRC(GETSUI(vbx[6].x), GETSUI(vbx[6].y), xm, ym,
	GETSUI(vbx[8].x), GETSUI(vbx[8].y), &xc[2], &yc[2], &r[2],
	&alpha[2], &beta[2]);
/*
 * Compute the 4th arc's mid-point
 */
xc[3] = fbx[9] + fbx[11] - fx4;
yc[3] = fby[9] + fby[11] - fy4;
tf   = (fx4 + (xc[3] - fx4) * .2929) * GEFSUIPP;
GEINT(tf, xm);
tf   = (fy4 + (yc[3] - fy4) * .2929) * GEFSUIPP;
GEINT(tf, ym);
vbx[10].x = GETPIX(xm);
vbx[10].y = GETPIX(ym);
geGenRC(GETSUI(vbx[9].x), GETSUI(vbx[9].y), xm, ym,
	GETSUI(vbx[11].x), GETSUI(vbx[11].y), &xc[3], &yc[3], &r[3],
	&alpha[3], &beta[3]);

if (gca != geGC5 && filstyle != GETRANSPARENT)
   {GEFILSTYLE_FOS(geGC3, filstyle, filcol);
    GEFORG(geGC3, filcol);
    GEMODE(geGC3, filmode);
    GESTIPPLE(geGC3, filht);
    XFillPolygon(geDispDev, win, geGC3, vbx, 12,  Complex, CoordModeOrigin);
   }

if (r[0])
   geDrawArcNu(win, gca, xc[0], yc[0], r[0], alpha[0], beta[0], lw, lp, pixcol,
	       linstyle, linht, linmode, filstyle, filcol, filht, filmode,
	       vbx[0].x, vbx[0].y, vbx[2].x, vbx[2].y);

else
if (linstyle != GETRANSPARENT)
   XDrawLine(geDispDev, win, gca, vbx[0].x, vbx[0].y, vbx[2].x, vbx[2].y);

if (linstyle != GETRANSPARENT)
  XDrawLine(geDispDev, win, gca, vbx[2].x, vbx[2].y, vbx[3].x, vbx[3].y);

if (r[1])
   geDrawArcNu(win, gca, xc[1], yc[1], r[1], alpha[1], beta[1], lw, lp, pixcol,
	       linstyle, linht, linmode, filstyle, filcol, filht, filmode,
	       vbx[3].x, vbx[3].y, vbx[5].x, vbx[5].y);
else
if (linstyle != GETRANSPARENT)
   XDrawLine(geDispDev, win, gca, vbx[3].x, vbx[3].y, vbx[5].x, vbx[5].y);

if (linstyle != GETRANSPARENT)
  XDrawLine(geDispDev, win, gca, vbx[5].x, vbx[5].y, vbx[6].x, vbx[6].y);

if (r[2])
   geDrawArcNu(win, gca, xc[2], yc[2], r[2], alpha[2], beta[2], lw, lp, pixcol,
	       linstyle, linht, linmode, filstyle, filcol, filht, filmode,
	       vbx[6].x, vbx[6].y, vbx[8].x, vbx[8].y);
else
if (linstyle != GETRANSPARENT)
   XDrawLine(geDispDev, win, gca, vbx[6].x, vbx[6].y, vbx[8].x, vbx[8].y);

if (linstyle != GETRANSPARENT)
  XDrawLine(geDispDev, win, gca, vbx[8].x, vbx[8].y, vbx[9].x, vbx[9].y);

if (r[3])
   geDrawArcNu(win, gca, xc[3], yc[3], r[3], alpha[3], beta[3], lw, lp, pixcol,
	       linstyle, linht, linmode, filstyle, filcol, filht, filmode,
	       vbx[9].x, vbx[9].y, vbx[11].x, vbx[11].y);
else
if (linstyle != GETRANSPARENT)
   XDrawLine(geDispDev, win, gca, vbx[9].x, vbx[9].y, vbx[11].x, vbx[11].y);

if (linstyle != GETRANSPARENT)
  XDrawLine(geDispDev, win, gca, vbx[11].x, vbx[11].y, vbx[0].x, vbx[0].y);


}
                                     
