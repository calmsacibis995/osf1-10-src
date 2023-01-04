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
**	LinkWorks Services
**
**  Version: X0.1
**
**  Abstract:
**	Support routines for Surrogate Apply operation
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
#include "surrogte.h"
#include "regstry.h"
#include "apply.h"
#else /* !MSDOS */
#include "his_include.h"
#include "lwk_surrogate.h"
#include "his_registry.h"
#include "his_apply.h"
#endif

#ifdef VMS
#include <descrip.h>
#include <clidef.h>
#include <lnmdef.h>
#endif /* VMS */

/*
**  Macro Definitions
*/

#ifdef VMS
#define _ScratchDirectory "SYS$SCRATCH:"
#define _ScratchPrefix "LWK_"
#define _ScratchFileType ".TMP"
#define _LogicalNameTable "LNM$PROCESS"
#else
#define _ScratchPrefix "lwk_"
#endif /* VMS */

/*
**  Type Definitions
*/

/*
**  Forward Routine Declarations
*/

_DeclareFunction(static _String GenerateTemporaryFileName, (void));
_DeclareFunction(static void AugmentEnvironment, (char *variable, char *value));
_DeclareFunction(static void ResetEnvironment, (char *variable));
_DeclareFunction(static void InvokeMethod, (_String method));
_DeclareFunction(static _Boolean ExportObjectId,
    (_Persistent volatile persistent, _VaryingString volatile *encoding,
    _Integer *size));

/*
**  Static Data Definitions
*/

/*
**  Global Data Definitions
*/

/*
**  External Data Declarations
*/

#ifndef VMS
extern char **environ;
#endif


void  LwkApply(surrogate, operation)
_Surrogate surrogate;
 _String operation;

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
    FILE *fp;
    _Integer len;
    _Integer volatile size;
    _String volatile method;
    _Boolean volatile is_oid;
    _String volatile file_name;
    _VaryingString volatile encoding;

    /*
    ** Initialize
    */

    method = (_String) _NullObject;
    encoding = (_VaryingString) _NullObject;

    _StartExceptionBlock

    /*
    ** Generate a temporary file name
    */

    file_name = GenerateTemporaryFileName();

    /*
    ** Open the file
    */

    fp = fopen((char *) file_name, "w");

    if (fp == NULL)
	_Raise(invocation_error);

    /*
    ** Write out the Operation identifier
    */

    len = _LengthString(operation) + 1;

    fwrite(&len, sizeof(_Integer), 1, fp);
    fwrite(operation, sizeof(char), len, fp);

    /*
    ** Export an Object Identifier for the Surrogate or the Surrogate itself
    ** and write it out.
    */

    is_oid = ExportObjectId(surrogate, &encoding, &size);

    fwrite((char *) &is_oid, sizeof(_Boolean), 1, fp);
    fwrite((char *) &size, sizeof(_Integer), 1, fp);
    fwrite(encoding, sizeof(char), size, fp);

    /*
    ** Close the temporary file
    */

    if (fclose(fp) == EOF)
	_Raise(invocation_error);

    /*
    ** Find the method associated with the Surrogate Subtype and operation
    */

    method = LwkRegOperationMethod(_SurrogateSubType_of(surrogate), operation);

    /*
    ** Save the temporary file name in the environment so that the Initialize
    ** function in the invoked application can find it.
    */

    AugmentEnvironment(_InitializeEnvironmentVariable, (char *) file_name);

    /*
    ** Invoke the method
    */

    InvokeMethod(method);

    /*
    ** Clean up
    */

    ResetEnvironment(_InitializeEnvironmentVariable);

    _DeleteString(&file_name);
    _DeleteString(&method);
    _DeleteEncoding(&encoding);

    /*
    **  If an exception is raised, clean up then reraise it.
    */

    _Exceptions
	_WhenOthers
	    _DeleteString(&file_name);
	    _DeleteString(&method);
	    _DeleteEncoding(&encoding);

	    ResetEnvironment(_InitializeEnvironmentVariable);

	    _Reraise;
    _EndExceptionBlock

    return;
    }


static _String  GenerateTemporaryFileName()
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
    char *name;
    _String file_name;

#ifdef VMS
    name = (char *) malloc(sizeof(_ScratchDirectory) + sizeof(_ScratchPrefix)
	+ L_tmpnam + sizeof(_ScratchFileType) + 1);

    sprintf(name, "%s%s%s%s", _ScratchDirectory, _ScratchPrefix, tmpnam(NULL),
	_ScratchFileType);
#else
    name = tempnam(NULL, _ScratchPrefix);
#endif /* VMS */

    file_name = _CopyString((_String) name);

    free(name);

    return file_name;
    }


static void  AugmentEnvironment(variable, value)
char *variable;
 char *value;

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
#ifdef VMS
    int status;
    struct dsc$descriptor_s d_name;
    struct dsc$descriptor_s d_table;
    struct {
	unsigned short int length;
	unsigned short int code;
	unsigned long  int buffer_address;
	unsigned long  int return_address;
	unsigned long  int end_of_list;
    } item_list;

    d_table.dsc$w_length = strlen(_LogicalNameTable);
    d_table.dsc$b_dtype = DSC$K_DTYPE_T;
    d_table.dsc$b_class = DSC$K_CLASS_S;
    d_table.dsc$a_pointer = _LogicalNameTable;

    d_name.dsc$w_length = strlen(variable);
    d_name.dsc$b_dtype = DSC$K_DTYPE_T;
    d_name.dsc$b_class = DSC$K_CLASS_S;
    d_name.dsc$a_pointer = variable;

    item_list.length = strlen(value);
    item_list.code = LNM$_STRING;
    item_list.buffer_address = value;
    item_list.return_address = 0;
    item_list.end_of_list = 0;

    status = sys$crelnm(0, &d_table, &d_name, 0, &item_list);

    if ((status & 1) == 0)
	_Raise(invocation_error);

#else

    char *env_variable;

    env_variable = (char *) malloc(strlen(variable) + sizeof(char) +
	                           strlen(value) + sizeof(char));

    sprintf(env_variable, "%s=%s", variable, value);
    putenv(env_variable);
    
#endif /* VMS */

    return;
    }


static void  InvokeMethod(method)
_String method;

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
#ifdef VMS
    int status;
    int flags;
    struct dsc$descriptor_s d_method;

    flags = CLI$M_NOWAIT;

    d_method.dsc$w_length = strlen(method);
    d_method.dsc$b_dtype = DSC$K_DTYPE_T;
    d_method.dsc$b_class = DSC$K_CLASS_S;
    d_method.dsc$a_pointer = method;

    /*
    ** Execute the method in a child process
    */

    status = lib$spawn(&d_method, 0, 0, &flags, 0, 0, 0, 0, 0, 0, 0, 0);

    if ((status & 1) == 0)
	_Raise(invocation_error);

#else
    int i;
    char *tokens;
    char **argv;

    /*
    ** Parse the method into individual tokens (separated by white space).
    */

    tokens = (char *) malloc(strlen(method) + 1);
    strcpy(tokens, method);

    i = 0;
    argv = (char **) malloc(10 * sizeof(char *));

    _BeginCriticalSection

    argv[i] = strtok(tokens, " \t\n");

    while (argv[i] != NULL) {
	if ((++i % 10) == 0)
	    argv = (char **) realloc(argv, (i + 10) * sizeof(char *));

	argv[i] = strtok(NULL, " \t\n");
    }

    _EndCriticalSection

    /*
    ** Execute the method in a child process
    */

    if (vfork() == 0) {
	execvp(argv[0], argv);
	fprintf(stderr, "?LWK Application invocation failed: %s\n", method);
	_exit(0);
    }

    /*
    ** Clean up
    */

    free(argv);
    free(tokens);
#endif /* VMS */

    return;
    }


static _Boolean  ExportObjectId(persistent, encoding, size)
_Persistent volatile persistent;

    _VaryingString volatile *encoding;
 _Integer *size;

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
    _Boolean is_oid;
    _ObjectId volatile oid;

    /*
    ** Initialize
    */

    is_oid = _True;
    oid = (_ObjectId) _NullObject;

    _StartExceptionBlock

    oid = (_ObjectId) _GetObjectId(persistent);

    *size = (_Integer) _Export(oid, _False, (_VaryingStringPtr) encoding);

    _Delete(&oid);

    _Exceptions
	_WhenOthers
	    _Delete(&oid);
	    is_oid = _False;
	    *size = (_Integer) _Export(persistent, _False,
		(_VaryingStringPtr) encoding);
    _EndExceptionBlock

    return is_oid;
    }

static void  ResetEnvironment(variable)
char *variable;

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
    if (variable == (char *) 0)
	return;

#ifdef VMS
    {
    int status;
    struct dsc$descriptor_s d_name;
    struct dsc$descriptor_s d_table;

    /*
    ** Deassign the logical name
    */

    d_table.dsc$w_length = strlen(_LogicalNameTable);
    d_table.dsc$b_dtype = DSC$K_DTYPE_T;
    d_table.dsc$b_class = DSC$K_CLASS_S;
    d_table.dsc$a_pointer = _LogicalNameTable;

    d_name.dsc$w_length = strlen(variable);
    d_name.dsc$b_dtype = DSC$K_DTYPE_T;
    d_name.dsc$b_class = DSC$K_CLASS_S;
    d_name.dsc$a_pointer = variable;

    status = sys$dellnm(&d_table, &d_name, 0);

    if ((status & 1) == 0)
	_Raise(invocation_error);
    }
    
#else
    {
    char *env_variable;

    env_variable = (char *) malloc(strlen(variable) + sizeof(char) +
	                           sizeof(char));

    sprintf(env_variable, "%s=", variable);
    putenv(env_variable);
    }
    
#endif /* VMS */

    return;
    }

