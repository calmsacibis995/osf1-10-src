/*
** COPYRIGHT (c) 1989, 1991, 1992 BY
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
**	SVN support routines for the linkbase window.
**
**  Keywords:
**	{@keywords or None@}
**
**  Environment:
**	{@environment description@}
**
**  Author:
**	Patricia Avigdor
**	[@author information@]...
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
**  Type definitions
*/
typedef struct __IterateDataComp {
	    lwk_object parent;
	    lwk_domain domain;
	    _Boolean   retrievable;
	    _Boolean   found;
	    } _IterateDataCompInstance, *_IterateDataComp;

/*
**  Forward Routine Declarations
*/                                                      
                                             
_DeclareFunction(static lwk_termination SvnLbWindowListLinkbase,
	(_SvnIterateData iterate_data, lwk_list list, lwk_domain domain,
	lwk_object *object));
_DeclareFunction(static lwk_termination SvnLbWindowListComposite,
	(_SvnData head_svn_data, lwk_list list, lwk_domain domain,
	lwk_object_descriptor *object_desc));
_DeclareFunction(static _Void SvnLbWindowSetTitle,
	(_WindowPrivate private));
_DeclareFunction(static _Void SvnLbWindowSetEntry,
	(_WindowPrivate private, _SvnData svn_data));
_DeclareFunction(static _Void SvnLbWindowSetComponents,
	(_WindowPrivate private, _SvnData svn_data, int entry_number,
	Pixmap pixmap, _CString	name, _CString linkbase_name,
	_Boolean index, _Boolean small_font));
_DeclareFunction(static _Void SvnLbWindowConfirm,
	(Widget w, _WindowPrivate private, DXmSvnCallbackStruct *cb));
_DeclareFunction(static _Void SvnLbWindowGetListPointers,
    (_WindowPrivate private, lwk_domain domain, _SvnData **begin_list,
    _SvnData **end_list));
_DeclareFunction(static _Void SvnLbWindowInsertQuickCopy,
    (_WindowPrivate private, lwk_object object,
     lwk_domain domain, _Integer x, _Integer y));
_DeclareFunction(static _Void SvnLbWindowInsertEntry,
    (_WindowPrivate private, lwk_object object,
     lwk_domain domain, _SvnData selected_entry, _Boolean existing));
_DeclareFunction(static _Void SvnLbWindowInsertObject,
    (_WindowPrivate private, _SvnData selected_data, lwk_object object,
    lwk_domain domain, _InsertEntry insert_entry, _Boolean existing));
_DeclareFunction(static _Boolean SearchIfParentInComposite,
    (lwk_object parent, lwk_object object, lwk_domain domain));
_DeclareFunction(static lwk_termination ObjectDescInComposite,
    (_IterateDataComp iterate_data, lwk_persistent object, lwk_domain domain,
    lwk_object_descriptor *object_desc));
_DeclareFunction(static _Boolean SvnLbWindowObjectInSegment,
    (_WindowPrivate private, lwk_object object));
_DeclareFunction(static _Void UpdateNonRetrievableObjects,
    (_WindowPrivate private, _SvnData entry, lwk_string linkbase_id,
     _CString linkbase_name));
_DeclareFunction(static _SvnData SvnLbWindowGetPreviousEntry,
    (_WindowPrivate private, lwk_domain domain));
_DeclareFunction(static _SvnData SvnLbWindowGetNextEntry,
    (_WindowPrivate private, lwk_domain domain));
_DeclareFunction(static _Void SvnLbWindowSetLbModified,
    (_WindowPrivate private));
_DeclareFunction(static _Void SvnLbWindowQuickCopyOrMove,
    (_Widget widget, _WindowPrivate private, _Integer x, _Integer y,
    Time timestamp, _Boolean move));

    
/*
**  Macro Definitions
*/
         
#define _LbMapTableInitialSize	50
#define _LbWindowPrimObjLevel	2

/*
**  Static Data Definitions
*/

static Pixmap HsEmptyIcon = NULL;

/*
**  Global Data Definitions
*/

/*
**  External Data Declarations
*/

_DeclareFunction(_Void EnvSvnWindowUpdateComposite,
    (_SvnData entry));
_DeclareFunction(_Void EnvSvnRemvEntryDataFromTable,
    (_SvnData entry, _MapTable table));


_Void  EnvSvnLbWindow(private)
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
    

_Void  EnvSvnLbWindowInitialize(private)
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
    Arg		arglist[20];
    XtCallbackRec confirm_cb[2];
    XtCallbackRec select_cb[2];

    confirm_cb[0].callback = (XtCallbackProc) SvnLbWindowConfirm;
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
    
                        
_Void  EnvSvnLbWindowLoadLinkbase(private, lb_entry, linkbase)
_WindowPrivate private;
 _SvnData *lb_entry;

    _HsObject linkbase;

/*
**++
**  Functional Description:
**	iterates over a linkbase for all the object contained in it
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
    lwk_status		status;
    lwk_closure		return_value;
    _SvnData		svn_data,
			tmp_svn_data;
    _SvnIterateData	iterate_data;
    lwk_linkbase	his_lb;
    
    _StartExceptionBlock
    
    /*
    ** Create a map table.
    */
    
    private->map_table = _MapTableCreate(_LbMapTableInitialSize);

    /*
    ** Allocate an svn_data structure to be the head of the list.
    */
    svn_data = EnvSvnWindowCreateSvnData(private, (_SvnData) 0, 1);

    /*
    ** Allocate an iterate data structure.
    */
    iterate_data = (_SvnIterateData)
	    _AllocateMem(sizeof(_SvnIterateDataInstance));
    _ClearMem(iterate_data, sizeof(_SvnIterateDataInstance));
	    
    /*
    **  Create a temporary svn_data to start building the list of children
    */
    tmp_svn_data = EnvSvnWindowCreateSvnData(private, svn_data, 2);
    
    /*
    ** Set the iterate data structure.
    */
    iterate_data->head_svn_data = tmp_svn_data;
    iterate_data->end_list = (_SvnData) 0;
    
    /*
    ** Get the his linkbase object.
    */
    _GetValue(linkbase, _P_HisObject, hs_c_domain_lwk_object, &his_lb);
    
    /*
    **  Iterate the linkbase and collect the composite net object ids
    */
    status = lwk_iterate(his_lb, lwk_c_domain_comp_linknet, iterate_data,
	SvnLbWindowListLinkbase, &return_value);

    if (status != lwk_s_success) {
	EnvSvnWindowFreeSvnSiblData(tmp_svn_data);
	_FreeMem(iterate_data);
	_SaveHisStatus(status);
	_Raise(iterate_failed);/*iterate failed*/
    }
    /*
    ** If the linkbase contains composite networks, set the comp_net list
    ** pointer and hook the list to the head svn data.
    */
    
    if (tmp_svn_data->child_cnt != 0) {
	((_LbWindowPrivate)(private->specific))->comp_net_list =
		tmp_svn_data->siblings;
	((_LbWindowPrivate)(private->specific))->end_comp_net_list =
		iterate_data->end_list;
	EnvSvnWindowInsertEndOfList(&svn_data->children,
				    tmp_svn_data->siblings, _True);
	svn_data->child_cnt += tmp_svn_data->child_cnt;
	/*
	** Reinitialize the work structures.
	*/
	tmp_svn_data->siblings = (_SvnData) 0;
	tmp_svn_data->child_cnt = 0;
	iterate_data->end_list = (_SvnData) 0;
    }	
    /*
    **  Iterate the linkbase and collect the network object ids
    */
    
    status = lwk_iterate(his_lb, lwk_c_domain_linknet, iterate_data,
	SvnLbWindowListLinkbase, &return_value);

    if (status != lwk_s_success) {
	EnvSvnWindowFreeSvnSiblData(tmp_svn_data);
	_FreeMem(iterate_data);
	_SaveHisStatus(status);
	_Raise(iterate_failed); /*iterate failed*/
    }
    /*
    ** If the linkbase contains networks, set the net list
    ** pointer and hook the list into the head svn data.
    */
    if (tmp_svn_data->child_cnt != 0) {
	((_LbWindowPrivate)(private->specific))->net_list =
		tmp_svn_data->siblings;
	((_LbWindowPrivate)(private->specific))->end_net_list =
		iterate_data->end_list;
	EnvSvnWindowInsertEndOfList(&svn_data->children,
				    tmp_svn_data->siblings, _True);
	svn_data->child_cnt += tmp_svn_data->child_cnt;
	/*
	** Reinitialize the work structures.
	*/
	tmp_svn_data->siblings = (_SvnData) 0;
	tmp_svn_data->child_cnt = 0;
	iterate_data->end_list = (_SvnData) 0;
    }				    	

    /*
    **  Iterate the linkbase and collect the composite path object ids
    */
    
    status = lwk_iterate(his_lb, lwk_c_domain_comp_path, iterate_data,
	SvnLbWindowListLinkbase, &return_value);

    if (status != lwk_s_success) {
	EnvSvnWindowFreeSvnSiblData(tmp_svn_data);
	_FreeMem(iterate_data);
	_SaveHisStatus(status);
	_Raise(iterate_failed);/*iterate failed*/
    }
    /*
    ** If the linkbase contains composite paths, set the comp path list
    ** pointer and hook the list into the head svn data.
    */
    if (tmp_svn_data->child_cnt != 0) {
	((_LbWindowPrivate)(private->specific))->comp_path_list =
		tmp_svn_data->siblings;
	((_LbWindowPrivate)(private->specific))->end_comp_path_list =
		iterate_data->end_list;
	EnvSvnWindowInsertEndOfList(&svn_data->children,
				    tmp_svn_data->siblings, _True);
	svn_data->child_cnt += tmp_svn_data->child_cnt;
	/*
	** Reinitialize the work structures.
	*/
	tmp_svn_data->siblings = (_SvnData) 0;
	tmp_svn_data->child_cnt = 0;
	iterate_data->end_list = (_SvnData) 0;
    }				    	
    /*
    **  Iterate the linkbase and collect the path object ids
    */
    status = lwk_iterate(his_lb, lwk_c_domain_path, iterate_data,
 	SvnLbWindowListLinkbase, &return_value);

    if (status != lwk_s_success) {
	EnvSvnWindowFreeSvnSiblData(tmp_svn_data);
	_FreeMem(iterate_data);
	_SaveHisStatus(status);
	_Raise(iterate_failed);/*iterate failed*/
    }
    /*
    ** If the linkbase contains paths, set the path list
    ** pointer and hook the list into the head svn data.
    */
    if (tmp_svn_data->child_cnt != 0) {
	((_LbWindowPrivate)(private->specific))->path_list =
		tmp_svn_data->siblings;
	((_LbWindowPrivate)(private->specific))->end_path_list =
		iterate_data->end_list;
	EnvSvnWindowInsertEndOfList(&svn_data->children,
				    tmp_svn_data->siblings, _True);
	svn_data->child_cnt += tmp_svn_data->child_cnt;
	/*
	** Reinitialize the work structures.
	*/
	tmp_svn_data->siblings = (_SvnData) 0;
	tmp_svn_data->child_cnt = 0;
	iterate_data->end_list = (_SvnData) 0;
    }				    	

    _FreeMem(tmp_svn_data);
    _FreeMem(iterate_data);
    
    *lb_entry = svn_data;

    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */
    
    _Exceptions
        _WhenOthers
	    _FreeMem(svn_data);
	    if (tmp_svn_data != (_SvnData) _NullObject)
		_FreeMem(tmp_svn_data);
            _Reraise;
	    
    _EndExceptionBlock

    return;
    }

static lwk_termination  SvnLbWindowListLinkbase(iterate_data, list, domain, object)
_SvnIterateData iterate_data;

	lwk_list list;
 lwk_domain domain;
 lwk_object *object;

/*
**++
**  Functional Description:
**	Creates a svn_data structure for an object contained in the linkbase.
**	Creates a corresponding aggregate object.
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
    _SvnData	head_svn_data,
		svn_data;
    
    head_svn_data = iterate_data->head_svn_data;

    /*
    ** Check if the object isn't a LinkWorks Manager private object.
    */
    if (!_IsPrivateObject(*object)) {
	/*
	** Create a HsObject and the svn data structure associated.
	*/
	svn_data = EnvSvnWindowCreateHsObject(head_svn_data, domain,
	    object);
	svn_data->retrievable = _Retrievable;
		    
	/*
	** Add the entry to the map table.
	*/
	
	_MapTableAddEntry(&((_WindowPrivate)head_svn_data->private)->map_table,
			*object, svn_data);

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
	switch (domain) {
	    case lwk_c_domain_comp_linknet :
	    case lwk_c_domain_comp_path :
		    EnvSvnWindowRetrieveEntry(svn_data,
			SvnLbWindowListComposite);
		    break;
	}
    }
    	    
    return (lwk_termination) 0;
    }

static lwk_termination  SvnLbWindowListComposite(head_svn_data, list, domain, object_desc)
_SvnData head_svn_data;

    lwk_list list;
 lwk_domain domain;
 lwk_object_descriptor *object_desc;

/*
**++
**  Functional Description:
**	This routine iterates over the given object for the given domain and
**	builds up a list of entries
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
    lwk_object  object;
    lwk_status	status;
    _Integer	retrievable;
    
		              	
    /*
    ** It's an object descriptor.
    */
    if (domain == lwk_c_domain_object_desc) {
	/*
	** Retrieve the persistent object.
	*/
	status = lwk_retrieve(*object_desc, &object);
	if (status == lwk_s_success) {
	    retrievable = _Retrievable;
	    /*
	    ** Get the domain.
	    */
	    status = lwk_get_domain(object, &domain);

	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(get_domain_failed);
	    }
	}
	/*
	** All we can get is an object descriptor.
	*/
	else {
	    object = *object_desc;

	    /*
	    ** if the linkbase has been deleted mark its retrievable field
	    */
	    if (status == lwk_s_no_such_linkbase)
		retrievable = _LbNotRetrievable;
	    else
		retrievable = _NotRetrievable;
	}
    }
    else {
	object = *object_desc;
	retrievable = _Retrievable;
    }
    /*
    ** Create a HsObject and the svn data structure associated.
    */
    svn_data = EnvSvnWindowCreateHsObject(head_svn_data, domain,
		&object);

    svn_data->retrievable = retrievable;
    	
    /*
    ** Add the entry to the map table only if the entry is retrievable
    */
    if (retrievable == _Retrievable)
	_MapTableAddEntry(&((_WindowPrivate)head_svn_data->private)->map_table,
			object, svn_data);

    /*
    **  Insert the new svn_data in the right place.
    */
    EnvSvnWindowInsertEndOfList(&head_svn_data->siblings, svn_data, _False);

    /*
    ** Increment child count.
    */
    head_svn_data->child_cnt++;
    
    return (lwk_termination) 0;
}

_Void  EnvSvnLbWindowDisplayLb(private)
_WindowPrivate private;

/*
**++
**  Functional Description:
**	This routine adds the linkbase entries in the Svn display list.
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
    SvnLbWindowSetTitle(private);

    /*
    ** Add all the linkbase contents.
    */
    EnvSvnLbWindowExpand(private, private->svn_entries);

    /*
    ** Clear the selection.
    */
    DXmSvnClearSelections(private->svn);
    

    return;
    }

static _Void  SvnLbWindowSetTitle(private)
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
    _CString cs_linkbase;
    _Boolean index = _True;
    _Integer entry_number = 1;

    /*
    ** Add the entry to the svn display list.
    */
    DXmSvnAddEntries(private->svn, 0, 1, 0,
	(XtPointer *) &private->svn_entries, index);

    /*
    ** Set the column header entry to insensitive.
    */
    DXmSvnSetEntrySensitivity(private->svn, entry_number, _False);
    
    /*
    ** Increment the number of open entries.
    */
    private->opened_entries++;

    /*
    ** Set the title.
    */
    EnvDWGetCStringLiteral(_DwtSvnTitleName, &cs_name);
    EnvDWGetCStringLiteral(_DwtSvnTitleLinkbase, &cs_linkbase);
    if (HsEmptyIcon == NULL)
	EnvDWFetchIcon(private, _DrmEmptyIcon, &HsEmptyIcon, _False);

    /*
    ** Set the user data.
    */
    DXmSvnSetEntryTag(private->svn, entry_number,
	(XtPointer) private->svn_entries);

    /*
    ** Set the number of components.
    */
    DXmSvnSetEntryNumComponents(private->svn, entry_number, 3);
    
    /*
    ** Set the components.
    */
    
    DXmSvnSetComponentPixmap(private->svn, entry_number, _LbSvnIconComponent,
	0, 0, HsEmptyIcon, _IconHeight, _IconWidth); 
    
    DXmSvnSetComponentText(private->svn, entry_number, _LbSvnNameComponent,
	20, 0, (XmString) cs_name, NULL);

    DXmSvnSetComponentText(private->svn, entry_number,
	_LbSvnLinkbaseComponent, 100, 0, (XmString) cs_linkbase, NULL);
	
    return;
    }

_Void  EnvSvnLbWindowExpand(private, svn_data)
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
    EnvSvnWindowExpand(private, svn_data, SvnLbWindowSetEntry);

    return;
    }

static _Void  SvnLbWindowSetEntry(private, svn_data)
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
    _HsObject	hslinkbase;
    _CString	linkbase_name;
    _Integer	desc_domain;
    _Boolean	index = _False;
    _Boolean	lb_exist;
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
	** Get the linkbase object displayed in the window.
	*/
	
	_GetValue(private->window, _P_Linkbase, hs_c_domain_hsobject,
	    &hslinkbase);
	    
	/*
	** Get its name.
	*/
	  
	_GetCSProperty(hslinkbase, lwk_c_p_name, &linkbase_name);
				    
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

	    lb_exist = _True;
				
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
	
	    _GetCSProperty(svn_data->object, lwk_c_p_name, &cs_name);
	    
	    if (cs_name == (_CString) 0) 
		_GetCSProperty(svn_data->object, lwk_c_p_description, &cs_name);
		
	    if (cs_name == (_CString) 0)
		EnvDWGetCStringLiteral(_DwtSvnUnnamed, &cs_name);
		
	    lb_exist = _GetLinkbase(svn_data->object, &cs_linkbase,
		&lb_id);
	    
	    /*
	    ** Set the pixmap.
	    */
	    
	    pixmap = EnvSvnWindowSetPixmap(private, type);
	    
	    /*
	    ** Set the index depending on the type.
	    */
	    
	    index = EnvSvnWindowSetIndex(type);

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
	/*
	** Compare the object's linkbase to current one to know if it should be
	** displayed.
	*/

	if (lb_exist) {
	    if (!_CompareCString(cs_linkbase, linkbase_name)) 
		cs_linkbase = _NullObject;
	}
	else
	    cs_linkbase = _NullObject;
    }
    else
	_Raise(object_not_found); /* No corresponding HsObject */
    
    /*
    ** Set the entry.
    */
    
    SvnLbWindowSetComponents(private, svn_data, entry_number, pixmap,
	cs_name, cs_linkbase, index, small_font);	    

    return;
    }
    
    
static _Void  SvnLbWindowSetComponents(private, svn_data, entry_number, pixmap, name, linkbase_name, index, small_font)
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
    XmFontList font;
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
    ** Set the components.
    */ 
    if (linkbase_name != (_CString) _NullObject) {
	DXmSvnSetEntryNumComponents(private->svn, entry_number, 3);
	
	DXmSvnSetComponentText(private->svn, entry_number,
	    _LbSvnLinkbaseComponent, 100, 0, (XmString) linkbase_name, lb_font);
    }
    else
	DXmSvnSetEntryNumComponents(private->svn, entry_number, 2);

    DXmSvnSetComponentPixmap(private->svn, entry_number, _LbSvnIconComponent,
	0, 0, pixmap, _IconHeight, _IconWidth); 
    
    DXmSvnSetComponentText(private->svn, entry_number, _LbSvnNameComponent,
	20, 0, (XmString) name, font); 
    
    return;
    }
    


static _Void  SvnLbWindowConfirm(w, private, cb)
Widget w;
 _WindowPrivate private;

    DXmSvnCallbackStruct *cb;

/*
**++
**  Functional Description:
**	SelectAndConfirm callback routine.This routine is called when one and
**	only one entry is selected by a double click MB1. The entry number
**	selected is provided in the callback structure.
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
    _IsExpandable   expand;
    
    svn_data = (_SvnData) cb->entry_tag;

    /*
    ** Display watch cursor.
    */
    
    _SetCursor(private->window, _WaitCursor);

    /*
    ** Disable display.
    */
    DXmSvnDisableDisplay(private->svn);

    _StartExceptionBlock

    switch(cb->component_number) {
	case _LbSvnIconComponent:
	case _LbSvnNameComponent:
	    /*							
	    **  Determine if the entry can be expanded. If so,
	    **	retrieve the entry tag value for the given entry number.
	    **	If the corresponding svn_data is opened, close it.
	    **	Otherwise open it.
	    */
			
	    expand = EnvSvnWindowIsExpandable(private, svn_data,
		cb->entry_number, cb->component_number);
			    
	    switch (expand) {
		case _DimItem:
		    XBell(XtDisplay(private->svn), 0);
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
	/*
	** If linkbase name, open it in a new window if isn't already opened.
	*/
	case _LbSvnLinkbaseComponent:
	    if (svn_data->object != _NullObject)
		EnvDWLbWindowOpenLinkbase(private, svn_data->object);
	    break;

	default:
	    break;	    
    }
    
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
    

_Void  EnvSvnLbWindowRemoveEntry(private, svn_data, timestamp)
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
    _SvnData	*begin_list,
		*end_list;
    _Integer	type;
    _Integer	desc_domain;
    _Status	status[1];
    lwk_object	object;
    
    /*
    ** If it's a retrievable object.
    */
    
    if (svn_data->retrievable == _Retrievable) {
    
	/*
	** Get the his object.
	*/
	
	_GetValue(svn_data->object, _P_HisObject, hs_c_domain_lwk_object,
	    &object);
	
	/*
	** Get the object domain.
	*/
	
	_GetValue(svn_data->object, _P_HisType, hs_c_domain_integer, &type);
    }
    else {
    
	/*
	** Get real domain.
	*/
	
	_GetProperty(svn_data->object, lwk_c_domain_integer,
	    lwk_c_p_object_domain, &type);
    }
    
    /*
    ** Remove the entry from the map table.
    */

    if (svn_data->retrievable == _Retrievable)
	EnvSvnRemvEntryDataFromTable(svn_data, private->map_table);
    
    /*
    ** Get the list pointers.
    */
    
    SvnLbWindowGetListPointers(private, (lwk_domain) type, &begin_list, &end_list);
    
    /*
    ** If the entry is non-retrievable or it is a child of a composite (which
    ** means that it is an object desc not the real object!) just delete the
    ** entry in Svn but don't delete the real object itself
    */

    if ((svn_data->retrievable != _Retrievable) || (svn_data->child_level == 3))
	EnvSvnWindowRemoveEntry(private, begin_list, end_list, svn_data, _True);

    else {

	EnvSvnWindowRemoveEntry(private, begin_list, end_list, svn_data,
	    _False);

        /*
	** If it is the first object the user is deleting, prompt him to save
	** his linkbase.
	*/
	if (private->deleted_entries == (_SvnData) 0) {
	    status[0] = _StatusCode(lb_to_be_saved);
	    _DisplayMessage(private->window, status, 1);
	}
	    
	/*
	** Add the entry into the deleted list.
	*/
	
	EnvSvnWindowInsertEndOfList(&private->deleted_entries, svn_data,
	    _False);
    }

    /*
    ** If the entry was into a composite, update the parent
    */
    
    if (((_SvnData)(svn_data->parent))->child_level == _LbWindowPrimObjLevel) {
    
	_SetHsObjState(((_SvnData)(svn_data->parent))->object, _StateModified,
	    _StateSet);
	EnvSvnWindowUpdateComposite(svn_data->parent);
    }
	
    /*
    ** Set the linkbase to modified.
    */
    
    SvnLbWindowSetLbModified(private);

    /*
    ** Relinquish selection ownership.
    */
    
    EnvDWQuickCopyDisown(private, timestamp);

    return;
    }
    

_Void  EnvSvnLbWindowInsertEntry(private, object, domain, existing)
_WindowPrivate private;
 lwk_object object;

     lwk_domain domain;
 _Boolean existing;

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
    ** If it's not an object creation, check if there is something selected.
    */
    if (existing) {
    
	/*
	** Get the selected data.
	*/
	if (_GetSelection(private->window, _SingleSelection, &select_data))
	    selected_entry = select_data->svn_data[0];
	else
	    selected_entry = (_SvnData) 0;
    }
    else
	selected_entry = (_SvnData) 0;
    		
    /*
    ** Insert the entry in the right place.
    */
    SvnLbWindowInsertEntry(private, object, domain,
			    selected_entry, existing);

    if (select_data != (_SelectData) 0)
	EnvSvnWindowFreeSelection(select_data);
    
    return;
    }

static _Void  SvnLbWindowInsertEntry(private, object, domain, selected_entry, existing)
_WindowPrivate private;
 lwk_object object;

     lwk_domain domain;
 _SvnData selected_entry;
 _Boolean existing;

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
    lwk_object	    hisobject;
    _Integer	    entry_number;
    _Boolean	    inserted = _False;

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
	    
	    _GetValue(selected_entry->object, _P_HisType, hs_c_domain_integer,
		&type);

	    /*
	    ** If selected object is at first level, check domain
	    */
	    
	    if (selected_entry->child_level == 2) {
	    
		switch ((lwk_domain)type) {
		
		    case lwk_c_domain_comp_linknet:
			if (domain == lwk_c_domain_comp_linknet ||
			    domain == lwk_c_domain_linknet) {
			    SvnLbWindowInsertObject(private, selected_entry,
				object, domain, _IntoList, _True);
			    inserted = _True;
			}    
			break;
			
		    case lwk_c_domain_comp_path:
			if (domain == lwk_c_domain_comp_path ||
			    domain == lwk_c_domain_path) {
			    SvnLbWindowInsertObject(private, selected_entry,
				object, domain, _IntoList, _True);
			    inserted = _True;
			}
			break;
			
		    case lwk_c_domain_linknet:
			if (domain == lwk_c_domain_linknet) {
			    	SvnLbWindowInsertObject(private,
				    selected_entry, object, domain,
				    _BeforeEntry, _True);
				inserted = _True;
			}
			break;
			
		    case lwk_c_domain_path:
			if (domain == lwk_c_domain_path) {
			    	SvnLbWindowInsertObject(private,
				    selected_entry, object, domain,
				    _BeforeEntry, _True);
				inserted = _True;
			    }
			break;
		}
	    }
	    else
		if (selected_entry->child_level == 3) 
		
		switch ((lwk_domain)type) {
		
		    case lwk_c_domain_comp_linknet:
		    case lwk_c_domain_linknet:
			if (domain == lwk_c_domain_comp_linknet ||
			    domain == lwk_c_domain_linknet) {
			    SvnLbWindowInsertObject(private,
				    selected_entry, object, domain, _BeforeInto,
				    _True);
			    inserted = _True;
			}
			break;
			
		    case lwk_c_domain_comp_path:
		    case lwk_c_domain_path:
			if (domain == lwk_c_domain_comp_path ||
			    domain == lwk_c_domain_path) {
			    SvnLbWindowInsertObject(private,
				    selected_entry, object, domain, _BeforeInto,
				    _True);
			    inserted = _True;
			}
			break;
		}
	}
    }
    
    /*
    ** If not inserted, insert at the end of the corresponding list.
    */
    
    if (!inserted) 
	SvnLbWindowInsertObject(private, (_SvnData) 0, object, domain,
	_EndOfList, existing);

    /*
    ** Set the linkbase to modified.
    */
    
    SvnLbWindowSetLbModified(private);
        
    return;
    }

static _Void  SvnLbWindowGetListPointers(private, domain, begin_list, end_list)
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
	    *begin_list =
		&((_LbWindowPrivate)(private->specific))->comp_net_list;
	    *end_list =
		&((_LbWindowPrivate)(private->specific))->end_comp_net_list;
	    break;
	    
	case lwk_c_domain_linknet:
	    *begin_list =
		&((_LbWindowPrivate)(private->specific))->net_list;
	    *end_list =
		&((_LbWindowPrivate)(private->specific))->end_net_list;
	    break;
	    
	case lwk_c_domain_comp_path:
	    *begin_list =
		&((_LbWindowPrivate)(private->specific))->comp_path_list;
	    *end_list =
		&((_LbWindowPrivate)(private->specific))->end_comp_path_list;
	    break;
	    
	case lwk_c_domain_path:
	    *begin_list =
		&((_LbWindowPrivate)(private->specific))->path_list;
	    *end_list =
		&((_LbWindowPrivate)(private->specific))->end_path_list;
	    break;
    }
    
    return;
    }
    

static _Void  SvnLbWindowInsertObject(private, selected_data, object, domain, insert_entry, existing)
_WindowPrivate private;

    _SvnData selected_data;
 lwk_object object;
 lwk_domain domain;
 _InsertEntry
    insert_entry;
 _Boolean existing;

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
    _SvnData	    *begin_list,
		    *end_list;
    lwk_object	    new_object,
		    copied_object;
    lwk_status	    status;
    lwk_object	    parent_object;
    _Integer	    entry_number,
		    type;
    _Status	    message_status[1];
		        
    /*
    ** Get the parent entry.
    */
    switch (insert_entry) {
	case _EndOfList:
	    parent = private->svn_entries;
	    break;
	case _IntoList:
	    parent = selected_data;
	    break;
	case _BeforeEntry:
	case _BeforeInto:
	    parent = selected_data->parent;
	    break;
    }	    

    switch (insert_entry) {
	case _IntoList:
	case _BeforeInto:
	    /*
	    ** Get the parent his object.
	    */
	    
	    _GetValue(parent->object, _P_HisObject, hs_c_domain_lwk_object,
	       &parent_object);
       
	    /*
	    ** Compare the parent object to the one to insert.
	    */
	    if (parent_object == object) 
		_Raise(no_self_insert);

            /*
	    ** If the parent exists and has the same object domain as the object
	    ** to be inserted (composite), avoid to create recursive list.
	    */
	    if (parent != (_SvnData) 0) {
		
		_GetValue(parent->object, _P_HisType, hs_c_domain_integer,
		    &type);

		if ((lwk_domain) type == domain) 
		    if(SearchIfParentInComposite(parent_object, object,
			domain))
		    _Raise(recursive_list);
	    }
	    break;
	    
	case _EndOfList:
	case _BeforeEntry:
	    /*
	    ** Get the beginning and end of list pointers corresponding
	    ** to the object domain.
	    */
	    SvnLbWindowGetListPointers(private, domain, &begin_list, &end_list);

	    break;
    }
    
    /*
    **  Create a new svn_data entry
    */
    
    svn_data = EnvSvnWindowCreateSvnData(private, parent,
			parent->child_level + 1);
    
    /*
    ** If it's a first level object, copy the complete object from its linkbase.
    */

    copied_object = (lwk_object) lwk_c_null_object;

    if ((insert_entry == _EndOfList && (existing)) ||
	(insert_entry == _BeforeEntry)) {
	status = lwk_copy_aggregate(object, &new_object);
	if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(copy_failed);
	}
	copied_object = object;
	object = new_object;
    }

    /*
    ** Create a LinkWorks Manager object.
    */
    
    svn_data->object = (_HsObject) _CreateHsObject(_TypeHsObject, domain,
	object);

    svn_data->retrievable = _Retrievable;

    /*
    ** Insert the entry in the svn data list.
    */
    switch(insert_entry) {
	case _EndOfList:
	    /*
	    ** If the list isn't empty, end of list is the previous one.
	    */
	    if (*end_list != (_SvnData) 0) {
		/*
		** If it's a composite object, check if the previous one isn't
		** expanded.
		*/
		if (domain == lwk_c_domain_comp_linknet ||
		    domain == lwk_c_domain_comp_path) {
		    entry_number = DXmSvnGetEntryNumber(private->svn,
			(XtPointer) *end_list);
		    if (EnvSvnWindowIsExpandable(private, *end_list,
			entry_number, _LbSvnNameComponent) == _CollapseEntry) {
			previous_entry = ((_SvnData)(*end_list))->children;
			while (previous_entry->siblings != (_SvnData) 0)
			    previous_entry = previous_entry->siblings;
		    }
		    else
			previous_entry = *end_list;
		}
		else
		    previous_entry = *end_list;
		/*
		** Insert the svn_data at the end of the list.
		*/
		EnvSvnWindowInsertAfter(end_list, svn_data);
		
	    }
	    /*
	    ** The list is empty, fix the pointers and get the previous entry.
	    */
	    else {
		previous_entry = SvnLbWindowGetPreviousEntry(private, domain);
		*begin_list = svn_data;
		*end_list = svn_data;
		svn_data->siblings = SvnLbWindowGetNextEntry(private, domain);

		/*
		** Insert the entry after the previous entry.
		*/
		if (previous_entry != private->svn_entries) 
		    previous_entry->siblings = svn_data;
		else 
		    private->svn_entries->children = svn_data;

		/*
		** Increment parent's child count.
		*/
		parent->child_cnt++;

		/*
		** Check if the previous entry isn't expanded (previous list).
		*/
		if (previous_entry != private->svn_entries) {
		    entry_number = DXmSvnGetEntryNumber(private->svn,
			(XtPointer) previous_entry);
		    if (EnvSvnWindowIsExpandable(private, previous_entry,
			entry_number, _LbSvnNameComponent) == _CollapseEntry) {
			previous_entry = ((_SvnData)(previous_entry))->children;
			while (previous_entry->siblings != (_SvnData) 0)
			    previous_entry = previous_entry->siblings;
		    }
		}
	    }
    
	    break;

	case _IntoList:
	    /*
	    ** Insert into the composite object.
	    */
	    
	    previous_entry = EnvSvnWindowInsertInto(selected_data, svn_data);
	    break;

	case _BeforeInto:
	    previous_entry =
		EnvSvnWindowInsertBeforeInto(selected_data, svn_data);

	    break;

	case _BeforeEntry:
	    previous_entry = EnvSvnWindowInsertBefore(begin_list,
		    selected_data, svn_data);

	    break;
    }

    /*
    ** If this new entry is at the second level, check if this object already
    ** appears at the second level. We tell the user to change its name to avoid
    ** an eventually future confusion, because these two objects have the same
    ** name but are different.
    */

    if ((svn_data->child_level == 2)
	&& (copied_object != (lwk_object) lwk_c_null_object))
	if (SvnLbWindowObjectInSegment(private, copied_object)) {
	    message_status[0] = _StatusCode(obj_name_to_be_changed);
	    _DisplayMessage(private->window, message_status, 1);
	}

    /*
    ** Insert into the map table.
    */
    
    _MapTableAddEntry(&private->map_table, object, svn_data);

    /*
    ** Set the modified state.
    */
    switch (insert_entry) {
	case _EndOfList:
	case _BeforeEntry:
	    /*
	    ** Set the object to modified.
	    */
	    _SetHsObjState(svn_data->object, _StateModified, _StateSet);

	    /*
	    ** If it's a composite object, retrieve its children if any.
	    */
	    switch (domain) {
		case lwk_c_domain_comp_linknet :
		case lwk_c_domain_comp_path :
			EnvSvnWindowRetrieveEntry(svn_data,
			    SvnLbWindowListComposite);
			break;
	    }
	    
	    break;

	case _IntoList:
	case _BeforeInto:
	    /*
	    ** Set the object to modified. ** Not really needed because the
	    ** parent composite object will be updated
	    */
	/*  _SetHsObjState(svn_data->object, _StateModified, _StateSet); */
	    
	    /*
	    ** Update the composite object to modified
	    */
	    EnvSvnWindowUpdateComposite(parent);
	    _SetHsObjState(parent->object, _StateModified, _StateSet);

	    break;
    }

    /*
    ** Display the entry.
    */
    
    EnvSvnWindowDisplayEntry(private, previous_entry, svn_data,
	SvnLbWindowSetEntry);
        
    return;
    }


static _Boolean  SearchIfParentInComposite(parent, object, domain)
lwk_object parent;
 lwk_object object;

    lwk_domain domain;

/*
**++
**  Functional Description:
**	Search if parent is contained in the children of object. If one
**	composite child is not retrievable, we cannot detect a potential
**	recursivity.
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
    _IterateDataComp    iterate_data;
    lwk_status		status;
    lwk_termination	termination;

    /*
    ** Allocate an iterate data structure.
    */
    iterate_data = (_IterateDataComp)
	    _AllocateMem(sizeof(_IterateDataCompInstance));
    _ClearMem(iterate_data, sizeof(_IterateDataCompInstance));

    iterate_data->parent = parent;
    iterate_data->domain = domain;
    iterate_data->retrievable = _True;
    iterate_data->found = _False;
	    
    /*
    ** Iterate over the object desc contained in the object
    */
    status = lwk_iterate(object, lwk_c_domain_object_desc, iterate_data,
	ObjectDescInComposite, &termination);

    if (status != lwk_s_success) {
	_FreeMem(iterate_data);
	_SaveHisStatus(status);
	_Raise(iterate_failed);/*iterate failed*/
    }

    if (iterate_data->found) {
	_FreeMem(iterate_data);
	return (_True);
    }
    else {
	if (iterate_data->retrievable) {
	    _FreeMem(iterate_data);
	    return (_False);
	}
	else {
	    _FreeMem(iterate_data);
	    _Raise(cannot_test_recursive);
	}
    }
    
    }


static lwk_termination  ObjectDescInComposite(iterate_data, object, domain, object_desc)
_IterateDataComp iterate_data;

    lwk_persistent object;
 lwk_domain domain;

    lwk_object_descriptor *object_desc;

/*
**++
**  Functional Description:
**	Get the object and only treat composite.Compare it to the parent
**	stored in the iterate_data.
**	If it's the same, stop the iteration. Else, continue the iteration
**	recursively if the object is also a composite and can be retrieved.
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
    lwk_persistent  persistent;
    lwk_integer	    object_domain;
    lwk_termination termination;

    /*
    ** We need to know the real domain of the object descriptor because we only
    ** treat composite object.
    */

    status = lwk_get_value(*object_desc, lwk_c_p_object_domain,
		lwk_c_domain_integer, &object_domain);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(get_value_failed);
    }

    if ((lwk_domain) object_domain == iterate_data->domain) {
    
	status = lwk_retrieve(*object_desc, &persistent);

	/*
	** If the retrieve failed on a composite, we cannot allow the insertion.
	*/
	if (status != lwk_s_success) {
	    iterate_data->retrievable = _False;
	    return (lwk_termination) 2;
	}

	else {

            /*
	    ** compare the composite object to the parent
	    */
	    if (persistent == iterate_data->parent) {

                /*
		** We found it so stop the iteration.
		*/
		iterate_data->found = _True;
		return (lwk_termination) 2;
	    }

	    else {

                /*
		** Iterate again on the children of this composite.
		*/
		status = lwk_iterate(persistent, lwk_c_domain_object_desc,
		    iterate_data, ObjectDescInComposite, &termination);

		if (status != lwk_s_success) {
		    _SaveHisStatus(status);
		    _Raise(iterate_failed);/*iterate failed*/
		}
		else
		    if (termination == (lwk_termination) 2)
			return (lwk_termination) 2;
	    }
	    
	}
    }
    
    return (lwk_termination) 0;    
    }
    

static _Boolean  SvnLbWindowObjectInSegment(private, object)
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
	    if (entries[i]->child_level == 2)
		found = _True;
	    else
		i++;

    return(found);
    }


_Void  EnvSvnLbWindowUpdate(private, hs_object, update)
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
    _SvnData	    *entries;
    _Integer	    count;
    _Integer	    i;
    _Integer	    entry_num;
    _Status	    stat[2];
    _HsObject	    hs_lb;
    _String	    lb_title;
    _CString	    title;
    _CString	    linkbase_name;
    _Boolean	    ignored;
    _EnvWindow	    env_window;
    lwk_string	    linkbase_id;
    lwk_domain      domain;
    _Integer	    type;
    lwk_persistent  his_obj;

    DXmSvnDisableDisplay(private->svn);
    
    _SetCursor(private->window, _WaitCursor);

    _StartExceptionBlock

    /*
    **  Get info  from the object
    */
        
    _GetValue(hs_object, _P_HisObject, hs_c_domain_lwk_object, &his_obj);

    _GetValue(hs_object, _P_HisType, hs_c_domain_integer, &type);

    domain = (lwk_domain) type;

    /*
    **  Mark the linkbase as modified
    */
    
    _GetValue(private->window, _P_Linkbase, hs_c_domain_hsobject, &hs_lb);

    _SetHsObjState(hs_lb, _StateModified, _StateSet);

    env_window = (_EnvWindow) _GetEnvWindow(private->window);

    if (domain == lwk_c_domain_linkbase) {

	/*
	**  Update the title in the window
	*/

	_GetCSProperty(hs_object, lwk_c_p_name, &linkbase_name);

	EnvDWGetCStringLiteral(_DwtLbWindowTitle, &lb_title);

	title = EnvDWConcatCString(lb_title, linkbase_name);
	
	EnvDWSetTitle(private->shell, title);

        /*
	** Reflect the linkbase name change on non-retrievable object
	*/
	_GetProperty(hs_object, lwk_c_domain_string, lwk_c_p_identifier,
	    &linkbase_id);

	UpdateNonRetrievableObjects(private, private->svn_entries, linkbase_id,
	    linkbase_name);	

   	/*
	**  Reflect the linkbase name change in the environment window
	*/

	_Update(env_window, hs_object, update);
	
	/*
	** Clean up.
	*/
	
	_DeleteCString(&title);
	_DeleteCString(&linkbase_name);
	_DeleteString(&lb_title);
    }
    else {

	/*
	**  Get all the entries for this object if it still exists
	*/

	if (_MapTableGetEntry(private->map_table, his_obj, &entries, &count)) {

	    if (update == _DeletedObject)
		EnvSvnWindowSetEntriesNonRetr(private, entries, count);

	    /*
	    **  Process each of them
	    */

	    for (i = 0; i < count; i++) {

		/*
		**  Update the Svn entry only if it is visible
		*/

		entry_num = (_Integer) DXmSvnGetEntryNumber(private->svn,
		    (XtPointer) entries[i]);

		if (entry_num > 0)

		    SvnLbWindowSetEntry(private, entries[i]);
	    }

	    _SetHsObjState(hs_object, _StateModified, _StateSet);
	}
	else {

	    /*
	    **  Since Show Prop is modeless the object could have been deleted,
	    **	so don't do anything, unless it is really a new object
	    */

	    if (update == _NewObject)
	    
		/*
		**  It's a new object insert it in the display
		*/
		
		EnvSvnLbWindowInsertEntry(private, his_obj, domain, _False);
	}
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
    


static _Void  UpdateNonRetrievableObjects(private, entry, linkbase_id, linkbase_name)
_WindowPrivate private;

    _SvnData entry;
 lwk_string linkbase_id;
 _CString linkbase_name;
	
/*
**++
**  Functional Description:
**	Update the linkbase name of a non retrievable object whose linkbase
**	identifier is the same as linkbase_id
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
    _CString    cs_name;
    _String     identifier;
    int         entry_number;

    /*
    ** Run down the list of SVN entries to found the non-retrievable entries
    */

    if (entry->children != (_SvnData) 0)
	UpdateNonRetrievableObjects(private, entry->children, linkbase_id,
	    linkbase_name);

    if (entry->siblings != (_SvnData) 0)
	UpdateNonRetrievableObjects(private, entry->siblings, linkbase_id,
	    linkbase_name);

    if (entry->retrievable == _NotRetrievable) {

	if (entry->object != _NullObject) {

	    _GetLinkbase(entry->object, &cs_name, &identifier);

	    if (_CompareString(linkbase_id, identifier) == 0) {

                /*
		** Get the entry number. Update the SVN line only if it's
		** visible
		*/
		entry_number = DXmSvnGetEntryNumber(private->svn,
		    (XtPointer) entry);
    
		if (entry_number != 0) {
		    /*
		    ** Invalidate the entry so that it gets redrawn.
		    */
		    DXmSvnInvalidateEntry(private->svn, entry_number);

                    /*
		    ** Set the entry
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
		** If the entry was into a composite, mark its parent modified
		** in order to save it
		*/
		if (((_SvnData)(entry->parent))->child_level ==
		    _LbWindowPrimObjLevel) {

		    _SetHsObjState(((_SvnData)(entry->parent))->object,
			_StateModified, _StateSet);

		    EnvSvnWindowUpdateComposite(entry->parent);
		}

		/*
		** Set the linkbase to modified.
		*/
		
		SvnLbWindowSetLbModified(private);

	    }
	}
    }
	
    return;
    }
        

static _SvnData  SvnLbWindowGetPreviousEntry(private, domain)
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
		    *begin_list,
		    *end_list;
    
    switch (domain) {
	case lwk_c_domain_comp_linknet:
	    previous_entry = private->svn_entries;
	    break;
	case lwk_c_domain_linknet:	
	    SvnLbWindowGetListPointers(private,
		lwk_c_domain_comp_linknet, &begin_list, &end_list);
	    if (*end_list != (_SvnData) 0)
		previous_entry = *end_list;
	    else
		previous_entry = private->svn_entries;
	    break;
	case lwk_c_domain_comp_path:	
	    SvnLbWindowGetListPointers(private,
		lwk_c_domain_linknet, &begin_list, &end_list);
	    if (*end_list != (_SvnData) 0)
		previous_entry = *end_list;
	    else 
		previous_entry = SvnLbWindowGetPreviousEntry(private,
			lwk_c_domain_linknet);
	    break;
	case lwk_c_domain_path:	
	    SvnLbWindowGetListPointers(private,
		lwk_c_domain_comp_path, &begin_list, &end_list);
	    if (*end_list != (_SvnData) 0)
		previous_entry = *end_list;
	    else
		previous_entry = SvnLbWindowGetPreviousEntry(private,
			lwk_c_domain_comp_path);
	    break;
    }	        
    return(previous_entry);
    }

static _SvnData  SvnLbWindowGetNextEntry(private, domain)
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
    _SvnData	    next_entry,
		    *begin_list,
		    *end_list;
    
    switch (domain) {
	case lwk_c_domain_comp_linknet:
	    SvnLbWindowGetListPointers(private,
		lwk_c_domain_linknet, &begin_list, &end_list);
	    if (*begin_list != (_SvnData) 0)
		next_entry = *begin_list;
	    else
		next_entry =
		    SvnLbWindowGetNextEntry(private, lwk_c_domain_comp_path);
	    break;
	case lwk_c_domain_linknet:	
	    SvnLbWindowGetListPointers(private,
		lwk_c_domain_comp_path, &begin_list, &end_list);
	    if (*begin_list != (_SvnData) 0)
		next_entry = *begin_list;
	    else
		next_entry =
		    SvnLbWindowGetNextEntry(private, lwk_c_domain_path);
	    break;
	case lwk_c_domain_comp_path:	
	    SvnLbWindowGetListPointers(private,
		lwk_c_domain_path, &begin_list, &end_list);
		next_entry = *begin_list;
	    break;
	case lwk_c_domain_path:	
		next_entry = (_SvnData) 0;
	    break;
    }	        
    return(next_entry);
    }

static _Void  SvnLbWindowSetLbModified(private)
_WindowPrivate private;

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
    _HsObject	    hs_linkbase;

    /*
    ** Get the linkbase object displayed in the window
    */
    
    _GetValue(private->window, _P_Linkbase, hs_c_domain_hsobject,
	&hs_linkbase);
    
    /*
    ** Set the linkbase to modified.
    */
    _SetHsObjState(hs_linkbase, _StateModified, _StateSet);

    return;
    }
    

_Void  EnvSvnLbWindowClear(private)
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

    /*
    **  Clear the Svn display list
    */
    
    EnvSvnWindowClearDisplay(private);

    /*
    **  Free the internal display list
    */
    
    EnvSvnWindowFreeSvnSiblData(private->svn_entries);

    /*
    **  Initialize the Svn pointers
    */
    
    private->svn_entries = (_SvnData) 0;
    private->deleted_entries = (_SvnData) 0;
    
    ((_LbWindowPrivate) (private->specific))->comp_net_list = (_SvnData) 0;
    ((_LbWindowPrivate) (private->specific))->comp_path_list = (_SvnData) 0;
    ((_LbWindowPrivate) (private->specific))->net_list = (_SvnData) 0;
    ((_LbWindowPrivate) (private->specific))->path_list = (_SvnData) 0;
    ((_LbWindowPrivate) (private->specific))->end_comp_net_list = (_SvnData) 0;
    ((_LbWindowPrivate) (private->specific))->end_comp_path_list = (_SvnData) 0;
    ((_LbWindowPrivate) (private->specific))->end_net_list = (_SvnData) 0;
    ((_LbWindowPrivate) (private->specific))->end_path_list = (_SvnData) 0;
    
    return;
    }
    

static _Void  SvnLbWindowInsertQuickCopy(private, object, domain, x, y)
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
    SvnLbWindowInsertEntry(private, object, domain, selected_entry, _True);
    
    return;
    }

static _Void  SvnLbWindowQuickCopyOrMove(widget, private, x, y, timestamp, move)
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
	    SvnLbWindowInsertQuickCopy(private, hisobject, domain, x, y);
	}    
    }

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
    

_Void  EnvSvnLbWindowAddActions(private)
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


_Void  EnvSvnLbWindowEntryTransfer(w, private, cb)
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
    switch (cb->transfer_mode) {

	/* Primary Copy - aka CopyTo    */
	case DXmSvnKtransferUnknown:
    	case DXmSvnKtransferCopy:
	    {
	    SvnLbWindowQuickCopyOrMove(w, private, cb->x, cb->y,
		cb->time, _False);
	    break;
	    }

	/* Primary Paste - aka MoveTo   */
    	case DXmSvnKtransferMove: {
	    SvnLbWindowQuickCopyOrMove(w, private, cb->x, cb->y,
		cb->time, _True);
	    break;
	}

	/* Neither Primary Copy or Primary Paste */
    	default: {
	    break;
	}
    }
}
    

_Void  EnvSvnLbWindowSelectionsDrag(w, private, cb)
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
