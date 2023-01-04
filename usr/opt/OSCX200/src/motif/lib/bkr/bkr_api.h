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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_API.H*/
/* *12   17-JUN-1992 13:18:34 GOSSELIN "finished updating with HyperHelp function prototypes"*/
/* *11    8-JUN-1992 19:19:22 BALLENGER "UCX$CONVERT"*/
/* *10    8-JUN-1992 11:14:29 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *9    22-APR-1992 21:14:37 BALLENGER "Use XtResolvePathname and remove wait cursor support."*/
/* *8    16-APR-1992 18:22:52 BALLENGER "Fix syncronization & add work-in-progress dialog"*/
/* *7     3-MAR-1992 16:55:54 KARDON "UCXed"*/
/* *6     4-NOV-1991 15:45:49 GOSSELIN "fixed to make use of arglists"*/
/* *5    15-OCT-1991 09:29:05 GOSSELIN "isolated files"*/
/* *4    14-OCT-1991 12:09:30 BALLENGER "Fix synchronization problems."*/
/* *3    25-SEP-1991 18:07:58 BALLENGER "Fix bkr_api.h include problems"*/
/* *2    25-SEP-1991 16:41:50 BALLENGER "inlcude br_api.h and br_prototypes.h directly"*/
/* *1    16-SEP-1991 12:44:35 PARMENTER "Function Prototypes for bkr_api.c"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_API.H*/
/*
*****************************************************************************
**                                                                          *
**                     COPYRIGHT (c) 1990, 1991 BY                          *
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
*/

/*
**
**  FACILITY:  Bkr API (Callable Bookreader) routines
**
**  DESCRIPTION:  Public header file
**
**  AUTHOR:  Dan Gosselin, OSAG
**
**  CREATION DATE:  03-Nov-1991
**
**  MODIFICATION HISTORY:  last modification 17-Jun-1992
**  
*/

#include <X11/Intrinsic.h>

/* Constants */

#define BkrNfilename		"Bkr_Filename"
#define BkrNfilePath		"Bkr_FilePath"
#define BkrNfileType		"Bkr_FileType"
#define BkrNfileSuffix		"Bkr_FileSuffix"
#define BkrNobject		"Bkr_Object"
#define BkrNobjectName		"Bkr_ObjectName"
#define BkrNwindowUsage		"Bkr_Window_Usage"
#define BkrNxPosition		"Bkr_X_Position"
#define BkrNyPosition		"Bkr_Y_Position"

#define Bkr_Success		    0
#define Bkr_Busy		    1
#define Bkr_Send_Event_Failure	    2
#define Bkr_Startup_Failure	    3
#define Bkr_Create_Client_Failure   4
#define Bkr_Invalid_Object	    5
#define Bkr_Get_Data_Failure	    6
#define Bkr_Bad_Filename	    7

#define Bkr_Topic		    0
#define Bkr_Directory		    1
#define Bkr_Book		    2
#define Bkr_Widget		    3

#define Bkr_Default_Window	    0
#define Bkr_New_Window		    1


#ifndef PROTOTYPE
#if defined (VAXC) || defined(__STDC__) || defined(USE_PROTOS)
#define PROTOTYPE(args) args
#define PARAM_NAMES(names) (
#define PARAM_SEP ,
#define PARAM_END )
#define VOID_PARAM void
#else
#define PROTOTYPE(args) ()
#define PARAM_NAMES(names) names
#define PARAM_SEP ;
#define PARAM_END ;
#define VOID_PARAM 
#endif
#endif 


/* Bkr API function prototypes */

extern int BkrOpen
    PROTOTYPE((Opaque *, Widget, ArgList, int));

extern int BkrDisplay
    PROTOTYPE((Opaque, ArgList, int));

extern int BkrClose
    PROTOTYPE((Opaque, ArgList, int));


#ifndef VMS
/* Motif Help System function prototypes */

extern void DXmHelpSystemOpen
    PROTOTYPE((Opaque *, Widget, char *, void ((*)()), Opaque));

extern void DXmHelpSystemDisplay
    PROTOTYPE((Opaque, char *, char *, char *, void ((*)()), Opaque));

extern void DXmHelpSystemClose
    PROTOTYPE((Opaque, void ((*)()), Opaque));
#endif
