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
static char SccsId[] = "@(#)geprooffse.c	1.10\t8/7/89";
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
**	GEPROOFFSE			Writes given window to FSE bin file
**
**  ABSTRACT:
**
**  Note:  The definition of the FSE format requires that the written image
**         data be 16 bit aligned with a minimum width of 20 bytes - 160
**         pixels.  This routine requires that the window from which the
**         image is to be extracted (or the image passed in) be large
**         enough to meet these requirements.
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 07/25/88 Created
**
**--
**/

#include "geGks.h"
#include <X11/Xutil.h>

extern XImage *geXImgToSing();

geProofFSE(Win, ImgPtr, FileName, x, y, width, height)
Window       Win;
XImage       *ImgPtr;
char         *FileName;
int          x, y;
unsigned int width, height;
{                                                                         
#define MAXACCEPTABLE 1000000
char   *p, t[255],t2[255];
int    bpl, i;
Window root_win;
unsigned int border, depth, cropw;
char	*error_string;

union l_word				/* allows the reading of a longword */
  {					/* in 4 one byte chunks, and access */
  unsigned char  lw_char[4];		/* as an acutal longword            */
  unsigned long  lw_num;
  };

struct flags_bits			/* flag bits from the CSE header */
  {
  unsigned b0:1;
  unsigned b1:1;
  unsigned b2:1;
  unsigned b3:1;
  unsigned b4:1;
  unsigned b5:1;
  unsigned b6:1;
  unsigned b7:1;
  };

union flags_byte
  {
  unsigned char  chr;
  struct flags_bits  bits;
  };

struct CSE_image			/* the CSE image header */
  {
  char  tag[3];
  char  data_fmt;
  char  orient;
  union l_word  num_pix;
  union l_word  num_lines;
  union l_word  scan_density;
  char  scale_cnt;
  union flags_byte  flags;
  unsigned int  num_bytes;
  unsigned char  *bitmap;
  };

  int              n_bytes;
  unsigned char    *cur_bitmap, *Buf;
  register         cnt;
  struct CSE_image img;
  FILE             *outfile;
  XImage           *TImgPtr;

cropw = width;
if (width < 160) width = 160;
else
  width = ((width + 15) >> 4) << 4;

if (!ImgPtr)
  {if (!(ImgPtr = TImgPtr = XGetImage(geDispDev, Win, x, y, width, height,
				      AllPlanes, ZPixmap)))
     return(NULL);
  }
else
  TImgPtr = NULL;

if (ImgPtr->depth > 1 || ImgPtr->bits_per_pixel != 1)
  {ImgPtr = geXImgToSing(ImgPtr);
   if (TImgPtr)
     {geXDestImg(TImgPtr);
      TImgPtr = ImgPtr;
     }

   if (!ImgPtr) return(NULL);
  }

geGenFileExt(FileName, geDefProofFSE, FALSE);
n_bytes = width >> 3;
sprintf(t, "mrs=%d\0", n_bytes);
sprintf(t2,"alq=%d\0", ((n_bytes * height) >> 9));

if (!(outfile = fopen(geUtilBuf,"w",t,t2,"rfm=fix")))
  {error_string = (char *) geFetchLiteral("GE_ERR_XOPEN", MrmRtypeChar8);
   if (error_string != NULL) 
     {sprintf(geErrBuf, error_string, geUtilBuf);
      geError(geErrBuf, FALSE);
      XtFree(error_string);
     }
   return(NULL);
  }

bpl                     = n_bytes;
img.num_pix.lw_num      = width;
img.num_lines.lw_num    = height;
img.scan_density.lw_num = (int)((float)XDisplayWidth  (geDispDev, geScreen) /
			       ((float)XDisplayWidthMM(geDispDev, geScreen) /
				 25.4));
img.scale_cnt           = 0;
img.flags.chr           = 0;
if (cropw != width)
  img.flags.bits.b0     = 1;
fprintf(outfile,"CSENP");
p                       = (char *)&img.num_pix.lw_num;
fputc(*p++, outfile); fputc(*p++, outfile);
fputc(*p++, outfile); fputc(*p  , outfile);
p                       = (char *)&img.num_lines.lw_num;
fputc(*p++, outfile); fputc(*p++, outfile);
fputc(*p++, outfile); fputc(*p  , outfile);
p                       = (char *)&img.scan_density.lw_num;
fputc(*p++, outfile); fputc(*p++, outfile);
fputc(*p++, outfile); fputc(*p  , outfile);
fputc(img.scale_cnt, outfile);
fputc(img.flags.chr, outfile);
if (img.flags.bits.b0)
  fputc((unsigned char)(width - cropw), outfile);
else
  fputc(0x00, outfile);

/*
 * Pad rest of header
 */
if (bpl > 20)
  for (i = 21; i <= bpl; i++)
    fputc(0x00, outfile);

n_bytes = bpl * height;
/*
 * Kludge - have to invert image for some reason
 */
cnt = 0;
Buf = (unsigned char *)ImgPtr->data;
while (cnt++ < ImgPtr->bytes_per_line * ImgPtr->height)
	{*Buf = ~*Buf;
	 Buf++;
        }
/*
 * Write the image data 1 cropped line at a time
 */
p = ImgPtr->data;
i = 0;
while (i < height)
  {fwrite(p, bpl, 1, outfile);
   p += ImgPtr->bytes_per_line;
   i++;
  }
/*
 * Kludge - reinvert image now
 */
if(ImgPtr != TImgPtr)
   {cnt = 0;
    Buf = (unsigned char *)ImgPtr->data;
    while (cnt++ < ImgPtr->bytes_per_line * ImgPtr->height)
	{*Buf = ~*Buf;
	 Buf++;
        }
   }
/*
 * Release image resources
 */
if (TImgPtr)
  geXDestImg(TImgPtr);

fclose(outfile);

}




