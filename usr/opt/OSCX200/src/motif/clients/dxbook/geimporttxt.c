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
static char SccsId[] = "@(#)geimporttxt.c	1.1\t11/22/88";
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
**	GEIMPORTTXT			Loads text from an external ascii file
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
**	GNE 11/15/88 Created                            
**
**--
**/

#include "geGks.h"
#include <sys/stat.h>

extern char           	*geMalloc();
extern  struct GE_SEG 	*geGenSegL();
extern			 geGrp();
extern XFontStruct 	*geLoadFont();

geImportTxt(FileName, cmd)
char          	*FileName;
long		cmd;
{                          
unsigned char      Import[GE_MAX_MSG_CHAR], *pi1, *pi2, *po;
short              InMemSav, Empty;
int                i, ix, n, retval;
long               dy, MouseXSav, MouseYSav;
struct stat        StatPtr;
struct GE_SEG	   *TSegP, *FSegP, *LSegP, *GSegP, *GSegPSav, *NSegP;
struct GE_CO	   Co;
int		   width, height;	
char 	       	    *error_string;

if (!FileName) return(1);

GEVEC(GEINQLIVE, geSeg0);
if (!geState.LiveSegs) Empty = TRUE;
else		       Empty = FALSE;

retval = 0;

LSegP = geGenSegL();
/*
 * Open the file and buffer it in
 */
if (!(geFdI = fopen(FileName, "r")))
  {error_string = (char *) geFetchLiteral("GE_ERR_XOPEN", MrmRtypeChar8);
   if (error_string != NULL) 
     {sprintf(geErrBuf, error_string, FileName);
      geError(geErrBuf, FALSE);
      XtFree(error_string);
     }
   return(GERRXOPEN);
  }

if (stat(FileName, &StatPtr) == -1)
  {error_string = (char *) geFetchLiteral("GE_ERR_XOPEN", MrmRtypeChar8);
   if (error_string != NULL) 
     {sprintf(geErrBuf, error_string, FileName);
      geError(geErrBuf, FALSE);
      XtFree(error_string);
     }
   return(GERRXOPEN);
  }

#ifdef GERAGS
geMnStat(GEDISP, 29);			/* Tell them we're reading it */
#endif
/*
InMemSav         = geMemIO.InMem;
geMemIO.RagsOwn  = TRUE;
geMemIO.InMem    = TRUE;
geMemIO.PtrC     = geMemIO.PtrS;
geMemIO.NumBytes = StatPtr.st_size;
geMemIO.PtrE     = geMemIO.PtrS + geMemIO.NumBytes;
n                = fread(geMemIO.PtrS, 1, geMemIO.NumBytes, geFdI);
*/
/*
geMaskEvent(GE_MBd, &geState.Event);
if (geState.Event.xbutton.button == GEBUT_R)
   goto import_done;
*/

if (geState.Event.xbutton.window != geState.APagP->Surf.self)
  {retval = GERRWRONGWINDOW;
   error_string = (char *) geFetchLiteral("GE_ERR_WRONGWINDOW", MrmRtypeChar8);
   if (error_string != NULL) 
     {geError(error_string, FALSE);
      XtFree(error_string);
     }
   goto import_done;
  }
/*  
geState.Mouse.x = GETSUI(geState.Event.xbutton.x);
geState.Mouse.y = GETSUI(geState.Event.xbutton.y);
*/
if (!geFontUser[geAttr.Font])
    geFontUser[geAttr.Font] = geLoadFont(GEXFIXED_NAME);
ix = GETPIX(geState.Mouse.x);
dy = GETSUI(geFontUser[geAttr.Font]->ascent + geFontUser[geAttr.Font]->descent);

#ifdef GERAGS
geSetWaitCurs(TRUE);
#endif

XFlush(geDispDev);
while (!geFeof(geFdI))                          /* Keep going till hit EOF   */
  {geFgets(geUtilBuf, GE_MAX_MSG_CHAR, geFdI);
   /*
    * Filter CR/LF - just move down a line and continue
    */
   if (geUtilBuf[0] == 012 || geUtilBuf[0] == 015)
     {geState.Mouse.y += dy;
      continue;
     }
   geUtilBuf[strlen(geUtilBuf) - 1] = '\0';     /* Strip off trailing CR/LF  */
   /*
    * Replace tabs with the appropriate number of spaces
    */
   pi1 = pi2 = (unsigned char *)geUtilBuf;
   po  = Import;
   while (*pi2)
     {if (*pi2 == 011)                          /* Got a tab (horizontal)    */
	{if ((n  = pi2 - pi1))                  /* Copy chars between last   */
	   {strncpy(po, pi1, n);                /* and this tab              */
	    po += n;                            /* Update output pointer     */
	   }
	 for (i = geState.Tab; i > 0; i--)      /* Copy # of spaces = a tab  */
	   strcpy(po++, " ");
	 pi1 = pi2 = pi2 + 1;                   /* Update input pointers     */
        }
      else
	pi2++;
     }
   
   if (po != Import)                            /* 1 or more tabs have been  */
     {if (*pi1) strcpy(po, pi1);                /* have been expanded, so    */
      strcpy(geUtilBuf, Import);                /* copy back text with expan-*/
     }                                          /* tabs                      */

   geTxt(GECREATEIMPORT, NULL);                 /* Create a new TXT seg      */
   geState.Mouse.y += dy;
  }

#ifdef GERAGS
geSetWaitCurs(FALSE);
#endif

GEVEC(GEBOUNDS, geSeg0);
/*
 * Position crop box around drawing and prompt user to place it.
 */
  if (Empty)
    {Co.x1 = geSeg0->Co.x1;
     Co.y1 = geSeg0->Co.y1;
     width  = GETPIX(geSeg0->Co.x2 - geSeg0->Co.x1);
     height = GETPIX(geSeg0->Co.y2 - geSeg0->Co.y1);
    }
  else 	
    {TSegP = LSegP->Next;		/* get first text segment */
     if (TSegP)
       {Co.x1 = Co.y1 =  GEMAXLONG;
        Co.x2 = Co.y2 = -GEMAXLONG;
        while (TSegP)	
          {if (TSegP->Live)
             {GEVEC(GEBOUNDS, TSegP);
	      Co.x1 = min(Co.x1, min(TSegP->Co.x1, TSegP->Co.x2));
	      Co.y1 = min(Co.y1, min(TSegP->Co.y1, TSegP->Co.y2));
	      Co.x2 = max(Co.x2, max(TSegP->Co.x1, TSegP->Co.x2));
	      Co.y2 = max(Co.y2, max(TSegP->Co.y1, TSegP->Co.y2));
	     }
	   TSegP = TSegP->Next;
          }
       }
     width  = GETPIX(Co.x2 - Co.x1);
     height = GETPIX(Co.y2 - Co.y1);
    }

FSegP      = LSegP->Next;

#ifdef GERAGS
geMnStat(GEDISP, 38);
if (cmd != GECREATESTAT)
   {geMouseXY(geState.APagP->Surf.self);

    MouseXSav  = Co.x1;
    MouseYSav  = Co.y1;
    geGenBx.x1 = GETPIX(geState.Mouse.x);
    geGenBx.y1 = GETPIX(geState.Mouse.y);
    geGenBx.x2 = geGenBx.x1 + width;
    geGenBx.y2 = geGenBx.y1 + height;
    if (LSegP) LSegP->Next = NULL;
    geGenBoxMove(geState.APagP->Surf.self, FALSE);
    if (LSegP) LSegP->Next = FSegP;
    geState.Dx = geState.Mouse.x - MouseXSav;
    geState.Dy = geState.Mouse.y - MouseYSav;
    geGrf(GEMOVE,   LSegP);
   }
else
   {if (LSegP) LSegP->Next = FSegP;
    geState.Dx = 0;
    geState.Dy = GETSUI(geFontUser[geAttr.Font]->ascent);
    geGrf(GEMOVE,   LSegP);
   }

#else
if (LSegP) LSegP->Next = FSegP;
geState.Dx = 0;
geState.Dy = GETSUI(geFontUser[geAttr.Font]->ascent);
geGrf(GEMOVE,   LSegP);
#endif

/*
 * Group the incoming elements?
 */
if (geState.GroupOnImport) 
    {if (Empty)
	{/*
          * Can just group the GRF.
	  * There must be at least 2 LIVE and VISIBLE segments
          */
         GEVEC(GEINQLIVE, geSeg0);
         if (geState.LiveSegs >= 2)
	    {GEVEC(GEINQVISIBLE, geSeg0);
             if (geState.VisibleSegs >= 2)
               geGrp(GEJOINALL, NULL);
 	    }
    	}
     else
	{GSegPSav = geState.GSegP;

	 geSegCr();
	 geState.ASegP->FillStyle  = GETRANSPARENT;
	 geState.ASegP->Handle[0]  = 'G';
	 geState.ASegP->Handle[1]  = 'R';
	 geState.ASegP->Handle[2]  = 'P';
	 geState.ASegP->Handle[3]  = '\0';
	 geState.ASegP->Handler    = geGrp;
	 geState.GSegP = GSegP     = geState.ASegP;
	 /*
	  * Traverse all the new text segments and JOIN them to the
	  * group.
	  */
    	 TSegP = FSegP;				/* get first text segment     */
     	 if (TSegP)
       	    {while (TSegP && TSegP != GSegP)	
	   	{NSegP = TSegP->Next;
	 	 geGrp(GEJOIN, TSegP);
	   	 TSegP = NSegP;
          	}
	     /*
	      * Finalize the group
	      */
	     GSegP->Next   = NULL;             	/* Grp segs, priv to grp     */
	     GEVEC(GEBOUNDS, GSegP);           	/* Establish group bounds    */
	     geState.ASegP = GSegP;
       	    }
	 else					
	    {geSegKil(&geState.GSegP);	 	/* Some real serious problem */
	     geState.ASegP = NULL;
	    }
	 geState.GSegP = GSegPSav;
	}
    }
geGrf(GEBOUNDS, geSeg0);

#ifdef GERAGS
if (Empty) geGrf(GEDISP, geSeg0);             	/* Display whole grf         */
else       geGrf(GEDISP, LSegP);        	/* Display NEW segments      */
#endif

import_done:
geFclose(geFdI);
/*
geMemIO.InMem = InMemSav;
*/
return(retval);
}
                                                                 
