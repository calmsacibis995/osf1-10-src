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
**	Common header file for all modules which access the Database
**	implementation which underlies Repositories.
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
**  Creation Date: 13-Dec-88
**
**  Modification History:
**--
*/


/*
**  Include files
*/

/*
**  Macro Definitions
*/

#define _UidIncrementalAllocation 10

/*
**  Type Definitions
*/

/*
**  External Routine Declarations
*/

_DeclareFunction(void LwkLbInitialize, (void));
_DeclareFunction(_String LwkLbExpandIdentifier, (_String identifier));
_DeclareFunction(void LwkLbOpen, (_Linkbase repository, _Boolean create));
_DeclareFunction(void LwkLbClose, (_Linkbase repository));
_DeclareFunction(void LwkLbSetIdentifier, (_Linkbase repository));
_DeclareFunction(void LwkLbSetName, (_Linkbase repository));
_DeclareFunction(void LwkLbAssignIdentifier,
    (_Linkbase repository, _Persistent persistent));
_DeclareFunction(void LwkLbStartTransaction,
    (_Linkbase repository, _Transaction transaction));
_DeclareFunction(void LwkLbCommit, (_Linkbase repository));
_DeclareFunction(void LwkLbRollback, (_Linkbase repository));
_DeclareFunction(void LwkLbStore,
    (_Linkbase repository, _Domain domain, _Persistent persistent));
_DeclareFunction(_Persistent LwkLbRetrieve,
    (_Linkbase repository, _Domain domain, _Integer uid,
	_Integer container_id));
_DeclareFunction(void LwkLbDrop, (_Domain domain, _Persistent persistent));
_DeclareFunction(_Closure LwkLbIterate,
    (_Linkbase repository, _Domain volatile domain, _Closure closure,
	_Callback routine));
_DeclareFunction(void LwkLbUnCache,
    (_Linkbase repository, _Integer uid, _Persistent persistent));
_DeclareFunction(void LwkLbQuerySurrInContainer,
    (_Linkbase repository, _Persistent container, _Integer container_id,
	lwk_query_expression expression));
_DeclareFunction(void LwkLbQueryConnInNetwork,
    (_Linkbase repository, _Linknet network, _Integer net_id,
	lwk_query_expression expression));
_DeclareFunction(void LwkLbRetrieveSurrConnections,
    (_Linkbase repository, _Surrogate surrogate, _Linknet network,
	_Set interconnections));
_DeclareFunction(_Boolean LwkLbVerify,
    (_Linkbase repository, _Integer flags, _OSFile file));

#ifndef MSDOS

_DeclareFunction(void LwkLbQueryStepInPath,
    (_Linkbase repository, _Path path, _Integer path_id,
	lwk_query_expression expression));
_DeclareFunction(void LwkLbRetrieveSurrSteps,
    (_Linkbase repository, _Surrogate surrogate, _Path path,
	_Set interconnections));

#endif /* !MSDOS */
