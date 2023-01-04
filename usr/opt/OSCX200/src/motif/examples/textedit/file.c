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
static char *rcsid = "@(#)$RCSfile: file.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 18:42:34 $";
#endif
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: file.c,v $ $Revision: 1.1.2.2 $ $Date: 1993/11/05 18:42:34 $"
#endif
#endif

/************************************************************
 *     file.c -- Code for dealing with files and filenames
 *
 *  Contains code to read, write, copy, move & remove files
 *     
 ************************************************************/

#include <limits.h>
#include <stdio.h>

#include "basic.h"

/************************************************************
 * Remove File
 ************************************************************/

FileRemove( filnam )
    char *filnam;
{
    return -1;
}

/************************************************************
 * Save Text to File
 ************************************************************/

void FileSaveText( fil, textchars )
    FILE *fil;
    char *textchars;
{
    rewind( fil );
    fprintf( fil, "%s", textchars );
    fflush( fil );
}

/************************************************************
 * Read Text from File
 ************************************************************/

char *FileGetText( fil )
    FILE *fil;
{
    char *textchars;
    int position = 0;
    int num;

    textchars = BasicMalloc( BUFSIZ );
    rewind( fil );
    while ( (num = read( fileno(fil),
                         textchars+position, BUFSIZ)) > 0 ) {
      position += num;
      textchars = BasicRealloc( textchars, position+BUFSIZ );
    }
    *( textchars+position ) = 0;
    return textchars;
}

/************************************************************
 * Return Trailing part of current filename
 ************************************************************/

char *FileTrailingPart( filnam )
    char *filnam;
{
    char *trailnam;
    while (*filnam != '\0') filnam++;
#ifdef VMS
    while (*filnam != ']') filnam--;
#else
    while (*filnam != '/') filnam--;
#endif
    filnam++;
    strdup( trailnam, filnam );
    return trailnam;
}
