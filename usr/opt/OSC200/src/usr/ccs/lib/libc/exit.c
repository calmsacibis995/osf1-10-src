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
static char	*sccsid = "@(#)$RCSfile: exit.c,v $ $Revision: 4.2.8.3 $ (DEC) $Date: 1993/10/05 21:00:50 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <sys/types.h>  /* for mon.h */
#include <mon.h>        /* for _mondata.proftype definition */
#include <malloc.h>

#ifdef _THREAD_SAFE
#include "rec_mutex.h"

extern struct rec_mutex _exit_rmutex;
#endif

/*
 * NAME: atexit
 *
 * FUNCTION: Special program termination sequence.  atexit() may be used
 *	     to register up to 32 functions that are to be executed before
 *           normal program termination
 *
 * PARAMETERS: 
 *  	     *func() - pointer to function to be executed before normal
 *	   	       program termination
 *
 * RETURN VALUE DESCRIPTIONS:
 *	     - 0 if registration succeeds
 *	     - 1 if registration does not succeed
 *
 */
#define NINC 32

typedef struct atexitarray {
	void (*func)(void);	
} ex_action_t;

/* for those who follow behind, the static array of 32 elements is there
 * to satisfy the ansii standard that requires support for at least 32
 * atexit calls.
 */
static ex_action_t sex_actions[NINC];
static ex_action_t *ex_actions = sex_actions;

static number_of_ex_actions = NINC;

static int action_cnt;


int	
atexit(void (*func)(void))
{
	register ex_action_t *tmp;

#ifdef _THREAD_SAFE
	rec_mutex_lock(&_exit_rmutex);
#endif
        if(action_cnt >= number_of_ex_actions) {
            if(ex_actions == sex_actions) {
                number_of_ex_actions += NINC;
                tmp = (struct atexitarray *)
                                malloc(number_of_ex_actions * sizeof(void *));
                if(tmp == NULL) {
#ifdef _THREAD_SAFE
                    rec_mutex_unlock(&_exit_rmutex);
#endif
                    return(1);
		}
                memcpy(tmp, sex_actions, sizeof(sex_actions));
            } else {
                number_of_ex_actions += NINC;
                tmp = (struct atexitarray *)realloc(ex_actions,
                                number_of_ex_actions * sizeof(void *));
                if(tmp == NULL) {
#ifdef _THREAD_SAFE
                    rec_mutex_unlock(&_exit_rmutex);
#endif
                    return(1);
		}
            }
            ex_actions = tmp;
        }
        ex_actions[action_cnt].func = func;
        action_cnt++;
#ifdef _THREAD_SAFE
	rec_mutex_unlock(&_exit_rmutex);
#endif
	return(0);
}

/*
 * NAME: exit
 *
 * FUNCTION: Normal program termination
 *
 * PARAMETERS: 
 *	     int code - status of exit
 *
 */

void
exit(int status)
{

/*** Leave out until profiling works. ****
	extern struct monglobal _mondata; 
	static struct monglobal *z=&_mondata; 

	if ( z->prof_type != 0 ){    
		monitor((caddr_t)0); 
	}
***/

#ifdef _THREAD_SAFE
	/*
	 * Don't bother unlocking as this would cause a race to get to the
	 * exit.
	 */
	rec_mutex_lock(&_exit_rmutex);
#endif
	while (action_cnt > 0)
		(*ex_actions[--action_cnt].func)();
        /*
         * ldr_atexit calls user termination routines, which must be
         * called before cleanup
         */
	ldr_atexit();	/* last-minute loader/obj file cleanup */
	_cleanup();
	_exit(status);
}
