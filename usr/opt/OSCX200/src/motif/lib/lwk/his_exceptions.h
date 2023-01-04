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
**	Header file for Exception Handling Service
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

#include <setjmp.h>

/*
**  Type Definitions
*/

typedef int _Exception;
typedef jmp_buf _ExcContext;

/*
**  Macro Definitions
*/

#define _StartExceptionBlock \
    { /* Start of Exception Handling Block */ \
	_Status volatile _code_; \
	LwkExcBeginException(); \
	_code_ = (_Status) setjmp(LwkExcContexts[LwkExcCurrentContext]); \
	if (_code_ == _StatusCode(unknown)) {  /* Normal return from setjmp */ \

#define _Exceptions \
	    LwkExcLeaveExceptionNormally(); \
	} \
	else {  /* longjmp return from setjmp */ \
	    LwkExcLeaveExceptionRaised(); \
	    if (_False) {

#define _When(Code) \
	    } \
	    else if (_code_ == _StatusCode(Code)) {

#define _WhenOthers \
	    } \
	    else if (_code_ == _code_) {

#define _EndExceptionBlock \
	    } \
	    else \
		_Reraise; \
	} \
    } /* End of Exception Handling Block */

#define _Others _code_

#define _Signal(Code) LwkExcSignalException(_ExceptionCode(Code))
#define _Raise(Code) LwkExcRaiseException(_ExceptionCode(Code))
#define _Reraise LwkExcRaiseException((_Exception) _code_)

/*
**  Define Exception Codes
*/

#if defined(MSDOS) || defined(__osf__)
#define _ExceptionCode(Code) lwk_x_ ## Code
#else
#define _ExceptionCode(Code) lwk_x_/**/Code
#endif

#define _ExceptionCodes \
    _X_(failure), \
    _X_(bad_logic), \
    _X_(not_yet_impl), \
    _X_(alloc_error), \
    _X_(dealloc_error), \
    _X_(inv_object), \
    _X_(inv_argument), \
    _X_(inv_operation), \
    _X_(no_such_property), \
    _X_(inv_domain), \
    _X_(list_empty), \
    _X_(property_is_readonly), \
    _X_(dupl_element), \
    _X_(no_such_element), \
    _X_(inv_set_operation), \
    _X_(property_not_deleted), \
    _X_(single_valued_property), \
    _X_(cannot_move_surrogate), \
    _X_(cannot_move_link), \
    _X_(cannot_move_step), \
    _X_(cannot_delete_surrogate), \
    _X_(inv_date), \
    _X_(inv_ddif_string), \
    _X_(inv_string), \
    _X_(drm_open_error), \
    _X_(drm_fetch_error), \
    _X_(drm_close_error), \
    _X_(get_surrogate_cb_error), \
    _X_(create_surrogate_cb_error), \
    _X_(apply_cb_error), \
    _X_(close_view_cb_error), \
    _X_(complete_link_cb_error), \
    _X_(env_change_cb_error), \
    _X_(inv_encoding), \
    _X_(internal_encoding_error), \
    _X_(internal_decoding_error), \
    _X_(inv_widget_id), \
    _X_(stored_elsewhere), \
    _X_(no_such_linkbase), \
    _X_(db_not_open), \
    _X_(no_such_object), \
    _X_(db_read_error), \
    _X_(db_write_error), \
    _X_(db_lock_error), \
    _X_(db_commit_error), \
    _X_(db_version_error), \
    _X_(linkbase_in_use), \
    _X_(not_linked), \
    _X_(no_recording_linknet), \
    _X_(no_pending_source), \
    _X_(no_pending_target), \
    _X_(no_history), \
    _X_(history_is_empty), \
    _X_(no_active_cpath), \
    _X_(no_more_steps), \
    _X_(inv_query_expr), \
    _X_(abstract_object), \
    _X_(unexpected_error), \
    _X_(db_access_error), \
    _X_(apply_error), \
    _X_(invocation_error), \
    _X_(not_registered), \
    _X_(normal_return)

/*
**  External Routine Declarations
*/

_DeclareFunction(void LwkExcBeginException, (void));
_DeclareFunction(void LwkExcLeaveExceptionNormally, (void));
_DeclareFunction(void LwkExcLeaveExceptionRaised, (void));
_DeclareFunction(void LwkExcSignalException, (_Exception code));
_DeclareFunction(void LwkExcRaiseException, (_Exception code));

/*
**  External Data Definitions
*/

_External _ExcContext *LwkExcContexts;
_External int LwkExcCurrentContext;

#define _X_(Code) _ExceptionCode(Code)

_External _Exception _Constant
    _ExceptionCodes;

#undef _X_
