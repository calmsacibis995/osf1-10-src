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
static char SccsId[] = "@(#)gealloccolor.c	1.3\t8/7/89";
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
**	GEALLOCCOLOR					Allocates a color
**
**  ABSTRACT:
**
**        This routine will return a pixel value for a given RGB or 
**	  ascii color str.
**
**        It will check for the display device being monochrome and will
**        return either black or white depending on whether the luminance
**        value of the color falls in the lower or upper half of the
**        color range.
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**      GNE 12/16/87 Created
**--
**/

#include "geGks.h"

extern unsigned long geColCr();

unsigned long
geAllocColor(R, G, B, ColName)
unsigned short	R, G, B;
char		*ColName;
{                       
unsigned long		lumi, pixel;
static XColor           Col, ColExact;

/*
 * Get RGB values, if given a color name
 */
if (ColName)
  {geStat = XLookupColor(geDispDev, geCmap, ColName, &ColExact, &Col);
   if (geStat)
     {R = ColExact.red;
      G = ColExact.green;
      B = ColExact.blue;
     }
  }

/*
 * Guess gotta try to get a real color - ColCr is guaranteed to succeed (at the
 *worst it will return either BLACK or WHITE depending on luminance value.
 */
return(geColCr(R, G, B, FALSE));

}

/*
 * Determine number of open slots remaining in the current color map, after
 * releasing all unused colors.
 */
geQueryCMapOpenSlots()
{
#define GE_LOC_MAX_COLS	(1 << 8)

int 		i, NumFree, Freed, CMapLoc[256], depth, ColStatSav;
unsigned long 	pixel;

if ((depth = XDefaultDepth(geDispDev, geScreen)) > GE_LOC_MAX_COLS)
   {/*
     * More than 8 planes, assume can use up to half of it (someday do
     * something better).
     */
    return((depth >> 1));
   }

/*
 * Depth is <= 8, figure out how many slots left by:
 *
 *	1)	Fill up the color map with a dummy collor (BLACK)
 *	2)	Allocate one more dummy color and allow unsed cells to be
 *		freed.
 *	3)	Allocate BLACK (insist on a unique pixel and DON'T allow un-
 *		used cells to be freed)till allocation fails - this will be
 *		indicated by geColCr returning "geBlack.pixel" instead of a
 *		unique pixel
 *	4)	Return number successfully allocated.
 */

/*
 * Fill up map
 */
ColStatSav = geColStat;
geColStat |= GECOL_CMAP_INHIBIT_FREE;
for (NumFree = 0; NumFree < GE_LOC_MAX_COLS; NumFree++)
   {if ((pixel = geColCr(11111, 22222, 33333, TRUE)) == geBlack.pixel)
	 break;
   }
/*
 * Free unused colors
 */
geColStat &= ~GECOL_CMAP_INHIBIT_FREE;
if ((pixel = geColCr(11111, 22222, 33333, TRUE)) == geBlack.pixel) return(2);

/*
 * Fill map again with dummy color, count how many
 */
geColStat |= GECOL_CMAP_INHIBIT_FREE;
for (NumFree = 1; NumFree < GE_LOC_MAX_COLS; NumFree++)
   {if ((pixel = geColCr(11111, 22222, 33333, TRUE)) == geBlack.pixel)
	 break;
   }

geColStat = ColStatSav;

return(NumFree);

}
