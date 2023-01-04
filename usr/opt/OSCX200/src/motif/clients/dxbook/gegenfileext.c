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
static char SccsId[] = "@(#)gegenfileext.c	1.1\t11/22/88";
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
**      GEGENFILEEXT             		Combines file name and extension
**
**  ABSTRACT:
**
**  The function of this routine is:
**	1) Resolve "~", in the case of ULTRIX and "-", in the case of VMS
**         because fopen can't seem to figure them out.
**		and
**      2) To attach the requested extension onto the file name, if necessary
**
**  The resultant filename is provided in "geUtilBuf".
**
**  If the Override flag is TRUE the new extension is forced after the "."
**  in the name, or if there is no ".", ".ext" is tacked on at the end of the
**  buff.
**  If the Override flag is FALSE, then
**  this routine behaves sligthly differently depending on whether it is
**  compiled on ULTRIX or VMS.
**  ULTRIX
**  ______
**  If the name ends in a "." WITHOUT an extension, the new extension is tacked
**  on.
**  Otherwise it is left UNTOUCHED.
**
**
**  VMS
**  ___
**  If the name does NOT contain a ".", the new extension is tacked on.
**  If the name ends in a "." WITHOUT an extension, the "." is stripped off.
**  Otherwise it is left UNTOUCHED.
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**      GNE 02/16/88 Created
**
**--
**/

#include "geGks.h"

extern char *geMalloc();

geGenFileExt(Str, Ext, Override)
char  *Str, *Ext;
short Override;
{                       
static char *p, *p1, *p2, *locbuf;
int         Slen, Slene, Shift;

if (!Str || !(Slen = strlen(Str)))	       /* CAN'T let this happen    */
  {strcpy(geUtilBuf, geDefRags);               /* Avoid crash              */
   return;
  }

locbuf = geMalloc(Slen + 1);
strcpy (locbuf, Str);                          /* Make copy of original str*/

geGenFileHome(locbuf);

if (!Ext || !(Slene = strlen(Ext)) || !(Slen = strlen(geUtilBuf)))
  {strcpy(geUtilBuf, locbuf);
   return;
  }

geFree(&locbuf, NULL);			       /* Allocate a buffer large  */
locbuf = geMalloc(Slen + Slene + 1);	       /* enough to hold full name */
					       /* and extension.           */
strcpy (locbuf, geUtilBuf);                    /* Make copy of string      */
p = locbuf + Slen - 1;			       /* Point at last char in str*/
/*
 * Search backwards from end of string for the beginning of the extension "."
 */
#ifdef VMS
while (p != locbuf && *p != '.')	       /* VMS			   */
  {if (*p == '.') break;                       /* Found "." go on to next  */
   if (*p == ']')                              /* Found "dir" end designatr*/
     {while (*p && *p != ';') p++;             /* - no ".", search forward */
      break;                                   /* for ";" this is where "."*/
     }                                         /* should go                */
   p--;
  }
if (p == locbuf) p = locbuf + Slen;

#else					       /* ULTRIX                   */
while(p != locbuf && *p != '.') p--;
if (*p != '.') p = locbuf + Slen;
#endif

/*
 * p should be pointing to either the "." or to where the "." should be
 */
if (Override)                                  /* Force in an extension    */
  {if (*p != '.' && *p != '\0')
     {Shift = strlen(Ext) + 1;                 /* Have to INSERT the extent*/
      p1 = locbuf + Slen;                      /* Shift to the right the   */
      p2 = p1 + Shift;                         /* stuff that is there now, */
      while (p1 >= p) *p2-- = *p1--;           /* before which want to     */
      *p++ = '.';                              /* insert ".Ext".           */
      p1   = Ext;                              /* Have inserted the ".",   */
      while (*p1) *p++ = *p1++;                /* now insert the extension */
     }
   else
     {*p++ =  '.';                             /* Just tack the ".Ext" onto*/
      *p   =  '\0';                            /* the end of the           */
      strcat(locbuf, Ext);                     /* string                   */
     }
   strcpy(geUtilBuf, locbuf);
   geFree (&locbuf, NULL);
   return;
  }

#ifdef VMS
if(*p == '\0')
     {*p++ =  '.';                             /* Just tack the ".Ext" onto*/
      *p   =  '\0';                            /* the end of the           */
      strcat(locbuf, Ext);                     /* string                   */
     }
else
if(*p == ';')                                  /* Only do it for "notdot;?"*/
     {Shift = strlen(Ext) + 1;                 /* Have to INSERT the extent*/
      p1 = locbuf + Slen;                      /* Shift to the right the   */
      p2 = p1 + Shift;                         /* stuff that is there now, */
      while (p1 >= p) *p2-- = *p1--;           /* before which want to     */
      *p++ = '.';                              /* insert ".Ext".           */
      p1   = Ext;                              /* Have inserted the ".",   */
      while (*p1) *p++ = *p1++;                /* now insert the extension */
     }
#else
if(*p == '.')
  {if (*(p + 1) == '\0')
     strcat(locbuf, Ext);
  }
#endif

strcpy(geUtilBuf, locbuf);
geFree (&locbuf, NULL);
}

/*
 * This routine will resolve "~" in the filename (if present)
 * and will substitute the path "geUsrLoc" for the benefit of "fopen".
 * The resultant, or original name is placed into "geUtilBuf".
 */
geGenFileHome(pi)
char *pi;
{
char *p;
int  Slen;

if (!pi || !(Slen = strlen(pi)))
  {*geUtilBuf = '\0';
   return;
  }

strcpy(geUtilBuf, pi);

p = pi;
/*
 * Locate HOME indicator, if any
 */
while(*p && *p != '~') p++;

if (!*p) return;

if (p == pi) *geUtilBuf = '\0';
else
  strncpy (geUtilBuf, pi, p - pi);

strcat  (geUtilBuf, geUsrLoc);
if (*++p == '/') p++;

strcat  (geUtilBuf, p);

}
