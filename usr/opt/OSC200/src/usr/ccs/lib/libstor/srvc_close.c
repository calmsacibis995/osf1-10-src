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
static char *rcsid = "@(#)$RCSfile: srvc_close.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/07/27 18:07:40 $";
#endif

/************************************************************************
 *
 *  ROUTINE NAME: rd_srvc_close()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine handles the close system call for the services 
 *	driver. It provides no function today...it's only use is for
 *	symmetry (i.e. open/close).
 *
 *  FORMAL PARAMETERS:
 *	None.
 *
 *  IMPLICIT INPUTS:
 *	None.
 *
 *  IMPLICIT OUTPUTS:
 *	none
 *
 *  RETURN VALUE:
 *	0	- indicates success.
 *
 *  SIDE EFFECTS:
 *	None.
 *
 *  ADDITIONAL INFORMATION:
 *	None.
 *
 ************************************************************************/
int
rd_srvc_close(fd)
int fd;
{

	return (close(fd));

}

