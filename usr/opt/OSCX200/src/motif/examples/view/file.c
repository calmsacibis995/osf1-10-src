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
static char *rcsid = "@(#)$RCSfile: file.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 19:24:00 $";
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
static char rcsid[] = "$RCSfile: file.c,v $ $Revision: 1.1.2.2 $ $Date: 1993/11/05 19:24:00 $"
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include "file.h"

#ifndef VMS
#ifndef X_NOT_STDC_ENV
#include <unistd.h>
#endif
#endif

/* =====================================================================
 * Open File 
 */

#ifdef _NO_PROTO
FILE * OpenFile(path)
		char *path;
#else
FILE * OpenFile(char *path)
#endif
{
   return  fopen(path, "r");
}

/* =====================================================================
 * Close File
 */

#ifdef _NO_PROTO
void CloseFile(file)
		FILE * file;
#else
void CloseFile(FILE * file)
#endif
{
   fclose (file);
}

/* =====================================================================
 * Read File in buffer
 */

#ifdef _NO_PROTO
char * ReadFile(file, filesize)
		FILE * file;
		int *filesize;
#else
char * ReadFile(FILE * file, int *filesize)
#endif
{
   char * buffer;

   fseek(file, 0L, SEEK_END);
   * filesize = ftell(file);
/*   fgetpos(file, (fpos_t *) filesize);*/
   rewind(file);
   buffer = (char *) XtMalloc(*filesize+1);
   if (fread(buffer, 1, *filesize, file) == *filesize ) {
      buffer[*filesize] = '\0';
      return buffer;
   }
   XtFree(buffer);
   return NULL;
}
