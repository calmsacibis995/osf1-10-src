/*
**++
**  COPYRIGHT (c) 1988, 1989, 1990, 1991, 1992 BY
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
**	 4-Feb-1992	Edward P Luwish
**		Performance and I18N modifications
**
**	16-Apr-1991	Edward P Luwish
**		Motif Print Widget support
**
**	29-Oct 1990	KMR
**	    Really use SetValues to set data syntax - there was an error before
**
**	30-Jun-1990	KMR
**	    Really allow DDIF print - there was an error before
**
**	13-Jun-1990	KMR
**	    Cleanup condition handling
**	    use SetValues to set data syntax
**	    allow DDIF print
**
**	17-May-1990	KMR		Kathy Robinson
**	    Get filename from callback - fixes multiple print problem
**
**	28-Jun-1989	B. Bazemore
**	    Pass filename directly to print cancel callback.
**
*/

#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /b5/aguws3.0/aguws3.0_rcs/src/dec/clients/print/getqueue.c,v 1.2 91/12/30 12:48:20 devbld Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */

#include "iprdw.h"

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <Mrm/MrmAppl.h>
#include <DXm/DXmPrint.h>
#include <DXm/DECspecific.h>
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

#include "prdw_entry.h"

static Widget	toplevel;		/* Widget id of toplevel widget       */
static Widget	print_widget_id = NULL;	/* Widget id of print widget          */
static int	delete_flag = 0xFFFFFFFF; /* Flag to suppress all options     */
static  char	buffer[100];
static XmString	user_supplied_data_syntaxes [3] = {NULL, NULL, NULL};
static XtCallbackRec	ok_callback[2], cancel_callback[2];

/*
** Static routines.
*/
static void dxPrscCancelCall PROTOTYPE((
    Widget	w,
    char	*print_filename,
    int		*datastruct
));

static void dxPrscOKCall PROTOTYPE((
    Widget	w,
    char	*print_filename,
    int		*datastruct
));

int dxPrscInitialize()
{

/*  Initialize DRM                                                            */
/*  This MUST be done before the XtInitialize call...                         */
/*                                                                            */

}

int getqueue
#if _PRDW_PROTO_
(
    dxPrscOptions	*options,
    Widget		top
)
#else
(options,top)
    dxPrscOptions	*options;
    Widget		top;
#endif
{
	int	status, synindex;
	static int	save_synindex = -1;
	long *ptr;
	XmString	title;
	XmString	testfilename[1];
	Arg 		arglist[10];
	int		ac;
	long		byte_count, cvt_status;

	toplevel = top;

    memcpy (buffer, command.print_dest, strlen(command.print_dest));
    buffer[strlen(command.print_dest)]=0;

    testfilename [0] = (XmString)DXmCvtFCtoCS
	(buffer, &byte_count, &cvt_status);

    if (user_supplied_data_syntaxes [0] == NULL)
    {
	user_supplied_data_syntaxes [0] = DXmCvtFCtoCS
	    ("ANSI2", &byte_count, &cvt_status);
	user_supplied_data_syntaxes [1] = DXmCvtFCtoCS
	    ("PostScript(R)", &byte_count, &cvt_status);
	user_supplied_data_syntaxes [2] = DXmCvtFCtoCS
	    ("DDIF", &byte_count, &cvt_status);
    }

    if (command.options.storage_format == dxPrscSixel)
	synindex=0;
    else if (command.options.storage_format == dxPrscPostscript)
	synindex=1;
    else if (command.options.storage_format == dxPrscDDIF)
	synindex=2;
    else
	synindex=1;

    /*
    ** Single file, derived queue names, user-supplied data syntax,
    ** suppress delete-after-print
    */
    ac = 0;
    XtSetArg(arglist[ac], DXmNfileNameList, testfilename); ac++;
    XtSetArg(arglist[ac], DXmNfileNameCount, 1); ac++;
    /*
    ** In order to not screw up the selection of printer queue within the
    ** list for a particular format, we don't modify the FormatList unless
    ** the format has changed.
    */
    if (save_synindex != synindex)
    {
	XtSetArg
	(
	    arglist[ac],
	    DXmNprintFormatList,
	    &user_supplied_data_syntaxes[synindex]
	); ac++;
	XtSetArg(arglist[ac], DXmNprintFormatCount, 1); ac++;
	save_synindex = synindex;
    }
    XtSetArg(
	arglist[ac],
	DXmNdeleteFile,
	(command.options.send_to_printer == dxPrscPrint)
    ); ac++;

    /*
    ** These 2 callbacks should be LAST on the list.  This allows us to
    ** leave them off of the SetValues call by just shortening the count by
    ** 2.
    */
    ok_callback[0].callback = (XtCallbackProc) dxPrscOKCall;
    ok_callback[0].closure = buffer;
    ok_callback[1].callback = NULL;
    ok_callback[1].closure = NULL;
    XtSetArg (arglist[ac], XmNokCallback, ok_callback); ac++;

    cancel_callback[0].callback = (XtCallbackProc) dxPrscCancelCall;
    cancel_callback[0].closure = buffer;
    cancel_callback[1].callback = NULL;
    cancel_callback[1].closure = NULL;
    XtSetArg(arglist[ac], XmNcancelCallback, cancel_callback); ac++;

    if (print_widget_id == NULL)
    {
	print_widget_id = DXmCreatePrintDialog
	(
	    toplevel,
	    "Queue Options", /* title for display */
	    arglist,
	    ac
	);
    }
    else
    {
	XtSetValues (print_widget_id, arglist, ac-2);
    }

    XmStringFree (testfilename[0]);

    /*
    ** Only display the printwidget if necessary.
    */
    if (command.options.print_widget == dxPrscPrintWidget)
    {
	XtManageChild (print_widget_id);
    }

    return (Normal);

}

static void dxPrscCancelCall
#if _PRDW_PROTO_
(
    Widget	w,
    char	*print_filename,
    int		*datastruct
)
#else
(w, print_filename, datastruct)
    Widget	w;
    char	*print_filename;
    int		*datastruct;
#endif
/* Receives control when the CANCEL button is pressed.                        */
/* For now, just destroy ourselves.                                           */
/*                                                                            */
{
    XtUnmanageChild (w);
    return;
}

static void dxPrscOKCall
#if _PRDW_PROTO_
(
    Widget	w,
    char	*print_filename,
    int		*datastruct
)
#else
(w, print_filename, datastruct)
    Widget	w;
    char	*print_filename;
    int		*datastruct;
#endif
/* Receives control when OK button is pressed.                                */
/* Call the PwPrintJob to actually output the file(s).                        */
/*                                                                            */
{
    long	    byte_count, cvt_status;

    ChfEstablish (img_signal_handler);

    XtUnmanageChild (w);
    XFlush(XtDisplay(w));

    timed_capture_screen ();

    command.options.print_widget = dxPrscPrintWidget;

    return;
}

send_print_job ()
{
    XmString		filename[1];
    long		byte_count, cvt_status;
    unsigned long	status;

    filename [0] = (XmString)DXmCvtFCtoCS
	(command.print_dest, &byte_count, &cvt_status);

    status = DXmPrintWgtPrintJob (print_widget_id, filename, 1);

    XmStringFree (filename);
}
