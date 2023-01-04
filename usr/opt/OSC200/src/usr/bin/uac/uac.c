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
static char *rcsid = "@(#)$RCSfile: uac.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1992/11/06 15:48:07 $";
#endif

/*
 *			usage: uac: [sp] [01]
 */

#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/dir.h>
#include <sys/proc.h>
#include <sys/exec.h>

main(argc, argv)
long argc;
char *argv[];
{
	int err = 0, sflg = 0, val;
	unsigned arg;
	char *ap = (char *)0;

	if(--argc < 1 || argc > 2)
		usage();

	ap = *++argv;
	switch(*ap) {
	      case 's':
		arg = GSI_UACSYS;
		if(--argc) {
			arg = SSIN_UACSYS;
			sscanf( *++argv, "%x", &val);
			++sflg;
		}
		break;
	      case 'p':
		arg = GSI_UACPARNT;
		if(--argc) {
			arg = SSIN_UACPARNT;
			sscanf( *++argv, "%x", &val);
			++sflg;
		}
		break;
	      default:
		usage();
	}
	
	if(sflg) {
		if(val == 0)
			val |= UAC_NOPRINT;
		else if(val == 1)
			val &= ~UAC_NOPRINT;
		else
			usage();
		err = setinfo(arg, val);
	} else
		err = getinfo(arg);

	if(err < 0)
		perror("uac");
	exit(err);
}

getinfo(arg)
unsigned arg;
{
	int start = 0, buf = 0, err = 0;
	char *str = (arg == GSI_UACSYS) ? "sys" : "parent";

	err = getsysinfo((long)arg, (caddr_t)&buf, (long)sizeof(buf),
						(caddr_t )&start, (long)0);
	if(err == 0)
		printf(" %s unavailable\n", str);
	else if(err == 1)
		printf("%s is %s\n", str, (buf & UAC_NOPRINT) ? "off" : "on");
	return(err);
}


setinfo(arg, val)
unsigned arg;
int val;
{
	int buf[2];

	buf[0] = arg;
	buf[1] = val;
	return(setsysinfo((long)SSI_NVPAIRS, (caddr_t)buf, 1L, 0L, 0L));
}

usage()
{
	printf("usage: uac [sp] [01]\n");
	exit(1);
}

