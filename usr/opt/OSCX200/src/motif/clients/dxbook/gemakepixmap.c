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
/* DEC/CMS REPLACEMENT HISTORY, Element GEMAKEPIXMAP.C*/
/* *3    25-JAN-1991 17:00:11 FITZELL "V3_EFT_24_JAN"*/
/* *2    12-DEC-1990 12:39:52 FITZELL "V3 IFT update"*/
/* *1     8-NOV-1990 11:24:05 FITZELL "V3 IFT"*/
/* DEC/CMS REPLACEMENT HISTORY, Element GEMAKEPIXMAP.C*/
static char SccsId[] = "%W%\t%G%";
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
**	GEMAKEPIXMAP				Creates a pixmap
**
**  ABSTRACT:
**
** Note:	This is courtesy of Tom Woodburn.
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	TW 11/19/87 Created
**
**--
**/

#include "geGks.h"
#include <Xatom.h>
#include <Xutil.h>

extern char   *geMalloc();
extern XImage *geXCrImg();

Pixmap 
geMakePixmap(root, data, width, height, depth)
	Drawable	root;
	char *		data;
	int		width;
	int		height;
	int		depth;
{
	char *		buff;
	Pixmap		pxmap;
	XGCValues	gcValues;
	unsigned int	valueMask;
	GC		scratchGC;
	int		offset, buffsize;
	int		xpad;
	int		bytesPerLine;
	XImage *	pImage;

	pxmap = XCreatePixmap(geDispDev, root, width, height, depth);
	gcValues.background = WhitePixel(geDispDev, 0);
	gcValues.foreground = BlackPixel(geDispDev, 0);

	valueMask = GCForeground | GCBackground;

	scratchGC = XCreateGC(geDispDev, pxmap, valueMask, &gcValues);

	offset = 0;
	xpad = 8;
	bytesPerLine = ((width + 15) / 16) * 2;
	buffsize = height * bytesPerLine;
	buff         = geMalloc(buffsize);
	memcpy(buff, data, buffsize);
	pImage = geXCrImg(geDispDev, DefaultVisual(geDispDev, geScreen),
			  1, XYBitmap, offset,	buff, width, height,
			  xpad, bytesPerLine);

	XPutImage(geDispDev, pxmap, scratchGC, pImage, 0, 0, 0, 0,
		  width, height);

	geXDestImg(pImage);
	XFreeGC(geDispDev, scratchGC);

	return(pxmap);

}
