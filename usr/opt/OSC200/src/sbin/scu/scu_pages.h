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
 * @(#)$RCSfile: scu_pages.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/19 09:41:51 $
 */
/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived	from  software	received  from	the	*
 *   University    of	California,   Berkeley,   and	from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is	subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * File:	scu_pages.h
 * Author:	Robin T. Miller
 * Date:	September 10, 1991
 *
 * Description:
 *	Definitions for processing mode pages.
 *
 * Modification History:
 *
 */

/*
 * Define Operation Type Command Codes.
 */
#define SET_OPERATION		0	/* Set parameters.	*/
#define SHOW_OPERATION		1	/* Show parameters.	*/
#define CHANGE_OPERATION	2	/* Change parameters.	*/
#define VERIFY_OPERATION	3	/* Verify parameters.	*/

#define QUIET_MODE		0	/* Quiet verbose msgs.	*/
#define VERBOSE_MODE		1	/* Force verbose output.*/

/*
 * Strings:
 */
extern char *too_large_str, *setting_str, *leaving_str;
extern char *changing_str, *verifying_str;
extern char *pcf_table[], *yesno_table[], *endis_table[];

/*
 * External Declarations:
 */
extern void Printf();
extern void PrintAscii (char *field_str, char *ascii_str, int nl_flag);
extern void PrintDecimal (char *field_str, int numeric_value, int nl_flag);
extern void PrintHex (char *field_str, int numeric_value, int nl_flag);
extern void PrintNumeric (char *field_str, int numeric_value, int nl_flag);

extern void ShowModeParameters(), ShowBlockDesc(), ShowModeHeader();
extern void ShowParamHeader(), ShowPageHeader();

/*
 * Mode Page Function Dispatch Structure:
 */
typedef struct mode_page_funcs {
	u_char	pf_page_code;		/* The mode page code.		*/
	int	(*pf_setpage)();	/* Set mode page function.	*/
	int	(*pf_showpage)();	/* Show mode page function.	*/
	int	(*pf_changepage)();	/* Change mode page function.	*/
	int	(*pf_verifypage)();	/* Verify mode page function.	*/
	int	(*pf_sensepage)();	/* Sense mode page function.	*/
	int	(*pf_selectpage)();	/* Select mode page function.	*/
} MODE_PAGE_FUNCS;

