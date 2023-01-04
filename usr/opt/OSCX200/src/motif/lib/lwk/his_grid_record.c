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
**  Macro Definitions
*/

/*
**  Forward Routine Declarations
*/

_DeclareFunction(static _Boolean NextRecord,
    (_GridCursor cursor, _Boolean skip_current, _Boolean skip_null));
_DeclareFunction(static _Bucket StoreData,
    (_GridFile file, _Bucket bucket, _AnyPtr data, _Integer size));
_DeclareFunction(static _Integer SkipData,
    (_GridCursor cursor, _Integer position, _Integer size));
_DeclareFunction(static _Integer RetrieveData,
    (_GridCursor cursor, _Integer cursor_position, _AnyPtr data,
    _Integer size));


_FileStatus  LwkGRFetchIdByDomain(cursor, id, container_id)
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
    _Domain domain;
    _Integer this_id;
    _Integer position;
    _Integer this_container_id;

    /*
    **  Loop checking records for the ones we seek
    */

    while (_True) {
	/*
	**  Position to next Record
	*/

	if (!NextRecord(cursor, _True, _True))
	    return _FileStsCursorEmpty;

	/*
	**  Get the header of the next record
	*/

	position = LwkGRRetrieveRecordHeader(cursor, (_IntegerPtr) 0,
	    &this_id, &domain, &this_container_id);

	/*
	**  If it is of the requested Domain, return its Id
	*/

	if (cursor->keys->domain == domain) {
	    *id = this_id;
	    *container_id = this_container_id;
	    break;
	}
    }

    /*
    **  Count a Record selected
    */

    cursor->select_count++;

    return _FileStsSuccess;
    }


_Boolean  LwkGRFindById(cursor)
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
    _Boolean found;
    _ConjunctKeys keys;
    _Integer this_id;

    /*
    **  Loop checking Records until we find the one we seek, or hit the end of
    **	the Bucket Buffer
    */

    while (_True) {
	if (!NextRecord(cursor, _True, _True))
	    return _False;

	LwkGRRetrieveRecordHeader(cursor, (_IntegerPtr) 0, &this_id,
	    (_DomainPtr) 0, (_IntegerPtr) 0);

	found = _False;
	keys = cursor->keys;

	while (keys != (_ConjunctKeys) 0) {
	    if (this_id == keys->identifier) {
		found = _True;
		break;
	    }

	    keys = keys->disjunct;
	}

	if (found)
	    break;
    }

    /*
    **  Count a Record selected
    */

    cursor->select_count++;

    return _True;
    }


_Boolean  LwkGRFindByContainer(cursor, id)
_GridCursor cursor;
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
    _Integer position;
    _Integer container;
    _Domain domain;

    /*
    **  Loop checking records for the ones we seek
    */

    while (_True) {
	if (!NextRecord(cursor, _True, _True))
	    return _False;

	/*
	**  Get the header of the next record
	*/

	position = LwkGRRetrieveRecordHeader(cursor, (_IntegerPtr) 0,
	    id, &domain, &container);

	/*
	**  See if this record is of the requested Domain and container
	*/

	if (domain == cursor->keys->domain
		&& container == cursor->keys->container)
	    break;
    }

    /*
    **  Count a Record selected
    */

    cursor->select_count++;

    return _True;
    }


_Boolean  LwkGRNextRecord(cursor, skip_current)
_GridCursor cursor;
 _Boolean skip_current;

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
    **  Use the common routine
    */

    return NextRecord(cursor, skip_current, _True);
    }


_Boolean  LwkGRNextRecordBucket(cursor)
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
    /*
    **  Use the common routine
    */

    return NextRecord(cursor, _True, _False);
    }


void  LwkGRMoveRecord(cursor, bucket)
_GridCursor cursor;
 _Bucket bucket;

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

    /*
    **	Note: if a Bucket has one or more Extensions, then it contains only a
    **	single Record (one that is larger than will fit in a single Bucket).
    **	When we move such a Record, we only copy that portion of the Record
    **	stored in the main Bucket and move the remainder of the Record by
    **	re-linking the Bucket Extension(s).
    */

    /*
    **  Get the size of the Record.
    */

    RetrieveData(cursor, cursor->position, &size, sizeof(_Integer));

    /*
    **	Copy the Record (or a portion of it) to the new Bucket.  If the Record
    **	required Bucket Extension(s), re-link them to the new Bucket.
    */

    if (size > _BucketDataSize(cursor->bucket)) {
	size = _BucketDataSize(cursor->bucket);

	bucket->buffer->extension = cursor->bucket->buffer->extension;
	cursor->bucket->buffer->extension = (_BucketNumber) 0;
    }

    _CopyMem(&cursor->bucket->buffer->data[cursor->position],
	&bucket->buffer->data[bucket->buffer->first_free], size);

    bucket->buffer->first_free += size;

    /*
    **  Delete the Record from the old Bucket Buffer
    */

    LwkGRDeleteRecord(cursor);

    /*
    **  Increment the Record count
    */

    bucket->buffer->record_count++;

    /*
    **  Set the Bucket modified state
    */

    bucket->modified = _True;

    return;
    }


void  LwkGRDeleteRecord(cursor)
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
    _Integer size;

    /*
    **	Note: if a Bucket has one or more Extensions, then it contains only a
    **	single Record (one that is larger than will fit in a single Bucket).
    **	When we delete such a Record, we just keep any allocated Bucket
    **	Extensions for possible future use.
    */

    /*
    **  Get the size of the Record.
    */

    RetrieveData(cursor, cursor->position, &size, sizeof(_Integer));

    /*
    **	If there are other Records in this Bucket, move them up.
    */

    if (cursor->position + size < cursor->bucket->buffer->first_free)
	_CopyMem(&cursor->bucket->buffer->data[cursor->position + size],
	    &cursor->bucket->buffer->data[cursor->position],
	    (cursor->bucket->buffer->first_free - (cursor->position + size)));

    /*
    **	Reset the first free pointer.  If this was an extended Record, clean up
    **	the Cursor as well so that NextRecord will do the right thing.
    */

    if (size <= _BucketDataSize(cursor->bucket))
	cursor->bucket->buffer->first_free -= size;
    else {
	cursor->bucket->buffer->first_free = 0;
	cursor->ext_bucket = cursor->bucket;
	cursor->ext_base = 0;
    }

    /*
    **  Set the Bucket modified state
    */

    cursor->bucket->modified = _True;

    /*
    **  Decrement Record count
    */

    cursor->bucket->buffer->record_count--;

    /*
    **  Mark the File modified
    */

    cursor->file->file_modified = _True;

    return;
    }


void  LwkGRDeleteRewrittenRecord(file, number, domain, identifier)
_GridFile file;
 _BucketNumber number;

    _Domain domain;
 _Integer identifier;

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
    _GridCursor cursor;

    /*
    **  Create a Cursor that will seach only the Bucket where the new Record
    **	will be stored
    */

    cursor = LwkGCCreateRewriteCursor(file, number, domain, identifier);

    /*
    **	Search for the old Record.  If it is found, delete the old version of
    **	the Record from the Bucket.  It is possible that it will not be found
    **	-- this can happen if some keyed property of the Record was changed.
    **	In this case, delete the Record from wherever it is stored.
    */

    if (LwkGRFindById(cursor))
	LwkGRDeleteRecord(cursor);
    else
	LwkGridDeleteObject(file, domain, identifier);

    /*
    **  Free the Cursor
    */

    LwkGCFreeCursor(cursor);

    return;
    }


_Integer  LwkGRVerifyRecord(cursor, flags, list_file)
_GridCursor cursor;
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
    _Integer size;
    _Domain domain;
    _Integer container;
    _Integer identifier;

    /*
    **  Verify/List the information from the Record header
    */

    LwkGRRetrieveRecordHeader(cursor, &size, &identifier, &domain,
	&container);

    fprintf(list_file,
	"    Record: Size = %ld, Id = %ld, Domain = %s, Container = %ld\n",
	size, identifier, LwkGDDomainToName(domain), container);

    return size;
    }


_Bucket  LwkGRStoreRecordHeader(file, bucket, size, id, domain, container)
_GridFile file;
 _Bucket bucket;
 _Integer size;

    _Integer id;
 _Domain domain;
 _Integer container;

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
    **  Store the record size
    */

    bucket = StoreData(file, bucket, &size, sizeof(_Integer));

    /*
    **  Copy the Object Id to the Bucket Buffer
    */

    bucket = StoreData(file, bucket, &id, sizeof(_Integer));

    /*
    **  Store the Object Domain
    */

    bucket = StoreData(file, bucket, &domain, sizeof(_Domain));

    /*
    **  Store the Container Id
    */

    bucket = StoreData(file, bucket, &container, sizeof(_Integer));

    return bucket;
    }


_Bucket  LwkGRStoreInteger(file, bucket, integer)
_GridFile file;
 _Bucket bucket;
 _Integer integer;

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
    **  Store the Integer
    */

    return StoreData(file, bucket, &integer, sizeof(_Integer));
    }


_Bucket  LwkGRStoreIntegers(file, bucket, count, integers)
_GridFile file;
 _Bucket bucket;
 _Integer count;

    _IntegerPtr integers;

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
    **  Store the Integers
    */

    return StoreData(file, bucket, integers, count * sizeof(_Integer));
    }


_Bucket  LwkGRStoreFloat(file, bucket, float_pt)
_GridFile file;
 _Bucket bucket;
 _Float float_pt;

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
    **  Store the Float
    */

    return StoreData(file, bucket, &float_pt, sizeof(_Float));
    }


_Bucket  LwkGRStoreString(file, bucket, string)
_GridFile file;
 _Bucket bucket;
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
    _Integer length;

    /*
    **  Get the length of the String
    */

    if (string == (_String) 0)
	length = 0;
    else
	length = _LengthString(string) + 1;

    /*
    **  Store the length of the String
    */

    bucket = StoreData(file, bucket, &length, sizeof(_Integer));

    /*
    **  Store the String
    */

    if (length > 0)
	bucket = StoreData(file, bucket, string, length);

    return bucket;
    }


_Bucket  LwkGRStoreDDIFString(file, bucket, ddifstring)
_GridFile file;
 _Bucket bucket;

     _DDIFString ddifstring;

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
    _Integer length;

    /*
    **  Get the length of the DDIFString
    */

    if (ddifstring == (_DDIFString) 0)
	length = 0;
    else
	length = _LengthDDIFString(ddifstring);

    /*
    **  Store the length of the DDIF String
    */

    bucket = StoreData(file, bucket, &length, sizeof(_Integer));

    /*
    **  Store the DDIF String
    */

    if (length > 0)
	bucket = StoreData(file, bucket, ddifstring, length);

    return bucket;
    }


_Bucket  LwkGRStoreDate(file, bucket, date)
_GridFile file;
 _Bucket bucket;
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
    _Integer length;

    /*
    **  Get the length of the Date
    */

    if (date == (_Date) 0)
	length = 0;
    else
	length = _LengthDate(date) + 1;

    /*
    **  Store the length of the Date
    */

    bucket = StoreData(file, bucket, &length, sizeof(_Integer));

    /*
    **  Store the Date
    */

    if (length > 0)
	bucket = StoreData(file, bucket, date, length);

    return bucket;
    }


_Bucket  LwkGRStoreVaryingString(file, bucket, vstring)
_GridFile file;
 _Bucket bucket;

    _VaryingString vstring;

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
    _Integer length;

    /*
    **  Get the length of the Varying String
    */

    length = _VSize_of(vstring);

    /*
    **  Store the length of the Varying String
    */

    bucket = StoreData(file, bucket, &length, sizeof(_Integer));

    /*
    **  Store the Varying String
    */

    if (length > 0)
	bucket = StoreData(file, bucket, vstring, length);

    return bucket;
    }


_Integer  LwkGRSkipRecordHeader(cursor)
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
    return SkipData(cursor, cursor->position, _RecordHeaderSize);
    }


_Integer  LwkGRSkipInteger(cursor, position)
_GridCursor cursor;
 _Integer position;

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
    return SkipData(cursor, position,  sizeof(_Integer));
    }


_Integer  LwkGRSkipVaryingString(cursor, position)
_GridCursor cursor;
 _Integer position;

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

    /*
    **  Retrieve the size of the Varying String from the Bucket Buffer
    */

    position = RetrieveData(cursor, position, &size, sizeof(_Integer));

    /*
    **  Skip over the size and the Varying String itself in the Bucket Buffer
    */

    return SkipData(cursor, position, size);
    }


_Integer  LwkGRRetrieveRecordHeader(cursor, size, id, domain, container)
_GridCursor cursor;
 _IntegerPtr size;

    _IntegerPtr id;
 _DomainPtr domain;
 _IntegerPtr container;

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
    **  Retrieve/Skip the record size
    */

    position = cursor->position;

    if (size != (_IntegerPtr) 0)
	position = RetrieveData(cursor, position, size, sizeof(_Integer));
    else
	position = SkipData(cursor, position, sizeof(_Integer));

    /*
    **  Retrieve/Skip the Object Id from/in the Bucket Buffer
    */

    if (id != (_IntegerPtr) 0)
	position = RetrieveData(cursor, position, id, sizeof(_Integer));
    else
	position = SkipData(cursor, position, sizeof(_Integer));

    /*
    **  Retrieve/Skip the Object Domain from/in the Bucket Buffer
    */

    if (domain != (_DomainPtr) 0)
	position = RetrieveData(cursor, position, domain, sizeof(_Domain));
    else
	position = SkipData(cursor, position, sizeof(_Domain));

    /*
    **  Retrieve/Skip the Container Id from/in the Bucket Buffer
    */

    if (container != (_IntegerPtr) 0)
	position = RetrieveData(cursor, position, container, sizeof(_Integer));
    else
	position = SkipData(cursor, position, sizeof(_Integer));

    return position;
    }


_Integer  LwkGRRetrieveInteger(cursor, position, integer)
_GridCursor cursor;
 _Integer position;

    _IntegerPtr integer;

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
    **  Retrieve the Integer
    */

    return RetrieveData(cursor, position, integer, sizeof(_Integer));;
    }


_Integer  LwkGRRetrieveIntegers(cursor, position, count, integers)
_GridCursor cursor;
 _Integer position;

    _Integer count;
 _IntegerPtr integers;

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
    **  Retrieve the Integer
    */

    return RetrieveData(cursor, position, integers, count * sizeof(_Integer));
    }


_Integer  LwkGRRetrieveFloat(cursor, position, float_pt)
_GridCursor cursor;
 _Integer position;

    _FloatPtr float_pt;

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
    **  Retrieve the Float
    */

    return RetrieveData(cursor, position, float_pt, sizeof(_Float));;
    }


_Integer  LwkGRRetrieveString(cursor, position, string)
_GridCursor cursor;
 _Integer position;

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
    _Integer length;

    /*
    **  Retrieve the length of the String
    */

    position = RetrieveData(cursor, position, &length, sizeof(_Integer));;

    /*
    **  Retrieve the String
    */

    if (length > 0) {
	*string = (_String) _AllocateMem(length);
	position = RetrieveData(cursor, position, *string, length);
    }
    else
	*string = (_String) 0;

    return position;
    }


_Integer  LwkGRRetrieveDDIFString(cursor, position, ddifstring)
_GridCursor cursor;
 _Integer position;

    _DDIFStringPtr ddifstring;

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
    _Integer length;

    /*
    **  Retrieve the length of the DDIF String
    */

    position = RetrieveData(cursor, position, &length, sizeof(_Integer));;

    /*
    **  Retrieve the DDIF String
    */

    if (length > 0) {
	*ddifstring = (_DDIFString) _AllocateMem(length);
        position = RetrieveData(cursor, position, *ddifstring, length);
    }
    else
	*ddifstring = (_DDIFString) 0;

    return position;
    }


_Integer  LwkGRRetrieveDate(cursor, position, date)
_GridCursor cursor;
 _Integer position;

    _DatePtr date;

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
    _Integer length;

    /*
    **  Retrieve the length of the Date
    */

    position = RetrieveData(cursor, position, &length, sizeof(_Integer));;

    /*
    **  Retrieve the Date
    */

    if (length > 0) {
	*date = (_Date) _AllocateMem(length);
        position = RetrieveData(cursor, position, *date, length);
    }
    else
	*date = (_Date) 0;

    return position;
    }


_Integer  LwkGRRetrieveVaryingString(cursor, position, dvstring)
_GridCursor cursor;

    _Integer position;
 _DynamicVString dvstring;

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
    _Integer length;

    /*
    **  Retrieve the length of the Varying String
    */

    position = RetrieveData(cursor, position, &length, sizeof(_Integer));;

    /*
    **  Make sure that there is room in the Dynamic Varying String
    */

    if (length > _DVAllocation_of(dvstring))
	_ReallocateDVString(dvstring, length);

    /*
    **  Retrieve the Varying String
    */

    if (length > 0)
        position = RetrieveData(cursor, position, _DVString_of(dvstring),
	    length);
    else
	_VLength_of(_DVString_of(dvstring)) = 0;

    return position;
    }


static _Boolean  NextRecord(cursor, skip_current, skip_null)
_GridCursor cursor;
 _Boolean skip_current;

    _Boolean skip_null;

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
    _Integer id;
    _Integer size;
    _Integer first_free;

    /*
    **	Notes: the current Bucket of a Cursor (cursor->bucket) always contains
    **	the header for the current Record (at an offset specified by
    **	cursor->position).  The current Bucket Extension of a Cursor
    **	(cursor->ext_bucket -- initially equal to cursor->bucket) contains the
    **	most recently retrieved data from the current Record of the Cursor
    **	(though not necessarily the end of that Record).  The offset of the
    **	base of the current Bucket Extension from the beginning of the current
    **	Bucket is specified by cursor->ext_base.
    */

    /*
    **	Get the size of the current Record.  If we have none, or if we were
    **	asked not to skip the current Record, or if the current Record was
    **	deleted, set the size to zero -- effectively turning the "skip" we do
    **	below into a no-op.
    */

    if (cursor->current_bucket < 0
	    || cursor->bucket->buffer->first_free == 0
	    || !skip_current)
	size = 0;
    else {
	LwkGRRetrieveRecordHeader(cursor, &size, &id, (_IntegerPtr) 0,
	    (_IntegerPtr) 0);

	if (id != cursor->current_id)
	    size = 0;
    }

    /*
    **	Determine the boundary of written data in this Bucket.  This may
    **	require reading additional Bucket Extensions!
    */

    if (cursor->current_bucket < 0)
	first_free = 0;
    else {
	while (_True) {
	    first_free =
		cursor->ext_base + cursor->ext_bucket->buffer->first_free;

	    /*
	    **	If the end of the current Record is in the current Bucket
	    **	Extension, we know what we need to know.  This is also the case
	    **	when there are no Records in this Bucket.
	    */

	    if (first_free == 0 || cursor->position + size < first_free)
		break;

	    /*
	    **	Otherwise, read Bucket Extensions until we find the one with
	    **	the end of the current Record.
	    */

	    if (cursor->ext_bucket->buffer->extension == (_BucketNumber) 0)
		break;
	    else {
		cursor->ext_base += _BucketDataSize(cursor->ext_bucket);

		cursor->ext_bucket = LwkGBReadBucket(cursor->file,
		    cursor->ext_bucket->buffer->extension);
	    }
	}
    }

    /*
    **	If there is another Record in this Bucket (or its Bucket Extension),
    **	use it, else move to the first Record in the next Bucket in the
    **	Cursor's Bucket list.
    */

    if (cursor->position + size < first_free) {
	/*
	**  There is another record in this Bucket -- update the Bucket
	**  position.
	*/

	cursor->position = (cursor->position + size)
	    % _BucketDataSize(cursor->bucket);
    }
    else {
	/*
	**  No more records in this Bucket -- position to the first (possibly
	**  null) Record in the next Bucket in the Cursor.
	*/

	while (_True) {
	    /*
	    **  If we reach the end of the Bucket list, there are no more
	    **	Records!
	    */

	    if (cursor->current_bucket >= cursor->bucket_list.count - 1)
		return _False;

	    /*
	    **  Get the next Bucket in the list
	    */

	    cursor->current_bucket++;
	    cursor->b_search_count++;

	    cursor->bucket = LwkGBReadBucket(cursor->file,
		cursor->bucket_list.numbers[cursor->current_bucket]);

	    /*
	    **  Skip empty Buckets only if we were asked to do so.
	    */

	    if (!skip_null || cursor->bucket->buffer->first_free > 0)
		break;
	}
	
	/*
	**  Reset the Bucket position.
	*/

	cursor->position = 0;
    }

    /*
    **  Reset the Bucket extension pointer and offset.
    */

    cursor->ext_base = 0;
    cursor->ext_bucket = cursor->bucket;

    /*
    ** Remember the current Record Id (zero if there is none).  This will allow
    ** us to notice the next time around if this Record was deleted, so that we
    ** will not try to skip it!
    */

    if (cursor->bucket->buffer->first_free == 0)
	cursor->current_id = 0;
    else {
	LwkGRRetrieveRecordHeader(cursor, &size, &cursor->current_id,
	    (_IntegerPtr) 0, (_IntegerPtr) 0);

	if (size == 0)
	    cursor->current_id = 0;
    }
	
    /*
    **  Count a Record searched
    */

    cursor->r_search_count++;

    return _True;
    }


static _Bucket  StoreData(file, bucket, data, size)
_GridFile file;
 _Bucket bucket;
 _AnyPtr data;

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
    _Integer left;
    _Integer chunk;
    _Integer copied;
    _Bucket current;

    /*
    **  Write it out into one or more Buckets
    */

    copied = 0;
    left = size;
    current = bucket;

    while (left > 0) {
	/*
	**  Write a chunk to this Bucket
	*/

	if (left <= _BucketFreeSize(current))
	    chunk = left;
	else
	    chunk = _BucketFreeSize(current);

	_CopyMem(&((_CharPtr) data)[copied],
	    &current->buffer->data[current->buffer->first_free], chunk);

	current->buffer->first_free += chunk;
	current->modified = _True;

	copied += chunk;
	left -= chunk;

	/*
	**  If necessary, extend this Bucket for the remainder
	*/

	if (left > 0)
	    current = LwkGBExtendBucket(file, current);
    }

    return current;
    }


static _Integer  SkipData(cursor, position, size)
_GridCursor cursor;
 _Integer position;
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
    return RetrieveData(cursor, position, (_AnyPtr) 0, size);
    }


static _Integer  RetrieveData(cursor, cursor_position, data, size)
_GridCursor cursor;
 _Integer cursor_position;

    _AnyPtr data;
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
    _Integer left;
    _Integer base;
    _Integer chunk;
    _Integer copied;
    _Integer position;
    _Bucket bucket;

    /*
    **  Read data from one or more Buckets
    */

    copied = 0;
    left = size;

    /*
    **	The Cursor position may be in the current Bucket, or the current Bucket
    **	Extension.
    */

    if (cursor_position <= _BucketDataSize(cursor->bucket)) {
	base = 0;
	position = cursor_position;
	bucket = cursor->bucket;
    }
    else {
	base = cursor->ext_base;
	position = cursor_position - cursor->ext_base;
	bucket = cursor->ext_bucket;
    }

    /*
    **  Read the Data
    */

    while (left > 0) {
	/*
	**  Read a chunk from this Bucket
	*/

	if (left <= _BucketDataSize(bucket) - position)
	    chunk = left;
	else
	    chunk = _BucketDataSize(bucket) - position;

	if (data != (_AnyPtr) 0)
	    _CopyMem(&bucket->buffer->data[position],
		&((_CharPtr) data)[copied], chunk);

	position += chunk;
	copied += chunk;
	left -= chunk;

	/*
	**  If necessary, Read this Bucket's extension(s) for the remainder
	*/

	if (left > 0) {
	    position = 0;
	    base += _BucketDataSize(bucket);
	    bucket = LwkGCReadBucketExtension(cursor);
	}
    }

    return base + position;
    }
