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
#endif /* MSDOS */

/*
**  Type Definitions
*/

#ifdef _Prototypes
typedef _Termination (*_SelectCallback)(_Region region, _Closure closure);
#else
typedef _Termination (*_SelectCallback)();
#endif

/*
**  Macro Definitions
*/

/*
**  Forward Routine Declarations
*/

_DeclareFunction(static void FreeKeys, (_GridFile file));
_DeclareFunction(static _Boolean ReadDirectoryHeader,
    (_GridFile file, _GridCursor cursor, _DynamicVString name));
_DeclareFunction(static void ReadKeys,
    (_GridFile file, _GridCursor cursor));
_DeclareFunction(static _Key ReadKey,
    (_GridFile file, _GridCursor cursor));
_DeclareFunction(static _KeyInterval ReadKeyInterval,
    (_GridFile file, _GridCursor cursor, _Domain domain));
_DeclareFunction(static void ReadKeyValue,
    (_GridFile file, _GridCursor cursor, _Domain domain,
	_KeyValuePtr value));
_DeclareFunction(static _Bucket WriteDirectoryHeader,
    (_GridFile file, _Bucket bucket));
_DeclareFunction(static _Bucket WriteDirectoryKeys,
    (_GridFile file, _Bucket bucket));
_DeclareFunction(static _Bucket WriteKey,
    (_GridFile file, _Bucket bucket, _Key key));
_DeclareFunction(static _Bucket WriteKeyInterval,
    (_GridFile file, _Bucket bucket, _Domain domain, _KeyInterval interval));
_DeclareFunction(static _Bucket WriteKeyValue,
    (_GridFile file, _Bucket bucket, _Domain domain, _KeyValue value));
_DeclareFunction(static _Region CreateRegion,
    (_GridFile file, _RegionVertex base, _RegionVertex extent,
	_BucketNumber number));
_DeclareFunction(static void AddRegion, (_GridFile file, _Region region));
_DeclareFunction(static void RemoveRegion, (_GridFile file, _Region region));
_DeclareFunction(static _Region MergeRegions,
    (_GridFile file, _Region region1, _Region region2, _BucketNumber bucket));
_DeclareFunction(static void SelectIntersectingRegions,
    (_GridFile file, _Region region, _SelectCallback callback,
	_Closure closure));
_DeclareFunction(static void FreeRegions, (_GridFile file));
_DeclareFunction(static void FreeRegion, (_Region region));
_DeclareFunction(static void ReadRegions, (_GridFile file));
_DeclareFunction(static void WriteRegions, (_GridFile file, _Bucket bucket));
_DeclareFunction(static void RefineRegions,
    (_GridFile file, _RegionVertex vertex));
_DeclareFunction(static void VerifyRegions,
    (_GridFile file, _Integer flags, _OSFile list_file));
_DeclareFunction(static void SplitRegion,
    (_GridFile file, _Region region, _RegionVertex vertex));
_DeclareFunction(static _Boolean SplitDimension,
    (_GridFile file, _Integer dimension, _Integer plane));
_DeclareFunction(static _Region RegionToStore,
    (_GridFile file, _ConjunctKeys keys, _RegionVertex vertex));
_DeclareFunction(static _Boolean GetKeyPropertyValue,
    (_Key key, _ConjunctKeys keys, _KeyValue *value));
_DeclareFunction(static int KeyRetrieveInterval, (_Key key, _KeyValue value));
_DeclareFunction(static int KeyStoreInterval,
    (_GridFile file, _Key key, _KeyValue value));
_DeclareFunction(static int KeyInterval,
    (_Key key, _KeyValue value, _KeyInterval *interval));
_DeclareFunction(static _Boolean IsBetterSplit,
    (_KeyValue *base, _KeyValue *extent, _KeyValue *split, _KeyValue *value,
	_Domain domain));
_DeclareFunction(static void VerifyKeys,
    (_GridFile file, _Integer flags, _OSFile list_file));
_DeclareFunction(static void ListIntervalValue,
    (_Key key, _KeyValue value, _OSFile list_file));
_DeclareFunction(static void ListString, (_String string, _OSFile list_file));
_DeclareFunction(static void PrintInterval, (_Key key, _Integer index));
_DeclareFunction(static _Termination _EntryPt FindProperty,
    (_Closure closure, _Set set, _Domain domain, _Property *property));
_DeclareFunction(static _Termination AddBucketToList,
    (_Region region, _Closure closure));
_DeclareFunction(static _Termination SelectRegion,
    (_Region region, _Closure closure));
_DeclareFunction(static _Termination RefineRegion,
    (_Region region, _Closure closure));
_DeclareFunction(static int MoveRecords,
    (_GridFile file, _Region old_region, _Region new_region,
     _Bucket new_bucket));

/*
**  Static storage
*/

static _PredefinedKey PredefinedKeys[] = _PreDefinedKeysInitialization;

static int PredefinedKeyCount =
    sizeof(PredefinedKeys) / sizeof(_PredefinedKey);


void  LwkGDInitializeDirectory(file)
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
    int i;
    _Bucket bucket;
    _Region region;
    _RegionVertex base_vertex;
    _RegionVertex extent_vertex;

    /*
    **  Allocate Buckets for the Directory and Grid Regions
    */

    bucket = LwkGBAllocateBucket(file, _DirectoryBucket);
    bucket = LwkGBAllocateBucket(file, _RegionsBucket);

    /*
    **  Allocate a new data Bucket in the File
    */

    bucket = LwkGBAllocateNextBucket(file);

    /*
    **  Create a single Grid Region based on the current key dimensions
    */

    for (i = 0; i < file->key_count; i++) {
	base_vertex[i] = 0;
	extent_vertex[i] = file->keys[i]->interval_count;
    }

    region = CreateRegion(file, base_vertex, extent_vertex, bucket->number);

    AddRegion(file, region);

    /*
    **  Mark the Grid Regions modified
    */

    file->regions_modified = _True;

    return;
    }


void  LwkGDFreeDirectory(file)
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
    **  Free all Keys
    */

    FreeKeys(file);

    /*
    **  Free the Regions
    */

    FreeRegions(file);

    return;
    }


void  LwkGDReadDirectory(file)
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
    _Key key;
    _Integer file_sn;
    _Integer regions_sn;
    _Integer directory_sn;
    _GridCursor cursor;
    _DynamicVString name;

    /*
    **  Initialize
    */

    _CreateDVString(name);
    cursor = (_GridCursor) 0;

    _StartExceptionBlock

    /*
    **  Save the last known File serial numbers
    */

    file_sn = file->file_sn;
    regions_sn = file->regions_sn;
    directory_sn = file->directory_sn;

    /*
    **  Read the Directory information from the File.  It might take more than
    **	one attempt if the default Bucket size does not match the actual Bucket
    **	size.
    */

    if (_TraceRead)
	_Trace("Read Directory -- File %s\n", file->identifier);

    while (_True) {
	/*
	**  Create a Cursor to read the Directory
	*/

	cursor = LwkGCCreateDirectoryCursor(file);

	/*
	**  Position the Cursor to the beginning
	*/

	if (!LwkGRNextRecord(cursor, _True))
	    _Raise(db_read_error);

	/*
	**  Read the Directory Header.  If it fails because of Bucket size
	**  mis-match, try again with the correct Bucket size.
	*/

	if (ReadDirectoryHeader(file, cursor, name))
	    break;

	LwkGCFreeCursor(cursor);
    }

    /*
    **  Check that the serial numbers have remained static or increased -- if
    **	this is not the case, there is some flaw in the Lock management!
    */

    if (file->regions_sn < regions_sn || file->directory_sn < directory_sn)
	_Raise(db_read_error);

    /*
    **  If the File has changed, re-read the remaining File information
    */

    if (file->file_sn > file_sn) {
	/*
	**  Invalidate the Bucket cache
	*/

	LwkGBInvalidateBuckets(file);

	/*
	**  Refresh File information
	*/

	LwkGFRefreshFile(file);

	/*
	**  If necessary, free the old Keys and re-read
	*/

	if (file->directory_sn > directory_sn) {
	    FreeKeys(file);
	    ReadKeys(file, cursor);
	}

	/*
	**  If necessary, free the old Grid Regions and re-read
	*/

	if (file->regions_sn > regions_sn) {
	    FreeRegions(file);
	    ReadRegions(file);
	}

	/*
	**  Delete any old Name and reset it
	*/

	_DeleteDDIFString(&file->name);
	_VaryingStringToDDIFString(_DVString_of(name), file->name);
    }

    /*
    **  Clean up
    */

    _DeleteDVString(&name);

    LwkGCFreeCursor(cursor);

    /*
    **	If any exceptions are raised, clean up then reraise the exception.
    */
    
    _Exceptions
	_WhenOthers
	    _DeleteDVString(&name);
	    LwkGCFreeCursor(cursor);
	    LwkGFRollbackFile(file, _False);
	    _Reraise;
    _EndExceptionBlock

    return;
    }


void  LwkGDWriteDirectory(file)
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
    **	If the File has not been modified since the last time it was written,
    **	return now.
    */

    if (!(file->file_modified || file->directory_modified
	    || file->regions_modified))
	return;

    /*
    **	If necessary, write the Grid Regions.  We do this first since it might
    **	allocate new Buckets in the File.
    */

    if (file->regions_modified) {
	/*
	**  Increment the Regions serial number
	*/

	file->regions_sn++;

	/*
	**  Read Bucket where the Regions are stored
	*/

	bucket = LwkGBReadBucket(file, _RegionsBucket);
	bucket->buffer->first_free = 0;

	/*
	**  Store the Grid Regions in it
	*/

	WriteRegions(file, bucket);
    }

    /*
    **	Increment the File serial number.  If the Directory has been modified,
    **	increment its serial number too.
    */

    file->file_sn++;

    if (file->directory_modified)
	file->directory_sn++;

    /*
    **  Write the Directory
    */

    if (_TraceWrite)
	_Trace("Write Directory -- File %s\n", file->identifier);

    /*
    **  Read Bucket where the Directory is stored
    */

    bucket = LwkGBReadBucket(file, _DirectoryBucket);
    bucket->buffer->first_free = 0;

    /*
    **  Write the Directory Header
    */

    bucket = WriteDirectoryHeader(file, bucket);

    /*
    **  Write all Key information
    */

    bucket = WriteDirectoryKeys(file, bucket);

    /*
    **  Reset the modified state of the File
    */

    file->regions_modified = _False;
    file->directory_modified = _False;

    return;
    }


void  LwkGDVerifyDirectory(file, flags, list_file)
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
    _String string;

    /*
    **  Verify/List general Grid File attributes if requested
    */

    fprintf(list_file, "Repository Identifier: %s\n", file->identifier);

    string = _DDIFStringToString(file->name);
    fprintf(list_file, "Repository Name: %s\n", string);
    _DeleteString(&string);

    fprintf(list_file, "Bucket size: %ld\n", file->bucket_size);
    fprintf(list_file, "File version: %ld\n", file->version);
    fprintf(list_file, "File serial number: %ld\n", file->file_sn);
    fprintf(list_file, "Regions serial number: %ld\n", file->regions_sn);
    fprintf(list_file, "Directory serial number: %ld\n", file->directory_sn);
    fprintf(list_file, "Next free Bucket: %ld\n", file->next_bucket);
    fprintf(list_file, "Next free Object Identifier: %ld\n", file->next_id);

    /*
    **  Verify/List Keys if requested
    */

    VerifyKeys(file, flags, list_file);

    /*
    **  Verify/List Grid Regions
    */

    VerifyRegions(file, flags, list_file);

    return;
    }


_Bucket  LwkGDBucketToStore(file, size, new, keys)
_GridFile file;
 _Integer size;
 _Boolean new;

    _ConjunctKeys keys;

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
    _Region region;
    _RegionVertex vertex;

    /*
    **  Loop looking for a Bucket in which to store this Record.
    */

    while (_True) {
	/*
	**  Get the vertex where this Record should be stored, and the Region
	**  which contains it.
	*/

	region = RegionToStore(file, keys, vertex);

	/*
	**  If this is a rewrite of the Record, delete the old version of the
	**  Record.
	*/

	if (!new) {
	    LwkGRDeleteRewrittenRecord(file, region->bucket, keys->domain,
		keys->identifier);

	    new = _True;
	}

	/*
	**  Read in the Bucket
	*/

	bucket = LwkGBReadBucket(file, region->bucket);

	/*
	**  If there is room for this Record in the Bucket we are done.
	*/

	if (_BucketFreeSize(bucket) >= size)
	    break;

	/*
	**  If this Bucket is empty and the Record still won't fit, we'll just
	**  have to extend the Bucket!
	*/

	if (bucket->buffer->first_free == 0)
	    break;

	/*
	**  No room in this Bucket -- split the Region, and try again.
	*/

	SplitRegion(file, region, vertex);
    }

    if (_TraceStore)
	_Trace("Store Record -- Bucket %ld File %s\n", bucket->number,
	    file->identifier);

    /*
    **  Increment Record count
    */

    bucket->buffer->record_count++;

    /*
    **  Mark the File modified
    */

    file->file_modified = _True;

    return bucket;
    }


void  LwkGDBucketsToRetrieve(file, keys, bucket_list)
_GridFile file;
 _ConjunctKeys keys;

    _BucketList bucket_list;

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
    _KeyValue value;
    _Boolean specified;
    _Boolean none_such;
    _RegionVertex base_vertex;
    _RegionVertex extent_vertex;

    /*
    **  Initialize
    */

    bucket_list->count = 0;
    bucket_list->numbers = (_BucketNumberPtr) 0;

    /*
    **  Loop over each disjunct set of Key values
    */

    while (keys != (_ConjunctKeys) 0) {
	none_such = _False;

	/*
	** Check each conjunct Key value
	*/

	for (i = 0; i < file->key_count; i++) {
	    specified = _True;

	    switch (file->keys[i]->type) {
		case _KeyDomain :
		    value.integer = (_Integer) keys->domain;
		    break;

		case _KeyContainer :
		    if (keys->container > 0)
			value.integer = keys->container;
		    else
			specified = _False;

		    break;

		case _KeyIdentifier :
		    if (keys->identifier > 0)
			value.integer = keys->identifier;
		    else
			specified = _False;

		    break;

		case _KeyProperty :
		    if (keys->key_properties != (_Set) _NullObject)
			specified = GetKeyPropertyValue(file->keys[i],
			    keys, &value);
		    else
			specified = _False;

		    break;
	    }

	    /*
	    **  If the Key was specified, search the appropriate Key Interval,
	    **	otherwise, search all Key intervals
	    */

	    if (specified) {
		base_vertex[i] = extent_vertex[i] =
		    KeyRetrieveInterval(file->keys[i], value);

		/*
		**  Delete the value of it came from the Key Property Set
		*/

		if (file->keys[i]->type == _KeyProperty)
		    _DeleteValue(&value, file->keys[i]->domain);

		/*
		**  If this Key value is known not to exist, exit loop now
		*/

		if (base_vertex[i] < 0) {
		    none_such = _True;
		    break;
		}
	    }
	    else {
		base_vertex[i] = 0;
		extent_vertex[i] = file->keys[i]->interval_count;
	    }
	}

	/*
	**  Unless we are certain that there are no such Records, get the
	**  potential list of Buckets for selected Records.
	*/

	if (!none_such)
	    LwkGDGetBucketList(file, base_vertex, extent_vertex, bucket_list);

	/*
	**  Check the next disjunct set of Key values
	*/

	keys = keys->disjunct;
    }

    return;
    }


void  LwkGDGetBucketList(file, base_vertex, extent_vertex, bucket_list)
_GridFile file;
 _RegionVertex base_vertex;

    _RegionVertex extent_vertex;
 _BucketList bucket_list;

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
    _Region region;

    /*
    ** If there are no Regions in the File, return right now
    */

    if (file->regions == (_Region) 0) {
	bucket_list->count = 0;
	return;
    }

    /*
    ** Create a dummy Region with the given vertices
    */
    
    region = CreateRegion(file, base_vertex, extent_vertex, 0);

    /*
    ** Traverse the Region list to find all Regions which intersect the Region
    ** specified and add the associated Bucket numbers to the Bucket list.
    */

    SelectIntersectingRegions(file, region, AddBucketToList,
	(_Closure) bucket_list);

    /*
    ** Clean up
    */

    FreeRegion(region);

    return;
    }


void  LwkGDInitializeKeys(file)
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
    int i;
    int adk_cnt;
    _Key key;
    _KeyInterval interval;

    /*
    **  Create all predefined Keys
    */

    adk_cnt = 0;

    for (i = 0; i < PredefinedKeyCount; i++) {
	key = (_Key) _AllocateMem(sizeof(_KeyInstance));

	key->type = PredefinedKeys[i].type;
	key->name = _CopyString(PredefinedKeys[i].name);
	key->domain = PredefinedKeys[i].domain;
	key->intervals = (_KeyInterval) 0;
	key->interval_count = 0;

	switch (key->domain) {
	    case lwk_c_domain_integer :
		key->base.integer = LONG_MAX;
		key->split.integer = LONG_MIN;
		key->extent.integer = LONG_MIN;
		break;

	    case lwk_c_domain_string :
		key->base.string = (_String) 0;
		key->split.string = (_String) 0;
		key->extent.string = (_String) 0;
		break;

	    case lwk_c_domain_ddif_string :
		key->base.ddifstring = (_DDIFString) 0;
		key->split.ddifstring = (_DDIFString) 0;
		key->extent.ddifstring = (_DDIFString) 0;
		break;

	    case lwk_c_domain_date :
		key->base.date = (_Date) 0;
		key->split.date = (_Date) 0;
		key->extent.date = (_Date) 0;
		break;
	}

	if (key->type == _KeyProperty) {
	    file->keys[(int) _KeyProperty + adk_cnt] = key;
	    adk_cnt++;
	}
	else
	    file->keys[(int) key->type] = key;

	file->key_count++;
    }

    /*
    **  Initialize the Domain Key intervals (one for each possible Domain)
    */

    key = file->keys[(int) _KeyDomain];

    interval = (_KeyInterval) _AllocateMem(sizeof(_KeyIntervalInstance));

    interval->base.integer = (_Integer) lwk_c_domain_comp_path;
    interval->split.integer = (_Integer) lwk_c_domain_comp_path;
    interval->next = key->intervals;
    key->intervals = interval;
    key->interval_count++;

    interval = (_KeyInterval) _AllocateMem(sizeof(_KeyIntervalInstance));

    interval->base.integer = (_Integer) lwk_c_domain_path;
    interval->split.integer = (_Integer) lwk_c_domain_path;
    interval->next = key->intervals;
    key->intervals = interval;
    key->interval_count++;

    interval = (_KeyInterval) _AllocateMem(sizeof(_KeyIntervalInstance));

    interval->base.integer = (_Integer) lwk_c_domain_step;
    interval->split.integer = (_Integer) lwk_c_domain_step;
    interval->next = key->intervals;
    key->intervals = interval;
    key->interval_count++;

    interval = (_KeyInterval) _AllocateMem(sizeof(_KeyIntervalInstance));

    interval->base.integer = (_Integer) lwk_c_domain_comp_linknet;
    interval->split.integer = (_Integer) lwk_c_domain_comp_linknet;
    interval->next = key->intervals;
    key->intervals = interval;
    key->interval_count++;

    interval = (_KeyInterval) _AllocateMem(sizeof(_KeyIntervalInstance));

    interval->base.integer = (_Integer) lwk_c_domain_linknet;
    interval->split.integer = (_Integer) lwk_c_domain_linknet;
    interval->next = key->intervals;
    key->intervals = interval;
    key->interval_count++;

    interval = (_KeyInterval) _AllocateMem(sizeof(_KeyIntervalInstance));

    interval->base.integer = (_Integer) lwk_c_domain_link;
    interval->split.integer = (_Integer) lwk_c_domain_link;
    interval->next = key->intervals;
    key->intervals = interval;
    key->interval_count++;

    /*
    **  Mark the Directory modified
    */

    file->directory_modified = _True;

    return;
    }


_CharPtr  LwkGDDomainToName(domain)
_Domain domain;

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

    switch (domain) {
	case lwk_c_domain_comp_linknet :
	    name = "Composite Network";
	    break;

	case lwk_c_domain_comp_path :
	    name = "Composite Path";
	    break;

	case lwk_c_domain_linknet :
	    name = "Network";
	    break;

	case lwk_c_domain_path :
	    name = "Path";
	    break;

	case lwk_c_domain_link :
	    name = "Connection";
	    break;

	case lwk_c_domain_step :
	    name = "Step";
	    break;

	case lwk_c_domain_surrogate :
	    name = "Surrogate";
	    break;

	case lwk_c_domain_integer :
	    name = "Integer";
	    break;

	case lwk_c_domain_float :
	    name = "Float";
	    break;

	case lwk_c_domain_string :
	    name = "String";
	    break;

	case lwk_c_domain_ddif_string :
	    name = "DDIF String";
	    break;

	case lwk_c_domain_date :
	    name = "Date";
	    break;

	case lwk_c_domain_unknown :
	    name = "Any";
	    break;

	default :
	    name = "Unknown";
	    break;
    }

    return name;
    }


static void  FreeKeys(file)
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
    int i;
    _Key key;
    _KeyInterval interval;

    /*
    **  Free storage for each Key
    */

    for (i = 0; i < file->key_count; i++) {
	key = file->keys[i];
	file->keys[i] = (_Key) 0;

	_DeleteString(&key->name);

	_DeleteValue(&key->base, key->domain);
	_DeleteValue(&key->split, key->domain);
	_DeleteValue(&key->extent, key->domain);

	/*
	**  Free each Key Interval
	*/

	while (key->intervals != (_KeyInterval) 0) {
	    interval = key->intervals;

	    _DeleteValue(&interval->base, key->domain);
	    _DeleteValue(&interval->split, key->domain);

	    key->intervals = interval->next;

	    _FreeMem(interval);

	    key->interval_count--;
	}

	_FreeMem(key);
    }

    file->key_count = 0;

    return;
    }


static _Boolean  ReadDirectoryHeader(file, cursor, name)
_GridFile file;
 _GridCursor cursor;

    _DynamicVString name;

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
    _Integer sn;

    /*
    **  Get the Bucket size
    */

    cursor->position = LwkGRRetrieveInteger(cursor, cursor->position,
	&file->bucket_size);

    /*
    **  Get the File version
    */

    cursor->position = LwkGRRetrieveInteger(cursor, cursor->position,
	&file->version);

    /*
    **	If the File version is different from the version we support, raise an
    **	exception
    */

    if (file->version != _FileVersion)
	_Raise(db_version_error);

    /*
    **	If the Bucket size is different from the size of the Bucket we used to
    **	read the Directory, we'll need to start over again
    */

    if (cursor->bucket->size != file->bucket_size)
	return _False;

    /*
    **  Save the last known File serial number and retrieve the current one.
    */

    sn = file->file_sn;

    cursor->position = LwkGRRetrieveInteger(cursor, cursor->position,
	&file->file_sn);

    /*
    **  Check that the serial number remained static or increased -- if
    **	this is not the case, there is some flaw in the Lock management!
    */

    if (file->file_sn < sn)
	_Raise(db_read_error);

    /*
    **	If the serial number has changed, read the remaining generic File
    **	information.
    */

    if (file->file_sn > sn) {
	cursor->position = LwkGRRetrieveInteger(cursor, cursor->position,
	    &file->regions_sn);

	cursor->position = LwkGRRetrieveInteger(cursor, cursor->position,
	    &file->directory_sn);

	cursor->position = LwkGRRetrieveInteger(cursor, cursor->position,
	    &file->next_id);

	cursor->position = LwkGRRetrieveInteger(cursor, cursor->position,
	    &file->next_bucket);

	cursor->position = LwkGRRetrieveVaryingString(cursor,
	    cursor->position, name);
    }

    return _True;
    }


static void  ReadKeys(file, cursor)
_GridFile file;
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
    int i;
    _Key key;

    if (_TraceRead)
	_Trace("Read Keys -- File %s\n", file->identifier);

    /*
    **  Read the Key count
    */

    cursor->position = LwkGRRetrieveInteger(cursor, cursor->position,
	&file->key_count);

    /*
    **  Read all Keys
    */

    for (i = 0; i < file->key_count; i++)
	file->keys[i] = ReadKey(file, cursor);

    return;
    }


static _Key  ReadKey(file, cursor)
_GridFile file;
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
    int i;
    _Key key;
    _Integer integer;
    _KeyInterval interval;

    /*
    **  Allocate a new Key
    */

    key = (_Key) _AllocateMem(sizeof(_KeyInstance));

    /*
    **  Read the Key Name
    */

    cursor->position = LwkGRRetrieveString(cursor, cursor->position,
	&key->name);

    /*
    **  Read the Key Type
    */

    cursor->position = LwkGRRetrieveInteger(cursor, cursor->position,
	&integer);

    key->type = (_KeyType) integer;

    /*
    **  Read the Key Domain
    */

    cursor->position = LwkGRRetrieveInteger(cursor, cursor->position,
	&integer);

    key->domain = (_Domain) integer;

    /*
    **  Read Key base/split/extent
    */

    ReadKeyValue(file, cursor, key->domain, &key->base);
    ReadKeyValue(file, cursor, key->domain, &key->split);
    ReadKeyValue(file, cursor, key->domain, &key->extent);

    /*
    **  Read the Key Interval count
    */

    cursor->position = LwkGRRetrieveInteger(cursor, cursor->position,
	&key->interval_count);

    /*
    **  Read all Key Intervals
    */

    key->intervals = (_KeyInterval) 0;

    for (i = 0; i < key->interval_count; i++) {
	interval = ReadKeyInterval(file, cursor, key->domain);
	interval->next = key->intervals;
	key->intervals = interval;
    }

    return key;
    }


static _KeyInterval  ReadKeyInterval(file, cursor, domain)
_GridFile file;
 _GridCursor cursor;

    _Domain domain;

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
    _KeyInterval interval;

    /*
    **  Allocate a new Key Interval
    */

    interval = (_KeyInterval) _AllocateMem(sizeof(_KeyIntervalInstance));

    /*
    **  Read the Key Interval base and split values
    */

    ReadKeyValue(file, cursor, domain, &interval->base);
    ReadKeyValue(file, cursor, domain, &interval->split);

    return interval;
    }


static void  ReadKeyValue(file, cursor, domain, value)
_GridFile file;
 _GridCursor cursor;
 _Domain domain;

    _KeyValuePtr value;

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
    **  Deal with each Domain individually
    */

    switch (domain) {
	case lwk_c_domain_integer :
	    cursor->position = LwkGRRetrieveInteger(cursor, cursor->position,
		(_IntegerPtr) value);
	    break;

	case lwk_c_domain_float :
	    cursor->position = LwkGRRetrieveFloat(cursor, cursor->position,
		(_FloatPtr) value);
	    break;

	case lwk_c_domain_date :
	    cursor->position = LwkGRRetrieveDate(cursor, cursor->position,
		(_DatePtr) value);
	    break;

	case lwk_c_domain_string :
	    cursor->position = LwkGRRetrieveString(cursor, cursor->position,
		(_StringPtr) value);
	    break;

	case lwk_c_domain_ddif_string :
	    cursor->position = LwkGRRetrieveDDIFString(cursor,
		cursor->position, (_DDIFStringPtr) value);
	    break;

	default :
	    _Raise(inv_domain);
    }

    return;
    }


static _Bucket  WriteDirectoryHeader(file, bucket)
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
    _Bucket current;
    _VaryingString name;

    /*
    **  Initialize
    */

    name = (_VaryingString) 0;

    _StartExceptionBlock

    /*
    **  Write the generic File information
    */

    current = bucket;
    current = LwkGRStoreInteger(file, current, file->bucket_size);
    current = LwkGRStoreInteger(file, current, file->version);
    current = LwkGRStoreInteger(file, current, file->file_sn);
    current = LwkGRStoreInteger(file, current, file->regions_sn);
    current = LwkGRStoreInteger(file, current, file->directory_sn);
    current = LwkGRStoreInteger(file, current, file->next_id);
    current = LwkGRStoreInteger(file, current, file->next_bucket);

    _CreateVaryingString(name, file->name, _LengthDDIFString(file->name));

    current = LwkGRStoreVaryingString(file, current, name);

    /*
    **  Clean up
    */

    _DeleteVaryingString(&name);

    /*
    **	If any exceptions are raised, clean up then reraise the exception.
    */
    
    _Exceptions
	_WhenOthers
	    _DeleteVaryingString(&name);
	    _Reraise;
    _EndExceptionBlock

    return current;
    }


static _Bucket  WriteDirectoryKeys(file, bucket)
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
    int i;
    _Bucket current;

    /*
    **  Write the Key count
    */

    current = bucket;
    current = LwkGRStoreInteger(file, current, file->key_count);

    /*
    **  Write out each Key 
    */

    for (i = 0; i < file->key_count; i++)
	current = WriteKey(file, current, file->keys[i]);

    return current;
    }


static _Bucket  WriteKey(file, bucket, key)
_GridFile file;
 _Bucket bucket;
 _Key key;

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
    _Bucket current;
    _KeyInterval interval;

    /*
    **  Write the Key Name
    */

    current = bucket;
    current = LwkGRStoreString(file, current, key->name);

    /*
    **  Write the Key Type
    */

    current = LwkGRStoreInteger(file, current, (_Integer) key->type);

    /*
    **  Write the Key Domain
    */

    current = LwkGRStoreInteger(file, current, (_Integer) key->domain);

    /*
    **  Write Key base/split/extent
    */

    current = WriteKeyValue(file, current, key->domain, key->base);
    current = WriteKeyValue(file, current, key->domain, key->split);
    current = WriteKeyValue(file, current, key->domain, key->extent);

    /*
    **  Write the Key Interval count
    */

    current = LwkGRStoreInteger(file, current, key->interval_count);

    /*
    **  Write out the Key Intervals
    */

    current = WriteKeyInterval(file, current, key->domain, key->intervals);

    return current;
    }


static _Bucket  WriteKeyInterval(file, bucket, domain, interval)
_GridFile file;
 _Bucket bucket;
 _Domain domain;

    _KeyInterval interval;

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
    _Bucket current;

    current = bucket;

    /*
    **  If this is the end of the Interval list, return
    */

    if (interval == (_KeyInterval) 0)
	return current;

    /*
    **  Write out the next Interval first (this recursive approach writes them
    **	out FIFO, so that we can read them back in a convenient manner (FILO).
    */

    current = WriteKeyInterval(file, current, domain, interval->next);

    /*
    **  Write this Key Interval base and split values
    */

    current = WriteKeyValue(file, current, domain, interval->base);
    current = WriteKeyValue(file, current, domain, interval->split);

    return current;
    }


static _Bucket  WriteKeyValue(file, bucket, domain, value)
_GridFile file;
 _Bucket bucket;
 _Domain domain;

    _KeyValue value;

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
    _Bucket current;

    /*
    **  Deal with each Domain individually
    */

    current = bucket;

    switch (domain) {
	case lwk_c_domain_integer :
	    current = LwkGRStoreInteger(file, current, value.integer);
	    break;

	case lwk_c_domain_float :
	    current = LwkGRStoreFloat(file, current, value.float_pt);
	    break;

	case lwk_c_domain_date :
	    current = LwkGRStoreDate(file, current, value.date);
	    break;

	case lwk_c_domain_string :
	    current = LwkGRStoreString(file, current, value.string);
	    break;

	case lwk_c_domain_ddif_string :
	    current = LwkGRStoreDDIFString(file, current, value.ddifstring);
	    break;

	default :
	    _Raise(inv_domain);
    }

    return current;
    }


static _Region  CreateRegion(file, base, extent, number)
_GridFile file;
 _RegionVertex base;

    _RegionVertex extent;
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
    int i;
    _Region region;

    /*
    ** Allocate and initialize a Grid Region
    */

    region = (_Region) _AllocateMem(sizeof(_RegionInstance));

    region->closer = (_Region) 0;
    region->further = (_Region) 0;

    region->bucket = number;

    region->base_radius = 0;
    region->extent_radius = 0;

    for (i = 0; i < file->key_count; i++) {
	region->base_vertex[i] = base[i];
	region->extent_vertex[i] = extent[i];

	region->base_radius += region->base_vertex[i];
	region->extent_radius += region->extent_vertex[i];
    }

    return region;
    }


static void  AddRegion(file, region)
_GridFile file;
 _Region region;

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
    _Region closer;
    _Region further;

    /*
    ** Find the proper closer Region for the new Region.  Regions are kept in a
    ** list in increasing base radius order.
    */

    if (file->regions == (_Region) 0) {
	file->regions = region;
	region->closer = (_Region) 0;
	region->further = (_Region) 0;
    }
    else {
	closer = (_Region) 0;
	further = file->regions;

	while (further != (_Region) 0) {
	    if (region->base_radius < further->base_radius)
		break;

	    closer = further;
	    further = further->further;
	}

	region->closer = closer;
	region->further = further;

	if (closer == (_Region) 0)
	    file->regions = region;
	else
	    closer->further = region;

	if (further != (_Region) 0)
	    further->closer = region;
    }

    return;
    }


static void  RemoveRegion(file, region)
_GridFile file;
 _Region region;

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
    if (region->closer == (_Region) 0)
	file->regions = region->further;
    else
	region->closer->further = region->further;

    if (region->further != (_Region) 0)
	region->further->closer = region->closer;

    return;
    }


static _Region  MergeRegions(file, region1, region2, bucket)
_GridFile file;
 _Region region1;
 _Region region2;

    _BucketNumber bucket;

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
    _Region closer;
    _Region further;

    /*
    ** Determine which is closer
    */

    if (region1->base_radius < region2->base_radius) {
	closer = region1;
	further = region2;
    }
    else {
	closer = region2;
	further = region1;
    }

    /*
    ** Make the closer Region subsume both, then remove and free the further
    ** Region from the Region tree.
    */

    for (i = 0; i < file->key_count; i++)
	closer->extent_vertex[i] = further->extent_vertex[i];

    closer->extent_radius = further->extent_radius;

    closer->bucket = bucket;

    RemoveRegion(file, further);
    FreeRegion(further);

    return closer;
    }


static void  SelectIntersectingRegions(file, region, callback, closure)
_GridFile file;
 _Region region;

    _SelectCallback callback;
 _Closure closure;

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
    _Region regions;
    _Region selected;
    _Region modified;
    _Boolean intersect;
    _Boolean base_modified;

    regions = file->regions;
    modified = (_Region) 0;

    while (regions != (_Region) 0) {
	/*
	** When the extent of the Region is closer than the base radius
	** of the remaining Regions, we can stop.
	*/

	if (region->extent_radius < regions->base_radius)
	    break;

	/*
	** If these two Regions intersect, invoke the callback routine
	*/

	intersect = _True;

	for (i = 0; i < file->key_count; i++) {
	    if (region->extent_vertex[i] < regions->base_vertex[i]
		    || region->base_vertex[i] > regions->extent_vertex[i]) {
		intersect = _False;
		break;
	    }
	}

	selected = regions;
	regions = regions->further;

	if (intersect) {
	    base_modified = (_Boolean) (callback)(selected, closure);

            /*
	    ** If the callback modified the base radius, remove the Region from
	    ** the list for now and save it away -- we will re-insert it later.
            */
	    
	    if (base_modified) {
		RemoveRegion(file, selected);

		selected->further = modified;
		modified = selected;
	    }
	}
    }

    /*
    ** Re-insert any Regions which had their base radii modified
    */

    while (modified != (_Region) 0) {
	selected = modified;

	modified = modified->further;

	AddRegion(file, selected);
    }

    return;
    }


static void  FreeRegions(file)
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
    _Region region;
    _Region remaining;

    /*
    **  Free all the Grid Regions
    */

    remaining = file->regions;

    while (remaining != (_Region) 0) {
	region = remaining;
	remaining = region->further;

	FreeRegion(region);
    }

    file->regions = (_Region) 0;

    return;
    }


static void  FreeRegion(region)
_Region region;

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
    _FreeMem(region);

    return;
    }


static void  ReadRegions(file)
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
    _Region region;
    _GridCursor cursor;
    _BucketNumber number;
    _RegionVertex base;
    _RegionVertex extent;

    if (_TraceRead)
	_Trace("Read Regions -- File %s\n", file->identifier);

    /*
    **  Initialize
    */

    cursor = (_GridCursor) 0;

    _StartExceptionBlock

    /*
    **  Create a Cursor to read the Regions
    */

    cursor = LwkGCCreateRegionsCursor(file);

    /*
    **  Position the Cursor to the beginning
    */

    if (!LwkGRNextRecord(cursor, _True))
	_Raise(db_read_error);

    /*
    **  Read the Regions until the end
    */

    while (_True) {
	/*
	** Retrieve the Bucket number.  If it is the end-of-Region marker,
	** we are done.
	*/

	cursor->position = LwkGRRetrieveInteger(cursor, cursor->position,
	    &number);

	if (number < 0)
	    break;

	/*
	** Get the base and extent vertices
	*/

	cursor->position = LwkGRRetrieveIntegers(cursor, cursor->position,
	    file->key_count, base);

	cursor->position = LwkGRRetrieveIntegers(cursor, cursor->position,
	    file->key_count, extent);

	/*
	** Create a new Region and add it to the list.
	*/
	
	region = CreateRegion(file, base, extent, number);

	AddRegion(file, region);
    }

    /*
    **  Free the Cursor
    */

    LwkGCFreeCursor(cursor);

    /*
    **	If any exceptions are raised, clean up then reraise the exception.
    */
    
    _Exceptions
	_WhenOthers
	    LwkGCFreeCursor(cursor);
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  WriteRegions(file, bucket)
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
    _Bucket current;
    _Region region;

    if (_TraceWrite)
	_Trace("Write Regions -- File %s\n", file->identifier);

    /*
    ** Write the Regions
    */

    current = bucket;
    region = file->regions;

    while (region != (_Region) 0) {
	/*
	** Write the Bucket number, the base vertex, and the extent vertex
	*/

	current = LwkGRStoreInteger(file, current, region->bucket);

	current = LwkGRStoreIntegers(file, current, file->key_count,
	    region->base_vertex);

	current = LwkGRStoreIntegers(file, current, file->key_count,
	    region->extent_vertex);

	region = region->further;
    }

    /*
    ** Terminate the list of Regions with a marker (an invalid Bucket number)
    */

    current = LwkGRStoreInteger(file, current, -1);

    return;
    }


static void  RefineRegions(file, vertex)
_GridFile file;
 _RegionVertex vertex;

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
    _Integer plane;
    _Integer dimension;
    _Integer closure[2];
    _KeyValue base;
    _Region region;
    _RegionVertex base_vertex;
    _RegionVertex extent_vertex;

    /*
    **	Grid refinement entails opening a new plane along one dimension of
    **	the Grid Directory.  We select a dimension somewhat arbitrarily -- we
    **	simply cycle though them one by one as Buckets get split.  Note the
    **	check to avoid an (unexpected!) infinite loop!
    */

    for (i = 0; i < file->key_count; i++) {
	dimension = file->next_to_split;
	plane = vertex[dimension];

	if (SplitDimension(file, dimension, plane))
	    break;

	file->next_to_split = (file->next_to_split + 1) % file->key_count;
    }

    if (i >= file->key_count)
	_Raise(db_write_error);

    if (_TraceStore) {
	_Trace("Refine Regions -- File %s\n", file->identifier);
	_Trace("    Split Key %s at ", file->keys[dimension]->name);
	PrintInterval(file->keys[dimension], plane);
	_Trace("\n");
    }

    /*
    ** Create a dummy Region that extends from the new plane to the extent of
    ** this dimension
    */

    for (i = 0; i < file->key_count; i++) {
	if (i == dimension) {
	    base_vertex[i] = plane;
	    extent_vertex[i] = file->keys[dimension]->interval_count;
	}
	else {
	    base_vertex[i] = 0;
	    extent_vertex[i] = file->keys[i]->interval_count;
	}
    }

    region = CreateRegion(file, base_vertex, extent_vertex, 0);

    /*
    ** Modify all intersecting Regions
    */

    closure[0] = dimension;
    closure[1] = plane;

    SelectIntersectingRegions(file, region, RefineRegion, (_Closure) closure);

    /*
    ** Clean up
    */

    FreeRegion(region);

    /*
    **  Mark the Regions modified
    */

    file->regions_modified = _True;

    return;
    }


static void  VerifyRegions(file, flags, list_file)
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
    int i;
    _Integer cnt;
    _Integer area;
    _Integer size;
    _Bucket bucket;
    _Region region;

    bucket = LwkGBReadBucket(file, _RegionsBucket);

    fprintf(list_file, "\nRegion Bucket(s): %ld", bucket->number);

    i = 0;

    while (bucket->buffer->extension != (_BucketNumber) 0) {
	bucket = LwkGBReadBucket(file, bucket->buffer->extension);

	if (i == 0)
	    fputs("\n    ", list_file);
	else if ((i % 10) == 0)
	    fputs(",\n    ", list_file);
	else
	    fputs(", ", list_file);

	fprintf(list_file, "%ld", bucket->number);

	i++;
    }

    fputs("\n", list_file);

    cnt = 0;
    region = file->regions;

    while (region != (_Region) 0) {
	fprintf(list_file, "    Region: Bucket %ld -- ", region->bucket);

	area = 1;

	for (i = 0; i < file->key_count; i++) {
	    size = region->extent_vertex[i] - region->base_vertex[i] + 1;

	    if (i == 0)
		fprintf(list_file, "%ld", size);
	    else
		fprintf(list_file, "x%ld", size);

	    area = area * size;
	}

	fprintf(list_file, " = %ld\n", area);

	cnt++;
	region = region->further;
    }

    fprintf(list_file, "Total number of Regions: %ld\n", cnt);

    return;
    }


static void  SplitRegion(file, region, vertex)
_GridFile file;
 _Region region;
 _RegionVertex vertex;

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
    _Integer pass;
    _Integer count;
    _Integer base;
    _Integer extent;
    _Integer dimension;
    _Integer first_dimension;
    _Bucket new_bucket;
    _Region new_region;
    _RegionVertex base_vertex;
    _RegionVertex extent_vertex;

    /*
    **	If we have the unit region (base_radius == extent_radius), we must
    **	"refine the grid" (break one key interval into two).
    */

    if (region->base_radius == region->extent_radius) {
	RefineRegions(file, vertex);
	return;
    }

    /*
    **	Allocate a new Bucket -- we will use it for the new Region
    */

    new_bucket = LwkGBAllocateNextBucket(file);

    /*
    ** We must move at least one Record from this Region so that the new Record
    ** can have some hope of getting stored.  To do this, we try all possible
    ** means to split this region in two along some dimension.
    */

    pass = 0;
    first_dimension = file->next_to_split;

    while (_True) {
        /*
	** Pick another dimension to split.  If we have made two passes over
	** the available dimensions, we give up and try another Refinement.
        */

	while (_True) {
	    if (file->next_to_split == first_dimension)
		pass++;

	    dimension = file->next_to_split;

	    file->next_to_split = (file->next_to_split + 1) % file->key_count;

	    if (pass > 2) {
		LwkGBDeallocateBucket(file, new_bucket);

		RefineRegions(file, vertex);

		return;
	    }

	    if (region->extent_vertex[dimension]
		    > region->base_vertex[dimension])
		break;
	}

	/*
	** Split this dimension at the vertex previously calculated for storing
	** the new Record.  On the first pass, split off the upper range of the
	** dimension, if possible.  On the second pass, split off the lower
	** range of the dimension, if possible.
	*/

	if (pass == 1) {
            /*
            ** If there is no upper range to the Region, we can't split this
	    ** dimension on pass one.
            */

	    if (region->extent_vertex[dimension] - vertex[dimension] <= 0)
		continue;
	}
	else {
            /*
            ** If there is no lower range to the Region, we can't split this
	    ** dimension on pass two.
            */

	    if (vertex[dimension] - region->base_vertex[dimension] <= 0)
		continue;
	}            

	for (i = 0; i < file->key_count; i++) {
	    base_vertex[i] = region->base_vertex[i];
	    extent_vertex[i] = region->extent_vertex[i];
	}

	base = region->base_vertex[dimension];
	extent = region->extent_vertex[dimension];

	if (pass == 1) {
	    /*
	    ** Pass one -- make the new Region cover the upper range.
	    */

	    base_vertex[dimension] = vertex[dimension] + 1;

	    region->extent_radius -=
		region->extent_vertex[dimension] - vertex[dimension];

	    region->extent_vertex[dimension] = vertex[dimension];
	}
	else {
	    /*
	    ** Pass two -- make the new Region cover the lower range, and
	    ** Remove/Add the old Region since its base radius has changed (the
	    ** sort key for the Region list).
	    */

	    extent_vertex[dimension] = vertex[dimension] - 1;

	    RemoveRegion(file, region);

	    region->base_radius +=
		vertex[dimension] - region->base_vertex[dimension];

	    region->base_vertex[dimension] = vertex[dimension];

	    AddRegion(file, region);
	}

	/*
	** Create the new Region and add it to the Region list.
	*/
	
	new_region = CreateRegion(file, base_vertex, extent_vertex,
	    new_bucket->number);

	AddRegion(file, new_region);

	if (_TraceStore) {
	    _Trace("Split Region -- File %s\n", file->identifier);
	    _Trace("    Region %ld, Key %s\n    ", region->bucket,
		file->keys[dimension]->name);

	    PrintInterval(file->keys[dimension], base);

	    _Trace(" <=> ");

	    if (pass == 1)
		PrintInterval(file->keys[dimension], base_vertex[dimension]);
	    else
		PrintInterval(file->keys[dimension], extent_vertex[dimension]);

	    _Trace(" <=> ");

	    PrintInterval(file->keys[dimension], extent);

	    _Trace("\n");
        }

	/*
	** Try to move some Records out of this Region
	*/

	count = MoveRecords(file, region, new_region, new_bucket);

	if (count > 0) {
            /*
            ** We successfully moved some Records.  Mark the Regions modified
	    ** and return.
            */
	    
	    if (_TraceStore)
		_Trace("Moved %d records\n", count);

	    file->regions_modified = _True;

	    break;
        }
	else {
            /*
	    ** We didn't move any Records.  Re-merge the split Regions and try
	    ** again.
            */

	    if (_TraceStore)
		_Trace("No Records moved -- merging split Regions\n");

	    region = MergeRegions(file, region, new_region, region->bucket);
	}
    }

    /*
    ** Go back and try again to store the Record
    */
    
    return;
    }


static _Boolean  SplitDimension(file, dimension, plane)
_GridFile file;
 _Integer dimension;

    _Integer plane;

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
    _Key key;
    _KeyValue base;
    _KeyValue split;
    _KeyValue mid_point;
    _KeyInterval new;
    _KeyInterval interval;

    /*
    **  Find the corresponding Key Interval range along this dimension
    */

    key = file->keys[dimension];

    base = key->base;
    split = key->split;

    i = 1;
    interval = key->intervals;

    while (interval != (_KeyInterval) 0) {
	if (i == plane) {
	    base = interval->base;
	    split = interval->split;
	    break;
	}

	i++;
	interval = interval->next;
    }
	
    /*
    **  If the Base and the Split are equal, we can't split the dimension!
    */

    if (_CompareValue(&base, &split, key->domain) == 0)
	return _False;

    /*
    **	Create the new Key Interval.  Its Base and Split are the selected Split
    **	value.
    */

    new = (_KeyInterval) _AllocateMem(sizeof(_KeyIntervalInstance));

    _MoveValue(&split, &new->base, key->domain);
    _CopyValue(&new->base, &new->split, key->domain);

    /*
    **  Link in the new Key Interval and reset the Split value of the old
    **	Interval.
    */

    if (plane == 0) {
	new->next = key->intervals;
	key->intervals = new;
	_CopyValue(&key->base, &key->split, key->domain);
    }
    else {
	new->next = interval->next;
	interval->next = new;	
	_CopyValue(&interval->base, &interval->split, key->domain);
    }

    key->interval_count++;

    /*
    **  Mark the Directory modified
    */

    file->directory_modified = _True;

    return _True;
    }


static int  MoveRecords(file, old_region, new_region, new_bucket)
_GridFile file;
 _Region old_region;
 _Region new_region;

    _Bucket new_bucket;

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
    int count;
    _Domain domain;
    _Region region;
    _Integer position;
    _Integer container;
    _Integer identifier;
    _ConjunctKeys keys;
    _RegionVertex vertex;
    _Boolean skip_current;
    _GridCursor volatile cursor;
    _DynamicVString volatile encoding;

    /*
    **	Loop over all the Records in the original Bucket and move those which
    **	belong in the new Bucket.
    */

    if (_TraceStore)
	_Trace("Attempt move of Records to new Bucket\n");

    /*
    ** Initialize
    */

    count = 0;
    skip_current = _True;

    cursor = LwkGCCreateSplitCursor(file, old_region->bucket);

    _CreateDVString(encoding);

    _StartExceptionBlock

    while (LwkGRNextRecord(cursor, skip_current)) {
	/*
	**  Retrieve the Record header
	*/

	position = LwkGRRetrieveRecordHeader(cursor, (_IntegerPtr) 0,
	    &identifier, &domain, &container);

	/*
	**  Retrieve object encoding
	*/

	position = LwkGRRetrieveVaryingString(cursor, position, encoding);

	/*
	**  Determine where the Record should now be stored
	*/

	keys = LwkGCCreateConjunctKeys(domain);

	keys->identifier = identifier;
	keys->container = container;

	LwkGCSetKeyProperties(keys, _DVString_of(encoding));

	region = RegionToStore(file, keys, vertex);

	LwkGCFreeConjunctKeys(keys, _True);

	/*
	**  If it should be in the new Region, move it there
	*/

	if (region != old_region && region != new_region)
	    _Raise(db_write_error);
	else if (region != new_region)
	    skip_current = _True;
	else {
	    if (_TraceStore)
		_Trace("        ...moved\n");

	    LwkGRMoveRecord(cursor, new_bucket);

	    count++;
	    cursor->select_count++;
	    skip_current = _False;
	}
    }

    /*
    **  Clean up
    */

    LwkGCFreeCursor(cursor);

    _DeleteDVString(&encoding);

    /*
    **	If any exceptions are raised, clean up then reraise the exception.
    */
    
    _Exceptions
	_WhenOthers
	    LwkGCFreeCursor(cursor);
	    _DeleteDVString(&encoding);
	    _Reraise;
    _EndExceptionBlock

    return count;
    }


static _Region  RegionToStore(file, keys, vertex)
_GridFile file;
 _ConjunctKeys keys;

    _RegionVertex vertex;

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
    _KeyValue value;
    _Region region;
    _Region container;

    /*
    **  Analyze the Keys to determine which Region vertex should hold this
    **  Record.
    */

    for (i = 0; i < file->key_count; i++) {
	switch (file->keys[i]->type) {
	    case _KeyDomain :
		value.integer = (_Integer) keys->domain;
		break;

	    case _KeyIdentifier :
		value.integer = keys->identifier;
		break;

	    case _KeyContainer :
		value.integer = keys->container;
		break;

	    case _KeyProperty :
		GetKeyPropertyValue(file->keys[i], keys, &value);
		break;
	}

	vertex[i] = KeyStoreInterval(file, file->keys[i], value);

	/*
	**  Delete the value of it came from the Key Property Set
	*/

	if (file->keys[i]->type == _KeyProperty)
	    _DeleteValue(&value, file->keys[i]->domain);
    }

    /*
    ** Create a dummy Region at this vertex
    */

    region = CreateRegion(file, vertex, vertex, 0);

    /*
    ** Find the Region which contains it
    */

    container = (_Region) 0;

    SelectIntersectingRegions(file, region, SelectRegion,
	(_Closure) &container);

    /*
    ** Clean up
    */

    FreeRegion(region);

    return container;
    }


static _Boolean  GetKeyPropertyValue(key, keys, value)
_Key key;
 _ConjunctKeys keys;

    _KeyValue *value;

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
    _Boolean specified;
    _Property property;

    /*
    **	Iterate over the Set of Key Properties looking for one with the name
    **	and domain as the given Key.
    */

    if (keys->key_properties == (_Set) _NullObject)
	property = (_Property) _NullObject;
    else
	property = (_Property) _Iterate(keys->key_properties,
	    lwk_c_domain_property, (_Closure) key, FindProperty);

    /*
    **  Return the value of that Property, or the null value if there was no
    **	such Property.
    */

    if (property == (_Property) _NullObject) {
	specified = _False;
	_ClearValue(value, key->domain);
    }
    else {
	specified = _True;
	_GetValue(property, _P_Value, key->domain, value);
    }

    return specified;
    }


static int  KeyRetrieveInterval(key, value)
_Key key;
 _KeyValue value;

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
    _KeyInterval interval;

    /*
    **  Check base/extent of this Key to see if record exists
    */

    if (_CompareValue(&value, &key->base, key->domain) < 0
	    || _CompareValue(&value, &key->extent, key->domain) > 0)
	return -1;

    /*
    **  Yes, the Key value exists -- find the specific Key Interval
    */

    return KeyInterval(key, value, &interval);
    }


static int  KeyStoreInterval(file, key, value)
_GridFile file;
 _Key key;
 _KeyValue value;

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
    int index;
    _KeyValue *base;
    _KeyValue *split;
    _KeyValue *extent;
    _KeyInterval interval;

    /*
    **	Find the Key Interval where the Record should be stored.
    */

    index = KeyInterval(key, value, &interval);

    /*
    **  Update Base/Extent of this Key
    */

    if (_CompareValue(&value, &key->base, key->domain) < 0) {
	_DeleteValue(&key->base, key->domain);
	_CopyValue(&value, &key->base, key->domain);
	file->directory_modified = _True;
    }

    if (_CompareValue(&value, &key->extent, key->domain) > 0) {
	_DeleteValue(&key->extent, key->domain);
	_CopyValue(&value, &key->extent, key->domain);
	file->directory_modified = _True;
    }

    /*
    ** Update the potential Split of this Interval
    */

    if (index == 0) {
	base = &key->base;
	split = &key->split;

	if (key->intervals == (_KeyInterval) 0)
	    extent = &key->extent;
	else
	    extent = &key->intervals->base;
    }
    else {
	base = &interval->base;
	split = &interval->split;

	if (interval->next == (_KeyInterval) 0)
	    extent = &key->extent;
	else
	    extent = &interval->next->base;
    }

    if (IsBetterSplit(base, extent, split, &value, key->domain)) {
	_DeleteValue(split, key->domain);
	_CopyValue(&value, split, key->domain);
	file->directory_modified = _True;
    }

    /*
    **	Return the index of the Key Interval.
    */

    return index;
    }


static int  KeyInterval(key, value, interval)
_Key key;
 _KeyValue value;
 _KeyInterval *interval;

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
    _Integer index;
    _KeyInterval next;

    /*
    **  Find the appropriate Interval for this Key value
    */

    index = 0;
    next = key->intervals;
    *interval = (_KeyInterval) 0;

    while (next != (_KeyInterval) 0) {
	if (_CompareValue(&value, &next->base, key->domain) < 0)
	    break;

	*interval = next;

	index++;
	next = next->next;
    }

    /*
    **	Return the index of the Key Interval where the Record should be
    **	stored.
    */

    return index;
    }


static _Boolean  IsBetterSplit(base, extent, split, value, domain)
_KeyValue *base;
 _KeyValue *extent;

    _KeyValue *split;
 _KeyValue *value;
 _Domain domain;

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
    if (_CompareValue(value, split, domain) > 0)
	return _True;
    else
	return _False;
    }


static void  VerifyKeys(file, flags, list_file)
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
    int i;
    _Key key;
    _KeyInterval interval;

    /*
    **  Verify/List general Key attributes if requested
    */

    for (i = 0; i < file->key_count; i++) {
	key = file->keys[i];

	fprintf(list_file, "\nKey %d: %s\n", i, key->name);
	fprintf(list_file, "    Domain: %s\n",
	    LwkGDDomainToName(key->domain));

	fputs("    Base: ", list_file);
	ListIntervalValue(key, key->base, list_file);
	fputs("\n", list_file);

	fputs("    Next Split: ", list_file);
	ListIntervalValue(key, key->split, list_file);
	fputs("\n", list_file);

	fputs("    Extent: ", list_file);
	ListIntervalValue(key, key->extent, list_file);
	fputs("\n", list_file);

	fprintf(list_file, "    Intervals: %ld\n", key->interval_count);

	interval = key->intervals;

	while (interval != (_KeyInterval) 0) {
	    fputs("        Base: ", list_file);
	    ListIntervalValue(key, interval->base, list_file);

	    fputs(", Next Split: ", list_file);
	    ListIntervalValue(key, interval->split, list_file);

	    fputs("\n", list_file);

	    interval = interval->next;
	}
    }

    return;
    }


static void  ListIntervalValue(key, value, list_file)
_Key key;
 _KeyValue value;
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
    _String string;

    switch (key->domain) {
	case lwk_c_domain_integer :
	    if (key->type == _KeyDomain)
		fprintf(list_file, "%s",
		    LwkGDDomainToName((_Domain) value.integer));
	    else
		fprintf(list_file, "%ld", value.integer);
	    break;

	case lwk_c_domain_float :
	    fprintf(list_file, "%f", value.float_pt);
	    break;

	case lwk_c_domain_string :
	    if (value.string == (_String) 0)
		fputs("Null", list_file);
	    else
		ListString(value.string, list_file);

	    break;

	case lwk_c_domain_ddif_string :
	    if (value.ddifstring == (_DDIFString) 0)
		fputs("Null", list_file);
	    else {
		string = _DDIFStringToString(value.ddifstring);
		ListString(string, list_file);
		_DeleteString(&string);
	    }

	    break;

	case lwk_c_domain_date :
	    if (value.date == (_Date) 0)
		fputs("Null", list_file);
	    else {
		string = _DateToString(value.date);
		ListString(string, list_file);
		_DeleteString(&string);
	    }

	    break;

	default :
	    fputs("Unknown", list_file);
	    break;
    }

    return;
    }


static void  ListString(string, list_file)
_String string;
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
    _CharPtr cp;

    /*
    **  Don't try to list any non-printable characters
    */

    cp = (_CharPtr) string;

    fputc('"', list_file);

    while (*cp != _EndOfString) {
	if (isprint(*cp))
	    fputc(*cp, list_file);
	else
	    fputc('¿', list_file);

	cp++;
    }

    fputc('"', list_file);

    return;
    }


static void  PrintInterval(key, index)
_Key key;
 _Integer index;

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
    _KeyInterval interval;

    if (index >= key->interval_count)
	ListIntervalValue(key, key->extent, LwkGridTraceFile);
    else {
	interval = key->intervals;

	for (i = 0; i < index; i++)
	    interval = interval->next;

	ListIntervalValue(key, interval->base, LwkGridTraceFile);
    }

    return;
    }


static _Termination _EntryPt  FindProperty(closure, set, domain, property)
_Closure closure;
 _Set set;

    _Domain domain;
 _Property *property;

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
    _Key key;
    _Termination answer;

    /*
    **	If the _PropertyName of the Property matches, and the Domain of the
    **	Property match, return the Property.  Otherwise, return zero so that
    **	the iteration will continue.
    */

    key = (_Key) closure;

    if (!_IsNamed(*property, key->name))
	answer = (_Termination) 0;
    else {
	_Integer integer;

	_GetValue(*property, _P_Domain, lwk_c_domain_integer, &integer);

	if (_IsDomain(key->domain, (_Domain) integer))
	    answer = (_Termination) *property;
	else
	    answer = (_Termination) 0;
    }

    return answer;
    }


static _Termination  AddBucketToList(region, closure)
_Region region;
 _Closure closure;

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
    _Integer where;
    _Boolean insert;
    _BucketList bucket_list;

    bucket_list = (_BucketList) closure;

    /*
    **  Add the Bucket to the list in proper ascending order, ignoring
    **	duplicates
    */

    insert = _True;
    where = bucket_list->count;

    for (i = 0; i < bucket_list->count; i++) {
	/*
	**  If duplicate, skip it
	*/

	if (bucket_list->numbers[i] == region->bucket) {
	    insert = _False;
	    break;
	}

	/*
	**  If this is where the Bucket belongs, leave the loop
	*/

	if (bucket_list->numbers[i] > region->bucket) {
	    insert = _True;
	    where = i;
	    break;
	}
    }

    if (insert) {
	/*
	**  Make sure the list is big enough
	*/

	if (bucket_list->count % 20 == 0)
	    bucket_list->numbers = (_BucketNumberPtr)
		_ReallocateMem(bucket_list->numbers,
		    ((bucket_list->count + 20) * sizeof(_BucketNumber)));

	/*
	**  Make room for the new Bucket
	*/

	for (i = bucket_list->count - 1; i >= where; i--)
	    bucket_list->numbers[i + 1] = bucket_list->numbers[i];

	/*
	**  Add the new Bucket
	*/

	bucket_list->numbers[where] = region->bucket;

	/*
	**  Count the new Bucket
	*/

	bucket_list->count++;
    }

    return (_Termination) _False;
    }


static _Termination  SelectRegion(region, closure)
_Region region;
 _Closure closure;

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
    ** Return the selected Region
    */

    *((_Region *) closure) = region;

    return (_Termination) _False;
    }


static _Termination  RefineRegion(region, closure)
_Region region;
 _Closure closure;

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
    _Integer plane;
    _Integer dimension;
    _Boolean base_modified;

    dimension = ((_Integer *) closure)[0];
    plane = ((_Integer *) closure)[1];

    /*
    ** We have just added a new plane in the given dimension.  Since this
    ** Region intersects that plane, modify its base and/or extent
    ** vertex/radius.
    */

    if (region->base_vertex[dimension] <= plane)
	base_modified = _False;
    else {
	region->base_vertex[dimension]++;
	region->base_radius++;

	base_modified = _True;
    }

    region->extent_vertex[dimension]++;
    region->extent_radius++;

    return (_Termination) base_modified;
    }
