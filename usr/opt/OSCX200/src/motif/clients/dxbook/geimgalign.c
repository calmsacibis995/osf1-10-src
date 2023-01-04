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
static char SccsId[] = "@(#)geimgalign.c	1.5\t8/29/89";
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
**	GEIMGALIGN			Image alignment routine
**
**  ABSTRACT:
**
**      This routine will align the width of the given image to the
**      supplied factor.
**      
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 04/12/89 Created     
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

geImgAlign(image, alignment)
     XImage **image;
     int    alignment;
{

#ifdef GEISL

  int stride, nuitmlst[21], fid, nufid, dummy, size;
  char label[21];

strcpy (label, "RAGS Generated Image");

/*
 * Check requested alignment and make sure it is necessary
 */
if (alignment < 1 || !image || !*image ||
    (((*image)->width / alignment) * alignment) == (*image)->width) return(-1);

  geXimageToFid(*image, &fid, (*image)->width, (*image)->height);
  /*
   * Align the image
   */
  (*image)->bytes_per_line =
    ((((*image)->width + alignment - 1) / alignment) * alignment) >> 3;
  (*image)->width       = stride = (*image)->bytes_per_line << 3;
  nuitmlst[0]           = IMG$_SCANLINE_STRIDE;
  nuitmlst[1]           = 4;
  nuitmlst[2]           = (int)&stride;
  nuitmlst[3]           = 0;
  nuitmlst[4]           = 0;

  nufid                 = ImgCopy(fid, 0, nuitmlst);
  ImgDeleteFrame(fid);
  fid                   = nufid;
  nuitmlst[0]           = IMG$_PIXELS_PER_LINE;
  nuitmlst[1]           = 4;
  nuitmlst[2]           = (int)&((*image)->width);
  nuitmlst[3]           = 0;
  nuitmlst[4]           = 0;
  fid = ImgSetFrameAttributes(fid, nuitmlst);

  geXDestImg(*image);
  geFidToXimage(image, fid);
  ImgDeleteFrame(fid);

#endif

}
