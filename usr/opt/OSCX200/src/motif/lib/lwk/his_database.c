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
**	Database support routines
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
**
**--
*/


/*
**  Include Files
*/

#ifdef MSDOS
#include "include.h"
#include "repostry.h"
#include "database.h"
#include "grdfile.h"
#else /* !MSDOS */
#include "his_include.h"
#include "lwk_linkbase.h"
#include "his_database.h"
#include "his_grid_file.h"
#endif

/*
**  Macro Definitions
*/

/*
**  Type Definitions
*/

/*
**  External Routine Declarations
*/

/*
**  Forward Routine Declarations
*/

_DeclareFunction(static _Integer GenerateIdentifier, (_Linkbase linkbase));
_DeclareFunction(static void StoreSurrogate,
    (_Linkbase linkbase, _Persistent persistent, _Integer uid,
	_VaryingString encoding, _Boolean new));
_DeclareFunction(static void StoreLink,
    (_Linkbase linkbase, _Persistent persistent, _Integer uid,
	_VaryingString encoding, _Boolean new));
_DeclareFunction(static void StoreStep,
    (_Linkbase linkbase, _Persistent persistent, _Integer uid,
	_VaryingString encoding, _Boolean new));
_DeclareFunction(static void StoreNetwork,
    (_Linkbase linkbase, _Persistent persistent, _Integer uid,
	_VaryingString encoding, _Boolean new));
_DeclareFunction(static void StorePath,
    (_Linkbase linkbase, _Persistent persistent, _Integer uid,
	_VaryingString encoding, _Boolean new));
_DeclareFunction(static void StoreCompositeNet,
    (_Linkbase linkbase, _Persistent persistent, _Integer uid,
	_VaryingString encoding, _Boolean new));
_DeclareFunction(static void StoreCompositePath,
    (_Linkbase linkbase, _Persistent persistent, _Integer uid,
	_VaryingString encoding, _Boolean new));
_DeclareFunction(static void DeleteFile, (_Linkbase linkbase));
_DeclareFunction(static _Persistent RetrieveSurrogate,
    (_Linkbase linkbase, _Integer uid, _Integer container_id));
_DeclareFunction(static _Persistent RetrieveLink,
    (_Linkbase linkbase, _Integer uid, _Integer network_id));
_DeclareFunction(static _Persistent RetrieveStep,
    (_Linkbase linkbase, _Integer uid, _Integer path_id));
_DeclareFunction(static _Persistent RetrieveNetwork,
    (_Linkbase linkbase, _Integer uid));
_DeclareFunction(static _Persistent RetrievePath,
    (_Linkbase linkbase, _Integer uid));
_DeclareFunction(static _Persistent RetrieveCompositeNet,
    (_Linkbase linkbase, _Integer uid));
_DeclareFunction(static _Persistent RetrieveCompositePath,
    (_Linkbase linkbase, _Integer uid));
_DeclareFunction(static _Boolean SetSelfRelativeDescriptors,
    (_Persistent persistent, _Domain domain, _Linkbase linkbase,
	_Boolean store));
_DeclareFunction(static _Integer GetInterLinkIds,
    (_Surrogate surrogate, _Integer **interconnect_ids));
_DeclareFunction(static _Integer InterLinkSetToIds,
    (_List interlinks, _Integer **interconnect_ids, _Boolean set_of_ids));
_DeclareFunction(static _List InterLinkIdsToSet,
    (_Integer count, _IntegerPtr ids));
_DeclareFunction(static void AddToCache,
    (_Linkbase linkbase, _Integer uid, _Persistent persistent));
_DeclareFunction(static _Boolean SearchCache,
    (_Linkbase linkbase, _Integer uid, _Persistent *persistent));
_DeclareFunction(static void RemoveFromCache,
    (_Linkbase linkbase, _Integer uid, _Persistent persistent));

/*
**  Static Data Definitions
*/

/*
**  Global Data Definitions
*/

/*
**  External Data Declarations
*/


void  LwkLb()
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


void  LwkLbInitialize()
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
    **  Ask the File Module to Initialize itself
    */

    LwkGridInitialize();

    return;
    }


_String  LwkLbExpandIdentifier(identifier)
_String identifier;

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
    _FileStatus status;
    _String expanded_id;

    /*
    **  Expand the Linkbase Identifier
    */

    status = LwkGridExpandIdentifier(identifier, &expanded_id);

    if (!_FileSuccess(status))
	_Raise(no_such_linkbase);

    return expanded_id;
    }


void  LwkLbOpen(linkbase, create)
_Linkbase linkbase;
 _Boolean create;

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
    _FileStatus status;

    /*
    **  If the Linkbase is already Open, just increment the use count and
    **	return.
    */

    if (_Open_of(linkbase)) {
	_UseCount_of(linkbase)++;
	return;
    }

    /*
    **  Open the Database to see if it exists.
    */

    status = LwkGridOpen(_Identifier_of(linkbase), create,
	&_File_of(linkbase), &_Name_of(linkbase));

    if (!_FileSuccess(status))
	_Raise(db_read_error);

    /*
    **  Default the Name if necessary to be the same as the Identifier
    */

    if (_Name_of(linkbase) == (_DDIFString) _NullObject)
	_Name_of(linkbase) = _StringToDDIFString(_Identifier_of(linkbase));

    /*
    **  Mark the Linkbase Open and increment it's use count.
    */

    _Open_of(linkbase) = _True;

    _UseCount_of(linkbase)++;

    return;
    }


void  LwkLbClose(linkbase)
_Linkbase linkbase;

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
    _FileStatus status;

    /*
    **	If Linkbase was open, reduce the use count.  If use count goes less
    **	than zero, close the actual database.
    **
    **	Note: This may be a bit strange, but it is intended as a performance
    **	improvement.  It takes an extra close on the Linkbase to actually
    **	close the File.  Since a Delete of a Linkbase does an implicit Close,
    **	this will usually causes the right things to happen!
    */

    if (_Open_of(linkbase)) {
	_UseCount_of(linkbase)--;

	if (_UseCount_of(linkbase) < 0) {
	    status = LwkGridClose((_GridFile) _File_of(linkbase));

	    if (!_FileSuccess(status))
		_Raise(db_commit_error);

	    _Open_of(linkbase) = _False;
	    _UseCount_of(linkbase) = 0;
	}
    }

    return;
    }


void  LwkLbSetIdentifier(linkbase)
_Linkbase linkbase;

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
    _FileStatus status;

    /*
    **  Ask the File module to rename the file
    */

    status = LwkGridRenameFile((_GridFile) _File_of(linkbase),
	_Identifier_of(linkbase));

    if (!_FileSuccess(status))
	_Raise(db_write_error);

    return;
    }


void  LwkLbSetName(linkbase)
_Linkbase linkbase;

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
    _FileStatus status;

    /*
    **  If the Linkbase is not Open, that is an error.
    */

    if (!_Open_of(linkbase))
	_Raise(db_not_open);

    /*
    **  Ask the File module to set the Name of the file
    */

    status = LwkGridSetName((_GridFile) _File_of(linkbase),
	_Name_of(linkbase));

    if (!_FileSuccess(status))
	_Raise(db_write_error);

    return;
    }

void  LwkLbAssignIdentifier(linkbase, persistent)
_Linkbase linkbase;
 _Persistent persistent;

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
    _Integer uid;

    /*
    **  If the Linkbase is not Open, that is an error.
    */

    if (!_Open_of(linkbase))
	_Raise(db_not_open);

    /*	
    **  Get the object's unique identifier -- if none has been assigned,
    **	generate a new one.
    */

    _GetValue(persistent, _P_Identifier, lwk_c_domain_integer, &uid);

    if (uid == 0) {
	uid = GenerateIdentifier(linkbase);

	_SetValue(persistent, _P_Identifier, lwk_c_domain_integer, &uid,
	    lwk_c_set_property);
    }

    return;
    }


void  LwkLbStartTransaction(linkbase, transaction)
_Linkbase linkbase;
 _Transaction transaction;

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
    _FileStatus status;

    /*
    **  If the Linkbase is not Open, that is an error.
    */

    if (!_Open_of(linkbase))
	_Raise(db_not_open);

    /*
    **  Let the File module do the grungy work.
    */

    switch (transaction) {
	case lwk_c_transact_read :
	    status = LwkGridSetTransaction((_GridFile) _File_of(linkbase),
		lwk_c_transact_read);
	    break;

	case lwk_c_transact_read_write :
	    status = LwkGridSetTransaction((_GridFile) _File_of(linkbase),
		lwk_c_transact_read_write);
	    break;

	default :
	    _Raise(inv_argument);
	    break;
    }


    if (!_FileSuccess(status))
	_Raise(db_write_error);

    return;
    }


void  LwkLbCommit(linkbase)
_Linkbase linkbase;

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
    _FileStatus status;

    /*
    **  If the Linkbase is not Open, that is an error.
    */

    if (!_Open_of(linkbase))
	_Raise(db_not_open);

    /*
    **  Let the File module do the grungy work.
    */

    status = LwkGridSetTransaction((_GridFile) _File_of(linkbase),
	lwk_c_transact_commit);

    if (!_FileSuccess(status))
	_Raise(db_commit_error);

    return;
    }


void  LwkLbRollback(linkbase)
_Linkbase linkbase;

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
    _FileStatus status;

    /*
    **  If the Linkbase is not Open, that is an error.
    */

    if (!_Open_of(linkbase))
	_Raise(db_not_open);

    /*
    **  Let the File module do the grungy work.
    */

    status = LwkGridSetTransaction((_GridFile) _File_of(linkbase),
	lwk_c_transact_rollback);

    if (!_FileSuccess(status))
	_Raise(db_write_error);

    return;
    }


void  LwkLbStore(linkbase, domain, persistent)
_Linkbase linkbase;
 _Domain domain;
 _Persistent persistent;

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
    _Boolean new;
    _Integer uid;
    _Boolean volatile modified;
    _Linkbase old_linkbase;
    _VaryingString volatile encoding;

    /*
    **  Make sure that the object is not already stored in some other
    **	Linkbase -- this is an error.
    */

    _GetValue(persistent, _P_Linkbase, lwk_c_domain_linkbase,
	&old_linkbase);

    if (old_linkbase != (_Linkbase) _NullObject
	&& old_linkbase != linkbase)
	    _Raise(stored_elsewhere);

    /*
    **  If the Linkbase is not Open, that is an error.
    */

    if (!_Open_of(linkbase))
	_Raise(db_not_open);

    /*
    **	If the Object had not been modified, don't bother to Store it.
    */

    if (!(_Boolean) _SetState(persistent, _StateModified, _StateGet))
	return;

    /*
    **  Initialize temporary storage
    */

    modified = _False;
    encoding = (_VaryingString) _NullObject;

    _StartExceptionBlock

    /*
    ** Perform any pre-processing
    */
    
#ifndef MSDOS
    switch (domain) {
	case lwk_c_domain_comp_linknet :
	case lwk_c_domain_comp_path :
            /*
	    ** Set any ObjectDescriptors which point to the same Linkbase to
	    ** be self-relative (i.e., clear their Linkbase Identifier).  We
	    ** reset them to the appropriate value on a Retrieve.
            */
	    
	    modified = SetSelfRelativeDescriptors(persistent, domain,
		linkbase, _True);

	    break;
    }
#endif /* !MSDOS */

    /*
    **  Get the object's encoding
    */

    _Export(persistent, _False, (_AnyPtr) &encoding);

    /*
    ** Perform any post-processing
    */
    
#ifndef MSDOS
    if (modified) {
	switch (domain) {
	    case lwk_c_domain_comp_linknet :
	    case lwk_c_domain_comp_path :
		/*
		** Reset any self-relative Object Descriptors
		*/

		SetSelfRelativeDescriptors(persistent, domain, linkbase,
		    _False);

		break;
	}

	modified = _False;
    }
#endif /* !MSDOS */

    /*
    **  Get the object's unique identifier -- if none has been assigned,
    **	generate a new one.
    */

    _GetValue(persistent, _P_Identifier, lwk_c_domain_integer, &uid);

    if (uid == 0) {
	uid = GenerateIdentifier(linkbase);

	_SetValue(persistent, _P_Identifier, lwk_c_domain_integer, &uid,
	    lwk_c_set_property);
    }

    /*
    **  Make a note whether this is a new Object or an update of an existing
    **	Object.
    */

    if (linkbase == old_linkbase)
	new = _False;
    else
	new = _True;

    /*
    **	Depending on the domain of the object, store the object in the
    **	linkbase.
    */

    switch (domain) {
	case lwk_c_domain_surrogate :
	    StoreSurrogate(linkbase, persistent, uid, encoding, new);
	    break;

	case lwk_c_domain_link :
	    StoreLink(linkbase, persistent, uid, encoding, new);
	    break;

	case lwk_c_domain_linknet :
	    StoreNetwork(linkbase, persistent, uid, encoding, new);
	    break;

#ifndef MSDOS

	case lwk_c_domain_path :
	    StorePath(linkbase, persistent, uid, encoding, new);
	    break;

	case lwk_c_domain_step :
	    StoreStep(linkbase, persistent, uid, encoding, new);
	    break;

	case lwk_c_domain_comp_linknet :
	    StoreCompositeNet(linkbase, persistent, uid, encoding, new);
	    break;

	case lwk_c_domain_comp_path :
	    StoreCompositePath(linkbase, persistent, uid, encoding, new);
	    break;

#endif /* !MSDOS */

	default :
	    _Raise(inv_domain);
	    break;
    }

    /*
    **  Add the object to the Linkbase cache.
    */

    AddToCache(linkbase, uid, persistent);

    /*
    **  Clear the Modified state
    */

    _SetState(persistent, _StateModified, _StateClear);

    /*
    **  Clean up
    */

    _DeleteEncoding(&encoding);

    /*
    **	If any exceptions are raised, clean up, abort the transaction, then
    **	reraise the exception.
    */
    
    _Exceptions
	_WhenOthers
#ifndef MSDOS
	    if (modified) {
		switch (domain) {
		    case lwk_c_domain_comp_linknet :
		    case lwk_c_domain_comp_path :
			SetSelfRelativeDescriptors(persistent, domain,
			    linkbase, _False);
			break;
		}
	    }
#endif /* !MSDOS */

	    _DeleteEncoding(&encoding);
	    _Reraise;
    _EndExceptionBlock

    return;
    }


_Persistent  LwkLbRetrieve(linkbase, domain, uid, container_id)
_Linkbase linkbase;
 _Domain domain;

    _Integer uid;
 _Integer container_id;

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
    _Persistent persistent;
	
    /*
    **  If the Linkbase is not Open, that is an error.
    */

    if (!_Open_of(linkbase))
	_Raise(db_not_open);

    /*
    **	Retrieve the object based on its Domain.
    */

    switch (domain) {
	case lwk_c_domain_surrogate :
	    persistent = RetrieveSurrogate(linkbase, uid, container_id);
	    break;

	case lwk_c_domain_link :
	    persistent = RetrieveLink(linkbase, uid, container_id);
	    break;

	case lwk_c_domain_linknet :
	    persistent = RetrieveNetwork(linkbase, uid);
	    break;

#ifndef MSDOS

	case lwk_c_domain_path :
	    persistent = RetrievePath(linkbase, uid);
	    break;

	case lwk_c_domain_step :
	    persistent = RetrieveStep(linkbase, uid, container_id);
	    break;

	case lwk_c_domain_comp_linknet :
	    persistent = RetrieveCompositeNet(linkbase, uid);
	    break;

	case lwk_c_domain_comp_path :
	    persistent = RetrieveCompositePath(linkbase, uid);
	    break;

#endif /* !MSDOS */

	default :
	    _Raise(inv_domain);
	    break;
    }

    return persistent;
    }


void  LwkLbDrop(domain, object)
_Domain domain;
 _Persistent object;

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
    _Integer uid;
    _FileStatus status;
    _Linkbase linkbase;
	
    /*
    **  If the Domain is Linkbase, do it then return.
    */

    if (_IsDomain(lwk_c_domain_linkbase, domain)) {
	DeleteFile((_Linkbase) object);
	return;
    }

    /*
    **	Object must be a Persistent object.  Get the object's unique identifier
    **	-- if none has been assigned, the object was never stored/retrieved, so
    **	do not try to Drop it.
    */

    _GetValue(object, _P_Identifier, lwk_c_domain_integer, &uid);

    if (uid == 0)
	return;

    /*
    **  Get the Linkbase in which the object is stored -- if none, the object
    **	is not stored anywhere so do not try to Drop it.
    */

    _GetValue(object, _P_Linkbase, lwk_c_domain_linkbase, &linkbase);

    if (linkbase == (_Linkbase) _NullObject)
	return;

    /*
    **  If the Linkbase is not Open, that is an error.
    */

    if (!_Open_of(linkbase))
	_Raise(db_not_open);

    /*
    **  Clear the object from the Linkbase cache.
    */

    RemoveFromCache(linkbase, uid, object);

    /*
    **  Call the File module to actually delete the Record
    */

    status = LwkGridDeleteObject((_GridFile) _File_of(linkbase),
	domain, uid);

    if (!_FileSuccess(status))
	_Raise(db_write_error);

    return;
    }


_Termination  LwkLbIterate(linkbase, domain, closure, routine)
_Linkbase linkbase;
 _Domain volatile domain;

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
    _Integer uid;
    _CString cstring;
    _FileStatus status;
    _Integer container_id;
    _Persistent persistent;
    _GridCursor volatile cursor;
    _Termination termination;

    /*
    **  If the Linkbase is not Open, that is an error.
    */

    if (!_Open_of(linkbase))
	_Raise(db_not_open);

    /*
    **  Initialize temporary storage
    */

    termination = (_Termination) 0;
    cursor = (_GridCursor) _NullObject;

    _StartExceptionBlock

    /*
    **  Open the appropriate Cursor.
    */

    status = LwkGridOpenDirectory((_GridFile) _File_of(linkbase), domain,
	&cursor);

    /*
    **  If there are no objects, we're done.
    */

    if (status != _FileStsNoneFound) {

	if (!_FileSuccess(status))
	    _Raise(db_read_error);

	/*
	**  Loop fetching the Objects.
	*/

	while(_True) {
	
	    /*
	    **  Ask File to fetch the appropriate object.
	    */

	    status = LwkGridFetchDirectory(cursor, &uid, &container_id);

	    /*
	    **  If the Cursor is empty, we have all of them.
	    */

	    if (status == _FileStsCursorEmpty)
		break;

	    if (!_FileSuccess(status))
		_Raise(db_read_error);

	    /*
	    **  Retrieve the Object
	    */

	    persistent = LwkLbRetrieve(linkbase, domain, uid, container_id);

	    /*
	    **  Call the application callback routine.
	    */

	    termination = (*routine)(closure, linkbase, domain, &persistent);

	    /*
	    **  Quit now if the application returned a non-zero value.
	    */

	    if (termination != (_Termination) 0)
		break;
	}

	/*
	**  Close the appropriate Cursor.
	*/

	status = LwkGridCloseCursor(cursor);

	if (!_FileSuccess(status))
	    _Raise(db_read_error);
    }

    /*
    **	If any exceptions are raised, clean up then reraise the exception.
    */
    
    _Exceptions
	_WhenOthers
	    if (cursor != (_GridCursor) _NullObject)
		LwkGridCloseCursor(cursor);

	    _Reraise;
    _EndExceptionBlock

    /*
    **  Return most recent callback return value.
    */

    return termination;
    }


void  LwkLbUnCache(linkbase, uid, persistent)
_Linkbase linkbase;
 _Integer uid;
 _Persistent persistent;

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
    **  Clear the object from the Linkbase cache.
    */

    if (linkbase != (_Linkbase) _NullObject)
	RemoveFromCache(linkbase, uid, persistent);

    return;
    }


void  LwkLbQuerySurrInContainer(linkbase, container, container_id, expression)
_Linkbase linkbase;
 _Persistent container;

    _Integer container_id;
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
    _Domain domain;
    _Integer count;
    _Integer surr_id;
    _FileStatus status;
    _GridCursor cursor;
    _Surrogate surrogate;
    _Integer has_incomming;
    _Integer has_outgoing;
    _IntegerPtr interconnect_ids;
    _Set volatile interconnect_set;
    _DynamicVString volatile encoding;

    /*
    **  If the Linkbase is not Open, that is an error.
    */

    if (!_Open_of(linkbase))
	_Raise(db_not_open);

    /*
    **  Open the appropriate Cursor.
    */

    status = LwkGridOpenQuerySurrogates((_GridFile) _File_of(linkbase),
	container_id, expression, &cursor);

    /*
    **  If there are no Surrogates in this Container, we are done.
    */

    if (status == _FileStsNoneFound)
	return;

    if (!_FileSuccess(status))
	_Raise(db_read_error);

    /*
    **  Initialize temporary storage.
    */

    interconnect_set = (_Set) _NullObject;
    surrogate = (_Surrogate) _NullObject;
    _CreateDVString(encoding);

    _StartExceptionBlock

    /*
    **  Loop fetching Surrogates.
    */

    while (_True) {
	/*
	**  Ask File to fetch the Surrogate.
	*/

	status = LwkGridFetchQuerySurrogates(cursor, &surr_id,
	    &has_incomming, &has_outgoing, &count, &interconnect_ids,
	    encoding);

	/*
	**  If the Cursor is empty, we have all the Surrogates.
	*/

	if (status == _FileStsCursorEmpty)
	    break;

	if (!_FileSuccess(status))
	    _Raise(db_read_error);

	/*
	**  If this Surrogate is already in the cache, we don't need to
	**  reconstitute it -- just clean up and return.
	*/

	if (SearchCache(linkbase, surr_id, &surrogate)) {
	    if (count > 0)
		_FreeMem(interconnect_ids);
	}
	else {
	    /*
	    **  Reconstitute the Surrogate using Import.
	    */

	    surrogate = (_Surrogate) _Import(_TypeSurrogate,
		_DVString_of(encoding));

	    /*
	    **	Add the Surrogate to the Linkbase cache.  Note: we do this
	    **	earlier than we normally would because the Surrogate needs to
	    **	"exist" before we set some of its properties (e.g., setting the
	    **	Path property will add this Surrogate to Surrogates list and
	    **	may try to retrieve its Interlinks -- which will look for
	    **	the Surrogte!).
	    */

	    AddToCache(linkbase, surr_id, surrogate);

	    /*
	    **  Set the InterLink state appropriately
	    */

	    if (has_incomming || has_outgoing) {
		_SetState(surrogate, _StateHaveInterLinks, _StateClear);

		if (has_incomming)
		    _SetState(surrogate, _StateHasInComming, _StateSet);

		if (has_outgoing)
		    _SetState(surrogate, _StateHasOutGoing, _StateSet);
	    }

	    /*
	    **	Convert any InterLink Ids into a Set (destroying the Id
	    **	array) and set them as the InterLinks of the SUrrogate.
	    */

	    if (count > 0) {
		interconnect_set = InterLinkIdsToSet(count,
		    interconnect_ids);

		_SetValue(surrogate, _P_InterLinks, lwk_c_domain_set,
		    (_AnyPtr) &interconnect_set, lwk_c_set_property);

		_Delete(&interconnect_set);
	    }

	    /*
	    **  Set the Container property of the Surrogate
	    */

	    _SetValue(surrogate, _P_Container, lwk_c_domain_persistent,
		&container, lwk_c_set_property);

	    /*
	    **  Clear the modified flag
	    */

	    _SetState(surrogate, _StateModified, _StateClear);
	}
    }

    /*
    **  Close the Cursor.
    */

    status = LwkGridCloseCursor(cursor);

    if (!_FileSuccess(status))
	_Raise(db_read_error);

    /*
    **  Clean up
    */

    _DeleteDVString(&encoding);

    /*
    **  If any exceptions are raised, clean up then reraise the exception.
    */
    
    _Exceptions
	_WhenOthers
	    LwkGridCloseCursor(cursor);

	    _Delete(&interconnect_set);
	    _DeleteDVString(&encoding);

	    _Reraise;
    _EndExceptionBlock

    return;
    }


void  LwkLbQueryConnInNetwork(linkbase, network, net_id, expression)
_Linkbase linkbase;
 _Linknet network;

    _Integer net_id;
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
    _Integer conn_id;
    _Integer source_id;
    _Integer target_id;
    _GridCursor cursor;
    _FileStatus status;
    _Link link;
    _DynamicVString volatile encoding;

    /*
    **  If the Linkbase is not Open, that is an error.
    */

    if (!_Open_of(linkbase))
	_Raise(db_not_open);

    /*
    **  Open the appropriate Cursor.
    */

    status = LwkGridOpenQueryLinks((_GridFile) _File_of(linkbase),
	net_id, expression, &cursor);

    /*
    **  If there are no Links in this Network, we are done.
    */

    if (status == _FileStsNoneFound)
	return;

    if (!_FileSuccess(status))
	_Raise(db_read_error);

    /*
    **  Initialize temporary storage.
    */

    link = (_Link) _NullObject;
    _CreateDVString(encoding);

    _StartExceptionBlock

    /*
    **  Loop fetching Links.
    */

    while (_True) {
	/*
	**  Ask File to fetch the Link.
	*/

	status = LwkGridFetchQueryLinks(cursor, &conn_id, &source_id,
	    &target_id, encoding);

	/*
	**  If the Cursor is empty, we have all the Links.
	*/

	if (status == _FileStsCursorEmpty)
	    break;

	if (!_FileSuccess(status))
	    _Raise(db_read_error);

	/*
	**  If this Link is already in the cache, no need to proceed.
	*/

	if (!SearchCache(linkbase, conn_id, &link)) {
	    /*
	    **  Reconstitute the Link using Import.
	    */

	    link = (_Link) _Import(_TypeLink,
		_DVString_of(encoding));

	    /*
	    **  Set the Link's Network, Source, and Target properties.
	    */

	    _SetValue(link, _P_Linknet, lwk_c_domain_linknet, &network,
		lwk_c_set_property);

	    if (source_id != 0)
		_SetValue(link, _P_Source, lwk_c_domain_integer,
		    &source_id, lwk_c_set_property);

	    if (target_id != 0)
		_SetValue(link, _P_Target, lwk_c_domain_integer,
		    &target_id, lwk_c_set_property);

	    /*
	    **  Add the Link to the Linkbase cache.
	    */

	    AddToCache(linkbase, conn_id, link);

	    /*
	    **  Clear the Modified flag.
	    */

	    _SetState(link, _StateModified, _StateClear);
	}
    }

    /*
    **  Close the Cursor.
    */

    status = LwkGridCloseCursor(cursor);

    if (!_FileSuccess(status))
	_Raise(db_read_error);

    /*
    **  Clean up
    */

    _DeleteDVString(&encoding);

    /*
    **  If any exceptions are raised, clean up then reraise the exception.
    */
    
    _Exceptions
	_WhenOthers
	    LwkGridCloseCursor(cursor);

	    _DeleteDVString(&encoding);

	    _Reraise;
    _EndExceptionBlock

    return;
    }


#ifndef MSDOS
void  LwkLbQueryStepInPath(linkbase, path, path_id, expression)
_Linkbase linkbase;
 _Path path;

    _Integer path_id;
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
    _Step step;
    _Integer step_id;
    _GridCursor cursor;
    _FileStatus status;
    _Integer origin_id;
    _Integer destination_id;
    _Integer previous_id;
    _Integer next_id;
    _DynamicVString volatile encoding;

    /*
    **  If the Linkbase is not Open, that is an error.
    */

    if (!_Open_of(linkbase))
	_Raise(db_not_open);

    /*
    **  Temporary: just retrieve all of them
    */

    /*
    **  Open the Cursor.
    */

    status = LwkGridOpenQuerySteps((_GridFile) _File_of(linkbase), path_id,
	expression, &cursor);

    /*
    **  If there are no Steps in this Path, we are done.
    */

    if (status == _FileStsNoneFound)
	return;

    if (!_FileSuccess(status))
	_Raise(db_read_error);

    /*
    **  Initialize temporary storage.
    */

    step = (_Step) _NullObject;
    _CreateDVString(encoding);

    _StartExceptionBlock

    /*
    **  Loop fetching Steps.
    */

    while (_True) {
	/*
	**  Ask File to fetch the Step.
	*/

	status = LwkGridFetchQuerySteps(cursor, &step_id, &origin_id,
	    &destination_id, &previous_id, &next_id, encoding);

	/*
	**  If the Cursor is empty, we have all the Steps.
	*/

	if (status == _FileStsCursorEmpty)
	    break;

	if (!_FileSuccess(status))
	    _Raise(db_read_error);

	/*
	**  If this Step is already in the cache, no need to proceed.
	*/

	if (!SearchCache(linkbase, step_id, &step)) {
	    /*
	    **  Reconstitute the Step using Import.
	    */

	    step = (_Step) _Import(_TypeStep, _DVString_of(encoding));

	    /*
	    **	Add the Step to the Linkbase cache.  Note: we do this earlier
	    **	than we normally would because the Step needs to "exist" before
	    **	we set some of its properties (e.g., setting the Path property
	    **	will add this Step to Steps list and may try to retrieve its
	    **	Origin or Destination -- which will look for the Step!).
	    */

	    AddToCache(linkbase, step_id, step);

	    /*
	    **  Set the PreviousStep, NextStep properties (needed before Path
	    **  property is set)
	    */

	    if (previous_id != 0)
		_SetValue(step, _P_PreviousStep, lwk_c_domain_integer,
		    &previous_id, lwk_c_set_property);

	    if (next_id != 0)
		_SetValue(step, _P_NextStep, lwk_c_domain_integer, &next_id,
		    lwk_c_set_property);

	    /*
	    **  Set the Path
	    */

	    _SetValue(step, _P_Path, lwk_c_domain_path, &path,
		lwk_c_set_property);

	    /*
	    **  Set the Origin, Destination
	    */

	    if (origin_id != 0)
		_SetValue(step, _P_Origin, lwk_c_domain_integer, &origin_id,
		    lwk_c_set_property);

	    if (destination_id != 0)
		_SetValue(step, _P_Destination, lwk_c_domain_integer,
		    &destination_id, lwk_c_set_property);

	    /*
	    **  Clear the modified flag.
	    */

	    _SetState(step, _StateModified, _StateClear);
	}
    }

    /*
    **  Close the Cursor.
    */

    status = LwkGridCloseCursor(cursor);

    if (!_FileSuccess(status))
	_Raise(db_read_error);

    /*
    **  Clean up
    */

    _DeleteDVString(&encoding);

    /*
    **  If any exceptions are raised, clean up then reraise the exception.
    */
    
    _Exceptions
	_WhenOthers
	    _DeleteDVString(&encoding);

	    status = LwkGridCloseCursor(cursor);

	    _Reraise;
    _EndExceptionBlock

    return;
    }
#endif /* !MSDOS */


void  LwkLbRetrieveSurrLinks(linkbase, surrogate, network, interlinks)
_Linkbase linkbase;
 _Surrogate surrogate;

    _Linknet network;
 _Set interlinks;

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
    _Integer net_id;
    _Integer surr_id;
    _Integer conn_id;
    _Integer source_id;
    _Integer target_id;
    _Boolean modified;
    _GridCursor cursor;
    _FileStatus status;
    _Link link;
    _Integer *interconnect_ids;
    _DynamicVString volatile encoding;

    /*
    **  If the Linkbase is not Open, that is an error.
    */

    if (!_Open_of(linkbase))
	_Raise(db_not_open);

    /*
    **  Get the InterLink Ids from the Set if they were provided (this
    **	destroys the Set).  If there aren't any, return now.
    */

    count = InterLinkSetToIds(interlinks, &interconnect_ids, _True);

    if (count <= 0)
	return;

    _StartExceptionBlock

    /*
    **  Get the Network's and Surrogate's Identifier
    */

    _GetValue(network, _P_Identifier, lwk_c_domain_integer, &net_id);
    _GetValue(surrogate, _P_Identifier, lwk_c_domain_integer, &surr_id);

    /*
    ** First, search the cache for these Links
    */

    i = 0;

    while (_True) {
	if (!SearchCache(linkbase, interconnect_ids[i], &link)) {
            /*
            ** Not in cache.  Increment loop index.  Leave loop if done.
            */

	    i++;

	    if (i == count)
		break;
	}
	else {
	    /*
	    ** The Link was found in the cache.  Set either its Source or
	    ** Target while preserving its Modified state.
	    */

	    modified = (_Boolean) _SetState(link, _StateModified,
		_StateGet);

	    if ((_Boolean) _SetState(link, _StateSourceIsId, _StateGet)) {
		_GetValue(link, _P_Source, lwk_c_domain_integer,
		    &source_id);

		if (surr_id == source_id)
		    _SetValue(link, _P_Source, lwk_c_domain_surrogate,
			&surrogate, lwk_c_set_property);
	    }

	    if ((_Boolean) _SetState(link, _StateTargetIsId, _StateGet)) {
		_GetValue(link, _P_Target, lwk_c_domain_integer,
		    &target_id);

		if (surr_id == target_id)
		    _SetValue(link, _P_Target, lwk_c_domain_surrogate,
			&surrogate, lwk_c_set_property);
	    }

	    if (!modified)
		_SetState(link, _StateModified, _StateClear);

            /*
	    ** Move the last Id to this index and decrement count.  Leave loop
	    ** if done.
            */

	    count--;

	    interconnect_ids[i] = interconnect_ids[count];

	    if (i == count)
		break;
	    }
    }

    _Exceptions
	_WhenOthers
	    _FreeMem(interconnect_ids);
	    _Reraise;
    _EndExceptionBlock

    /*
    **	If we didn't find all the InterLinks in the cache, open the
    **	Cursor to retrieve the remaining Links from the File, then
    **	discard the InterLink Ids
    */

    if (count > 0)
	status = LwkGridOpenSurrConn((_GridFile) _File_of(linkbase), net_id,
	    surr_id, count, interconnect_ids, &cursor);
    else
	status = _FileStsNoneFound;

    _FreeMem(interconnect_ids);

    /*
    **  If there are no Links for this Surrogate, we are done.
    */

    if (status == _FileStsNoneFound)
	return;

    if (!_FileSuccess(status))
	_Raise(db_read_error);

    /*
    **  Initialize temporary storage.
    */

    link = (_Link) _NullObject;
    _CreateDVString(encoding);

    _StartExceptionBlock

    /*
    **  Loop fetching Links.
    */

    while (_True) {
	/*
	**  Ask File to fetch the Link.
	*/

	status = LwkGridFetchSurrConn(cursor, &conn_id, &source_id,
	    &target_id, encoding);

	/*
	**  If the Cursor is empty, we have all the Links.
	*/

	if (status == _FileStsCursorEmpty)
	    break;

	if (!_FileSuccess(status))
	    _Raise(db_read_error);

	/*
	**  Reconstitute the Link using Import.
	*/

	link = (_Link) _Import(_TypeLink,
	    _DVString_of(encoding));

	/*
	**  Set the Link's Network, Source, and Target properties.
	*/

	_SetValue(link, _P_Linknet, lwk_c_domain_linknet, &network,
	    lwk_c_set_property);

	if (surr_id == source_id)
	    _SetValue(link, _P_Source, lwk_c_domain_surrogate, &surrogate,
		lwk_c_set_property);
	else if (source_id != 0)
	    _SetValue(link, _P_Source, lwk_c_domain_integer, &source_id,
		lwk_c_set_property);

	if (surr_id == target_id)
	    _SetValue(link, _P_Target, lwk_c_domain_surrogate, &surrogate,
		lwk_c_set_property);
	else if (target_id != 0)
	    _SetValue(link, _P_Target, lwk_c_domain_integer, &target_id,
		lwk_c_set_property);

	/*
	**  Add the Link to the Linkbase cache.
	*/

	AddToCache(linkbase, conn_id, link);

	/*
	**  Clear the Modified flag.
	*/

	_SetState(link, _StateModified, _StateClear);
    }

    /*
    **  Close the Cursor.
    */

    status = LwkGridCloseCursor(cursor);

    if (!_FileSuccess(status))
	_Raise(db_read_error);

    /*
    **  Clean up
    */

    _DeleteDVString(&encoding);

    /*
    **  If any exceptions are raised, clean up then reraise the exception.
    */
    
    _Exceptions
	_WhenOthers
	    _DeleteDVString(&encoding);
	    LwkGridCloseCursor(cursor);
	    _Reraise;
    _EndExceptionBlock

    return;
    }


#ifndef MSDOS
void  LwkLbRetrieveSurrSteps(linkbase, surrogate, path, interlinks)
_Linkbase linkbase;
 _Surrogate surrogate;

    _Path path;
 _Set interlinks;

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
    _Step step;
    _Integer count;
    _Integer path_id;
    _Integer surr_id;
    _Integer step_id;
    _Integer origin_id;
    _Integer previous_id;
    _Integer next_id;
    _Boolean modified;
    _GridCursor cursor;
    _FileStatus status;
    _Integer destination_id;
    _Integer *interconnect_ids;
    _DynamicVString volatile encoding;

    /*
    **  If the Linkbase is not Open, that is an error.
    */

    if (!_Open_of(linkbase))
	_Raise(db_not_open);

    /*
    **  Get the InterLink Ids from the Set if they were provided (this
    **	destroys the Set).  If there aren't any, return now.
    */

    count = InterLinkSetToIds(interlinks, &interconnect_ids, _True);

    if (count <= 0)
	return;

    /*
    **  Get the Path's and Surrogate's Identifier
    */

    _GetValue(path, _P_Identifier, lwk_c_domain_integer, &path_id);
    _GetValue(surrogate, _P_Identifier, lwk_c_domain_integer, &surr_id);

    /*
    ** First, search the cache for these Steps
    */

    _StartExceptionBlock

    i = 0;

    while (_True) {
	if (!SearchCache(linkbase, interconnect_ids[i], &step)) {
            /*
            ** Not in cache.  Increment loop index.  Leave loop if done.
            */

	    i++;

	    if (i == count)
		break;
	}
	else {
	    /*
	    ** The Step was found in the cache.  Set either its Origin or
	    ** Destination while preserving its Modified state.
	    */

	    modified = (_Boolean) _SetState(step, _StateModified, _StateGet);

	    if ((_Boolean) _SetState(step, _StateOriginIsId, _StateGet)) {
		_GetValue(step, _P_Origin, lwk_c_domain_integer, &origin_id);

		if (surr_id == origin_id)
		    _SetValue(step, _P_Origin, lwk_c_domain_surrogate,
			&surrogate, lwk_c_set_property);
	    }

	    if ((_Boolean) _SetState(step, _StateDestinationIsId, _StateGet)) {
		_GetValue(step, _P_Destination, lwk_c_domain_integer,
		    &destination_id);

		if (surr_id == destination_id)
		    _SetValue(step, _P_Destination, lwk_c_domain_surrogate,
			&surrogate, lwk_c_set_property);
	    }

	    if (!modified)
		_SetState(step, _StateModified, _StateClear);

            /*
	    ** Move the last Id to this index and decrement count.  Leave loop
	    ** if done.
            */

	    count--;

	    interconnect_ids[i] = interconnect_ids[count];

	    if (i == count)
		break;
	}
    }

    _Exceptions
	_WhenOthers
	    _FreeMem(interconnect_ids);
	    _Reraise;
    _EndExceptionBlock

    /*
    **	If we didn't find all the InterLinks in the cache, open the
    **	Cursor to retrieve the remaining Steps from the File, then discard the
    **	InterLink Ids
    */

    if (count > 0)
	status = LwkGridOpenSurrStep((_GridFile) _File_of(linkbase), path_id,
	    surr_id, count, interconnect_ids, &cursor);
    else
	status = _FileStsNoneFound;

    _FreeMem(interconnect_ids);

    /*
    **  If there are no Steps in this Path, we are done.
    */

    if (status == _FileStsNoneFound)
	return;

    if (!_FileSuccess(status))
	_Raise(db_read_error);

    /*
    **  Initialize temporary storage.
    */

    step = (_Step) _NullObject;
    _CreateDVString(encoding);

    _StartExceptionBlock

    /*
    **  Loop fetching Steps.
    */

    while (_True) {
	/*
	**  Ask File to fetch the Step.
	*/

	status = LwkGridFetchSurrStep(cursor, &step_id, &origin_id,
	    &destination_id, &previous_id, &next_id, encoding);

	/*
	**  If the Cursor is empty, we have all the Steps.
	*/

	if (status == _FileStsCursorEmpty)
	    break;

	if (!_FileSuccess(status))
	    _Raise(db_read_error);

	/*
	**  Reconstitute the Step using Import.
	*/

	step = (_Step) _Import(_TypeStep, _DVString_of(encoding));

	/*
	** Add the Step to the Linkbase cache.  Note: we do this earlier than
	** we normally would because the Step needs to "exist" before we set
	** some of its properties (e.g., setting the Path property will add
	** this Step to Steps list and may try to retrieve its Origin or
	** Destination -- which will look for the Step!).
	*/

	AddToCache(linkbase, step_id, step);

	/*
	**  Set the PreviousStep, NextStep properties (needed before Path
	**  property is set)
	*/

	if (previous_id != 0)
	    _SetValue(step, _P_PreviousStep, lwk_c_domain_integer,
		&previous_id, lwk_c_set_property);

	if (next_id != 0)
	    _SetValue(step, _P_NextStep, lwk_c_domain_integer, &next_id,
		lwk_c_set_property);

	/*
	**  Set the Path
	*/

	_SetValue(step, _P_Path, lwk_c_domain_path, &path,
	    lwk_c_set_property);

	/*
	**  Set the Origin, Destination
	*/

	if (surr_id == origin_id)
	    _SetValue(step, _P_Origin, lwk_c_domain_surrogate, &surrogate,
		lwk_c_set_property);
	else if (origin_id != 0)
		_SetValue(step, _P_Origin, lwk_c_domain_integer, &origin_id,
		    lwk_c_set_property);

	if (surr_id == destination_id)
	    _SetValue(step, _P_Destination, lwk_c_domain_surrogate,
		&surrogate, lwk_c_set_property);
	else if (destination_id != 0)
	    _SetValue(step, _P_Destination, lwk_c_domain_integer,
		&destination_id, lwk_c_set_property);

	/*
	**  Clear the modified flag.
	*/

	_SetState(step, _StateModified, _StateClear);
    }

    /*
    **  Close the Cursor.
    */

    status = LwkGridCloseCursor(cursor);

    if (!_FileSuccess(status))
	_Raise(db_read_error);

    /*
    **  Clean up
    */

    _DeleteDVString(&encoding);

    /*
    **  If any exceptions are raised, clean up then reraise the exception.
    */
    
    _Exceptions
	_WhenOthers
	    _DeleteDVString(&encoding);
	    LwkGridCloseCursor(cursor);
	    _Reraise;
    _EndExceptionBlock

    return;
    }
#endif /* !MSDOS */


_Boolean  LwkLbVerify(linkbase, flags, file)
_Linkbase linkbase;
 _Integer flags;
 _OSFile file;

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
    _FileStatus status;

    /*
    **  If the Linkbase is not Open, that is an error.
    */

    if (!_Open_of(linkbase))
	_Raise(db_not_open);

    /*
    **  Let the File module to do the grungy work
    */

    status = LwkGridVerify((_GridFile) _File_of(linkbase), flags, file);

    if (_FileSuccess(status))
	return _True;
    else
	return _False;
    }


static _Integer  GenerateIdentifier(linkbase)
_Linkbase linkbase;

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
    _FileStatus status;

    /*
    **  If we have run out of allocated Uid's, allocate some more.  Otherwise,
    **	increment the current Uid and use it.
    */

    if (_CurrentUid_of(linkbase) < _AllocatedUid_of(linkbase)) {
	_CurrentUid_of(linkbase)++;
    }
    else {
	status = LwkGridAllocateUids((_GridFile) _File_of(linkbase),
	    _UidIncrementalAllocation, &_CurrentUid_of(linkbase));

	if (!_FileSuccess(status))
	    _Raise(db_read_error);

	_AllocatedUid_of(linkbase) = _CurrentUid_of(linkbase) +
	    _UidIncrementalAllocation - 1;
    }

    return _CurrentUid_of(linkbase);
    }


static void  StoreSurrogate(linkbase, persistent, uid, encoding, new)
_Linkbase linkbase;
 _Persistent persistent;

    _Integer uid;
 _VaryingString encoding;
 _Boolean new;

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
    _FileStatus status;
    _Persistent container;
    _Integer has_incomming;
    _Integer has_outgoing;
    _Integer container_id;
    _Integer container_domain;
    _Integer volatile count;
    _Integer *interconnect_ids;

    /*
    **  Initialize
    */

    count = 0;

    _StartExceptionBlock

    /*
    **  Get the _Container of the Surrogate
    */

    _GetValue(persistent, _P_Container, lwk_c_domain_persistent, &container);

    /*
    **  Get the _Identifier of the Container
    */

    _GetValue(container, _P_Identifier, lwk_c_domain_integer, &container_id);

    /*
    **  Get the Domain of the Container
    */

    container_domain = (_Integer) _TypeToDomain(_Type_of(container));

    /*
    **  Get the InterLink state
    */

    has_incomming = (_Boolean) _SetState(persistent, _StateHasInComming,
	_StateGet);
    has_outgoing = (_Boolean) _SetState(persistent, _StateHasOutGoing,
	_StateGet);

    /*
    **  Get the InterLink Ids
    */

    count = GetInterLinkIds(persistent, &interconnect_ids);

    /*
    **  Call the File module to actually Insert or Update the row in the proper
    **	table.
    */

    status = LwkGridInsertSurrogate((_GridFile) _File_of(linkbase), uid,
	container_id, container_domain, has_incomming, has_outgoing,
	count, interconnect_ids, encoding, new);

    if (!_FileSuccess(status))
	_Raise(db_write_error);

    /*
    **  Clean up
    */

    if (count > 0)
	_FreeMem(interconnect_ids);

    /*
    **	If any exceptions are raised, clean up then reraise the exception.
    */
    
    _Exceptions
	_WhenOthers
	    if (count > 0)
		_FreeMem(interconnect_ids);
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  StoreLink(linkbase, persistent, uid, encoding, new)
_Linkbase linkbase;
 _Persistent persistent;

    _Integer uid;
 _VaryingString encoding;
 _Boolean new;

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
    _FileStatus status;
    _Linknet network;
    _Integer network_id;
    _Surrogate source;
    _Integer source_id;
    _Surrogate target;
    _Integer target_id;

    /*
    **  Get the _Linknet of the Link
    */

    _GetValue(persistent, _P_Linknet, lwk_c_domain_linknet, &network);

    /*
    **  Get the _Identifier of the Network
    */

    _GetValue(network, _P_Identifier, lwk_c_domain_integer, &network_id);

    /*
    **  Get the _Identifier of the _Source of the Link
    */

    if (_SetState(persistent, _StateSourceIsId, _StateGet))
	_GetValue(persistent, _P_Source, lwk_c_domain_integer, &source_id);
    else {
	_GetValue(persistent, _P_Source, lwk_c_domain_surrogate, &source);

	if (source == (_Surrogate) _NullObject)
	    source_id = 0;
	else
	    _GetValue(source, _P_Identifier, lwk_c_domain_integer, &source_id);
    }

    /*
    **  Get the _Identifier of the _Target of the Link
    */

    if (_SetState(persistent, _StateTargetIsId, _StateGet))
	_GetValue(persistent, _P_Target, lwk_c_domain_integer, &target_id);
    else {
	_GetValue(persistent, _P_Target, lwk_c_domain_surrogate, &target);

	if (target == (_Surrogate) _NullObject)
	    target_id = 0;
	else
	    _GetValue(target, _P_Identifier, lwk_c_domain_integer, &target_id);
    }

    /*
    **  Call the File module to actually Insert or Update the row in the proper
    **	table.
    */

    status = LwkGridInsertLink((_GridFile) _File_of(linkbase), uid,
	source_id, target_id, network_id, encoding, new);

    if (!_FileSuccess(status))
	_Raise(db_write_error);

    return;
    }


static void  StoreNetwork(linkbase, persistent, uid, encoding, new)
_Linkbase linkbase;
 _Persistent persistent;

    _Integer uid;
 _VaryingString encoding;
 _Boolean new;

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
    _FileStatus status;

    /*
    **  Call the File module to actually Insert or Update the row in the proper
    **	table.
    */

    status = LwkGridInsertNetwork((_GridFile) _File_of(linkbase), uid,
	encoding, new);

    if (!_FileSuccess(status))
	_Raise(db_write_error);

    return;
    }


#ifndef MSDOS
static void  StoreStep(linkbase, persistent, uid, encoding, new)
_Linkbase linkbase;
 _Persistent persistent;

    _Integer uid;
 _VaryingString encoding;
 _Boolean new;

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
    _FileStatus status;
    _Path path;
    _Step step;
    _Integer path_id;
    _Integer next_id;
    _Integer previous_id;
    _Surrogate origin;
    _Integer origin_id;
    _Surrogate destination;
    _Integer destination_id;

    /*
    **  Get the _Path of the Step
    */

    _GetValue(persistent, _P_Path, lwk_c_domain_path, &path);

    /*
    **  Get the _Identifier of the Path
    */

    _GetValue(path, _P_Identifier, lwk_c_domain_integer, &path_id);

    /*
    **  Get the _Identifier of the _Origin of the Step
    */

    if (_SetState(persistent, _StateOriginIsId, _StateGet))
	_GetValue(persistent, _P_Origin, lwk_c_domain_integer, &origin_id);
    else {
	_GetValue(persistent, _P_Origin, lwk_c_domain_surrogate, &origin);

	if (origin == (_Surrogate) _NullObject)
	    origin_id = 0;
	else
	    _GetValue(origin, _P_Identifier, lwk_c_domain_integer, &origin_id);
    }

    /*
    **  Get the _Identifier of the _Destination of the Step
    */

    if (_SetState(persistent, _StateDestinationIsId, _StateGet))
	_GetValue(persistent, _P_Destination, lwk_c_domain_integer,
	    &destination_id);
    else {
	_GetValue(persistent, _P_Destination, lwk_c_domain_surrogate,
	    &destination);

	if (destination == (_Surrogate) _NullObject)
	    destination_id = 0;
	else
	    _GetValue(destination, _P_Identifier, lwk_c_domain_integer,
		&destination_id);
    }

    /*
    **  Get the _Identifier of the _PreviousStep of the Step
    */

    if (_SetState(persistent, _StatePreviousStepIsId, _StateGet))
	_GetValue(persistent, _P_PreviousStep, lwk_c_domain_integer,
	    &previous_id);
    else {
	_GetValue(persistent, _P_PreviousStep, lwk_c_domain_step, &step);

	if (step == (_Step) _NullObject)
	    previous_id = 0;
	else
	    _GetValue(step, _P_Identifier, lwk_c_domain_integer, &previous_id);
    }

    /*
    **  Get the _Identifier of the _NextStep of the Step
    */

    if (_SetState(persistent, _StateNextStepIsId, _StateGet))
	_GetValue(persistent, _P_NextStep, lwk_c_domain_integer, &next_id);
    else {
	_GetValue(persistent, _P_NextStep, lwk_c_domain_step, &step);

	if (step == (_Step) _NullObject)
	    next_id = 0;
	else
	    _GetValue(step, _P_Identifier, lwk_c_domain_integer, &next_id);
    }

    /*
    **  Call the File module to actually Insert or Update the row in the proper
    **	table.
    */

    status = LwkGridInsertStep((_GridFile) _File_of(linkbase), uid,
	origin_id, destination_id, previous_id, next_id, path_id, encoding,
	new);

    if (!_FileSuccess(status))
	_Raise(db_write_error);

    return;
    }


static void  StorePath(linkbase, persistent, uid, encoding, new)
_Linkbase linkbase;
 _Persistent persistent;

    _Integer uid;
 _VaryingString encoding;
 _Boolean new;

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
    _FileStatus status;
    _Step step;
    _Integer first_step;
    _Integer last_step;
    _Integer current_step;

    /*
    **  Get the Uid of the FirstStep
    */

    _GetValue(persistent, _P_FirstStep, lwk_c_domain_step, &step);

    if (step == (_Step) _NullObject)
	first_step = 0;
    else
	_GetValue(step, _P_Identifier, lwk_c_domain_integer, &first_step);

    /*
    **  Get the Uid of the LastStep
    */

    _GetValue(persistent, _P_LastStep, lwk_c_domain_step, &step);

    if (step == (_Step) _NullObject)
	last_step = 0;
    else
	_GetValue(step, _P_Identifier, lwk_c_domain_integer, &last_step);

    /*
    **  Get the Uid of the CurrentStep
    */

    _GetValue(persistent, _P_FollowedStep, lwk_c_domain_step, &step);

    if (step == (_Step) _NullObject)
	current_step = 0;
    else
	_GetValue(step, _P_Identifier, lwk_c_domain_integer, &current_step);

    /*
    **  Call the File module to actually Insert or Update the row in the proper
    **	table.
    */

    status = LwkGridInsertPath((_GridFile) _File_of(linkbase), uid,
	first_step, last_step, current_step, encoding, new);

    if (!_FileSuccess(status))
	_Raise(db_write_error);

    return;
    }


static void  StoreCompositeNet(linkbase, persistent, uid, encoding, new)
_Linkbase linkbase;
 _Persistent persistent;

    _Integer uid;
 _VaryingString encoding;
 _Boolean new;

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
    _FileStatus status;

    /*
    **  Call the File module to actually Insert or Update the row in the proper
    **	table.
    */

    status = LwkGridInsertCompositeNet((_GridFile) _File_of(linkbase), uid,
	encoding, new);

    if (!_FileSuccess(status))
	_Raise(db_write_error);

    return;
    }


static void  StoreCompositePath(linkbase, persistent, uid, encoding, new)
_Linkbase linkbase;
 _Persistent persistent;

    _Integer uid;
 _VaryingString encoding;
 _Boolean new;

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
    _FileStatus status;

    /*
    **  Call the File module to actually Insert or Update the row in the proper
    **	table.
    */

    status = LwkGridInsertCompositePath((_GridFile) _File_of(linkbase), uid,
	encoding, new);

    if (!_FileSuccess(status))
	_Raise(db_write_error);

    return;
    }
#endif /* !MSDOS */


static void  DeleteFile(linkbase)
_Linkbase linkbase;

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
    _FileStatus status;

    /*
    **  Can't drop a Linkbase that is in use!
    */

    if (_UseCount_of(linkbase) > 0)
	_Raise(linkbase_in_use);

    /*
    **  Make sure the File associated with the Linkbase is closed
    */

    if (_Open_of(linkbase))
	LwkLbClose(linkbase);

    /*
    **  Ask File to do the grungy work.
    */

    status = LwkGridDeleteFile((_GridFile) _File_of(linkbase),
	_Identifier_of(linkbase));

    if (!_FileSuccess(status))
	_Raise(db_write_error);

    return;
    }


static _Persistent  RetrieveSurrogate(linkbase, uid, container_id)
_Linkbase linkbase;
 _Integer uid;

    _Integer container_id;

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
    _Integer count;
    _Boolean modified;
    _FileStatus status;
    _Surrogate surrogate;
    _Persistent container;
    _Integer has_outgoing;
    _Integer has_incomming;
    _Integer container_domain;
    _IntegerPtr interconnect_ids;
    _Set volatile interconnect_set;
    _DynamicVString volatile encoding;


    /*
    **  If the object is in the cache, take it from there.  Otherwise,
    **	retrieve the object from the database.
    */

    if (!SearchCache(linkbase, uid, &surrogate)) {
	/*
	**  Initialize temporary storage.
	*/

	interconnect_set = (_Set) _NullObject;
	surrogate = (_Surrogate) _NullObject;
	_CreateDVString(encoding);

	_StartExceptionBlock

	/*
	**  Ask File to fetch the object from the proper table.
	*/

	status = LwkGridSelectSurrogate((_GridFile) _File_of(linkbase), uid,
	    container_id, &container_domain, &has_incomming, &has_outgoing,
	    &count, &interconnect_ids, encoding);

	if (status == _FileStsCursorEmpty)
	    _Raise(no_such_object);
	    
	if (!_FileSuccess(status))
	    _Raise(db_read_error);

	/*
	**  Reconstitute the Surrogate using Import.
	*/

	surrogate = (_Surrogate) _Import(_TypeSurrogate,
	    _DVString_of(encoding));

	/*
	**  Add the Surrogate to the Linkbase cache.  Note: we do this
	**  earlier than we normally would because the Surrogate needs to
	**  "exist" before we set some of its properties (e.g., setting the
	**  Path property will add this Surrogate to Surrogates list and may
	**  try to retrieve its Interlinks -- which will look for the
	**  Surrogte!).
	*/

	AddToCache(linkbase, uid, surrogate);

	/*
	**  Set the InterLink state appropriately
	*/

	if (has_incomming || has_outgoing) {
	    _SetState(surrogate, _StateHaveInterLinks, _StateClear);

	    if (has_incomming)
		_SetState(surrogate, _StateHasInComming, _StateSet);

	    if (has_outgoing)
		_SetState(surrogate, _StateHasOutGoing, _StateSet);
	}

	/*
	**  Convert any InterLink Ids into a Set (destroying the Id
	**  array) and set them as the InterLinks of the SUrrogate.
	*/

	if (count > 0) {
	    interconnect_set = InterLinkIdsToSet(count,
		interconnect_ids);

	    _SetValue(surrogate, _P_InterLinks, lwk_c_domain_set,
		(_AnyPtr) &interconnect_set, lwk_c_set_property);

	    _Delete(&interconnect_set);
	}

	/*
	**  Retrieve the Container of the Surrogate and set that Property of
	**  the Surrogate, while preserving the Modified state of the
	**  Container.
	*/

	switch ((_Domain) container_domain) {
	    case lwk_c_domain_linknet :
		container = RetrieveNetwork(linkbase, container_id);

		modified = (_Boolean) _SetState(container, _StateModified,
		    _StateGet);

		_SetValue(surrogate, _P_Container, lwk_c_domain_linknet,
		    &container, lwk_c_set_property);

		if (!modified)
		    _SetState(container, _StateModified, _StateClear);

		break;

#ifndef MSDOS

	    case lwk_c_domain_path :
		container = RetrievePath(linkbase, container_id);

		modified = (_Boolean) _SetState(container, _StateModified,
		    _StateGet);

		_SetValue(surrogate, _P_Container, lwk_c_domain_path, &container,
		    lwk_c_set_property);

		if (!modified)
		    _SetState(container, _StateModified, _StateClear);

		break;

#endif /* !MSDOS */

	}

	/*
	**  Clean up
	*/

	_DeleteDVString(&encoding);

	/*
	**  Clear the modified flag
	*/

	_SetState(surrogate, _StateModified, _StateClear);

	/*
	**  If any exceptions are raised, clean up then reraise the exception.
	*/

	_Exceptions
	    _WhenOthers
		_Delete(&interconnect_set);
		_DeleteDVString(&encoding);
		_Delete(&surrogate);
		_Reraise;
	_EndExceptionBlock
    }

    return (_Persistent) surrogate;
    }


static _Persistent  RetrieveLink(linkbase, uid, network_id)
_Linkbase linkbase;
 _Integer uid;

    _Integer network_id;

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
    _Boolean modified;
    _Linknet network;
    _FileStatus status;
    _Integer source_id;
    _Integer target_id;
    _Link link;
    _DynamicVString volatile encoding;

    /*
    **  If the object is in the cache, take it from there.  Otherwise,
    **	retrieve the object from the database.
    */

    if (!SearchCache(linkbase, uid, &link)) {
	/*
	**  Initialize temporary storage.
	*/

	link = (_Link) _NullObject;
	_CreateDVString(encoding);

	_StartExceptionBlock

	/*
	**  Ask File to fetch the object from the proper table.
	*/

	status = LwkGridSelectLink((_GridFile) _File_of(linkbase), uid,
	    network_id, &source_id, &target_id, encoding);

	if (status == _FileStsCursorEmpty)
	    _Raise(no_such_object);
	    
	if (!_FileSuccess(status))
	    _Raise(db_read_error);

	/*
	**  Reconstitute the Link using Import.
	*/

	link = (_Link) _Import(_TypeLink,
	    _DVString_of(encoding));

	/*
	**  Retrieve the Network of the Link and set that Property of
	**  the Link, while preserving the Modified state of the
	**  Network.
	*/

	network = RetrieveNetwork(linkbase, network_id);

	modified = (_Boolean) _SetState(network, _StateModified, _StateGet);

	_SetValue(link, _P_Linknet, lwk_c_domain_linknet, &network,
	    lwk_c_set_property);

	if (!modified)
	    _SetState(network, _StateModified, _StateClear);

	/*
	**  Set the Source and Target properties.
	*/

	if (source_id != 0)
	    _SetValue(link, _P_Source, lwk_c_domain_integer, &source_id,
		lwk_c_set_property);

	if (target_id != 0)
	    _SetValue(link, _P_Target, lwk_c_domain_integer, &target_id,
		lwk_c_set_property);

	/*
	**  Clean up
	*/

	_DeleteDVString(&encoding);

	/*
	**  Add the Object to the Linkbase cache.
	*/

	AddToCache(linkbase, uid, link);

	/*
	**  Clear the modified flag
	*/

	_SetState(link, _StateModified, _StateClear);

	/*
	**  If any exceptions are raised, clean up then reraise the exception.
	*/
	
	_Exceptions
	    _WhenOthers
		_DeleteDVString(&encoding);
		_Delete(&link);
		_Reraise;
	_EndExceptionBlock
    }

    return (_Persistent) link;
    }


static _Persistent  RetrieveNetwork(linkbase, uid)
_Linkbase linkbase;
 _Integer uid;

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
    _Linknet network;
    _FileStatus status;
    _DynamicVString volatile encoding;

    /*
    **  If the object is in the cache, take it from there.  Otherwise,
    **	retrieve the object from the database.
    */

    if (!SearchCache(linkbase, uid, &network)) {
	/*
	**  Initialize temporary storage.
	*/

	network = (_Linknet) _NullObject;
	_CreateDVString(encoding);

	_StartExceptionBlock

	/*
	**  Ask File to fetch the object from the proper table.
	*/

	status = LwkGridSelectNetwork((_GridFile) _File_of(linkbase), uid,
	    encoding);

	if (status == _FileStsCursorEmpty)
	    _Raise(no_such_object);
	    
	if (!_FileSuccess(status))
	    _Raise(db_read_error);

	/*
	**  Reconstitute the object using Import.
	*/

	network = (_Linknet) _Import(_TypeLinknet, _DVString_of(encoding));

	/*
	**  Set the state to indicate we do not have all the Surrogates and
	**  Links yet.
	*/

	_SetState(network, _StateHaveAllSurrogates, _StateClear);
	_SetState(network, _StateHaveAllLinks, _StateClear);

	/*
	**  Clean up
	*/

	_DeleteDVString(&encoding);

	/*
	**  Add the Object to the Linkbase cache.
	*/

	AddToCache(linkbase, uid, network);

	/*
	**  Clear the modified flag
	*/

	_SetState(network, _StateModified, _StateClear);

	/*
	**  If any exceptions are raised, clean up then reraise the exception.
	*/
	
	_Exceptions
	    _WhenOthers
		_DeleteDVString(&encoding);
		_Delete(&network);
		_Reraise;
	_EndExceptionBlock
    }

    return (_Persistent) network;
    }


#ifndef MSDOS
static _Persistent  RetrievePath(linkbase, uid)
_Linkbase linkbase;
 _Integer uid;

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
    _Path path;
    _Step step;
    _FileStatus status;
    _Integer first_step;
    _Integer last_step;
    _Integer current_step;
    _DynamicVString volatile encoding;

    /*
    **  If the object is in the cache, take it from there.  Otherwise,
    **	retrieve the object from the database.
    */

    if (!SearchCache(linkbase, uid, &path)) {
	/*
	**  Initialize temporary storage.
	*/

	path = (_Path) _NullObject;
	_CreateDVString(encoding);

	_StartExceptionBlock

	/*
	**  Ask File to fetch the object from the proper table.
	*/

	status = LwkGridSelectPath((_GridFile) _File_of(linkbase), uid,
	    &first_step, &last_step, &current_step, encoding);

	if (status == _FileStsCursorEmpty)
	    _Raise(no_such_object);
	    
	if (!_FileSuccess(status))
	    _Raise(db_read_error);

	/*
	**  Reconstitute the object using Import.
	*/

	path = (_Path) _Import(_TypePath, _DVString_of(encoding));

	/*
	**  Set the state to indicate we do not have all the Surrogates and
	**  Steps yet.
	*/

	_SetState(path, _StateHaveAllSurrogates, _StateClear);
	_SetState(path, _StateHaveAllSteps, _StateClear);

	/*
	**  Add the Object to the Linkbase cache.
	*/

	AddToCache(linkbase, uid, path);

	/*
	**  Now that the Path "exists", we can Retrieve the First/Last/Current
	**  Steps. [The Path must exist so that the retrieved Steps can be
	**  added to it].
	*/

	if (first_step == 0)
	    step = (_Step) _NullObject;
	else
	    step = RetrieveStep(linkbase, first_step, uid);

	_SetValue(path, _P_FirstStep, lwk_c_domain_step, &step,
	    lwk_c_set_property);

	/*
	**  Set the LastStep
	*/

	if (last_step == 0)
	    step = (_Step) _NullObject;
	else
	    step = RetrieveStep(linkbase, last_step, uid);

	_SetValue(path, _P_LastStep, lwk_c_domain_step, &step,
	    lwk_c_set_property);

	/*
	**  Set the CurrentStep
	*/

	if (current_step == 0)
	    step = (_Step) _NullObject;
	else
	    step = RetrieveStep(linkbase, current_step, uid);

	_SetValue(path, _P_FollowedStep, lwk_c_domain_step, &step,
	    lwk_c_set_property);

	/*
	**  Clear the modified flag
	*/

	_SetState(path, _StateModified, _StateClear);

	/*
	**  Clean up
	*/

	_DeleteDVString(&encoding);

	/*
	**  If any exceptions are raised, clean up then reraise the exception.
	*/
	
	_Exceptions
	    _WhenOthers
		_DeleteDVString(&encoding);
		_Delete(&path);
		_Reraise;
	_EndExceptionBlock
    }

    return (_Persistent) path;
    }


static _Persistent  RetrieveStep(linkbase, uid, path_id)
_Linkbase linkbase;
 _Integer uid;

    _Integer path_id;

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
    _Step step;
    _Path path;
    _Boolean modified;
    _FileStatus status;
    _Integer origin_id;
    _Integer next_id;
    _Integer previous_id;
    _Integer destination_id;
    _DynamicVString volatile encoding;

    /*
    **  If the object is in the cache, take it from there.  Otherwise,
    **	retrieve the object from the database.
    */

    if (!SearchCache(linkbase, uid, &step)) {
	/*
	**  Initialize temporary storage.
	*/

	step = (_Step) _NullObject;
	_CreateDVString(encoding);

	_StartExceptionBlock

	/*
	**  Ask File to fetch the object from the proper table.
	*/

	status = LwkGridSelectStep((_GridFile) _File_of(linkbase), uid,
	    path_id, &origin_id, &destination_id, &previous_id, &next_id,
	    encoding);

	if (status == _FileStsCursorEmpty)
	    _Raise(no_such_object);
	    
	if (!_FileSuccess(status))
	    _Raise(db_read_error);

	/*
	**  Reconstitute the Step using Import.
	*/

	step = (_Step) _Import(_TypeStep, _DVString_of(encoding));

	/*
	**  Add the Step to the Linkbase cache.  Note: we do this earlier
	**  than we normally would because the Step needs to "exist" before we
	**  set some of its properties (e.g., setting the Path property will
	**  add this Step to Steps list and may try to retrieve its Origin or
	**  Destination -- which will look for the Step!).
	*/

	AddToCache(linkbase, uid, step);

	/*
	**  Set the PreviousStep, NextStep properties (needed before Path
	**  property is set)
	*/

	if (previous_id != 0)
	    _SetValue(step, _P_PreviousStep, lwk_c_domain_integer, &previous_id,
		lwk_c_set_property);

	if (next_id != 0)
	    _SetValue(step, _P_NextStep, lwk_c_domain_integer, &next_id,
		lwk_c_set_property);

	/*
	**  Retrieve the Path of the Step and set that Property of the Step,
	**  while preserving the Modified state of the Path.
	*/

	path = RetrievePath(linkbase, path_id);

	modified = (_Boolean) _SetState(path, _StateModified, _StateGet);

	_SetValue(step, _P_Path, lwk_c_domain_path, &path, lwk_c_set_property);

	if (!modified)
	    _SetState(path, _StateModified, _StateClear);

	/*
	**  Set the Origin, Destination
	*/

	if (origin_id != 0)
	    _SetValue(step, _P_Origin, lwk_c_domain_integer, &origin_id,
		lwk_c_set_property);

	if (destination_id != 0)
	    _SetValue(step, _P_Destination, lwk_c_domain_integer,
		&destination_id, lwk_c_set_property);

	/*
	**  Clean up
	*/

	_DeleteDVString(&encoding);

	/*
	**  Clear the modified flag
	*/

	_SetState(step, _StateModified, _StateClear);

	/*
	**  If any exceptions are raised, clean up then reraise the exception.
	*/
	
	_Exceptions
	    _WhenOthers
		_DeleteDVString(&encoding);
		_Delete(&step);
		_Reraise;
	_EndExceptionBlock
    }

    return (_Persistent) step;
    }


static _Persistent  RetrieveCompositeNet(linkbase, uid)
_Linkbase linkbase;
 _Integer uid;

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
    _FileStatus status;
    _CompLinknet cnet;
    _DynamicVString volatile encoding;

    /*
    **  If the object is in the cache, take it from there.  Otherwise,
    **	retrieve the object from the database.
    */

    if (!SearchCache(linkbase, uid, &cnet)) {
	/*
	**  Initialize temporary storage.
	*/

	cnet = (_CompLinknet) _NullObject;
	_CreateDVString(encoding);

	_StartExceptionBlock

	/*
	**  Ask File to fetch the object from the proper table.
	*/

	status = LwkGridSelectCompositeNet((_GridFile) _File_of(linkbase),
	    uid, encoding);

	if (status == _FileStsCursorEmpty)
	    _Raise(no_such_object);
	    
	if (!_FileSuccess(status))
	    _Raise(db_read_error);

	/*
	**  Reconstitute the object using Import.
	*/

	cnet = (_CompLinknet) _Import(_TypeCompLinknet, _DVString_of(encoding));

	/*
	** Reset any self-relative Object Descriptors
	*/

	SetSelfRelativeDescriptors(cnet, lwk_c_domain_comp_linknet,
	    linkbase, _False);

	/*
	**  Clean up
	*/

	_DeleteDVString(&encoding);

	/*
	**  Add the Object to the Linkbase cache.
	*/

	AddToCache(linkbase, uid, cnet);

	/*
	**  Clear the modified flag
	*/

	_SetState(cnet, _StateModified, _StateClear);

	/*
	**  If any exceptions are raised, clean up then reraise the exception.
	*/
	
	_Exceptions
	    _WhenOthers
		_DeleteDVString(&encoding);
		_Delete(&cnet);
		_Reraise;
	_EndExceptionBlock
    }

    return (_Persistent) cnet;
    }


static _Persistent  RetrieveCompositePath(linkbase, uid)
_Linkbase linkbase;
 _Integer uid;

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
    _FileStatus status;
    _CompPath cpath;
    _DynamicVString volatile encoding;

    /*
    **  If the object is in the cache, take it from there.  Otherwise,
    **	retrieve the object from the database.
    */

    if (!SearchCache(linkbase, uid, &cpath)) {
	/*
	**  Initialize temporary storage.
	*/

	cpath = (_CompPath) _NullObject;
	_CreateDVString(encoding);

	_StartExceptionBlock

	/*
	**  Ask File to fetch the object from the proper table.
	*/

	status = LwkGridSelectCompositePath((_GridFile) _File_of(linkbase),
	    uid, encoding);

	if (status == _FileStsCursorEmpty)
	    _Raise(no_such_object);
	    
	if (!_FileSuccess(status))
	    _Raise(db_read_error);

	/*
	**  Reconstitute the object using Import.
	*/

	cpath = (_CompPath) _Import(_TypeCompPath, _DVString_of(encoding));

	/*
	** Reset any self-relative Object Descriptors
	*/

	SetSelfRelativeDescriptors(cpath, lwk_c_domain_comp_path,
	    linkbase, _False);

	/*
	**  Clean up
	*/

	_DeleteDVString(&encoding);

	/*
	**  Add the Object to the Linkbase cache.
	*/

	AddToCache(linkbase, uid, cpath);

	/*
	**  Clear the modified flag
	*/

	_SetState(cpath, _StateModified, _StateClear);

	/*
	**  If any exceptions are raised, clean up then reraise the exception.
	*/
	
	_Exceptions
	    _WhenOthers
		_DeleteDVString(&encoding);
		_Delete(&cpath);
		_Reraise;
	_EndExceptionBlock
    }

    return (_Persistent) cpath;
    }


static _Boolean  SetSelfRelativeDescriptors(persistent, domain, linkbase, store)
_Persistent persistent;

    _Domain domain;
 _Linkbase linkbase;
 _Boolean store;

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
    _Boolean modified;
    _List volatile old_descriptors;
    _List volatile new_descriptors;
    _String volatile descriptor_id;
    _ObjectDesc volatile descriptor;

    /*
    ** Initialize
    */
    
    modified = _False;
    old_descriptors = (_List) _NullObject;
    new_descriptors = (_List) _NullObject;
    descriptor_id = (_String) _NullObject;
    descriptor = (_ObjectDesc) _NullObject;

    _StartExceptionBlock

    /*
    ** Get the List of ObjectDescriptors
    */
    
    switch (domain) {
	case lwk_c_domain_comp_linknet :
	    _GetValue(persistent, _P_Linknets, lwk_c_domain_set,
		(_AnyPtr) &old_descriptors);
	    break;

	case lwk_c_domain_comp_path :
	    _GetValue(persistent, _P_Paths, lwk_c_domain_list,
		(_AnyPtr) &old_descriptors);
	    break;

	default :
	    _Raise(inv_domain);
	    break;
    }

    /*
    ** Modify them appropriately
    */
    
    if (old_descriptors != (_List) _NullObject) {
	_GetValue(old_descriptors, _P_ElementCount, lwk_c_domain_integer,
	    &count);

	switch (domain) {
	    case lwk_c_domain_comp_linknet :
		new_descriptors = (_List) _CreateSet(_TypeSet,
		    lwk_c_domain_object_desc, count);
		break;

	    case lwk_c_domain_comp_path :
		new_descriptors = (_List) _CreateList(_TypeList,
		    lwk_c_domain_object_desc, count);
		break;
	}

	for (i = 0; i < count; i++) {
	    _RemoveElement(old_descriptors, lwk_c_domain_object_desc,
		(_AnyPtr) &descriptor);

	    if (descriptor == (_ObjectDesc) _NullObject)
		_Raise(inv_object);

	    _GetValue(descriptor, _P_LinkbaseIdentifier, lwk_c_domain_string,
		(_AnyPtr) &descriptor_id);

	    if (store) {
		/*
		** When Storing an object: If the $LinkbaseIdentifier of this
		** ObjectDescriptor is the same as the $Identifier of the
		** Linkbase, zap it.
		*/
		
		if (_CompareString(descriptor_id,
			_Identifier_of(linkbase)) != 0)
		    _DeleteString(&descriptor_id);
		else {
		    modified = _True;

		    _DeleteString(&descriptor_id);

		    _SetValue(descriptor, _P_LinkbaseIdentifier,
			lwk_c_domain_string, (_AnyPtr) &descriptor_id,
			lwk_c_set_property);
		}
	    }
	    else {
		/*
		** When Retrieving an object: If the $LinkbaseIdentifier of
		** this ObjectDescriptor is null, set it to be the same as the
		** $Identifier of the Linkbase.
		*/
		
		if (descriptor_id != (_String) _NullObject)
		    _DeleteString(&descriptor_id);
		else {
		    modified = _True;

		    _SetValue(descriptor, _P_LinkbaseIdentifier,
			lwk_c_domain_string, &_Identifier_of(linkbase),
			lwk_c_set_property);
		}
	    }

	    _AddElement(new_descriptors, lwk_c_domain_object_desc,
		(_AnyPtr) &descriptor, _False);

	    descriptor = (_ObjectDesc) _NullObject;
	}

        /*
        ** If we modified some ObjectDescriptors, reset the value
        */
	
	if (modified) {
	    switch (domain) {
		case lwk_c_domain_comp_linknet :
		    _SetValue(persistent, _P_Linknets, lwk_c_domain_set,
			(_AnyPtr) &new_descriptors, lwk_c_set_property);
		    break;

		case lwk_c_domain_comp_path :
		    _SetValue(persistent, _P_Paths, lwk_c_domain_list,
			(_AnyPtr) &new_descriptors, lwk_c_set_property);
		    break;
	    }
	}

        /*
        ** Clean up
        */
	
	_Delete(&old_descriptors);
	_Delete(&new_descriptors);
    }

    /*
    **	If any exceptions are raised, clean up, then reraise the exception.
    */
    
    _Exceptions
	_WhenOthers
	    _Delete(&descriptor);
	    _Delete(&old_descriptors);
	    _Delete(&new_descriptors);
	    _DeleteString(&descriptor_id);
	    _Reraise;
    _EndExceptionBlock

    return modified;
    }
#endif /* !MSDOS */


static _Integer  GetInterLinkIds(surrogate, interconnect_ids)
_Surrogate surrogate;

    _Integer **interconnect_ids;

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
    _Integer *ids;
    _Integer count;
    _Boolean set_of_ids;
    _Set volatile interlinks;

    /*
    **  Initialize
    */

    interlinks = (_Set) _NullObject;

    _StartExceptionBlock

    /*
    **  Get either the Set of Links or the Set of Link Ids
    */

    if ((_Boolean) _SetState(surrogate, _StateHaveInterLinks,
	    _StateGet)) {

	/*
	**  The InterLinks are there -- get them
	*/

	_GetValue(surrogate, _P_InterLinks, lwk_c_domain_set,
	    (_AnyPtr) &interlinks);

	set_of_ids = _False;
    }
    else if ((_Boolean) _SetState(surrogate, _StateInterLinksAreIds,
	    _StateGet)) {
	
	/*
	**  The InterLink Ids are there -- get them.  To do this, we
	**  temporarily set the state to make it look like the InterLinks
	**  are there (otherwise they would get Retrieved!).
	*/

	_SetState(surrogate, _StateHaveInterLinks, _StateSet);

	_GetValue(surrogate, _P_InterLinks, lwk_c_domain_set,
	    (_AnyPtr) &interlinks);

	_SetState(surrogate, _StateHaveInterLinks, _StateClear);

	set_of_ids = _True;
    }


    /*
    **  Turn the Set into an array of Ids
    */

    count = InterLinkSetToIds(interlinks, &ids, set_of_ids);

    _Exceptions
	_WhenOthers
	    _Delete(&interlinks);
	    _Reraise;
    _EndExceptionBlock

    /*
    **  Return the array of Ids and the count
    */

    *interconnect_ids = ids;

    return count;
    }


static _Integer  InterLinkSetToIds(interlinks, interconnect_ids, set_of_ids)
_Set interlinks;

    _Integer **interconnect_ids;
 _Boolean set_of_ids;

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
    _IntegerPtr ids;
    _Integer volatile count;
    _Persistent interlink;

    /*
    **  Initialize
    */

    count = 0;
    ids = (_IntegerPtr) _NullObject;

    _StartExceptionBlock

    if (interlinks != (_Set) _NullObject) {
	_GetValue(interlinks, _P_ElementCount, lwk_c_domain_integer,
	    (_AnyPtr) &count);

	if (count > 0) {
	    ids = (_IntegerPtr) _AllocateMem(count * sizeof(_Integer));

	    for (i = 0; i < count; i++)
		if (set_of_ids)
		    _RemoveElement(interlinks, lwk_c_domain_integer,
			&ids[i]);
		else {
		    _RemoveElement(interlinks, lwk_c_domain_persistent,
			&interlink);

		    _GetValue(interlink, _P_Identifier,
			lwk_c_domain_integer, &ids[i]);
		}
	}

	_Delete(&interlinks);
    }

    /*
    **  If any exceptions are raised, clean up then reraise the exception.
    */
    
    _Exceptions
	_WhenOthers
	    _Delete(&interlinks);
	    if (count > 0)
		_FreeMem(ids);
	    _Reraise;
    _EndExceptionBlock

    /*
    **  Return the array of Ids and the count
    */

    *interconnect_ids = ids;

    return count;
    }


static _List  InterLinkIdsToSet(count, ids)
_Integer count;
 _IntegerPtr ids;

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
    _Set volatile set;

    /*
    **  Initialize
    */

    set = (_Set) _NullObject;

    /*
    **  If there are no Ids, return a null Set
    */

    if (count <= 0)
	return set;

    /*
    **  Otherwise, add all the Ids to a Set
    */

    _StartExceptionBlock

    set = (_Set) _CreateSet(_TypeSet, lwk_c_domain_integer, count);

    for (i = 0; i < count; i++)
	_AddElement(set, lwk_c_domain_integer, &ids[i], _True);

    /*
    **  If any exceptions are raised, clean up then reraise the exception.
    */
    
    _Exceptions
	_WhenOthers
	    _Delete(&set);
	    _FreeMem(ids);
	    _Reraise;
    _EndExceptionBlock

    /*
    **  Free the array of Ids
    */

    _FreeMem(ids);

    return set;
    }


static void  AddToCache(linkbase, uid, persistent)
_Linkbase linkbase;
 _Integer uid;

    _Persistent persistent;

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
    _Boolean inserted;
    
    /*
    **  Insert the Object in the Linkbase cache
    */

    inserted = _HashInsert(_HashTable_of(linkbase), _LinkbaseHashSize, uid,
	persistent);

    if (inserted) {
	_CacheCount_of(linkbase)++;

	/*
	**  Be sure the object's Identifier and Linkbase properties are set.
	*/

	_SetValue(persistent, _P_Identifier, lwk_c_domain_integer, &uid,
	    lwk_c_set_property);

	_SetValue(persistent, _P_Linkbase, lwk_c_domain_linkbase,
	    &linkbase, lwk_c_set_property);
    }

    return;
    }


static _Boolean  SearchCache(linkbase, uid, persistent)
_Linkbase linkbase;
 _Integer uid;

    _Persistent *persistent;

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
    **  Search for the object in the Linkbase cache
    */

    return _HashSearch(_HashTable_of(linkbase), _LinkbaseHashSize, uid,
	persistent);
    }


static void  RemoveFromCache(linkbase, uid, persistent)
_Linkbase linkbase;
 _Integer uid;

    _Persistent persistent;

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
    _Boolean found;

    /*
    **  Delete the object from the Linkbase cache
    */

    found = _HashDelete(_HashTable_of(linkbase), _LinkbaseHashSize, uid,
	persistent);

    if (found)
	_CacheCount_of(linkbase)--;

    /*
    **  Clear the object's UID and Linkbase properties.
    */

    uid = 0;

    _SetValue(persistent, _P_Identifier, lwk_c_domain_integer, &uid,
	lwk_c_set_property);

    linkbase = (_Linkbase) _NullObject;

    _SetValue(persistent, _P_Linkbase, lwk_c_domain_linkbase, &linkbase,
	lwk_c_set_property);

    return;
    }
