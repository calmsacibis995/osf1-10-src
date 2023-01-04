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
static char SccsId[] = "@(#)geximagetofid.c	1.7\t10/5/89";
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
**	XIMAGETOFID			Gets an ISL fid given an Ximg struct
**
**  ABSTRACT:
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 01/20/89 Created (compliments of Jeff Orthober)
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

#ifdef GEDDIF

#ifdef VMS
#include <ddif$def.h>
#else
#include <ddif_def.h>
#endif

#endif

#endif

geXimageToFid(image,fid, cropw, croph)
	XImage *image;
	int    *fid;
	int	cropw, croph;
{
#ifdef GEISL

	int stride, itmlst[60], size, boxw, boxh, zero,
	    CurRes, PixWidth, MMWidth, frame_fixed, value_constant;
	char label[100];

	strcpy (label, "DocGraphics Generated Image");

	if (cropw > (stride = image->bytes_per_line << 3)) cropw = stride;
	if (croph <= 0 || croph > image->height) croph = image->height;

	zero       = 0;
	PixWidth   = DisplayWidth  (geDispDev, geScreen);
	MMWidth    = DisplayWidthMM(geDispDev, geScreen);
	CurRes     = (int)((float)PixWidth / ((float)MMWidth / 25.4));

	boxw       = cropw * 1200 / CurRes;
	boxh       = croph * 1200 / CurRes;

	itmlst[0]  = IMG$_SCANLINE_STRIDE;
	itmlst[1]  = sizeof(int);
	itmlst[2]  = (int)&stride;
	itmlst[3]  = 0;

	itmlst[4]  = IMG$_NUMBER_OF_LINES;
	itmlst[5]  = sizeof(int);
	itmlst[6]  = (int)&croph;
	itmlst[7]  = 0;

	itmlst[8]  = IMG$_PIXELS_PER_LINE;
	itmlst[9]  = sizeof(int);
	itmlst[10] = (int)&cropw;
	itmlst[11] = 0;

	itmlst[12] = IMG$_USER_LABEL;
	itmlst[13] = strlen(label);
	itmlst[14] = (int)label;
	itmlst[15] = 0;

	itmlst[16] = IMG$_BITS_PER_PIXEL;
	itmlst[17] = sizeof(int);
	itmlst[18] = (int)&image->bits_per_pixel;
	itmlst[19] = 0;

#ifdef GEDDIF

	frame_fixed = DDIF$K_FRAME_FIXED;
	itmlst[20] = IMG$_FRM_POSITION_C;

#endif

	itmlst[21] = sizeof(int);
	itmlst[22] = (int)&frame_fixed;
	itmlst[23] = 0;

#ifdef GEDDIF
	value_constant = DDIF$K_VALUE_CONSTANT;
#else
	value_constant = 0;
#endif

	itmlst[24] = IMG$_FRM_BOX_LL_X_C;
	itmlst[25] = sizeof(int);
	itmlst[26] = (int)&value_constant;
	itmlst[27] = 0;

	itmlst[28] = IMG$_FRM_BOX_LL_X;
	itmlst[29] = sizeof(int);
	itmlst[30] = (int)&zero;
	itmlst[31] = 0;

	itmlst[32] = IMG$_FRM_BOX_LL_Y_C;
	itmlst[33] = sizeof(int);
	itmlst[34] = (int)&value_constant;
	itmlst[35] = 0;

	itmlst[36] = IMG$_FRM_BOX_LL_Y;
	itmlst[37] = sizeof(int);
	itmlst[38] = (int)&zero;
	itmlst[39] = 0;

	itmlst[40] = IMG$_FRM_BOX_UR_X_C;
	itmlst[41] = sizeof(int);
	itmlst[42] = (int)&value_constant;
	itmlst[43] = 0;

	itmlst[44] = IMG$_FRM_BOX_UR_X;
	itmlst[45] = sizeof(int);
	itmlst[46] = (int)&boxw;
	itmlst[47] = 0;

	itmlst[48] = IMG$_FRM_BOX_UR_Y_C;
	itmlst[49] = sizeof(int);
	itmlst[50] = (int)&value_constant;
	itmlst[51] = 0;

	itmlst[52] = IMG$_FRM_BOX_UR_Y;
	itmlst[53] = sizeof(int);
	itmlst[54] = (int)&boxh;
	itmlst[55] = 0;

	itmlst[56] = 0;

	*fid = ImgCreateFrame(itmlst, IMG$K_STYPE_BITONAL);

	size = image->bytes_per_line * image->height;

	ImgImportBitmap(*fid,image->data,size,0,0,0,0);

#endif

}
