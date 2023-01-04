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
static char SccsId[] = "@(#)gesegreplace.c	1.1\t11/22/88";
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
**	GESEGREPLACE			Replaces 1st Seg with 2nd Seg
**
**  ABSTRACT:
**
**      This routine will KILL the 1st seg and move the 2nd seg into its
**      slot.  Primarily envisioned to be used by POLY and GROUP handlers
**      for SCRUB case; ie, for when grp or pol has only single seg as
**      their content.
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 10/24/88 Created                            
**
**--
**/

#include "geGks.h"

extern geGrp(), gePol();

geSegReplace(Seg1, Seg2)
struct GE_SEG **Seg1, **Seg2;
{                                                                         
struct GE_SEG *PSegP, *NSegP, *SSegP, *ASegP, *TSegP;

if (!*Seg1 || !*Seg2 || *Seg1 == *Seg2) return;
/*
 * Save Prev and Next ptrs from Seg1.
 */
ASegP = *Seg1;
PSegP = ASegP->Prev;
NSegP = ASegP->Next;
/*
 * KILL any children of Seg1 that may exist - BE SURE NOT TO KILL Seg2, should
 * it happen to be a descendant of Seg1.
 */
if ((ASegP->Handler == geGrp || ASegP->Handler == gePol) &&
     (SSegP = (struct GE_SEG *)ASegP->Private))
  {do
     {TSegP = SSegP->Next;
      if (SSegP != *Seg2)
	{GEVEC(GEKILL, SSegP);}
     }while (SSegP = TSegP);
  }
/*
 * Now copy contents of Seg2 into Seg1, then reset Prev and Next pointers
 * just saved.  This will effectively insert a copy of Seg2 into the slot
 * where Seg1 resides.
 */
**Seg1      = **Seg2;
ASegP->Prev = PSegP;
ASegP->Next = NSegP;
/*
 * Check to see whether or not Seg2 is the first
 * segment in a group or polygon.
 * If so, the Private pointer in the group or polygon must be readjusted
 * because this segment will now move out of that slot.
 */
TSegP         = geState.ASegP;
geState.ASegP = *Seg2;
geGrf(GEPGRELINK, geSeg0);
geState.ASegP = TSegP;
/*
 * Collapse the segment chain about Seg2, as it has successfully migrated now.
 */
if ((*Seg2)->Prev) (*Seg2)->Prev->Next = (*Seg2)->Next;
if ((*Seg2)->Next) (*Seg2)->Next->Prev = (*Seg2)->Prev;
geFree(Seg2, sizeof(struct GE_SEG));

}
