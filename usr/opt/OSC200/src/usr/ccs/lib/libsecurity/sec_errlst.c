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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: sec_errlst.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/04/01 20:25:29 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	sec_errlst.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.9.2.2  1992/06/11  14:29:09  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  14:26:47  hosking]
 *
 * Revision 1.9  1991/08/16  09:44:01  devrcs
 * 	Fixed an 1.1 i18n bug: removed the usage of NLgetamsg
 * 	[91/08/13  09:29:31  aster]
 * 
 * Revision 1.8  91/03/23  17:57:39  devrcs
 * 	<<<replace with log message for ./usr/ccs/lib/libsecurity/sec_errlst.c>>>
 * 	[91/03/12  11:30:56  devrcs]
 * 
 * 	Merge fixes up from 1.0.1
 * 	[91/03/11  15:26:01  seiden]
 * 
 * 	Add message 28.
 * 	[91/02/22  11:17:06  seiden]
 * 
 * Revision 1.5  90/10/07  20:08:39  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:17:12  gm]
 * 
 * Revision 1.4  90/08/09  14:24:35  devrcs
 * 	Changes for widened types: make ushorts into longwords.
 * 	[90/08/02  15:37:19  seiden]
 * 
 * Revision 1.3  90/07/27  10:32:32  devrcs
 * 	Fixed the initialization i18n problem
 * 	[90/07/17  08:44:25  staffan]
 * 
 * Revision 1.2  90/07/17  12:21:17  devrcs
 * 	Internationalized
 * 	[90/07/05  07:44:20  staffan]
 * 
 * 	Initial version from SecureWare
 * 	new table of sec_errno printed strings
 * 	[90/06/26  09:59:00  staffan]
 * 
 * $OSF_EndLog$
 */
/*
 * Copyright (c) 1990, SecureWare, Inc.
 *   All rights reserved.
 *
 * Security error message list.
 */

/* #ident "@(#)sec_errlst.c	1.1 22:33:11 5/28/90 SecureWare" */

#include "libsecurity.h"

char *sys_secerrlist[] = {
    /*  0 */	"Security error 0",
    /*  1 */	"MAC configuration file error",
    /*  2 */	"Operation fails on wildcard tag",
    /*  3 */	"ACL configuration file error",
    /*  4 */	"Subject has wildcard sensitivity label",
    /*  5 */	"Directory has wildcard sensitivity label",
    /*  6 */	"Subject and object sensitivity labels not equal",
    /*  7 */	"Privilege required for operation",
    /*  8 */	"No decision available from daemon",
    /*  9 */	"Subject sensitivity label does not dominate",
    /* 10 */	"Object sensitivity label does not dominate",
    /* 11 */	"Setting sensitivity but clearance not set",
    /* 12 */	"Setting sensitivity label to strictly dominate clearance",
    /* 13 */	"Cannot change other MAC labels if sensitivity label not set",
    /* 14 */	"Cannot raise clearance",
    /* 15 */	"Violation of sensitivity/information label relationship",
    /* 16 */	"Violation of increasing tree",
    /* 17 */	"Need write access for operation",
    /* 18 */	"Must be owner of object",
    /* 19 */	"Operation invalid on multilevel directory",
    /* 20 */	"Operation invalid on non-empty directory",
    /* 21 */	"Cannot combine information labels",
    /* 22 */	"Not extended filesystem format",
    /* 23 */	"File is not regular file",
    /* 24 */	"Cannot set file attributes",
    /* 25 */	"Cannot map internal representation to tag",
    /* 26 */	"Nationality caveat configuration file error",
    /* 27 */	"Directory is not multilevel",
    /* 28 */    "Operation invalid on extended format filesystem"
};

int sys_secerrnums[] = {
    /*  0 */	MSESEC_NOERROR,
    /*  1 */	MSESEC_MAC_CONFIG_FAILURE,
    /*  2 */	MSESEC_WILDCARD_TAG,
    /*  3 */	MSESEC_ACL_CONFIG_FAILURE,
    /*  4 */	MSESEC_WILD_SUBJ_SL,
    /*  5 */	MSESEC_WILD_DIR_SL,
    /*  6 */	MSESEC_MAC_NOT_EQUAL,
    /*  7 */	MSESEC_MAC_NEED_PRIVILEGE,
    /*  8 */	MSESEC_NO_DECISION,
    /*  9 */	MSESEC_MAC_SDOM,
    /* 10 */	MSESEC_MAC_ODOM,
    /* 11 */	MSESEC_MAC_NO_CLEARANCE,
    /* 12 */	MSESEC_MAC_DOM_CLEARANCE,
    /* 13 */	MSESEC_MAC_NO_SL,
    /* 14 */	MSESEC_MAC_UP_CLEARANCE,
    /* 15 */	MSESEC_MAC_IL_RELATION,
    /* 16 */	MSESEC_MAC_FS_RELATION,
    /* 17 */	MSESEC_MAC_NO_WRITE,
    /* 18 */	MSESEC_NOT_OWNER,
    /* 19 */	MSESEC_IS_MLD,
    /* 20 */	MSESEC_DIR_NOT_EMPTY,
    /* 21 */	MSESEC_MAC_IL_NOCOMB,
    /* 22 */	MSESEC_UNEXT_FS,
    /* 23 */	MSESEC_NOT_REG,
    /* 24 */	MSESEC_SETATTR_FAIL,
    /* 25 */	MSESEC_INVALID_IR,
    /* 26 */	MSESEC_NCAV_CONFIG_FAILURE,
    /* 27 */	MSESEC_NOT_MLD,
    /* 28 */    MSESEC_EXT_FS
};

int sys_nsecerr = sizeof(sys_secerrlist) / sizeof(sys_secerrlist[0]);
