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
static char *rcsid = "@(#)$RCSfile: logmsg.c,v $ $Revision: 1.1.3.7 $ (DEC) $Date: 1992/06/25 22:32:10 $";
#endif

#include <sys/file.h>
#include <string.h>
#include <stdio.h>
#include <sys/param.h>
#include <dec/binlog/binlog.h>
#include <sys/errno.h>

#define ERROR -1
#define SUCCESS 0

/*
 *  Log an ascii message string into binary event log
 *
 *  Its the callers responsibility to use a correct and supported class code!
 */
binlogmsg(class, msgstr)
int class;                   /* the event class code */
char *msgstr;                /* the formated ascii message string to log */
{

   int fd;
   int rval;
   int size;
   int msglen;

   struct event_record  {         /* generic ascii message buffer */
	  struct el_sub_id elsubid;
	  struct el_msg    elmsg;
    } *msg;
 
   errno = 0;

   fd = open("/dev/kbinlog", O_WRONLY, 0); 
   if (fd < 0){
	errno = EIO;
	return(ERROR);
   }

   msglen = strlen(msgstr) + 1;         /* include the \0 */
   if (msglen == 0)  {
	 errno = EINVAL;
         return(ERROR);
   }
   if (msglen > (sizeof(msg->elmsg.msg_asc)))  {
	  errno = E2BIG;
	  return(ERROR);
   }

   /* get a generic buffer to build message in */
   size = sizeof(struct event_record);
   msg = (struct event_record *)malloc(size);

   /* build the data for the event log */
   if (msg != 0)  {
	 LSUBID(msg,class,EL_UNDEF,EL_UNDEF,EL_UNDEF,EL_UNDEF,EL_UNDEF);
	 strcpy(msg->elmsg.msg_asc, msgstr);
	 msg->elmsg.msg_len = (msglen + 1);   /* include the \0 */
	 msg->elmsg.msg_type = EL_UNDEF;   /* msg type not currently used */
   }
   else  {
	 errno = ENOMEM;
	 return(ERROR);
   }

   /* get the real size of the event data */
   size = EL_SUBIDSIZE + sizeof(msg->elmsg.msg_len) + 
				     sizeof(msg->elmsg.msg_type) + msglen;
   /* log it */
   rval = write(fd, msg, size);
   if (rval < size) {
	  errno = EIO;
	  return(ERROR);
    }

    free((void *)msg);
    (void)close(fd);
}

