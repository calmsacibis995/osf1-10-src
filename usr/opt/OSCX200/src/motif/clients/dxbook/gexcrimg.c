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
static char SccsId[] = "@(#)gexcrimg.c	1.3\t5/15/89";
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
**	GEXCRIMG			Layer routine for XCreateImage
**
**  ABSTRACT:
**
**      This routine is required because of the inconsistent way
**      XGetImage and XCreateImage handle the image data buffer allocation.
**      XGetImage uses an internal allocation routine to create the buffer
**      necessary to hold the image, while XCreateImage requires that the
**      user allocate his own buffer.  When it comes time to release the
**      image resources, it is necessary to allow XDestroyImage to free
**      the image buffer, if it was created with XGetImage, BUT it is
**      essential that it not be allowed to free the buffer if the image
**      was created with XCreateImage.  In this case the user must take
**      of freeing the buffer.  To try to make this work I've written
**      layer routines for XCreateImage and XDestroyImage.  In the
**      XCreateImage layer routine I've nulled out the pointer to the
**      Destroy routine, so that when the XDestroyImage layer routine
**      is called it can check this pointer and if it's NULL, it can
**      free the image buffer and call XFree to release the X image
**      struct;  if it's not NULL, then it can go ahead and call
**      XDestroyImage.
**      
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 03/24/89 Created     
**
**--
**/

#include "geGks.h"
#include <X11/Xutil.h>


XImage *
geXCrImg(display, visual, depth, format, offset, data, width, height,
	 bitmap_pad, bytes_per_line)
Display *display;
Visual *visual;
unsigned int depth;
int format;
int offset;
char *data;
unsigned int width;
unsigned int height;
int bitmap_pad;
int bytes_per_line;
{                                                                         
XImage *          ImgPtr;

ImgPtr                   = XCreateImage(display, visual, depth, format,
					offset, data, width, height,
					bitmap_pad, bytes_per_line);
if (ImgPtr->depth == 1)
    ImgPtr->width        = ImgPtr->bytes_per_line << 3;
ImgPtr->f.destroy_image  = NULL;
ImgPtr->byte_order       = LSBFirst;
ImgPtr->bitmap_bit_order = LSBFirst;

return(ImgPtr);
}






