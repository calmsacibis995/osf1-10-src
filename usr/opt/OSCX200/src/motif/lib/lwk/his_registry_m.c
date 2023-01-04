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
** COPYRIGHT (c) 1990 BY
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
**	MEMEX HyperInformation Services
**
**  Version: V1.0
**
**  Abstract:
**	Support routines for the MEMEX Hyperapplication Registry.
**
**  Keywords:
**	Registry, DRM
**
**  Author:
**	Doug Rayner, MEMEX Project
**	W. Ward Clark, MEMEX Project
**
**  Creation Date: 16-Mar-1990
**
**  Modification History:
**  X0.10-1 WWC  29-Mar-90  use DRM files as the Registry database
**  X0.11   LG	 19-Jul-90  Return DDIF to be platform independent
**--
*/


/*
**  Include Files
*/
#include "his_include.h"
#include "his_registry.h"

#ifdef VMS

#include <Xm/XmP.h>
#include <Mrm/MrmPublic.h>

#else
#include <Xm/XmP.h>
#include <Mrm/MrmPublic.h>
#endif

/*
**  Macro Definitions
*/

#ifdef VMS
#define _MaxFileNameLength 256
#else
#ifdef ultrix
#define _MaxFileNameLength NAME_MAX
#else
#define _MaxFileNameLength 256
#endif
#endif

#define _RegistryDrmFileName "lwk_reg_%s"

/*
** Type Registry DRM resource names
*/

#define _SuperTypeId	"LWK_SURROGATE_SUPER_TYPE_IDENTIFIER"
#define _TypeName	"LWK_SURROGATE_SUB_TYPE_NAME"
#define _Icon17x17	"LWK_SURROGATE_ICON_17X17"
#define _Icon32x32	"LWK_SURROGATE_ICON_32X32"
#define _OpNames	"LWK_OPERATION_NAMES"
#define _OpIdentifiers	"LWK_OPERATION_IDENTIFIERS"

#if VMS
#define _OpMethods	"LWK_VMS_INVOKE_COMMANDS"
#else
#define _OpMethods	"LWK_UNIX_INVOKE_COMMANDS"
#endif

/*
**  Type Definitions
*/

/*
** Operation entry in the Hyperapplication Registry
*/

typedef struct __OperationRegistry {
	struct __OperationRegistry *next;
	char			   *identifier;
	_DDIFString		   name;
	char			   *method;
    } _OperationRegistryInstance, *_OperationRegistry;

/*
** Type entry in the Hyperapplication Registry
*/

typedef struct __TypeRegistry {
	struct __TypeRegistry	*next;
	struct __TypeRegistry	*super_type;
	char			*identifier;
	_DDIFString		name;
	Pixmap			icon_17x17;
	Pixmap			icon_32x32;
	_OperationRegistry	operations;
	int			operation_count;
    } _TypeRegistryInstance, *_TypeRegistry;

/*
**  Forward Routine Declarations
*/

_DeclareFunction(static _TypeRegistry FindType, (_String type_id));
_DeclareFunction(static _TypeRegistry LoadType, (_String type_id));
_DeclareFunction(static void NormalizeFileSpec, (char *file_spec));
_DeclareFunction(static int SizeofDrmTable, (_AnyPtr *drm_table));
_DeclareFunction(static void RegistryWarningHandler, (String message));

/*
**  Static Data Definitions
*/

static _TypeRegistry	Types = (_TypeRegistry) 0;

static XtAppContext	AppContext = (XtAppContext) 0;

/*
**  Global Data Definitions
*/

/*
**  External Data Declarations
*/


_DDIFString  LwkRegTypeName(type_id)
_String type_id;

/*
**++
**  Functional Description:
**
**	Provides a user-visible type name (ddif string) for a
**	specific Surrogate Sub Type.
**
**  Keywords:
**	Registry
**
**  Formal Parameters:
**
**      type_id :
**          Surrogate Sub Type string.
**
**  Result:
**
**	type_name :
**	    Type name (ddif string).
**
**  Exceptions:
**
**      not_registered :
**          The requested type is not registered on the current host system
**	    or the registration is invalid.
**--
*/
{
    _TypeRegistry   type;

    /*
    ** Find the type definition.
    */

    type = FindType(type_id);

    /*
    ** Return the type name.
    */

    return _CopyDDIFString(type->name);
}


int  LwkRegTypeOperations(type_id, operation_ids)
_String type_id;
 _String **operation_ids;

/*
**++
**  Functional Description:
**
**	Provides a list of supported operations for a
**	specific Surrogate Sub Type.
**
**  Keywords:
**	Registry
**
**  Formal Parameters:
**
**      type_id :
**          Surrogate Sub Type string.
**
**      operation_ids :
**          List (vector) of operation identifiers.
**
**  Result:
**
**      operation_count :
**          Number of operations.
**
**  Exceptions:
**
**      not_registered :
**          The requested type is not registered on the current host system
**	    or the registration is invalid.
**--
*/
{
    _TypeRegistry	type;
    _OperationRegistry	operation;
    int			count;
    int			op_index;

    /*
    ** Find the type definition.
    */

    type = FindType(type_id);

    /*
    ** Count the number of operations
    */

    count = 0;

    while (type != (_TypeRegistry) 0) {
	count += type->operation_count;
	type = type->super_type;
    }

    /*
    ** Return a vector of operation identifier pointers.
    */

    *operation_ids =
	(_String *) _AllocateMem(count * sizeof(_String));

    count = 0;

    type = FindType(type_id);

    while (type != (_TypeRegistry) 0) {
	operation = type->operations;

	for (op_index = 0; op_index < type->operation_count; op_index++) {
	    (*operation_ids)[count] =
		_CopyString((_String) operation->identifier);

	    count++;

	    operation = operation->next;
	}

	type = type->super_type;
    }

    /*
    ** Return the number of operations.
    */

    return count;
}


_String  LwkRegTypeDefaultOperation(type_id)
_String type_id;

/*
**++
**  Functional Description:
**
**	Provides the default operation for a specific Surrogate Sub Type.
**
**  Keywords:
**	Registry
**
**  Formal Parameters:
**
**      type_id :
**          Surrogate Sub Type string.
**
**  Result:
**
**      default_operation :
**          Default operation identifier.
**
**  Exceptions:
**
**      not_registered :
**          The requested type is not registered on the current host system
**	    or the registration is invalid.
**--
*/
{
    _TypeRegistry   type;

    /*
    ** Find the type definition.
    */

    type = FindType(type_id);

    /*
    ** Return the identifier of the first entry in the operation table.
    */

    return _CopyString((_String) type->operations->identifier);
}


_Boolean  LwkRegIsSuperType(type_id, supertype_id)
_String type_id;
 _String supertype_id;

/*
**++
**  Functional Description:
**
**	Determines if one type identifier is a supertype of another
**
**  Keywords:
**	Registry
**
**  Formal Parameters:
**
**      type_id :
**          Surrogate Sub Type string.
**       
**      supertype_id :
**          Surrogate Sub Type string.
**
**  Result:
**
**      True/False
**
**  Exceptions:
**
**      not_registered :
**          The requested type is not registered on the current host system
**	    or the registration is invalid.
**--
*/
{
    _Boolean		match;
    _TypeRegistry	type;
    _OperationRegistry	operation;

    /*
    ** Find the type definition.
    */

    type = FindType(type_id);

    match = _False;

    while (type != (_TypeRegistry) 0) {
	if (strcmp(type->identifier, supertype_id) == 0) {
	    match = _True;
	    break;
	}

	type = type->super_type;
    }

    return match;
}


_DDIFString  LwkRegOperationName(type_id, operation_id)
_String type_id;
 _String operation_id;

/*
**++
**  Functional Description:
**
**	Translates an operation identifier into a user-visible operation
**	name (DDIF string).
**
**  Keywords:
**	Registry
**
**  Formal Parameters:
**
**      type_id :
**          Surrogate Sub Type string.
**
**      operation_id :
**          Operation identifier string.
**
**  Result:
**
**	operation_name :
**	    Operation name (compound string).  If the operation identifier
**	    is not registered, the identifier is simply converted into a
**	    name.
**
**  Exceptions:
**
**      not_registered :
**          The requested type is not registered on the current host system
**	    or the registration is invalid.
**--
*/
{
    _DDIFString		name;
    _TypeRegistry	type;
    _OperationRegistry	operation;

    /*
    ** Find the type definition.
    */

    type = FindType(type_id);

    /*
    ** Search the list of operations for the specified identifier,
    ** and extract the corresponding name string.
    */

    for (operation = type->operations;
	    operation != (_OperationRegistry) 0;
	    operation = operation->next) {
	if (strcmp(operation_id, operation->identifier) == 0) {
	    name = _CopyDDIFString(operation->name);
	    break;
	}
    }

    /*
    ** If the specified identifier is not registered, convert the
    ** identifier string into a name string.
    */

    if (operation == (_OperationRegistry) 0)
	name = _StringToDDIFString((_String) operation_id);

    /*
    ** Return the operation name.
    */

    return name;
}


_String  LwkRegOperationType(type_id, operation_id)
_String type_id;
 _String operation_id;

/*
**++
**  Functional Description:
**
**	Translates an operation identifier into a the type identifier which
**	provides the associated method.
**
**  Keywords:
**	Registry
**
**  Formal Parameters:
**
**      type_id :
**          Surrogate Sub Type string.
**       
**      operation_id :
**          Operation identifier string.
**
**  Result:
**
**	type_id :
**	    Type identifier (string).
**
**  Exceptions:
**
**      not_registered :
**          The requested type is not registered on the current host system
**	    or the registration is invalid.
**--
*/
{
    _TypeRegistry	type;
    _OperationRegistry	operation;

    /*
    ** Find the type definition.
    */

    type = FindType(type_id);

    /*
    ** Search the list of operations for the specified identifier,
    ** and extract the corresponding name string.  If the operation is not
    ** defined for this type, try its supertype(s).
    */

    while (type != (_TypeRegistry) 0) {
	for (operation = type->operations;
		operation != (_OperationRegistry) 0;
		operation = operation->next) {
	    if (strcmp(operation_id, operation->identifier) == 0) {
		break;
	    }
	}

	if (operation != (_OperationRegistry) 0)
	    break;

	type = type->super_type;
    }

    if (type == (_TypeRegistry) 0)
	_Raise(not_registered);

    /*
    ** Return the type identifier.
    */

    return _CopyString(type->identifier);
}


_String  LwkRegOperationMethod(type_id, operation_id)
_String type_id;
 _String operation_id;

/*
**++
**  Functional Description:
**
**	Provides a host-specific method (command) for a specific
**	operation identifier.
**
**  Keywords:
**	Registry
**
**  Formal Parameters:
**
**      type_id :
**          Surrogate Sub Type string.
**
**      operation_id :
**          Operation identifier string.
**
**  Result:
**
**	operation_method :
**	    Operation method string.
**
**  Exceptions:
**
**      not_registered :
**          The requested type is not registered on the current host system
**	    or the registration is invalid.
**--
*/
{
    _String		method;
    _TypeRegistry	type;
    _OperationRegistry	operation;

    /*
    ** Find the type definition.
    */

    type = FindType(type_id);

    method = (_String) _NullObject;

    /*
    ** Search the list of operations for the specified identifier,
    ** and extract the corresponding method string.
    */

    while (type != (_TypeRegistry) 0) {
	for (operation = type->operations;
		operation != (_OperationRegistry) 0;
		operation = operation->next) {
	    if (strcmp(operation_id, operation->identifier) == 0) {
		method = _CopyString((_String) operation->method);
		break;
	    }
	}

	if (operation != (_OperationRegistry) 0)
	    break;

	type = type->super_type;
    }

    /*
    ** If the specified identifier is not registered,
    ** convert the identifier string into a method string.
    */

    if (operation == (_OperationRegistry) 0)
	method = _CopyString(operation_id);

    /*
    ** Return the method string.
    */

    return method;
}


static _TypeRegistry  FindType(type_id)
_String type_id;

/*
**++
**  Functional Description:
**
**	Returns an in-memory type definition, loading it from a
**	type-dependent DRM file, if necessary.
**
**  Keywords:
**	Registry
**
**  Formal Parameters:
**
**      type_id :
**          Surrogate Sub Type string.
**
**  Implicit Input Parameters:
**
**      Types :
**          In-memory linked list of type information.
**
**  Implicit Output Parameters:
**
**      Types :
**	    List is updated if a new type is loaded.
**
**  Result:
**
**      type :
**          Entry in the in-memory type list.
**
**  Exceptions:
**
**      not_registered :
**          The requested type is not registered on the current host system
**	    or the registration is invalid.
**--
*/
    {
    _TypeRegistry   type;

    /*
    ** Block another thread from doing a simultaneous search
    ** (and possible extension) of the list of loaded types.
    */

    _BeginCriticalSection

    /*
    ** Search the list of loaded types for a match.
    */

    type = Types;

    while (type != (_TypeRegistry) 0) {
	if (strcmp(type_id, type->identifier) == 0)
	    break;

	type = type->next;
    }

    /*
    ** No match was found, attempt to load the requested type.
    */

    if (type == (_TypeRegistry) 0) {
        type = LoadType(type_id);

        /*
        ** Add a new type to the beginning of the list.
        */
        if (type != (_TypeRegistry) 0) {
            type->next = Types;
	    Types = type;
        }
    }

    /*
    ** Allow other threads to proceed.
    */

    _EndCriticalSection

    /*
    ** Raise an exception if the requested type is unknown.
    */

    if (type == (_TypeRegistry) 0)
	_Raise(not_registered);

    /*
    ** Return the requested type entry.
    */

    return type;
    }


static _TypeRegistry  LoadType(type_id)
_String type_id;

/*
**++
**  Functional Description:
**
**	Fetches type information from a type-specific DRM file.
**
**  Keywords:
**	DRM
**
**  Formal Parameters:
**
**	type_id :
**	    Surrogate Sub Type string.
**
**  Result:
**
**	type :
**	    Entry to be added to the in-memory type list.
**
**      0 :
**          The requested type is not registered on the current host system
**	    or the registration is invalid.
**
**  Exceptions:
**
**      None
**--
*/
{
    String		drm_file_name;
    char		drm_file_name_buffer[_MaxFileNameLength];

    MrmHierarchy	hierarchy;
    MrmCode		drm_type;

    XmString		type_name;

    String		super_type_id;

    int			op_id_count;
    String		*op_id_table;

    int			op_name_count;
    XmString	*op_name_table;

    int			op_method_count;
    String		*op_method_table;

    _TypeRegistry	type;
    _OperationRegistry	operation;
    int			op_index;

    XtErrorHandler	orig_handler;


    /*
    ** Create an application context if we don't have one for
    ** this module.
    */

    if (AppContext == (XtAppContext) 0)
	AppContext = XtCreateApplicationContext();

    /*
    ** Compute the name of the DRM file.
    */

    sprintf(drm_file_name_buffer, _RegistryDrmFileName, type_id);

    NormalizeFileSpec(drm_file_name_buffer);


    /*
    ** Enable our own Warning Hanlder, because we expect
    ** warning messages concerning the lack of an
    ** annotation capability.  This error is handled
    ** properly from a coding point of view, but the
    ** toolkit insists on issuing warning messages as
    ** well as returning a status code.
    */

    orig_handler = XtAppSetWarningHandler(AppContext,
	(XtErrorHandler) RegistryWarningHandler);


    /*
    ** Open the DRM hierarchy.
    */

    drm_file_name = drm_file_name_buffer;


    if (MrmOpenHierarchy((MrmCount) 1, &drm_file_name,
	    (MrmOsOpenParamPtr *) 0, &hierarchy) != MrmSUCCESS) {
	XtAppSetWarningHandler(AppContext, (XtErrorHandler) orig_handler);
	return (_TypeRegistry) 0;
    }
    

    /*
    ** Fetch the DRM values for all type attributes.
    */

    /*
    ** Supertype identifier
    */


    if (MrmFetchLiteral(hierarchy, _SuperTypeId, (Display *) 0,
	    &super_type_id, &drm_type) != MrmSUCCESS) {
	XtAppSetWarningHandler(AppContext, (XtErrorHandler) orig_handler);
	super_type_id = (String) _NullObject;
    }
    

    /*
    ** Restore the original Warning Handler.  End of annotation
    ** related messages from Mrm.
    */

    XtAppSetWarningHandler(AppContext, (XtErrorHandler) orig_handler);


    /*
    ** Type name
    */

    if (MrmFetchLiteral(hierarchy, _TypeName, (Display *) 0,
	    (caddr_t *) &type_name, &drm_type) != MrmSUCCESS)
	return (_TypeRegistry) 0;

    /*
    ** Icons	** not yet implemented **
    */

    /*
    ** Tables of operation identifiers, names and invocation methods.
    */

    if (MrmFetchLiteral(hierarchy, _OpIdentifiers, (Display *) 0,
	    (caddr_t *) &op_id_table, &drm_type) == MrmSUCCESS)
	op_id_count = SizeofDrmTable(op_id_table);
    else
	op_id_count = 0;

    if (MrmFetchLiteral(hierarchy, _OpNames, (Display *) 0,
	    (caddr_t *) &op_name_table, &drm_type) == MrmSUCCESS)
	op_name_count = SizeofDrmTable(op_name_table);
    else
	op_name_count = 0;

    if (MrmFetchLiteral(hierarchy, _OpMethods, (Display *) 0,
	    (caddr_t *) &op_method_table, &drm_type) == MrmSUCCESS)
	op_method_count = SizeofDrmTable(op_method_table);
    else
	op_method_count = 0;

    /*
    ** Return an error code to the caller if the type registration
    ** is incomplete or inconsistent.
    */

    if (op_id_count == 0
	    || op_id_count > op_name_count
	    || op_id_count > op_method_count)
        return (_TypeRegistry) 0;

    /*
    ** Allocate a new in-memory type entry and fill it in with
    ** values fetched from the DRM file.
    */

    type = (_TypeRegistry) malloc(sizeof(_TypeRegistryInstance));

    /*
    ** Supertype
    */

    if (super_type_id != (_String) _NullObject)
	type->super_type = FindType(super_type_id);
	
    else {
    
	if (strcmp(type_id, _AnyTypeIdentifier) == 0)
	    type->super_type = (_TypeRegistry) 0;
	    
	else {
	    _StartExceptionBlock
	    
	    type->super_type = FindType(_AnyTypeIdentifier);

            /*
	    ** If the SuperType is not registered, just clear the super_type
	    ** field 
	    */

	    _Exceptions
		_When(not_registered)
		    type->super_type = (_TypeRegistry) 0;

		_WhenOthers
		    _Reraise;

	    _EndExceptionBlock
	}
    }

    /*
    ** Type identifier
    */

    type->identifier = (char *) malloc(strlen(type_id)+1);
    strcpy(type->identifier, type_id);

    /*
    ** Type name
    */

    type->name = _CStringToDDIFString((_CString) type_name);

    /*
    ** Icons	** not yet implemented **
    */

    type->icon_17x17 = (Pixmap) 0;
    type->icon_32x32 = (Pixmap) 0;

    /*
    ** Operation count, identifiers, names and invocation methods.
    */

    type->operation_count = op_id_count;

    type->operations = (_OperationRegistry) 0;

    for (op_index = op_id_count-1; op_index >= 0; op_index--) {
        operation =
	    (_OperationRegistry) malloc(sizeof(_OperationRegistryInstance));

	operation->next = type->operations;
	type->operations = operation;

        operation->identifier =
	    (char *) malloc(strlen(op_id_table[op_index])+1);
        strcpy(operation->identifier, op_id_table[op_index]);

        operation->name = _CStringToDDIFString(
	    (_CString) op_name_table[op_index]);

        operation->method =
	    (char *) malloc(strlen(op_method_table[op_index])+1);
	strcpy(operation->method, op_method_table[op_index]);
    }

    /*
    ** Close the DRM hierarchy.
    */

    MrmCloseHierarchy(hierarchy);

    /*
    ** Return the new type entry to the caller.
    */

    return type;
}


static void  NormalizeFileSpec(file_spec)
char *file_spec;

/*
**++
**  Functional Description:
**
**	Translates invalid file-spec characters to '_'.
**
**  Keywords:
**      None
**
**  Formal Parameters:
**
**      file_spec :
**          File specification string.
**
**  Result:
**
**      None
**
**  Exceptions:
**
**      None
**--
*/
{
    int spec_index;

    for (spec_index = 0; spec_index < strlen(file_spec); spec_index++) {
        switch (file_spec[spec_index]) {
            case ' ':
                file_spec[spec_index] = '_';
                break;
        }
    }

    return;
}


static int  SizeofDrmTable(drm_table)
_AnyPtr *drm_table;

/*
**++
**  Functional Description:
**
**	Determines the size of a DRM table returned by MrmFetchLiteral
**	by scanning the table vector for a zero address.
**
**  Keywords:
**	DRM
**
**  Formal Parameters:
**
**      drm_table :
**          A pointer to the DRM table.
**
**  Result:
**
**      table_count :
**          Number of table entries.
**
**  Exceptions:
**
**      None
**--
*/
{
    int	table_size;

    /*
    ** Loop thru the DRM text table, looking for a zero address.
    */

    table_size = 0;

    while (drm_table[table_size] != (_AnyPtr) 0)
        table_size++;

    /*
    ** Return the length of the table.
    */

    return table_size;
}

static void  RegistryWarningHandler(message)
String message;

{
    return;
}

