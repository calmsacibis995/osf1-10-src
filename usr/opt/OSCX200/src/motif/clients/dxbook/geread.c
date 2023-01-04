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
static char SccsId[] = "@(#)geread.c	1.6\t9/21/89";
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
**	GEREAD			Loads segments from the input meta file
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
**	GNE 02/11/86 Created                            
**
**--
**/

#include "geGks.h"
#ifdef __osf__
#include <sys/stat.h>
#else
#include "stat.h"
#endif

#define GEISDELIM(mp_x) ((mp_x == 040 || (mp_x >= 011 && mp_x <= 015)) ? 1 : 0)

extern char          *geMalloc();
extern struct GE_SEG *geGenSegL();

geRead(ASegP)
struct GE_SEG *ASegP;
{                          
char 		p[10], *f, *error_string;
int  		c, NumBytes, ColStatSav;
struct GE_SEG 	*NSegP;
struct stat   	StatPtr;
long 		readstate, ErrReport;

if (!ASegP || (!geMemIO.InMem && !geInFile)) return(1);

readstate       = geState.State;		/* Determine OPEN or INCLUDE */
geState.State   = GEREAD;			/* before setting to READ    */

if (geReadWriteType == GE_RW_DDIF)
  {GEVEC(GEREAD,  ASegP);
   GEVEC(GEBOUNDS, ASegP);                      /* Get obj bounds            */
   geState.State = 0;
   geRun.ErrReport = ErrReport;
   return(0);
  }
/*
 * If don't yet have the data in memory somewhere, then go get from disk now.
 * Note: otherwise it is expect that the following have been set up prior to
 * reaching this point:
 *               geMemIO.PtrS
 *               geMemIO.PtrC
 *               geMemIO.PtrE
 *               geMemIO.NumBytes
 */
if (!geMemIO.PtrS)
  {geGenFileDir(geInFile, geDefDirIn);
   geInFile = geUtilBuf;
   if (!(geFdI = fopen(geInFile, "r")))
     {geState.State = 0;
      return(GERRXOPEN);
     }

   if (geMemIO.InMem)                           /* Want to perform InMem IO */
     {if (stat(geInFile, &StatPtr) == -1 || 
	  !(geMemIO.PtrS = geMalloc(StatPtr.st_size)))
	{geState.State = 0;
	 return(GERRXOPEN);
	}

      geMemIO.RagsOwn  = TRUE;		/* RAGS frees buff later on   */
      geMemIO.PtrC     = geMemIO.PtrS;
      geMemIO.NumBytes = StatPtr.st_size;
      geMemIO.PtrE     = geMemIO.PtrS + geMemIO.NumBytes;
      NumBytes         = fread(geMemIO.PtrS, 1, geMemIO.NumBytes, geFdI);
     }
  }
else
  geFdI = NULL;

NSegP = geGenSegL();				/* "Father" of 1st NEW seg   */
/*
 * Locate start of RAGS metafile = "<"
 */
while (!geFeof(geFdI))
   {c = geFgetc(geFdI);                  	/* Get a char		     */
    if (geFeof(geFdI)) break;			/* Encountered end of file   */
    if ((char)c == '<')                  	/* Found it		     */
	{geUngetc((char)c, geFdI); break;}
   }
	   

if (geFeof(geFdI) || !geFgets(p, 7, geFdI))     /* Get Version     - "<VXX-X"*/
  {geFclose(geFdI);                             /* Some problem              */
   geState.State = 0;
   return(GERRNOTMETA);
  }
/*
 * Does it seem like a metafile at all?
 */
if (strncmp(p, "<V01-", 5))
  {geFclose(geFdI);                             /* Some problem              */
   geState.State = 0;
   return(GERRNOTMETA);
  }

/*
 * Determine version
 */
if (!(strcmp(p, "<V01-8")))	geVersionIn = 8;
else
if (!(strcmp(p, "<V01-7")))	geVersionIn = 7;
else
if (!(strcmp(p, "<V01-6")))	geVersionIn = 6;
else
if (!(strcmp(p, "<V01-5")))	geVersionIn = 5;
else
if (!(strcmp(p, "<V01-4")))	geVersionIn = 4;
else
if (!(strcmp(p, "<V01-3")))	geVersionIn = 3;
else
if (!(strcmp(p, "<V01-2")))	geVersionIn = 2;
else
if (!(strcmp(p, "<V01-1")))	geVersionIn = 1;
else						/* No legitimate version     */
  {geFclose(geFdI);                             /*found ...............      */
   geState.State   = 0;
   return(GERRVERSION);				/*(with msg) & continue on   */
  }                        

ColStatSav = geColStat;
geColStat |= GECOL_CMAP_INHIBIT_FREE_COND;

while (!geFeof(geFdI))                          /* Keep going till hit EOF   */
  {c = geFgetc(geFdI);                          /* Get seg STRT char  - "["  */
   if (geFeof(geFdI) || (char)c != '[') goto read_eof;

   if (!geFgets (p, 4, geFdI))                  /* Get seg descriptor - "SSS"*/
     {geFclose(geFdI);                          /* Some problem              */
      geState.State = 0;
      return(GERRBADMETA);
     }

   ASegP->Handle[0] = p[0];                     /* Copy                      */
   ASegP->Handle[1] = p[1];                     /*over its                   */
   ASegP->Handle[2] = p[2];                     /*name                       */
   ASegP->Handle[3] = '\0';			/* TERMINATOR		     */
   geGenHandle(ASegP);                          /* Get Obj handler ptr       */
   if (ASegP->Handle)
     {GEVEC(GEREAD, ASegP);                     /* Get all of the priv. data */
      GEVEC(GEBOUNDS, ASegP);                   /* Get obj bounds            */
     }

   c = geFgetc(geFdI);                          /* Get seg END  char  - "]"  */
   if (geFeof(geFdI)) goto read_eof;

  }

read_eof:

#ifdef GERAGS
if (geColStat & GECOL_CMAP_NOALLOC)
  {geFclose(geFdI);                             /* Some problem              */
   geState.State = 0;
   return (GERROUTOFCOLS);
  }
#endif

geColStat = ColStatSav;

geFclose(geFdI);
/*
 * KILL all EMPTY text segments
 */
GEVEC(GESCRUB, geSeg0);
/*
 * Flag empty groups and polygons as non-live
 */
GEVEC(GEGPDELEMPTY, geSeg0);
geState.State = 0;
return(0);
}
                                                                 
