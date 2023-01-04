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
 * xman - X window system manual page display program.
 *
 * $XConsortium: globals.h,v 1.8 91/09/03 17:42:51 dave Exp $
 *
 * Copyright 1987, 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:    Chris D. Peterson, MIT Project Athena
 * Created:   October 22, 1987
 */

#include "man.h"

extern Xman_Resources resources;	/* Resource manager sets these. */

/* bookkeeping global variables. */

extern Widget help_widget;		/* The help widget. */

extern int default_height,default_width; /* Approximately the default with and
					    height, of the manpage when shown,
					    the the top level manual page 
					    window */
extern int man_pages_shown;		/* The current number of manual
					   pages being shown, if 0 we exit. */

extern Manual * manual;		        /* The manual structure. */
extern int sections;			/* The number of manual sections. */

extern XContext manglobals_context;	/* The context for man_globals. */

extern Widget initial_widget;	      /* The initial widget, never realized. */

extern char * option_names[];

extern char **saved_argv;
extern int saved_argc;
extern char **saved_argv;
extern int saved_argc;
