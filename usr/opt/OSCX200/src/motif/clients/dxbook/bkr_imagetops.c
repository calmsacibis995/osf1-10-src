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
/*
***************************************************************
**  Copyright (c) Digital Equipment Corporation, 1988, 1991  **
**  All Rights Reserved.  Unpublished rights reserved	     **
**  under the copyright laws of the United States.  	     **
**  	    	    	    	    	    	    	     **
**  The software contained on this media is proprietary	     **
**  to and embodies the confidential technology of  	     **
**  Digital Equipment Corporation.  Possession, use, 	     **
**  duplication or dissemination of the software and	     **
**  media is authorized only pursuant to a valid written     **
**  license from Digital Equipment Corporation.	    	     **
**  	    	    	    	    	    	    	     **
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 	     **
**  disclosure by the U.S. Government is subject to 	     **
**  restrictions as set forth in Subparagraph (c)(1)(ii)     **
**  of DFARS 252.227-7013, or in FAR 52.227-19, as  	     **
**  applicable.	    	    	    	    	    	     **
***************************************************************
*/


/*
 * This code courtesy of Mike Swatko, CUIP Strategic Tools and Consulting Grp.
 */

/*
 * To produce an image of the proper luminosity, I had to invert the bits
 * using the '~' operator.  This inversion code was tested only for depth == 1
 * images.  It remains to be tested for depth != 1 images.  -Mike
 */

/* ***********************************************************
 * Be sure to replace all printf's with your appropriate error
 * reporting methods!
 * ***********************************************************
 */

#include	<string.h>
#include	<stdio.h>
#include	<time.h>
#include        <X11/Xlib.h>
#include        <X11/Xutil.h>
#include        "br_meta_data.h"
#include        "br_typedefs.h"
#include        "br_common_defs.h"
#include        "br_globals.h"
#include        "br_printextract.h"
#include        "bkr_imagetops.h"

extern int
bkr_imagetops(image,fp,colortable,x_off,y_off,y_max,imagesize,
                  out_fname,parent_w)
    XImage *image;
    FILE   *fp;
    XColor colortable[256];
    int    x_off, y_off, y_max;
    int    imagesize;
    char   *out_fname;
    Widget parent_w;
{
    float picas_width;
    int  fullbytes_per_line;
    int  width_pts,height_pts;
    int  llx,lly,urx,ury;
    register unsigned int  x,y,xbits,cnt;
    unsigned long  pix_val;
    unsigned char pspix, pspix1, pspix2, pspix3, pspix4;
    char *pntr;
    char *error_string;
    char workbuf[256];

    time_t time_val;
    unsigned char  greytable[256];	/*unsigned int*/

    if (image->depth != 1 &&
	image->depth != 2 &&
	image->depth != 4 &&
	image->depth != 8) {
	printf("\n Image depth must be 1, 2, 4 or 8\n");
	return(0);
    }

    if (fp == NULL)
       return(0);

    /* Note: Order of multiplies and divides in the following lines are */
    /* important!  Multiplies must happen first because we are using integers */
    fullbytes_per_line = image->width * image->depth / 8;
    if (fullbytes_per_line * 8 / image->depth < image->width)
	fullbytes_per_line = fullbytes_per_line + 1;

    /* set pixel size to 300dpi.  let compiler to the optimization */
    width_pts = image->width * imagesize;
    height_pts = image->height * imagesize;

    sprintf(workbuf,"\nsave\n");
    F_PUTS(workbuf);
    sprintf(workbuf,"/picstr %d string def\n", fullbytes_per_line);
    F_PUTS(workbuf);

    /* sprintf(workbuf,"%d %d SXY Xpos Ypos translate\n",x_off, y_off); */
    sprintf(workbuf,"%d %d translate\n",x_off, y_max - y_off);
    F_PUTS(workbuf);

    sprintf(workbuf,"%d %d scale\n", width_pts, height_pts);
    F_PUTS(workbuf);
    sprintf(workbuf,"%d %d %d [%d 0 0 -%d 0 %d]\n",
	image->width, image->height, image->depth, image->width, image->height,
	image->height);
    F_PUTS(workbuf);
    sprintf(workbuf,"{currentfile picstr readhexstring pop} image\n");
    F_PUTS(workbuf);

    if (image->depth == 1) {
        for (y = 0; y < image->height; y++) {
            pntr = image->data + image->bytes_per_line * y;
            cnt = 0;
            for (x = 1; x <= fullbytes_per_line; x++) {
                pspix = *pntr;
                pspix =   ((pspix &   1) << 7) |
                        ((pspix &   2) << 5) |
                        ((pspix &   4) << 3) |
                        ((pspix &   8) << 1) |
                        ((pspix &  16) >> 1) |
                        ((pspix &  32) >> 3) |
                        ((pspix &  64) >> 5) |
                        ((pspix & 128) >> 7);
		pspix = ~pspix;
    
                sprintf(workbuf,"%02X",pspix);
                F_PUTS(workbuf);
                pntr++; cnt++;
                if (cnt >= 39) {
                   sprintf(workbuf,"\n");
                   F_PUTS(workbuf);
                   cnt = 0;
                }
            }
            sprintf(workbuf,"\n");
            F_PUTS(workbuf);
        }    
    } else if (image->depth == 8) {
	for (x = 0; x < 256; x++)
	    greytable[colortable[x].pixel] = ~((colortable[x].red
					    + colortable[x].blue
					    + colortable[x].green) / 771);
        for (y = 0; y < image->height; y++) {
	    cnt = 0;
            for (x = 0; x < image->width; x++) {
		sprintf(workbuf,"%02X",greytable[XGetPixel(image,x,y)]);
                F_PUTS(workbuf);
		cnt++;
		if (cnt >= 39) {
		    sprintf(workbuf,"\n");
		    F_PUTS(workbuf);
                    cnt = 0;
		}
            }
            sprintf(workbuf,"\n");
            F_PUTS(workbuf);
        }    
    } else if (image->depth == 4) {
	for (x = 0; x < 16; x++)
	    greytable[colortable[x].pixel] = ~((colortable[x].red
					    + colortable[x].blue
					    + colortable[x].green) / 771);
        for (y = 0; y < image->height; y++) {
	    cnt = 0;
            for (x = 0; x < image->width; x += 2) {
		pspix1 = greytable[XGetPixel(image,x,y)];
		if (x+1 < image->width)
		    pspix2 = greytable[XGetPixel(image,x+1,y)];
		else
		    pspix2 = 0;
		pspix = ((pspix1 & 15) << 4) | (pspix2 & 15);
		sprintf(workbuf,"%02X",pspix);
                F_PUTS(workbuf);
		cnt++;
		if (cnt >= 39) {
		    sprintf(workbuf,"\n");
                    F_PUTS(workbuf);
		    cnt = 0;
		}
            }
            sprintf(workbuf,"\n");
            F_PUTS(workbuf);
        }    
    } else if (image->depth == 2) {
	for (x = 0; x < 4; x++)
	    greytable[colortable[x].pixel] = ~((colortable[x].red
					    + colortable[x].blue
					    + colortable[x].green) / 771);
        for (y = 0; y < image->height; y++) {
	    cnt = 0;
            for (x = 0; x < image->width; x += 4) {
		pspix1 = greytable[XGetPixel(image,x,y)];
		if (x+1 < image->width)
		    pspix2 = greytable[XGetPixel(image,x+1,y)];
		else
		    pspix2 = 0;
		if (x+2 < image->width)
		    pspix3 = greytable[XGetPixel(image,x+2,y)];
		else
		    pspix3 = 0;
		if (x+3 < image->width)
		    pspix4 = greytable[XGetPixel(image,x+3,y)];
		else
		    pspix4 = 0;
		pspix = ((pspix1 & 3) << 6) | ((pspix2 & 3) << 4)
			| ((pspix3 & 3) << 2) | (pspix4 & 3);
		sprintf(workbuf,"%02X",pspix);
                F_PUTS(workbuf);
		cnt++;
		if (cnt >= 39) {
		    sprintf(workbuf,"\n");
                    F_PUTS(workbuf);
		    cnt = 0;
		}
            }
            sprintf(workbuf,"\n");
            F_PUTS(workbuf);
        }    
    }
    sprintf(workbuf,"restore\n\n");
    F_PUTS(workbuf);

    return(0);
}


#if 0


/* this routine will read the fse binary format and return that as */
/* an image structure                                              */

int ort_ximage_read_fsebin(image,filename)
    XImage *image;
    char   *filename;
{
    FILE *f;
    int		x,y,bytes,density,width,height,size;
    unsigned char *pntr,tag1,tag2,tag3,format,orient,scalecnt,
		  flags,cropbackpix,junk;
    XImage *timage;

    f = fopen(filename,"r");

    if (f == NULL)
	return(0);

    tag1 = fgetc(f);		/* identifying tag, should be "CSE" */
    tag2 = fgetc(f);
    tag3 = fgetc(f);

    format = fgetc(f);		/* format, "C" Huffman compression,	*/
				/*	   "N"ormal,			*/
				/*	   "R" Read compression		*/

    orient = fgetc(f);		/* orientation */

    if (tag1 != 'C' || tag2 != 'S' || tag3 != 'E' || format != 'N') {
       fclose(f);
       return(0);
    }

    width  = read_long(f);	/* image width	*/
    height = read_long(f);	/* image height	*/

    density = read_long(f);	/* density	*/
    
    scalecnt = fgetc(f);	/* scale cnt	*/
    flags = fgetc(f);		/* flags	*/

    cropbackpix = fgetc(f);	/* if CROP flag is set, */
				/* num pix to crop back */

    if ((flags & 1) != 1) {
	cropbackpix = 0;
    }

    timage = XCreateImage(bkr_display,global_visual,1,XYBitmap,0,NULL,
		width,height,8,0);

    size = timage->bytes_per_line * timage->height;

    timage->data = XtMalloc(size);

    bytes = (width - 1) / 8 + 1;

    /* remove header record padding */
    for (x = 21; x <= bytes; x++)
        junk = fgetc(f);

    for (y = 0; y < timage->height; y++) {
        pntr = timage->data + timage->bytes_per_line * y;

        for (x = 0; x < bytes; x++, pntr++) {
            *pntr = fgetc(f);
       }
   }

   fclose(f);

   if (image->data != NULL)
      XtFree(image->data);

   if (cropbackpix != 0) {
	*image = *XSubImage(timage,0,0,width - cropbackpix,height);
   } else {
	*image = *timage;
   }

/*   if (timage->bits_per_pixel == 1)
 *      ort_ximage_invert_all(image);
 */

   return(1);
}

#endif
