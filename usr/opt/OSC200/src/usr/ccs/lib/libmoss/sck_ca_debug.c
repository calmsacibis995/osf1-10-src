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
static char *rcsid = "@(#)$RCSfile: sck_ca_debug.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 15:59:57 $";
#endif
#ifndef lint
static char *sccsid = "%W%      ULTRIX %G%" ;
#endif
/****************************************************************************
 *
 * Copyright (c) Digital Equipment Corporation, 1991, 1992, 1993.
 * All Rights Reserved.  Unpublished rights reserved
 * under the copyright laws of the United States.
 *
 * The software contained on this media is proprietary
 * to and embodies the confidential technology of
 * Digital Equipment Corporation.  Possession, use,
 * duplication or dissemination of the software and
 * media is authorized only pursuant to a valid written
 * license from Digital Equipment Corporation.
 *
 * RESTRICTED RIGHTS LEGEND   Use, duplication, or
 * disclosure by the U.S. Government is subject to
 * restrictions as set forth in Subparagraph (c)(1)(ii)
 * of DFARS 252.227-7013, or in FAR 52.227-19, as
 * applicable.
 *
 ****************************************************************************
 *
 *
 * Facility:
 *
 *    Management - POLYCENTER (tm) Common Agent
 *
 * Abstract:
 *
 *    The following routines support debug code in the common agent. 
 *
 * Author:
 *
 *    Pat Mulligan, copied from ca_debug.c
 *
 * Date:
 *
 *    January 5th, 1993 
 *
 * Revision History :
 *
 */

#if defined(DEBUG) && !defined(NOIPC)
#include <rpc.h>
#include <rpcexc.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


char *error_text(st)

/*
 * Function description:
 *
 *    prints out error text
 *
 * Arguments:
 *
 *    st        in        Thec error status
 *
 * Return values:
 *
 *    None.
 *
 * Side effects:
 *
 *    None.
 */

unsigned int st;
{
  char *buf = NULL;

  return buf;
}


void print_exception(e)
      EXCEPTION *e;
/*
 * Function description:
 *
 *    prints out an exception
 *
 * Arguments:
 *
 *    e         in        The exception
 *
 * Return values:
 *
 *    None.
 *
 * Side effects:
 *
 *    None.
 */

{
      char *t = NULL;

      /* CMA exceptions */

      if (exc_matches(e,&exc_e_uninitexc)) t = "exc_e_uninitexc";
        else if (exc_matches(e,&exc_e_illaddr)) t = "exc_e_illaddr";
	else if (exc_matches(e,&exc_e_exquota)) t = "exc_e_exquota";
	else if (exc_matches(e,&exc_e_insfmem)) t = "exc_e_insfmem";
	else if (exc_matches(e,&exc_e_nopriv)) t = "exc_e_nopriv";
	else if (exc_matches(e,&exc_e_illinstr)) t = "exc_e_illinstr";
	else if (exc_matches(e,&exc_e_resaddr)) t = "exc_e_resaddr";
	else if (exc_matches(e,&exc_e_privinst)) t = "exc_e_privinst";
	else if (exc_matches(e,&exc_e_resoper)) t = "exc_e_resoper";
	else if (exc_matches(e,&exc_e_SIGTRAP)) t = "exc_e_SIGTRAP";
	else if (exc_matches(e,&exc_e_SIGIOT)) t = "exc_e_SIGIOT";
	else if (exc_matches(e,&exc_e_SIGEMT)) t = "exc_e_SIGEMT";
	else if (exc_matches(e,&exc_e_aritherr)) t = "exc_e_aritherr";
	else if (exc_matches(e,&exc_e_SIGSYS)) t = "exc_e_SIGSYS";
	else if (exc_matches(e,&exc_e_SIGPIPE)) t = "exc_e_SIGPIPE";
	else if (exc_matches(e,&exc_e_excpu)) t = "exc_e_excpu";
	else if (exc_matches(e,&exc_e_exfilsiz)) t = "exc_e_exfilsiz";
	else if (exc_matches(e,&exc_e_intovf)) t = "exc_e_intovf";
	else if (exc_matches(e,&exc_e_intdiv)) t = "exc_e_intdiv";
	else if (exc_matches(e,&exc_e_fltovf)) t = "exc_e_fltovf";
	else if (exc_matches(e,&exc_e_fltdiv)) t = "exc_e_fltdiv";
	else if (exc_matches(e,&exc_e_fltund)) t = "exc_e_fltund";
	else if (exc_matches(e,&exc_e_decovf)) t = "exc_e_decovf";
	else if (exc_matches(e,&exc_e_subrng)) t = "exc_e_subrng";
	else if (exc_matches(e,&cma_e_alerted)) t = "cma_e_alerted";
	else if (exc_matches(e,&cma_e_assertion)) t = "cma_e_assertion";
	else if (exc_matches(e,&cma_e_badparam)) t = "cma_e_badparam";
	else if (exc_matches(e,&cma_e_bugcheck)) t = "cma_e_bugcheck";
	else if (exc_matches(e,&cma_e_exit_thread)) t = "cma_e_exit_thread";
	else if (exc_matches(e,&cma_e_existence)) t = "cma_e_existence";
	else if (exc_matches(e,&cma_e_in_use)) t = "cma_e_in_use";
	else if (exc_matches(e,&cma_e_use_error)) t = "cma_e_use_error";
	else if (exc_matches(e,&cma_e_wrongmutex)) t = "cma_e_wrongmutex";
	else if (exc_matches(e,&cma_e_stackovf)) t = "cma_e_stackovf";
	else if (exc_matches(e,&cma_e_nostackmem)) t = "cma_e_nostackmem";
	else if (exc_matches(e,&cma_e_notcmastack)) t = "cma_e_notcmastack";
	else if (exc_matches(e,&cma_e_unimp)) t = "cma_e_unimp";
	else if (exc_matches(e,&cma_e_inialrpro)) t = "cma_e_inialrpro";
	else if (exc_matches(e,&cma_e_defer_q_full)) t = "cma_e_defer_q_full";
	else if (exc_matches(e,&cma_e_signal_q_full)) t = "cma_e_signal_q_full";
	else if (exc_matches(e,&cma_e_alert_nesting)) t = "cma_e_alert_nesting";


      /* 
       *  If we found a named exception that matches print it, otherwise check
       *  if it is a status and display that if possible.
       */

      if (t != NULL)
          printf("***FAILURE***: unexpected CMA exception %s\n",t);
      else
      {
          if (e->kind ==exc_kind_status_c)
          {
#ifndef VMS
              printf("***FAILURE***: CMA status exception %d (%s)\n",
		e->status.status, strerror(e->status.status));
#else
              printf("***FAILURE***: CMA status exception %d (%s)\n",
		e->status-kind.status, strerror(EVMSERR,e->status_kind.status));
#endif      
          }
          else if (e->kind == exc_kind_address_c)
              printf("***FAILURE***: unexpected user-defined CMA exception address 0x%x\n",e->address.address);
          else
              printf("***FAILURE***: corrupted CMA exception\n",t);
      }
}

#else

#include <stdio.h>

char *error_text()

{
	return "error_text() not implemented for NOIPC";
}

void print_exception()

{
	printf("print_exception() not implemented for NOIPC\n");
}

#endif /* NOIPC && DEBUG */
