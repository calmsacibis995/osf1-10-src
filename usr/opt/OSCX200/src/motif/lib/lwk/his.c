/*
** COPYRIGHT (c) 1988, 1989, 1990, 1991 BY
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
**  Version: X0.1
**
**  Abstract:
**	General utility routines used throughout the Services
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
#include "ddisencd.h"
#else /* !MSDOS */
#include "his_include.h"
#include "lwk_abstract_objects.h"
#include "his_ddis_encoding.h"
#endif /* MSDOS */

/*
**  Macro Definitions
*/

#define _StatusTablePart1 \
    _X_(lwk_s_unknown, "StsUnknown", "Unknown error status", ""), \
    _X_(lwk_s_success, "StsSuccess", "Success", ""), \
    _X_(lwk_s_failure, "StsFailure", "Failure", ""), \
    _X_(lwk_s_bad_logic, "StsBadLogic", "Bad logic error", ""), \
    _X_(lwk_s_not_yet_impl, "StsNotYetImplemented", "Not yet implemented", ""), \
    _X_(lwk_s_alloc_error, "StsAllocationError", "Memory allocation error", ""), \
    _X_(lwk_s_dealloc_error, "StsDeallocationError", \
	"Memory deallocation error", ""), \
    _X_(lwk_s_inv_object, "StsInvalidObject", "Invalid Object", ""), \
    _X_(lwk_s_inv_argument, "StsInvalidArgument", "Invalid Argument", ""), \
    _X_(lwk_s_inv_operation, "StsInvalidOperation", "Invalid operation", ""), \
    _X_(lwk_s_no_such_property, "StsNoSuchProperty", "No such property", ""), \
    _X_(lwk_s_inv_domain, "StsInvalidDomain", "Invalid Domain", ""), \
    _X_(lwk_s_list_empty, "StsListEmpty", "List empty", ""), \
    _X_(lwk_s_property_is_readonly, "StsPropertyIsReadOnly", \
	"Property is read only", ""), \
    _X_(lwk_s_dupl_element, "StsDuplicateElement", "Duplicate element", ""), \
    _X_(lwk_s_no_such_element, "StsNoSuchElement", "No such element", ""), \
    _X_(lwk_s_inv_set_operation, "StsInvalidSetOpr", \
	"Invalid Set Value operation", ""), \
    _X_(lwk_s_property_not_deleted, "StsPropertyNotDeleted", \
	"Property not deleted", ""), \
    _X_(lwk_s_single_valued_property, "StsSingleValuedProperty", \
	"Single-valued property", ""), \
    _X_(lwk_s_cannot_move_surrogate, "StsCanNotMoveSurrogate", \
	"Cannot move Surrogate", ""), \
    _X_(lwk_s_cannot_move_link, "StsCanNotMoveConnection", \
	"Cannot move Link", ""),

#define _StatusTablePart2 \
    _X_(lwk_s_cannot_move_step, "StsCanNotMoveStep", "Cannot move Step", ""), \
    _X_(lwk_s_cannot_delete_surrogate, "StsCanNotDeleteSurrogate", \
	"Cannot delete Surrogate", ""), \
    _X_(lwk_s_inv_date, "StsInvalidDate", "Invalid Date", ""), \
    _X_(lwk_s_inv_ddif_string, "StsInvalidDDIFString", \
	"Invalid DDIF String", ""), \
    _X_(lwk_s_inv_string, "StsInvalidString", "Invalid String", ""), \
    _X_(lwk_s_drm_open_error, "StsDRMHierarchyOpenError", \
	"DRM Hierarchy open error", ""), \
    _X_(lwk_s_drm_fetch_error, "StsDRMHierarchyFetchError", \
	"DRM Hierarchy fetch error", ""), \
    _X_(lwk_s_drm_close_error, "StsDRMHierarchyCloseError", \
	"DRM Hierarchy close error", ""), \
    _X_(lwk_s_get_surrogate_cb_error, "StsGetSurrogateCBError", \
	"Get Surrogate callback error", ""), \
    _X_(lwk_s_create_surrogate_cb_error, "StsCreateSurrogateCBError", \
	"Create Surrogate callback error", ""), \
    _X_(lwk_s_apply_cb_error, "StsApplyCBError", "Apply callback error", ""), \
    _X_(lwk_s_close_view_cb_error, "StsCloseViewCBError", \
	"Close view callback error", ""), \
    _X_(lwk_s_complete_link_cb_error, "StsConnectCBError", \
	"Connect callback error", ""), \
    _X_(lwk_s_env_change_cb_error, "StsCurrencyChangeCBError", \
	"Currency change callback error", ""), \
    _X_(lwk_s_inv_encoding, "StsInvalidEncoding", "Invalid encoding", ""), \
    _X_(lwk_s_internal_encoding_error, "StsInternalEncodingError", \
	"Internal encoding error", ""), \
    _X_(lwk_s_internal_decoding_error, "StsInternalDecodingError", \
	"Internal decoding error", ""), \
    _X_(lwk_s_inv_widget_id, "StsInvalidWidgetId", "Invalid Widget Id", ""), \
    _X_(lwk_s_stored_elsewhere, "StsStoredElsewhere", \
	"Object already stored elsewhere", ""), \
    _X_(lwk_s_no_such_linkbase, "StsNoSuchRepository", \
	"No such LinkBase", ""), \
    _X_(lwk_s_db_not_open, "StsDBNotOpen", "Database not open", ""),

#define _StatusTablePart3 \
    _X_(lwk_s_no_such_object, "StsNoSuchObject", "No such Object", ""), \
    _X_(lwk_s_db_read_error, "StsDBReadError", "Database read error", ""), \
    _X_(lwk_s_db_write_error, "StsDBWriteError", "Database write error", ""), \
    _X_(lwk_s_db_lock_error, "StsDBLockError", "Database lock error", ""), \
    _X_(lwk_s_db_commit_error, "StsDBCommitError", \
	"Database commit error", ""), \
    _X_(lwk_s_db_version_error, "StsDBVersionError", \
	"Database version error", ""), \
    _X_(lwk_s_linkbase_in_use, "StsRepositoryInUse", "LinkBase in use", ""), \
    _X_(lwk_s_not_linked, "StsNotConnected", "Not Connected", ""), \
    _X_(lwk_s_no_recording_linknet, "StsNoCurrentNetwork", \
	"No Current LinkNet", ""), \
    _X_(lwk_s_no_pending_source, "StsNoCurrentSource", "No Pending Source", ""), \
    _X_(lwk_s_no_pending_target, "StsNoCurrentTarget", "No Pending Target", ""), \
    _X_(lwk_s_no_history, "StsNoCurrentHistory", \
	"No Current History", ""), \
    _X_(lwk_s_history_is_empty, "StsHistoryIsEmpty", \
	"The Current History is empty", ""), \
    _X_(lwk_s_no_active_cpath, "StsNoCurrentCPath", \
	"No Current Composite Path", ""), \
    _X_(lwk_s_no_more_steps, "StsNoMoreSteps", \
	"No More Steps", ""), \
    _X_(lwk_s_inv_query_expr, "StsInvalidQueryExpression", \
	"Invalid Query expression", ""), \
    _X_(lwk_s_abstract_object, "StsAbstractObject", \
	"Cannot create Abstract Object", ""), \
    _X_(lwk_s_highlight_error, "StsHighlightError", "Highlight error", ""), \
    _X_(lwk_s_go_to_error, "StsGoToError", "Go To error", ""), \
    _X_(lwk_s_visit_error, "StsVisitError", "Visit error", ""), \
    _X_(lwk_s_show_history_error, "StsShowHistoryError", \
	"Show Historyerror", ""), \

#define _StatusTablePart4 \
    _X_(lwk_s_step_forward_error, "StsStepForwardError", \
	"Step Forward error", ""), \
    _X_(lwk_s_go_back_error, "StsUndoError", "Goback error", ""), \
    _X_(lwk_s_start_link_error, "StsSetSourceError", "Set Source error", ""), \
    _X_(lwk_s_complete_link_error, "StsConnectError", "Connect error", ""), \
    _X_(lwk_s_annotate_error, "StsAnnotateError", "Annotate error", ""), \
    _X_(lwk_s_show_link_error, "StsShowConnectionError", \
	"Show Link error", ""), \
    _X_(lwk_s_delete_link_error, "StsDeleteConnectionError", \
	"Delete Link error", ""), \
    _X_(lwk_s_show_links_error, "StsShowConnectionsError", \
	"Show Links error", ""), \
    _X_(lwk_s_link_update_error, "StsConnectUpdateError", \
	"Link Update error", ""), \
    _X_(lwk_s_show_step_error, "StsShowStepError", "Show Step error", ""), \
    _X_(lwk_s_delete_step_error, "StsDeleteStepError", "Delete Step error", ""), \
    _X_(lwk_s_step_update_error, "StsStepUpdateError", "Step Update error", ""), \
    _X_(lwk_s_unexpected_error, "StsUnexpectedError", "Unexpected error", ""), \
    _X_(lwk_s_db_access_error, "StsDBAccessError", "LinkBase access error", ""), \
    _X_(lwk_s_apply_error, "StsApplyError", "Application startup error", ""), \
    _X_(lwk_s_invocation_error, "StsInvocationError", \
	"Application invocation error", ""), \
    _X_(lwk_s_not_registered, "StsNotRegistered", \
	"Application not registered", "")

/*
**  Type Definitions
*/

typedef struct __StatusToStringEntry {
	    _Status status;
	    _String string;
	} _StatusToStringEntry;

typedef struct __TypeToDomainEntry {
	    _Type type;
	    _Domain domain;
	} _TypeToDomainEntry;

/*
**  Forward Routine Declarations
*/

_DeclareFunction(static _Termination _EntryPt DeleteElementFromList,
    (_Closure property_list, _List value_list, _Domain domain, _AnyPtr value));
_DeclareFunction(static _Termination _EntryPt RemoveProperty,
    (_Closure property, _List element_list, _Domain domain, _AnyPtr value));
_DeclareFunction(static _DDISstatus CDA_CALLBACK ExtendDDISBuffer,
    (_VaryingStringPtr vstring, _DDISsizePtr size, _DDISdata pointer,
	_DDISsizePtr new_size, _DDISdataPtr new_pointer));
_DeclareFunction(static _DDISstatus CDA_CALLBACK GetDDISBuffer,
    (_VaryingString vstring, _DDISsizePtr size, _DDISdataPtr pointer));
_DeclareFunction(static _DDISstatus CDA_CALLBACK AllocateDDISMem,
    (_DDISsizePtr size, _DDIStablePtr pointer, _DDISuserparam user_data));
_DeclareFunction(static _DDISstatus CDA_CALLBACK DeallocateDDISMem,
    (_DDISsizePtr size, _DDIStablePtr pointer, _DDISuserparam user_data));
_DeclareFunction(static void DateToTm, (_Date date, _OSTmPtr tm));

/*
**  Global Data Definitions
*/

/*
**  External Data Declarations
*/

/*
**  Static Data Definitions
*/

/*
**  Memory Usage Statistics
*/

static _Integer TotalAllocations;
static _Integer TotalAllocatedMemory;
static _Integer TotalReallocations;
static _Integer TotalDeallocations;
static _Integer TotalDeallocatedMemory;
static _Integer PeakAllocatedMemory;

/*
**  Domain to Type Conversion Table
*/

#ifdef MSDOS
static _TypeToDomainEntry _Constant TypeToDomain[lwk_c_domain_max] = {
    {(_Type) _NullObject, lwk_c_domain_unknown},
    {(_Type) _NullObject, lwk_c_domain_integer},
    {(_Type) _NullObject, lwk_c_domain_float},
    {(_Type) _NullObject, lwk_c_domain_date},
    {(_Type) _NullObject, lwk_c_domain_string},
    {(_Type) _NullObject, lwk_c_domain_ddif_string},
    {(_Type) _NullObject, lwk_c_domain_boolean},
    {(_Type) _NullObject, lwk_c_domain_routine},
    {(_Type) _NullObject, lwk_c_domain_closure},
    {&_TypeObjectInstance, lwk_c_domain_object},
    {&_TypeListInstance, lwk_c_domain_list},
    {&_TypeSetInstance, lwk_c_domain_set},
    {&_TypePropertyInstance, lwk_c_domain_property},
    {&_TypeObjectIdInstance, lwk_c_domain_object_id},
    {(_Type) _NullObject, lwk_c_domain_object_desc},
    {(_Type) _NullObject, lwk_c_domain_ui},
    {(_Type) _NullObject, lwk_c_domain_dxm_ui},
    {&_TypePersistentInstance, lwk_c_domain_persistent},
    {&_TypeSurrogateInstance, lwk_c_domain_surrogate},
    {&_TypeLinkInstance, lwk_c_domain_link},
    {&_TypeLinknetInstance, lwk_c_domain_linknet},
    {(_Type) _NullObject, lwk_c_domain_comp_linknet},
    {(_Type) _NullObject, lwk_c_domain_step},
    {(_Type) _NullObject, lwk_c_domain_path},
    {(_Type) _NullObject, lwk_c_domain_comp_path},
    {&_TypeLinkbaseInstance, lwk_c_domain_linkbase},
    {(_Type) _NullObject, lwk_c_domain_environment_state}
};
#else
static _TypeToDomainEntry _Constant TypeToDomain[lwk_c_domain_max] = {
    {(_Type) _NullObject, lwk_c_domain_unknown},
    {(_Type) _NullObject, lwk_c_domain_integer},
    {(_Type) _NullObject, lwk_c_domain_float},
    {(_Type) _NullObject, lwk_c_domain_date},
    {(_Type) _NullObject, lwk_c_domain_string},
    {(_Type) _NullObject, lwk_c_domain_ddif_string},
    {(_Type) _NullObject, lwk_c_domain_boolean},
    {(_Type) _NullObject, lwk_c_domain_routine},
    {(_Type) _NullObject, lwk_c_domain_closure},
    {&_TypeObjectInstance, lwk_c_domain_object},
    {&_TypeListInstance, lwk_c_domain_list},
    {&_TypeSetInstance, lwk_c_domain_set},
    {&_TypePropertyInstance, lwk_c_domain_property},
    {&_TypeObjectIdInstance, lwk_c_domain_object_id},
    {&_TypeObjectDescInstance, lwk_c_domain_object_desc},
    {&_TypeUiInstance, lwk_c_domain_ui},
    {&_TypeDXmUiInstance, lwk_c_domain_dxm_ui},
    {&_TypePersistentInstance, lwk_c_domain_persistent},
    {&_TypeSurrogateInstance, lwk_c_domain_surrogate},
    {&_TypeLinkInstance, lwk_c_domain_link},
    {&_TypeLinknetInstance, lwk_c_domain_linknet},
    {&_TypeCompLinknetInstance, lwk_c_domain_comp_linknet},
    {&_TypeStepInstance, lwk_c_domain_step},
    {&_TypePathInstance, lwk_c_domain_path},
    {&_TypeCompPathInstance, lwk_c_domain_comp_path},
    {&_TypeLinkbaseInstance, lwk_c_domain_linkbase},
    {&_TypeDXmEnvStateInstance, lwk_c_domain_environment_state}
};
#endif

/*
**  Status Code to String Conversion Table
*/

#define _X_(Status, Resource, String, HelpKey) {(Status), (String)}

static _StatusToStringEntry _Constant StatusToString[_StatusCode(max)] = {
    _StatusTablePart1
    _StatusTablePart2
    _StatusTablePart3
    _StatusTablePart4
};

/*
**  Status Code to Resource Name Conversion Table
*/

#undef  _X_
#define _X_(Status, Resource, String, HelpKey) {(Status), (Resource)}

static _StatusToStringEntry _Constant StatusToResource[_StatusCode(max)] = {
    _StatusTablePart1
    _StatusTablePart2
    _StatusTablePart3
    _StatusTablePart4
};

/*
**  Status Code to Help Key Name Conversion Table
*/

#undef  _X_
#define _X_(Status, Resource, String, HelpKey) {(Status), (HelpKey)}

static _StatusToStringEntry _Constant StatusToHelpKeyResource[_StatusCode(max)] = {
    _StatusTablePart1
    _StatusTablePart2
    _StatusTablePart3
    _StatusTablePart4
};


void  Lwk()
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


_AnyPtr  LwkReference(reference)
_AnyPtr reference;

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
    return reference;
    }


void  LwkMemoryStatistics()
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
    _CharPtr cp;
    _OSFile fp;
    _Integer difference;

    cp = (_CharPtr) getenv("HIS_STATS_FILE");

    if (cp == NULL) {
#ifdef MSWINDOWS
	cp = "MEMSTATS.LST";
#else /* !MSWINDOWS */
	fp = stdout;
#endif /* MSWINDOWS */

    }

    if (cp != NULL)
	fp = fopen(cp, "w");

    if (fp == (_OSFile) 0)
	return;

    fputs("\nMemory Usage Statistics:\n", fp);

    fprintf(fp, "    Total memory allocations: %ld\n", TotalAllocations);
    fprintf(fp, "    Total memory reallocations: %ld\n", TotalReallocations);
    fprintf(fp, "    Total memory deallocations: %ld\n", TotalDeallocations);

    difference = TotalAllocations - TotalDeallocations;

    if (difference != 0) {
	if (difference > 0)
	    cp = "less";
	else {
	    cp = "more";
	    difference = - difference;
	}
	fprintf(fp, "        (%ld %s than allocations)\n", difference, cp);
    }

    fprintf(fp, "    Total memory allocated: %ld\n", TotalAllocatedMemory);
    fprintf(fp, "    Total memory deallocated: %ld\n", TotalDeallocatedMemory);

    difference = TotalAllocatedMemory - TotalDeallocatedMemory;

    if (difference != 0) {
	if (difference > 0)
	    cp = "less";
	else {
	    cp = "more";
	    difference = - difference;
	}
	fprintf(fp, "        (%ld %s than allocated)\n", difference, cp);
    }

    fprintf(fp, "    Peak memory allocation: %ld\n", PeakAllocatedMemory);

    fputs("\n", fp);

#ifndef MSWINDOWS
    if (fp != stdout)
#endif /* !MSWINDOWS */
	fclose(fp);

    return;
    }


_AnyPtr  LwkAllocateMem(size)
_Integer size;

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
    _CharPtr cp;

    cp = (_CharPtr) _HeapMalloc((size_t) size + sizeof(_CharPtr));

    if (cp == (_CharPtr) 0)
	_Raise(alloc_error);

    *((int *) cp) = size;

    cp += sizeof(_CharPtr);

/*
    _ClearMem(cp, size);
*/

    TotalAllocations += 1;
    TotalAllocatedMemory += size;

    if ((TotalAllocatedMemory - TotalDeallocatedMemory) > PeakAllocatedMemory)
	PeakAllocatedMemory = TotalAllocatedMemory - TotalDeallocatedMemory;

    return ((_AnyPtr) cp);
    }


_AnyPtr  LwkReallocateMem(pointer, new_size)
_AnyPtr pointer;
 _Integer new_size;

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
    _CharPtr cp;
    _Integer old_size;

    if (pointer == (_AnyPtr) _NullObject)
	cp = _AllocateMem(new_size);
    else {
	cp = ((_CharPtr) pointer) - sizeof(_CharPtr);

	old_size = *((int *) cp);

	cp = (_CharPtr) _HeapRealloc(cp, (size_t) (new_size + sizeof(_CharPtr)));

	if (cp == (_CharPtr) 0)
	    _Raise(alloc_error);

	*((int *) cp) = new_size;

	cp += sizeof(_CharPtr);

	TotalAllocatedMemory += new_size - old_size;
	TotalReallocations += 1;
    }

    return (_AnyPtr) cp;
    }


void  LwkFreeMem(pointer)
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
    _CharPtr cp;

    if (pointer == ((_AnyPtr) _NullObject))
	_Raise(dealloc_error);

    cp = ((_CharPtr) pointer) - sizeof(_CharPtr);

    size = *((int *) cp);

    _HeapFree(cp);

    TotalDeallocations += 1;
    TotalDeallocatedMemory += size;

    return;
    }


_String  LwkCopyString(src)
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
    _Integer len;

    if (src == (_String) _NullObject)
	dst = src;
    else {
	len = _LengthString(src) + 1;
	dst = (_String) _AllocateMem(len);

	_CopyMem(src, dst, len);
    }

    return dst;
    }


_String  LwkConcatString(string1, string2)
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


_Integer  LwkLengthString(string)
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
	return (_Integer) _Strlen(string);
    }


void  LwkDeleteString(string)
_StringPtr string;

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


_Integer  LwkCompareString(string1, string2)
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

    return (_Integer) _Strcmp(string1, string2);
    }


_Boolean  LwkIsCaselessEqualString(string1, string2)
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
	if (string1[i] == _EndOfString) {
	    if (string2[i] == _EndOfString)
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


_Boolean  LwkContainsString(string1, string2)
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
	if ((char) string1[i1] == _EndOfString) {
	    if ((char) string2[i2] == _EndOfString)
		return _True;
	    else
		return _False;
	}
	
	if ((char) string2[i2] == _EndOfString)
	    return _True;

	if ((char) string1[i1] == (char) string2[i2])
	    i2++;
	else
	    i2 = 0;

	i1++;
    }

    }


_Boolean  LwkHasPrefixString(string, prefix)
_String string;
 _String prefix;

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

    if (string == (_String) _NullObject || prefix == (_String) _NullObject)
	return _False;

    i = 0;

    while (_True) {
	/*
	**  If the string is exactly the prefix, return True.  If the prefix is
	**  longer than the string, return False.
	*/

	if ((char) string[i] == _EndOfString) {
	    if ((char) prefix[i] == _EndOfString)
		return _True;
	    else
		return _False;
	}

	/*
	**  If we've reached the end of the prefix, return True.
	**  not a prefix!
	*/

	if ((char) prefix[i] == _EndOfString)
	    return _True;

	/*
	**  If the corresponding characters are not equal, return False.
	*/

	if ((char) string[i] != (char) prefix[i])
	    return _False;

	/*
	**  Go look at the next character.
	*/

	i++;
    }
    }


_Integer  LwkCompareDate(date1, date2)
_Date date1;
 _Date date2;

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
    time_t b_time1;
    time_t b_time2;

    if (date1 == (_Date) _NullObject) {
	if (date2 == (_Date) _NullObject)
	    return (_Integer) 0;
	else
	    return (_Integer) -1;
    }
    else if (date2 == (_Date) _NullObject)
	return (_Integer) 1;

    b_time1 = _DateToTime(date1);
    b_time2 = _DateToTime(date2);

    if (b_time1 < b_time2)
	return (_Integer) -1;
    else if (b_time1 == b_time2)
	return (_Integer) 0;
    else
	return (_Integer) 1;
    }


_Boolean  LwkIsValidDate(date)
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
    char delim;
    int args;
    int year,
	month,
	day,
	hour,
	minute,
	second,
	tdf_hour,
	tdf_minute;

    /*
    **  Null Dates are invalid
    */

    if (date == (_Date) _NullObject)
	return _False;

    /*
    **  Parse the Date
    */

    args = sscanf(date, "%4d%2d%2d%2d%2d%2d%c%2d%2d", &year, &month,
	&day, &hour, &minute, &second, &delim, &tdf_hour, &tdf_minute);

    /*
    **	Must have at least year, month, day, hour, minute, second, plus "Z" or
    **	a Time Differential Factor (TDF).
    */

    if (args < 7)
	return _False;

    /*
    **  Range check the fields
    */

    if (month > 12 || day > 31 || hour > 60 || minute > 60 || second > 60)
	return _False;

    /*
    **  Now, there must be either a "Z" and nothing else
    */

    if (delim == 'Z' && args == 7)
	return _True;

    /*
    **  Or there must be a valid TDF: +hhmm or -hhmm
    */

    if ((delim == '+' || delim == '-') && args == 9) {
	if (tdf_hour > 60 || tdf_minute > 60)
	    return _False;
	else
	    return _True;
    }

    return _False;
    }


_String  LwkDateToString(date)
_Date date;

/*
**++
**  Functional Description:
**	Convert the date to a string.
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	date: date to convert.
**	string: string to return.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
    {
    _OSTm tm;
    char string[101];

    string[100] = _EndOfString;

    DateToTm(date, &tm);

    strftime(string, 100, "%c", &tm);

    return _CopyString((_String) string);
    }


_Date  LwkNowToDate()
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
    time_t b_time;

    time(&b_time);

    return _TimeToDate(b_time);
    }


time_t  LwkDateToTime(date)
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
    _OSTm tm;

    DateToTm(date, &tm);

    return mktime(&tm);
    }


_Date  LwkTimeToDate(b_time)
time_t b_time;

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
    _OSTmPtr tm;
    char string[20];

    tm = localtime(&b_time);

    sprintf(string, "%04d%02d%02d%02d%02d%02dZ", 1900 + tm->tm_year,
	tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

    return (_Date) _CopyString((_String) string);
    }


void  LwkDeleteVaryingString(vstring)
_VaryingStringPtr vstring;

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
    if (*vstring != (_VaryingString) _NullObject) {
	_FreeMem((_AnyPtr) *vstring);

	*vstring = (_VaryingString) _NullObject;
    }

    return;
    }


void  LwkDeleteDVString(dvstring)
_DynamicVStringPtr dvstring;

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
    if (*dvstring != (_DynamicVString) _NullObject) {
	_DeleteVaryingString(&_DVString_of(*dvstring));

	_FreeMem((_AnyPtr) *dvstring);

	*dvstring = (_DynamicVString) _NullObject;
    }

    return;
    }


_String  LwkStatusToString(code)
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
    int i;

    /*
    **  Look for the Status Code in the table, and return its corresponsing
    **	string form
    */

    for (i = 0; i < (int) _StatusCode(max); i++)
	if (StatusToString[i].status == code)
	    return StatusToString[i].string;

    /*
    **  If it is not there, turn Status Code "unknown" into a string
    */

    for (i = 0; i < (int) _StatusCode(max); i++)
	if (StatusToString[i].status == _StatusCode(unknown))
	    return StatusToString[i].string;

    /*
    **  As a last resort return a literal "unknown"
    */

    return (_String) _UnknownStatusString;
    }


_String  LwkStatusToResource(code)
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
    int i;

    /*
    **  Look for the Status Code in the table, and return its corresponsing
    **	DRM Resource
    */

    for (i = 0; i < (int) _StatusCode(max); i++)
	if (StatusToResource[i].status == code)
	    return StatusToResource[i].string;

    /*
    **  If it is not there, turn Status Code "unknown" into a resource
    */

    for (i = 0; i < (int) _StatusCode(max); i++)
	if (StatusToResource[i].status == _StatusCode(unknown))
	    return StatusToResource[i].string;

    /*
    **  As a last resort return a literal "unknown"
    */

    return (_String) _UnknownStatusString;
    }


_String  LwkStatusToHelpKeyResource(code)
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
    int i;

    /*
    **  Look for the Status Code in the table, and return its corresponding
    **	Help Key Resource
    */

    for (i = 0; i < (int) _StatusCode(max); i++)
	if (StatusToHelpKeyResource[i].status == code)
	    if (*StatusToHelpKeyResource[i].string != _EndOfString)
		return StatusToHelpKeyResource[i].string;
	    else
		return _GenericHelpKeyResource;
    }


int  LwkSearchTable(table, entry, include_private)
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
    int i;

    if (table == (_PropertyNameTable) _NullObject
	|| entry == (_String) _NullObject)
	    return -1;

    i = 0;

    while (table[i].name != (_String) _NullObject) {
	if (table[i].public || (!table[i].public && include_private))
	    if (_CompareString(table[i].name, entry) == 0)
		return i;
	i++;
    }

    return -1;
    }


void  LwkListTable(table, list, include_private)
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
	    _AddElement(list, lwk_c_domain_string, &table[i].name, _True);
	i++;
    }

    return;
    }


_Type  LwkDomainToType(domain)
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
    int i;

    for (i = 0; i < (int) lwk_c_domain_max; i++)
	if (TypeToDomain[i].domain == domain)
	    return TypeToDomain[i].type;

    return (_Type) _NullObject;
    }


_Domain  LwkTypeToDomain(type)
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
    int i;

    for (i = 0; i < (int) lwk_c_domain_max; i++)
	if (TypeToDomain[i].type == type)
	    return TypeToDomain[i].domain;

    return lwk_c_domain_unknown;
    }


_Boolean  LwkIsDomain(domain1, domain2)
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

    type1 = _DomainToType(domain1);
    type2 = _DomainToType(domain2);

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


_Boolean  LwkIsValidObject(object)
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


_Boolean  LwkIsValidType(type)
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


_Integer  LwkCompareValue(value1, value2, domain)
_AnyPtr value1;
 _AnyPtr value2;
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

    if (_IsDomain(lwk_c_domain_object, domain))
	domain = lwk_c_domain_object;

    /*
    **	Depending on the domain, compare the values appropriately.
    */

    switch (domain) {
	case lwk_c_domain_integer :
	    if (*((_IntegerPtr) value1) == *((_IntegerPtr) value2))
		answer = 0;
	    else if (*((_IntegerPtr) value1) < *((_IntegerPtr) value2))
		answer = -1;
	    else
		answer = 1;

	    break;

	case lwk_c_domain_closure :
	    if (*((_ClosurePtr) value1) == *((_ClosurePtr) value2))
		answer = 0;
	    else if (*((_ClosurePtr) value1) < *((_ClosurePtr) value2))
		answer = -1;
	    else
		answer = 1;

	    break;

	case lwk_c_domain_boolean :
	    if (*((_BooleanPtr) value1) == *((_BooleanPtr) value2))
		answer = 0;
	    else if (*((_BooleanPtr) value1) < *((_BooleanPtr) value2))
		answer = -1;
	    else
		answer = 1;

	    break;

	case lwk_c_domain_routine :
	    if (*((_CallbackPtr) value1) == *((_CallbackPtr) value2))
		answer = 0;
	    else
		answer = 2;

	    break;

	case lwk_c_domain_float :
	    if (*((_FloatPtr) value1) == *((_FloatPtr) value2))
		answer = 0;
	    else if (*((_FloatPtr) value1) < *((_FloatPtr) value2))
		answer = -1;
	    else
		answer = 1;

	    break;

	case lwk_c_domain_date :
	    answer = _CompareDate(*((_DatePtr) value1),
		*((_DatePtr) value2));
	    break;

	case lwk_c_domain_ddif_string :
	    answer = _CompareDDIFString(*((_DDIFStringPtr) value1),
		*((_DDIFStringPtr) value2));
	    break;

	case lwk_c_domain_string :
	    answer = _CompareString(*((_StringPtr) value1),
		*((_StringPtr) value2));
	    break;

	case lwk_c_domain_object :
	    if (*((_ObjectPtr) value1) == *((_ObjectPtr) value2))
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


void  LwkCopyValue(source, target, domain)
_AnyPtr source;
 _AnyPtr target;
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
    ** Primitive datatypes (e.g, integer, float) are copied.  Some
    ** non-persistent objects (e.g., List/Set, ObjectId, Property), are also
    ** copied.  Persistent objects and a few non-persistent objects (e.g., Ui,
    ** Currency, Repository) are NOT copied -- only a reference to the object
    ** is copied.
    */

    if (_IsDomain(lwk_c_domain_object, domain)) {
	if (_IsDomain(lwk_c_domain_persistent, domain)
	    || _IsDomain(lwk_c_domain_ui, domain)
	    || _IsDomain(lwk_c_domain_environment_state, domain)
	    || _IsDomain(lwk_c_domain_linkbase, domain))
		domain = lwk_c_domain_persistent;
	else
		domain = lwk_c_domain_object;
    }

    /*
    **	Depending on its domain, copy the value appropriately.
    */

    switch (domain) {
	case lwk_c_domain_integer :
	    *((_IntegerPtr) target) = *((_IntegerPtr) source);
	    break;

	case lwk_c_domain_closure :
	    *((_ClosurePtr) target) = *((_ClosurePtr) source);
	    break;

	case lwk_c_domain_boolean :
	    *((_BooleanPtr) target) = *((_BooleanPtr) source);
	    break;

	case lwk_c_domain_routine :
	    *((_CallbackPtr) target) = *((_CallbackPtr) source);
	    break;

	case lwk_c_domain_float :
	    *((_FloatPtr) target) = *((_FloatPtr) source);
	    break;

	case lwk_c_domain_date :
	    if (*((_DatePtr) source) == (_Date) _NullObject)
		*((_DatePtr) target) = (_Date) _NullObject;
	    else {
		if (!_IsValidDate(*((_DatePtr) source)))
		    _Raise(inv_date);
		*((_DatePtr) target) = _CopyDate(*((_DatePtr) source));
	    }
	    break;

	case lwk_c_domain_ddif_string :
	    if (*((_DDIFStringPtr) source) == (_DDIFString) _NullObject)
		*((_DDIFStringPtr) target) = (_DDIFString) _NullObject;
	    else {
		if (!_IsValidDDIFString(*((_DDIFStringPtr) source)))
		    _Raise(inv_ddif_string);
		*((_DDIFStringPtr) target) =
		    _CopyDDIFString(*((_DDIFStringPtr) source));
	    }
	    break;

	case lwk_c_domain_string :
	    if (*((_StringPtr) source) == (_String) _NullObject)
		*((_StringPtr) target) = (_String) _NullObject;
	    else
		*((_StringPtr) target) =
		    _CopyString(*((_StringPtr) source));
	    break;

	case lwk_c_domain_object :
	    if (*((_ObjectPtr) source) == (_Object) _NullObject)
		*((_ObjectPtr) target) = (_Object) _NullObject;
	    else
		*((_ObjectPtr) target) = (_Object)
		    _Copy(*((_ObjectPtr) source), _False);
	    break;

	case lwk_c_domain_persistent :
	    *((_ObjectPtr) target) = *((_ObjectPtr) source);
	    break;

	default :
	    _Raise(inv_domain);
	    break;
    }

    return;
    }


void  LwkMoveValue(source, target, domain)
_AnyPtr source;
 _AnyPtr target;
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

    if (_IsDomain(lwk_c_domain_object, domain))
	domain = lwk_c_domain_object;

    /*
    **	Depending on its domain, move the value appropriately.
    */

    switch (domain) {
	case lwk_c_domain_integer :
	    *((_IntegerPtr) target) = *((_IntegerPtr) source);
	    break;

	case lwk_c_domain_closure :
	    *((_ClosurePtr) target) = *((_ClosurePtr) source);
	    break;

	case lwk_c_domain_boolean :
	    *((_BooleanPtr) target) = *((_BooleanPtr) source);
	    break;

	case lwk_c_domain_routine :
	    *((_CallbackPtr) target) = *((_CallbackPtr) source);
	    break;

	case lwk_c_domain_float :
	    *((_FloatPtr) target) = *((_FloatPtr) source);
	    break;

	case lwk_c_domain_date :
	    *((_DatePtr) target) = *((_DatePtr) source);
	    break;

	case lwk_c_domain_ddif_string :
	    *((_DDIFStringPtr) target) = *((_DDIFStringPtr) source);
	    break;

	case lwk_c_domain_string :
	    *((_StringPtr) target) = *((_StringPtr) source);
	    break;

	case lwk_c_domain_object :
	    *((_ObjectPtr) target) = *((_ObjectPtr) source);
	    break;

	default :
	    _Raise(inv_domain);
	    break;
    }

    return;
    }


void  LwkListValue(value, list, domain)
_AnyPtr value;
 _ObjectPtr list;
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

    if (_IsDomain(lwk_c_domain_object, domain))
	domain = lwk_c_domain_object;

    /*
    **	Depending on its domain, copy the value appropriately.
    */

    switch (domain) {
	case lwk_c_domain_integer :
	    *list = (_List) _CreateList(_TypeList, lwk_c_domain_integer,
		(_Integer) 1);
	    _AddElement(*list, lwk_c_domain_integer, value, _True);
	    break;

	case lwk_c_domain_closure :
	    *list = (_List) _CreateList(_TypeList, lwk_c_domain_closure,
		(_Integer) 1);
	    _AddElement(*list, lwk_c_domain_closure, value, _True);
	    break;

	case lwk_c_domain_boolean :
	    *list = (_List) _CreateList(_TypeList, lwk_c_domain_boolean,
		(_Integer) 1);
	    _AddElement(*list, lwk_c_domain_boolean, value, _True);
	    break;

	case lwk_c_domain_routine :
	    *list = (_List) _CreateList(_TypeList, lwk_c_domain_routine,
		(_Integer) 1);
	    _AddElement(*list, lwk_c_domain_routine, value, _True);
	    break;

	case lwk_c_domain_float :
	    *list = (_List) _CreateList(_TypeList, lwk_c_domain_float,
		(_Integer) 1);
	    _AddElement(*list, lwk_c_domain_float, value, _True);
	    break;

	case lwk_c_domain_date :
	    *list = (_List) _CreateList(_TypeList, lwk_c_domain_date,
		(_Integer) 1);
	    _AddElement(*list, lwk_c_domain_date, value, _True);
	    break;

	case lwk_c_domain_ddif_string :
	    *list = (_List) _CreateList(_TypeList, lwk_c_domain_ddif_string,
		(_Integer) 1);
	    _AddElement(*list, lwk_c_domain_ddif_string, value, _True);
	    break;

	case lwk_c_domain_string :
	    *list = (_List) _CreateList(_TypeList, lwk_c_domain_string,
		(_Integer) 1);
	    _AddElement(*list, lwk_c_domain_string, value, _True);
	    break;

	case lwk_c_domain_object :
	    *list = (_List) _CreateList(_TypeList, lwk_c_domain_object,
		(_Integer) 1);
	    _AddElement(*list, lwk_c_domain_object, value, _True);
	    break;

	default :
	    _Raise(inv_domain);
	    break;
    }

    return;
    }


void  LwkClearValue(value, domain)
_AnyPtr value;
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

    if (_IsDomain(lwk_c_domain_object, domain))
	domain = lwk_c_domain_object;

    /*
    **	Depending on its domain, clear the value appropriately.
    */

    switch (domain) {
	case lwk_c_domain_integer :
	    *((_IntegerPtr) value) = 0;
	    break;

	case lwk_c_domain_closure :
	    *((_ClosurePtr) value) = (_Closure) 0;
	    break;

	case lwk_c_domain_boolean :
	    *((_BooleanPtr) value) = _False;
	    break;

	case lwk_c_domain_routine :
	    *((_CallbackPtr) value) = (_Callback) _NullObject;
	    break;

	case lwk_c_domain_float :
	    *((_FloatPtr) value) = 0.0;
	    break;

	case lwk_c_domain_date :
	    *((_DatePtr) value) = (_Date) _NullObject;
	    break;

	case lwk_c_domain_ddif_string :
	    *((_DDIFStringPtr) value) = (_DDIFString) _NullObject;
	    break;

	case lwk_c_domain_string :
	    *((_StringPtr) value) = (_String) _NullObject;
	    break;

	case lwk_c_domain_object :
	    *((_ObjectPtr) value) = (_Object) _NullObject;
	    break;

	default :
	    _Raise(inv_domain);
	    break;
    }

    return;
    }


void  LwkDeleteValue(value, domain)
_AnyPtr value;
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
    ** Primitive datatypes (e.g, integer, float) are copied.  Some
    ** non-persistent objects (e.g., List/Set, ObjectId, Property), are also
    ** copied.  Persistent objects and a few non-persistent objects (e.g., Ui,
    ** Currency, Repository) are NOT copied -- only a reference to the object
    ** is copied.  Datatypes must be freed accordingly.
    */

    if (_IsDomain(lwk_c_domain_object, domain)) {
	if (_IsDomain(lwk_c_domain_persistent, domain)
	    || _IsDomain(lwk_c_domain_ui, domain)
	    || _IsDomain(lwk_c_domain_environment_state, domain)
	    || _IsDomain(lwk_c_domain_linkbase, domain))
		domain = lwk_c_domain_persistent;
	else
		domain = lwk_c_domain_object;
    }

    /*
    **	Depending on its domain, delete the value appropriately.
    */

    switch (domain) {
	case lwk_c_domain_integer :
	case lwk_c_domain_closure :
	case lwk_c_domain_boolean :
	case lwk_c_domain_routine :
	case lwk_c_domain_float :
	    break;

	case lwk_c_domain_date :
	    if (*((_DatePtr) value) != (_Date) _NullObject)
		_DeleteDate(((_DatePtr) value));
	    break;

	case lwk_c_domain_ddif_string :
	    if (*((_DDIFStringPtr) value) != (_DDIFString) _NullObject)
		_DeleteDDIFString(((_DDIFStringPtr) value));
	    break;

	case lwk_c_domain_string :
	    if (*((_StringPtr) value) != (_String) _NullObject)
		_DeleteString(((_StringPtr) value));
	    break;

	case lwk_c_domain_object :
	    if (*((_ObjectPtr) value) != (_Object) _NullObject)
		_Delete(((_ObjectPtr) value));
	    break;

	case lwk_c_domain_persistent :
	    break;

	default :
	    _Raise(inv_domain);
	    break;
    }

    return;
    }


void  LwkSetMultiValProp(list, property_domain, value_domain, value, flag, value_is_list, property_is_set)
_ObjectPtr list;
 _Domain property_domain;

    _Domain value_domain;
 _AnyPtr value;
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
    _Integer integer;

    /*
    **	If Set Value is called for a multi-valued property, the call could be
    **	providing either a single value (which must be put in a list/set), or a
    **	List.  We have to check for this latter case, and turn it into the
    **	equivalent of a SetValueList.
    */

    if (!value_is_list)
	if (_IsDomain(lwk_c_domain_list, value_domain))
	    value_is_list = _True;

    /*
    **	If the value is a list, we are really interested in the list domain,
    **	not the value domain.
    */

    if (value_is_list)
	if (value != (_AnyPtr) _NullObject)
	    if (*((_ListPtr) value) != (_List) _NullObject) {
		_GetValue(*((_ListPtr) value), _P_Domain,
		    lwk_c_domain_integer, &integer);

		value_domain = (_Domain) integer;
	    }

    /*
    **  Now, deal with each type of value setting individually.
    */

    switch (flag) {
	case lwk_c_clear_property :
	    /*
	    **	Delete the old value if there was one, then set the property to
	    **	the appropriate null value.
	    */

	    _DeleteValue((_AnyPtr) list, lwk_c_domain_list);
	    _ClearValue((_AnyPtr) list, lwk_c_domain_list);

	    break;

	case lwk_c_add_property :
	    /*
	    **  Make sure we have a valid value
	    */

	    if (value == (_AnyPtr) _NullObject)
		_Raise(inv_argument);

	    if (value_is_list) {
		/*
		**  Make sure we have a valid List
		*/

		if (*((_ListPtr) value) == (_List) _NullObject)
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

		    _GetValue(*((_ListPtr) value), _P_ElementCount,
			lwk_c_domain_integer, &size);

		    if (property_is_set)
			*list = (_List) _CreateSet(_TypeSet, property_domain,
			    size);
		    else
			*list = (_List) _CreateList(_TypeList, property_domain,
			    size);
		}

		/*
		**  Append the elements in the value list to the property list
		*/

		_AppendElements(*list, *((_ListPtr) value));
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
			*list = (_List) _CreateSet(_TypeSet, property_domain,
			    (_Integer) 0);
		    else
			*list = (_List) _CreateList(_TypeList, property_domain,
			    (_Integer) 0);
		}

		/*
		**  Add the value to the property list
		*/

		_AddElement(*list, value_domain, value, _True);
	    }

	    break;

	case lwk_c_set_property :
	    /*
	    **  Make sure we have a valid value
	    */

	    if (value == (_AnyPtr) _NullObject)
		_Raise(inv_argument);

	    /*
	    **  Save old property List
	    */

	    old_list = *list;

	    if (value_is_list) {
		if (*((_ListPtr) value) != (_List) _NullObject) {
		    /*
		    **  Make sure the elements are of the correct domain.
		    */

		    if (!_IsDomain(property_domain, value_domain))
			_Raise(inv_domain);

		    if (property_is_set)
			if (!_IsType(*((_ListPtr) value), _TypeSet))
			    _Raise(inv_domain);
		}

		/*
		**  Set property value.
		*/

		_CopyValue(value, (_AnyPtr) list, lwk_c_domain_list);
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

		if (property_is_set)
		    *list = (_List) _CreateSet(_TypeSet, property_domain,
			(_Integer) 1);
		else
		    *list = (_List) _CreateList(_TypeList, property_domain,
			(_Integer) 1);

		_AddElement(*list, value_domain, value, _True);
	    }

	    /*
	    **  If there was an old property List, delete it
	    */

	    if (old_list != (_List) _NullObject)
		_Delete(&old_list);

	    break;

	case lwk_c_remove_property :
	    /*
	    **  Make sure we have a valid value
	    */

	    if (value == (_AnyPtr) _NullObject)
		_Raise(inv_argument);

	    if (value_is_list) {
		/*
		**  Make sure we have a valid List
		*/

		if (*((_ListPtr) value) == (_List) _NullObject)
		    _Raise(inv_argument);

		/*
		**  A valid list was provided.  Iterate over the list, and
		**  remove all occurances of each element.
		*/

		if (*list != (_List) _NullObject)
		    _Iterate(*((_ListPtr) value), property_domain,
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

	case lwk_c_delete_property :
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


_Boolean  LwkIsValidPropertyName(name)
_String name;

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
    if (name == (_String) _NullObject)
	return _False;

    if (_Strspn(name, _ValidPropertyNameCharacters) != _Strlen(name))
	return _False;

    return _True;
    }


void  LwkSetSingleValProp(property, property_domain, value_domain, value, flag, value_is_list)
_AnyPtr property;
 _Domain property_domain;

    _Domain value_domain;
 _AnyPtr value;
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
    _Integer tmp_dom;

    /*
    **  Deal with each type of value setting individually.
    */

    switch (flag) {
	case lwk_c_clear_property :
	    /*
	    **	Delete the old value if there was one, then set the property to
	    **	the appropriate null value.
	    */

	    _DeleteValue(property, property_domain);
	    _ClearValue(property, property_domain);

	    break;

	case lwk_c_add_property :
	    /*
	    **  Can not add a value to a single-valued property.
	    */

	    _Raise(single_valued_property);
	    break;

	case lwk_c_set_property :
	    /*
	    **  Make sure we have a valid value
	    */

	    if (value == (_AnyPtr) _NullObject)
		_Raise(inv_argument);

	    if (value_is_list) {
		/*
		**  Make sure we have a valid List
		*/

		if (*((_ListPtr) value) == (_List) _NullObject)
		    _Raise(inv_argument);

		/*
		**  Make sure its elements are of the correct domain.
		*/

		_GetValue(*((_ListPtr) value), _P_Domain,
		    lwk_c_domain_integer, &tmp_dom);
                value_domain = (_Domain) tmp_dom;

		if (!_IsDomain(property_domain, value_domain))
		    _Raise(inv_domain);

		/*
		**  Delete any old value, then set the property to the first
		**  element of the list.
		*/

		_DeleteValue(property, property_domain);

		_SelectElement(*((_ListPtr) value), (_Integer) 0,
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

	case lwk_c_remove_property :
	    /*
	    **  Make sure we have a valid value
	    */

	    if (value == (_AnyPtr) _NullObject)
		_Raise(inv_argument);

	    if (value_is_list) {
		/*
		**  Make sure we have a valid List
		*/

		if (*((_ListPtr) value) == (_List) _NullObject)
		    _Raise(inv_argument);

		/*
		**  Iterate over the list, and delete the property if its value
		**  appears in the list.
		*/

		_Iterate(*((_ListPtr) value), property_domain,
		    (_Closure) property, RemoveProperty);
	    }
	    else {
		/*
		**  If the value match matches, clear the property value.
		*/

		RemoveProperty((_Closure) property, (_List) _NullObject,
		    value_domain, value);
	    }

	    break;

	case lwk_c_delete_property :
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


static _Termination _EntryPt  DeleteElementFromList(property_list, value_list, domain, value)
_Closure property_list;

    _List value_list;
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
    /*
    **  Simply delete all occurances of the string from the list.
    */

    _DeleteElement((_List) property_list, domain, value);

    return ((_Termination) 0);
    }


static _Termination _EntryPt  RemoveProperty(property, element_list, domain, value)
_Closure property;

    _List element_list;
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
    _Termination answer;

    /*
    **	If the value matches, delete any old value, then clear the property.
    */

    if (_CompareValue((_AnyPtr) property, value, domain) == 0) {
	_DeleteValue((_AnyPtr) property, domain);
	_ClearValue((_AnyPtr) property, domain);
	answer = (_Termination) 1;
    }
    else
	answer = (_Termination) 0;

    return answer;
    }


_DDIShandle  LwkCreateDDISStream(vstring, parse_tables)
_VaryingStringPtr vstring;

    _AnyPtr parse_tables;

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
    _DDISsize buffer_size;
    _DDISstatus status;
    _DDIShandle handle;

    /*
    **  Initialize a descriptor for the buffer which will hold the encoding.
    */

    _AllocateVaryingString(*vstring, _EncodingBufferInitialLength);

    /*
    **  Initialize the DDIS toolkit stream.
    */

    buffer_size = _EncodingBufferInitialLength;

    status = ddis_create_stream(AllocateDDISMem, DeallocateDDISMem,
	(_DDISuserparam) 0, ExtendDDISBuffer, vstring, &buffer_size,
	(_DDISdata) _VString_of(*vstring), parse_tables, &handle);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    return handle;
    }


_DDIShandle  LwkOpenDDISStream(vstring, parse_tables)
_VaryingString vstring;
 _AnyPtr parse_tables;

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
    _DDISstatus status;
    _DDIShandle handle;

    /*
    **  Initialize the DDIS toolkit stream.
    */

    status = ddis_open_stream(AllocateDDISMem, DeallocateDDISMem,
	(_DDISuserparam) 0, GetDDISBuffer, vstring, parse_tables, &handle);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    return handle;
    }


void  LwkCloseDDISStream(handle)
_DDIShandle handle;

    {
    _DDISstatus status;

    status = (_DDISstatus) ddis_close_stream(&handle);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    return;
    }


static _DDISstatus CDA_CALLBACK  ExtendDDISBuffer(vstring, size, pointer, new_size, new_pointer)
_VaryingStringPtr vstring;

    _DDISsizePtr size;
 _DDISdata pointer;
 _DDISsizePtr new_size;

    _DDISdataPtr new_pointer;

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
    _Integer remaining;

    /*
    **  Update the descriptor to reflect the bytes added.
    */

    if (_VLength_of(*vstring) + *size > _VaryingStringMaxSize)
	_Raise(internal_encoding_error);

    _VLength_of(*vstring) += *size;

    /*
    **  If necessary, extend the encoding buffer.
    */

    if (_VLength_of(*vstring) <= _EncodingBufferInitialLength)
	remaining = _EncodingBufferInitialLength - _VLength_of(*vstring);
    else
	remaining = (_VLength_of(*vstring) - _EncodingBufferInitialLength)
	    % (_EncodingBufferIncrement);

    if (remaining == 0) {
	_ReallocateVaryingString(*vstring, _VLength_of(*vstring)
	    + _EncodingBufferIncrement);

	remaining = _EncodingBufferIncrement;
    }

    /*
    **  Return the size of the remaining buffer, and a pointer to it.
    */

    *new_size = remaining;
    *new_pointer = (_DDISdata) _VString_of(*vstring) + _VLength_of(*vstring);

    return (_DDISstatus) DDIS_NORMAL;
    }


static _DDISstatus CDA_CALLBACK  GetDDISBuffer(vstring, size, pointer)
_VaryingString vstring;

    _DDISsizePtr size;
 _DDISdataPtr pointer;

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
    **  Return the size of the buffer, and a pointer to it.
    */

    *size = _VLength_of(vstring);
    *pointer = (_DDISdata) _VString_of(vstring);

    return (_DDISstatus) DDIS_NORMAL;
    }


static _DDISstatus CDA_CALLBACK  AllocateDDISMem(size, pointer, user_data)
_DDISsizePtr size;

    _DDIStablePtr pointer;
 _DDISuserparam user_data;

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
    **  Call internal routine to allocate the memory.
    */

    *pointer = (_DDIStable) _AllocateMem(*size);

    return (_DDISstatus) DDIS_NORMAL;
    }


static _DDISstatus CDA_CALLBACK  DeallocateDDISMem(size, pointer, user_data)
_DDISsizePtr size;

    _DDIStablePtr pointer;
 _DDISuserparam user_data;

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
    **  Call internal routine to allocate the memory.
    */

    _FreeMem(*pointer);

    return (_DDISstatus) DDIS_NORMAL;
    }


void  LwkHashInitialize(table, size)
_HashTableEntryPtr table;
 _Integer size;

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

    for (i = 0; i < size; i++)
	table[i] = (_HashTableEntry) 0;

    return;
    }


_Boolean  LwkHashInsert(table, size, key, item)
_HashTableEntryPtr table;
 _Integer size;

    _Integer key;
 _AnyPtr item;

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
    int hash_code;
    _AnyPtr old_item;
    _Boolean inserted;
    _HashTableEntry entry;

    /*
    **  If the entry is not there, create a new one
    */

    if (_HashSearch(table, size, key, &old_item))
	inserted = _False;
    else {
	hash_code = key % size;

	entry = (_HashTableEntry) _AllocateMem(sizeof(_HashTableEntryInstance));

	_HashKey_of(entry) = key;
	_HashItem_of(entry) = item;
	_HashNext_of(entry) = table[hash_code];

	table[hash_code] = entry;

	inserted = _True;
    }

    return inserted;
    }


_Boolean  LwkHashSearch(table, size, key, item)
_HashTableEntryPtr table;
 _Integer size;

    _Integer key;
 _AnyPtrPtr item;

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
    int hash_code;
    _Boolean found;
    _HashTableEntry entry;

    hash_code = key % size;

    entry = table[hash_code];

    found = _False;

    while (entry != (_HashTableEntry) 0) {
	if (_HashKey_of(entry) == key) {
	    *item = _HashItem_of(entry);
	    found = _True;
	    break;
	}

	entry = _HashNext_of(entry);
    }

    return found;
    }


_Boolean  LwkHashDelete(table, size, key, item)
_HashTableEntryPtr table;
 _Integer size;

    _Integer key;
 _AnyPtr item;

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
    int hash_code;
    _Boolean found;
    _HashTableEntry previous;
    _HashTableEntry entry;

    hash_code = key % size;

    found = _False;
    previous = (_HashTableEntry) 0;
    entry = table[hash_code];

    while (entry != (_HashTableEntry) 0) {
	if (_HashKey_of(entry) == key) {
	    if (previous == (_HashTableEntry) 0)
		table[hash_code] = _HashNext_of(entry);
	    else
		_HashNext_of(previous) = _HashNext_of(entry);

	    _FreeMem((_AnyPtr) entry);

	    found = _True;

	    break;
	}

	previous = entry;
	entry = _HashNext_of(entry);
    }

    return found;
    }


void  LwkHashFree(table, size)
_HashTableEntryPtr table;
 _Integer size;

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
    _HashTableEntry entry, tentry;

    for (i = 0; i < size; i++) {
	entry = table[i];

	while (entry != (_HashTableEntry) 0) {
	    tentry = entry;
	    entry = _HashNext_of(entry);
	    _FreeMem((_AnyPtr) tentry);
	}
    }

    return;
    }


static void  DateToTm(date, tm)
_Date date;
 _OSTmPtr tm;

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
    char delim;
    int args;
    int tdf_hour;
    int tdf_minute;

    /*
    **  Initialize unused fields
    */

    tm->tm_wday = 0;
    tm->tm_yday = 0;
    tm->tm_isdst = 0;

    /*
    **  Parse the Date into its component fields
    */

    args = sscanf(date, "%4d%2d%2d%2d%2d%2d%c%2d%2d", &tm->tm_year,
	&tm->tm_mon, &tm->tm_mday, &tm->tm_hour, &tm->tm_min, &tm->tm_sec,
	&delim, &tdf_hour, &tdf_minute);

    /*
    **  Make some minor adjustments
    */

    tm->tm_mon--;
    tm->tm_year -= 1900;

    /*
    **  Adjust for any Time Differential Factor (TDF)
    */

    if (args > 7) {
	if (delim == '+') {
	    tm->tm_hour += tdf_hour;
	    tm->tm_min += tdf_minute;
	}
	else if (delim == '-') {
	    tm->tm_hour -= tdf_hour;
	    tm->tm_min -= tdf_minute;
	}
    }

    return;
    }
