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
#ifndef globaldef
#define globaldef
#endif /* not globaldef */
#ifndef globalref
#define globalref extern
#endif /* not globalref */

globaldef char	*color_filename = "SYS$LOGIN:DECW$SM_COLOR.DAT";
globaldef char	*system_color_filename = "DECW$SYSTEM_DEFAULTS:DECW$SM_COLOR.DAT";
globaldef char	*bw_filename = "SYS$LOGIN:DECW$SM_BW.DAT";
globaldef char	*system_bw_filename = "DECW$SYSTEM_DEFAULTS:DECW$SM_BW.DAT";
globaldef char	*generic_filename = "SYS$LOGIN:DECW$SM_GENERAL.DAT";
globaldef char	*gray_filename = "SYS$LOGIN:DECW$SM_GRAY.DAT";
globaldef char	*system_gray_filename = "DECW$SYSTEM_DEFAULTS:DECW$SM_GRAY.DAT";
char *user_filename = ".Xdefaults";
char *system_generic_filename = "/usr/lib/X11/app-defaults/Xdefaults";

