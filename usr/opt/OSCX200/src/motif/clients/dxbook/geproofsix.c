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
static char SccsId[] = "@(#)geproofsix.c	1.12\t9/29/89";
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
**	GEPROOFSIX			Writes given window to sixel file
**
**  ABSTRACT:
**
** This routine is compliments of Jeff Orthober.
**
** This routine will convert the image structure passed to
** a sixel file written to the filename passed.  If the
** hires flag is true the sixel file will be headed with
** the 300 dpi parameters, else just normal default sixel
** is generated.  care is taken about writing the last
** byte in the image and the last line in the image
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 09/14/88 Created - compliments of Jeff Orthober
**
**--
**/

#include "geGks.h"

extern XImage *geXImgToSing();

geProofSIX(Win, ImgPtr, FileName, x0, y0, width, height, hires, monoflag)
Window       Win;
XImage       *ImgPtr;
char         *FileName;
int          x0, y0;
unsigned int width, height;
int          hires;
short        monoflag;

{
    int           i,p,cnt,chcnt,lcnt,lines,x, format, depth, cropw, croph;
    char          ch,oldch;
    unsigned char power, xor;
    unsigned char *pntr, *pntr0,*pntr1,*pntr2,*pntr3,*pntr4,*pntr5;
    char          xorarr[6];
    FILE          *f;
    int           markers[256], numcols;
    static short  gotcol = FALSE;
    static XColor colortable[256];
    XWindowAttributes watts;
    Colormap cmap;
    XImage        *TImgPtr;
    char	  *error_string;

    cropw = width;
    croph = height;
    /*
     * For SIXELS, the width must be byte aligned and the
     * height must be a multiple of 6 - it is expected that the window
     * will accomodate this necessary padding.
     */
    width  = ((width + 7) >> 3) << 3;
    height = ((height + 5) / 6) * 6;
    /*
     * Can't handle depth > 8
     */
    if ((depth = XDefaultDepth(geDispDev, geScreen)) > 8)
       monoflag = TRUE;

    numcols = 2;
    if (!monoflag && (depth > 1))
      {numcols = 1 << depth;
       XGetWindowAttributes(geDispDev, XRootWindow(geDispDev, geScreen),
			    &watts);
       if (!watts.map_installed)
	 {XGetWindowAttributes(geDispDev, geRgRoot.self, &watts);
	  if (!watts.map_installed)
      	     {error_string = (char *) geFetchLiteral("GE_ERR_UNAVAILABLE", MrmRtypeChar8);
	      if (error_string != NULL) 
	  	{geError(error_string, FALSE);
	   	 XtFree(error_string);
	  	}
	      return(GERRUNAVAILABLE);
             }
	 }
       cmap = watts.colormap;

       for (i = 0; i < numcols; i++)
	 colortable[i].pixel = i;

       XQueryColors(geDispDev, cmap, colortable,
		    min(numcols,geNumCols));

      }

    xorarr[0] = 0x01; xorarr[1] = 0x03; xorarr[2] = 0x07; xorarr[3] = 0x0F;
    xorarr[4] = 0x1F; xorarr[5] = 0x3F;

    if (!ImgPtr)
      {if (!(ImgPtr = TImgPtr = XGetImage(geDispDev, Win, x0, y0,
					   width, height,
					   AllPlanes, ZPixmap)))
	 return(NULL);
      }
    else
      TImgPtr = NULL;

    if (depth > 1)
      {if (monoflag && ImgPtr->depth > 1)
	 {ImgPtr = geXImgToSing(ImgPtr);
	  if (TImgPtr)
	    {geXDestImg(TImgPtr);
	     TImgPtr = ImgPtr;
	    }

	  if (!ImgPtr) return(NULL);
	 }
      }

    geGenFileExt(FileName, geDefProofSix, FALSE);
    if (!(f = fopen(geUtilBuf, "w", "rfm=var", "rat=cr")))
      {error_string = (char *) geFetchLiteral("GE_ERR_XOPENIMAGE", MrmRtypeChar8);
       if (error_string != NULL) 
	  {sprintf(geErrBuf, error_string, geUtilBuf);
	   geError(geErrBuf, FALSE);
	   XtFree(error_string);
	  }
       return(NULL);
      }

    ImgPtr->byte_order       = LSBFirst;
    ImgPtr->bitmap_bit_order = LSBFirst;

    if (hires) { /* -- 300 dpi -- */
       if (ImgPtr->depth == 1) {  /* -- lps40 300 dpi -- */
       /*  -- set dot size to device pixel -- */
          fputc(0X9b,f); fputc('7',f); fputc(0x20,f); fputc('I',f);

	/* -- sixel intro -- */
          fputc(27,f);  fputc('P',f);
          fputc('9',f);  fputc(';',f); fputc(';',f);
          fputc('1',f);  fputc('q',f);

       } else {	/*			-- lj250 90 dpi 256 colors -- */
	/*  -- sixel intro -- */
          fputc(27,f);  fputc('P',f);
          fputc('0',f);  fputc(';',f); fputc('0',f); fputc(';',f);
          fputc('1',f);  fputc('0',f); fputc('q',f); 

       }

    } else {/*   -- default dpi -- */
   /*
    * Set dot size to decipoints and print the sixel introducer
    */
    fputc(0X9b,f); fputc('2',f); fputc(0x20,f); fputc('I',f);
    fputc(27,f);  fputc('P',f); fputc('0',f);
    fputc(';',f); fputc('0',f); fputc(';',f);
    fputc('9',f); fputc('q',f);
    }
    /* raster attribs */
    fprintf(f,"\"1;1;%d;%d",cropw,croph);
    fprintf(f,"\n");

    pntr = (unsigned char *)ImgPtr->data;

    lcnt = 0;

    if (ImgPtr->depth == 1) {

    while (lcnt < ImgPtr->height) {

        cnt = 0;
        oldch = -1;
        chcnt = 0; 

        for (p = 1; p <= ImgPtr->bytes_per_line; p++) {

            power = 1;

            pntr0 = pntr;
            pntr1 = pntr + ImgPtr->bytes_per_line;
            pntr2 = pntr + ImgPtr->bytes_per_line * 2;
            pntr3 = pntr + ImgPtr->bytes_per_line * 3;
            pntr4 = pntr + ImgPtr->bytes_per_line * 4;
            pntr5 = pntr + ImgPtr->bytes_per_line * 5;

            for (i = 1; (i <= 8) && ((p-1) * 8 + i <= ImgPtr->width); i++) {

                xor = 5;

                if (lcnt <= ImgPtr->height - 6) 
                   ch =   (*pntr0 & power) / power +
                          (*pntr1 & power) / power * 2  +
                          (*pntr2 & power) / power * 4  +
                          (*pntr3 & power) / power * 8  +
                          (*pntr4 & power) / power * 16 +
                          (*pntr5 & power) / power * 32;
                 else {
                     ch = 0;
                     xor = ImgPtr->height % 6 - 1;
                     switch (xor) {
                       case 5: ch = ch + (*pntr5 & power) / power * 32;  /* this one should never happen */
                       case 4: ch = ch + (*pntr4 & power) / power * 16;
                       case 3: ch = ch + (*pntr3 & power) / power * 8;
                       case 2: ch = ch + (*pntr2 & power) / power * 4;
                       case 1: ch = ch + (*pntr1 & power) / power * 2;
                       case 0: ch = ch + (*pntr0 & power) / power;
                     }
                }

                ch = ch ^ xorarr[xor];     /* xor (not 6 bits) */

                if (ch == oldch || oldch == -1)
                   cnt = cnt + 1;
                else {
                       if (cnt > 1) {
                          chcnt = chcnt + 4;
                          fprintf(f,"!%d%c",cnt,oldch+63);
                       } else {
                          chcnt = chcnt + 1;
                          fprintf(f,"%c",oldch+63);
                       }
                       cnt = 1;
                }
                oldch = ch;
                power = power * 2;
                if (chcnt > 70) {
                  fprintf(f,"\n");
                  chcnt = 0;
                }
            } /* end for bits */
            pntr = pntr + 1;
        } /* end for bytes per line */

        if (cnt > 1)
           fprintf(f,"!%d%c",cnt,oldch+63);
        else
           fprintf(f,"%c",oldch+63);

        fprintf(f,"-\n");

        lcnt = lcnt + 6;

        pntr = (unsigned char *)ImgPtr->data + lcnt * ImgPtr->bytes_per_line;

    } /* end while rows */

    } /* end if depth == 1 */

    if (ImgPtr->depth > 1 || ImgPtr->bits_per_pixel > 1) {  /* lets generate color sixel */
       
       for (i = 0; i < numcols; i++) {
           fprintf(f,"#%d;2;%d;%d;%d\n",colortable[i].pixel,
                                  (int)(colortable[i].red   / 655.35),
                                  (int)(colortable[i].green / 655.35),
                                  (int)(colortable[i].blue  / 655.35) );
       }

       pntr = (unsigned char *)ImgPtr->data;

       lines = 0;
 
       while (lines < ImgPtr->height) {

          for (i = 0; i < numcols; i++)
              markers[i] = 0;

          pntr0 = pntr + ImgPtr->bytes_per_line * 0;
          pntr1 = pntr + ImgPtr->bytes_per_line * 1;
          pntr2 = pntr + ImgPtr->bytes_per_line * 2;
          pntr3 = pntr + ImgPtr->bytes_per_line * 3;
          pntr4 = pntr + ImgPtr->bytes_per_line * 4;
          pntr5 = pntr + ImgPtr->bytes_per_line * 5;

          for (x = 0; x < ImgPtr->width; x++) {
              markers[*(pntr0++)] = 1;                                                
              markers[*(pntr1++)] = 1;
              markers[*(pntr2++)] = 1;
              markers[*(pntr3++)] = 1;
              markers[*(pntr4++)] = 1;
              markers[*(pntr5++)] = 1;
          }

         for (i = 0; i < numcols; i++) {

             if (markers[i] == 1) {

             pntr0 = pntr + ImgPtr->bytes_per_line * 0;
             pntr1 = pntr + ImgPtr->bytes_per_line * 1;
             pntr2 = pntr + ImgPtr->bytes_per_line * 2;
             pntr3 = pntr + ImgPtr->bytes_per_line * 3;
             pntr4 = pntr + ImgPtr->bytes_per_line * 4;
             pntr5 = pntr + ImgPtr->bytes_per_line * 5;

                fprintf(f,"#%d",i);

                oldch = -1;
                cnt   = 0; chcnt = 0;
                for (x = 0; x < ImgPtr->width; x++) {
                    ch = 0;
                    if (*(pntr0++) == i) ch = ch | 1;
   
                    if (lines+1 < ImgPtr->height)
                       if (*(pntr1++) == i) ch = ch | 2;
                    if (lines+2 < ImgPtr->height)   
                       if (*(pntr2++) == i) ch = ch | 4;
                    if (lines+3 < ImgPtr->height)
                       if (*(pntr3++) == i) ch = ch | 8;
                    if (lines+4 < ImgPtr->height)
                       if (*(pntr4++) == i) ch = ch | 16;
                    if (lines+5 < ImgPtr->height)
                       if (*(pntr5++) == i) ch = ch | 32;

                    if (ch == oldch || oldch == -1)
                       cnt = cnt + 1;
                    else {
                           if (cnt > 1) {
                              chcnt = chcnt + 4;
                              fprintf(f,"!%d%c",cnt,oldch+63);
                           } else {
                              chcnt = chcnt + 1;
                              fprintf(f,"%c",oldch+63);
                           }
                           cnt = 1;
                         }
                    oldch = ch;

                    if (chcnt > 70) {
                       fprintf(f,"\n");
                       chcnt = 0;
                    }
                 } /* endfor for color */

              if (cnt > 1)
                 fprintf(f,"!%d%c",cnt,oldch+63);
              else
                 fprintf(f,"%c",oldch+63);

              fprintf(f,"$\n");   /* graphics carriage return */

              } /* end if markers */

           } /* end for for pixel line */

       fprintf(f,"-\n");  /* graphic new line */

       pntr = pntr + ImgPtr->bytes_per_line * 6;

       lines = lines + 6;

       } /* end for for line */
                     
    } /* end if for depth > 1 */
       
    fputc(27,f); fputc('\\',f);
    fclose(f);

    /*
     * Release image resources
     */
if (TImgPtr)
  geXDestImg(TImgPtr);

}
