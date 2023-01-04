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
static char SccsId[] = "@(#)gefidtoximage.c	1.8\t8/29/89";
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
**	FIDTOXIMAGE			Gets an Ximg struct from an ISL fid
**
**  ABSTRACT:
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 01/20/89 Created (compliments of Jeff Orthober)
**
**--
**/

#include "geGks.h"

#ifdef GEISL

#ifdef VMS
#include <img$def.h>
#else
#include <img/img_def.h>
#endif

#endif

extern char   *geMalloc();
extern XImage *geXCrImg();
extern XImage *geXImgToSing(), *geXImgTcToSing(), *geXImgTcToCMap();

geFidToXimage(timage, fid)
	XImage       **timage;
	int	     fid;
{
char	*error_string;

#ifdef GEISL

	char *ptr;
	int i, stride, itmlst[41], nuitmlst[41], nufid, compress, dummy, size,
            components, polarity, pixel_stride;
	XImage *image, *ImgPtr, *TImgPtr;

	image = ImgPtr = TImgPtr = NULL;
	nufid = 0;

        image = geXCrImg(geDispDev, DefaultVisual(geDispDev, geScreen),
			 1,XYBitmap,0,0,0,0,8,0);

	itmlst[0]  = IMG$_SCANLINE_STRIDE;
	itmlst[1]  = 4;
	itmlst[2]  = (int)&stride;
	itmlst[3]  = (int)&dummy;
	itmlst[4]  = 0;

	itmlst[5]  = IMG$_NUMBER_OF_LINES;
	itmlst[6]  = 4;
	itmlst[7]  = (int)&image->height;
	itmlst[8]  = (int)&dummy;
	itmlst[9]  = 0;

	itmlst[10] = IMG$_PIXELS_PER_LINE;
	itmlst[11] = 4;
	itmlst[12] = (int)&image->width;
	itmlst[13] = (int)&dummy;
	itmlst[14] = 0;

	itmlst[15] = IMG$_BITS_PER_PIXEL;
	itmlst[16] = sizeof(int);
	itmlst[17] = (int)&image->bits_per_pixel;
	itmlst[18] = (int)&dummy;
	itmlst[19] = 0;

	itmlst[20] = IMG$_PIXEL_STRIDE;
	itmlst[21] = sizeof(int);
	itmlst[22] = (int)&pixel_stride;
	itmlst[23] = (int)&dummy;
	itmlst[24] = 0;

	itmlst[25] = IMG$_NUMBER_OF_COMP;
	itmlst[26] = sizeof(int);
	itmlst[27] = (int)&components;
	itmlst[28] = (int)&dummy;
	itmlst[29] = 0;

	itmlst[30] = IMG$_BRT_POLARITY;
	itmlst[31] = sizeof(int);
	itmlst[32] = (int)&polarity;
	itmlst[33] = (int)&dummy;
	itmlst[34] = 0;

	itmlst[35] = IMG$_COMPRESSION_TYPE;
	itmlst[36] = 4;
	itmlst[37] = (int)&compress;
	itmlst[38] = (int)&dummy;
	itmlst[39] = 0;
	itmlst[40] = 0;

	fid = ImgGetFrameAttributes(fid,itmlst);

	image->bytes_per_line = stride >> 3;

	if (compress != IMG$K_PCM_COMPRESSION) 
	  {fid = ImgDecompress(fid);
	   fid = ImgGetFrameAttributes(fid,itmlst);
	  }

	if (image->bits_per_pixel != 1)
	   {if ((image->depth = (pixel_stride / components)) > 8)
		{/*
		  * Can't handle it
	       	  */
	         error_string = (char *) geFetchLiteral("GE_ERR_DDIFNOTBITONAL", MrmRtypeChar8);
	         if (error_string != NULL) 
	  	   {geError(error_string, FALSE);
	  	    XtFree(error_string);
	  	   }
	   	 *timage = NULL;
	   	 return;
	   	}

	    ImgPtr = geXCrImg(geDispDev, DefaultVisual(geDispDev, geScreen),
			 image->depth,ZPixmap,0,0,0,0,8,0);

	    ImgPtr->height 	   = image->height;
	    ImgPtr->width 	   = image->width;
	    ImgPtr->bits_per_pixel = image->bits_per_pixel;
	    ImgPtr->depth 	   = image->depth;
	    ImgPtr->bytes_per_line = image->bytes_per_line;

	    size         	   = ImgPtr->bytes_per_line * ImgPtr->height;
	    ImgPtr->data  	   = geMalloc(size);

	    ImgExportBitmap(fid,0,ImgPtr->data,size,0,0,0,0);

 	    if (geRun.VoyerCalling)
	    	{*timage = ImgPtr;

	   	 if ((GEMONO) || components != 3 ||
		     geVisual->class != PseudoColor || geDispChar.Depth > 8)
	    	     TImgPtr = geXImgTcToSing(ImgPtr, components, polarity);
	    	 else
	    	     TImgPtr = geXImgTcToCMap(ImgPtr, components, polarity);
		}
	    else
	         TImgPtr = geXImgTcToSing(ImgPtr, components, polarity);

	    if (ImgPtr)
		{geXDestImg(ImgPtr);
		 ImgPtr  = TImgPtr;
		 TImgPtr = NULL;
	        }

	    if (image)
		{geXDestImg(image);
		 image = NULL;
		}
	    *timage = ImgPtr;

	   }
	else
	   {if (image->bits_per_pixel == 1 &&
		(image->bytes_per_line << 3) != image->width)
	  	{image->bytes_per_line = (image->width + 7) >> 3;
	   	 stride 		 = image->bytes_per_line << 3;
	  	 nuitmlst[0]           = IMG$_SCANLINE_STRIDE;
	  	 nuitmlst[1]           = 4;
	  	 nuitmlst[2]           = (int)&stride;
	  	 nuitmlst[3]           = 0;
	  	 nuitmlst[4]           = 0;

	  	 nufid                 = ImgCopy(fid, 0, nuitmlst);
	  	 fid                   = nufid;
	 	}
	    else
	  	nufid                  = 0;

	    size         = image->bytes_per_line * image->height;
	    image->data  = geMalloc(size);

	    ImgExportBitmap(fid,0,image->data,size,0,0,0,0);

	    if (image->bits_per_pixel == 1 &&
		polarity != IMG$K_ZERO_MAX_INTENSITY)
		{ptr = image->data;
		 for (i = 0; i < size; i++)
		    {*ptr = *ptr ^ 255;
		      ptr++;
		    }
		}
	    *timage = image;
	   }

	if (nufid)
	  ImgDeleteFrame(nufid);

#endif

}
