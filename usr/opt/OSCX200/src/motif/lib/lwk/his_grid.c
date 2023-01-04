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
** COPYRIGHT (c) 1988, 1989, 1990 BY
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
**	Grid File support routines
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
**  Creation Date: 15-Nov-89
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
#include "grdfile.h"
#else /* !MSDOS */
#include "his_include.h"
#include "lwk_abstract_objects.h"
#include "his_grid_file.h"
#endif

/*
**  Type Definitions
*/

/*
#undef _DefaultTrace
#define _DefaultTrace (_TraceReadMask | _TraceWriteMask | _TraceStoreMask \
    | _TraceQueryMask | _TraceQueryResultsMask | _TraceTransactMask \
    | _TraceLockMask)
*/

/*
**  Macro Definitions
*/

/*
**  Forward Routine Declarations
*/

/*
**  Static Data Definitions
*/

_Global _OSFile LwkGridTraceFile = (_OSFile) 0;
_Global _Integer LwkGridTrace = _DefaultTrace;
_Global _Integer LwkGridCacheSize  = _DefaultCacheSize;
_Global _Integer LwkGridBucketSize = _DefaultBucketSize;
_Global _Integer LwkGridExtensionBuckets  = _DefaultExtensionBuckets;
_Global _Boolean LwkGridRecordMode = _DefaultRecordMode;
_Global _Boolean LwkGridLock = _DefaultLock;


void  LwkGridInitialize()
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
    _CharPtr value;

    /*
    **  Initialize Trace flags
    */

    value = (_CharPtr) getenv("HIS_TRACE");

    if (value != NULL)
	LwkGridTrace = atoi(value);

    if (LwkGridTrace != 0) {
	value = (_CharPtr) getenv("HIS_TRACE_FILE");

	if (value == NULL) {
#ifdef MSWINDOWS

	    value = "TRACE.LST";

#else /* !MSWINDOWS */

	    LwkGridTraceFile = stdout;

#endif /* MSWINDOWS */
	}

	if (value != NULL)
	    LwkGridTraceFile = fopen(value, "w");

	if (LwkGridTraceFile == (_OSFile) 0) {

#ifndef MSWINDOWS
	    printf("?Could not open Trace file: %s\n?Trace cancelled\n",
		value);
#endif /* !MSWINDOWS */

	    LwkGridTrace = 0;
	}
    }

    /*
    **  Initialize Record Mode flag
    */

    value = (_CharPtr) getenv("HIS_RECORD_MODE");

    if (value != NULL)
	LwkGridRecordMode = atoi(value);

    /*
    **  Initialize Lock flag
    */

    value = (_CharPtr) getenv("HIS_LOCK");

    if (value != NULL)
	LwkGridLock = atoi(value);

    /*
    **  Initialize Bucket size
    */

    value = (_CharPtr) getenv("HIS_BUCKET_SIZE");

    if (value != NULL) {
	size = atoi(value);

	if (size > 0)
	    LwkGridBucketSize = size;
    }

    /*
    **  Initialize Bucket cache size
    */

    value = (_CharPtr) getenv("HIS_CACHE_SIZE");

    if (value != NULL) {
	size = atoi(value);

	if (size > 0)
	    LwkGridCacheSize = size;
    }

    /*
    **  Initialize File Extension
    */

    value = (_CharPtr) getenv("HIS_EXTENSION_BUCKETS");

    if (value != NULL) {
	size = atoi(value);

	if (size > 0)
	    LwkGridExtensionBuckets = size;
    }

    return;
    }


#ifdef MSDOS
#define FCT_ARGS (_CharPtr format, ...)
#else
#define FCT_ARGS (va_alist) va_dcl
#endif

void LwkGridLogTrace FCT_ARGS
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
    va_list ap;
#ifndef MSDOS
    _CharPtr format;
#endif

    if (LwkGridTraceFile != (_OSFile) 0) {

#ifdef MSDOS
	char time_string[50];

	fprintf(LwkGridTraceFile, "%s: ", _strtime(time_string));
	va_start(ap, format);

#else /* !MSDOS */

	va_start(ap);
	format = va_arg(ap, _CharPtr);

#endif /* MSDOS */

	vfprintf(LwkGridTraceFile, format, ap);

	fflush(LwkGridTraceFile);
    }

    return;
    }


_FileStatus  LwkGridExpandIdentifier(identifier, expanded_id)
_String identifier;

    _StringPtr expanded_id;

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
    **  Expand the File Identifier
    */

    *expanded_id = LwkGFExpandIdentifier(identifier);

    return _FileStsSuccess;
    }


_FileStatus  LwkGridOpen(identifier, create, file, name)
_String identifier;
 _Boolean create;

    _GridFilePtr file;
 _DDIFStringPtr name;

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
    _Boolean created;
    _GridFile volatile grid;

    /*
    **  Inititialize
    */

    grid = (_GridFile) 0;

    _StartExceptionBlock

    /*
    **  Initialize the Grid File data structures
    */

    grid = LwkGFInitializeFile(identifier);

    /*
    **  Open or Create the Grid File
    */

    created = LwkGFOpenFile(grid, create);

    /*
    **	If we created a new file, start a read/write transaction, then
    **	initialize the Keys and Directory.  If the file already exists, start a
    **	read transaction and read in the Directory.
    */

    if (created) {
	LwkGFStartTransaction(grid, lwk_c_transact_read_write);

	LwkGDInitializeKeys(grid);

	LwkGDInitializeDirectory(grid);
    }
    else {
	LwkGFStartTransaction(grid, lwk_c_transact_read);

	LwkGDReadDirectory(grid);
    }

    /*
    **  Return the name of the File
    */

    *name = _CopyDDIFString(grid->name);

    /*
    **  Return a pointer to the File
    */

    *file = grid;

    /*
    **	If any exceptions are raised, clean up and reraise.
    */
    
    _Exceptions
	_WhenOthers
	    LwkGFRollbackFile(grid, _False);
	    LwkGFFreeFile(grid);
	    _Reraise;
    _EndExceptionBlock

    return _FileStsSuccess;
    }


_FileStatus  LwkGridClose(file)
_GridFile file;

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
    **  Commit any changes then close the File.
    */

    if (file->state != lwk_c_transact_commit)
	LwkGFCommitFile(file);

    LwkGFCloseFile(file);

    /*
    **  Free all internal storage
    */

    LwkGFFreeFile(file);

    return _FileStsSuccess;
    }


_FileStatus  LwkGridDeleteFile(file, identifier)
_GridFile file;
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
    /*
    **  Delete the Grid File
    */

    LwkGFDeleteFile(identifier);

    return _FileStsSuccess;
    }


_FileStatus  LwkGridRenameFile(file, identifier)
_GridFile file;
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
    /*
    **  Rename the Grid File
    */

    LwkGFRenameFile(file, identifier);

    return _FileStsSuccess;
    }


_FileStatus  LwkGridSetName(file, name)
_GridFile file;
 _DDIFString name;

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
    **  Delete any old Name
    */

    _DeleteDDIFString(&file->name);

    /*
    **  Set the new Name
    */

    file->name = _CopyDDIFString(name);

    /*
    **  Mark the Directory modified
    */

    file->directory_modified = _True;

    return _FileStsSuccess;
    }


_FileStatus  LwkGridAllocateUids(file, count, id)
_GridFile file;
 _Integer count;

    _IntegerPtr id;

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
    **  Return next Id
    */

    *id = file->next_id;

    /*
    **  Allocate the requested Id's
    */

    file->next_id += count;

    /*
    **  The Directory has been modified
    */

    file->directory_modified = _True;

    return _FileStsSuccess;
    }


_FileStatus  LwkGridSetTransaction(file, state)
_GridFile file;
 _Transaction state;

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
    **  Set the Transaction state appropriately, and re-read the Directory if
    **	necessary.
    */

    switch (state) {
	case lwk_c_transact_read :
	    if (file->state == lwk_c_transact_commit)
		LwkGFStartTransaction(file, lwk_c_transact_read);

	    LwkGDReadDirectory(file);

	    break;

	case lwk_c_transact_read_write :
	    if (file->state == lwk_c_transact_commit
		    || file->state == lwk_c_transact_read)
		LwkGFStartTransaction(file, lwk_c_transact_read_write);

	    LwkGDReadDirectory(file);

	    break;

	case lwk_c_transact_commit :
	    if (file->state != lwk_c_transact_commit)
		LwkGFCommitFile(file);
	    break;

	case lwk_c_transact_rollback :
	    if (file->state == lwk_c_transact_read)
		LwkGFCommitFile(file);
	    else if (file->state == lwk_c_transact_read_write)
		LwkGFRollbackFile(file, _True);
	    break;
    }

    return _FileStsSuccess;
    }


_FileStatus  LwkGridDeleteObject(file, domain, id)
_GridFile file;
 _Domain domain;
 _Integer id;

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
    _ConjunctKeys keys;
    _GridCursor cursor;

    /*
    **  Create a description of the Query Keys
    */

    keys = LwkGCCreateConjunctKeys(domain);

    keys->identifier = id;

    /*
    **  Create a Cursor to find the Record.
    */

    cursor = LwkGCCreateCursor("Delete by Id", file, keys);

    /*
    **  Find the Record
    */

    if (!LwkGRFindById(cursor)) {
	LwkGCFreeCursor(cursor);
	return _FileStsNoneFound;
    }

    /*
    **  Delete the Record
    */

    LwkGRDeleteRecord(cursor);

    /*
    **  Free the Cursor
    */

    LwkGCFreeCursor(cursor);

    return _FileStsSuccess;
    }


_FileStatus  LwkGridInsertNetwork(file, id, encoding, new)
_GridFile file;
 _Integer id;

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
    _Integer size;
    _Bucket bucket;
    _ConjunctKeys keys;

    /*
    **  Determine in which Bucket we should store this record.
    */

    size = _RecordHeaderSize + _RecordVSize(encoding);

    keys = LwkGCCreateConjunctKeys(lwk_c_domain_linknet);

    keys->identifier = id;
    keys->container = 0;

    LwkGCSetKeyProperties(keys, encoding);

    bucket = LwkGDBucketToStore(file, size, new, keys);

    LwkGCFreeConjunctKeys(keys, _True);

    /*
    **  Store the record in this Bucket
    */

    bucket = LwkGRStoreRecordHeader(file, bucket, size, id,
	lwk_c_domain_linknet, (_Integer) 0);

    bucket = LwkGRStoreVaryingString(file, bucket, encoding);

    return _FileStsSuccess;
    }


_FileStatus  LwkGridSelectNetwork(file, id, encoding)
_GridFile file;
 _Integer id;

    _DynamicVString encoding;

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
    _ConjunctKeys keys;
    _GridCursor cursor;
    _Integer position;

    /*
    **  Create a description of the Query Keys
    */

    keys = LwkGCCreateConjunctKeys(lwk_c_domain_linknet);

    keys->identifier = id;

    /*
    **  Create a Cursor to find the Record.
    */

    cursor = LwkGCCreateCursor("Net by Id", file, keys);

    /*
    **  Find the Record
    */

    if (!LwkGRFindById(cursor)) {
	LwkGCFreeCursor(cursor);
	return _FileStsCursorEmpty;
    }

    /*
    **  Skip over the Record header
    */

    position = LwkGRSkipRecordHeader(cursor);

    /*
    **  Retrieve the Encoding
    */

    position = LwkGRRetrieveVaryingString(cursor, position, encoding);

    /*
    **  Free the Cursor
    */

    LwkGCFreeCursor(cursor);

    return _FileStsSuccess;
    }


_FileStatus  LwkGridInsertLink(file, id, source, target, network, encoding, new)
_GridFile file;
 _Integer id;

    _Integer source;
 _Integer target;
 _Integer network;
 
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
    _Integer size;
    _Bucket bucket;
    _ConjunctKeys keys;

    /*
    **  Determine in which Bucket we should store this record.
    */

    size = _RecordHeaderSize + (2 * _RecordISize) + _RecordVSize(encoding);

    keys = LwkGCCreateConjunctKeys(lwk_c_domain_link);

    keys->identifier = id;
    keys->container = network;

    LwkGCSetKeyProperties(keys, encoding);

    bucket = LwkGDBucketToStore(file, size, new, keys);

    LwkGCFreeConjunctKeys(keys, _True);

    /*
    **  Store the record in this Bucket
    */

    bucket = LwkGRStoreRecordHeader(file, bucket, size, id,
	lwk_c_domain_link, network);

    bucket = LwkGRStoreVaryingString(file, bucket, encoding);
    bucket = LwkGRStoreInteger(file, bucket, source);
    bucket = LwkGRStoreInteger(file, bucket, target);

    return _FileStsSuccess;
    }


_FileStatus  LwkGridSelectLink(file, id, network, source, target, encoding)
_GridFile file;
 _Integer id;

    _Integer network;
 _IntegerPtr source;
 _IntegerPtr target;

    _DynamicVString encoding;

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
    _ConjunctKeys keys;
    _GridCursor cursor;
    _Integer position;

    /*
    **  Create a description of the Query Keys
    */

    keys = LwkGCCreateConjunctKeys(lwk_c_domain_link);

    keys->identifier = id;
    keys->container = network;

    /*
    **  Create a Cursor to find the Record.
    */

    cursor = LwkGCCreateCursor("Conn by Id", file, keys);

    /*
    **  Find the Record
    */

    if (!LwkGRFindById(cursor)) {
	LwkGCFreeCursor(cursor);
	return _FileStsCursorEmpty;
    }

    /*
    **  Skip the Record header
    */

    position = LwkGRSkipRecordHeader(cursor);

    /*
    **  Retrieve the Encoding
    */

    position = LwkGRRetrieveVaryingString(cursor, position, encoding);

    /*
    **  Retrieve the Source and Target Id's
    */

    position = LwkGRRetrieveInteger(cursor, position, source);
    position = LwkGRRetrieveInteger(cursor, position, target);

    /*
    **  Free the Cursor
    */

    LwkGCFreeCursor(cursor);

    return _FileStsSuccess;
    }


_FileStatus  LwkGridInsertSurrogate(file, id, container, container_domain, has_incomming, has_outgoing, count, interconnect_ids, encoding, new)
_GridFile file;
 _Integer id;

    _Integer container;
 _Integer container_domain;
 _Integer has_incomming;

    _Integer has_outgoing;
 _Integer count;
 _IntegerPtr interconnect_ids;

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
    _Integer size;
    _Bucket bucket;
    _ConjunctKeys keys;

    /*
    **  Determine in which Bucket we should store this record.
    */

    size = _RecordHeaderSize + ((4 + count) * _RecordISize)
	+ _RecordVSize(encoding);

    keys = LwkGCCreateConjunctKeys(lwk_c_domain_surrogate);

    keys->identifier = id;
    keys->container = container;

    LwkGCSetKeyProperties(keys, encoding);

    bucket = LwkGDBucketToStore(file, size, new, keys);

    LwkGCFreeConjunctKeys(keys, _True);

    /*
    **  Store the record in this Bucket
    */

    bucket = LwkGRStoreRecordHeader(file, bucket, size, id,
	lwk_c_domain_surrogate, container);

    bucket = LwkGRStoreVaryingString(file, bucket, encoding);
    bucket = LwkGRStoreInteger(file, bucket, container_domain);
    bucket = LwkGRStoreInteger(file, bucket, has_incomming);
    bucket = LwkGRStoreInteger(file, bucket, has_outgoing);

    bucket = LwkGRStoreInteger(file, bucket, count);

    if (count > 0)
	bucket = LwkGRStoreIntegers(file, bucket, count, interconnect_ids);

    return _FileStsSuccess;
    }


_FileStatus  LwkGridSelectSurrogate(file, id, container, container_domain, has_incomming, has_outgoing, count, interconnect_ids, encoding)
_GridFile file;
 _Integer id;

    _Integer container;
 _IntegerPtr container_domain;

    _IntegerPtr has_incomming;
 _IntegerPtr has_outgoing;

    _IntegerPtr count;
 _IntegerPtr *interconnect_ids;

    _DynamicVString encoding;

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
    _ConjunctKeys keys;
    _GridCursor cursor;
    _Integer position;

    /*
    **  Create a description of the Query Keys
    */

    keys = LwkGCCreateConjunctKeys(lwk_c_domain_surrogate);

    keys->identifier = id;
    keys->container = container;

    /*
    **  Create a Cursor to find the Record.
    */

    cursor = LwkGCCreateCursor("Surr by Id", file, keys);

    /*
    **  Find the Record
    */

    if (!LwkGRFindById(cursor)) {
	LwkGCFreeCursor(cursor);
	return _FileStsCursorEmpty;
    }

    /*
    **  Skip the Record header
    */

    position = LwkGRSkipRecordHeader(cursor);

    /*
    **  Retrieve the Encoding
    */

    position = LwkGRRetrieveVaryingString(cursor, position, encoding);

    /*
    **  Retrieve the Container Domain, and the InterLink flags
    */

    position = LwkGRRetrieveInteger(cursor, position, container_domain);
    position = LwkGRRetrieveInteger(cursor, position, has_incomming);
    position = LwkGRRetrieveInteger(cursor, position, has_outgoing);

    /*
    **  Retrieve the InterLink Ids
    */

    position = LwkGRRetrieveInteger(cursor, position, count);

    if (*count > 0) {
	int i;
	_Integer *ids;

	ids = (_IntegerPtr) _AllocateMem(*count * sizeof(_Integer)); 

	for (i = 0; i < *count; i++)
	    position = LwkGRRetrieveInteger(cursor, position, &ids[i]);

	*interconnect_ids = ids;
    }

    /*
    **  Free the Cursor
    */

    LwkGCFreeCursor(cursor);

    return _FileStsSuccess;
    }


_FileStatus  LwkGridCloseCursor(cursor)
_GridCursor cursor;

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
    LwkGCFreeCursor(cursor);

    return _FileStsSuccess;
    }


_FileStatus  LwkGridOpenDirectory(file, domain, cursor)
_GridFile file;
 _Domain domain;

    _GridCursorPtr cursor;

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
    _CharPtr name;
    _ConjunctKeys keys;

    /*
    **  Create a description of the Query Keys
    */

    keys = LwkGCCreateConjunctKeys(domain);

    /*
    **  Create the appropriate Cursor
    */

    switch (domain) {
	case lwk_c_domain_comp_linknet :
	    name = "CNets in Rep";
	    break;

	case lwk_c_domain_comp_path :
	    name = "CPaths in Rep";
	    break;

	case lwk_c_domain_linknet :
	    name = "Nets in Rep";
	    break;

	case lwk_c_domain_path :
	    name = "Paths in Rep";
	    break;

	default :
	    _Raise(inv_domain);
    }

    *cursor = LwkGCCreateCursor(name, file, keys);

    /*
    **  If there are no records in the Cursor, free the Cursor and return
    **	the appropriate status code.
    */

    if ((*cursor)->bucket_list.count <= 0) {
	LwkGCFreeCursor(*cursor);
	*cursor = (_GridCursor) 0;
	return _FileStsNoneFound;
    }

    return _FileStsSuccess;
    }


_FileStatus  LwkGridFetchDirectory(cursor, id, container_id)
_GridCursor cursor;
 _IntegerPtr id;

    _IntegerPtr container_id;

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
    **  Fetch the next record
    */

    return LwkGRFetchIdByDomain(cursor, id, container_id);
    }


_FileStatus  LwkGridOpenConnInNetwork(file, network, cursor)
_GridFile file;
 _Integer network;

    _GridCursorPtr cursor;

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
    _ConjunctKeys keys;

    /*
    **  Create a description of the Query Keys
    */

    keys = LwkGCCreateConjunctKeys(lwk_c_domain_link);

    keys->container = network;

    /*
    **  Create the appropriate Cursor
    */

    *cursor = LwkGCCreateCursor("Conns in Net", file, keys);

    /*
    **  If there are no records in the Cursor, free the Cursor and return
    **	the appropriate status code.
    */

    if ((*cursor)->bucket_list.count <= 0) {
	LwkGCFreeCursor(*cursor);
	*cursor = (_GridCursor) 0;
	return _FileStsNoneFound;
    }

    return _FileStsSuccess;
    }


_FileStatus  LwkGridFetchConnInNetwork(cursor, link, source, target, encoding)
_GridCursor cursor;

    _IntegerPtr link;
 _IntegerPtr source;
 _IntegerPtr target;

    _DynamicVString encoding;

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
    _Integer position;

    /*
    **  Position to the next record
    */

    if (!LwkGRFindByContainer(cursor, link))
	return _FileStsCursorEmpty;

    /*
    **  Skip the Record Header
    */

    position = LwkGRSkipRecordHeader(cursor);

    /*
    **  Retrieve the encoding
    */

    position = LwkGRRetrieveVaryingString(cursor, position, encoding);

    /*
    **  Fetch the other fields
    */

    position = LwkGRRetrieveInteger(cursor, position, source);
    position = LwkGRRetrieveInteger(cursor, position, target);

    return _FileStsSuccess;
    }


_FileStatus  LwkGridOpenSurrInContainer(file, container, cursor)
_GridFile file;
 _Integer container;

    _GridCursorPtr cursor;

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
    _ConjunctKeys keys;

    /*
    **  Create a description of the Query Keys
    */

    keys = LwkGCCreateConjunctKeys(lwk_c_domain_surrogate);

    keys->container = container;

    /*
    **  Create the appropriate Cursor
    */

    *cursor = LwkGCCreateCursor("Surrs in Cont", file, keys);

    /*
    **  If there are no records in the Cursor, free the Cursor and return
    **	the appropriate status code.
    */

    if ((*cursor)->bucket_list.count <= 0) {
	LwkGCFreeCursor(*cursor);
	*cursor = (_GridCursor) 0;
	return _FileStsNoneFound;
    }

    return _FileStsSuccess;
    }


_FileStatus  LwkGridFetchSurrInContainer(cursor, surrogate, has_incomming, has_outgoing, count, interconnect_ids, encoding)
_GridCursor cursor;

    _IntegerPtr surrogate;
 _IntegerPtr has_incomming;

    _IntegerPtr has_outgoing;
 _IntegerPtr count;

    _IntegerPtr *interconnect_ids;
 _DynamicVString encoding;

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
    _Integer position;

    /*
    **  Position to the next record
    */

    if (!LwkGRFindByContainer(cursor, surrogate))
	return _FileStsCursorEmpty;

    /*
    **  Skip the Record Header
    */

    position = LwkGRSkipRecordHeader(cursor);

    /*
    **  Retrieve the encoding
    */

    position = LwkGRRetrieveVaryingString(cursor, position, encoding);

    /*
    **  Skip the Container Domain
    */

    position = LwkGRSkipInteger(cursor, position);

    /*
    **  Retrieve InterLink flags
    */

    position = LwkGRRetrieveInteger(cursor, position, has_incomming);
    position = LwkGRRetrieveInteger(cursor, position, has_outgoing);

    /*
    **  Retrieve the InterLink Ids
    */

    position = LwkGRRetrieveInteger(cursor, position, count);

    if (*count > 0) {
	int i;
	_Integer *ids;

	ids = (_IntegerPtr) _AllocateMem(*count * sizeof(_Integer)); 

	for (i = 0; i < *count; i++)
	    position = LwkGRRetrieveInteger(cursor, position, &ids[i]);

	*interconnect_ids = ids;
    }

    return _FileStsSuccess;
    }


_FileStatus  LwkGridOpenSurrConn(file, network, surrogate, count, interconnect_ids, cursor)
_GridFile file;
 _Integer network;

    _Integer surrogate;
 _Integer count;
 _IntegerPtr interconnect_ids;

    _GridCursorPtr cursor;

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
    _ConjunctKeys keys;
    _ConjunctKeys disjunct;

    /*
    **	If there are no InterLink Ids, then there are no InterLinks
    */
    
    if (count <= 0)
	return _FileStsNoneFound;

    /*
    **  Create a disjunction of the Query Keys
    */

    keys = (_ConjunctKeys) 0;

    for (i = 0; i < count; i++) {
	disjunct = LwkGCCreateConjunctKeys(lwk_c_domain_link);

	disjunct->identifier = interconnect_ids[i];
	disjunct->container = network;

	disjunct->disjunct = keys;
	keys = disjunct;
    }

    /*
    **  Create the appropriate Cursor
    */

    *cursor = LwkGCCreateCursor("Surr Conns", file, keys);

    /*
    **  If there are no records in the Cursor, free the Cursor and return
    **	the appropriate status code.
    */

    if ((*cursor)->bucket_list.count <= 0) {
	LwkGCFreeCursor(*cursor);
	*cursor = (_GridCursor) 0;
	return _FileStsNoneFound;
    }

    return _FileStsSuccess;
    }


_FileStatus  LwkGridFetchSurrConn(cursor, link, source, target, encoding)
_GridCursor cursor;
 _IntegerPtr link;

    _IntegerPtr source;
 _IntegerPtr target;
 _DynamicVString encoding;

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
    _Integer position;

    /*
    **  Position to the next Link Record
    */

    if (!LwkGRFindById(cursor))
	return _FileStsCursorEmpty;

    /*
    **  Get the Link Id from the Record Header
    */

    position = LwkGRRetrieveRecordHeader(cursor, (_IntegerPtr) 0,
	link, (_DomainPtr) 0, (_IntegerPtr) 0);

    /*
    **  Retrieve the encoding
    */

    position = LwkGRRetrieveVaryingString(cursor, position, encoding);

    /*
    **  Retrieve the other fields
    */

    position = LwkGRRetrieveInteger(cursor, position, source);
    position = LwkGRRetrieveInteger(cursor, position, target);

    return _FileStsSuccess;
    }


_FileStatus  LwkGridOpenQuerySurrogates(file, container, expression, cursor)
_GridFile file;
 _Integer container;

    _QueryExpression expression;
 _GridCursorPtr cursor;

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
    _ConjunctKeys keys;

    /*
    **  Create a description of the Query Keys
    */

    keys = LwkGCCreateConjunctKeys(lwk_c_domain_surrogate);

    keys->container = container;

    LwkGCExtractQueryKeys(file, expression, keys);

    /*
    **  Create the appropriate Cursor
    */

    *cursor = LwkGCCreateCursor("Query Surrs", file, keys);

    /*
    **  If there are no records in the Cursor, free the Cursor and return
    **	the appropriate status code.
    */

    if ((*cursor)->bucket_list.count <= 0) {
	LwkGCFreeCursor(*cursor);
	*cursor = (_GridCursor) 0;
	return _FileStsNoneFound;
    }

    return _FileStsSuccess;
    }


_FileStatus  LwkGridFetchQuerySurrogates(cursor, surrogate, has_incomming, has_outgoing, count, interconnect_ids, encoding)
_GridCursor cursor;

    _IntegerPtr surrogate;
 _IntegerPtr has_incomming;

    _IntegerPtr has_outgoing;
 _IntegerPtr count;

    _IntegerPtr *interconnect_ids;
 _DynamicVString encoding;

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
    **	The appropriate selection has been done, so just call a common routine.
    */

    return LwkGridFetchSurrInContainer(cursor, surrogate, has_incomming,
	has_outgoing, count, interconnect_ids, encoding);
    }


_FileStatus  LwkGridOpenQueryLinks(file, network, expression, cursor)
_GridFile file;
 _Integer network;

    _QueryExpression expression;
 _GridCursorPtr cursor;

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
    _ConjunctKeys keys;

    /*
    **  Create a description of the Query Keys
    */

    keys = LwkGCCreateConjunctKeys(lwk_c_domain_link);

    keys->container = network;

    LwkGCExtractQueryKeys(file, expression, keys);

    /*
    **  Create the appropriate Cursor
    */

    *cursor = LwkGCCreateCursor("Query Conns", file, keys);

    /*
    **  If there are no records in the Cursor, free the Cursor and return
    **	the appropriate status code.
    */

    if ((*cursor)->bucket_list.count <= 0) {
	LwkGCFreeCursor(*cursor);
	*cursor = (_GridCursor) 0;
	return _FileStsNoneFound;
    }

    return _FileStsSuccess;
    }


_FileStatus  LwkGridFetchQueryLinks(cursor, link, source, target, encoding)
_GridCursor cursor;

    _IntegerPtr link;
 _IntegerPtr source;
 _IntegerPtr target;

    _DynamicVString encoding;

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
    **	The appropriate selection has been done, so just call a common routine.
    */

    return LwkGridFetchConnInNetwork(cursor, link, source, target,
	encoding);
    }


_FileStatus  LwkGridVerify(file, flags, list_file)
_GridFile file;
 _Integer flags;
 _OSFile list_file;

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
    **  Verify/List the Grid Directory
    */

    LwkGDVerifyDirectory(file, flags, list_file);

    /*
    **  Verify/List the Grid Buckets
    */

    LwkGBVerifyBuckets(file, flags, list_file);

    return _FileStsSuccess;
    }


#ifndef MSDOS
_FileStatus  LwkGridInsertCompositePath(file, id, encoding, new)
_GridFile file;
 _Integer id;

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
    _Integer size;
    _Bucket bucket;
    _ConjunctKeys keys;

    /*
    **  Determine in which Bucket we should store this record.
    */

    size = _RecordHeaderSize + _RecordVSize(encoding);

    keys = LwkGCCreateConjunctKeys(lwk_c_domain_comp_path);

    keys->identifier = id;
    keys->container = 0;

    LwkGCSetKeyProperties(keys, encoding);

    bucket = LwkGDBucketToStore(file, size, new, keys);

    LwkGCFreeConjunctKeys(keys, _True);

    /*
    **  Store the record in this Bucket
    */

    bucket = LwkGRStoreRecordHeader(file, bucket, size, id,
	lwk_c_domain_comp_path, (_Integer) 0);

    bucket = LwkGRStoreVaryingString(file, bucket, encoding);

    return _FileStsSuccess;
    }


_FileStatus  LwkGridSelectCompositePath(file, id, encoding)
_GridFile file;
 _Integer id;

    _DynamicVString encoding;

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
    _ConjunctKeys keys;
    _GridCursor cursor;
    _Integer position;

    /*
    **  Create a description of the Query Keys
    */

    keys = LwkGCCreateConjunctKeys(lwk_c_domain_comp_path);

    keys->identifier = id;

    /*
    **  Create a Cursor to find the Record.
    */

    cursor = LwkGCCreateCursor("CPath by Id", file, keys);

    /*
    **  Find the Record
    */

    if (!LwkGRFindById(cursor)) {
	LwkGCFreeCursor(cursor);
	return _FileStsCursorEmpty;
    }

    /*
    **  Skip over the Record header
    */

    position = LwkGRSkipRecordHeader(cursor);

    /*
    **  Retrieve the Encoding
    */

    position = LwkGRRetrieveVaryingString(cursor, position, encoding);

    /*
    **  Free the Cursor
    */

    LwkGCFreeCursor(cursor);

    return _FileStsSuccess;
    }


_FileStatus  LwkGridInsertCompositeNet(file, id, encoding, new)
_GridFile file;
 _Integer id;

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
    _Integer size;
    _Bucket bucket;
    _ConjunctKeys keys;

    /*
    **  Determine in which Bucket we should store this record.
    */

    size = _RecordHeaderSize + _RecordVSize(encoding);

    keys = LwkGCCreateConjunctKeys(lwk_c_domain_comp_linknet);

    keys->identifier = id;
    keys->container = 0;

    LwkGCSetKeyProperties(keys, encoding);

    bucket = LwkGDBucketToStore(file, size, new, keys);

    LwkGCFreeConjunctKeys(keys, _True);

    /*
    **  Store the record in this Bucket
    */

    bucket = LwkGRStoreRecordHeader(file, bucket, size, id,
	lwk_c_domain_comp_linknet, (_Integer) 0);

    bucket = LwkGRStoreVaryingString(file, bucket, encoding);

    return _FileStsSuccess;
    }


_FileStatus  LwkGridSelectCompositeNet(file, id, encoding)
_GridFile file;
 _Integer id;

    _DynamicVString encoding;

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
    _ConjunctKeys keys;
    _GridCursor cursor;
    _Integer position;

    /*
    **  Create a description of the Query Keys
    */

    keys = LwkGCCreateConjunctKeys(lwk_c_domain_comp_linknet);

    keys->identifier = id;

    /*
    **  Create a Cursor to find the Record.
    */

    cursor = LwkGCCreateCursor("CNet by Id", file, keys);

    /*
    **  Find the Record
    */

    if (!LwkGRFindById(cursor)) {
	LwkGCFreeCursor(cursor);
	return _FileStsCursorEmpty;
    }

    /*
    **  Skip over the Record header
    */

    position = LwkGRSkipRecordHeader(cursor);

    /*
    **  Retrieve the Encoding
    */

    position = LwkGRRetrieveVaryingString(cursor, position, encoding);

    /*
    **  Free the Cursor
    */

    LwkGCFreeCursor(cursor);

    return _FileStsSuccess;
    }


_FileStatus  LwkGridInsertPath(file, id, first_step, last_step, current_step, encoding, new)
_GridFile file;
 _Integer id;

    _Integer first_step;
 _Integer last_step;
 _Integer current_step;

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
    _Integer size;
    _Bucket bucket;
    _ConjunctKeys keys;

    /*
    **  Determine in which Bucket we should store this record.
    */

    size = _RecordHeaderSize + (3 * _RecordISize) + _RecordVSize(encoding);

    keys = LwkGCCreateConjunctKeys(lwk_c_domain_path);

    keys->identifier = id;
    keys->container = 0;

    LwkGCSetKeyProperties(keys, encoding);

    bucket = LwkGDBucketToStore(file, size, new, keys);

    LwkGCFreeConjunctKeys(keys, _True);

    /*
    **  Store the record in this Bucket
    */

    bucket = LwkGRStoreRecordHeader(file, bucket, size, id, lwk_c_domain_path,
	(_Integer) 0);

    bucket = LwkGRStoreVaryingString(file, bucket, encoding);
    bucket = LwkGRStoreInteger(file, bucket, first_step);
    bucket = LwkGRStoreInteger(file, bucket, last_step);
    bucket = LwkGRStoreInteger(file, bucket, current_step);

    return _FileStsSuccess;
    }


_FileStatus  LwkGridSelectPath(file, id, first_step, last_step, current_step, encoding)
_GridFile file;
 _Integer id;

    _IntegerPtr first_step;
 _IntegerPtr last_step;

    _IntegerPtr current_step;
 _DynamicVString encoding;

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
    _ConjunctKeys keys;
    _GridCursor cursor;
    _Integer position;

    /*
    **  Create a description of the Query Keys
    */

    keys = LwkGCCreateConjunctKeys(lwk_c_domain_path);

    keys->identifier = id;

    /*
    **  Create a Cursor to find the Record.
    */

    cursor = LwkGCCreateCursor("Path by Id", file, keys);

    /*
    **  Find the Record
    */

    if (!LwkGRFindById(cursor)) {
	LwkGCFreeCursor(cursor);
	return _FileStsCursorEmpty;
    }

    /*
    **  Skip over the Record header
    */

    position = LwkGRSkipRecordHeader(cursor);

    /*
    **  Retrieve the Encoding
    */

    position = LwkGRRetrieveVaryingString(cursor, position, encoding);

    /*
    **  Retrieve the First/Last/Current Step Ids
    */

    position = LwkGRRetrieveInteger(cursor, position, first_step);
    position = LwkGRRetrieveInteger(cursor, position, last_step);
    position = LwkGRRetrieveInteger(cursor, position, current_step);

    /*
    **  Free the Cursor
    */

    LwkGCFreeCursor(cursor);

    return _FileStsSuccess;
    }


_FileStatus  LwkGridInsertStep(file, id, origin, destination, previous, next, path, encoding, new)
_GridFile file;
 _Integer id;

    _Integer origin;
 _Integer destination;
 _Integer previous;
 _Integer next;

    _Integer path;
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
    _Integer size;
    _Bucket bucket;
    _ConjunctKeys keys;

    /*
    **  Determine in which Bucket we should store this record.
    */

    size = _RecordHeaderSize + (4 * _RecordISize) + _RecordVSize(encoding);

    keys = LwkGCCreateConjunctKeys(lwk_c_domain_step);

    keys->identifier = id;
    keys->container = path;

    LwkGCSetKeyProperties(keys, encoding);

    bucket = LwkGDBucketToStore(file, size, new, keys);

    LwkGCFreeConjunctKeys(keys, _True);

    /*
    **  Store the record in this Bucket
    */

    bucket = LwkGRStoreRecordHeader(file, bucket, size, id,
	lwk_c_domain_step, path);

    bucket = LwkGRStoreVaryingString(file, bucket, encoding);
    bucket = LwkGRStoreInteger(file, bucket, origin);
    bucket = LwkGRStoreInteger(file, bucket, destination);
    bucket = LwkGRStoreInteger(file, bucket, previous);
    bucket = LwkGRStoreInteger(file, bucket, next);

    return _FileStsSuccess;
    }


_FileStatus  LwkGridSelectStep(file, id, path, origin, destination, previous, next, encoding)
_GridFile file;
 _Integer id;
 _Integer path;

    _IntegerPtr origin;
 _IntegerPtr destination;
 _IntegerPtr previous;

    _IntegerPtr next;
 _DynamicVString encoding;

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
    _ConjunctKeys keys;
    _GridCursor cursor;
    _Integer position;

    /*
    **  Create a description of the Query Keys
    */

    keys = LwkGCCreateConjunctKeys(lwk_c_domain_step);

    keys->identifier = id;
    keys->container = path;

    /*
    **  Create a Cursor to find the Record.
    */

    cursor = LwkGCCreateCursor("Step by Id", file, keys);

    /*
    **  Find the Record
    */

    if (!LwkGRFindById(cursor)) {
	LwkGCFreeCursor(cursor);
	return _FileStsCursorEmpty;
    }

    /*
    **  Skip the Record header
    */

    position = LwkGRSkipRecordHeader(cursor);

    /*
    **  Retrieve the Encoding
    */

    position = LwkGRRetrieveVaryingString(cursor, position, encoding);

    /*
    **  Retrieve the Origin, Destination, Previous and Next Id's
    */

    position = LwkGRRetrieveInteger(cursor, position, origin);
    position = LwkGRRetrieveInteger(cursor, position, destination);
    position = LwkGRRetrieveInteger(cursor, position, previous);
    position = LwkGRRetrieveInteger(cursor, position, next);

    /*
    **  Free the Cursor
    */

    LwkGCFreeCursor(cursor);

    return _FileStsSuccess;
    }


_FileStatus  LwkGridOpenSurrStep(file, path, surrogate, count, interconnect_ids, cursor)
_GridFile file;
 _Integer path;

    _Integer surrogate;
 _Integer count;
 _IntegerPtr interconnect_ids;

    _GridCursorPtr cursor;

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
    _ConjunctKeys keys;
    _ConjunctKeys disjunct;

    /*
    **	If there are no InterLink Ids, then there are no InterLinks
    */
    
    if (count <= 0)
	return _FileStsNoneFound;

    /*
    **  Create a disjunction of the Query Keys
    */

    keys = (_ConjunctKeys) 0;

    for (i = 0; i < count; i++) {
	disjunct = LwkGCCreateConjunctKeys(lwk_c_domain_step);

	disjunct->identifier = interconnect_ids[i];
	disjunct->container = path;

	disjunct->disjunct = keys;
	keys = disjunct;
    }

    /*
    **  Create the appropriate Cursor
    */

    *cursor = LwkGCCreateCursor("Surr Steps", file, keys);

    /*
    **  If there are no records in the Cursor, free the Cursor and return
    **	the appropriate status code.
    */

    if ((*cursor)->bucket_list.count <= 0) {
	LwkGCFreeCursor(*cursor);
	*cursor = (_GridCursor) 0;
	return _FileStsNoneFound;
    }

    return _FileStsSuccess;
    }


_FileStatus  LwkGridFetchSurrStep(cursor, step, origin, destination, previous, next, encoding)
_GridCursor cursor;
 _IntegerPtr step;

    _IntegerPtr origin;
 _IntegerPtr destination;
 _IntegerPtr previous;

    _IntegerPtr next;
 _DynamicVString encoding;

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
    _Integer position;

    /*
    **  Position to the next Step record
    */

    if (!LwkGRFindById(cursor))
	return _FileStsCursorEmpty;

    /*
    **  Get the Step Id from the Record Header
    */

    position = LwkGRRetrieveRecordHeader(cursor, (_IntegerPtr) 0,
	step, (_DomainPtr) 0, (_IntegerPtr) 0);

    /*
    **  Retrieve the other fields
    */

    position = LwkGRRetrieveVaryingString(cursor, position, encoding);
    position = LwkGRRetrieveInteger(cursor, position, origin);
    position = LwkGRRetrieveInteger(cursor, position, destination);
    position = LwkGRRetrieveInteger(cursor, position, previous);
    position = LwkGRRetrieveInteger(cursor, position, next);

    return _FileStsSuccess;
    }


_FileStatus  LwkGridOpenStepInPath(file, path, cursor)
_GridFile file;
 _Integer path;

    _GridCursorPtr cursor;

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
    _ConjunctKeys keys;

    /*
    **  Create a description of the Query Keys
    */

    keys = LwkGCCreateConjunctKeys(lwk_c_domain_step);

    keys->container = path;

    /*
    **  Create the appropriate Cursor
    */

    *cursor = LwkGCCreateCursor("Steps in Path", file, keys);

    /*
    **  If there are no records in the Cursor, free the Cursor and return
    **	the appropriate status code.
    */

    if ((*cursor)->bucket_list.count <= 0) {
	LwkGCFreeCursor(*cursor);
	*cursor = (_GridCursor) 0;
	return _FileStsNoneFound;
    }

    return _FileStsSuccess;
    }


_FileStatus  LwkGridFetchStepInPath(cursor, step, origin, destination, previous, next, encoding)
_GridCursor cursor;
 _IntegerPtr step;

    _IntegerPtr origin;
 _IntegerPtr destination;
 _IntegerPtr previous;

    _IntegerPtr next;
 _DynamicVString encoding;

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
    _Integer position;

    /*
    **  Position to the next record
    */

    if (!LwkGRFindByContainer(cursor, step))
	return _FileStsCursorEmpty;

    /*
    **  Skip the Record Header
    */

    position = LwkGRSkipRecordHeader(cursor);

    /*
    **  Retrieve the other fields
    */

    position = LwkGRRetrieveVaryingString(cursor, position, encoding);
    position = LwkGRRetrieveInteger(cursor, position, origin);
    position = LwkGRRetrieveInteger(cursor, position, destination);
    position = LwkGRRetrieveInteger(cursor, position, previous);
    position = LwkGRRetrieveInteger(cursor, position, next);

    return _FileStsSuccess;
    }


_FileStatus  LwkGridOpenQuerySteps(file, path, expression, cursor)
_GridFile file;
 _Integer path;

    _QueryExpression expression;
 _GridCursorPtr cursor;

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
    _ConjunctKeys keys;

    /*
    **  Create a description of the Query Keys
    */

    keys = LwkGCCreateConjunctKeys(lwk_c_domain_step);

    keys->container = path;

    LwkGCExtractQueryKeys(file, expression, keys);

    /*
    **  Create the appropriate Cursor
    */

    *cursor = LwkGCCreateCursor("Steps in Path", file, keys);

    /*
    **  If there are no records in the Cursor, free the Cursor and return
    **	the appropriate status code.
    */

    if ((*cursor)->bucket_list.count <= 0) {
	LwkGCFreeCursor(*cursor);
	*cursor = (_GridCursor) 0;
	return _FileStsNoneFound;
    }

    return _FileStsSuccess;
    }


_FileStatus  LwkGridFetchQuerySteps(cursor, step, origin, destination, previous, next, encoding)
_GridCursor cursor;
 _IntegerPtr step;

    _IntegerPtr origin;
 _IntegerPtr destination;
 _IntegerPtr previous;

    _IntegerPtr next;
 _DynamicVString encoding;

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
    **	The appropriate selection has been done, so just call a common routine.
    */

    return LwkGridFetchStepInPath(cursor, step, origin, destination, previous,
	next, encoding);
    }

#endif /* !MSDOS */
