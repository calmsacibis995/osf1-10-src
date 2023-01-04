#
# *****************************************************************
# *                                                               *
# *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
# *                                                               *
# *   All Rights Reserved.  Unpublished rights  reserved  under   *
# *   the copyright laws of the United States.                    *
# *                                                               *
# *   The software contained on this media  is  proprietary  to   *
# *   and  embodies  the  confidential  technology  of  Digital   *
# *   Equipment Corporation.  Possession, use,  duplication  or   *
# *   dissemination of the software and media is authorized only  *
# *   pursuant to a valid written license from Digital Equipment  *
# *   Corporation.                                                *
# *                                                               *
# *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
# *   by the U.S. Government is subject to restrictions  as  set  *
# *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
# *   or  in  FAR 52.227-19, as applicable.                       *
# *                                                               *
# *****************************************************************
#
#
# (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# @(#)$RCSfile$ $Revision$ (DEC) $Date$
#
#
# HISTORY
#
BSDOFILES = \
	bsd_addbytes.o bsd_addch.o bsd_addstr.o bsd_box.o bsd_clear.o \
	bsd_clrtobot.o bsd_clrtoeol.o bsd_cr_put.o bsd_cr_tty.o \
	bsd_curses.o bsd_delch.o bsd_deleteln.o bsd_delwin.o bsd_endwin.o \
	bsd_erase.o bsd_fullname.o bsd_getch.o bsd_getstr.o bsd_idlok.o \
	bsd_idsubwins.o bsd_initscr.o bsd_insch.o bsd_insertln.o \
	bsd_longname.o bsd_move.o bsd_mvprintw.o bsd_mvscanw.o \
	bsd_mvwin.o bsd_newwin.o bsd_overlay.o bsd_overwrite.o \
	bsd_printw.o bsd_putchar.o bsd_refresh.o bsd_scanw.o bsd_scroll.o \
	bsd_toucholap.o bsd_standout.o bsd_touchwin.o bsd_tstp.o \
	bsd_termcap.o bsd_tgoto.o bsd_tputs.o

OFILES = ${BSDOFILES} \
	capnames.o   \
	__cflush.o   \
	__sscans.o   \
	_blanks.o    \
	_c_clean.o   \
	_clearhl.o   \
	_clearline.o \
	_comphash.o  \
	_delay.o     \
	_delchars.o  \
	_dellines.o  \
	_dumpwin.o   \
	_ec_quit.o   \
	_fixdelay.o  \
	_forcehl.o   \
	_hlmode.o    \
	_id_char.o   \
	_init_cost.o \
	_inschars.o  \
	_insmode.o   \
	_kpmode.o    \
	_line_free.o \
	_ll_move.o   \
	_outch.o     \
	_outchar.o   \
	_pos.o       \
	_redraw.o    \
	_reset.o     \
	_scrdown.o   \
	_scrollf.o   \
	_sethl.o     \
	_setmode.o   \
	_setwind.o   \
	_shove.o     \
	_sprintw.o   \
	_sputc.o     \
	_syncmodes.o \
	_tscroll.o   \
	_window.o    \
	addch.o      \
	addstr.o     \
	baudrate.o   \
	beep.o       \
	box.o        \
	cbreak.o     \
	chktypeahd.o \
	clear.o      \
	clearok.o    \
	clreolinln.o \
	clrtobot.o   \
	clrtoeol.o   \
	cntcostfn.o  \
	crmode.o     \
	curses.o     \
	def_prog.o   \
	def_shell.o  \
	delayoutpt.o \
	delch.o      \
	deleteln.o   \
	delwin.o     \
	doupdate.o   \
	draino.o     \
	echo.o       \
	endwin.o     \
	erase.o      \
	erasechar.o  \
	fixterm.o    \
	flash.o      \
	flushinp.o   \
	getcap.o     \
	getch.o      \
	getstr.o     \
	gettmode.o   \
	has_ic.o     \
	has_il.o     \
	idln.getst.o \
	idlok.o      \
	initkeypad.o \
	initscr.o    \
	insch.o      \
	insertln.o   \
	intrflush.o  \
	keypad.o     \
	killchar.o   \
	leaveok.o    \
	line_alloc.o \
	ll_refresh.o \
	longname.o   \
	m_addch.o    \
	m_addstr.o   \
	m_clear.o    \
	m_erase.o    \
	m_move.o     \
	m_refresh.o  \
	m_tstp.o     \
	makenew.o    \
	meta.o       \
	miniinit.o   \
	move.o       \
	mvcur.o      \
	mvprintw.o   \
	mvscanw.o    \
	mvwin.o      \
	mvwprintw.o  \
	mvwscanw.o   \
	naps.o       \
	newpad.o     \
	newterm.o    \
	newwin.o     \
	nl.o         \
	nocbreak.o   \
	nocrmode.o   \
	nodelay.o    \
	noecho.o     \
	nonl.o       \
	noraw.o      \
	nttychktrm.o \
	overlay.o    \
	overwrite.o  \
	pnoutrfrsh.o \
	prefresh.o   \
	printw.o     \
	putp.o       \
	raw.o        \
	reset_prog.o \
	resetshell.o \
	resetterm.o  \
	resetty.o    \
	restarttrm.o \
	saveterm.o   \
	savetty.o    \
	scanw.o      \
	scroll.o     \
	scrollok.o   \
	select.o     \
	set_term.o   \
	setbuffred.o \
	setterm.o    \
	setupterm.o  \
	showstring.o \
	subwin.o     \
	termcap.o    \
	tgoto.o      \
	touchwin.o   \
	toucholap.o  \
	tparm.o      \
	tputs.o      \
	traceonoff.o \
	tstp.o       \
	two.twostr.o \
	typeahead.o  \
	unctrl.o     \
	vidattr.o    \
	vidputs.o    \
	vsscanf.o    \
	vwprintw.o   \
	vwscanw.o    \
	wattroff.o   \
	wattron.o    \
	wattrset.o   \
	wnoutrfrsh.o \
	wprintw.o    \
	wrefresh.o   \
	writechars.o \
	wscanw.o     \
	wstandend.o  \
	wstandout.o  \
	addwch.o     \
	addwstr.o    \
	getwch.o     \
	getwstr.o    \
	inswch.o     \
	winwch.o     \
	addwchstr.o  \
	inswstr.o    \
	winwstr.o    \
	ungetwch.o   \
	cursor_on.o  \
	cursor_off.o 

