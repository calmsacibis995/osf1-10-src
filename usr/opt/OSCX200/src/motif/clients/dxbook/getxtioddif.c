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
static char SccsId[] = "@(#)getxtioddif.c	1.1\t11/22/88";
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
**	GETXTIODDIF	             	       Text object IO handler DDIF format
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
**	GNE 05/31/88 Created
**
**--
**/

#include "geGks.h"

#ifdef GEDDIF

#ifdef VMS
#include <ddif$def.h>
#include <cda$def.h>
#include <cda$msg.h>
#else
#include <ddif_def.h>
#include <cda_def.h>
#include <cda_msg.h>
#endif

extern unsigned long cda$create_aggregate(),
                     cda$insert_aggregate(),
                     cda$store_item();

#endif

extern char          *geMalloc();

geTxtIODDIF(cmd, ASegP)
long     	cmd;
struct GE_SEG	*ASegP;
{                       

#ifdef GEDDIF

/*
 * KLUDGE - If vcc and lk ever get their act straightened out so that this junk
 * can be compiled and linked on ULTRIX, take out this "ifdef VMS" and the
 * completing "endif" at the end of the module.
 */
#ifdef VMS
#define BOMB(x)         (!((x) & 1))

static unsigned long   CdaAggregateItem, CdaBufLen, CdaAggregateIndex, CdaLVal;

int                    i;
struct GE_TXT          *priv;
char		       *error_string;

priv = (struct GE_TXT *)ASegP->Private;

#ifdef GERAGS

if (cmd == GEWRITE)
  {/* geCdaAggType = DDIF$_SEG;
   geCdaTrk         = 100;
   geCdaStat = cda$create_aggregate(
                        &geCdaRtAgg,
                        &geCdaAggType,
                        &geCdaAgg);
   if (BOMB(geCdaStat))
     {error_string = (char *) geFetchLiteral("GE_ERR_DDIFO", MrmRtypeChar8);
      if (error_string != NULL) 
	{sprintf(geErrBuf, error_string, geCdaStat, geCdaTrk);
         geError(geErrBuf, FALSE);
	 XtFree(error_string);
	}
      return(GERRDDIFO);
     }

   CdaAggregateItem   = DDIF$_SEG_CONTENT;
   CdaBufLen          = sizeof(geCdaAgg);
   geCdaTrk         = 101;
   geCdaStat = cda$store_item(
                        &geCdaRtAgg,
                        &geCdaSegAgg,
                        &CdaAggregateItem,
                        &CdaBufLen,
                        &geCdaAgg);
   if (BOMB(geCdaStat))
     {error_string = (char *) geFetchLiteral("GE_ERR_DDIFO", MrmRtypeChar8);
      if (error_string != NULL) 
	{sprintf(geErrBuf, error_string, geCdaStat, geCdaTrk);
         geError(geErrBuf, FALSE);
	 XtFree(error_string);
	}
      return(GERRDDIFO);
     }
  geCdaPrevAgg = geCdaAgg;
*/
   /*
    * Object type Header
    */
   geCdaAggType = DDIF$_TXT;              /* TEXT aggregate            */
   geCdaTrk         = 102;
   geCdaStat = cda$create_aggregate(
                        &geCdaRtAgg,
                        &geCdaAggType,
                        &geCdaAgg);
   if (BOMB(geCdaStat))
     {error_string = (char *) geFetchLiteral("GE_ERR_DDIFO", MrmRtypeChar8);
      if (error_string != NULL) 
	{sprintf(geErrBuf, error_string, geCdaStat, geCdaTrk);
         geError(geErrBuf, FALSE);
	 XtFree(error_string);
	}
      return(GERRDDIFO);
     }

   geCdaTrk         = 103;
   geCdaStat = cda$insert_aggregate(
                        &geCdaAgg,
                        &geCdaPrevAgg);
   if (BOMB(geCdaStat))
     {error_string = (char *) geFetchLiteral("GE_ERR_DDIFO", MrmRtypeChar8);
      if (error_string != NULL) 
	{sprintf(geErrBuf, error_string, geCdaStat, geCdaTrk);
         geError(geErrBuf, FALSE);
	 XtFree(error_string);
	}
      return(GERRDDIFO);
     }
 /*
    * Write the string
    */
   CdaAggregateItem   = DDIF$_TXT_CONTENT;
   CdaBufLen          = strlen(priv->Str);
   geCdaTrk         = 104;
   geCdaStat = cda$store_item(
                        &geCdaRtAgg,
                        &geCdaAgg,
                        &CdaAggregateItem,
                        &CdaBufLen,
                        priv->Str);
   if (BOMB(geCdaStat))
     {error_string = (char *) geFetchLiteral("GE_ERR_DDIFO", MrmRtypeChar8);
      if (error_string != NULL) 
	{sprintf(geErrBuf, error_string, geCdaStat, geCdaTrk);
         geError(geErrBuf, FALSE);
	 XtFree(error_string);
	}
      return(GERRDDIFO);
     }
   geCdaPrevAgg = geCdaAgg;

   return;
  }

#endif
/*
 * Must be reading in
 */
return;

#endif
/*
 * KLUDGE - End of ifdef for VMS - if vcc and lk ever get their act straightened
 * out so that this junk can be compiled and linked on ULTRIX, take this out.
 */

#endif
}
