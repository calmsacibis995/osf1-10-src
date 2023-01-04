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
**	LinkWorks Manager
**
**  Version: V1.0
**
**  Abstract:
**	Support for Exception Handling
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	André Pavanello
**
**  Creation Date: 2-Nov-89
**
**  Modification History:
**--
*/


/*
**  Include Files
*/

#include "hs_include.h"

/*
**  Macro Definitions
*/

#define _MaxExceptionNesting 15

/*
**  Type Definitions
*/

/*
**  Forward Routine Declarations
*/

/*
**  Static Data Definitions
*/

/*
**  Global Data Definitions
*/

_Global jmp_buf EnvExcContexts[_MaxExceptionNesting];
_Global int     EnvExcContext = -1;
_Global int     EnvExcTrace = 0;

#define _X_(Code) _ExceptionCode(Code) = (_Exception) _StatusCode(Code)

_Global _Exception _Constant
    _ExceptionCodes;

#undef _X_

/*
**  External Data Declarations
*/


static _Void  EnvExc()
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
    return;
    }


_Void  EnvExcBeginException()
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
    **  Increment the exception context index.  Fatal error if maximum
    **	exception nesting is exceeded.
    */

    EnvExcContext++;

    if (EnvExcContext >= _MaxExceptionNesting) {
	printf("?LinkWorks Manager error: too many nested exception contexts\n");
	exit(0);
    }

    if (EnvExcTrace)
	printf("[Entering exception context %d]\n", EnvExcContext);

    return;
    }


_Void  EnvExcLeaveExceptionNormally()
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
    if (EnvExcTrace)
	printf("[Leaving exception context %d normally]\n", EnvExcContext);

    /*
    **  Decrement the exception context index.
    */

    EnvExcContext--;

    return;
    }


_Void  EnvExcLeaveExceptionRaised()
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
    if (EnvExcTrace)
	printf("[Leaving exception context %d with exception]\n",
	    EnvExcContext);

    /*
    **  Decrement the exception context index.
    */

    EnvExcContext--;

    return;
    }


_Void  EnvExcSignalException(code)
_Exception code;

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
    if (EnvExcTrace)
	printf("[Exception signaled in context %d]\n", EnvExcContext);

    /*
    **  Decrement the exception context index.  Fatal error if decremented past
    **	zero.
    */

    EnvExcContext--;

    if (EnvExcContext < 0) {
	printf("?Exception signaled outside of any exception context\n");
	exit(0);
    }

    /*
    **  Do the long jump with the exception code as an argument so that the
    **	exception context will be executed.
    */

    longjmp(EnvExcContexts[EnvExcContext], code);
    }


_Void  EnvExcRaiseException(code)
_Exception code;

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
    **  Fatal error if no context in use.
    */

    if (EnvExcContext < 0) {
	printf("?Exception raised outside of any exception context\n");
	exit(0);
    }

    if (EnvExcTrace)
	printf("[Exception raised in context %d]\n", EnvExcContext);

    /*
    **  Do the long jump with the exception code as an argument so that the
    **	exception context will be executed.
    */

    longjmp(EnvExcContexts[EnvExcContext], code);
    }
