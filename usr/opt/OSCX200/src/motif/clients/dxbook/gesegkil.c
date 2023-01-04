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
static char SccsId[] = "@(#)gesegkil.c	1.3\t4/25/89";
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
**	GESEGKIL			Irrevocably destroys given segment
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
**	GNE 03/05/87 Created                            
**
**--
**/

#include "geGks.h"

geSegKil(SegP)
struct GE_SEG **SegP;
{                                                                         
struct GE_XDO	*XDoP;
struct GE_SEG 	*ASegP;

if (*SegP)
  {if (!geState.Exit)
     {/*
       * Release any private resources that may be applicable.
       */
      GEVEC(GERELPRIV, (*SegP));
      GEVEC(GEKILPRIV, (*SegP));
      /*
       * Release description buffer, if it exists
       */
      if ((*SegP)->Descrip) geFree(&((*SegP)->Descrip), 0);
      /*
       * Check to see whether or not the segment to be killed is the first
       * segment in a group or polygon.
       * If so, the Private pointer in the group or polygon must be readjusted
       * because this segment will now move out of that slot.
       */
       ASegP         = geState.ASegP;
       geState.ASegP = *SegP;
       geGrf(GEPGRELINK, geSeg0);
       geState.ASegP = ASegP;
       ASegP	     = *SegP;

#ifdef GERAGS
       /*
	* Search Undo chain and if segment is there, remove it
	*/
	if (geState.APagP && geState.APagP->XDoP)
	   {XDoP = geState.APagP->XDoP;
	    do
		{if (ASegP == XDoP->Seg)
		   {if (XDoP->Prev) XDoP->Prev->Next = XDoP->Next;
		    if (XDoP->Next) XDoP->Next->Prev = XDoP->Prev;
	    	    if (XDoP == geState.APagP->XDoP)
	    	    	geState.APagP->XDoP    = geState.APagP->XDoP->Prev;
	    	    geFree(&XDoP, sizeof(struct GE_XDO));
		    break;
		   }
		 XDoP = XDoP->Prev;
		}while (XDoP);
	   }
#endif

      }

    /*
     * Adjust segment chain
     */
    if ((*SegP)->Prev) (*SegP)->Prev->Next = (*SegP)->Next;
    if ((*SegP)->Next) (*SegP)->Next->Prev = (*SegP)->Prev;
    if ((*SegP)->AnimCtrl.Frames) geFree(&(*SegP)->AnimCtrl.Frames, 0);
    geFree(SegP, sizeof(struct GE_SEG));
   }

geColStat = 0;
}

/*
 * This routine is for destroying simple segments which are:
 *	1) NOT a group
 *	2) NOT the first element of a group
 *	3) NOT in the UNDELETE chain
 */
geSegKilQuick(SegP)
struct GE_SEG **SegP;
{                                                                         

if (*SegP)
  {/*
    * Release any private resources that may be applicable.
    */
    GEVEC(GERELPRIV, (*SegP));
    GEVEC(GEKILPRIV, (*SegP));

    /*
     * Adjust segment chain
     */
    if ((*SegP)->Prev) (*SegP)->Prev->Next = (*SegP)->Next;
    if ((*SegP)->Next) (*SegP)->Next->Prev = (*SegP)->Prev;
    if ((*SegP)->AnimCtrl.Frames) geFree(&(*SegP)->AnimCtrl.Frames, 0);
    geFree(SegP, sizeof(struct GE_SEG));
   }

geColStat = 0;
}
