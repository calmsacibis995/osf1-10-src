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
static char *rcsid = "@(#)$RCSfile: mailbox.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/06/25 22:32:22 $";
#endif

#include <sys/errno.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <dec/binlog/binlogd.h>
#include <dec/binlog/binlog.h>

#include <stdio.h>


#define SUCCESS 0
#define ERROR   -1

int mbdebug = 0;
#define dprintf    if (mbdebug) printf

int server;                   /* binlogd mailbox control socket */
int client;                   /* to receive data from binlogd on */



/*
 *  For info on how to use the mailbox read sys/binlogd.h.
*/


/*
 *  Make a client side mailbox connection to binlogd.  
*/
int mailbox_open(mbd)
struct mailbox_desc *mbd;
{
     struct mailbox_ctrlmsg  msg;
     int rmsg;
     int rval;
     pid_t  mypid;
     struct sockaddr_un s_sun;
     struct sockaddr_un c_sun;
     int blfd;                     /* /dev/kbinlog file descriptor */
     struct binlog_getstatus bsc;  /* /dev/kbinlog getstatus ioctl return */


     /*
      * Make a unique name for the clients UNIX domain socket if none given.
      */
     if (strlen(mbd->path) == 0)  {
            mypid = getpid();
            sprintf(mbd->path, "/tmp/MailBoxPid%d", mypid);
	    mbd->flags |= MBFLAG_PATHNAME;
            dprintf("created client mailbox: %s\n", mbd->path);
     }


     /*
      * Build a mailbox connect request message.
      */
     msg.command = MAILBOX_CONNECT_REQ;
     strcpy(msg.path, mbd->path);
     msg.event = mbd->event;
     msg.priority = mbd->priority;


     /*
      * Setup the connection for receiving from binlogd.
      */
     (void)unlink(mbd->path);
     client = socket(AF_UNIX, SOCK_DGRAM, 0);
     if (client < 0 )
	     return(ERROR);
     c_sun.sun_family = AF_UNIX;
     strcpy((char *)c_sun.sun_path, mbd->path);
     if (bind(client, (struct sockaddr_un *)&c_sun, 
                                           sizeof(struct sockaddr_un)) < 0)   {
	  dprintf("mailbox_open(): can't bind to: %s\n", mbd->path);
	  mailbox_error(mbd);
	  return(ERROR);
     }
     mbd->fd = client;


     /*
      * Setup the connection for sending to the binlogd mailbox control socket.
      */
     server = socket(AF_UNIX, SOCK_DGRAM, 0);
     if (server < 0 )  {
	     mailbox_error(mbd);
	     return(ERROR);
     }
     s_sun.sun_family = AF_UNIX;
     strcpy(s_sun.sun_path, MAILBOX_CTRL_PATHNAME);


     /*
      *  Send mailbox connect request message to binlogd
      */
     rval = sendto(server, (char *)&msg, sizeof(struct mailbox_ctrlmsg), 0,
	            (struct sockaddr_un *)&s_sun, sizeof(struct sockaddr_un));
     if (rval < 0 )  {
	  dprintf("mailbox_open(): error sending connect request to binlogd\n");
	  mailbox_error(mbd);
	  return(ERROR);
     }
     dprintf("sent connect request\n");
     

     /*
      * receive mailbox connect request response from binlogd
      */
     rval = read(client, (char *)&rmsg, MAILBOX_RESPONSE_SIZE);
     if (rval != MAILBOX_RESPONSE_SIZE)  {
	     dprintf("mailbox_open(): bad connect ACK message received from binlogd\n");
	     mailbox_error(mbd);
	     return(ERROR);
     }
     if (rmsg != MAILBOX_CONNECT_ACK)  {
	     dprintf("mailbox_open(): connect ACK not received from binlogd\n");
	     mailbox_error(mbd);
	     return(ERROR);
     }
     (void)close(server);
     dprintf("connect ACK received\n");


     /*
      * Allocate a buffer to receive data from binlogd if the user didn't.
      */
     if (mbd->bufsize == 0 || mbd->buf == (char *)0)  {
            if ((blfd = open("/dev/kbinlog", O_RDONLY)) < 0)  {
	            dprintf("mailbox_open(): can't open /dev/kbinlog\n");
		    mailbox_error(mbd);
                    return(ERROR);
            }
           if (ioctl(blfd, BINLOG_GETSTATUS, &bsc) < 0)  {
	          dprintf("mailbox_open(): BINLOG_GETSTATUS ioctl failed \n");
		  mailbox_error(mbd);
		  return(ERROR);
           } 
           mbd->buf = (char *)malloc(bsc.sc_size);
           if (mbd->buf == (char *)0)  {
	           dprintf("mailbox_open(): can't allocate buffer\n");
		   mailbox_error(mbd);
		   return(ERROR);
           }
	   mbd->bufsize = bsc.sc_size;
	   mbd->flags |= MBFLAG_BUFALLOC;
	   dprintf("allocated buffer of size: %d\n", mbd->bufsize);
     }


     dprintf("mailbox_open(): binlogd mailbox connection successful\n");
     return(SUCCESS);
}



/*
 * Clean up on an error before we return.
*/
int mailbox_error(mbd)
struct mailbox_desc *mbd;
{
     (void)close(client);
     mbd->fd = -1;
     (void)unlink(mbd->path);
     (void)close(server);
} 
	





/*
 *  Reads the mailbox data received from binlogd.  Detects a disconnect
 *  message from binlog and shutsdown.  Returns -1 on a disconnect or
 *  a problem reading the sockect, otherwise returns the number of
 *  bytes received.
*/
int mailbox_read(mbd)
struct mailbox_desc *mbd;
{
     int rval;

     errno = 0;
     rval = read(mbd->fd, mbd->buf, mbd->bufsize);
     if (rval < 0 )  {
	    dprintf("mailbox_read():  error receiving from binlogd, errno: %d\n",errno);
	    return(ERROR);
     }
     if ((rval == MAILBOX_RESPONSE_SIZE) && 
				     ((int)*mbd->buf == MAILBOX_DISCONNECT))  {
	    dprintf("mailbox_read():  disconnect received from binlogd\n");
	    return(ERROR);
     }
     dprintf("mailbox_read(): received %d bytes from binlogd\n", rval);
     return(rval);
}
	      
	       




/*
 *  Nicely shutsdown and clean up a mailbox connection to binlogd.
*/
mailbox_close(mbd)
struct mailbox_desc *mbd;
{
   struct mailbox_ctrlmsg  msg;
   struct sockaddr_un s_sun;
   int rval;


   /*
    * Build a mailbox disconnect message.
    */
   msg.command = MAILBOX_DISCONNECT;
   strcpy(msg.path, mbd->path);
   msg.event = 0;
   msg.priority = 0;

   /*
    * Setup for the connection for sending to the binlogd mailbox control
    * socket.
    */
   server = socket(AF_UNIX, SOCK_DGRAM, 0);
   if (server >= 0 )  {
           s_sun.sun_family = AF_UNIX;
           strcpy(s_sun.sun_path, MAILBOX_CTRL_PATHNAME);
	   /* send the disconnect message */
           rval = sendto(server, (char *)&msg, sizeof(struct mailbox_ctrlmsg),
	           0, (struct sockaddr_un *)&s_sun, sizeof(struct sockaddr_un));
   }
   /* clean up after ourselves */
   (void)close(mbd->fd);              /* close or mailbox socket */
   mbd->fd = -1;
   (void)unlink(mbd->path);           /* delete the mailbox socket */
   if (mbd->flags & MBFLAG_PATHNAME)  /* clear pathname if we created it */
	     mbd->path[0] = '\0';
   if (mbd->flags & MBFLAG_BUFALLOC)  { /* if we alloc'd memory than free it */
             free(mbd->buf);
	     mbd->bufsize = 0;
	     mbd->buf = (char *)0;
   }
}




