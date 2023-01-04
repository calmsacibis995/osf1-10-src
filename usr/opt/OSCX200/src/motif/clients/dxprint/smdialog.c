/*
*****************************************************************************
**                                                                          *
**  COPYRIGHT (c) 1988, 1989, 1991, 1992 BY                                 *
**  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.                  *
**  ALL RIGHTS RESERVED.                                                    *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED   *
**  ONLY IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE   *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER   *
**  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY   *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE IS  HEREBY   *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE   *
**  AND  SHOULD  NOT  BE  CONSTRUED AS  A COMMITMENT BY DIGITAL EQUIPMENT   *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS   *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
**                                                                          *
*****************************************************************************
**
** FACILITY: PrintScreen
**
** ABSTRACT:
**
**	This module handles the x,y,width,height,label resources of the
**	print screen window.
**
** ENVIRONMENT:
**
**      VAX/VMS operating system.
**
** AUTHOR:  Karen Brouillette October 1989
**
** Modified by:
**
**	 4-Feb-1992	Edward P Luwish
**		Performance and I18N modifications
**
**	04-Apr-1991	Edward P Luwish
**		Port to Motif UI
**
*/

/*
! Include files
*/
#include "iprdw.h"
#include "smdata.h"
#include "smresource.h"
#include "smconstants.h"
#include "prdw_entry.h"
#ifdef VMS
#include <ssdef.h>
#endif /* VMS */

int	smattr_get_values()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This routine will get the resources from the resource database
**      for each item in the Session Customization.  We need to
**      set the values of the widgets when the dialog box is managed.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
char	svalue[256];
int	value[4];
int	size;

#if 0
no header...
/* message region header text */
size = sm_get_any_resource(smtext, svalue, value);
if ((smsetup.header_text) != NULL)
    XtFree(smsetup.header_text);
smsetup.header_text = XtMalloc(strlen(svalue) + 1);
strcpy(smsetup.header_text, svalue);
#endif

/* sm width and height */
size = sm_get_any_resource(smrows, svalue, value);
smsetup.rows = value[0];
size = sm_get_any_resource(smcols, svalue, value);
smsetup.cols = value[0];

/* sm x and y */
size = sm_get_any_resource(smx, svalue, value);
smsetup.x = value[0];
size = sm_get_any_resource(smy, svalue, value);
smsetup.y = value[0];

}

void set_control_size()
{
Arg	arglist[10];
Position    x,y;
Dimension   width,height,border_width;
char    *dummy;
unsigned long cols = 0;
unsigned long rows = 0;
long	byte_count, cvt_status;

/* Get the current x and y of the dialog box */
XtSetArg (arglist[0], XmNx, &x);
XtSetArg (arglist[1], XmNy, &y);
XtGetValues (smdata.toplevel, arglist, 2);

if ((x != smsetup.x) || (y != smsetup.y))
    {
    XtSetArg (arglist[0], XmNx, smsetup.x);
    XtSetArg (arglist[1], XmNy, smsetup.y);
    XtSetValues (smdata.toplevel, arglist, 2);
    }

if (smdata.message_area != 0)
    {
    /* Get the current width and height of the dialog box */
    XtSetArg (arglist[0], XmNheight, &rows);
    XtSetArg (arglist[1], XmNwidth, &cols);
    XtGetValues (XtParent(XtParent(smdata.message_area)), arglist, 2);

    if ((rows != smsetup.rows) || (cols != smsetup.cols))
	{
	XtSetArg (arglist[0], XmNheight, smsetup.rows);
	XtSetArg (arglist[1], XmNwidth, smsetup.cols);
	XtSetValues (XtParent(XtParent(smdata.message_area)), arglist, 2);
	}
    }

#if 0
no header...
/* set the correct header text in the control panel */
if (smdata.header_label != 0)
    {
    XtUnmanageChild(smdata.header_label);

#ifdef NOTV11

    dummy = XmStringCreate(smsetup.header_text, XmSTRING_DEFAULT_CHARSET);

#else

    dummy = DXmCvtFCtoCS(smsetup.header_text, &byte_count, &cvt_status);

#endif

    XtSetArg(arglist[0], XmNlabelString, dummy);
    XtSetValues(smdata.header_label, arglist, 1);
    XtManageChild(smdata.header_label);
    XtFree(dummy);
    }
#endif
}
