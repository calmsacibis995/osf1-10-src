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
static char *sccsid  =  "@(#)$RCSfile: safesyscalls.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/10/13 14:40:04 $";
#endif

/*
 *      safesyscalls.c
 *
 *      Description:
 *
 *      This file provides `safe' system calls for use by lprsetup
 *      Hopefully a proper cleanup (or replacement) of lprsetup will
 *      make this unnecessary.
 *
 *      The system calls chmod, chown, rmdir and unlink are considered
 *      dangerous since they have the potentiality for system damage.
 *
 *      The strategy is to allow the operations only in particular sub-trees
 *      of the file system, currently, /usr/spool and /usr/adm
 *
 *      In addition, since the unlink call allows root to unlink populated
 *      directories this is explicitly protected against by preventing the
 *      unlinking of any directory.
 *
 *      So that code which checks for error returns and uses perror()
 *      produces a reasonable error message, errno is set to EROFS in
 *      the event of the call being disallowed.
 *      The associated string is "Restricted operation on a file system"
 *      which is fairly appropriate.
 *
 *      Modification History:
 *
 * 03-Mar-1991 - Adrian Thoms
 *      First version.
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>

#ifdef LOCAL
#define SPOOLAREA "~/spool"
#define ADMAREA "~/adm"
#else
#define SPOOLAREA "/usr/spool"
#define ADMAREA "/usr/adm"
#endif

char *safe_dirs[] = 
{
    SPOOLAREA,
    ADMAREA,
    NULL
};

static int issafedir(char *path)
{/* check that the path to the file is 'safe' */
    register char **dirp;

    for(dirp=safe_dirs; *dirp; dirp++) 
    {/* scan through the acceptable paths */
        if (!strncmp(*dirp, path, strlen(*dirp))) 
        {/* if the path is acceptable return when we find it */
            return 1;
        }
    }

    /* the path was not one of the acceptable paths */
    printf("lprsetup not allowed to ");
    return 0;
}

int safechmod(char *path, mode_t mode)
{ /* do a chmod only if the file is on an acceptable path */

    if (issafedir(path)) 
    { /* the path is acceptable so do it */
        return chmod(path, mode);
    } 
    else 
    { /* the path was not one of the acceptable paths, return an error */
        printf("chmod(%s, 0%o)\n", path, mode);
        errno = EROFS;
        return -1;
    }
}

int safechown(char *path, uid_t owner, gid_t group)
{ /* do a chown only if the file is on an acceptable path */
    if (issafedir(path)) 
    { /* the path is acceptable so do it */
        return chown(path, owner, group);
    } 
    else 
    { /* the path was not one of the acceptable paths, return an error */
        printf("chown(%s, %d, %d)\n", path, owner, group);
        errno = EROFS;
        return -1;
    }
}

int safeunlink(char *path)
{ /* do an unlink only if the file is on an acceptable path */
    struct stat sb;

    if (issafedir(path)) 
    { /* the path is acceptable so do it */
        /* check the file type */
        stat(path, &sb);
        if ((sb.st_mode & S_IFMT) != S_IFDIR) 
        { /* the file is not a directory so it is OK to unlink */
            return unlink(path);
        } 
        else 
        { /* the file was a directory, issue a warning and return an error */
            printf("lprsetup not allowed to unlink directory %s\n",
                   path);
            errno = EROFS;
            return (-1);
        }
    } 
    else 
    { /* the path was not one of the acceptable paths, return an error */
        printf("unlink(%s)\n", path);
        errno = EROFS;
        return (-1);
    }
}

int safermdir(char *path)
{ /* do an rmdir only if the directory is on an acceptable path */
    if (issafedir(path)) 
    { /* the path is acceptable so do it */
        return rmdir(path);
    } 
    else 
    { /* the path was not one of the acceptable paths, return an error */
        printf("rmdir(%s)\n", path);
        errno = EROFS;
        return (-1);
    }
}
