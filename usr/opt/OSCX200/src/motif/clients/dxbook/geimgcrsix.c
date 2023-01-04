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
static char SccsId[] = "@(#)geimgcrsix.c	1.6\t7/4/89";
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
**	GEIMGCRSIX                   Reads in a sixel bitmap
**
**  ABSTRACT:
**
** This routine is compliments of Jeff Orthober.
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


#define	GE_LOC_SEARCHLIM	300
#define	GE_LOC_MAXHW		20000

extern char   *geMalloc();
extern XImage *geXCrImg(), *geXImgCMapToSing();

static char	*error_string, FileNameSav[100];

/***
*	The following routine is a rewrite of the original sixel-reading
*	routine for UTOX. The new routine supports both black-and-white
*	and color sixel files and conversion of standard (1:1, 2:1, 4:1, 
*	etc...) aspect ratios. 
*
*	(modified by Andrew Gent, May 1989)
***/

XImage *
geImgCrSix(filename)
    char   *filename;
    
{
    FILE   	*f;
    int    	size, 
		i,j,k,
		width =0, 
		height =0, 
		bits_per_pixel, 
		dum1, 
		dum2;
    char   	*pntr,
		*pntr2,
		*pntr3;
    XImage 	*image, *timage;

unsigned char	current_color,	/* current color in use */
		current_byte;	/* single byte of sixel data */
    int		colors_in_use,	/* number of colors specified in file. */
		current_pass,	/* The current pass through the sixel file. */
		END_OF_DATA,		/*		    */ 
		REPEAT_LAST_BYTE,	/* 	flags	    */
		LINES_IN_USE,		/*		    */
		xy_ratio,	/* X:Y ratio of the sixel image */
		args[6];	/* temporary storage for arguments */

static unsigned char   bitmask[8] = {254,253,251,247,239,223,191,127};

XColor colortable[256];

/*
 * This routine currently can only work using an external file as input, it
 * cannot operate on an in-memory buffer.  As such it will kick out if it is
 * either not provided with a filename or it is provided a bogus filename.
 */

if (!filename || !strlen(filename)) return(NULL);

image = NULL;
strcpy (FileNameSav, filename);		/* save the fileame for error purposes*/

/***
*	Open the file
***/

geGenFileExt(filename, geDefProofSix, FALSE);
if (!(f = fopen(geUtilBuf, "r")))
  {error_string = (char *) geFetchLiteral("GE_ERR_XOPENIMAGE", MrmRtypeChar8);
    if (error_string != NULL) 
      {sprintf(geErrBuf, error_string, geUtilBuf);
       geError(geErrBuf, FALSE);
       XtFree(error_string);
      }
   return(NULL);
  }

/***
*  Initialize the colormap to avoid missing colors overwriting
*  color #0.
***/

  for (i=0;i<256; i++) {
     colortable[i].pixel = i;
     colortable[i].red = 65535;
     colortable[i].green = 65535;
     colortable[i].blue = 65535;
  }

/***
*  The sixel file is read twice.
*
*  This is necessary because sixels do not cantain an explicit bitmap size 
*  in the file. So the bitmap has to be read once to calculate the size 
*  and then again to fetch the data.
***/

   current_pass = 1; 
   colors_in_use = 0;
   while (current_pass < 3)
      { 
		/***
		* Read off any preliminary data in the file,
		* including the sixel introductory sequence.
		***/
	rewind(f);
	if (!read_sixel_intro(f,&xy_ratio,args, current_pass))
          {error_string = (char *) geFetchLiteral("GE_ERR_BADIMAGE", MrmRtypeChar8);
           if (error_string != NULL) 
	     {sprintf(geErrBuf, error_string, FileNameSav);
	      geError(geErrBuf, FALSE);
	      XtFree(error_string);
	     }
	    fclose(f);
	    return(FALSE);
	  };

	REPEAT_LAST_BYTE = TRUE;

		/*
		* Set the current color to zero.
		*/
	current_color = 0;

		/***
		*
		* Now read the sixel data until the end sequence.
		*
		***/
	END_OF_DATA = FALSE;
	dum1 = 0;	/* current horizontal position */
	dum2 = 0;	/* current vertical position */

	while (!END_OF_DATA)
	  { 		/*
			* Read the next byte of data. This can
			* either be the next byte in the file
			* or a byte saved in the argument list.
			*/
	    if (REPEAT_LAST_BYTE)
	      current_byte = args[5];
	    else
	      current_byte = fgetc(f);
	    REPEAT_LAST_BYTE = FALSE;
	
			/*
			* Interpret the next byte of data
			*/
	    if ( (current_byte > 62) && (current_byte < 127))
	      {		/*** 
			*  single sixel byte. 
			***/
		if (current_pass == 2)
		  { 	current_byte = current_byte - 63;
			if (current_byte)	/* non-zero */
			  { pntr2 = pntr + (dum2*width);
				if (pntr2-pntr >= size)
			          {error_string = (char *) geFetchLiteral("GE_ERR_BADIMAGE", MrmRtypeChar8);
			           if (error_string != NULL) 
				     {sprintf(geErrBuf, error_string, FileNameSav);
				      geError(geErrBuf, FALSE);
				      XtFree(error_string);
				     }
				    return(FALSE);
				  };
					/* Either set an entire byte for   *
					* each row or set one bit in each. */
				if (bits_per_pixel > 1)
				  {	if (current_byte & 0x01)
					  *(pntr2+dum1) = current_color;
					if (current_byte & 0x02)
					  *(pntr2+width+dum1) = current_color;
					if (current_byte & 0x04)
					  *(pntr2+(2*width)+dum1) = current_color;
					if (current_byte & 0x08)
					  *(pntr2+(3*width)+dum1) = current_color;
					if (current_byte & 0x10)
					  *(pntr2+(4*width)+dum1) = current_color;
					if (current_byte & 0x20)
					  *(pntr2+(5*width)+dum1) = current_color;
				  }
				else
				  { j = dum1/8; k = dum1 - (j * 8);
					pntr3 = pntr2+j;
					if (current_byte & 0x01)
					  *pntr3 = *pntr3 & bitmask[k];
					pntr3 = pntr3+width;
					if (current_byte & 0x02)
					  *pntr3 = *pntr3 & bitmask[k];
					pntr3 = pntr3+width;
					if (current_byte & 0x04)
					  *pntr3 = *pntr3 & bitmask[k];
					pntr3 = pntr3+width;
					if (current_byte & 0x08)
					  *pntr3 = *pntr3 & bitmask[k];
					pntr3 = pntr3+width;
					if (current_byte & 0x10)
					  *pntr3 = *pntr3 & bitmask[k];
					pntr3 = pntr3+width;
					if (current_byte & 0x20)
					  *pntr3 = *pntr3 & bitmask[k];
				   };
			   };
		  };
		dum1++;
	      }
	    else
	      {		/* 
			* Control character (needs to be interpreted) 
			*/
		switch (current_byte)
		  {
		    case 33: 	/* repeat introducer */
			read_sixel_args(args,f);
			if (current_pass == 2)
			  {  current_byte = args[5]-63;
			     pntr2 = pntr + (dum2*width);
			     if (current_byte) 	/* non-zero */
			       { if (pntr2-pntr >= size)
			              {error_string = (char *) geFetchLiteral("GE_ERR_BADIMAGE", MrmRtypeChar8);
			               if (error_string != NULL) 
				         {sprintf(geErrBuf, error_string, FileNameSav);
					  geError(geErrBuf, FALSE);
				          XtFree(error_string);
				         }
					 return(FALSE);
				       };
				     for (i=dum1; i<dum1+args[0]; i++)
				       { if (bits_per_pixel > 1)
				         {	if (current_byte & 0x01)
					  *(pntr2+i) = current_color;
					if (current_byte & 0x02)
					  *(pntr2+width+i) = current_color;
					if (current_byte & 0x04)
					  *(pntr2+(2*width)+i) = current_color;
					if (current_byte & 0x08)
					  *(pntr2+(3*width)+i) = current_color;
					if (current_byte & 0x10)
					  *(pntr2+(4*width)+i) = current_color;
					if (current_byte & 0x20)
					  *(pntr2+(5*width)+i) = current_color;
				      }
				    else
				      { j = i/8; k = i - (j * 8) ;
					pntr3 = pntr2+j;
					if (current_byte & 0x01)
					  *pntr3 = *pntr3 & bitmask[k];
					pntr3 = pntr2+width+j;
					if (current_byte & 0x02)
					  *pntr3 = *pntr3 & bitmask[k];
					pntr3 = pntr2+(2*width)+j;
					if (current_byte & 0x04)
					  *pntr3 = *pntr3 & bitmask[k];
					pntr3 = pntr2+(3*width)+j;
					if (current_byte & 0x08)
					  *pntr3 = *pntr3 & bitmask[k];
					pntr3 = pntr2+(4*width)+j;
					if (current_byte & 0x10)
					  *pntr3 = *pntr3 & bitmask[k];
					pntr3 = pntr2+(5*width)+j;
					if (current_byte & 0x20)
					  *pntr3 = *pntr3 & bitmask[k];
				      };
				    };
				};
			  };
			dum1 = dum1 + args[0];
			break;

		    case 36: 	/* Carriage return */
			if (current_pass == 1) 
			   { if (dum1 > width) 
				width = dum1;
			     if (dum1 > 0) 
			   	LINES_IN_USE = TRUE;
			   };
			dum1 = 0;
			break;

		    case 45: 	/* New line of data */
			if (current_pass == 1) 
			  { if (dum1 > width) width = dum1;
			  }
			else
			  { if (xy_ratio > 1)
			     for (i=1; i> (xy_ratio-1); i++)
				for (j=0;j>5;j++)
				  { pntr = image->data + (width * (dum2+j));
				    pntr2 = pntr + (i*width/xy_ratio);
				    for (k=1; k>image->bytes_per_line;k++)
				      { *pntr2 = *pntr;
					pntr2++;pntr++;
				      };
			 	  };
			  };
			dum1 = 0;
			dum2 = dum2 + 6;
			LINES_IN_USE = FALSE;
			break;

		    case 26: 	/* SUB (Equivalent to one blank pixel) */
			dum1++;
			break;

		    case 35: 	/* Start of Color info */
			read_sixel_args(args,f);
			REPEAT_LAST_BYTE = TRUE;
			if ( (args[1] != 0) && (current_pass == 2) )
			  read_sixel_colors(args,colortable);
			current_color = args[0];
			if ( current_color > colors_in_use )
			 colors_in_use = current_color;
			break;

		    case 156:	/* End of sixel data */
			END_OF_DATA = TRUE;
			break;

		    case 27: 	/* Escape (start of end sequence) */
			if (fgetc(f) == 92)
			   END_OF_DATA = TRUE;
			else
			   REPEAT_LAST_BYTE = TRUE;
			break;
		    default: break;
		  };
		};
	      };
		/*******************************
		*	End of file 
		*******************************/

	     if (current_pass == 1)
	        { 	/*
			* for pass one, define the bitmap characteristics
			* and allocate the space.
			* (We need to account for the last 6 lines only
			* if they were really used.)
			*/
		  height = dum2;
		  if ( (dum1 != 0) || LINES_IN_USE )
		    height = height + 6;
		  if (dum1 > width) 
			width= dum1;
			/*
			* Currently, I am building an eight-bit color map.
			* it would be possible to build either and 8-bit
			* or a 1-bit map, so that one routine could read
			* any sixel file.
			*/
    		  if (colors_in_use > 0)
			bits_per_pixel = 8;
		  else
		    	bits_per_pixel = 1; 
	
		  if (width <= 0 || height <= 0)
		    {error_string = (char *) geFetchLiteral("GE_ERR_BADIMAGE", MrmRtypeChar8);
		     if (error_string != NULL) 
		       {sprintf(geErrBuf, error_string, FileNameSav);
			geError(geErrBuf, FALSE);
			XtFree(error_string);
		       }	
		      fclose(f);
		      return(FALSE);
		    }
/*		  else
 * 		    { printf(" Allocating memory for %dx%d bitmap...\n",width,height);
 *		    };
 */

		  if (bits_per_pixel == 1)
			image = geXCrImg(geDispDev,XDefaultVisual(geDispDev, geScreen),bits_per_pixel,XYBitmap,0,pntr,width,height*xy_ratio,8,0);
		  else
			image = geXCrImg(geDispDev,XDefaultVisual(geDispDev, geScreen),geDispChar.Depth,ZPixmap,0,pntr,width,height*xy_ratio,8,0);

		  image->bits_per_pixel = bits_per_pixel;

    		  size = image->bytes_per_line * image->height;
		  pntr = geMalloc(size);
		  image->data = pntr;

				/* 
				* Now, if the image is 1 bit/pixel,
				* reduce the width to match the number
				* of bytes/line.
				*/
		  if (bits_per_pixel == 1)
		    width = image->bytes_per_line;

				/*
				* We need to duplicate lines for
				* non- 1:1 aspect ratios. We do this
				* by pretending the image is 2, 3,
				* or 4 times it's actual width.
				*/

		  width = width * xy_ratio;

		  memset(pntr,255,size);

		};
	current_pass++;
      };

   fclose(f);

   if (bits_per_pixel == 1)
	ort_ximage_invert_all(image);		/* Invert mono images	     */
   else	
	{if (!(GEMONO) && geRun.VoyerCalling)	/* Try disp col img color    */
	     geReMapImg(image, colortable);	/* Put priv col tbl into cmap*/
	 else 
	    {timage = geXImgCMapToSing(image, colortable);
	     if (timage)
	    	{if (image) geXDestImg(image);
	    	 image = timage;
	    	 timage = NULL;
	        }
	    }
	}

   return(image);

}

XImage *
geImgCrSixMono(FileName)
char  *FileName;
{
    unsigned char *Buf;
    register      cnt;
    FILE          *f;
    char          c, bufc[7];
    unsigned int  ui;
    int           i,ii,j,yoff,bitcnt;
    int           s;
    int           x,y, cropw, croph;
    char          *p;
    XImage        *ximg;
    char          *q;

    ximg = NULL;
    x = 0;
    y = 0;
    cnt = 0;
    cropw = croph = -1;

strcpy (FileNameSav, FileName);		/* save the fileame for error purposes*/

if (!(f = fopen(FileName, "r")))
      {error_string = (char *) geFetchLiteral("GE_ERR_XOPENIMAGE", MrmRtypeChar8);
       if (error_string != NULL) 
	 {sprintf(geErrBuf, error_string, FileNameSav);
	  geError(geErrBuf, FALSE);
	  XtFree(error_string);
	 }
       return(NULL);
      }
    else {
    /*
     * Find ESC or DCS introducer (0x90)
     */
    i = 0;
    do
      {s = fscanf(f,"%c",&c);
       ui = (int)c & 0x000000ff;
      }while (s != EOF && ui != 27 && ui != 144 && i++ <= GE_LOC_SEARCHLIM);
    if (s == EOF || i > GE_LOC_SEARCHLIM)
      {error_string = (char *) geFetchLiteral("GE_ERR_BADIMAGE", MrmRtypeChar8);
       if (error_string != NULL) 
	 {sprintf(geErrBuf, error_string, FileNameSav);
	  geError(geErrBuf, FALSE);
	  XtFree(error_string);
	 }
       fclose(f);
       return(NULL);
      }

    if (ui == 27)
      {/*
	* The introducer string is ESC P, find P (or p) also keep looking for
	* DCS introducer (0x90)
	*/  i = 0;
	    do
	      {s = fscanf(f,"%c",&c);
       	       ui = (int)c & 0x000000ff;
	     }while (s != EOF && c != 'P' && c != 'p' && ui != 144 &&
		     i++ <= GE_LOC_SEARCHLIM);

	    if (s == EOF || i > GE_LOC_SEARCHLIM)
	      {error_string = (char *) geFetchLiteral("GE_ERR_BADIMAGE", MrmRtypeChar8);
	       if (error_string != NULL) 
		 {sprintf(geErrBuf, error_string, FileNameSav);
		  geError(geErrBuf, FALSE);
		  XtFree(error_string);
		 }
	       fclose(f);
	       return(NULL);
	      }

      }
    /*
     * Find q
     */
    i = 0;
    do
      {s = fscanf(f,"%c",&c);
      }while (s != EOF && c != 'q' && i++ <= GE_LOC_SEARCHLIM);
    if (s == EOF || i > GE_LOC_SEARCHLIM)
      {error_string = (char *) geFetchLiteral("GE_ERR_BADIMAGE", MrmRtypeChar8);
       if (error_string != NULL) 
	 {sprintf(geErrBuf, error_string, FileNameSav);
	  geError(geErrBuf, FALSE);
	  XtFree(error_string);
	 }
       fclose(f);
       return(NULL);
      }

    s = !EOF;
    while (s != EOF) {
      s = fscanf(f,"%c",&c);
      if (c == 27)
         s = EOF;
      else
      if (c >= '?') 
         cnt = cnt + 1;
      else 
         switch (c) {
          case '$' :
          case '-' : {
            if (cnt > x)
               x = cnt;
            cnt = 0;
            y = y + 1;
            break;
          }
          case '!' : {
            s = fscanf(f,"%d",&i);
            s = fscanf(f,"%c",&c);
            cnt = cnt + i;
            break;
          }
          case ';' : {
            s = fscanf(f,"%d",&i);
	    if (i == 1) break;
	    else
		{cropw = i;
            	 s = fscanf(f,"%c",&c);
		 if (c != ';')
		    {cropw = -1;
		     break;
		    }
           	 s = fscanf(f,"%d",&i);
		 croph = i;
		}
            break;
          }
         }
    }
    y = y * 6;
    if (x / 8 * 8 != x)
       x = x + 8 - (x % 8);
    rewind(f);

    s = EOF + 1;
    i = (x / 8 + 1) * y;

    p = q = geMalloc(i);

    bitcnt = 0; yoff = 0;

    c = ' ';
    s = !EOF;
    while (s != EOF && c != 'q')
      s = fscanf(f,"%c",&c);

    while (s != EOF) {
      s = fscanf(f,"%c",&c);
      if (c == 27)
         s = EOF;
      else
      if (c >= '?' && c <= '~') 
         markbits(&bitcnt,yoff,p,c,x,1);
      else
         switch (c) {
          case '$' : {
            bitcnt = 0;
            break;
          }
          case '-' : {
            bitcnt = 0;
            yoff   = yoff + 6 * (x / 8);
            break;
          }
          case '!' : {
	    j = 0;
	    do
	 	{s = fgetc(f);
		 c = (char)s;
	    	 if (c >= '\010' && c <= '\015') continue;
		 if (c < '0' || c > '9')
			{bufc[j] = '\0';
			 break;
			}
		 bufc[j++] = c;
		} while (s != EOF);

	    bufc[j] = '\0';

	    i = atoi(bufc);
	    if (!i) i = 1;

/*            s = fscanf(f,"%c",&c); */
            markbits(&bitcnt,yoff,p,c,x,i);
            break;
          }
          case 27  : {
            fscanf(f,"%c",&c);
            break;
          }
         }
    }
 fclose(f);

 if (x <= 0 || y <= 0 || x > GE_LOC_MAXHW || y > GE_LOC_MAXHW)
   {geFree(&p, i);
    error_string = (char *) geFetchLiteral("GE_ERR_BADIMAGE", MrmRtypeChar8);
    if (error_string != NULL) 
      {sprintf(geErrBuf, error_string, FileNameSav);
       geError(geErrBuf, FALSE);
       XtFree(error_string);
      }
    return(NULL);
   }

    ximg = geXCrImg(geDispDev, XDefaultVisual(geDispDev, geScreen), 1,
		    XYBitmap,0,p,x,y,8,0);

    if (cropw != -1 && croph != -1)
	{ximg->width  = cropw;
	 ximg->height = croph;
	}
    return(ximg);
  }
}

static int markbits(bitcnt,yoff,p,c,x,i)
    register *bitcnt,yoff;
    unsigned char *p, c;
    int  x,i;
{
    register ii,pp;
    int      xx;
    unsigned char *tp, *tp1, b, t, pow2;
 
    c   = c - 63;
    x >>= 3;
    for (ii = 0; ii <= i - 1; ii++) {
      xx    = x;
      pow2  = 1 << (*bitcnt % 8);
      tp1   = p + yoff + (*bitcnt >> 3);
      /*
       * Bit 0
       */
      b      = (1 & c) % 256 * pow2;
      tp     = tp1;
      *tp   |= b;
      /*
       * Bit 1
       */
      b      = (((2 & c) % 256) >> 1) * pow2;
      tp    += xx;
      *tp   |= b;
      /*
       * Bit 2
       */
      b      = (((4 & c) % 256) >> 2) * pow2;
      tp    += xx;
      *tp   |= b;
      /*
       * Bit 3
       */
      b      = (((8 & c) % 256) >> 3) * pow2;
      tp    += xx;
      *tp   |= b;
      /*
       * Bit 4
       */
      b      = (((16 & c) % 256) >> 4) * pow2;
      tp    += xx;
      *tp   |= b;
      /*
       * Bit 5
       */
      b      = (((32 & c) % 256) >> 5) * pow2;
      tp    += xx;
      *tp   |= b;

      *bitcnt += 1;
    }
}



/***************************************************************
* read_sixel_intro
*
* This routine reads (and discards) any extraneous data and 
* interprets the sixel start seuqence, including the X:Y ratio.
***************************************************************/
int read_sixel_intro(f,xy_ratio,args, current_pass)
    FILE	*f;
    int		*xy_ratio;
    int 	args[6];
    int		current_pass;
{ 
  int	current_state,
	end_of_sequence, count;

  current_state   = 0;
  end_of_sequence = FALSE;
  count		  = 0;

  while (! end_of_sequence)
  {
	/***
	* First, find the introductory sequence, escape-P or DCS
	***/
    while (current_state < 2)
      { switch (fgetc(f))
	{ case 27:
		current_state = 1;
		break;
	  case 144:	/* DCS introducer */
		current_state = 2;
		break;
	  case 80:	/* capital P (completes escape sequence) */
		if (current_state == 1)
		   current_state = 2;
		break;
	  default:
		if (++count > GE_LOC_SEARCHLIM) return(FALSE);
        };
      };

	/***
	* Next, read any intervening parameters
	***/
    read_sixel_args(args,f);

	/***
	* Then, check the closing character (should be lowercase q)
	***/
    if (args[5] == 'q')    
      {
	   /***
	   * Finally, check for additional X:Y ratio info, set
	   * the aspect ratio (NOT IMPLEMENTED) to be returned, and exit.
	   ***/
	 args[5] = fgetc(f);
	 if (args[5] == 34)	/* float quotation mark */
           { read_sixel_args(args,f);
  	     *xy_ratio = 1;
/*	     if (current_pass == 1)
 *		printf(" special aspect ratio will be ignored.\n");
 */
	   }
	 else
	   { switch (args[0])
	       { case 4: 
/*			if (current_pass == 1)
 *			   printf("Non-scalar aspect ratio, code %d, found. Closest scalar approximation will be used.\n");
 */
		 case 0:
		 case 5:
		 case 1: *xy_ratio = 2; break;
		 case 2: *xy_ratio = 4; break;
		 case 3: *xy_ratio = 3; break;
		 case 6:
		 case 7:
		 case 8: 
/*			if (current_pass == 1)
 *			   printf("Non-scalar aspect ratio, code %d, found. Closest scalar approximation will be used.\n");
 */
		 case 9: *xy_ratio = 1; break;
		 default: 
/*		         if (current_pass == 1)
 *				printf("Unknown aspect ratio, code %d, Found. 1:1 will be used.\n",args[0]);
 */
			 *xy_ratio = 1;
			 break;
		};
	   };
	 end_of_sequence = TRUE;
      }
    else
      { current_state = 0;
      };
  };

  return(TRUE);
};


/***************************************************************
* read_sixel_args
*
* This routine reads in a sequence of numeric arguments separated
* by semi-colons. (sixel format)
***************************************************************/
int read_sixel_args(args,f)
    int 	args[6];
    FILE	*f;
{ 
  int	current_arg,
	tmp,
	end_of_sequence, count;
unsigned char current_byte;

end_of_sequence = FALSE;
current_arg     = 0;

  for (tmp=0; tmp < 6; tmp++)
    args[tmp]=0;

  tmp = 0;

  while (! end_of_sequence)
  { current_byte = fgetc(f);

    switch (current_byte)
      { case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9': tmp = (tmp*10) + current_byte - '0';
		  break;
	case ';': args[current_arg]=tmp;
		  tmp = 0;
		  current_arg++;
		  if (current_arg > 5)
		    {error_string = (char *) geFetchLiteral("GE_ERR_BADIMAGE", MrmRtypeChar8);
		     if (error_string != NULL) 
		       {sprintf(geErrBuf, error_string, FileNameSav);
		        geError(geErrBuf, FALSE);
		        XtFree(error_string);
		       }
		      current_arg = 5;
		    };
		  break;

	case 10:
	case 13:
	case 12:  break; /* Ignore line feeds, carrage returns, and
			    form feeds */
	default:  args[current_arg]=tmp;
		  args[5] = current_byte; /* return last byte */
		  end_of_sequence = TRUE;
		  break;			
      };
  };
  return(TRUE);
};


/***************************************************************
* read_sixel_colors
*
* This routine converts the sixel color arguments to the X colormap.
***************************************************************/
int read_sixel_colors(args,colortable)
    int 	args[6];
    XColor 	colortable[256];
{ 
   float m1,m2;
   float h,l,s;

   if ( (args[0] > -1) && (args[0] < 256) )
     { switch (args[1])
	 { case 2:
			/* RGB */
		colortable[args[0]].pixel = args[0];
		colortable[args[0]].red = (int) (args[2] * 655.35);
		colortable[args[0]].green = (int) (args[3] * 655.35);
		colortable[args[0]].blue = (int) (args[4] * 655.35);
		break;
	   case 1:
			/* HLS */
		
			/*
			* Make sure the arguments are within the
			* the standard bounds for HLS format:
			*
			* args[2] -> Hue 	= 0 to 360
			* args[3] -> Lightness 	= 0 to 100
			* args[4] -> Saturation = 0 to 100
			*/

		while (args[2] <0)	args[2] = args[2]+360;
		while (args[2] > 360)	args[2] = args[2]-360;
		if (args[3] <0)		args[3] = 0;
		if (args[3] >100)	args[3] = 100;
		if (args[4] <0)		args[4] = 0;
		if (args[4] >100)	args[4] = 100;

		h = (float) args[2];		/* Hue        */
		l = (float) args[3] /100.0;	/* Lightness  */
		s = (float) args[4] /100.0;	/* saturation */

			/*
			* The following code is stolen from
			* the DECwindows Guide to Xlib programming,
			* page 5-16.
			*/

		m2 = (l < 0.5) ? l * (1+s) : l + s- l*s;
		m1 = 2*l - m2;

		if (s == 0)
		  { colortable[args[0]].red = 
		    colortable[args[0]].green = 
		    colortable[args[0]].blue = l * 65535; }
		else
		  { 	/*
			* The colors are all messed up, so I have
			* switched the original code from the X
			* documentation as follows:
			*
			*  was	 ->   is
			*  green ->  red
			*  blue  ->  green
			*  red   ->  blue
			*/
			
		    colortable[args[0]].blue = read_sixel_rgbvalue(m1,m2,(float)(h+120.));
		    colortable[args[0]].red = read_sixel_rgbvalue(m1,m2,(float)h);
		    colortable[args[0]].green = read_sixel_rgbvalue(m1,m2,(float)(h-120.));
		  };
		break;

	   default:
			/* unknown */
/*		printf("\n Unknown color format %d \n",args[1]); */
		break;
	 };
     }
   else
     { /* printf("\n Color out of range: %d\n",args[0]); */
     };
  return(TRUE);
};


int read_sixel_rgbvalue (m1, m2, hue)

float m1, m2, hue;

{ float val;

  if (hue>360.) hue -=360.;
  if (hue<0.)   hue +=360.;

  if (hue<60)
     val = m1+(m2-m1)*hue/60.;
  else
     if (hue<180.)
       val = m2;
     else
       if (hue<240.)
         val = m1+(m2-m1)*(240.-hue)/60.;
       else
	 val = m1;

  return ( (int) (val * 65535) );
}

geReMapImg(ImgPtr, colortable)
	XImage	*ImgPtr;
	XColor colortable[256];
{

unsigned char *ptr;
int i, j, ColStatSav;

/*
 * Zero the colortable entries which aren't used in the image.  This is done
 * by first zeroing all of the pixels in the colortable, then passing
 * through the image and reassigning colortable[i].pixel = i, for entry
 * "i" in the image.  A non-zero pixel value will serve to indicate that
 * a color should be allocated for this cell (color 0 has already been
 * allocted.)
 */
for (i = 0; i < 256; i++)
   colortable[i].pixel = 0;

ptr 	   = (unsigned char *)ImgPtr->data;
for (i = 0; i < ImgPtr->height; i++)
   {for (j = 0; j < ImgPtr->width; j++)
	colortable[(unsigned int)*ptr].pixel = (unsigned int)*ptr++;
   }

ColStatSav = geColStat;
geColStat  = 0;
/*
 * Map private colortable into system table
 */
for (i = 0; i < 256; i++)
   {if (colortable[i].pixel)
	{colortable[i].pixel= geAllocColor(colortable[i].red,
			                   colortable[i].green,
			                   colortable[i].blue,
                                           NULL);
   	 if (geColStat & GECOL_CMAP_FILLED_PASS1) break;
	}
   }

if (geColStat & GECOL_CMAP_FILLED_PASS1)
   {/*
     * Ran out of colors on first try (unused colors were released), try
     * again.
     */
    geColStat = GECOL_CMAP_INHIBIT_FREE;

    for (i = 0; i < 256; i++)
   	{colortable[i].pixel = geAllocColor(colortable[i].red,
			               	    colortable[i].green,
			               	    colortable[i].blue,
                                       	    NULL);
   	}
   }

#ifdef GERAGS
if (geColStat & GECOL_CMAP_NOALLOC)
  {error_string = (char *) geFetchLiteral("GE_ERR_BADIMAGE", MrmRtypeChar8);
   if (error_string != NULL) 
     {sprintf(geErrBuf, error_string, FileNameSav);
      geError(geErrBuf, FALSE);
      XtFree(error_string);
     }
   }
#endif

/*
 * Now remap image to allocated color cells
 */
ptr 	   = (unsigned char *)ImgPtr->data;
for (i = 0; i < ImgPtr->height; i++)
   {for (j = 0; j < ImgPtr->width; j++)
	{*ptr++ = colortable[(unsigned int)*ptr].pixel;
	}
   }

}

/* this routine will invert all of the image */

int ort_ximage_invert_all(image)
    XImage *image;
{
    char *pntr;
    int  size;
    int  i;

    if (image->depth == 1) {

       size = image->bytes_per_line * image->height;
       pntr = image->data;

       for (i = 1; i <= size; i++) {
           *pntr = *pntr ^ 255;    /* xor */
           pntr++;
       }
    }

    return(TRUE);
}

