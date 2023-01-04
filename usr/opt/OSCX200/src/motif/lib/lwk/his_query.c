/*
** COPYRIGHT (c) 1988 BY
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
**	HyperInformation Services
**
**  Version: X0.1
**
**  Abstract:
**	Support routines for Connection Network Query operation
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	Doug Rayner
**
**  Creation Date: 7-Jul-88
**
**  Modification History:
**--
*/


/*
**  Include Files
*/

#ifdef MSDOS
#include "include.h"
#include "abstobjs.h"
#else /* !MSDOS */
#include "his_include.h"
#include "lwk_abstract_objects.h"
#endif

/*
**  Macro Definitions
*/

/*
**  Type Definitions
*/

/*
**  Forward Routine Declarations
*/

_DeclareFunction(static _Boolean CompareStrings,
    (_Object object, lwk_query_operator operator,
	_QueryExpression operand_1, _QueryExpression operand_2));
_DeclareFunction(static _Boolean CompareCStrings,
    (_Object object, lwk_query_operator operator,
	_QueryExpression operand_1, _QueryExpression operand_2));
_DeclareFunction(static _Boolean CompareDate,
    (_Object object, lwk_query_operator operator,
	_QueryExpression operand_1, _QueryExpression operand_2));
_DeclareFunction(static _Boolean CompareIntegers,
    (_Object object, lwk_query_operator operator,
	_QueryExpression operand_1, _QueryExpression operand_2));
_DeclareFunction(static _Boolean CompareFloats,
    (_Object object, lwk_query_operator operator,
	_QueryExpression operand_1, _QueryExpression operand_2));
_DeclareFunction(static _Boolean CompareBooleans,
    (_Object object, lwk_query_operator operator,
	_QueryExpression operand_1, _QueryExpression operand_2));

/*
**  Static Data Definitions
*/

/*
**  Global Data Definitions
*/

/*
**  External Data Declarations
*/


static void  LwkQuery()
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
    return;
    }


_Boolean  LwkQuerySelectProperties(object, expression)
_Object object;
 _QueryExpression expression;

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
    _Boolean result;
    lwk_query_operator operator;
    _QueryExpression operand_1;
    _QueryExpression operand_2;

    /*
    **	Dispatch based on the operator in the root of the Selection
    **	sub-Expression.
    */
    
    if (expression == (_QueryExpression) _NullObject)
	_Raise(inv_query_expr);

    operator  = expression->lwk_operator;
    operand_1 = (_QueryExpression) expression->lwk_operand_1;
    operand_2 = (_QueryExpression) expression->lwk_operand_2;

    switch (expression->lwk_operator) {
	case lwk_c_and :
	    /*
	    **	True if both the left and right Selection sub-Expressions are
	    **	True.
	    */

	    result = _SelectProperties(object, operand_1)
		&& _SelectProperties(object, operand_2);

	    break;

	case lwk_c_or :
	    /*
	    **	True if either the left or right Selection sub-Expressions is
	    **	True.
	    */

	    result = _SelectProperties(object, operand_1)
		|| _SelectProperties(object, operand_2);

	    break;

	case lwk_c_not :
	    /*
	    **	True if the left Selection sub-Expressions is False.
	    */

	    result = !_SelectProperties(object, operand_1);

	    break;

	case lwk_c_string_is_eql :
	case lwk_c_string_is_neq :
	case lwk_c_string_contains :
	case lwk_c_string_contains_no :
	    /*
	    **  True if Strings compare as specified
	    */

	    result = CompareStrings(object, operator, operand_1, operand_2);

	    break;

	case lwk_c_ddif_string_is_eql :
	case lwk_c_ddif_string_is_neq :
	case lwk_c_ddif_string_contains :
	case lwk_c_ddif_string_contains_no :
	    /*
	    **  True if Compound Strings compare as specified
	    */

	    result = CompareCStrings(object, operator, operand_1, operand_2);

	    break;

	case lwk_c_date_is_eql :
	case lwk_c_date_is_neq :
	case lwk_c_date_is_lss :
	case lwk_c_date_is_gtr :
	case lwk_c_date_is_leq :
	case lwk_c_date_is_geq :
	    /*
	    **  True if Dates compare as specified
	    */

	    result = CompareDate(object, operator, operand_1, operand_2);

	    break;

	case lwk_c_integer_is_eql :
	case lwk_c_integer_is_neq :
	case lwk_c_integer_is_lss :
	case lwk_c_integer_is_gtr :
	case lwk_c_integer_is_leq :
	case lwk_c_integer_is_geq :
	    /*
	    **  True if Integers compare as specified
	    */

	    result = CompareIntegers(object, operator, operand_1, operand_2);

	    break;

	case lwk_c_float_is_eql :
	case lwk_c_float_is_neq :
	case lwk_c_float_is_lss :
	case lwk_c_float_is_gtr :
	case lwk_c_float_is_leq :
	case lwk_c_float_is_geq :
	    /*
	    **  True if Floats compare as specified
	    */

	    result = CompareFloats(object, operator, operand_1, operand_2);

	    break;

	case lwk_c_boolean_is_eql :
	case lwk_c_boolean_is_neq :
	case lwk_c_boolean_is_true :
	case lwk_c_boolean_is_false :
	    /*
	    **  True if Booleans compare as specified
	    */

	    result = CompareBooleans(object, operator, operand_1, operand_2);

	    break;

	default :
	    _Raise(inv_query_expr);
	    break;

    }

    return result;
    }


static _Boolean  CompareStrings(object, operator, operand_1, operand_2)
_Object object;
 lwk_query_operator operator;

    _QueryExpression operand_1;
 _QueryExpression operand_2;

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
    int i1, i2;
    _Boolean result;
    _Boolean multi1;
    _Boolean multi2;
    _Integer cnt1;
    _Integer cnt2;
    _Boolean volatile var1;
    _Boolean volatile var2;
    _String volatile str1;
    _String volatile str2;
    _List_of(_String) volatile list1;
    _List_of(_String) volatile list2;

    /*
    **  Initialize temporary variables
    */

    str1 = (_String) _NullObject;
    str2 = (_String) _NullObject;
    list1 = (_List) _NullObject;
    list2 = (_List) _NullObject;

    _StartExceptionBlock

    /*
    **  Get the first operand(s).
    */

    if (operand_1 == (_QueryExpression) _NullObject)
	_Raise(inv_query_expr);

    multi1 = _False;
    var1 = _True;
    cnt1 = 1;

    switch (operand_1->lwk_operator) {
	case lwk_c_string_literal :
	    str1 = (_String) operand_1->lwk_operand_1;
	    var1 = _False;
	    break;

	case lwk_c_property_value :
	    _GetValue(object, (_String) operand_1->lwk_operand_1,
		lwk_c_domain_string, (_AnyPtr) &str1);
	    break;

	case lwk_c_some_property_value :
	    list1 = (_List) _GetValueList(object,
		(_String) operand_1->lwk_operand_1);
	    _GetValue(list1, _P_ElementCount, lwk_c_domain_integer, &cnt1);
	    multi1 = _True;
	    break;

	default :
	    _Raise(inv_query_expr);
    }

    /*
    **  Get the second operand(s).
    */

    if (operand_2 == (_QueryExpression) _NullObject)
	_Raise(inv_query_expr);

    multi2 = _False;
    var2 = _True;
    cnt2 = 1;

    switch (operand_2->lwk_operator) {
	case lwk_c_string_literal :
	    str2 = (_String) operand_2->lwk_operand_1;
	    var2 = _False;
	    break;

	case lwk_c_property_value :
	    _GetValue(object, (_String) operand_2->lwk_operand_1,
		lwk_c_domain_string, (_AnyPtr) &str2);
	    break;

	case lwk_c_some_property_value :
	    list2 = (_List) _GetValueList(object,
		(_String) operand_2->lwk_operand_1);
	    _GetValue(list2, _P_ElementCount, lwk_c_domain_integer, &cnt2);
	    multi2 = _True;
	    break;

	default :
	    _Raise(inv_query_expr);
    }

    /*
    **  Compare the values
    */

    for (i1 = 0; i1 < cnt1; i1++) {
        for (i2 = 0; i2 < cnt2; i2++) {
	    if (multi1)
		_RemoveElement(list1, lwk_c_domain_string, (_AnyPtr) &str1);

	    if (multi2)
		_RemoveElement(list2, lwk_c_domain_string, (_AnyPtr) &str2);

	    if (str1 == (_String) _NullObject || str2 == (_String) _NullObject)
		result = _False;
	    else
		switch (operator) {
		    case lwk_c_string_is_eql :
			result =
			    (_CompareValue(&str1, &str2, lwk_c_domain_string) == 0);
			break;

		    case lwk_c_string_is_neq :
			result =
			    (_CompareValue(&str1, &str2, lwk_c_domain_string) != 0);
			break;

		    case lwk_c_string_contains :
			result = _ContainsString(str1, str2);
			break;

		    case lwk_c_string_contains_no :
			result = !_ContainsString(str1, str2);
			break;
		}

	    /*
	    **  Delete the strings if they where not Literals.
	    */

	    if (var1)
		_DeleteString(&str1);
	    if (var2)
		_DeleteString(&str2);

	    /*
	    **  If we had a successful comparison, we can quite now.
	    */

	    if (result)
		break;
	}

	if (result)
	    break;
    }

    /*
    **  Delete any value lists we may have created.
    */

    if (multi1)
	_Delete(&list1);

    if (multi2)
	_Delete(&list2);

    /*
    **	If there was an exception, clean up.  If it is one we can anticipate
    **	(e.g., NoSuchProperty, InvalidDomain), return False.  Otherwise,
    **	reraise the exception.
    */

    _Exceptions
	_WhenOthers
	    _Delete(&list1);
	    _Delete(&list2);

	    if (var1)
		_DeleteString(&str1);
	    if (var2)
		_DeleteString(&str2);

	    if (_Others == _StatusCode(inv_domain)
		    || _Others == _StatusCode(no_such_property))
		result = _False;
	    else
		_Reraise;

    _EndExceptionBlock

    return result;
    }


static _Boolean  CompareCStrings(object, operator, operand_1, operand_2)
_Object object;
 lwk_query_operator operator;

    _QueryExpression operand_1;
 _QueryExpression operand_2;

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
    int i1, i2;
    _Boolean result;
    _Boolean multi1;
    _Boolean multi2;
    _Integer cnt1;
    _Integer cnt2;
    _Boolean volatile var1;
    _Boolean volatile var2;
    _DDIFString volatile cstr1;
    _DDIFString volatile cstr2;
    _List_of(_DDIFString) volatile list1;
    _List_of(_DDIFString) volatile list2;

    /*
    **  Initialize temporary variables
    */

    cstr1 = (_DDIFString) _NullObject;
    cstr2 = (_DDIFString) _NullObject;
    list1 = (_List) _NullObject;
    list2 = (_List) _NullObject;

    _StartExceptionBlock

    /*
    **  Get the first operand(s).
    */

    if (operand_1 == (_QueryExpression) _NullObject)
	_Raise(inv_query_expr);

    multi1 = _False;
    var1 = _True;
    cnt1 = 1;

    switch (operand_1->lwk_operator) {
	case lwk_c_ddif_string_literal :
	    cstr1 = (_DDIFString) operand_1->lwk_operand_1;
	    var1 = _False;
	    break;

	case lwk_c_property_value :
	    _GetValue(object, (_String) operand_1->lwk_operand_1,
		lwk_c_domain_ddif_string, (_AnyPtr) &cstr1);
	    break;

	case lwk_c_some_property_value :
	    list1 = (_List) _GetValueList(object,
		(_String) operand_1->lwk_operand_1);
	    _GetValue(list1, _P_ElementCount, lwk_c_domain_integer, &cnt1);
	    multi1 = _True;
	    break;

	default :
	    _Raise(inv_query_expr);
    }

    /*
    **  Get the second operand(s).
    */

    if (operand_2 == (_QueryExpression) _NullObject)
	_Raise(inv_query_expr);

    multi2 = _False;
    var2 = _True;
    cnt2 = 1;

    switch (operand_2->lwk_operator) {
	case lwk_c_ddif_string_literal :
	    cstr2 = (_DDIFString) operand_2->lwk_operand_1;
	    var2 = _False;
	    break;

	case lwk_c_property_value :
	    _GetValue(object, (_String) operand_2->lwk_operand_1,
		lwk_c_domain_ddif_string, (_AnyPtr) &cstr2);
	    break;

	case lwk_c_some_property_value :
	    list2 = (_List) _GetValueList(object,
		(_String) operand_2->lwk_operand_1);
	    _GetValue(list2, _P_ElementCount, lwk_c_domain_integer, &cnt2);
	    multi2 = _True;
	    break;

	default :
	    _Raise(inv_query_expr);
    }

    /*
    **  Compare the values
    */

    for (i1 = 0; i1 < cnt1; i1++) {
        for (i2 = 0; i2 < cnt2; i2++) {
	    if (multi1)
		_RemoveElement(list1, lwk_c_domain_ddif_string,
		    (_AnyPtr) &cstr1);

	    if (multi2)
		_RemoveElement(list2, lwk_c_domain_ddif_string,
		    (_AnyPtr) &cstr2);

	    if (cstr1 == (_DDIFString) _NullObject
		    || cstr2 == (_DDIFString) _NullObject)
		result = _False;
	    else
		switch (operator) {
		    case lwk_c_ddif_string_is_eql :
			result = (_CompareValue(&cstr1, &cstr2,
			    lwk_c_domain_ddif_string) == 0);
			break;

		    case lwk_c_ddif_string_is_neq :
			result = (_CompareValue(&cstr1, &cstr2,
			    lwk_c_domain_ddif_string) != 0);
			break;

		    case lwk_c_ddif_string_contains :
			result = _ContainsDDIFString(cstr1, cstr2);
			break;

		    case lwk_c_ddif_string_contains_no :
			result = !_ContainsDDIFString(cstr1, cstr2);
			break;
		}

	    /*
	    **  Delete the strings if they where not Literals.
	    */

	    if (var1)
		_DeleteDDIFString(&cstr1);
	    if (var2)
		_DeleteDDIFString(&cstr2);

	    /*
	    **  If we had a successful comparison, we can quite now.
	    */

	    if (result)
		break;
	}

	if (result)
	    break;
    }

    /*
    **  Delete any value lists we may have created.
    */

    if (multi1)
	_Delete(&list1);

    if (multi2)
	_Delete(&list2);

    /*
    **	If there was an exception, clean up.  If it is one we can anticipate
    **	(e.g., NoSuchProperty, InvalidDomain), return False.  Otherwise,
    **	reraise the exception.
    */

    _Exceptions
	_WhenOthers
	    _Delete(&list1);
	    _Delete(&list2);
	    if (var1)
		_DeleteDDIFString(&cstr1);
	    if (var2)
		_DeleteDDIFString(&cstr2);

	    if (_Others == _StatusCode(inv_domain)
		    || _Others == _StatusCode(no_such_property))
		result = _False;
	    else
		_Reraise;

    _EndExceptionBlock

    return result;
    }


static _Boolean  CompareDate(object, operator, operand_1, operand_2)
_Object object;
 lwk_query_operator operator;

    _QueryExpression operand_1;
 _QueryExpression operand_2;

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
    int i1, i2;
    int relation;
    _Boolean result;
    _Boolean multi1;
    _Boolean multi2;
    _Integer cnt1;
    _Integer cnt2;
    _Boolean volatile var1;
    _Boolean volatile var2;
    _Date volatile date1;
    _Date volatile date2;
    _List_of(_Date) volatile list1;
    _List_of(_Date) volatile list2;

    /*
    **  Initialize temporary variables
    */

    date1 = (_Date) _NullObject;
    date2 = (_Date) _NullObject;
    list1 = (_List) _NullObject;
    list2 = (_List) _NullObject;

    _StartExceptionBlock

    /*
    **  Get the first operand(s).
    */

    if (operand_1 == (_QueryExpression) _NullObject)
	_Raise(inv_query_expr);

    multi1 = _False;
    var1 = _True;
    cnt1 = 1;

    switch (operand_1->lwk_operator) {
	case lwk_c_date_literal :
	    date1 = (_Date) operand_1->lwk_operand_1;
	    var1 = _False;
	    break;

	case lwk_c_property_value :
	    _GetValue(object, (_String) operand_1->lwk_operand_1, lwk_c_domain_date,
		(_AnyPtr) &date1);
	    break;

	case lwk_c_some_property_value :
	    list1 = (_List) _GetValueList(object,
		(_String) operand_1->lwk_operand_1);
	    _GetValue(list1, _P_ElementCount, lwk_c_domain_integer, &cnt1);
	    multi1 = _True;
	    break;

	default :
	    _Raise(inv_query_expr);
    }

    /*
    **  Get the second operand(s).
    */

    if (operand_2 == (_QueryExpression) _NullObject)
	_Raise(inv_query_expr);

    multi2 = _False;
    var2 = _True;
    cnt2 = 1;

    switch (operand_2->lwk_operator) {
	case lwk_c_date_literal :
	    date2 = (_Date) operand_2->lwk_operand_1;
	    var2 = _False;
	    break;

	case lwk_c_property_value :
	    _GetValue(object, (_String) operand_2->lwk_operand_1, lwk_c_domain_date,
		(_AnyPtr) &date2);
	    break;

	case lwk_c_some_property_value :
	    list2 = (_List) _GetValueList(object,
		(_String) operand_2->lwk_operand_1);
	    _GetValue(list2, _P_ElementCount, lwk_c_domain_integer, &cnt2);
	    multi2 = _True;
	    break;

	default :
	    _Raise(inv_query_expr);
    }

    /*
    **  Compare the values
    */

    for (i1 = 0; i1 < cnt1; i1++) {
        for (i2 = 0; i2 < cnt2; i2++) {
	    if (multi1)
		_RemoveElement(list1, lwk_c_domain_date, (_AnyPtr) &date1);

	    if (multi2)
		_RemoveElement(list2, lwk_c_domain_date, (_AnyPtr) &date2);

	    if (date1 == (_Date) _NullObject || date2 == (_Date) _NullObject)
		result = _False;
	    else {
		relation = _CompareDate(date1, date2);

		switch (operator) {
		    case lwk_c_date_is_eql :
			result = (relation == 0);
			break;

		    case lwk_c_date_is_neq :
			result = (relation != 0);
			break;

		    case lwk_c_date_is_lss :
			result = (relation < 0);
			break;

		    case lwk_c_date_is_leq :
			result = (relation <= 0);
			break;

		    case lwk_c_date_is_gtr :
			result = (relation > 0);
			break;

		    case lwk_c_date_is_geq :
			result = (relation >= 0);
			break;
		}
	    }

	    /*
	    **  Delete the strings if they where not Literals.
	    */

	    if (var1)
		_DeleteDate(&date1);
	    if (var2)
		_DeleteDate(&date2);

	    /*
	    **  If we had a successful comparison, we can quite now.
	    */

	    if (result)
		break;
	}

	if (result)
	    break;
    }

    /*
    **  Delete any value lists we may have created.
    */

    if (multi1)
	_Delete(&list1);

    if (multi2)
	_Delete(&list2);

    /*
    **	If there was an exception, clean up.  If it is one we can anticipate
    **	(e.g., NoSuchProperty, InvalidDomain), return False.  Otherwise,
    **	reraise the exception.
    */

    _Exceptions
	_WhenOthers
	    _Delete(&list1);
	    _Delete(&list2);

	    if (var1)
		_DeleteDate(&date1);
	    if (var2)
		_DeleteDate(&date2);

 	    if (_Others == _StatusCode(inv_domain)
		    || _Others == _StatusCode(no_such_property))
		result = _False;
	    else
		_Reraise;

    _EndExceptionBlock

    return result;
    }


static _Boolean  CompareIntegers(object, operator, operand_1, operand_2)
_Object object;
 lwk_query_operator operator;

    _QueryExpression operand_1;
 _QueryExpression operand_2;

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
    int i1, i2;
    _Boolean result;
    _Boolean multi1;
    _Boolean multi2;
    _Integer cnt1;
    _Integer cnt2;
    _Integer int1;
    _Integer int2;
    _List_of(_Integer) volatile list1;
    _List_of(_Integer) volatile list2;

    /*
    **  Initialize temporary variables
    */

    list1 = (_List) _NullObject;
    list2 = (_List) _NullObject;

    _StartExceptionBlock

    /*
    **  Get the first operand(s).
    */

    if (operand_1 == (_QueryExpression) _NullObject)
	_Raise(inv_query_expr);

    multi1 = _False;
    cnt1 = 1;

    switch (operand_1->lwk_operator) {
	case lwk_c_integer_literal :
	    int1 = *((_IntegerPtr) operand_1->lwk_operand_1);
	    break;

	case lwk_c_property_value :
	    _GetValue(object, (_String) operand_1->lwk_operand_1,
		lwk_c_domain_integer, &int1);
	    break;

	case lwk_c_some_property_value :
	    list1 = (_List) _GetValueList(object,
		(_String) operand_1->lwk_operand_1);
	    _GetValue(list1, _P_ElementCount, lwk_c_domain_integer, &cnt1);
	    multi1 = _True;
	    break;

	default :
	    _Raise(inv_query_expr);
    }

    /*
    **  Get the second operand(s).
    */

    if (operand_2 == (_QueryExpression) _NullObject)
	_Raise(inv_query_expr);

    multi2 = _False;
    cnt2 = 1;

    switch (operand_2->lwk_operator) {
	case lwk_c_integer_literal :
	    int2 = *((_IntegerPtr) operand_2->lwk_operand_1);
	    break;

	case lwk_c_property_value :
	    _GetValue(object, (_String) operand_2->lwk_operand_1,
		lwk_c_domain_integer, &int2);
	    break;

	case lwk_c_some_property_value :
	    list2 = (_List) _GetValueList(object,
		(_String) operand_2->lwk_operand_1);
	    _GetValue(list2, _P_ElementCount, lwk_c_domain_integer, &cnt2);
	    multi2 = _True;
	    break;

	default :
	    _Raise(inv_query_expr);
    }

    /*
    **  Compare the values
    */

    for (i1 = 0; i1 < cnt1; i1++) {
        for (i2 = 0; i2 < cnt2; i2++) {
	    if (multi1)
		_RemoveElement(list1, lwk_c_domain_integer, &int1);

	    if (multi2)
		_RemoveElement(list2, lwk_c_domain_integer, &int2);

	    switch (operator) {
		case lwk_c_integer_is_eql :
		    result = (int1 == int2);
		    break;

		case lwk_c_integer_is_neq :
		    result = (int1 != int2);
		    break;

		case lwk_c_integer_is_lss :
		    result = (int1 < int2);
		    break;

		case lwk_c_integer_is_leq :
		    result = (int1 <= int2);
		    break;

		case lwk_c_integer_is_gtr :
		    result = (int1 > int2);
		    break;

		case lwk_c_integer_is_geq :
		    result = (int1 >= int2);
		    break;
	    }

	    /*
	    **  If we had a successful comparison, we can quite now.
	    */

	    if (result)
		break;
	}

	if (result)
	    break;
    }

    /*
    **  Delete any value lists we may have created.
    */

    if (multi1)
	_Delete(&list1);

    if (multi2)
	_Delete(&list2);

    /*
    **	If there was an exception, clean up.  If it is one we can anticipate
    **	(e.g., NoSuchProperty, InvalidDomain), return False.  Otherwise,
    **	reraise the exception.
    */

    _Exceptions
	_WhenOthers
	    _Delete(&list1);
	    _Delete(&list2);

	    if (_Others == _StatusCode(inv_domain)
		    || _Others == _StatusCode(no_such_property))
		result = _False;
	    else
		_Reraise;

    _EndExceptionBlock

    return result;
    }


static _Boolean  CompareFloats(object, operator, operand_1, operand_2)
_Object object;
 lwk_query_operator operator;

    _QueryExpression operand_1;
 _QueryExpression operand_2;

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
    int i1, i2;
    _Boolean result;
    _Boolean multi1;
    _Boolean multi2;
    _Integer cnt1;
    _Integer cnt2;
    _Float volatile float1;
    _Float volatile float2;
    _List_of(_Float) volatile list1;
    _List_of(_Float) volatile list2;

    /*
    **  Initialize temporary variables
    */

    list1 = (_List) _NullObject;
    list2 = (_List) _NullObject;

    _StartExceptionBlock

    /*
    **  Get the first operand(s).
    */

    if (operand_1 == (_QueryExpression) _NullObject)
	_Raise(inv_query_expr);

    multi1 = _False;
    cnt1 = 1;

    switch (operand_1->lwk_operator) {
	case lwk_c_float_literal :
	    float1 = *((_FloatPtr) operand_1->lwk_operand_1);
	    break;

	case lwk_c_property_value :
	    _GetValue(object, (_String) operand_1->lwk_operand_1,
		lwk_c_domain_float, (_AnyPtr) &float1);
	    break;

	case lwk_c_some_property_value :
	    list1 = (_List) _GetValueList(object,
		(_String) operand_1->lwk_operand_1);
	    _GetValue(list1, _P_ElementCount, lwk_c_domain_integer, &cnt1);
	    multi1 = _True;
	    break;

	default :
	    _Raise(inv_query_expr);
    }

    /*
    **  Get the second operand(s).
    */

    if (operand_2 == (_QueryExpression) _NullObject)
	_Raise(inv_query_expr);

    multi2 = _False;
    cnt2 = 1;

    switch (operand_2->lwk_operator) {
	case lwk_c_float_literal :
	    float2 = *((_FloatPtr) operand_2->lwk_operand_1);
	    break;

	case lwk_c_property_value :
	    _GetValue(object, (_String) operand_2->lwk_operand_1,
		lwk_c_domain_float, (_AnyPtr) &float2);
	    break;

	case lwk_c_some_property_value :
	    list2 = (_List) _GetValueList(object,
		(_String) operand_2->lwk_operand_1);
	    _GetValue(list2, _P_ElementCount, lwk_c_domain_integer, &cnt2);
	    multi2 = _True;
	    break;

	default :
	    _Raise(inv_query_expr);
    }

    /*
    **  Compare the values
    */

    for (i1 = 0; i1 < cnt1; i1++) {
        for (i2 = 0; i2 < cnt2; i2++) {
	    if (multi1)
		_RemoveElement(list1, lwk_c_domain_float, (_AnyPtr) &float1);

	    if (multi2)
		_RemoveElement(list2, lwk_c_domain_float, (_AnyPtr) &float2);

	    switch (operator) {
		case lwk_c_float_is_eql :
		    result = (float1 == float2);
		    break;

		case lwk_c_float_is_neq :
		    result = (float1 != float2);
		    break;

		case lwk_c_float_is_lss :
		    result = (float1 < float2);
		    break;

		case lwk_c_float_is_leq :
		    result = (float1 <= float2);
		    break;

		case lwk_c_float_is_gtr :
		    result = (float1 > float2);
		    break;

		case lwk_c_float_is_geq :
		    result = (float1 >= float2);
		    break;
	    }

	    /*
	    **  If we had a successful comparison, we can quite now.
	    */

	    if (result)
		break;
	}

	if (result)
	    break;
    }

    /*
    **  Delete any value lists we may have created.
    */

    if (multi1)
	_Delete(&list1);

    if (multi2)
	_Delete(&list2);

    /*
    **	If there was an exception, clean up.  If it is one we can anticipate
    **	(e.g., NoSuchProperty, InvalidDomain), return False.  Otherwise,
    **	reraise the exception.
    */

    _Exceptions
	_WhenOthers
	    _Delete(&list1);
	    _Delete(&list2);

	    if (_Others == _StatusCode(inv_domain)
		    || _Others == _StatusCode(no_such_property))
		result = _False;
	    else
		_Reraise;

    _EndExceptionBlock

    return result;
    }


static _Boolean  CompareBooleans(object, operator, operand_1, operand_2)
_Object object;
 lwk_query_operator operator;

    _QueryExpression operand_1;
 _QueryExpression operand_2;

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
    int i1, i2;
    _Boolean result;
    _Boolean multi1;
    _Boolean multi2;
    _Integer cnt1;
    _Integer cnt2;
    _Boolean volatile bool1;
    _Boolean volatile bool2;
    _List_of(_Boolean) volatile list1;
    _List_of(_Boolean) volatile list2;

    /*
    **  Initialize temporary variables
    */

    list1 = (_List) _NullObject;
    list2 = (_List) _NullObject;

    _StartExceptionBlock

    /*
    **  Get the first operand(s).
    */

    if (operand_1 == (_QueryExpression) _NullObject)
	_Raise(inv_query_expr);

    multi1 = _False;
    cnt1 = 1;

    switch (operand_1->lwk_operator) {
	case lwk_c_boolean_literal :
	    bool1 = *((_BooleanPtr) operand_1->lwk_operand_1);
	    break;

	case lwk_c_property_value :
	    _GetValue(object, (_String) operand_1->lwk_operand_1,
		lwk_c_domain_boolean, (_AnyPtr) &bool1);
	    break;

	case lwk_c_some_property_value :
	    list1 = (_List) _GetValueList(object,
		(_String) operand_1->lwk_operand_1);
	    _GetValue(list1, _P_ElementCount, lwk_c_domain_integer, &cnt1);
	    multi1 = _True;
	    break;

	default :
	    _Raise(inv_query_expr);
    }

    /*
    **  Get the second operand(s).
    */

    multi2 = _False;
    cnt2 = 1;

    if (operand_2 == (_QueryExpression) _NullObject) {
	if (operator != lwk_c_boolean_is_true
		&& operator != lwk_c_boolean_is_false)
	    _Raise(inv_query_expr);
    }
    else {
	switch (operand_2->lwk_operator) {
	    case lwk_c_boolean_literal :
		bool2 = *((_BooleanPtr) operand_2->lwk_operand_1);
		break;

	    case lwk_c_property_value :
		_GetValue(object, (_String) operand_2->lwk_operand_1,
		    lwk_c_domain_boolean, (_AnyPtr) &bool2);
		break;

	    case lwk_c_some_property_value :
		list2 = (_List) _GetValueList(object,
		    (_String) operand_2->lwk_operand_1);
		_GetValue(list2, _P_ElementCount, lwk_c_domain_integer, &cnt2);
		multi2 = _True;
		break;

	    default :
		_Raise(inv_query_expr);
	}
    }

    /*
    **  Compare the values
    */

    for (i1 = 0; i1 < cnt1; i1++) {
        for (i2 = 0; i2 < cnt2; i2++) {
	    if (multi1)
		_RemoveElement(list1, lwk_c_domain_boolean, (_AnyPtr) &bool1);

	    if (multi2)
		_RemoveElement(list2, lwk_c_domain_boolean, (_AnyPtr) &bool2);

	    switch (operator) {
		case lwk_c_boolean_is_eql :
		    result = (bool1 == bool2);
		    break;

		case lwk_c_boolean_is_neq :
		    result = (bool1 != bool2);
		    break;

		case lwk_c_boolean_is_true :
		    result = (bool1);
		    break;

		case lwk_c_boolean_is_false :
		    result = (!bool1);
		    break;
	    }

	    /*
	    **  If we had a successful comparison, we can quite now.
	    */

	    if (result)
		break;
	}

	if (result)
	    break;
    }

    /*
    **  Delete any value lists we may have created.
    */

    if (multi1)
	_Delete(&list1);

    if (multi2)
	_Delete(&list2);

    /*
    **	If there was an exception, clean up.  If it is one we can anticipate
    **	(e.g., NoSuchProperty, InvalidDomain), return False.  Otherwise,
    **	reraise the exception.
    */

    _Exceptions
	_WhenOthers
	    _Delete(&list1);
	    _Delete(&list2);

	    if (_Others == _StatusCode(inv_domain)
		    || _Others == _StatusCode(no_such_property))
		result = _False;
	    else
		_Reraise;

    _EndExceptionBlock

    return result;
    }
