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
static char	*sccsid = "@(#)$RCSfile: usersmtp.c,v $ $Revision: 4.2.2.4 $ (DEC) $Date: 1992/12/09 08:18:48 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

# include "sendmail.h"

#if !defined(lint) && !defined(_NOIDENT)
#ifdef SMTP

#else

#endif
#endif
/*
 * Copyright (c) 1983 Eric P. Allman
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
#ifndef lint
#ifdef SMTP

#else

#endif
#endif 
*/
# include <sysexits.h>
# include <errno.h>

# ifdef SMTP

/*
**  USERSMTP -- run SMTP protocol from the user end.
**
**	This protocol is described in RFC821.
*/

#define REPLYTYPE(r)	((r) / 100)		/* first digit of reply code */
#define REPLYCLASS(r)	(((r) / 10) % 10)	/* second digit of reply code */
#define SMTPGOODREPLY	250			/* positive SMTP response */
#define SMTPCLOSING	421			/* "Service Shutting Down" */

char	SmtpMsgBuffer[MAXLINE];		/* buffer for commands */
char	SmtpReplyBuffer[MAXLINE];	/* buffer for replies */
char	SmtpError[MAXLINE] = "";	/* save failure error messages */
bool	SmtpNeedIntro;			/* set before first error */
FILE	*SmtpOut;			/* output file */
FILE	*SmtpIn;			/* input file */
int	SmtpPid;			/* pid of mailer */

/* following represents the state of the SMTP connection */
int	SmtpState;			/* connection state, see below */

#define SMTP_CLOSED	0		/* connection is closed */
#define SMTP_OPEN	1		/* connection is open for business */
#define SMTP_SSD	2		/* service shutting down */
/*
**  SMTPINIT -- initialize SMTP.
**
**	Opens the connection and sends the initial protocol.
**
**	Parameters:
**		m -- mailer to create connection to.
**		pvp -- pointer to parameter vector to pass to
**			the mailer.
**
**	Returns:
**		appropriate exit status -- EX_OK on success.
**		If not EX_OK, it should close the connection.
**
**	Side Effects:
**		creates connection and sends initial protocol.
*/

jmp_buf	CtxGreeting;

smtpinit(m, pvp, e)
	struct mailer *m;
	char **pvp;
	ENVELOPE *e;
{
	register int r;
	time_t SavedReadTimeout;
	char buf[MAXNAME];
	extern greettimeout();

	/*
	**  Open the connection to the mailer.
	*/

	if (SmtpState == SMTP_OPEN)
		syserr("smtpinit: already open");

	SmtpIn = SmtpOut = NULL;
	SmtpState = SMTP_CLOSED;
	SmtpError[0] = '\0';
	SmtpNeedIntro = TRUE;
	SmtpPhase = "user open";
	setproctitle("%s %s: %s", CurEnv->e_id, pvp[1], SmtpPhase);
	SmtpPid = openmailer(m, pvp, (ADDRESS *) NULL, TRUE, &SmtpOut, &SmtpIn);
	if (SmtpPid < 0)
	{
		if (tTd(18, 1))
			printf("smtpinit: cannot open %s: stat %d errno %d\n",
			   pvp[0], ExitStat, errno);
		if (CurEnv->e_xfp != NULL)
		{
			register char *p;
			extern char *errstring();
			extern char *statstring();

			if (errno == 0)
			{
				p = statstring(ExitStat);
				fprintf(CurEnv->e_xfp,
					"%.3s %s (%s)... %s\n",
					p, pvp[1], m->m_name, p);
			}
			else
			{
				r = errno;
				fprintf(CurEnv->e_xfp,
					"421 %s (%s)... Deferred: %s\n",
					pvp[1], m->m_name, errstring(errno));
				errno = r;
			}
		}
		return (ExitStat);
	}
	SmtpState = SMTP_OPEN;

	/*
	**  Get the greeting message.
	**	This should appear spontaneously.  Give it five minutes to
	**	happen.
	*/

	SmtpPhase = "greeting wait";
	setproctitle("%s %s: %s", CurEnv->e_id, CurHostName, SmtpPhase);
	/* change reply() timout to be timeout for a greeting */
	SavedReadTimeout = ReadTimeout;
	if (ReadTimeout > 300)
		ReadTimeout = 300;
	r = reply(m);
	ReadTimeout = SavedReadTimeout;
	if (REPLYTYPE(r) == 5)
	{
		/*
		 * for the case when remote mailer didn't
		 * even get off the ground...
		 */
		SmtpState = SMTP_CLOSED;
		goto unavailable;
	}
	if (r < 0 || REPLYTYPE(r) != 2)
		goto tempfail;

	/*
	**  Send the HELO command.
	**	My mother taught me to always introduce myself.
	*/

	smtpmessage("HELO %s", m, MyHostName);
	SmtpPhase = "HELO wait";
	setproctitle("%s %s: %s", CurEnv->e_id, CurHostName, SmtpPhase);
	r = reply(m);
	if (r < 0)
		goto tempfail;
	else if (REPLYTYPE(r) == 5)
		goto unavailable;
	else if (REPLYTYPE(r) != 2)
		goto tempfail;

	/*
	**  If this is expected to be another sendmail, send some internal
	**  commands.
	*/

	if (bitnset(M_INTERNAL, m->m_flags))
	{
		/* tell it to be verbose */
		smtpmessage("VERB", m);
		r = reply(m);
		if (r < 0)
			goto tempfail;

		/* tell it we will be sending one transaction only */
		smtpmessage("ONEX", m);
		r = reply(m);
		if (r < 0)
			goto tempfail;
	}

#ifdef M_XDEVSTATUS
	/*
	** for DECnet mail11 - ask mailer if it will
	** be returning multi-status for DATA command
	*/
	if( bitnset(M_XDEVSTATUS, m->m_flags) )
	{
		smtpmessage("MULT", m);
		r = reply(m);
		if ((r < 0) || REPLYTYPE(r) == 4)
			goto tempfail;
		if (REPLYTYPE(r) == 2)
			setbitn(M_XDEV_YES, m->m_flags);
		else
			clrbitn(M_XDEV_YES, m->m_flags);
	}
#endif /* M_XDEVSTATUS */

#ifdef M_EARLYHEAD
	/*
	** for DECnet mail11 - if this mailer wants the
	** envelope header early, see if he will accept it.
	*/
	if( bitnset(M_EARLYHEAD, m->m_flags) )
	{
		/* Ask it if it wants early header now */
		smtpmessage("HEAD", m);
		r = reply(m);
		if ((r < 0 ) || REPLYTYPE(r) == 4)
			goto tempfail;

		if (REPLYTYPE(r) == 2 || REPLYTYPE(r) == 3)
		{
			/* Send the header and message body */
			(*e->e_puthdr)(SmtpOut, m, e);
			putline("\n", SmtpOut, m);
			(*e->e_putbody)(SmtpOut, m, e);

			/* terminate the message */
			fprintf(SmtpOut, ".%s", m->m_eol);

			/* Finish protocol for this */
			r = reply(m);
			if ((r < 0) || REPLYTYPE(r) == 4)
				goto tempfail;
			if (REPLYTYPE(r) == 5)
				goto unavailable;
		}
	}
#endif /* M_EARLYHEAD */

	/*
	**  Send the MAIL command.
	**	Designates the sender.
	*/

	expand("\001g", buf, &buf[sizeof buf - 1], CurEnv);
	if (CurEnv->e_from.q_mailer == LocalMailer ||
	    !bitnset(M_FROMPATH, m->m_flags))
	{
		smtpmessage("MAIL From:<%s>", m, buf);
	}
	else
	{
		smtpmessage("MAIL From:<@%s%c%s>", m, MyHostName,
			buf[0] == '@' ? ',' : ':', buf);
	}
	SmtpPhase = "MAIL wait";
	setproctitle("%s %s: %s", CurEnv->e_id, CurHostName, SmtpPhase);
	r = reply(m);
	if (r < 0 || REPLYTYPE(r) == 4)
		goto tempfail;
	else if (r == 250)
		return (EX_OK);
	else if (r == 552)
		goto unavailable;
# ifdef MAIL11V3
	else if (r == 559)
	{
		smtpquit(m);
		return (EX_NOHOST);
	}
# endif /* MAIL11V3 */
	/* protocol error -- close up */
	smtpquit(m);
	return (EX_PROTOCOL);

	/* signal a temporary failure */
  tempfail:
	smtpquit(m);
	return (EX_TEMPFAIL);

	/* signal service unavailable */
  unavailable:
	smtpquit(m);
	return (EX_UNAVAILABLE);
}


/*
**  SMTPRCPT -- designate recipient.
**
**	Parameters:
**		to -- address of recipient.
**		m -- the mailer we are sending to.
**
**	Returns:
**		exit status corresponding to recipient status.
**
**	Side Effects:
**		Sends the mail via SMTP.
*/

smtprcpt(to, m)
	ADDRESS *to;
	register MAILER *m;
{
	register int r;
	extern char *remotename();

	smtpmessage("RCPT To:<%s>", m, to->q_user);

	SmtpPhase = "RCPT wait";
	setproctitle("%s %s: %s", CurEnv->e_id, CurHostName, SmtpPhase);
	r = reply(m);
	if (r < 0 || REPLYTYPE(r) == 4)
		return (EX_TEMPFAIL);
	else if (REPLYTYPE(r) == 2)
		return (EX_OK);
	else if (r == 550 || r == 551 || r == 553)
		return (EX_NOUSER);
	else if (r == 552 || r == 554)
		return (EX_UNAVAILABLE);
	return (EX_PROTOCOL);
}
/*
**  SMTPDATA -- send the data and clean up the transaction.
**
**	Parameters:
**		m -- mailer being sent to.
**		e -- the envelope for this message.
**
**	Returns:
**		exit status corresponding to DATA command.
**
**	Side Effects:
**		none.
*/

smtpdata(m, e, count, rcodes)
	struct mailer *m;
	register ENVELOPE *e;
	int count;
	int *rcodes;
{
	register int r;

	/*
	**  Send the data.
	**	First send the command and check that it is ok.
	**	Then send the data.
	**	Follow it up with a dot to terminate.
	**	Finally get the results of the transaction.
	*/

	/* send the command and check ok to proceed */
	smtpmessage("DATA", m);
	SmtpPhase = "DATA wait";
	setproctitle("%s %s: %s", CurEnv->e_id, CurHostName, SmtpPhase);
	r = reply(m);
	if (r < 0 || REPLYTYPE(r) == 4)
		return (EX_TEMPFAIL);
	else if (r == 554)
		return (EX_UNAVAILABLE);
	else if (r != 354 && r != 250)
		return (EX_PROTOCOL);

	/* now output the actual message */
	(*e->e_puthdr)(SmtpOut, m, CurEnv);
	putline("\n", SmtpOut, m);
	(*e->e_putbody)(SmtpOut, m, CurEnv);

	/* terminate the message */
	fprintf(SmtpOut, ".%s", m->m_eol);
	if (Verbose && !HoldErrs)
		nmessage(Arpa_Info, ">>> .");

	/* check for the results of the transaction */
	SmtpPhase = "result wait";
	setproctitle("%s %s: %s", CurEnv->e_id, CurHostName, SmtpPhase);
#ifdef M_XDEVSTATUS
	/* for DECnet mail11 */
	if (bitnset(M_XDEV_YES, m->m_flags) && rcodes != NULL)
	{
		register int i;
		int final = EX_OK;

		for (i = 0; i < count; i++)
			/* in case of early termination */
			rcodes[i] = EX_TEMPFAIL;

		for (i = 0; i < count; i++)
		{
			r = reply(m);
			if ((r < 0) || REPLYTYPE(r) == 4)
				rcodes[i] = EX_TEMPFAIL;
			else if (REPLYTYPE(r) == 2)
				rcodes[i] = EX_OK;
			else if (r == 550 || r == 551 || r == 553)
				rcodes[i] = EX_NOUSER;
			else if (r == 552 || r == 554)
				final = (rcodes[i] = EX_UNAVAILABLE);
			else
				final = (rcodes[i] = EX_PROTOCOL);
		}
		return (final);
	}
#endif /* M_XDEVSTATUS */
	r = reply(m);
	if (r < 0 || REPLYTYPE(r) == 4)
		return (EX_TEMPFAIL);
	else if (r == 250)
		return (EX_OK);
	else if (r == 552 || r == 554)
		return (EX_UNAVAILABLE);
	return (EX_PROTOCOL);
}
/*
**  SMTPQUIT -- close the SMTP connection.
**
**	Parameters:
**		m -- a pointer to the mailer.
**
**	Returns:
**		none.
**
**	Side Effects:
**		sends the final protocol and closes the connection.
*/

smtpquit(m)
	register MAILER *m;
{
	int i;

	/* if the connection is already closed, don't bother */
	if (SmtpIn == NULL)
		return;

	/* send the quit message if not a forced quit */
	if (SmtpState == SMTP_OPEN || SmtpState == SMTP_SSD)
	{
		smtpmessage("QUIT", m);
		(void) reply(m);
		if (SmtpState == SMTP_CLOSED)
			return;
	}

	/* now actually close the connection */
	i = errno;
	(void) fclose(SmtpIn);
	(void) fclose(SmtpOut);
	errno = i;
	SmtpIn = SmtpOut = NULL;
	SmtpState = SMTP_CLOSED;

	/* and pick up the zombie */
	i = endmailer(SmtpPid, m->m_argv[0]);
	if (i != EX_OK)
		syserr("smtpquit %s: stat %d", m->m_argv[0], i);
}
/*
**  REPLY -- read arpanet reply
**
**	Parameters:
**		m -- the mailer we are reading the reply from.
**
**	Returns:
**		reply code it reads.
**
**	Side Effects:
**		flushes the mail file.
*/

reply(m)
	MAILER *m;
{
	if (SmtpOut != NULL)
		(void) fflush(SmtpOut);

	if (tTd(18, 1))
		printf("reply\n");

	if (bitnset(M_BSMTP, m->m_flags))
		return(SMTPGOODREPLY);

	/*
	**  Read the input line, being careful not to hang.
	*/

	for (;;)
	{
		register int r;
		register char *p;

		/* actually do the read */
		if (CurEnv->e_xfp != NULL)
			(void) fflush(CurEnv->e_xfp);	/* for debugging */

		/* if we are in the process of closing just give the code */
		if (SmtpState == SMTP_CLOSED)
			return (SMTPCLOSING);

		/* get the line from the other side */
		p = sfgets(SmtpReplyBuffer, sizeof SmtpReplyBuffer, SmtpIn);
		if (p == NULL)
		{
			extern char MsgBuf[];		/* err.c */
			extern char Arpa_TSyserr[];	/* conf.c */

			/* if the remote end closed early, fake an error */
			if (errno == 0)
# ifdef ECONNRESET
				errno = ECONNRESET;
# else /* ECONNRESET */
				errno = EPIPE;
# endif /* ECONNRESET */
			/* Report that connection ended prematurely */
			if (CurEnv->e_xfp != NULL)
			{
				extern char *errstring();
				extern char *statstring();

				fprintf(CurEnv->e_xfp,
					"421 %s (%s)... Deferred: %s\n",
					CurHostName, m->m_name,
					errstring(errno));
			}

			/* if debugging, pause so we can see state */
			if (tTd(18, 100))
				pause();
# ifdef LOG
			syslog(LOG_INFO, "%s", &MsgBuf[4]);
# endif /* LOG */
			SmtpState = SMTP_CLOSED;
			smtpquit(m);
			return (-1);
		}
		fixcrlf(SmtpReplyBuffer, TRUE);

		if (CurEnv->e_xfp != NULL && index("45", SmtpReplyBuffer[0]) != NULL)
		{
			/* serious error -- log the previous command */
			/* also record who we were talking before first error */
			if (SmtpNeedIntro)
				fprintf(CurEnv->e_xfp,
					"While talking to %s:\n", CurHostName);
			SmtpNeedIntro = FALSE;
			if (SmtpMsgBuffer[0] != '\0')
				fprintf(CurEnv->e_xfp, ">>> %s\n", SmtpMsgBuffer);
			SmtpMsgBuffer[0] = '\0';

			/* now log the message as from the other side */
			fprintf(CurEnv->e_xfp, "<<< %s\n", SmtpReplyBuffer);
		}

		/* display the input for verbose mode */
		if (Verbose && !HoldErrs)
			nmessage(Arpa_Info, "%s", SmtpReplyBuffer);

		/* if continuation is required, we can go on */
		if (SmtpReplyBuffer[3] == '-' || !isdigit(SmtpReplyBuffer[0]))
			continue;

		/* decode the reply code */
		r = atoi(SmtpReplyBuffer);

		/* extra semantics: 0xx codes are "informational" */
		if (r < 100)
			continue;

		/* reply code 421 is "Service Shutting Down" */
		if (r == SMTPCLOSING && SmtpState != SMTP_SSD)
		{
			/* send the quit protocol */
			SmtpState = SMTP_SSD;
			smtpquit(m);
		}

		/* save temporary failure messages for posterity */
		if (SmtpReplyBuffer[0] == '4' && SmtpError[0] == '\0')
			(void) strcpy(SmtpError, &SmtpReplyBuffer[4]);

		return (r);
	}
}
/*
**  SMTPMESSAGE -- send message to server
**
**	Parameters:
**		f -- format
**		m -- the mailer to control formatting.
**		a, b, c -- parameters
**
**	Returns:
**		none.
**
**	Side Effects:
**		writes message to SmtpOut.
*/

/*VARARGS1*/
smtpmessage(f, m, a, b, c)
	char *f;
	MAILER *m;
        caddr_t a,b,c;
{
	(void) sprintf(SmtpMsgBuffer, f, a, b, c);
	if (tTd(18, 1) || (Verbose && !HoldErrs))
		nmessage(Arpa_Info, ">>> %s", SmtpMsgBuffer);
	if (SmtpOut != NULL)
		fprintf(SmtpOut, "%s%s", SmtpMsgBuffer,
			m == 0 ? " \r\n" : m->m_eol);
}

# endif /* SMTP */
