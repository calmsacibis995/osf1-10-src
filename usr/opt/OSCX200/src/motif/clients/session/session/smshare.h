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
#include <X11/Xlib.h>
#include <X11/Xresource.h>

/* type of system */

#define black_white_system 0
#define color_system	1
#define gray_system	2
#define num_system_types 3

/* resource indexes */
#define rdb_color   0
#define rdb_generic 1
#define rdb_merge 2
#define num_databases 3

#define	do_user 0
#define do_manager 1

/* define string macro */

#define init_str_desc(desc, string) 		\
{   desc.dscw_length = strlen(string);		\
    desc.dsca_pointer = string;		\
    desc.dscb_class = DSCK_CLASS_S;		\
    desc.dscb_dtype = DSCK_DTYPE_T;		\
}
#define init_null_str_desc(desc, string)	\
{   desc.dscw_length = sizeof(string);		\
    desc.dsca_pointer = string;		\
    desc.dscb_class = DSCK_CLASS_S;		\
    desc.dscb_dtype = DSCK_DTYPE_T;		\
}

struct statusblock
	{
	short	stat;
	short	count;
	short	terminator;
	short	terminator_size;
	};

struct resourcelist
	{
	XrmDatabase	xrmlist[num_system_types][num_databases];
	};

globalref	char	*color_filename;
globalref	char	*system_color_filename; 
globalref	char	*bw_filename;
globalref	char	*system_bw_filename;
globalref	char	*generic_filename;
globalref	char	*system_generic_filename;
globalref	char	*user_filename;
globalref	char	*gray_filename;
globalref	char	*system_gray_filename;
globalref	char	*wm_name;
