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
/*
 *      @(#)$RCSfile: binlogd.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1992/06/25 22:30:49 $
 */
/*
 * logs from origional file location (kernel/sys/binlogd.h)
 *
 * Revision 1.1.3.2  92/03/18  18:43:13  Al_Delorey
 * 	From Silver: AG_BL5
 * 	[92/03/18  16:52:20  Al_Delorey]
 * 
 * 	bcreate'ed file wrong and had an 'rcsid' in it.
 * 	[92/01/15  08:58:27  Scott_Cranston]
 * 
 * 	nitial file creation.
 * 	[92/01/14  16:01:20  Scott_Cranston]
 * 
 */


#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/param.h>


#ifndef  __BINLOGD__
#define  __BINLOGD__

#define DEFAULTBUFSIZE      (8 * 1024)       /* 8Kb  */
#define LINESIZE            100


/*
 *  A linked list of these structures is made by reading and processing the 
 *  binlog.conf file.  When an event record is ready to output this list is
 *  traversed and the event record is sent to everybody who is registered.
*/
struct configfile {
	struct configfile *next;     /* a linked list */
	int    inuse;		     /* local, remote, invalid */
	int    event;                /* event type selection code */
	int    priority;             /* severe, high, low */
	union  {
	    struct {
		char	           remotehost[MAXHOSTNAMELEN+1];
		struct sockaddr_in addr;
	    } socket;		               /* socket destination info */
	    struct {
	        int    fd;		      /* destination file descriptor */
	        char filename[LINESIZE];
	    }file;
	} whereto;
};

/* inuse types  */
#define LOG_INVALID    0x1         /* an invalid output description */
#define LOG_LOCAL      0x2         /* output to a local file */
#define LOG_REMOTE     0x4         /* forward to a remote system */
#define LOG_DUMP       0x8         /* buffer recovery file from a crash dump */


#define LOG_ALLPRI     0            /* all priority levels */
#define LOG_ALLEVENTS  0            /* all event classes */





/*
 *  The binlogd mailbox:
 *
 *  This is a UNIX domain socket based facility whereby a user process
 *  can have a connection to binlogd and receive the binary event log
 *  records.  A library (libbinlog) of functions to facilitate using the
 *  mailbox is supplied:
 *
 *       mailbox_open(struct mailbox_desc *)
 *             This function creates the mailbox connection to binlogd.
 *             A value of -1 is returned upon error and 0 is returned
 *             for a successful connection.  The function use the parameters
 *             passed via the data structure argument to make the connection.
 *             The user can optionaly specify the path name of the socket
 *             to receive on.  The user can optionaly allocate the receive
 *             buffer.  This function will supply and optional items not
 *             supplied by the user.
 *
 *       mailbox_read(struct mailbox_desc *)
 *             Performs a blocking read on the mailbox data from binlogd.
 *             The wait for data can be considerable, dependent on the
 *             frequence with which events are occuring.  The received data
 *             is placed in the buffer described in the data structure argument.
 *             A -1 is returned on an error condition or if a disconnect was
 *             received.  Otherwise, the length (in bytes) of the received
 *             data is returned.
 *
 *       mailbox_close(struct mailbox_desc *)
 *             Cleans up when the user is done with the mailbox.  If the
 *             connection to binlogd is still open a disconnect is done.
 *             The file descriptor is closed and the path is deleted.  If
 *             the user did not furnish the buffer, and thus it was
 *             allocated by mailbox_open() the buffer memory is free'ed.
 *             This function should be called upon an error condition or 
 *             when finished using the mailbox.
 *
 *  The user can set the variable 'extern int mbdebug' to a non-zero value
 *  to enable debug messages from the mailbox library functions.
*/





#define MAILBOX_MAX_CONNECTS         6   /* maximum mailbox connections */
#define MAILBOXPATHLEN               60  /* max length of client mailbox name */
#define MAILBOX_CTRL_PATHNAME        "/dev/binlogdmb"
#define MAILBOX_NAMESIZE             sizeof(MAILBOX_CTRL_PATHNAME)
#define MAILBOX_CTRLMSG_SIZE         sizeof(struct mailbox_ctrlmsg)
#define MAILBOX_RESPONSE_SIZE        sizeof(int)
#define MAILBOX_CONNECT_REQ          0xfec0020
#define MAILBOX_CONNECT_ACK          0xfec0021
#define MAILBOX_CONNECT_NAK          0xfec0022
#define MAILBOX_DISCONNECT           0xfec0023 
#define MBFLAG_PATHNAME 0x2  /* path name was created by mailbox_open() */
#define MBFLAG_BUFALLOC 0x4  /* buffer memory was allocated by mailbox_open() */



/*
 *  A linked list of these structures is maintained by binlogd for each
 *  client that makes a connect on the mailbox control socket.
*/
struct mailbox  {
	struct mailbox  *next;
	int    fd;
	int    event;
	int    priority;
	struct sockaddr_un sock;
};



/*
 *  A pointer to this data structure is the argument to the all mailbox library
 *  routines.  The data structure is used to communicate parameters between
 *  the users application and the mailbox library routines.  The data
 *  structure elements are used as follows:
 *
 *    fd       - File descriptor of the socket used to receive on from binlogd.
 *               Filled in by mailbox_open() and is used by mailbox_read() 
 *               and mailbox_close().
 *
 *    flags    - These flag bits are used internally by the mailbox library
 *               functions and must not be set or cleared by the user.
 *
 *    event    - A specific event class code, or LOG_ALLEVENTS.  Specifies
 *               the flavor of binary event record to be put in the mailbox.
 *               The parameter is supplied by the user of the mailbox.
 *
 *    priority - A specific priority code, or LOG_ALLPRI.  Specifies the 
 *               priority level of binary event record to be put in the
 *               mailbox.  The parameter is supplied by the user of the 
 *               mailbox.
 *
 *    path     - The pathname of the UNIX domain socket to receive the mailbox
 *               data from binlogd on.  Can optionaly be supplied by the user,
 *               otherwise mailbox_open() will create one and fill it in.
 *
 *    bufsize  - Gives the size in bytes of the buffer in which received
 *               data from the malbox is placed.  If the user leaves this
 *               parameter zero (0) than the mailbox_open() function will
 *               allocate a buffer and fill in the size.  The size used
 *               is that same as the kernel binary event log buffer.  This
 *               size is obtained from the BINLOG_GETSTATUS ioctl on
 *               /dev/kbinlog.
 *
 *    buf     -  A pointer to the mailbox receive data buffer.  This is
 *               optionaly supplied by the user... see 'bufsize' description.
 *
*/
struct mailbox_desc  {
       int fd;                       /* file descriptor to receive on */
       int flags;                    /* misc internal use flags */
       int event;                    /* event record class code */
       int priority;                 /* event record priority */
       char path[MAILBOXPATHLEN];    /* full pathname of the clients mailbox*/
       int bufsize;                  /* size of buffer allocated */
       char *buf;                    /* pointer to the buffer */
};




/*
 *  The Client sends binlogd Control messages to initiate or terminate a 
 *  mailbox connection.  Binlogd sends simple 4 byte (an int) messages to
 *  the mailbox clients.... this is either: MAILBOX_CONNECT_ACK, 
 *  MAILBOX_CONNECT_NAK or MAILBOX_DISCONNECT.
*/
struct mailbox_ctrlmsg {
       unsigned int  command;        /* the command to be sent */
       char path[MAILBOXPATHLEN];    /* full pathname of the mailbox client */
       unsigned int  event;          /* the event class wanted */
       unsigned int  priority;       /* the event priority level wanted */
};




/*
 *
 *  Mailbox usage programming example:
 *
 *    #include <stdio.h>
 *    #include <sys/binlogd.h>
 *    
 *    extern int *mailbox_open();
 *    
 *    main()
 *    {
 *        struct mailbox_desc mbd;
 *        int rval;
 *        extern int mbdebug;
 *    
 *        mbdebug = 1;
 *        mbd.event = LOG_ALLEVENTS;
 *        mbd.priority = LOG_ALLPRI;
 *        mbd.path[0] = '\0';
 *        mbd.bufsize = 0;
 *    
 *        if (mailbox_open(&mbd) < 0)  {
 *               printf("mailbox_open() failed\n");
 *               exit(0);
 *        }
 *        printf("connect successful, client path: %s\n", mbd.path);
 *        rval = mailbox_read(&mbd);
 *        if (rval < 0)  {
 *               printf("mailbox_read() failure\n");
 *               exit(0);
 *        }
 *        printf("received %d bytes\n", rval);
 *        mailbox_close(&mbd);
 *        printf("\n");
 *    }
 *
*/

#endif  /* __BINLOGD__ */
