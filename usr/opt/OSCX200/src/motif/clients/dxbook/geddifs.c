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
static char SccsId[] = "@(#)geddifs.c	1.1\t11/22/88";
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
**	GEDDIFS	             	       Real calls to ddif routines
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
**	GNE 07/13/88 Created
**
**--
**/

#ifdef GEDDIF

#include "geGks.h"
#ifdef VMS
#include <ddif$def.h>
#include <cda$def.h>
#include <cda$msg.h>
#else
#include <ddif_def.h>
#include <cda_def.h>
#include <cda_msg.h>
#endif

#define BOMB(x)         (!((x) & 1))

#ifdef VMS
#define cdaopenfile	cda$open_file
#define cdaclosefile	cda$close_file
#else
#define cdaopenfile	CdaOpenFile
#define cdaclosefile	CdaCloseFile
#endif

extern unsigned long	cdaopenfile(),
			cdaclosefile();

#endif

static char  		*error_string;

#if (GEDDIF && VMS && GERAGS)
extern unsigned long cda$create_root_aggregate(),
                     cda$create_file(),
                     cda$create_aggregate(),
                     cda$insert_aggregate(),
                     cda$store_item(),
		     cda$put_document(),
		     cda$get_document(),
		     cda$delete_root_aggregate(),
		     cda$locate_item();


/*
 * Make the root aggregate
 */
geCdaCrRtAgg()
{

unsigned long Type;

Type      = DDIF$_DDF;
geCdaTrk  = 0;
geCdaStat = cda$create_root_aggregate(0, 0, 0, 0,
				      &Type,
				      &geCdaRtAgg);
if (BOMB(geCdaStat))
  {error_string = (char *) geFetchLiteral("GE_ERR_DDIFO", MrmRtypeChar8);
   if (error_string != NULL) 
     {sprintf(geErrBuf, error_string, geCdaStat, geCdaTrk);
      geError(geErrBuf, FALSE);
      XtFree(error_string);
     }
   return(GERRDDIFO);
  }

return(0);

}

/*
 * Create a file
 */
geCdaCrFile(File)
char *File;
{
unsigned char ResFile[255];
unsigned long Len, ResLen;

Len       = strlen(File);
ResLen    = sizeof(ResFile);
geCdaTrk  = 1;
geCdaStat = cda$create_file(&Len,
			    File,
			    0, 0, 0, 0, 0,
			    &geCdaRtAgg,
			    &ResLen,
			    ResFile,
			    &ResLen,
			    &geCdaStrm,
			    &geCdaFile,
			    &geCdaRtAgg);
if (BOMB(geCdaStat))
  {error_string = (char *) geFetchLiteral("GE_ERR_DDIFO", MrmRtypeChar8);
   if (error_string != NULL) 
     {sprintf(geErrBuf, error_string, geCdaStat, geCdaTrk);
      geError(geErrBuf, FALSE);
      XtFree(error_string);
     }
   return(GERRDDIFO);
  }

return(0);

}

/*
 * Create an aggregate
 */
geCdaCrAgg(Type)
unsigned long Type;
{

geCdaTrk  = 3;
geCdaStat = cda$create_aggregate(&geCdaRtAgg,
				 &Type,
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

return(0);

}

/*
 * Get an aggregate
 */
geCdaGetAgg(Type)
unsigned long Type;
{

geCdaTrk  = 4;
geCdaStat = cda$get_aggregate(&geCdaRtAgg,
			      &geCdaStrm,
			      &geCdaAgg,
			      &geCdaAggType);
if (BOMB(geCdaStat))
  {error_string = (char *) geFetchLiteral("GE_ERR_DDIFO", MrmRtypeChar8);
   if (error_string != NULL) 
     {sprintf(geErrBuf, error_string, geCdaStat, geCdaTrk);
      geError(geErrBuf, FALSE);
      XtFree(error_string);
     }
   return(GERRDDIFO);
  }

return(0);

}

/*
 * Store item
 */
geCdaStoreItem(Agg, Item, BufLen, Buf, AggIndex, AddInfo)
unsigned long Agg, Item, BufLen;
unsigned char *Buf;
unsigned long AggIndex, AddInfo;
{

/* geCdaTrk     = 5; */
geCdaStat = cda$store_item(&geCdaRtAgg,
			   &Agg,
			   &Item,
			   &BufLen,
			   Buf,
			   &AggIndex,
			   &AddInfo);
if (BOMB(geCdaStat))
  {error_string = (char *) geFetchLiteral("GE_ERR_DDIFO", MrmRtypeChar8);
   if (error_string != NULL) 
     {sprintf(geErrBuf, error_string, geCdaStat, geCdaTrk);
      geError(geErrBuf, FALSE);
      XtFree(error_string);
     }
   return(GERRDDIFO);
  }

return(0);

}

/*
 * Write out the doc
 */
geCdaPutDoc()
{

geCdaTrk  = 200;
geCdaStat = cda$put_document(&geCdaRtAgg,
			     &geCdaStrm);
if (BOMB(geCdaStat))
  {error_string = (char *) geFetchLiteral("GE_ERR_DDIFO", MrmRtypeChar8);
   if (error_string != NULL) 
     {sprintf(geErrBuf, error_string, geCdaStat, geCdaTrk);
      geError(geErrBuf, FALSE);
      XtFree(error_string);
     }
   return(GERRDDIFO);
  }

return(0);

}
/*
 * Delete root aggregate
 */
geCdaDelRtAgg()
{

geCdaTrk  = 202;
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

return(0);

}

/*
 * Locate item
 */
geLocItem(Seg, Item, Buf, BufLen, AggIndex, AggInfo)
unsigned long Seg, Item;
char          *Buf;
unsigned long *BufLen, *AggIndex, *AggInfo;
{

geCdaTrk  = 203;
geCdaStat = cda$locate_item(&geCdaRtAgg,
			    &Seg,
			    Item,
			    Buf,
			    BufLen,
			    AggIndex,
			    AggInfo);
if (BOMB(geCdaStat))
  {error_string = (char *) geFetchLiteral("GE_ERR_DDIFO", MrmRtypeChar8);
   if (error_string != NULL) 
     {sprintf(geErrBuf, error_string, geCdaStat, geCdaTrk);
      geError(geErrBuf, FALSE);
      XtFree(error_string);
     }
   return(GERRDDIFO);
  }

return(0);

}

#endif
/*
 * END of #if (GEDDIF && VMS && GERAGS)
 */


/*
 * Open a file
 */
geCdaOpenFile(File)
char *File;
{
#ifdef GEDDIF

unsigned char ResFile[255];
unsigned long Len, ResLen, AggType;

Len      = strlen(File);
ResLen   = sizeof(ResFile);
geCdaTrk = 2;
AggType  = DDIF$_DDF;

#ifdef VMS

geCdaStat = cdaopenfile(&Len,
			File,
			0, 0, 0, 0, 0,
			&AggType,
			0,
			0,
			0,
			0,
			&geCdaStrm,
			&geCdaFile,
			&geCdaRtAgg);
#else

geCdaStat = cdaopenfile(Len,
			File,
			0, 0, 0, 0, 0,
			AggType,
			0,
			0,
			0,
			0,
			&geCdaStrm,
			&geCdaFile,
			&geCdaRtAgg);
#endif
if (BOMB(geCdaStat))
  {return(GERRDDIFO);}            /* geError was already called in geImgCrDDIF*/

#endif

return(0);

}

/*
 * Close the file
 */
geCdaCloseFile()
{
#ifdef GEDDIF


geCdaTrk  = 201;

#ifdef VMS

geCdaStat = cdaclosefile(&geCdaStrm,
			 &geCdaFile);

#else

geCdaStat = cdaclosefile(geCdaStrm,
			 geCdaFile);

#endif

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

return(0);

}
