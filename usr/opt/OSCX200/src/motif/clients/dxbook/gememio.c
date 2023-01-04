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
static char SccsId[] = "@(#)gememio.c	1.1\t11/22/88";
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
**	GEMEMIO.C         	  	Mem/file meta I/O routines
**
**  ABSTRACT:
**
**      This module contains several functions which are layer routines
**      for the c run-time file IO functions used to read the RAGS meta file.
**      The layer routines are sensitive to whether the I/O is to be conducted
**      straight from the file, or whetther the data is to be broken out of 
**      a buffer in memory.
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 09/06/88 Created
**
**--
**/

#include "geGks.h"
#include <ctype.h>
#include <varargs.h>
#include <stdlib.h>

#define GEGETDELIM \
while (*geMemIO.PtrC == ' ') geMemIO.PtrC++; \
while(!isspace(*geMemIO.PtrC) && *geMemIO.PtrC != '{' && \
      *geMemIO.PtrC != '}' && *geMemIO.PtrC != '[' && \
      *geMemIO.PtrC != ']') \
   geMemIO.PtrC++;

#define GEIGNORESPACE \
while (geMemIO.PtrC < geMemIO.PtrE && *geMemIO.PtrC == ' ') geMemIO.PtrC++;

/*
 * fgets
 */
char *
geFgets(p, len, f)
char *p;
int  len;
FILE *f;
{
char *ptr;
int  i;

if (geMemIO.InMem)
  {if (len == GE_MAX_MSG_CHAR)                  /* This indicates that want  */
     {ptr = p;                                  /* get as many chars as poss-*/
      i   = 0;                                  /* ible, up to a LF or       */
      while (i++ <= len)                        /* CR or NULL                */
	{*ptr = *geMemIO.PtrC++;
	 if (*ptr == 012 || *ptr == 015 || *ptr == '\0')
	   {*++ptr = '\0';
	    return(geMemIO.PtrC);
	   }
	 ptr++;
        }
     }
   else
     {len--;
      strncpy(p, geMemIO.PtrC, len);
      *(p + len) = '\0';
      geMemIO.PtrC += len;
      if (geMemIO.PtrC >= geMemIO.PtrE) return(0);
      else                              return(geMemIO.PtrC);
     }
  }
else
  return(fgets(p, len, f));
}
/*
 * fclose
 */
geFclose(f)
FILE *f;
{

if (geMemIO.InMem)
  {if (geMemIO.RagsOwn)
     {geFree(&geMemIO.PtrS, geMemIO.NumBytes);
      geMemIO.PtrS     = geMemIO.PtrC = geMemIO.PtrE = NULL;
      geMemIO.NumBytes = 0;
     }
  }
if (f) return(fclose(f));
}
/*
 * feof
 */
geFeof(f)
FILE *f;
{
if (geMemIO.InMem)
  {if (geMemIO.PtrC >= geMemIO.PtrE || *geMemIO.PtrC == '\0') return(TRUE);
   else                              			      return(FALSE);
  }
else
  return(feof(f));
}
/*
 * fgetc
 */
geFgetc(f)
FILE *f;
{
if (geMemIO.InMem)
   return(*geMemIO.PtrC++);
else
  return(fgetc(f));
}
/*
 * ungetc
 */
geUngetc(c, f)
char  c;
FILE *f;
{
if (geMemIO.InMem) geMemIO.PtrC--;
else               ungetc(c, f);
}
/*
 * Straight fscanf analogue
 */
geFscanf(f, str, iarg)
FILE *f;
char *str, *iarg;
{
char *op;

if (geMemIO.InMem)
  {GEIGNORESPACE
   op = iarg;
   while (geMemIO.PtrC < geMemIO.PtrE && !isspace(*geMemIO.PtrC))
	  {*op++ = *geMemIO.PtrC++;}
   *op = (char)NULL;
  }
else
  fscanf(f, str, iarg);
}
/*
 * fscanf (1)
 */
geFscanf1(f, str, iarg)
FILE *f;
char *str;
int  *iarg;
{
if (geMemIO.InMem)
  {*iarg  = atoi(geMemIO.PtrC);
   GEGETDELIM
  }
else
  fscanf(geFdI, str, iarg);
}
/*
 * fscanf (2)
 */
geFscanf2(f, str, iarg, iarg2)
FILE *f;
char *str;
int  *iarg, *iarg2;
{
if (geMemIO.InMem)
  {*iarg  = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg2 = atoi(geMemIO.PtrC);
   GEGETDELIM
  }
else
  fscanf(geFdI, str, iarg, iarg2);
}
/*
 * fscanf (3)
 */
geFscanf3(f, str, iarg, iarg2, iarg3)
FILE *f;
char *str;
int  *iarg, *iarg2, *iarg3;
{
if (geMemIO.InMem)
  {*iarg  = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg2 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg3 = atoi(geMemIO.PtrC);
   GEGETDELIM
  }
else
  fscanf(geFdI, str, iarg, iarg2, iarg3);
}
/*
 * fscanf (4)
 */
geFscanf4(f, str, iarg, iarg2, iarg3, iarg4)
FILE *f;
char *str;
int  *iarg, *iarg2, *iarg3, *iarg4;
{
if (geMemIO.InMem)
  {*iarg  = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg2 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg3 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg4 = atoi(geMemIO.PtrC);
   GEGETDELIM
  }
else
  fscanf(geFdI, str, iarg, iarg2, iarg3, iarg4);
}
/*
 * fscanf (5)
 */
geFscanf5(f, str, iarg, iarg2, iarg3, iarg4, iarg5)
FILE *f;
char *str;
int  *iarg, *iarg2, *iarg3, *iarg4, *iarg5;
{
if (geMemIO.InMem)
  {*iarg  = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg2 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg3 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg4 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg5 = atoi(geMemIO.PtrC);
   GEGETDELIM
  }
else
  fscanf(geFdI, str, iarg, iarg2, iarg3, iarg4, iarg5);
}

/*
 * fscanf (6)
 */
geFscanf6(f, str, iarg, iarg2, iarg3, iarg4, iarg5, iarg6)
FILE *f;
char *str;
int  *iarg, *iarg2, *iarg3, *iarg4, *iarg5, *iarg6;
{
if (geMemIO.InMem)
  {*iarg  = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg2 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg3 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg4 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg5 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg6 = atoi(geMemIO.PtrC);
   GEGETDELIM
  }
else
  fscanf(geFdI, str, iarg, iarg2, iarg3, iarg4, iarg5, iarg6);
}

/*
 * fscanf (7)
 */
geFscanf7(f, str, iarg, iarg2, iarg3, iarg4, iarg5, iarg6, iarg7)
FILE *f;
char *str;
int  *iarg, *iarg2, *iarg3, *iarg4, *iarg5, *iarg6, *iarg7;
{
if (geMemIO.InMem)
  {*iarg  = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg2 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg3 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg4 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg5 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg6 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg7 = atoi(geMemIO.PtrC);
   GEGETDELIM
  }
else
  fscanf(geFdI, str, iarg, iarg2, iarg3, iarg4, iarg5, iarg6, iarg7);
}
/*
 * fscanf (8)
 */
geFscanf8(f, str, iarg, iarg2, iarg3, iarg4, iarg5, iarg6, iarg7, iarg8)
FILE *f;
char *str;
int  *iarg, *iarg2, *iarg3, *iarg4, *iarg5, *iarg6, *iarg7, *iarg8;
{
if (geMemIO.InMem)
  {*iarg  = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg2 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg3 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg4 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg5 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg6 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg7 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg8 = atoi(geMemIO.PtrC);
   GEGETDELIM
  }
else
  fscanf(geFdI, str, iarg, iarg2, iarg3, iarg4, iarg5, iarg6, iarg7, iarg8);
}
/*
 * fscanf (9)
 */
geFscanf9(f, str, iarg, iarg2, iarg3, iarg4, iarg5, iarg6, iarg7, iarg8, iarg9)
FILE *f;
char *str;
int  *iarg, *iarg2, *iarg3, *iarg4, *iarg5, *iarg6, *iarg7, *iarg8, *iarg9;
{
if (geMemIO.InMem)
  {*iarg  = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg2 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg3 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg4 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg5 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg6 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg7 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg8 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg9 = atoi(geMemIO.PtrC);
   GEGETDELIM
  }
else
  fscanf(geFdI, str, iarg, iarg2, iarg3, iarg4, iarg5, iarg6, iarg7, iarg8,
	 iarg9);
}
/*
 * fscanf (10)
 */
geFscanf10(f, str, iarg, iarg2, iarg3, iarg4, iarg5, iarg6, iarg7, iarg8, iarg9,
	   iarg10)
FILE *f;
char *str;
int  *iarg, *iarg2, *iarg3, *iarg4, *iarg5, *iarg6, *iarg7, *iarg8, *iarg9,
     *iarg10;
{
if (geMemIO.InMem)
  {*iarg  = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg2 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg3 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg4 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg5 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg6 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg7 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg8 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg9 = atoi(geMemIO.PtrC);
   GEGETDELIM
   *iarg10 = atoi(geMemIO.PtrC);
   GEGETDELIM
  }
else
  fscanf(geFdI, str, iarg, iarg2, iarg3, iarg4, iarg5, iarg6, iarg7, iarg8,
	 iarg9, iarg10);
}

/*
 * fscanff (3)
 */
geFscanff3(f, str, farg, farg2, farg3)
FILE *f;
char *str;
float  *farg, *farg2, *farg3;
{
if (geMemIO.InMem)
  {*farg  = (float) atof(geMemIO.PtrC);
   GEGETDELIM
   *farg2 = (float) atof(geMemIO.PtrC);
   GEGETDELIM
   *farg3 = (float) atof(geMemIO.PtrC);
   GEGETDELIM
  }
else
  fscanf(geFdI, str, farg, farg2, farg3);
}
/*
 * fscanff (6)
 */
geFscanff6(f, str, farg, farg2, farg3, farg4, farg5, farg6)
FILE *f;
char *str;
float  *farg, *farg2, *farg3, *farg4, *farg5, *farg6;
{
if (geMemIO.InMem)
  {*farg  = (float) atof(geMemIO.PtrC);
   GEGETDELIM
   *farg2 = (float) atof(geMemIO.PtrC);
   GEGETDELIM
   *farg3 = (float) atof(geMemIO.PtrC);
   GEGETDELIM
   *farg4 = (float) atof(geMemIO.PtrC);
   GEGETDELIM
   *farg5 = (float) atof(geMemIO.PtrC);
   GEGETDELIM
   *farg6 = (float) atof(geMemIO.PtrC);
   GEGETDELIM
  }
else
  fscanf(geFdI, str, farg, farg2, farg3, farg4, farg5, farg6);
}


/*
 * fscanff (8)
 */
geFscanff8(f, str, farg, farg2, farg3, farg4, farg5, farg6, farg7, farg8)
FILE *f;
char *str;
float  *farg, *farg2, *farg3, *farg4, *farg5, *farg6, *farg7, *farg8;
{
if (geMemIO.InMem)
  {*farg  = (float) atof(geMemIO.PtrC);
   GEGETDELIM
   *farg2 = (float) atof(geMemIO.PtrC);
   GEGETDELIM
   *farg3 = (float) atof(geMemIO.PtrC);
   GEGETDELIM
   *farg4 = (float) atof(geMemIO.PtrC);
   GEGETDELIM
   *farg5 = (float) atof(geMemIO.PtrC);
   GEGETDELIM
   *farg6 = (float) atof(geMemIO.PtrC);
   GEGETDELIM
   *farg7 = (float) atof(geMemIO.PtrC);
   GEGETDELIM
   *farg8 = (float) atof(geMemIO.PtrC);
   GEGETDELIM
  }
else
  fscanf(geFdI, str, farg, farg2, farg3, farg4, farg5, farg6, farg7, farg8);
}
