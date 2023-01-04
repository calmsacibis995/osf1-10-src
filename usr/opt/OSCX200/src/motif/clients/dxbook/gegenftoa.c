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
/* DEC/CMS REPLACEMENT HISTORY, Element GEGENFTOA.C*/
/* *3    25-JAN-1991 16:57:07 FITZELL "V3_EFT_24_JAN"*/
/* *2    12-DEC-1990 12:37:00 FITZELL "V3 IFT update"*/
/* *1     8-NOV-1990 11:22:15 FITZELL "V3 IFT"*/
/* DEC/CMS REPLACEMENT HISTORY, Element GEGENFTOA.C*/
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
**	GEGENFTOA	             	       Converts given float to ascii
**
**  ABSTRACT:
**
**       The given float is converted to an ascii string containing 4 sig-
**       nificant digits to the right of the decimal point.
**       The end of the NULL terminated string is padded with an ascii space.
**       This is in an attempt to elliminate debris at the end of the string
**       when one string is written over another.  This seems to be a con-
**       sequence of the fact that spaces are smaller than digits in the some
**       fonts.
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems, RISC
**
**  MODIFICATION HISTORY:
**
**	GNE 05/04/87 Created
**
**--
**/

#include "geGks.h"


geGenFtoA(f, buf)
float   f;
char    buf[];
{                       
#define dig {it = i; i /= 10; buf[j--] = '0' + it - i * 10;}

static unsigned int i, i1, i2, it, j;

if (!f) {strcpy (buf, "     0.0    "); return;}

buf[12] = '\0';
buf[11] = ' ';
if (f < 0) {f = -f; buf[0] = '-';}
else       buf[0] = ' ';

i1 = (unsigned int)f;
i2 = i = (int)(10000. * (f - (float)i1));
j  = 11;

dig
dig
dig
dig
buf[j--] = '.';
i        = i1;
if (i)  dig
else   {buf[1] = buf[2] = buf[3] = buf[4] = buf[5] = buf[6] = ' '; return;}
if (i)  dig
else   {buf[1] = buf[2] = buf[3] = buf[4] = buf[5] = ' '; return;}
if (i)  dig
else   {buf[1] = buf[2] = buf[3] = buf[4] = ' '; return;}
if (i)  dig
else   {buf[1] = buf[2] = buf[3] = ' '; return;}
if (i)  dig
else   {buf[1] = buf[2] = ' '; return;}
if (i)  dig
else    buf[1] =' ';

}
