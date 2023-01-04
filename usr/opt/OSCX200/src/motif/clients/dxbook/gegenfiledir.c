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
static char SccsId[] = "@(#)gegenfiledir.c	1.1\t1/20/89";
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
**	GEGENFILEDIR		Generates full file path from given name
**	GEGENFILELEN		Generates length of proper filename
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
**	GNE 03/02/88 Created
**
**--
**/

#include "geGks.h"
         
extern char *geMalloc();

geGenFileDir(Name, Dir)
char *Name, *Dir;
{                       
char *LocBuf;
int LocBufSize, offset;

if (!Name || !strlen(Name))
  {geUtilBuf[0] = '\0';
   return;
  }

LocBuf = geMalloc((LocBufSize = strlen(Name) + 3));
strcpy(LocBuf, Name);

if (Dir && strlen(Dir))
  {if (!(offset = geGenParseF(Name)))
     {if (Dir != geUtilBuf) strcpy(geUtilBuf, Dir);
#ifndef VMS
      if (Dir[strlen(Dir) - 1] != '/')
	strcat(geUtilBuf, "/");
#endif
      strcat(geUtilBuf, LocBuf);
     }
   else
     strcpy(geUtilBuf, LocBuf);
  }
else
  strcpy(geUtilBuf, LocBuf);
geFree(&LocBuf, LocBufSize);
}

/****************************************************************************/
geGenFileLen(Name)
char	*Name;
{
static char  locbuf[1000];
int	     i,	offset;

if (!Name || !strlen(Name))  return(0);

offset = geGenParseF(Name);			/* Make copy of original */
strcpy(locbuf, Name + offset);                  /* without path name     */

i = 0;
while (locbuf[i] && locbuf[i] != '.') i++;	/* Dump anything past the '.' */
locbuf[i] = '\0';

return (strlen(locbuf));			/* Return length of proper */
}


