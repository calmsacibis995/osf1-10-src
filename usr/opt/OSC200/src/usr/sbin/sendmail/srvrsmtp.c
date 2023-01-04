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
static char	*sccsid = "@(#)$RCSfile: srvrsmtp.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/08 20:24:01 $";
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
# include <errno.h>
# include <signal.h>

# ifdef SMTP

/*
**  SMTP -- run the SMTP protocol.
**
**	Parameters:
**		none.
**
**	Returns:
**		never.
**
**	Side Effects:
**		Reads commands from the input channel and processes
**			them.
*/

struct cmd
{
	char	*cmdname;	/* command name */
	int	cmdcode;	/* internal code, see below */
};

/* values for cmdcode */
# define CMDERROR	0	/* bad command */
# define CMDMAIL	1	/* mail -- designate sender */
# define CMDRCPT	2	/* rcpt -- designate recipient */
# define CMDDATA	3	/* data -- send message text */
# define CMDRSET	4	/* rset -- reset state */
# define CMDVRFY	5	/* vrfy -- verify address */
# define CMDEXPN	6	/* expn -- expand alias */
# define CMDHELP	7	/* help -- give usage info */
# define CMDNOOP	8	/* noop -- do nothing */
# define CMDQUIT	9	/* quit -- close connection and die */
# define CMDHELO	10	/* helo -- be polite */
# define CMDONEX	11	/* onex -- sending one transaction only */
# define CMDVERB	12	/* verb -- go into verbose mode */
/* debugging-only commands, only enabled if SMTPDEBUG is defined */
# define CMDDBGQSHOW	13	/* showq -- show send queue */
# define CMDDBGDEBUG	14	/* debug -- set debug mode */

static struct cmd	CmdTab[] =
{
	"mail",		CMDMAIL,
	"rcpt",		CMDRCPT,
	"data",		CMDDATA,
	"rset",		CMDRSET,
	"vrfy",		CMDVRFY,
	"expn",		CMDEXPN,
	"help",		CMDHELP,
	"noop",		CMDNOOP,
	"quit",		CMDQUIT,
	"helo",		CMDHELO,
	"verb",		CMDVERB,
	"onex",		CMDONEX,
	/*
	 * remaining commands are here only
	 * to trap and log attempts to use them
	 */
	"showq",	CMDDBGQSHOW,
	"debug",	CMDDBGDEBUG,
	NULL,		CMDERROR,
};

bool	InChild = FALSE;		/* true if running in a subprocess */
bool	OneXact = FALSE;		/* one xaction only this run */

#define EX_QUIT		22		/* special code for QUIT command */

smtp(batched)
	bool batched;
{
	register char *p;
	register struct cmd *c;
	char *cmd;
	char *skipword();
	bool hasmail;			/* mail command received */
	bool aliaserror;		/* bad address int the alias file */
	auto ADDRESS *vrfyqueue;
	ADDRESS *a;
	char *sendinghost;
	char inp[MAXLINE];
	char cmdbuf[100];
	extern char Version[];
	char hostbuf[MAXNAME];
	char *cp;
	struct sockaddr sock;
	int socklen;
	extern char *aliaslookup();
	extern char *macvalue();
	extern ADDRESS *recipient();
	extern ENVELOPE BlankEnvelope;
	extern ENVELOPE *newenvelope();

	hasmail = aliaserror = FALSE;
	if (OutChannel != stdout)
	{
		/* arrange for debugging output to go to remote host */
		(void) close(1);
		(void) dup(fileno(OutChannel));
	}
	settime();
	if (RealHostName != NULL)
	{
		CurHostName = RealHostName;
		setproctitle("srvrsmtp %s", CurHostName);
	}
	else
	{
		/* this must be us!! */
		CurHostName = MyHostName;
	}
	expand("\001e", inp, &inp[(sizeof(inp)-1)], CurEnv);
	message("220", inp);
	SmtpPhase = "startup";
	sendinghost = NULL;
	for (;;)
	{
		/* arrange for backout */
		if (setjmp(TopFrame) > 0 && InChild)
			finis();
		QuickAbort = FALSE;
		HoldErrs = FALSE;

		/* setup for the read */
		CurEnv->e_to = NULL;
		Errors = 0;
		(void) fflush(stdout);

		/* read the input line */
		p = sfgets(inp, sizeof inp, InChannel);

		/* handle errors */
		if (p == NULL)
		{
			/* end of file, just die */
			message("421", MSGSTR(SV_LOST, "%s Lost input channel from %s"),
				MyHostName, CurHostName);
			finis();
		}

		/* clean up end of line */
		fixcrlf(inp, TRUE);

		/* echo command to transcript */
		if (CurEnv->e_xfp != NULL)
			fprintf(CurEnv->e_xfp, "<<< %s\n", inp);

		/* break off command */
		for (p = inp; isspace(*p); p++)
			continue;
		cmd = p;
		for (cmd = cmdbuf; *p != '\0' && !isspace(*p); )
			*cmd++ = *p++;
		*cmd = '\0';

		/* throw away leading whitespace */
		while (isspace(*p))
			p++;

		/* decode command */
		for (c = CmdTab; c->cmdname != NULL; c++)
		{
			if (!strcasecmp(c->cmdname, cmdbuf))
				break;
		}

		/* process command */
		switch (c->cmdcode)
		{
		  case CMDHELO:		/* hello -- introduce yourself */
			SmtpPhase = "HELO";
			setproctitle("%s: %s", CurHostName, inp);
			/* find canonical name if an IP connection*/
			strcpy(hostbuf, p);
			socklen=sizeof(sock);
			if (!getsockname(fileno(InChannel), &sock, &socklen)
			&& (sock.sa_family == AF_INET))
				maphostname(hostbuf, sizeof(hostbuf));
			if (!strcasecmp(p, MyHostName))
			{
				/*
				 * didn't know about alias,
				 * or connected to an echo server
				 */
				message("553", MSGSTR(SV_CONFIG, "Local configuration error, hostname not recognized as local"));
				break;
			}
			if (RealHostName != NULL && strcasecmp(hostbuf, RealHostName))
			{
				(void) sprintf(hostbuf, "%s (%s)", p, RealHostName);
				sendinghost = newstr(hostbuf);
			message("250", MSGSTR(SV_CALLU, "Hello %s, why did you call yourself %s "),
					RealHostName, p);
			} else {
				sendinghost = newstr(p);
				message("250", MSGSTR(SV_GREET, "%s Hello %s, pleased to meet you"),
				MyHostName, sendinghost);
			}
			break;

		  case CMDMAIL:		/* mail -- designate sender */
			SmtpPhase = "MAIL";

			/* force a sending host even if no HELO given */
			if (RealHostName != NULL && macvalue('s', CurEnv) == NULL)
				sendinghost = RealHostName;

			/* check for validity of this command */
			if (hasmail)
			{
				message("503", MSGSTR(SV_SNDR, "Sender already specified"));
				break;
			}
			if (InChild)
			{
				errno = 0;
				syserr(MSGSTR(SV_NEST, "Nested MAIL command"));
				exit(0);
			}

			/* fork a subprocess to process this command */
			if (CurEnv->e_xfp != NULL)
				(void) fflush(CurEnv->e_xfp);
			if (!batched && runinchild("SMTP-MAIL") > 0)
				break;
			define('s', sendinghost, CurEnv);
			define('r', "SMTP", CurEnv);
			initsys();
			setproctitle("%s %s: %s", CurEnv->e_id,
				CurHostName, inp);

			/* child -- go do the processing */
			p = skipword(p, "from");
			if (p == NULL)
				break;
			setsender(p);
			if (Errors == 0)
			{
				message("250", MSGSTR(SV_SOK, "Sender ok"));
				hasmail = TRUE;
			}
			else if (InChild)
				finis();
			break;

		  case CMDRCPT:		/* rcpt -- designate recipient */
			SmtpPhase = "RCPT";
			setproctitle("%s %s: %s", CurEnv->e_id,
				CurHostName, inp);
			if (setjmp(TopFrame) > 0)
			{
				if (!batched)
					CurEnv->e_flags &= ~EF_FATALERRS;
				break;
			}
			QuickAbort = TRUE;
			p = skipword(p, "to");
			if (p == NULL)
				break;
			a = parseaddr(p, (ADDRESS *) NULL, 1, '\0');
			if (a == NULL)
				break;
			a->q_flags |= QPRIMARY;

			a = recipient(a, &CurEnv->e_sendqueue);

			CurEnv->e_to = p;
			if (bitset(QISALIAS|QISFORWD, a->q_flags))
			{
				/*
				** If the rcpt to: line is a local alias,
				** tell the sender it is okay. It may
				** expand to an invalid address, but as
				** far as the SMTP conversation goes we
				** know about this recipient.  If there
				** are any bad addresses in the expanded
				** alias, notification will happen by mail.
				** This permits us to honor the owner- alias
				** and to deliver to any valid addresses on
				** the alias list.
				**
				** If there is an error, remember it since
				** it will be lost if there is another rcpt to:
				** line and we want to report the error later.
				*/
				message("250", MSGSTR(SV_ROK, "Recipient ok"));
				if (bitset(QBADALIAS, a->q_flags))
					aliaserror = TRUE;
				if (!batched)
					CurEnv->e_flags &= ~EF_FATALERRS;
			}
			else
			{
				if (!bitset(QBADADDR, a->q_flags))
				message("250", MSGSTR(SV_ROK, "Recipient ok"));
				else
				{
					/*
					** This will cause the sending
					** mailer to notify the user.
					** We clear the error bit so we
					** don't notify him by mail as
					** well.
					*/
				message("550", MSGSTR(SV_UKN, "Addressee unknown"));
					if (!batched)
						CurEnv->e_flags &= ~EF_FATALERRS;
				}

			}
			CurEnv->e_to = NULL;
			break;

		  case CMDDATA:		/* data -- text of mail */
			SmtpPhase = "DATA";
			if (!hasmail)
			{
				message("503", MSGSTR(SV_NEEDM, "Need MAIL command"));
				if (batched)
					Errors++;
				else
					break;
			}
			else if (CurEnv->e_nrcpts <= 0)
			{
				message("503", MSGSTR(SV_NEEDR, "Need RCPT (recipient)"));
				if (batched)
					Errors++;
				else
					break;
			}

			/* collect the text of the message */
			SmtpPhase = "collect";
			setproctitle("%s %s: %s", CurEnv->e_id,
				CurHostName, inp);
			collect(TRUE);
			if (Errors != 0)
				break;

			/*
			**  Arrange to send to everyone.
			**	If sending to multiple people, mail back
			**		errors rather than reporting directly.
			**	In any case, don't mail back errors for
			**		anything that has happened up to
			**		now (the other end will do this).
			**	Truncate our transcript -- the mail has gotten
			**		to us successfully, and if we have
			**		to mail this back, it will be easier
			**		on the reader.
			**	Then send to everyone.
			**	Finally give a reply code.  If an error has
			**		already been given, don't mail a
			**		message back.
			**	We goose error returns by clearing error bit.
			*/

			SmtpPhase = "delivery";
			if (CurEnv->e_nrcpts != 1 || batched || aliaserror)
			{
				HoldErrs = TRUE;
				ErrorMode = EM_MAIL;
			}
			if (!batched) {
				CurEnv->e_flags &= ~EF_FATALERRS;
				if (!aliaserror)
					CurEnv->e_xfp = freopen(queuename(CurEnv, 'x'),
								"w", CurEnv->e_xfp);
			}

			/* send to all recipients */
			sendall(CurEnv, SM_DEFAULT);
			CurEnv->e_to = NULL;

			/* save statistics */
			markstats(CurEnv, (ADDRESS *) NULL);

			/* issue success if appropriate and reset */
			if (Errors == 0 || HoldErrs)
				message("250", MSGSTR(SV_OK, "Ok"));
			/*
			** Was there an error expanding the alias?
			** if so, make sure we mail it back (unless
			** we are just queueing at the moment)
			*/
			if (aliaserror && SendMode != SM_QUEUE)
				CurEnv->e_flags |= EF_FATALERRS;

			/* if in a child, pop back to our parent */
			if (InChild)
				finis();

			/* clean up a bit */
			hasmail = 0;
			dropenvelope(CurEnv);
			CurEnv = newenvelope(CurEnv);
			CurEnv->e_flags = BlankEnvelope.e_flags;
			break;

		  case CMDRSET:		/* rset -- reset state */
			message("250", MSGSTR(SV_RESET, "Reset state"));
			if (InChild)
				finis();
			break;

		  case CMDVRFY:		/* vrfy -- verify address */
			/*
			** VRFY has been rewritten to make it more RFC-ish
			*/
			SmtpPhase = "VRFY";
			if (!batched && runinchild("SMTP-VRFY") > 0)
				break;
			setproctitle("%s: %s", CurHostName, inp);
			vrfyqueue = NULL;

			if (p == NULL)
				break;
			a = parseaddr(p, (ADDRESS *) NULL, 1, '\0');
			if (a == NULL)
				break;
			a = recipient(a, &vrfyqueue);

			if (bitset(QISALIAS, a->q_flags))
				message("252", MSGSTR(SV_ISALIAS, "<%s> is an alias"), a->q_paddr);
			else if (bitset(QBADADDR, a->q_flags))
				message("550", MSGSTR(SV_USERU, "User Unknown"));
			else
			{
				ADDRESS *ap;
				struct apl {
					ADDRESS	*addrp;
					struct apl *next;
				} *fal, **rap;

				fal = (struct apl *) 0;
				rap = &fal;

				for (ap = vrfyqueue; ap != NULL; ap = ap->q_next)
					if ((ap->q_alias == a && bitset(QISFORWD, a->q_flags)) ||
					    (ap == a && !bitset(QDONTSEND, a->q_flags)))
					{
						*rap = (struct apl *) xalloc(sizeof(struct apl));
						(*rap)->addrp = ap;
						(*rap)->next = (struct apl *) 0;
						rap = &(*rap)->next;
					}

				rap = &fal;
				if (*rap == NULL)
					message("554", MSGSTR(SV_ALIAS, "Self destructive alias loop"));
				else
					while (*rap != NULL)
					{
						ap = (*rap)->addrp;
						rap = &(*rap)->next;
						if (bitset(QISALIAS, ap->q_flags))
							message(*rap == NULL ?
								"252" : "252-",
								MSGSTR(SV_ISALIAS, "<%s> is an alias"),
								ap->q_paddr);
						else if (bitset(QBADADDR, ap->q_flags))
							message(*rap == NULL ?
								"550" : "550-",
								MSGSTR(SV_SUSERU, "<%s> User Unknown"),
								ap->q_paddr);
						else if (ap->q_fullname == NULL)
							message(*rap == NULL ?
								"250" : "250-",
								"<%s>",
								ap->q_paddr);
						else
							message(*rap == NULL ?
								"250" : "250-",
								"%s <%s>",
								ap->q_fullname,
								ap->q_paddr);
					}
			}

			if (InChild)
				finis();
			break;

		  case CMDEXPN:		/* expn -- expand an alias */
			/*
			** EXPN has been rewritten to make it more RFC-ish
			*/
			SmtpPhase = "EXPN";
			if (!batched && runinchild("SMTP-VRFY") > 0)
				break;
			setproctitle("%s: %s", CurHostName, inp);
			vrfyqueue = NULL;

			if (p == NULL)
				break;
			a = parseaddr(p, (ADDRESS *) NULL, 1, '\0');
			if (a == NULL)
				break;
			a = recipient(a, &vrfyqueue);

			if (!bitset(QISALIAS, a->q_flags))
				message("252", 
					MSGSTR(SV_NOTALIAS, "<%s> is not an alias"), 
					a->q_paddr);
			else
			{
				ADDRESS *ap, *yaap;
				struct apl {
					ADDRESS	*addrp;
					struct apl *next;
				} *fal, **rap;

				fal = (struct apl *) 0;
				rap = &fal;

				for (ap = vrfyqueue; ap != NULL; ap = ap->q_next)
				{
					if (!(ap->q_alias == a || ap == a))
						continue;
					if (strncmp(ap->q_user, ":include:", 9) != 0)
					{
						if (ap == a && bitset(QDONTSEND, ap->q_flags))
							continue;
						*rap = (struct apl *) xalloc(sizeof(struct apl));
						(*rap)->addrp = ap;
						(*rap)->next = (struct apl *) 0;
						rap = &(*rap)->next;
					}
					else
					{
						for (yaap = vrfyqueue; yaap != NULL; yaap = yaap->q_next)
						{
							if (yaap->q_alias != ap)
								continue;
							*rap = (struct apl *) xalloc(sizeof(struct apl));
							(*rap)->addrp = yaap;
							(*rap)->next = (struct apl *) 0;
							rap = &(*rap)->next;
						}
					}
				}

				rap = &fal;
				if (*rap == NULL)
					message("554", 
						MSGSTR(SV_ALIAS, "Self destructive alias loop"));
				else
					while (*rap != NULL)
					{
						ap = (*rap)->addrp;
						rap = &(*rap)->next;
						if (bitset(QISALIAS, ap->q_flags))
							message(*rap == NULL ?
								"252" : "252-",
								"<%s> is an alias",
								ap->q_paddr);
						else if (bitset(QBADADDR, ap->q_flags))
							message(*rap == NULL ?
								"550" : "550-",
								"<%s> User Unknown",
								ap->q_paddr);
						else if (ap->q_fullname == NULL)
							message(*rap == NULL ?
								"250" : "250-",
								"<%s>",
								ap->q_paddr);
						else
							message(*rap == NULL ?
								"250" : "250-",
								"%s <%s>",
								ap->q_fullname,
								ap->q_paddr);
					}
			}

			if (InChild)
				finis();
			break;

		  case CMDHELP:		/* help -- give user info */
			if (*p == '\0')
				p = "SMTP";
			help(p);
			break;

		  case CMDNOOP:		/* noop -- do nothing */
			message("200", MSGSTR(SV_OK2, "OK"));
			break;

		  case CMDQUIT:		/* quit -- leave mail */
			message("221", MSGSTR(SV_CLOSE, "%s closing connection"), 
				MyHostName);
			if (InChild)
				ExitStat = EX_QUIT;
			finis();

		  case CMDVERB:		/* set verbose mode */
			Verbose = TRUE;
			SendMode = SM_DELIVER;
			message("200", MSGSTR(SV_VERBOSE, "Verbose mode"));
			break;

		  case CMDONEX:		/* doing one transaction only */
			OneXact = TRUE;
			message("200", MSGSTR(SV_ONLY1, "Only one transaction"));
			break;

# ifdef SMTPDEBUG
		  case CMDDBGQSHOW:	/* show queues */
			printf("Send Queue=");
			printaddr(CurEnv->e_sendqueue, TRUE);
			break;

		  case CMDDBGDEBUG:	/* set debug mode */
			tTsetup(tTdvect, sizeof tTdvect, "0-99.1");
			tTflag(p);
			message("200", "Debug set");
			break;

# else /* not SMTPDEBUG */

		  case CMDDBGQSHOW:	/* show queues */
		  case CMDDBGDEBUG:	/* set debug mode */
# ifdef LOG
			if (RealHostName != NULL && LogLevel > 0)
				syslog(LOG_NOTICE,
				    MSGSTR(SV_TRACK, "\"%s\" command from %s (%s)\n"),
				    cmdbuf, RealHostName,
				    inet_ntoa(RealHostAddr.sin_addr));
# endif
			/* FALL THROUGH */
# endif /* SMTPDEBUG */

		  case CMDERROR:	/* unknown command */
			message("500", MSGSTR(SV_UKNCMD, "Command unrecognized"));
			syslog(LOG_NOTICE, "command \"%s\" from %s (%s) unrecognised",
			    cmdbuf, RealHostName,
			    inet_ntoa(RealHostAddr.sin_addr));
			break;

		  default:
			errno = 0;
			syserr(MSGSTR(SV_ERR, "smtp: unknown code %d"), c->cmdcode);
			break;
		}
	}
}
/*
**  SKIPWORD -- skip a fixed word.
**
**	Parameters:
**		p -- place to start looking.
**		w -- word to skip.
**
**	Returns:
**		p following w.
**		NULL on error.
**
**	Side Effects:
**		clobbers the p data area.
*/

static char *
skipword(p, w)
	register char *p;
	char *w;
{
	register char *q;

	/* find beginning of word */
	while (isspace(*p))
		p++;
	q = p;

	/* find end of word */
	while (*p != '\0' && *p != ':' && !isspace(*p))
		p++;
	while (isspace(*p))
		*p++ = '\0';
	if (*p != ':')
	{
	  syntax:
		message("501", MSGSTR(SV_SYNTX, "Syntax error"));
		Errors++;
		return (NULL);
	}
	*p++ = '\0';
	while (isspace(*p))
		p++;

	/* see if the input word matches desired word */
	if (strcasecmp(q, w))
		goto syntax;

	return (p);
}
/*
**  HELP -- implement the HELP command.
**
**	Parameters:
**		topic -- the topic we want help for.
**
**	Returns:
**		none.
**
**	Side Effects:
**		outputs the help file to message output.
*/

help(topic)
	char *topic;
{
	register FILE *hf;
	int len;
	char buf[MAXLINE];
	bool noinfo;

	if (HelpFile == NULL || (hf = fopen(HelpFile, "r")) == NULL)
	{
		/* no help */
		errno = 0;
		message("502", MSGSTR(SV_NOHLP, "HELP not implemented"));
		return;
	}

	len = strlen(topic);
	makelower(topic);
	noinfo = TRUE;

	while (fgets(buf, sizeof buf, hf) != NULL)
	{
		if (strncmp(buf, topic, len) == 0)
		{
			register char *p;

			p = index(buf, '\t');
			if (p == NULL)
				p = buf;
			else
				p++;
			fixcrlf(p, TRUE);
			message("214-", p);
			noinfo = FALSE;
		}
	}

	if (noinfo)
		message("504", MSGSTR(SV_NOTOP, "HELP topic unknown"));
	else
		message("214", MSGSTR(SV_ENDHLP, "End of HELP info"));
	(void) fclose(hf);
}
/*
**  RUNINCHILD -- return twice -- once in the child, then in the parent again
**
**	Parameters:
**		label -- a string used in error messages
**
**	Returns:
**		zero in the child
**		one in the parent
**
**	Side Effects:
**		none.
*/

runinchild(label)
	char *label;
{
	int childpid;

	if (!OneXact)
	{
		childpid = dofork();
		if (childpid < 0)
		{
			syserr(MSGSTR(SV_FORK, "%s: cannot fork"), label);
			return (1);
		}
		if (childpid > 0)
		{
			auto int st;

			/* parent -- wait for child to complete */
			st = waitfor(childpid);
			if (st == -1)
				syserr(MSGSTR(SV_LOST2, "%s: lost child"), label);

			/* if we exited on a QUIT command, complete the process */
			if (st == (EX_QUIT << 8))
				finis();

			return (1);
		}
		else
		{
			/* child */
			InChild = TRUE;
			QuickAbort = FALSE;
			clearenvelope(CurEnv, FALSE);
		}
	}

	/* open alias database */
	initaliases(FALSE);

	return (0);
}

# endif SMTP
