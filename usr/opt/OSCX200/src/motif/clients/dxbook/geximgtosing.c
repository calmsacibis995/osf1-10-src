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
static char SccsId[] = "@(#)geximgtosing.c	1.4\t10/31/89";
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
**	XIMGTOSING			Converts the Ximg to single plane
**
**  ABSTRACT:
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 05/10/89 Created (compliments of Jeff Orthober)
**
**--
**/

#include "geGks.h"
#include <math.h>

#ifdef GEISL

#ifdef VMS
#include <img$def.h>
#else
#include <img/img_def.h>
#endif

#endif


extern XImage 		*geXCrImg();
extern char   		*geMalloc();
extern unsigned long 	geAllocColor();

static char		*error_string;

XImage *
geXImgToSing(image)
	XImage *image;
{
	int    size,x,y,i,tobase, numcols, depth;
	XImage *timage;
	unsigned char *pntrf,*pntrt;
	int black[256];
	int blackcnt;
	XColor colortable[256];

	if ((depth = XDefaultDepth(geDispDev, geScreen)) > 8 ||
	    (image->depth < 4 || image->depth > 8 ||
	     image->bits_per_pixel < 4 || image->bits_per_pixel > 8))
	  {error_string = (char *) geFetchLiteral("GE_ERR_UNAVAILABLE", MrmRtypeChar8);
           if (error_string != NULL) 
	     {geError(error_string, FALSE);
	      XtFree(error_string);
	     }
	   return(NULL);
	  }

	numcols = 1 << depth;

        timage = geXCrImg(geDispDev, geVisual,
			  1, XYBitmap, 0, NULL, image->width, image->height,
			  8, 0);

	size = timage->bytes_per_line * timage->height;

	timage->data = geMalloc(size);

	memset(timage->data, 255, size);

       for (i = 0; i < numcols; i++)
	 colortable[i].pixel = i;

       XQueryColors(geDispDev, geCmap, colortable,
		    min(numcols,geNumCols));

	for (i = 0; i < 256; i++)
		black[i] = FALSE;

	/* each color R, G, or B ranges within and including 0 to 65535 */

	for (i = 0; i < numcols; i++) {
	  if (colortable[i].red + colortable[i].green + colortable[i].blue
	      < 95300) {
		/* Total R+G+B < 1/2 maximum sum of R+G+B */
		blackcnt = blackcnt + 1;
		black[i] = TRUE;
	      }
	}

	if (blackcnt == -1)
	  {error_string = (char *) geFetchLiteral("GE_ERR_UNKNOWN", MrmRtypeChar8);
           if (error_string != NULL) 
	     {geError(error_string, FALSE);
	      XtFree(error_string);
	     }
	   geXDestImg(timage);
	   return(NULL);
	  }

	for (y = 0; y < timage->height; y++) {
		pntrf = (unsigned char *)image->data +
		  image->bytes_per_line * y;
		pntrt = (unsigned char *)timage->data +
		  timage->bytes_per_line * y;
		tobase = 1;
		for (x = 0; x < timage->width; x++) {
			
			if (black[*pntrf]) {
/*				*pntrt = *pntrt | tobase; */
				*pntrt = *pntrt & (~tobase);
			}
			pntrf++;
			tobase = tobase * 2;
			if (tobase > 128) {
				tobase = 1;
				pntrt++;
			}
		}
	}

	return(timage);
}

/*
 * Convert color image to single plane using supplied colortable
 */
XImage *
geXImgToSingConv(image, colortable)
	XImage *image;
	XColor colortable[256];
{
	int    size,x,y,i,tobase, numcols, depth;
	XImage *timage;
	unsigned char *pntrf,*pntrt;
	int black[256];
	int blackcnt;

	if ((depth = XDefaultDepth(geDispDev, geScreen)) > 8 ||
	    (image->depth < 4 || image->depth > 8 ||
	     image->bits_per_pixel < 4 || image->bits_per_pixel > 8))
	  {error_string = (char *) geFetchLiteral("GE_ERR_UNAVAILABLE", MrmRtypeChar8);
           if (error_string != NULL) 
	     {geError(error_string, FALSE);
	      XtFree(error_string);
	     }
	   return(NULL);
	  }

	numcols = 1 << depth;

        timage = geXCrImg(geDispDev, geVisual,
			  1, XYBitmap, 0, NULL, image->width, image->height,
			  8, 0);

	size = timage->bytes_per_line * timage->height;

	timage->data = geMalloc(size);

	memset(timage->data, 255, size);

	for (i = 0; i < 256; i++)
		black[i] = FALSE;

	/* each color R, G, or B ranges within and including 0 to 65535 */

	for (i = 0; i < numcols; i++) {
	  if (colortable[i].red + colortable[i].green + colortable[i].blue
	      < 95300) {
		/* Total R+G+B < 1/2 maximum sum of R+G+B */
		blackcnt = blackcnt + 1;
		black[i] = TRUE;
	      }
	}

	if (blackcnt == -1)
	  {error_string = (char *) geFetchLiteral("GE_ERR_UNKNOWN", MrmRtypeChar8);
           if (error_string != NULL) 
	     {geError(error_string, FALSE);
	      XtFree(error_string);
	     }
	   geXDestImg(timage);
	   return(NULL);
	  }

	for (y = 0; y < timage->height; y++) {
		pntrf = (unsigned char *)image->data +
		  image->bytes_per_line * y;
		pntrt = (unsigned char *)timage->data +
		  timage->bytes_per_line * y;
		tobase = 1;
		for (x = 0; x < timage->width; x++) {
			
			if (black[*pntrf]) {
/*				*pntrt = *pntrt | tobase; */
				*pntrt = *pntrt & (~tobase);
			}
			pntrf++;
			tobase = tobase * 2;
			if (tobase > 128) {
				tobase = 1;
				pntrt++;
			}
		}
	}

	return(timage);
}

/*
 * Convert a color mapped image (with private color map) to monochrome
 */
XImage *
geXImgCMapToSing(image, colortable)
	XImage 	*image;
	XColor 	colortable[256];
{
	int    size,x,y,i,tobase, numcols;
	XImage *timage;
	unsigned char *pntrf,*pntrt;
	int black[256];
	int blackcnt;

	if (geDispChar.Depth > 8 ||
	    (image->depth < 4 || image->depth > 8 ||
	     image->bits_per_pixel < 4 || image->bits_per_pixel > 8))
	  {error_string = (char *) geFetchLiteral("GE_ERR_UNAVAILABLE", MrmRtypeChar8);
           if (error_string != NULL) 
	     {geError(error_string, FALSE);
	      XtFree(error_string);
	     }
	   return(NULL);
	  }

	numcols = 1 << image->depth;

        timage = geXCrImg(geDispDev, geVisual, 1, XYBitmap, 0, NULL,
			  image->width, image->height, 8, 0);

	size = timage->bytes_per_line * timage->height;

	timage->data = geMalloc(size);

	memset(timage->data, 255, size);

	for (i = 0; i < 256; i++)
		black[i] = FALSE;

	/* each color R, G, or B ranges within and including 0 to 65535 */

	for (i = 0; i < numcols; i++) {
	  if (colortable[i].red + colortable[i].green + colortable[i].blue
	      < 95300) {
		/* Total R+G+B < 1/2 maximum sum of R+G+B */
		blackcnt = blackcnt + 1;
		black[i] = TRUE;
	      }
	}

	if (blackcnt == -1)
	  {error_string = (char *) geFetchLiteral("GE_ERR_UNKNOWN", MrmRtypeChar8);
           if (error_string != NULL) 
	     {geError(error_string, FALSE);
	      XtFree(error_string);
	     }
	   geXDestImg(timage);
	   return(NULL);
	  }

	for (y = 0; y < timage->height; y++) {
		pntrf = (unsigned char *)image->data +
		  image->bytes_per_line * y;
		pntrt = (unsigned char *)timage->data +
		  timage->bytes_per_line * y;
		tobase = 1;
		for (x = 0; x < timage->width; x++) {
			
			if (black[*pntrf]) {
/*				*pntrt = *pntrt | tobase; */
				*pntrt = *pntrt & (~tobase);
			}
			pntrf++;
			tobase = tobase * 2;
			if (tobase > 128) {
				tobase = 1;
				pntrt++;
			}
		}
	}

	return(timage);
}

/*
 * Converts true color and gray scale image to monochrome
 */
XImage *
geXImgTcToSing(image, components, polarity)
	XImage *image;
	int components, polarity;
{
/*
 * 1 component error distribution macro
 */
#define GE_ERR_DIST_1 \
   {val = (int)*pntrf + err38.l; \
    val = val < 0 ? 0 : (val > maxlum ? maxlum: val); \
    *pntrf = (unsigned char)val; \
					\
    val = (int)*pntrd + err38.l; \
    val = val < 0 ? 0 : (val > maxlum ? maxlum: val); \
    *pntrd = (unsigned char)val; \
					\
    pntrd += components; \
					\
    val = (int)*pntrd + err14.l; \
    val = val < 0 ? 0 : (val > maxlum ? maxlum: val); \
    *pntrd = (unsigned char)val; \
   }

/*
 * 3 component error distribution macro
 */
#define GE_ERR_DIST_3 \
   {val = (int)*pntrf + err38.r; \
    val = val < 0 ? 0 : (val > maxlum ? maxlum: val); \
    *pntrf = (unsigned char)val; \
    val = (int)*(pntrf + 1) + err38.g; \
    val = val < 0 ? 0 : (val > maxlum ? maxlum: val); \
    *(pntrf + 1) = (unsigned char)val; \
    val = (int)*(pntrf + 2) + err38.b; \
    val = val < 0 ? 0 : (val > maxlum ? maxlum: val); \
    *(pntrf + 2) = (unsigned char)val; \
					\
    val = (int)*pntrd + err38.r; \
    val = val < 0 ? 0 : (val > maxlum ? maxlum: val); \
    *pntrd = (unsigned char)val; \
    val = (int)*(pntrd + 1) + err38.g; \
    val = val < 0 ? 0 : (val > maxlum ? maxlum: val); \
    *(pntrd + 1) = (unsigned char)val; \
    val = (int)*(pntrd + 2) + err38.b; \
    val = val < 0 ? 0 : (val > maxlum ? maxlum: val); \
    *(pntrd + 2) = (unsigned char)val; \
					\
    pntrd += components; \
					\
    val = (int)*pntrd + err14.r; \
    val = val < 0 ? 0 : (val > maxlum ? maxlum: val); \
    *pntrd = (unsigned char)val; \
    val = (int)*(pntrd + 1) + err14.g; \
    val = val < 0 ? 0 : (val > maxlum ? maxlum: val); \
    *(pntrd + 1) = (unsigned char)val; \
    val = (int)*(pntrd + 2) + err14.b; \
    val = val < 0 ? 0 : (val > maxlum ? maxlum: val); \
    *(pntrd + 2) = (unsigned char)val; \
   }

struct	GE_IMG_ERR
	{int	r;
	 int	g;
	 int	b;
	 int	l;
	};

	struct	GE_IMG_ERR	err14, err38;
	int    size,x,y,i,tobase, numcols, depth, hlim, wlim;
	XImage *timage;
	register unsigned char *pntrf, *pntrt, *pntrd;
	register int val;
	unsigned int lumi, thres, maxlum;

	maxlum = (1 << (image->bits_per_pixel / components)) - 1;
	thres = ((1 << (image->bits_per_pixel / components)) * components) >> 1;

        timage = geXCrImg(geDispDev, geVisual,
			  1, XYBitmap, 0, NULL, image->width, image->height,
			  8, 0);

	size = timage->bytes_per_line * timage->height;

	timage->data = geMalloc(size);

#ifdef GEISL
	if (polarity == IMG$K_ZERO_MAX_INTENSITY)
	    memset(timage->data, 255, size);
	else
	    memset(timage->data, 0, size);
#else
	    memset(timage->data, 255, size);
#endif
	/*
 	 * Lines 1 -> (h-1)
	 */
	hlim = timage->height - 1;
	wlim = timage->width - 1;
	for (y = 0; y < hlim; y++) {
		pntrf = (unsigned char *)image->data +
		  image->bytes_per_line * y;
		pntrt = (unsigned char *)timage->data +
		  timage->bytes_per_line * y;
		tobase = 1;

		/*
		 * Pixel # 1
		 */
		if (components == 3)
		   {lumi = *pntrf + *(pntrf + 1) + *(pntrf + 2);
		    pntrf += components;
		   }
		else
		    lumi = *pntrf++;

#ifdef GEISL
		if (lumi < thres) {
		    if (polarity == IMG$K_ZERO_MAX_INTENSITY)
			*pntrt = *pntrt & (~tobase);
		    else
			*pntrt = *pntrt | tobase;
		}
#else
			*pntrt = *pntrt & (~tobase);
#endif
		tobase <<= 1;
		if (tobase > 128) {
			tobase = 1;
			pntrt++;
		}
	 	/*
		 * Pixels 2 -> (w-1)
		 */	
 		for (x = 1; x < wlim; x++) {
			if (components == 3)
			   {/*
			     * Calculate luminance (3X, actually), applying
			     * NTS correction of  .3R,  .59G, .11B - 3X the
			     * corrections =      .9R, 1.77G, .33B.  The
			     * closest approximations to these, which allow
			     * for the resultant to be calculated through
			     * bit shifting rather than division, are:
			     * R: .9 	~= 29/32  ~= .90625  (/3 ~= .3020833)
			     * G: 1.77 	~= 14/8   ~= 1.75    (/3 ~= .5833333)
			     * B: .33	~= 21/64  ~= .328125 (/3 ~= .109375)
			     */
			    lumi =  ((((err14.r = (int)*pntrf) * 29) >> 5) +
				     (((err14.g = (int)*(pntrf + 1)) * 14) >> 3) +
				     (((err14.b = (int)*(pntrf + 2)) * 21) >> 6));

			    if (lumi > thres)
			   	{err14.r   = *pntrf - maxlum;
			   	 err14.g   = *(pntrf + 1) - maxlum;
			   	 err14.b   = *(pntrf + 2) - maxlum;
				}

			   err14.r >>= 2;
			   err14.g >>= 2;
			   err14.b >>= 2;

			   err38.r = err14.r + (err14.r >> 1);
			   err38.g = err14.g + (err14.g >> 1);
			   err38.b = err14.b + (err14.b >> 1);

			   }
			else
			   {lumi = err14.l = (int)*pntrf;
			    if (lumi > thres)
				{err14.l = *pntrf - maxlum;
				}
			
			    err14.l >>= 2;
			    err38.l   = err14.l + (err14.l >> 1);
			   }

			pntrd   = pntrf + image->bytes_per_line;
			pntrf  += components;
			/*
			 * Map the pixel to BLACK or WHITE and distribute
			 * the resultant error to the neighboring pixels
			 */
#ifdef GEISL
			if (lumi < thres) {
			    if (polarity == IMG$K_ZERO_MAX_INTENSITY)
				*pntrt = *pntrt & (~tobase);
			    else
				*pntrt = *pntrt | tobase;
			}
#else
				*pntrt = *pntrt & (~tobase);
#endif
			/*
			 * Adjust neighboring pixel(s)
			 */
			if (components == 3)
			   {GE_ERR_DIST_3;}
			else
			   {GE_ERR_DIST_1;}

			tobase <<= 1;
			if (tobase > 128) {
				tobase = 1;
				pntrt++;
			}
		}
		/*
		 * Last pixel
		 */
		if (components == 3)
		   {lumi = *pntrf + *(pntrf + 1) + *(pntrf + 2);
		   }
		else
		    lumi = *pntrf++;

#ifdef GEISL
		if (lumi < thres) {
		    if (polarity == IMG$K_ZERO_MAX_INTENSITY)
			*pntrt = *pntrt & (~tobase);
		    else
			*pntrt = *pntrt | tobase;
		}
#else
			*pntrt = *pntrt & (~tobase);
#endif
	}

	/*
 	 * Last line
	 */
	pntrf = (unsigned char *)image->data +
	  image->bytes_per_line * y;
	pntrt = (unsigned char *)timage->data +
	  timage->bytes_per_line * y;
	tobase = 1;

	for (x = 0; x < timage->width; x++) {
		if (components == 3)
		   {lumi = *pntrf + *(pntrf + 1) + *(pntrf + 2);
		    pntrf += components;
		   }
		else
		    lumi = *pntrf++;
		if (lumi < thres) {
#ifdef GEISL
		    if (polarity == IMG$K_ZERO_MAX_INTENSITY)
			*pntrt = *pntrt & (~tobase);
		    else
			*pntrt = *pntrt | tobase;
#else
			*pntrt = *pntrt & (~tobase);
#endif
		}

		tobase <<= 1;
		if (tobase > 128) {
			tobase = 1;
			pntrt++;
		}
	}

	return(timage);
}

/*
 * Converts true color and gray scale image to monochrome
 */
XImage *
geXImgTcToCMap(image, components, polarity)
XImage *image;
int components, polarity;
{
int    			x, y, size;
register unsigned char *pntr_in, *pntr_out;
unsigned int 		maxlum, maxlum_reduced, xr, xg, xb;

XImage *timage;

maxlum = (1 << (image->bits_per_pixel / components)) - 1;
/*
 * Take a guess as to a good reduction value
 */
switch (geNumCols)
   {case 2:
	maxlum_reduced = 2;
	break;
    case 16:
	maxlum_reduced = 3;
	break;
    case 256:
	maxlum_reduced = 8;
	break;
    default:
	maxlum_reduced = (int)sqrt(geNumCols);
	break;
   }

maxlum_reduced = maxlum_reduced > maxlum ? maxlum : maxlum_reduced;

timage = geXCrImg(geDispDev, geVisual,
	 	  XDefaultDepth(geDispDev, geScreen), ZPixmap, 0,
		  NULL, image->width, image->height, 8, 0);

size = timage->bytes_per_line * timage->height;

timage->data = geMalloc(size);
/*
 * Fit image into available space in the color map
 */
for (;;)
   {geReduceColImg(image, timage, maxlum, maxlum_reduced, components);
    if (!geColStat) break;		/* DONE				     */
    else
	{maxlum_reduced--;		/* DIDN'T fit, try lower resolution  */
	 if (maxlum_reduced >= 2)
	    {
#ifdef GERAGS
	     geMnStat(GEDISP,   86);	/* Put up message - doing it */
   	     geSetWaitCurs(TRUE);
  	     XFlush(geDispDev);
#endif
	     continue;			/* Try again			     */
	    }
	 else
	    {geXDestImg(timage);	/* No more space left, forget it     */
	     timage = NULL;
	     break;
	    }
	}
   }

return(timage);
}

/*
 * Fit image into given space
 */
geReduceColImg(image, timage, maxlum, maxlum_reduced, components)
XImage		*image, *timage;
unsigned int 	maxlum, maxlum_reduced;
int		components;

{
register int		x, y, size;
register unsigned char 	*pntr_in, *pntr_out;
unsigned int 		xr, xg, xb;

if (maxlum == maxlum_reduced)
  {/*
    * Lines
    */
   for (y = 0; y < timage->height; y++)
   	{pntr_in  = (unsigned char *)image->data + image->bytes_per_line * y;
   	 pntr_out = (unsigned char *)timage->data + timage->bytes_per_line * y;
   	 /*
    	  * Pixels
    	  */	
   	 for (x = 0; x < timage->width; x++)
	    {if (components == 3)
	    	{/*
	     	  * Calculate X colors
	    	  */
	    	 xr = ((unsigned int)*pntr_in       * GEMAXUSHORT) / maxlum_reduced;
	    	 xg = ((unsigned int)*(pntr_in + 1) * GEMAXUSHORT) / maxlum_reduced;
	    	 xb = ((unsigned int)*(pntr_in + 2) * GEMAXUSHORT) / maxlum_reduced;
	

	 	 /*
	     	  * Allocate color
	     	  */
		 geColStat = 0;
	    	 *pntr_out++ = (unsigned char)geAllocColor((unsigned short)xr, (unsigned short)xg, (unsigned short)xb, FALSE);
		 if (geColStat) return;		/* Ran out of cells, quit    */
	    	 pntr_in    += components;
	    	}				/* END if color		     */
	    }					/* END PIXEL loop	     */
   	}					/* END LINE  loop	     */
   }						/* END if(maxlum==maxlum_red)*/
else
   {/*
     * Lines
     */
    for (y = 0; y < timage->height; y++)
   	{pntr_in  = (unsigned char *)image->data + image->bytes_per_line * y;
    	 pntr_out = (unsigned char *)timage->data + timage->bytes_per_line * y;
    	 /*
    	  * Pixels
    	  */	
    	 for (x = 0; x < timage->width; x++)
	    {if (components == 3)
	    	{/*
	      	  * Calculate X colors
	     	  */
	    	 xr = ((unsigned int)*pntr_in       * maxlum_reduced) / maxlum;
	    	 xg = ((unsigned int)*(pntr_in + 1) * maxlum_reduced) / maxlum;
	    	 xb = ((unsigned int)*(pntr_in + 2) * maxlum_reduced) / maxlum;
	

	    	 xr = (xr * GEMAXUSHORT) / maxlum_reduced;
	    	 xg = (xg * GEMAXUSHORT) / maxlum_reduced;
	    	 xb = (xb * GEMAXUSHORT) / maxlum_reduced;
	    	 /*
	     	  * Allocate color
	     	  */
		 geColStat = 0;
	    	 *pntr_out++ = (unsigned char)geAllocColor((unsigned short)xr, (unsigned short)xg, (unsigned short)xb, FALSE);
		 if (geColStat) return;		/* Ran out of cells, quit    */
	    	 pntr_in    += components;
	    	}				/* END if color		     */
	    }					/* END PIXEL loop	     */
   	}					/* END LINE  loop	     */
   }						/* END else		     */
}

/*
 * Convert a color mapped image (with private color map) to dithered monochrome
 */
XImage *
geXImgCMapToSingDith(image, colortable)
	XImage *image;
	XColor colortable[256];
{
	int    size,x,y,i,tobase;
	XImage *timage;
	unsigned char *pntrf,*pntrt;
	float *downerr,*downptr;
	float  greytable[256];
	float  currpix,error,righterr;

	if (geDispChar.Depth > 8 ||
	    (image->depth < 4 || image->depth > 8 ||
	     image->bits_per_pixel < 4 || image->bits_per_pixel > 8))
	  {error_string = (char *) geFetchLiteral("GE_ERR_UNAVAILABLE", MrmRtypeChar8);
           if (error_string != NULL) 
	     {geError(error_string, FALSE);
	      XtFree(error_string);
	     }
	   return(NULL);
	  }

        timage = geXCrImg(geDispDev,geVisual,1,XYBitmap,0,NULL,image->width,image->height,8,0);

	size = timage->bytes_per_line * timage->height;

	timage->data = geMalloc(size);

	downerr = (float *) geMalloc(timage->width * sizeof(unsigned));
	downptr = downerr;

	memset(downerr,0,timage->width * sizeof(unsigned));

	memset(timage->data,255,size);

	/* each color R, G, or B ranges within and including 0 to 65535 */

	for (i = 0; i < 256; i++) {
	     /* grey = 0.3*RED/65535 + 0.59*GREEN/65535 + 0.11*BLUE/65535 */
		greytable[i] =  ((float)colortable[i].red * 4.5777e-06) +
				((float)colortable[i].green * 9.00282e-06) +
				((float)colortable[i].blue * 1.67849e-06);
	}

	for (y = 0; y < timage->height; y++) {
		pntrf = (unsigned char *)(image->data + image->bytes_per_line * y);
		pntrt = (unsigned char *)(timage->data + timage->bytes_per_line * y);
		tobase = 1;
		downptr = downerr;
		*downptr = 0;
		righterr = 0;
		for (x = 0; x < timage->width; x++) {
			currpix = greytable[*pntrf] + *downptr + righterr;
			if (currpix > 0.5) { /* if brighter than 50% white */
				/* make pixel white */
				error = currpix - 1;
			} else {
				/* make pixel black */
				*pntrt = *pntrt & (~tobase);
				error = currpix;
			}
			righterr = 3 * error / 8;
			*downptr = *downptr + (3 * error / 8);
			*(downptr+1) = error / 4;
			downptr++;
			pntrf++;
			tobase = tobase * 2;
			if (tobase > 128) {
				tobase = 1;
				pntrt++;
			}
		}
	}

	geFree(&downerr, timage->width * sizeof(unsigned));

	return(timage);

}

/*
 * Convert a color image to monochrome using ordered dither halftone
 */
XImage  *geXImgToSingOrdDith(image, ctable, ncolors)
    XImage 	*image;
    XColor 	*ctable;
    int		ncolors;
{

#define GEMLUMINRANGE	100	/* Max luminance in matrix M */

static int	geM[GEMSIZE][GEMSIZE] = {
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


    int		size, x, y, i, pixel;
    XImage	*timage;
    int		*greytable;


    timage = geXCrImg(geDispDev, XDefaultVisual(geDispDev, geScreen),
			  1, XYBitmap, 0, NULL, image->width, image->height,
			  geDispChar.Depth, 0);
    size = timage->bytes_per_line * timage->height;
    timage->data = geMalloc(size);

    size = ncolors * sizeof(int);
    greytable = (int *) geMalloc(size);

    /* Convert RGB values (range 0 to 65535) to grey values (range 0 to 100) */
    for (i = 0; i < ncolors; i++) {
	if (ctable[i].pixel != i) {
	    fprintf(stderr, "INTERNAL ERROR: Misformed color table!  Pixel entries are out of order.\n\n");
	    geFree(&greytable, NULL);
	    return(NULL);
	}

	/* Since this is done only once, the mults and divides are cheap */
	/* I'll replace this later with the macro.  */
	greytable[i] =  (ctable[i].red / 655 * 0.3) +
			(ctable[i].green / 655 * 0.59) +
			(ctable[i].blue / 655 * 0.11);
    }


    for (y = 0; y < image->height; y++) {
	for (x = 0; x < image->width; x++) {

	    pixel = XGetPixel(image, x, y);

	    if (greytable[pixel] <= geM[x%GEMSIZE][y%GEMSIZE])
		XPutPixel(timage, x, y, 0);
	    else
		XPutPixel(timage, x, y, 1);
	}
    }

    geFree(&greytable, NULL);

    return(timage);
}
