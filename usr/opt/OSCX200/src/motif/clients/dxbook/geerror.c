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
static char SccsId[] = "@(#)geerror.c	1.1\t11/22/88";
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
**	GEERROR				Error handler (errors detected within)
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
**	GNE 10/30/86 Created
**	DAA 11/22/88 Revised
**
**--
**/

#include "geGks.h"

void	geMnError();                            /* forward declaration */

geError(ErrString, Abort)
char	*ErrString;
Boolean  Abort;
{                       
Widget  	cancel_button, help_button, shell;
XmString	cs_error_msg;
long            byte_cnt, stat;
Arg		al[1];

/*
 * Note that the default response to an error is to CONTINUE EXECUTION.
 * If an ABORT is desired then the flag "Abort" must be set to TRUE
 * inside the case
 */

if (Abort)
  {
#ifdef VOILA_ERR  
   if (geRun.ErrReport) voila$error(ErrString);
#else
   if (geRun.ErrReport) printf (ErrString);
   _exit();
#endif
  }

if (geRun.ErrReport)
  {
#ifdef VOILA_ERR
   voila$error(ErrString);
#else

#ifdef GERAGS
   if (geDispDev && geRun.ErrReport)
     {XBell(geDispDev, GEBEEP_ERR);
      XBell(geDispDev, GEBEEP_ERR);
     }

   if (!geUI.WidgetArray[GEWERROR])
     {if (geState.APagP)
	   shell = XtParent(geState.APagP->WidgetId);	
      else shell = geUI.ToplevelWidget;
      if (MrmFetchWidget(geUI.Id, GEERROR_MESSAGE, shell,
                       &geUI.WidgetArray[GEWERROR], &geUI.Class) != MrmSUCCESS)
          {/*
       	    * Can't find error widget, print the error to the terminal
	    */
           printf(ErrString);
	   return;
          }
     }
/* Get rid of the Cancel and Help buttons */
   if (cancel_button = XmMessageBoxGetChild(geUI.WidgetArray[GEWERROR],
					  	      XmDIALOG_CANCEL_BUTTON))
     XtUnmanageChild(cancel_button);
   if (help_button = XmMessageBoxGetChild(geUI.WidgetArray[GEWERROR],
    	    	    	    	    	   	      XmDIALOG_HELP_BUTTON))
     XtUnmanageChild(help_button);

   cs_error_msg = DXmCvtFCtoCS(ErrString, &byte_cnt, &stat);
   GESET_ARG(geUI.WidgetArray[GEWERROR], XmNmessageString, cs_error_msg);
   XmStringFree(cs_error_msg);
   geSetModalWMHints(geUI.WidgetArray[GEWERROR]);	
   XtManageChild(geUI.WidgetArray[GEWERROR]);
#else
  printf (ErrString);
#endif

#endif
 }
return;

}

/****************************************************************************/
#ifdef GERAGS
void
geMnError(widg,tag,data)
Widget			widg;
int			*tag;
XmAnyCallbackStruct	*data;
{

XtUnmanageChild(geUI.WidgetArray[GEWERROR]);
XtDestroyWidget(geUI.WidgetArray[GEWERROR]);
geUI.WidgetArray[GEWERROR] = 0;	

}
#endif

/**************************************************************************/
caddr_t
geFetchLiteral(index_string, data_type)
String  	index_string;
MrmCode 	data_type;
{
    int	    	status;
    caddr_t 	value;
    MrmCode 	value_type;

#if defined (GERAGS) || defined (GEVOYER)

    status = MrmFetchLiteral( 
    	    	geUI.Id,
    	    	index_string,	    /* name of literal to fetch    */
    	    	geDispDev,
    	    	&value,	    	    /* value of the literal 	   */
    	    	&value_type);	    /* returned literals data type */
    if (status != MrmSUCCESS)
    	return (caddr_t)NULL;

    if (value_type != data_type)
    	return (caddr_t) NULL;

    return (caddr_t)value;

#else
return (caddr_t)NULL;
#endif

};  /* end of geFetchLiteral */

/*
 * Based on the input error code, this routine
 * will fetch the correct error string from the UID file and pass it on to
 * geError.
 */
geErrorReport(Code, Abort)
long     Code;
Boolean  Abort;
{
char *sp, *ep;

sp = ep = NULL;

switch (Code)
  {default:
     return;

   case GERRXOPENMETAI:
     break;

   case GERRVERSION:
     sp = ep = (char *)geFetchLiteral("GE_ERR_VERSION",MrmRtypeChar8);
     break;

   case GERRBADMETA:
     sp = ep = (char *)geFetchLiteral("GE_ERR_BADMETA", MrmRtypeChar8);
     break;

   case GERRNOTMETA:
     sp = ep = (char *)geFetchLiteral("GE_ERR_NOTMETA", MrmRtypeChar8);
     break;

   case GERROUTOFCOLS:
     sp = ep = (char *)geFetchLiteral("GE_ERR_OUTOFCOLS", MrmRtypeChar8);
     break;

   case GERRBADIMAGE:
     break;

   case GERRDDIFO:
     break;

   case GERRUNAVAILABLE:
     sp = ep = (char *) geFetchLiteral("GE_ERR_UNAVAILABLE", MrmRtypeChar8);
     break;

   case GERRGRAVERSION:
     sp = ep = (char *) geFetchLiteral("GE_ERR_GRA_VERSION", MrmRtypeChar8);
     break;

   case GERRXOPEN:
     if (sp = (char *)geFetchLiteral("GE_ERR_XOPEN", MrmRtypeChar8))
       {sprintf(geErrBuf, sp, geInFile);
	ep = geErrBuf;
       }
     break;

   case GERRWRONGWINDOW:
     break;

   case GERRPIXMAPALLOC:
     sp = ep = (char *) geFetchLiteral("GE_ERR_PIXMAPALLOC", MrmRtypeChar8);
     break;

  }

if (ep) geError(ep, Abort);
if (sp) XtFree(sp);

}
