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
static char SccsId[] = "@(#)geimgcrx.c	1.3\t8/7/89";
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
**	GEIMGCRX			Reads in a Xbitmap
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
**	GNE 02/08/88 Created     
**
**--
**/

#include "geGks.h"
#include <X11/Xutil.h>


extern char *geMalloc();

XImage *
geImgCrX(FileName)
char  *FileName;
{                                                                         
XImage *ximg;
int    xh,yh;
unsigned int    width,height;
Pixmap pixmap;
int    s;
char  *error_string;

/*
 * This routine currently can only work using an external file as input, it
 * cannot operate on an in-memory buffer.  As such it will kick out if it is
 * either not provided with a filename or it is provided a bogus filename.
 */

if (!FileName || !strlen(FileName)) return(NULL);

ximg = NULL;

geGenFileExt(FileName, geDefProofX, FALSE);

if (!fopen(geUtilBuf, "r"))
 {error_string = (char *) geFetchLiteral("GE_ERR_XOPENIMAGE", MrmRtypeChar8);
  if (error_string != NULL) 
    {sprintf(geErrBuf, error_string, geUtilBuf);
     geError(geErrBuf, FALSE);
     XtFree(error_string);
    }	
   return(NULL);
  }

s = XReadBitmapFile(geDispDev, geRgRoot.self, geUtilBuf, &width, &height,
		    &pixmap, &xh, &yh);
if (s != BitmapSuccess)
 {error_string = (char *) geFetchLiteral("GE_ERR_BADIMAGE", MrmRtypeChar8);
  if (error_string != NULL) 
    {sprintf(geErrBuf, error_string, geUtilBuf);
     geError(geErrBuf, FALSE);
     XtFree(error_string);
    }	
   return(NULL);
  }

ximg = XGetImage(geDispDev, pixmap, 0, 0, width, height, 1, XYPixmap);
XFreePixmap(geDispDev, pixmap);
/*
 * Kludge - 10/24/88 - Can only handle mono images for now.
 */
ximg->format = XYBitmap;
return(ximg);
}
