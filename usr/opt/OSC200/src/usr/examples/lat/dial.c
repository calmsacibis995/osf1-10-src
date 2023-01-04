
/*
 * d i a l 
 *
 * Description: This sample program illustrates the use of a LAT Host
 *              Initiated Connection.  It connects /dev/ttyxx to a DEC
 *              SCHOLAR modem that is attached to the port "LAT_PORT"
 *              on the DECserver 200 "LAT_SERVER".  After a successful
 *              open, it autodials a phone number to a host computer
 *              and emulates a terminal connected to the host computer.
 *
 * Setup:       Before invoking 'dial', LAT_SERVER and LAT_PORT must be
 *              defined by the latcp command:
 *
 *              # /usr/sbin/latcp -A -p ttyxx -H LAT_SERVER -R LAT_PORT -Q 
 *
 *              Access to '/dev/ttyxx' must be Read/Write for the user
 *              of 'dial'.
 *
 * To compile:  cc -o dial dial.c
 *
 * Usage:       /usr/sbin/dial phone_number /dev/ttyxx
 *
 * Comments:    In terminal emulation:
 *                ^](CTRL/]) for escape character
 *                ^]? for help
 *                ^]b to send break signal
 */

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h> 
#include <sys/time.h> 
#include <sys/file.h> 
#include <sys/ioctl.h>
#include <sys/termios.h>

/* 
 * For DEC SCHOLAR modem (See SCHOLAR 2400 Modem Owner's Manual)
 * byte 1:     1 (CTRL/A) - autodialer
 * byte 2:     P - pulse dialing  T - tone dialing
 * last byte:    ! - start dialing
 */

u_char nl[20]={0x01, 'T',1,2,3,4,5,6,7,'!'};

int fd;    
void nodial();
extern errno;
void resettty();


main(argc,argv)
int argc;
char *argv[];
{
       char buf[BUFSIZ];    
       int len, flags;
       struct termios tty_termios; 

       /* 
        * Open reverse LAT device. 
        */

       if ( (fd = open(argv[2],O_RDWR)) < 0 ) {
    	      perror(argv[0]);
    	      exit(1);
       }
   
       /* get current line attributes */
       if ((tcgetattr(fd,&tty_termios) == -1)){
              perror("tcgetattr() failed");
              exit(1);
       }

       /* If CLOCAL happened not to be set, then set it. We need to 
        * be in "local" mode to talk to the modem".
        */
       if ((tty_termios.c_cflag & CLOCAL) == 0) {
              tty_termios.c_cflag |= CLOCAL;
              if ((tcsetattr (fd, TCSANOW, &tty_termios) == -1)) {
                     perror("tcsetattr() failed at TCSANOW");
                     exit(1);
              }
       }

       /* turn off O_NONBLOCK, we don't need it any more */
       flags = fcntl (fd, F_GETFL);
       if (flags == -1) {
              perror("fcntl() failed at command F_GETFL");
              exit(1);
       }

       if ((fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) == -1)) {
              perror("fcntl() failed at command F_SETFL");
              exit(1);
       }
 
       len = strlen(argv[1]);       /* get phone number  */
       strcpy(&nl[2], argv[1]);
       nl[len+2] = '!';             /* ! for start dialing */
       write(0, "Dialing ", 8);     /* print 'Dialing phone#, wait...' */
       write(0, argv[1], len);
       write(0, ", wait... ", 10);  
       write(fd, nl, len+3);  

       signal(SIGALRM, nodial);     /* Give call 60 seconds to go thru */
       alarm(60);
       read(fd, buf, 80);      
       signal(SIGALRM, SIG_IGN);
       read(fd, buf, 80); 
       read(fd, buf, 80);
       printf("\n\n%s\n", buf); 
       if (buf[0] == 'A' || buf[0] == 'a') {
              alarm(0);
              termmain();
       }
}

void nodial()
{
       char buf[BUFSIZ];    /* Read/write buffer */

       printf("\nDial out failed\n");
       exit(1);
}


/*
 * The remainder of this program is a terminal emulator.
 */

struct sgttyb Isgttyb, sgttyb, sgttyb1;
struct tchars Itchars, tchars1;
struct ltchars Iltchars, ltchars;
int fd, outfile, ret, ret1;    
int  readfd, writefd, exception;
struct timeval timeout;


termmain()
{

       char buf[BUFSIZ];
       char *bufptr;
       int on = 1;
       struct termios tty_termios;
     

       if (ioctl(0, TIOCGETP, &Isgttyb) < 0) 
             perror("ioctl() failed at command TIOCGETP");
       if (ioctl(0, TIOCGETC, &Itchars) < 0) 
             perror("ioctl() failed at command  TIOCGETC");
       if (ioctl(0, TIOCGLTC, &Iltchars) < 0) 
             perror("ioctl() failed at command TIOCGLTC");

       /*
        * Set the terminal into CBREAK | NOECHO | -CRMOD mode so
        * that we can handle character buffering and echo ourselves. We will
        * also disable all special character handling except ^S and ^Q.
        */
       sgttyb = Isgttyb;
       sgttyb.sg_flags |= CBREAK;
       sgttyb.sg_flags &= ~(ECHO | CRMOD);
       if (ioctl(0, TIOCSETP, &sgttyb) < 0) 
             perror("ioctl() failed at command TIOCSETP");
       tchars1 = Itchars;
       tchars1.t_intrc = tchars1.t_quitc = tchars1.t_eofc = tchars1.t_brkc = -1;
       if (ioctl(0, TIOCSETC, &tchars1) < 0) 
             perror("ioctl() failed at command TIOCSETC");
       ltchars.t_suspc = ltchars.t_dsuspc = ltchars.t_rprntc = ltchars.t_flushc
                       = ltchars.t_werasc = ltchars.t_lnextc = -1;
       if (ioctl(0, TIOCSLTC, &ltchars) < 0) 
             perror("ioctl() failed at command TIOCSLTC");

       if (ioctl(fd, TIOCGETP, &sgttyb1) < 0) 
             perror("ioctl() failed at command TIOCGETP");
       sgttyb1.sg_flags |= RAW; 
       sgttyb1.sg_flags &= ~ECHO;
       if (ioctl(fd, TIOCSETP, &sgttyb1) < 0) 
             perror("ioctl() failed at command TIOCSETP");
       if (ioctl(fd, FIONBIO, &on) < 0) 
             perror("ioctl() failed at command FIONBIO");

       if ((tcgetattr(fd,&tty_termios) == -1))
             perror("tcgetattr() failed");

       if ((tty_termios.c_cflag & CLOCAL) != 0) {
             tty_termios.c_cflag &= ~CLOCAL;
             if ((tcsetattr (fd, TCSANOW, &tty_termios) == -1)) 
                    perror("tcsetattr() failed at TCSANOW");
       }

       signal(SIGHUP, resettty);
       signal(SIGINT, resettty);
       signal(SIGQUIT, resettty);
       signal(SIGBUS, resettty);
       signal(SIGSEGV, resettty);

       printf("escape character: ^];   help: ^]?\r\n\n");
       for (;;) {
           readfd = exception = (1 << fd) + (1 << 0);
           errno=0;
           if ((select(fd+1, &readfd, 0, &exception, 0)) > 0) {
                if (readfd & (1 << fd)) {
                       if ((ret = read(fd,buf,BUFSIZ)) <= 0) {
                             printf("\nEXIT! ");
                             resettty();
                       }
                       ret1 = write(0,buf,ret);     
                       ret -= ret1;
                       bufptr = buf + ret1;

                       while (ret) {
                              writefd = 1 << 0;
                              select(fd+1, 0, &writefd, 0, 0);
                              if (writefd & (1 << 0)) {
                                     ret1 = write(0,bufptr,ret);     
                                     ret -= ret1;
                                     bufptr = bufptr + ret1;
                              }
                       }
                }
                if (readfd & (1 << 0)) {
                       ret = read(0,buf,BUFSIZ);
                       if (*buf == 0x1d) {
                               if ( !(*buf = esccommands()))
                                        continue;
                       }
                       write(fd,buf,ret);     
                }
                if (exception & (1 << fd)) {
                       printf("exception: \n");
                       printf("\n\nEXIT!\n ");
    	               resettty();
                }
           }
           else {
               perror("select: EXIT");
               resettty();
           }
       }
}

void resettty()
{
       int off = 0;

       /*
        * Restore the terminal characteristics to their state before the
        * current session was entered.
        */

       if (ioctl(0, TIOCSETP, &Isgttyb) < 0) 
             perror("ioctl() failed at command TIOCSETP");
       if (ioctl(0, TIOCSETC, &Itchars) < 0) 
             perror("ioctl() failed at command TIOCSETC");
       if (ioctl(0, TIOCSLTC, &Iltchars) < 0) 
             perror("ioctl() failed at command TIOCSLTC");
       close(fd);
       printf("\nDEC OSF/1 LAT dial out disconnected\n\n");
       exit(0);
}


/*
 *        e s c c o m m a n d s
 *
 * for input character:
 * ?:        this menu
 * p:         escape to local command mode
 * b:         send a break
 * esc:     send ^]
 * all others:     exit escape mode
 *
 */
esccommands()
{
       char ch;
       int ret;

       puts("\r\n");
       printf("\r\n\t?\tthis menu\r\n");
       printf("\tp\tescape to local command mode (? for help)\r\n");
       printf("\tb\tsend a break\r\n");
       printf("\tescape\tsend ^]\r\n");
       printf("\tothers\texit escape mode\r\n");
       printf("\nSelect one only - 'p', 'b', escape, '?'   ");
       ret = read(0,&ch,1);
       switch(ch) 
       {
          case 'p':
                   localcommands();
                   break;

          case 'b':
                   if (ioctl(fd, TIOCSBRK, 0) < 0) 
                        perror("ioctl() failed at command TIOCSBRK");
                   else 
                        printf("\r\nSend a break successfully\r\n");
                   break;
      
          case 0x1b:
                   printf("\rYou selected  'escape' \r\n");
                   return (0x1d);

          case '?':
                   printf("\r\n\t?\tthis menu\r\n");
                   printf("\tp\tescape to local command mode (? for help)\r\n");
                   printf("\tb\tsend a break\r\n");
                   printf("\tescape\tsend ^]\r\n");
                   printf("\tothers\texit escape mode\r\n");

       }
       return(0);
}


/*
 *        l o c a l c o m m a n d s
 */
extern char **environ;
localcommands()
{
       char command[512];
       int notdone = 1,pid;


       /*
        * Reset the terminal to its original state.
        */
       if (ioctl(0, TIOCSETP, &Isgttyb) < 0) 
             perror("ioctl() failed at command TIOCSETP");
       if (ioctl(0, TIOCSETC, &Itchars) < 0) 
             perror("ioctl() failed at command TIOCSETC");
       if (ioctl(0, TIOCSLTC, &Iltchars) < 0) 
             perror("ioctl() failed at command TIOCSLTC");

       printf("\r\n\n\t\tLocal Command Menu\r\n\n");
       printf("\tsuspend\tsuspends LAT\n");
       printf("\texit\texits\n");
       printf("\t^D\texits\n");
       printf("\tcmd\tinvoke shell to execute command\n");
       printf("\t\tblank line resumes LAT\n\n");

       printf("\r\n");
       while (notdone) {
          printf("\n\nlocal command> ");
          if (gets(command) == NULL) {
               printf("\nEXIT! ");
               resettty();
          }
          switch (command[0])
          {
              case '?':
                      printf("\tsuspend\tsuspends LAT\n");
                      printf("\texit\texits\n");
                      printf("\t^D\texits\n");
                      printf("\tcmd\tinvoke shell to execute command\n");
                      printf("\t\tblank line resumes LAT\n\n");

              case '\0':
                      notdone = 0;
                      break;

              default:
                     /*
               	      * Check for special commands that we handle locally.
                      */
                     if (strcmp(command, "suspend") == 0) {
                            kill(getpid(), SIGTSTP);
                            break;
                     }

              	     if (strcmp(command, "exit") == 0) {
                            printf("\nEXIT! ");
                   	    resettty();
                     }

                     if ((pid = fork()) < 0) {
                            perror("LAT server - fork failed");
                            break;
                     }
                     if (pid == 0) {
                            if (execle(getenv("SHELL"), getenv("SHELL"), "-c", 
                                  command, 0, environ) < 0) {
                                  perror("LAT server - unable to exec shell");
                                  exit(1);
                            }
                     }

                     wait(0);
                     break;
          }
       }

       /*
        * Reset the terminal to its state on entry.
        */

       if (ioctl(0, TIOCSETP, &sgttyb) < 0) 
             perror("ioctl() failed at command TIOCSETP");
       if (ioctl(0, TIOCSETC, &tchars1) < 0) 
             perror("ioctl() failed at command TIOCSETC");
       if (ioctl(0, TIOCSLTC, &ltchars) < 0) 
             perror("ioctl() failed at command TIOCSLTC");
}

