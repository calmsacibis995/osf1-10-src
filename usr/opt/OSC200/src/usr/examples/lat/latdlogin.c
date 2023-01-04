
/*
 * l a t d l o g i n
 *
 * Description: This sample program acts as a LAT to DLOGIN gateway.
 *              With it, a user at a terminal connected to a terminal
 *              server can log into remote DECnet nodes without
 *              having to log into (or even have an account on) the
 *              local system.
 *
 * To compile:  cc -o latdlogin latdlogin.c
 *
 * Setup:       This program requires that DECnet be installed on
 *              your system.  It is necessary to dedicate one or
 *              more lat ttys to the service.
 *
 *              1. As super user, copy latdlogin to /usr/sbin
 *              2. Add LAT/dlogin Gateway service
 *                 latcp -A -a svcdlgn -i "LAT/dlogin Gateway" -o
 *              3. Add dedicated tty process entry into /etc/inittab
 *                 lattty14:234:respawn:/usr/sbin/latdlogin /dev/tty14 \
 *                 dloginsvc
 *              4. Make the new entry to take effect
 *                 /sbin/init q
 *              5. Login from terminal server
 *                 LOCAL> connect svcdlgn node hostname dest loginhost
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
                perror ("usage: latdloign tty service");
                exit(1);
          }

          if ((tty = (char *)malloc(strlen(argv[1])+sizeof(_PATH_DEV)+1))
                ==NULL) {
                perror ("malloc() failed, no buffer available");
                exit(1);
          }

          strcpy(tty, argv[1]);
          chown(tty, 0, 0);
          chmod(tty, 0622);

          /*
           * open LAT line
           */

          if ((latfd = open(tty, O_RDWR|O_NONBLOCK)) < 0) {
                perror(open);
                exit(1);
          }

          if ((fcntl(latfd, F_SETFL, fcntl(latfd, F_GETFL, 0) &
                ~(FNONBLOCK|FNDELAY))) == -1) {
                perror("fcntl() failed at command ,F_SETFL");
                exit(1);
          }

          if ((fcntl(latfd, F_SETFD, 0)) == -1) {
                perror("fcntl() failed at command F_SETFD");
                exit(1);
          }

         /*
           * do the LIOCBIND ioctl
           */

          bzero(&ttyi, sizeof(struct latioctl_ttyi));
          strcpy(ttyi.li_service, argv[2]);
          if (ioctl(latfd, LIOCBIND, &ttyi) < 0) {
                perror("ioctl() failed at command  LIOCBIND");
                exit(1);
          }

          /*
           * get DESTINATION field
           */
          if ((ioctl(latfd, LIOCTTYI, &ttyi)) < 0) {
                perror("ioctl() failed at command LIOCTTYI");
                exit(1);
          }

          (void) dup2(latfd, 0);
          (void) dup2(latfd, 1);
          (void) dup2(latfd, 2);
          if (latfd > 2)
                (void) close(latfd);

          /*
           * set tty flags & mode
           */

          if((tcgetattr(0, &termios)) == -1) {
                perror("tcgetattr() failed");
                exit(1);
          }

          termios.c_cflag = TTYDEF_CFLAG;
          termios.c_iflag = TTYDEF_IFLAG;
          termios.c_lflag = TTYDEF_LFLAG;
          termios.c_oflag = TTYDEF_OFLAG;
          termios.c_cc[VSUSP] = _POSIX_VDISABLE;

          if((tcsetattr(0, TCSAFLUSH, &termios)) == -1) {
                perror("tcsetattr() failed at command TCSAFLUSH");
                exit(1);
          }

          (void) signal(SIGINT, SIG_DFL);
          (void) signal(SIGHUP, SIG_DFL);

          for (np = (char *)ttyi.li_service; *np; np++) {
                if (isupper(*np))
                      *np = tolower(*np);
          }

          execl("/usr/bin/dlogin","dlogin",ttyi.li_service,0);
          perror("/usr/bin/dlogin");
          exit(1);
}
