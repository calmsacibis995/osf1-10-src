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
**	Support routines for Service Initialization
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
**  Creation Date: 13-Sep-89
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
#include "apply.h"
#else /* !MSDOS */
#include "his_include.h"
#include "lwk_abstract_objects.h"
#include "his_apply.h"
#endif

/*
**  Macro Definitions
*/

#ifdef VMS
#define _DeleteFile delete
#else
#define _DeleteFile unlink
#endif /* VMS */

/*
**  Type Definitions
*/

/*
**  Forward Routine Declarations
*/

/*
**  Static Data Definitions
*/

static _Boolean Initialized = _False;

/*
**  Global Data Definitions
*/

/*
**  External Data Declarations
*/


#ifdef MSWINDOWS
int _EntryPt  LibMain(hInstance, wDataSeg, wHeapSize, lpszCmdLine)
HANDLE hInstance;
 WORD wDataSeg;
 WORD wHeapSize;

    LPSTR lpszCmdLine;

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
    ** This routine is called once per DLL instance.  It may do any
    ** one-time-only initialization it wishes, then returns a non-zero value to
    ** indicate success.
    */
    
    /*
    ** Initialize other modules
    */

    LwkLbInitialize();

    return 1;
    }

#endif /* MSWINDOWS */


void  LwkOpInitialize(hyperinvoked, operation, surrogate)
_BooleanPtr hyperinvoked;

    _StringPtr volatile operation;
 _SurrogatePtr volatile surrogate;

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
#ifndef MSDOS
    _Integer len;
    _Integer size;
    _Boolean is_oid;
    FILE volatile *fp;
    _ObjectId volatile oid;
    char volatile *file_name;
    _Boolean was_initialized;
    _VaryingString volatile encoding;
#endif /* !MSDOS */

    /*
    ** Initialize
    */

    *hyperinvoked = _False;
    *operation = (_String) _NullObject;
    *surrogate = (_Surrogate) _NullObject;

#ifndef MSDOS

    /*
    **  Don't do this more than once
    */

    _BeginCriticalSection

    was_initialized = Initialized;
    Initialized = _True;

    _EndCriticalSection

    if (was_initialized)
	return;

    /*
    **  Initialize other modules
    */

    LwkLbInitialize();
    LwkOpDWInitialize();

    /*
    **  Get any HyperInvocation arguments -- if there aren't any, return.
    */

    file_name = (_CharPtr) getenv(_InitializeEnvironmentVariable);

    if ((file_name == NULL) || (strlen((char *) file_name) == 0))
	return;

    fp = fopen((char *) file_name, "r");

    if (fp == NULL)
	return;

    oid = (_ObjectId) _NullObject;

    _StartExceptionBlock

    /*
    ** Read the Operation identifier
    */

    if (fread(&len, sizeof(_Integer), 1, (FILE *) fp) < 1)
	_Raise(failure);

    *operation = (_String) _AllocateMem(len);

    if (fread(*operation, sizeof(char), len, (FILE *) fp) < len)
	_Raise(failure);

    /*
    ** Read the Object Identifier or Surrogate encoding and Import it
    */

    if (fread(&is_oid, sizeof(_Boolean), 1, (FILE *) fp) < 1)
	_Raise(failure);

    if (fread(&size, sizeof(_Integer), 1, (FILE *) fp) < 1)
	_Raise(failure);

    encoding = (_VaryingString) _AllocateMem(size);

    if (fread(encoding, sizeof(char), size, (FILE *) fp) < size)
	_Raise(failure);

    if (is_oid) {
	oid = (_ObjectId) _Import(_TypeObjectId, encoding);
	*surrogate = (_Surrogate) _Retrieve(oid);
	_Delete(&oid);
    }
    else
	*surrogate = (_Surrogate) _Import(_TypeSurrogate, encoding);

    /*
    ** Clean up
    */

    _DeleteEncoding(&encoding);

    fclose((FILE *) fp);

    _DeleteFile((char *) file_name);

    /*
    ** This was a HyperInvocation!
    */

    *hyperinvoked = _True;

    /*
    **  If an exception is raised, clean up then reraise it.
    */

    _Exceptions
	_WhenOthers
	    fclose((FILE *) fp);
	    _DeleteFile((char *) file_name);

	    _Delete(&oid);
	    _DeleteEncoding(&encoding);
	    _DeleteString(operation);
	    _Delete(surrogate);

	    _Reraise;
    _EndExceptionBlock

#endif /* !MSDOS */

    return;
    }
