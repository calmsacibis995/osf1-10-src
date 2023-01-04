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
static char SccsId[] = "@(#)geinitht.c	1.2\t1/31/90";
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
**	GEINITHT.C        		Creates half-tone stipple GCs
**
**  ABSTRACT:
**
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 11/20/87 Created
**	MAS 04/23/91 Modified to use 8x8 ordered dithered halftones
**
**--
**/

#include "geGks.h"
#include "geHT.h"

/*
 * Ordered dither matrix 8x8 from Fundamentals of Interactive Computer Graphics
 * by Foley & Van Dam, page 601.
 */

/*
static int	M[8][8] = {
		{  0, 32,  8, 40,  2, 34, 10, 42},
		{ 48, 16, 56, 24, 50, 18, 58, 26},
		{ 12, 44,  4, 36, 14, 46,  6, 38},
		{ 60, 28, 52, 20, 62, 30, 54, 22},
		{  3, 35, 11, 43,  1, 33,  9, 41},
		{ 51, 19, 59, 27, 49, 17, 57, 25},
		{ 15, 47,  7, 39, 13, 45,  5, 37},
		{ 63, 31, 55, 23, 61, 29, 53, 21}
		};
*/

static int	M[GEMSIZE][GEMSIZE] = {
 	{  0, 50, 12, 62,  3, 53, 16, 66,  1, 50, 13, 63,  4, 54, 16, 66},
 	{ 75, 25, 87, 37, 78, 28, 91, 41, 75, 26, 88, 38, 79, 29, 91, 41},
 	{ 19, 69,  6, 56, 22, 72,  9, 59, 19, 69,  7, 57, 22, 72, 10, 60},
 	{ 94, 44, 81, 31, 96, 47, 84, 34, 94, 44, 82, 32, 97, 47, 85, 35},
 	{  5, 55, 17, 67,  2, 51, 14, 64,  5, 55, 18, 68,  2, 52, 15, 65},
 	{ 79, 29, 92, 42, 76, 26, 89, 39, 80, 30, 93, 43, 77, 27, 90, 40},
 	{ 23, 73, 11, 61, 20, 70,  8, 58, 24, 74, 11, 61, 21, 71,  8, 58},
 	{ 98, 48, 86, 36, 95, 45, 83, 33, 97, 49, 86, 36, 96, 46, 83, 33},
 	{  1, 50, 13, 63,  4, 54, 16, 66,  0, 50, 12, 62,  3, 53, 16, 66},
 	{ 75, 26, 88, 38, 79, 29, 91, 41, 75, 25, 87, 37, 78, 28, 91, 41},
 	{ 19, 69,  7, 57, 22, 72, 10, 60, 19, 69,  6, 56, 22, 72,  9, 59},
 	{ 94, 44, 82, 32, 97, 47, 85, 35, 94, 44, 81, 31, 97, 47, 84, 34},
 	{  5, 55, 18, 68,  2, 52, 15, 65,  5, 55, 17, 67,  2, 51, 14, 64},
 	{ 80, 30, 93, 43, 77, 27, 90, 40, 79, 29, 92, 42, 76, 26, 89, 39},
 	{ 24, 74, 11, 61, 21, 71,  8, 58, 23, 73, 11, 61, 20, 70,  8, 58},
 	{ 99, 49, 86, 36, 96, 46, 83, 33, 98, 48, 86, 36, 95, 45, 83, 33}
 	};


Pixmap geMakeHTPixmap(lum)
    int         lum;
{
    int		row, col;
    short       ht[GEMSIZE];
    Pixmap      pixmap;

    for (row = 0; row < GEMSIZE; row++) {
        ht[row] = 0;
	for (col = 0; col < GEMSIZE; col++)
	    if (M[row][col] < lum) ht[row] |= 1 << col;
    }

    pixmap = XCreateBitmapFromData(geDispDev, geRgRoot.self, (char *)ht,
				   GEMSIZE, GEMSIZE);

    return(pixmap);
}
                 


geInitHT()

{
int i;
/*
 * Half-tones
 */
for (i = 0; i <= 100; i++) geHTStipple[i]  = geMakeHTPixmap(i);

/*
 * Patterns
 */
gePATStipple[0]  = XCreateBitmapFromData(geDispDev, geRgRoot.self, (char *)pat000,
					stip_w, stip_h);
gePATStipple[1]  = XCreateBitmapFromData(geDispDev, geRgRoot.self, (char *)pat001,
					stip_w, stip_h);
gePATStipple[2]  = XCreateBitmapFromData(geDispDev, geRgRoot.self, (char *)pat002,
					stip_w, stip_h);
gePATStipple[3]  = XCreateBitmapFromData(geDispDev, geRgRoot.self, (char *)pat003,
					stip_w, stip_h);
gePATStipple[4]  = XCreateBitmapFromData(geDispDev, geRgRoot.self, (char *)pat004,
					stip_w, stip_h);
gePATStipple[5]  = XCreateBitmapFromData(geDispDev, geRgRoot.self, (char *)pat005,
					stip_w, stip_h);
gePATStipple[6]  = XCreateBitmapFromData(geDispDev, geRgRoot.self, (char *)pat006,
					stip_w, stip_h);
gePATStipple[7]  = XCreateBitmapFromData(geDispDev, geRgRoot.self, (char *)pat007,
					stip_w, stip_h);
gePATStipple[8]  = XCreateBitmapFromData(geDispDev, geRgRoot.self, (char *)pat008,
					stip_w, stip_h);
gePATStipple[9]  = XCreateBitmapFromData(geDispDev, geRgRoot.self, (char *)pat009,
					stip_w, stip_h);
gePATStipple[10] = XCreateBitmapFromData(geDispDev, geRgRoot.self, (char *)pat010,
					stip_w, stip_h);
gePATStipple[11] = XCreateBitmapFromData(geDispDev, geRgRoot.self, (char *)pat011,
					stip_w, stip_h);
gePATStipple[12] = XCreateBitmapFromData(geDispDev, geRgRoot.self, (char *)pat012,
					stip_w, stip_h);
gePATStipple[13] = XCreateBitmapFromData(geDispDev, geRgRoot.self, (char *)pat013,
					stip_w, stip_h);
gePATStipple[14] = XCreateBitmapFromData(geDispDev, geRgRoot.self, (char *)pat014,
					stip_w, stip_h);
gePATStipple[15] = XCreateBitmapFromData(geDispDev, geRgRoot.self, (char *)pat015,
					stip_w, stip_h);
gePATStipple[16] = XCreateBitmapFromData(geDispDev, geRgRoot.self, (char *)pat016,
					stip_w, stip_h);
gePATStipple[17] = XCreateBitmapFromData(geDispDev, geRgRoot.self, (char *)pat017,
					stip_w, stip_h);
gePATStipple[18] = XCreateBitmapFromData(geDispDev, geRgRoot.self, (char *)pat018,
					stip_w, stip_h);
gePATStipple[19] = XCreateBitmapFromData(geDispDev, geRgRoot.self, (char *)pat019,
					stip_w, stip_h);
gePATStipple[20] = XCreateBitmapFromData(geDispDev, geRgRoot.self, (char *)pat020,
					stip_w, stip_h);
gePATStipple[21] = XCreateBitmapFromData(geDispDev, geRgRoot.self, (char *)pat021,
					stip_w, stip_h);
gePATStipple[22] = XCreateBitmapFromData(geDispDev, geRgRoot.self, (char *)pat022,
					stip_w, stip_h);

for (i = 23; i < GEMAXPAT; i++) gePATStipple[i] = 0;
}
