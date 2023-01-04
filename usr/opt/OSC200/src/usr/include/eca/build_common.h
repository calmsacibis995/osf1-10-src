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
 * @(#)$RCSfile: build_common.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:55:12 $
 */
 /*
 **++
 **  FACILITY:  [[facility]] 
 **
 **  Copyright (c) [[copyright_date]]  [[copyright_owner]]
 **
 **  MODULE DESCRIPTION:
 **
 **	 COMMON.H
 **
 **	 This header file defines the [[mom_name]] data structures 
 ** 	 and contains the definitions for the attributes, directives, etc.
 **
 **  AUTHORS:
 **
 **	[[author]]
 **
 **     This code was initially created with the 
 **	[[system]] MOM Generator - version [[version]]
 **
 **  CREATION DATE:  [[creation_date]]
 **
 **  MODIFICATION HISTORY:
 **
 **--
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define	CREATE	1
#define DELETE  2
#define ACTION  3
#define SET	4
#define GET	5

#define GROUP_IDENTIFIERS	1
#define GROUP_STATUS		2
#define GROUP_COUNTERS		3
#define GROUP_CHARACTERISTICS	4

/*
 * Merged from "dms_common.h":
 */
#ifdef VAXC
/*
 * Macro which returns "TRUE" if the supplied status value
 * represents an error condition.  Lower-case version should
 * not be used; it is supplied for compatability with older code.
 */
#define ERROR_CONDITION(status) (((int) (status) & 1) == 0)
#define error_condition(status) (((int) (status) & 1) == 0)
#else
#define ERROR_CONDITION(status) (status != MAN_C_SUCCESS)
#define error_condition(status) (status != MAN_C_SUCCESS)
#endif

/*
 * Data sharing macros from the VAX C manual.
 */
#ifdef VAXC
#define EXPORT globaldef 
#define IMPORT globalref
#else
#define EXPORT
#define IMPORT extern
#endif

/*-insert-code-define-version-*/

/*-insert-code-define-classes-*/

/*-insert-code-defs-*/

#include "import_oids.h"

#include "mom_prototypes.h"
