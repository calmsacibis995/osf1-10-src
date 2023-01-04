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
static char	*sccsid = "@(#)$RCSfile: getcmd.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/10/19 14:19:28 $";
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

/*
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */


/*
 * FUNCTION: /etc/inittab file parser for /etc/init
 */

#include	<errno.h>
#include	<ctype.h>
#include	"init.h"
#if defined(NLS) || defined(KJI)
#include	<sys/NLchar.h>
#endif
#include	<stdio.h>

#ifdef	MSG
#include	"init_msg.h"
#endif



/**********************/
/****    getcmd    ****/
/**********************/

/*	"getcmd" parses lines from /etc/inittab.  Each time it finds	*/
/*	a command line it will return TRUE as well as fill the passed	*/
/*	CMD_LINE structure and the shell command string.  When the end	*/
/*	of /etc/inittab is reached, FALSE is returned.			*/
/*									*/
/*	/etc/inittab is automatically opened if it is not currently	*/
/*	open and is closed when the end of the file is reached.		*/

static FILE    *fp_inittab = NULL;

getcmd(cmd, shcmd)
	register struct CMD_LINE *cmd;
	char           *shcmd;
{
	extern FILE    *fp_inittab;
	int             i, answer, proceed, errnum;
	register char  *ptr;
	register int    c;
	register int    state;
	char            lastc, *ptr1;
	extern int      errno;
	static char    *actions[] = {
		       "off", "respawn", "ondemand", "once", "wait", "boot",
			"bootwait", "powerfail", "powerwait", "initdefault",
			     "sysinit",
	};
	static short    act_masks[] = {
	 M_OFF, M_RESPAWN, M_ONDEMAND, M_ONCE, M_WAIT, M_BOOT, M_BOOTWAIT,
	 M_PF, M_PWAIT, M_INITDEFAULT, M_SYSINIT,
	};

#ifdef	XDEBUG
	debug("We have entered getcmd().\n");
#endif
	if (fp_inittab == NULL) {
		/* Be very persistent in trying to open /etc/inittab.   */
		for (i = 0; i < 3; i++) {
			if ((fp_inittab = fopen(INITTAB, "r")) != NULL)
				break;
			else {
				errnum = errno;	/* Remember for error message */
				timer(3);	/* Wait 3 seconds to see if
						 * file appears. */
			}
		}
		/*
		 * If unable to open /etc/inittab, print error message and
		 * return FALSE to caller.
		 */
		if (fp_inittab == NULL) {
#ifdef MSG
			console(NLgetamsg(MF_INIT, MS_INIT, M_OPEN,
			   "Cannot open %s  errno: %d\n"), INITTAB, errnum);
#else
			console("Cannot open %s  errno: %d\n", INITTAB, errnum);
#endif
			return (FAILURE);
		}
	}
	/*
	 * Keep getting commands from /etc/inittab until you find a good
	 * one or run out of file.
	 */
	for (answer = FALSE; answer == FALSE;) {
		/* Zero out the cmd itself before trying next line. */
		memset((char *) cmd, NULL, sizeof(struct CMD_LINE));
		/*
		 * Read in lines of /etc/inittab, parsing at colons, until a
		 * line is read in which doesn't end with a backslash.  Do not
		 * start if the first character read is an EOF.  Note that this
		 * means that should a line fail to end in a newline, it will
		 * still be processed, since the "for" will terminate normally
		 * once started, regardless of whether line terminates with a
		 * newline or an EOF.
		 */
		state = FAILURE;
		if ((c = fgetc(fp_inittab)) != EOF) {
			if (c == '#') {
				while ((c = fgetc(fp_inittab)) != '\n' &&
								c != EOF)
					;
				continue;
			}
			for (proceed = TRUE, ptr = shcmd, state = ID, lastc = '\0';
			     proceed && c != EOF;
			     lastc = c, c = fgetc(fp_inittab)) {
				/*
				 * If we are not in the FAILURE state and
				 * haven't yet reached the shell command field,
				 * process the line, otherwise just look for a
				 * real end of line.
				 */
				if (state != FAILURE && state != COMMAND) {

					/* Squeeze out spaces and tabs. */
					if (c == ' ' || c == '\t')
						continue;
					/*
					 * If the character is a ':', then check
					 * the previous field for correctness
					 * and advance to the next field. 
					 */
					if (c == ':') {
						switch (state) {
						/*
						 * Check to see that there are
						 * up to IDENT_LEN characters
						 * for id. 
						 */
						case ID:

						if ((i = ptr - shcmd) < 1 || i > IDENT_LEN) {
							state = FAILURE;
						}
						else {
							memcpy(cmd->c_id, shcmd, i);
							ptr = shcmd;	/* Reset pointer */
							state = LEVELS;
						}
						break;

						case LEVELS:
						/*
						 * Build a mask for all the
						 * levels that this command
						 * will be legal
						 */
						for (cmd->c_levels = 0, ptr1 = shcmd; ptr1 < ptr; ptr1++) {
							if (*ptr1 >= '0' && *ptr1 <= '9')
								cmd->c_levels |= (MASK0 << (*ptr1 - '0'));
							else if (*ptr1 >= 'a' && *ptr1 <= 'c')
								cmd->c_levels |= (MASKa << (*ptr1 - 'a'));
							else if (*ptr1 == 's' || *ptr1 == 'S')
								cmd->c_levels |= MASKSU;
							else if (*ptr1 == 'm' || *ptr1 == 'M')
								cmd->c_levels |= MASKSU;
							else {
								state = FAILURE;
								break;
							}
						}
						if (state != FAILURE) {
							state = ACTION;
							ptr = shcmd;	/* Reset the buffer */
						}
						break;

						case ACTION:
						/*
						 * Null terminate string in
						 * shcmd buffer and then try to
						 * match against legal actions.
						 * If the field is of length 0,
						 * then the default of "RESPAWN"
						 * is used if the id is numeric,
						 * otherwise the default is "OFF"
						 */
						if (ptr == shcmd) {
							if (isdigit(cmd->c_id[0])
							    && (cmd->c_id[1] == '\0' || isdigit(cmd->c_id[1]))
							    && (cmd->c_id[2] == '\0' || isdigit(cmd->c_id[2]))
							    && (cmd->c_id[3] == '\0' || isdigit(cmd->c_id[3]))
							    && (cmd->c_id[4] == '\0' || isdigit(cmd->c_id[4]))
							    && (cmd->c_id[5] == '\0' || isdigit(cmd->c_id[5]))
							    && (cmd->c_id[6] == '\0' || isdigit(cmd->c_id[6]))
							    && (cmd->c_id[7] == '\0' || isdigit(cmd->c_id[7]))
							    && (cmd->c_id[8] == '\0' || isdigit(cmd->c_id[8]))
							    && (cmd->c_id[9] == '\0' || isdigit(cmd->c_id[9]))
							    && (cmd->c_id[10] == '\0' || isdigit(cmd->c_id[10]))
							    && (cmd->c_id[11] == '\0' || isdigit(cmd->c_id[11]))
							    && (cmd->c_id[12] == '\0' || isdigit(cmd->c_id[12]))
							    && (cmd->c_id[13] == '\0' || isdigit(cmd->c_id[13])))
								cmd->c_action = M_RESPAWN;
							else
								cmd->c_action = M_OFF;
						}
						else {
							for (cmd->c_action = 0, i = 0, *ptr = '\0';
							     i < sizeof(actions) / sizeof(char *); i++) {
								if (strcmp(shcmd, actions[i]) == 0) {
										cmd->c_action = act_masks[i];
									break;
								}
							}
						}
						/*
						 * If the action didn't match
						 * any legal action, set state
						 * to FAILURE.
						 */
						if (cmd->c_action == 0)
							state = FAILURE;
						else {
							state = COMMAND;
							/*
							 * Insert the prefix
							 * string of "exec "
							 * into the command
							 * buffer before
							 * inserting any
							 * characters.
							 */
							strcpy(shcmd, "exec ");
						}
						ptr = shcmd + EXEC;
						break;
						}	/* switch(state) */
						continue;
					}	/* end if(c == :) */
				}	/* end if (state != FAILURE) */
				/*
				 * If the character is a '\n', then this is
				 * the end of a line. If the '\n' wasn't
				 * preceded by a backslash, it is also the end 
				 * of an /etc/inittab command.  If it was
				 * preceded by a backslash then the next line is
				 * a continuation. Note that the continuation
				 * '\n' falls through and is treated like other 
				 * characters and is stored in the shell
				 * command line. 
				 */
				if (c == '\n')
					if (lastc != '\\') {
						proceed = FALSE;
						*ptr = '\0';
						break;
					}
				/*
				 * For all other characters just stuff them into
				 * the command as long as there aren't too many
				 * of them. Make sure there is room for a
				 * terminating '\0' also.
				 */
				if (ptr >= (shcmd + MAXCMDL - 1))
					state = FAILURE;
				else
					*ptr++ = c;
				/*
				 * If the character we just stored was a quoted
				 * backslash, then change "c" to '\0', so that
				 * this backslash will not cause a subsequent
				 * '\n' to appear quoted.  In otherwords
				 * '\' '\' '\n' is the real end of a command,
				 * while '\''\n' is a continuation. 
				 */
				if (c == '\\' && lastc == '\\')
					c = '\0';
			}	/* end for(proceed == TRUE...) */
		}
		/*
		 * Make sure all the fields are properly specified for a good
		 * command line.
		 */
		if (state == COMMAND) {
			answer = TRUE;
			cmd->c_command = shcmd;
			/*
			 * If no default level was supplied, insert all
			 * numerical levels. 
			 */
			if (cmd->c_levels == 0)
				cmd->c_levels = MASK0 | MASK1 | MASK2 | MASK3 | MASK4 | MASK5 | MASK6 | MASK7 | MASK8 | MASK9;
			/*
			 * If no action has been supplied, declare this entry
			 * to be OFF. 
			 */
			if (cmd->c_action == 0)
				cmd->c_action = M_OFF;
			/*
			 * If no shell command has been supplied, make sure
			 * there is a null string in the command field.
			 * EXEC is the length of the string "exec " minus
			 * null at the end. 
			 */
			if (ptr == (shcmd + EXEC))
				*shcmd = '\0';
		}
		else
			answer = FALSE;
		/*
		 * If we have reached the end of /etc/inittab, then close it
		 * and quit trying to find a good command line.
		 */
		if (c == EOF) {
			fclose(fp_inittab);
			fp_inittab = NULL;
			break;
		}
	}			/* end for(answer = FALSE...) */
	return (answer);
}
