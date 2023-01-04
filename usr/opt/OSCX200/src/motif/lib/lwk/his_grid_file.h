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
**	LinkWorks Services
**
**  Version: V1.0
**
**  Abstract:
**	Common header file for all modules which access the Grid File
**	implementation which underlies the LinkWorks Database.
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
**  Creation Date: 19-Jul-88
**
**  Modification History:
**  X0.16   WWC  22-Feb-91  LWK name translation
**			    ".his" --> ".linkbase"
**--
*/


/*
**  Include Files
*/

#if VMS
#include <rms.h>
#include <lckdef.h>
#include <descrip.h>
#include <varargs.h>

#else /* !VMS */

#ifdef MSDOS
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys\stat.h>
#include <sys\locking.h>
#include <stdarg.h>

#else /* !VMS && !MSDOS */
#include <sys/file.h>
#include <sys/stat.h>
#include <varargs.h>

#endif /* MSDOS */
#endif /* VMS */

/*
**  Macro Definitions
*/

#define _FileVersion 1004

#define _DirectoryBucket ((_BucketNumber) 0)
#define _RegionsBucket ((_BucketNumber) 1)
#define _InitialDataBucket ((_BucketNumber) 2)

#define _PreferredBucketSize 4096
#define _DefaultTrace 0
#define _DefaultCacheSize 10
#define _DefaultBucketSize (_Max(1, _PreferredBucketSize / BUFSIZ) * BUFSIZ)
#define _DefaultExtensionBuckets 10

#if VMS
#define _DefaultRecordMode _True
#define _DefaultLock _False
#elif MSDOS
#define _DefaultRecordMode _False
#define _DefaultLock _False
#else
#define _DefaultRecordMode _False
#define _DefaultLock _True
#endif

#define _BlocksPerBucket (LwkGridBucketSize / BUFSIZ)

#define _ExtensionBuckets LwkGridExtensionBuckets
#define _ExtensionBlocks (LwkGridExtensionBuckets * _BlocksPerBucket)

#define _BucketHeaderSize (sizeof(_BufferInstance) - sizeof(_BufferData))
#define _BucketDataSize(Bucket) ((Bucket)->size - _BucketHeaderSize)
#define _BucketFreeSize(Bucket) \
    (_BucketDataSize(Bucket) - (Bucket)->buffer->first_free)

#define _RecordHeaderSize ((3 * sizeof(_Integer)) + sizeof(_Domain))
#define _RecordISize (sizeof(_Integer))
#define _RecordVSize(VString) (sizeof(_Integer) + _VSize_of(VString))

#define _DefaultFileName ".linkbase"

#ifndef VMS
#ifdef MSDOS
#define _PathDelimiterCharacter '\\'
#define _FullPath(Path) \
    (_Strchr((Path), ':') != NULL || (Path)[0] == _PathDelimiterCharacter)
#else
#define _PathDelimiterCharacter '/'
#define _FullPath(Path) ((Path)[0] == _PathDelimiterCharacter)
#endif /* MSDOS */
#endif /* VMS */

#define _FileSuccess(Status) ((Status) == _FileStsSuccess)

/*
** Trace support
*/

#define _Trace LwkGridLogTrace

#define _TraceReadMask 1
#define _TraceWriteMask 2
#define _TraceStoreMask 4
#define _TraceQueryMask 8
#define _TraceQueryResultsMask 16
#define _TraceTransactMask 32
#define _TraceLockMask 64

#define _TraceRead  ((LwkGridTrace & 1) != 0)
#define _TraceWrite ((LwkGridTrace & 2) != 0)
#define _TraceStore ((LwkGridTrace & 4) != 0)
#define _TraceQuery ((LwkGridTrace & 8) != 0)
#define _TraceQueryResults ((LwkGridTrace & 16) != 0)
#define _TraceTransact ((LwkGridTrace & 32) != 0)
#define _TraceLock ((LwkGridTrace & 64) != 0)

/*
**  Predefined Keys
*/

#define _MaxKeys 9

#define _PreDefinedKeysInitialization { \
	{_KeyDomain, "$Domain", lwk_c_domain_integer}, \
	{_KeyIdentifier, "$Identifier", lwk_c_domain_integer}, \
	{_KeyContainer, "$Container", lwk_c_domain_integer}, \
	{_KeyProperty, _GenericTypeKeyName, lwk_c_domain_string}, \
	{_KeyProperty, "%Type", lwk_c_domain_string}, \
	{_KeyProperty, "%Container", lwk_c_domain_string}, \
	{_KeyProperty, "%SubContainer", lwk_c_domain_string}, \
 	{_KeyProperty, "%Key", lwk_c_domain_ddif_string}, \
	{_KeyProperty, "%Id", lwk_c_domain_integer}, \
    }

/*
**  Type Definitions
*/

typedef enum __FileStatus {
	    _FileStsSuccess,
	    _FileStsFailure,
	    _FileStsNoneFound,
	    _FileStsCursorEmpty,
	    _FileStsNoSuchDatabase
	} _FileStatus, _PtrTo _FileStatusPtr;

typedef _Integer _BucketNumber, _PtrTo _BucketNumberPtr;

typedef struct __BucketList {
	    _Integer count;
	    _BucketNumber _PtrTo numbers;
	} _BucketListInstance, _PtrTo _BucketList, _PtrTo _PtrTo _BucketListPtr;

typedef char _BufferData[sizeof(_Integer)];

typedef _Integer _RegionVertex[_MaxKeys];

typedef struct __Region {
	    struct __Region _PtrTo closer;
	    struct __Region _PtrTo further;
	    _Integer base_radius;
	    _Integer extent_radius;
	    _RegionVertex base_vertex;
	    _RegionVertex extent_vertex;
	    _BucketNumber bucket;
	} _RegionInstance, _PtrTo _Region, _PtrTo _PtrTo _RegionPtr;

typedef struct __Buffer {
	    unsigned first_free;
	    _Integer record_count;
	    _BucketNumber extension;
	    _BufferData data;
	} _BufferInstance, _PtrTo _Buffer, _PtrTo _PtrTo BufferPtr;

typedef struct __Bucket {
	    struct __Bucket _PtrTo next;
	    struct __Bucket _PtrTo previous;
	    _Buffer buffer;
	    _BucketNumber number;
	    _Integer size;
	    _Boolean modified;
	    _Boolean buffer_valid;
	} _BucketInstance, _PtrTo _Bucket, _PtrTo _PtrTo _BucketPtr;

typedef enum __KeyType {
	    _KeyDomain = 0,
	    _KeyIdentifier,
	    _KeyContainer,
	    _KeyProperty
	} _KeyType;

typedef union __KeyValue {
	    _Integer integer;
	    _Float float_pt;
	    _String string;
	    _DDIFString ddifstring;
	    _Date date;
	} _KeyValue, _PtrTo _KeyValuePtr;

typedef struct __KeyInterval {
	    struct __KeyInterval _PtrTo next;
	    _KeyValue base;
	    _KeyValue split;
	} _KeyIntervalInstance, _PtrTo _KeyInterval,
	    _PtrTo _PtrTo _KeyIntervalPtr;

typedef struct __Key {
	    _KeyType type;
	    _String name;
	    _KeyInterval intervals;
	    _Integer interval_count;
	    _Domain domain;
	    _KeyValue base;
	    _KeyValue split;
	    _KeyValue extent;
	} _KeyInstance, _PtrTo _Key, _PtrTo _PtrTo _KeyPtr;

typedef struct __PredefinedKey {
	    _KeyType type;
	    _String name;
	    _Domain domain;
	} _PredefinedKey;

typedef struct __Lock {
	    struct __Lock _PtrTo lower;
	    struct __Lock _PtrTo higher;
	    _BucketNumber number;
	    _Transaction state;
#ifdef VMS
	    int id;
#endif
	} _LockInstance, _PtrTo _Lock, _PtrTo _PtrTo _LockPtr;

typedef struct __GridFile {
	    _String identifier;
	    _DDIFString name;
	    _Key keys[_MaxKeys];
	    _AnyPtr cursors;
	    _Bucket most_recent;
	    _Bucket least_recent;
	    _Lock locks;
	    _Region regions;
	    _BucketNumber next_bucket;
#ifdef VMS
	    struct RAB *rab;
	    int lock;
#else
	    int file;
#endif
	    _Integer version;
	    _Integer file_sn;
	    _Integer regions_sn;
	    _Integer directory_sn;
	    _Integer key_count;
	    _Integer bucket_size;
	    _Integer cache_size;
	    _Integer cache_count;
	    _Integer next_id;
	    _Integer next_to_split;
	    _Transaction state;
	    _Boolean read_only;
	    _Boolean file_modified;
	    _Boolean regions_modified;
	    _Boolean directory_modified;
	} _GridFileInstance, _PtrTo _GridFile, _PtrTo _PtrTo _GridFilePtr;

typedef struct __ConjunctKeys {
	    struct __ConjunctKeys _PtrTo disjunct;
	    _Set key_properties;
	    _Integer identifier;
	    _Integer container;
	    _Domain domain;
	} _ConjunctKeysInstance, _PtrTo _ConjunctKeys,
	    _PtrTo _PtrTo _ConjunctKeysPtr;

typedef struct __GridCursor {
	    struct __GridCursor _PtrTo next;
	    _GridFile file;
	    _String name;
	    _Bucket bucket;
	    _Bucket ext_bucket;
	    _ConjunctKeys keys;
	    _Integer current_id;
	    _Integer current_bucket;
	    _Integer b_search_count;
	    _Integer ext_b_search_count;
	    _Integer r_search_count;
	    _Integer select_count;
	    _Integer position;
	    _Integer ext_base;
	    _BucketListInstance bucket_list;
	} _GridCursorInstance, _PtrTo _GridCursor, _PtrTo _PtrTo _GridCursorPtr;

/*
**  External Data Declarations
*/

_External FILE *LwkGridTraceFile;
_External _Integer LwkGridTrace;
_External _Integer LwkGridBucketSize;
_External _Integer LwkGridCacheSize;
_External _Integer LwkGridExtensionBuckets;
_External _Boolean LwkGridRecordMode;
_External _Boolean LwkGridLock;

/*
**  External Routine Declarations
*/

/*
**  Grid Module
*/

#ifdef MSDOS
_DeclareFunction(void LwkGridLogTrace, (_CharPtr format, ...));
#else
_DeclareFunction(void LwkGridLogTrace, ());
#endif

_DeclareFunction(void LwkGridInitialize, (void));
_DeclareFunction(_FileStatus LwkGridExpandIdentifier,
    (_String identifier, _StringPtr expanded_id));
_DeclareFunction(_FileStatus LwkGridOpen,
    (_String identifier, _Boolean create, _GridFilePtr file,
	_DDIFStringPtr name));
_DeclareFunction(_FileStatus LwkGridClose, (_GridFile file));
_DeclareFunction(_FileStatus LwkGridDeleteFile,
    (_GridFile file, _String identifier));
_DeclareFunction(_FileStatus LwkGridRenameFile,
    (_GridFile file, _String identifier));
_DeclareFunction(_FileStatus LwkGridSetName,
    (_GridFile file, _DDIFString name));
_DeclareFunction(_FileStatus LwkGridAllocateUids,
    (_GridFile file, _Integer count, _IntegerPtr id));
_DeclareFunction(_FileStatus LwkGridSetTransaction,
    (_GridFile file, _Transaction state));
_DeclareFunction(_FileStatus LwkGridSetTransactionReadWrite, (_GridFile file));
_DeclareFunction(_FileStatus LwkGridDeleteObject,
    (_GridFile file, _Domain domain, _Integer id));
_DeclareFunction(_FileStatus LwkGridInsertNetwork,
    (_GridFile file, _Integer id, _VaryingString encoding, _Boolean new));
_DeclareFunction(_FileStatus LwkGridSelectNetwork,
    (_GridFile file, _Integer id, _DynamicVString encoding));
_DeclareFunction(_FileStatus LwkGridInsertLink,
    (_GridFile file, _Integer id, _Integer source, _Integer target,
	_Integer network, _VaryingString encoding, _Boolean new));
_DeclareFunction(_FileStatus LwkGridSelectLink,
    (_GridFile file, _Integer id, _Integer network, _IntegerPtr source,
	_IntegerPtr target, _DynamicVString encoding));
_DeclareFunction(_FileStatus LwkGridInsertSurrogate,
    (_GridFile file, _Integer id, _Integer container,
	_Integer container_domain, _Integer has_incomming,
	_Integer has_outgoing, _Integer count, _IntegerPtr interconnect_ids,
	_VaryingString encoding, _Boolean new));
_DeclareFunction(_FileStatus LwkGridSelectSurrogate,
    (_GridFile file, _Integer id, _Integer container,
	_IntegerPtr container_domain, _IntegerPtr has_incomming,
	_IntegerPtr has_outgoing, _IntegerPtr count,
	_IntegerPtr *interconnect_ids, _DynamicVString encoding));
_DeclareFunction(_FileStatus LwkGridCloseCursor, (_GridCursor cursor));
_DeclareFunction(_FileStatus LwkGridOpenDirectory,
    (_GridFile file, _Domain domain, _GridCursorPtr cursor));
_DeclareFunction(_FileStatus LwkGridFetchDirectory,
    (_GridCursor cursor, _IntegerPtr id, _IntegerPtr container_id));
_DeclareFunction(_FileStatus LwkGridOpenConnInNetwork,
    (_GridFile file, _Integer network, _GridCursorPtr cursor));
_DeclareFunction(_FileStatus LwkGridFetchConnInNetwork,
    (_GridCursor cursor, _IntegerPtr connection, _IntegerPtr source,
	_IntegerPtr target, _DynamicVString encoding));
_DeclareFunction(_FileStatus LwkGridOpenSurrInContainer,
    (_GridFile file, _Integer container, _GridCursorPtr cursor));
_DeclareFunction(_FileStatus LwkGridFetchSurrInContainer,
    (_GridCursor cursor, _IntegerPtr surrogate, _IntegerPtr has_incomming,
	_IntegerPtr has_outgoing, _IntegerPtr count,
	_IntegerPtr *interconnect_ids,
	_DynamicVString encoding));
_DeclareFunction(_FileStatus LwkGridOpenSurrConn,
    (_GridFile file, _Integer network, _Integer surrogate, _Integer count,
	_IntegerPtr interconnect_ids, _GridCursorPtr cursor));
_DeclareFunction(_FileStatus LwkGridFetchSurrConn,
    (_GridCursor cursor, _IntegerPtr connection, _IntegerPtr source,
	_IntegerPtr target, _DynamicVString encoding));
_DeclareFunction(_FileStatus LwkGridOpenQuerySurrogates,
    (_GridFile file, _Integer container, _QueryExpression query,
	_GridCursorPtr cursor));
_DeclareFunction(_FileStatus LwkGridFetchQuerySurrogates,
    (_GridCursor cursor, _IntegerPtr surrogate, _IntegerPtr has_incomming,
    _IntegerPtr has_outgoing, _IntegerPtr count,
    _IntegerPtr *interconnect_ids, _DynamicVString encoding));
_DeclareFunction(_FileStatus LwkGridOpenQueryLinks,
    (_GridFile file, _Integer network, _QueryExpression query,
	_GridCursorPtr cursor));
_DeclareFunction(_FileStatus LwkGridFetchQueryLinks,
    (_GridCursor cursor, _IntegerPtr connection, _IntegerPtr source,
	_IntegerPtr target, _DynamicVString encoding));
_DeclareFunction(_FileStatus LwkGridVerify,
    (_GridFile file, _Integer flags, _OSFile list_file));

#ifndef MSDOS

_DeclareFunction(_FileStatus LwkGridInsertCompositePath,
    (_GridFile file, _Integer id, _VaryingString encoding, _Boolean new));
_DeclareFunction(_FileStatus LwkGridSelectCompositePath,
    (_GridFile file, _Integer id, _DynamicVString encoding));
_DeclareFunction(_FileStatus LwkGridInsertCompositeNet,
    (_GridFile file, _Integer id, _VaryingString encoding, _Boolean new));
_DeclareFunction(_FileStatus LwkGridSelectCompositeNet,
    (_GridFile file, _Integer id, _DynamicVString encoding));
_DeclareFunction(_FileStatus LwkGridInsertPath,
    (_GridFile file, _Integer id, _Integer first_step, _Integer last_step,
	_Integer current_step, _VaryingString encoding, _Boolean new));
_DeclareFunction(_FileStatus LwkGridSelectPath,
    (_GridFile file, _Integer id, _IntegerPtr first_step,
	_IntegerPtr last_step, _IntegerPtr current_step,
	_DynamicVString encoding));
_DeclareFunction(_FileStatus LwkGridInsertStep,
    (_GridFile file, _Integer id, _Integer origin,
	_Integer destination, _Integer previous, _Integer next, _Integer path,
	_VaryingString encoding, _Boolean new));
_DeclareFunction(_FileStatus LwkGridSelectStep,
    (_GridFile file, _Integer id, _Integer path, _IntegerPtr origin,
	_IntegerPtr destination, _IntegerPtr previous,
	_IntegerPtr next, _DynamicVString encoding));
_DeclareFunction(_FileStatus LwkGridOpenSurrStep,
    (_GridFile file, _Integer path, _Integer surrogate, _Integer count,
	_IntegerPtr interconnect_ids, _GridCursorPtr cursor));
_DeclareFunction(_FileStatus LwkGridFetchSurrStep,
    (_GridCursor cursor, _IntegerPtr step, _IntegerPtr origin,
	_IntegerPtr destination, _IntegerPtr previous,
	_IntegerPtr next, _DynamicVString encoding));
_DeclareFunction(_FileStatus LwkGridOpenStepInPath,
    (_GridFile file, _Integer path, _GridCursorPtr cursor));
_DeclareFunction(_FileStatus LwkGridFetchStepInPath,
    (_GridCursor cursor, _IntegerPtr step, _IntegerPtr origin,
	_IntegerPtr destination, _IntegerPtr previous,
	_IntegerPtr next, _DynamicVString encoding));
_DeclareFunction(_FileStatus LwkGridOpenQuerySteps,
    (_GridFile file, _Integer path, _QueryExpression expression,
     _GridCursorPtr cursor));
_DeclareFunction(_FileStatus LwkGridFetchQuerySteps,
    (_GridCursor cursor, _IntegerPtr step,
     _IntegerPtr origin, _IntegerPtr destination, _IntegerPtr previous,
     _IntegerPtr next, _DynamicVString encoding));

#endif /* !MSDOS */

/*
**  Grid File Module
*/

_DeclareFunction(_GridFile LwkGFInitializeFile, (_String identifier));
_DeclareFunction(void LwkGFFreeFile, (_GridFile file));
_DeclareFunction(_String LwkGFExpandIdentifier, (_String identifier));
_DeclareFunction(_Boolean LwkGFOpenFile, (_GridFile file, _Boolean create));
_DeclareFunction(void LwkGFCloseFile, (_GridFile file));
_DeclareFunction(void LwkGFDeleteFile, (_String identifier));
_DeclareFunction(void LwkGFRenameFile, (_GridFile file, _String identifier));
_DeclareFunction(void LwkGFRefreshFile, (_GridFile file));
_DeclareFunction(void LwkGFReadFile, (_GridFile file, _Bucket bucket));
_DeclareFunction(void LwkGFWriteFile, (_GridFile file, _Bucket bucket));
_DeclareFunction(void LwkGFStartTransaction,
    (_GridFile file, _Transaction state));
_DeclareFunction(void LwkGFCommitFile, (_GridFile file));
_DeclareFunction(void LwkGFRollbackFile,
    (_GridFile file, _Boolean raise_exceptions));
_DeclareFunction(void LwkGFAllocateBucket,
    (_GridFile file, _BucketNumber number));

/*
**  Directory Module
*/

_DeclareFunction(void LwkGDInitializeDirectory, (_GridFile file));
_DeclareFunction(void LwkGDFreeDirectory, (_GridFile file));
_DeclareFunction(void LwkGDReadDirectory, (_GridFile file));
_DeclareFunction(void LwkGDWriteDirectory, (_GridFile file));
_DeclareFunction(void LwkGDVerifyDirectory,
    (_GridFile file, _Integer flags, _OSFile list_file));
_DeclareFunction(_Bucket LwkGDBucketToStore,
    (_GridFile file, _Integer size, _Boolean new, _ConjunctKeys keys));
_DeclareFunction(void LwkGDBucketsToRetrieve,
    (_GridFile file, _ConjunctKeys keys, _BucketList bucket_list));
_DeclareFunction(void LwkGDGetBucketList,
    (_GridFile file, _RegionVertex base_vertex, _RegionVertex extent_vertex,
	_BucketList bucket_list));
_DeclareFunction(void LwkGDInitializeKeys, (_GridFile file));
_DeclareFunction(_CharPtr LwkGDDomainToName, (_Domain domain));

/*
**  Bucket Module
*/

_DeclareFunction(void LwkGBFreeBuckets, (_GridFile file));
_DeclareFunction(_Bucket LwkGBAllocateBucket,
    (_GridFile file, _BucketNumber number));
_DeclareFunction(_Bucket LwkGBAllocateNextBucket, (_GridFile file));
_DeclareFunction(void LwkGBDeallocateBucket,
    (_GridFile file, _Bucket bucket));
_DeclareFunction(_Bucket LwkGBReadBucket,
    (_GridFile file, _BucketNumber number));
_DeclareFunction(_Bucket LwkGBExtendBucket, (_GridFile file, _Bucket bucket));
_DeclareFunction(void LwkGBVerifyBuckets,
    (_GridFile file, _Integer flags, _OSFile list_file));
_DeclareFunction(void LwkGBFlushBuckets, (_GridFile file));
_DeclareFunction(void LwkGBInvalidateBuckets, (_GridFile file));

/*
**  Cursor Module
*/

_DeclareFunction(_GridCursor LwkGCCreateCursor,
    (_String name, _GridFile file, _ConjunctKeys keys));
_DeclareFunction(_GridCursor LwkGCCreateDirectoryCursor, (_GridFile file));
_DeclareFunction(_GridCursor LwkGCCreateRegionsCursor, (_GridFile file));
_DeclareFunction(_GridCursor LwkGCCreateSplitCursor,
    (_GridFile file, _BucketNumber number));
_DeclareFunction(_GridCursor LwkGCCreateRewriteCursor,
    (_GridFile file, _BucketNumber number, _Domain domain,
	_Integer identifier));
_DeclareFunction(_GridCursor LwkGCCreateVerifyCursor, (_GridFile file));
_DeclareFunction(void LwkGCFreeCursor, (_GridCursor cursor));
_DeclareFunction(_Bucket LwkGCReadBucketExtension,
    (_GridCursor cursor));
_DeclareFunction(_ConjunctKeys LwkGCCreateConjunctKeys, (_Domain domain));
_DeclareFunction(void LwkGCSetKeyProperties,
    (_ConjunctKeys keys, _VaryingString encoding));
_DeclareFunction(void LwkGCFreeConjunctKeys, (_ConjunctKeys keys,
    _Boolean store));
_DeclareFunction(void LwkGCExtractQueryKeys,
    (_GridFile file, _QueryExpression expression, _ConjunctKeys keys));

/*
**  Record Module
*/

_DeclareFunction(_FileStatus LwkGRFetchIdByDomain,
    (_GridCursor cursor, _IntegerPtr id, _IntegerPtr container_id));
_DeclareFunction(_Boolean LwkGRFindById, (_GridCursor cursor));
_DeclareFunction(_Boolean LwkGRFindByContainer,
    (_GridCursor cursor, _IntegerPtr id));
_DeclareFunction(_Boolean LwkGRNextRecord,
    (_GridCursor cursor, _Boolean skip_current));
_DeclareFunction(_Boolean LwkGRNextRecordBucket, (_GridCursor cursor));
_DeclareFunction(void LwkGRMoveRecord, (_GridCursor cursor, _Bucket bucket));
_DeclareFunction(void LwkGRDeleteRecord, (_GridCursor cursor));
_DeclareFunction(void LwkGRDeleteRewrittenRecord,
    (_GridFile file, _BucketNumber number, _Domain domain,
	_Integer identifier));
_DeclareFunction(_Integer LwkGRVerifyRecord,
    (_GridCursor cursor, _Integer flags, _OSFile list_file));
_DeclareFunction(_Bucket LwkGRStoreRecordHeader,
    (_GridFile file, _Bucket bucket, _Integer size, _Integer id, _Domain domain,
	_Integer container));
_DeclareFunction(_Bucket LwkGRStoreInteger,
    (_GridFile file, _Bucket bucket, _Integer integer));
_DeclareFunction(_Bucket LwkGRStoreIntegers,
    (_GridFile file, _Bucket bucket, _Integer count, _IntegerPtr integers));
_DeclareFunction(_Bucket LwkGRStoreFloat,
    (_GridFile file, _Bucket bucket, _Float float_pt));
_DeclareFunction(_Bucket LwkGRStoreString,
    (_GridFile file, _Bucket bucket, _String string));
_DeclareFunction(_Bucket LwkGRStoreDDIFString,
    (_GridFile file, _Bucket bucket, _DDIFString ddifstring));
_DeclareFunction(_Bucket LwkGRStoreDate,
    (_GridFile file, _Bucket bucket, _Date date));
_DeclareFunction(_Bucket LwkGRStoreVaryingString,
    (_GridFile file, _Bucket bucket, _VaryingString vstring));
_DeclareFunction(_Integer LwkGRSkipRecordHeader, (_GridCursor cursor));
_DeclareFunction(_Integer LwkGRSkipInteger,
    (_GridCursor cursor, _Integer position));
_DeclareFunction(_Integer LwkGRSkipVaryingString,
    (_GridCursor cursor, _Integer position));
_DeclareFunction(_Integer LwkGRRetrieveRecordHeader,
    (_GridCursor cursor, _IntegerPtr size, _IntegerPtr id,
	_DomainPtr domain, _IntegerPtr container));
_DeclareFunction(_Integer LwkGRRetrieveInteger,
    (_GridCursor cursor, _Integer position, _IntegerPtr integer));
_DeclareFunction(_Integer LwkGRRetrieveIntegers,
    (_GridCursor cursor, _Integer position, _Integer count,
	_IntegerPtr integers));
_DeclareFunction(_Integer LwkGRRetrieveFloat,
    (_GridCursor cursor, _Integer position, _FloatPtr float_pt));
_DeclareFunction(_Integer LwkGRRetrieveString,
    (_GridCursor cursor, _Integer position, _StringPtr string));
_DeclareFunction(_Integer LwkGRRetrieveDDIFString,
    (_GridCursor cursor, _Integer position, _DDIFStringPtr ddifstring));
_DeclareFunction(_Integer LwkGRRetrieveDate,
    (_GridCursor cursor, _Integer position, _DatePtr date));
_DeclareFunction(_Integer LwkGRRetrieveVaryingString,
    (_GridCursor cursor, _Integer position, _DynamicVString dvstring));

/*
**  Lock Module
*/

_DeclareFunction(void LwkGLLockFile, (_GridFile file, _Transaction state));
_DeclareFunction(void LwkGLUnlockFile,
    (_GridFile file, _Boolean raise_exceptions));
_DeclareFunction(void LwkGLLockBucket,
    (_GridFile file, _BucketNumber number, _Transaction state));
