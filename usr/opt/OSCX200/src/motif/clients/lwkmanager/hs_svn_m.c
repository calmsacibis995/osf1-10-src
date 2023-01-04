/*
** COPYRIGHT (c) 1988, 1991 BY
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
**	SVN driver for LinkWorks Manager
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	Patricia Avigdor
**
**  Creation Date: 12-Dec-89
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

#ifdef VMS
#include <decw$include/Xatom.h>
#else
#include <X11/Xatom.h>
#endif

/*
**  Macro Definitions
*/

#define _SVN_FONT "svn_font"
#define _SVN_SMALL_FONT "svn_small_font"

/*
**  Static Data Definitions
*/
Pixmap	HsCompositeNetIcon  = (Pixmap) 0;
Pixmap	HsNetworkIcon	    = (Pixmap) 0;
Pixmap	HsCompositePathIcon = (Pixmap) 0;
Pixmap	HsPathIcon	    = (Pixmap) 0;


/*
**  Forward routine declarations
*/

_DeclareFunction(static _SvnData FindObjectTag,
    (_SvnData entry, _HsObject hs_object));
_DeclareFunction(static _SvnData SvnWindowGetPreviousEntryTag,
    (_SvnData entry));
_DeclareFunction(static _SvnData SvnWindowGetPreviousSiblingTag,
    (_SvnData entry));


_Void  EnvSvnWindow()
/*
**++
**  Functional Description:
**	{@description}
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


_Void  EnvSvnWindowLoadFont(private, hierarchy)
_WindowPrivate private;
 MrmHierarchy hierarchy;

/*
**++
**  Functional Description:
**	{@description}
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
    _FontList	flist;
    int		ac = 0;
    Arg		arglist[1];
    MrmCode     data_type;
    int		status;
        
    status = MrmFetchLiteral(hierarchy, _SVN_FONT,
	XtDisplay(private->main_widget), (caddr_t *) &flist, &data_type);

    /*									    
    ** Set the font if it is valid, otherwise take the default SVN font
    */
    
    if (status == MrmSUCCESS) {
	ac = 0;
	XtSetArg(arglist[ac], DXmSvnNfontList, flist); ac++;
	XtSetValues(private->svn, arglist, ac);
    };


    status = MrmFetchLiteral(hierarchy, _SVN_SMALL_FONT,
	XtDisplay(private->main_widget), (caddr_t *) &(private->small_font),
	&data_type);

    /*									    
    ** Set the font if it is valid, otherwise indicate no font was loaded
    */
    
    if (status != MrmSUCCESS)
	private->small_font = 0;

    return;
    }


_SvnData  EnvSvnWindowCreateSvnData(private, parent, child_level)
_WindowPrivate private;
 _SvnData parent;

    _Integer child_level;

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
    _SvnData svn_data;

    /*
    ** Allocate an svn_data structure.
    */

    svn_data = (_SvnData) _AllocateMem(sizeof(_SvnDataInstance));
    _ClearMem(svn_data, sizeof(_SvnDataInstance));
    
    svn_data->parent = parent;
    svn_data->child_level = child_level;
    svn_data->active = _NotActive;
    svn_data->retrievable = _NotRetrievable;
    svn_data->private = (_AnyPtr) private;

    return svn_data;
    }


_Void  EnvSvnWindowFreeSvnData(svn_data)
_SvnData svn_data;

/*
**++
**  Functional Description:
**	Close and free a node space (including children)
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

    if (svn_data != NULL) {

	/*
	**  delete this node's children.
	*/
	
	if (svn_data->children != (_SvnData) 0) 
	    EnvSvnWindowFreeSvnSiblData(svn_data->children);
	    
	/*
	**  clean this node's object
	*/
	
        if (svn_data->object != _NullObject) {

	    /*
	    ** Get and delete the his object.
	    */
	
	    _GetValue(svn_data->object, _P_HisObject, hs_c_domain_lwk_object,
		&object);

	    /*
	    **	Delete the hsobject only
	    */
	
	    _Delete(&(svn_data->object));
	}
	
	_FreeMem(svn_data);
    }

    return;
    }


_Void  EnvSvnWindowFreeSvnSiblData(svn_data)
_SvnData svn_data;

/*
**++
**  Functional Description:
**	Close and free a node space (including children), and propagate
**	to siblings 
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

    if (svn_data != NULL) {

	/*
	**  delete this node's siblings
	*/
	
	if (svn_data->siblings != (_SvnData) 0) 
	    EnvSvnWindowFreeSvnSiblData(svn_data->siblings);
	    
	/*
	**  delete this node's children.
	*/
	
	if (svn_data->children != (_SvnData) 0) 
	    EnvSvnWindowFreeSvnSiblData(svn_data->children);
	    
	/*
	**  clean this node's object
	*/
	
        if (svn_data->object != _NullObject) {
	
	    /*
	    ** Get and delete the his object.
	    */
	
	    _GetValue(svn_data->object, _P_HisObject, hs_c_domain_lwk_object,
		&object);

	    /*
	    **	Delete the hsobject only
	    */
	
	    _Delete(&(svn_data->object));
	}
	
	_FreeMem(svn_data);
    }

    return;
    }
    

_Void  EnvSvnWindowInsertEndOfList(head_svn_data, svn_data, list)
_SvnData *head_svn_data;

    _SvnData svn_data;
 _Boolean list;

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
    _SvnData tmp_svn_data;

    /*
    ** If it's not a list of svn data to insert, then set the siblings to zero.
    */

    if (!list)
	svn_data->siblings = (_SvnData) 0;

    /*
    ** List is empty.
    */

    if (*head_svn_data == (_SvnData) 0)
	*head_svn_data = svn_data;
	
    /*
    ** else loop till the end of the list.
    */

    else {

	tmp_svn_data = *head_svn_data;
	while (tmp_svn_data->siblings != (_SvnData) 0)
	    tmp_svn_data = tmp_svn_data->siblings;
	tmp_svn_data->siblings = svn_data;
    }	

    return;
    }


_Void  EnvSvnWindowInsertAfter(end_list, svn_data)
_SvnData *end_list;
 _SvnData svn_data;

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
    ** Insert the svn_data at the end of the list.
    */
    if (*end_list != (_SvnData) 0) {
	svn_data->siblings = (*end_list)->siblings;
	(*end_list)->siblings = svn_data;
    }
    /*
    ** Set the end list pointer.
    */

    *end_list = svn_data;

    /*
    ** Increment the parent's number of children.
    */

    svn_data->parent->child_cnt++;

    return;
    }


_SvnData  EnvSvnWindowInsertBeforeInto(ref_svn_data, svn_data)
_SvnData ref_svn_data;

    _SvnData svn_data;

/*
**++
**  Functional Description:
**	Inserts before a given entry into an expandable entry.
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
    _SvnData previous_entry,
	     same_level_prev_entry,
	     parent;

    parent = svn_data->parent;

    /*
    ** Get the previous entry in the display list.
    */

    previous_entry = SvnWindowGetPreviousEntryTag(ref_svn_data);

    /*
    ** If the reference is at the head of the list of children, change
    ** the children pointer and set previous entry to the parent
    */

    if (parent->children == ref_svn_data) {
	parent->children = svn_data;
	previous_entry = svn_data->parent;
    }

    /*
    ** Else set the same level previous entry siblings.
    */

    else {
	same_level_prev_entry = previous_entry;
	
	while (same_level_prev_entry->child_level > svn_data->child_level)
	    same_level_prev_entry = same_level_prev_entry->parent;
	    
	same_level_prev_entry->siblings = svn_data;
    }

    /*
    ** Set the svn data siblings.
    */

    svn_data->siblings = ref_svn_data;

    /*
    ** Increment the parent's child count.
    */

    parent->child_cnt++;

    return(previous_entry);
    }


_SvnData  EnvSvnWindowInsertBefore(head_svn_data, ref_svn_data, svn_data)
_SvnData *head_svn_data;

    _SvnData ref_svn_data;
 _SvnData svn_data;

/*
**++
**  Functional Description:
**	Inserts before a given entry at level 1.
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
    _SvnData previous_entry,
	     same_level_prev_entry,
	     parent;

    parent = svn_data->parent;

    /*
    ** Get the previous entry in the display list.
    */

    previous_entry = SvnWindowGetPreviousEntryTag(ref_svn_data);
    	
    /*
    ** If the reference is at the head of the given list, change the pointer.
    */

    if (*head_svn_data == ref_svn_data)
	*head_svn_data = svn_data;
	
    /*
    ** If the previous entry is the parent, change the children pointer of it,
    ** else change the previous entry siblings.
    */

    if (parent == previous_entry)
	parent->children = svn_data;

    /*
    ** Else set the same level previous entry siblings.
    */

    else {
	same_level_prev_entry = previous_entry;
	
	while (same_level_prev_entry->child_level > svn_data->child_level)
	    same_level_prev_entry = same_level_prev_entry->parent;
	    
	same_level_prev_entry->siblings = svn_data;
    }
	
    /*
    ** Set the svn data siblings.
    */

    svn_data->siblings = ref_svn_data;

    /*
    ** Increment parent child count.
    */

    parent->child_cnt++;

    return(previous_entry);
    }


static _SvnData  SvnWindowGetPreviousSiblingTag(entry)
_SvnData entry;

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
    _SvnData	previous_entry;
    _SvnData	parent;

    parent = entry->parent;

    if (parent->children == entry)
	previous_entry = parent;
    else {
	previous_entry = parent->children;
	while (previous_entry->siblings != entry &&
	       previous_entry->siblings != (_SvnData)0)
	       previous_entry = previous_entry->siblings;
	if (previous_entry->siblings == (_SvnData) 0)
	    _Raise(svn_entry_not_found);/*entry  missing in the display list*/
    }
    	    	
    return(previous_entry);
    }

static _SvnData  SvnWindowGetPreviousEntryTag(entry)
_SvnData entry;

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
    _SvnData	previous_entry;
    _SvnData	parent;
    _Integer	entry_num;

    parent = entry->parent;

    if (parent->children == entry)
	previous_entry = parent;
    else {

	entry_num = DXmSvnGetEntryNumber(
	    ((_WindowPrivate)(entry->private))->svn, (XtPointer) entry);
	
	if (entry_num != 0)
	    previous_entry = (_SvnData) DXmSvnGetEntryTag(
		((_WindowPrivate)(entry->private))->svn, entry_num - 1);
    }
    	    	
    return(previous_entry);
    }


_SvnData  EnvSvnWindowInsertInto(ref_svn_data, svn_data)
_SvnData ref_svn_data;
 _SvnData svn_data;

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
    _SvnData tmp_svn_data;

    svn_data->siblings = (_SvnData) 0;

    /*
    ** If list of children empty, hook the entry at the beginning.
    */

    if (ref_svn_data->children == (_SvnData) 0){

	ref_svn_data->children = svn_data;
	tmp_svn_data = ref_svn_data;
    }

    /*
    ** else add at the end of the list.
    */

    else {

	tmp_svn_data = ref_svn_data->children;
	while (tmp_svn_data->siblings != (_SvnData) 0)
	    tmp_svn_data = tmp_svn_data->siblings;
	tmp_svn_data->siblings = svn_data;
    }

    /*
    ** Increment child count.
    */

    ref_svn_data->child_cnt++;

    return(tmp_svn_data);
    }


_Void  EnvSvnWindowClearDisplay(private)
_WindowPrivate private;

/*
**++
**  Functional Description:
**	This routine collapse all the entries.
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
    **	Clear an SVN display -- close all entries, then delete the remaining
    **	top level entries.
    */

    DXmSvnDisableDisplay(private->svn);

    DXmSvnDeleteEntries(private->svn, 0, private->opened_entries);

    private->opened_entries = 0;

    DXmSvnEnableDisplay(private->svn);

    return;
    }


_Void  EnvSvnWindowSelection(w, private, cb)
Widget w;
 _WindowPrivate private;

    DXmSvnCallbackStruct *cb;

/*
**++
**  Functional Description:
**	This routine is called whenever an Entry transitions from the
**	unselected to the selected state.  
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**      SVN Arguments passed in cb:
**	    reason
**	    entry_number
**	    component_number
**	    entry_tag
**	    first_selected
**	    time
**	    entry_level
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	DO NOT raise an exception within this routine since it
**	is called from the Toolkit.
**--
*/
    {
    if (cb->component_number == _SvnIconComponent ||
	cb->component_number == _SvnNameComponent)
	/*
	** Become the global selection owner.
	*/
	XtOwnSelection(w, XA_PRIMARY, cb->time,
	    (XtConvertSelectionProc) EnvDWQuickCopyConvertSelection,
	    (XtLoseSelectionProc) EnvDWQuickCopyLoseSelection,
	    (XtSelectionDoneProc) EnvDWQuickCopyConversionDone);
    else
	/*
	** Relinquish the global selection
	*/
	EnvDWQuickCopyDisown(private, cb->time);
	
    return;
    }


_Void  EnvSvnWindowGetSelectedEntry(svn, entry)
_Widget svn;
 _SvnData *entry;

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
    _Integer	cnt;
    _Integer	entry_number;
    _Integer	component;


    /*
    **	Get number of selected entries
    */

    cnt = DXmSvnGetNumSelections(svn);

    /*
    ** If single selection, get it.
    */
    if (cnt == 1)
        DXmSvnGetSelections(svn, (int *) &entry_number,
	     (int *) &component, (XtPointer *) entry, cnt);
    else
	*entry = (_SvnData) 0;
	
    return;
    }

_Void  EnvSvnWindowCollapse(private, svn_data)
_WindowPrivate private;
 _SvnData svn_data;

/*
**++
**  Functional Description:
**	This routine closes a svn_data.
**	This routine should be called with disabled display
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
    _SvnData	child,
		grandchild;

    /*
    **  Collapse and entry so that its children are no longer visible.
    */

    child = svn_data->children;

    while (child != NULL) {

	entry_number = DXmSvnGetEntryNumber(private->svn, (XtPointer) child);

	if (entry_number != 0) {
	
	    grandchild = (_SvnData) DXmSvnGetEntryTag(private->svn,
		entry_number + 1);

	    if (child->children == grandchild)
		EnvSvnWindowCollapse(private, child);
	}

	child = child->siblings;
    }

    entry_number = DXmSvnGetEntryNumber(private->svn, (XtPointer) svn_data);

    if (entry_number != 0) {

	DXmSvnDeleteEntries(private->svn, entry_number, svn_data->child_cnt);
	private->opened_entries -= svn_data->child_cnt;
    }

    return;
    }


_Boolean  EnvSvnWindowGetSelection(private, operation, select_data)
_WindowPrivate private;

    _GetSelectOperation operation;
 _SelectData *select_data;

/*
**++
**  Functional Description:
**	Return the Entry structure for the currently selected entry, if any.
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
    _Boolean	get_selection;
    _Integer	cnt;

    _StartExceptionBlock

    get_selection = _False;

    /*
    **	Get number of selected entries
    */

    cnt = DXmSvnGetNumSelections(private->svn);

    if (cnt > 0) {

	switch (operation) {
	
	    case _SingleSelection:
		if (cnt == 1)
		    get_selection = _True;
		break;
		
	    case _FirstSelection:
		cnt = 1;
		get_selection = _True;
		break;
		
	    case _MultipleSelection:
		get_selection = _True;
		break;
		
	    default:
		break;
	}
    }

    /*
    ** Fill in the select data structure and num value.
    */

    if (get_selection) {

	/*
	**  Allocate the selectdata structure and fields
	*/
	
	(*select_data) =
	    (_SelectData) _AllocateMem(sizeof(_SelectDataInstance));
	_ClearMem((*select_data), sizeof(_SelectDataInstance));

	(*select_data)->entry = (_Integer *)
	    _AllocateMem(sizeof(_Integer) * cnt);
	_ClearMem((*select_data)->entry, sizeof(_Integer) * cnt);

	(*select_data)->component = (_Integer *)
	    _AllocateMem(sizeof(_Integer) * cnt);
	_ClearMem((*select_data)->component, sizeof(_Integer) * cnt);

	(*select_data)->svn_data =
	    (_SvnData *) _AllocateMem(sizeof(_SvnDataInstance) * cnt);
	_ClearMem((*select_data)->svn_data, sizeof(_SvnDataInstance) * cnt);

	DXmSvnGetSelections(private->svn, (int *) (*select_data)->entry,
	     (int *) (*select_data)->component,
	     (XtPointer *) (*select_data)->svn_data, cnt);
	
	(*select_data)->count = cnt;
	
    }

    else
	*select_data = (_SelectData) 0;

    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
        _WhenOthers
	    if (*select_data != (_SelectData) 0)
		EnvSvnWindowFreeSelection(*select_data);
            _Reraise;
	
    _EndExceptionBlock

    return get_selection;
    }


_Void  EnvSvnWindowFreeSelection(selection)
_SelectData selection;

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

    _FreeMem(selection->svn_data);
    _FreeMem(selection->entry);
    _FreeMem(selection->component);
    _FreeMem(selection);

    return;
    }


_Void  EnvSvnWindowSelect(private, hs_object)
_WindowPrivate private;
 _HsObject hs_object;

/*
**++
**  Functional Description:
**	{@description}
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
    _Integer	    entry_num;
    _SvnData	    tag;

    /*
    **  Find the object tag in the internal display list
    */

    tag = FindObjectTag(private->svn_entries, hs_object);

    if (tag != (_SvnData) 0) {

	DXmSvnDisableDisplay(private->svn);

	/*
	**  Get the position of the object in the Svn display
	*/
	
	entry_num = DXmSvnGetEntryNumber(private->svn, (XtPointer) tag);

	if (entry_num > 0) {

	    /*
	    **  Select the entry in the Svn display
	    */
	
	    DXmSvnSelectEntry(private->svn, entry_num);

	    /*
	    **  Scroll it insight if needed
	    */
	
	    DXmSvnPositionDisplay(private->svn, entry_num, DXmSVN_POSITION_MIDDLE);
	}

	DXmSvnEnableDisplay(private->svn);
    }

    return;
    }


_Void  EnvSvnWindowHighlight(private, hs_object)
_WindowPrivate private;
 _HsObject hs_object;

/*
**++
**  Functional Description:
**	{@description}
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
    _Integer	    entry_num;
    _SvnData	    tag;

    tag = FindObjectTag(private->svn_entries, hs_object);

    if (tag != (_SvnData) 0) {

	DXmSvnDisableDisplay(private->svn);

	entry_num = DXmSvnGetEntryNumber(private->svn, (XtPointer) tag);

	if (entry_num > 0) {

	    /*
	    **  Select the entry in the Svn display
	    */
	
	    DXmSvnHighlightEntry(private->svn, entry_num);

	    /*
	    **  Scroll it insight if needed
	    */

	    DXmSvnPositionDisplay(private->svn, entry_num, DXmSVN_POSITION_MIDDLE);
	}

	DXmSvnEnableDisplay(private->svn);
    }

    return;
    }


static _SvnData  FindObjectTag(entry, hs_object)
_SvnData entry;
 _HsObject hs_object;

/*
**++
**  Functional Description:
**	{@description}
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
    _SvnData	tag;

    tag = (_SvnData) 0;

    if ((entry->siblings != (_SvnData) 0) && (tag == (_SvnData) 0))
	tag = FindObjectTag(entry->siblings, hs_object);

    if ((entry->children != (_SvnData) 0) && (tag == (_SvnData) 0))
	tag = FindObjectTag(entry->children, hs_object);

    if ((entry->object == hs_object) && (tag == (_SvnData) 0))
	tag = entry;

    return tag;
    }


_Void  EnvSvnWindowSetHighlight(private, turn_on)
_WindowPrivate private;
 _Boolean turn_on;

/*
**++
**  Functional Description:
**	{@description}
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

    DXmSvnDisableDisplay(private->svn);

    if (turn_on)
	DXmSvnShowHighlighting(private->svn);
    else
	DXmSvnHideHighlighting(private->svn);

    DXmSvnEnableDisplay(private->svn);

    return;
    }


_IsExpandable  EnvSvnWindowIsExpandable(private, svn_data, entry_number, component)
_WindowPrivate private;

    _SvnData svn_data;
 _Integer entry_number;
 _Integer component;

/*
**++
**  Functional Description:
**	This routine checks if the selected svn component is expandable or not.
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
**	_ExpandEntry: the entry can be expanded
**	_CollapseEntry: the entry can be collapsed
**	_DimItem: the entry is invalid
**	_EmptyEntry: the entry has no child
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _SvnData	next;
    lwk_domain		    domain;
    _Integer                type;
    lwk_object		    his_obj;

    if ((component != _SvnIconComponent) &&
	 (component != _SvnNameComponent))
	 return(_DimItem);

    if ((svn_data->object == _NullObject) && !(svn_data->segment))
	return(_DimItem);

    if ((svn_data->object != _NullObject) &&
	(svn_data->retrievable != _Retrievable))
	return(_DimItem);
	
    if (svn_data->children == (_SvnData) 0) {
    
	_GetValue(svn_data->object, _P_HisType, hs_c_domain_integer, &type);

        domain = (lwk_domain) type;

	if ((domain == lwk_c_domain_comp_linknet) ||
	    (domain == lwk_c_domain_comp_path))
	    return(_EmptyEntry);
	else
	    return(_DimItem);
    }

    next = (_SvnData) DXmSvnGetEntryTag(private->svn, entry_number + 1);

    if (next != (_SvnData) 0) {

	if (next->child_level > svn_data->child_level)
	    return(_CollapseEntry);
    }

    return(_ExpandEntry);
    }


_Void  EnvSvnWindowRetrieveEntry(svn_data, iterate_call)
_SvnData svn_data;
 _Callback iterate_call;

/*
**++
**  Functional Description:
**	{@description}
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
    _SvnData		tmp_svn_data;
    lwk_status		status;
    lwk_termination	return_value;
    lwk_object		hisobject;

    /*
    ** Allocate an svn_data structure to be the head of the list.
    */

    tmp_svn_data = EnvSvnWindowCreateSvnData(svn_data->private, svn_data,
		(svn_data->child_level+1));

    /*
    ** Iterate the composite object and collect the object ids.
    */

    _GetValue(svn_data->object, _P_HisObject, hs_c_domain_lwk_object,
	    &hisobject);

    status = lwk_iterate(hisobject, lwk_c_domain_object_desc,
	    tmp_svn_data, (lwk_callback) iterate_call, &return_value);

    if (status != lwk_s_success) {
	EnvSvnWindowFreeSvnData(tmp_svn_data);
	_SaveHisStatus(status);
	_Raise(iterate_failed);
    }

    /*
    ** Hook the list of children to the head svn_data.
    */

    svn_data->children = tmp_svn_data->siblings;
    svn_data->child_cnt = tmp_svn_data->child_cnt;

    _FreeMem(tmp_svn_data);

    return;
    }


_SvnData  EnvSvnWindowCreateHsObject(head_svn_data, domain, object)
_SvnData head_svn_data;

    lwk_domain domain;
 lwk_object *object;

/*
**++
**  Functional Description:
**	{@description}
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
    _SvnData	svn_data;
    _HsObject	hs_object;

    /*
    **  Create a new svn_data entry
    */

    svn_data = EnvSvnWindowCreateSvnData(head_svn_data->private,
	head_svn_data->parent, head_svn_data->child_level);

    /*
    ** Create a LinkWorks Manager object.
    */

    hs_object = (_HsObject) _CreateHsObject(_TypeHsObject, domain, *object);

    svn_data->object = hs_object;

    return(svn_data);
    }


_Void  EnvSvnWindowExpand(private, svn_data, set_entry)
_WindowPrivate private;
 _SvnData svn_data;

    _Callback set_entry;

/*
**++
**  Functional Description:
**	{@description}
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
    _SvnData	child;

    /*
    ** Check if the given entry has children.
    */

    if (svn_data->children != NULL) {

	/*
	** Check if the given entry is currently displayed.
	*/
	
	entry_number = DXmSvnGetEntryNumber(private->svn,
	    (XtPointer) svn_data);

	if (entry_number != 0) {
	
	    /*
	    ** Check if the next entry displayed in svn isn't the same as the
	    ** child of the given entry.
	    */
	
	    child = (_SvnData) DXmSvnGetEntryTag(private->svn, entry_number + 1);

	    if (child != svn_data->children) {
	
		child = svn_data->children;
		
		while (child != NULL) {
		
		    /*
		    ** Add the entry to the svn widget display list.
		    */
		
		    DXmSvnAddEntries(private->svn, entry_number, 1,
			(child->child_level - 1), (XtPointer *) &child,
			_True);
		    private->opened_entries++;

		    /*
		    ** Set the entry.
		    */
		
		    set_entry(private, child);
		    child = child->siblings;
		
		    /*
		    ** Increment the entry_number.
		    */
		
		    entry_number++;
		}
	    }
	}
    }	
    return;
    }


Pixmap  EnvSvnWindowSetPixmap(private, domain)
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
    Pixmap pixmap;

    switch ((lwk_domain)domain) {

	case lwk_c_domain_comp_linknet:
	    if (HsCompositeNetIcon == NULL)
		EnvDWFetchIcon(private, _DrmCompositeNetIcon,		
			    &HsCompositeNetIcon, _False);
	    pixmap = HsCompositeNetIcon;
	    break;

	case lwk_c_domain_linknet:
	    if (HsNetworkIcon == NULL)
		EnvDWFetchIcon(private, _DrmNetworkIcon,		
			    &HsNetworkIcon, _False);
	    pixmap = HsNetworkIcon;
	    break;

	case lwk_c_domain_comp_path:
	    if (HsCompositePathIcon == NULL)
		EnvDWFetchIcon(private, _DrmCompositePathIcon,		
			    &HsCompositePathIcon, _False);
	    pixmap = HsCompositePathIcon;
	    break;
		
	case lwk_c_domain_path:
	    if (HsPathIcon == NULL)
		EnvDWFetchIcon(private, _DrmPathIcon,		
			    &HsPathIcon, _False);
	    pixmap = HsPathIcon;
	    break;
	default:
	    _Raise(inv_domain);
	    break;
    }

    return pixmap;
    }


_Boolean  EnvSvnWindowSetIndex(domain)
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
    _Boolean index = _False;

    if (domain == lwk_c_domain_comp_linknet ||
	domain == lwk_c_domain_comp_path)
	index = _True;

    return index;

    }


_Void  EnvSvnWindowRemoveEntry(private, list, end_list, svn_data, free)
_WindowPrivate private;
 _SvnData *list;

    _SvnData *end_list;
 _SvnData svn_data;
 _Boolean free;

/*
**++
**  Functional Description:
**	{@description}
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
		    parent;
    int		    entry_number;
    _IsExpandable   expand;

    parent = svn_data->parent;

    /*
    ** Get the entry number.
    */
    entry_number = DXmSvnGetEntryNumber(private->svn, (XtPointer) svn_data);

    /*
    ** Get the previous_entry.
    */

    previous_entry = SvnWindowGetPreviousSiblingTag(svn_data);

    /*
    ** Collapse the given entry.
    */

    expand = EnvSvnWindowIsExpandable(private, svn_data,
		entry_number, _SvnNameComponent);

    if (expand == _CollapseEntry)
	EnvSvnWindowCollapse(private, svn_data);

    /*
    ** If the svn data is the only one in the list, set the pointers to zero.
    */
    if (*list == *end_list)
	*list = *end_list = (_SvnData) 0;
    else {
	/*
	** If the svn data is in beginning of list, set the beginning of list
	** pointer to be the next one.
	*/
	if (*list == svn_data)
	    *list = svn_data->siblings;
	
	/*
	** If the svn data is at the end of the list set the end list
	** pointer to the previous one.
	*/
	else
	    if (*end_list == svn_data)
		*end_list = previous_entry;
    }
    /*
    ** Remove the entry from the list.
    */

    if (previous_entry == parent)
	parent->children = svn_data->siblings;
    else
	previous_entry->siblings = svn_data->siblings;
	
    parent->child_cnt--;

    /*
    ** Delete the entry from the display list.
    */

    DXmSvnDeleteEntries(private->svn, entry_number-1, 1);


    /*
    ** If free, free the svn data structure.
    */
    if (free)
	EnvSvnWindowFreeSvnData(svn_data);

    return;
    }


_Void  EnvSvnWindowDisplayEntry(private, previous_entry, entry, set_entry)
_WindowPrivate private;

    _SvnData previous_entry;
 _SvnData entry;
 _Callback set_entry;

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
    int		    entry_number;
    _IsExpandable   expand;

    /*
    ** Get the entry number of the insertion point.
    */

    entry_number = DXmSvnGetEntryNumber(private->svn,
	(XtPointer) previous_entry);

    /*
    ** If the selected entry is displayed, display the inserted entry.
    */

    if (entry_number != 0) {

	/*
	** Check if the selected entry is the parent.
	*/
	
	if (entry->parent == previous_entry) {
	    	
	    /*
	    ** If it has more than one child.
	    */
	    if (previous_entry->child_cnt > 1) {
		/*
		** Check if the parent is expanded
		*/
		expand = EnvSvnWindowIsExpandable(private, previous_entry,
		    entry_number, _SvnNameComponent);
			
		/*
		** If the parent isn't expanded, return.
		*/
		
		if (expand == _ExpandEntry)
		    return;
	    }
	}

	/*
	** Clear the selection.
	*/
	
	DXmSvnClearSelections(private->svn);
	
	/*
	** Add the entry to the display list.
	*/
	
	DXmSvnAddEntries(private->svn, entry_number, 1,
	    (entry->child_level - 1), (XtPointer *) &entry, _True);
	private->opened_entries++;

	/*
	** Set the entry.
	*/
	
	set_entry(private, entry);

	/*
	** Select the entry.
	*/
	
	DXmSvnSelectComponent(private->svn, entry_number + 1,
				_SvnNameComponent);

	/*
	**  Scroll it insight if needed
	*/

	DXmSvnPositionDisplay(private->svn, entry_number + 1, DXmSVN_POSITION_MIDDLE);

    }

    return;
    }


_Void  EnvSvnWindowCollectObjectList(list, head)
lwk_list list;
 _AnyPtr head;

/*
**++
**  Functional Description:
**	{@description}
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
    lwk_status		    status;
    lwk_domain		    domain;
    _Integer                type;
    lwk_object_descriptor   obj_dsc;
    lwk_object		    his_obj;
    _SvnData		    entry = (_SvnData) head;

   /*
   **  Run dowm the list of Svn entries
   */

    while (entry != (_SvnData) 0) {

	if (entry->object != (_HsObject) 0) {

	    /*
	    **  Get info about the his object
	    */
	
	    _GetValue(entry->object, _P_HisObject, hs_c_domain_lwk_object,
		&his_obj);

	    _GetValue(entry->object, _P_HisType, hs_c_domain_integer, &type);

            domain = (lwk_domain) type;

	    if (domain == lwk_c_domain_object_desc)
		obj_dsc = his_obj;
	    else {

		/*
		**  Get the object descriptor from the persistent
		*/

		status = lwk_get_object_descriptor(his_obj, &obj_dsc);
		if (status != lwk_s_success) {
		    _SaveHisStatus(status);
		    _Raise(get_objdsc_failed);
		}
	    }

	    /*
	    **  Add the obj desc into the list of obj desc
	    */
	
	    status = lwk_add_element(list, lwk_c_domain_object_desc, &obj_dsc);
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(add_ele_failed);
	    }

	    if (domain != lwk_c_domain_object_desc) {
	
		status = lwk_delete(&obj_dsc);
		if (status != lwk_s_success) {
		    _SaveHisStatus(status);
		    _Raise(delete_err);
		}
	    }
	}

	entry = entry->siblings;
    }

    return;
    }


_Void  EnvSvnWindowUpdateComposite(entry)
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
    _String	    property;
    lwk_status	    status;
    lwk_domain	    obj_domain;
    lwk_domain	    domain;
    lwk_persistent  composite;
    lwk_list	    list;
    _Integer        type;

    /*
    **  Check the domain of the object and create a list or a set
    */

    _GetValue(entry->object, _P_HisType, hs_c_domain_integer, &type);

    obj_domain = (lwk_domain) type;

    switch (obj_domain) {

	case lwk_c_domain_comp_linknet :
	
	    status = lwk_create_set(lwk_c_domain_object_desc, 10, &list);
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(create_list_failed);
	    }
	    property = lwk_c_p_linknets;
	    domain = lwk_c_domain_set;
	    break;

	case lwk_c_domain_comp_path :
	
	    status = lwk_create_list(lwk_c_domain_object_desc, 10, &list);
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(create_list_failed); /* his list creation failed */
	    }
	    property = lwk_c_p_paths;
	    domain = lwk_c_domain_list;
	    break;

	default :
	    _Raise(inv_domain);
	    break;
    }

    EnvSvnWindowCollectObjectList(list, entry->children);

    /*
    **  Update the property on the in-memory composite object
    */

    _GetValue(entry->object, _P_HisObject, hs_c_domain_lwk_object, &composite);

    status = lwk_set_value(composite, property, domain, &list, lwk_c_set_property);
    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(set_value_failed);
    }

    return;
    }


_Void  RemvEntrySiblingsDataFromTable(entry, table)
_SvnData entry;
 _MapTable table;

/*
**++
**  Functional Description:
**	Remove all the siblings and children of the data entry from the map
**	table
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
    lwk_object his_obj;

    /*
    ** Remove the siblings
    */
    if (entry->siblings != (_SvnData) 0)
	RemvEntrySiblingsDataFromTable(entry->siblings, table);

    /*
    ** Remove the children 
    */
    if (entry->children != (_SvnData) 0)
	RemvEntrySiblingsDataFromTable(entry->children, table);

    if (entry->retrievable == _Retrievable) {
	_GetValue(entry->object, _P_HisObject, hs_c_domain_lwk_object,
	    &his_obj);
	_MapTableRemoveEntryData(table, his_obj, entry);
    }

    return;
    }


_Void  EnvSvnRemvEntryDataFromTable(entry, table)
_SvnData entry;
 _MapTable table;

/*
**++
**  Functional Description:
**	Remove the data entry and all its children from the map table
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
    lwk_object his_obj;

    /*
    ** Remove the children 
    */
    if (entry->children != (_SvnData) 0)
	RemvEntrySiblingsDataFromTable(entry->children, table);

    /*
    ** Remove the entry
    */
    if (entry->retrievable == _Retrievable) {

	_GetValue(entry->object, _P_HisObject, hs_c_domain_lwk_object, &his_obj);
	_MapTableRemoveEntryData(table, his_obj, entry);
    }

    return;
    }

    
_Void  EnvSvnWindowSetEntriesNonRetr(private, entries, count)
_WindowPrivate private;

    _SvnData *entries;
 _Integer count;

/*
**++
**  Functional Description:
**	Set the entries to non retrievable, and collapse them if they are
**	expanded
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
    _SvnData		    entry;
    _Integer		    i;
    _Integer		    entry_number;
    lwk_persistent	    his_obj;
    lwk_object_descriptor   obj_dsc;
    lwk_status		    status;
    _Integer		    domain;

    /*
    **  Get the corresponding HIS object of the first entry in the list
    */

    entry = (_SvnData) entries[0];
    _GetValue(entry->object, _P_HisObject, hs_c_domain_lwk_object, &his_obj);

    /*
    **  Get the object descriptor for this object and reset the corresponding
    **	hs_object on all the entries
    */

    status = lwk_get_object_descriptor(his_obj, &obj_dsc);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(get_objdsc_failed);
    }

    domain = lwk_c_domain_object_desc;

    for (i = 0; i < count; i++) {

	entry = (_SvnData) entries[i];

        /*
	** If this entry is expanded, collapse it
	*/
	entry_number = DXmSvnGetEntryNumber(private->svn, (XtPointer) entry);

	if (EnvSvnWindowIsExpandable(private, entry, entry_number,
	    _SvnNameComponent) == _CollapseEntry) 
	    _Collapse(private->window, entry);

        /*
	** If this entry has children, remove their entries from the map table
	** and free the Svn entries
	*/
	if (entry->children != (_SvnData) 0) {
	    RemvEntrySiblingsDataFromTable(entry->children, private->map_table);
	    EnvSvnWindowFreeSvnSiblData(entry->children);
	}
        
	entry->retrievable = _NotRetrievable;

	_SetValue(entry->object, _P_HisObject, hs_c_domain_lwk_object, &obj_dsc,
	    hs_c_set_property);
	
	_SetValue(entry->object, _P_HisType, hs_c_domain_integer, &domain,
	    hs_c_set_property);
    }

    return;
    }
