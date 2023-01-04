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
/*
 * @(#)$RCSfile: dxmcbrwidget.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/08/24 18:38:11 $
 */
/****************************************************************************/
/*                                                                          */  
/*  Copyright (c) Digital Equipment Corporation, 1990                       */
/*  All Rights Reserved.  Unpublished rights reserved                       */
/*  under the copyright laws of the United States.                          */
/*                                                                          */  
/*  The software contained on this media is proprietary                     */
/*  to and embodies the confidential technology of                          */
/*  Digital Equipment Corporation.  Possession, use,                        */
/*  duplication or dissemination of the software and                        */
/*  media is authorized only pursuant to a valid written                    */
/*  license from Digital Equipment Corporation.                             */
/*                                                                          */  
/*  RESTRICTED RIGHTS LEGEND   Use, duplication, or                         */
/*  disclosure by the U.S. Government is subject to                         */
/*  restrictions as set forth in Subparagraph (c)(1)(ii)                    */
/*  of DFARS 252.227-7013, or in FAR 52.227-19, as                          */
/*  applicable.                                                             */
/*                                                                          */  
/****************************************************************************/

#ifndef _CbrWidget_h_
#define _CbrWidget_h_

/*---------------------------------------------------------------------*/
/* CbrServices widget class declarations                               */
/*---------------------------------------------------------------------*/

#define DXmCbrClassCbrShell		        "DXmCbrServicesShellClass"
#define DXmCbrClassCbr			        "DXmCbrServices"
#define DXmCbrClassEdit			        "DXmCbrEdit"
#define DXmCbrClassConcepts           	"DXmCbrConcepts"
#define DXmCbrClassWordList           	"DXmCbrWordlist"
#define DXmCbrClassQuery                "DXmCbrConceptByExample"
#define DXmCbrClassResults            	"DXmCbrResults"

#define	DXmNlanguage			        "language"
#define	DXmCLanguage			        "Language"

#define	DXmNsearchLabelString        	"searchLabelString"
#define	DXmCSearchLabelString        	"SearchLabelString"

#define	DXmNcancelLabelString        	"CancelLabelString"
#define	DXmCCancelLabelString        	"CancelLabelString"

#define	DXmNreviewCallback      	    "reviewCallback"
#define	DXmCreviewCallBack      	    "ReviewCallback"

#define DXmNquickQuery			        "quickQuery"
#define DXmCquickQuery			        "QuickQuery"

#define DXmNstemming	    		    "stemming"
#define DXmCstemming	    		    "Stemming"

/*---------------------------------------------------------------------*/
/* Cbr widget constants                                                */
/*---------------------------------------------------------------------*/

#define DXmCBR_NONE                     0
#define DXmCBR_CONCEPT                  1
#define DXmCBR_RESULTS                  2
#define DXmCBR_EDIT                     3
#define DXmCBR_QUERY                   	4
#define DXmCBR_USER_DEFINED             5

/*----------------------------------------------------------------------*/
/* Cbr Widget callback reasons                                          */
/*----------------------------------------------------------------------*/

#define DXmCR_CBR_QUERY_SEARCH		    1	/* QUERY:  search	*/
#define DXmCR_CBR_QUERY_REVIEW		    2	/* QUERY:  review	*/
#define DXmCR_CBR_CONCEPT_SEARCH	    3	/* concept:  search	*/
#define DXmCR_CBR_CONCEPT_REVIEW	    4	/* concept:  review	*/
#define DXmCR_CBR_CONCEPT_SELECT	    5	/* concept:  select	*/
#define DXmCR_CBR_EDIT_SEARCH		    6	/* edit: search		*/
#define DXmCR_CBR_EDIT_SAVE		        7	/* edit: save		*/
#define DXmCR_CBR_EDIT_FETCH_CONCEPT	8	/* edit: fetch concept 	*/
#define DXmCR_CBR_RESULTS_SELECT	    9	/* result:  select	*/

/*----------------------------------------------------------------------*/
/* Cbr language constants                                          	*/
/*----------------------------------------------------------------------*/

#define DXmCBRENGLISH    2
#define DXmCBRFRENCH     3
#define DXmCBRGERMAN     4
#define DXmCBRDUTCH      5
#define DXmCBRSWEDISH    6
#define DXmCBRDANISH     7
#define DXmCBRNORWEGIAN  8
#define DXmCBRSPANISH    9
#define DXmCBRITALIAN   10

/*----------------------------------------------------------------------*/
/* Cbr main activate Callback definition                                */
/*----------------------------------------------------------------------*/

typedef struct cbr_callback_struct
{
    int         reason;			/* DXmCR_CBR_whatever		*/
    XEvent     	*event;			/* XEvent 			        */
    int         language;       /* language 			    */
    Boolean	    quick_query;	/* quick query? 		    */
    Boolean	    stemming;		/* stemming?			    */
    char       	*value;			/* query, concept, etc.		*/
    int		    position;		/* position in list 		*/
    XmString    name;           /* name identifier of value */    

} DXmCbrCallbackStruct;

/*---------------------------------------------------------------------*/
/* List access constants					                           */
/*---------------------------------------------------------------------*/

#define DXmCBR_ADD_AT_TOP	    -1
#define DXmCBR_ADD_AT_BOTTOM	-2

/*---------------------------------------------------------------------*/
/* Cbr Mrm initialize                                                  */
/*---------------------------------------------------------------------*/

void	DXmCbrInitializeForMrm ();

#endif /* _CbrWidget_h_ */

/* DON'T ADD STUFF AFTER THIS #endif */
