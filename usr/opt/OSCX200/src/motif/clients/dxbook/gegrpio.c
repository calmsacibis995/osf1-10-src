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
static char SccsId[] = "@(#)gegrpio.c	1.2\t1/31/90";
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
**	GEGRPIO	             	       Group object IO handler
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
**	GNE 04/25/88 Created
**
**--
**/

#include "geGks.h"


geGrpIO(cmd, ASegP)
long     	cmd;
struct GE_SEG	*ASegP;
{                       
char             p[10];
int              i, c;
struct GE_SEG    *SSegP, *GSegP;

if (!ASegP || !ASegP->Live) return;

if (cmd == GEWRITE)
  {/*
    * See if it's ascii or ddif
    */
    if (geReadWriteType == GE_RW_DDIF)
      {return;
     }
    else
      {/*
        * It's ASCII
        */
	if (!ASegP || !ASegP->Private)                           return;
	if (geVersionOut == GECURRENTVERSION)
	  {/*
	    * Writing out LATEST version
	    */
	   if (geGenSIOTM (cmd, &geCTM))  		            return;
	  fprintf(geFdO, " %d %d",
		   ASegP->Z,
		   ASegP->Visible);
	   if (geGenSIOCol(cmd, &ASegP->Col))                       return;
	   if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT)) return;
	   fprintf(geFdO, " %d\n",
		   ASegP->FillWritingMode);
	   if (geGenSIODesc(cmd, ASegP))			    return;
	   if (geGenSIOAnim(cmd, ASegP))			    return;
	   SSegP = (struct GE_SEG *)ASegP->Private;
	   while (SSegP)
	     {if (SSegP->Live && SSegP->Handler)
		{fputc  ('{' ,geFdO);
		 fputs  (SSegP->Handle ,geFdO);
		 GEVEC(cmd, SSegP);
		 fputc  ('}' ,geFdO);
		 fputc  ('\n' ,geFdO);
	        }
	      SSegP = SSegP->Next;
	     }
	  }
	else                                    /* Version 5                 */
	  {fprintf(geFdO, " %d %d",
		   ASegP->Z,
		   ASegP->Visible);
	   if (geGenSIOCol(cmd, &ASegP->Col))                       return;
	   if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT)) return;
	   fprintf(geFdO, " %d", ASegP->FillWritingMode);
	   SSegP = (struct GE_SEG *)ASegP->Private;
	   while (SSegP)
	     {if (SSegP->Live && SSegP->Handler)
		{fputc  ('{' ,geFdO);
		 fputs  (SSegP->Handle ,geFdO);
		 GEVEC(cmd, SSegP);
		 fputc  ('}' ,geFdO);
		 fputc  ('\n' ,geFdO);
	        }
	      SSegP = SSegP->Next;
	     }
	  }
      }

    return;
   }

/*
 * Must be reading in, see if it's ascii or ddif
 */
if (geReadWriteType == GE_RW_DDIF)
  {
    return;
  }
/*
 * Input is ascii - the first case = latest version
 */
switch (geVersionIn)
  {case 8:
   case 7:
     /*
      * Diff between 7 and prev - this contains the transformation matrix
      * associated with the object.
      */
     if (GSegP = ASegP)
       {if (geGenSIOTM (cmd, &geCTM))  		                return;
	geFscanf2(geFdI, " %d %d",
		  &ASegP->Z,
		  &i);
	ASegP->Visible = i;
	if (geGenSIOCol(cmd, &ASegP->Col))              break;
	if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT))  break;
	geFscanf1(geFdI, " %d",
		  &ASegP->FillWritingMode);
	c = geFgetc(geFdI);                     /* Read past new line        */
	if (geGenSIODesc(cmd, ASegP))			          break;
	if (geGenSIOAnim(cmd, ASegP))				  break;
	
	while (!geFeof(geFdI))
	  {c = geFgetc(geFdI);                  /* Get seg STRT char  - "{"  */
	   if (geFeof(geFdI)) break;
	   if ((char)c != '{')                  /* Either there is a problem */
	     {geUngetc((char)c, geFdI); break;} /* or this group is done     */
	   
	   if (!geFgets(p, 4, geFdI))           /* Get seg descriptor - "SSS"*/
	     break;

	   geSegCr();                           /* Create a new segment slot */
	   SSegP = geState.ASegP;
	   SSegP->Handle[0] = p[0];
	   SSegP->Handle[1] = p[1];
	   SSegP->Handle[2] = p[2];
	   SSegP->Handle[3] = '\0';		/* TERMINATOR		     */
	   geGenHandle(SSegP);                  /* Get Obj handler ptr       */
	   if (SSegP->Handle)
	     GEVEC(cmd, SSegP);                 /* Get all of the priv. data */

	   c = geFgetc(geFdI);                  /* Get seg END  char  - "}"  */
	   if (geFeof(geFdI)) break;
	   c = geFgetc(geFdI);                  /* Get CR		     */

	   if (geFeof(geFdI)) break;
	  }
       }
	
     if (GSegP)                                 /* Poly sub-segs=grp private */
       {if (GSegP->Next) GSegP->Next->Prev = NULL;
	GSegP->Private    = (char *)GSegP->Next;
	GSegP->Next       = NULL;
	geState.ASegP     = GSegP;
	GEVEC(GEBOUNDS, GSegP);
       }
     break;

   case 6:
   case 5:
   case 4:
   case 3:
        /*
         * Diff between 3 and 2:
         *    - 3 is reading and writing the Visible flag
         */
	if (GSegP = ASegP)
	  {geFscanf2(geFdI, " %d %d",
		     &ASegP->Z,
		     &i);
           ASegP->Visible = i;
	   if (geGenSIOCol(cmd, &ASegP->Col))              break;
	   if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT))  break;
	   geFscanf1(geFdI, " %d", &ASegP->FillWritingMode);

	   while (!geFeof(geFdI))
	     {c = geFgetc(geFdI);               /* Get seg STRT char  - "{"  */
	      if (geFeof(geFdI)) break;
	      if ((char)c != '{')               /* Either there is a problem */
		{geUngetc((char)c, geFdI); break;}
						/* or this group is done     */
	      
	      if (!geFgets(p, 4, geFdI))        /* Get seg descriptor - "SSS"*/
		break;
	      c = geFgetc(geFdI);               /* Get parm seperator - " "  */
	      if (geFeof(geFdI) || (char)c != ' ')/* Something is wrong - might*/
		break;                          /*as well just stop gracefuly*/

	      geSegCr();                        /* Create a new segment slot */
	      SSegP = geState.ASegP;
	      SSegP->Handle[0] = p[0];
	      SSegP->Handle[1] = p[1];
	      SSegP->Handle[2] = p[2];
	      SSegP->Handle[3] = '\0';		/* TERMINATOR		     */
	      geGenHandle(SSegP);               /* Get Obj handler ptr       */
	      if (SSegP->Handle)
		GEVEC(cmd, SSegP);              /* Get all of the priv. data */

	      c = geFgetc(geFdI);               /* Get seg END  char  - "}"  */
	      if (geFeof(geFdI)) break;
	      c = geFgetc(geFdI);               /* Get CR		     */

	      if (geFeof(geFdI)) break;
	    }
	  }
	
	if (GSegP)                              /* Poly sub-segs=grp private */
	  {if (GSegP->Next) GSegP->Next->Prev = NULL;
	   GSegP->Private    = (char *)GSegP->Next;
	   GSegP->Next       = NULL;
	   geState.ASegP     = GSegP;
	   GEVEC(GEBOUNDS, GSegP);
	  }
	break;

   case 2:
   case 1:
	if (GSegP = ASegP)
	  {if (geGenSIOCol(cmd, &ASegP->Col))              break;
	   if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT))  break;
	   while (!geFeof(geFdI))
	     {c = geFgetc(geFdI);               /* Get seg STRT char  - "{"  */
	      if (geFeof(geFdI)) break;
	      if ((char)c != '{')               /* Either there is a problem */
		{geUngetc((char)c, geFdI); break;}
						/* or this group is done     */
	      
	      if (!geFgets(p, 4, geFdI))        /* Get seg descriptor - "SSS"*/
		break;
	      c = geFgetc(geFdI);               /* Get parm seperator - " "  */
	      if (geFeof(geFdI) || (char)c != ' ')/* Something is wrong - might*/
		break;                          /*as well just stop gracefuly*/

	      geSegCr();                        /* Create a new segment slot */
	      SSegP = geState.ASegP;
	      SSegP->Handle[0] = p[0];
	      SSegP->Handle[1] = p[1];
	      SSegP->Handle[2] = p[2];
	      SSegP->Handle[3] = '\0';		/* TERMINATOR		     */
	      geGenHandle(SSegP);               /* Get Obj handler ptr       */
	      if (SSegP->Handle)
		GEVEC(cmd, SSegP);              /* Get all of the priv. data */

	      c = geFgetc(geFdI);               /* Get seg END  char  - "}"  */
	      if (geFeof(geFdI)) break;
	      c = geFgetc(geFdI);               /* Get CR		     */

	      if (geFeof(geFdI)) break;
	    }
	  }
	
	if (GSegP)                              /* Poly sub-segs=grp private */
	  {if (GSegP->Next) GSegP->Next->Prev = NULL;
	   GSegP->Private    = (char *)GSegP->Next;
	   GSegP->Next       = NULL;
	   geState.ASegP     = GSegP;
	   GEVEC(GEBOUNDS, GSegP);
	  }
        ASegP->Visible         = TRUE;
        ASegP->FillWritingMode = GXcopy;
	break;

    default:
	break;
   }
/*
 * These did not exist prior to version 7 - the equivalent is that they
 * were OFF.  Though these are not used at this point, set them appropriately
 * to avoid possible problems in the future.
 */
if (geVersionIn < 7)
  {ASegP->WhiteOut = FALSE;
   ASegP->ConLine  = FALSE;
  }

/*
 * The following is for the benefit of MOPS - it does not deal in FillSolid
 */
if (ASegP->FillStyle == FillSolid)
  {ASegP->FillStyle = FillOpaqueStippled;
   ASegP->FillHT    = 100;
 }

}


