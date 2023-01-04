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
**	Support for Exception Handling
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

#ifdef MSDOS
#include "include.h"
#else /* !MSDOS */
#include "his_include.h"
#endif

/*
**  Macro Definitions
*/

#define _ExceptionDepthIncrement 10

/*
**  Type Definitions
*/

/*
**  Forward Routine Declarations
*/

/*
**  Static Data Definitions
*/

static int ExcContextsDepth = 0;
static int ExcTrace = 0;

/*
**  Global Data Definitions
*/

_Global _ExcContext *LwkExcContexts = (_ExcContext *) 0;
_Global int LwkExcCurrentContext = -1;

#define _X_(Code) _ExceptionCode(Code) = (_Exception) _StatusCode(Code)

_Global _Exception _Constant
    _ExceptionCodes;

#undef _X_

/*
**  External Data Declarations
*/


void  LwkExcBeginException()
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
    **  Increment the exception context index.  If necessary, increase the size
    **	of the exception stack.
    */

    LwkExcCurrentContext++;

    if (LwkExcCurrentContext >= ExcContextsDepth) {
	ExcContextsDepth += _ExceptionDepthIncrement;

	if (LwkExcContexts == (_ExcContext *) 0)
	    LwkExcContexts = (_ExcContext *)
		malloc(ExcContextsDepth * sizeof(_ExcContext));
	else
	    LwkExcContexts = (_ExcContext *) realloc(LwkExcContexts,
		ExcContextsDepth * sizeof(_ExcContext));

	if (LwkExcContexts == (_ExcContext *) 0) {
#ifndef MSWINDOWS
	    printf("?HIS -- Memory allocation error in exception handler\n");
#endif /* !MSWINDOWS */
	    exit(0);
	}
    }

#ifndef MSWINDOWS
    if (ExcTrace)
	printf("[Entering exception context %d]\n", LwkExcCurrentContext);
#endif /* !MSWINDOWS */

    return;
    }


void  LwkExcLeaveExceptionNormally()
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
#ifndef MSWINDOWS
    if (ExcTrace)
	printf("[Leaving exception context %d normally]\n",
	    LwkExcCurrentContext);
#endif /* !MSWINDOWS */

    /*
    **  Decrement the exception context index.
    */

    LwkExcCurrentContext--;

    return;
    }


void  LwkExcLeaveExceptionRaised()
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
#ifndef MSWINDOWS
    if (ExcTrace)
	printf("[Leaving exception context %d with exception]\n",
	    LwkExcCurrentContext);
#endif /* !MSWINDOWS */

    /*
    **  Decrement the exception context index.
    */

    LwkExcCurrentContext--;

    return;
    }


void  LwkExcSignalException(code)
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
#ifndef MSWINDOWS
    if (ExcTrace)
	printf("[Exception signaled in context %d]\n", LwkExcCurrentContext);
#endif /* !MSWINDOWS */

    /*
    **  Decrement the exception context index.  Fatal error if decremented past
    **	zero.
    */

    LwkExcCurrentContext--;

    if (LwkExcCurrentContext < 0) {
#ifndef MSWINDOWS
	printf("?HIS -- Exception signaled outside of any exception context\n");
#endif /* !MSWINDOWS */
	exit(0);
    }

    /*
    **  Do the long jump with the exception code as an argument so that the
    **	exception context will be executed.
    */

    longjmp(LwkExcContexts[LwkExcCurrentContext], code);
    }


void  LwkExcRaiseException(code)
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

    if (LwkExcCurrentContext < 0) {
#ifndef MSWINDOWS
	printf("?HIS -- Exception raised outside of any exception context\n");
#endif /* !MSWINDOWS */
	exit(0);
    }

#ifndef MSWINDOWS
    if (ExcTrace)
	printf("[Exception raised in context %d]\n", LwkExcCurrentContext);
#endif /* !MSWINDOWS */

    /*
    **  Do the long jump with the exception code as an argument so that the
    **	exception context will be executed.
    */

    longjmp(LwkExcContexts[LwkExcCurrentContext], code);
    }
