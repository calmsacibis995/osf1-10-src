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
**	List object methods 
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
#include "list.h"
#else /* !MSDOS */
#include "his_include.h"
#include "lwk_list.h"
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

/*
**  Global Data Definitions
*/

_Global _ListTypeInstance;	/* Instance record for Type */
_Global _ListType;              /* Type */

/*
**  External Data Declarations
*/

/*
**  Static Data Definitions
*/

static _Type _Constant MyType = &_TypeListInstance;
static _ListPropertyNameTable;
static _PropertyNameTableEntry _Constant *MyProperties = ListPropertyNameTable;



void  LwkOpList(list)
_List list;

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



_List  LwkOpListCreate(type, domain, estimated_size)
_Type type;
 _Domain domain;
 _Integer estimated_size;

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
    int cnt;
    _List volatile new_list;

    /*
    **  Create a new List
    */
    
    new_list = (_List) _Create(type);

    _StartExceptionBlock

    /*
    **  Initialize the properties of the List
    */

    _Domain_of(new_list) = domain;

    _ElementCount_of(new_list) = 0;

    if (estimated_size == 0)
	_ElementAlloc_of(new_list) = _DefaultListLength;
    else
	_ElementAlloc_of(new_list) = estimated_size;

    _Elements_of(new_list) = (_AnyValuePtr)
	_AllocateMem(_ElementAlloc_of(new_list) * sizeof(_AnyValue));

    for (cnt = 0; cnt < _ElementAlloc_of(new_list); cnt++)
	_ClearValue(&_Elements_of(new_list)[cnt], lwk_c_domain_integer);

    /*
    **  If any exceptions are raised, delete the List then reraise the
    **	exception.
    */
    
    _Exceptions
	_WhenOthers
	    _Delete(&new_list);
	    _Reraise;
    _EndExceptionBlock

    return new_list;
    }



void  LwkOpListInitialize(list, proto, aggregate)
_List list;
 _List proto;
 _Boolean aggregate;

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
    int cnt;
    _Domain domain;

    /*
    **  Initialize the properties defined by our type
    */

    _Domain_of(list) = lwk_c_domain_unknown;

    _ClearValue(&_ElementCount_of(list), lwk_c_domain_integer);
    _ClearValue(&_ElementAlloc_of(list), lwk_c_domain_integer);

    _Elements_of(list) = (_AnyValuePtr) _NullObject;

    /*
    **  Invoke the Initialize operation in our supertype
    */

    _Initialize_S(list, (_Object) proto, aggregate, MyType);
    
    /*
    **  If a prototype was provided, copy properties from it.
    */

    if (proto != (_List) _NullObject) {
	_Domain_of(list) = _Domain_of(proto);

	_CopyValue(&_ElementAlloc_of(proto), &_ElementAlloc_of(list),
	    lwk_c_domain_integer);

	_Elements_of(list) = (_AnyValuePtr)
	    _AllocateMem(_ElementAlloc_of(list) * sizeof(_AnyValue));

	/*
	** Append the elements of the prototype to the new list.
	*/
	
	_AppendElements((_Object) list, (_Object) proto);
    }

    return;
    }



void  LwkOpListFree(list)
_List list;

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
    int cnt;

    /*
    **  Delete the elements of the list.
    */

    for (cnt = 0; cnt < _ElementCount_of(list); cnt++)
	_DeleteValue(&_Elements_of(list)[cnt], _Domain_of(list));

    /*
    **  Free the storage used by the properties defined by our Type.
    */

    _DeleteValue(&_ElementCount_of(list), lwk_c_domain_integer);
    _DeleteValue(&_ElementAlloc_of(list), lwk_c_domain_integer);

    /*
    **  Free the list elements array.
    */

    if (_Elements_of(list) != (_AnyValuePtr) _NullObject)
	_FreeMem(_Elements_of(list));

    /*
    **  Ask our supertype to free its properties
    */

    _Free_S(list, MyType);

    return;
    }



_Set  LwkOpListEnumProps(list)
_List list;

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
    _Set set;

    /*
    **  Ask our supertype to enumerate its properties.
    */

    set = (_Set) _EnumerateProperties_S(list, MyType);

    /*
    **  Add our properties to the set.
    */

    _ListTable(MyProperties, set, _False);

    /*
    **  Return the property name set to the caller
    */

    return set;
    }



_Boolean  LwkOpListIsMultiValued(list, property)
_List list;
 _String property;

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
    int index;
    _Boolean answer;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	answer = _IsMultiValued_S(list, property, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _DomainIndex :
	    case _ElementCountIndex :
		answer = _False;
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return answer;
    }



_Domain  LwkOpListGetValueDomain(list, property)
_List list;
 _String property;

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
    int index;
    _Domain domain;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	domain = (_Domain) _GetValueDomain_S(list, property, MyType);
    }
    else {
	/*
	**  Domain depends on the property
	*/

	switch (index) {
	    case _DomainIndex :
	    case _ElementCountIndex :
		domain = lwk_c_domain_integer;
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return domain;
    }



void  LwkOpListGetValue(list, property, domain, value)
_List list;
 _String property;
 _Domain domain;

    _AnyPtr value;

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
    int index;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_GetValue_S(list, property, domain, value, MyType);
    }
    else {
	/*
	**  Value depends on the property
	*/

	switch (index) {
	    case _DomainIndex :
		if (_IsDomain(domain, lwk_c_domain_integer))
		    *((_IntegerPtr) value) = (_Integer) _Domain_of(list);
		else
		    _Raise(inv_domain);

		break;

	    case _ElementCountIndex :
		if (_IsDomain(domain, lwk_c_domain_integer))
		    _CopyValue(&_ElementCount_of(list), value,
			lwk_c_domain_integer);
		else
		    _Raise(inv_domain);

		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return;
    }



_List  LwkOpListGetValueList(list, property)
_List list;
 _String property;

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
    int index;
    _List value_list;
    _Integer integer;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	value_list = (_List) _GetValueList_S(list, property, MyType);
    }
    else {
	/*
	**  Value depends on the property
	*/

	switch (index) {
	    case _DomainIndex :
		integer = (_Integer) _Domain_of(list);

		_ListValue(&integer, (_ObjectPtr) &value_list,
		    lwk_c_domain_integer);

		break;

	    case _ElementCountIndex :
		_ListValue(&_ElementCount_of(list),
		    (_ObjectPtr) &value_list, lwk_c_domain_integer);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return value_list;
    }


void  LwkOpListSetValue(list, property, domain, value, flag)
_List list;
 _String property;
 _Domain domain;

    _AnyPtr value;
 _SetFlag flag;

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
    int index;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_SetValue_S(list, property, domain, value, flag, MyType);
    }
    else {
	/*
	**  Value depends on the property
	*/

	switch (index) {
	    case _DomainIndex :
	    case _ElementCountIndex :
		_Raise(property_is_readonly);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return;
    }



void  LwkOpListSetValueList(list, property, values, flag)
_List list;
 _String property;
 _List values;

    _SetFlag flag;

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
    int index;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_SetValueList_S(list, property, values, flag, MyType);
    }
    else {
	/*
	**  Value depends on the property
	*/

	switch (index) {
	    case _DomainIndex :
	    case _ElementCountIndex :
		_Raise(property_is_readonly);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return;
    }



void  LwkOpListSelectElement(list, index, domain, element)
_List list;
 _Integer index;
 _Domain domain;

    _AnyPtr element;

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
    **  Make sure that the domain of the element is compatible with the
    **	domain of the list.
    */

    if (!_IsDomain(domain, _Domain_of(list)))
	_Raise(inv_object);

    /*
    **	Get the requested element from the list.  An index of -1 means the last
    **	element in the List.  If the index is oustide the bounds of the List,
    **	return a null value.
    */

    if (index == -1)
	index = _ElementCount_of(list) - 1;

    if (index < 0 || index >= _ElementCount_of(list))
	_ClearValue(element, domain);
    else
	_CopyValue(&_Elements_of(list)[index], element, domain);

    return;
    }



void  LwkOpListAddElement(list, domain, element, copy)
_List list;
 _Domain domain;
 _AnyPtr element;

    _Boolean copy;

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
    **	If this List was created using a simple Create operation, the List
    **	domain was set to Unknown.  So, if this is the first AddElement after
    **	the simple Create, set the List domain to the domain of this first
    **	value.
    */

    if (_Domain_of(list) == lwk_c_domain_unknown)
	_Domain_of(list) = domain;

    /*
    **  Make sure that the domain of the new element is compatible with the
    **	domain of the list.
    */

    if (!_IsDomain(_Domain_of(list), domain))
	_Raise(inv_domain);

    /*
    **	If there is not already space in the current element list allocation,
    **	make space for this element.
    */

    if (_ElementCount_of(list) >= _ElementAlloc_of(list)) {
	_ElementAlloc_of(list) += _DefaultListExtension;

	_Elements_of(list) =
	    (_AnyValuePtr) _ReallocateMem(_Elements_of(list),
		(_ElementAlloc_of(list) * sizeof(_AnyValue)));
    }

    /*
    **	Add the new element (or a copy of the new element) to the list.
    */

    if (copy)
	_CopyValue(element, &_Elements_of(list)[_ElementCount_of(list)],
	    _Domain_of(list));
    else
	_MoveValue(element, &_Elements_of(list)[_ElementCount_of(list)],
	    _Domain_of(list));

    /*
    **  There is now one more element in the list
    */

    _ElementCount_of(list)++;

    return;
    }



void  LwkOpListAppendElements(list, append)
_List list;
 _List append;

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
    int i;

    /*
    **  Make sure that the domain of the new elements is compatible with the
    **	domain of the list.
    */

    if (!_IsDomain(_Domain_of(list), _Domain_of(append)))
	_Raise(inv_domain);

    /*
    **  Add a copy of each element of append to the list.
    */
    
    for (i = 0; i < _ElementCount_of(append); i++)
	_AddElement((_Object) list, _Domain_of(append),
	    &_Elements_of(append)[i], _True);

    return;
    }



void  LwkOpListDeleteElement(list, domain, element)
_List list;
 _Domain domain;
 _AnyPtr element;

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
    int i, j;
    _Boolean no_duplicates;
    _Boolean found;

    /*
    **	Make sure that the domain of the element to delete is compatible with
    **	the domain of the list.
    */

    if (!_IsDomain(_Domain_of(list), domain))
	_Raise(inv_domain);

    /*
    **	Note that if the type of the list is really Set, then we can assume
    **	that there are no more than one occurence of the element to be deleted.
    */

    if (_IsType((_Object) list, _TypeSet))
	no_duplicates = _True;
    else
	no_duplicates = _False;

    found = _False;

    /*
    **	Search for and delete all instances of the element from the list.  If
    **	the element was copied when added to the list, it must be freed.
    */

    for (i = 0; i < _ElementCount_of(list); i++) {
	if (_CompareValue(&_Elements_of(list)[i], element, domain) == 0) {
	    found = _True;

	    /*
	    **	Match -- delete the element.
	    */

	    _DeleteValue(&_Elements_of(list)[i], domain);

	    /*
	    **	Move all the other elements up one.
	    */

	    for (j = i + 1; j < _ElementCount_of(list); j++)
		_MoveValue(&_Elements_of(list)[j],
		    &_Elements_of(list)[j - 1], domain);

	    /*
	    **  Decrement the element count.
	    */

	    _ElementCount_of(list)--;

	    /*
	    **	If this is a set, we are done.  Otherwise, we must
	    **	continue checking from the element we just moved up to
	    **	this slot.
	    */

	    if (no_duplicates)
		break;
	    else
		i--;
	}
    }

    if (!found)
	_Raise(no_such_element);

    return;
    }



void  LwkOpListRemoveElement(list, domain, element)
_List list;
 _Domain domain;
 _AnyPtr element;

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
    int i;

    /*
    **  Make sure there is at least one element left in the list.
    */

    if (_ElementCount_of(list) < 1)
	_Raise(list_empty);

    /*
    **  Make sure that the domain of the removed element is compatible with the
    **	domain of the list.
    */

    if (!_IsDomain(domain, _Domain_of(list)))
	_Raise(inv_domain);

    /*
    **	Return the first element in the list.
    */

    _MoveValue(&_Elements_of(list)[0], element, domain);

    /*
    **	Move all the remaining elements up one in storage.  We won't bother
    **	trying to reclaim any storage.
    */

    for (i = 1; i < _ElementCount_of(list); i++)
	_MoveValue(&_Elements_of(list)[i], &_Elements_of(list)[i - 1],
	    domain);

    /*
    **  There is now one less element in the list
    */

    _ElementCount_of(list)--;

    return;
    }



_Termination  LwkOpListIterate(list, domain, closure, routine)
_List list;
 _Domain domain;
 _Closure closure;

    _Callback routine;

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
    int i;
    _Integer count;
    _AnyValue *elements;
    _Termination termination;

    /*
    **	Make sure that the domain of the iterated element is compatible with
    **	the domain of the list.
    */

    if (!_IsDomain(domain, _Domain_of(list)))
	_Raise(inv_domain);

    /*
    **  Make sure that we have a valid routine to invoke
    */

    if (routine == (_Callback) _NullObject)
	_Raise(inv_argument);

    /*
    ** If there are no elements in the Lisst, return now
    */

    if (_ElementCount_of(list) <= 0)
	return (_Termination) 0;

    /*
    ** Initialize
    */
    
    elements = (_AnyValuePtr) _NullObject;

    _StartExceptionBlock

    /*
    **  Copy the List elements and List count in case the Callback routine
    **	attempts to modify the List.  This is not complete protection, however.
    **	If the routine deletes any elements that have not yet been iterated,
    **	the results are unpredictable (e.g., an access violation may result).
    */

    count = _ElementCount_of(list);

    elements = (_AnyValuePtr) _AllocateMem(count * sizeof(_AnyValue));

    _CopyMem(_Elements_of(list), elements, count * sizeof(_AnyValue));

    /*
    **  Callback the routine provided once for each element of the list.
    */

    termination = (_Termination) 0;

    for (i = 0; i < count; i++) {
	termination = (_Termination) (*routine)(closure, list, _Domain_of(list),
	    (_AnyPtr) &elements[i]);

	/*
	**  If the return value is non-zero, stop iterating.
	*/

	if (termination != (_Termination) 0)
	    break;
    }

    /*
    **  Free the copy of the List elements
    */

    _FreeMem(elements);

    /*
    **	If any exceptions are raised, free the copy of the List elements then
    **	reraise the exception.
    */
    
    _Exceptions
	_WhenOthers
	    if (elements != (_AnyValuePtr) _NullObject)
		_FreeMem(elements);
	    _Reraise;
    _EndExceptionBlock

    /*
    **  Return the last value from the callback routine to the caller
    */

    return termination;
    }
