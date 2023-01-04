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
/* $XConsortium: page.c,v 1.5 91/07/26 00:40:20 keith Exp $ */

/*
 * page.c
 *
 * map page numbers to file position
 */

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xos.h>
#include <stdio.h>
#include <ctype.h>
#include "DviP.h"

#ifdef X_NOT_STDC_ENV
extern long	ftell();
#endif

static DviFileMap *
MapPageNumberToFileMap (dw, number)
	DviWidget	dw;
	int		number;
{
	DviFileMap	*m;

	for (m = dw->dvi.file_map; m; m=m->next)
		if (m->page_number == number)
			break;
	return m;
}

DestroyFileMap (m)
	DviFileMap	*m;
{
	DviFileMap	*next;

	for (; m; m = next) {
		next = m->next;
		XtFree ((char *) m);
	}
}

ForgetPagePositions (dw)
	DviWidget	dw;
{
	DestroyFileMap (dw->dvi.file_map);
	dw->dvi.file_map = 0;
}

RememberPagePosition(dw, number)
	DviWidget	dw;
	int		number;
{
	DviFileMap	*m;

	if (!(m = MapPageNumberToFileMap (dw, number))) {
		m = (DviFileMap *) XtMalloc (sizeof *m);
		m->page_number = number;
		m->next = dw->dvi.file_map;
		dw->dvi.file_map = m;
	}
	if (dw->dvi.tmpFile)
		m->position = ftell (dw->dvi.tmpFile);
	else
		m->position = ftell (dw->dvi.file);
}

SearchPagePosition (dw, number)
	DviWidget	dw;
	int		number;
{
	DviFileMap	*m;

	if (!(m = MapPageNumberToFileMap (dw, number)))
		return -1;
	return m->position;
}

FileSeek(dw, position)
DviWidget	dw;
long		position;
{
	if (dw->dvi.tmpFile) {
		dw->dvi.readingTmp = 1;
		fseek (dw->dvi.tmpFile, position, 0);
	} else
		fseek (dw->dvi.file, position, 0);
}

