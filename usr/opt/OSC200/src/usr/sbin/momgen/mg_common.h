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
 * @(#)$RCSfile: mg_common.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 22:10:47 $
 */
 /*
 **++
 **  FACILITY:	MOMGEN templates
 **
 **  MODULE DESCRIPTION:
 **
 **	 This header file used by the MOMGENerator
 **
 **  AUTHORS:
 **
 **	Rich Bouchard
 **
 **  CREATION DATE:  18-March-1991
 **
 **  MODIFICATION HISTORY:
 **
 **	X-2	Marc Nozell	21-May-1991
 **	
 **	Use standard defines.
 **
 **--
 */

#define error_condition(status) ((status & 1) == 0)
#define return_if_error(status) if error_condition(status) {return status;}
#define ERROR_CONDITION(status) error_condition(status)
#define RETURN_IF_ERROR(status) return_if_error(status)

#ifdef VAXC
#define EXPORT globaldef
#define IMPORT globalref
#include <descrip>
#else
#define EXPORT
#define IMPORT extern
#endif

#define GROUP_IDENTIFIERS	1
#define GROUP_STATUS		2
#define GROUP_COUNTERS		3
#define GROUP_CHARACTERISTICS	4

#define VMS_K_MONITOR_ID 1
#define VMS_K_LOG_ID 2
#define VMS_K_REQUEST_ID 3
