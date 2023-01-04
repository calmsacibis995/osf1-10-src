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
static char rcsid[] = "@(#)$RCSfile: ex_cmds.c,v $ $Revision: 4.3.8.2 $ (DEC) $Date: 1993/06/10 19:42:30 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.1
 */
/*
 * COMPONENT_NAME: (CMDEDIT) ex_cmds.c
 *
 * FUNCTION: commands
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.11  com/cmd/edit/vi/ex_cmds.c, , bos320, 9134320 8/11/91 12:28:09
 * 
 */
/* Copyright (c) 1981 Regents of the University of California */

#include "ex.h"
#include "ex_argv.h"
#include "ex_temp.h"
#include "ex_tty.h"
#include "ex_vis.h"

short	pflag, nflag;
int	poffset;

#define nochng()	lchng = chng
/* convert a char string to an wide character string */
#define S2WCS(s)	(mbstowcs(bufc, s, WCSIZE(bufc)),bufc)
#define dotail(s)	tail(S2WCS(s))
#define dotail2of(s)	tail2of(S2WCS(s))
static	wchar_t bufc[30];	/* char string to wide char string conversion */
/*
 * Main loop for command mode command decoding.
 * A few commands are executed here, but main function
 * is to strip command addresses, do a little address oriented
 * processing and call command routines to do the real work.
 */
void
commands(short noprompt, short exitoneof)
{
	register line *addr;
	register int c;
	register int lchng;
	int given;
	int seensemi;
	int cnt;
	short hadpr;
	resetflav();
	nochng();
	for (;;) {
		/*
		 * If dot at last command
		 * ended up at zero, advance to one if there is a such.
		 */
		if (dot <= zero) {
			dot = zero;
			if (dol > zero)
				dot = one;
		}
		shudclob = 0;

		/*
		 * If autoprint or trailing print flags,
		 * print the line at the specified offset
		 * before the next command.
		 */
		if (pflag ||
		    lchng != chng && value(AUTOPRINT) && !inglobal && !inopen && endline) {
			pflag = 0;
			nochng();
			if (dol != zero) {
				addr1 = addr2 = dot + poffset;
				if (addr1 < one || addr1 > dol)
					error(MSGSTR(M_013, "Offset out-of-bounds|Offset after command too large"), DUMMY_INT);
				setdot1();
				goto print;
			}
		}
		nochng();

		/*
		 * Print prompt if appropriate.
		 * If not in global flush output first to prevent
		 * going into pfast mode unreasonably.
		 */
		if (inglobal == 0) {
			flush();
			if (!hush && value(PROMPT) && !globp && !noprompt && endline) {
				ex_putchar(':');
				hadpr = 1;
			}
			TSYNC();
		}

		/*
		 * Gobble up the address.
		 * Degenerate addresses yield ".".
		 */
		addr2 = 0;
		given = seensemi = 0;
		do {
			addr1 = addr2;
			addr = address((wchar_t *)0);
			c = getcd();
			if (addr == 0)
				if (c == ',')
					addr = dot;
				else if (addr1 != 0) {
					addr2 = dot;
					break;
				} else
					break;
			addr2 = addr;
			given++;
			if (c == ';') {
				c = ',';
				dot = addr;
				seensemi = 1;
			}
		} while (c == ',');
		if (c == '%') {
			/* %: same as 1,$ */
			addr1 = one;
			addr2 = dol;
			given = 2;
			c = ex_getchar();
		}
		if (addr1 == 0)
			addr1 = addr2;
		if (c == ':')
			c = ex_getchar();

		/*
		 * Set command name for special character commands.
		 */
		tailspec((wchar_t)c);

		/*
		 * If called via : escape from open or visual, limit
		 * the set of available commands here to save work below.
		 */
		if (inopen) {
			if (c=='\n' || c=='\r' || c==Ctrl('D') || c==EOF) {
				if (addr2)
					dot = addr2;
				if (c == EOF)
					return;
				continue;
			}
			if (any(c, "o")) {
notinvis:
				tailprim(S2WCS(Command), 1, (short)1);
			}
		}
		laste = 0;
		switch (c) {

		case 'a':

			switch(peekchar()) {
			case 'b':
/* abbreviate */
				dotail("abbreviate");
				setnoaddr();
				mapcmd(0, 1);
				anyabbrs = 1;
				continue;
			case 'r':
/* args */
				dotail("args");
				setnoaddr();
				eol();
				pargs();
				continue;
			}

/* append */
			if (inopen)
				goto notinvis;
			dotail("append");
			setdot();
			aiflag = exclam();
			donewline();
			vmacchng((short)0);
			deletenone();
			setin(addr2);
			inappend = 1;
			ignore(append(gettty, addr2));
			inappend = 0;
			nochng();
			continue;

		case 'c':
			switch (peekchar()) {

/* copy */
			case 'o':
				dotail("copy");
				vmacchng((short)0);
				move();
				continue;

#ifdef CHDIR
/* cd */
			case 'd':
				dotail("cd");
				goto changdir;
/* chdir */
			case 'h':
				ignchar();
				if (peekchar() == 'd') {
					register char *p;
					dotail2of("chdir");
changdir:
					if (savedfile[0] == '/' || !value(WARN))
						ignore(exclam());
					else
						ignore(quickly());
					if (skipend()) {
						p = getenv("HOME");
						if (p == NULL)
							error(MSGSTR(M_015, "Home directory unknown"), DUMMY_INT);
					} else
						getone(), p = file;
					eol();
					if (chdir(p) < 0)
						filioerr(p);
					if (savedfile[0] != '/')
						edited = 0;
					continue;
				}
				if (inopen)
				    tailprim(S2WCS("change"), 2, (short)1);
				dotail2of("change");
				break;
#endif
			default:
				if (inopen)
					goto notinvis;
				dotail("change");
				break;
			}
/* change */
			aiflag = exclam();
			setCNL();
			vmacchng((short)0);
			setin(addr1);
			delete((short)0);
			inappend = 1;
			ignore(append(gettty, addr1 - 1));
			inappend = 0;
			nochng();
			continue;

/* delete */
		case 'd':
			/*
			 * Caution: dp and dl have special meaning already.
			 */
			dotail("delete");
			c = cmdreg();
			setCNL();
			vmacchng((short)0);
			if (c)
				YANKreg(c);
			else {
				DEL[0] = 0;
				DEL_OVFLOW = FALSE;
			}
			delete((short)0);
			appendnone();
			continue;

/* edit */
/* ex */
		case 'e':
			dotail(peekchar() == 'x' ? "ex" : "edit");
editcmd:
			if (!exclam() && chng)
				c = 'E';
			filename(c);
			if (c == 'E') {
				ungetchar(lastchar());
				ignore(quickly());
			}
			setnoaddr();
doecmd:
			init();
			addr2 = zero;
			laste++;
			ex_sync();
			rop(c,0);
			nochng();
			continue;

/* file */
		case 'f':
			dotail("file");
			setnoaddr();
			filename(c);
			noonl();
			continue;

/* global */
		case 'g':
			dotail("global");
			global((short)(!exclam()));
			nochng();
			continue;

/* insert */
		case 'i':
			if (inopen)
				goto notinvis;
			dotail("insert");
			setdot();
			nonzero();
			aiflag = exclam();
			donewline();
			vmacchng((short)0);
			deletenone();
			setin(addr2);
			inappend = 1;
			ignore(append(gettty, addr2 - 1));
			inappend = 0;
			if (dot == zero && dol > zero)
				dot = one;
			nochng();
			continue;

/* join */
		case 'j':
			dotail("join");
			c = exclam();
			setcount();
			nonzero();
			donewline();
			vmacchng((short)0);
			if (given < 2 && addr2 != dol)
				addr2++;
			join(c);
			continue;

/* k */
		case 'k':
casek:
			pastwh();
			c = ex_getchar();
			if (endcmd(c))
			    serror(MSGSTR(M_016, "Mark what?|%s requires following letter"), Command);
			donewline();
			if (c > 'z' || c < 'a')
				error(MSGSTR(M_017, "Bad mark|Mark must specify a letter"), DUMMY_INT);
			setdot();
			nonzero();
			names[c - 'a'] = *addr2 &~ 01;
			anymarks = 1;
			continue;

/* list */
		case 'l':
			dotail("list");
			setCNL();
			ignorf(setlist(1));
			pflag = 0;
			goto print;

		case 'm':
			if (peekchar() == 'a') {
				ignchar();
				if (peekchar() == 'p') {
/* map */
					dotail2of("map");
					setnoaddr();
					mapcmd(0, 0);
					continue;
				}
/* mark */
				dotail2of("mark");
				goto casek;
			}
/* move */
			dotail("move");
			vmacchng((short)0);
			move();
			continue;

		case 'n':
			if (peekchar() == 'u') {
				dotail("number");
				goto numberit;
			}
/* next */
			dotail("next");
			setnoaddr();
            if( chng && value(AUTOWRITE) && !value(READONLY))
                if( !exclam())  /* only do this if AUTOWRITE */
                    wop((short)0);
                else
                    chng = 0; /* clear the chng flag as if a write has been done*/
                        ignore(quickly()); /* this consumes a ! if the AUTOWRITE is off */
			if (getargs())
				makargs();
			next();
			c = 'e';
			filename(c);
			goto doecmd;

/* open */
		case 'o':
			dotail("open");
			oop();
			pflag = 0;
			nochng();
			continue;

		case 'p':
		case 'P':
			switch (peekchar()) {

/* put */
			case 'u':
				dotail("put");
				setdot();
				c = cmdreg();
				eol();
				vmacchng((short)0);
				if (c)
					putreg((wchar_t)c);
				else
					put();
				continue;

			case 'r':
				ignchar();
				if (peekchar() == 'e') {
/* preserve */
					dotail2of("preserve");
					eol();
					if (preserve() == 0)
						error(MSGSTR(M_018, "Preserve failed!"), DUMMY_INT);
					else
						error(MSGSTR(M_019, "File preserved."), DUMMY_INT);
				}
				dotail2of("print");
				break;

			default:
				dotail("print");
				break;
			}
/* print */
			setCNL();
			pflag = 0;
print:
			nonzero();
			if (clear_screen && span() > lines) {
				flush1();
				vclear();
			}
			plines(addr1, addr2, (short)1);
			continue;

/* quit */
		case 'q':
			dotail("quit");
			setnoaddr();
			c = quickly();
			eol();
			if (!c)
quit:
				nomore();
			if (inopen) {
				vgoto(WECHO, 0);
				if (!ateopr())
					vnfl();
				else {
					tostop();
				}
				flush();
				setty(normf);
			}
			cleanup((short)1);
			catclose(ex_catd);
			exit(0);

		case 'r':
			if (peekchar() == 'e') {
				ignchar();
				switch (peekchar()) {

/* rewind */
				case 'w':
					dotail2of("rewind");
					setnoaddr();
					if (!exclam()) {
						ckaw();
						if (chng && dol > zero)
							error(MSGSTR(M_020, "No write@since last change (:rewind! overrides)"), DUMMY_INT);
					}
					eol();
					erewind();
					next();
					c = 'e';
					ungetchar(lastchar());
					filename(c);
					goto doecmd;

/* recover */
				case 'c':
					dotail2of("recover");
					setnoaddr();
					c = 'e';
					if (!exclam() && chng)
						c = 'E';
					filename(c);
					if (c == 'E') {
						ungetchar(lastchar());
						ignore(quickly());
					}
					init();
					addr2 = zero;
					laste++;
					ex_sync();
					recover();
					rop2();
					revocer();
					if (status == 0)
						rop3(c);
					if (dol != zero)
						change();
					nochng();
					continue;
				}
				dotail2of("read");
			} else
				dotail("read");
/* read */
			if (savedfile[0] == 0 && dol == zero)
				c = 'e';
			pastwh();
			vmacchng((short)0);
			if (peekchar() == '!') {
				setdot();
				ignchar();
				unix0((short)0);
				filter(0);
				continue;
			}
			filename(c);
			rop(c,1);
			nochng();
			if (inopen && endline && addr1 > zero && addr1 < dol)
				dot = addr1 + 1;
			continue;

		case 's':
			switch (peekchar()) {
			/*
			 * Caution: 2nd wide char cannot be c, g, or r
			 * because these have meaning to substitute.
			 */

/* set */
			case 'e':
				dotail("set");
				setnoaddr();
				set();
				continue;

/* shell */
			case 'h':
{
				ttymode savtty;
				dotail("shell");
				setNAEOL();
				vnfl();
				putpad(exit_ca_mode);
				flush();
				resetterm();
				savtty = unixex("-i", (char *) 0, 0, 0);
				unixwt((short)1, savtty);
				vcontin((short)0);
				continue;
}

/* source */
			case 'o':
				dotail("source");
				setnoaddr();
				getone();
				eol();
				source(file, (short)0);
				continue;
#ifdef SIGTSTP
/* stop, suspend */
			case 't':
				dotail("stop");
				goto suspend;
			case 'u':
				dotail("suspend");
suspend:
				c = exclam();
				eol();
				if (!c)
					ckaw();
				onsusp(0);
				continue;
#endif

			}
			/* fall into ... */

/* & */
/* ~ */
/* substitute */
		case '&':
		case '~':
			Command = "substitute";
			if (c == 's')
				dotail(Command);
			vmacchng((short)0);
			if (!substitute(c))
				pflag = 0;
			continue;

/* t */
		case 't':
			if (peekchar() == 'a') {
				dotail("tag");
				tagfind((short)exclam());
				if (!inopen)
					lchng = chng - 1;
				else
					nochng();
				continue;
			}
			dotail("t");
			vmacchng((short)0);
			move();
			continue;

		case 'u':
			if (peekchar() == 'n') {
				ignchar();
				switch(peekchar()) {
/* unmap */
				case 'm':
					dotail2of("unmap");
					setnoaddr();
					mapcmd(1, 0);
					continue;
/* unabbreviate */
				case 'a':
					dotail2of("unabbreviate");
					setnoaddr();
					mapcmd(1, 1);
					anyabbrs = 1;
					continue;
				}
/* undo */
				dotail2of("undo");
			} else
				dotail("undo");
			setnoaddr();
			markDOT();
			c = exclam();
			donewline();
			undo((wchar_t)c);
			continue;

		case 'v':
			switch (peekchar()) {

			case 'e':
/* version */
				dotail("version");
				setNAEOL();
#ifdef VERSION_STRING
				ex_printf(VERSION_STRING);
#else
				ex_printf("@(#) $Revision: 4.3.8.2 $ $Date: 1993/06/10 19:42:30 $"+6);
#endif
				noonl();
				continue;

/* visual */
			case 'i':
				dotail("visual");
				if (inopen) {
					c = 'e';
					goto editcmd;
				}
				vop();
				pflag = 0;
				nochng();
				continue;
			}
/* v */
			dotail("v");
			global((short)0);
			nochng();
			continue;

/* write */
		case 'w':
			c = peekchar();
			dotail(c == 'q' ? "wq" : "write");
wq:
			if (skipwh() && peekchar() == '!') {
				pofix();
				ignchar();
				setall();
				unix0((short)0);
				filter(1);
			} else {
				setall();
				wop((short)1);
				nochng();
			}
			if (c == 'q')
				goto quit;
			continue;

/* xit */
		case 'x':
			dotail("xit");
			if (!chng)
				goto quit;
			c = 'q';
			goto wq;

/* yank */
		case 'y':
			dotail("yank");
			c = cmdreg();
			setcount();
			eol();
			vmacchng((short)0);
			if (c)
				YANKreg(c);
			else
				yank();
			continue;

/* z */
		case 'z':
			zop(0);
			pflag = 0;
			continue;

/* * */
/* @ */
		case '*':
		case '@':
			c = ex_getchar();
			if (c=='\n' || c=='\r')
				ungetchar(c);
			if (any(c, "@*\n\r"))
				c = lastmac;
			if (c <= 'z' && iswupper(c))
				c = towlower(c);
			if (c > 'z' || !iswlower(c))
				error(MSGSTR(M_022, "Bad register"), DUMMY_INT);
			donewline();
			setdot();
			cmdmac((char)c);
			continue;

/* | */
		case '|':
			endline = 0;
			goto caseline;

/* \n */
		case '\n':
			endline = 1;
caseline:
			notempty();
			if (addr2 == 0) {
				if (cursor_up != NULL && c == '\n' && !inglobal)
					c = Ctrl('K');
				if (inglobal)
					addr1 = addr2 = dot;
				else {
					if (dot == dol)
						error(MSGSTR(M_023, "At EOF|At end-of-file"), DUMMY_INT);
					addr1 = addr2 = dot + 1;
				}
			}
			setdot();
			nonzero();
			if (seensemi)
				addr1 = addr2;
			getline(*addr1);
			if (c == Ctrl('K')) {
				flush1();
				destline--;
				if (hadpr)
					shudclob = 1;
			}
			plines(addr1, addr2, (short)1);
			continue;

/* " */
		case '"':
			comment();
			continue;

/* # */
		case '#':
numberit:
			setCNL();
			ignorf(setnumb(1));
			pflag = 0;
			goto print;

/* = */
		case '=':
			donewline();
			setall();
			if (inglobal == 2)
				pofix();
			ex_printf("%d", lineno(addr2));
			noonl();
			continue;

/* ! */
		case '!':
			if (addr2 != 0) {
				vmacchng((short)0);
				unix0((short)0);
				setdot();
				filter(2);
			} else {
				ttymode savtty;
				unix0((short)1);
				pofix();
				putpad(exit_ca_mode);
				flush();
				resetterm();
				savtty = unixex("-c", uxb, 0, 0);
				unixwt((short)1, savtty);
				vclrech((short)1);	/* vcontin(0); */
				nochng();
			}
			continue;

/* < */
/* > */
		case '<':
		case '>':
			for (cnt = 1; peekchar() == c; cnt++)
				ignchar();
			setCNL();
			vmacchng((short)0);
			shift(c, cnt);
			continue;

/* ^D */
/* EOF */
		case Ctrl('D'):
		case EOF:
			if (exitoneof) {
				if (addr2 != 0)
					dot = addr2;
				return;
			}
			if (!isatty(0)) {
				if (intty)
					/*
					 * Chtty sys call at UCB may cause a
					 * input which was a tty to suddenly be
					 * turned into /dev/null.
					 */
					onhup(0);
				return;
			}
			if (addr2 != 0) {
				setlastchar('\n');
				putnl();
			}
			if (dol == zero) {
				if (addr2 == 0)
					putnl();
				notempty();
			}
			ungetchar(EOF);
			zop(hadpr);
			continue;

		default:
			if (!iswalpha(c))
				break;
			ungetchar(c);
			tailprim(WCemptystr, 0, (short)0);
		}
		error(MSGSTR(M_024, "What?|Unknown command character '%c'"), c);
	}
}
