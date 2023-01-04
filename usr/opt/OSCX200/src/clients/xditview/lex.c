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
#include <X11/Xos.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <stdio.h>
#include <ctype.h>
#include "DviP.h"

DviGetAndPut(dw, cp)
    DviWidget	dw;
    int		*cp;
{
    if (dw->dvi.ungot)
    {
	dw->dvi.ungot = 0;
	*cp = getc (dw->dvi.file);
    }
    else
    {
	*cp = getc (dw->dvi.file);
	putc (*cp, dw->dvi.tmpFile);
    }
    return *cp;
}

char *
GetLine(dw, Buffer, Length)
	DviWidget	dw;
	char	*Buffer;
	int	Length;
{
	int 	i = 0, c;
	char	*p = Buffer;
	
	Length--;			    /* Save room for final NULL */
	
	while (i < Length && DviGetC (dw, &c) != EOF && c != '\n')
		if (p)
			*p++ = c;
	if (c == '\n' && p)		    /* Retain the newline like fgets */
		*p++ = c;
	if (c == '\n')
		DviUngetC(dw, c);
	if (p)	
		*p = NULL;
	return (Buffer);
} 

char *
GetWord(dw, Buffer, Length)
	DviWidget	dw;
	char	*Buffer;
	int	Length;
{
	int 	i = 0, c;
	char	*p = Buffer;
	
	Length--;			    /* Save room for final NULL */
	while (DviGetC(dw, &c) != EOF && isspace(c))
		;
	if (c != EOF)
		DviUngetC(dw, c);
	while (i < Length && DviGetC(dw, &c) != EOF && !isspace(c))
		if (p)
			*p++ = c;
	if (c != EOF)
		DviUngetC(dw, c);
	if (p)
		*p = NULL;
	return (Buffer);
} 

GetNumber(dw)
	DviWidget	dw;
{
	int	i = 0,  c;

	while (DviGetC(dw, &c) != EOF && isspace(c))
		;
	if (c != EOF)
		DviUngetC(dw, c);
	while (DviGetC(dw, &c) != EOF && isdigit(c))
		i = i*10 + c - '0';
	if (c != EOF)
		DviUngetC(dw, c);
	return (i);
}
	
	    
