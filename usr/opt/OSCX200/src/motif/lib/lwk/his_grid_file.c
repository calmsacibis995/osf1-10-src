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
** COPYRIGHT (c) 1988, 1992 BY
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
**  Version: V1.1
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
**  Creation Date: 20-Jul-89
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

#ifdef VMS

typedef struct __ItemList {
	    short int length;
	    short int code;
	    _AnyPtr address;
	    _AnyPtr return_length_address;
	    int eol;
	} _ItemList;

#endif /* VMS */             

/*
**  Macro Definitions
*/

#ifdef VMS
#define _MaxLockNameLength 31
#define _LockNamePrefix "LwkOp"

#define DVI$_DEVCHAR2 230
#define DEV$M_DFS 16384

#define _RMSFailure(Status) (((Status) & 1) == 0)
#define _RMSSuccess(Status) (((Status) & 1) == 1)
#define _RMSProtectionFailure(Status) ((Status) == RMS$_PRV)
#define _RMSFileNotFound(Status) ((Status) == RMS$_FNF)
#define _RMSLockError(Status) \
    ((Status) == RMS$_DEADLOCK || \
     (Status) == RMS$_ENQ || \
     (Status) == RMS$_EXENQLM || \
     (Status) == RMS$_RLK || \
     (Status) == RMS$_RNL)

#endif /* VMS */             

/*
**  Forward Routine Declarations
*/

_DeclareFunction(static void ExtendFile, (_GridFile file));
_DeclareFunction(static void UnlockFile,
    (_GridFile file, volatile _Boolean raise_exceptions));
_DeclareFunction(static void FlushFile, (_GridFile file));


_GridFile  LwkGFInitializeFile(identifier)
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
    int i;
    _GridFile new;

    /*
    **  Create and initialize the Grid File structure
    */

    new = (_GridFile) _AllocateMem(sizeof(_GridFileInstance));

    new->identifier = _CopyString(identifier);
    new->name = _StringToDDIFString(new->identifier);

    new->bucket_size = LwkGridBucketSize;
    new->cache_size = LwkGridCacheSize;
    new->cursors = (_AnyPtr) 0;
    new->most_recent = (_Bucket) 0;
    new->least_recent = (_Bucket) 0;
    new->locks = (_Lock) 0;
    new->next_bucket = _InitialDataBucket;
    new->version = _FileVersion;
    new->file_sn = 0;
    new->regions_sn = 0;
    new->directory_sn = 0;
    new->file_modified = _False;
    new->regions_modified = _False;
    new->directory_modified = _False;
    new->regions = (_Region) 0;
    new->key_count = 0;
    new->cache_count = 0;
    new->next_id = 1;
    new->next_to_split = 0;
    new->state = lwk_c_transact_commit;

#ifdef VMS
    new->rab = (struct RAB *) 0;
    new->lock = 0;
#else /* !VMS */
    new->file = 0;
#endif /* VMS */

    for (i = 0; i < _MaxKeys; i++)
	new->keys[i] = (_Key) 0;	

    /*
    **  Return the initialized File structure
    */

    return new;
    }


void  LwkGFFreeFile(file)
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
    **  If there was no File, return now.
    */

    if (file == (_GridFile) 0)
	return;

    /*
    **  Free all Buckets
    */

    LwkGBFreeBuckets(file);

    /*
    **  Free the Keys and Regions
    */

    LwkGDFreeDirectory(file);

#ifdef VMS

    /*
    **	Free the FAB, NAM, XAB and RAB blocks if there are any
    */

    if (file->rab != (struct RAB *) 0) {
	struct XABALL *xaball;
	struct XABFHC *xabfhc;
	struct XABPRO *xabpro;

	xaball = file->rab->rab$l_fab->fab$l_xab;
	xabfhc = xaball->xab$l_nxt;
	xabpro = xabfhc->xab$l_nxt;

	_FreeMem(xaball);
	_FreeMem(xabfhc);
	_FreeMem(xabpro);

	_FreeMem(file->rab->rab$l_fab->fab$l_nam);
	_FreeMem(file->rab->rab$l_fab);
	_FreeMem(file->rab);
    }

#endif /* VMS */

    /*
    **  Free the File itself
    */

    _DeleteString(&file->identifier);
    _DeleteDDIFString(&file->name);

    _FreeMem(file);

    return;
    }


_String  LwkGFExpandIdentifier(identifier)
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
    _String expanded_id;
#ifdef VMS
    int status;
    struct FAB fab;
    struct NAM nam;
    char file_name[NAM$C_MAXRSS + 1];

    /*
    **  Allocate and initialize the FAB and NAM with default values
    */

    fab = cc$rms_fab;
    nam = cc$rms_nam;

    /*
    **  Set the appropriate non-default fields
    */

    fab.fab$l_fna = identifier;
    fab.fab$b_fns = strlen(identifier);

    fab.fab$l_dna = _DefaultFileName;
    fab.fab$b_dns = strlen(_DefaultFileName);

    fab.fab$l_nam = &nam;

    nam.nam$l_esa = file_name;
    nam.nam$b_ess = NAM$C_MAXRSS;

    /*
    **  Parse the file
    */

    status = sys$parse(&fab);

    if (_RMSFailure(status)) {
	if (_TraceRead) {
	    int vector[3];

	    vector[0] = 2;
	    vector[1] = fab.fab$l_sts;
	    vector[2] = fab.fab$l_stv;
	    
	    sys$putmsg(vector);
	}
	    
	_Raise(no_such_linkbase);
    }

    /*
    **  Get the Expanded file name
    */

    file_name[nam.nam$b_esl] = _EndOfString;
    expanded_id = _CopyString((_String) file_name);

#else /* !VMS */
    int cwd_len;
    char cwd[256];

    if (!_FullPath(identifier)) {
	if ((_CharPtr) getcwd(cwd, 256) != NULL) {
	    cwd_len = _LengthString(cwd);

	    if (cwd[cwd_len - 1] != _PathDelimiterCharacter) {
		cwd[cwd_len] = _PathDelimiterCharacter;
		cwd[cwd_len + 1] = _EndOfString;
	    }

	    expanded_id = _CopyString((_String) cwd);

	    expanded_id = _ConcatString(expanded_id, identifier);
	}
	else
	    expanded_id = _CopyString(identifier);
    }
    else
	expanded_id = _CopyString(identifier);

#endif /* VMS */

    return expanded_id;
    }


_Boolean  LwkGFOpenFile(file, create)
_GridFile file;
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
    _Boolean created;
    int status;

#ifdef VMS
    int devchar;
    _Boolean dfsdevice;
    int dc_length;
    _ItemList item;
    struct FAB *fab;
    struct NAM *nam;
    struct RAB *rab;
    struct XABALL *xaball;
    struct XABFHC *xabfhc;
    struct XABPRO *xabpro;
    char file_name[NAM$C_MAXRSS + 1];
    struct dsc$descriptor_s d_dev_name;
#else /* !VMS */
    int open_flags;
    int open_mode;
    _CharPtr obsolete_identifier;
    _CharPtr obsolete_extension;
#endif /* VMS */

    if (_TraceRead)
	_Trace("Open -- File %s\n", file->identifier);

#ifdef VMS
    /*
    **  Allocate and initialize the FAB, NAM and RAB with default values
    */

    fab = _AllocateMem(sizeof(struct FAB));
    nam = _AllocateMem(sizeof(struct NAM));
    rab = _AllocateMem(sizeof(struct RAB));
    xaball = _AllocateMem(sizeof(struct XABALL));
    xabfhc = _AllocateMem(sizeof(struct XABFHC));
    xabpro = _AllocateMem(sizeof(struct XABPRO));

    *fab = cc$rms_fab;
    *nam = cc$rms_nam;
    *rab = cc$rms_rab;
    *xaball = cc$rms_xaball;
    *xabfhc = cc$rms_xabfhc;
    *xabpro = cc$rms_xabpro;

    /*
    **  Set the appropriate non-default fields
    */

    fab->fab$l_fna = file->identifier;
    fab->fab$b_fns = strlen(file->identifier);

    fab->fab$l_nam = nam;
    fab->fab$l_xab = xaball;

/*    fab->fab$l_fop = FAB$M_CIF;*/

    fab->fab$l_fop = FAB$M_MXV;

    fab->fab$b_rfm = FAB$C_FIX;
    fab->fab$w_mrs = file->bucket_size;

    if (LwkGridRecordMode) {
	fab->fab$b_fac = FAB$M_GET | FAB$M_PUT | FAB$M_UPD;
	fab->fab$b_shr = FAB$M_SHRGET | FAB$M_SHRPUT | FAB$M_SHRUPD;

	rab->rab$b_rac = RAB$C_KEY;
	rab->rab$b_ksz = 4;

	rab->rab$l_rop = RAB$M_UIF;

	if (LwkGridLock)
	    rab->rab$l_rop |= RAB$M_NLK;
	else
	    rab->rab$l_rop |= RAB$M_REA | RAB$M_WAT | RAB$M_ULK;
    }
    else {
	fab->fab$b_fac = FAB$M_BIO | FAB$M_GET | FAB$M_PUT;
	fab->fab$b_shr = FAB$M_SHRGET | FAB$M_SHRPUT | FAB$M_UPI;
    }

    xaball->xab$l_nxt = xabfhc;

    xabfhc->xab$l_nxt = xabpro;

    nam->nam$l_esa = file_name;
    nam->nam$b_ess = NAM$C_MAXRSS;

    rab->rab$l_fab = fab;
    
    /*
    **  Save RAB pointer in File
    */

    file->rab = rab;

    /*
    ** Parse the file spec (we need the device name)
    */

    status = sys$parse(fab);

    if (_RMSProtectionFailure(status))
	_Raise(db_access_error);

    if (_RMSFileNotFound(status))
	_Raise(no_such_linkbase);

    if (_RMSFailure(status)) {
	if (_TraceRead) {
	    int vector[3];

	    vector[0] = 2;
	    vector[1] = fab->fab$l_sts;
	    vector[2] = fab->fab$l_stv;
	    
	    sys$putmsg(vector);
	}

	if (create)
	    _Raise(db_write_error);
	else
	    _Raise(db_read_error);
    }

    /*
    ** Find out if the device containing the file is a DFS-served device
    ** (if the device is not on a remote node)
    */

    dfsdevice = _False; /* assume it isn't a DFS device */

    if (nam->nam$b_node == 0) { /* If there is no node name involved */

	d_dev_name.dsc$w_length = nam->nam$b_dev;
	d_dev_name.dsc$b_dtype = DSC$K_DTYPE_T;
	d_dev_name.dsc$b_class = DSC$K_CLASS_S;
	d_dev_name.dsc$a_pointer = nam->nam$l_dev;

	item.code = DVI$_DEVCHAR2;
	item.length = sizeof(int);
	item.address = &devchar;
	item.return_length_address = &dc_length;
	item.eol = 0;

	status = sys$getdvi(0, (short) 0, &d_dev_name, &item, 0, 0, 0, 0);

	if (_RMSFailure(status)) {
	    devchar = 0;

	    if (_TraceRead) {
		int vector[3];

		vector[0] = 1;
		vector[1] = status;
		
		sys$putmsg(vector);
	    }
	}

	if ((devchar & DEV$M_DFS) != 0)
	    dfsdevice = _True;
    }


    /*
    **	Unless the file is on a DFS-served device, try to open/create the file
    **	for read/write access.  If it is a DFS-served device, pretend we had a
    **	protection failure so that we will try to open it read-only.
    */

    nam->nam$b_ess = 0;

    nam->nam$l_rsa = file_name;
    nam->nam$b_rss = NAM$C_MAXRSS;

    if (dfsdevice)
	status = RMS$_PRV;
    else {
	if (create)
	    status = sys$create(fab);
	else
	    status = sys$open(fab);
    }

    /*
    ** If there was a read/write access failure, try to open for read-only
    ** access
    */

    if (_RMSProtectionFailure(status)) {
	fab->fab$b_fac &= ~(FAB$M_PUT | FAB$M_UPD);

	if (create)
	    status = sys$create(fab);
	else
	    status = sys$open(fab);
    }

    if (_RMSProtectionFailure(status))
	_Raise(db_access_error);

    if (_RMSFileNotFound(status))
	_Raise(no_such_linkbase);

    if (_RMSFailure(status)) {
	if (_TraceRead) {
	    int vector[3];

	    vector[0] = 2;
	    vector[1] = fab->fab$l_sts;
	    vector[2] = fab->fab$l_stv;
	    
	    sys$putmsg(vector);
	}

	if (create)
	    _Raise(db_write_error);
	else
	    _Raise(db_read_error);
    }

    /*
    ** If the file was created, and NoDelete for owner (deletion protection) is
    ** not already set, set it then close/reopen the file so that it takes
    ** effect.  This is the only way to get all the other protection fields
    ** (system, group, world) set to the proper default values.
    */
    
    if (_RMSSuccess(fab->fab$l_sts) && (create)) {
	created = _True;

        /*
	** For some reason $Create is not filling in the XABPRO fields with
	** what was used to created the file, so use an explicit $Display to
	** get them filled in.
        */
	
	status = sys$display(fab);

	if ((xabpro->xab$v_own & XAB$M_NODEL) == 0) {
	    xabpro->xab$v_own |= XAB$M_NODEL;

	    status = sys$close(fab);

	    if (!_RMSFailure(status))
		status = sys$open(fab);

	    if (_RMSFailure(status)) {
		if (_TraceRead) {
		    int vector[3];

		    vector[0] = 2;
		    vector[1] = fab->fab$l_sts;
		    vector[2] = fab->fab$l_stv;
		    
		    sys$putmsg(vector);
		}

		_Raise(db_write_error);
	    }
	}
    }
    else
	created = _False;

    /*
    ** If we only got read access, remember that
    */

    if ((fab->fab$b_fac & FAB$M_PUT) == 0)
	file->read_only = _True;
    else
	file->read_only = _False;

    /*
    **  Connect the RAB to the FAB
    */

    status = sys$connect(rab);

    if (_RMSFailure(status)) {
	if (_TraceRead) {
	    int vector[3];

	    vector[0] = 2;
	    vector[1] = rab->rab$l_sts;
	    vector[2] = rab->rab$l_stv;
	    
	    sys$putmsg(vector);
	}

	_Raise(db_read_error);
    }

    /*
    **  Set the File Identifier to the actual file name
    */

    _DeleteString(&file->identifier);

    file_name[nam->nam$b_rsl] = _EndOfString;
    file->identifier = _CopyString((_String) file_name);

    nam->nam$l_rsa = 0;
    nam->nam$b_rss = 0;

    if (_TraceWrite)
	_Trace("    Size: %ld; Allocated: Fab: %ld, Xab: %ld\n",
	    xabfhc->xab$l_ebk - 1, fab->fab$l_alq, xaball->xab$l_alq);

    /*
    ** Remember the actual Bucket size
    */

    file->bucket_size = fab->fab$w_mrs;

#else /* !VMS */

    /*
    **  Try to open the File for read/write access
    */

    created = _False;

#ifdef MSDOS

    open_flags = O_RDWR | O_BINARY;

    file->file = sopen((_CharPtr) file->identifier, open_flags,
	SH_DENYNO, 0);

#else /* !VMS && !MSDOS */

    /*
    ** If you have been asked to create the file and the file already exists,
    ** rename the existing file and open a new one.
    */

    if (create) {

        /*
	** Search if the file alreday exists.
	*/
	
	open_flags = O_CREAT | O_EXCL | O_RDWR;

	open_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
	
	file->file = open((_CharPtr) file->identifier, open_flags, open_mode);

	if (file->file == -1 && errno == EEXIST) {

	    /*
	    ** The file exists, so renames it.
	    */
	    
	    obsolete_extension = ".obsolete";

	    obsolete_identifier = (_CharPtr)
				  malloc(strlen((_CharPtr) file->identifier)
				  + sizeof(char)
				  + strlen(obsolete_extension)
				  + sizeof(char));
				  
	    strcpy(obsolete_identifier, (_CharPtr) file->identifier);

	    obsolete_identifier = strcat(obsolete_identifier,
					 obsolete_extension);

	    status = rename((_CharPtr) file->identifier, obsolete_identifier);

	    free(obsolete_identifier);
	    
	    if (status != 0)
		_Raise(db_write_error);

	    /*
	    ** Create a new one.
	    */

	    open_flags = O_RDWR | O_CREAT;

	    open_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

	    file->file = open((_CharPtr) file->identifier, open_flags,
		open_mode);

	}
	
	created = _True;
    }

    else { /* try to open a file */
	
	open_flags = O_RDWR;

	file->file = open((_CharPtr) file->identifier, open_flags, 0);

#endif /* MSDOS */

	/*
	** If there was a read/write access failure, try to open for read-only
	** access
	*/

	if (file->file == -1 && errno == EACCES) {

#ifdef MSDOS

	    open_flags = O_RDONLY | O_BINARY;

	    file->file = sopen((_CharPtr) file->identifier, open_flags,
		SH_DENYNO, 0);

#else /* !VMS && !MSDOS */

	    open_flags = O_RDONLY;

	    file->file = open((_CharPtr) file->identifier, open_flags, 0);

#endif /* MSDOS */

	    if (file->file == -1 && errno == EACCES)
		_Raise(db_access_error);
	}
		
	/*
	**	If the open failed, and the error was not FileNotFound or the caller
	**	asked us not to try to create a new file, raise the appropriate
	**	exception.  Otherwise, try to create a new File.
	*/

	if (file->file == -1) {
	    if (errno != ENOENT)
		_Raise(db_read_error);

	    if (!create)
		_Raise(no_such_linkbase);

#ifdef MSDOS

	    open_flags = O_RDWR | O_CREAT | O_BINARY;

	    open_mode = S_IREAD | S_IWRITE;

	    file->file = sopen((_CharPtr) file->identifier, open_flags,
		SH_DENYNO, open_mode);

#else /* !VMS && !MSDOS */

	    open_flags = O_RDWR | O_CREAT;

	    open_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

	    file->file = open((_CharPtr) file->identifier, open_flags,
		open_mode);

#endif /* MSDOS */

	    if (file->file == -1) {
		if (errno == EACCES)
		    _Raise(db_access_error);
		else
		    _Raise(db_write_error);
	    }

	    created = _True;
	}

    }
    /*
    ** If we only got read access, remember that
    */

    if ((open_flags & O_RDONLY) == 0)
	file->read_only = _False;
    else {
	file->read_only = _True;

	if (_TraceRead)
	    _Trace("    opened read-only\n");
    }

#endif /* VMS */

    /*
    **  Inform the caller if we created a new file
    */

    return created;
    }


void  LwkGFCloseFile(file)
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
    int status;
#ifdef VMS
    struct FAB *fab;
    struct RAB *rab;
    struct NAM *nam;
    struct XABALL *xaball;
    struct XABFHC *xabfhc;
    struct XABFHC *xabpro;
#endif /* VMS */

    if (_TraceRead)
	_Trace("Close -- File %s\n", file->identifier);

#ifdef VMS

    rab = file->rab;
    fab = rab->rab$l_fab;
    nam = fab->fab$l_nam;
    xaball = fab->fab$l_xab;
    xabfhc = xaball->xab$l_nxt;
    xabpro = xabfhc->xab$l_nxt;

    if (_TraceWrite)
	_Trace("    Size: %ld; Allocated: Fab: %ld, Xab: %ld\n",
	    xabfhc->xab$l_ebk - 1, fab->fab$l_alq, xaball->xab$l_alq);

    /*
    **	Close the file and free the FAB, NAM, XAB and RAB blocks
    */

    status = sys$close(fab);

    _FreeMem(nam);
    _FreeMem(fab);
    _FreeMem(rab);

    _FreeMem(xaball);
    _FreeMem(xabfhc);
    _FreeMem(xabpro);

    file->rab = (struct RAB *) 0;

    if (_RMSFailure(status)) {
	if (_TraceRead) {
	    int vector[3];

	    vector[0] = 2;
	    vector[1] = fab->fab$l_sts;
	    vector[2] = fab->fab$l_stv;
	    
	    sys$putmsg(vector);
	}

	_Raise(db_write_error);
    }

#else /* !VMS */

    status = close(file->file);

    if (status != 0)
	_Raise(db_write_error);

    file->file = -1;

#endif /* VMS */

    return;
    }


void  LwkGFDeleteFile(identifier)
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
    int status;
#ifdef VMS
    struct FAB fab;
    struct XABPRO xabpro;
#endif /* VMS */

    if (_TraceWrite)
	_Trace("Delete -- File %s\n", identifier);

#ifdef VMS
    /*
    **  Initialize the FAB and XAB with default values
    */

    fab = cc$rms_fab;
    xabpro = cc$rms_xabpro;

    /*
    **  Set the appropriate non-default fields
    */

    fab.fab$l_fna = identifier;
    fab.fab$b_fns = strlen(identifier);

    fab.fab$b_fac = FAB$M_PUT | FAB$M_DEL;

    fab.fab$l_xab = &xabpro;

    /*
    **  Open the file
    */

    status = sys$open(&fab);

    if (_RMSFileNotFound(status))
	_Raise(no_such_linkbase);

    if (_RMSFailure(status)) {
	if (_TraceWrite) {
	    int vector[3];

	    vector[0] = 2;
	    vector[1] = fab.fab$l_sts;
	    vector[2] = fab.fab$l_stv;
	    
	    sys$putmsg(vector);
	}

	_Raise(db_write_error);
    }

    /*
    ** Make sure we have premission to delete it (we delete protected the file
    ** from the owner when we created it!).
    */

    if ((xabpro.xab$v_own & XAB$M_NODEL) != 0) {
	xabpro.xab$v_own &= ~XAB$M_NODEL;

	status = sys$close(&fab);

	if (!_RMSFailure(status)) {
	    status = sys$open(&fab);

	    if (_RMSFailure(status)) {
		if (_TraceWrite) {
		    int vector[3];

		    vector[0] = 2;
		    vector[1] = fab.fab$l_sts;
		    vector[2] = fab.fab$l_stv;
		    
		    sys$putmsg(vector);
		}

		_Raise(db_write_error);
	    }
	}
    }

    /*
    ** Close the file requesting that it be deleted.
    */

    fab.fab$l_fop = FAB$M_DLT;

    status = sys$close(&fab);

    if (_RMSFailure(status)) {
	if (_TraceWrite) {
	    int vector[3];

	    vector[0] = 2;
	    vector[1] = fab.fab$l_sts;
	    vector[2] = fab.fab$l_stv;
	    
	    sys$putmsg(vector);
	}

	_Raise(db_write_error);
    }

#else /* !VMS */

    status = unlink((_CharPtr) identifier);

    if (status != 0) {
	if (errno == ENOENT)
	    _Raise(no_such_linkbase);
	else
	    _Raise(db_write_error);
    }

#endif /* VMS */

    return;
    }


void  LwkGFRenameFile(file, identifier)
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
    int status;
#ifdef VMS
    struct FAB ofab;
    struct FAB nfab;
    struct NAM nam;
    struct XABPRO xabpro;
    char file_name[NAM$C_MAXRSS + 1];
#endif /* VMS */

    if (_TraceWrite)
	_Trace("Rename -- File %s\n", file->identifier);

#ifdef VMS

    /*
    **  Initialize the FAB's, NAM and XAB with default values
    */

    ofab = cc$rms_fab;
    nfab = cc$rms_fab;
    nam = cc$rms_nam;
    xabpro = cc$rms_xabpro;

    /*
    **  Set the appropriate non-default fields
    */

    ofab.fab$l_fna = file->identifier;
    ofab.fab$b_fns = strlen(file->identifier);

    ofab.fab$b_fac = FAB$M_PUT | FAB$M_DEL;
    ofab.fab$b_shr = FAB$M_SHRPUT;

    ofab.fab$l_xab = &xabpro;

    nfab.fab$l_fna = identifier;
    nfab.fab$b_fns = strlen(identifier);

    nfab.fab$b_fac = FAB$M_PUT;
    nfab.fab$b_shr = FAB$M_SHRPUT;

    nfab.fab$l_nam = &nam;

    nam.nam$l_rsa = file_name;
    nam.nam$b_rss = NAM$C_MAXRSS;

    /*
    ** Open the file
    */

    status = sys$open(&ofab);

    if (_RMSFileNotFound(status))
	_Raise(no_such_linkbase);

    if (_RMSFailure(status)) {
	if (_TraceWrite) {
	    int vector[3];

	    vector[0] = 2;
	    vector[1] = ofab.fab$l_sts;
	    vector[2] = ofab.fab$l_stv;
	    
	    sys$putmsg(vector);
	}

	_Raise(db_write_error);
    }

    /*
    ** Make sure we have premission to rename it (we delete/rename protected
    ** the file from the owner when we created it!).
    */

    if ((xabpro.xab$v_own & XAB$M_NODEL) != 0)
	xabpro.xab$v_own &= ~XAB$M_NODEL;

    status = sys$close(&ofab);

    /*
    **  Rename the file
    */

    status = sys$rename(&ofab, (void (*)()) 0, (void (*)()) 0, &nfab);

    if (_RMSFailure(status)) {
	if (_TraceWrite) {
	    int vector[3];

	    vector[0] = 2;
	    vector[1] = ofab.fab$l_sts;
	    vector[2] = ofab.fab$l_stv;
	    
	    sys$putmsg(vector);
	}

	_Raise(db_write_error);
    }

    /*
    **  Reset delete protection for owner.
    */

    nfab.fab$l_xab = &xabpro;

    status = sys$open(&nfab);

    if (_RMSFailure(status)) {
	if (_TraceWrite) {
	    int vector[3];

	    vector[0] = 2;
	    vector[1] = nfab.fab$l_sts;
	    vector[2] = nfab.fab$l_stv;
	    
	    sys$putmsg(vector);
	}

	_Raise(db_write_error);
    }

    xabpro.xab$v_own |= XAB$M_NODEL;

    status = sys$close(&nfab);

    /*
    **  Reset the File identifier to the actual file name
    */

    _DeleteString(&file->identifier);

    file_name[nam.nam$b_rsl] = _EndOfString;
    file->identifier = _CopyString((_String) file_name);

#else /* !VMS */

    status = rename((_CharPtr) file->identifier, (_CharPtr) identifier);

    if (status != 0)
	_Raise(db_write_error);

    /*
    **  Reset the File identifier to the actual file name
    */

    _DeleteString(&file->identifier);

    file->identifier = _CopyString(identifier);

#endif /* VMS */

    return;
    }


void  LwkGFRefreshFile(file)
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
#ifdef VMS
    int status;
    struct FAB *fab;
    struct RAB *rab;
    struct XABALL *xaball;
    struct XABFHC *xabfhc;

    /*
    ** Not required if we're in Record Mode
    */

    if (LwkGridRecordMode)
	return;

    rab = file->rab;
    fab = rab->rab$l_fab;
    xaball = fab->fab$l_xab;
    xabfhc = xaball->xab$l_nxt;

    status = sys$display(fab);

    if (_RMSFailure(status)) {
	if (_TraceRead) {
	    int vector[3];

	    vector[0] = 2;
	    vector[1] = fab->fab$l_sts;
	    vector[2] = fab->fab$l_stv;
	    
	    sys$putmsg(vector);
	}

	_Raise(db_read_error);
    }

    if (_TraceRead) {
	_Trace("Refresh -- File %s\n", file->identifier);

	_Trace("    Size: %ld; Allocated Fab: %ld, Xab: %ld\n",
	    xabfhc->xab$l_ebk - 1, fab->fab$l_alq, xaball->xab$l_alq);

    }

#else /* !VMS */

    /*
    **  No need to do anything here on Ultrix or MSDOS
    */

#endif /* VMS */

    return;
    }


void  LwkGFReadFile(file, bucket)
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
    int status;
#ifdef VMS
    int record;

    /*
    **  Position the file
    */

    if (LwkGridRecordMode) {
	record = bucket->number + 1;
	file->rab->rab$l_kbf = &record;
    }
    else
	file->rab->rab$l_bkt =
	    ((bucket->number * file->bucket_size) / BUFSIZ) + 1;

    /*
    ** If we are not doing explicit locking, ask for an implicit read lock
    */

    if (!LwkGridLock && LwkGridRecordMode)
	file->rab->rab$l_rop &= ~RAB$M_RLK;

    /*
    **  Read the data
    */

    file->rab->rab$l_ubf = bucket->buffer;
    file->rab->rab$w_usz = bucket->size;

    if (LwkGridRecordMode)
	status = sys$get(file->rab);
    else
	status = sys$read(file->rab);

    if (_RMSFailure(status)) {
	if (_TraceRead) {
	    int vector[3];

	    vector[0] = 2;
	    vector[1] = file->rab->rab$l_sts;
	    vector[2] = file->rab->rab$l_stv;
	    
	    sys$putmsg(vector);
	}

	if (_RMSLockError(status))
	    _Raise(db_lock_error);
	else
	    _Raise(db_read_error);
    }

#else /* !VMS */

    /*
    **  Position the file
    */

    status = lseek(file->file, bucket->number * file->bucket_size, SEEK_SET);

    if (status == -1)
	_Raise(db_read_error);

    /*
    **  Read the data
    */

    status = read(file->file, bucket->buffer, bucket->size);

    if (status == -1)
	_Raise(db_read_error);

#endif /* VMS */

    return;
    }


void  LwkGFWriteFile(file, bucket)
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
    int status;
#ifdef VMS
    int record;

    /*
    **  Position the file
    */

    if (LwkGridRecordMode) {
	record = bucket->number + 1;
	file->rab->rab$l_kbf = &record;
    }
    else
	file->rab->rab$l_bkt =
	    ((bucket->number * file->bucket_size) / BUFSIZ) + 1;

    /*
    ** If we are not doing explicit locking, ask for an implicit write lock
    */

    if (!LwkGridLock && LwkGridRecordMode)
	file->rab->rab$l_rop |= RAB$M_RLK;

    /*
    **  Write the data
    */

    file->rab->rab$l_rbf = bucket->buffer;
    file->rab->rab$w_rsz = bucket->size;

    if (LwkGridRecordMode)
	status = sys$put(file->rab);
    else
	status = sys$write(file->rab);

    if (_RMSFailure(status)) {
	if (_TraceWrite) {
	    int vector[3];

	    vector[0] = 2;
	    vector[1] = file->rab->rab$l_sts;
	    vector[2] = file->rab->rab$l_stv;
	    
	    sys$putmsg(vector);
	}

	if (_RMSLockError(status))
	    _Raise(db_lock_error);
	else
	    _Raise(db_write_error);
    }

#else /* !VMS */

    /*
    **  Position the file
    */

    status = lseek(file->file, bucket->number * file->bucket_size, SEEK_SET);

    if (status == -1)
	_Raise(db_write_error);

    /*
    **  Write the data
    */

    status = write(file->file, bucket->buffer, bucket->size);

    if (status == -1)
	_Raise(db_write_error);

#endif /* VMS */

    return;
    }


void  LwkGFStartTransaction(file, state)
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
    ** Disallow read/write transactions on read-only files
    */

    if (state == lwk_c_transact_read_write && file->read_only)
	_Raise(db_access_error);

    if (_TraceTransact) {
	_CharPtr type;

	switch (state) {
	    case lwk_c_transact_read :
		type = "RD";
		break;

	    case lwk_c_transact_read_write :
		type = "RD/WR";
		break;

	    default :
		type = "unknown";
		break;
	}

	_Trace("Transact %s -- File %s\n", type, file->identifier);
    }

#ifdef MSDOS

    /*
    ** If necessary, re-Open the file.
    */
    
    if (file->file == -1)
	LwkGFOpenFile(file, _False);

#endif /* MSDOS */

    /*
    **  Take out the appropriate locks
    */

    LwkGLLockFile(file, state);

    /*
    **  Remember current transaction state
    */

    file->state = state;

    return;
    }


void  LwkGFCommitFile(file)
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
    if (_TraceTransact)
	_Trace("Commit -- File %s\n", file->identifier);

    /*
    **  Write the Directory if it has changed
    */

    LwkGDWriteDirectory(file);

    /*
    **  Write back the modified Buckets and trim Bucket cache
    */

    LwkGBFlushBuckets(file);

    /*
    **  Make sure the disk copy of all Buckets is up-to-date
    */

    FlushFile(file);

    /*
    **  Free File locks -- errors are significant
    */

    UnlockFile(file, _True);

    /*
    **  Set the Transaction state
    */

    file->state = lwk_c_transact_commit;

    return;
    }


void  LwkGFRollbackFile(file, raise_exceptions)
_GridFile file;
 _Boolean raise_exceptions;

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
    **	If no File, return now (caller might be trying to recover from an Open
    **	failure).
    */

    if (file == (_GridFile) 0)
	return;

    if (_TraceTransact)
	_Trace("Rollback -- File %s\n", file->identifier);

    /*
    **  Reset the File serial numbers so that we will re-read the Directory
    **	information.
    */

    file->file_sn = 0;
    file->regions_sn = 0;
    file->directory_sn = 0;

    /*
    **  Free File locks -- raise exceptions as requested
    */

    UnlockFile(file, raise_exceptions);

    /*
    **  Set the Transaction state
    */

    file->state = lwk_c_transact_commit;

    return;
    }


void  LwkGFAllocateBucket(file, number)
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
    if (_TraceWrite)
	_Trace("Allocate Bucket %ld -- File %s\n", number, file->identifier);

    /*
    **  Extend the File if necessary
    */

    if ((number % _ExtensionBuckets) == 0)
	ExtendFile(file);

    /*
    **  Set read lock on the Bucket
    */

    LwkGLLockBucket(file, number, lwk_c_transact_read);

    return;
    }


static void  ExtendFile(file)
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
#ifdef VMS
    int status;
    _Integer extension;
    struct FAB *fab;
    struct RAB *rab;
    struct XABALL *xaball;
    struct XABFHC *xabfhc;

    /*
    ** Not required if we're in Record Mode
    */

    if (LwkGridRecordMode)
	return;

    /*
    **  Extend the File
    */

    rab = file->rab;
    fab = rab->rab$l_fab;
    xaball = fab->fab$l_xab;
    xabfhc = xaball->xab$l_nxt;

    extension = _ExtensionBlocks - (fab->fab$l_alq % _ExtensionBlocks);

    if (_TraceWrite) {
	_Trace("Extend -- File %s\n", file->identifier);

	_Trace("    Size: %ld; Allocated Fab: %ld, Xab: %ld -- Extend: %ld\n",
	    xabfhc->xab$l_ebk - 1, fab->fab$l_alq, xaball->xab$l_alq,
	    extension);
    }

    xaball->xab$l_alq = extension;

    status = sys$extend(fab);

    if (_RMSFailure(status)) {
	if (_TraceWrite) {
	    int vector[3];

	    vector[0] = 2;
	    vector[1] = fab->fab$l_sts;
	    vector[2] = fab->fab$l_stv;
	    
	    sys$putmsg(vector);
	}

	_Raise(db_write_error);
    }

    fab->fab$l_alq += xaball->xab$l_alq;

    if (_TraceWrite)
	_Trace("    New size: %ld; Allocated Fab: %ld, Xab: %ld\n",
	     xabfhc->xab$l_ebk - 1, fab->fab$l_alq, xaball->xab$l_alq);

#else /* !VMS */

    /*
    **  No need to do anything here on Ultrix or MSDOS
    */

#endif /* VMS */

    return;
    }


static void  UnlockFile(file, raise_exceptions)
_GridFile file;
 volatile _Boolean raise_exceptions;

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

    /*
    **  Free either the explicit or the implicit Locks -- raise exceptions as
    **	requested
    */

    if (LwkGridLock)
	LwkGLUnlockFile(file, raise_exceptions);
    else {
	if (_TraceLock)
	    _Trace("Free -- File %s\n", file->identifier);

	status = sys$free(file->rab);

	if (raise_exceptions && _RMSFailure(status)) {
	    if (_TraceLock) {
		int vector[3];

		vector[0] = 2;
		vector[1] = file->rab->rab$l_sts;
		vector[2] = file->rab->rab$l_stv;
		
		sys$putmsg(vector);
	    }

	    _Raise(db_lock_error);
	}
    }

#else /* !VMS */

    /*
    **  Free any explicit File locks -- raise exceptions as requested
    */

    LwkGLUnlockFile(file, raise_exceptions);

#ifdef MSDOS

    /*
    ** Close the file. -- we will re-Open it on the next File Start
    ** Transaction.
    */
    
    _StartExceptionBlock

    LwkGFCloseFile(file);

    _Exceptions
	_WhenOthers
	    if (raise_exceptions)
		_Reraise;
    _EndExceptionBlock

#endif /* MSDOS */
#endif /* VMS */

    return;
    }


static void  FlushFile(file)
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
    int status;

    /*
    **  Make sure disk is up-to-date
    */

    if (_TraceWrite)
	_Trace("Flush -- File %s\n", file->identifier);

#ifdef VMS

    status = sys$flush(file->rab);

    if (_RMSFailure(status)) {
	if (_TraceWrite) {
	    int vector[3];

	    vector[0] = 2;
	    vector[1] = file->rab->rab$l_sts;
	    vector[2] = file->rab->rab$l_stv;
	    
	    sys$putmsg(vector);
	}

	_Raise(db_write_error);
    }

    LwkGFRefreshFile(file);

#elif MSDOS

    /*
    ** Nothing to do here -- we close the file on File Commit/Rollback and
    ** re-Open it in File Start Transaction
    */
    
#else /* !VMS && !MSDOS */

    status = fsync(file->file);

    if (status == -1)
	_Raise(db_write_error);

#endif /* VMS */

    return;
    }
