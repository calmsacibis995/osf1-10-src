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
/* BuildSystemHeader added automatically */
/* $Header: /b5/aguws3.0/aguws3.0_rcs/src/dec/clients/print/smdata.h,v 1.2 91/12/30 12:48:20 devbld Exp $ */
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
**	The header files of printscreen global variables
**
** ENVIRONMENT:
**
**      VAX/VMS operating system.
**
** AUTHOR:  Karen Brouillette December 1989
**
** Modified by:
**
**	04-Apr-1991	Edward P Luwish
**		Port to Motif UI
**
*/

#if defined (VMS) && !defined (__DECC)
#pragma nostandard
#endif
#include	<Mrm/MrmPublic.h>
#if defined (VMS) && !defined (__DECC)
#pragma standard
#endif

#include	"smstruct.h"
#include	"setupstruct.h"

#ifndef $DESCRIPTOR_S
#define $DESCRIPTOR_S(name)     \
struct dsc$descriptor name = { 0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0 }
#endif

#define GETSCREEN(screennum,display) \
(screennum <= (ScreenCount(display) - 1) ? screennum : DefaultScreen(display))

#define		icon_standard	0
#define		icon_reverse	1
#define		not_icon	0
#define		am_icon		1

#if defined (VMS) && !defined (__DECC)
#pragma nostandard
#endif

globalref	struct smdata	smdata;

globalref	XrmDatabase	xrmdb;

globalref       struct  smattr_di       smsetup;

globalref	struct	prtattr_di	prtsetup;

globalref	struct	screenattr_di	screensetup;

globalref	Display	*display_id;

globalref	Cursor	*current_cursor;

globalref	Font	cursor_font;

globalref   Widget  return_focus;

globalref   unsigned	int icon_state;

globalref   unsigned	int icon_type;

globalref       MrmHierarchy s_DRMHierarchy;

globalref       MrmType drm_dummy_class;

globalref	char	temp[256];

#if defined (VMS) && !defined (__DECC)
#pragma standard
#endif
