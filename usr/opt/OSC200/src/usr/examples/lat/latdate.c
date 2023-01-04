
/*
 * l a t d a t e
 *
 * Description: This sample program illustrates the use of multiple
 *              lat services.  When a user at a terminal connected
 *              to a terminal server issues a "CONNECT showdate" command
 *              the date & time will be printed on his terminal.
 *
 * To compile:  cc -o latdate latdate.c
 *              
 * Setup:       1. As super user, copy latdate to /usr/sbin/
 * 		2. Add showdate service
 *		   latcp -A -a showdate -i "LAT/date showdate service" -o
 *		3. Add dedicated tty process entry into /etc/inittab
 *		   lattty09:234:respawn:/usr/sbin/latdate  /dev/tty09 showdate
 *		4. Make new entry to take effect
 *		   /sbin/init q
 *		5. Login from terminal server and get the date & time
 *		   connect showdate
 *
 */

#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/termios.h>
#include <dec/lat/lat.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <paths.h>

main( int argc, char *argv[])
{
    int latfd;
    struct termios termios;
    struct latioctl_ttyi ttyi;
    char *tty, *np;

    if (argc < 3) {
	perror ("usage: latdate tty service");
	exit(1);
    }

    tty = (char *) malloc(strlen(argv[1]) + sizeof(_PATH_DEV) + 1);

    strcpy(tty, argv[1]);
    chown(tty, 0, 0);
    chmod(tty, 0622);

    /* 
     * open LAT line 
     */
    latfd = open(tty, O_RDWR|O_NONBLOCK);
    if (latfd < 0) {
        perror(open);
	exit(1);
    }

    (void) fcntl(latfd,F_SETFL,fcntl(latfd,F_GETFL,0) & ~(FNONBLOCK|FNDELAY));
    (void) fcntl(latfd, F_SETFD, 0);
    /* 
     * do the LIOCBIND ioctl
     */ 
    bzero(&ttyi, sizeof(struct latioctl_ttyi));
    strcpy(ttyi.li_service, argv[2]);
    if (ioctl(latfd, LIOCBIND, &ttyi) < 0) {
	perror(ioctl);
	exit(1);
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

    for (np = (char *)ttyi.li_service; *np; np++) {
        if (isupper(*np))
            *np = tolower(*np);
    }
    execl("/bin/date","date",NULL);
    perror("/bin/date");
    exit(1);
}
