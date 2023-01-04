/*
*****************************************************************************
**                                                                          *
**  COPYRIGHT (c) 1988, 1989, 1991, 1992 BY                                       *
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
** FACILITY:  PrintScreen
**
** ABSTRACT:
**
**	This module handles displaying messages 
**	to the message box.
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
**		I18N modifications
**
**	03-Apr-1991	Edward P Luwish
**		Port to Motif UI
**
*/
#include "iprdw.h"

#include "smdata.h"
#ifdef VMS
#include <ssdef.h>
#else
#endif

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <X11/Vendor.h>
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

#include "prdw_entry.h"

void put_error
#if _PRDW_PROTO_
(
    unsigned int	status,
    int			text_index
)
#else
(status, text_index)
    unsigned int	status;
    int			text_index;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Put an error in the error dialog box.
**
**  FORMAL PARAMETERS:
**
**	status - A VMS status value
**	other_text - An index into the UIL message array. Or -1 if no
**		     text is to be displayed
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
*/
{
#ifdef vms
#pragma nostandard
    $DESCRIPTOR_S(mes_desc);
#pragma standard
#endif
    unsigned short	message_length;
    unsigned int	flags = 01;
    char		dummy[4];
    char		other_message[256];
    char		*theend;
    XmString 	message_text;
    XmString 	status_text;
    XmString 	whole_text;
    long		byte_count, cvt_status;

    /* nothing sent to this routine, just return */
    if ((status == 0) && (text_index < 0)) return;

    other_message[0] = 0;

    /* get the text for the status number */
#ifdef vms
    if (status != 0)
    {
	mes_desc.dsc$w_length=256;
	mes_desc.dsc$a_pointer=other_message;
	sys$getmsg(status, &message_length, &mes_desc, flags, dummy); 
	other_message[message_length] = 0;
	if (dummy[1] != 0)
	{
	    /* get rid of the FAO parameters, a status value alone does
		not give good values for FAO parameters */
	    theend = strchr(other_message, ',');
	    if (theend != 0)
		*theend = 0;
	}	    
    }
#endif
    /* get the UIL text to go with this error */
    if (text_index >= 0)
    {
	message_text = (XmString)get_drm_message (text_index);
	if (message_text == NULL) 
	{
	    printf
		("Session Error: DRM message #%d doesn't exist\n", text_index);
	    return;
	}
    }
    else
    {
	message_text = NULL;
    }

    /* it is possible that we haven't been able to open the display yet.  in
    ** that case, print the message to the log file.  Otherwise, put it in
    ** a dialog box.
    */
    if (smdata.toplevel != 0)
    {
	status_text = (XmString)DXmCvtFCtoCS(other_message, &byte_count, &cvt_status);
	whole_text = XmStringConcat(message_text, status_text);
	decw$error_display(whole_text);
	XtFree((char *)status_text);
	XtFree((char *)whole_text);
    }
    else
    {
	if (message_text != NULL)
	{	

	    theend = (char *)DXmCvtCStoFC(message_text, &byte_count, &cvt_status);
	    printf("Session Error: %s\n", theend);
	    XtFree(theend);
	}
	printf("%s\n", other_message);
    }
    return;
}
