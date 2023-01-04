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
 *	@(#)$RCSfile: ldr_windows.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:37:26 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 *	loader window interfaces for loader modules.
 *
 * OSF/1 Release 1.0
 */

#ifndef _H_LDR_WINDOW
#define _H_LDR_WINDOW

#include "ldr_macro_help.h"

struct ldr_window_t	{
	univ_t		map;		/* window refers to this mapping */
	ldr_file_t	fd;		/* file being windowed */
	off_t		start;		/* start pt. in file being windowed */
	size_t		length;		/* window size in bytes */
};

typedef struct ldr_window_t ldr_window_t;

extern ldr_window_t *
ldr_init_window __((ldr_file_t fd));

extern univ_t
ldr_file_window __((int start, size_t len, ldr_window_t *wp));

/*
 *	macro ldr_unwindow: get rid of window mapping
 */

#ifndef FAILURE
#define FAILURE 	0
#endif

#ifndef INVALID_FD
#define INVALID_FD  	-1
#endif

#define ldr_unwindow(wp) \
	MACRO_BEGIN \
	    if (dec_map_refcount((wp)->map) != FAILURE) { \
		(wp)->map = NULL; \
		(wp)->fd = INVALID_FD; \
		(wp)->start = 0; \
		(wp)->length = 0; \
		} \
		MACRO_END

extern int
ldr_flush_mappings __((ldr_file_t fd));

extern int
ldr_flush_lru_maps __(());

extern int 
dec_map_refcount __((univ_t map));  /* was ldr_mapping_t* */

#endif /* _H_LDR_WINDOW */







