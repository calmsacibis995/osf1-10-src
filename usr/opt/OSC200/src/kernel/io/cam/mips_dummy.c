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
static char *rcsid = "@(#)$RCSfile: mips_dummy.c,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1992/08/31 12:17:54 $";
#endif
/* dummy files to make CAM builds for ALpha happy
 */

#ifdef __alpha
clean_dcache(x, y)
{
panic("clean_dcache");
}

int PHYS_TO_K0(x)
int x;
{
panic("PHYS_TO_K0");
}

int PHYS_TO_K1(x)
int x;
{
panic("PHYS_TO_K1");
}

int IS_SYS_VA(x)
int x;
{
panic("IS_SYS_VA");
}

/*
 * Convert an ASCII string of hex characters to a binary number.
 */
xtob(str)
        char *str;
{
        register int hexnum;

        if (str == 0 || *str == 0)
                return(0);
        hexnum = 0;
        if (str[0] == '0' && str[1] == 'x')
                str = str + 2;
        for ( ; *str; str++) {
                if (*str >= '0' && *str <= '9')
                        hexnum = hexnum * 16 + (*str - 48);
                else if (*str >= 'a' && *str <= 'f')
                        hexnum = hexnum * 16 + (*str - 87);
                else if (*str >= 'A' && *str <= 'F')
                        hexnum = hexnum * 16 + (*str - 55);
        }
        return(hexnum);
}

/* reverse a string in place
 * stolen from K&R 1 page 59
 */
static void
reverse(s)
char s[];
{
	int c, i, j;

	for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}
#endif
