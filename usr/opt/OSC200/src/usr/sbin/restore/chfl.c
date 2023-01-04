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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: chfl.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:25:07 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined( lint) && !defined(_NOIDENT)

#endif

#include	"restore.h"

chfl	       *
scanstr(dest, src)
	chfl	       *dest;
	char	       *src;
{
	chfl	       *save_dest;
	char		quote;

	save_dest = dest;

	while (*src != '\0')
	{
		/*
		 * Handle back slashes.
		 */

		if (*src == '\\')
		{
			/* skip back slash */

			++src;

			if (*src == '\0')
			{
				msg(MSGSTR(CANTC, "lines cannot be continued\n"));
				continue;
			}

			/* copy escaped (back-slashed) character */

			CHR(dest) = *src;
			FLG(dest) = NOFLG;
			if (*src == '/')
			{
				FLG(dest) |= SLASH;
			}
			++dest, ++src;

			continue;
		}

		/*
		 * Handle single and double quotes.
		 */

		if (*src == '\'' || *src == '"')
		{
			/* get quote character */

			quote = *src;

			/* skip opening quote */

			++src;

			/* copy while looking for closing quote */

			while (*src != quote && *src != '\0')
			{
				CHR(dest) = *src;
				FLG(dest) = NOFLG;
				FLG(dest) |= QUOTED;
				if (*src == '/')
				{
					FLG(dest) |= SLASH;
				}
				++dest, ++src;
			}

			if (*src == '\0')
			{
				msg(MSGSTR(MISSQ, "missing closing quote: %c\n"), quote);
				continue;
			}

			/* skip closing quote */

			++src;

			continue;
		}

		/*
		 * The usual unquoted case, copy character
		 */

		CHR(dest) = *src;
		FLG(dest) = NOFLG;
		if (*src == '/')
		{
			FLG(dest) |= SLASH;
		}
		++dest, ++src;
	}

	/* terminate output string */

	CHR(dest) = '\0';

	return(save_dest);
}

chfl	       *
strtocfs(dest, src)
	chfl	       *dest;
	char	       *src;
{
	chfl	       *save_dest;

	save_dest = dest;

	while(*src != '\0')
	{
		CHR(dest) = *src;
		FLG(dest) = NOFLG;
		if (*src == '/')
		{
			FLG(dest) |= SLASH;
		}
		++dest, ++src;
	}

	CHR(dest) = '\0';

	return(save_dest);
}

char	       *
cfstostr(dest, src)
	char	       *dest;
	chfl	       *src;
{
	char	       *save_dest;

	save_dest = dest;

	while (CHR(src) != '\0')
	{
		*dest = CHR(src);
		++dest, ++src;
	}

	*dest = '\0';

	return(save_dest);
}

unsigned int
cfslen(cfs)
	chfl	       *cfs;
{
	unsigned int	len;

	len = 0;

	while (CHR(cfs) != '\0')
	{
		++cfs;
		++len;
	}

	return(len);
}

chfl	       *
cfscpy(dest, src)
	chfl	       *dest;
	chfl	       *src;
{
	chfl	       *save_dest;

	save_dest = dest;

	while (CHR(src) != '\0')
	{
		CHR(dest) = CHR(src);
		FLG(dest) = FLG(src);
		++dest, ++src;
	}

	CHR(dest) = '\0';

	return(save_dest);
}

chfl	       *
cfscat(dest, src)
	chfl	       *dest;
	chfl	       *src;
{
	chfl	       *save_dest;

	save_dest = dest;

	while (CHR(dest) != '\0')
	{
		++dest;
	}
	(void) cfscpy(dest, src);

	return(save_dest);
}

unsigned int
cfscmp(cfs1, cfs2)
	chfl	       *cfs1;
	chfl	       *cfs2;
{
	while (CHR(cfs1) == CHR(cfs2) && CHR(cfs1) != '\0' && CHR(cfs2) != '\0')
	{
		++cfs1, ++cfs2;
	}
	return(CHR(cfs1) - CHR(cfs2));
}

unsigned int
cfsncmp(cfs1, cfs2, len)
	chfl	       *cfs1;
	chfl	       *cfs2;
	unsigned int	len;
{
	while (len > 0 && CHR(cfs1) == CHR(cfs2) && CHR(cfs1) != '\0' && CHR(cfs2) != '\0')
	{
		++cfs1, ++cfs2;
		--len;
	}
	if (len == 0)
	{
		return(0);
	}
	return(CHR(cfs1) - CHR(cfs2));
}

chfl	       *
cfsdup(cfs)
	chfl	       *cfs;
{
	chfl	       *newcfs;

	newcfs = (chfl *) malloc((cfslen(cfs) + 1) * sizeof(chfl));
	(void) cfscpy(newcfs, cfs);
	return(newcfs);
}
