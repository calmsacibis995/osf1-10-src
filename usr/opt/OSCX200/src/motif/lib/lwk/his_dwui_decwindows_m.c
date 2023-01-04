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
** COPYRIGHT (c) 1988, 1989, 1990, 1991, 1992 BY
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
**	LinkWorks Services
**
**  Version: V1.0
**
**  Abstract:
**	DECwindows User Interface support for DXmUi object
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	Pat Avigdor
**
**  Creation Date: 12-Oct-88
**
**  Modification History:
**	BL4  dpr  24-Jan-89 -- some serious clean up after BL4 code review
**--
*/


/*
**  Include Files
*/

#include "his_include.h"
#include "lwk_abstract_objects.h"
#include "his_dwui_decwindows_m.h"
#include <Xm/Text.h>

/*
**  Widget Border fudge factors for DXmPositionWidget
*/

#define LEFT_MARGIN 10
#define RIGHT_MARGIN 10
#define BOTTOM_MARGIN 10
#define TOP_MARGIN 25

/*
** Constants needed for finding the length of a ASN.1 encoded string.
*/

/* Define the extended id. */
#define TAG_ID_EXTENDED       0x1F
#define TAG_ID_MORE           0x80

/* Define the extended length. */
#define TAG_LEN_COUNT         0x7F
#define TAG_LEN_EXTENDED      0x80

/** WARNING:  remove when DXm provides this constant - DXmCvtStatusOK **/

#define DXmCvtStatusOK 1

/*
**  Type Definitions
*/

typedef struct rect {
	Position x1, y1, /* (X,Y) lower left coordinate of rectangle   */
	         x2, y2; /* (X,Y) upper right coordinate of rectangle  */
	Dimension width, /* Width of rectangle			       */
	         height; /* Height of rectangle			       */
	int      area;   /* Area of rectangle			       */
    } rect;
			
/*
**  Table of Contents (static routines)
*/

_DeclareFunction(static void screen_to_rect,
    (Widget widget, rect *screen_rect));
_DeclareFunction(static void widget_to_rect,
    (Widget widget, rect *widget_rect));
_DeclareFunction(static void calc_position,
    (rect *widget, rect *ref_rect, rect *screen_rect));
_DeclareFunction(static void set_position,
    (Widget widget, Position x, Position y));
_DeclareFunction(static void SetIcons,
    (Widget shell, _DXmUiPrivate private));
_DeclareFunction(static _String GetIconIndexName,
    (Display *dpy, _String root_icon_literal,
    unsigned int *icon_size_rtn, char **supported_icon_sizes,
    int num_supported_sizes));
_DeclareFunction(static void FetchBitmap,
    (Widget shell, _DXmUiPrivate private, _String icon_literal, Pixmap *icon));
_DeclareFunction(static void XUIWMSetIconifyPixmap,
    (Widget widget, Pixmap pixmap));
_DeclareFunction(static int asn_1_str_len,
    (unsigned char *asn_1_str));


_CString  LwkDXmCopyCString(cstring)
_CString cstring;

/*
**++
**  Functional Description:
**	{@description@}
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
    int len;
    _CString copy;

    if (cstring == (_CString) _NullObject)
	copy = cstring;
    else {
	len = _LengthCString(cstring);

	copy = (_CString) _AllocateMem(len);

	_CopyMem((_Void *) cstring, (_Void *) copy, len);
    }

    return copy;
    }


_CString  LwkDXmConcatCString(cstring1, cstring2)
_CString cstring1;
 _CString cstring2;

/*
**++
**  Functional Description:
**	{@description@}
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
    XmString xstring;
    _CString cstring;

    if (cstring2 == (_CString) _NullObject) {
	if (cstring1 == (_CString) _NullObject)
	    cstring = cstring1;
	else
	    cstring = _CopyCString(cstring1);
    }
    else {
	if (cstring1 == (_CString) _NullObject)
	    cstring = _CopyCString(cstring2);
	else {
	    xstring = XmStringConcat((XmString) cstring1, (XmString) cstring2);

	    cstring = _CopyCString((_CString) xstring);

	    XmStringFree(xstring);
	}
    }

    return cstring;
    }


void  LwkDXmDeleteCString(cstring)
_CString *cstring;

/*
**++
**  Functional Description:
**	{@description@}
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
    if (*cstring != (_CString) _NullObject) {
	_FreeMem(*cstring);

	*cstring = (_CString) _NullObject;
    }

    return;
    }

_Integer  LwkDXmLengthCString(cstring)
_CString cstring;

/*
**++
**  Functional Description:
**	{@description@}
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
    if (cstring == (_CString) _NullObject)
	return (_Integer) 0;
    else
	return (_Integer) XmStringLength((XmString) cstring);
    }


_Integer  LwkDXmLengthDDIFString(ddifstring)
_DDIFString ddifstring;

/*
**++
**  Functional Description:
**	{@description@}
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

    long byte_cnt;

    byte_cnt = (_Integer) 0;

    if (ddifstring != (_DDIFString) _NullObject) 
	/*								    
	**  This is not an official routine but it works.
	**  We will use a DDIF lib routine when it becomes available.
	*/

	byte_cnt = asn_1_str_len(ddifstring);

    return (_Integer) byte_cnt;
    }


void  LwkDXmDeleteDDIFString(ddifstring)
_DDIFString *ddifstring;

/*
**++
**  Functional Description:
**	{@description@}
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
    if (*ddifstring != (_DDIFString) _NullObject) {
	_FreeMem(*ddifstring);

	*ddifstring = (_DDIFString) _NullObject;
    }

    return;
    }


_String  LwkDXmDDIFStringToString(ddifstring)
_DDIFString ddifstring;

/*
**++
**  Functional Description:
**	{@description@}
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
    _String string;
    XmString cstring;

    /* If the ddifstring is 0, return an empty string. */

    if (ddifstring == (_DDIFString) 0) {
	string = _CopyString((_String) "");
    }
    else {              

	/* convert the DDIF string to a compound string */

	cstring = (XmString) _DDIFStringToCString(ddifstring);

	/* convert the compound string to a string */

	string = (_String) _CStringToString(cstring);

	_DeleteCString(&cstring);
    }

    return string;
    }


_CString  LwkDXmStringToDDIFString(string)
_String string;

/*
**++
**  Functional Description:
**	{@description@}
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
    XmString xstring;
    _DDIFString ddifstring;

    if (string == (_String) _NullObject)
	ddifstring = (_DDIFString) _NullObject;
	
    else {

	xstring = _StringToXmString((char *) string);

	ddifstring = _CStringToDDIFString(xstring);

	XmStringFree(xstring);
    }

    return ddifstring;
    }


_CString  LwkDXmDateToCString(date)
_Date date;

/*
**++
**  Functional Description:
**	{@description@}
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
    XmString xtstring;
    _String string;
    _CString cstring;

    if (date == (_Date) _NullObject)
	cstring = (_CString) _NullObject;
    else {
	string = _DateToString(date);

	xtstring = _StringToXmString((char *) string);

	cstring = _CopyCString((_CString) xtstring);

	_DeleteString(&string);
	XmStringFree(xtstring);
    }

    return cstring;
    }



_Boolean  LwkDXmContainsDDIFString(ddifstring1, ddifstring2)
 _DDIFString ddifstring1;

    _DDIFString ddifstring2;

/*
**++
**  Functional Description:
**	{@description@}
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
    _Integer answer;
    _String string1;
    _String string2;

    string1 = _DDIFStringToString(ddifstring1);
    string2 = _DDIFStringToString(ddifstring2);

    answer = _ContainsString(string1, string2);

    _DeleteString(&string1);
    _DeleteString(&string2);

    return answer;
    }


_Boolean  LwkDXmIsValidDDIFString(ddifstring)
_DDIFString ddifstring;

/*
**++
**  Functional Description:
**	{@description@}
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
    /*
    **	This is an expensive way to check the validity of a compound string,
    **	but there don't seem to be many alternative!
    */

/*
    if (_LengthDDIFString(ddifstring) > 0)
	return _True;
    else
	return _False;
*/
    return _True;
    }


_CString  LwkDXmUserNameToDDIFString()
/*
**++
**  Functional Description:
**	{@description@}
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
    return _StringToDDIFString((_String) cuserid(NULL));
    }



_CString  LwkDXmStringToCString(string)
_String string;

/*
**++
**  Functional Description:
**	{@description@}
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
    XmString xmstring;
    _CString cstring;

    if (string == (_String) _NullObject)
	cstring = (_CString) _NullObject;
	
    else {
    
	xmstring = _StringToXmString((char *) string);

	cstring = _CopyCString((_CString) xmstring);

	XmStringFree(xmstring);
    }

    return cstring;
    }


XmString  LwkDXmStringToXmString(string)
char *string;

/*
**++
**  Functional Description:
**  
**	Given a file code string, create a compound string.
**	The XmString returned from this routine should be freed using
**	the toolkit XmStringFree routine.
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
    XmString xmstring;
    long byte_count;
    long status;
    
    xmstring = (XmString) DXmCvtFCtoCS((Opaque) string, &byte_count, &status);

    if (status != DXmCvtStatusOK)
	_Raise(inv_string);

    return xmstring;
    }
    

_CString  LwkDXmStatusToHelpKey(code)
_Status code;

/*
**++
**  Functional Description:
**	{@description@}
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
    int status;
    MrmCode type;
    String filename;
    _String string;
    _CString cstring;
    XmString xstring;
    MrmHierarchy hierarchy;

    /*
    **  Convert the status code into a DRM value identifier
    */

    string = _StatusToHelpKeyResource(code);

    /*
    ** Open the DRM hierarchy.
    */

    filename = _UidFileName;

    status = MrmOpenHierarchy((MrmCount) 1, &filename, (MrmOsOpenParamPtr *) 0,
	&hierarchy);

    /*
    **  Fetch the value of the identifier
    */

    if (status == MrmSUCCESS) {
	xstring = (XmString) 0;

	status = MrmFetchLiteral(hierarchy, string, (Display *) 0,
	    (caddr_t *) &xstring, &type);

	/*
	**  If not found, try the generic help key
	*/

	if (status != MrmSUCCESS || xstring == (XmString) 0) {
	    string = _GenericHelpKeyResource;

	    status = MrmFetchLiteral(hierarchy, string, (Display *) 0,
		(caddr_t *) &xstring, &type);

	    if (status != MrmSUCCESS)
		xstring = (XmString) 0;
	}

	/*
	** Close the DRM hierarchy.
	*/

	MrmCloseHierarchy(hierarchy);
    }

    /*
    **  If no valid string was found, return a default
    */

    if (xstring == (XmString) 0)
	if (type == MrmRtypeCString)
	    cstring = _StringToCString(_GenericHelpKeyResource);
	else
	    cstring = _CopyString(_GenericHelpKeyResource);
    else
	if (type == MrmRtypeCString)
	    cstring = _CopyCString((_CString) xstring);
	else
	    cstring = _CopyString((_String) xstring);

    /*
    **  Return the value
    */

    return cstring;
    }


_CString  LwkDXmStatusToCString(code)
_Status code;

/*
**++
**  Functional Description:
**	{@description@}
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
    int status;
    MrmCode type;
    String filename;
    _String string;
    _CString cstring;
    XmString xstring;
    MrmHierarchy hierarchy;

    /*
    **  Convert the status code into a DRM value identifier
    */

    string = _StatusToResource(code);

    /*
    ** Open the DRM hierarchy.
    */

    filename = _UidFileName;

    status = MrmOpenHierarchy((MrmCount) 1, &filename, (MrmOsOpenParamPtr *) 0,
	&hierarchy);

    /*
    **  Fetch the value of the identifier
    */

    if (status == MrmSUCCESS) {
	xstring = (XmString) 0;

	status = MrmFetchLiteral(hierarchy, string, (Display *) 0,
	    (caddr_t *) &xstring, &type);

	/*
	**  If not found, try Unknown Status
	*/

	if (status != MrmSUCCESS || xstring == (XmString) 0) {
	    string = _StatusToResource(_StatusCode(unknown));

	    status = MrmFetchLiteral(hierarchy, string, (Display *) 0,
		(caddr_t *) &xstring, &type);

	    if (status != MrmSUCCESS)
		xstring = (XmString) 0;
	}

	/*
	** Close the DRM hierarchy.
	*/

	MrmCloseHierarchy(hierarchy);
    }

    /*
    **  If no valid string was found, return a default
    */

    if (xstring == (XmString) 0)
	cstring = _StringToCString(_UnknownStatusString);
    else
	cstring = _CopyCString((_CString) xstring);

    /*
    **  Return the value
    */

    return cstring;
    }


_DDIFString  LwkDXmStatusToDDIFString(code)
_Status code;

/*
**++
**  Functional Description:
**	{@description@}
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
    int status;
    MrmCode type;
    String filename;
    _String string;
    XmString xstring;
    MrmHierarchy hierarchy;
    _DDIFString ddifstring;

    /*
    **  Convert the status code into a DRM value identifier
    */

    string = _StatusToResource(code);

    /*
    ** Open the DRM hierarchy.
    */

    filename = _UidFileName;

    status = MrmOpenHierarchy((MrmCount) 1, &filename, (MrmOsOpenParamPtr *) 0,
	&hierarchy);

    /*
    **  Fetch the value of the identifier
    */

    if (status == MrmSUCCESS) {
	xstring = (XmString) 0;

	status = MrmFetchLiteral(hierarchy, string, (Display *) 0,
	    (caddr_t *) &xstring, &type);

	/*
	**  If not found, try Unknown Status
	*/

	if (status != MrmSUCCESS || xstring == (XmString) 0) {
	    string = _StatusToResource(_StatusCode(unknown));

	    status = MrmFetchLiteral(hierarchy, string, (Display *) 0,
		(caddr_t *) &xstring, &type);

	    if (status != MrmSUCCESS)
		xstring = (XmString) 0;
	}

	/*
	** Close the DRM hierarchy.
	*/

	MrmCloseHierarchy(hierarchy);
    }

    /*
    **  If no valid string was found, return a default
    */

    if (xstring == (XmString) 0)
	ddifstring = _StringToDDIFString(_UnknownStatusString);
    else
	ddifstring = _CStringToDDIFString(xstring);

    XmStringFree(xstring);
	
    /*
    **  Return the value
    */

    return ddifstring;
    }



_Integer  LwkDXmCompareDDIFString(ddifstring1, ddifstring2)
_DDIFString ddifstring1;
 _DDIFString ddifstring2;

/*
**++
**  Functional Description:
**	{@description@}
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
    _Integer answer;
    _String string1;
    _String string2;

    string1 = _DDIFStringToString(ddifstring1);
    string2 = _DDIFStringToString(ddifstring2);

    answer = _CompareString(string1, string2);

    _DeleteString(&string1);
    _DeleteString(&string2);

    return answer;
    }


_DDIFString  LwkDXmCopyDDIFString(ddifstring)
_DDIFString ddifstring;

/*
**++
**  Functional Description:
**	{@description@}
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
    int len;
    _DDIFString copy;

    if (ddifstring == (_DDIFString) _NullObject)
	copy = ddifstring;
	
    else {

	len = _LengthDDIFString(ddifstring);

	copy = (_DDIFString) _AllocateMem(len);

	_CopyMem((_Void *) ddifstring, (_Void *) copy, len);
    }

    return copy;
    }


void  LwkGetCSValue(object, property, cstring)
_Object object;
 _String property;
 _CString *cstring;

/*
**++
**  Functional Description:
**	{@description@}
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
    _DDIFString	ddif_str = (_DDIFString) _NullObject;

    _GetValue(object, property, lwk_c_domain_ddif_string, &ddif_str);

    /*
    ** WARNING: this should be replaced with the macro when it gets defined
    **
    **	_DDIFStringToCString(ddif_str);
    */

    *cstring = LwkDXmDDIFStringToCString(ddif_str);

    _DeleteDDIFString(&ddif_str);

    return;
    }


_CString  LwkDXmDDIFStringToCString(ddifstring)
_DDIFString ddifstring;

/*
**++
**  Functional Description:
**	{@description@}
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
    _CString	cstring;
    long	byte_cnt;
    long	status;
    XmString 	xm_string;

    _StartExceptionBlock

    cstring = (_CString) _NullObject;
    xm_string = (XmString) _NullObject;

    if (ddifstring != _NullObject) {

	xm_string =  (XmString) DXmCvtDDIFtoCS((Opaque) ddifstring, &byte_cnt,
	    (long *) &status);

	if (status != DXmCvtStatusOK)
	    _Raise(inv_ddif_string);
	
	cstring = _CopyCString((_CString) xm_string);

	XmStringFree(xm_string);
    }
	
    /*
    **  If an exception is raised, clean up then reraise it.
    */

    _Exceptions
	_WhenOthers
	    _DeleteCString(&cstring);
	    if (xm_string != (XmString) 0)
		XmStringFree(xm_string);
	    _Reraise;
    _EndExceptionBlock

    return cstring;
    }



_DDIFString  LwkDXmCStringToDDIFString(cstring)
_CString cstring;

/*
**++
**  Functional Description:
**	{@description@}
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
    Opaque	tmp_ddifstring;
    _DDIFString	ddifstring;
    long	byte_cnt;
    long	status;
    XmString 	xm_string;

    _StartExceptionBlock

    ddifstring = (_DDIFString) _NullObject;
    tmp_ddifstring = (Opaque) _NullObject;

    if (cstring != _NullObject) {

	tmp_ddifstring =  (Opaque) DXmCvtCStoDDIF((Opaque) cstring, &byte_cnt,
	    &status);

	if (status != DXmCvtStatusOK)
	    _Raise(inv_ddif_string);

	ddifstring = _CopyDDIFString((_DDIFString) tmp_ddifstring);

	XtFree(tmp_ddifstring);
    }

    /*
    **  If an exception is raised, clean up then reraise it.
    */

    _Exceptions
	_WhenOthers
	    _DeleteDDIFString(&ddifstring);
	    if (tmp_ddifstring != (Opaque) 0)
		XtFree(tmp_ddifstring);
	    _Reraise;
    _EndExceptionBlock
	
    return ddifstring;
    }


_String  LwkDXmCStringToString(cstring)
_CString cstring;

/*
**++
**  Functional Description:
**	{@description@}
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
    int status;
    char *text;
    _String string;
    XmStringCharSet charset;
    XmStringContext context;
    XmStringDirection direction;
    Boolean separator;

    status = XmStringInitContext (&context, (XmString) cstring);

    if (status != TRUE)
	string = _CopyString((_String) "");
    else {
	/*
	**  Get first segment
	*/

	status = XmStringGetNextSegment (context, &text, &charset,
	    &direction, &separator);

	if (status != TRUE)
	    string = _CopyString((_String) "");
	else {
	    string = _CopyString((_String) text);

	    XtFree(charset);
	    XtFree(text);

	    /*
	    **  And then get any additional segments
	    */

	    while (_True) {
		status = XmStringGetNextSegment (context, &text, &charset,
		    &direction, &separator);

		if (status != TRUE)
		    break;

		string = _ConcatString(string, (_String) text);

		XtFree(charset);
		XtFree(text);
	    }
	}
	XmStringFreeContext(context);
    }

    return string;
}

static void  XUIWMSetIconifyPixmap(widget, pixmap)
Widget widget;
 Pixmap pixmap;

/*
**++
**  Functional Description:
**
**	Set the iconify pixmap for interoperation with the DEC XUI
**	window manager.
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

typedef struct {
    int value_mask;
    int iconify_pixmap;
    int icon_box_x;
    int icon_box_y;
    int tiled;
    int sticky;
    int no_iconify_button;
    int no_lower_button;
    int no_resize_button;
} internalDECWmHintsRec, *internalDECWmHints;

#define WmNumDECWmHintsElements (sizeof(internalDECWmHintsRec)/sizeof(int))

    internalDECWmHints	  prop = 0;
    internalDECWmHintsRec prop_rec;
    Atom    	    	  decwmhints;
    Atom    	    	  actual_type;
    int     	    	  actual_format;
    unsigned long    	  leftover;
    unsigned long   	  nitems;
    Boolean 	    	  free_prop = True;

    decwmhints = XmInternAtom(XtDisplay(widget), "DEC_WM_HINTS", FALSE);

    if ( XGetWindowProperty( XtDisplay(widget), XtWindow(widget), decwmhints,
    	    	0L, (long)WmNumDECWmHintsElements, False, decwmhints, 
    	    	&actual_type, &actual_format, &nitems, 
    	    	&leftover, (unsigned char **)&prop ) 
    	    != Success ) 
    	return;

    if ( ( actual_type != decwmhints ) 
    	 || ( nitems < WmNumDECWmHintsElements ) || ( actual_format != 32 ) )
    {
    	if ( prop != 0 ) XFree ( (char *)prop );

    	/*  An empty "value" was returned, so create a new one.
    	 */
    	free_prop = False;
    	prop = (internalDECWmHints) &prop_rec;
    	prop->value_mask    	= 0;
    	prop->icon_box_x 	= -1;
    	prop->icon_box_y 	= -1;
    	prop->tiled 	    	= False;
    	prop->sticky     	= False;
    	prop->no_iconify_button	= False;
    	prop->no_lower_button	= False;
    	prop->no_resize_button	= False;
    }

    prop->value_mask	 |= DECWmIconifyPixmapMask;
    prop->iconify_pixmap  = pixmap;

    XChangeProperty( XtDisplay(widget), XtWindow(widget), decwmhints, 
    	    	     decwmhints, sizeof(int)*8, PropModeReplace, 
    	    	     (unsigned char *) prop, WmNumDECWmHintsElements );

    if ( free_prop )
    	if ( prop != 0 ) XFree ( (char *)prop );

    return;    
}


void  LwkDXmSetShellIconState (shell, state)
Widget shell;
 long state;

/*
**++
**  Functional Description:
**
**	Set the iconic state of the given shell.
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
    XEvent event;
    Screen *scrn = XtScreen(shell);
    Display *dpy = XtDisplay(shell);

    if (LwkDXmIsXUIWMRunning(shell)) {

	/*
        **  Special case XUI WM
	*/
	
        XWMHints hints;
        hints.flags = StateHint;
        hints.initial_state = state;

        XSetWMHints(dpy, XtWindow(shell), &hints);

	if (state == NormalState)
	    XtPopup(shell, XtGrabNone);
	    
    } else {
    
	/*
        **  Assume that we have an ICCCM compliant WM running
	*/
	
        event.xclient.type = ClientMessage;
        event.xclient.display = dpy;
        event.xclient.window = XtWindow(shell);
        event.xclient.message_type = XmInternAtom (dpy,
            "WM_CHANGE_STATE", False);
        event.xclient.format = 32;
        event.xclient.data.l[0] = state;
        if (event.xclient.message_type != None)
	    XSendEvent(dpy, RootWindowOfScreen(scrn), False,
		SubstructureRedirectMask|SubstructureNotifyMask, &event);
    }
	
    if (state == NormalState) 
	XRaiseWindow(XtDisplay(shell), XtWindow(shell));
	
    }

                                                            
void  LwkDXmPositionWidget(new_widget, ref_widget)
Widget new_widget;
 Widget ref_widget;

/*
**++
**  Functional Description:
**	Position a new Widget on the screen according to the position
**	of a reference widget.
**	
**  Keywords:
**	Position, Widget
**
**  Arguments:
**	new_widget   : The ID of the Widget to be positioned (must be realized)
**	ref_widget   : Widget to reference
**
**  Result:
**	Void
**
**  Exceptions:
**	None
**--
*/
    {                        
    rect widget_rect;	/* Rectangle describing new widget		*/
    rect ref_rect;	/* Rectangle describing a widget to reference   */
    rect screen_rect;	/* Rectangle describing the screen		*/

    /*
    **  Get the screen coordinates.
    */

    screen_to_rect(new_widget, &screen_rect);

    /*
    **  Get the rectangular coordinates of the new widget.
    */

    widget_to_rect(new_widget, &widget_rect);

    /*
    **  If the new Widget is larger than the screen, just position it at (0,0).
    **	If there is no reference widget, center the new widget on the screen.
    */

    if (widget_rect.area >= screen_rect.area) {
	set_position(new_widget, 0, 0);
	return;
    }

    if (ref_widget == NULL) {
	set_position(new_widget,
		     (screen_rect.width - widget_rect.width) / 2,
		     (screen_rect.height - widget_rect.height) / 2);
	return;
    }

    widget_to_rect(ref_widget, &ref_rect);

    /*
    **  Position widget according to reference widget
    */

    calc_position(&widget_rect, &ref_rect, &screen_rect);
    
    set_position(new_widget, widget_rect.x1, widget_rect.y1);

    return;
    }

static void  screen_to_rect(widget, pscr)
Widget widget;
 rect *pscr;

/*
**++
**  Functional Description:
**	Get the coordinates of the screen
**
**  Keywords:
**	None
**
**  Arguments:
**	widget : any Widget on the screen
**	
**  Result:
**	Void
**
**  Exceptions:
**	None
**--
*/
    {
    pscr->width = WidthOfScreen(XtScreen(widget));
    pscr->height = HeightOfScreen(XtScreen(widget));

    pscr->area = pscr->height * pscr->width;
    
    pscr->x1 = 0;
    pscr->y1 = 0;
    pscr->x2 = pscr->width;
    pscr->y2 = pscr->height;

    return;
    }


static void  widget_to_rect(widget, widget_rect)
Widget widget;
 rect *widget_rect;

/*
**++
**  Functional Description:
**	Determine the screen coordinates of a Widget.
**
**  Keywords:
**	None
**
**  Arguments:
**	widget : Widget to analyze
**	widget_rect : rect structure to fill
**
**  Result:
**	Void
**
**  Exceptions:
**	None
**--
*/
    {
    int x, y;
    Dimension width, height;
    Arg	arglist[2];
    Window window;

    /*
    **	Get the size and the coordinates of the Widget.  If it is not realized
    **	we can't get the coordintates, so create a dummy rectangle.
    */

    if (XtIsRealized(widget)) {
	XtSetArg(arglist [0], XmNwidth,  &width);
	XtSetArg(arglist [1], XmNheight, &height);
	XtGetValues(widget, arglist, 2);

	XTranslateCoordinates(XtDisplay(widget), XtWindow(widget),
	    RootWindowOfScreen(XtScreen(widget)), 0, 0, &x, &y,
	    &window);

	/*
	**  Fudge in a border for the Widget
	*/

	widget_rect->width = width + LEFT_MARGIN + RIGHT_MARGIN;
	widget_rect->height = height + TOP_MARGIN + BOTTOM_MARGIN;
	widget_rect->x1 = x - LEFT_MARGIN;
	widget_rect->y1 = y - TOP_MARGIN;
    }
    else {
	widget_rect->width = 1;
	widget_rect->height = 1;
	widget_rect->x1 = 0;
	widget_rect->y1 = 0;
    }

    /*
    **  Calculate the other rectangular coordinates
    */

    widget_rect->x2 = widget_rect->x1 + widget_rect->width;
    widget_rect->y2 = widget_rect->y1 + widget_rect->height;
    widget_rect->area = widget_rect->height * widget_rect->width;

    return;
    }
                           

static void  calc_position(widget, pref, pscr)
rect *widget;
 rect *pref;
 rect *pscr;

/*
**++
**  Functional Description:
**
**	Attempt to place a widget centered over the reference widget
**
**  Keywords:
**	None
**
**  Arguments:
**	widget : rectangle structure for the widget to be positioned
**	pref :  rectangle structure for parent widget to be referenceed
**	pscr   : rectangle structure for screen 
**
**  Result:
**	Void
**
**  Exceptions:
**	None
**--
*/
    {
    Position best_x, best_y;

    /*
    ** Attempt to place the widget centered on the reference widget
    */
    
    best_x = (pref->x1 + pref->x2 - widget->width)/2;
    best_y = (pref->y1 + pref->y2 - widget->height)/2;

    /*
    **	Shift the widget if it is off the screen
    */
    
    if (best_y + widget->height > pscr->y2)
	best_y = pscr->y2 - widget->height;	    /* shift up */
    if (best_y < pscr->y1)
	best_y = pscr->y1;			    /* shift down */
    if (best_x < pscr->x1)
	best_x = pscr->x1;			    /* shift right */
    if (best_x + widget->width > pscr->x2)
	best_x = pscr->x2 - widget->width;	    /* shift left */

    /*
    **	Set the rectangle coordinates of the new widget to what we think is
    **	best.
    */
    
    widget->x1 = best_x;
    widget->y1 = best_y;
    widget->x2 = widget->x1 + widget->width;
    widget->y2 = widget->y1 + widget->height;
    
    return;
    }


static void  set_position(widget, x, y)
Widget widget;
 Position x;
 Position y;

/*
**++
**  Functional Description:
**	Set the postion of a widget to a given (X,Y) coordinate
**
**  Keywords:
**	Nonme
**
**  Arguments:
**	widget : ID of the widget to be positioned
**	x : X coordinate
**	y : Y coordinate
**
**  Result:
**	Void
**
**  Exceptions:
**	None
**--
*/
    {
    Arg	arglist[2];

    XtSetArg(arglist[0], XmNx, x + LEFT_MARGIN);
    XtSetArg(arglist[1], XmNy, y + TOP_MARGIN);
    XtSetValues(widget, arglist, 2);
               
    return;
    }



void  LwkDXmSetXUIDialogTitle(dialog, part1, part2)
Widget dialog;
 _CString part1;
 _CString part2;

/*
**++
**  Functional Description:
**	Set the dialog title when XUI WM is running - given two
**	_CStrings that are appended to create the title.
**	
**	If the XUI Window Manager is running, create a single segment
**	XmString from the two strings. This is done because the
**	XUI window manager does not handle COMPOUND_TEXT
**	dialog titles, only STRING.  If the XmString is multi segment then
**	the toolkit turns it into COMPOUND_TEXT, otherwise it is
**	turned into a STRING for transfer to the WM.  The XUI
**	Window Manager ignores COMPOUND_TEXT titles.
**	
**
**  Keywords:
**	Nonme
**
**  Arguments:
**	dialog : widget id of the dialog box
**	part1 : first part of title
**	part2 : second part of title
**
**  Result:
**	Void
**
**  Exceptions:
**	None
**--
*/
    {
    _String str1;
    _String str2;
    _CString title;
    Arg	arglist[2];
    int ac;

    /*    
    ** Convert _CStrings to ascii strings
    */

    str1 = _CStringToString((_CString) part1);
    str2 = _CStringToString((_CString) part2);

    /*
    ** Concatenate ascii strings
    */

    str1 = _ConcatString(str1, str2);

    /*
    ** Convert ascii string to an XmString
    */

    title = _StringToCString(str1);

    /*
    ** Set the title
    */

    ac = 0;
    XtSetArg(arglist[ac], XmNdialogTitle, title);
    ac++;

    XtSetValues(dialog, arglist, ac);

    _DeleteString(&str1);
    _DeleteString(&str2);

    return;
    }


Boolean  LwkDXmIsXUIWMRunning(widget)
Widget widget;

/*
**++
**  Functional Description:
**
**	Returns true if the DECwindows XUI window manager
**	is running.
**
**  Keywords:
**	Nonme
**
**  Arguments:
**	widget : widget id 
**
**  Result:
**	Void
**
**  Exceptions:
**	None
**--
*/
{
typedef unsigned long int   INT32;
typedef struct {
	INT32 title_font;
	INT32 icon_font;
	INT32 border_width;
	INT32 title_height;
	INT32 non_title_width;
	INT32 icon_name_width;
	INT32 iconify_width;
	INT32 iconify_height;
} internalDecorationGeometryRec, *internalDecorationGeometry;

#define WmNumDecorationGeometryElements (sizeof(internalDecorationGeometryRec)/4)

    static int first = True;
    static int result;
    Screen *scrn = XtScreen(widget);
    Display *dpy = XtDisplay(widget);

    /*
    ** Check for an undocumented property name to find out if the
    ** XUI WM has been run on this server.  Of course, this test doesn't
    ** tell you if the window manager is still running - So, there is
    ** room for improvment here.
    */
    
    if (first) {       
	Atom dec_wm;

	/*		       
	** Assume XUI WM is not running, until proven otherwise.
	*/

	result = False;

	dec_wm = XmInternAtom(dpy, "DEC_WM_DECORATION_GEOMETRY", True);

	if (dec_wm != None) {

	    internalDecorationGeometry prop = 0;
	    Atom actual_type;
	    int actual_format;
	    unsigned long leftover;
	    unsigned long nitems;
	    
    	    /*
            ** Check to see a property with the given name exists.
	    ** The XUI WM is the only client we know
	    ** about that sets a property with this name.  Therefore,
	    ** if the property exists, we assume the XUI WM is running.
	    */

	    XGetWindowProperty(dpy, RootWindowOfScreen(scrn),
		dec_wm, 0L, (long)WmNumDecorationGeometryElements,
		False, dec_wm, &actual_type, &actual_format,
		&nitems, &leftover, (unsigned char **)&prop);

	    if ((actual_type != dec_wm) ||
		(nitems < WmNumDecorationGeometryElements) ||
		(actual_format != 32)) 
		result = False;
	    else
		result = True;
		
	    if (prop != 0) XFree ((char *)prop);

	    }

	first = False;
    }

    return(result);
}


void  LwkDXmSetIconsOnShell(shell, private, event, continue_to_dispatch)
Widget shell;
 _DXmUiPrivate private;

    XEvent *event;
 Boolean *continue_to_dispatch;

/*
**++
**  Functional Description:
**  
**      Callback routine which sets the icon pixmaps for Reparenting window
**	managers.
**
**  Keywords:
**      XUI, WindowManager
**
**  Arguments:
**      {@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**      {@return-value-list-or-none@}
**
**  Exceptions:
**      {@identifier-list-or-none@}
**--
*/
{
    Display 	    *dpy = XtDisplay(shell);
    Window  	    root_window = XDefaultRootWindow(dpy);
    XReparentEvent  *reparent = (XReparentEvent *) &event->xreparent;

    if (event->type != ReparentNotify)
    	return;

    /* Ignore reparents back to the root window.
     */
    if (reparent->parent == root_window)
    	return;

    /*  Set the icons for this shell.
     */
    SetIcons(shell, private);

    return;
}


static void  SetIcons(shell, private)
Widget shell;
 _DXmUiPrivate private;

/*
**++
**  Functional Description:
**  
**      Sets the icon and iconify pixmaps for the given shell widget.
**
**  Keywords:
**      XUI, WindowManager
**
**  Arguments:
**      {@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**      {@return-value-list-or-none@}
**
**  Exceptions:
**      {@identifier-list-or-none@}
**--
*/
{
    _Integer		status;
    Dimension		width;
    Dimension		height;
    Display 	    	*dpy = XtDisplay(shell);
    Screen  	    	*scr = XtScreen(shell);
    unsigned int    	icon_size;
    char	    	*icon_name;
    static char     	*shell_icon_sizes[] = { "75", "50", "32", "17" };
    static int	    	num_sizes = XtNumber(shell_icon_sizes);

    /*
    ** Determine the icon pixmap name and size to fetch.
    */
    icon_name = GetIconIndexName(dpy, _MrmDBoxIcon, &icon_size, 
    	    	    	    	 shell_icon_sizes, num_sizes);
    if (icon_name != NULL) {

	/*
	** If the icon sizes are different we need to free the current ones, and
	** re-fetch new icons.  We assume that re-fetching new icons is an
	** infrequent operation, so we don't cache the old icons.
	*/

    	if ((private->dbox_icon_size != 0)	    	/* Icon exists.     */
    	     && (private->dbox_icon_size != icon_size))	/* New icon needed. */
    	{
    	    if (private->dbox_icon)
    	    	XFreePixmap(dpy, private->dbox_icon);
    	    if ((private->dbox_iconify_pixmap)
		&& (private->dbox_iconify_pixmap != private->dbox_icon))
    	    	XFreePixmap(dpy, private->dbox_iconify_pixmap);
    	    private->dbox_icon = (Pixmap) 0;
    	    private->dbox_iconify_pixmap = (Pixmap) 0;
    	    private->dbox_icon_size = (int) 0;
    	}
	
    	if (private->dbox_icon_size == 0 )
    	{
    	    private->dbox_icon_size = icon_size;
	    FetchBitmap(shell, private, icon_name, &private->dbox_icon);
    	}
	
    	XtFree(icon_name);
    	icon_name = NULL;
    }
    
    else    /* Can't get icon sizes for some reason */
    	return;

    /*
    ** Fetch the iconify pixmap for compatibility with the XUI window manager.
    */

    if (LwkDXmIsXUIWMRunning(shell))
    {
    	if (icon_size == 17 )  	    /* Don't fetch icon twice */
    	    private->dbox_iconify_pixmap = private->dbox_icon;
    	else if (icon_size > 17) {
	    icon_size = 17;
	    icon_name = (char *) XtMalloc(strlen(_MrmDBoxIcon)	+
    	    	    	    	    	  sizeof("_")		+
    	    	    	    	          sizeof("17")		+
    	    	    	    	    	  1 );    /* for \0 char */
	    strcpy( icon_name, _MrmDBoxIcon);
	    strcat( icon_name, "_");
	    strcat( icon_name, "17");

	    FetchBitmap(shell, private, icon_name, &private->dbox_iconify_pixmap);

	    XtFree(icon_name);
	    }
    }

    /*
    ** Set the iconify pixmap for the XUI window manager 
    */

    if (private->dbox_iconify_pixmap)
    	XUIWMSetIconifyPixmap(shell, private->dbox_iconify_pixmap);

    /*
    ** Set the icon pixmap on the shell.
    */

    if (private->dbox_icon)
    {
    	if (XtWindow(shell) != 0)
    	{
    	    /* HACK: Under Motif 1.1 changing iconPixmap will cause the window 
    	    *  	 to go to its initial state.  This appears to be a side-effect 
    	    *  	 of ICCCM-compliant behavior, and doing XtSetValues in the
    	    *  	 X toolkit, so we need to call Xlib directly instead of 
    	    *  	 setting XtNiconPixmap. 
    	    */
    	    XWMHints    *wmhints = NULL;

    	    wmhints = XGetWMHints(dpy, XtWindow(shell));
    	    if (wmhints != NULL)
    	    {
	    	wmhints->flags &= ~StateHint;
    	    	wmhints->flags |= IconPixmapHint;
    	    	wmhints->icon_pixmap = private->dbox_icon;
    	    	XSetWMHints(dpy, XtWindow(shell), wmhints);
    	    	XFree(wmhints);
    	    } 
    	    else
    	    {
	    	wmhints = (XWMHints *) XtCalloc(1, sizeof(XWMHints));
	    	wmhints->flags &= ~StateHint;
    	    	wmhints->flags |= IconPixmapHint;
    	    	wmhints->icon_pixmap = private->dbox_icon;
    	    	XSetWMHints(dpy, XtWindow(shell), wmhints);
	    	XtFree((_Void *) wmhints);
    	    }
    	}
    	else
    	{
    	    Arg	arglist[1];
    	    XtSetArg(arglist[0], XmNiconPixmap, private->dbox_icon);
    	    XtSetValues(shell, arglist, 1);
    	}
    }

    return;
}


static _String  GetIconIndexName(dpy, root_icon_literal, icon_size_rtn, supported_icon_sizes, num_supported_sizes)
Display *dpy;
 _String root_icon_literal;

    unsigned int *icon_size_rtn;
 char **supported_icon_sizes;

    int num_supported_sizes;

/*
**++
**  Functional Description:
**  
**      Finds the largest icon supported by the window manager and returns a
**	string which represents that icon in UIL.
**
**  Keywords:
**      XUI, WindowManager
**
**  Arguments:
**      {@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**      {@return-value-list-or-none@}
**
**  Exceptions:
**      {@identifier-list-or-none@}
**--
*/
{
    XIconSize	*size_list;
    int	    	num_sizes;
    int	    	cursize;
    int		i;
    char    	*icon_index = NULL;
    int	    	icon_size;
    char    	*icon_size_ptr;

    *icon_size_rtn = (int) 0;	    /* Initial value */
    icon_size = (int) 0;
    
    if (XGetIconSizes(dpy, XDefaultRootWindow(dpy), &size_list, &num_sizes)) {

	/*
	** Find the largest icon supported by the window manager.
	*/
	cursize = 0;
	
	for (i = 1; i < num_sizes; i++) {
	    if ((size_list[i].max_width >= size_list[cursize].max_width)
		  && (size_list[i].max_height >= size_list[cursize].max_height))
		cursize = i;
	}
	
	if ((size_list[cursize].max_width <= 0) 
	     || (size_list[cursize].max_height <= 0))
	{
	    XFree(size_list);
	    return (NULL);
	}

        /*
	** Find the largest icon we support.
	*/
	for (i = 0; i < num_supported_sizes; i++) {

	    icon_size = atoi(supported_icon_sizes[i]);
	    
	    if ((icon_size <= size_list[cursize].max_width)
		  && (icon_size <= size_list[cursize].max_height)) {

		icon_size_ptr = supported_icon_sizes[i];
		break;
	    }
	}
	XFree(size_list);
    }

    else {
        /*
	** Default to 32x32 icon, because it works for both XUI and Motif window
	** manager.
	*/

	icon_size = 32;
	icon_size_ptr = "32";
    }
	
	
    /*
    ** Build the icon index name
    **	    format: root_icon_literal + "_" + icon_size_ptr
    */

    if (icon_size > 0) {

    	icon_index = (char *) XtMalloc(strlen(root_icon_literal) +
    	    	    	    	       sizeof("_")	    	    +
    	    	    	    	       sizeof(icon_size_ptr)	    +
    	    	    	    	       1 );    /* for \0 char */
    	strcpy(icon_index, root_icon_literal);
    	strcat(icon_index, "_");
    	strcat(icon_index, icon_size_ptr);

    	*icon_size_rtn = (unsigned int) icon_size;
    }

    return(icon_index);

}  


static void  FetchBitmap(shell, private, icon_literal, icon)
Widget shell;
 _DXmUiPrivate private;

    _String icon_literal;
 Pixmap *icon;

/*
**++
**  Functional Description:
**	{@description@}
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
    _Integer	    status;
    Dimension	    width;
    Dimension	    height;
    MrmHierarchy    hierarchy;

    /*
    ** Open the DRM Hierarchy
    */
    hierarchy = LwkDXmOpenDRMHierarchy(private);
    
    /*
    ** Fetch the bitmap.
    */

    *icon = (Pixmap) 0;

    status = MrmFetchBitmapLiteral(hierarchy, icon_literal,
	    XtScreen(shell), XtDisplay(shell),
	    icon, &width, &height);
	    
    if (status != MrmSUCCESS)
	_Raise(drm_fetch_error);

    /*
    ** Close the DRM hierarchy.
    */

    LwkDXmCloseDRMHierarchy(hierarchy);

    return;
    }



static int  asn_1_str_len (asn_1_str)
unsigned char	*asn_1_str;

/*
**++
**  Functional Description:
**	This routine has been coded by Mark Bramhall, CDA architecture, to find
**	the encoding's length (actually, the allocation size) of an ASN.1
**	encoded string (which includes DDIF, DTIF, DOTS, LinkWorks's strings,
**	etc.).
**	asn_1_str_len takes a single parameter which is the address of an
**	ASN.1 (Abstract Syntax Notation 1, ISO 8824) string encoded according
**	to the BER (Basic Encoding Rules, ISO 8825) rules and returns the
**	string's "length." That is, the string's allocation size.
**	This routines assumes that the outermost TLV (Tag Length Value) is
**	a constructor surrounding the entire ASN.1 string [else the end is
**	impossible to find without detailed knowledge of the specific grammar]
**	and that the string is a well formed string [no error checking is
**	performed].
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
    unsigned char	*ptr;
    int			lvl;
    int			byt;
    int			len;

    /*
    * Get a working pointer.
    * Set nesting level to zero.
    */
    ptr = asn_1_str;
    lvl = 0;

    /*
    * Start of processing loop...
    */
loop:

    /*
    * Get the next tag.
    * Check for EOC (End Of Constructor).
    */
    byt = *ptr++;
    if (byt)
	{
	/*
	* Not an EOC.
	*
	* Check for an extended id.
	*/
	if (!((~byt) & TAG_ID_EXTENDED))
	    {
	    /*
	    * Pick up all of the extended id.
	    */
	    while (*ptr++ & TAG_ID_MORE)
		;
	    }

	/*
	* Get the length.
	* Check for an extended length.
	*/
	len = *ptr++;
	if (len & TAG_LEN_EXTENDED)
	    {
	    /*
	    * Process extended length.
	    *
	    * Find byte count.
	    * Check for indefinite.
	    */
	    byt = len & TAG_LEN_COUNT;
	    if (!byt)
		{
		/*
		* Indefinite length.
		*
		* Increment the nesting level and loop...
		*/
		lvl++;
		goto loop;
		}

	    /*
	    * Pick up the extended length.
	    */
	    len = 0;
	    while (--byt >= 0)
		{
		len <<= 8;
		len |= *ptr++;
		}
	    }

	/*
	* Bump working pointer over that length.
	*/
	ptr += len;

	/*
	* If we're into nesting then loop...
	*/
	if (lvl)
	    {
	    goto loop;
	    }
	}
    else
	{
	/*
	* EOC.
	*
	* Skip over the length byte.
	*/
	ptr++;

	/*
	* Decrement the nesting level.
	* If still into nesting then loop...
	*/
	if (--lvl > 0)
	    {
	    goto loop;
	    }
	}

    /*
    * Return the allocation size.
    */
    return ((int)ptr) - ((int)asn_1_str);
}


