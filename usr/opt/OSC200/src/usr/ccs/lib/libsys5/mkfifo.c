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

#include <fcntl.h>
#include <sys/errno.h>
#include <stropts.h>
#include <sys/mode.h>

mkfifo(path,mode)
char *path;
mode_t mode;
{
        register int retval;

        /* first make the node */
        retval = open(path,O_RDWR|O_CREAT,mode & ~S_IFIFO,0);
        if(retval == -1)
                return retval;

        close(retval);

        retval = open("/dev/streams/pipe",O_RDWR);
        if(retval == -1)
                return retval;

        if(ioctl(retval,I_FIFO,0) == -1)
                return -1;

        if(fattach(retval,path) == -1)
        {
                close(retval);
                return -1;
        }
        close(retval);
        return 0;
}
