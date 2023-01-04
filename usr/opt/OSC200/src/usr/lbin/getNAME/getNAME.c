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
static char	*sccsid = "@(#)$RCSfile: getNAME.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/10/13 14:10:42 $";
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


/*
 * COMPONENT_NAME: (CMDMAN) commands that allow users to read online
 * documentation 
 *
 * FUNCTIONS: getfrom, trimln, doname, split, dorefname
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Get name sections from manual pages.
 *      -t      for building toc
 *      -i      for building intro entries
 *      other   apropos database
 */
#include <string.h>
#include <stdio.h>

#if defined(NLS) || defined(KJI)
#include        <NLchar.h>
#endif

int tocrc;
int intro;

main(argc, argv)
        int argc;
        char *argv[];
{

        argc--, argv++;
        if (!NLstrcmp(argv[0], "-t"))
                argc--, argv++, tocrc++;
        if (!NLstrcmp(argv[0], "-i"))
                argc--, argv++, intro++;
        while (argc > 0)
                getfrom(*argv++), argc--;
        exit(0);
}

getfrom(name)
        char *name;
{
        char headbuf[BUFSIZ];
        char linbuf[BUFSIZ];
        register char *cp;
        int i = 0;

        if (freopen(name, "r", stdin) == 0) {
                perror(name);
                return;
        }
        for (;;) {
                if (fgets(headbuf, (int)sizeof(headbuf), stdin) == NULL)
                        return;
                if (headbuf[0] != '.')
                        continue;
#ifdef  KJI
                if (NCisshift(headbuf[1]))
                        continue;
#endif
                if ((headbuf[1] == 'T' && headbuf[2] == 'H') ||
		    (headbuf[1] == 't' && headbuf[2] == 'h') )
		  	break;
        }
        for (;;) {
                if (fgets(linbuf, (int)sizeof(linbuf), stdin) == NULL)
                        return;
                if (linbuf[0] != '.')
                        continue;
#ifdef  KJI
                if (NCisshift(linbuf[1]))
                        continue;
#endif
                if ((linbuf[1] == 'S' && linbuf[2] == 'H') ||
		    (linbuf[1] == 's' && linbuf[2] == 'h') )
                        break;
        }
        trimln(headbuf);
        if (tocrc)
                doname(name);
        if (!intro)
                NLprintf("%s\t", headbuf);
        for (;;) {
                if (fgets(linbuf, (int)sizeof(linbuf), stdin) == NULL)
                        break;
                if (
#ifdef  KJI
                    !NCisshift(linbuf[1]) &&
#endif
                /* if */        linbuf[0] == '.')
                {
                        if (linbuf[1] == 'S' && linbuf[2] == 'H')
                                break;
                        if (linbuf[1] == 's' && linbuf[2] == 'h')
                                break;
			/* skip troff comments: .\" or ...\" */
			if ( linbuf[1] == '\\' && linbuf[2] == '"' ||
				(linbuf[1] == '.'  && linbuf[2] == '.' &&
				 linbuf[3] == '\\' && linbuf[4] == '"') )
			continue;

								/* start 001 */
			/* skip nroff/troff commands (only those that
			 * do not contain parameters, or those that
			 * have parameters that do not contain manpage
			 * information).
			 */
			/* skip ".sp[ <parameter>]" lines */
			if (linbuf[1] == 's' && linbuf[2] == 'p')
				continue;
								/* end   001 */

								/* start 002 */
			/* skip RSML (rsml(1)) macros calls (only those that
			 * do not contain parameters, or those that
			 * have parameters that do not contain manpage
			 * information).
			 */
			/* skip ".wH[ <parameter>]..." lines */
			if (linbuf[1] == 'w' && linbuf[2] == 'H')
				continue;
								/* end   002 */
                }
                trimln(linbuf);
                if (intro) {
                        split(linbuf, name);
                        continue;
                }
                if (i != 0)
                        NLprintf(" ");
                i++;
                NLprintf("%s", linbuf);
        }
        NLprintf("\n");
}

trimln(cp)
        register char *cp;
{

        while (*cp)
                cp++;
        if (*--cp == '\n')
                *cp = 0;
}

#ifndef KJI
doname(name)
        char *name;
{
        register char *dp = name, *ep;

again:
        while (*dp && *dp != '.')
                putchar(*dp++);
        if (*dp)
                for (ep = dp+1; *ep; ep++)
                        if (*ep == '.') {
                                putchar(*dp++);
                                goto again;
                        }
        putchar('(');
        if (*dp)
                dp++;
        while (*dp)
                putchar (*dp++);
        putchar(')');
        putchar(' ');
}
#else /*        KJI     */
doname(name)
        char *name;
{
        register char *dp = name, *ep;

again:
        while (*dp && *dp != '.')
        {
                putchar(*dp);
                dp += NLchrlen(dp);
        }
        if (*dp)
                for (ep = dp+1; *ep; ep+=NLchrlen(ep))
                        if (*ep == '.')
                        {
                                putchar(*dp);
                                dp += NLchrlen(dp);
                                goto again;
                        }
        putchar('(');
        if (*dp)
                dp += NLchrlen(dp);
        while (*dp)
        {
                putchar (*dp);
                dp += NLchrlen(dp);
        }
        putchar(')');
        putchar(' ');
}
#endif /* KJI */

split(line, name)
        char *line, *name;
{
        register char *cp, *dp;
        char *sp, *sep;


        cp = (void*)NLstrchr((void*)line, '-');
        if (cp == 0)
                return;
        sp = cp + 1;
#ifdef  KJI
        for (--cp; ((cp -1 <= line && !NCisshift(cp[-1])) || cp == line) &&
                   *cp == ' ' || *cp == '\t' || *cp == '\\'; cp--)
                ;
#else
        for (--cp; *cp == ' ' || *cp == '\t' || *cp == '\\'; cp--)
                ;
#endif
        *++cp = '\0';
        while (*sp && (*sp == ' ' || *sp == '\t'))
                sp++;
        for (sep = "", dp = line; dp && *dp; dp = cp, sep = "\n") {
                cp = (void*)NLstrchr((void*)dp, ',');
                if (cp) {
                        register char *tp;

                        for (tp = cp - 1; *tp == ' ' || *tp == '\t'; tp--)
                                ;
                        *++tp = '\0';
                        for (++cp; *cp == ' ' || *cp == '\t'; cp++)
                                ;
                }
                NLprintf("%s%s\t", sep, dp);
                dorefname(name);
                NLprintf("\t%s", sp);
        }
}

#ifndef KJI
dorefname(name)
        char *name;
{
        register char *dp = name, *ep;

again:
        while (*dp && *dp != '.')
                putchar(*dp++);
        if (*dp)
                for (ep = dp+1; *ep; ep++)
                        if (*ep == '.') {
                                putchar(*dp++);
                                goto again;
                        }
        putchar('.');
        if (*dp)
                dp++;
        while (*dp)
                putchar (*dp++);
}
#else
dorefname(name)
        char *name;
{
        register char *dp = name, *ep;

again:
        while (*dp && *dp != '.')
        {
                putchar(*dp);
                dp += NLchrlen(dp);
        }
        if (*dp)
                for (ep = dp+1; *ep; ep+=NLchrlen(ep))
                        if (*ep == '.')
                        {
                                putchar(*dp);
                                dp += NLchrlen(dp);
                                goto again;
                        }
        putchar('.');
        if (*dp)
                dp += NLchrlen(dp);
        while (*dp)
        {
                putchar (*dp);
                dp += NLchrlen(dp);
        }
}
#endif  /*      KJI     */
