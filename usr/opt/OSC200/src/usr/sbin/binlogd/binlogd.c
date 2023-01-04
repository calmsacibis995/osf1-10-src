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
static char *rcsid = "@(#)$RCSfile: binlogd.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/06/25 22:32:39 $";
#endif



/*
 *  binlogd -- system binary event log daemon
 */


#include <sys/secdefines.h>

#include <stdio.h>
#include <strings.h>

#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <dec/binlog/binlog.h>
#include <dec/binlog/binlogd.h>


#define	dprintf		if (dbug) printf

#define FROMLOCAL   1
#define FROMNET     2

char *ConfFilePath = "/etc/binlog.conf";     /* binlogd config file name */
char *SavedPID = "/var/run/binlogd.pid";     /* file to save our PID in */

struct configfile  *cflist = (struct configfile *)0;
struct mailbox     *mblist = (struct mailbox *)0;

int  dbug = 0;			        /* debug flag */
char myhostname[MAXHOSTNAMELEN+1];	/* our hostname */
char *workbuf = (char *)0;              /* the buffer we read everything into */
int  workbufsize = 0;
int  b_mask, n_mask, m_mask;   /* select(2) masks */
int  b_fd, n_fd, m_fd;         /* file descriptors */
int  mb_connects;              /* active mailbox connections */
long port;                     /* the binlog/udp port number */


void terminate_nicely();
void initialize();


main(argc, argv)
	int argc;
	char **argv;
{
    register char *p;
    register int rval;
    struct hostent *rh;
    int selmask;
    int nc;

    dprintf("Parse command line\n");
    while (--argc > 0) {
	p = *++argv;
	if (p[0] != '-')
		rtfm();
	switch (p[1]) {
	   case 'f':		/* configuration file */
		    if (p[2] != '\0')
			    ConfFilePath = &p[2];
		    break;

	   case 'd':		/* debug */
		    dbug++;
		    break;

	   default:
		    rtfm();
	}
    }

    if (!dbug)  {                            /* make us into a daemon */
         switch (fork())  {
	     case   0:         break;
	     case  -1:         printf("binlogd: failed to fork, EXITING\n");
	     default :         exit(0);
	 }
         (void) setsid();
    }

    once_only_setup();  /* setup file/socket descriptors, malloc buffers */
    initialize();       /* process config file */
    dump_recovery();    /* process binary event log buffer saved from a dump */

    /*
     * m_mask/m_fd - local unix domain socket (mailbox)
     *
     * n_mask/n_fd - network socket (DGRAM) for forwarded event records
     *
     * b_mask/b_fd - /dev/kbinlog, local event log source
     */

    for (;;) {                                         /* event loop */
	dprintf("waiting in select(2) loop\n");
	selmask = m_mask | n_mask | b_mask;
	rval = 0;
	errno = 0;
	rval = select(20, (fd_set *) &selmask, (fd_set *) NULL,
				  (fd_set *) NULL, (struct timeval *) NULL);
        if (rval <= 0) {      /* was anything selected ? */
	      if (errno != EINTR)
		    dprintf("select() error\n");
              continue;
	}

	rval = 0;
	if (selmask & b_mask) {       /*  /dev/kbinlog has data to read */
	      rval = read(b_fd, workbuf, workbufsize);
	      dprintf("\tselect on /dev/kbinlog, bytes read: %d\n", rval);
              if ((rval < 0) && (errno != EINTR)) {
		     dprintf("\t/dev/kbinlog  bad read\n");
                     b_fd = 0;
		     b_mask = 0;  /* don't use /dev/kbinlog any more */
	      }
	      else
		     logevent(workbuf, rval, FROMLOCAL); /* process the data */
	}
	if (selmask & n_mask) {      /* incomming event records from network */
              rval = read(n_fd,workbuf,workbufsize); 
	      dprintf("\tselect on INET, bytes received: %d\n", rval);
	      if (rval > 0) {
		    logevent(workbuf, rval, FROMNET);
	      }
	      else if (rval < 0 && errno != EINTR)
		    dprintf("\tbad read on INET\n");

	} 
	if (selmask & m_mask) {        /* incomming mailbox control messages */
              rval = read(m_fd, workbuf,workbufsize); 
	      dprintf("\tselect on mailbox, bytes received: %d\n", rval);
	      if (rval > 0) {
		    mailbox_ctrl(workbuf, rval);
	      } else if (rval < 0 && errno != EINTR)
		    dprintf("\tbad read on mailbox\n");
	}
    }
}





rtfm()               /* command line syntax error help message */
{
   fprintf(stderr, "usage: binlogd [-d] [-fconfigfile]\n");
   exit(1);
}



/*
 *  Do setup/initialization that only can happen once.
*/
once_only_setup()
{
   int rval;
   FILE *pidfp;
   pid_t  mypid;
   struct servent *portid;
   struct hostent *hostid;
   struct sockaddr_un mb_addr;    /* UNIX domain socket for mailbox */
   struct sockaddr_in in_addr;    /* INET socket for recv'ing remote events */
   struct binlog_getstatus bsc;

   dprintf("once_only_setup()\n");

   b_mask = n_mask = m_mask = 0;       /* clear select masks */

   errno = 0;


   /* save our PID in a file for easy use with kill(8) and such */
   mypid = getpid();
   pidfp = fopen(SavedPID, "w");
   if (pidfp != NULL) {
	   fprintf(pidfp, "%d\n", mypid);
	   (void) fclose(pidfp);
   }


   /*
    * setup signal handling
    */
   (void) signal(SIGTERM, terminate_nicely);
   (void) signal(SIGALRM, SIG_IGN);
   if (dbug) {                         /* let us be killed easy in debug mode */
	(void) signal(SIGQUIT, terminate_nicely);
	(void) signal(SIGINT, terminate_nicely);
   } else {                           /* running as a daemon */
	(void) signal(SIGQUIT, SIG_IGN);
	(void) signal(SIGINT, SIG_IGN);
        (void) signal(SIGHUP, initialize);
   }


   /*
    * setup local host event source
    */
   if ((b_fd = open("/dev/kbinlog", O_RDONLY)) < 0)  {
           printf("binlogd: can't open /dev/kbinlog\n");
   }
   else  {
	   dprintf("\t/dev/kbinlog open'ed\n");
   }
   if (b_fd >= 0)  {
           b_mask = 1<<b_fd;      /* make select(2) mask */

	  /* Give the driver our pid so that buffer pointers can be
	   * moved after we do a read.
	   */
          if (ioctl(b_fd, BINLOG_SETPID, (char *)&mypid) < 0)
	          dprintf("\tcan't do BINLOG_SETPID on /dev/kbinlog\n");
   }


   /*
    * get some general network info that we'll need
    */
   portid = getservbyname("binlog", "udp");
   if (portid == 0)  {
           dprintf("\tbinlog/udp: unknown service\n");
   }
   else  {
	   port = portid->s_port;
   }
   dprintf("\tport being used: %d\n", ntohs(port));
   (void) gethostname(myhostname, sizeof(myhostname));
   hostid = gethostbyname(myhostname);
   if (hostid == 0)  {
           dprintf("\tcan't get local host info: %s\n", myhostname);
   }

   /*
    * setup network datagram socket for receiving remotely logged events
    */
   n_fd = socket(AF_INET, SOCK_DGRAM, 0);
   if ((n_fd >= 0)  &&  (portid != 0)) {
	   bzero((char *)&in_addr, sizeof(in_addr));
	   bcopy(hostid->h_addr, (char *)&in_addr.sin_addr, hostid->h_length);
	   in_addr.sin_family = AF_INET;
	   in_addr.sin_port = port;
	   if (bind(n_fd, (struct sockaddr *)&in_addr, sizeof(in_addr)) < 0)   {
		  dprintf("\tcan't bind to remote socket, errno: %d\n", errno);
		  (void)close(n_fd);
		  n_fd = -1;
	   }
	   else  {
		 dprintf("\tremote logging socket setup\n");
	   }
   }
   else
	   n_fd = -1;
   if (n_fd >= 0)
          n_mask = 1<<n_fd;      /* make select(2) mask */



   /*
    * setup Unix domain datagram socket for receiving mailbox control messages
    */
   mblist = 0;
   mb_connects = 0;
   (void) unlink(MAILBOX_CTRL_PATHNAME);
   mb_addr.sun_family = AF_UNIX;
   (void) strcpy(mb_addr.sun_path, MAILBOX_CTRL_PATHNAME);
   m_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
   if (m_fd < 0 || bind(m_fd, (struct sockaddr *)&mb_addr, 
	                     sizeof(struct sockaddr_un)) < 0 || 
		                   chmod(MAILBOX_CTRL_PATHNAME, 0660) < 0)   {
	  dprintf("\tcannot create mailbox control socket %s\n", 
						        MAILBOX_CTRL_PATHNAME);
   }
   else  {
          dprintf("\tmailbox control socket setup\n");
   }
   if (m_fd >= 0)
          m_mask = 1<<m_fd;      /* make select(2) mask */



   /*
    * exit if there is no place working to get event data from
    */
   if (b_fd < 0  &&  n_fd < 0)  {
	  printf("binlogd:  no event data sources, EXITING\n");
	  terminate_nicely();
   }



   /*
    *  We only know how big an event record can be on the local host,
    *  because we can find out how big the kernel binary event log buffer is.
    *  Allocate a work buffer that big and hope that its big enough to
    *  handle stuff comming from a remote system.  If there is a problem
    *  with /dev/kbinlog than try to get a default sized (best guess)
    *  buffer to use for the network and the mailbox.
    */
   errno = 0;
   if (b_fd >= 0 )  {
          if (ioctl(b_fd, BINLOG_GETSTATUS, &bsc) < 0)  {
	       dprintf("\tGETSTATUS ioctl failed on /dev/kbinlog: %d\n", errno);
	       workbufsize = DEFAULTBUFSIZE;  /* force a default size */
	  }
          else
	       workbufsize = bsc.sc_size;   /* use our systems size */
   }
   if ((workbuf = (char *)malloc(workbufsize)) == NULL)  {
	 printf("binlogd:  can't allocate buffer space, EXITING\n");
	 (void) unlink(MAILBOX_CTRL_PATHNAME);
	 exit(0);
   }
}





/*
 *  [re]initialization
*/
void initialize()
{
   register char *p;
   struct configfile *cfl;

   dprintf("initialize()\n");

   /* get our systems name */
   (void) gethostname(myhostname, sizeof(myhostname));
   if (p = index(myhostname, '.'))   /* chop off the domain name stuff */
	*p++ = '\0';


   /*
    *  Close any open log files, and delete the all list elements.  We do 
    *  this because we could be REinitializing, in which case we always want
    *  to use the current flavor of the config file.
    */
   for (cfl = cflist; cfl > (struct configfile *)0; cfl = cflist)  {
	 switch (cfl->inuse)  {

	      case LOG_LOCAL:
		   close(cfl->whereto.file.fd);
		   break;

	      default:
		   break;
	 }
	 cflist = cfl->next;
	 free((char *)cfl);
   }
   cflist = (struct configfile *)0;
   read_configfile();          /* process config file */
}





/*
 *  Read the binlogd config file and build a linked list of descriptors
 *  for each valid line in the file.
*/
read_configfile()
{
   register char *p;
   char linebuf[LINESIZE];       /* buffer for a line from config file */
   char event[LINESIZE];         /* config file line priority/event specifier */
   char outpath[LINESIZE];       /* config file line output path specifier */
   FILE *cfp;                    /* config file pointer */
   char *priority;               /* event part */
   struct configfile *cfl, *cfl_last;   /* list of config file output structs */
   int fd;                       /* file descriptor */
   struct hostent *host;         /* network info about a system */


   dprintf("read_configfile()\n");

   /* init configfile linked list pointers */
   cfl = (struct configfile *)0;
   cfl_last = (struct configfile *)0;

   /* open the config file */
   if ((cfp = fopen(ConfFilePath, "r")) == NULL)  {
	  dprintf("\tCan't open %s\n", ConfFilePath);
	  exit();
   }

   /*
    * process each line in the config file and build an output description
    * list.
    */
   while (fgets(linebuf, LINESIZE, cfp) != NULL)  {
       for (p = linebuf; isspace(*p); ++p);     /* skip leading white space */
       if (*p == NULL || *p == '#')   /* skip blank lines and comment lines */
                continue;
       sscanf(linebuf, "%s %s",event, outpath); /* separate the line elements */
       if (p = (char *)index(event, '.'))  { /* get priority specifier if any */
		*p++ = '\0';
		priority = p;
       }
       p = outpath;

       /* get a new list element if we don't have one and initialize it */
       if (cfl == (struct configfile *)0)  {
               cfl = (struct configfile *)calloc(1, sizeof(struct configfile));
	       cfl->next = (struct configfile *)0;
	       cfl->inuse = LOG_INVALID;
	}

       /*
	* parse event type specifier
	*/
       /* file for binlog kernel buffer taken from vmunix dump on a reboot */
       if ((strcmp(event, "dumpfile")) == 0)  {
	       if (*p == '/')  {
	             strcpy(cfl->whereto.file.filename, p);
	             cfl->inuse = LOG_DUMP;
	       }
	       else  {
                     dprintf("\tinvalid output path specifier: %s\n", outpath);
	       }
	       goto donext;     /* do the next config file entry */
       }


       if (*event != NULL)  {
	     if ((strcmp(event, "*")) == 0)
		   cfl->event = LOG_ALLEVENTS;
             else
		   cfl->event = atoi(event);
       }
       else
	     cfl->event = LOG_ALLEVENTS;      /* default action */


       /*
	* parse priority specifier
	*/
       if ((priority == NULL) || (strcmp(priority, "*") == 0))
		cfl->priority = LOG_ALLPRI;       /* default */
       else if ((strcmp(priority, "severe")) == 0)
	      cfl->priority =  EL_PRISEVERE; 
       else if ((strcmp(priority, "high")) == 0)
	      cfl->priority = EL_PRIHIGH;
       else if ((strcmp(priority, "low")) == 0)
	      cfl->priority = EL_PRILOW;
       else  {
		dprintf("\tunknown priority keyword: %s\n", priority);
		continue;            /* try next config file entry */
	}

      
       /*
	* handle output path specifier
	*/
        switch (*p)  {

	    case '/':                                   /* local file */
		fd = open(p, (O_WRONLY | O_APPEND | O_CREAT), 
			      (S_IRUSR | S_IWUSR | S_IRGRP) ); /* mode 640 */
		if (fd < 0)  {
		       dprintf("\tcan't open log file: %s\n", p);
		       break;
		}
		cfl->whereto.file.fd = fd;
		strcpy(cfl->whereto.file.filename, p);
		cfl->inuse = LOG_LOCAL;
		break;


	    case '@':                                  /* remote system name */
		if (n_fd < 0)
			break;
		bzero(&cfl->whereto.socket.addr, sizeof(struct sockaddr));
		cfl->whereto.socket.addr.sin_family = AF_INET;
		cfl->whereto.socket.addr.sin_port = port;
		p++;   /* skip past the "@" */
		strcpy(cfl->whereto.socket.remotehost, p);
		host = gethostbyname(p);
		if (host == 0)  {
		     dprintf("\tunknown hostname for remote system: %s\n", p);
		     break;
		}
		bcopy(host->h_addr, (char *)&cfl->whereto.socket.addr.sin_addr,
				                                host->h_length);
		cfl->inuse = LOG_REMOTE;
		break;


	    default:
		dprintf("\tinvalid output path specifier: %s\n", outpath);
		break;
        }
donext:
	/*
	 * If the current element actually got used than add it to the 
	 * list.
	 */
	if (cfl->inuse != LOG_INVALID)  {
                 if (cflist == (struct configfile *)0)  { /* the very first */
			 cflist = cfl;
			 cfl_last = cfl;
			 cfl = (struct configfile *)0;
		  }
                  else  {                  /* add to end of existing list */
			 cfl_last->next = cfl;
			 cfl_last = cfl;
                         cfl = (struct configfile *)0;
		  }
	}
   }
   fclose(cfp);       /* all done, close config file */

   if (dbug)  {       /* dump out the linked list we just made */
      dprintf("\tConfig File List Dump:\n");
      for (cfl = cflist; cfl > (struct configfile *)0; cfl = cfl->next)  {
	  switch (cfl->inuse)  {
	      case LOG_LOCAL:
		  printf("\t\tLOCAL: %s\n", cfl->whereto.file.filename);
		  break;
	      case LOG_REMOTE:
		  printf("\t\tREMOTE: %s\n", cfl->whereto.socket.remotehost);
		  break;
	      case LOG_DUMP:
		  printf("\t\tDUMP: %s\n", cfl->whereto.file.filename);
		  break;
	      case LOG_INVALID:
		  printf("\t\tINVALID config entry\n");
		  break;
	      default:
		  printf("\t\tInvalid config file type: %d\n", cfl->inuse);
		  break;
	  }
      }
   }
}





/*
 *  dump_recovery()
 *
 *  This routine checks for a file that contains the binary event logger 
 *  kernel buffer (blbuf) saved from a crash dump by savecore.  This
 *  file can have several copies of the msgbuf in it if the system kept
 *  crashing before this routine could run. The file is deleted after
 *  being processed.
 *
 *  This routine reads in the blbuf from the dump save file (one blbuf
 *  amount at a time) and puts it in a form that logevent() likes.
 *
 *  Savecore has already verified the best that you can that the buffer
 *  is valid and had something in it.  Also the pointers in the header
 *  (in, out, le) were turned into offsets from the beginning of the buffer.
*/
dump_recovery()
{

   register struct configfile *cfl;
   int  ifd;                       /* input file descriptor */
   int rval;
   long kbufsize;                  /* size of the kernel buffer */
   char *buf;                      /* work buffer */
   struct binlog_bufhdr *b;


   dprintf("dump_recovery()\n");

   /* find the dump file specifier in the config file list */
   for (cfl = cflist; (cfl > (struct configfile *)0) && 
				   (cfl->inuse != LOG_DUMP); cfl = cfl->next);
   if (cfl == (struct configfile *)0)
	    return;        /* no dump file specified in config file */

   /* open the dump file */
   errno = 0;
   ifd = open(cfl->whereto.file.filename, O_RDONLY, 0);    /* open dump file */
   if (ifd < 0)  {
         if (errno == ENOENT)
               dprintf("\tno dump file found\n");
         else
               dprintf("\tcan't open dump file: %s\n",
					           cfl->whereto.file.filename);
	 return;
   }

   /* read just a little bit of the dump file to see what we have */
   b = (struct binlog_bufhdr *)malloc(BUFHDRSIZE);
   rval = read(ifd, (char *)b, BUFHDRSIZE);
   if (rval < BUFHDRSIZE)  {
	  dprintf("\tcan't read dump file: %s\n",
					           cfl->whereto.file.filename);
	  (void)close(ifd);
	  (void)unlink(cfl->whereto.file.filename);
	  free(b);
	  return;
   }

   kbufsize = b->size;
   free(b);
   buf = (char *)malloc(kbufsize);
   lseek(ifd, 0, SEEK_SET);                  /* go back to beginning of file */


   /* looks ok so far - read and process the dump file */
   while( (rval = read(ifd, buf, kbufsize)) > 0)  {
	if (rval < kbufsize)  {
		  dprintf("\tbad dump file read size: %d\n", rval);
	          (void)close(ifd);
	          (void)unlink(cfl->whereto.file.filename);
	          free(buf);
	          return;
        }
        b = (struct binlog_bufhdr *)buf;
        /*
         * paranoid - we did this in savecore already, but just in case....
         * If this stuff is screwed up than we can't trust anything!
         */
        if ((b->size < BINLOG_BUFMIN) || (b->size > BINLOG_BUFMAX) ||
	            (b->magic != BINLOG_MAGIC)  ||
				      (b->in == b->out))   {
	       (void)close(ifd);
	       (void)unlink(cfl->whereto.file.filename);
	       free(buf);
	       dprintf("dump file is corrupted\n");
	       return;
        }

        /*
         * Process the buffer as if it were read normally from /dev/kbinlog
         */
        if (b->in > b->out)   /* buffer not wrapped */
	       logevent((buf + (int)b->out), (int)(b->in - b->out), FROMLOCAL);
         else  {               /* buffer wrapped - handle both parts */
	       logevent((buf + (int)b->out), (int)(b->le - b->out), FROMLOCAL);
               if ((int)b->in > BUFHDRSIZE)
		        logevent((buf + BUFHDRSIZE), (int)(b->in), FROMLOCAL);
         }
   }
   (void)close(ifd);
   (void)unlink(cfl->whereto.file.filename);
   free(buf);
}



/*
 *  Process the event record(s) just read.  There could be
 *  multiple records in the buffer if it came from /dev/kbinlog, or
 *  only one if it was forwarded from a remote system.  
 *
 *  This function fills in some items of the event record that were not
 *  done in the kernel when the event record was created.
 *
 *  Each event record is sent to the places indicated in the config file 
 *  and to any connected mailbox clients.
 *
 *  THIS IS THE ONLY FUNCTION IN BINLOGD THAT HAS KNOWLEDGE OF WHAT AN EVENT
 *  RECORD LOOKS LIKE.... YOU CHANGE THE EVENT RECORD FORMAT AND YOU CHANGE
 *  THIS ROUTINE!  The only knowledge we have is of the non-event-specific
 *  (vendor independent) components of the record.
*/
logevent(buf, len, from)
char *buf;              /* event log buffer from /dev/kbinlog or network */
int  len;               /* amount of data in the buffer */
int  from;              /* from local system or network */
{
   register struct el_rhdr  *hdrp;      /* event record header pointer */
   register struct el_sub_id *sidp;     /* event record sub_id pointer */
   register char *p;
   time_t thetime;

   dprintf("logevent()\n");

   /*
    * We can ONLY know about the vendor common/independent components of
    * a binary event log record.... so we play some games with just these
    * parts of the record.
    */
   hdrp = (struct el_rhdr *)buf;
   sidp = (struct el_sub_id *)((char *)hdrp + EL_RHDRSIZE); 

   if (len < EL_MISCSIZE)
	    return;         /* nothing of valid minimal length in buffer */
   while ((char *)hdrp < (buf + len))  {          /* process the whole buffer */

	/* If a record length is corrupted than nothing else in the buffer
	 * can be used because we can't trust finding the next record.
	 * Determine as best we can if the record length is any good.
	 */
        if ((hdrp->rhdr_reclen < EL_MISCSIZE) || 
			      (hdrp->rhdr_reclen > len))  {
		dprintf("\tcorrupted header record length value\n");
		return;
	}

        if (hdrp->rhdr_valid != EL_VALID)  {
	       dprintf("\tvalid indicator not set in record\n");
	       goto donext;
	}

	/* Check to see if the whole event record got read into the buffer */
	if ( ( (char *)hdrp + hdrp->rhdr_reclen) > (buf + len) )  {
	       dprintf("\tincomplete event record read\n");
	       return;
	}

	if (from == FROMLOCAL)  {
	    /*
	     *  Fill in the time.  The record could been logged early enough
	     *  that time in the kernel was not set yet... the usual case for
	     *  the startup record.
	     */
	     if (hdrp->rhdr_time < 3600)  {
		     (void) time(&thetime);
		     hdrp->rhdr_time = thetime;
	     } 
	     /*
	      * Fill in the hostname.  We don't do this in the kernel because
	      * we can be logging events before the hostname has been set.
	      */
	     (void)strncpy(hdrp->rhdr_hname, myhostname, EL_SIZE16);
        }

	/* Output the event record to the places specified in the config file */
	conffile_output((char *)hdrp,
			 hdrp->rhdr_reclen, 
		         sidp->subid_class,
		         hdrp->rhdr_pri);

	/* Output the event record to mailbox connections */
	mailbox_output((char *)hdrp, 
			 hdrp->rhdr_reclen,
			 sidp->subid_class,
			 hdrp->rhdr_pri);

donext:
	/* Point to the next record */
	hdrp = (struct el_rhdr *)((char *)hdrp + hdrp->rhdr_reclen);
        sidp = (struct el_sub_id *)((char *)hdrp + EL_RHDRSIZE); 
   }
}



/*
 *  Send the current event record to all valid places listed in the config
 *  file that want this flavor of event.
*/
conffile_output(d, l, event, pri)
char *d;                       /* the event data to output */
int   l;                       /* length of data output */
int   event;                   /* event type */
int   pri;                     /* event priority */
{

  register struct configfile *cfl;
  int fd;
  static int retry_server = 0;   /* time since we last tried to start server */
  time_t thetime;

  dprintf("conffile_output()\n");

  (void)time(&thetime);       /* get the current time */

  /* go through the config file output list */
  for (cfl = cflist; cfl != 0; cfl = cfl->next)  { 
      switch (cfl->inuse)  {

	 case  LOG_LOCAL:          /* write event record to local file */
	     errno = 0;
	     /* Put this event type and priority in this file ? */
             if((cfl->event == LOG_ALLEVENTS || cfl->event == event) &&
                       (cfl->priority == LOG_ALLPRI || cfl->priority == pri))  {
	           if (write(cfl->whereto.file.fd, d, l) != l)  {
		          /* write to binary log file failed */
		          if (errno != EINTR)  {
		                 (void) close(cfl->whereto.file.fd);
		                  cfl->inuse = LOG_INVALID;
			  }
		          break;
		   }
	      }
	      (void) fsync(cfl->whereto.file.fd);
	      dprintf("\twrote %d bytes to: %s\n",l,cfl->whereto.file.filename);
	      break;


	 case  LOG_REMOTE:            /* send event record to remote binlogd */
	       errno = 0;
	       /* Send this event type and priority sent to this system ? */
               if((cfl->event == LOG_ALLEVENTS || cfl->event == event) &&
                       (cfl->priority == LOG_ALLPRI || cfl->priority == pri))  {

	           if (sendto(n_fd, d, l, 0, &cfl->whereto.socket.addr,
			                    sizeof(struct sockaddr_in)) < l)
		          if (errno != EMSGSIZE || errno != EINTR)
		                 cfl->inuse = LOG_INVALID;
	       }
	       dprintf("\tsent %d bytes to: %s\n", l,
					       cfl->whereto.socket.remotehost);
	       break;

	 /* ignore these */
	 case  LOG_DUMP:
	 case  LOG_INVALID:
	       break;

	 default:
	       cfl->inuse = LOG_INVALID;
	       break;
      }
  }
  retry_server = thetime + 300;
}



/*
 *  Receive mailbox control messages (connect & disconnect) and add or
 *  remove a client from the list of active mailboc connections.
*/
mailbox_ctrl(buf, len)
char *buf;
int  len;
{

  int rval;
  struct sockaddr_un name;
  struct mailbox *mb, *m;
  struct mailbox_ctrlmsg *msg;
  int rmsg;

  dprintf("mailbox_ctrl()\n");

  msg = (struct mailbox_ctrlmsg *)buf;

  switch (msg->command)  {

      case MAILBOX_CONNECT_REQ:

	   if (mb_connects < MAILBOX_MAX_CONNECTS)  {
	         if (mblist == (struct mailbox *)0)  { /* first element */
	               mb = (struct mailbox *)calloc(1, sizeof(struct mailbox));
	               mb->next = (struct mailbox *)0;
	               mb->event = msg->event;
	               mb->priority = msg->priority;
	               mb->fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	               mb->sock.sun_family = AF_UNIX;
	               strcpy(mb->sock.sun_path, msg->path);
                       mblist = mb;
	         }
	         else  {        /* find unused entry or add a new one to end */
		       m = mblist;
		       while (m->next > (struct mailbox *)0 && m->fd >= 0)
			      m = m->next;
		       if (m->fd < 0)  {               /* recycle an entry */
			    mb = m;
	                    mb->event = msg->event;
	                    mb->priority = msg->priority;
	                    mb->fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	                    mb->sock.sun_family = AF_UNIX;
	                    strcpy(mb->sock.sun_path, msg->path);
	               }
	               else   {                        /* make a new entry */
	                    mb = (struct mailbox *)calloc(1,
							sizeof(struct mailbox));
	                    mb->next = (struct mailbox *)0;
	                    mb->event = msg->event;
	                    mb->priority = msg->priority;
	                    mb->fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	                    mb->sock.sun_family = AF_UNIX;
	                    strcpy(mb->sock.sun_path, msg->path);
		            m->next = mb;
	              }
	         }
		 mb_connects++;
	         rmsg = MAILBOX_CONNECT_ACK;
		 dprintf("\tmailbox connect from: %s\n", mb->sock.sun_path);
           } else
		 rmsg = MAILBOX_CONNECT_NAK;

	   /* send connect request response */
           rval = sendto(mb->fd, (char *)&rmsg, MAILBOX_RESPONSE_SIZE, 0, 
					    (struct sockaddr_un *)&mb->sock,
					            sizeof(struct sockaddr_un));
           if (rval < 0)  {     /* error sending, close entry */
		       close(mb->fd);
		       mb->fd = -1;
		       mb_connects--;
	   }
	   break;

      case MAILBOX_DISCONNECT:
           for (mb = mblist; mb > (struct mailbox *)0; mb = mb->next)  {
               if (strcmp(mb->sock.sun_path, msg->path) == 0) {
		  close(mb->fd);
		  mb->fd = -1;
		  mb_connects--;
		  dprintf("\tmailbox disconnect from: %s\n", mb->sock.sun_path);
	       }
	   }
	   break;

      default:
	      dprintf("\tinvalid control message received, cmd: 0x%x\n", 
							       msg->command);
	      break;
   }
}



/*
 *  Send the current event record to all conencted mailbox clients that
 *  want this flavor of event.
*/
mailbox_output(d, l, event, pri)
char *d;                       /* the event data to output */
int   l;                       /* length of data output */
int   event;                   /* event type */
int   pri;                     /* event priority */
{


   register struct mailbox *m;
   register int rval;

   dprintf("mailbox_output()\n");

   for (m = mblist; m > (struct mailbox *)0; m = m->next)  {
	if (m->fd < 0)
		continue;    /* in active mail box */
	/* does the client want this event type and priority ? */
        if((m->event == LOG_ALLEVENTS || m->event == event) &&
                       (m->priority == LOG_ALLPRI || m->priority == pri))  {
               rval = sendto(m->fd, d, l, 0, (struct sockaddr_un *)&m->sock,
					            sizeof(struct sockaddr_un));
               if (rval < 0)  {  /* error sending, terminate mailbox entry */
		  close(m->fd);
		  m->fd = -1;
		  mb_connects--;
		  dprintf("\terror sending to mailbox: %s\n", m->sock.sun_path);
	       }
	       else
	            dprintf("\tsent %d bytes to %s\n", rval, m->sock.sun_path);
	 }
   }
}



/*
 *  Exit binlogd by first nicely cleaning up everything.
*/
void terminate_nicely()
{
   register struct configfile *cfp;
   register struct mailbox    *mbp;
   struct sockaddr_in svr;       /* remote system (server) to send to */
   int msg;

   dprintf("terminate_nicely()\n");

   (void)close(b_fd);
   (void)close(n_fd);
   (void)close(m_fd);
   (void)unlink(MAILBOX_CTRL_PATHNAME);

   /* close any open local files and notify remote client systems */
   for(cfp = cflist; cfp > (struct configfile *)0; cfp = cfp->next)  {
	switch(cfp->inuse)  { 
	    case LOG_LOCAL:
		   (void)close(cfp->whereto.file.fd);
		   break;

	    default:
		   break;
	}
   }
	    

   /* send disconnect message to any active mailbox connections */
   msg = MAILBOX_DISCONNECT;
   for (mbp = mblist; mbp > (struct mailbox *)0; mbp = mbp->next)  {
	   if (mbp->fd < 0)
		   continue;    /* in active mail box */
           sendto(mbp->fd, (char *)&msg, MAILBOX_RESPONSE_SIZE, 0, 
					 (struct sockaddr_un *)&mbp->sock,
					           sizeof(struct sockaddr_un));
	   close(mbp->fd);
   }

   exit(0);
}

