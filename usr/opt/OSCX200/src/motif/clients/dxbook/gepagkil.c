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
static char SccsId[] = "@(#)gepagkil.c	1.2\t5/25/89";
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
** 	GEPAGKIL			Irrevocably destroys given page
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
** 	GNE 10/05/87 Created                            
** 	DAA 11/08/89 Revised                            
**
**--                
**/
#include "geGks.h"

gePagKil(PagP)
struct GE_PAG *PagP;
{                                                                         
short          ExitSav;
struct GE_PAG  *Pag0, *APagP;

APagP = NULL;

if (PagP)
   {if (PagP->Pixmap)
      {XFreePixmap(geDispDev, PagP->Pixmap);
       if (geState.Pixmap == PagP->Pixmap) geState.Pixmap = NULL;
       PagP->Pixmap = NULL;
      }

    PagP->Surf.self  = NULL;
    geSeg0           = PagP->Seg0;
    ExitSav          = geState.Exit;            /* Setting Exit = TRUE to    */
    geState.Exit     = TRUE;                    /* tell geSegKil to not waste*/
    geGrf(GEKILL, geSeg0);                      /* time tracking down parent */
    geState.Exit     = ExitSav;                 /* of Seg being killed.      */

    APagP = PagP->Next ? PagP->Next:PagP->Prev; /* Will use reset State.APagP*/
    /*
     * Adjust page chain
     */
    if (PagP->Prev) PagP->Prev->Next = PagP->Next;
    if (PagP->Next) PagP->Next->Prev = PagP->Prev;
    if (PagP == gePag0)
	{Pag0 = PagP->Next;
   	 geFree(&PagP, sizeof(struct GE_PAG));
	 gePag0 = Pag0;
	 if (!gePag0) geState.Exit = TRUE;	/* This may be overriden, but*/
	}					/*def. must = "EXIT - NOW!!!"*/
    else
   	geFree(&PagP, sizeof(struct GE_PAG));
   }

#ifdef GERAGS
geGenAPag(APagP);				/* Setup a new active page   */
#endif

}
