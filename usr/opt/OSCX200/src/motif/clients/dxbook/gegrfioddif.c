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
static char SccsId[] = "@(#)gegrfioddif.c	1.1\t11/22/88";
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
**	GEGRFIODDIF	             	       Graph object IO handler DDIF format
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
**	GNE 05/25/88 Created
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

#ifdef VMS
extern unsigned long cda$create_root_aggregate(),
                     cda$create_file(),
                     cda$open_file(),
                     cda$create_aggregate(),
                     cda$insert_aggregate(),
                     cda$store_item(),
		     cda$put_document(),
		     cda$get_document(),
		     cda$delete_root_aggregate(),
		     cda$locate_item(),
		     cda$close_file();
#endif

#endif

geGrfIODDIF(cmd, ASegP)
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

static char             RagsName[] = "RAGS",
                        Language[] = "English",
                        TestTxt [] = "It took 25 calls (up to now) to put out this single line of text";
static unsigned char    CdaResultFileSpec[255], CdaResultBuffer[255];
static unsigned long    CdaBufLen, CdaLVal, CdaAggregateItem, CdaAggregateIndex,
                        CdaAddInfo, CdaResultLen;
struct GE_SEG        	*SSegP;
static struct item_list CdaProcessingOptions = {0, 0, 0};
char  			*error_string;


if (!(SSegP = ASegP)) return;

#ifdef GERAGS

if (cmd == GEWRITE)
  {if (geCdaCrRtAgg())                                                   return;
   if (geCdaCrFile (geInFile))                                           return;
   /*
    * Header trash - Major, Minor version #'s
    */
   if (geCdaCrAgg (DDIF$_DSC))                                           return;
geCdaTrk = 5;
   if (geCdaStoreItem(geCdaRtAgg, DDIF$_DDF_DESCRIPTOR,
		      sizeof(geCdaAgg), &geCdaAgg, 0, 0))
                                                                         return;
   CdaLVal            =  DDIF$K_MAJOR_VERSION;
geCdaTrk = 6;
   if (geCdaStoreItem(geCdaAgg, DDIF$_DSC_MAJOR_VERSION,
		      sizeof(CdaLVal), &CdaLVal, 0, 0))                  return;
   CdaLVal            =  DDIF$K_MINOR_VERSION;
geCdaTrk = 7;
   if (geCdaStoreItem(geCdaAgg, DDIF$_DSC_MINOR_VERSION,
		      sizeof(CdaLVal), &CdaLVal, 0, 0))                  return;
geCdaTrk = 8;
   if (geCdaStoreItem(geCdaAgg, DDIF$_DSC_PRODUCT_IDENTIFIER,
		      strlen(RagsName), RagsName, 0, 0))                 return;
   sprintf(geUtilBuf, "<V01-%d>", geVersionOut);
geCdaTrk = 9;
   if (geCdaStoreItem(geCdaAgg, DDIF$_DSC_PRODUCT_NAME,
		      strlen(geUtilBuf), geUtilBuf, 0, CDA$K_ISO_LATIN1))
                                                                         return;
   /*
    * Define the languages that are going to be used
    */
   if (geCdaCrAgg (DDIF$_DHD))                                           return;
   CdaLVal            =  DDIF$K_ISO_639_LANGUAGE;
geCdaTrk = 10;
   if (geCdaStoreItem(geCdaAgg, DDIF$_DHD_LANGUAGES_C,
		      sizeof(CdaLVal), &CdaLVal, 0, 0))                  return;

geCdaTrk = 11;
   if (geCdaStoreItem(geCdaAgg, DDIF$_DHD_LANGUAGES,
		      strlen(Language), Language, 0, 0))                 return;
   /*
    * Link header aggregate to root - why ? beats me
    */
geCdaTrk = 12;
   if (geCdaStoreItem(geCdaAgg, DDIF$_DDF_HEADER,
		      sizeof(geCdaAgg), &geCdaAgg, 0, 0))
                                                                         return;
   if (geCdaCrAgg (DDIF$_SEG))                                           return;
   geCdaPrevAgg = geCdaAgg;
   geCdaSegAgg  = geCdaAgg;

geCdaTrk = 13;
   if (geCdaStoreItem(geCdaRtAgg, DDIF$_DDF_CONTENT,
		      sizeof(geCdaAgg), &geCdaAgg, 0, 0))
   /*                                                                     return;
    * Ok, now have all the individual objects write themselves out
    */
   while (SSegP = SSegP->Next)
     if (SSegP->Live && SSegP->Handler) GEVEC(GEWRITE, SSegP);

   /*
    * Clean up...
    */
   if (geCdaPutDoc())                                                    return;
   if (geCdaCloseFile())                                                 return;
   if (geCdaDelRtAgg())                                                  return;

   return;
  } 
#endif

/*
 * Must be reading in - open the file
 */
geCdaLen     = strlen(geInFile);
CdaResultLen = sizeof(CdaResultFileSpec);
geCdaAggType = DDIF$_DDF;
geCdaTrk           = 33;
geCdaStat = cda$open_file(&geCdaLen, geInFile, 0, 0, 0, 0, 0,
		        &geCdaAggType,
			0,
			&CdaResultLen,
		       	CdaResultFileSpec,
		       	&CdaResultLen,
			&geCdaStrm,
			&geCdaFile,
		       	&geCdaRtAgg);
   if ((BOMB(geCdaStat)) || (!geCdaRtAgg || ! geCdaStrm))
      {error_string = (char *) geFetchLiteral("GE_ERR_DDIFO", MrmRtypeChar8);
       if (error_string != NULL) 
	  {sprintf(geErrBuf, error_string, geCdaStat, geCdaTrk);
           geError(geErrBuf, FALSE);
	   XtFree(error_string);
	  }
       return(GERRDDIFO);
      }

/*
 * Disassemble the mess
 */
do
  {geCdaTrk            = 34;
   geCdaStat = cda$get_aggregate(&geCdaRtAgg,
			&geCdaStrm,
			&geCdaAgg,
                        &geCdaAggType);
   if (geCdaAgg && (BOMB(geCdaStat)))
      {error_string = (char *) geFetchLiteral("GE_ERR_DDIFO", MrmRtypeChar8);
       if (error_string != NULL) 
	  {sprintf(geErrBuf, error_string, geCdaStat, geCdaTrk);
           geError(geErrBuf, FALSE);
	   XtFree(error_string);
	  }
       return(GERRDDIFO);
      }

printf("<grfioddif>RtAggHan = %d, geCdaAggHan = %d\n",
geCdaRtAgg, geCdaAgg);

   if (geCdaAggType == DDIF$_SEG)
     {printf("<grfioddif> AggType = DDIF$_SEG\n");
      if (geCdaSegAgg = geCdaAgg)
        {CdaAggregateItem   = DDIF$_AGGREGATE_TYPE;
         geCdaTrk         = 35;
         geCdaStat = cda$locate_item(&geCdaRtAgg,
                              &geCdaSegAgg,
                              &CdaAggregateItem,
                              &CdaLVal,
                              &CdaBufLen,
                              &CdaAggregateIndex,
                              &CdaAddInfo);
         if (BOMB(geCdaStat))
            {error_string = (char *) geFetchLiteral("GE_ERR_DDIFO", MrmRtypeChar8);
             if (error_string != NULL) 
	       {sprintf(geErrBuf, error_string, geCdaStat, geCdaTrk);
                geError(geErrBuf, FALSE);
	   	XtFree(error_string);
	  	}
       	     return(GERRDDIFO);
 	    }
         printf("<grfioddif>CdaAggIt, CdaLVal = %d, %d\n",
                CdaAggregateItem, CdaLVal);
        }
      for (;;)
	{
/*	 geCdaTrk            = 36;
	 geCdaStat = cda$next_aggregate(&geCdaAgg,
                                  &geCdaNextAgg);
	 if (geCdaNextAgg && (BOMB(geCdaStat)))
      	   {error_string = (char *) geFetchLiteral("GE_ERR_DDIFO", MrmRtypeChar8);
             if (error_string != NULL) 
	       {sprintf(geErrBuf, error_string, geCdaStat, geCdaTrk);
                geError(geErrBuf, FALSE);
	   	XtFree(error_string);
	       }
            return(GERRDDIFO);
           }
*/
         CdaAggregateItem   = DDIF$_AGGREGATE_TYPE;
         geCdaTrk         = 35;
         geCdaStat = cda$locate_item(&geCdaRtAgg,
                              &geCdaAgg,
                              &CdaAggregateItem,
                              &CdaLVal,
                              &CdaBufLen,
                              &CdaAggregateIndex,
                              &CdaAddInfo);
         if (BOMB(geCdaStat))
           {error_string = (char *) geFetchLiteral("GE_ERR_DDIFO", MrmRtypeChar8);
            if (error_string != NULL) 
	      {sprintf(geErrBuf, error_string, geCdaStat, geCdaTrk);
               geError(geErrBuf, FALSE);
	       XtFree(error_string);
	      }
            return(GERRDDIFO);
           }
         printf("<grfioddif>CdaAggIt, CdaLVal = %d, %d\n",
                CdaAggregateItem, CdaLVal);
	 if (CdaAggregateItem == DDIF$_EOS)
	   {printf("<grfioddif> AggItem = DDIF$_EOS\n"); break;}
	 switch (CdaAggregateItem)
	   {case DDIF$_DSC: printf("<grfioddif> AggItem = DDIF$_DSC\n"); break;
	    case DDIF$_DHD: printf("<grfioddif> AggItem = DDIF$_DHD\n"); break;
	    case DDIF$_TXT: printf("<grfioddif> AggItem = DDIF$_TXT\n"); break;
            case DDIF$_GTX: printf("<grfioddif> AggItem = DDIF$_GTX\n"); break;
            case DDIF$_HRD: printf("<grfioddif> AggItem = DDIF$_HRD\n"); break;
            case DDIF$_SFT: printf("<grfioddif> AggItem = DDIF$_SFT\n"); break;
            case DDIF$_BEZ: printf("<grfioddif> AggItem = DDIF$_BEZ\n"); break;
            case DDIF$_LIN: printf("<grfioddif> AggItem = DDIF$_LIN\n"); break;
            case DDIF$_ARC: printf("<grfioddif> AggItem = DDIF$_ARC\n"); break;
            case DDIF$_FAS: printf("<grfioddif> AggItem = DDIF$_FAS\n"); break;
            case DDIF$_IMG: printf("<grfioddif> AggItem = DDIF$_IMG\n"); break;
            case DDIF$_CRF: printf("<grfioddif> AggItem = DDIF$_CRF\n"); break;
            case DDIF$_EXT: printf("<grfioddif> AggItem = DDIF$_EXT\n"); break;
            case DDIF$_PVT: printf("<grfioddif> AggItem = DDIF$_PVT\n"); break;
            case DDIF$_GLY: printf("<grfioddif> AggItem = DDIF$_GLY\n"); break;
            default:                                                     break;
	   }		      

        }
      continue;
     }

      switch (geCdaAggType)
	{case DDIF$_DSC: printf("<grfioddif> AggType = DDIF$_DSC\n"); break;
	 case DDIF$_DHD: printf("<grfioddif> AggType = DDIF$_DHD\n"); break;
	 case DDIF$_TXT: printf("<grfioddif> AggType = DDIF$_TXT\n"); break;
         case DDIF$_GTX: printf("<grfioddif> AggType = DDIF$_GTX\n"); break;
         case DDIF$_HRD: printf("<grfioddif> AggType = DDIF$_HRD\n"); break;
         case DDIF$_SFT: printf("<grfioddif> AggType = DDIF$_SFT\n"); break;
         case DDIF$_BEZ: printf("<grfioddif> AggType = DDIF$_BEZ\n"); break;
         case DDIF$_LIN: printf("<grfioddif> AggType = DDIF$_LIN\n"); break;
         case DDIF$_ARC: printf("<grfioddif> AggType = DDIF$_ARC\n"); break;
         case DDIF$_FAS: printf("<grfioddif> AggType = DDIF$_FAS\n"); break;
         case DDIF$_IMG: printf("<grfioddif> AggType = DDIF$_IMG\n"); break;
         case DDIF$_CRF: printf("<grfioddif> AggType = DDIF$_CRF\n"); break;
         case DDIF$_EXT: printf("<grfioddif> AggType = DDIF$_EXT\n"); break;
         case DDIF$_PVT: printf("<grfioddif> AggType = DDIF$_PVT\n"); break;
         case DDIF$_GLY: printf("<grfioddif> AggType = DDIF$_GLY\n"); break;
         case DDIF$_EOS: printf("<grfioddif> AggType = DDIF$_EOS\n"); break;
         default:                                                     break;
	}		      
  }while (geCdaAgg);


geCdaTrk         = 37;
geCdaStat = cda$close_file(&geCdaStrm,
			&geCdaFile);
if (BOMB(geCdaStat))
  {error_string = (char *) geFetchLiteral("GE_ERR_DDIFO", MrmRtypeChar8);
   if (error_string != NULL) 
      {sprintf(geErrBuf, error_string, geCdaStat, geCdaTrk);
       geError(geErrBuf, FALSE);
       XtFree(error_string);
      }
   return(GERRDDIFO);
  }

geCdaTrk         = 38;
geCdaStat = cda$delete_root_aggregate(&geCdaRtAgg);
if (BOMB(geCdaStat))
  {error_string = (char *) geFetchLiteral("GE_ERR_DDIFO", MrmRtypeChar8);
   if (error_string != NULL) 
      {sprintf(geErrBuf, error_string, geCdaStat, geCdaTrk);
       geError(geErrBuf, FALSE);
       XtFree(error_string);
      }
   return(GERRDDIFO);
  }

#endif 
/*
 * KLUDGE - End of ifdef for VMS - if vcc and lk ever get their act straightened
 * out so that this junk can be compiled and linked on ULTRIX, take this out.
 */

#endif
}
