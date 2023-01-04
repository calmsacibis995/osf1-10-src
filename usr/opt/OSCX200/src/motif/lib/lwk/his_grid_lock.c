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
**  Creation Date: 17-Nov-89
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

#define _MaxLockNameLength 31
#define _LockNamePrefix "LwkOp"

/*
**  Forward Routine Declarations
*/

_DeclareFunction(static void FreeLock, (_GridFile file, _Lock lock));
_DeclareFunction(static _Lock FindLock, (_GridFile file, _BucketNumber number));
_DeclareFunction(static _Lock CreateLock, (_BucketNumber number));
_DeclareFunction(static void Lock,
    (_GridFile file, _Lock lock, _Transaction state));

/*
** Static Data Definitions
*/

static int RootLockId = 0;
static _Boolean HaveRootLockId = _False;


void  LwkGLLockFile(file, state)
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
#ifdef VMS
    int len;
    int status;
    int lsb[2];
    char lock_name[_MaxLockNameLength];
    struct dsc$descriptor_s d_lock_name;
#endif /* VMS */

    /*
    **  Return now if locking has been disabled
    */

    if (!LwkGridLock)
	return;

    if (_TraceLock) {
	_Trace("Lock File %s\n", file->identifier);
    }

#ifdef VMS
    /*
    **  Can't lock remote files
    */

    if ((file->rab->rab$l_fab->fab$l_nam->nam$l_fnb & NAM$M_NODE) != 0)
	_Raise(db_lock_error);

    /*
    ** If we don't have one yet, get the Cluster-wide root lock for this
    ** process.  All other locks will be sub-locks of this one.
    */
    
    _BeginCriticalSection

    if (!HaveRootLockId) {
	d_lock_name.dsc$w_length = strlen("LwkOpROOT");
	d_lock_name.dsc$b_dtype = DSC$K_DTYPE_T;
	d_lock_name.dsc$b_class = DSC$K_CLASS_S;
	d_lock_name.dsc$a_pointer = "LwkOpROOT";

	if (_TraceLock)
	    _Trace("Request Root Lock -- File %s\n", file->identifier);

	status = sys$enqw(0, LCK$K_NLMODE, lsb, LCK$M_SYSTEM, &d_lock_name,
	    0, 0, 0, 0, 0, 0);

	if ((status & 1) == 0)
	    _Raise(db_lock_error);

	if (_TraceLock)
	    _Trace("Root Lock granted -- File %s\n", file->identifier);

	HaveRootLockId = _True;
    }

    _EndCriticalSection

    /*
    **  Construct the File-level lock name from a private prefix, the unique
    **	device identifier, and the unique file identifier.
    */

    if (strlen(_LockNamePrefix) > _MaxLockNameLength)
	_Raise(db_lock_error);

    _CopyMem(_LockNamePrefix, lock_name, strlen(_LockNamePrefix));

    len = strlen(_LockNamePrefix);

    if (len + NAM$C_DVI > _MaxLockNameLength)
	_Raise(db_lock_error);

    _CopyMem(file->rab->rab$l_fab->fab$l_nam->nam$t_dvi, &lock_name[len],
	NAM$C_DVI);

    len += NAM$C_DVI;

    if (len + (3 * sizeof(short)) > _MaxLockNameLength)
	_Raise(db_lock_error);

    _CopyMem(file->rab->rab$l_fab->fab$l_nam->nam$w_fid, &lock_name[len],
	(3 * sizeof(short)));

    len += (3 * sizeof(short));

    d_lock_name.dsc$w_length = len;
    d_lock_name.dsc$b_dtype = DSC$K_DTYPE_T;
    d_lock_name.dsc$b_class = DSC$K_CLASS_S;
    d_lock_name.dsc$a_pointer = lock_name;

    /*
    **	Create a null lock on this File which can be used as the parent of any
    **	subsequent Bucket locks.
    */

    status = sys$enqw(0, LCK$K_NLMODE, lsb, 0, &d_lock_name, RootLockId,
	0, 0, 0, 0, 0);

    if ((status & 1) == 0)
	_Raise(db_lock_error);

    /*
    **  Save the Lock Id for later use
    */

    file->lock = lsb[1];

#elif MSDOS

    /*
    **  No need to do anything here on MS-DOS
    */

#else

    /*
    **  No need to do anything here on Ultrix
    */

#endif /* VMS */

    return;
    }


void  LwkGLUnlockFile(file, raise_exceptions)
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
    int status;
#ifdef VMS
#elif MSDOS
#else
    struct flock flck;
#endif

    /*
    **  Return now if locking has been disabled
    */

    if (!LwkGridLock)
	return;

    if (_TraceLock)
	_Trace("Unlock File %s\n", file->identifier);

#ifdef VMS

    if (file->lock != 0) {
	/*
	**  Release any Bucket locks
	*/

	status = sys$deq(file->lock, 0, 0, LCK$M_DEQALL);

	if (raise_exceptions && ((status & 1) == 0))
	    _Raise(db_lock_error);

	/*
	**  Release File lock
	*/

	status = sys$deq(file->lock, 0, 0, 0);

	if (raise_exceptions && ((status & 1) == 0))
	    _Raise(db_lock_error);

	file->lock = 0;
    }

#elif MSDOS

    /*
    ** On MS-DOS we have to do this by unlocking the locks on a one-for-one
    ** basis -- so this function is done in FreeLock.
    */
    
#else

    /*
    **  Release all locks on the File
    */

    flck.l_type = F_UNLCK;
    flck.l_whence = 0;
    flck.l_start = 0;
    flck.l_len = 0;

    status = fcntl(file->file, F_SETLKW, &flck);

    if (raise_exceptions && status == -1)
	_Raise(db_lock_error);

#endif /* VMS */

    /*
    **  Free Lock tree
    */

    FreeLock(file, file->locks);
    file->locks = (_Lock) 0;

    return;
    }


void  LwkGLLockBucket(file, number, state)
_GridFile file;
 _BucketNumber number;
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
    _Lock lock;

    /*
    **	Verify that the current File transaction state is compatible with the
    **	requested locking mode.
    */

    switch (state) {
	case lwk_c_transact_read :
	    if (!(file->state == lwk_c_transact_read
		    || file->state == lwk_c_transact_read_write))
		_Raise(unexpected_error);

	    break;

	case lwk_c_transact_read_write :
	    if (!(file->state == lwk_c_transact_read_write))
		_Raise(unexpected_error);

	    break;

	default :
	    _Raise(unexpected_error);
    }

    /*
    **  Return now if locking has been disabled
    */

    if (!LwkGridLock)
	return;

    /*
    **  Find/Create the associated Lock
    */

    lock = FindLock(file, number);

    /*
    **  If necessary, request the Lock
    */

    switch (state) {
	case lwk_c_transact_read :
	    if (!(lock->state == lwk_c_transact_read
		    || lock->state == lwk_c_transact_read_write))
		Lock(file, lock, state);

	    break;

	case lwk_c_transact_read_write :
	    if (!(lock->state == lwk_c_transact_read_write))
		Lock(file, lock, state);

	    break;
    }


    return;
    }


static void  FreeLock(file, lock)
_GridFile file;
 _Lock lock;

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
    **	Free the left sub-tree of locks, then the right, and finally, this
    **	Lock.
    */

#ifdef MSDOS

    if (lock->state != lwk_c_transact_commit) {
	int status;

	status = lseek(file->file, lock->number * file->bucket_size, SEEK_SET);

	if (status == -1)
	    _Raise(db_lock_error);

	status = locking(file->file, LK_UNLCK, file->bucket_size);

	if (status == -1)
	    _Raise(db_lock_error);

	lock->state = lwk_c_transact_commit;
    }

#endif /* MSDOS */

    if (lock != (_Lock) 0) {
	FreeLock(file, lock->lower);
	FreeLock(file, lock->higher);
	_FreeMem(lock);
    }
    
    return;
    }


static _Lock  FindLock(file, number)
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
    _Lock lock;

    /*
    **	Search the Lock tree for an existing Lock.  If not found, create a new
    **	one and link it into the Lock tree.
    */

    lock = file->locks;

    if (lock == (_Lock) 0) {
	lock = CreateLock(number);
	file->locks = lock;
    }
    else {
	while (_True) {
	    if (number == lock->number)
		break;

	    if (number < lock->number) {
		if (lock->lower != (_Lock) 0)
		    lock = lock->lower;
		else {
		    lock->lower = CreateLock(number);
		    lock = lock->lower;
		    break;
		}
	    }
	    else {
		if (lock->higher != (_Lock) 0)
		    lock = lock->higher;
		else {
		    lock->higher = CreateLock(number);
		    lock = lock->higher;
		    break;
		}
	    }
	}
    }

    return lock;
    }


static _Lock  CreateLock(number)
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
    _Lock lock;

    /*
    **  Allocate and initialize a new Lock
    */

    lock = (_Lock) _AllocateMem(sizeof(_LockInstance));

    lock->lower = (_Lock) 0;
    lock->higher = (_Lock) 0;
    lock->number = number;
    lock->state = lwk_c_transact_commit;

    return lock;
    }


static void  Lock(file, lock, state)
_GridFile file;
 _Lock lock;
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
    int status;
    _Boolean upgrade;
#ifdef VMS
    int len;
    int lsb[2];
    char lock_name[_MaxLockNameLength];
    struct dsc$descriptor_s d_lock_name;
#elif MSDOS
    int mode;
#else
    struct flock flck;
#endif /* VMS */

    if (_TraceLock) {
	_CharPtr type;

	switch (state) {
	    case lwk_c_transact_read :
		type = "RD";
		break;

	    case lwk_c_transact_read_write :
		type = "RD/WR";
		break;
	}

	_Trace("Request %s Lock -- Bucket %ld File %s\n", type, lock->number,
	    file->identifier);
    }

    upgrade = _False;

#ifdef VMS
    /*
    **	If we will need one, construct a lock name from a private prefix, and
    **	the Lock number. (The only case where we don't need one is if we are
    **	converting an existing read lock to a read/write lock).
    */

    if (!(state == lwk_c_transact_read_write
	    && lock->state == lwk_c_transact_read)) {
	if (strlen(_LockNamePrefix) > _MaxLockNameLength)
	    _Raise(db_lock_error);

	_CopyMem(_LockNamePrefix, lock_name, strlen(_LockNamePrefix));

	len = strlen(_LockNamePrefix);

	if (len + sizeof(_BucketNumber) > _MaxLockNameLength)
	    _Raise(db_lock_error);

	_CopyMem(&lock->number, &lock_name[len], sizeof(_BucketNumber));

	len += sizeof(_BucketNumber);

	d_lock_name.dsc$w_length = len;
	d_lock_name.dsc$b_dtype = DSC$K_DTYPE_T;
	d_lock_name.dsc$b_class = DSC$K_CLASS_S;
	d_lock_name.dsc$a_pointer = lock_name;
    }

    /*
    **	Now, either wait for 1) a read lock, 2) the upgrade of an existing
    **	read lock to a read/write lock, or 3) a read/write lock.
    */

    switch (state) {
	case lwk_c_transact_read :
	    status = sys$enqw(0, LCK$K_PRMODE, lsb, 0, &d_lock_name,
		file->lock, 0, 0, 0, 0, 0);
	    break;

	case lwk_c_transact_read_write :
	    if (lock->state == lwk_c_transact_read) {
		lsb[0] = 0;
		lsb[1] = lock->id;

		status = sys$enqw(0, LCK$K_EXMODE, lsb, LCK$M_CONVERT,
		    0, 0, 0, 0, 0, 0, 0);

		upgrade = _True;
	    }
	    else {
		status = sys$enqw(0, LCK$K_EXMODE, lsb, 0, &d_lock_name,
		    file->lock, 0, 0, 0, 0, 0);
	    }

	    break;
    }

    if ((status & 1) == 0)
	_Raise(db_lock_error);

    /*
    **  Save the Lock Id in case we need to upgrade it later
    */

    lock->id = lsb[1];

#elif MSDOS

    switch (state) {
	case lwk_c_transact_read :
	    mode = LK_LOCK;
	    break;

	case lwk_c_transact_read_write :
	    mode = LK_RLCK;

	    if (lock->state == lwk_c_transact_read)
		upgrade = _True;

	    break;
    }

    status = lseek(file->file, lock->number * file->bucket_size,
	SEEK_SET);

    if (status == -1)
	_Raise(db_lock_error);

    status = locking(file->file, mode, file->bucket_size);

    if (status == -1)
	_Raise(db_lock_error);

#else

    /*
    **  Request the Lock in the specified mode
    */

    switch (state) {
	case lwk_c_transact_read :
	    flck.l_type = F_RDLCK;
	    break;

	case lwk_c_transact_read_write :
	    if (lock->state == lwk_c_transact_read)
		upgrade = _True;

	    flck.l_type = F_WRLCK;

	    break;
    }

    flck.l_whence = 0;
    flck.l_start = lock->number * file->bucket_size;
    flck.l_len = file->bucket_size;

    status = fcntl(file->file, F_SETLKW, &flck);

    if (status == -1)
	_Raise(db_lock_error);

#endif /* VMS */

    if (_TraceLock) {
	if (upgrade)
	    _Trace("Lock upgraded -- File %s\n", file->identifier);
	else
	    _Trace("Lock granted -- File %s\n", file->identifier);
    }

    /*
    **  Remember the Lock state
    */

    lock->state = state;

    return;
    }
