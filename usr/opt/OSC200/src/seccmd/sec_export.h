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
 * @(#)$RCSfile: sec_export.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/04/01 20:17:07 $
 */
#ifndef SEC_EXPORT_H
#define SEC_EXPORT_H

/*
 * @OSF_COPYRIGHT@
 */
/*
 * Copyright (c) 1988-1990 SecureWare, Inc.
 * All Rights Reserved.
 */
/*
 * HISTORY
 * $OSF_Log:	sec_export.h,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.5.2.2  1992/06/10  21:25:49  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/10  21:25:02  hosking]
 *
 * Revision 1.5  1991/08/15  19:23:59  devrcs
 * 	Fix the NLgetamsg().
 * 	[91/08/13  09:48:23  aster]
 * 
 * Revision 1.4  91/07/18  10:38:57  devrcs
 * 	Fix the catalog.
 * 	[91/07/05  04:37:02  aster]
 * 
 * Revision 1.3  90/10/07  15:41:39  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  13:04:00  gm]
 * 
 * Revision 1.2  90/09/13  11:59:22  devrcs
 * 	Initial version from SecureWare.
 * 	[90/08/21  14:13:47  seiden]
 * 
 * 	Initial version from SecureWare.
 * 	[90/08/21  10:32:57  seiden]
 * 
 * $OSF_EndLog$
 */

/* #ident "@(#)sec_export.h	5.1 14:54:12 8/15/90 SecureWare" */
/*
 * Based on (DEC PORT VERSION):
 *   "@(#)sec_export.h	3.1 07:03:44 4/6/90 SecureWare, Inc."
 */

/*
 * This Module contains Proprietary Information of SecureWare, Inc. and
 * should be treated as Confidential.
 */
/*
 * Error message definition table.
 * The constants define message triplets
 * "User message", "Audit op", "Audit result".
 * Null entries are skipped.
 */

#include "sec_export_msg.h"
#define MSGSTR(n,s) nlgetamsg(MF_SEC_EXPORT,MS_SEC_EXPORT,n,s)

#define NOACTION		0
#define TERMINATE		1

#define MSG_NOACL		0
#define MSG_NOMEM		1
#define MSG_NOAUTH		2
#define MSG_PROCLVL		3
#define MSG_GETPRIVS		4
#define MSG_INTERNAL		5
#define MSG_DEVLAB		6
#define MSG_DEVDB		7
#define MSG_DEVASSIGN		8
#define MSG_DEVIMPEXP		9
#define MSG_DEVSENSLEV		10
#define MSG_DEVNOLEV		11
#define MSG_DEVMAXLEV		12
#define MSG_DEVMINLEV		13
#define MSG_UPGRADE		14
#define MSG_DOWNGRADE		15
#define MSG_NOTEXP		16
#define MSG_NOTIMP		17
#define MSG_SETACL		18
#define MSG_BADACL		19
#define MSG_SETLAB		20
#define MSG_BADLAB		21
#define MSG_PNOTDOM		22
#define MSG_NODEMON		23
#define MSG_BADPOLICY		24
#define MSG_RDPOLICY		25
#define MSG_WRPOLICY		26
#define MSG_BADMAGIC		27
#define MSG_NOLAB		28
#define MSG_BADCAV		29
#define MSG_SETCAV		30
#define MSG_NOCAV		31
#define MSG_BADMLD		32
#define MSG_CHGLEV		33
#define MSG_GETPPRIVS		34
#define MSG_GETGPRIVS		35
#define MSG_SETPPRIVS		36
#define MSG_SETGPRIVS		37
#define MSG_BADILB		38
#define MSG_SETILB		39
#define MSG_BADKID		40
#define MSG_COPYLAB		41
#define MSG_COPYATTR		42
#define MSG_SET2PERSON		43
#define MSG_SYSDEF		44
#define MSG_DEVILBLEV		45
#define MSG_DEVNCAVLEV		46
#define MSG_PROCNCAV		47
#define MSG_PNCAVNOTDOM		48
#define MSG_BADNCAV		49
#define MSG_AUTHDEV		50
#define MSG_RECOVERY		51
#define MSG_RECPRVS		52
#define MSG_DEVSTAT		53
#define MSG_UTMPREAD		54
#define MSG_UTMPOPEN		55
#define MSG_INITLEV		56
#define MSG_SLNCAV		57
#define MSG_SLILB		58
#define MSG_SLSLABEL		59
#define MSG_NOTEXPCAV		60
#define MSG_TAPEFMT		61
#define MSG_DEVNOILB		62
#define MSG_RECNOTIMP		63
#define MSG_PIPE		64
#define MSG_SETPRIVS		65
#define MSG_DEVSLMATCH 		66
#define MSG_NOTIMPCAV		67


typedef struct {
		char *msg;
		char *aud_op;
		char *aud_res;
		int  action;
	} msg_t;
static msg_t msg[] = {
	{ 						/* 0 MSG_NOACL */
	   	"Can't find ACL",
	   	"Locate file ACL",
	   	"Failed",
	   	TERMINATE
	},
	{  	"Memory allocation error",		/* 1 MSG_NOMEM */
		"Allocate memory",
		"Failed",
		TERMINATE
	},
    	{	"No authorization for attempted operation", /* 2 MSG_NOAUTH */
		"Tape authorization",
		"Authorization not granted",
		TERMINATE
	}, 
	{	"Cannot determine process level",	/* 3 MSG_PROCLVL */
		"Determine process level",
		"Failed",
		TERMINATE
	},
	{	"Cannot determine process privileges",	/* 4 MSG_GETPRIVS */
		"Determine process privs",
		"Failed",
		TERMINATE
	},
	{	"Internal error",			/* 5 MSG_INTERNAL */
		"Internal error",
		"Error",
		TERMINATE
	},
	{ 	"Cannot determine device label",	/* 6 MSG_DEVLAB */
		"Determine device label",
		"Failed",
		TERMINATE
	},
	{	"Device not in database",		/* 7 MSG_DEVDB */
		"Lookup device in device database",
		"Entry missing",
		TERMINATE
	},

	{	"Incorrect import/export level",	/* 8 MSG_DEVASSIGN */
		"Check single/multi level assignment",
		"Failed",
		TERMINATE
	},
	{	"Device not enabled for import/export",	/* 9 MSG_DEVIMPEXP */
		"Check import/export enabled",
		"Failed",
		TERMINATE
	},
	{	"Unrecognized device level",		/* 10 MSG_DEVSENSLEV */
		"Check assigned device level",
		"Unrecognized level",
		TERMINATE
	},
	{	"Device has no assigned level",		/* 11 MSG_DEVNOLEV */
		"Check assigned device level",
		"No level set",
		TERMINATE
	},
							/* 12 MSG_DEVMAXLEV */
	{	"Missing or unrecognized maximum device level",
		"Check device maximum level",
		"Missing or unrecognized",
		TERMINATE
	},
							/* 13 MSG_DEVMINLEV */
	{	"Missing or unrecognized minimum device level",
		"Check device minimum level",
		"Missing or unrecognized",
		TERMINATE
	},
	{	"",					/* 14 MSG_UPGRADE */
		"Upgrade file for export",
		"Allowed",
		NOACTION
	},
	{	"",					/* 15 MSG_DOWNGRADE */
		"Downgrade file for export",
		"Allowed",
		NOACTION
	},
	{	"File not exported",			/* 16 MSG_NOTEXP */
		"Export file",
		"Denied by MAC",
		NOACTION
	},
	{	"File not imported",			/* 17 MSG_NOTIMP */
		"Import file",
		"Denied by MAC",
		NOACTION
	},
	{	"Can't set ACL",			/* 18 MSG_SETACL */
		"Set ACL",
		"Denied, file deleted",
		NOACTION
	},
	{	"Unrecognized ACL",			/* 19 MSG_BADACL */
		"Set ACL",
		"Unrecognized ACL - Denied, file deleted",
		NOACTION
	},
	{	"Can't set file label",			/* 20 MSG_SETLAB */
		"Set file label",
		"Denied, file deleted",
		NOACTION
	},
	{	"Unrecognized file label",		/* 21 MSG_BADLAB */
		"Set file label",
		"Unrecognized label - Denied, file ignored",
		NOACTION
	},
	{	"Process level does not dominate tape", /* 22 MSG_PNOTDOM */
		"Check process:tape mav level",
		"Process level too low",
		TERMINATE
	},
	{	"Can't get security policy demon",	/* 23 MSG_NODEMON */
		"Open security policy demon",
		"Failed",
		TERMINATE
	},
	{	"Security policies don't match",	/* 24 MSG_BADPOLICY */
		"Compare tape/system security policy",
		"Mismatched",
		TERMINATE
	},
	{	"Cannot read policy table",		/* 25 MSG_RDPOLICY */
		"Read tape policy table",
		"Failed",
		TERMINATE
	},
	{	"Cannot write policy table",		/* 26 MSG_WRPOLICY */
		"Write tape policy table",
		"Failed",
		TERMINATE
	},
	{	"Not a multi-level tape",		/* 27 MSG_BADMAGIC */
		"Check mutli-level tape magic",
		"Failed",
		TERMINATE
	},
	{	"File not labeled - skipped",		/* 28 MSG_NOLAB    */
		"Unlabeled file detected in multi-level export",
		"File not exported",
		NOACTION
	},
	{	"Unrecognized file caveat",		/* 29 MSG_BADCAV */
		"Set file caveat",
		"Unrecognized caveat - Denied, file deleted",
		TERMINATE
	},
	{	"Can't set file caveat",		/* 30 MSG_SETCAV */
		"Set file caveat",
		"Denied, file deleted",
		NOACTION
	},
	{	"File has no caveat - skipped",		/* 31 MSG_NOCAV    */
		"No-caveat file detected in multi-level export",
		"File not exported",
		NOACTION
	},
	{	"Cannot convert to multi-level directory", /* 32 MSG_BADMLD */
		"Make multilevel dir",
		"Denied, terminated",
		TERMINATE
	},
	{	"Cannot change process level",		   /* 33 MSG_CHGLEV */
		"Change process level",
		"Denied, terminated",
		NOACTION
	},
	{	"Cannot determine potential priv set",	  /* 34 MSG_GETPPRIVS */
		"Determine potential set",
		"Denied, ignored",
		NOACTION
	},
	{	"Cannot determine granted priv set",	  /* 35 MSG_GETGPRIVS */
		"Determine granted set",
		"Denied, ignored",
		NOACTION
	},
	{	"Cannot set potential priv set",	  /* 36 MSG_SETPPRIVS */
		"Set potential set",
		"Denied, ignored",
		NOACTION
	},
	{	"Cannot set granted priv set",	   	  /* 37 MSG_SETGPRIVS */
		"Set granted set",
		"Denied, ignored",
		NOACTION
	},
	{	"Unrecognized information label",	/* 38 MSG_BADILB */
		"Get information label",
		"Unrecognized information label - Denied, file ignored",
		NOACTION
	},
	{	"Can't set information label",		/* 39 MSG_SETILB */
		"Set information label",
		"Denied, file deleted",
		NOACTION
	},
	{	"Invalid mld child level",		/* 40 MSG_BADKID */
		"Get mld child parent level",
		"Denied, file ignored",
		NOACTION
	},
	{	"Error copying file label",		/* 41 MSG_COPYLAB*/
		"Copy file label",
		"Denied, not set",
		NOACTION
	},
	{	"Cannot copy file attributes",		/* 42 MSG_COPYATTR*/
		"Copy file attributes",
		"Denied, not set",
		NOACTION
	},
	{	"Cannot convert to 2-person rule",	/* 43 MSG_SET2PERSON*/
		"Convert to 2-person rule",
		"Denied, not set",
		NOACTION
	},
	{	"Error getting system defaults",	/* 44 MSG_SYSDEF */
		"Read system defaults",
		"Failed",
		TERMINATE
	},
	{	"Unrecognized information label",	/* 45 MSG_DEVILBLEV */
		"Get IL",
		"Unrecognized Info. label, terminated",
		TERMINATE
	},
	{	"Device NCAV level",			/* 46 MSG_DEVNCAVLEV */
		"Device NCAV level",
		"Device NCAV level, terminated",
		TERMINATE
	},
	{	"Process NCAV level",			/* 47 MSG_PROCNCAV */
		"Process NCAV level",
		"Process NCAV level, terminated",
		TERMINATE
	},
	{	"Process NCAV does not dominate device", /* 48 MSG_PROCNCAV */
		"Process NCAV not dominate",
		"Process NCAV not dominate, terminated",
		TERMINATE
	},
	{	"Unrecognized national caveats",	/* 49 MSG_BADNCAV */
		"Set caveats",
		"Unrecognized nat. caveats - Denied, file ignored",
		NOACTION
	},
	{	"Not authorized for this device",	/* 50 MSG_AUTHDEV */
		"Authorized for device",
		"Not authorized for device, terminated",
		TERMINATE
	},
	{	"Not authorized for system recovery",	/* 51 MSG_RECOVERY */
		"Authorized for system recovery",
		"Not authorized for system recovery, terminated",
		TERMINATE
	},
	{	"Insufficient privs for system recovery",   /* 52 MSG_RECPRVS */
		"Insufficient privs system recovery",
		"Insufficient privs for system recovery, terminated",
		TERMINATE
	},
	{	"Error stat(2)ing utmp file",   	/* 53 MSG_DEVSTAT */
		"Error stat(2)ing utmp file",
		"Error stat(2)ing utmp file, terminated",
		TERMINATE
	},
	{	"Error reading utmp file",   		/* 54 MSG_UTMPREAD */
		"Error reading utmp file",
		"Error reading utmp file, terminated",
		TERMINATE
	},
	{	"Error opening utmp file",   		/* 55 MSG_UTMPOPEN */
		"Error opening utmp file",
		"Error opening utmp file, terminated",
		TERMINATE
	},
	{	"Wrong init level",   			/* 56 MSG_INITLEV */
		"Wrong init level",
		"Wrong init level, terminated",
		TERMINATE
	},
	{	"Not single-level NCAVs",   		/* 57 MSG_SLNCAV */
		"Not single-level NCAVs",
		"Not single-level NCAVs, terminated",
		TERMINATE
	},
	{	"Not single-level ILBs",   		/* 58 MSG_SLILB */
		"Not single-level ILBs",
		"Not single-level ILBs, terminated",
		TERMINATE
	},
	{	"Not single-level sens. labels",  	/* 59 MSG_SLSLABEL */
		"Not single-level sens. labels",
		"Not single-level sens. labels, terminated",
		TERMINATE
	},
	{	"File not exported",			/* 60 MSG_NOTEXPCAV */
		"Export file",
		"Denied by NCAV",
		NOACTION
	},
	{	"Incorrect tape format",		/* 61 MSG_TAPEFMT */
		"Incorrect tape format",
		"Failed",
		TERMINATE
	},
	{	"No assigned information level",	/* 62 MSG_DEVNOILB */
		"No assigned information level",
		"Failed",
		TERMINATE
	},
	{	"-R Option only valid with -i option",	/* 63 MSG_RECNOTIMP */
		"Recover attempted not in import",
		"Failed",
		TERMINATE
	},
	{	"Pipe(2) failure",			/* 64 MSG_PIPE */
		"Pipe(2) failure",	
		"Failed",
		TERMINATE
	},
	{	"setpriv(3) failure",			/* 65 MSG_SETPRIVS */
		"setpriv(3) failure",	
		"Failed",
		TERMINATE
	},
	{						/* 66 MSG_DEVSLMATCH */
		"Device sensitivity level does not match assigned level",
		"Device sensitivity level not set properly",
		"Failed",
		TERMINATE
	},
	{						/* 67 MSG_NOTIMPCAV */
		"File not imported",
		"Import file",
		"Denied by NCAV",
		NOACTION,
	},
	};

#endif /* SEC_EXPORT_H */
