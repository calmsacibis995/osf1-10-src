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
#define dps_err_unknownError_text "Unrecognized error code %d\n"
/* Arguments:  errorcode (integer ) */

#define dps_err_ps_text "PostScript language error in figure\n\tError: %s\n\tOffending Command: %s\n"
/* Arguments: errorName, erroneousCommand */

#define dps_err_nameTooLong_text "PostScript language user name too long\n\tName: %s\n"
/* Arguments: badName */

#define dps_err_invalidContext_text "Invalid Display PostScript context\n\tContext Id: %d\n"
/* Arguments: badContextId (integer) */

#define dps_err_resultTagCheck_text "Erroneous Display PostScript wrap result tag\n\tTag: %s\n"
/* Arguments: badTag */

#define dps_err_resultTypeCheck_text "Incompatable Display PostScript wrap result type\n\tType: %s\n"
/* Arguments: badType */

#define dps_err_invalidAccess_text "Invalid Display PostScript context access\n"
/* Arguments:  <none> */

#define dps_err_encodingCheck_text "Invalid Display PostScript name/program encoding\n\t%d/%d\n"
/* Arguments:  arg1, arg2 (both integers) */

#define dps_err_closedDisplay_text "Broken display connection %d\n"
/* Arguments:  arg1 (integer) */

#define dps_err_deadContext_text "Dead context 0x0%x\n"
/* Arguments:  arg1 (hex integer ) */

