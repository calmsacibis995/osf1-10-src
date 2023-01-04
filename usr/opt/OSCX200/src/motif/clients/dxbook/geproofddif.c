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
static char SccsId[] = "@(#)geproofddif.c	1.4\t6/12/90";
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
**	GEPROOFDDIF			Writes given window to DDIF type file
**
**  ABSTRACT:
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 10/10/88 Created
**
**--
**/

#include "geGks.h"
#include <X11/Xutil.h>

#ifdef GEISL

#ifdef VMS
#include <img$def.h>
#include <descrip.h>
#else
#include <img/img_def.h>
#endif

#endif

extern char *geMalloc();
extern XImage *geXImgToSing();

geProofDDIF(Win, ImgPtr, FileName, x, y, width, height)
Window       Win;
XImage       *ImgPtr;
char         *FileName;
int          x, y;
unsigned int width, height;
{                                                                         
#ifdef GEDDIF
#ifdef GEISL

unsigned char           *Buf;
int                     fid, ctx, format, n_bytes, cnt, depth, cropw, croph;
XImage                  *TImgPtr;
#ifdef VMS
struct dsc$descriptor_s fdesc;
#endif

cropw = width;
croph = height;

geGenFileExt(FileName, geDefProofDDIF, FALSE);

depth = XDefaultDepth(geDispDev, geScreen);
format = XYPixmap;

/*
 * It is necessary that the image width be byte aligned - it is expected
 * that the window or passed in image is large enough for this.
 */
width = ((width + 7) >> 3) << 3;
if (!ImgPtr)
     {if (!(ImgPtr = TImgPtr = XGetImage(geDispDev, Win, x, y, width, height,
				         AllPlanes, ZPixmap)))
     	return(NULL);
     }
/*
  {if (depth == 1)

     {if (!(ImgPtr = TImgPtr = XGetImage(geDispDev, Win, x, y, width, height, 1,
					format)))
       	return(NULL);
     }
   
   else
     {if (!(ImgPtr = TImgPtr = XGetImage(geDispDev, Win, x, y, width, height,
				         AllPlanes, ZPixmap)))
     	return(NULL);
     }
  }

*/

else
  TImgPtr = NULL;

if (ImgPtr->depth > 1)
  {ImgPtr = geXImgToSing(ImgPtr);
   if (TImgPtr)
     {geXDestImg(TImgPtr);
      TImgPtr = ImgPtr;
     }

   if (!ImgPtr) return(NULL);
  }

ImgPtr->byte_order       = LSBFirst;
ImgPtr->bitmap_bit_order = LSBFirst;
/*
 * Kludge - have to invert image for some reason
 */
cnt = 0;
Buf = (unsigned char *)ImgPtr->data;
n_bytes = ImgPtr->bytes_per_line * ImgPtr->height;
while (cnt++ < n_bytes)
	{*Buf = ~*Buf;
	 Buf++;
        }

#ifdef VMS

fdesc.dsc$w_length  = strlen(FileName);
fdesc.dsc$a_pointer = FileName;
fdesc.dsc$b_class   = DSC$K_CLASS_S;
fdesc.dsc$b_dtype   = DSC$K_DTYPE_T;
ctx = ImgOpenDDIFFile(IMG$K_MODE_EXPORT,&fdesc,0,0);
#else
ctx = ImgOpenDDIFFile(IMG$K_MODE_EXPORT,strlen(FileName),FileName,0,0,0);
#endif

geXimageToFid(ImgPtr, &fid, cropw, croph);
ImgExportDDIFFrame(fid,0,ctx,0,0,0,0,0);

ImgCloseDDIFFile(ctx, 0);

ImgDeleteFrame(fid);

/*
 * Kludge - reinvert image now
 */
if (ImgPtr != TImgPtr)
   {cnt = 0;
    Buf = (unsigned char *)ImgPtr->data;
    n_bytes = ImgPtr->bytes_per_line * ImgPtr->height;
    while (cnt++ < n_bytes)
	{*Buf = ~*Buf;
	 Buf++;
        }
   }

/*
 * Release image resources
 */
if (TImgPtr)
  geXDestImg(TImgPtr);

#endif
#endif

}
