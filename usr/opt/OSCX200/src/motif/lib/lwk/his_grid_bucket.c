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

_DeclareFunction(static void FreeBucket, (_GridFile file, _Bucket bucket));
_DeclareFunction(static _Bucket AllocateBucket,
    (_GridFile file, _BucketNumber number));
_DeclareFunction(static _Bucket CreateBucket,
    (_GridFile file, _BucketNumber number));
_DeclareFunction(static void WriteBucket, (_GridFile file, _Bucket bucket));
_DeclareFunction(static _Boolean BucketIsReusable,
    (_GridFile file, _Bucket bucket));
_DeclareFunction(static void SetMostRecentBucket,
    (_GridFile file, _Bucket bucket));


void  LwkGBFreeBuckets(file)
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
    **  Free all Buckets
    */

    while (file->most_recent != (_Bucket) 0)
	FreeBucket(file, file->most_recent);

    return;
    }


_Bucket  LwkGBAllocateNextBucket(file)
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
    _Bucket bucket;

    /*
    **	Allocate and Create the next free Bucket.
    */

    bucket = LwkGBAllocateBucket(file, file->next_bucket);

    /*
    **  Update the Directory
    */

    file->next_bucket++;
    file->directory_modified = _True;

    return bucket;
    }


_Bucket  LwkGBAllocateBucket(file, number)
_GridFile file;
 _BucketNumber number;

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
    _Bucket bucket;

    /*
    **  Verify transaction state
    */

    if (file->state != lwk_c_transact_read_write)
	_Raise(db_read_error);

    /*
    **	Allocate and create the Bucket.
    */

    bucket = AllocateBucket(file, number);

    return bucket;
    }


void  LwkGBDeallocateBucket(file, bucket)
_GridFile file;
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
    /*
    ** We currently only support deallocation of the most recently allocated
    ** Bucket.
    */

    if (bucket->number == file->next_bucket - 1) {
	if (_TraceWrite)
	    _Trace("Deallocate Bucket %ld -- File %s\n",
		bucket->number, file->identifier);

	file->next_bucket--;
	file->directory_modified = _True;
    }

    /*
    ** Free the Bucket
    */

    FreeBucket(file, bucket);

    return;
    }


_Bucket  LwkGBReadBucket(file, number)
_GridFile file;
 _BucketNumber number;

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
    _Bucket bucket;

    /*
    **  Search for the Bucket in the cache
    */

    bucket = file->most_recent;

    while (bucket != (_Bucket) 0) {
	if (bucket->number == number)
	    break;

	/*
	**  If we are back to the beginning of the list, we are done
	*/

	if ((bucket = bucket->next) == file->most_recent)
	    bucket = (_Bucket) 0;
    }

    /*
    **	If the Bucket was not in the cache and we have reached the maximum
    **	Bucket count, try to reuse an existing, unmodified, inactive Bucket.
    */

    if (bucket == (_Bucket) 0 && file->cache_count >= file->cache_size) {
	bucket = file->least_recent;

	while (bucket != (_Bucket) 0) {
	    if (BucketIsReusable(file, bucket)) {
		if (_TraceRead)
		    _Trace("Reuse Bucket %ld -- File %s\n",
			bucket->number, file->identifier);

		bucket->number = number;
		bucket->buffer_valid = _False;

		break;
	    }

	    /*
	    **  If we are back to the beginning of the list, we are done
	    */

	    if ((bucket = bucket->previous) == file->least_recent)
		bucket = (_Bucket) 0;
	}
    }

    /*
    **	If we must, create a new Bucket (automatically most-recently-used).  If
    **	we are re-using an existing Bucket, make it the most-recently-used.
    */

    if (bucket == (_Bucket) 0)
	bucket = CreateBucket(file, number);
    else
	SetMostRecentBucket(file, bucket);

    /*
    **  Set a read lock on the Bucket
    */

    LwkGLLockBucket(file, bucket->number, lwk_c_transact_read);

    /*
    **  Read in the Bucket from the File if necessary
    */

    if (!bucket->buffer_valid) {
	if (_TraceRead)
	    _Trace("Read Bucket %ld -- File %s\n", bucket->number,
		file->identifier);

	LwkGFReadFile(file, bucket);

	bucket->buffer_valid = _True;
    }

    return bucket;
    }


_Bucket  LwkGBExtendBucket(file, bucket)
_GridFile file;
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
    _Bucket next;

    /*
    **  Either use the existing extension, or create a new one
    */

    if (bucket->buffer->extension != (_BucketNumber) 0) {
	next = LwkGBReadBucket(file, bucket->buffer->extension);
	next->buffer->first_free = 0;
    }
    else {
	next = LwkGBAllocateNextBucket(file);
	bucket->buffer->extension = next->number;
    }

    return next;
    }


void  LwkGBVerifyBuckets(file, flags, list_file)
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
    _Integer bytes_free;
    _Integer bytes_available;
    _Integer bytes_in_use;
    _Integer record_count;
    _Integer total_free;
    _Integer total_bytes;
    _Integer total_buckets;
    _BucketNumber number;
    _GridCursor cursor;

    /*
    **  Create a Cursor which will select all records
    */

    cursor = LwkGCCreateVerifyCursor(file);

    /*
    **  Loop over each record and Verify/List it.
    */

    number = -1;
    total_buckets = 0;
    total_bytes = 0;
    total_free = 0;
    record_count = 0;
    bytes_in_use = 0;
    bytes_free = 0;

    while (LwkGRNextRecordBucket(cursor)) {
	if (cursor->bucket->number != number) {
	    total_buckets++;

	    if (number >= 0)
		fprintf(list_file,
		    "    Summary: %ld Records, %ld bytes in use\n",
		    record_count, bytes_in_use);

	    record_count = 0;

	    total_bytes += bytes_in_use;
	    bytes_in_use = 0;

	    bytes_available = _BucketDataSize(cursor->bucket);

	    bytes_free = _BucketDataSize(cursor->bucket)
		- cursor->bucket->buffer->first_free;

	    total_free += bytes_free;

	    number = cursor->bucket->number;

	    fprintf(list_file, "\nBucket %ld: Size: %ld, Records: %ld,", number,
		cursor->bucket->size, cursor->bucket->buffer->record_count);

	    if (cursor->bucket->buffer->extension != (_BucketNumber) 0) {
		_Bucket bucket;
		_BucketNumber extension;

		fputs(" Extension(s):", list_file);

		extension = cursor->bucket->buffer->extension;

		do {
		    fprintf(list_file, " %ld,", extension);

		    bucket = LwkGBReadBucket(cursor->file, extension);
		    extension = bucket->buffer->extension;

		    bytes_available += _BucketDataSize(bucket);

		    /*
		    **	Be careful here -- the first_free values for Bucket
		    **	extensions are not updated as Records are deleted!
		    */

		    if (bytes_free > 0)
			bytes_free += _BucketDataSize(bucket);
		    else
			bytes_free += _BucketDataSize(bucket)
			    - bucket->buffer->first_free;
		} while (extension != (_BucketNumber) 0);
	    }

	    fprintf(list_file, " Free: %ld of %ld\n",
		bytes_free, bytes_available);
	}

	if (cursor->bucket->buffer->first_free > 0) {
	    bytes_in_use += LwkGRVerifyRecord(cursor, flags, list_file);

	    record_count++;
	    cursor->select_count++;
	}
    }

    if (number >= 0) {
	total_buckets++;
	total_bytes += bytes_in_use;

	fprintf(list_file, "    Summary: %ld Records, %ld bytes in use\n",
	    record_count, bytes_in_use);
    }

    fprintf(list_file,
	"\nGrand Total of %ld buckets, %ld Records, %ld Bytes, %ld Free",
	total_buckets, cursor->select_count, total_bytes, total_free);

    /*
    **  Free the Cursor
    */

    LwkGCFreeCursor(cursor);

    return;
    }


void  LwkGBFlushBuckets(file)
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
    int cache_count;
    _Bucket bucket;
    _BucketNumber next;
    _BucketNumber subsequent;

    if (_TraceWrite)
	_Trace("Flush Buckets -- File %s\n", file->identifier);

    /*
    **  Write out all modified Buckets in ascending order
    */

    next = subsequent = -1;

    while (_True) {
	/*
	**  Make a loop over the Bucket Cache list writting the next Bucket and
	**  checking for the subsequent
	*/

	bucket = file->most_recent;

	while (bucket != (_Bucket) 0) {
	    /*
	    **	On the first pass, request read/write locks for all modified
	    **	Buckets, and invalidate the root Directory Bucket so that
	    **	we will be sure to re-read it at the beginning of the next
	    **	Transaction
	    */

	    if (next < 0) {
		if (bucket->modified)
		    LwkGLLockBucket(file, bucket->number,
			lwk_c_transact_read_write);

		if (bucket->number == _DirectoryBucket)
		    bucket->buffer_valid = _False;
	    }

	    /*
	    **	If this is the next Bucket to write, do so, else see if it will
	    **	be the one to write on the subsequent pass.
	    */

	    if (bucket->modified) {
		if (bucket->number == next) {
		    WriteBucket(file, bucket);
		    bucket->modified = _False;
		}

		if (bucket->number > next &&
			(next == subsequent || bucket->number < subsequent))
		    subsequent = bucket->number;
	    }

	    /*
	    **  If we are back to the beginning of the list, this pass is
	    **	complete
	    */

	    if ((bucket = bucket->next) == file->most_recent)
		bucket = (_Bucket) 0;
	}

	/*
	**  Prepare to write the next Bucket, if there is one
	*/

	if (next != subsequent)
	    next = subsequent;
	else
	    break;
    }

    /*
    **  Try to trim back the Bucket cache to the maximum size -- limit the
    **	number of times we try since in some cases, it might not be possible.
    */

    cache_count = file->cache_count;

    while (cache_count > 0 && file->cache_count > file->cache_size) {
	if (BucketIsReusable(file, file->least_recent))
	    FreeBucket(file, file->least_recent);

	cache_count--;
    }

    /*
    **  Reset the modified state of the File
    */

    file->file_modified = _False;

    return;
    }


void  LwkGBInvalidateBuckets(file)
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
    _Bucket bucket;

    if (_TraceRead)
	_Trace("Invalidate Buckets -- File %s\n", file->identifier);

    /*
    **  Invalidate all Buckets
    */

    bucket = file->most_recent;

    while (bucket != (_Bucket) 0) {
	bucket->buffer_valid = _False;
	bucket->modified = _False;

	/*
	**  If we are back to the beginning of the list, we are done
	*/

	if ((bucket = bucket->next) == file->most_recent)
	    bucket = (_Bucket) 0;
    }

    return;
    }


static _Bucket  AllocateBucket(file, number)
_GridFile file;
 _BucketNumber number;

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
    _Bucket bucket;

    /*
    ** Allocate a new Bucket in the File
    */

    LwkGFAllocateBucket(file, number);

    /*
    **	Create the Bucket.
    */

    bucket = CreateBucket(file, number);

    /*
    **	Mark the Bucket valid and modified so it will be written into the file
    **	even if no data is stored in the Bucket
    */

    bucket->buffer_valid = _True;
    bucket->modified = _True;

    return bucket;
    }


static void  FreeBucket(file, bucket)
_GridFile file;
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
    if (_TraceRead)
	_Trace("Free Bucket %ld -- File %s\n", bucket->number,
	    file->identifier);

    /*
    **  Reduce the Bucket cache count
    */

    file->cache_count--;

    /*
    **  Remove this Bucket from the Bucket cache
    */

    if (file->cache_count == 0)
	file->most_recent = file->least_recent = (_Bucket) 0;
    else {
	if (file->most_recent == bucket)
	    file->most_recent = bucket->next;

	if (file->least_recent == bucket)
	    file->least_recent = bucket->previous;

	bucket->previous->next = bucket->next;
	bucket->next->previous = bucket->previous;
    }

    /*
    **  Free the Bucket and its Buffer
    */

    _FreeMem(bucket->buffer);
    _FreeMem(bucket);

    return;
    }


static _Bucket  CreateBucket(file, number)
_GridFile file;
 _BucketNumber number;

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
    _Bucket bucket;
    _Buffer buffer;

    /*
    **  Create and initialize a new Bucket
    */

    bucket = (_Bucket) _AllocateMem(sizeof(_BucketInstance));

    bucket->number = number;
    bucket->modified = _False;
    bucket->buffer_valid = _False;
    bucket->size = file->bucket_size;

    buffer = (_Buffer) _AllocateMem(bucket->size);

    buffer->first_free = 0;
    buffer->record_count = 0;
    buffer->extension = (_BucketNumber) 0;

    bucket->buffer = buffer;

    /*
    **  Link this Bucket into the beginning of the Bucket list
    */

    if (file->most_recent == (_Bucket) 0) {
	file->most_recent = bucket;
	file->least_recent = bucket;

	bucket->next = bucket;
	bucket->previous = bucket;
    }
    else {
	file->most_recent->previous = bucket;
	file->least_recent->next = bucket;

	bucket->next = file->most_recent;
	bucket->previous = file->least_recent;
    }

    file->most_recent = bucket;

    /*
    **  Count the new Bucket
    */

    file->cache_count++;

    return bucket;
    }


static void  WriteBucket(file, bucket)
_GridFile file;
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
    /*
    **  Write the Bucket
    */

    if (_TraceWrite)
	_Trace("Write Bucket %ld -- File %s\n", bucket->number,
	    file->identifier);

    LwkGFWriteFile(file, bucket);

    return;
    }


static _Boolean  BucketIsReusable(file, bucket)
_GridFile file;
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
    _GridCursor cursor;

    /*
    **  If the Bucket is modified, it can not be reused
    */

    if (bucket->modified)
	return _False;

    /*
    **  If the Bucket is in use by an active Cursor, it can not be reused
    */

    cursor = (_GridCursor) file->cursors;

    while (cursor != (_GridCursor) 0) {
	if (cursor->bucket == bucket || cursor->ext_bucket == bucket)
	    return _False;

	cursor = cursor->next;
    }

    /*
    **  Otherwise, away!
    */

    return _True;
    }


static void  SetMostRecentBucket(file, bucket)
_GridFile file;
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
    /*
    **  If this Bucket is not already the most-recently-user, make it so.
    */

    if (file->most_recent != bucket) {
	/*
	**  Unlink this Bucket from the list
	*/

	bucket->previous->next = bucket->next;
	bucket->next->previous = bucket->previous;

	if (file->least_recent == bucket)
	    file->least_recent = bucket->previous;

	/*
	**  Relink this Bucket into the list
	*/

	file->most_recent->previous = bucket;
	file->least_recent->next = bucket;

	bucket->next = file->most_recent;
	bucket->previous = file->least_recent;

	file->most_recent = bucket;
    }

    return;
    }
