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
** COPYRIGHT (c) 1989, 1990, 1991, 1992 BY
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
**	DXm general support routines (some are imported from LWK Services)
**
**  Keywords:
**	LWM, UI
**
**  Environment:
**	{@environment description@}
**
**  Author:
**	Patricia Avigdor
**
**  Creation Date: 13-Nov-89
**
**  Modification History:
**  
**	X0.7-1  Pat	21-Nov-89   add HS$$DWGetStringLiteral
**--
*/

/*
**  Include Files
*/

#include "hs_include.h"
#include "hs_abstract_objects.h"
#include "hs_decwindows.h"

#if defined (__osf__)
#define DXmCvtStatusOK 1
#define DXmCvtStatusFail 3
#else
#include <DXm/DECspecific.h>
#endif

#ifdef VMS
#include <decw$cursor.h>
#else
#include <X11/decwcursor.h>
#endif

/*
**  Table of Contents
*/

/*
**  Macro Definitions
*/

/*
**  Offsets used for position widget
*/

#define _Xoffset    75
#define _Yoffset    75

/*
** Constants needed for finding the length of a ASN.1 encoded string.
*/

/* Define the extended id. */
#define TAG_ID_EXTENDED       0x1F
#define TAG_ID_MORE           0x80

/* Define the extended length. */
#define TAG_LEN_COUNT         0x7F
#define TAG_LEN_EXTENDED      0x80

/*
**  Static Data Definitions
*/

static Cursor WaitCursor;
static Cursor InactiveCursor;

static MrmHierarchy MainHierarchy = (MrmHierarchy) 0;

/*
**  Global Data Definitions
*/


/*                                                                        
**  External Data Declarations
*/

/*
**  Forward Routine Declarations	
*/
_DeclareFunction(static void EnvDWI18NInvertString,
    (char *str));
_DeclareFunction(static int asn_1_str_len,
    (unsigned char *asn_1_str));


_CString  EnvDWCopyCString(cstring)
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


_DDIFString  EnvDWCopyDDIFString(ddif_string)
_DDIFString ddif_string;

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
    long	len;
    _DDIFString copy;

    if (ddif_string == (_DDIFString) _NullObject)
	copy = ddif_string;
    else {
    
	len = _LengthDDIFString(ddif_string);

	copy = (_DDIFString) _AllocateMem(len);

	_CopyMem((_Void *) ddif_string, (_Void *) copy, len);
    }

    return copy;
    }


_CString  EnvDWConcatCString(cstring1, cstring2)
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
    XmString xtstring;
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

	    xtstring = XmStringConcat((XmString) cstring1,
		(XmString) cstring2);

	    cstring = _CopyCString((_CString) xtstring);

	    XmStringFree(xtstring);
	}
    }

    return cstring;
    }


_Integer  EnvDWLengthCString(cstring)
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


_Integer  EnvDWLengthDDIFString(ddif_string)
_DDIFString ddif_string;

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

    if (ddif_string != (_DDIFString) _NullObject) 
	/*								    
	**  This is not an official routine but it works.
	**  We will use a DDIF lib routine when it becomes available.
	*/

	byte_cnt = asn_1_str_len(ddif_string);

    return (_Integer) byte_cnt;
    }


_Boolean  EnvDWIsEmptyCString(cstring)
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
	return (_False);
    else
	return (_Boolean) XmStringEmpty((XmString) cstring);
    }


_Void  EnvDWDeleteCString(cstring)
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


_Void  EnvDWDeleteDDIFString(ddifstring)
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


_String  EnvDWCStringToString(cstring)
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


_CString  EnvDWStringToCString(string)
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
	xmstring = _StringToXmString((char *)string);

	cstring = _CopyCString((_CString) xmstring);

	XmStringFree(xmstring);
    }

    return cstring;
    }


XmString  EnvDWStringToXmString(string)
char *string;

/*
**++
**  Functional Description:
**	Convert a File Code string into a toolkit compound string.
**	The XmString returned should be freed with XmStringFree.
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

                                               
    /** WARNING:  This should raise an inv_string error - when that	    */
    /*  exception code exists						    */

    if (status != DXmCvtStatusOK)
	_Raise(inv_comp_string);


    return xmstring;
    }
    

_DDIFString  EnvDWCStringToDDIFString(cstring)
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
    _DDIFString	    ddif_string;
    long	    status;
    long	    byte_cnt;
    Opaque	    xm_ddif_string;

    _StartExceptionBlock

    ddif_string = (_DDIFString) _NullObject;
    xm_ddif_string = (Opaque) _NullObject;

    if (cstring != _NullObject) {
    
	xm_ddif_string = (Opaque) DXmCvtCStoDDIF((XmString) cstring, &byte_cnt,
	    &status);

	if (status != DXmCvtStatusOK)
	    _Raise(inv_comp_string);

	ddif_string = _CopyDDIFString((_DDIFString) xm_ddif_string);

	XtFree(xm_ddif_string);
    }

    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
	_WhenOthers

	    if (xm_ddif_string != (Opaque) 0)
		XtFree(xm_ddif_string);
	    _Reraise;
	
    _EndExceptionBlock

    return ddif_string;
    }


_DDIFString  EnvDWStringToDDIFString(string)
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
    _DDIFString	    ddif_string;
    _CString 	    cstring;
    long	    status;
    long	    byte_cnt;
    Opaque	    xm_ddif_string;

    _StartExceptionBlock

    cstring = _StringToCString(string);

    ddif_string = (_DDIFString) _NullObject;
    xm_ddif_string = (Opaque) _NullObject;

    if (cstring != _NullObject) {
    
	xm_ddif_string = (Opaque) DXmCvtCStoDDIF((XmString) cstring, &byte_cnt,
	    &status);

	if (status != DXmCvtStatusOK)
	    _Raise(inv_comp_string);

	ddif_string = _CopyDDIFString((_DDIFString) xm_ddif_string);

	XtFree(xm_ddif_string);
        _DeleteCString(&cstring);
    }

    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
	_WhenOthers

	    _DeleteCString(&cstring);
	    if (xm_ddif_string != (Opaque) 0)
		XtFree(xm_ddif_string);	
	    _Reraise;
	
    _EndExceptionBlock

    return ddif_string;
    }


_CString  EnvDWDDIFStringToCString(ddifstring)
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

    if (ddifstring == _NullObject)
	cstring = _NullObject;
	
    else {
    
	xm_string =  (XmString) DXmCvtDDIFtoCS ((Opaque) ddifstring, &byte_cnt,
	    &status);

	if (status != DXmCvtStatusOK)
	    _Raise(inv_comp_string);

	cstring = _CopyCString((_CString) xm_string);

	XmStringFree(xm_string);
    }
	
    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
	_WhenOthers

	    if (xm_string == (XmString) 0)
		XmStringFree(xm_string);	
	    _Reraise;
	
    _EndExceptionBlock

    return cstring;
    }


_String  EnvDWDDIFStringToString(ddifstring)
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
    _String	string;
    long	byte_cnt;
    long	status;
    XmString 	xm_string;

    if (ddifstring == _NullObject)
	string = _NullObject;
	
    else {

	xm_string =  (XmString) DXmCvtDDIFtoCS((Opaque) ddifstring, &byte_cnt,
	    &status);

	if (status != DXmCvtStatusOK)
	    _Raise(inv_comp_string);

	string = _CStringToString((_CString) xm_string);

	XmStringFree(xm_string);
    }
	
    return string;
    }


_Integer  EnvDWCompareCString(cstring1, cstring2)
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
    _Integer answer;
    _String string1;
    _String string2;

    string1 = _CStringToString(cstring1);
    string2 = _CStringToString(cstring2);

    answer = _CompareString(string1, string2);

    _DeleteString(&string1);
    _DeleteString(&string2);

    return answer;
    }


_Boolean  EnvDWContainsCString(cstring1, cstring2)
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
    _Integer answer;
    _String string1;
    _String string2;

    string1 = _CStringToString(cstring1);
    string2 = _CStringToString(cstring2);

    answer = _ContainsString(string1, string2);

    _DeleteString(&string1);
    _DeleteString(&string2);

    return answer;
    }


_Boolean  EnvDWIsValidCString(cstring)
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
    if (_LengthCString(cstring) > 0)
	return _True;
    else
	return _False;
    }


Pixmap  EnvDWCreatePixmap(w, bits, width, height)
Widget w;
 unsigned short *bits;
 int width;

    int height;

/*
**++
**  Functional Description:
**	Create a pixmap.
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	w: widget in which to create the pixmap.
**	pixmap_bits: array of bits composing the pixmap.
**	pixmap_width: width of the pixmap.
**	pixmap_height: height of the pixmap.
**
**  Result:
**	Pixmap
**
**  Exceptions:
**	None
**--
*/
    {
    GC		gc;
    int		ac;
    Screen	*screen;
    Display	*display;
    XImage	image;
    Pixmap	pixmap;
    XGCValues	values;
    Pixel	foreground;
    Pixel	background;
    Arg		arglist[2];

    /*
    ** Get the current screen and current display.
    */

    screen = XtScreen(w);
    display = XtDisplay(w);

    /*
    ** Get the current foreground and background colors.
    */

    ac = 0;

    XtSetArg(arglist[ac], XmNforeground, &foreground); ac++;
    XtSetArg(arglist[ac], XmNbackground, &background); ac++;

    XtGetValues(w, arglist, ac);

    /*
    ** Initialize the image record.
    */

    image.height           = height;
    image.width            = width;
    image.xoffset          = 0;
    image.format           = XYBitmap;
    image.data             = (char*) bits;
    image.byte_order       = LSBFirst;
    image.bitmap_pad       = 8;
    image.bitmap_bit_order = LSBFirst;
    image.bitmap_unit      = 8;
    image.depth            = 1;
    image.bytes_per_line   = (width + 7) / 8;
    image.obdata           = NULL;

    /*
    ** Get a chunk of off-screen display = create an empty pixmap.
    */

    pixmap = XCreatePixmap(display, RootWindowOfScreen(screen),
	width, height, (unsigned) DefaultDepthOfScreen(screen));

    /*
    ** Initialize the graphic context.
    */

    values.foreground = foreground;
    values.background = background;
    values.fill_style = FillTiled;
    values.tile       = pixmap;

    /*
    ** Create the graphic context.
    */

    gc = XCreateGC(display, RootWindowOfScreen(screen),
	GCForeground | GCBackground | GCFillStyle | GCTile, &values);


    /*
    ** Put bits into the pixmap.
    */

    XPutImage(display, pixmap, gc, &image, 0, 0, 0, 0, width, height);

    /*
    ** Delete the graphic context.
    */

    XFreeGC(display, gc);

    return pixmap;
    }

                                        
_Void  EnvDWGetStringLiteral(identifier, literal)
_String identifier;
 _String *literal;

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
    MrmCode code;
    char *string;

    /*
    ** Fetch the value of the identifier.
    */

    string = (char *) 0;

    status = MrmFetchLiteral(MainHierarchy, identifier, (Display *) 0,
	(caddr_t *) &string, &code);
	
    if (status != MrmSUCCESS)
	*literal = (_String) 0;
    else
	*literal = (_String) _CopyString(string);

    return;
    }


_Void  EnvDWGetCStringLiteral(identifier, literal)
_String identifier;
 _CString *literal;

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
    MrmCode code;
    XmString cstring;

    /*
    ** Fetch the value of the identifier.
    */

    cstring = (XmString) 0;

    status = MrmFetchLiteral(MainHierarchy, identifier, (Display *) 0,
	(caddr_t *) &cstring, &code);
	
    if (status != MrmSUCCESS)
	*literal = (_CString) 0;
    else
	*literal = (_CString) _CopyCString((_CString) cstring);

    return;
    }

                                        
_Void  EnvDWGetKeySym(identifier, key)
_String identifier;
 KeySym *key;

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
    MrmCode code;

    /*
    ** Fetch the value of the identifier.
    */
    status = MrmFetchLiteral(MainHierarchy, identifier, (Display *) 0,
	(caddr_t *) key, &code);
	
    if (status != MrmSUCCESS)
	*key = (KeySym) 0;

    return;
    }


_Void  EnvDWGetIntegerValue(identifier, value)
_String identifier;
 _Integer *value;

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
    int		    status;
    int		    *int_ptr;
    MrmCode	    code;

    /*
    ** Fetch the value of the identifier
    */

    status = MrmFetchLiteral(MainHierarchy, identifier, (Display *) 0,
	(caddr_t *) &int_ptr, &code);
	
    if (status != MrmSUCCESS)
	*value = (_Integer) 0;
    else
	*value = *int_ptr;

    return;
    }


_Void  EnvDWFetchIcon(private, icon_literal, icon, BW)
_WindowPrivate private;
 _String icon_literal;
 Pixmap *icon;

    _Boolean BW;

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
    _Integer	    count = 0;
    Arg		    arglist[5];
    Pixel	    foreground;
    Pixel	    background;
    Widget	    shell;

    /*
    ** Fetch the value of the identifier.
    */

    *icon = (Pixmap) 0;
    shell = private->shell;

    if (BW)

	status = MrmFetchIconLiteral(MainHierarchy, icon_literal,
	    XtScreen(shell), XtDisplay(private->shell),
	    BlackPixelOfScreen(XtScreen(private->shell)),
	    WhitePixelOfScreen(XtScreen(private->shell)), icon);

    else {

        XtSetArg(arglist[count], XmNforeground, &foreground); count++;
        XtSetArg(arglist[count], XmNbackground, &background); count++;

	XtGetValues(private->main_widget, arglist, count);

	status = MrmFetchIconLiteral(MainHierarchy, icon_literal,
	    XtScreen(private->shell), XtDisplay(private->shell), foreground,
	    background, icon);
    }
           
    if (status != MrmSUCCESS)
	_Raise(drm_fetch_error);

    return;
    }


_Void  EnvDWFetchBitmap(shell, icon_literal, icon)
Widget shell;
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

    /*
    ** Fetch the bitmap.
    */

    *icon = (Pixmap) 0;

    status = MrmFetchBitmapLiteral(MainHierarchy, icon_literal,
	    XtScreen(shell), XtDisplay(shell),
	    icon, &width, &height);
	    
    if (status != MrmSUCCESS)
	_Raise(drm_fetch_error);

    return;
    }


MrmHierarchy  EnvDWOpenDRMHierarchy(filename)
_String filename;

/*
**++
**  Functional Description:
**      {@description@}
**
**  Keywords:
**      {@keyword-list-or-none@}
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
    int		    status;
    MrmHierarchy    hierarchy;

    /*
    ** Open the DRM hierarchy.
    */

    status = MrmOpenHierarchy((MrmCount) 1, &filename, (MrmOsOpenParamPtr *) 0,
        &hierarchy);

    if (status != MrmSUCCESS)
        _Raise(drm_open_error);

    return hierarchy;
    }


_Void  EnvDwOpenMainHierarchy()
/*
**++
**  Functional Description:
**      {@description@}
**
**  Keywords:
**      {@keyword-list-or-none@}
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
    int		status;
    _String	filename;
    
    /*
    ** Open the general message file hierarchy 
    */

    filename = _LinkWorksMgrUidFileName;

    status = MrmOpenHierarchy((MrmCount) 1, &filename, (MrmOsOpenParamPtr *) 0,
        &MainHierarchy);

    if (status != MrmSUCCESS)
        _Raise(drm_open_error);

    return;
    }


MrmHierarchy  EnvDwGetMainHierarchy()
/*
**++
**  Functional Description:
**      {@description@}
**
**  Keywords:
**      {@keyword-list-or-none@}
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
    if (MainHierarchy == (MrmHierarchy) 0)
	_Raise(drm_open_error);

    return (MainHierarchy);
    }


_Void  EnvDWCloseDRMHierarchy(hierarchy)
MrmHierarchy hierarchy;

/*
**++
**  Functional Description:
**      {@description@}
**
**  Keywords:
**      {@keyword-list-or-none@}
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
    /*
    ** Close the DRM hierarchy.
    */

    if (MrmCloseHierarchy(hierarchy) != MrmSUCCESS)
        _Raise(drm_close_error);

    return;
    }


_Void  EnvDWRegisterDRMNames(private)
_WindowPrivate private;

/*
**++
**  Functional Description:
**      {@description@}
**
**  Keywords:
**      {@keyword-list-or-none@}
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
    MrmRegisterArg  drm_register[1];

    /*
    ** Register PrivateData name with DRM.
    */

    drm_register[0].name = _DrmPrivateIdentifier;
    drm_register[0].value = (caddr_t) private;

    MrmRegisterNames(drm_register, (MrmCount) 1);
    }


_CString  EnvDWStatusToCString(code)
_Status code;

/*
**++
**  Functional Description:
**      {@description@}
**
**  Keywords:
**      {@keyword-list-or-none@}
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
    int		    status;
    MrmCode	    type;
    _String	    string;
    _CString	    cstring;
    XmString   xstring;

    /*
    **  Convert the status code into a DRM value identifier
    */

    string = _StatusToResource(code);

    /*
    **  Fetch the value of the identifier
    */

    xstring = (XmString) 0;

    status = MrmFetchLiteral(MainHierarchy, string, (Display *) 0,
	(caddr_t *) &xstring, &type);

    /*
    **  If not found, try Unknown Status
    */

    if (status != MrmSUCCESS || xstring == (XmString) 0) {
	string = _StatusToResource(_StatusCode(unknown));

	status = MrmFetchLiteral(MainHierarchy, string, (Display *) 0,
	    (caddr_t *) &xstring, &type);

	if (status != MrmSUCCESS)
	    xstring = (XmString) 0;
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


_Void  EnvDwCreateCursors(toplevel)
Widget toplevel;

/*
**++
**  Functional Description:
**      {@description@}
**
**  Keywords:
**      {@keyword-list-or-none@}
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

    /*
    **  Load the Wait Cursor
    */

    WaitCursor = DXmCreateCursor(toplevel, decw$c_wait_cursor);

    /*
    **  Load the Inactive Cursor
    */

    InactiveCursor = DXmCreateCursor(toplevel, decw$c_inactive_cursor);

    return;
    }


_Void  EnvDwSetCursor(widget, cursor_type)
Widget widget;
 _CursorType cursor_type;

/*
**++
**  Functional Description:
**      {@description@}
**
**  Keywords:
**      {@keyword-list-or-none@}
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

    switch (cursor_type) {
    	case _WaitCursor : {
	    XDefineCursor(XtDisplay(widget), XtWindow(widget), WaitCursor);
	    break;
	}
    	case _InactiveCursor : {
	    XDefineCursor(XtDisplay(widget), XtWindow(widget), InactiveCursor);
	    break;
	}
	case _DefaultCursor : {
	    XUndefineCursor(XtDisplay(widget), XtWindow(widget));
	    break;
	}
    	default:
	    break;
    }

    XFlush(XtDisplay(widget));

    return;
    }


_Void  EnvDWGetTime(reason, timestamp)
_Reason reason;
 Time *timestamp;

/*
**++
**  Functional Description:
**      {@description@}
**
**  Keywords:
**      {@keyword-list-or-none@}
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

    if (((XmAnyCallbackStruct *)reason)->event != 0)

	switch (((XmAnyCallbackStruct *)reason)->event->type) {
	
	    case KeyPress:
	
	    case KeyRelease:
		*timestamp = reason->event->xkey.time;
		break;
		
	    case ButtonPress:
	
	    case ButtonRelease:
		*timestamp = reason->event->xbutton.time;
		break;
		
	    default:
		_Raise(no_timestamp); /*no time stamp */
	}
    else

	*timestamp = CurrentTime;
	
    return;
    }


_Void  PositionWidgetOffset(parent, child)
Widget parent;
 Widget child;

/*
**++
**  Functional Description:
**      {@description@}
**
**  Keywords:
**      {@keyword-list-or-none@}
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
    Arg		arglist[2];
    Cardinal	x_value;
    Cardinal	y_value;

    XtSetArg(arglist[0], XmNx, &x_value);
    XtSetArg(arglist[1], XmNy, &y_value);
    XtGetValues(parent, arglist, 2);

    x_value += _Xoffset;
    y_value += _Yoffset;

    XtSetArg(arglist[0], XmNx, x_value);
    XtSetArg(arglist[1], XmNy, y_value);
    XtSetValues(child, arglist, 2);

    return;
    }


_CString  EnvDWUserNameToCString()
/*
**++
**  Functional Description:
**      {@description@}
**
**  Keywords:
**      {@keyword-list-or-none@}
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
    return _StringToCString((_String) cuserid(NULL));
    }


_Void  EnvDWWmSetIconifyPixmap(widget, pixmap)
Widget widget;
 Pixmap pixmap;

/*
**++
**  Functional Description:
**  
**      Set the Iconify pixmap for the XUI Window Manager if the
**	Motif window manager is not running.
**
**  Keywords:
**      XUI, WindowManager, IconifyPixmap
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
    unsigned long      	  leftover;
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


_Boolean  EnvDWIsXUIWMRunning(widget)
Widget widget;

/*
**++
**  Functional Description:
**  
**      Return True if XUI window manager is running.
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

    /* Check for an undocumented property name to find out if
    ** the XUI WM has been run on this server.  Of course, 
    ** this test doesn't tell you if the window manager is still
    ** running - So, there is room for improvment here.
    */
    
    if (first) {
	Atom dec_wm;

	/*		       
	** Assume XUI WM is not running, until proven otherwise.
	*/
	result = False;

	dec_wm = XmInternAtom(XtDisplay(widget), "DEC_WM_DECORATION_GEOMETRY",
	    True);

	if (dec_wm != None) {

	    internalDecorationGeometry prop = 0;
	    Atom actual_type;
	    int actual_format;
	    unsigned long leftover;
	    unsigned long nitems;
	    
    	    /*
            ** Check to see a property with the given name exists.
	    ** The XUI WM is the only client we know
	    ** about that sets a property with this atom name.  Therefore,
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


_String  EnvDWGetIconIndexName(dpy, root_icon_literal, icon_size_rtn, supported_icon_sizes, num_supported_sizes)
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
    	    	    	    	       sizeof("_")	    	 +
    	    	    	    	       sizeof(icon_size_ptr)	 +
    	    	    	    	       sizeof(char) );    /* for \0 char */
    	strcpy(icon_index, root_icon_literal);
    	strcat(icon_index, "_");
    	strcat(icon_index, icon_size_ptr);

    	*icon_size_rtn = (unsigned int) icon_size;
    }

    return(icon_index);

}  


_Void  EnvDWFitFormDialogOnScreen (formdialog)
Widget formdialog;

/*
**++
**  Functional Description:
**  
**  This routine fits a formdialog to the screen. The formdialog needs to have
**  been  set up in UIL with the following structure:
**	FormDialog
**	    |
**	ScrolledWindow (policy XmAUTOMATIC)
**	    |
**	Form (contains the "real" info)
**
**  Basically, this routine, gets the size of the form, sets the scrolledwindow
**  to the smaller of that size or the screen size for width and height.
**
**  This routine was lifted from the DECwindows Calendar code and modified
**  slightly to fit in here.
**  
**  Side Effects:
**	Plays around with the sizes of the children of the FormDialog passed in.
**
**  Keywords:
**      I14Y, PC
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
    int			ac;
    Dimension		margin_height,
			margin_width,
			scr_width,
			scr_height,
			height,
			width;
    CompositeWidget	scrolledwclip_widget;
    CompositeWidget	scrolledwindow_widget;
    CompositeWidget	form_widget;
    int			i;
    Arg			arglist[5];
    Display		*dpy = XtDisplay(formdialog);

#define	    DISTANCE_TO_EDGE_OF_SCREEN  20
#define	    SCROLLWINDOW_DRESSING	3

    /*	  
    **  Let's do some runtime twiddling. (get first child)
    */	  
    scrolledwindow_widget =
	    (CompositeWidget)
		(((CompositeWidget)formdialog)->composite.children[0]);
    if (XmIsScrolledWindow(scrolledwindow_widget))
    {
	/*	  
	**  Get the size of the scrolledwindows margins for latter figuring.
	*/
	ac = 0;	  
	XtSetArg(arglist[ac], XmNscrolledWindowMarginWidth, &margin_width);
	ac++;
	XtSetArg(arglist[ac], XmNscrolledWindowMarginHeight, &margin_height);
	ac++;
	XtGetValues((Widget) scrolledwindow_widget, arglist, ac);

	/*	  
	**  If the margins on the scrolled window are 0 then we better do
	**	something about it.
	*/	  
	if (margin_width == 0)
	    margin_width = SCROLLWINDOW_DRESSING;

	if (margin_height == 0)
	    margin_height = SCROLLWINDOW_DRESSING;
	
	/*	  
	**  This should be the ScrolledWindowClipWindow (first child)
	*/	  
	scrolledwclip_widget =
	    (CompositeWidget)
	    (((CompositeWidget)scrolledwindow_widget)->composite.children[0]);

	/*	  
	**  Let's find the form under the scrolled window clip window
	*/	  
	for( i=0; i < ((scrolledwclip_widget)->composite.num_children); i++)
	{
	    form_widget =
	    (CompositeWidget)((scrolledwclip_widget)->composite.children[i]);
	    if ( XmIsForm( form_widget))
	    {
		/*	  
		**	This is the form kid of the scrolledwindow. Find out
		**	how big it is.
		*/

		ac = 0;	  
		XtSetArg(arglist[ac], XmNwidth, &width);
		ac++;
		XtSetArg(arglist[ac], XmNheight, &height);
		ac++;
		XtGetValues((Widget) form_widget, arglist, ac);

		/*	  
		**	We want to make the scrolledwindow the right size. We
		**	add the scrollwindow margins so that we don't get the
		**	scrollbars by default since the scrollwindow will
		**	compare its size (minus thickness) to the form to
		**	decide whether or not to display the scrollbars. We
		**	want to make sure that we also fit on the screen (with
		**	some margin "DISTANCE_TO_EDGE_OF_SCREEN").
		*/	  
		scr_width = WidthOfScreen(XtScreen(formdialog));
		scr_height = HeightOfScreen(XtScreen(formdialog));

		width = _Min(width + (margin_width * 2),
		    scr_width - (DISTANCE_TO_EDGE_OF_SCREEN * 2));
		height = _Min(height + (margin_height * 2),
		    scr_height - (DISTANCE_TO_EDGE_OF_SCREEN * 2));

		ac = 0;	  
		XtSetArg(arglist[ac], XmNwidth, width);
		ac++;
		XtSetArg(arglist[ac], XmNheight, height);
		ac++;
		XtSetValues(formdialog, arglist, ac);

		break;  /* we've found what we were looking for */
	    }
	} /* end of the for */
    } /* end of scrolled window if */
}

/*
**++
**  ROUTINE NAME: EnvDWSetTitle
**
**  FUNCTIONAL DESCRIPTION:
**	This routine sets the window title.  If it contains iso-latin1 only,
**      the encoding will be STRING.  Otherwise, it will be COMPOUND_TEXT
**      encoding.
**
**  NOTE:
**  	
**  This code was provided by Japan R&D to improve I18N support.
**  This code is available in a library of routines on VMS.  When
**  and if the library is made available on other platforms, this
**  routine should be removed from the LinkWorks code, and replaced
**  by a call to the library.
**
**  FORMAL PARAMETERS:
**      widget   - (I) The widget in which the current window title is to be
**		       changed.
**      cs_title - (I) A compound string to be converted and be set as the
**		       window title
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/

_Void  EnvDWSetTitle(widget, cs_title)
Widget widget;
  _CString cs_title;

{
  char			*language;
  char			*title,*title_os=NULL;
  char			*encoding;
  long			byte_count;
  long			cvt_status;
  Arg			arglist[2];

  if (!cs_title)
      return;

  language = (char *)xnl_getlanguage();         /* Retrieve language	  */

  if (DXmCSContainsStringCharSet(cs_title))	/* See if ISO_Latin1 only */
    {
	encoding = "STRING";
	title = (char *) DXmCvtCStoFC(cs_title, &byte_count, &cvt_status);
	if (cvt_status == DXmCvtStatusFail)
	  {
		title = NULL;
	  }
    }						/* See if Hebrew	  */
  else if (language[0] == 'i' && language[1] == 'w')
    {						/* For Hebrew		  */
  	if (EnvDWIsXUIWMRunning(widget))
	    {
		encoding = "STRING";
		title = (char *) DXmCvtCStoFC(cs_title, &byte_count, &cvt_status);
		if (cvt_status == DXmCvtStatusFail)
		  {
		     title = NULL;
		  }
		else 
		  {
		     title_os = (char *) DXmCvtCStoOS(cs_title, &byte_count,
			&cvt_status);
		     if (title_os != NULL) XmStringFree((XmString) title_os);
		     if (cvt_status != DXmCvtStatusOK)
		  	{
			    EnvDWI18NInvertString(title);
		  	}
		   }
	    }
	else
	    {
		encoding = "COMPOUND_TEXT";
		title = (char *) XmCvtXmStringToCT((XmString) cs_title);
	    }
    }
  else					       /* For other locales	  */
    {
	encoding = "COMPOUND_TEXT";
	title = (char *) XmCvtXmStringToCT((XmString) cs_title);
    }	  

  if (title)
    {
	  XtSetArg(arglist[0], XmNtitle, title);
	  XtSetArg(arglist[1], XmNtitleEncoding,
		   XmInternAtom(XtDisplay(widget), encoding, FALSE));
	  XtSetValues (widget, arglist, 2);
	  XtFree((char *) title);
    }
}

/*
**++
**  ROUTINE NAME: EnvDWSetIconName
**
**  FUNCTIONAL DESCRIPTION:
**	This routine sets the icon name. If it contains iso-latin1 only,
**      the encoding will be STRING.  Otherwise, it will be COMPOUND_TEXT
**      encoding.
**	
**  NOTE:
**  	
**  This code was provided by Japan R&D to improve I18N support.
**  This code is available in a library of routines on VMS.  When
**  and if the library is made available on other platforms, this
**  routine should be removed from the LinkWorks code, and replaced
**  by a call to the library.
**
**  FORMAL PARAMETERS:
**      widget  - (I) The widget in which the current icon name is to be
**		      changed.
**      cs_icon - (I) A compound string to be converted and be set as the icon
**		      name
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
_Void  EnvDWSetIconName(widget, cs_icon)
Widget widget;
 _CString cs_icon;

{
  char			*language;
  char			*iconname,*icon_os=NULL;
  char			*encoding;
  long			byte_count;
  long			cvt_status;
  Arg			arglist[2];

  if (!cs_icon)
      return;

  language = (char *)xnl_getlanguage();         /* Retrieve language	  */

  if (DXmCSContainsStringCharSet(cs_icon))	/* See if ISO_Latin1 Only */
    {
	encoding = "STRING";
	iconname = (char *) DXmCvtCStoFC(cs_icon, &byte_count, &cvt_status);
	if (cvt_status == DXmCvtStatusFail)
	  {
		iconname = NULL;
	  }
    }						/* See if Hebrew	  */
  else if (language[0] == 'i' && language[1] == 'w')
    {						/* For Hebrew		  */
  	if (EnvDWIsXUIWMRunning(widget))
	    {
		encoding = "STRING";
		iconname = (char *) DXmCvtCStoFC(cs_icon, &byte_count,
		    &cvt_status);
		if (cvt_status == DXmCvtStatusFail)
		  {
		      iconname = NULL;
		  }
		else 
		  {
		      icon_os = (char *) DXmCvtCStoOS(cs_icon, &byte_count,
			&cvt_status);
		      if (icon_os != NULL) XmStringFree((XmString) icon_os);
		      if (cvt_status != DXmCvtStatusOK)
		  	{
			     EnvDWI18NInvertString(iconname);                   
		  	}
		  }
	    }
	else
	    {
		encoding = "COMPOUND_TEXT";
		iconname = (char *) XmCvtXmStringToCT((XmString) cs_icon);
	    }
    }
  else						/* For other locales	  */
    {
	encoding = "COMPOUND_TEXT";
	iconname = (char *) XmCvtXmStringToCT((XmString) cs_icon);
    }	  

  if (iconname)
    {
	  XtSetArg(arglist[0], XmNiconName, iconname);
	  XtSetArg(arglist[1], XmNiconNameEncoding,
		   XmInternAtom(XtDisplay(widget), encoding, FALSE));
	  XtSetValues (widget, arglist, 2);
	  XtFree((char *) iconname);
    }
}

/*
**++
**  ROUTINE NAME: EnvDWI18NInvertString
**
**  This code was provided by Japan R&D to improve I18N support.
**  This code is available in a library of routines on VMS.  When
**  and if the library is made available on other platforms, this
**  routine should be removed from the LinkWorks code, and replaced
**  by a call to the library.
**
**  FUNCTIONAL DESCRIPTION:
**	This routine inverts the string.
**
**  FORMAL PARAMETERS:
**      str 	- (I/O) A pointer to the string to be inverted
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/

static void  EnvDWI18NInvertString (str)
char *str;

{
  int	head, tail;
  char	buf;

  head = 0;
  tail = strlen(str) - 1;

  while (tail > head)
     {
      	buf = str[head];
	str[head] = str[tail];
	str[tail] = buf;
	head++;
	tail--;
     }
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




