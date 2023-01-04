$ 
$ *****************************************************************
$ *                                                               *
$ *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
$ *                                                               *
$ *   All Rights Reserved.  Unpublished rights  reserved  under   *
$ *   the copyright laws of the United States.                    *
$ *                                                               *
$ *   The software contained on this media  is  proprietary  to   *
$ *   and  embodies  the  confidential  technology  of  Digital   *
$ *   Equipment Corporation.  Possession, use,  duplication  or   *
$ *   dissemination of the software and media is authorized only  *
$ *   pursuant to a valid written license from Digital Equipment  *
$ *   Corporation.                                                *
$ *                                                               *
$ *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
$ *   by the U.S. Government is subject to restrictions  as  set  *
$ *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
$ *   or  in  FAR 52.227-19, as applicable.                       *
$ *                                                               *
$ *****************************************************************
$ 
$ 
$ HISTORY
$ 
$ 
$ @(#)$RCSfile: sec_export.msg,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:17:14 $
$ 
$
$ @OSF_COPYRIGHT@
$
$
$ HISTORY
$ $OSF_Log:	sec_export.msg,v $
$ Revision 1.1.1.1  92/03/07  00:45:00  devrcs
$ *** OSF1_1B23 version ***
$ 
$ Revision 1.2  1991/07/18  10:38:59  devrcs
$ 	Fix the catalog.
$ 	[91/07/05  04:37:30  aster]
$
$ $OSF_EndLog$
$quote "
$set MS_SEC_EXPORT

$ security cpio & mltap efor import export data.

TYPE_FLAG	"\n\tTYPE FLAGS"
PRIVS		"\n\tPRIVS"
SENS_LBL	"\n\tSENS. LABEL"
ACL_I_E		"\n\tACL"
ERR_I_OPTION	"ERROR: specify input device with -I option\n"
ERR_O_OPT	"ERROR: specify output device with -O option\n"
ERR_DEV_MLS	"ERROR: the device is configured for multi-level sls\n"
ERR_DEV_ML_USE	"ERROR: the device is configured for multi-level use\n"
MLTAPE_ERR	"\nMLTAPE ERROR: Incorrect tape format\n\n"
TAPE_FORMAT	"Tape Format = "
DEV_FORMAT	"Dev. Format = "
PRIVS_I_E	"    privs: %s\n"
LABEL_I_E	"    label: %s\n"
MISSING_SL	"%s: missing sensitivity label\n"
UNEXPECTED_SL	"%s: unexpected sensitivity label\n"
ERR_SYS_NOT_CONF_SL	"ERROR: System not configured for sensitivity labels\n"
ERR_SYS_NOT_CONF_PRIV	"ERROR: System not configured for file privilege sets\n"
OPT_I_E	"\tOpt: %s"
MLD_I_E	"\tMld"
MLD_BRANCH	"\tMld branch: %s"
I_E_LABEL	"\tSLABEL: %s\n"
I_E_ACL		"\tACL:    %s\n"
I_E_PRIVS	"\tPPRIVS: %s\n"
I_E_GPRIVS	"\tGPRIVS: %s\n"
CANT_SET_SL	"%s: cannot set sensitivity label\n"
I_E_FILE	"file %s: "
ERR_CONV_IF_SL	"ERROR: Converting input file Sensitivity Label = "
ERR_CONV_IF_ACL	"ERROR: Converting input file ACL List = "
ENTER_CORRECT_LBL	"Please enter a correct label -> "
I_E_MKDIR	"mkdir: "
ILLEGAL_SYNTAX	"find: illegal syntax for mltape\n"
NOT_VALID_EXEC	"find: -exec not valid with mltape\n"
NOT_VALID_DEPTH	"find: -depth not valid with mltape\n"
PRINT_REQUIRED	"find: -print required with mltape\n"
