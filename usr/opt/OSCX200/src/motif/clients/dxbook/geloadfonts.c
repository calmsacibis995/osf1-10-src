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
/* DEC/CMS REPLACEMENT HISTORY, Element GELOADFONTS.C*/
/* *3    25-JAN-1991 17:00:03 FITZELL "V3_EFT_24_JAN"*/
/* *2    12-DEC-1990 12:39:46 FITZELL "V3 IFT update"*/
/* *1     8-NOV-1990 11:24:00 FITZELL "V3 IFT"*/
/* DEC/CMS REPLACEMENT HISTORY, Element GELOADFONTS.C*/
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
**	GELOADFONTS			Loads Font names contained in "geFnt.dat"
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
**	GNE 02/17/87 Created
**
**--
**/

#include "geGks.h"
#include "geFnt.h"
#include <stdio.h>
         

extern char		*geMalloc();
extern XFontStruct 	*XLoadQueryFont();

geLoadFonts()
{                       
static FILE	*fd;
static char     *ptr;
static int	msgnum, i, j;

i = 0;
while (GenFnts[i])
  {geFntPtr[i] = GenFnts[i]; i++;}

geFntPtr[i] = NULL;

geGenFileName("geFnt.dat");
/*
 * If there is no additional font file, just silently return
 */
if ((fd = fopen(geUtilBuf, "r")) == NULL) return;

j               = 0;
geFntAlloced[0] = NULL;
while ((fscanf(fd, "%d", &msgnum) != EOF) && fgetc(fd)
       && fgets(geUtilBuf, GE_MAX_MSG_CHAR, fd))
   {if (msgnum == 0) continue;                  /* This is just a comment    */
    if (msgnum == -1) break;                    /* This is the END           */
    if (i >= GE_MAX_FNTS) {geError(GERRTOOMANYFNTS); break;}
    geUtilBuf[strlen(geUtilBuf) - 1] = '\0';
    GEMSGALLOC(ptr, geUtilBuf);
    geFntXName[i] = geFntAlloced[j++] = ptr;
    geFntPtr[i++] = ptr;
   }

fclose(fd);

while (j < GE_MAX_FNTS)
  geFntAlloced[j++] = NULL;

while (i < GE_MAX_FNTS)
  geFntPtr[i++] = NULL;
}
