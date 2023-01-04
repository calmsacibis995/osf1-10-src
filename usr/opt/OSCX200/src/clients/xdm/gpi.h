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
 * @(#)$RCSfile: gpi.h,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/08/20 15:17:45 $
 */
/*****************************************************************************
 * gpi.h - A generic prompting interface				     *
 *****************************************************************************/

#ifndef _GPI_H
#define _GPI_H

/*****************************************************************************
 * These routines allow an application to display messages to the	     *
 * user (and wait for an acknowledgement) and to prompt the user for	     *
 * input.  All routines provide for timing out (when you cannot wait	     *
 * around forever), except in the case of the LoginPrompt		     *
 * convenience routine. 						     *
 * 									     *
 * A return value of GpiContinue means that input has been accepted.	     *
 * A return value of GpiAbort means that input was unsuccessful, or	     *
 * that the user canceled the input.  In the abort case, the return	     *
 * parameters are invalid. 						     *
 * 									     *
 * All parameters to the routines must be supplied by the caller.	     *
 * In particular, the storage for answerString is supplied by the	     *
 * caller, and the size of this storage is specified by			     *
 * maxAnswerLength. 							     *
 * 									     *
 * The GpiQueryUserWithList is intended to help in such situations	     *
 * as the the forced password update problem. 				     *
 * 									     *
 * Timeout values may be specified, in milliseconds, or as zero (0)	     *
 * implying no timeout is active. 					     *
 * 									     *
 * The shell widget may be supplied as NULL.  However, if Xt has	     *
 * been initialized, then the (top level) shell widget must be		     *
 * supplied. 								     *
 * 									     *
 * Resources available to the user of the GPI.				     *
 * 									     *
 * Resource				    Type      Default		     *
 * 									     *
 * *GpiDisplayMessage.dialogTitle	    String    "GpiDisplayMessage"    *
 * *GpiDisplayMessage.okLabelString	    String    "Acknowledge"	     *
 * 									     *
 * 									     *
 * *GpiQueryUserYesNo.dialogTitle	    String    "GpiQueryUser"	     *
 * *GpiQueryuserYesNo.okLabelString	    String    "Yes"		     *
 * *GpiQueryuserYesNo.cancelLabelString	    String    "No"		     *
 * 									     *
 * 									     *
 * *GpiQueryUser.dialogTitle		    String    "GpiQueryUser"	     *
 * *GpiQueryuser.okLabelString		    String    "Continue"	     *
 * *GpiQueryuser.cancelLabelString	    String    "Abort"		     *
 * 									     *
 * 									     *
 * *GpiQueryUserWithList.dialogTitle	    String    "GpiQueryUserWithList" *
 * *GpiQueryuserWithList.okLabelString	    String    "Continue"	     *
 * *GpiQueryuserWithList.cancelLabelString String    "Abort"		     *
 * 									     *
 * 									     *
 * *GpiLoginPrompt.title		    String    "GpiLoginPrompt"	     *
 * *GpiLoginPrompt.okLabelString	    String    "Ok"		     *
 * *GpiLoginPrompt.cancelLabelString	    String    "Abort"		     *
 * *GpiLoginPrompt.foreground		    Pixel     Black		     *
 * *GpiLoginPrompt.promptColor		    Pixel     Black		     *
 * *GpiLoginPrompt.greetColor		    Pixel     Black		     *
 * *GpiLoginPrompt.namePrompt		    String    "Name:"		     *
 * *GpiLoginPrompt.passwdPrompt	    	    String    "Password:"	     *
 * *GpiLoginPrompt.font		    	    String			     *
 *                        "*-new century schoolbook-medium-r-normal-*-180-*" *
 * *GpiLoginPrompt.promptFont		    String			     *
 *                        "*-new century schoolbook-bold-r-normal-*-180-*"   *
 * *GpiLoginPrompt.greetFont		    String			     *
 *                        "*-new century schoolbook-bold-i-normal-*-240-*"   *
 * *GpiLoginPrompt.greeting		    String			     *
 *                        "Welcome to the X Window System."		     *
 * 									     *
 * 									     *
 * *GpiLoginFail.dialogTitle		    String    "GpiLoginFail"	     *
 * *GpiLoginFail.okLabel		    String    "Acknowledge"	     *
 * *GpiLoginFail.failTimeout		    Int	      10		     *
 * *GpiLoginFail.failColor		    Pixel     Black		     *
 * *GpiLoginFail.failFont		    String			     *
 *                        "*-new century schoolbook-bold-r-normal-*-180-*"   *
 * *GpiLoginFail.fail			    String			     *
 *                        "Login failed, please try again."		     * 
 * 									     *
 * Note:  If the *GpiLoginPrompt.greeting contains the substring	     *
 * "%hostname%, that substring will be replaced by the name of the	     *
 * host machine for which the prompt is being done.			     *
 * 									     *
 *****************************************************************************/

/*****************************************************************************
 * Defines								     *
 *****************************************************************************/

#define XgpiNgreetColor		"greetColor"
#define XgpiNgreetFont		"greetFont"
#define XgpiNgreeting		"greeting"
#define XgpiNnamePrompt		"namePrompt"
#define XgpiNpasswdPrompt	"passwdPrompt"
#define XgpiNpromptColor	"promptColor"
#define XgpiNpromptFont		"promptFont"

#define XgpiCGreeting		"Greeting"
#define XgpiCNamePrompt		"NamePrompt"
#define XgpiCPasswdPrompt	"PasswdPrompt"

#define XgpiNfail		"fail"
#define XgpiNfailColor		"failColor"
#define XgpiNfailFont		"failFont"
#define XgpiNfailTimeout	"failTimeout"

#define XgpiCFail		"Fail"
#define XgpiCFailTimeout	"FailTimeout"

#ifdef unused
#define XgpiNunsecureGreeting	"unsecureGreeting"
#endif

/*****************************************************************************
 * Includes								     *
 *****************************************************************************/

#include <X11/Intrinsic.h>

/*****************************************************************************
 * Return values							     *
 *****************************************************************************/

typedef enum _GpiReturnValue
  {
    GpiAbort,			/* Abort further processing */
    GpiContinue			/* Continue processing */
  } GpiReturnValue;

extern Widget GpiShellWidget;


/*****************************************************************************
 * Prototypes								     *
 *****************************************************************************/

GpiReturnValue GpiEstablishShell( Widget shellWidget );

void GpiRemoveShell();

void GpiDisplayMessage( Widget	      shellWidget,
			unsigned long timeout,
			Boolean	      displayTimeout,
			unsigned char*	warningString );


int GpiLoginFail( Widget	 shellWidget,
		   unsigned long timeout,
		   char*	 failString );

/*****************************************************************************
 * Prototypes, added for SIA.                                                *
 *****************************************************************************/

GpiReturnValue GpiList(
   Widget shellWidget,
   unsigned long timeout,
   unsigned char* promptString,
   unsigned char* top_title,
   unsigned char** arrayOfStrings,
   int numArrayStrings,
   unsigned char** answerString,
   int maxAnswerLength[],
   Boolean multiple
);


GpiReturnValue GpiForm(
  Widget shellWidget,
  unsigned long timeout,
  unsigned char* client_title,
  XFontStruct* client_title_font,
  Pixel client_title_color,
  unsigned char* title,
  XFontStruct* title_font,
  Pixel title_color,
  int nprompts,
  unsigned char* promptStrings[],
  unsigned char* answerStrings[],
  XFontStruct* prompt_font,
  Pixel prompt_color,
  XFontStruct* answer_font,
  Pixel answer_color,
  int maxNameLength[],
  Boolean visible[]
);


#endif /* _GPI_H */
