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
** COPYRIGHT (c) 1989, 1990, 1991 BY
** DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
** ALL RIGHTS RESERVED.
**
** THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
** ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
** INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
** COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
** OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
** TRANSFERRED.
**
** THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
** AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
** CORPORATION.
**
** DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
** SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
*/


/*
**++
**  Subsystem:
**	LinkWorks Manager User Interface
**
**  Version: V1.0
**
**  Abstract:
**	Clipboard support routines..
**
**  Keywords:
**	{keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	Patricia Avigdor
**
**  Creation Date: 14-Feb-1990
**
**  Modification History:
**--
*/

/*
**  Include Files
*/

#include "hs_include.h"
#include "hs_abstract_objects.h"
#include "hs_decwindows.h"
#include <Xm/CutPaste.h>

#define _HsFormat	    "LINKWORKSMGR"
#define _HsFormatLength	    8
/*
**  Forward Routine Declarations	
*/
/*
**  Static Data Definitions
*/

_Void  EnvDWClipboardCopyToClipboard(private, select_data, timestamp)
_WindowPrivate private;
 _SelectData
select_data;
 Time timestamp;

/*
**++
**  Functional Description:
**	Copies a selected data to the clipboard.
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    Display		    *display;
    Window		    window;
    unsigned int	    clip_status ;
    long		    item_id;
    unsigned int	    data_id ;
    Cardinal		    length ;
    XmString		    application_name;
    _SvnData		    tag;
    _Integer		    type;
    lwk_object		    hisobject;
    lwk_object		    obj_desc;
    lwk_encoding	    encoding;
    lwk_integer		    encoding_size;
    _CString		    cs_name;
    _DDIFString		    ddif_name;
    lwk_status		    status;

    /*
    ** Set the values to be copied to the clipboard.
    */
    encoding = (lwk_encoding) 0;
    tag = select_data->svn_data[0];

    if (tag->object != (_HsObject)_NullObject) {
	/*
	** Get the object name.
	*/
	EnvDWGetObjectName(tag, &ddif_name);

	/*
	** Get the object descriptor encoding.
	*/
	EnvDWGetObjectEncoding(tag, &encoding, &encoding_size);

	/*
	** Set the parameters and start the copy to clipboard.
	*/
	display = XtDisplay(private->main_widget);
	window = XtWindow(private->main_widget);
	application_name = _StringToXmString((char *)"LinkWorks Manager");
	
	clip_status = XmClipboardStartCopy (display, window,
	    application_name, timestamp, 0, 0, &item_id) ;
	if (clip_status != ClipboardSuccess) {
	    _Raise(clip_locked); /* clipboard locked */
	}
	/*
	** Copy the object name to the clipboard.
	*/
	length = _LengthDDIFString(ddif_name);
	clip_status = XmClipboardCopy (display, window,
		item_id, "DDIF", (_DDIFString)ddif_name,
		length, 0, (int *) &data_id);
	if (clip_status != ClipboardSuccess) {
	    XmClipboardCancelCopy(display, window, item_id);
	    _Raise(clip_locked); /* clipboard locked */
	}
				
	/*
	** Copy the object descriptor to the clipboard.
	*/
	clip_status = XmClipboardCopy (display, window,
		 item_id, _HsFormat, (char *)encoding,
		 encoding_size, 0, (int *) &data_id);
	if (clip_status != ClipboardSuccess) {
	    XmClipboardCancelCopy(display, window, item_id);
	    _Raise(clip_locked); /* clipboard locked*/
	}
	/*
	** End the copy to clipboard.
	*/
	clip_status =  XmClipboardEndCopy (display, window, item_id);
	if (clip_status != ClipboardSuccess) {
	    XmClipboardCancelCopy(display, window, item_id);
	    _Raise(clip_locked); /* clipboard locked*/
        }
	/*
	** Delete the encoding.
	*/
	status = lwk_delete_encoding(&encoding);
	if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(encode_deletion_failed); /*encoding deletion failed*/
        }
	XmStringFree(application_name);
    }


    return;
    }
                                                    
_Void  EnvDWClipboardCopyFromClipboard(private, object, timestamp)
_WindowPrivate private;

lwk_object *object;
 Time timestamp;

/*
**++
**  Functional Description:
**	Copies from the clipboard.
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    Display		    *display;
    Window		    window;
    unsigned int	    clip_status;
    unsigned int	    private_id;
    Cardinal		    length;
    Cardinal		    outlength;
    lwk_object		    hisobject;
    lwk_encoding	    encoding = NULL;
    lwk_integer		    encoding_size;
    lwk_status		    status;
    long		    item_id;

    _StartExceptionBlock

    /*
    ** Set parameters.
    */
    display = XtDisplay(private->main_widget);
    window = XtWindow(private->main_widget);

    /*
    ** Start copy from.
    */
    clip_status = XmClipboardStartRetrieve (display, window, timestamp);

    if (clip_status != ClipboardSuccess) {
	_Raise(clip_locked); /* clipboard locked */
    }
    /*
    ** Inquire next paste length.
    */
    clip_status = XmClipboardInquireLength(display, window, _HsFormat,
	(unsigned long *) &length);

    if (clip_status != ClipboardSuccess) {
	_Raise(copy_from_clip_failed); /* copy from clipboard failed */
    }
    /*
    ** Copy from clipboard.
    */
    encoding = (char *) _AllocateMem(length);
    _ClearMem(encoding, length);
    clip_status = XmClipboardRetrieve(display, window, _HsFormat,
	(char *)encoding, length, (unsigned long *) &outlength,
	(int *) &private_id);
    if (clip_status != ClipboardSuccess) {
	_Raise(copy_from_clip_failed); /* copy from clipboard failed */
    }
    /*
    ** End copy from clipboard.
    */
    clip_status = XmClipboardEndRetrieve(display, window);
    if (clip_status != ClipboardSuccess) {
	_Raise(copy_from_clip_failed); /* copy from clipboard failed */
    }

    /*
    ** Create an object from the encoding returned.
    */
    status = lwk_import(lwk_c_domain_object_desc, encoding, object);	
    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(objdsc_decode_error); /*object descriptor decoding failed */
    }

    _FreeMem(encoding);

    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
        _WhenOthers
	    if (encoding != NULL)
		_FreeMem(encoding);
            _Reraise;
	
    _EndExceptionBlock

    return;
    }

_Boolean  EnvDWClipboardInquireNextPaste(private, timestamp)
_WindowPrivate private;
 Time timestamp;

/*
**++
**  Functional Description:
**	Copies from the clipboard.
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    Display		    *display;
    Window		    window;
    unsigned int	    clip_status;
    int			    max_length;
    int			    count;
    char		    *format_name;
    int			    i;
    unsigned long	    copied_length;
    _Boolean		    success = _False;
    Window		    owner;

    /*
    ** Set the parameters.
    */
    display = XtDisplay(private->main_widget);
    window = XtWindow(private->main_widget);

    /*
    ** Work around because nextpastecount locks the clipboard if it's empty.
    */
    owner = XGetSelectionOwner(display,
	XInternAtom(display, "CLIPBOARD", FALSE));

    if (owner != NULL) {
	/*
	** Inquire next paste count.
	*/
	clip_status = XmClipboardInquireCount(display, window,
	    &count, &max_length);

	if (clip_status != ClipboardSuccess) {
	    if (clip_status == ClipboardNoData)
		success = _False;
	    else
		_Raise(clip_locked); /* clipboard locked*/
	}
	else {
	    if (count >= 1) {
		format_name = (char *) _AllocateMem(max_length);
		_ClearMem(format_name, max_length);
		for(i = 1; i <= count; i++) {
		clip_status = XmClipboardInquireFormat(display, window,
		    i, format_name, max_length, &copied_length);
		if (clip_status != ClipboardSuccess)
		    _Raise(clip_locked); /*clipboard locked */
		if (!_CompareStringN(format_name, _HsFormat, copied_length))
		    success = _True;
		}
	    _FreeMem(format_name);
	    }	
	}
    }

    return success;
    }


_Void  EnvDWGetObjectName(svn_data, ddif_name)
_SvnData svn_data;
 _DDIFString *ddif_name;

/*
**++
**  Functional Description:
**	Gets the object name.
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _Integer		    type;
    _CString		    cs_name;
    lwk_status		    status;


    _StartExceptionBlock

    if (svn_data->object != (_HsObject)_NullObject) {
	/*
	** Get the object domain.
	*/
	_GetValue(svn_data->object, _P_HisType, hs_c_domain_integer, &type);

	/*
	** It's a persistent object.
	*/
	if ((lwk_domain)type != lwk_c_domain_object_desc) {
	
	    /*
	    ** Get the object name.
	    */
	
	    _GetCSProperty(svn_data->object, lwk_c_p_name, &cs_name);	
	}
	else {
	
	    /*
	    ** Get the object descriptor name.
	    */
	
	    _GetCSProperty(svn_data->object, lwk_c_p_object_name, &cs_name);
	}
	/*
	** Convert the object name.
	*/

	*ddif_name = _CStringToDDIFString(cs_name);

	_FreeMem(cs_name);
    }
    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception. Unlock the clipboard.
    */

    _Exceptions
        _WhenOthers
	    if (cs_name != (_CString) 0)
		_FreeMem(cs_name);
            _Reraise;
	
    _EndExceptionBlock

    return;
    }

_Void  EnvDWGetObjectEncoding(svn_data, encoding, encoding_size)
_SvnData svn_data;
 lwk_encoding *encoding;

lwk_integer *encoding_size;

/*
**++
**  Functional Description:
**	Returns the object encoding
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _Integer		    type;
    lwk_object		    hisobject;
    lwk_object		    obj_desc;
    lwk_status		    status;

    _StartExceptionBlock
    if (svn_data->object != (_HsObject)_NullObject) {
	/*
	** Get the HIS object.
	*/
	_GetValue(svn_data->object, _P_HisObject, hs_c_domain_lwk_object,
	   &hisobject);
	/*
	** Get the object domain.
	*/
	_GetValue(svn_data->object, _P_HisType, hs_c_domain_integer, &type);

	/*
	** It's a persistent object.
	*/
	if ((lwk_domain)type != lwk_c_domain_object_desc) {
	    /*
	    ** Get the object descriptor.
	    */
	    status = lwk_get_object_descriptor(hisobject, &obj_desc);
	
	    if (status != lwk_s_success) 
		_Raise(get_objdsc_failed); /*object descriptor query failed */
	}
	else {
	    obj_desc = hisobject;
	}
	/*
	** Encode the object descriptor.
	*/
	status = lwk_export(obj_desc, encoding, encoding_size);
	
	if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(objdsc_encode_error); /*object descriptor encoding failed */
	}
    }
    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception. Unlock the clipboard.
    */

    _Exceptions
        _WhenOthers
	    if (*encoding != (lwk_encoding) 0)
		lwk_delete_encoding(encoding);
            _Reraise;
	
    _EndExceptionBlock
    return;
    }
