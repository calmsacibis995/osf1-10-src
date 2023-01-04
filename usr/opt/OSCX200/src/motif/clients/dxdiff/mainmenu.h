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
/* $Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxdiff/mainmenu.h,v 1.1.4.2 1993/06/25 16:58:32 Lynda_Rice Exp $ */

/*
 * Copyright 1988 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 *
 *	dxdiff
 *
 *	mainmenu.h - main menu
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : 3rd July 1988
 *
 *
 *	Description
 *	-----------
 *
 *
 *	Modification History
 *	------------ -------
 *	
 */

typedef	enum _filesmenubuttonindex {
	OpenFilesButton,
#ifdef	VIEWBTNS
	ViewNextButton,
	ViewPrevButton,
#endif
	QuitButton
} FilesMenuButtonIndex;


typedef	enum _searchmenubuttonindex {
	SetREButton,
	FindNextButton,
	FindPrevButton
} SearchMenuButtonIndex;

typedef	enum _optionsmenubuttonindex {
	SlaveScrollVButton,
	SlaveScrollHButton,
	RenderDiffsButton,
	RenderLineNumbersButton
} OptionsMenuButtonIndex;


typedef	enum _differencesmenubuttonindex {
	DoDifferencesButton,
	DoDifferencesInNewButton
} DifferencesMenuButtonIndex;


typedef	enum	_mainmenubarbuttonsindex {
	FilesButton,
#ifdef	SEARCH
	SearchButton,
#endif
	OptionsButton,
	DifferencesButton,
	HelpButton
} MainMenuBarButtonsIndex;

