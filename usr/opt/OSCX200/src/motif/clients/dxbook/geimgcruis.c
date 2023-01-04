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
/* DEC/CMS REPLACEMENT HISTORY, Element GEIMGCRUIS.C*/
/* *3    25-JAN-1991 16:59:36 FITZELL "V3_EFT_24_JAN"*/
/* *2    12-DEC-1990 12:39:21 FITZELL "V3 IFT update"*/
/* *1     8-NOV-1990 11:23:41 FITZELL "V3 IFT"*/
/* DEC/CMS REPLACEMENT HISTORY, Element GEIMGCRUIS.C*/
static char SccsId[] = "%W%\t%G%";
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
**	GEIMGCRUIS			Reads in a UIS bitmap
**
**  ABSTRACT:
**
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
#include "Xutil.h"


extern char   *geMalloc();
extern XImage *geXCrImg();

XImage *
geImgCrUis(FileName)
char  *FileName;
{                                                                         
XImage *ximg;

    FILE *f;
    char c2,c;
    int length, x1,y1,x2,y2, outatb, size, i, width, height, bits_per_pixel;
    char *pntr,*pntr2;

ximg = NULL;

    f = fopen(FileName,"r");

    if (f == NULL) 
       return(NULL);
    else {

    c2 = 0;

    while (c2 != 29 || c != 0) {
     c2 = c;
     c = fgetc(f);
     if (c == EOF)
        return(NULL);
    }

    length = read_integer(f);
    outatb = read_integer(f);
    x1     = read_long(f);
    y1     = read_long(f);
    x2     = read_long(f);
    y2     = read_long(f);
   
    y2     = read_long(f);

    width            = read_integer(f);
    height           = read_integer(f);
    bits_per_pixel   = read_integer(f);

    if (width <= 0 || height <= 0) {
       fclose(f);
       return(NULL);
    }

 ximg = geXCrImg(geDispDev, DefaultVisual(geDispDev, geScreen),
		 DefaultDepth(geDispDev, geScreen), 
		 XYPixmap,0,pntr,width,height,8,0);

   fclose(f);
   }
return(ximg);

}

static read_integer(f)
    FILE *f;

{   unsigned char c1,c2;
    c1 = fgetc(f);
    c2 = fgetc(f);
    return c1 + c2 * 256;
}

static read_long(f)
    FILE *f;

{   unsigned char c1,c2,c3,c4;
    c1 = fgetc(f);
    c2 = fgetc(f);
    c3 = fgetc(f);
    c4 = fgetc(f);

    return c1 + c2 * 0x00000100 +
                c3 * 0x00010000 + 
                c4 * 0x01000000;
}
