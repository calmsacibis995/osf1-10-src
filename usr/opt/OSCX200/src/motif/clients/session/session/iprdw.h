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
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "prdw.h"

#define BITSINBYTE	8
#define	BITSINTWOBYTES	16
#define	BITSINFOURBYTES	32
#define	BYTESINWORD	4

/* 
 * Postscript sizes of 8.5x11 piece of paper 
 */
#define	PSPAGELEN	792
#define	UPSPAGELEN	756
#define PSPAGEWID	612
#define UPSPAGEWID	576

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

/*
 * error definitions
 */
#define XERROR		dxPrscFunError	/* internal X error		*/
#define	FUNERROR 	dxPrscXError	/* functionality error (limited)*/

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
} dxPrscOptions ;

typedef dxPrscOptions DECWC_PRSC_OPTIONS;

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
