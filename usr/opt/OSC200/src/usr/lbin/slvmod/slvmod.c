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
static char *rcsid = "@(#)$RCSfile: slvmod.c,v $ $Revision: 1.1.10.2 $ (DEC) $Date: 1993/05/12 16:22:05 $";
#endif
#if !defined(lint) && !defined(_NOIDENT)

#endif

#if defined(LIBC_SCCS) && !defined(lint)

#endif /* LIBC_SCCS and not lint */

#include <sys/file.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <grp.h>
#include <errno.h>
/* constant for max advertised or documented pty supported */
/* (versus number of ptys supported internally - 3162)     */
#define MAX_PTY_ADV 816

main(ac,av)
	int ac;
	char **av;
{
	register char *line = "/dev/ttyXX";
	char *cp1 = "pqrstuvwxyzabcefghijklmnoABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char *cp2 = "0123456789abcdef";
	char *cp3 = "ghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char char_8, char_9;
	register int  ruid, ttygid;
	dev_t  dev;
	struct group *gr;

	if ((gr = getgrnam("tty")) != NULL)
		ttygid = gr->gr_gid;
	else
		ttygid = -1;
	ruid = getuid();

	if ((dev = (dev_t)ioctl(1,ISPTM,0)) < 0){
		exit(-1);
	}
	if (minor(dev) < MAX_PTY_ADV) {
		char_8 = cp1[(minor(dev))/16];	
		char_9 = cp2[(minor(dev))%16];
	}
	else {
		char_8 = cp1[(minor(dev) - MAX_PTY_ADV)/46];	
		char_9 = cp3[(minor(dev) - MAX_PTY_ADV)%46];
	}
	line[8] = char_8;
	line[9] = char_9;
	(void) chown(line, ruid, ttygid);
	(void) chmod(line, 0620);
	(void) revoke(line);
	exit(0);
}
