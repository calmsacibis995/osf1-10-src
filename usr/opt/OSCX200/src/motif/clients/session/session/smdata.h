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

#define GETSCREEN(screennum,display) \
(screennum <= (ScreenCount(display) - 1) ? screennum : DefaultScreen(display))

/* ifdef ONE_DOT_TWO */
#define def_char_set XmFONTLIST_DEFAULT_TAG
/* else define def_char_set XmSTRING_DEFAULT_CHARSET */

#define		icon_standard	0
#define		icon_reverse	1
#define		not_icon	0
#define		am_icon		1

#ifdef HYPERHELP
#define 	DOHELP
#include	<DXm/bkr_api.h>
#define		dxsession_help	"dxsession"
globalref	Opaque		help_context;
#endif

globalref	struct smdata	smdata;

globalref	struct statusblock;

globalref	struct	resourcelist	xrmdb;

globalref	struct	smattr_di	smsetup;

globalref	struct	prtattr_di	prtsetup;

globalref	struct	keyattr_di	keysetup;

globalref	struct	pointattr_di	pointsetup;

globalref	struct	interattr_di	intersetup;

globalref	struct	windowattr_di	windowsetup;

globalref	struct	screenattr_di	screensetup;

globalref	struct	appmenuattr_di	appmenusetup;

globalref	struct	autostartattr_di	autostartsetup;

globalref	struct	appdefattr_di	appdefsetup;

globalref	unsigned	int	exiting;

globalref	Display	*display_id;

globalref	Window	control_window;

globalref	int	screen;

globalref	Window	root_window;

globalref	Cursor	*current_cursor;

globalref	Font	cursor_font;

globalref	unsigned	int	system_color_type;

globalref struct securityattr_di securitysetup;

globalref   Widget  return_focus;

globalref   unsigned	int icon_state;

globalref   unsigned	int icon_type;

globalref unsigned int vtype;

globalref       MrmHierarchy s_DRMHierarchy;

globalref       MrmType drm_dummy_class;

globalref	char	**removelist;

globalref	unsigned    int	removecount;

globalref Boolean session_debug;

/* ifdef ONE_DOT_TWO */
globalref Boolean sm_inited;

typedef struct {
	Boolean session_debug;
	Boolean session_security;
	char *session_wm;
	char *session_default_wm;
	Boolean session_rootpasswd;
	int session_pausesaver;
	Boolean session_waitforwm;
	char *session_pausefile;
        char *startupfile;
        char *wmstring;
        char *termstring;
        char *uestring;
        Boolean nocautions;
        Boolean dontPutDatabase;
        Boolean dontStartMultipleWM;
} OptionsRec;
