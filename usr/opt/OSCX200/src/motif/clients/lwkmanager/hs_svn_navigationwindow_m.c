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
**	LinkWorks Manager
**
**  Version: 1.0
**
**  Abstract:
**	SVN support routines for the environment window.
**
**  Keywords:
**	{@keywords or None@}
**
**  Environment:
**	{@environment description@}
**
**  Author:
**	Patricia Avigdor
**	Andre Pavanello
**
**  Creation Date: 11-Dec-89
**
**  Modification History:
**
**--
*/

/*
**  Include Files
*/

#include "hs_include.h"
#include "hs_abstract_objects.h"
#include "hs_decwindows.h"

/*
**  Type Definitions
*/
typedef struct __IterateActiveIndex {
	    _SvnData		    head;
	    _SvnData		    prev_index_entry;
	    _Boolean		    good_sequence;
	    lwk_object		    end_seq_obj;
    }_IterateActiveIndexInstance, *_IterateActiveIndex;

/*
**  Forward Routine Declarations
*/

_DeclareFunction(static _Void SvnEnvWindowIterateList,
    (_WindowPrivate private, _HsObject envcontext, _String property,
     _SvnIterateData iterate_data));
_DeclareFunction(static _Void SvnEnvWindowIterateActive,
    (_WindowPrivate private, _HsObject envcontext, _String property));
_DeclareFunction(static _Void SvnEnvWindowIterateActivePaths,
    (_WindowPrivate private, _HsObject envcontext, _String property));
_DeclareFunction(static _Void SvnEnvWindowIterateCurrent,
    (_WindowPrivate private, _HsObject envcontext, _String property));
_DeclareFunction(static lwk_termination SvnEnvWindowListEnvCtxt,
    (_SvnIterateData iterate_data, lwk_list list, lwk_domain domain,
     lwk_object *object));
_DeclareFunction(static lwk_termination SvnEnvWindowListComposite,
    (_SvnData svn_data, lwk_list list, lwk_domain domain,
     lwk_object *object));
_DeclareFunction(static lwk_termination SvnEnvWindowListActiveNets,
    (_WindowPrivate private, lwk_list list, lwk_domain domain,
     lwk_object *object));
_DeclareFunction(static lwk_termination SvnEnvWindowListActivePaths,
    (_IterateActiveIndex iterate_data, lwk_list list,
     lwk_domain domain, lwk_object_descriptor *object_desc));
_DeclareFunction(static _Void FindObjEntryInSiblings,
    (_SvnData from_entry, lwk_object persistent_object,
     _SvnData *matched_entry));
_DeclareFunction(static lwk_termination SvnEnvWindowListCurrent,
    (_WindowPrivate private, lwk_list list, lwk_domain domain,
     lwk_object *object));
_DeclareFunction(static _Void SvnEnvWindowGetHisObject,
    (_HsObject envcontext, _String property, lwk_object *hisobject));
_DeclareFunction(static _Void SvnEnvWindowSetTitle,
    (_WindowPrivate private));
_DeclareFunction(static _Void SvnEnvWindowExpandSegment,
    (_WindowPrivate private));
_DeclareFunction(static _Void SvnEnvWindowSetEntry,
    (_WindowPrivate private, _SvnData svn_data));
_DeclareFunction(static _Void SvnEnvWindowSetComponents,
    (_WindowPrivate private, _SvnData svn_data, int entry_number,
     Pixmap pixmap, _CString	name, _CString linkbase_name, _Boolean index,
     _Boolean small_font));
_DeclareFunction(static _Void SvnEnvWindowSetRecording,
    (_WindowPrivate private, _SvnData svn_data));
_DeclareFunction(static _Void SvnEnvWindowSetCheckMarkComponent,
    (_WindowPrivate private, _Integer entry_number, _Integer component,
     _Integer state));
_DeclareFunction(static _Void SvnEnvWindowConfirm,
    (Widget w, _WindowPrivate private, DXmSvnCallbackStruct *cb));
_DeclareFunction(static _Void SvnEnvWindowToggleRecord,
    (_WindowPrivate private, _SvnData entry));
_DeclareFunction(static _Void SvnEnvWindowToggleActive,
    (_WindowPrivate private, _SvnData entry));
_DeclareFunction(static _Void TogglePartActiveComposite,
    (_WindowPrivate private, _SvnData entry, lwk_domain domain));
_DeclareFunction(static _Void ToggleActivateSimpleObject,
    (_WindowPrivate private, _SvnData entry, _Integer old_active_state));
_DeclareFunction(static _Void ToggleActivateEntry,
    (_WindowPrivate private, _SvnData entry, _Integer old_active_state));
_DeclareFunction(static _Void ToggleActivateComposite,
    (_WindowPrivate private, _SvnData entry, _Integer old_active_state));
_DeclareFunction(static _Void ToggleActivateObjectInComp,
    (_WindowPrivate private, _SvnData entry, _Integer old_active_state));
_DeclareFunction(static _Void ToggleActivateObject,
    (_WindowPrivate private, _SvnData entry, _Integer old_active_state));
_DeclareFunction(static _Integer SwitchActiveState,
    (_Integer active_state));
_DeclareFunction(static _Boolean ActiveStateEqual,
    (_Integer active_state_1, _Integer active_state_2));
_DeclareFunction(static _Void SvnEnvWindowClearRecord,
    (_WindowPrivate private, _SvnData svn_data));
_DeclareFunction(static _Void SetActiveStateOff,
    (_WindowPrivate private, _SvnData entry));

_DeclareFunction(static _Void SvnEnvWindowGetListPointers,
    (_WindowPrivate private, lwk_domain domain, _SvnData **begin_list,
    _SvnData **end_list));
_DeclareFunction(static _Void SvnEnvWindowInsertQuickCopy,
    (_WindowPrivate private, lwk_object object,
     lwk_domain domain, _Integer x, _Integer y));
_DeclareFunction(static _Void SvnEnvWindowInsertEntry,
    (_WindowPrivate private, lwk_object object,
     lwk_domain domain, _SvnData selected_entry));
_DeclareFunction(static _Boolean SvnEnvWindowObjectInSegment,
    (_WindowPrivate private, lwk_object object));
_DeclareFunction(static _Void SvnEnvWindowInsertObject,
    (_WindowPrivate private, _SvnData selected_data, lwk_object object,
    lwk_domain domain, _InsertEntry insert_entry));
_DeclareFunction(static _Void SvnCheckNetStateInNetList,
    (_WindowPrivate private, _SvnData parent));
_DeclareFunction(static _SvnData SvnEnvWindowBuildSegment,
    (_WindowPrivate private, lwk_domain domain));
_DeclareFunction(static _Void SvnEnvWindowRemoveSegment,
    (_WindowPrivate private, _SvnData segment));
_DeclareFunction(static _Void SvnEnvWindowQuickCopyOrMove,
    (_Widget widget, _WindowPrivate private, _Integer x, _Integer y,
    Time timestamp, _Boolean move));
_DeclareFunction(static _Void UpdateNonRetrievableEntries,
    (_WindowPrivate private, _SvnData entry,
    _String linkbase_id, _CString linkbase_name));
_DeclareFunction(static lwk_termination UpdatePersistent,
    (_WindowPrivate *private, lwk_linkbase linkbase, lwk_domain domain,
     lwk_persistent *persistent));
_DeclareFunction(static _Boolean UpdateEntryInDisplay,
    (_WindowPrivate private, lwk_persistent his_obj, _UpdateOperation update));
_DeclareFunction(static _Void SetParentStateOn,
    (_WindowPrivate private, _SvnData parent));
_DeclareFunction(static _Void SvnEnvWindowRetrieveEntry,
    (_WindowPrivate private, _SvnData entry));
_DeclareFunction(static _Void SvnEnvWindowUpdateEntry,
    (_WindowPrivate private, _SvnData entry));


/*
**  Macro Definitions
*/

#define _EnvMapTableInitialSize	100

/*
**  Static Data Definitions
*/

static Pixmap	HsSegmentIcon		= (Pixmap) 0;
static Pixmap	HsEmptyIcon		= (Pixmap) 0;
static Pixmap	HsNotActiveCheckMarkPx	= (Pixmap) 0;
static Pixmap	HsActiveCheckMarkPx	= (Pixmap) 0;
static Pixmap	HsPartActiveCheckMarkPx   = (Pixmap) 0;

/*
**  Global Data Definitions
*/

/*
**  External Data Declarations
*/

_DeclareFunction(_Void EnvSvnRemvEntryDataFromTable,
    (_SvnData entry, _MapTable table));
_DeclareFunction(_Void RemvEntrySiblingsDataFromTable,
    (_SvnData entry, _MapTable table));


_Void  EnvSvnEnvWindow(private)
_WindowPrivate private;

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


_Void  EnvSvnEnvWindowInitialize(private)
_WindowPrivate private;

/*
**++
**  Functional Description:
**	Svn initialization routine.
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
    int		ac = 0;
    Arg		arglist[10];
    XtCallbackRec confirm_cb[2];
    XtCallbackRec select_cb[2];

    confirm_cb[0].callback = (XtCallbackProc) SvnEnvWindowConfirm;
    confirm_cb[0].closure = (XtPointer) private;
    confirm_cb[1].callback = (XtCallbackProc) 0;

    select_cb[0].callback = (XtCallbackProc) EnvSvnWindowSelection;
    select_cb[0].closure = (XtPointer) private;
    select_cb[1].callback = (XtCallbackProc) 0;

    XtSetArg (arglist[ac], DXmSvnNselectAndConfirmCallback, confirm_cb); ac++;
    XtSetArg (arglist[ac], DXmSvnNentrySelectedCallback, select_cb); ac++;

    XtSetValues(private->svn, arglist, ac);

    /*
    ** Manage the svn widget
    */

    XtManageChild(private->svn);

    return;
    }


_Void  EnvSvnEnvWindowLoad(private, entry, envcontext)
_WindowPrivate private;
 _SvnData *entry;

    _HsObject envcontext;

/*
**++
**  Functional Description:
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
    _SvnData	    head_svn_data,
		    seg_svn_data,
		    tmp_svn_data;
    _SvnIterateData iterate_data;

    _StartExceptionBlock

    /*
    ** Create a map table.
    */

    private->map_table = _MapTableCreate(_EnvMapTableInitialSize);

    /*
    ** Allocate an svn_data structure to be the head of the list.
    */

    head_svn_data = EnvSvnWindowCreateSvnData(private, (_SvnData) 0, 1);

    /*
    ** Create an svn_data structure to be a segment.
    */

    seg_svn_data = EnvSvnWindowCreateSvnData(private, head_svn_data, 2);
    seg_svn_data->segment = _NetworkSegment;

    /*
    ** Allocate an iterate data structure.
    */
    iterate_data = (_SvnIterateData)
	_AllocateMem(sizeof(_SvnIterateDataInstance));
    _ClearMem(iterate_data, sizeof(_SvnIterateDataInstance));


    /*
    **  Create a temporary svn_data to start building the list of children
    */

    tmp_svn_data = EnvSvnWindowCreateSvnData(private, seg_svn_data, 3);

    /*
    ** Set the iterate data structure.
    */
    iterate_data->head_svn_data = tmp_svn_data;
    iterate_data->end_list = (_SvnData) 0;

    /*
    ** Iterate over the network list.
    */

    SvnEnvWindowIterateList(private, envcontext, _P_Networks, iterate_data);

    /*
    ** Iterate over the active network list.
    */

    SvnEnvWindowIterateActive(private, envcontext, _P_ActiveNetworks);

    /*
    ** Iterate over the current network list.
    */

    SvnEnvWindowIterateCurrent(private, envcontext, _P_CurrentNetwork);

    /*
    ** If the segment has children hook it to the head svn data children list.
    */

    if (tmp_svn_data->child_cnt != 0) {

	/*
	** Insert the segment to the head svn data.
	*/
	
        EnvSvnWindowInsertInto(head_svn_data, seg_svn_data);
	
	/*
	** Hook the list of children to the segment svn_data.
	*/
	
	seg_svn_data->children = tmp_svn_data->siblings;
	seg_svn_data->child_cnt = tmp_svn_data->child_cnt;
	

	/*
	** Set the list pointers.
	*/
	((_EnvWindowPrivate)(private->specific))->networks_head=
		tmp_svn_data->siblings;
	((_EnvWindowPrivate)(private->specific))->networks_end=
		iterate_data->end_list;
	/*
	** Free the temporary svn_data.
	*/
	
	_FreeMem(tmp_svn_data);
	
	/*
	** Create a new segment.
	*/
	
	seg_svn_data = EnvSvnWindowCreateSvnData(private, head_svn_data, 2);
	
	/*
	**  Create a temporary svn_data to start building the list of children
	*/
	
	tmp_svn_data = EnvSvnWindowCreateSvnData(private, seg_svn_data, 3);
	
	/*
	** Reinitialize work structure.
	*/
	iterate_data->head_svn_data = tmp_svn_data;
	iterate_data->end_list = (_SvnData) 0;
    }

    seg_svn_data->segment = _PathSegment;

    /*
    ** Iterate over the path list
    */

    SvnEnvWindowIterateList(private, envcontext, _P_Paths, iterate_data);

    /*
    ** If the segment has children hook it to the head svn data children list.
    */

    if (tmp_svn_data->child_cnt != 0) {

	/*
	** Insert the segment.
	*/
	
        EnvSvnWindowInsertInto(head_svn_data, seg_svn_data);
	
	/*
	** Hook the list of children to the segment svn_data.
	*/
	
	seg_svn_data->children = tmp_svn_data->siblings;
	seg_svn_data->child_cnt = tmp_svn_data->child_cnt;

	/*
	** Set the list pointers.
	*/
	((_EnvWindowPrivate)(private->specific))->paths_head=
		tmp_svn_data->siblings;
	((_EnvWindowPrivate)(private->specific))->paths_end=
		iterate_data->end_list;
    }

    /*
    ** Iterate over the active path list index.
    */
    SvnEnvWindowIterateActivePaths(private, envcontext, _P_ActivePathIndex);

    /*
    ** Iterate over the current path list.
    */

    SvnEnvWindowIterateCurrent(private, envcontext, _P_Trail);


    _FreeMem(tmp_svn_data);
    _FreeMem(iterate_data);

    *entry = head_svn_data;

    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
        _WhenOthers
	    _FreeMem(head_svn_data);
	
	    if (seg_svn_data != (_SvnData) _NullObject)
		_FreeMem(seg_svn_data);
		
	    if (tmp_svn_data != (_SvnData) _NullObject)
		_FreeMem(tmp_svn_data);
		
            _Reraise;
	
    _EndExceptionBlock

    return;
    }

static _Void  SvnEnvWindowIterateList(private, envcontext, property, iterate_data)
_WindowPrivate private;
 _HsObject envcontext;

    _String property;
 _SvnIterateData iterate_data;

/*
**++
**  Functional Description:
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
    lwk_object		hisobject;
    lwk_status		status;
    lwk_termination	return_value;

    /*
    ** Get the his object associated with the list.
    */

    SvnEnvWindowGetHisObject(envcontext, property, &hisobject);

    /*
    ** Iterate over the composite object and collect the objects it contains.
    */

    status = lwk_iterate(hisobject, lwk_c_domain_object_desc, iterate_data,
	SvnEnvWindowListEnvCtxt, &return_value);

    if (status != lwk_s_success) {
	EnvSvnWindowFreeSvnSiblData(iterate_data->head_svn_data);
	_FreeMem(iterate_data);
	_SaveHisStatus(status);
	_Raise(iterate_failed); /* Iterate on environment list failed */
    }

    return;
    }


static lwk_termination  SvnEnvWindowListEnvCtxt(iterate_data, list, domain, object)
_SvnIterateData iterate_data;

    lwk_list list;
 lwk_domain domain;
 lwk_object *object;

/*
**++
**  Functional Description:
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
    _SvnData	    head_svn_data,
		    svn_data;
    lwk_persistent  persistent_object;
    lwk_domain	    persistent_domain;
    lwk_status	    status;

    head_svn_data = iterate_data->head_svn_data;

    /*
    ** Retrieve the persistent object.
    */

    status = lwk_retrieve(*object, &persistent_object);

    if (status == lwk_s_success) {

	/*
	** Get the persistent object's domain.
	*/
	
	status = lwk_get_domain(persistent_object, &persistent_domain);
	
	if (status != lwk_s_success)
	    _Raise(get_domain_failed); /* Get domain failed on persistent object */
	
	/*
	** Create a HsObject and the svn data structure associated.
	*/
	
	svn_data = EnvSvnWindowCreateHsObject(head_svn_data, persistent_domain,
		    &persistent_object);

	svn_data->retrievable = _Retrievable;
	
	/*
	** Add the entry to the map table.
	*/
	
	_MapTableAddEntry(&((_WindowPrivate)head_svn_data->private)->map_table,
			persistent_object, svn_data);

	/*
	**  Insert the new svn_data in the right place.
	*/
	
	EnvSvnWindowInsertEndOfList(&head_svn_data->siblings, svn_data, _False);

	/*
	** Increment number of children.
	*/
	
	head_svn_data->child_cnt++;
	
	/*
	** Set the iterate data end of list.
	*/
	iterate_data->end_list = svn_data;
	
	/*
	** If it's a composite object, retrieve its children if any.
	*/
	
	switch (persistent_domain) {
	
	    case lwk_c_domain_comp_linknet :
	    case lwk_c_domain_comp_path :
		    EnvSvnWindowRetrieveEntry(svn_data,
		    SvnEnvWindowListComposite);
		    break;
	}
    }

    /*
    ** It's an object descriptor.
    */

    else {

	/*
	** Create a HsObject and the svn data structure associated.
	*/
	
	svn_data = EnvSvnWindowCreateHsObject(head_svn_data, domain, object);

        /*
	** if the linkbase has been deleted mark its retrievable field
	*/
	if (status == lwk_s_no_such_linkbase)
	    svn_data->retrievable = _LbNotRetrievable;

	/*
	**  Insert the new svn_data in the right place.
	*/
	
	EnvSvnWindowInsertEndOfList(&head_svn_data->siblings, svn_data, _False);

	/*
	** Increment number of children.
	*/
	
	head_svn_data->child_cnt++;

	/*
	** Set the iterate data end of list.
	*/

	iterate_data->end_list = svn_data;
    }

    return (lwk_termination) 0;
    }


static _Void  SvnEnvWindowIterateActivePaths(private, envcontext, property)
_WindowPrivate private;

    _HsObject envcontext;
 _String property;

/*
**++
**  Functional Description:
**	Iterate over the path index property to recreate the active paths for
**	the Svn display.
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
    _IterateActiveIndex	iterate_data;
    lwk_object		hisobject;
    lwk_status		status;
    lwk_termination	return_value;

    /*
    ** Get the his object associated with the list.
    */

    SvnEnvWindowGetHisObject(envcontext, property, &hisobject);

    /*
    ** Allocate memory for the iterate data structure
    */
    iterate_data = (_IterateActiveIndex)
	_AllocateMem(sizeof(_IterateActiveIndexInstance));
    _ClearMem(iterate_data,sizeof(_IterateActiveIndexInstance));
    
    iterate_data->head = ((_EnvWindowPrivate)(private->specific))->paths_head;
    iterate_data->prev_index_entry = (_SvnData) 0;
    iterate_data->good_sequence = _True;
    iterate_data->end_seq_obj = hisobject;

    /*
    ** Iterate over the composite object and collect the objects it contains.
    */

    status = lwk_iterate(hisobject, lwk_c_domain_object_desc, iterate_data,
	SvnEnvWindowListActivePaths, &return_value);

    if (status != lwk_s_success) {
	_FreeMem(iterate_data);
	_SaveHisStatus(status);
	_Raise(iterate_failed); /* Iterate on environment list failed */
    }

    /*
    ** Free the iterate data structure.
    */
    _FreeMem(iterate_data);

    return;
    }



static lwk_termination  SvnEnvWindowListActivePaths(iterate_data, list, domain, object_desc)

    _IterateActiveIndex iterate_data;
 lwk_list list;
 lwk_domain domain;

    lwk_object_descriptor *object_desc;

/*
**++
**  Functional Description:
**	During the iteration you keep track of the entry matching the previous
**	visited object descriptor. When we are on the end sequence object
**	descriptor (the object descriptor of the active path index), we are at
**	the end of a sequence and we have to activate the entry, else we have
**	to find the matching entry.
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
    _SvnData	    last_active_entry,
		    prev_entry,
		    prev_entry_sibl,
		    matched_entry;
    lwk_persistent  persistent_object;
    lwk_domain	    persistent_domain;
    lwk_status	    status;
    _Integer        type;

    /*
    ** Retrieve the persistent object.
    */
    status = lwk_retrieve(*object_desc, &persistent_object);

    if (status != lwk_s_success) {

        /*
	** The object cannot be retrieved, so this is a bad sequence.
	*/
	iterate_data->good_sequence = _False;
    }

    else {

        /*
	** Check if you are at the end of a sequence.
	*/

	if (persistent_object == iterate_data->end_seq_obj) {

            /*
	    ** If the sequence if a good one treat the previous object
	    */
	    if (iterate_data->good_sequence == _True) {

		prev_entry = (_SvnData) iterate_data->prev_index_entry;
		
		/*
		** The previous index entry should be activated
		*/
		if (prev_entry != (_SvnData) 0) {

		    last_active_entry = (_SvnData) 0;

		    /*
		    ** Search if one of the potential siblings of the previous index
		    ** entry is not already activated
		    */
		    prev_entry_sibl = (_SvnData) prev_entry->siblings;
			
		    while (prev_entry_sibl != (_SvnData) 0) {

			if (prev_entry_sibl->active == _Active)
			    last_active_entry = prev_entry_sibl;

			prev_entry_sibl = prev_entry_sibl->siblings;
		    }

		    if (last_active_entry == (_SvnData) 0) {

			/*
			** Active the previous index entry
			*/

			_GetValue(prev_entry->object, _P_HisType,
			    hs_c_domain_integer, &type);

                        persistent_domain = (lwk_domain) type;

			if (persistent_domain == lwk_c_domain_path)
			    ToggleActivateSimpleObject(prev_entry->private,
				prev_entry, _NotActive);

			else
			    if (prev_entry->children != (_SvnData) 0) {
			
				ToggleActivateComposite(prev_entry->private,
				    prev_entry, _NotActive);

				/*
				** if the active state of the composite has not
				** changed, it's because one of its child
				** active state cannot be changed, so change it
				** now.
				*/
				
				if (prev_entry->active == _NotActive)
				     TogglePartActiveComposite(
					prev_entry->private, prev_entry,
					persistent_domain);
			    }
		    }
		
		    else {

			/*
			** Search after the last active entry an entry matching the
			** object of previous entry
			*/
			_GetValue(prev_entry->object, _P_HisObject,
			    hs_c_domain_lwk_object, &persistent_object);

			if (persistent_object != (lwk_object) 0) {
			
			    FindObjEntryInSiblings(last_active_entry,
				persistent_object, &matched_entry);

			    /*
			    ** Active the matched entry if any
			    */
			    if (matched_entry != (_SvnData) 0)
				matched_entry->active = _Active;
			}
		    }
				
		    /*
		    ** Reset the previous index entry to null
		    */
		    iterate_data->prev_index_entry = (_SvnData) 0;
			
		}
	    }

	    else

                /*
		** We are at the end of a bad sequence, prepare for a new good
		** sequence
		*/
		iterate_data->good_sequence == _True;

            /*
	    ** Prepare for a new sequence.
	    */
	    iterate_data->prev_index_entry = (_SvnData) 0;

	}

	else {

            /*
	    ** We are not at the end of a sequence. Treat the object only if we
	    ** are in a good sequence
	    */
	    if (iterate_data->good_sequence == _True) {

		prev_entry = (_SvnData) iterate_data->prev_index_entry;

		if (prev_entry == (_SvnData) 0)

                    /*
		    ** Search an entry matching the object in the first level
		    ** siblings
		    */
		    FindObjEntryInSiblings(iterate_data->head,
			persistent_object, &matched_entry);

		else {

                    /*
		    ** Search an entry matching the object in the eventual
		    ** children of previous index entry
		    */
		    if (prev_entry->children != (_SvnData) 0)
			FindObjEntryInSiblings(prev_entry->children,
			    persistent_object, &matched_entry);
		    else
                        /*
			** This is an invalid sequence
			*/
			iterate_data->good_sequence = _False;
		}

		/*
		** If a matched entry has been found, save it in the iterate
		** data structure.
		*/
		if (matched_entry != (_SvnData) 0)
		    iterate_data->prev_index_entry = matched_entry;
		
	    }
	}
    }

    return (lwk_termination) 0;
    }


static _Void  FindObjEntryInSiblings(from_entry, persistent_object, matched_entry)
_SvnData from_entry;

    lwk_object persistent_object;
 _SvnData *matched_entry;

/*
**++
**  Functional Description:
**	Search in from_entry and its siblings if there is an entry matching the
**	persistent object
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
    _SvnData	entry;
    lwk_object	his_obj;

    entry = from_entry;
    *matched_entry = (_SvnData) 0;

    while ((entry != (_SvnData) 0) && (*matched_entry == (_SvnData) 0)) {

	_GetValue(entry->object, _P_HisObject, hs_c_domain_lwk_object, &his_obj);

	if (his_obj == persistent_object)
	    *matched_entry = entry;

	entry = entry->siblings;
    }

    return;
    }


static lwk_termination  SvnEnvWindowListComposite(head_svn_data, list, domain, object)
_SvnData head_svn_data;

    lwk_list list;
 lwk_domain domain;
 lwk_object *object;

/*
**++
**  Functional Description:
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
    _SvnData	    svn_data;
    lwk_persistent  persistent_object;
    lwk_domain	    persistent_domain;
    lwk_status	    status;

    /*
    ** Retrieve the persistent object.
    */

    status = lwk_retrieve(*object, &persistent_object);

    if (status == lwk_s_success) {

	/*
	** Get the persistent object's domain.
	*/
	
	status = lwk_get_domain(persistent_object, &persistent_domain);
	
	if (status != lwk_s_success)
	    _Raise(get_domain_failed); /* Get domain failed on persistent object */
	
	/*
	** Create a HsObject and the svn data structure associated.
	*/
	
	svn_data = EnvSvnWindowCreateHsObject(head_svn_data, persistent_domain,
		    &persistent_object);

	svn_data->retrievable = _Retrievable;
	
	/*
	** Add the entry to the map table.
	*/
	
	_MapTableAddEntry(&((_WindowPrivate)head_svn_data->private)->map_table,
			persistent_object, svn_data);

	/*
	**  Insert the new svn_data in the right place.
	*/
	
	EnvSvnWindowInsertEndOfList(&head_svn_data->siblings, svn_data, _False);

	/*
	** Increment number of children.
	*/
	
	head_svn_data->child_cnt++;
	
	/*
	** If it's a composite object, retrieve its children if any.
	*/
	
	switch (persistent_domain) {
	
	    case lwk_c_domain_comp_linknet :
	    case lwk_c_domain_comp_path :
		    EnvSvnWindowRetrieveEntry(svn_data,
		    SvnEnvWindowListComposite);
		    break;
	}
    }

    /*
    ** It's an object descriptor.
    */

    else {

	/*
	** Create a HsObject and the svn data structure associated.
	*/
	
	svn_data = EnvSvnWindowCreateHsObject(head_svn_data, domain, object);

        /*
	** if the linkbase has been deleted mark its retrievable field
	*/
	if (status == lwk_s_no_such_linkbase)
	    svn_data->retrievable = _LbNotRetrievable;

	/*
	**  Insert the new svn_data in the right place.
	*/
	
	EnvSvnWindowInsertEndOfList(&head_svn_data->siblings, svn_data, _False);

	/*
	** Increment number of children.
	*/
	
	head_svn_data->child_cnt++;
    }

    return (lwk_termination) 0;
    }


static _Void  SvnEnvWindowIterateActive(private, envcontext, property)
_WindowPrivate private;
 _HsObject envcontext;

    _String property;

/*
**++
**  Functional Description:
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
    lwk_object		hisobject;
    lwk_status		status;
    lwk_termination	return_value;


    /*
    ** Get the his object associated with the list.
    */

    SvnEnvWindowGetHisObject(envcontext, property, &hisobject);

    /*
    ** Iterate over the composite object and set the active objects.
    */

    status = lwk_iterate(hisobject, lwk_c_domain_object_desc, private,
	SvnEnvWindowListActiveNets, &return_value);
	
    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(iterate_failed); /* Iterate on active list failed */
    }
	
    return;
    }


static lwk_termination  SvnEnvWindowListActiveNets(private, list, domain, object)
_WindowPrivate private;

	lwk_list list;
 lwk_domain domain;
 lwk_object *object;

/*
**++
**  Functional Description:
**	We should have only linknets in the active linknet list, but to be
**	version compatible with linkbases created for V1.0, we need to treat
**	also composite linknets.
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
    _SvnData	    *entries;
    _Integer	    count;
    _Integer	    i;
    lwk_persistent  persistent_object;
    lwk_status	    status;

    /*
    ** Retrieve the persistent object.
    */

    status = lwk_retrieve(*object, &persistent_object);

    if (status == lwk_s_success) {
	/*
	** Get its domain.
	*/
	status = lwk_get_domain(persistent_object, &domain);

	if (status != lwk_s_success)
	    _Raise(get_domain_failed);

	/*
	** Get the svn entries for this object and set them to active.
	*/
	
	if (_MapTableGetEntry(private->map_table, persistent_object, &entries,
	    &count)) {
	
	    for (i = 0; i < count; i++)

		switch (domain) {

		    /*
		    ** If it's a composite object, set its children to active.
		    */
		    case lwk_c_domain_comp_linknet:
			if (entries[i]->children != (_SvnData) 0)
			    ToggleActivateComposite(private, entries[i],
				_NotActive);
				
			/*
			** if the active state of the composite has not changed,
			** it's because one of its child active state cannot
			** be changed, so change it now.
			*/
			if (entries[i]->active == _NotActive)
			     TogglePartActiveComposite(private, entries[i],
				domain);
			
			break;

		    case lwk_c_domain_linknet:
			if (entries[i]->active == _NotActive) {
			    entries[i]->active = _Active;

			/*
			** Mark eventually the active state of its parent
			*/
			if (entries[i]->parent != (_SvnData) 0)
			    SetParentStateOn(private, entries[i]->parent);
			
			}
			break;
		}
	}
    }

    return ((lwk_termination) 0);
    }

static _Void  SvnEnvWindowIterateCurrent(private, envcontext, property)
_WindowPrivate private;

    _HsObject envcontext;
 _String property;

/*
**++
**  Functional Description:
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
    lwk_object		hisobject;
    lwk_status		status;
    lwk_termination	return_value;

    /*
    ** Get the his object associated with the list.
    */

    SvnEnvWindowGetHisObject(envcontext, property, &hisobject);

    /*
    ** Iterate over the composite object and set the current object.
    */

    status = lwk_iterate(hisobject, lwk_c_domain_object_desc, private,
	SvnEnvWindowListCurrent, &return_value);
	
    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(iterate_failed);    /* Iterate on current list failed */
    }
	
    return;
    }


static lwk_termination  SvnEnvWindowListCurrent(private, list, domain, object)
_WindowPrivate private;

	lwk_list list;
 lwk_domain domain;
 lwk_object *object;

/*
**++
**  Functional Description:
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
    lwk_persistent  persistent_object;
    lwk_domain	    persistent_domain;
    lwk_status	    status;

    /*
    ** Retrieve the persistent object.
    */

    status = lwk_retrieve(*object, &persistent_object);

    if (status == lwk_s_success) {

	/*
	** Get the persistent object's domain.
	*/
	
	status = lwk_get_domain(persistent_object, &persistent_domain);
	
	if (status != lwk_s_success)
	    _Raise(get_domain_failed); /* Get domain failed on persistent object */

	/*
	** Set the current object in the private structure.
	*/
	
	if (persistent_domain == lwk_c_domain_linknet)
	    ((_EnvWindowPrivate)(private->specific))->current_network = persistent_object;
	else
	    ((_EnvWindowPrivate)(private->specific))->trail = persistent_object;
    }
    return ((lwk_termination) 0);
    }


static _Void  SvnEnvWindowGetHisObject(envcontext, property, hisobject)
_HsObject envcontext;
 _String property;

    lwk_object *hisobject;

/*
**++
**  Functional Description:
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
    _HsObject objects;

    /*									
    ** Get the hs Object list.
    */

    _GetValue(envcontext, property, hs_c_domain_hsobject, &objects);

    /*
    ** Get the associated his composite object.
    */

    _GetValue(objects, _P_HisObject, hs_c_domain_lwk_object, hisobject);

    return;
    }


_Void  EnvSvnEnvWindowDisplayEnv(private)
_WindowPrivate private;

/*
**++
**  Functional Description:
**	This routine adds the environment entries in the Svn display list.
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
    ** Add the title entry.
    */

    SvnEnvWindowSetTitle(private);

    /*
    ** Expand the segment entries if any.
    */

    SvnEnvWindowExpandSegment(private);

    /*
    ** Clear the selection.
    */

    DXmSvnClearSelections(private->svn);

    return;
    }


static _Void  SvnEnvWindowSetTitle(private)
_WindowPrivate private;

/*
**++
**  Functional Description:
**	This routine sets the information associated with a particular
**	entry.
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
    _CString cs_name;
    _CString cs_active;
    _CString cs_record;
    _CString cs_linkbase;
    _Boolean index = _True;
    _Integer entry_number = 1;

    /*
    ** Add the entry to the svn display list.
    */

    DXmSvnAddEntries(private->svn, 0, 1, 0,
    (XtPointer *) &private->svn_entries, index);

    /*
    ** Increment the number of open entries.
    */

    private->opened_entries++;

    /*
    ** Set the title.
    */

    EnvDWGetCStringLiteral(_DwtSvnTitleName, &cs_name);
    EnvDWGetCStringLiteral(_DwtSvnTitleActive, &cs_active);
    EnvDWGetCStringLiteral(_DwtSvnTitleRecord, &cs_record);
    EnvDWGetCStringLiteral(_DwtSvnTitleLinkbase, &cs_linkbase);

    if (HsEmptyIcon == NULL)
	EnvDWFetchIcon(private, _DrmEmptyIcon, &HsEmptyIcon, _False);

    /*
    ** Set the column header entry to insensitive.
    */
    DXmSvnSetEntrySensitivity(private->svn, entry_number, _False);
    
    /*
    ** Set the user data.
    */

    DXmSvnSetEntryTag(private->svn, entry_number,
	(XtPointer) private->svn_entries);

    /*
    ** Set the number of components.
    */

    DXmSvnSetEntryNumComponents(private->svn, entry_number, 5);

    /*
    ** Set the components.
    */

    DXmSvnSetComponentPixmap(private->svn, entry_number, _EnvSvnIconComponent,
	0, 0, HsEmptyIcon, _IconHeight, _IconWidth);

    DXmSvnSetComponentText(private->svn, entry_number, _EnvSvnNameComponent,
	20, 0, (XmString) cs_name, NULL);

    DXmSvnSetComponentText(private->svn, entry_number,
	_EnvSvnActivateComponent, 60, 0, (XmString) cs_active, NULL);

    DXmSvnSetComponentText(private->svn, entry_number,
	_EnvSvnRecordComponent, 80, 0, (XmString) cs_record, NULL);

    DXmSvnSetComponentText(private->svn, entry_number,
	_EnvSvnLinkbaseComponent, 100, 0, (XmString) cs_linkbase, NULL);
	
    return;
    }


static _Void  SvnEnvWindowExpandSegment(private)
_WindowPrivate private;

/*
**++
**  Functional Description:
**	This routine sets the information associated with a environment window
**	display list.
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
    int		entry_number;
    _SvnData	svn_data,
		child;

    svn_data = private->svn_entries;

    /*
    ** Check if the given entry has children.
    */

    if (svn_data->children != NULL) {

	entry_number = DXmSvnGetEntryNumber(private->svn,
	    (XtPointer) svn_data);

	child = svn_data->children;

	while (child != NULL) {
	
	    /*
	    ** Add the entry.
	    */
	
	    DXmSvnAddEntries(private->svn, entry_number, 1,
		(child->child_level - 1), (XtPointer *) &child, _True);
	    private->opened_entries++;

	    /*
	    ** Set the entry.
	    */
	
	    SvnEnvWindowSetEntry(private, child);

	    /*
	    ** Expand all children of the segment.
	    */
	
	    EnvSvnEnvWindowExpand(private, child);
	    child = child->siblings;
	
	    /*
	    ** Increment the entry_number.
	    */
	
	    entry_number = private->opened_entries;
	}
    }

    return;
    }


_Void  EnvSvnEnvWindowExpand(private, svn_data)
_WindowPrivate private;
 _SvnData svn_data;

/*
**++
**  Functional Description:
**	This routine sets the information associated with a particular
**	entry.
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
    ** Call svn expand routine.
    */

    EnvSvnWindowExpand(private, svn_data, SvnEnvWindowSetEntry);

    return;
    }


static _Void  SvnEnvWindowSetEntry(private, svn_data)
_WindowPrivate private;
 _SvnData svn_data;

/*
**++
**  Functional Description:
**	This routine sets the information associated with a particular
**	entry.
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
    _CString	cs_name;
    _CString	cs_linkbase;
    _String	lb_id;
    _Integer	type;
    _Integer	desc_domain;
    _Boolean	index = _False;
    Pixmap	pixmap;
    int		entry_number;
    _Boolean    small_font;

    /*
    ** Get the entry number.
    */

    entry_number = DXmSvnGetEntryNumber(private->svn, (XtPointer) svn_data);

    /*
    ** Invalidate the entry so that it gets redrawn.
    */
    DXmSvnInvalidateEntry(private->svn, entry_number);

    /*
    ** Set the font flag.
    */
    small_font = _False;

    /*
    ** If it's an HsObject.
    */

    if (svn_data->object != _NullObject) {

	/*
	** Get the object domain.
	*/
	
	_GetValue(svn_data->object, _P_HisType, hs_c_domain_integer, &type);

	/*
	** It's an object descriptor.
	*/
	
	if ((lwk_domain)type == lwk_c_domain_object_desc) {
	
	    /*
	    ** Get the object name and the linkbase name.
	    */
	
	    _GetCSProperty(svn_data->object, lwk_c_p_object_name, &cs_name);
	    _GetCSProperty(svn_data->object, lwk_c_p_linkbase_name,
		&cs_linkbase);
				
	    /*
	    ** It's not a real object, only a descriptor.
	    ** But we need to know its real domain to set the pixmap.
	    */
	
	    _GetProperty(svn_data->object, lwk_c_domain_integer,
		lwk_c_p_object_domain, &desc_domain);
	    pixmap = EnvSvnWindowSetPixmap(private, (lwk_domain) desc_domain);
	
	}
	
	/*
	** It's a real object.
	*/
	
	else {
	
	    /*
	    ** Get its name and the linkbase name.
	    */
	
	    _GetCSProperty(svn_data->object, lwk_c_p_name, &cs_name);
	
	    if (cs_name == (_CString) 0)
		_GetCSProperty(svn_data->object, lwk_c_p_description, &cs_name);
		
	    if (cs_name == (_CString) 0)
		EnvDWGetCStringLiteral(_DwtSvnUnnamed, &cs_name);
			
	    _GetLinkbase(svn_data->object, &cs_linkbase, &lb_id);
	
	    /*
	    ** Set the pixmap.
	    */
	
	    pixmap = EnvSvnWindowSetPixmap(private, type);
	
	    /*
	    ** Set the index to false
	    */
	
	    index = _False;
	}
	
	/*
	** If it's a non retrievable object, change the font.
	*/
	if (svn_data->retrievable != _Retrievable) {
	    /*
	    ** Set the entry to a different font or to insensitive.
	    */
	    if (private->small_font == NULL)
		DXmSvnSetEntrySensitivity(private->svn, entry_number, _False);
	    else
		small_font = _True;
	}
    }
    else {

	/*
	** Check if it's a segment.
	*/
	
	if (svn_data->segment != 0) {
	
	    /*
	    ** Set the segment
	    */
	
	    if (svn_data->segment == _NetworkSegment)
		EnvDWGetCStringLiteral(_DwtSvnNetworkName, &cs_name);
	    else
		EnvDWGetCStringLiteral(_DwtSvnPathName, &cs_name);
		
	    cs_linkbase = (_CString) _NullObject;

	    /*
	    ** Set the pixmap.
	    */
	
	    if (HsSegmentIcon == NULL)
		EnvDWFetchIcon(private, _DrmSegmentIcon,&HsSegmentIcon, _False);
		
	    pixmap = HsSegmentIcon;
	
	    /*
	    ** Set the index.
	    */
	
	    index = _True;
	}
    }

    /*
    ** Set the entry.
    */

    SvnEnvWindowSetComponents(private, svn_data, entry_number, pixmap, cs_name,
	cs_linkbase, index, small_font);	

    return;
    }


static _Void  SvnEnvWindowSetComponents(private, svn_data, entry_number, pixmap, name, linkbase_name, index, small_font)
_WindowPrivate private;

    _SvnData svn_data;
 int entry_number;
 Pixmap pixmap;
 _CString name;

    _CString linkbase_name;
 _Boolean index;
 _Boolean small_font;

/*
**++
**  Functional Description:
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
    XmFontList  font;
    XmFontList  lb_font;

    /*
    ** Set the user data.
    */

    DXmSvnSetEntryTag(private->svn, entry_number, (XtPointer) svn_data);

    /*
    ** Set the entry for the index window.
    */

    DXmSvnSetEntryIndexWindow(private->svn, entry_number, index);

    /*
    ** Set the font.
    */

    if (small_font) {

        /*
	** font for the entry
	*/
	
	font = private->small_font;

        /*
	** font for the linkbase
	*/
	
	if (svn_data->retrievable == _LbNotRetrievable)
	    lb_font = font;
	else
	    lb_font = NULL;
    }
    else {
	font = NULL;
	lb_font = NULL;
    }
	
    /*
    ** Set the number of components.
    */

    if (svn_data->segment == 0)

	if (linkbase_name != (_CString) _NullObject) {

	    DXmSvnSetEntryNumComponents(private->svn, entry_number, 5);

	    DXmSvnSetComponentText(private->svn, entry_number,
		_EnvSvnLinkbaseComponent, 100, 0, (XmString) linkbase_name,
		lb_font);
	}
	else {
	    DXmSvnSetEntryNumComponents(private->svn, entry_number, 4);
	}
    else
	DXmSvnSetEntryNumComponents(private->svn, entry_number, 2);

    /*
    ** Set the other components
    */

    DXmSvnSetComponentPixmap(private->svn, entry_number, _EnvSvnIconComponent,
	0, 0, pixmap, _IconHeight, _IconWidth);

    DXmSvnSetComponentText(private->svn, entry_number, _EnvSvnNameComponent,
	20, 0, (XmString) name, font);
	
    if (svn_data->segment == 0) {

	SvnEnvWindowToggleActive(private, svn_data);	
	SvnEnvWindowSetRecording(private, svn_data);
    }

    return;
    }


static _Void  SvnEnvWindowSetRecording(private, svn_data)
_WindowPrivate private;
 _SvnData svn_data;

/*
**++
**  Functional Description:
**	This routine sets the information associated with a particular
**	entry.
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
    _Integer	type,
		entry_number;
    lwk_object  current_object,
		his_obj;
    _Integer	check_mark;

    check_mark = _NotActiveCheckMark;

    entry_number = DXmSvnGetEntryNumber(private->svn, (XtPointer) svn_data);

    if (svn_data->object != (_HsObject) _NullObject) {
	/*
	** Get its domain and check if it can be current.
	*/
	_GetValue(svn_data->object, _P_HisType, hs_c_domain_integer, &type);

	if (((lwk_domain)type == lwk_c_domain_linknet) ||
	    ((lwk_domain)type == lwk_c_domain_path)) {
	    /*
	    ** Get the his object.
	    */
	    _GetValue(svn_data->object, _P_HisObject, hs_c_domain_lwk_object, &his_obj);

	    /*
	    ** Compare it with the current object of the same domain and set the
	    ** check mark.
	    */
	    switch ((lwk_domain)type) {
		case lwk_c_domain_linknet:
		    current_object =
			((_EnvWindowPrivate)(private->specific))->current_network;
		    break;
		case lwk_c_domain_path:	
		    current_object =
			((_EnvWindowPrivate)(private->specific))->trail;
		    break;
	    }
	    if (his_obj == current_object)
		check_mark = _ActiveCheckMark;
	}
    }
    SvnEnvWindowSetCheckMarkComponent(private, entry_number,
			_EnvSvnRecordComponent, check_mark);
    return;
    }

static _Void  SvnEnvWindowSetCheckMarkComponent(private, entry_number, component, state)
_WindowPrivate private;

    _Integer entry_number;
 _Integer component;
 _Integer state;

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
    ** Invalidate the entry so that it gets redrawn.
    */
    DXmSvnInvalidateEntry(private->svn, entry_number);

    switch (state) {

	case _ActiveCheckMark :
	
	    if (HsActiveCheckMarkPx == (Pixmap) 0)
	
		/*
		**  Fetch the pixmap if needed
		*/
		
		EnvDWFetchIcon(private, _DwtSvnActiveCheckMarkIcon,
		    &HsActiveCheckMarkPx, _False);

	    /*
	    **  Update the pixmap component in the Svn display
	    */
	
	    DXmSvnSetComponentPixmap(private->svn, entry_number, component,
		_XOffset, _YOffset, HsActiveCheckMarkPx, _IconWidth,
		_IconHeight);
	    break;

	case _NotActiveCheckMark :
	
	    if (HsNotActiveCheckMarkPx == (Pixmap) 0)
	
		/*
		**  Fetch the pixmap if needed
		*/
		
		EnvDWFetchIcon(private, _DwtSvnNotActiveCheckMarkIcon,
		    &HsNotActiveCheckMarkPx, _False);
	    /*
	    **  Update the pixmap component in the Svn display
	    */

	    DXmSvnSetComponentPixmap(private->svn, entry_number, component,
		_XOffset, _YOffset, HsNotActiveCheckMarkPx, _IconWidth,
		_IconHeight);
	    break;
	
	case _PartActiveCheckMark :
	
	    if (HsPartActiveCheckMarkPx == (Pixmap) 0)
	
		/*
		**  Fetch the pixmap if needed
		*/
		
		EnvDWFetchIcon(private, _DwtSvnPartActiveCheckMarkIcon,
		    &HsPartActiveCheckMarkPx, _False);
	    /*
	    **  Update the pixmap component in the Svn display
	    */

	    DXmSvnSetComponentPixmap(private->svn, entry_number, component,
		_XOffset, _YOffset, HsPartActiveCheckMarkPx, _IconWidth,
		_IconHeight);
	    break;
    }

    return;
    }

static _Void  SvnEnvWindowConfirm(w, private, cb)
Widget w;
 _WindowPrivate private;

    DXmSvnCallbackStruct *cb;

/*
**++
**  Functional Description:
**	SelectAndConfirm callback routine.  This routine is called when one and
**	only one entry is selected by a double click MB1.  The entry number
**	selected is provided in the callback structure.
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**
**	Valid SVN arguments in cb:
**	    reason
**	    entry_number
**	    component_number
**	    entry_tag
**	    time
**	    entry_level
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _SvnData	    svn_data;
    _IsExpandable   expand;
    _SelectData	    select_data;

    svn_data = (_SvnData) cb->entry_tag;

    /*
    ** Disable display.
    */

    DXmSvnDisableDisplay(private->svn);

    _StartExceptionBlock

    /*
    ** Display watch cursor.
    */

    _SetCursor(private->window, _WaitCursor);

    /*
    **  Get the selected items in Svn
    */

    _GetSelection(private->window, _SingleSelection, &select_data);


    switch(cb->component_number) {

	case _EnvSvnIconComponent:
	case _EnvSvnNameComponent:
	
	    /*		
	    **	Determine if the entry can be expanded. If so, retrieve the
	    **	entry tag value for the given entry number. If the correponding
	    **	svn_data is opened, close it. Otherwise open it.
	    */
			
	    expand = EnvSvnWindowIsExpandable(private, svn_data,
		cb->entry_number, cb->component_number);
			
	    switch (expand) {

		case _DimItem:
		    XBell(XtDisplay(private->svn), 0);
		    break;

		case _ExpandEntry:
		    _Expand(private->window, svn_data);
		    break;
		
		case _CollapseEntry:
		    _Collapse(private->window, svn_data);
		    break;
		
		default:
		    break;
	    }
	    break;
	
	case _EnvSvnActivateComponent:

	    /*
	    ** Toggle the entry active check mark.
	    */

	    if (svn_data->object != _NullObject
		    && select_data != (_SelectData) 0 ) {
		EnvSvnEnvWindowToggleActivate(private, select_data);

		/*
		** Depending on what has been activated, update the currency.
		*/
		if (_SetWindowState(private->window, _StateActiveNetworks,
			_StateGet)) {
		    EnvDWEnvWinUpdateCurrencyCNet(private);
		    _SetWindowState(private->window, _StateActiveNetworks,
			_StateClear);
		}

		if (_SetWindowState(private->window, _StateActivePaths,
			_StateGet)) {
		    EnvDWEnvWinUpdateCurrencyCPath(private);
		    _SetWindowState(private->window, _StateActivePaths,
			_StateClear);
		}
	    }
		
	    break;
	
	case _EnvSvnRecordComponent:

	    /*
	    ** Toggle the entry record check mark.
	    */

	    if (svn_data->object != _NullObject) {
		EnvSvnEnvWindowToggleRecording(private, select_data);
	    }

	    break;
	
	/*
	** If linkbase name, open it in a new window if isn't already opened.
	*/
	
	case _EnvSvnLinkbaseComponent:
	    if (svn_data->object != _NullObject) {
		EnvDWLbWindowOpenLinkbase(private, svn_data->object);
	    }
	    
	    break;
	
	default:
	    break;	
    }
		
    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions

        _WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(sel_conf_cb_failed);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	
    _EndExceptionBlock

    DXmSvnEnableDisplay(private->svn);

    _SetCursor(private->window, _DefaultCursor);

    return;
    }

_Void  EnvSvnEnvWindowToggleRecording(private, select_data)
_WindowPrivate private;

    _SelectData select_data;

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
    lwk_domain		domain;
    _Integer            type;
    _SvnData		entry;
    _Integer		count;
    _Integer		i;
    _SvnData		*entries;

    /*
    **  Check validity of info.  We do this again because of the double click
    */

    if (select_data == (_SelectData) 0)
	return;

    entry = select_data->svn_data[0];

    _GetValue(entry->object, _P_HisType, hs_c_domain_integer, &type);

    domain = (lwk_domain) type;

    if ((domain != lwk_c_domain_linknet) && (domain != lwk_c_domain_path))
	XBell(XtDisplay(private->svn), 0);
    else
	SvnEnvWindowToggleRecord(private, entry);

    return;
    }


static _Void  SvnEnvWindowToggleRecord(private, entry)
_WindowPrivate private;
 _SvnData entry;

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
    _EnvContext		    env_ctxt;
    _CurrencyFlag	    currency;
    _Integer		    i;
    _Integer		    count;
    _Integer		    cnt;
    _Integer		    entry_number;
    _Integer		    type;
    _SvnData		    *entries;
    _SvnData		    *cur_entries;
    _Status		    stat[1];
    lwk_list		    list;
    lwk_object		    his_obj;
    lwk_status		    status;
    lwk_object_descriptor   obj_dsc;
    lwk_object		    current_object;

    /*
    **	Get the object domain
    */

    _GetValue(entry->object, _P_HisType, hs_c_domain_integer, &type);

    switch ((lwk_domain)type) {

	case lwk_c_domain_linknet :
	    current_object =
		((_EnvWindowPrivate)(private->specific))->current_network;
	    currency = lwk_c_env_recording_linknet;
	    status = lwk_create_set(lwk_c_domain_object_desc, 1, &list);
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(create_list_failed); /* his set creation failed */
	    }
	    break;

	case lwk_c_domain_path :
	    current_object =
		((_EnvWindowPrivate)(private->specific))->trail;
	    currency = lwk_c_env_recording_path;
	    status = lwk_create_list(lwk_c_domain_object_desc, 1, &list);
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(create_list_failed); /* his list creation failed */
	    }
	    break;

	default :
	    _Raise(inv_domain);
	    break;
    }

    /*
    **  Get the list of the Svn entries for the new current object.
    */

    _GetValue(entry->object, _P_HisObject, hs_c_domain_lwk_object, &his_obj);

    _MapTableGetEntry(private->map_table, his_obj, &entries, &count);

    /*
    **  Get the environment context object from the window object
    */

    _GetValue(private->window, _P_EnvironmentContext,
	hs_c_domain_environment_ctxt, &env_ctxt);

    if (his_obj == current_object) {

	/*
	**  Turn off recording
	*/
	
	current_object = lwk_c_null_object;

	for (i = 0; i < count; i++) {

	    entry_number = DXmSvnGetEntryNumber(private->svn,
		(XtPointer) entries[i]);

	    if (entry_number > 0)
		SvnEnvWindowSetCheckMarkComponent(private, entry_number,
		    _EnvSvnRecordComponent, _NotActiveCheckMark);
	}
		
	/*
	**  Update the currency
	*/
	
	_SetContextCurrency(env_ctxt, currency, list, (_HsObject) _NullObject);
    }

    else {

	if (current_object != lwk_c_null_object) {

        /*
	** Get the list of the Svn entries for the current object
	*/

	_MapTableGetEntry(private->map_table, current_object, &cur_entries,
	    &cnt);

	if (cnt > 0)

	    /*
	    ** Turn off recording
	    */
	    for (i = 0; i < cnt; i++) {

		entry_number = DXmSvnGetEntryNumber(private->svn,
		    (XtPointer) cur_entries[i]);

		if (entry_number > 0)
		    SvnEnvWindowSetCheckMarkComponent(private, entry_number,
			_EnvSvnRecordComponent, _NotActiveCheckMark);
	    }
	}

	/*
	**  Switch currency to new object
	*/

	current_object = his_obj;

	for (i = 0; i < count; i++) {

	    entry_number = DXmSvnGetEntryNumber(private->svn,
		(XtPointer) entries[i]);

	    if (entry_number > 0)
		SvnEnvWindowSetCheckMarkComponent(private, entry_number,
		    _EnvSvnRecordComponent, _ActiveCheckMark);
	}
	
	/*
	**  Add the current object in the list or set
	*/
		
	status = lwk_get_object_descriptor(his_obj, &obj_dsc);
	
	if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(get_objdsc_failed); /* get obj descript failed */
	}
	
	status = lwk_add_element(list, lwk_c_domain_object_desc, &obj_dsc);

	if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(add_ele_failed); /* his add element failed */
	}

	/*
	**  Update the currency
	*/
	
	_SetContextCurrency(env_ctxt, currency, list, entry->object);

	/*
	**  A path cannot be active and set to record at the same time, so
	**  if one entry is active turn off its active state.
	*/

	if ((lwk_domain) type == lwk_c_domain_path) {

	    for (i = 0; i < count; i++) {

		entry_number = DXmSvnGetEntryNumber(private->svn,
		    (XtPointer) entries[i]);

		if (entries[i]->active == _Active) {

                    /*
		    ** Set the active path flag.
		    */
		    _SetWindowState(private->window, _StateActivePaths,
			_StateSet);

                    /*
		    ** Remove the active mark.
		    */
		
		    if (entry_number > 0)
			SvnEnvWindowSetCheckMarkComponent(private, entry_number,
			    _EnvSvnActivateComponent, _NotActiveCheckMark);

		    entries[i]->active = _NotActive;

		    /*
		    **  If we are turning off a child of a composite, then
		    **	turn off the composite as well
		    */

		    if ((entries[i]->parent != (_SvnData) 0))
			SetActiveStateOff(private, entries[i]->parent);
		}
	    }
	
	    /*
	    **  Build a list of active objects and update currency if needed.
	    */

	    if (_SetWindowState(private->window, _StateActivePaths, _StateGet)) {
		EnvDWEnvWinUpdateCurrencyCPath(private);
		_SetWindowState(private->window, _StateActivePaths, _StateClear);
	    }

	}

    }

    /*
    **	Update the current object
    */

    switch ((lwk_domain)type) {

	case lwk_c_domain_linknet :
	    ((_EnvWindowPrivate)(private->specific))->current_network =
		    	    current_object;
	    break;

	case lwk_c_domain_path :
	    ((_EnvWindowPrivate)(private->specific))->trail = current_object;
	    break;
    }

    return;
    }


static _Integer  SwitchActiveState(active_state)
_Integer active_state;

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
    if ((active_state == _Active) || (active_state == _PartActive))
	return(_NotActive);
    else
	return(_Active);
    }


static _Boolean  ActiveStateEqual(active_state_1, active_state_2)
_Integer active_state_1;

    _Integer active_state_2;

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
    if (((active_state_1 == _Active) || (active_state_1 == _PartActive))
	&& ((active_state_2 == _Active) || (active_state_2 == _PartActive)))
	return (_True);

    if ((active_state_1 == _NotActive) && (active_state_2 == _NotActive))
	return (_True);

    return (_False);
    }


static _Void  SvnEnvWindowClearRecord(private, entry)
_WindowPrivate private;
 _SvnData entry;

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
    _EnvContext		    env_ctxt;
    _CurrencyFlag	    currency;
    _Integer		    i;
    _Integer		    count;
    _Integer		    entry_number;
    _Integer		    type;
    _SvnData		    *entries;
    lwk_list		    list;
    lwk_object		    his_obj;
    lwk_status		    status;
    lwk_object		    current_object;

    /*
    **	Get the object domain
    */

    _GetValue(entry->object, _P_HisType, hs_c_domain_integer, &type);

    /*
    ** If it's an object descriptor, we need to know its real domain to set the
    ** currency.
    */
    if ((lwk_domain)type == lwk_c_domain_object_desc)
	_GetProperty(entry->object, lwk_c_domain_integer, lwk_c_p_object_domain,
		     &type);

    switch ((lwk_domain)type) {

	case lwk_c_domain_linknet :
	    current_object =
		((_EnvWindowPrivate)(private->specific))->current_network;
	    currency = lwk_c_env_recording_linknet;
	    status = lwk_create_set(lwk_c_domain_object_desc, 1, &list);
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(create_list_failed); /* his set creation failed */
	    }
	    break;

	case lwk_c_domain_path :
	    current_object =
		((_EnvWindowPrivate)(private->specific))->trail;
	    currency = lwk_c_env_recording_path;
	    status = lwk_create_list(lwk_c_domain_object_desc, 1, &list);
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(create_list_failed); /* his list creation failed */
	    }
	    break;

	default :
	    break;
    }

    _GetValue(entry->object, _P_HisObject, hs_c_domain_lwk_object, &his_obj);

    if (his_obj == current_object) {

	/*
	** Get the list of the Svn entries for this object.
	*/

	_MapTableGetEntry(private->map_table, his_obj, &entries, &count);

	/*
	**  Get the environment context object from the window object
	*/

	_GetValue(private->window, _P_EnvironmentContext,
	    hs_c_domain_environment_ctxt, &env_ctxt);

	/*
	**  Turn off recording
	*/
	
	current_object = lwk_c_null_object;

	for (i = 0; i < count; i++) {

	    entry_number = DXmSvnGetEntryNumber(private->svn,
		(XtPointer) entries[i]);

	    if (entry_number > 0)
		SvnEnvWindowSetCheckMarkComponent(private, entry_number,
		    _EnvSvnRecordComponent, _NotActiveCheckMark);
	}

        /*
	** Update the current object.
	*/
	
	switch ((lwk_domain)type) {

	    case lwk_c_domain_linknet :
		/*
		** There is no more current network.
		*/
		((_EnvWindowPrivate)(private->specific))->current_network
			= current_object;
		break;
	
	    case lwk_c_domain_path :
		/*
		** There is no more current trail.
		*/
		((_EnvWindowPrivate)(private->specific))->trail
			= current_object;
		break;
	}
	
	/*
	**  Update the currency
	*/
	
	_SetContextCurrency(env_ctxt, currency, list, (_HsObject) _NullObject);
    }

    return;
    }


_Void  EnvSvnEnvWindowToggleActivate(private, data)
_WindowPrivate private;
 _SelectData data;

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
    lwk_domain	    domain;
    _Integer        type;
    _Integer	    count;
    _Integer	    index = 0;
    _SvnData	    entry;
    _Integer	    old_active_state;


    /*
    **	Note: the first item selected is valid and its active state should be
    **	toggled.  the map control menu routine ensures that the first item in
    **	the selection list is valid.
    */

    entry = data->svn_data[0];
    old_active_state = entry->active;
    count = data->count;

    /*
    **  Run down the list of selected items
    */

    while (count > 0) {
	count--;
	entry = data->svn_data[index];

	/*
	**  Process the entry only if the right component has been selected
	*/
	
	if ((data->component[index] == _EnvSvnIconComponent) ||
	    (data->component[index] == _EnvSvnNameComponent) ||
	    (data->component[index] == _EnvSvnActivateComponent)){

	    /*
	    **  Skip the title or segment headers
	    */

	    if (entry->object != _NullObject) {

		/*
		**  Only process entries which need to be processed
		*/

		if ((entry->retrievable == _Retrievable) &&
		    (ActiveStateEqual(entry->active, old_active_state))) {

		    _GetValue(entry->object, _P_HisType, hs_c_domain_integer,
			&type);
                    domain = (lwk_domain) type;
                    /*
		    ** Mark the state active flags depending of the domain.
		    */
		    if ((domain == lwk_c_domain_linknet) ||
			(domain == lwk_c_domain_comp_linknet))
			_SetWindowState(private->window, _StateActiveNetworks,
					_StateSet);
		    else
			if ((domain == lwk_c_domain_path) ||
			    (domain == lwk_c_domain_comp_path))
			    _SetWindowState(private->window, _StateActivePaths,
					    _StateSet);
		    
		    switch (domain) {
                                          
			case lwk_c_domain_linknet:
			case lwk_c_domain_path:
			    ToggleActivateSimpleObject(private, entry,
				old_active_state);
			    break;

			case lwk_c_domain_comp_linknet:
			case lwk_c_domain_comp_path:
			    if (entry->children != (_SvnData) 0) {
			
				ToggleActivateComposite(private, entry,
				    old_active_state);

				/*
				** if the active state of the composite has not
				** changed, it's because one of its child
				** active state cannot be changed, so change it
				** now.
				*/
				
				if (ActiveStateEqual(entry->active,
						     old_active_state))
				     TogglePartActiveComposite(private, entry,
					domain);
				
			    }
			    break;
		    }
		}
		else
		    if (entry->retrievable != _Retrievable)
			XBell(XtDisplay(private->svn), 0);

	    }
	}
	
	index++;
    }

    return;
    }


static _Void  TogglePartActiveComposite(private, entry, domain)
_WindowPrivate private;
 _SvnData entry;

    lwk_domain domain;

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
    lwk_object	his_obj;
    _SvnData	*entries;
    _Integer	cnt = 0;
    _Integer	i;
    _Integer	active_state;

    /*
    ** Change the active state of the entry depending of its old one.
    */
    if (entry->active == _NotActive)
	active_state = _PartActive;
    else
	active_state = _NotActive;

    switch (domain) {

	case lwk_c_domain_comp_linknet:
	
	    /*
	    ** Get all the instances of the same entry in the Svn display
	    */
	    _GetValue(entry->object, _P_HisObject, hs_c_domain_lwk_object,
		&his_obj);
	    _MapTableGetEntry(private->map_table, his_obj, &entries, &cnt);

	    for (i = 0; i < cnt; i++) {
		entries[i]->active = active_state;
		
		/*
		** Mark it with the Part Active Check Mark
		**	Icon.
		*/
		SvnEnvWindowToggleActive(private, entries[i]);

		/*
		** Mark eventually the active state of its parent
		*/
		if (entries[i]->parent != (_SvnData) 0)
		    SetParentStateOn(private, entries[i]->parent);
	    }
	    break;

	case lwk_c_domain_comp_path:

	    entry->active = active_state;
	
	    /*
	    ** Mark it with the Part Active Check Mark
	    **	Icon.
	    */
	    SvnEnvWindowToggleActive(private, entry);

	    /*
	    ** Mark eventually the active state of its parent
	    */
	    if (entry->parent != (_SvnData) 0)
		SetParentStateOn(private, entry->parent);

	    break;
    }
    	
    return;
    }


static _Void  ToggleActivateSimpleObject(private, entry, old_active_state)
_WindowPrivate private;
 _SvnData entry;

    _Integer old_active_state;

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
    _Integer	type;
    _Integer	cnt;
    _Integer	i;
    lwk_object	his_obj;
    _SvnData	*entries;
	
    if (ActiveStateEqual(entry->active, old_active_state)) {

        /*
	** Get the object domain
	*/
	_GetValue(entry->object, _P_HisType, hs_c_domain_integer, &type);

	switch ((lwk_domain) type) {
	
	    case lwk_c_domain_linknet:
		
                /*
		** Get all the instances of the same entry in the Svn display
		*/
		_GetValue(entry->object, _P_HisObject, hs_c_domain_lwk_object,
		    &his_obj);
		_MapTableGetEntry(private->map_table, his_obj, &entries, &cnt);

		for (i = 0; i < cnt; i++)
		    ToggleActivateEntry(private, entries[i],
			old_active_state);

		break;

	    case lwk_c_domain_path:

		if (entry->active == _NotActive)
		    /*
		    ** The path can be in record state so toggle it off.
		    */
		    SvnEnvWindowClearRecord(private, entry);

		ToggleActivateEntry(private, entry, old_active_state);

		break;

	    default:
		break;
	}	
    }

    return;
    }


static _Void  ToggleActivateEntry(private, entry, old_active_state)
_WindowPrivate private;
 _SvnData entry;

    _Integer old_active_state;

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
    entry->active = SwitchActiveState(old_active_state);

    /*
    ** Toggle the Svn component
    */
    SvnEnvWindowToggleActive(private, entry);

    /*
    ** If we turn off a child of a composite or if you try to activate a non
    ** retrievable entry, turn also the composite off
    */
    if (((entry->active == _NotActive) || (entry->retrievable != _Retrievable))
	&& (entry->parent != (_SvnData) 0))
	SetActiveStateOff(private, entry->parent);

    /*
    ** If we turn on a child of a composite you may turn on its parent
    */
    if ((entry->active == _Active) && (entry->parent != (_SvnData) 0))
	SetParentStateOn(private, entry->parent);

    return;
    }


static _Void  ToggleActivateComposite(private, entry, old_active_state)
_WindowPrivate private;
 _SvnData entry;

    _Integer old_active_state;

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
    _Integer	type;
    _Integer	cnt;
    _Integer	i;
    lwk_object	his_obj;
    _SvnData	*entries;
	
    if ((ActiveStateEqual(entry->active, old_active_state))
	&& (entry->children != (_SvnData) 0)) {

        /*
	** Get the object domain
	*/
	_GetValue(entry->object, _P_HisType, hs_c_domain_integer, &type);

	if ( (_Domain) type == lwk_c_domain_comp_linknet) {
		
	    /*
	    ** Get all the instances of the same entry in the Svn display
	    */
	    _GetValue(entry->object, _P_HisObject, hs_c_domain_lwk_object,
		&his_obj);
	    _MapTableGetEntry(private->map_table, his_obj, &entries, &cnt);

	    for (i = 0; i < cnt; i++)
		ToggleActivateObjectInComp(private, entries[i]->children,
		    old_active_state);

	}

	else
	    ToggleActivateObjectInComp(private, entry->children,
		old_active_state);

    }

    return;
    }



static _Void  ToggleActivateObjectInComp(private, entry, old_active_state)
_WindowPrivate private;
 _SvnData entry;

    _Integer old_active_state;

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
    if (entry->siblings != (_SvnData) 0)
	ToggleActivateObjectInComp(private, entry->siblings, old_active_state);

    ToggleActivateObject(private, entry, old_active_state);

    return;
    }



static _Void  ToggleActivateObject(private, entry, old_active_state)
_WindowPrivate private;
 _SvnData entry;

    _Integer old_active_state;

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
    _Integer	type;

    if ((entry->retrievable == _Retrievable) &&
	(ActiveStateEqual(entry->active, old_active_state))) {

	_GetValue(entry->object, _P_HisType, hs_c_domain_integer,
	    &type);

	switch ((lwk_domain) type) {

	    case lwk_c_domain_linknet:
	    case lwk_c_domain_path:
		ToggleActivateSimpleObject(private, entry, old_active_state);
		break;

	    case lwk_c_domain_comp_linknet:
	    case lwk_c_domain_comp_path:
		if (entry->children != (_SvnData) 0)
		    ToggleActivateComposite(private, entry, old_active_state);

		break;
	}
    }

    return;
    }


static _Void  SetParentStateOn(private, parent)
_WindowPrivate private;
 _SvnData parent;

/*
**++
**  Functional Description:
**	Mark the active state of a composite by testing the active state of its
**	children and propagate eventually to its parent.
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
    _SvnData	entry;
    _SvnData	*entries;
    _Integer	cnt;
    _Integer	type;
    _Integer	i;
    _Boolean	partial_active;
    _Boolean	partial_retrievable;
    lwk_object	his_obj;

    /*
    **  Just return if we are the segment level or the parent has no children
    */

    if ((parent->segment != 0) || (parent->children == (_SvnData) 0))
	return;

    partial_active = _False;
    partial_retrievable = _False;

    entry = parent->children;

    /*
    ** Check the active state of the children
    */

    while (entry != (_SvnData) 0) {

	if (entry->retrievable == _Retrievable)

	    switch (entry->active) {

		case _NotActive :
		    /*
		    **  If one entry is not active stop here
		    */
		    return;

		case _PartActive :
		    partial_active = _True;
		    partial_retrievable = _True;
		    break;

		case _Active :
		    partial_retrievable = _True;
		    break;
	    }

	else
            /*
	    ** if one entry is not retrievable the parent entry could be marked
	    ** partial active
	    */
	    partial_active = _True;

	entry = entry->siblings;
    }

    /*
    ** if all the children are not retrievable the parent entry cannot be marked
    ** active.
    */

    if (partial_retrievable == _False)
	return;
	
    /*
    ** Get the object domain
    */
    _GetValue(parent->object, _P_HisType, hs_c_domain_integer, &type);

    switch ((lwk_domain) type) {

	case lwk_c_domain_comp_path:

	    /*
	    **  Turn active state on
	    */

	    if (partial_active)
		parent->active = _PartActive;
	    else
		parent->active = _Active;

	    /*
	    **  Toggle the Svn component
	    */

	    SvnEnvWindowToggleActive(private, parent);

	    /*
	    **  If the entry is again in a composite, check if we should turn on
	    **  it's parent
	    */

	    if ((parent->parent != (_SvnData) 0))
		SetParentStateOn(private, parent->parent);
	
	    break;

	case lwk_c_domain_comp_linknet:

	    /*	
	    **  Get all the instances of the same entry in the SVN display
	    */

	    _GetValue(parent->object, _P_HisObject, hs_c_domain_lwk_object, &his_obj);

	    _MapTableGetEntry(private->map_table, his_obj, &entries, &cnt);

	    for (i = 0; i < cnt; i++) {

		/*
		**  Turn active state on
		*/

		if (partial_active)
		    entries[i]->active = _PartActive;
		else
		    entries[i]->active = _Active;

		/*
		**  Toggle the Svn component
		*/

		SvnEnvWindowToggleActive(private, entries[i]);

		/*
		**  If the entry is again in a composite, check if we should turn on
		**  it's parent
		*/

		if ((entries[i]->parent != (_SvnData) 0))
		    SetParentStateOn(private, entries[i]->parent);
	    }
    }
	
    return;
    }


static _Void  SetActiveStateOff(private, entry)
_WindowPrivate private;
 _SvnData entry;

/*
**++
**  Functional Description:
**	Mark inactive the active state of a composite and propagate eventually
**	to its parent.
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
    _SvnData	    *entries;
    _Integer	    cnt;
    _Integer	    i;
    _Integer	    type;
    lwk_object	    his_obj;

    /*
    **  If it is the segment entry, just return
    */

    if (entry->segment != 0)
	return;

    /*
    **  If parent is already off and in case we reach the segment header, return
    */

    if ((entry->active == _Active) || (entry->active == _PartActive)) {

	_GetValue(entry->object, _P_HisType, hs_c_domain_integer, &type);

	switch ((lwk_domain) type) {

	    case lwk_c_domain_comp_path:

		/*
		**  Turn active state off
		*/
		entry->active = _NotActive;

		/*
		**  Toggle the Svn component
		*/
		SvnEnvWindowToggleActive(private, entry);
	
		/*
		**  If the entry is again in a composite, turn off its parent
		*/
		if (entry->parent != (_SvnData) 0)
		    SetActiveStateOff(private, entry->parent);

		break;

	    case lwk_c_domain_comp_linknet:
	
		/*	
		**  Get all the instances of the same entry in the SVN
		**	display
		*/
		_GetValue(entry->object, _P_HisObject, hs_c_domain_lwk_object, &his_obj);
		
		_MapTableGetEntry(private->map_table, his_obj, &entries, &cnt);

		for (i = 0; i < cnt; i++) {
		
		    /*
		    **  Turn active state off
		    */
		    entries[i]->active = _NotActive;

		    /*
		    **  Toggle the Svn component
		    */
		    SvnEnvWindowToggleActive(private, entries[i]);
		
		    /*
		    **  If the entry is again in a composite, turn off its parent
		    */
		    if (entries[i]->parent != (_SvnData) 0)
			SetActiveStateOff(private, entries[i]->parent);
		}

		break;

	}
    }

    return;
    }


static _Void  SvnEnvWindowToggleActive(private, entry)
_WindowPrivate private;
 _SvnData entry;

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
    _Integer    entry_number;

    /*
    **  If the entry is displayed, toggle it's Svn checkmark component
    */

    entry_number = DXmSvnGetEntryNumber(private->svn, (XtPointer) entry);

    if (entry_number > 0) {

	switch (entry->active) {
	    case _NotActive:
		SvnEnvWindowSetCheckMarkComponent(private, entry_number,
		    _EnvSvnActivateComponent, _NotActiveCheckMark);
		break;

	    case _Active:
		SvnEnvWindowSetCheckMarkComponent(private, entry_number,
		    _EnvSvnActivateComponent, _ActiveCheckMark);
		break;

	    case _PartActive:
		SvnEnvWindowSetCheckMarkComponent(private, entry_number,
		    _EnvSvnActivateComponent, _PartActiveCheckMark);
		break;
	}
    }

    return;
    }


_Void  EnvSvnEnvWindowRemoveEntry(private, svn_data, timestamp)
_WindowPrivate private;
 _SvnData svn_data;

    Time timestamp;

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
    _SvnData		*begin_list,
			*end_list,
			parent,
			*entries;
    _Integer		count = 0,
			type;
    _HsObject		env_ctxt;
    _EnvWindowPrivate	env_private;
    _Boolean		active = _False;
    lwk_object		his_obj;

    if (svn_data->object != _NullObject) {

	/*
	** Check if the object is retrievable.
	*/
	
	if (svn_data->retrievable == _Retrievable) {
	
	    /*
	    ** Get the object domain.
	    */
	
	    _GetValue(svn_data->object, _P_HisType, hs_c_domain_integer, &type);
	    _GetValue(svn_data->object, _P_HisObject, hs_c_domain_lwk_object,
		&his_obj);

	    /*
	    **  If the entry was set to record and there are no other
	    **	corresponding objects, reset the currency
	    */

	    env_private = (_EnvWindowPrivate) private->specific;

	    _MapTableGetEntry(private->map_table, his_obj, &entries, &count);

	    if ((env_private->current_network == his_obj) ||
		(env_private->trail == his_obj)) {
		
		if (count == 1)
		    SvnEnvWindowToggleRecord(private, svn_data);
	    }
	}
	
	else {
	
	    /*
	    ** Get real domain.
	    */
	
	    _GetProperty(svn_data->object, lwk_c_domain_integer,
		lwk_c_p_object_domain, &type);
	}
	
        /*
	** Look if the entry was active.
	*/
	if (svn_data->active != _NotActive)
	    active = _True; 
	
	/*
	** Get the list pointers.
	*/
	
	parent = svn_data->parent;
	
	SvnEnvWindowGetListPointers(private, type, &begin_list, &end_list);
	
	/*
	** Remove the entry from the map table and all its children if any
	*/
	
	if (svn_data->retrievable == _Retrievable)
	    EnvSvnRemvEntryDataFromTable(svn_data, private->map_table);

	/*
	** Remove the entry and its children if any from the display
	*/
	
	EnvSvnWindowRemoveEntry(private, begin_list, end_list, svn_data, _True);

	/*
	** If the list is empty, remove the segment.
	*/
	
	if (*begin_list == (_SvnData) 0)
	    SvnEnvWindowRemoveSegment(private, parent);
	
	/*
	**  Update the currency depending on the domain of the deleted object.
	*/
	if ((type == (_Integer) lwk_c_domain_path) ||
	    (type == (_Integer) lwk_c_domain_comp_path))
	    EnvDWEnvWinUpdateCurrencyCPath(private);

	else 
	    if ((type == (_Integer) lwk_c_domain_linknet) ||
		(type == (_Integer) lwk_c_domain_comp_linknet))
                /*
		** Update the currency only if it is an active network that
		** appears only once.
		*/
		if ((active) && (count == 1))
		    EnvDWEnvWinUpdateCurrencyCNet(private);
		
	/*
	**  Get the environment context object from the window object
	*/
	
	_GetValue(private->window, _P_EnvironmentContext,
	    hs_c_domain_environment_ctxt, &env_ctxt);
	
	/*
	** Save the context for the given domain
	*/
	
	_SaveList(env_ctxt, (lwk_domain) type, *begin_list);

	/*
	** Relinquish selection ownership.
	*/

	EnvDWQuickCopyDisown(private, timestamp);
    }

    return;
    }


static _Void  SvnEnvWindowRemoveSegment(private, segment)
_WindowPrivate private;
 _SvnData segment;

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
    _SvnData parent;
    _Integer entry_number;


    parent = private->svn_entries;
    /*
    ** Get the entry number.
    */
    entry_number = DXmSvnGetEntryNumber(private->svn, (XtPointer) segment);

    /*
    ** Remove the segment from the structure.
    */
    if (parent->children == segment)
	parent->children = segment->siblings;
    else
	((_SvnData)(parent->children))->siblings = (_SvnData) 0;
	
    /*
    ** Decrement the parent's count of children.
    */
    parent->child_cnt--;
	
    /*
    ** Delete the entry from the display list.
    */

    DXmSvnDeleteEntries(private->svn, entry_number-1, 1);

    /*
    ** free the svn data structure.
    */
    EnvSvnWindowFreeSvnData(segment);

    return;
    }

_Void  EnvSvnEnvWindowInsertEntry(private, object, domain)
_WindowPrivate private;
 lwk_object object;

     lwk_domain domain;

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
    _SelectData	    select_data;
    _SvnData	    selected_entry;

    select_data = (_SelectData) 0;

    /*
    ** Get the selected data.
    */

    if (_GetSelection(private->window, _SingleSelection, &select_data)) {

	selected_entry = select_data->svn_data[0];
	
	/*
	**  If a segment header is selected, ignore the selection
	*/

	if (((_SvnData) (select_data->svn_data[0]))->segment != 0)
	    selected_entry = (_SvnData) 0;	
    }
    else
	selected_entry = (_SvnData) 0;
		
    /*
    ** Insert the entry
    */

    SvnEnvWindowInsertEntry(private, object, domain, selected_entry);

    if (select_data != (_SelectData) 0)
	EnvSvnWindowFreeSelection(select_data);
	
    return;
    }


static _Void  SvnEnvWindowInsertEntry(private, object, domain, selected_entry)
_WindowPrivate private;
 lwk_object object;

     lwk_domain domain;
 _SvnData selected_entry;

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
    _SvnData	    entry;
    _Integer	    type;
    _Integer	    entry_number;
    _Boolean	    inserted = _False;
    lwk_status	    status;
    lwk_object	    hisobject;

    /*
    ** Check if this object already appears at the second level. We
    ** don't allow duplicate entries.
    */

    if (SvnEnvWindowObjectInSegment(private, object))
	_Raise(duplicate_obj);
	
    /*
    ** Check if the selected entry isn't null.
    */

    if (selected_entry != (_SvnData) 0) {

	/*
	** Check if the selection is valid.
	*/
	
	if (selected_entry->object != (_HsObject) _NullObject) {
	
	    /*
	    ** Get the selected his object.
	    */
	
	    _GetValue(selected_entry->object, _P_HisObject,
		    hs_c_domain_lwk_object, &hisobject);
	
	    /*
	    ** Get the selected object domain.
	    */
	
	    _GetValue(selected_entry->object, _P_HisType,
		    hs_c_domain_integer, &type);

	    /*
	    ** Check the selection level.
	    */
	
	    if (selected_entry->child_level == 3) {
	
		switch ((lwk_domain)type) {
		
		    case lwk_c_domain_comp_linknet:
		    case lwk_c_domain_linknet:
			if (domain == lwk_c_domain_comp_linknet ||
			    domain == lwk_c_domain_linknet) {
			    SvnEnvWindowInsertObject(private, selected_entry,
				object, domain, _BeforeInto);
			    inserted = _True;
			}
			break;
			
		    case lwk_c_domain_comp_path:
		    case lwk_c_domain_path:
			if (domain == lwk_c_domain_comp_path ||
			    domain == lwk_c_domain_path) {
			    SvnEnvWindowInsertObject(private, selected_entry,
				object, domain, _BeforeInto);
			    inserted = _True;
			}
			break;
		}
	    }
	 }
	
	 /*
	 ** Should be a segment.
	 */
	
	 else {
	
	    /*
	    ** If selected object is at first level, check segment type
	    */
	
	    if (selected_entry->child_level == 2) {

		    switch (selected_entry->segment) {
		
			case _NetworkSegment:
			    if (domain == lwk_c_domain_comp_linknet ||
				domain == lwk_c_domain_linknet) {
				SvnEnvWindowInsertObject(private, selected_entry,
				    object, domain, _EndOfList);
				inserted = _True;
			    }
			    break;
			
			case _PathSegment:
			    if (domain == lwk_c_domain_comp_path ||
				domain == lwk_c_domain_path) {
				SvnEnvWindowInsertObject(private, selected_entry,
				    object, domain, _EndOfList);
				inserted = _True;
			    }
			    break;
		    }
	    }
	
	}		
    }

    /*
    ** If not already inserted, insert at the end of the corresponding list.
    */

    if (!inserted)
	SvnEnvWindowInsertObject(private, (_SvnData) 0, object, domain,
	    _EndOfList);

    return;
    }


static _Boolean  SvnEnvWindowObjectInSegment(private, object)
_WindowPrivate private;

    lwk_object object;

/*
**++
**  Functional Description:
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
    _Boolean	found = _False;
    _SvnData	*entries;
    _Integer	count,
		i = 0;

    /*
    ** Get the Svn entries from the map table and check if one of them is at the
    ** first level.
    */
    if (_MapTableGetEntry(private->map_table, object, &entries, &count))

	while ((i < count) && !found)
	    if (entries[i]->child_level == 3)
		found = _True;
	    else
		i++;

    return(found);
    }



static _Void  SvnEnvWindowGetListPointers(private, domain, begin_list, end_list)
_WindowPrivate private;

    lwk_domain domain;
 _SvnData **begin_list;
 _SvnData **end_list;

/*
**++
**  Functional Description:
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
    ** Get the corresponding end of list pointer for insertion.
    */

    switch (domain) {

	case lwk_c_domain_comp_linknet:
	case lwk_c_domain_linknet:
	    *begin_list =
		&((_EnvWindowPrivate)(private->specific))->networks_head;
	    *end_list =
		&((_EnvWindowPrivate)(private->specific))->networks_end;
	    break;
	
	case lwk_c_domain_comp_path:
	case lwk_c_domain_path:
	    *begin_list =
		&((_EnvWindowPrivate)(private->specific))->paths_head;
	    *end_list =
		&((_EnvWindowPrivate)(private->specific))->paths_end;
	    break;
	
    }

    return;
    }


static _Void  SvnEnvWindowInsertObject(private, selected_data, object, domain, insert_entry)
_WindowPrivate private;

    _SvnData selected_data;
 lwk_object object;
 lwk_domain domain;
 _InsertEntry
    insert_entry;

/*
**++
**  Functional Description:
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
    _SvnData	    svn_data,
		    previous_entry,
		    parent;
    _HsObject	    env_ctxt;
    _SvnData	    *begin_list,
		    *end_list,
		    *entries;
    _Integer	    entry_number,
		    cnt;
    				    		
    /*
    ** Get the beginning and end of list pointers corresponding to the object
    ** domain.
    */
    SvnEnvWindowGetListPointers(private, domain, &begin_list, &end_list);


    /*
    ** Get the parent entry.
    */
    switch (insert_entry) {
	case _EndOfList:
	    /*
	    ** If the list is empty, build the segment.
	    */
	    if (*begin_list == (_SvnData) 0)
		parent = SvnEnvWindowBuildSegment(private, domain);
	    else
		parent = ((_SvnData)(*end_list))->parent;
		
	    break;

	case _IntoList:
	    parent = selected_data;
	    break;

	case _BeforeInto:
	    parent = selected_data->parent;
	    break;
    }

    /*
    **  Create a new svn_data entry
    */

    svn_data = EnvSvnWindowCreateSvnData(private, parent,
		    parent->child_level + 1);

    /*
    ** Create a LinkWorks Manager object.
    */

    svn_data->object = (_HsObject) _CreateHsObject(_TypeHsObject, domain,
	object);

    svn_data->retrievable = _Retrievable;

    /*
    ** Insert into the segment.
    */
	
    switch (insert_entry) {
	case _EndOfList:
	    /*
	    ** If the list isn't empty, end of list is the previous one.
	    */
	    if (*end_list != (_SvnData) 0) {
		/*
		** Check if the previous entry isn't expanded.
		*/
		entry_number = DXmSvnGetEntryNumber(private->svn,
		    (XtPointer) *end_list);
		if (EnvSvnWindowIsExpandable(private, *end_list,
		    entry_number, _EnvSvnNameComponent) == _CollapseEntry) {
		    previous_entry = ((_SvnData)(*end_list))->children;
		    while (previous_entry->siblings != (_SvnData) 0)
			previous_entry = previous_entry->siblings;
		}
		else
		previous_entry = *end_list;
		/*
		** Insert the svn_data at the end of the list.
		*/
		EnvSvnWindowInsertAfter(end_list, svn_data);
	    }
	    /*
	    ** The list is empty, fix the pointers and set the segment to be previous
	    ** entry.
	    */
	    else {
		*begin_list = svn_data;
		*end_list = svn_data;
		
		/*
		** Insert the entry into the segment.
		*/
		previous_entry = EnvSvnWindowInsertInto(parent, svn_data);
	    }

	    break;
	case _IntoList:
	    previous_entry = EnvSvnWindowInsertInto(selected_data, svn_data);
	    break;
	case _BeforeInto:
	    /*
	    ** Insert in the svn data list
	    */
	
	    previous_entry =
		EnvSvnWindowInsertBeforeInto(selected_data, svn_data);

	    /*
	    ** Check the list pointers.
	    */

	    if (*begin_list == selected_data)
		*begin_list = svn_data;
			
	    break;
    }

    /*
    ** If it is a network or a network list, search if it is already in the
    ** environment window and preserve its active state. Preserve the record
    ** mode state of a network.
    */

    switch (domain) {

	case lwk_c_domain_linknet :
	
		if (((_EnvWindowPrivate) (private->specific))->current_network
		    == object)
		    {
		    entry_number = DXmSvnGetEntryNumber(private->svn,
			(XtPointer) svn_data);
		
		    if (entry_number > 0)
			SvnEnvWindowSetCheckMarkComponent(private, entry_number,
			    _EnvSvnRecordComponent, _ActiveCheckMark);
		    }
			
	case lwk_c_domain_comp_linknet :
	
		_MapTableGetEntry (private->map_table, object, &entries, &cnt);
		
		if ((cnt != 0) && (entries[0]->active == _Active))
		    svn_data->active = _Active;
		break;
    }

    /*
    ** Insert into the map table.
    */

    _MapTableAddEntry(&private->map_table, object, svn_data);
	
    /*
    **  Get the environment context object from the window object
    */

    _GetValue(private->window, _P_EnvironmentContext,
	hs_c_domain_environment_ctxt, &env_ctxt);


    /*
    ** Save the list.
    */

    _SaveList(env_ctxt, domain, *begin_list);

    /*
    ** If it's a composite object, retrieve its children if any. If it is a
    ** composite network, check the active state of its networks.
    */

    switch (domain) {

	case lwk_c_domain_comp_path :
		EnvSvnWindowRetrieveEntry(svn_data, SvnEnvWindowListComposite);
		break;

	case lwk_c_domain_comp_linknet :
		EnvSvnWindowRetrieveEntry(svn_data, SvnEnvWindowListComposite);
		SvnCheckNetStateInNetList(private, svn_data);
		
                /*
		** Check if you have turn active all its children.
		*/
		SetParentStateOn(private, svn_data);
		
		break;
    }

    /*
    ** Display the entry.
    */

    EnvSvnWindowDisplayEntry(private, previous_entry, svn_data,
	SvnEnvWindowSetEntry);
	
    return;
    }


static _Void  SvnCheckNetStateInNetList(private, parent)
_WindowPrivate private;

    _SvnData parent;

/*
**++
**  Functional Description:
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
    _SvnData	svn_data,
		*entries;
    _Integer    cnt,
		type;
    lwk_object  his_obj;

    /*
    ** If the Network List is empty, return.
    */
    if (parent->children == (_SvnData) 0)
	return;

    /*
    ** Check the active state of its components.
    */
    svn_data = parent->children;

    while (svn_data != (_SvnData) 0)
	{
	_GetValue(svn_data->object, _P_HisObject, hs_c_domain_lwk_object,
		  &his_obj);

	_MapTableGetEntry (private->map_table, his_obj, &entries, &cnt);
		
	if ((cnt != 0) && (entries[0]->active == _Active))
	    svn_data->active = _Active;

	/*
	** Get the object domain.
	*/

	_GetValue(svn_data->object, _P_HisType, hs_c_domain_integer, &type);

	/*
	** If the type is a network list, apply recursively this routine.
	*/

	if ((lwk_domain)type == lwk_c_domain_comp_linknet)
	    SvnCheckNetStateInNetList(private, svn_data);    	
	
	svn_data = svn_data->siblings;
	}
	
    return;
    }


static _SvnData  SvnEnvWindowBuildSegment(private, domain)
_WindowPrivate private;

    lwk_domain domain;

/*
**++
**  Functional Description:
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
    _SvnData	    previous_entry,
		    segment_entry,
		    parent,
		    *begin_list,
		    *end_list;
    _Integer	    entry_number;

    parent = private->svn_entries;

    /*
    ** Create an svn_data structure to be a segment.
    */

    segment_entry = EnvSvnWindowCreateSvnData(private, parent,
					     2);

    switch (domain) {
	case lwk_c_domain_comp_linknet:
	case lwk_c_domain_linknet:
	    /*
	    ** Set the segment type.
	    */
	    segment_entry->segment = _NetworkSegment;
	
	    /*
	    ** Set the previous entry to be the parent.
	    */
	    previous_entry = parent;
	
	    /*
	    ** Insert the segment just after the parent.
	    */
	    if (parent->children != (_SvnData) 0)
		segment_entry->siblings = parent->children;
	    parent->children = segment_entry;
	
	    break;
	case lwk_c_domain_comp_path:	
	case lwk_c_domain_path:
	    /*
	    ** Set the segment type.
	    */
	    segment_entry->segment = _PathSegment;

	    /*
	    ** Set the previous entry either to be the parent or the previous
	    ** end of list (if not empty)
	    */
	    SvnEnvWindowGetListPointers(private,
		lwk_c_domain_linknet, &begin_list, &end_list);

	    if (*end_list != (_SvnData) 0) {
		/*
		** Check if the previous entry isn't expanded.
		*/
		entry_number = DXmSvnGetEntryNumber(private->svn,
		    (XtPointer) *end_list);
		if (EnvSvnWindowIsExpandable(private, *end_list,
		    entry_number, _EnvSvnNameComponent) == _CollapseEntry) {
		    previous_entry = ((_SvnData)(*end_list))->children;
		    while (previous_entry->siblings != (_SvnData) 0)
			previous_entry = previous_entry->siblings;
		}
		else
		previous_entry = *end_list;
	    }
	    else
		previous_entry = parent;

	    /*
	    ** Insert the segment in the right place.
	    */
	    EnvSvnWindowInsertInto(parent, segment_entry);
	
	    break;
    }

    /*
    ** Add and display the entry if the insertion point is visible.
    */

    EnvSvnWindowDisplayEntry(private, previous_entry, segment_entry,
			    SvnEnvWindowSetEntry);
			
    /*
    ** Set previous entry to be the segment.
    */
    previous_entry = segment_entry;


    return(previous_entry);
    }



_Void  EnvSvnEnvWindowUpdate(private, hs_object, update)
_WindowPrivate private;
 _HsObject hs_object;

    _UpdateOperation update;

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
    _Status	    stat[2];
    lwk_persistent  his_obj;
    lwk_termination return_value;
    lwk_domain	    domain;
    _Integer        type;
    lwk_status	    status;
    _String	    linkbase_id;
    _CString	    linkbase_name;
    lwk_ddif_string ddif_str;

    DXmSvnDisableDisplay(private->svn);

    _SetCursor(private->window, _WaitCursor);

    _StartExceptionBlock

    _GetValue(hs_object, _P_HisObject, hs_c_domain_lwk_object, &his_obj);

    _GetValue(hs_object, _P_HisType, hs_c_domain_integer, &type);

    domain = (lwk_domain) type;

    if (domain == lwk_c_domain_linkbase) {

	status = lwk_iterate(his_obj, lwk_c_domain_comp_linknet,
	    (lwk_closure) &private, UpdatePersistent, &return_value);

	if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(iterate_failed);
	}

	status = lwk_iterate(his_obj, lwk_c_domain_linknet,
	    (lwk_closure) &private, UpdatePersistent, &return_value);

	if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(iterate_failed);
	}

	status = lwk_iterate(his_obj, lwk_c_domain_comp_path,
	    (lwk_closure) &private, UpdatePersistent, &return_value);

	if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(iterate_failed);
	}

	status = lwk_iterate(his_obj, lwk_c_domain_path,
	    (lwk_closure) &private, UpdatePersistent, &return_value);

	if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(iterate_failed);
	}
	
        /*
	** Update eventually the non retrievable entries in the Svn display
	*/
	status = lwk_get_value(his_obj, lwk_c_p_identifier, lwk_c_domain_string,
		    &linkbase_id);

	if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(get_value_failed);
	}

	status = lwk_get_value(his_obj, lwk_c_p_name, lwk_c_domain_ddif_string,
		    &ddif_str);

	if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(get_value_failed);
	}

	linkbase_name = _DDIFStringToCString(ddif_str);
	
	UpdateNonRetrievableEntries(private, private->svn_entries,
	    linkbase_id, linkbase_name);
	
    }
    else {

	/*
	**  Update the entry in the Svn display and in the linkbase
	*/

	if (UpdateEntryInDisplay(private, his_obj, update))
	    _Store(hs_object);
    }

    _Exceptions
        _WhenOthers

	    stat[0] = _StatusCode(update_err);
	    stat[1] = _Others;
	    _DisplayMessage(private->window, stat, 2);
	
    _EndExceptionBlock

    DXmSvnEnableDisplay(private->svn);

    _SetCursor(private->window, _DefaultCursor);

    return;
    }


static _Void  UpdateNonRetrievableEntries(private, entry, linkbase_id, linkbase_name)
_WindowPrivate private;
 _SvnData entry;

    _String linkbase_id;
 _CString linkbase_name;


    {
    _EnvContext	env_ctxt;
    _CString	cs_name;
    _CString	cs_linkbase;
    _String	identifier;
    int		entry_number;
    _SvnData	*begin_list,
		*end_list;
    _Domain	domain;
    _Integer    tmp_int;

    if (entry->children != (_SvnData) 0)
	UpdateNonRetrievableEntries(private, entry->children, linkbase_id,
	    linkbase_name);

    if (entry->siblings != (_SvnData) 0)
	UpdateNonRetrievableEntries(private, entry->siblings, linkbase_id,
	    linkbase_name);
	
    if (entry->retrievable == _NotRetrievable) {
	
	/*
	** If it's an HsObject.
	*/

	if (entry->object != _NullObject) {

	    _GetLinkbase(entry->object, &cs_name, &identifier);

	    if (_CompareString(linkbase_id, identifier) == 0) {

		/*
		** Get the entry number. Update the SVN line only if it's
		** visible.
		*/

		entry_number = DXmSvnGetEntryNumber(private->svn,
		    (XtPointer) entry);

		if (entry_number != 0) {
		    /*
		    ** Invalidate the entry so that it gets redrawn.
		    */
		    DXmSvnInvalidateEntry(private->svn, entry_number);

		    /*
		    ** Set the entry.
		    */
		    DXmSvnSetComponentText(private->svn, entry_number,
			_EnvSvnLinkbaseComponent, 100, 0,
			(XmString) linkbase_name, NULL);
		}
		
		/*
		** Set the linkbase name.
		*/
	
		_SetCSProperty(entry->object, lwk_c_p_linkbase_name,
		    linkbase_name, lwk_c_set_property);
				
		/*
		** It's not a real object, only a descriptor.
		** But we need to know its real domain to save this change.
		*/
	
		_GetProperty(entry->object, lwk_c_domain_integer,
		    lwk_c_p_object_domain, &tmp_int);

                domain = (_Domain) tmp_int;

		/*
		** Get the beginning and end of list pointers corresponding to the object
		** domain.
		*/
		SvnEnvWindowGetListPointers(private, domain, &begin_list, &end_list);

		/*
		**  Get the environment context object from the window object
		*/

		_GetValue(private->window, _P_EnvironmentContext,
		    hs_c_domain_environment_ctxt, &env_ctxt);

		/*
		** Save the list.
		*/

		_SaveList(env_ctxt, domain, *begin_list);

	    }
	}	
    }

    return;
    }


static lwk_termination  UpdatePersistent(private, linkbase, domain, persistent)
_WindowPrivate *private;

    lwk_linkbase linkbase;
 lwk_domain domain;
 lwk_persistent *persistent;

/*
**++
**  Functional Description:
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

    UpdateEntryInDisplay(*private, *persistent, _ModifiedProp);

    return 0;
    }


static _Boolean  UpdateEntryInDisplay(private, his_obj, update)
_WindowPrivate private;

    lwk_persistent his_obj;
 _UpdateOperation update;

/*
**++
**  Functional Description:
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
    _SvnData	    *entries;
    _Integer	    count;
    _Integer	    i;
    _Integer	    entry_num;
    _Boolean	    updated;
    _Integer	    active,
		    type;

    updated = _False;

    /*
    **  Get all the entries for this object if it still exists
    */

    if (_MapTableGetEntry(private->map_table, his_obj, &entries, &count)) {

	updated = _True;

	if (update == _DeletedObject)
	    EnvSvnWindowSetEntriesNonRetr(private, entries, count);

	/*
	**  Process each of them.
	**  If the object has been deleted, check if it was in active or record
	**  on mode, and reset it.
	**  Redisplay the entry.
	*/
	active = _NotActive;
	
	for (i = 0; i < count; i++) {

	    /*
	    ** If deleted, check if this object was active.
	    */
	    if (update == _DeletedObject) {
		if (entries[i]->active == _Active)
		    {
		    active = _Active;
		    entries[i]->active = _NotActive;

		    /*
		    ** Get the object type.
		    */
		    _GetValue(entries[i]->object, _P_HisType, hs_c_domain_integer,
			&type);

		    /*
		    ** Set the active state flag of the corresponding domain.
		    */
		    if ((type == (_Integer) lwk_c_domain_linknet) ||
			(type == (_Integer) lwk_c_domain_comp_linknet))
			_SetWindowState(private->window, _StateActiveNetworks,
			    _StateSet);
		    else
			if ((type == (_Integer) lwk_c_domain_path) ||
			    (type == (_Integer) lwk_c_domain_comp_path))
			    _SetWindowState(private->window, _StateActivePaths,
				_StateSet);
		    
		    SvnEnvWindowToggleActive(private, entries[i]);

		    /*
		    **  If we are turning off a child of a composite, then turn
		    **	off the composite as well
		    */
		    if ((entries[i]->parent != (_SvnData) 0))
			SetActiveStateOff(private, entries[i]->parent);
		    }

		/*
		** Check if this object was record.
		*/
		SvnEnvWindowClearRecord(private, entries[i]);
	    }

	    else {

		/*
		** When a composite object has been modified, we need to
		** do a *real* update operation
		*/
		if (update == _ModifiedObject) {

		    /*
		    ** Get the object type.
		    */
		    _GetValue(entries[i]->object, _P_HisType, hs_c_domain_integer,
			&type);

		    /*
		    ** Set the active state flag of the corresponding domain.
		    */
		    if ((type == (_Integer) lwk_c_domain_comp_linknet) ||
			(type == (_Integer) lwk_c_domain_comp_path))
			SvnEnvWindowUpdateEntry(private, entries[i]);
		}
	    }
	    
	    /*
	    **  Update the Svn entry only if it is visible
	    */

	    entry_num = (_Integer) DXmSvnGetEntryNumber(private->svn,
		(XtPointer) entries[i]);

	    if (entry_num > 0)
		SvnEnvWindowSetEntry(private, entries[i]);
	}

        /*
	** if the object was active, update the currency depending on the object
	** type.
	*/
	if (active == _Active) {
	    if (_SetWindowState(private->window, _StateActiveNetworks,
		    _StateGet)) {
		EnvDWEnvWinUpdateCurrencyCNet(private);
		_SetWindowState(private->window, _StateActiveNetworks,
		    _StateClear);
	    }

	    if (_SetWindowState(private->window, _StateActivePaths,
		    _StateGet)) {
		EnvDWEnvWinUpdateCurrencyCPath(private);
		_SetWindowState(private->window, _StateActivePaths,
		    _StateClear);
	    }
	}
    }

    return (updated);
    }


static _Void  SvnEnvWindowInsertQuickCopy(private, object, domain, x, y)
_WindowPrivate private;
 lwk_object object;

     lwk_domain domain;
 _Integer x;
 _Integer y;

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
    _Integer	entry_number;
    _Integer	component;
    _SvnData	selected_entry;

    selected_entry = (_SvnData) 0;

    /*
    ** Get entry under the given location.
    */
    DXmSvnMapPosition(private->svn, x, y, (int *) &entry_number,
	(int *) &component, (XtPointer *) &selected_entry);

    /*
    ** Insert the entry and its children in the right place.
    */
    SvnEnvWindowInsertEntry(private, object, domain, selected_entry);

    return;
    }

static _Void  SvnEnvWindowQuickCopyOrMove(widget, private, x, y, timestamp, move)
_Widget widget;
 _WindowPrivate private;

    _Integer x;
 _Integer y;
 Time timestamp;
 _Boolean move;

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
    lwk_object	    object;
    lwk_object	    hisobject;
    lwk_domain      domain;
    lwk_status	    status;
    Arg		    arglist[2];

    _SetCursor(private->window, _WaitCursor);

    DXmSvnDisableDisplay(private->svn);

    _StartExceptionBlock

    /*
    ** Get the current selected object from the selection owner.
    */
    if (EnvDWQuickCopyGetSelection(private->svn, timestamp, &object, move)) {

	/*
	** Retrieve the persistent object.
	*/
	
	status = lwk_retrieve(object, &hisobject);

	if (status == lwk_s_success) {
	    /*
	    ** Get the object domain.
	    */
	    status = lwk_get_domain(hisobject, &domain);
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(get_domain_failed); /* Get domain failed on object */
	    }

	    /*
	    ** Insert the object at the right location.
	    */
	    SvnEnvWindowInsertQuickCopy(private, hisobject, domain, x, y);
	}
    }

    else
	XBell(XtDisplay(private->svn), 0);
	
    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
        _WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(paste_failed);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	    	
    _EndExceptionBlock

    DXmSvnEnableDisplay(private->svn);

    _SetCursor(private->window, _DefaultCursor);

    return;
    }

_Void  EnvSvnEnvWindowAddActions(private)
_WindowPrivate private;

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


_Void  EnvSvnEnvWindowUpdateEntry(private, svn_data)
_WindowPrivate private;
 _SvnData svn_data;

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
    ** If the object was not retrievable, try to retrieve it, else update the
    ** retrievable object.
    */

    if (svn_data->retrievable != _Retrievable)
	SvnEnvWindowRetrieveEntry(private, svn_data);
    else
	SvnEnvWindowUpdateEntry(private, svn_data);

    /*
    ** Reset the entry.
    */
    SvnEnvWindowSetEntry(private, svn_data);

    return;
    }



static _Void  SvnEnvWindowRetrieveEntry(private, svn_data)
_WindowPrivate private;

    _SvnData svn_data;

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
    lwk_object	object;
    lwk_object	persistent_object;
    lwk_status	status;
    lwk_domain	persistent_domain;
    _Status	stat[1];
    _Integer	entry_number;


    /*
    ** Retrieve the persistent object.
    */

    _GetValue(svn_data->object, _P_HisObject, hs_c_domain_lwk_object, &object);

    status = lwk_retrieve(object, &persistent_object);

    /*
    ** If the object is retrievable, update the corresponding entry.
    */

    if (status != lwk_s_success) {

	stat[0] = _StatusCode(object_retrieve_failed);
	_DisplayMessage(private->window, stat, 1);	
    }

    else {

	/*
	** Get the persistent object's domain.
	*/
	
	status = lwk_get_domain(persistent_object, &persistent_domain);
	
	if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(get_domain_failed);
	}
	
	/*
	** Create a LinkWorks Manager object.
	*/
	
	svn_data->object = (_HsObject) _CreateHsObject(_TypeHsObject,
	    persistent_domain, persistent_object);

	svn_data->retrievable = _Retrievable;
	
	/*
	** Add the entry to the map table.
	*/
	
	_MapTableAddEntry(&private->map_table, persistent_object, svn_data);

	/*
	** If it's a composite object, retrieve its children if any. 
	*/

	switch (persistent_domain) {

	    case lwk_c_domain_comp_linknet :
	    case lwk_c_domain_comp_path :
		    EnvSvnWindowRetrieveEntry(svn_data, SvnEnvWindowListComposite);

                    /*
		    ** If it is a composite linknet, check the active state of
		    ** its children.
		    */
		    
		    if (persistent_domain == lwk_c_domain_comp_linknet) {

			SvnCheckNetStateInNetList(private, svn_data);
			
			/*
			** Check if you have turn active all its children.
			*/
			SetParentStateOn(private, svn_data);
		    }		    

	    break;
    
	}
    }	

    return;
    }



static _Void  SvnEnvWindowUpdateEntry(private, svn_data)
_WindowPrivate private;

    _SvnData svn_data;

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

    _Status	    stat[1];
    _Boolean	    expanded;
    _Integer	    entry_number;
    _SvnData	    children;  
    _SvnData	    *entries;
    _EnvContext	    env_ctxt;
    _CurrencyFlag   currency;
    _Integer	    count;
    lwk_object	    object;
    lwk_object	    persistent_object;
    lwk_status	    status;
    lwk_domain	    persistent_domain;
    lwk_list	    list;
    lwk_object	    current_object;

    /*
    ** Retrieve the persistent object.
    */

    _GetValue(svn_data->object, _P_HisObject, hs_c_domain_lwk_object, &object);

    /*
    ** It's not an object descriptor, get one to perform the retrieve.
    */
    
    status = lwk_get_object_descriptor(object, &object);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(get_objdsc_failed);
    }
    
    status = lwk_retrieve(object, &persistent_object);

    /*
    ** If the object is retrievable, update the corresponding entry.
    */

    if (status != lwk_s_success) {

	stat[0] = _StatusCode(object_retrieve_failed);
	_DisplayMessage(private->window, stat, 1);	
    }

    else {

	/*
	** Get the persistent object's domain.
	*/
	
	status = lwk_get_domain(persistent_object, &persistent_domain);
	
	if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(get_domain_failed);
	}


        /*
	** Delete the old LinkWorks Manager object.
	*/
	_Delete(&(svn_data->object));
        
	/*
	** Create a LinkWorks Manager object.
	*/
	
	svn_data->object = (_HsObject) _CreateHsObject(_TypeHsObject,
	    persistent_domain, persistent_object);

	svn_data->retrievable = _Retrievable;
	
	/*
	** If it's a composite object, we need to retrieve its children if any.
	** The composite object has been modified, so the list of its children
	** can have changed. This influences the active and record objects.
	**
	** Check if one object is still in record mode.
	** 
	** If it is a composite linknet, check the active state of its networks.
	** For a composite path, it needs more work to do it...
	*/

	switch (persistent_domain) {

	    case lwk_c_domain_comp_path :
	    case lwk_c_domain_comp_linknet :

		    expanded = _False;
		    
                    /*
		    ** If this entry is expanded, collapse it, but remember it
		    ** to expand it again.
		    */
		    entry_number = DXmSvnGetEntryNumber(private->svn,
			(XtPointer) svn_data);

		    if (EnvSvnWindowIsExpandable(private, svn_data,
			entry_number, _SvnNameComponent) == _CollapseEntry) {

			_Collapse(private->window, svn_data);
			expanded = _True;
		    }

                    /*
		    ** If this entry has children, remember them so that we can
		    ** free the Svn entries for these children.
		    */

		    children = (_SvnData) 0;
		    
		    if (svn_data->children != (_SvnData) 0)
			children = svn_data->children;

		    EnvSvnWindowRetrieveEntry(svn_data, SvnEnvWindowListComposite);

                    /*
		    ** The active state of this composite object could have
		    ** changed, so we need to check the state of its children.
		    */
		    svn_data->active = _NotActive;

                    /*
		    ** Check the active state of the composite linknet children.
		    */
		    if (persistent_domain == lwk_c_domain_comp_linknet) {

			SvnCheckNetStateInNetList(private, svn_data);
			
			/*
			** Check if we have turn active all its children, so
			** that we can set active the parent.
			*/
			SetParentStateOn(private, svn_data);
		    }		    

                    /*
		    ** Now that we are done with the old children, remove their
		    ** entry from the map table and free them.
		    */
		    if (children != (_SvnData) 0) {
			RemvEntrySiblingsDataFromTable(children,
						       private->map_table);
			EnvSvnWindowFreeSvnSiblData(children);
		    }
		    
                    /*
		    ** If the entry was expanded, expand it again.
		    */
		    if (expanded)
			_Expand(private->window, svn_data);

		    /*
		    ** If we had a record object, check if we still have it,
		    ** else update the currency.
		    */

		    switch (persistent_domain) {

			case lwk_c_domain_comp_linknet :
		    	    current_object =
				 ((_EnvWindowPrivate)(private->specific))->current_network;
			    break;
			    
			case lwk_c_domain_comp_path :
		    	    current_object =
				((_EnvWindowPrivate)(private->specific))->trail;
			    break;

		    }

		    if (current_object != lwk_c_null_object) {
			/*
			**  Get the list of the Svn entries for the current object.
			*/
			_MapTableGetEntry(private->map_table, current_object,
			    &entries, &count);

			/*
			** If we found some entries, there is still a current
			** object, so we don't have anything to do.
			** Else, there is no more current object, so we need to
			** update the currency with a null object.
			*/
			if (count == 0) {

			    current_object = lwk_c_null_object;

			    switch (persistent_domain) {

				case lwk_c_domain_comp_linknet :
				    currency = lwk_c_env_recording_linknet;
				    status = lwk_create_set(
					    lwk_c_domain_object_desc, 1, &list);
				    if (status != lwk_s_success) {
					_SaveHisStatus(status);
					_Raise(create_list_failed);
				    }
				    break;

				    
				case lwk_c_domain_comp_path :
				    currency = lwk_c_env_recording_path;
				    status = lwk_create_list(
					    lwk_c_domain_object_desc, 1, &list);
				    if (status != lwk_s_success) {
					_SaveHisStatus(status);
					_Raise(create_list_failed);
				    }
				    break;
			    }

			    /*
			    **  Get the environment context object from the
			    **	window object.
			    */
			    _GetValue(private->window, _P_EnvironmentContext,
				hs_c_domain_environment_ctxt, &env_ctxt);

			    /*
			    **  Update the currency
			    */
			    _SetContextCurrency(env_ctxt, currency, list,
				(_HsObject) _NullObject);

			    /*
			    **	Update the current object
			    */

			    switch (persistent_domain) {

				case lwk_c_domain_comp_linknet :
				    ((_EnvWindowPrivate)(private->specific))->current_network
					= current_object;
				    break;

				case lwk_c_domain_path :
				    ((_EnvWindowPrivate)(private->specific))->trail
					 = current_object;
				    break;
			    }
			}
		    }
		    
                    /*
		    ** Update the currency depending on the domain.
		    */
		    if (persistent_domain == lwk_c_domain_comp_linknet) 
			EnvDWEnvWinUpdateCurrencyCNet(private);

		    if (persistent_domain == lwk_c_domain_comp_path)
			EnvDWEnvWinUpdateCurrencyCPath(private);
					    
		    
	    break;
    
	}

    }
    return;
    }




_Void  EnvSvnEnvWindowEntryTransfer(w, private, cb)
Widget w;
 _WindowPrivate private;

    DXmSvnCallbackStruct *cb;

/*
**++
**  Functional Description:
**
**	Called on an MB2 click with no motion, modifiers are encoded
**	in the transfer_mode.  Passes a transfer mode.
**
**  Keywords:
**
**	Transfer, Copy, Move, Quick Copy, Cut and Paste
**
**  Arguments:
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
{

    switch (cb->transfer_mode) {

	/* Primary Copy - aka CopyTo    */
	case DXmSvnKtransferUnknown:
    	case DXmSvnKtransferCopy:
	    {
	    SvnEnvWindowQuickCopyOrMove(w, private, cb->x, cb->y,
		cb->time, _False);
	    break;
	    }

	/* Primary Paste - aka MoveTo   */
    	case DXmSvnKtransferMove: {
	    SvnEnvWindowQuickCopyOrMove(w, private, cb->x, cb->y,
		cb->time, _True);
	    break;
	}

	/* Neither Primary Copy or Primary Paste */
    	default: {
	    break;
	}
    }
}


_Void  EnvSvnEnvWindowSelectionsDrag(w, private, cb)
Widget w;
 _WindowPrivate private;

    DXmSvnCallbackStruct *cb;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**
**	Valid SVN arguments in cb:
**	    reason
**	    entry_number
**	    component_number
**	    x
**	    y
**	    entry_tag
**	    entry_level
**	    
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
{
    switch (cb->transfer_mode) {

	case DXmSvnKtransferUnknown: {
	    break;
	}
    	case DXmSvnKtransferCopy: {
	    break;
	}
    	case DXmSvnKtransferMove: {
	    break;
	}
    	default:
	    break;
    }
}

