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

typedef struct __Operator {
	    _QueryOperator operator;
	    _QueryOperator complement;
	    _CharPtr string;
	    int arity;
	    _Domain domain;
	} _Operator;

/*
**  Macro Definitions
*/

/*
**  Forward Routine Declarations
*/

_DeclareFunction(static _GridCursor AllocateCursor, (_String name,
    _GridFile file));
_DeclareFunction(static _QueryExpression KeyQuery,
    (_GridFile file, _QueryExpression query));
_DeclareFunction(static _Boolean IsKeyProperty, (_GridFile file, _String name));
_DeclareFunction(static _QueryExpression RemoveNegationFromQuery,
    (_QueryExpression query));
_DeclareFunction(static _QueryExpression ComplementQuery,
    (_QueryExpression query));
_DeclareFunction(static _QueryExpression QueryToDNF, (_QueryExpression query));
_DeclareFunction(static _QueryExpression DistributeConjunction,
    (_QueryExpression conjunction));
_DeclareFunction(static _QueryExpression CopyQueryExpression,
    (_QueryExpression query));
_DeclareFunction(static _QueryExpression CopyQueryNode,
    (_QueryExpression query));
_DeclareFunction(static void DeleteQueryExpression, (_QueryExpression query));
_DeclareFunction(static void QueryToConjunctKeys,
    (_GridFile file, _QueryExpression key_query, _ConjunctKeys keys));
_DeclareFunction(static _Boolean QueryKeyPropertyName,
    (_QueryExpression key_query, _String *name));
_DeclareFunction(static void QueryKeyPropertyValue,
    (_QueryExpression key_query, _Boolean name_first, _Domain domain,
	_AnyValue *value));
_DeclareFunction(static _String QueryToString, (_QueryExpression query));
_DeclareFunction(static _String QueryOperatorToString,
    (_QueryOperator operator));
_DeclareFunction(static int QueryOperatorToArity, (_QueryOperator operator));
_DeclareFunction(static _Domain QueryOperatorToDomain,
    (_QueryOperator operator));
_DeclareFunction(static _QueryOperator QueryOperatorToComplement,
    (_QueryOperator operator));
_DeclareFunction(static void PrintProperties, (_Set properties));
_DeclareFunction(static _Termination _EntryPt PrintProperty,
    (_Closure null, _Set set, _Domain domain, _Property *property));

/*
**  Table to convert Query Operators into various quantities: complement,
**  String (for trace/printing), arity, domain.
*/

static _Operator Operators[] = {
    {lwk_c_and, lwk_c_or, " .AND. ", 2, lwk_c_domain_unknown},
    {lwk_c_or, lwk_c_and, " .OR. ", 2, lwk_c_domain_unknown},
    {lwk_c_not, lwk_c_not, ".NOT. ", 1, lwk_c_domain_unknown},
    {lwk_c_any, lwk_c_not, "Any", 0, lwk_c_domain_unknown},
    {lwk_c_identity, lwk_c_not, "Identity:", 0, lwk_c_domain_unknown},
    {lwk_c_has_properties, lwk_c_not, ".HasProperties. ", 1,
	lwk_c_domain_unknown},
    {lwk_c_is_the_source_of, lwk_c_not, ".IsSourceOf. ", 1,
	lwk_c_domain_unknown},
    {lwk_c_is_the_target_of, lwk_c_not, ".IsTargetOf. ", 1,
	lwk_c_domain_unknown},
    {lwk_c_is_some_source_of, lwk_c_not, ".IsSomeSourceOf. ", 1,
	lwk_c_domain_unknown},
    {lwk_c_is_some_target_of, lwk_c_not, ".IsSomeTargetOf. ", 1,
	lwk_c_domain_unknown},
    {lwk_c_is_the_origin_of, lwk_c_not, ".IsOriginOf. ", 1,
	lwk_c_domain_unknown},
    {lwk_c_is_the_destination_of, lwk_c_not, ".IsDestinationOf. ", 1,
	lwk_c_domain_unknown},
    {lwk_c_is_some_origin_of, lwk_c_not, ".IsSomeOriginOf. ", 1,
	lwk_c_domain_unknown},
    {lwk_c_is_some_destination_of, lwk_c_not, ".IsSomeDestination. ", 1,
	lwk_c_domain_unknown},
    {lwk_c_has_source, lwk_c_not, ".HasSource. ", 1, lwk_c_domain_unknown},
    {lwk_c_has_target, lwk_c_not, ".HasTarget. ", 1, lwk_c_domain_unknown},
    {lwk_c_has_origin, lwk_c_not, ".HasOrigin. ", 1, lwk_c_domain_unknown},
    {lwk_c_has_destination, lwk_c_not, ".HasDestination. ", 1,
	lwk_c_domain_unknown},
    {lwk_c_string_is_eql, lwk_c_string_is_neq, " .EQL. ", 2,
	lwk_c_domain_string},
    {lwk_c_string_is_neq, lwk_c_string_is_eql, " .NEQ. ", 2,
	lwk_c_domain_string},
    {lwk_c_string_contains, lwk_c_string_contains_no, " .CTN. ", 2,
	lwk_c_domain_string},
    {lwk_c_string_contains_no, lwk_c_string_contains, " .NCTN. ", 2,
	lwk_c_domain_string},
    {lwk_c_ddif_string_is_eql, lwk_c_ddif_string_is_neq, " .EQL. ", 2,
	lwk_c_domain_ddif_string},
    {lwk_c_ddif_string_is_neq, lwk_c_ddif_string_is_eql, " .NEQ. ", 2,
	lwk_c_domain_ddif_string},
    {lwk_c_ddif_string_contains, lwk_c_ddif_string_contains_no, " .CTN. ", 2,
	lwk_c_domain_ddif_string},
    {lwk_c_ddif_string_contains_no, lwk_c_ddif_string_contains, " .NCTN. ", 2,
	lwk_c_domain_ddif_string},
    {lwk_c_date_is_eql, lwk_c_date_is_neq, " .EQL. ", 2, lwk_c_domain_date},
    {lwk_c_date_is_neq, lwk_c_date_is_eql, " .NEQ. ", 2, lwk_c_domain_date},
    {lwk_c_date_is_lss, lwk_c_date_is_geq, " .LSS. ", 2, lwk_c_domain_date},
    {lwk_c_date_is_gtr, lwk_c_date_is_leq, " .GTR. ", 2, lwk_c_domain_date},
    {lwk_c_date_is_leq, lwk_c_date_is_gtr, " .LEQ. ", 2, lwk_c_domain_date},
    {lwk_c_date_is_geq, lwk_c_date_is_lss, " .GEQ. ", 2, lwk_c_domain_date},
    {lwk_c_integer_is_eql, lwk_c_integer_is_neq, " .EQL. ", 2,
	lwk_c_domain_integer},
    {lwk_c_integer_is_neq, lwk_c_integer_is_eql, " .NEQ. ", 2,
	lwk_c_domain_integer},
    {lwk_c_integer_is_lss, lwk_c_integer_is_geq, " .LSS. ", 2,
	lwk_c_domain_integer},
    {lwk_c_integer_is_gtr, lwk_c_integer_is_leq, " .GTR. ", 2,
	lwk_c_domain_integer},
    {lwk_c_integer_is_leq, lwk_c_integer_is_gtr, " .LEQ. ", 2,
	lwk_c_domain_integer},
    {lwk_c_integer_is_geq, lwk_c_integer_is_lss, " .GEQ. ", 2,
	lwk_c_domain_integer},
    {lwk_c_float_is_eql, lwk_c_float_is_neq, " .EQL. ", 2, lwk_c_domain_float},
    {lwk_c_float_is_neq, lwk_c_float_is_eql, " .NEQ. ", 2, lwk_c_domain_float},
    {lwk_c_float_is_lss, lwk_c_float_is_geq, " .LSS. ", 2, lwk_c_domain_float},
    {lwk_c_float_is_gtr, lwk_c_float_is_leq," .GTR. ", 2, lwk_c_domain_float},
    {lwk_c_float_is_leq, lwk_c_float_is_gtr, " .LEQ. ", 2, lwk_c_domain_float},
    {lwk_c_float_is_geq, lwk_c_float_is_lss, " .GEQ. ", 2, lwk_c_domain_float},
    {lwk_c_boolean_is_eql, lwk_c_boolean_is_neq, " .EQL. ", 2,
	lwk_c_domain_boolean},
    {lwk_c_boolean_is_neq, lwk_c_boolean_is_eql," .NEQ. ", 2,
	lwk_c_domain_boolean},
    {lwk_c_boolean_is_true, lwk_c_boolean_is_false, " .EQL. True", 1,
	lwk_c_domain_boolean},
    {lwk_c_boolean_is_false, lwk_c_boolean_is_true, " .EQL. False", 1,
	lwk_c_domain_boolean},
    {lwk_c_property_value, lwk_c_property_value, "", 0, lwk_c_domain_unknown},
    {lwk_c_some_property_value, lwk_c_some_property_value, "", 0,
	lwk_c_domain_unknown},
    {lwk_c_string_literal, lwk_c_string_literal, "", 0, lwk_c_domain_unknown},
    {lwk_c_ddif_string_literal, lwk_c_ddif_string_literal, "", 0,
	lwk_c_domain_unknown},
    {lwk_c_date_literal, lwk_c_date_literal, "", 0, lwk_c_domain_unknown},
    {lwk_c_integer_literal, lwk_c_integer_literal, "", 0, lwk_c_domain_unknown},
    {lwk_c_float_literal, lwk_c_float_literal, "", 0, lwk_c_domain_unknown},
    {lwk_c_boolean_literal, lwk_c_boolean_literal, "", 0, lwk_c_domain_unknown}
};

static OperatorCount = sizeof(Operators) / sizeof(_Operator);


_GridCursor  LwkGCCreateCursor(name, file, keys)
_String name;
 _GridFile file;

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
    _GridCursor volatile new;

    /*
    **  Initialization
    */

    new = (_GridCursor) 0;

    _StartExceptionBlock

    /*
    **  Create the Cursor
    */

    new = AllocateCursor(name, file);

    /*
    **  Fill in some fields
    */

    new->keys = keys;
    LwkGDBucketsToRetrieve(file, keys, &new->bucket_list);

    /*
    **	If any exceptions are raised, clean up and reraise.
    */
    
    _Exceptions
	_WhenOthers
	    LwkGCFreeCursor(new);
	    _Reraise;
    _EndExceptionBlock

    return new;
    }


_GridCursor  LwkGCCreateDirectoryCursor(file)
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
    _GridCursor volatile new;

    /*
    **  Initialization
    */

    new = (_GridCursor) 0;

    _StartExceptionBlock

    /*
    **  Create the Cursor
    */

    new = AllocateCursor("Read Dir", file);

    /*
    **  Fill in some fields
    */

    new->keys = LwkGCCreateConjunctKeys(lwk_c_domain_unknown);

    new->bucket_list.count = 1;

    new->bucket_list.numbers =
	(_BucketNumberPtr) _AllocateMem(sizeof(_BucketNumber));

    new->bucket_list.numbers[0] = _DirectoryBucket;

    /*
    **	If any exceptions are raised, clean up and reraise.
    */
    
    _Exceptions
	_WhenOthers
	    LwkGCFreeCursor(new);
	    _Reraise;
    _EndExceptionBlock

    return new;
    }


_GridCursor  LwkGCCreateRegionsCursor(file)
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
    _GridCursor volatile new;

    /*
    **  Initialization
    */

    new = (_GridCursor) 0;

    _StartExceptionBlock

    /*
    **  Create the Cursor
    */

    new = AllocateCursor("Read Regions", file);

    /*
    **  Fill in some fields
    */

    new->keys = LwkGCCreateConjunctKeys(lwk_c_domain_unknown);

    new->bucket_list.count = 1;

    new->bucket_list.numbers =
	(_BucketNumberPtr) _AllocateMem(sizeof(_BucketNumber));

    new->bucket_list.numbers[0] = _RegionsBucket;

    /*
    **	If any exceptions are raised, clean up and reraise.
    */
    
    _Exceptions
	_WhenOthers
	    LwkGCFreeCursor(new);
	    _Reraise;
    _EndExceptionBlock

    return new;
    }


_GridCursor  LwkGCCreateSplitCursor(file, number)
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
    _GridCursor volatile new;

    /*
    **  Initialization
    */

    new = (_GridCursor) 0;

    _StartExceptionBlock

    /*
    **  Create the Cursor
    */

    new = AllocateCursor("Split Bucket", file);

    /*
    **  Fill in some fields
    */

    new->keys = LwkGCCreateConjunctKeys(lwk_c_domain_unknown);

    new->bucket_list.count = 1;

    new->bucket_list.numbers =
	(_BucketNumberPtr) _AllocateMem(sizeof(_BucketNumber));

    new->bucket_list.numbers[0] = number;

    /*
    **	If any exceptions are raised, clean up and reraise.
    */
    
    _Exceptions
	_WhenOthers
	    LwkGCFreeCursor(new);
	    _Reraise;
    _EndExceptionBlock

    return new;
    }


_GridCursor  LwkGCCreateRewriteCursor(file, number, domain, identifier)
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
    _GridCursor volatile new;

    /*
    **  Initialization
    */

    new = (_GridCursor) 0;

    _StartExceptionBlock

    /*
    **  Create the Cursor
    */

    new = AllocateCursor("Delete Record", file);

    /*
    **  Fill in some fields
    */

    new->keys = LwkGCCreateConjunctKeys(domain);

    new->keys->identifier = identifier;

    new->bucket_list.count = 1;

    new->bucket_list.numbers =
	(_BucketNumberPtr) _AllocateMem(sizeof(_BucketNumber));

    new->bucket_list.numbers[0] = number;

    /*
    **	If any exceptions are raised, clean up and reraise.
    */
    
    _Exceptions
	_WhenOthers
	    LwkGCFreeCursor(new);
	    _Reraise;
    _EndExceptionBlock

    return new;
    }


_GridCursor  LwkGCCreateVerifyCursor(file)
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
    _GridCursor volatile new;
    _RegionVertex base_vertex;
    _RegionVertex extent_vertex;

    /*
    **  Initialization
    */

    new = (_GridCursor) 0;

    _StartExceptionBlock

    /*
    **  Create the Cursor
    */

    new = AllocateCursor("Verify Buckets", file);

    /*
    **  Fill in some fields
    */

    new->keys = LwkGCCreateConjunctKeys(lwk_c_domain_unknown);

    /*
    ** Construct a base and extent vertex covering the entire Grid Region
    */

    for (i = 0; i < file->key_count; i++) {
	base_vertex[i] = 0;
	extent_vertex[i] = file->keys[i]->interval_count;
    }

    /*
    ** Get the sorted list of Buckets found in that Region
    */

    LwkGDGetBucketList(file, base_vertex, extent_vertex, &new->bucket_list);

    /*
    **	If any exceptions are raised, clean up and reraise.
    */
    
    _Exceptions
	_WhenOthers
	    LwkGCFreeCursor(new);
	    _Reraise;
    _EndExceptionBlock

    return new;
    }


void  LwkGCFreeCursor(cursor)
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
    _GridCursor next;

    /*
    **  If a null Cursor, return now.
    */

    if (cursor == (_GridCursor) 0)
	return;

    if (_TraceQueryResults)
	_Trace("Query Results: %s -- File %s\n", cursor->name,
	    cursor->file->identifier);

    /*
    **  Remove the Cursor from the active Cursor list
    */

    if ((_GridCursor) cursor->file->cursors == cursor)
	cursor->file->cursors = (_AnyPtr) cursor->next;
    else {
	next = (_GridCursor) cursor->file->cursors;

	while (next != (_GridCursor) 0) {
	    if (next->next == cursor) {
		next->next = cursor->next;
		break;
	    }

	    next = next->next;
	}
    }

    /*
    **  Free the Query Keys (here so that any Tracing output is aligned)
    */

    LwkGCFreeConjunctKeys(cursor->keys, _False);

    if (_TraceQueryResults) {
	_Trace("    Searched %ld", cursor->b_search_count);

	if (cursor->ext_b_search_count > 0)
	    _Trace("+%ld",  cursor->ext_b_search_count);

	_Trace(" of %ld Bucket(s), %ld Record(s); selected %ld\n",
	    cursor->bucket_list.count, cursor->r_search_count,
	    cursor->select_count);
    }

    /*
    **  Free any fields within the Cursor
    */

    if (cursor->bucket_list.count > 0)
	_FreeMem(cursor->bucket_list.numbers);

    /*
    **  Free the Cursor
    */

    _FreeMem(cursor);

    return;
    }


_Bucket  LwkGCReadBucketExtension(cursor)
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
    **  If there is no Bucket extension, raise an exception
    */

    if (cursor->ext_bucket->buffer->extension == (_BucketNumber) 0)
	_Raise(db_read_error);

    /*
    **  Read in the Bucket Extension
    */

    cursor->ext_bucket = LwkGBReadBucket(cursor->file,
	cursor->ext_bucket->buffer->extension);

    /*
    **  Update the Bucket Extension Base
    */

    cursor->ext_base += _BucketDataSize(cursor->ext_bucket);

    /*
    **  Count a Bucket Extension searched
    */

    cursor->ext_b_search_count++;

    /*
    **  Return the new Bucket
    */

    return cursor->ext_bucket;
    }


_ConjunctKeys  LwkGCCreateConjunctKeys(domain)
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
    _ConjunctKeys keys;

    /*
    **  Allocate and initialize a Conjunct Keys structure
    */

    keys = (_ConjunctKeys) _AllocateMem(sizeof(_ConjunctKeysInstance));

    keys->domain = domain;
    keys->identifier = -1;
    keys->container = -1;
    keys->key_properties = (_Set) _NullObject;

    keys->disjunct = (_ConjunctKeys) 0;

    return keys;
    }


void  LwkGCSetKeyProperties(keys, encoding)
_ConjunctKeys keys;
 _VaryingString encoding;

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
    _Type type;

    /*
    **  Get the Set of Keyed Properties
    */

    type = _DomainToType(keys->domain);

    _ImportKeyProperties(type, encoding, &keys->key_properties);

    return;
    }


void  LwkGCFreeConjunctKeys(keys, store)
_ConjunctKeys keys;
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
    if ((_TraceStore && store ) || (_TraceQueryResults && !store)) {
	_CharPtr null;

	if (store)
	    null = "None";
	else
	    null = "Any";

	_Trace("    $Domain: %s,", LwkGDDomainToName(keys->domain));

	if (keys->identifier <= 0)
	    _Trace(" $Identifier: %s,", null);
	else
	    _Trace(" $Identifier: %ld,", keys->identifier);

	if (keys->container < 0)
	    _Trace(" $Container: %s", null);
	else
	    _Trace(" $Container: %ld", keys->container);

	PrintProperties(keys->key_properties);
    }

    /*
    **  Free any disjunct Keys
    */

    if (keys->disjunct != (_ConjunctKeys) 0)
	LwkGCFreeConjunctKeys(keys->disjunct, store);

    /*
    **  Free allocated memory
    */

    _Delete(&keys->key_properties);

    _FreeMem(keys);

    return;
    }


void  LwkGCExtractQueryKeys(file, query, keys)
_GridFile file;
 _QueryExpression query;

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
    _QueryExpression reduced_query;

    if (_TraceQuery) {
	_Trace("Query -- File: %s\n", file->identifier);

	if (query == (_QueryExpression ) 0)
	    _Trace("    Null\n");
	else {
	    _String string;

	    string = QueryToString(query);

	    _Trace("    %s\n", string);

	    _DeleteString(&string);
	}
    }

    /*
    **  If the Query is empty, return now
    */

    if (query == (_QueryExpression) 0)
	return;

    /*
    **  Reduce the Query Expression to Keys-only
    */

    reduced_query = KeyQuery(file, query);

    /*
    **	Remove any Negation
    */

    if (reduced_query != (_QueryExpression) 0)
	reduced_query = RemoveNegationFromQuery(reduced_query);

    /*
    **  Transform to Disjunctive Normal Form (DNF)
    */

    if (reduced_query != (_QueryExpression) 0)
	reduced_query = QueryToDNF(reduced_query);

    if (_TraceQuery) {
	_Trace("DNF Key Query -- File: %s\n", file->identifier);

	if (reduced_query == (_QueryExpression ) 0)
	    _Trace("    Null\n");
	else {
	    _String string;

	    string = QueryToString(reduced_query);

	    _Trace("    %s\n", string);

	    _DeleteString(&string);
	}
    }

    /*
    **  If the reduced Query is empty, return now
    */

    if (reduced_query == (_QueryExpression) 0)
	return;

    /*
    **  Transcribe the reduced Query into Conjunct Keys
    */

    QueryToConjunctKeys(file, reduced_query, keys);

    /*
    **  Clean up
    */

    DeleteQueryExpression(reduced_query);

    return;
    }


static _GridCursor  AllocateCursor(name, file)
_String name;
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
    _GridCursor new;

    /*
    **  Allocate the Cursor
    */

    new = (_GridCursor) _AllocateMem(sizeof(_GridCursorInstance));

    /*
    **  Fill in all the fields
    */

    new->file = file;
    new->name = name;
    new->bucket_list.count = 0;
    new->bucket_list.numbers = (_BucketNumberPtr) 0;
    new->bucket = (_Bucket) 0;
    new->ext_bucket = (_Bucket) 0;
    new->keys = (_ConjunctKeys) 0;
    new->current_bucket = -1;
    new->b_search_count = 0;
    new->ext_b_search_count = 0;
    new->r_search_count = 0;
    new->select_count = 0;
    new->position = 0;
    new->ext_base = 0;

    /*
    **  Link the Cursor into the active Cursors lists
    */

    new->next = (_GridCursor) file->cursors;
    file->cursors = (_AnyPtr) new;

    return new;
    }


static _QueryExpression  KeyQuery(file, query)
_GridFile file;
 _QueryExpression query;

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
    _QueryExpression key_query;
    _QueryExpression operand1;
    _QueryExpression operand2;

    /*
    **  Check validity of the Query Expression
    */

    if (query == (_QueryExpression) 0)
	_Raise(inv_query_expr);

    /*
    **  Do the appropriate thing based on the Query Operator
    */

    switch(query->lwk_operator) {
	case lwk_c_any :
	case lwk_c_identity :
	case lwk_c_is_the_source_of :
	case lwk_c_is_the_target_of :
	case lwk_c_is_some_source_of :
	case lwk_c_is_some_target_of :
	case lwk_c_is_the_origin_of :
	case lwk_c_is_the_destination_of :
	case lwk_c_is_some_origin_of :
	case lwk_c_is_some_destination_of :
	case lwk_c_has_source :
	case lwk_c_has_target :
	case lwk_c_has_origin :
	case lwk_c_has_destination :
	    /*
	    **	Miscellanous operators:  return a null expression -- there
	    **	can't be any Key Properties involved in these operators.
	    */

	    key_query = (_QueryExpression) 0;

	    break;

	case lwk_c_has_properties :
	    /*
	    **	Has Properties operator: skip this node, but process its
	    **	operand.
	    */

	    key_query = KeyQuery(file, (_QueryExpression) query->lwk_operand_1);

	    break;

	case lwk_c_property_value :
	case lwk_c_some_property_value :
	    /*
	    **	Property value operands: copy iff Key property -- otherwise,
	    **	return a null expression.
	    */

	    if (IsKeyProperty(file, (_String) query->lwk_operand_1))
		key_query = CopyQueryNode(query);
	    else
		key_query = (_QueryExpression) 0;

	    break;

	case lwk_c_string_literal :
	case lwk_c_date_literal :
	case lwk_c_ddif_string_literal :
	case lwk_c_integer_literal :
	case lwk_c_float_literal :
	case lwk_c_boolean_literal :
	    /*
	    **	Literal value operands: copy this node in case it is
	    **	referenced by a Key Property.
	    */

	    key_query = CopyQueryNode(query);

	    break;

	case lwk_c_or :
	case lwk_c_string_is_eql :
	case lwk_c_ddif_string_is_eql :
	case lwk_c_date_is_eql :
	case lwk_c_integer_is_eql :
	case lwk_c_float_is_eql :
	case lwk_c_boolean_is_eql :
	case lwk_c_string_is_neq :
	case lwk_c_ddif_string_is_neq :
	case lwk_c_date_is_neq :
	case lwk_c_integer_is_neq :
	case lwk_c_float_is_neq :
	case lwk_c_boolean_is_neq :
	case lwk_c_date_is_lss :
	case lwk_c_integer_is_lss :
	case lwk_c_float_is_lss :
	case lwk_c_date_is_gtr :
	case lwk_c_integer_is_gtr :
	case lwk_c_float_is_gtr :
	case lwk_c_date_is_leq :
	case lwk_c_integer_is_leq :
	case lwk_c_float_is_leq :
	case lwk_c_date_is_geq :
	case lwk_c_integer_is_geq :
	case lwk_c_float_is_geq :
	case lwk_c_string_contains :
	case lwk_c_ddif_string_contains :
	case lwk_c_string_contains_no :
	case lwk_c_ddif_string_contains_no :
	    /*
	    **	Logical Or and binary Relational operators:  Process the first
	    **	operand.  If it involves Key Properties, process the second
	    **	operand.  If it involves Key Properties, copy the node.
	    **	Otherwise, return a null expression.
	    */

	    operand1 = KeyQuery(file, (_QueryExpression) query->lwk_operand_1);

	    if (operand1 == (_QueryExpression) 0)
		key_query = (_QueryExpression) 0;
	    else {
		operand2 = KeyQuery(file,
		    (_QueryExpression) query->lwk_operand_2);

		if (operand2 == (_QueryExpression) 0) {
		    DeleteQueryExpression(operand1);
		    key_query = (_QueryExpression) 0;
		}
		else {
		    key_query = CopyQueryNode(query);
		    key_query->lwk_operand_1 = (lwk_any_pointer) operand1;
		    key_query->lwk_operand_2 = (lwk_any_pointer) operand2;
		}
	    }

	    break;

	case lwk_c_and :
	    /*
	    **	Logical And operator:  create a node iff at least one operand
	    **	involves Key Properties.
	    */

	    operand1 = KeyQuery(file, (_QueryExpression) query->lwk_operand_1);
	    operand2 = KeyQuery(file, (_QueryExpression) query->lwk_operand_2);

	    if (operand1 == (_QueryExpression) 0) {
		if (operand2 == (_QueryExpression) 0)
		    key_query = (_QueryExpression) 0;
		else
		    key_query = operand2;
	    }
	    else {
		if (operand2 == (_QueryExpression) 0)
		    key_query = operand1;
		else {
		    key_query = CopyQueryNode(query);
		    key_query->lwk_operand_1 = (lwk_any_pointer) operand1;
		    key_query->lwk_operand_2 = (lwk_any_pointer) operand2;
		}
	    }

	    break;

	case lwk_c_not :
	case lwk_c_boolean_is_true :
	case lwk_c_boolean_is_false :
	    /*
	    **	Logical Not and unary Relational operators:  create a node iff
	    **	the operand involves Key Properties.
	    */

	    operand1 = KeyQuery(file, (_QueryExpression) query->lwk_operand_1);

	    if (operand1 == (_QueryExpression) 0)
		key_query = (_QueryExpression) 0;
	    else {
		key_query = CopyQueryNode(query);
		key_query->lwk_operand_1 = (lwk_any_pointer) operand1;
	    }

	    break;
    }

    return key_query;
    }


static _Boolean  IsKeyProperty(file, name)
_GridFile file;
 _String name;

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

    /*
    **  Validate Property name
    */

    if (name == (_String) _NullObject)
	_Raise(inv_query_expr);
    
    /*
    **  See if it matches one of the Key Properties
    */

    for (i = (int) _KeyProperty; i < file->key_count; i++) {
	if (_CompareString(file->keys[i]->name, name) == 0)
	    return _True;

	if (_CompareString(file->keys[i]->name, _GenericTypeKeyName) == 0) {
	    /*
	    **  Check for the specific names of the generic $Type Property
	    */

	    if (_CompareString(_SurrogateTypeKeyName, name) == 0)
		return _True;

	    if (_CompareString(_LinkTypeKeyName, name) == 0)
		return _True;

	}
    }

    return _False;
    }


static _QueryExpression  RemoveNegationFromQuery(query)
_QueryExpression query;

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
    int arity;
    _QueryExpression new_query;

    /*
    **	Use Shannon's inversion rule (i.e., de Morgans Law) to remove any
    **	Negation.  If we encounter a logical Not operator, return the
    **	complement of its operand.  Otherwise, keep looking.
    */

    if (query->lwk_operator == lwk_c_not) {
	new_query = ComplementQuery((_QueryExpression) query->lwk_operand_1);

	query->lwk_operand_1 = (lwk_any_pointer) 0;
	DeleteQueryExpression(query);
    }
    else {
	arity = QueryOperatorToArity(query->lwk_operator);

	new_query = query;

	if (arity > 0)
	    new_query->lwk_operand_1 = (lwk_any_pointer)
		RemoveNegationFromQuery((_QueryExpression)
		    query->lwk_operand_1);

	if (arity > 1)
	    new_query->lwk_operand_2 = (lwk_any_pointer)
		RemoveNegationFromQuery((_QueryExpression)
		    query->lwk_operand_2);
    }

    return new_query;
    }


static _QueryExpression  ComplementQuery(query)
_QueryExpression query;

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
    int arity;
    _QueryOperator complement;
    _QueryExpression operand1;
    _QueryExpression operand2;
    _QueryExpression new_query;

    /*
    **  If we encounter a logical Not, simple remove it.  Otherwise complement
    **	this operator, and its operands.  If the operator has no complement,
    **	delete the expression.
    */

    if (query->lwk_operator == lwk_c_not) {
	new_query =
	    RemoveNegationFromQuery((_QueryExpression) query->lwk_operand_1);

	query->lwk_operand_1 = (lwk_any_pointer) 0;
	DeleteQueryExpression(query);
    }
    else {
	complement = QueryOperatorToComplement(query->lwk_operator);

	if (complement == lwk_c_not) {
	    DeleteQueryExpression(query);
	    return (_QueryExpression) 0;
	}

	arity = QueryOperatorToArity(query->lwk_operator);

	if (arity > 0) {
	    operand1 = ComplementQuery((_QueryExpression) query->lwk_operand_1);

	    if (operand1 == (_QueryExpression) 0) {
		DeleteQueryExpression(query);
		return (_QueryExpression) 0;
	    }
	}

	if (arity > 1) {
	    operand2 = ComplementQuery((_QueryExpression) query->lwk_operand_2);

	    if (operand2 == (_QueryExpression) 0) {
		DeleteQueryExpression(query);
		return (_QueryExpression) 0;
	    }
	}

	new_query = query;
	new_query->lwk_operator = complement;

	if (arity > 0)
	    new_query->lwk_operand_1 = (lwk_any_pointer) operand1;

	if (arity > 1)
	    new_query->lwk_operand_2 = (lwk_any_pointer) operand2;
    }

    return new_query;
    }


static _QueryExpression  QueryToDNF(query)
_QueryExpression query;

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
    _QueryExpression dnf_query;

    /*
    **  Make sure that the Query is in Disjunctive Normal Form (DNF)
    */

    switch (query->lwk_operator) {
	case lwk_c_or :
	    /*
	    **  For Logical Or, make sure that both operands are in DNF
	    */

	    query->lwk_operand_1 = (lwk_any_pointer)
		QueryToDNF((_QueryExpression) query->lwk_operand_1);

	    query->lwk_operand_2 = (lwk_any_pointer)
		QueryToDNF((_QueryExpression) query->lwk_operand_2);

	    dnf_query = query;

	    break;

	case lwk_c_and :
	    /*
	    **	For Logical And, make sure that each operand is in DNF and then
	    **	distribute the conjunction over any lower disjunctions
	    */

	    query->lwk_operand_1 = (lwk_any_pointer)
		QueryToDNF((_QueryExpression) query->lwk_operand_1);

	    query->lwk_operand_2 = (lwk_any_pointer)
		QueryToDNF((_QueryExpression) query->lwk_operand_2);

	    dnf_query = DistributeConjunction(query);

	    break;

	default :
	    /*
	    **  Relational Operators are in DNF
	    */

	    dnf_query = query;

	    break;
    }

    return dnf_query;
    }


static _QueryExpression  DistributeConjunction(conjunction)
_QueryExpression conjunction;

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
    _QueryExpression dnf_query;
    _QueryExpression operand;
    _QueryExpression disjunction;
    _QueryExpression new_conjunction;

    /*
    **  Promote any Disjunctions (logical Or) over all Conjunctions (logical
    **	And) using the Distributive Law of Conjunction:
    **
    **	    X .And. (Y .Or. Z) == (X .And. Y) .Or. (X .And. Z)
    */

    operand = (_QueryExpression) conjunction->lwk_operand_1;

    if (operand->lwk_operator == lwk_c_or) {
	/*
	**  The first operand of the conjunction is a disjunction -- set up
	**  some parameters for the construction of the new conjunction and
	**  change the conjunction's first operand.
	*/

	disjunction = operand;
	operand = (_QueryExpression) conjunction->lwk_operand_2;
	conjunction->lwk_operand_1 = (lwk_any_pointer)
	    disjunction->lwk_operand_2;
    }
    else {
	operand = (_QueryExpression) conjunction->lwk_operand_2;

	if (operand->lwk_operator == lwk_c_or) {
	    /*
	    **	The second operand of the conjunction is a disjunction -- set
	    **	up some parameters for the construction of the new conjunction
	    **	and change the conjunction's second operand.
	    */

	    disjunction = operand;
	    operand = (_QueryExpression) conjunction->lwk_operand_1;
	    conjunction->lwk_operand_2 = (lwk_any_pointer)
		disjunction->lwk_operand_2;
	}
	else
	    disjunction = (_QueryExpression) 0;
    }

    /*
    **  If one of the conjunction's operands was a disjunction, we need to
    **	create a new conjunction and rearrange some operands.
    */

    if (disjunction == (_QueryExpression) 0)
	dnf_query = conjunction;
    else {
	/*
	**  Create a new conjunction
	*/

	new_conjunction = (_QueryExpression) _AllocateMem(sizeof(_QueryNode));
	new_conjunction->lwk_operator = lwk_c_and;

	/*
	**  Set its operands
	*/

	new_conjunction->lwk_operand_1 = (lwk_any_pointer)
	    CopyQueryExpression(operand);

	new_conjunction->lwk_operand_2 = (lwk_any_pointer)
	    disjunction->lwk_operand_1;

	/*
	**  Reset the disjunction operands
	*/

	disjunction->lwk_operand_1 = (lwk_any_pointer) new_conjunction;
	disjunction->lwk_operand_2 = (lwk_any_pointer) conjunction;

	/*
	**  Recurse on the two operands of the disjunction (now both
	**  conjunctions)
	*/

	disjunction->lwk_operand_1 = (lwk_any_pointer)
	    DistributeConjunction((_QueryExpression)
		disjunction->lwk_operand_1);

	disjunction->lwk_operand_2 = (lwk_any_pointer)
	    DistributeConjunction((_QueryExpression)
		disjunction->lwk_operand_2);

	/*
	**  Return the disjunction
	*/

	dnf_query = disjunction;
    }

    return dnf_query;
    }


static _QueryExpression  CopyQueryExpression(query)
_QueryExpression query;

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
    int arity;
    _QueryExpression copy;

    copy = CopyQueryNode(query);

    arity = QueryOperatorToArity(query->lwk_operator);

    if (arity > 0)
	copy->lwk_operand_1 = (lwk_any_pointer)
	    CopyQueryExpression((_QueryExpression) query->lwk_operand_1);

    if (arity > 1)
	copy->lwk_operand_2 = (lwk_any_pointer)
	    CopyQueryExpression((_QueryExpression) query->lwk_operand_2);

    return copy;
    }


static _QueryExpression  CopyQueryNode(query)
_QueryExpression query;

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
    _QueryExpression copy;

    copy = (_QueryExpression) _AllocateMem(sizeof(_QueryNode));

    copy->lwk_operator = query->lwk_operator;
    copy->lwk_operand_1 = query->lwk_operand_1;
    copy->lwk_operand_2 = query->lwk_operand_2;

    return copy;
    }


static void  DeleteQueryExpression(query)
_QueryExpression query;

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
    int arity;

    /*
    **  If a null node, just return
    */

    if (query == (_QueryExpression) 0)
	return;

    arity = QueryOperatorToArity(query->lwk_operator);

    /*
    **  Delete any operands
    */

    if (arity > 0)
	DeleteQueryExpression((_QueryExpression) query->lwk_operand_1);

    if (arity > 1)
	DeleteQueryExpression((_QueryExpression) query->lwk_operand_2);

    /*
    **  Delete the node
    */

    _FreeMem(query);

    return;
    }


static void  QueryToConjunctKeys(file, key_query, conjunct)
_GridFile file;
 _QueryExpression key_query;

    _ConjunctKeys conjunct;

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
    **  Do the appropriate thing, depending on the Operation
    */

    if (key_query->lwk_operator == lwk_c_and) {
	/*
	**  Logical And -- process both operands
	*/

	QueryToConjunctKeys(file, (_QueryExpression) key_query->lwk_operand_1,
	    conjunct);

	QueryToConjunctKeys(file, (_QueryExpression) key_query->lwk_operand_2,
	    conjunct);
    }
    else if (key_query->lwk_operator == lwk_c_or) {
	_ConjunctKeys disjunct;

	/*
	**  Logical Or -- process the first operand into the conjunct Keys,
	**  then process the second operand into a newly create disjunct Keys.
	*/

	QueryToConjunctKeys(file, (_QueryExpression) key_query->lwk_operand_1,
	    conjunct);

	disjunct = LwkGCCreateConjunctKeys(conjunct->domain);
	disjunct->identifier = conjunct->identifier;
	disjunct->container = conjunct->container;

	conjunct->disjunct = disjunct;

	QueryToConjunctKeys(file, (_QueryExpression) key_query->lwk_operand_2,
	    disjunct);
    }
    else {
	_String name;
	_Domain domain;
	_AnyValue value;
	_Property property;
	_Boolean name_first;

	name_first = QueryKeyPropertyName(key_query, &name);
	domain = QueryOperatorToDomain(key_query->lwk_operator);
	QueryKeyPropertyValue(key_query, name_first, domain, &value);

	property = (_Property) _Create(_TypeProperty);

	_SetValue(property, _P_PropertyName, lwk_c_domain_string, &name,
	    lwk_c_set_property);

	_SetValue(property, _P_Value, domain, &value, lwk_c_set_property);

	if (conjunct->key_properties == (_Set) _NullObject)
	    conjunct->key_properties =
		(_Set) _CreateSet(_TypeSet, lwk_c_domain_property,
		    (_Integer) 1);

	_AddElement(conjunct->key_properties, lwk_c_domain_property,
	    &property, _False);
    }

    return;
    }


static _Boolean  QueryKeyPropertyName(key_query, name)
_QueryExpression key_query;
 _String *name;

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
    _Boolean name_first;
    _QueryExpression property;

    /*
    **  The Property name should be one of the two operands
    */

    property = (_QueryExpression) key_query->lwk_operand_1;
    
    if (property->lwk_operator == lwk_c_property_value) {
	name_first = _True;
	*name = (_String) property->lwk_operand_1;
    }
    else {
	property = (_QueryExpression) key_query->lwk_operand_2;

	if (property->lwk_operator == lwk_c_property_value) {
	    name_first = _False;
	    *name = (_String) property->lwk_operand_1;
	}
	else
	    _Raise(inv_query_expr);
    }

    /*
    **  Check for the specific names of the generic $Type Property
    */

    if (_CompareString(*name, _SurrogateTypeKeyName) == 0
	    || _CompareString(*name, _LinkTypeKeyName) == 0)
	*name = (_String) _GenericTypeKeyName;

    return name_first;
    }


static void  QueryKeyPropertyValue(key_query, name_first, domain, value)
_QueryExpression key_query;

    _Boolean name_first;
 _Domain domain;
 _AnyValue *value;

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
    _QueryExpression literal;

    /*
    **  Get the value from the Query node
    */

    if (name_first)
	literal = (_QueryExpression) key_query->lwk_operand_2;
    else
	literal = (_QueryExpression) key_query->lwk_operand_1;

    switch (domain) {
	case lwk_c_domain_integer :
	case lwk_c_domain_float :
	case lwk_c_domain_boolean :
	    _MoveValue(literal->lwk_operand_1, value, domain);
	    break;

	case lwk_c_domain_string :
	case lwk_c_domain_ddif_string :
	case lwk_c_domain_date :
	    _MoveValue(&literal->lwk_operand_1, value, domain);
	    break;
    }

    return;
    }

    
static _String  QueryToString(query)
_QueryExpression query;

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
    int arity;
    _String string;
    _String operator;
    _String operand1;
    _String operand2;
    char buffer[50];

    /*
    **  Deal with Property names and literal values separately
    */

    switch (query->lwk_operator) {
	case lwk_c_property_value :
	case lwk_c_some_property_value :
	    return _CopyString((_String) query->lwk_operand_1);

	case lwk_c_string_literal :
	case lwk_c_date_literal :
	    string = _CopyString((_String) "\"");
	    string = _ConcatString(string, (_String) query->lwk_operand_1);
	    return _ConcatString(string, (_String) "\"");

	case lwk_c_ddif_string_literal :
	    operand1 = _DDIFStringToString((_String) query->lwk_operand_1);
	    string = _CopyString((_String) "\"");
	    string = _ConcatString(string, operand1);
	    _DeleteString(&operand1);
	    return _ConcatString(string, (_String) "\"");

	case lwk_c_integer_literal :
	    sprintf(buffer, "%ld", *((_IntegerPtr) query->lwk_operand_1));
	    return _CopyString((_String) buffer);

	case lwk_c_float_literal :
	    sprintf(buffer, "%g", *((_FloatPtr) query->lwk_operand_1));
	    return _CopyString((_String) buffer);

	case lwk_c_boolean_literal :
	    if (*((_BooleanPtr) query->lwk_operand_1))
		return _CopyString((_String) "True");
	    else
		return _CopyString((_String) "False");

	case lwk_c_identity :
	    sprintf(buffer, "%s %x", QueryOperatorToString(query->lwk_operator),
		query->lwk_operand_1);
	    return _CopyString((_String) buffer);

	case lwk_c_any :
	    return _CopyString(QueryOperatorToString(query->lwk_operator));

	case lwk_c_boolean_is_true :
	case lwk_c_boolean_is_false :
	    return _CopyString(QueryOperatorToString(query->lwk_operator));
	    break;
    }

    /*
    **  Get the Operator name and an indicator of its arity
    */

    operator = QueryOperatorToString(query->lwk_operator);
    arity = QueryOperatorToArity(query->lwk_operator);

    /*
    **  Get the operand(s)
    */

    if (arity > 0)
	operand1 = QueryToString((_QueryExpression) query->lwk_operand_1);

    if (arity > 1)
	operand2 = QueryToString((_QueryExpression) query->lwk_operand_2);
    else
	operand2 = (_String) _NullObject;

    /*
    **  Compose the string
    */

    string = _CopyString((_String) "(");

    if (arity == 1) {
	string = _ConcatString(string, operator);
	string = _ConcatString(string, operand1);
    }
    else if (arity == 2) {
	string = _ConcatString(string, operand1);
	string = _ConcatString(string, operator);
	string = _ConcatString(string, operand2);
    }

    string = _ConcatString(string, ")");

    /*
    **  Clean up
    */

    _DeleteString(&operand1);
    _DeleteString(&operand2);

    return string;
    }


static _String  QueryOperatorToString(operator)
_QueryOperator operator;

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
    _CharPtr string;

    string = "";

    for (i = 0; i < OperatorCount; i++)
	if (Operators[i].operator == operator) {
	    string = Operators[i].string;
	    break;
	}

    return (_String) string;
    }


static int  QueryOperatorToArity(operator)
_QueryOperator operator;

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
    int arity;

    arity = 0;

    for (i = 0; i < OperatorCount; i++)
	if (Operators[i].operator == operator) {
	    arity = Operators[i].arity;
	    break;
	}

    return arity;
    }


static _Domain  QueryOperatorToDomain(operator)
_QueryOperator operator;

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
    _Domain domain;

    domain = lwk_c_domain_unknown;

    for (i = 0; i < OperatorCount; i++)
	if (Operators[i].operator == operator) {
	    domain = Operators[i].domain;
	    break;
	}

    return domain;
    }


static _QueryOperator  QueryOperatorToComplement(operator)
_QueryOperator operator;

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
    _QueryOperator complement;

    complement = operator;

    for (i = 0; i < OperatorCount; i++)
	if (Operators[i].operator == operator) {
	    complement = Operators[i].complement;
	    break;
	}

    return complement;
    }


static void  PrintProperties(properties)
_Set properties;

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
    if (properties != (_Set) _NullObject)
	_Iterate(properties, lwk_c_domain_property, (_Closure) 0,
	    PrintProperty);

    _Trace("\n");

    return;
    }


static _Termination _EntryPt  PrintProperty(null, set, domain, property)
_Closure null;
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
    _String name;
    _String string;
    _Integer integer;
    _Domain value_domain;
    _AnyValue value;

    /*
    **  Get the Property Name, Domain and Value
    */

    _GetValue(*property, _P_PropertyName, lwk_c_domain_string, &name);
    _GetValue(*property, _P_Domain, lwk_c_domain_integer, &integer);

    value_domain = (_Domain) integer;

    _GetValue(*property, _P_Value, value_domain, &value);

    /*
    **  Print them out
    */

    _Trace(", %s: ", name);

    switch (value_domain) {
	case lwk_c_domain_integer :
	    _Trace("%ld", value.integer);
	    break;

	case lwk_c_domain_float :
	    _Trace("%f", value.float_pt);
	    break;

	case lwk_c_domain_string :
	    if (value.string == (_String) 0)
		_Trace("Null");
	    else
		_Trace(value.string);

	    break;

	case lwk_c_domain_ddif_string :
	    if (value.ddifstring == (_DDIFString) 0)
		_Trace("Null");
	    else {
		string = _DDIFStringToString(value.ddifstring);
		_Trace(string);
		_DeleteString(&string);
	    }

	    break;

	case lwk_c_domain_date :
	    if (value.date == (_Date) 0)
		_Trace("Null");
	    else {
		string = _DateToString(value.date);
		_Trace(string);
		_DeleteString(&string);
	    }

	    break;

	default :
	    _Trace("Unknown");
	    break;
    }

    /*
    **  Clean up
    */

    _DeleteString(&name);
    _DeleteValue(&value, value_domain);

    return (_Termination) 0;
    }
