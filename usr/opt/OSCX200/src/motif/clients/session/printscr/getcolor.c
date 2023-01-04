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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader = "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/session/printscr/getcolor.c,v 1.1 90/01/01 00:00:00 devrcs Exp $";
#endif		/* BuildSystemHeader */

/*
**++
**  COPYRIGHT (c) 1987, 1988, 1989 BY
**  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
**  ALL RIGHTS RESERVED.
**
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
**  ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
**  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
**  OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
**  TRANSFERRED.
**
**  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
**  AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
**  CORPORATION.
**
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
**--
**/
/*
**++
**  MODULE:
**
**	getcolor.c
**
**  FACILITY:
**      PrintScreen
**
**  ABSTRACT:
**
**	Subroutines which get and use the color infomation
**		   associated with an image
**
**  ENVIRONMENT:
**
**	VMS V5, Ultrix V2.2  DW FT1
**
**  AUTHORS:
**      Kathy Robinson
**
**  RELEASER:
**	Kathy Robinson
**
**  CREATION DATE:
**	26-OCT-1987
**
**  MODIFICATION HISTORY:
**	11-JAN-1987
**		added optins struc to fiximage parameters
**		fixed code that handles last byte of image
**
**
**
 *	Dec	11,	1987, KMR
 *		Converts image to B&W
 *	Oct	26,	1987, KMR
 *		Gets color info
**
**--
**/
 /*
 *	getcolor(dpy,win,vis)
 *		dpy	   - (RO) (*Display) the opened display
 *		win	  - (WO) (*Window) window ID
 *		vis	  - (WO) (*Visual) visual of win
 *
 *	fiximage(ximage,dpy,vis)
 *		ximage	  - (RO) (*XImage) the image structure
 *		dpy	  - (RO) (*Display) the opened display
 *		vis	  - (RO) (*Visual) visual of win
 *
 *	restrictions:
 *
 *	environment:
 *		VMS 4.5, [Ultrix 2.0], X11 BL 6.3
 *
 *	notes:
 *
 */


#include	<stdio.h>
#include	<iprdw.h>
#include	<ChfDef.h>



/* Several MACROS for getting/putting info in and out of bit arrays, described*/
/* by the parameters in descriptor class A or base/bit offset. Based on shift */
/* and mask algortihm and will work up to getting/putting 25 bit values.      */
/* IMG$AL_ONES_ASCENDING[] is a global array of values 0, 1, 11, 111, 1111 ...*/
/* Bits must be in LSB first order!					      */

/* get value nbits long (up to 25) described by the base and bit offset      */

#define $GET_VALUE(base,offset,nbit) \
   (*((int *)(base + ( offset >> 3))) >> \
   (offset & 0x7) & IMG$AL_ONES_ASCENDING[nbit])

/* put value nbits long (up to 25) described by the base and bit offset      */

#define PUT_VALUE(base,offset,nbit,value) \
   *((int *)(base + (offset >> 3))) ^= \
   (value & IMG$AL_ONES_ASCENDING[nbit]) << (offset & 0x7)

static int IMG$AL_ONES_ASCENDING[26]
    = { 0X0, 0X1, 0X3, 0X7,
	0X0F, 0X1F, 0X3F, 0X7F,
	0X0FF, 0X1FF, 0X3FF, 0X7FF,
	0X0FFF, 0X1FFF, 0X3FFF, 0X7FFF,
	0X0FFFF, 0X1FFFF, 0X3FFFF, 0X7FFFF,
	0X0FFFFF, 0X1FFFFF, 0X3FFFFF, 0X7FFFFF,
	0X0FFFFFF, 0X1FFFFFF };



XColor	*cmap = NULL;



getcolor( dpy, dfs, win, vis)
Display	*dpy;
int  		dfs;
Window		win;
Visual		*vis;
{
	Colormap 	*cmapid;
	int		size, i;
	int		n;		/* number of args returned by X call */
	XWindowAttributes	winattr;
	float		gray;
	XVisualInfo	*visinfo, visinfo_temp;

        XGetWindowAttributes(dpy, win, &winattr);

	cmapid = XListInstalledColormaps(dpy, win, &n);
	if ( (n != 1) || (winattr.colormap != *cmapid) )
		{
		return(XERROR);
		}

	if (vis->class == TrueColor)
		{
		cmap = NULL;
		return (Normal);
		}

	size = XDisplayCells(dpy,dfs); 
	if (size > 1024) size = 1024;     /* Lynx could be bigger */

	cmap = (XColor *)malloc (size*sizeof(XColor));
	if (!cmap)
		return (dxPrscNoMemory);


/*  Hack Alert Hack Alert !!!!!!!!! Hack Alert Hack Alert */
/*    I consider this a workaround for a server bug */

        if (size == 2)
                {
		cmap = (XColor *)realloc(cmap, 256*sizeof(XColor)); 
                cmap[254].red = cmap[254].green = cmap[254].blue = 0;
                cmap[255].red = cmap[255].green = cmap[255].blue = 0XFFFF;
                }
        else
        {
/*  Hack Alert Hack Alert !!!!!!!!! Hack Alert Hack Alert */
/* End of Workaround - but delete the closing } below ! */



	for (i=0; i<size; i++)
	    cmap[i].pixel = i;
	XQueryColors(dpy, *cmapid, cmap, size);
	} 
	XFree(cmapid);
	return (Normal);
}



int
fiximage (image, dpy, dfs, vis, options)
XImage	**image;
Display	*dpy;
int dfs;
Visual		*vis;
dxPrscOptions *options;
{
XImage	*ximage, 	/* The orignal image */
	*newim;         /* The "fixed" image */
unsigned char	*rbuf,	/* buffer to hold row of image		*/
	*brbuf,		/* always points to beginning of rbuf	*/
	*rowptr;	/* pointer to a row of original image data */
int	row,column;	/* image row and column indices		*/

unsigned char *ptr,	/* pointer to start of new image */
	*nimrowptr,	/* pointer to start of current row of new image */
	*nimptr;	/* pointer to current byte of new image */
int 	hei, wid;
int 	i,j;
int	pad, nbits;

ximage = *image;
if (cmap == NULL)
	return(Normal);

hei = ximage->height;
wid = ximage->width;
nimptr = ptr = (unsigned char *)malloc(hei* ximage->bytes_per_line + 1);
if (nimptr == NULL)
	return (dxPrscNoMemory);

/*
 * process data row by row, making appropriate conversions
 * if bytes or bits need to be reversed
 */
rbuf = (unsigned char*)malloc( ximage->bytes_per_line );
if (!rbuf)
	return (dxPrscNoMemory);
brbuf = rbuf;
rowptr = (unsigned char *)ximage->data;
nbits = ximage->bits_per_pixel;

for( row = 0, nimrowptr = ptr; row < ximage->height; row++, 
	rowptr += ximage->bytes_per_line, nimrowptr += ximage->bytes_per_line)
{
	bcopy( rowptr, brbuf, ximage->bytes_per_line );
	nimptr = nimrowptr;
	if( ximage->byte_order != LSBFirst )
	{
		switch( ximage->bitmap_unit )
		{
			case 8:
				break;
			case 16:
				swapshort( brbuf, ximage->bytes_per_line);
			    	break;
			default:
				swaplong( brbuf, ximage->bytes_per_line );
		}
	}


	/*
	 * now take each (8bit) pixel as an index into bit (array of 8 bit
	 * rgb values) and write the rgb out to the new image 
	 */                                    
	for( column = 0, rbuf = brbuf;
		column < ximage->width;
		rbuf++,  column++)
		{
		*nimptr =  (cmap[*rbuf].red >>13) & 7;
		*nimptr |= (cmap[*rbuf].green >>10) & 0X38;
		*nimptr |= (cmap[*rbuf].blue >>8) & 0X0C0;
		nimptr++;
		}
}


newim = XCreateImage (dpy,  XDefaultVisual (dpy, dfs),  8,
	ZPixmap, ximage->xoffset, ptr,	ximage->width,
	ximage->height, ximage->bitmap_pad, 
	ximage->bytes_per_line);

if (!newim) {
	free(cmap);
	free(brbuf);
	return (dxPrscNoImage);
}

newim->byte_order = LSBFirst;
newim->bitmap_bit_order = LSBFirst;
newim->bitmap_unit = 8;
newim->bits_per_pixel = 8;

free(cmap);
free(brbuf);
XDestroyImage (ximage);
*image = newim;
return (Normal);
}
