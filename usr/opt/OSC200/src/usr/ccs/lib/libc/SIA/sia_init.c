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
static char *rcsid = "@(#)$RCSfile: sia_init.c,v $ $Revision: 1.1.11.3 $ (DEC) $Date: 1993/06/08 01:19:55 $";
#endif
/*****************************************************************************
* Usage:  int sia_init()
*
* Description: The purpose of this routine is to do initialization of the
* mechanisms with respect to the system not a particular user or session.
* The first process running this code will open and lock the SIAINITLOCK 
* file. All subsequent processes will find the initialization file locked 
* and return with the SIA initialization as either SUCCESS or FAIL. This 
* routine provides each of the configured security mechanisms with an 
* initial system startup call.
*
* NOTE: Single user sia_init is doen by init.
*
*
* Parameter Descriptions:  none
*
*
* Assumed Inputs: The system default passport.
*
* Success return: SIASUCCESS or SIAFAIL 
*
* Error Conditions and returns otherthan parameter syntax checks listed above:
*
* 	Condition1: loader failure to load libsia or a package within libsia
*	Return: SIAFAIL
*
*	Condition2: Mechanism initialization error.
*	Return:	SIAFAIL
*
*****************************************************************************/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak sia_init = __sia_init
#endif
#include "siad.h"
#include "siad_bsd.h"

void __sia_init_thread_mutexes();
void __sia_reinit_thread_mutexes();

int 
  sia_init (void)
{
	struct stat siastatbuf;
	if(sia_initialized == SIAGOOD) /** continue to try to init the matrix until good **/
		{
		return(SIASUCCESS);
		}
	else	if(sia_initialized == SIANEW)
			{
			if(stat(SIAIGOODFILE,&siastatbuf) == 0)
				{
				if(sia_mat_init() == SIAFAIL)
					return(SIAFAIL);
				else	{
   			 		sia_initialized =  SIAGOOD;
					return(SIASUCCESS);
					}
				}
			return(SIAFAIL);
			}
}
#ifdef _THREAD_SAFE
void __sia_init_thread_mutexes()
{
 int i;

 for (i = 0; i < SIAMUTMAX; i++)
    rec_mutex_init(&sia_mutex[i]);
}
void __sia_reinit_thread_mutexes()
{
 int i;

 for (i = 0; i < SIAMUTMAX; i++)
    rec_mutex_reinit(&sia_mutex[i]);
}
#endif 
