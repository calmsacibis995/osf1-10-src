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
** COPYRIGHT (c) 1989, 1991 BY
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
**	LinkWorks Manager
**
**  Version: V1.0
**
**  Abstract:
**	General utility routines (imported from LWK services)
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	André Pavanello
**
**  Creation Date: 15-Nov-89
**
**  Modification History:
**--
*/

/*
**  Include Files
*/

#include "hs_include.h"
#include "hs_abstract_objects.h"

/*
**  Table of Contents
*/

/*
**  Macro Definitions
*/

#define _StatusTablePart1 \
    _X_(hs_s_unknown, "StsUnknown", "Unknown error status"), \
    _X_(hs_s_success, "StsSuccess", "Success"), \
    _X_(hs_s_failure, "StsFailure", "Failure"), \
    _X_(hs_s_not_yet_impl, "StsNotYetImplemented", "Not yet implemented"), \
    _X_(hs_s_alloc_error, "StsAllocationFailure", "Allocation failure"), \
    _X_(hs_s_dealloc_error, "StsDeallocationFailure", "Deallocation failure"), \
    _X_(hs_s_list_empty, "StsListEmpty", "List empty"), \
    _X_(hs_s_drm_open_error, "StsDRMHierarchyOpenFailed", "DRM Hierarchy open failed"), \
    _X_(hs_s_drm_fetch_error, "StsDRMHierarchyFetchFailed", "DRM Hierarchy fetch failed"), \
    _X_(hs_s_drm_close_error, "StsDRMHierarchyCloseFailed", "DRM Hierarchy close failed"), \
    _X_(hs_s_inv_widget_id, "StsInvalidWidgetId", "Invalid widget identifier"), \
    _X_(hs_s_inv_operation, "StsInvalidOperation", "Invalid operation"), \
    _X_(hs_s_inv_domain, "StsInvalidDomain", "Invalid Domain"), \
    _X_(hs_s_no_such_property, "StsNoSuchProperty", "No such property"), \
    _X_(hs_s_property_is_readonly, "StsPropertyIsReadOnly", "Property is read only"), \
    _X_(hs_s_abstract_object, "StsAbstractObject", "Abstract object"), \
    _X_(hs_s_inv_argument, "StsInvalidArgument", "Invalid argument"), \
    _X_(hs_s_inv_comp_string, "StsInvalidCompoundString", "Invalid compound string"), \
    _X_(hs_s_single_valued_property, "StsSingleValuedProperty", "Single valued property"), \
    _X_(hs_s_property_not_deleted, "StsPropertyNotDeleted", "Property not deleted"),

#define _StatusTablePart2 \
    _X_(hs_s_inv_set_operation, "StsInavlidSetOperation", "Invalid set operation"), \
    _X_(hs_s_no_such_element, "StsNoSuchElement", "No such element"), \
    _X_(hs_s_inv_object, "StsInvalidObject", "Invalid object"), \
    _X_(hs_s_inv_currency, "StsInvalidCurrency", "Invalid currency"), \
    _X_(hs_s_cur_set_failure, "StsCurrencySetFailure", "Currency set failure"), \
    _X_(hs_s_lb_not_exist, "StsLbNotExist", "Default Linkbase doesn't exist"), \
    _X_(hs_s_new_lb, "StsNewLinkbase", "Creating a new one"), \
    _X_(hs_s_retrieve_error, "StsRetrieveError", "Retrieve error"), \
    _X_(hs_s_wnd_open_error, "StsWndOpenError", "Window open error"), \
    _X_(hs_s_wnd_disp_error, "StsWndDisplayError", "Window display failed"), \
    _X_(hs_s_startup_failed, "StsStartupFailed", "Startup failed"), \
    _X_(hs_s_set_value_failed, "StsSetValueFailed", "Set value failed"), \
    _X_(hs_s_get_value_failed, "StsGetValueFailed", "Get value failed"), \
    _X_(hs_s_objdsc_encode_error, "StsObjdscEncodeError", "Object descriptor encode error"), \
    _X_(hs_s_objdsc_decode_error, "StsObjdscDecodeError", "Object descriptor decode error"), \
    _X_(hs_s_get_objdsc_failed, "StsGetObjdscFailed", "Get object descriptor failed"), \
    _X_(hs_s_add_ele_failed, "StsAddEleFailed", "Add element failed"), \
    _X_(hs_s_create_list_failed, "StsCreateListFailed", "Create list failed"), \
    _X_(hs_s_close_lb_err, "StsCloseLbErr", "Close linkbase error"), \
    _X_(hs_s_open_lb_err, "StsOpenLbErr", "Open linkbase error"), 

#define _StatusTablePart3 \
    _X_(hs_s_store_failed, "StsStoreFailed", "Store failed"), \
    _X_(hs_s_save_failed, "StsSaveFailed", "Save failed"), \
    _X_(hs_s_open_lb_failed, "StsOpenLbFailed", "Open linkbase failed"), \
    _X_(hs_s_quit_failed, "StsQuitFailed", "Exit failed"), \
    _X_(hs_s_map_menu_failed, "StsMapMenuFailed", "Map menu failed"), \
    _X_(hs_s_activate_failed, "StsActivateFailed", "Activate failed"), \
    _X_(hs_s_record_failed, "StsRecordFailed", "Record failed"), \
    _X_(hs_s_show_prop_failed, "StsShowPropFailed", "Show property failed"), \
    _X_(hs_s_expand_failed, "StsExpandFailed", "Expand failed"), \
    _X_(hs_s_save_lb_failed, "StsSaveLbFailed", "Save linkbase failed"), \
    _X_(hs_s_paste_failed, "StsPasteFailed", "Paste failed"), \
    _X_(hs_s_cut_failed, "StsCutFailed", "Cut failed"), \
    _X_(hs_s_copy_failed, "StsCopyFailed", "Copy failed"), \
    _X_(hs_s_delete_failed, "StsDeleteFailed", "Delete failed"), \
    _X_(hs_s_get_domain_failed, "StsGetDomainFailed", "Get domain failed"), \
    _X_(hs_s_persobj_creation_failed, "StsPersobjCreationFailed", "Persistent object creation failed"), \
    _X_(hs_s_persobj_deletion_failed, "StsPersobjDeletionFailed", "Persistent object deletion failed"), \
    _X_(hs_s_encode_deletion_failed, "StsEncodeDeletionFailed", "Encoding deletion failed"), \
    _X_(hs_s_envmgr_deletion_failed, "StsEnvmgrDeletionFailed", "Environment mgr deletion failed"), \
    _X_(hs_s_iterate_failed, "StsIterateFailed", "Iterate failed"),
    
#define _StatusTablePart4 \
    _X_(hs_s_svn_entry_not_found, "StsSvnEntryNotFound", "Svn entry not found"), \
    _X_(hs_s_object_not_found, "StsObjectNotFound", "Object not found"), \
    _X_(hs_s_copy_from_clip_failed, "StsCopyFromClipFailed", "Copy from clipboard failed"), \
    _X_(hs_s_clip_locked, "StsCliplocked", "Clipboard locked"), \
    _X_(hs_s_no_timestamp, "StsNoTimestamp", "No timestamp"), \
    _X_(hs_s_lb_already_open, "StsLbAlreadyOpen", "Linkbase already open"), \
    _X_(hs_s_new_wnd_not_open, "StsNewWndNotOpen", "New window not opened"), \
    _X_(hs_s_sel_conf_cb_failed, "StsSelConfCbFailed", "Select Confirm callback failed"), \
    _X_(hs_s_select_cb_failed, "StsSelectCbFailed", "Select callback failed"), \
    _X_(hs_s_dwui_creation_failed, "StsDwUiCreationFailed", "DwUi creation failed"), \
    _X_(hs_s_no_self_insert, "StsNoSelfInsert", "No self insertion"), \
    _X_(hs_s_create_lb_err, "StsCreateLbErr", "Create linkbase failed"), \
    _X_(hs_s_update_err, "StsUpdateErr", "Update failed"), \
    _X_(hs_s_delete_err, "StsDeleteErr", "Delete failed"), \
    _X_(hs_s_save_attr_failed, "StsSaveAttrFailed", "Save attribute failed"), \
    _X_(hs_s_restore_attr_failed, "StsRestoreAttrFailed", "Restore attribute failed"), \
    _X_(hs_s_restore_sys_attr_failed, "StsRestoreSysAttrFailed", "Restore systen attribute failed"), \
    _X_(hs_s_object_retrieve_failed, "StsObjectRetrieveFailed", "Object retrieve failed"), \
    _X_(hs_s_duplicate_obj, "StsDuplicateObj", "Item already in list"),

#define _StatusTablePart5 \
    _X_(hs_s_lb_to_be_saved, "StsLinkbaseToBeSaved", "Save will remove deleted objects from the linkbase"), \
    _X_(hs_s_obj_name_to_be_changed, "StsObjNameToBeChanged", "Please change the name of the new object."), \
    _X_(hs_s_cannot_test_recursive, "StsCannotTestRecursive", "Cannot test recursive"), \
    _X_(hs_s_recursive_list, "StsRecursiveList", "You are trying to create a recursive list."), \
    _X_(hs_s_cannot_load_path, "StsCannotLoadPath", "Cannot load path."), \
    _X_(hs_s_cannot_load_attr, "StsCannotLoadAttr", "Cannot load attributes."), \
    _X_(hs_s_cannot_load_trail, "StsCannotLoadTrail", "Cannot load trail."), \
    _X_(hs_s_version_error, "StsVersionError", "Version error"), \
    _X_(hs_s_get_selection_failed, "StsGetSelectionFailed", "XtGetSelection failed."), \
    _X_(hs_s_unexpected_error, "StsUnexpectedError", "Unexpected error")
    	
/*
**  Type Definitions
*/

typedef struct __StatusEntry {
            _Status status;
	    _String string;
	    } _StatusEntry;

/*
**  Forward Routine Declarations
*/

_DeclareFunction(static _Termination DeleteElementFromList,
    (_List property_list, _List value_list, _Domain domain,
        _AnyValuePtr value));
_DeclareFunction(static _Termination RemoveProperty,
    (_AnyValuePtr property, _List element_list, _Domain domain,
        _AnyValuePtr value));
	
/*
**  Static Data Definitions
*/

/*
**  Status Code to String Conversion Table
*/

#define _X_(Status, Resource, String) {(Status), (String)}

static _StatusEntry _Constant StatusToString[_StatusCode(max)] = {
    _StatusTablePart1
    _StatusTablePart2
    _StatusTablePart3
    _StatusTablePart4
    _StatusTablePart5
};

/*
**  Status Code to Resource Name Conversion Table
*/

#undef  _X_
#define _X_(Status, Resource, String) {(Status), (Resource)}

static _StatusEntry _Constant StatusToResource[_StatusCode(max)] = {
    _StatusTablePart1
    _StatusTablePart2
    _StatusTablePart3
    _StatusTablePart4
    _StatusTablePart5
};

/*
**  Memory Usage Statistics
*/

static int TotalAllocations;
static int TotalAllocatedMemory;
static int TotalReallocations;
static int TotalDeallocations;
static int TotalDeallocatedMemory;

/*
**  Domain to Type Conversion Table
*/

static _Type _Constant DomainToType[hs_c_domain_max] = {
    (_Type) _NullObject,                /* Unknown */
    (_Type) _NullObject,                /* Integer */
    (_Type) _NullObject,                /* String  */
    (_Type) _NullObject,                /* CString */
    (_Type) _NullObject,                /* Boolean */
    (_Type) _NullObject,                /* Routine */
    (_Type) _NullObject,                /* Any ptr */
    &_TypeObjectInstance,
    &_TypeListInstance,
    &_TypeWindowInstance,
    &_TypeEnvWindowInstance,
    &_TypeLbWindowInstance,
    &_TypeHsObjectInstance,
    &_TypeEnvContextInstance,
};

/*
**  Global Data Definitions
*/

/*
**  External Data Declarations
*/


_Void  Env()
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


_Void  EnvMemoryStatistics()
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
    char *cp;
    int difference;

    printf("\nMemory Usage Statistics:\n");

    printf("    Total memory allocations: %d\n", TotalAllocations);
    printf("    Total memory reallocations: %d\n", TotalReallocations);
    printf("    Total memory deallocations: %d\n", TotalDeallocations);

    difference = TotalAllocations - TotalDeallocations;

    if (difference != 0) {
        if (difference > 0)
            cp = "less";
        else {
            cp = "more";
            difference = - difference;
        }
        printf("        (%d %s than allocations)\n", difference, cp);
    }

    printf("    Total memory allocated: %d\n", TotalAllocatedMemory);
    printf("    Total memory deallocated: %d\n", TotalDeallocatedMemory);

    difference = TotalAllocatedMemory - TotalDeallocatedMemory;

    if (difference != 0) {
        if (difference > 0)
            cp = "less";
        else {
            cp = "more";
            difference = - difference;
        }
        printf("        (%d %s than allocated)\n", difference, cp);
    }

    printf("\n");

    return;
    }


_AnyPtr  EnvAllocateMem(size)
_Constant int size;

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
    char *cp;

    cp = (char *) XtMalloc(size + sizeof(char *));

    if (cp == ((char *) _NullObject))
        _Raise(alloc_error);

    TotalAllocations += 1;
    TotalAllocatedMemory += size;

    *((int *) cp) = size;

    cp += sizeof(char *);

    return ((_AnyPtr) cp);
    }


_AnyPtr  EnvReallocateMem(pointer, new_size)
_AnyPtr pointer;
 _Constant int new_size;

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
    int old_size;
    char *cp;

    if (pointer == (_AnyPtr) _NullObject)
	cp = _AllocateMem(new_size);
    else {
	cp = ((char *) pointer) - sizeof(char *);

	old_size = *((int *) cp);

	cp = (char *) XtRealloc(cp, (new_size + sizeof(char *)));

	if (cp == (char *) 0)
	    _Raise(alloc_error);

	*((int *) cp) = new_size;

	cp += sizeof(char *);

	TotalAllocatedMemory += new_size - old_size;
	TotalReallocations += 1;
    }

    return (_AnyPtr) cp;
    }


_Void  EnvFreeMem(pointer)
_AnyPtr pointer;

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
    int size;
    char *cp;

    if (pointer == ((_AnyPtr) _NullObject))
	_Raise(dealloc_error);

    cp = ((char *) pointer) - sizeof(char *);

    size = *((int *) cp);

    TotalDeallocations += 1;
    TotalDeallocatedMemory += size;

    XtFree((char *) cp);

    return;
    }


_String  EnvCopyString(src)
_String src;

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
    _String dst;

    if (src == (_String) _NullObject)
	dst = src;
    else {
	dst = (_String) _AllocateMem(strlen((char *) src) + sizeof(char));

	strcpy((char *) dst, (char *) src);
    }

    return dst;
    }


_String  EnvConcatString(string1, string2)
_String string1;
 _String string2;

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
    _Integer len1;
    _Integer len2;
    _String string;

    if (string2 == (_String) _NullObject)
	string = string1;
    else {
	if (string1 == (_String) _NullObject)
	    string = _CopyString(string2);
	else {
	    len1 = _LengthString(string1);
	    len2 = _LengthString(string2);

	    string = _ReallocateMem(string1, len1 + len2 + 1);

	    _CopyMem(string2, &string[len1], len2);
	    string[len1 + len2] = _EndOfString;

	}
    }

    return string;
    }


_Integer  EnvLengthString(string)
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
    if (string == (_String) _NullObject)
	return (_Integer) 0;
    else
	return (_Integer) strlen(string);
    }


_Void  EnvDeleteString(string)
_String *string;

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
    if (*string != (_String) _NullObject) {
	_FreeMem(*string);

	*string = (_String) _NullObject;
    }

    return;
    }


_Integer  EnvCompareString(string1, string2)
_String string1;
 _String string2;

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
    if (string1 == (_String) _NullObject) {
	if (string2 == (_String) _NullObject)
	    return (_Integer) 0;
	else
	    return (_Integer) -1;
    }
    else if (string2 == (_String) _NullObject)
	return (_Integer) 1;

    return (_Integer) strcmp((char *) string1, (char *) string2);
    }


_Integer  EnvCompareStringN(string1, string2, length
)
_String string1;
 _String string2;
 _Integer length;

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
    if (string1 == (_String) _NullObject) {
	if (string2 == (_String) _NullObject)
	    return (_Integer) 0;
	else
	    return (_Integer) -1;
    }
    else if (string2 == (_String) _NullObject)
	return (_Integer) 1;

    return (_Integer) strncmp((char *) string1, (char *) string2, (int) length);
    }


_Boolean  EnvIsCaselessEqualString(string1, string2)
_String string1;
 _String string2;

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
    char c1, c2;

    if (string1 == (_String) _NullObject) {
	if (string2 == (_String) _NullObject)
	    return _True;
	else
	    return _False;
    }
    else if (string2 == (_String) _NullObject)
	return _False;

    i = 0;

    while (_True) {
	if (string1[i] == '\0') {
	    if (string2[i] == '\0')
		return _True;
	    else
		return _False;
	}

	if (islower(string1[i]))
	    c1 = _toupper(string1[i]);
	else
	    c1 = string1[i];

	if (islower(string2[i]))
	    c2 = _toupper(string2[i]);
	else
	    c2 = string2[i];

	if (c1 != c2)
	    return _False;

	i++;
    }
    }


_Boolean  EnvContainsString(string1, string2)
_String string1;
 _String string2;

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

    if (string1 == (_String) _NullObject || string2 == (_String) _NullObject)
	return _False;

    i1 = 0;
    i2 = 0;

    while(_True) {
	if ((char) string1[i1] == '\0') {
	    if ((char) string2[i2] == '\0')
		return _True;
	    else
		return _False;
	}
	
	if ((char) string2[i2] == '\0')
	    return _True;

	if ((char) string1[i1] == (char) string2[i2])
	    i2++;
	else
	    i2 = 0;

	i1++;
    }

    }


_Integer  EnvSearchTable(table, entry, include_private)
_PropertyNameTable table;
 _String entry;

    _Boolean include_private;

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
    _Integer i;

    if (table == (_PropertyNameTable) _NullObject
	|| entry == (_String) _NullObject)
	    return (_Integer) -1;

    i = 0;

    while (table[i].name != (_String) _NullObject) {
	if (table[i].public || (!table[i].public && include_private))
	    if (_IsCaselessEqualString(table[i].name, entry))
		return i;
	i++;
    }

    return (_Integer) -1;
    }


_Void  EnvListTable(table, list, include_private)
_PropertyNameTable table;
 _List list;

    _Boolean include_private;

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

    i = 0;

    if (table == (_PropertyNameTable) _NullObject
	|| list == (_List) _NullObject)
	    return;

    while (table[i].name != (_String) _NullObject) {
	if (table[i].public || (!table[i].public && include_private))
	    _AddElement(list, hs_c_domain_string, &table[i].name, _True);
	i++;
    }

    return;
    }


_Type  EnvDomainToType(domain)
_Domain domain;

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
    if ((int) domain < (int) hs_c_domain_min || (int) domain >= (int) hs_c_domain_max)
	return (_Type) _NullObject;
    else
	return DomainToType[(int) domain];
    }


_Domain  EnvTypeToDomain(type)
_Type type;

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
    int domain;

    for (domain = (int) hs_c_domain_min; domain < (int) hs_c_domain_max; domain++)
	if (DomainToType[domain] == type)
	    return (_Domain) domain;

    return (_Domain) hs_c_domain_unknown;
    }


_Boolean  EnvIsDomain(domain1, domain2)
_Domain domain1;
 _Domain domain2;

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
    _Type type1, type2;

    /*
    **  If the two domains are identical, they are equal.
    */

    if (domain1 == domain2)
	return _True;

    /*
    **  The domains may be Objects, so we need to check their types.
    */

    type1 = DomainToType[(int) domain1];
    type2 = DomainToType[(int) domain2];

    /*
    **  If either domain is not an Object type, they are not equal.
    */

    if (type1 == (_Type) _NullObject || type2 == (_Type) _NullObject)
	return _False;

    /*
    **  If the types are identical, the domains are equal.
    */

    if (type1 == type2)
	return _True;

    /*
    **  If the second domain is a subtype of the first domain, they are equal.
    */

    while ((type2 = _Supertype_of(type2)) != (_Type) _NullObject)
	if (type1 == type2)
	    return _True;

    return _False;
    }


_Boolean  EnvIsValidObject(object)
_Object object;

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
    if (object == (_Object) _NullObject)
	return _False;

    if (!_IsValidType(_Type_of(object)))
	return _False;

    return _True;
    }


_Boolean  EnvIsValidType(type)
_Type type;

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
    if (type == (_Type) _NullObject)
	return _False;

    return _True;
    }


_Integer  EnvCompareValue(value1, value2, domain)
_AnyValuePtr value1;
 _AnyValuePtr value2;

    _Domain domain;

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

    /*
    **  Treat all subtypes of Object as type Object.
    */

    if (_IsDomain(hs_c_domain_object, domain))
	domain = (_Domain) hs_c_domain_object;

    /*
    **	Depending on the domain, compare the values appropriately.
    */

    switch (domain) {
	case hs_c_domain_integer :
	    if (*((_Integer *) value1) == *((_Integer *) value2))
		answer = 0;
	    else if (*((_Integer *) value1) < *((_Integer *) value2))
		answer = -1;
	    else
		answer = 1;

	    break;

	case hs_c_domain_boolean :
	    if (*((_Boolean *) value1) == *((_Boolean *) value2))
		answer = 0;
	    else if (*((_Boolean *) value1) < *((_Boolean *) value2))
		answer = -1;
	    else
		answer = 1;

	    break;

	case hs_c_domain_routine :
	    if (*((_Callback *) value1) == *((_Callback *) value2))
		answer = 0;
	    else
		answer = 2;

	    break;

	case hs_c_domain_any_ptr :
	    if (*((_AnyPtr *) value1) == *((_AnyPtr *) value2))
		answer = 0;
	    else if (*((_AnyPtr *) value1) < *((_AnyPtr *) value2))
		answer = -1;
	    else
		answer = 1;

	    break;

	case hs_c_domain_comp_string :
	    answer = _CompareCString(*((_CString *) value1),
		*((_CString *) value2));
	    break;

	case hs_c_domain_string :
	    answer = _CompareString(*((_String *) value1),
		*((_String *) value2));
	    break;

	case hs_c_domain_object :
	    if (*((_Object *) value1) == *((_Object *) value2))
		answer = 0;
	    else
		answer = 2;

	    break;

	default :
	    _Raise(inv_domain);
	    break;
    }

    return answer;
    }


_Void  EnvCopyValue(source, target, domain)
_AnyValuePtr source;
 _AnyValuePtr target;
 _Domain domain;

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
    ** Primitive datatypes (e.g, integer) are copied.  For the objects, only a
    ** reference to the object is copied except for the List object.
    */

    if ((_IsDomain(hs_c_domain_object, domain)) &&
	(!(_IsDomain(hs_c_domain_list, domain))))
	    domain = (_Domain) hs_c_domain_object;

    /*
    **	Depending on its domain, copy the value appropriately.
    */

    switch (domain) {
	case hs_c_domain_integer :
	    *((_Integer *) target) = *((_Integer *) source);
	    break;

	case hs_c_domain_boolean :
	    *((_Boolean *) target) = *((_Boolean *) source);
	    break;

	case hs_c_domain_routine :
	    *((_Callback *) target) = *((_Callback *) source);
	    break;

	case hs_c_domain_any_ptr :
	    *((_AnyPtr *) target) = *((_AnyPtr *) source);
	    break;

	case hs_c_domain_comp_string :
	    if (*((_CString *) source) == (_CString) _NullObject)
		*((_CString *) target) = (_CString) _NullObject;
	    else {
		if (!_IsValidCString(*((_CString *) source)))
		    _Raise(inv_comp_string);
		*((_CString *) target) = _CopyCString(*((_CString *) source));
	    }
	    break;

	case hs_c_domain_string :
	    if (*((_String *) source) == (_String) _NullObject)
		*((_String *) target) = (_String) _NullObject;
	    else
		*((_String *) target) = _CopyString(*((_String *) source));
	    break;

	case hs_c_domain_list :
	    if (*((_Object *) source) == (_Object) _NullObject)
		*((_Object *) target) = (_Object) _NullObject;
	    else
		*((_Object *) target) = (_Object) _Copy(*((_Object *) source));
	    break;

	case hs_c_domain_object :
	    *((_Object *) target) = *((_Object *) source);
	    break;

	case hs_c_domain_lwk_object :
	    *((lwk_object *) target) = *((lwk_object *) source);
	    break;

	default :
	    _Raise(inv_domain);
	    break;
    }

    return;
    }


_Void  EnvMoveValue(source, target, domain)
_AnyValuePtr source;
 _AnyValuePtr target;
 _Domain domain;

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
    **  Treat all subtypes of Object as type Object.
    */

    if (_IsDomain(hs_c_domain_object, domain))
	domain = (_Domain) hs_c_domain_object;

    /*
    **	Depending on its domain, move the value appropriately.
    */

    switch (domain) {
	case hs_c_domain_integer :
	    *((_Integer *) target) = *((_Integer *) source);
	    break;

	case hs_c_domain_boolean :
	    *((_Boolean *) target) = *((_Boolean *) source);
	    break;

	case hs_c_domain_routine :
	    *((_Callback *) target) = *((_Callback *) source);
	    break;

	case hs_c_domain_any_ptr :
	    *((_AnyPtr *) target) = *((_AnyPtr *) source);
	    break;

	case hs_c_domain_comp_string :
	    *((_CString *) target) = *((_CString *) source);
	    break;

	case hs_c_domain_string :
	    *((_String *) target) = *((_String *) source);
	    break;

	case hs_c_domain_object :
	    *((_Object *) target) = *((_Object *) source);
	    break;

	case hs_c_domain_lwk_object :
	    *((lwk_object *) target) = *((lwk_object *) source);
	    break;

	default :
	    _Raise(inv_domain);
	    break;
    }

    return;
    }


_Void  EnvListValue(value, list, domain)
_AnyValuePtr value;
 _List *list;
 _Domain domain;

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
    **  Treat all subtypes of Object as type Object.
    */

    if (_IsDomain(hs_c_domain_object, domain))
	domain = (_Domain) hs_c_domain_object;

    /*
    **	Depending on its domain, copy the value appropriately.
    */

    switch (domain) {
	case hs_c_domain_integer :
	    *list = (_List) _CreateList(_TypeList, hs_c_domain_integer, 1);
	    _AddElement(*list, hs_c_domain_integer, value, _True);
	    break;

	case hs_c_domain_boolean :
	    *list = (_List) _CreateList(_TypeList, hs_c_domain_boolean, 1);
	    _AddElement(*list, hs_c_domain_boolean, value, _True);
	    break;

	case hs_c_domain_routine :
	    *list = (_List) _CreateList(_TypeList, hs_c_domain_routine, 1);
	    _AddElement(*list, hs_c_domain_routine, value, _True);
	    break;

	case hs_c_domain_any_ptr :
	    *list = (_List) _CreateList(_TypeList, hs_c_domain_any_ptr, 1);
	    _AddElement(*list, hs_c_domain_any_ptr, value, _True);
	    break;

	case hs_c_domain_comp_string :
	    *list = (_List) _CreateList(_TypeList, hs_c_domain_comp_string, 1);
	    _AddElement(*list, hs_c_domain_comp_string, value, _True);
	    break;

	case hs_c_domain_string :
	    *list = (_List) _CreateList(_TypeList, hs_c_domain_string, 1);
	    _AddElement(*list, hs_c_domain_string, value, _True);
	    break;

	case hs_c_domain_object :
	    *list = (_List) _CreateList(_TypeList, hs_c_domain_object, 1);
	    _AddElement(*list, hs_c_domain_object, value, _True);
	    break;

	default :
	    _Raise(inv_domain);
	    break;
    }

    return;
    }


_Void  EnvClearValue(value, domain)
_AnyValuePtr value;
 _Domain domain;

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
    **  Treat all subtypes of Object as type Object.
    */

    if (_IsDomain(hs_c_domain_object, domain))
	domain = (_Domain) hs_c_domain_object;

    /*
    **	Depending on its domain, clear the value appropriately.
    */

    switch (domain) {
	case hs_c_domain_integer :
	    *((_Integer *) value) = 0;
	    break;

	case hs_c_domain_boolean :
	    *((_Boolean *) value) = _False;
	    break;

	case hs_c_domain_routine :
	    *((_Callback *) value) = (_Callback) _NullObject;
	    break;

	case hs_c_domain_any_ptr :
	    *((_AnyPtr *) value) = 0;
	    break;

	case hs_c_domain_comp_string :
	    *((_CString *) value) = (_CString) _NullObject;
	    break;

	case hs_c_domain_string :
	    *((_String *) value) = (_String) _NullObject;
	    break;

	case hs_c_domain_object :
	    *((_Object *) value) = (_Object) _NullObject;
	    break;

	default :
	    _Raise(inv_domain);
	    break;
    }

    return;
    }


_Void  EnvDeleteValue(value, domain)
_AnyValuePtr value;
 _Domain domain;

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
    ** Primitive datatypes (e.g, integer) are copied.  For the objects, only a
    ** reference to the object is copied except for the List object.  The types
    ** must be freed accordingly.
    */

    if ((_IsDomain(hs_c_domain_object, domain)) &&
	(!(_IsDomain(hs_c_domain_list, domain))))
	    domain = (_Domain) hs_c_domain_object;

    /*
    **	Depending on its domain, delete the value appropriately.
    */

    switch (domain) {
	case hs_c_domain_integer :
	case hs_c_domain_boolean :
	case hs_c_domain_routine :
	case hs_c_domain_any_ptr :
	case hs_c_domain_object :
	case hs_c_domain_lwk_object:
	    break;

	case hs_c_domain_comp_string :
	    if (*((_CString *) value) != (_CString) _NullObject)
		_DeleteCString(((_CString *) value));
	    break;

	case hs_c_domain_string :
	    if (*((_String *) value) != (_String) _NullObject)
		_DeleteString(((_String *) value));
	    break;

        case hs_c_domain_list :
            if (*((_Object *) value) != (_Object) _NullObject)
                _Delete(((_Object *) value));
            break;

	default :
	    _Raise(inv_domain);
	    break;
    }

    return;
    }


_Void  EnvSetSingleValProp(property, property_domain, value_domain, value, flag, value_is_list)
_AnyValuePtr property;
 _Domain property_domain;

    _Domain value_domain;
 _AnyValuePtr value;
 _SetFlag flag;

    _Boolean value_is_list;

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
    **  Deal with each type of value setting individually.
    */

    switch (flag) {
	case hs_c_clear_property :
	    /*
	    **	Delete the old value if there was one, then set the property to
	    **	the appropriate null value.
	    */

	    _DeleteValue(property, property_domain);
	    _ClearValue(property, property_domain);

	    break;

	case hs_c_add_property :
	    /*
	    **  Can not add a value to a single-valued property.
	    */

	    _Raise(single_valued_property);
	    break;

	case hs_c_set_property :
	    /*
	    **  Make sure we have a valid value
	    */

	    if (value == (_AnyValuePtr) _NullObject)
		_Raise(inv_argument);

	    if (value_is_list) {
		/*
		**  Make sure we have a valid List
		*/

		if (*((_List *) value) == (_List) _NullObject)
		    _Raise(inv_argument);

		/*
		**  Make sure its elements are of the correct domain.
		*/

		if (!_IsDomain(property_domain, value_domain))
		    _Raise(inv_domain);

		/*
		**  Delete any old value, then set the property to the first
		**  element of the list.
		*/

		_DeleteValue(property, property_domain);

		_SelectElement(*((_List *) value), (_Integer) 0,
		    property_domain, property);
	    }
	    else {
		/*
		**  Make sure that the value is of the correct domain.
		*/

		if (!_IsDomain(property_domain, value_domain))
		    _Raise(inv_domain);

		/*
		**  Delete any old value, then set the value appropriately.
		*/

		_DeleteValue(property, property_domain);

		_CopyValue(value, property, property_domain);
	    }

	    break;

	/*
	**  Remove each occurance of the given value(s) from the property set.
	*/

	case hs_c_remove_property :
	    /*
	    **  Make sure we have a valid value
	    */

	    if (value == (_AnyValuePtr) _NullObject)
		_Raise(inv_argument);

	    if (value_is_list) {
		/*
		**  Make sure we have a valid List
		*/

		if (*((_List *) value) == (_List) _NullObject)
		    _Raise(inv_argument);

		/*
		**  Iterate over the list, and delete the property if its value
		**  appears in the list.
		*/

		_Iterate(*((_List *) value), property_domain,
		    (_Closure) property, RemoveProperty);
	    }
	    else {
		/*
		**  If the value match matches, clear the property value.
		*/

		RemoveProperty(property, (_List) _NullObject, value_domain,
		    value);
	    }

	    break;

	case hs_c_delete_property :
	    /*
	    **  Can't delete any of the base properties.
	    */

	    _Raise(property_not_deleted);
	    break;

	default :
	    _Raise(inv_set_operation);
    }

    return;
    }


_Void  EnvSetMultiValProp(list, property_domain, value_domain, value, flag, value_is_list, property_is_set)
_List *list;
 _Domain property_domain;

    _Domain value_domain;
 _AnyValuePtr value;
 _SetFlag flag;

    _Boolean value_is_list;
 _Boolean property_is_set;

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
    _List old_list;
    _Integer tmp_domain;

    /*
    **	If Set Value is called for a multi-valued property, the call could be
    **	providing either a single value (which must be put in a list/set), or a
    **	List.  We have to check for this latter case, and turn it into the
    **	equivalent of a SetValueList.
    */

    if (!value_is_list)
	if (_IsDomain(hs_c_domain_list, value_domain))
	    value_is_list = _True;

    /*
    **	If the value is a list, we are really interested in the list domain,
    **	not the value domain.
    */

    if (value_is_list)
	if (value != (_AnyValuePtr) _NullObject)
	    if (*((_List *) value) != (_List) _NullObject) {
		_GetValue(*((_List *) value), _P_Domain, hs_c_domain_integer,
		    &tmp_domain);

		value_domain = (_Domain) tmp_domain;
	    }

    /*
    **  Now, deal with each type of value setting individually.
    */

    switch (flag) {
	case hs_c_clear_property :
	    /*
	    **	Delete the old value if there was one, then set the property to
	    **	the appropriate null value.
	    */

	    _DeleteValue(list, hs_c_domain_list);
	    _ClearValue(list, hs_c_domain_list);

	    break;

	case hs_c_add_property :
	    /*
	    **  Make sure we have a valid value
	    */

	    if (value == (_AnyValuePtr) _NullObject)
		_Raise(inv_argument);

	    if (value_is_list) {
		/*
		**  Make sure we have a valid List
		*/

		if (*((_List *) value) == (_List) _NullObject)
		    _Raise(inv_argument);

		/*
		**  Check that the value has the proper domain
		*/

		if (!_IsDomain(property_domain, value_domain))
		    _Raise(inv_domain);

		/*
		**  Create the property list if necessary
		*/

		if (*list == (_List) _NullObject) {
		    _Integer size;

		    _GetValue(*((_List *) value), _P_ElementCount,
			hs_c_domain_integer, &size);

		    if (property_is_set)
/* No sets			*list = (_List) _CreateSet(_TypeSet, property_domain,
			    size); */;
		    else
			*list = (_List) _CreateList(_TypeList, property_domain,
			    size);
		}

		/*
		**  Append the elements in the value list to the property list
		*/

		_AppendElements(*list, *((_List *) value));
	    }
	    else {
		/*
		**  Check that the value has the proper domain
		*/

		if (!_IsDomain(property_domain, value_domain))
		    _Raise(inv_domain);

		/*
		**  Create a property list if necessary
		*/

		if (*list == (_List) _NullObject) {
		    if (property_is_set)
/* No sets			*list = (_List) _CreateSet(_TypeSet, property_domain, 0);
						    */;
		    else
			*list = (_List) _CreateList(_TypeList, property_domain,0);
		}

		/*
		**  Add the value to the property list
		*/

		_AddElement(*list, value_domain, value, _True);
	    }

	    break;

	case hs_c_set_property :
	    /*
	    **  Make sure we have a valid value
	    */

	    if (value == (_AnyValuePtr) _NullObject)
		_Raise(inv_argument);

	    /*
	    **  Save old property List
	    */

	    old_list = *list;

	    if (value_is_list) {
		if (*((_List *) value) != (_List) _NullObject) {
		    /*
		    **  Make sure the elements are of the correct domain.
		    */

		    if (!_IsDomain(property_domain, value_domain))
			_Raise(inv_domain);

/* No sets		    if (property_is_set)
			if (!_IsType(*((_List *) value), _TypeSet))
			    _Raise(inv_domain);  */
		}

		/*
		**  Set property value.
		*/

		_CopyValue(value, list, hs_c_domain_list);
	    }
	    else {
		/*
		**  Make sure that the value is of the correct domain.
		*/

		if (!_IsDomain(property_domain, value_domain))
		    _Raise(inv_domain);

		/*
		**  Create a list/set with it as the single value.
		*/

/*		if (property_is_set)
		    *list = (_List) _CreateSet(_TypeSet, property_domain, 1);
		else */
		    *list = (_List) _CreateList(_TypeList, property_domain, 1);

		_AddElement(*list, value_domain, value, _True);
	    }

	    /*
	    **  If there was an old property List, delete it
	    */

	    if (old_list != (_List) _NullObject)
		_Delete(old_list);

	    break;

	case hs_c_remove_property :
	    /*
	    **  Make sure we have a valid value
	    */

	    if (value == (_AnyValuePtr) _NullObject)
		_Raise(inv_argument);

	    if (value_is_list) {
		/*
		**  Make sure we have a valid List
		*/

		if (*((_List *) value) == (_List) _NullObject)
		    _Raise(inv_argument);

		/*
		**  A valid list was provided.  Iterate over the list, and
		**  remove all occurances of each element.
		*/

		if (*list != (_List) _NullObject)
		    _Iterate(*((_List *) value), property_domain,
			(_Closure) *list, DeleteElementFromList);
	    }
	    else {
		/*
		**  Delete any occurance of the value from the list.
		*/

		if (*list != (_List) _NullObject)
		    _DeleteElement(*list, value_domain, value);
	    }

	    break;

	case hs_c_delete_property :
	    /*
	    **  Can't delete a base property
	    */

	    _Raise(property_not_deleted);
	    break;

	/*
	**  Must have given a bad SetFlag value.
	*/

	default :
	    _Raise(inv_set_operation);
    }

    return;
    }

static _Termination  DeleteElementFromList(property_list, value_list, domain, value)
_List property_list;
 _List value_list;

    _Domain domain;
 _AnyValuePtr value;

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
    **  Simply delete all occurances of the string from the list.
    */

    _DeleteElement(property_list, domain, value);

    return ((_Termination) 0);
    }

static _Termination  RemoveProperty(property, element_list, domain, value)
_AnyValuePtr property;
 _List element_list;

    _Domain domain;
 _AnyValuePtr value;

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
    _Termination answer;

    /*
    **  If the value matches, delete any old value, then clear the property.
    */

    if (_CompareValue(property, value, domain) == 0) {
        _DeleteValue(property, domain);
        _ClearValue(property, domain);
        answer = (_Termination) 1;
    }
    else
        answer = (_Termination) 0;

    return answer;
    }


_String  EnvStatusToString(code)
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
    int i;

    /*
    **	Look for the Status Code in the table, and return its corresponding
    **	string form
    */

    for (i = 0; i < (int) _StatusCode(max); i++)
	if (StatusToString[i].status == code)
	    return StatusToString[i].string;
	    
    /*
    **	If it is not there, turn Status Code "unknown" into a string
    */

    for (i = 0; i < (int) _StatusCode(max); i++)
	if (StatusToString[i].status == _StatusCode(unknown))
	    return StatusToString[i].string;

    /*			
    **	As a last resort return a literal "unknown"
    */

    return (_String) _UnknownStatusString;
    }


_String  EnvStatusToResource(code)
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
    int i;
    
    /*
    **	Look for the Status Code in the table, and return its corresponsing
    **	DRM Resource
    */
    
    for (i = 0; i < (int) _StatusCode(max); i++)
	if (StatusToResource[i].status == code)
	    return StatusToResource[i].string;
	    
    /*
    **	If it is not there, turn Status Code "unknown" into a resource
    */
    
    for (i = 0; i < (int) _StatusCode(max); i++)
	if (StatusToResource[i].status == _StatusCode(unknown))
	    return StatusToResource[i].string;
	    
    /*
    **	As a last resort return a literal "unknown"
    */
    
    return (_String) _UnknownStatusString;
    }


_Boolean  EnvIsPrivateObject(persistent)
lwk_persistent persistent;

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
    lwk_status	    status;
    lwk_boolean	    dummy;

    /*
    **  Try to get LinkWorks Manager's private property
    */

    status = lwk_get_value(persistent, _HsPrivateProperty, lwk_c_domain_boolean,
	(lwk_any_pointer) &dummy);

    switch(status) {

	case lwk_s_success :
	    return(_True);

	case lwk_s_no_such_property :
	    return(_False);

	default :
	    _Raise(get_value_failed);  /* get value on private prop failed */
    }
    
    return(_False);
    }
    

_Void  EnvSetPrivateProperty(persistent, linkbase)
lwk_persistent persistent;

    lwk_linkbase linkbase;

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
    lwk_status	    status;
    lwk_boolean	    ignored_value;

    ignored_value = _True;

    status = lwk_set_value(persistent, _HsPrivateProperty, lwk_c_domain_boolean,
	(lwk_any_pointer) &ignored_value, lwk_c_set_property);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(set_value_failed); /* Set of private property failed */
    }

    status = lwk_store(persistent, linkbase);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(store_failed); /* store failed */
    }

    return;
    }


_Date  EnvNowToDate()
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
    time_t b_time;

    time(&b_time);

    return _TimeToDate(b_time);
    }


_Date  EnvTimeToDate(b_time)
time_t b_time;

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
    tm_t *tm;
    char string[20];

    tm = localtime(&b_time);

    sprintf(string, "%04d%02d%02d%02d%02d%02dZ", 1900 + tm->tm_year,
        tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

    return (_Date) _CopyString((_String) string);
    }

