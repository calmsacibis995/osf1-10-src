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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: catio.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 16:45:23 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */

/***
 ***  "catio.c	1.5  com/cmd/msg,3.1,9013 10/24/89 11:01:19";
 ***/
/*
 * COMPONENT_NAME:  CMDMSG
 *
 * FUNCTIONS: descopen, descgets, descclose, descset, descerrck
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
FILE *descfil;

/*
 * EXTERNAL PROCEDURES CALLED: None
 */


/*
 * NAME: descopen
 *
 * FUNCTION: Perform message utility input operations
 *
 * EXECUTION ENVIRONMENT:
 *	User Mode.
 *
 * NOTES: The routines in this file are used to provide a mechanism by which
 *     	  the routines in gencat can get the next input character without
 *  	  worrying about details such as if the input is coming from a file
 *  	  or standard input, etc.
 *
 * RETURNS : 0 - successful open
 *          -1 - open failure
 */


/*   Open a catalog descriptor file and save file descriptor          */ 

descopen(filnam) 
char *filnam;
			/*
			  filnam - pointer to message catalog name
			*/
{
	if ((descfil = fopen(filnam,"r")) == 0) 
		return (-1);
	
return (0);
}



/*
 * NAME: descgets
 *
 * FUNCTION: Read message catalog
 *
 * EXECUTION ENVIRONMENT:
 *	User mode.
 *
 * NOTES: Read the next line from the opened message catalog descriptor file.
 *
 * RETURNS: Pointer to message buffer -- scccessful
 *          NULL pointer -- error or end-of-file
 */


char *descgets(buff, bufsize)
char *buff; int bufsize;
			/*
			  buff - pointer to message buffer
			  bufsize - size of message buffer in bytes
			*/
{
	char *str;

	str = fgets(buff, bufsize, descfil);
	buff[bufsize-1] = '\0';        /* terminate in case length exceeded */
	return (str);
}



/*
 * NAME: descclose
 *
 * FUNCTION: Close message catalog
 *
 * EXECUTION ENVIRONMENT:
 *	User mode.
 *
 * NOTES: Close the message catalog descriptor file.
 *
 * RETURNS: None
 */


descclose()
{
	fclose(descfil);
	descfil = 0;
}



/*
 * NAME: descset
 *
 * FUNCTION: Establish message catalog file descriptor
 *
 * EXECUTION ENVIRONMENT:
 *	User mode.
 *
 * NOTES: Set the file descriptor to be used for message catalog access.
 *        
 *
 * RETURNS: None
 */


descset(infil)
FILE *infil;
{
	descfil = infil;
}


/*
 * NAME: descerrck
 *
 * FUNCTION: Check I/O stream status
 *
 * EXECUTION ENVIRONMENT:
 *	User mode.
 *
 * RETURNS: 0 - no error encountered
 *         -1 - error encountered
 */

descerrck()
{
	if (ferror(descfil))
		return(-1);
	else    return(0);
}

