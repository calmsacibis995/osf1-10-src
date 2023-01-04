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
static char *rcsid = "@(#)$RCSfile: lattelnet.c,v $ $Revision: 1.1.9.3 $ (DEC) $Date: 1993/11/08 16:38:27 $";
#endif

/*
 * Program lattelnet.c
 *
 * MODULE DESCRIPTION:
 *
 * LAT/telnet gateway
 * Common data storage
 *
 * Another fine product by
 * Networks & Communications Software Engineering
 *
 * IDENT HISTORY:
 *
 * 1.00 22-Apr-88
 *
 *	11-Oct-1989	Matt Thomas
 *	Add perror("/usr/ucb/telnet") for errors on execl.
 *	Add trailing to execl call list.
 *	Change '||' to '|' for allflags.
 *	Restore default handling for SIGHUP.
 */

/*
 *	l a t t e l n e t
 *
 * This is the program for the DEC OSF/1 LAT/Telnet Gateway.  
 * It accepts connect requests from LAT terminals and exec's telnet.
 *
 * To compile: cc -o lattelnet lattelnet.c
 * 
 * See 'Configuring Your Network Software' for LAT and LAT/Telnet service set 
 * up.
 *
 * To access the LAT/Telnet service from a LAT terminal:
 *
 *	CONNECT service_name [NODE hostname [DEST telnet_hostname]]
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/termios.h>
#include <dec/lat/lat.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <ttyent.h>
#include <stdio.h>
#include <syslog.h>
#include <sysexits.h>
#include <paths.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>

/*
 *  Don't set the SHELL var so that the escape (!) at the telnet prompt
 *  will not escape to root shell.
 */
char shell[20] = "SHELL=";
char *env[2] = {shell,NULL};
char *targs[2] = {NULL,NULL};

#define _PATH_TELNET	"/usr/bin/telnet"

main(
    int argc,
    char *argv[])
{
    int latfd;
    struct termios termios;
    struct latioctl_ttyi ttyi;
    char hostname[MAXHOSTNAMELEN];
    char *tty, *image, *pathdev;

    setsid();

    openlog("lattelnet", LOG_PID, LOG_DAEMON);
    (void) signal(SIGINT, SIG_IGN);
    (void) signal(SIGHUP, SIG_IGN);
    gethostname(hostname, sizeof(hostname));
    pathdev = _PATH_DEV;

    if (argc < 3) {
	syslog(LOG_INFO, "usage: lattelnet tty service-id [command-path]\n");
	exit(EX_USAGE);
    }

    if (argc == 4) {
	image = argv[3];
    } else {
	image = _PATH_TELNET;
    }

    if ((targs[0] = strrchr(image, '/')) != NULL)
	targs[0]++;
    else
	targs[0] = image;

    if ( (strncmp(pathdev, argv[1], 4) != NULL) ) {
    	tty = (char *) malloc(strlen(argv[1]) + sizeof(_PATH_DEV) + 1);
    	strcpy(tty, _PATH_DEV);
    	strcat(tty, argv[1]);
    } else {
    	tty = (char *) malloc(strlen(argv[1]) + 1);
    	strcpy(tty, argv[1]);
    }

    chown(tty, 0, 0);
    chmod(tty, 0622);
    /* 
     * open LAT line 
     */
    latfd = open(tty, O_RDWR|O_NONBLOCK);
    if (latfd < 0) {
	syslog(LOG_INFO, "%s: %m\n", argv[1]);
	exit(EX_OSFILE);
    }
    if (tty_check(latfd, tty) != 0) {
        syslog(LOG_INFO, "TTY is not an appropriate LAT tty or is already configured/used.\n");
        exit(EX_UNAVAILABLE);
    }

    (void) fcntl(latfd, F_SETFL, fcntl(latfd, F_GETFL, 0) & ~(FNONBLOCK|FNDELAY));
    (void) fcntl(latfd, F_SETFD, 0);
    /* 
     * do the LIOCBIND ioctl
     */ 
    bzero(&ttyi, sizeof(struct latioctl_ttyi));
    strcpy(ttyi.li_service, argv[2]);
    if (ioctl(latfd, LIOCBIND, &ttyi) < 0) {
	syslog(LOG_INFO, "%s: service %s: service name %s: CHECK THAT THIS SERVICE NAME IS DEFINED WITH LATCP: %m\n", argv[1], argv[0], argv[2], errno);
	exit(EX_UNAVAILABLE);
    }

    /* 
     * get DESTINATION field 
     */ 
    (void) ioctl(latfd, LIOCTTYI, &ttyi);

    (void) dup2(latfd, 0);
    (void) dup2(latfd, 1);
    (void) dup2(latfd, 2);
    if (latfd > 2)
	(void) close(latfd);

    /* 
     * set tty flags & mode
     */
    tcgetattr(0, &termios);
    termios.c_cflag = TTYDEF_CFLAG;
    termios.c_iflag = TTYDEF_IFLAG;
    termios.c_lflag = TTYDEF_LFLAG;
    termios.c_oflag = TTYDEF_OFLAG;
    termios.c_cc[VSUSP] = _POSIX_VDISABLE;
    tcsetattr(0, TCSAFLUSH, &termios);

    (void) signal(SIGINT, SIG_DFL);
    (void) signal(SIGHUP, SIG_DFL);

    /*  Set the user id to user "nobody" to specify a known user  */
    if (setuid((uid_t)65534) == -1) {
        perror("setuid");
        syslog(LOG_INFO, "lattelnet: error: unable to change group id of process.\n");
        exit(EX_OSERR);
    }

    if (ttyi.li_service[0] != 0) {
	u_char *np;
        printf("\n   LAT connection to %s established\n \
	Now connecting via telnet gateway to %s\n",
                hostname,ttyi.li_service);
	for (np = ttyi.li_service; *np; np++) {
	    if (isupper(*np))
		*np = tolower(*np);
	}
	execl(image, targs[0], ttyi.li_service, NULL);
    } else {
	printf("\nLAT connection to %s established\n", hostname);
	execve(image, targs, env);
    }
    perror(image);
    exit(EX_OSERR);
}
int tty_check(latfd,  port_name )
int latfd;
char *port_name;
{
     struct latioctl_port latioctl_port;
     struct stat statbuf;

     bzero(&latioctl_port, sizeof(latioctl_port));
     bzero(&statbuf, sizeof(statbuf));

     if (stat(port_name, &statbuf) < 0)
           return;
     latioctl_port.lp_devno = (dev_t)statbuf.st_rdev;
     latioctl_port.lp_flags = 0;

     if (ioctl(latfd, LIOCGPORT, &latioctl_port) < 0)  {
           if (errno == ENODEV) {
		syslog(LOG_INFO, "No such device.\n");
                exit(2);
           }
           else  if (errno == ENOTCONN) {
                            return 1;
                 }

           else if (errno == ERANGE) {
		syslog(LOG_INFO, "Minor number on port %s out of range.\n", port_name);
                return 1;
                }
           else {
                syslog(LOG_INFO,"LIOCGPORT ioctl failed.  errno=%d.\n",errno);
		return 1;
           }

     }

     if (latioctl_port.lp_flags & LATIOCTL_PORT_OUTBOUND)
	return 1; 
     else return 0;
}	
