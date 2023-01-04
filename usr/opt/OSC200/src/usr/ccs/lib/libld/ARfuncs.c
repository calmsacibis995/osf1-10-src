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
static char	*sccsid = "@(#)$RCSfile: ARfuncs.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:05:33 $";
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
#ifndef lint
#ifndef _NOIDENT

#endif
#endif

/*
 * libld: ARfuncs
 *
 * ORIGIN: ISC
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 * FUNCTION: Functions to process archives without the user having to
 *	     know what format the archive is in.
 *
 */

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <ar.h>
#include <ldfcn.h>

/*
 * NAME: ARisarchive
 *
 * FUNCTION: Given a FILE, determine an archive starts at the current
 *	     position in the file.  Leave the file positioned as it was
 *	     when this function was entered. 
 *
 * PARAMETERS: File descriptor of file to check.
 *
 * RETURN VALUE DESCRIPTION:
 *	      0 if not an archive,
 *	      1 if an archive
 */
int ARisarchive(FILE *file)
{
  char magic[SARMAG];
  long curloc = ftell(file);

  if (fread((void *)&magic, (size_t)1, (size_t)SARMAG, file) < SARMAG)
    return(0);

  fseek(file,curloc,SEEK_SET);

  return (!strncmp(magic,ARMAG,SARMAG));
}


/*
 * NAME: ARforeach
 *
 * FUNCTION: Given a FILE, call the function parameter once for each
 *	     member of the archive, passing the FILE (positioned to the
 *	     beginning of the archive-member) and the arbuf_t structure.
 *
 * PARAMETERS: File descriptor for archive file.
 *	     Pointer to function to call for each member.
 *
 * RETURN VALUE DESCRIPTION:
 *	      0 for success,
 *	     -1 for internal problems, or
 *	     fcn return code if it is ever non-zero.
 */
int ARforeach(FILE *file, ARfunc_t fcn)
{

  int			retval;		/* Track error values  */
  struct arbuf_t	arbuf;		/* Archive-member numeric values */
  struct ar_hdr		ariobuf;	/* The raw ascii, in from the disk */
  char			magic[SARMAG];	/* Magic # for an archive */

  /* Read in the archive magic-number */

  retval=fread( (void *)&magic, 1, SARMAG, file);

  if (retval != SARMAG)
    return (-1);			/* Internal errors */

  if (strncmp(magic,ARMAG,SARMAG))	/* Oops - not really archive */
    return (-1);

  /* For each member of the archive */
  while (!feof(file))
    {
      off_t	pos;
      char	*p;
      /* Read in the member header */

      if (fread(&ariobuf, sizeof(ariobuf), (size_t)1, file)!=1)

	return(-1);

      (void)strncpy(arbuf.ar_name, ariobuf.ar_name, sizeof(arbuf.ar_name));
      p = strchr(arbuf.ar_name, '/');
      if(p)
	*p = '\0';			/* Mark end of member */
      else if (p=strchr(arbuf.ar_name, ' '))
	*p = '\0';

      arbuf.ar_date=atol(ariobuf.ar_date);
      arbuf.ar_uid=atol(ariobuf.ar_uid);
      arbuf.ar_gid=atol(ariobuf.ar_gid);
      arbuf.ar_mode=strtol(ariobuf.ar_mode,(char **)NULL,8);
      arbuf.ar_size=atol(ariobuf.ar_size);

      /* Member size is rounded up to word */ 
      arbuf.ar_size++; arbuf.ar_size &= ~1;

      /* One more sanity-check on the archive header! */
      if (strncmp(ariobuf.ar_fmag, ARFMAG, sizeof(ariobuf.ar_fmag)))
	return (-1);

      pos = ftell(file) + arbuf.ar_size;

      /*
       * Check for Archive index if present and jump around it!
       */
      if (!strncmp( arbuf.ar_name,  "__.SYMDEF", strlen("__.SYMDEF")))
	goto next;
      
      /* Call specified routine and check for errors */

      if (retval=(*fcn)(file, &arbuf))
	return(retval);

next:	
      /* Go to next member in archive */
      if (fseek(file,pos,0) != 0) {
	return(-1);
      }
    } /* while */

  return(0);
}
