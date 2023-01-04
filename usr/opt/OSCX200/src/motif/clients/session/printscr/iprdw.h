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
/* BuildSystemHeader added automatically */
/* $Header: /usr/sde/x11/rcs/x11/src/./motif/clients/session/printscr/iprdw.h,v 1.1 90/01/01 00:00:00 devrcs Exp $ */
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
**  MODULE NAME:
**	iprdw.h
**
**  FACILITY:
**      PrintScreen
**
**  ABSTRACT:
**	internal only definitions for PrintScreen DECwindows
**
**  ENVIRONMENT:
**
**	VMS V5, Ultrix V2.2  DW FT1
**
**  AUTHORS:
**      Kathy Robinson, Mark Antonelli
**
**  RELEASER:
**	Kathy Robinson
**
**  CREATION DATE:
**	March 1, 1987 MFA
**
**  MODIFICATION HISTORY:
**
**	Jun-27-1989	B. Bazemore
**	    Add DEFAULT_FILENAME.
**
**--
**/


#define ISL
#define PRINTWID

#include <prdw.h>

#ifdef VMS
#include <decw$include/Xlib.h>
#include <decw$include/Xutil.h>
#else
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif


#define TRUE 		1
#define FALSE           0
#define BITSINBYTE	8
#define	BITSINTWOBYTES	16
#define	BITSINFOURBYTES	32
#define	BYTESINWORD	4

#ifndef NULL
#define NULL 0
#endif
   
/*
 * Postscript sizes of 8.5x11 piece of paper
 */
#define	PSPAGELEN	792
#define	UPSPAGELEN	756
#define PSPAGEWID	612
#define UPSPAGEWID	576

#define MAX_OUTPUT_BUFFER     256        /* Maximum PS line size  */

/*
 * String definitions
 */

#ifdef VMS
#define DEFAULT_FILENAME    "decw$printscreen.tmp"
#else
#define DEFAULT_FILENAME    "decw_printscreen.tmp"
#endif

/*
 * some typedefs
 */
typedef int bool;

typedef struct
{
	int	first;
	int	last;
	Colormap cmap;
	Visual	*visual;
	struct occinfo *next;
} occinfo;

typedef struct
{
	occinfo	*row;
} poccinfo;

/*
 * some simple functions
 */
#define	ROUNDUP(nbits,bits) ((((nbits) - 1 ) + (bits))/(bits))
#define	RoundUp(nbits,bits) ((((nbits) - 1 ) + (bits))/(bits))
#define EucDis(a, b) ( (a) > (b) ) ? ( (a) - (b) ) : ( (b) - (a) )
#define EUCDIS(a, b) ( (a) > (b) ) ? ( (a) - (b) ) : ( (b) - (a) )
#define Min(a, b) (( (a) > (b) ) ? (b) : (a))
#define MIN(a, b) (( (a) > (b) ) ? (b) : (a))
#define Max(a, b) (( (a) > (b) ) ? (a) : (b))
#define MAX(a, b) (( (a) > (b) ) ? (a) : (b))

#define DPIof(dpy)  (XDisplayHeight(dpy, XDefaultScreen(dpy))*25.4 / XDisplayHeightMM(dpy, XDefaultScreen(dpy)) )

/*
 * error definitions
 */
#define XERROR		dxPrscXError	/* internal X error		*/
#define	FUNERROR 	dxPrscFunError	/* functionality error (limited)*/

/*
 * options structure which should be compatable
 * in VMS, print_dest should be used as a pointer to a descriptor
 */
typedef struct
{
	long	aspect;
	long	print_color;
	long	reverse_image;
	long	storage_format;
	long	form_feed;
	long	print_queue;
	long	ungrab;
	char	*print_dest;
	Display	*dpy;
} dxPrscOptions ;

typedef dxPrscOptions DECW$C_PRSC_OPTIONS;

typedef struct
{
	char	DisplayName[512];
	dxPrscOptions	options;
} Command;


/*
 * the bad function
 */
#define BAD(status) !((status) & 1)

/*
 * this is just debug info, remove later
 */
#define image_info( image ) \
	fprintf(stderr, "\nImage size is %dx%d\n", image->height, image->width ); \
	fprintf(stderr,"Pixels offset in X directions %d\n", image->xoffset ); \
	fprintf(stderr,"Format is %d\n", image->format ); \
	fprintf(stderr,"Byte order is %d\n", image->byte_order); \
	fprintf(stderr,"Bitmap Unit %d\n", image->bitmap_unit); \
	fprintf(stderr,"Bitmap Bit Order %d\n", image->bitmap_bit_order); \
	fprintf(stderr,"Bitmap Pad %d\n", image->bitmap_pad); \
	fprintf(stderr,"Image depth is %d\n", image->depth); \
	fprintf(stderr,"Bytes per line %d\n", image->bytes_per_line ); \
	fprintf(stderr,"Bits per pixel is %d\n", image->bits_per_pixel )


/*
 * external subroutines
 */
XImage		*XGetImage();
Window		XWindow();
char		*malloc();
