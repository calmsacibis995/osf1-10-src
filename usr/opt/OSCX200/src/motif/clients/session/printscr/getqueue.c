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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/session/printscr/getqueue.c,v 1.1 90/01/01 00:00:00 devrcs Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/*
**++
**  COPYRIGHT (c) 1988, 1989 BY
**  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
**  ALL RIGHTS RESERVED.
**
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
**  ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
**  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
**  OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
**  TRANSFERRED.
**
**  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
**  AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
**  CORPORATION.
**
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
**--
**/
/*
**++
**  MODULE:
**
**	getqueue.c
**
**  FACILITY:
**      PrintScreen
**
**  ABSTRACT:
**
**	Subroutines which call printwidget
**
**  ENVIRONMENT:
**
**	VMS V5, Ultrix V2.2  DW FT1
**
**  AUTHORS:
**      Kathy Robinson
**
**  RELEASER:
**	Kathy Robinson
**
**  MODIFICATIONS:
**
**	28-Jun-1989	B. Bazemore
**	    Pass filename directly to print cancel callback.
**
*/
#define NULL 0
				       
#include <stdio.h>
#include <signal.h>
#include <iprdw.h>		       

#ifdef VMS
#include <decw$include/DECwDwtApplProg.h>
#else
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#endif

#ifdef PRINTWID
#include <PrintWgtDef.h>
#endif

static void dxPrscOKCall();	/* receives control when OK finally selected  */
static void dxPrscCancelCall();  /* receives control when CANCEL selected.     */

static Widget	toplevel;		/* Widget id of toplevel widget       */
static Widget	print_widget_id = NULL;	/* Widget id of print widget          */
static Boolean	delete_flag = TRUE;	/* Flag to suppress the delete-after- */
					/* print option.                      */

extern int condition_handler();

static  char	buffer[100];

static XmString	user_supplied_data_syntaxes [2];



/* Define the callback lists identifying our callback routines.               */
/*                                                                            */
XtCallbackRec dxPrscOKCalls [] = 
{{dxPrscOKCall, NULL},{NULL,NULL}};

XtCallbackRec dxPrscCancelCalls [] = 
{{dxPrscCancelCall, NULL},{NULL,NULL}};

int
dxPrscInitialize()
{

/*  Initialize DRM                                                            */
/*  This MUST be done before the XtInitialize call...                         */
/*                                                                            */

#ifdef PRINTWID
PwInitializeForDRM();
#endif
 
}


int 
getqueue(options,top)
dxPrscOptions *options;
Widget	top;			/* Widget id of toplevel widget       */

	{
	int	status, synindex;
	long *ptr;
	XmString	title;
	XmString	testfilename[1];

	toplevel = top;

if (options->print_queue != dxPrscImmediate)
	return(status = Normal);

bcopy( options->print_dest, buffer, strlen(options->print_dest));
buffer[strlen(options->print_dest)]=0;

testfilename [0] = XmStringLtoRCreate(buffer, "ISO8859-1") ;

/* Setup cancel callback with filename information	*/

dxPrscCancelCalls[0].closure = options->print_dest;

user_supplied_data_syntaxes [0] = XmStringLtoRCreate("ANSI2", "ISO8859-1");
user_supplied_data_syntaxes [1] = XmStringLtoRCreate("PostScript R ", "ISO8859-1");


if (options->storage_format == dxPrscDDIF)
	{
#	ifdef VMS
	    delete(buffer);
#	else
/*	    unlink(buffer);	*/
#	endif
	return(status = dxPrscInvStorage);
	}



if (options->storage_format == dxPrscSixel)
	synindex=0;
if (options->storage_format == dxPrscPostscript)
	synindex=1;


	/* Single file, derived queue names, user-supplied data syntax,
					suppress delete-after-print          */
#ifdef PRINTWID
    
    if (print_widget_id == NULL)
	print_widget_id = PwPrintJobInfo (
		toplevel,		/* root widget id                    */
  		XmStringLtoRCreate(" Queue Options", "ISO8859-1"),   /* title for display */
		FALSE,			/* no default positioning            */
		100,			/* x-position of print queue widget  */
		50,			/* y-position of print queue widget  */
		NULL,			/* Array of queue names              */
		0,			/* count of queue names              */
		NULL,			/* no form names supplied            */
		0,			/* so count is zero                  */
		testfilename,		/* Array of names of files to print  */
		1,			/* count of file names               */
		&user_supplied_data_syntaxes[synindex], 
		1,			/* count of user-supplied data syn   */
		&delete_flag,		/* option-suppression flag supplied  */
		dxPrscOKCalls,		/* ok callback list                  */
		dxPrscCancelCalls);	/* cancel callback list              */
#endif

/* Make this new widget a "managed" child of the overall application          */

XtManageChild (print_widget_id);

/* Make it appear on the screen, ready to interact with                       */

XtRealizeWidget (toplevel);

/* Never gets here...                                                         */
return (Normal);

}


/* Receives control when the CANCEL button is pressed.                        */
/* For now, just destroy ourselves.                                           */
/*                                                                            */
static void dxPrscCancelCall (w, print_filename, datastruct )

Widget		w;
char		*print_filename;
int		*datastruct;
{
	

#ifdef VMS
   VAXC$ESTABLISH( condition_handler );
#else
   ChfEstablish( condition_handler );
#endif

#ifdef PRINTWID
/*printf ("Getqueue.c: Cancel callback:  file name is %s\n", options->print_dest); */

/* delete the file */
    if (print_filename != 0)
#ifdef VMS
        delete(print_filename);
#else
/*	unlink(print_filename);	*/
#endif

XtFree(user_supplied_data_syntaxes [0]);
XtFree(user_supplied_data_syntaxes [1]);

XtUnmanageChild (w);
#endif
return;
}

/* Receives control when OK button is pressed.                                */
/* Call the PwPrintJob to actually output the file(s).                        */
/*                                                                            */
static void dxPrscOKCall (w, tag, datastruct )
Widget	w;
char	*tag;
int	*datastruct;
{
unsigned int ret_status;		/* status returned from PwPrintJob    */
Arg al [2];
XmString	*filename;
void		(*signal_routine)();
int		signal_mask;

#ifdef VMS
   VAXC$ESTABLISH( condition_handler );
#else
   ChfEstablish ( condition_handler );
#endif

#ifdef PRINTWID
XtSetArg (al [0], DwtNdeleteFileChoice, FALSE);
XtSetValues (w, al, 1);

XtSetArg (al [0], DwtNfileNameList, &filename);
XtGetValues (w, al, 1);

/* The following four ugly lines are needed to get the current value of */
/* the SIGCHLD signal handler, since PwPrintJob mungs it never to */
/* see the light of day again. */

signal_mask = sigblock( sigmask(SIGCHLD) );
signal_routine = signal(SIGCHLD,SIG_DFL);
signal(SIGCHLD,signal_routine);
sigsetmask( signal_mask );

ret_status = PwPrintJob (w,		/* widget id */
			 filename,	/* array of file names */
			 1);		/* count of file names */

signal(SIGCHLD,signal_routine);		/* restore lost signal routine */

/* should free all the Latin 1 strings here! */
XtFree(user_supplied_data_syntaxes [0]);
XtFree(user_supplied_data_syntaxes [1]);
#endif

XtUnmanageChild (w);
return;
}

/* stubs to make this thing compile, if nothing else! */
void PwInitializeForDRM() {}
Widget PwPrintJobInfo() {}
unsigned int PwPrintJob() {}
