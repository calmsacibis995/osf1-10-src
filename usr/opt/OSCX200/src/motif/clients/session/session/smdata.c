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
#include	"smstruct.h"
#include	"setupstruct.h"

globaldef	struct smdata	smdata;

globaldef struct  termdata	tdata;

globaldef	struct	resourcelist	xrmdb;

globaldef	struct	smattr_di	smsetup;

globaldef	struct	prtattr_di	prtsetup;

globaldef	struct	keyattr_di	keysetup;

globaldef	struct	pointattr_di	pointsetup;

globaldef	struct	interattr_di	intersetup;

globaldef	struct	windowattr_di	windowsetup;

globaldef	struct	screenattr_di	screensetup;

globaldef	struct	appmenuattr_di	appmenusetup;

globaldef	struct	autostartattr_di	autostartsetup;

globaldef	struct	appdefattr_di	appdefsetup;

globaldef	unsigned	int	exiting = 0;

globaldef	Display	*display_id;

globaldef	Window	control_window = 0;

globaldef	int	screen = 0;

globaldef	Window	root_window = 0;

globaldef	Cursor	*current_cursor = 0;

globaldef	Font	cursor_font = 0;

globaldef	unsigned	int	system_color_type = 0;

globaldef struct securityattr_di securitysetup;

globaldef   Widget  return_focus = 0;

globaldef   unsigned	int icon_state = 0;

globaldef   unsigned	int icon_type = 0;

globaldef   unsigned	int vtype = 0;

globaldef       MrmHierarchy s_DRMHierarchy;

globaldef       MrmType drm_dummy_class;

globaldef	char	**removelist= NULL;

globaldef	unsigned    int	removecount = 0;

#ifdef HYPERHELP
globaldef	Opaque  help_context;
#endif

