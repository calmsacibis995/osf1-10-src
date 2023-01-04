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
static char	*sccsid = "@(#)$RCSfile: user_interf.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:48:31 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */

/*
 * OSF/1 Release 1.0
 */

/*
 *   user_interf.c
 *   
 *   Contents:
 *
 *   int read_uint(unsigned int *ptr);
 *	Returns EOF or !EOF, after reading from stdin a number (unsigned)
 *	in unsigned decimal format.
 *
 *   int read_line(char *buf, int sizeofbuf);
 *	Returns EOF or !EOF, after reading a line from stdin or at most
 *	sizeofbuf - 1 characters; the read in string is always null terminated
 *	and without line feed.
 *
 *   int lvm_perror(int ioctl_id);
 *	Depending on ioctl_id and errno, this routine prints either a
 *	meaningful error message (as documented in the LVDD spec.), or
 *	just calls perror(msg); this function returns OK or NOT_OK.
 *	In the current implementation many of the error messages which
 *	are displayed when different system calls return error, are
 *	the same. Since this can change in the future, different
 *	defines are used for messages which are the same now, but can
 *	change in the future. To save space in the message catalogue,
 *	many of the defines are mapped into the same define in lvmdefmsg.h 
 *	(which is included by lvmcmds.h).
 */

#include "lvmcmds.h"

int
read_uint(unsigned int *ptr)
{
   int ch;
   char *fmt;

   do {

      /* We store a 0, so that special cases don't have to care for that */
      *ptr = 0;
      fmt = "%u";

      do {
         ch = getchar();
         if (ch == EOF)
	    return(EOF);
      } while (!isdigit(ch));

      if (ch == '0') {

	 /* Should be either hex or oct; maybe it's just a zero */
	 ch = getchar();
	 switch (ch) {
	    case EOF: 
		  /* Do NOT return EOF: we successfully read (and store) a 0 */
		  return(!EOF);
	       break;
	    case 'x':
	    case 'X':
		  /* Hexadecimal number? It should be; try scanf */
		  fmt = "%x";
	       break;
	    default:
		  /* Some space? The 0 was the number; *ptr is already 0 */
		  if (isspace(ch))
		     return(!EOF);
		  else
		     if (isdigit(ch)) {
			/* Octal number? It should be; try scanf */
			fmt = "%o";

			/* ...but put back this digit! */
	 		ungetc(ch, stdin);
		     }
		     else
			/*
			 *   The '0' has something weird, following;
			 *   we return EOF, but an error status would be
			 *   better.
			 */
			return(EOF);

	 }
      }
      else
	 ungetc(ch, stdin);

   } while (scanf(fmt, ptr) != 1);

   return(!EOF);
}



int
read_line(char *buf, int sizeofbuf)
{
   register int c, i;

   /* Save one byte for '\0' */
   --sizeofbuf;
   for (i = 0; i < sizeofbuf; i++) {
      
      if ((c = getchar()) == EOF || c == '\n')
	 break;

      *buf++ = c;
   }

   *buf = '\0';
   if (c == EOF)
      return(EOF);
   else
      return(!EOF);
}



int
lvm_perror(int ioctl_id)
{
   char *err_msg;

   debug(dbg_entry("lvm_perror"));

   err_msg = NULL;

   /* Big switch, containing some small switches */
   switch (ioctl_id) {

      case LVM_ACTIVATEVG:
	    switch (errno) {
	       case ENODEV:	err_msg = MSG_ACTIVATEVG_ENODEV;	break;
	       case ENOMEM:	err_msg = MSG_ACTIVATEVG_ENOMEM;	break;
	       case EIO:	err_msg = MSG_ACTIVATEVG_EIO;		break;
	       case ENOENT:	err_msg = MSG_ACTIVATEVG_ENOENT;	break;
	       /* default causes this function to call perror() */
	    }
	 break;

      case LVM_ATTACHPV:
	    switch (errno) {
	       case ENODEV:	err_msg = MSG_ATTACHPV_ENODEV;	break;
	       case ENXIO:	err_msg = MSG_ATTACHPV_ENXIO;	break;
	       case ENOTTY:	err_msg = MSG_ATTACHPV_ENOTTY;	break;
	       /* default causes this function to call perror() */
	    }
	 break;

      case LVM_CHANGELV:
	    switch (errno) {
	       case ENODEV:	err_msg = MSG_CHANGELV_ENODEV;	break;
	       case EROFS:	err_msg = MSG_CHANGELV_EROFS;	break;
	       case EBUSY:	err_msg = MSG_CHANGELV_EBUSY;	break;
	       /* default causes this function to call perror() */
	    }
	 break;

      case LVM_CHANGEPV:
	    switch (errno) {
	       case ENOTTY:	err_msg = MSG_CHANGEPV_ENOTTY;	break;
	       /* default causes this function to call perror() */
	    }
	 break;

      case LVM_CREATELV:
	    switch (errno) {
	       case ENOMEM:	err_msg = MSG_CREATELV_ENOMEM;	break;
	       case ENOTTY:	err_msg = MSG_CREATELV_ENOTTY;	break;
	       case EROFS:	err_msg = MSG_CREATELV_EROFS;	break;
	       case EEXIST:	err_msg = MSG_CREATELV_EEXIST;	break;
	       /* default causes this function to call perror() */
	    }
	 break;

      case LVM_CREATEVG:
	    switch (errno) {
	       case ENODEV:	err_msg = MSG_CREATEVG_ENODEV;	break;
	       case ENOMEM:	err_msg = MSG_CREATEVG_ENOMEM;	break;
	       case EIO:	err_msg = MSG_CREATEVG_EIO;		break;
	       case ENXIO:	err_msg = MSG_CREATEVG_ENXIO;	break;
	       case ENOTTY:	err_msg = MSG_CREATEVG_ENOTTY;	break;
	       case ENOSPC:	err_msg = MSG_CREATEVG_ENOSPC;	break;
	       case EPERM:	err_msg = MSG_CREATEVG_EPERM;	break;
	       case ENOTBLK:	err_msg = MSG_CREATEVG_ENOTBLK;	break;
	       /* default causes this function to call perror() */
	    }
	 break;

      case LVM_DELETELV:
	    switch (errno) {
	       case ENODEV:	err_msg = MSG_DELETELV_ENODEV;	break;
	       case ENOTTY:	err_msg = MSG_DELETELV_ENOTTY;	break;
	       case EROFS:	err_msg = MSG_DELETELV_EROFS;	break;
	       case EBUSY:	err_msg = MSG_DELETELV_EBUSY;	break;
	       /* default causes this function to call perror() */
	    }
	 break;

      case LVM_DELETEPV:
	    switch (errno) {
	       case ENOTTY:	err_msg = MSG_DELETEPV_ENOTTY;	break;
	       /* default causes this function to call perror() */
	    }
	 break;

      case LVM_EXTENDLV:
	    switch (errno) {
	       case ENODEV:	err_msg = MSG_EXTENDLV_ENODEV;	break;
	       case EBUSY:	err_msg = MSG_EXTENDLV_EBUSY;	break;
	       /* default causes this function to call perror() */
	    }
	 break;

      case LVM_INSTALLPV:
	    switch (errno) {
	       case ENODEV:	err_msg = MSG_INSTALLPV_ENODEV;	break;
	       case ENOMEM:	err_msg = MSG_INSTALLPV_ENOMEM;	break;
	       case EIO:	err_msg = MSG_INSTALLPV_EIO;		break;
	       case ENXIO:	err_msg = MSG_INSTALLPV_ENXIO;	break;
	       case ENOTTY:	err_msg = MSG_INSTALLPV_ENOTTY;	break;
	       case EROFS:	err_msg = MSG_INSTALLPV_EROFS;	break;
	       case EPERM:	err_msg = MSG_INSTALLPV_EPERM;	break;
	       case ENOTBLK:	err_msg = MSG_INSTALLPV_ENOTBLK;	break;
	       case EACCES:	err_msg = MSG_INSTALLPV_EACCES;	break;
	       /* default causes this function to call perror() */
	    }
	 break;

      case LVM_OPTIONSET:
	    switch (errno) {
	       case ENOTTY:	err_msg = MSG_OPTIONSET_ENOTTY;	break;
	       /* default causes this function to call perror() */
	    }
	 break;

      case LVM_OPTIONGET:
	    switch (errno) {
	       case ENOTTY:	err_msg = MSG_OPTIONGET_ENOTTY;	break;
	       /* default causes this function to call perror() */
	    }
	 break;

      case LVM_QUERYLV:
	    switch (errno) {
	       case ENXIO:	err_msg = MSG_QUERYLV_ENXIO;	break;
	       /* default causes this function to call perror() */
	    }
	 break;

      case LVM_QUERYPV:
	    switch (errno) {
	       case ENOTTY:	err_msg = MSG_QUERYPV_ENOTTY;	break;
	       /* default causes this function to call perror() */
	    }
	 break;

      case LVM_QUERYPVMAP:
	    switch (errno) {
	       case ENOTTY:	err_msg = MSG_QUERYPVMAP_ENOTTY;	break;
	       /* default causes this function to call perror() */
	    }
	 break;

      case LVM_QUERYPVPATH:
	    switch (errno) {
	       case ENOTTY:	err_msg = MSG_QUERYPVPATH_ENOTTY;	break;
	       case ENODEV:	err_msg = MSG_QUERYPVPATH_ENODEV;	break;
	       case ENXIO:	err_msg = MSG_QUERYPVPATH_ENOXIO;	break;
	       /* default causes this function to call perror() */
	    }
	 break;

      case LVM_QUERYPVS:
	    switch (errno) {
	       case ENOTTY:	err_msg = MSG_QUERYPVS_ENOTTY;	break;
	       /* default causes this function to call perror() */
	    }
	 break;

      case LVM_RESYNCPV:
	    switch (errno) {
	       case ENOTTY:	err_msg = MSG_RESYNCPV_ENOTTY;	break;
	       /* default causes this function to call perror() */
	    }
	 break;

      case LVM_SETVGID:
	    switch (errno) {
	       case ENOTTY:	err_msg = MSG_SETVGID_ENOTTY;	break;
	       /* default causes this function to call perror() */
	    }
	 break;

      default:
	 debug_msg("No special messages for ioctl 0x%08x\n", ioctl_id);
   }

   /* Either call perror or print our own message */
   if (err_msg == NULL)
      perror(NULL);
   else
      fprintf(stderr, "%s\n", err_msg);

   debug(dbg_exit());
   return(OK);
}
