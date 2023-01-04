/*
*****************************************************************************
**                                                                          *
**  COPYRIGHT (c) 1988, 1989, 1991, 1992 BY                                 *
**  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.                  *
**  ALL RIGHTS RESERVED.                                                    *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED   *
**  ONLY IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE   *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER   *
**  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY   *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE IS  HEREBY   *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE   *
**  AND  SHOULD  NOT  BE  CONSTRUED AS  A COMMITMENT BY DIGITAL EQUIPMENT   *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS   *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
**                                                                          *
*****************************************************************************
**
** FACILITY:  PrintScreen
**
** ABSTRACT:
**
**	This module contains global session manager data.
**
** ENVIRONMENT:
**
**      VAX/VMS operating system.
**
** AUTHOR:  Karen Brouillette October 1989
**
** Modified by:
**
**	04-Apr-1991	Edward P Luwish
**		Port to Motif UI
**
*/

#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /b5/aguws3.0/aguws3.0_rcs/src/dec/clients/print/smdata.c,v 1.2 91/12/30 12:48:20 devbld Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */

/*
** Include files
*/
#include "iprdw.h"

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include	<Mrm/MrmPublic.h>
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

#include	"smstruct.h"
#include	"setupstruct.h"

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif

globaldef	struct smdata	smdata;

globaldef	XrmDatabase	xrmdb = 0;

globaldef       struct  smattr_di       smsetup;

globaldef	struct	prtattr_di	prtsetup;

globaldef	struct	screenattr_di	screensetup;

globaldef	Display	*display_id;

globaldef	Cursor	*current_cursor = 0;

globaldef	Font	cursor_font = 0;

globaldef   Widget  return_focus = 0;

globaldef   unsigned	int icon_state = 0;

globaldef   unsigned	int icon_type = 0;

globaldef       MrmHierarchy s_DRMHierarchy;

globaldef       MrmType *drm_dummy_class;

globaldef	char	temp[256];

#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif
