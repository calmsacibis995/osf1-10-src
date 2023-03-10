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
static char rcsid[] = "@(#)$RCSfile: ex_vadj.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/12/21 16:22:06 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.1
 */
/*
 * COMPONENT_NAME: (CMDEDIT) ex_vadj.c
 *
 * FUNCTION: sethard, vadjAL, vadjDL, vcloseup, vdellin, vdirty, vglitchup,
 * vinslin, vmoveitup, vopen, vopenup, vredraw, vreopen, vrepaint, vreplace,
 * vrollup, vscrap, vscroll, vsync, vsync1, vsyncCL, vup1
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.7  com/cmd/edit/vi/ex_vadj.c, cmdedit, bos320, 9123320e 6/5/91 23:32:33
 * 
 */
/* Copyright (c) 1981 Regents of the University of California */

#include "ex.h"
#include "ex_tty.h"
#include "ex_vis.h"

static void vopenup(int, short, int);
static void vadjAL(int, int);
static void vscroll(int);
static void vdellin(int, int, int);
static void vadjDL(int, int);
static void vcloseup(int, int);

/*
 * Routines to deal with management of logical versus physical
 * display, opening and redisplaying lines on the screen, and
 * use of intelligent terminal operations.  Routines to deal with
 * screen cleanup after a change.
 */

/*
 * Display a new line at physical line p, returning
 * the depth of the newly displayed line.  We may decide
 * to expand the window on an intelligent terminal if it is
 * less than a full screen by deleting a line above the top of the
 * window before doing an insert line to keep all the good text
 * on the screen in which case the line may actually end up
 * somewhere other than line p.
 */
void vopen(line *tp, int p)
{
	register int cnt;
	register struct vlinfo *vp, *vpc;

#ifdef ADEBUG
	if (trace != NULL)
		tfixnl(), fprintf(trace, "vopen(%d, %d)\n", lineno(tp), p);
#endif
	if (state != VISUAL) {
		if (vcnt)
			if (hold & HOLDROL)
				vup1();
			else
				vclean();

		/*
		 * Forget all that we once knew.
		 */
		vcnt = vcline = 0;
		p = WBOT; LASTLINE = WBOT + 1;
		state = bastate;
		WTOP = basWTOP;
		WLINES = basWLINES;
	}
	vpc = &vlinfo[vcline];
	for (vp = &vlinfo[vcnt]; vp >= vpc; vp--)
		vlcopy(vp[1], vp[0]);
	vcnt++;
	if (Pline == numbline)
		/*
		 * Dirtying all the lines is rather inefficient
		 * internally, but number mode is used rarely
		 * and so its not worth optimizing.
		 */
		vdirty(vcline+1, WECHO);
	getline(*tp);

	/*
	 * If we are opening at the top of the window, can try a window
	 * expansion at the top.
	 */
	if (state == VISUAL && vcline == 0 && vcnt > 1 && p > ZERO) {
		cnt = p + vdepth() - LINE(1);
		if (cnt > 0) {
			p -= cnt;
			if (p < ZERO)
				p = ZERO;
			WTOP = p;
			WLINES = WBOT - WTOP + 1;
		}
	}
	vpc->vliny = p, vpc->vdepth = 0, vpc->vflags = 0;
	cnt = vreopen(p, lineno(tp), vcline);
	if (vcline + 1 == vcnt)
		LINE(vcnt) = LINE(vcline) + cnt;
}

/*
 * Redisplay logical line l at physical line p with line number lineno.
 */
int vreopen(int p, int lineno, int l)
{
	register int d;
	register struct vlinfo *vp = &vlinfo[l];

	d = vp->vdepth;
	if (d == 0 || (vp->vflags & VDIRT))
		vp->vdepth = d = vdepth();
	vp->vliny = p, vp->vflags &= ~VDIRT;

	/*
	 * Try to win by making the screen larger rather than inserting
	 * a line and driving text off the bottom.
	 */
	p = vglitchup(l, 0);

	/*
	 * BUG:	Should consider using clr_eol here to clear to end of line.
	 *	As it stands we always strike over the current text.
	 *	Since often the current text is the same as what
	 *	we are overstriking with, it tends not to show.
	 *	On the other hand if it is different and we end up
	 *	spacing out a lot of text, we could have won with
	 *	a clr_eol.  This is probably worthwhile at low speed
	 *	only however, since clearly computation will be
	 *	necessary to determine which way to go.
	 */
	vigoto(p, 0);
	pline(lineno);

	/*
	 * When we are typing part of a line for hardcopy open, don't
	 * want to type the '$' marking an end of line if in list mode.
	 */
	if (hold & HOLDDOL)
		return (d);
	if (Putchar == (void(*)(int))listchar)
		ex_putchar('$');

	/*
	 * Optimization of cursor motion may prevent screen rollup if the
	 * line has blanks/tabs at the end unless we force the cursor to appear
	 * on the last line segment.
	 */
	if (vp->vliny + d - 1 > WBOT)
		vcsync();

	/*
	 * Switch into hardcopy open mode if we are in one line (adm3)
	 * open mode and this line is now too long.  If in hardcopy
	 * open mode, then call sethard to move onto the next line
	 * with appropriate positioning.
	 */
	if (state == ONEOPEN) {
		WCOLS = OCOLUMNS;
		if (vdepth() > 1) {
			WCOLS = TUBECOLS;
			sethard();
		} else
			WCOLS = TUBECOLS;
	} else if (state == HARDOPEN)
		sethard();

	/*
	 * Unless we filled (completely) the last line we typed on,
	 * we have to clear to the end of the line
	 * in case stuff is left from before.
	 */
	if (vp->vliny + d > destline) {
		if (insert_null_glitch && destcol == WCOLS)
			vigoto(vp->vliny + d - 1, 0);
		vclreol();
	}
	return (d);
}

/*
 * Real work for winning growing of window at top
 * when inserting in the middle of a partially full
 * screen on an intelligent terminal.  We have as argument
 * the logical line number to be inserted after, and the offset
 * from that line where the insert will go.
 * We look at the picture of depths and positions, and if we can
 * delete some (blank) lines from the top of the screen so that
 * later inserts will not push stuff off the bottom.
 */
int vglitchup(int l, int o)
{
	register struct vlinfo *vp = &vlinfo[l];
	register int need;
	register int p = vp->vliny;
	short oldhold, oldheldech;
	short glitched = 0;

 	if (l < vcnt - 1) {
		need = p + vp->vdepth - (vp+1)->vliny;
		if (need > 0) {
			if (state == VISUAL && WTOP - ZERO >= need && insert_line && delete_line) {
				glitched++;
				WTOP -= need;
				WLINES = WBOT - WTOP + 1;
				p -= need;
				if (p + o == WTOP) {
					vp->vliny = WTOP;
					return (WTOP + o);
				}
				vdellin(WTOP, need, -1);
				oldheldech = heldech;
				oldhold = hold;
				hold |= HOLDECH;
			}
			vinslin((vp+1)->vliny, need, l);
			if (glitched) {
				hold = oldhold;
				heldech = oldheldech;
			}
		}
	} else
		vp[1].vliny = vp[0].vliny + vp->vdepth;
	return (p + o);
}

/*
 * Insert cnt blank lines before line p,
 * logically and (if supported) physically.
 */
void vinslin(register int p, register int cnt, int l)
{
	register int i;
	short could = 1;

#ifdef ADEBUG
	if (trace)
		tfixnl(), fprintf(trace, "vinslin(%d, %d, %d)\n", p, cnt, l);
#endif
	if (p + cnt > WBOT && clr_eos) {
		/*
		 * Really quick -- clear to end of screen.
		 */
		cnt = WECHO + 1 - p;
		vgoto(p, 0), vputp((char *)clr_eos, cnt);
		vclrech((short)1);
		vadjAL(p, cnt);
	} else if (scroll_reverse && p == WTOP && costSR < costAL) {
		/*
		 * Use reverse scroll mode of the terminal, at
		 * the top of the window.  Reverse linefeed works
		 * too, since we only use it from line WTOP.
		 */
		for (i = cnt; i > 0; i--) {
			vgoto(p, 0), vputp((char *)scroll_reverse, 0);
			if (i > 1 && (hold & HOLDAT) == 0)
				ex_putchar('@');
			/*
			 * If we are at the top of the screen, and the
			 * terminal retains display above, then we
			 * should try to clear to end of line.
			 * Have to use clr_eol since we don't remember what is
			 * actually on the line.
			 */
			if (clr_eol && (memory_above || p != 0))
				vputp((char *)clr_eol, 1);
		}
		vadjAL(p, cnt);
	} else if (insert_line) {
		/*
		 * Use insert line.
		 */
		vgoto(p, 0);
		if (parm_insert_line && (cnt>1 || *insert_line==0)) {
			/* insert cnt lines.  Should do @'s too. */
			vputp(tparm(parm_insert_line, cnt, p), WECHO+1-p);
		}
		else if (change_scroll_region && *insert_line==0) {
			/* vt100 change scrolling region to fake insert_line */
			vputp(save_cursor, 1);
			vputp(tparm(change_scroll_region, p, lines-1), 1);
			vputp(restore_cursor, 1);	/* change_scroll_region homes stupid cursor */
			for (i=cnt; i>0; i--)
				vputp(scroll_reverse, 1);	/* should do @'s */
			vputp(tparm(change_scroll_region, 0, lines-1), 1);
			vputp(restore_cursor, 1);	/* Once again put it back */
		}
		else {
			vputp(insert_line, WECHO + 1 - p);
			for (i = cnt - 1; i > 0; i--) {
				vgoto(outline+1, 0);
				vputp(insert_line, WECHO + 1 - outline);
				if ((hold & HOLDAT) == 0)
					ex_putchar('@');
			}
		}
		vadjAL(p, cnt);
	} else
		could = 0;
	vopenup(cnt, could, l);
}

/*
 * Logically open up after line l, cnt of them.
 * We need to know if it was done ``physically'' since in this
 * case we accept what the hardware gives us.  If we have to do
 * it ourselves (brute force) we will squish out @ lines in the process
 * if this will save us work.
 */
static void vopenup(int cnt, short could, int l)
{
	register struct vlinfo *vc = &vlinfo[l + 1];
	register struct vlinfo *ve = &vlinfo[vcnt];

#ifdef ADEBUG
	if (trace)
		tfixnl(), fprintf(trace, "vopenup(%d, %d, %d)\n", cnt, could, l);
#endif
	if (could)
		/*
		 * This will push @ lines down the screen,
		 * just as the hardware did.  Since the default
		 * for intelligent terminals is to never have @
		 * lines on the screen, this should never happen,
		 * and the code makes no special effort to be nice in this
		 * case, e.g. squishing out the @ lines by delete lines
		 * before doing append lines.
		 */
		for (; vc <= ve; vc++)
			vc->vliny += cnt;
	else {
		/*
		 * Will have to clean up brute force eventually,
		 * so push the line data around as little as possible.
		 */
		vc->vliny += cnt, vc->vflags |= VDIRT;
		while (vc < ve) {
			register int i = vc->vliny + vc->vdepth;

			vc++;
			if (i <= vc->vliny)
				break;
			vc->vliny = i, vc->vflags |= VDIRT;
		}
	}
	vscrap();
}

/*
 * Adjust data structure internally to account for insertion of
 * blank lines on the screen.
 */
static void vadjAL(int p, int cnt)
{
        wchar_t *tlines[GBSIZE * 2];            /* WW-01 */
	register int from, to;

#ifdef ADEBUG
	if (trace)
		tfixnl(), fprintf(trace, "vadjal(%d, %d)\n", p, cnt);
#endif
	copy(tlines, vtube, sizeof vtube);	/*SASSIGN*/
	for (from = p, to = p + cnt; to <= WECHO; from++, to++)
		vtube[to] = tlines[from];
	for (to = p; from <= WECHO; from++, to++) {
		vtube[to] = tlines[from];
		vclrcp(vtube[to], WCOLS);
	}
	/*
	 * Have to clear the echo area since its contents aren't
	 * necessarily consistent with the rest of the display.
	 */
	vclrech((short)0);
}

/*
 * Roll the screen up logically and physically
 * so that line dl is the bottom line on the screen.
 */
void vrollup(int dl)
{
	register int cnt;
	register int dc = destcol;

#ifdef ADEBUG
	if (trace)
		tfixnl(), fprintf(trace, "vrollup(%d)\n", dl);
#endif
	cnt = dl - (splitw ? WECHO : WBOT);
	if (splitw && (state == VISUAL || state == CRTOPEN))
		holdupd = 1;
	vmoveitup(((int)cnt), ((short)1));
	vscroll(cnt);
	destline = dl - cnt, destcol = dc;
}

void vup1(void)
{

	vrollup(WBOT + 1);
}

/*
 * Scroll the screen up cnt lines physically.
 * If doclr is true, do a clear eol if the terminal
 * has standout (to prevent it from scrolling up)
 */
void vmoveitup(int cnt, short doclr)
{

	if (cnt == 0)
		return;
#ifdef ADEBUG
	if (trace)
		tfixnl(), fprintf(trace, "vmoveitup(%d)\n", cnt);
#endif
	if (doclr)
		vclrech((short)0);
	if (scroll_forward) {
		destline = WECHO;
		destcol = (NONL ? 0 : outcol % WCOLS);
		fgoto();
		while (cnt > 0)
			vputp(scroll_forward, 0), cnt--;
	}
	else {
		destline = WECHO + cnt;
		destcol = (NONL ? 0 : outcol % WCOLS);
		fgoto();
		if (state == ONEOPEN || state == HARDOPEN) {
			outline = destline = 0;
			vclrcp(vtube[0], WCOLS);
		}
	}
	/* Get rid of line we just rolled up */
	if (doclr && memory_below && clr_eol)
		vclrech((short)0);
}

/*
 * Scroll the screen up cnt lines logically.
 */
static void vscroll(register int cnt)
{
	register int from, to;
        wchar_t *tlines[GBSIZE * 2];    /* WW-01 */

#ifdef ADEBUG
	if (trace)
		fprintf(trace, "vscroll(%d)\n", cnt);
#endif
	if (cnt < 0 || cnt > TUBELINES)
		error(MSGSTR(M_233, "Internal error: vscroll"), DUMMY_INT);
	if (cnt == 0)
		return;

	copy(tlines, vtube, sizeof vtube);
	for (to = ZERO, from = ZERO + cnt; to <= WECHO - cnt; to++, from++)
		vtube[to] = tlines[from];
	for (from = ZERO; to <= WECHO; to++, from++) {
		vtube[to] = tlines[from];
		vclrcp(vtube[to], WCOLS);
	}
	for (from = 0; from <= vcnt; from++)
		LINE(from) -= cnt;
}

/*
 * Discard logical lines due to physical wandering off the screen.
 */
void vscrap(void)
{
	register int i, j;

#ifdef ADEBUG
	if (trace)
		tfixnl(), fprintf(trace, "vscrap\n"), tvliny();
#endif
	if (splitw)
		return;
	if (vcnt && WBOT != WECHO && LINE(0) < WTOP && LINE(0) >= ZERO) {
		WTOP = LINE(0);
		WLINES = WBOT - WTOP + 1;
	}
	for (j = 0; j < vcnt; j++)
		if (LINE(j) >= WTOP) {
			if (j == 0)
				break;
			/*
			 * Discard the first j physical lines off the top.
			 */
			vcnt -= j, vcline -= j;
			for (i = 0; i <= vcnt; i++)
				vlcopy(vlinfo[i], vlinfo[i + j]);
			break;
		}
	/*
	 * Discard lines off the bottom.
	 */
	if (vcnt) {
		for (j = 0; j <= vcnt; j++)
			if (LINE(j) > WBOT || LINE(j) + DEPTH(j) - 1 > WBOT) {
				vcnt = j;
				break;
			}
		LASTLINE = LINE(vcnt-1) + DEPTH(vcnt-1);
	}
#ifdef ADEBUG
	if (trace)
		tvliny();
#endif
	/*
	 * May have no lines!
	 */
}

/*
 * Repaint the screen, with cursor at curs, aftern an arbitrary change.
 * Handle notification on large changes.
 */
void vrepaint(wchar_t *curs)
{

	wdot = NOLINE;
	/*
	 * In open want to notify first.
	 */
	noteit((short)0);
	vscrap();

	/*
	 * Deal with a totally useless display.
	 */
	if (vcnt == 0 || vcline < 0 || vcline > vcnt || holdupd && state != VISUAL) {
		register line *odol = dol;

		vcnt = 0;
		if (holdupd)
			if (state == VISUAL)
				ignore(peekkey());
			else
				vup1();
		holdupd = 0;
		if (odol == zero)
			fixzero();
		vcontext(((line *)dot), ((wchar_t)'.'));
		if (noteit((short)1) == 0 && odol == zero) {
			CATCH
				error(MSGSTR(M_234, "No lines in buffer"), DUMMY_INT);
			ENDCATCH
			linebuf[0] = 0;
			splitw = 0;
		}
		vnline(curs);
		return;
	}

	/*
	 * Have some useful displayed text; refresh it.
	 */
	getDOT();

	/*
	 * This is for boundary conditions in open mode.
	 */
	if (FLAGS(0) & VDIRT)
		vsync(WTOP);
	
	/*
	 * If the current line is after the last displayed line
	 * or the bottom of the screen, then special effort is needed
	 * to get it on the screen.  We first try a redraw at the
	 * last line on the screen, hoping it will fill in where @
	 * lines are now.  If this doesn't work, then roll it onto
	 * the screen.
	 */
	if (vcline >= vcnt || LINE(vcline) > WBOT) {
		short oldhold = hold;
		hold |= HOLDAT, vredraw(LASTLINE), hold = oldhold;
		if (vcline >= vcnt) {
			register int i = vcline - vcnt + 1;

			dot -= i;
			vcline -= i;
			vroll(i);
		} else
			vsyncCL();
	} else
		vsync(vcline > 0 ? LINE(vcline - 1) : WTOP);

	/*
	 * Notification on large change for visual
	 * has to be done last or we may lose
	 * the echo area with redisplay.
	 */
	noteit((short)1);

	/*
	 * Finally.  Move the cursor onto the current line.
	 */
	vnline(curs);
}

/*
 * Fully cleanup the screen, leaving no @ lines except at end when
 * line after last won't completely fit.  The routine vsync is
 * more conservative and much less work on dumb terminals.
 */
void vredraw(register int p)
{
	register int l;
	register line *tp;
	wchar_t temp[LBSIZE];
	short anydl = 0;
	short oldhold = hold;

#ifdef ADEBUG
	if (trace)
		tfixnl(), fprintf(trace, "vredraw(%d)\n", p), tvliny();
#endif
	if (holdupd) {
		holdupd = 3;
		return;
	}
	if (state == HARDOPEN || splitw)
		return;
	if (p < 0 /* || p > WECHO */)
		error(MSGSTR(M_235, "Internal error: vredraw"), DUMMY_INT);

	/*
	 * Trim the ragged edges (lines which are off the screen but
	 * not yet logically discarded), save the current line, and
	 * search for first logical line affected by the redraw.
	 */
	vscrap();
	CP(temp, linebuf);
	l = 0;
	tp = dot - vcline;
	if (vcnt == 0)
		LINE(0) = WTOP;
	while (l < vcnt && LINE(l) < p)
		l++, tp++;

	/*
	 * We hold off echo area clearing during the redraw in deference
	 * to a final clear of the echo area at the end if appropriate.
	 */
	heldech = 0;
	hold |= HOLDECH;
	for (; l < vcnt && Peekkey != ATTN; l++) {
		if (l == vcline)
			strcLIN(temp);
		else
			getline(*tp);

		/*
		 * Delete junk between displayed lines.
		 */
		if (LINE(l) != LINE(l + 1) && LINE(l) != p) {
			if (anydl == 0 && memory_below && clr_eos) {
				hold = oldhold;
				vclrech((short)0);
				anydl = 1;
				hold |= HOLDECH;
				heldech = 0;
			}
			vdellin(p, LINE(l) - p, l);
		}

		/*
		 * If line image is not know to be up to date, then
		 * redisplay it;  else just skip onward.
		 */
		LINE(l) = p;
		if (FLAGS(l) & VDIRT) {
			DEPTH(l) = vdepth();
			if (l != vcline && p + DEPTH(l) - 1 > WBOT) {
				vscrap();
				break;
			}
			FLAGS(l) &= ~VDIRT;
			vreopen(p, lineno(tp), l);
			p = LINE(l) + DEPTH(l);
		} else
			p += DEPTH(l);
		tp++;
	}

	/*
	 * That takes care of lines which were already partially displayed.
	 * Now try to fill the rest of the screen with text.
	 */
	if (state == VISUAL && p <= WBOT) {
		int ovcline = vcline;

		vcline = l;
		for (; tp <= dol && Peekkey != ATTN; tp++) {
			getline(*tp);
			if (p + vdepth() - 1 > WBOT)
				break;
			vopen(tp, p);
			p += DEPTH(vcline);
			vcline++;
		}
		vcline = ovcline;
	}

	/*
	 * Thats all the text we can get on.
	 * Now rest of lines (if any) get either a ~ if they
	 * are past end of file, or an @ if the next line won't fit.
	 */
	for (; p <= WBOT && Peekkey != ATTN; p++)			
		vclrlin(p, tp);
	strcLIN(temp);
	hold = oldhold;
	if (heldech)
		vclrech((short)0);
#ifdef ADEBUG
	if (trace)
		tvliny();
#endif
}

/*
 * Do the real work in deleting cnt lines starting at line p from
 * the display.  First affected line is line l.
 */
static void vdellin(int p, int cnt, int l)
{
	register int i;

	if (cnt == 0)
		return;
	if (delete_line == (char *) 0 || cnt < 0) {
		/*
		 * Can't do it; just remember that line l is munged.
		 */
		FLAGS(l) |= VDIRT;
		return;
	}
#ifdef ADEBUG
	if (trace)
		tfixnl(), fprintf(trace, "vdellin(%d, %d, %d)\n", p, cnt, l);
#endif
	/*
	 * Send the deletes to the screen and then adjust logical
	 * and physical internal data structures.
	 */
	vgoto(p, 0);
	if (parm_delete_line && (cnt>1 || *delete_line==0)) {
		vputp(tparm(parm_delete_line, cnt, p), WECHO-p);
	}
	else if (change_scroll_region && *delete_line==0) {
		/* vt100: fake delete_line by changing scrolling region */
		vputp(save_cursor, 1);	/* Save since change_scroll_region homes stupid cursor */
		vputp(tparm(change_scroll_region, p, lines-1), 1);
		vputp(tparm(cursor_address, lines-1, 0), 1);/* Go to lower left corner */
		for (i=0; i<cnt; i++)		/* .. and scroll cnt times */
			putch('\n');		/* should check NL too */
		vputp(tparm(change_scroll_region, 0, lines-1), 1);/* restore scrolling region */
		vputp(restore_cursor, 1);			/* put cursor back */
	}
	else {
		for (i = 0; i < cnt; i++)
			vputp(delete_line, WECHO - p);
	}
	vadjDL(p, cnt);
	vcloseup(l, cnt);
}
/*
 * Adjust internal physical screen image to account for deleted lines.
 */
static void vadjDL(int p, int cnt)
{
        wchar_t *tlines[GBSIZE * 2];    /* WW-01 */
	register int from, to;

#ifdef ADEBUG
	if (trace)
		tfixnl(), fprintf(trace, "vadjDL(%d, %d)\n", p, cnt);
#endif
	/*
	 * Would like to use structured assignment but early
	 * v7 compiler (released with phototypesetter for v6)
	 * can't do it.
	 */
	copy(tlines, vtube, sizeof vtube);	/*SASSIGN*/
	for (from = p + cnt, to = p; from <= WECHO; from++, to++)
		vtube[to] = tlines[from];
	for (from = p; to <= WECHO; from++, to++) {
		vtube[to] = tlines[from];
		vclrcp(vtube[to], WCOLS);
	}
}
/*
 * Sync the screen, like redraw but more lazy and willing to leave
 * @ lines on the screen.  VsyncCL syncs starting at the current line.
 * In any case, if the redraw option is set then all syncs map to redraws
 * as if vsync didn't exist.
 */
void vsyncCL(void)
{

	vsync(LINE(vcline));
}

void vsync(register int p)
{

	if (value(REDRAW))
		vredraw(p);
	else
		vsync1(p);
}

/*
 * The guts of a sync.  Similar to redraw but
 * just less ambitous.
 */
void vsync1(register int p)
{
	register int l;
	wchar_t temp[LBSIZE];
	register struct vlinfo *vp = &vlinfo[0];
	short oldhold = hold;

#ifdef ADEBUG
	if (trace)
		tfixnl(), fprintf(trace, "vsync1(%d)\n", p), tvliny();
#endif
	if (holdupd) {
		if (holdupd < 3)
			holdupd = 2;
		return;
	}
	if (state == HARDOPEN || splitw)
		return;
	vscrap();
	CP(temp, linebuf);
	if (vcnt == 0)
		LINE(0) = WTOP;
	l = 0;
	while (l < vcnt && vp->vliny < p)
		l++, vp++;
	heldech = 0;
	hold |= HOLDECH;
	while (p <= WBOT && Peekkey != ATTN) {
		/*
		 * Want to put a line here if not in visual and first line
		 * or if there are lines left and this line starts before
		 * the current line, or if this line is piled under the
		 * next line (vreplace does this and we undo it).
		 */
		if (l == 0 && state != VISUAL ||
		    (l < vcnt && (vp->vliny <= p || vp[0].vliny == vp[1].vliny))) {
			if (l == 0 || vp->vliny < p || (vp->vflags & VDIRT)) {
				if (l == vcline)
					strcLIN(temp);
				else
					getline(dot[l - vcline]);
				/*
				 * Be careful that a long line doesn't cause the
				 * screen to shoot up.
				 */
				if (l != vcline && (vp->vflags & VDIRT)) {
					vp->vdepth = vdepth();
					vp->vflags &= ~VDIRT;
					if (p + vp->vdepth - 1 > WBOT)
						break;
				}
				vreopen(p, lineDOT() + (l - vcline), l);
			}
			p = vp->vliny + vp->vdepth;
			vp++;
			l++;
		} else
			/*
			 * A physical line between logical lines,
			 * so we settle for an @ at the beginning.
			 */
			vclrlin(p, dot + (l - vcline)), p++;
	}
	strcLIN(temp);
	hold = oldhold;
	if (heldech)
		vclrech((short)0);
}

/*
 * Subtract (logically) cnt physical lines from the 
 * displayed position of lines starting with line l.
 */
static void vcloseup(int l, register int cnt)
{
	register int i;

#ifdef ADEBUG
	if (trace)
		tfixnl(), fprintf(trace, "vcloseup(%d, %d)\n", l, cnt);
#endif
	for (i = l + 1; i <= vcnt; i++)
		LINE(i) -= cnt;
}

/*
 * Workhorse for rearranging line descriptors on changes.
 * The idea here is that, starting with line l, cnt lines
 * have been replaced with newcnt lines.  All of these may
 * be ridiculous, i.e. l may be -1000, cnt 50 and newcnt 0,
 * since we may be called from an undo after the screen has
 * moved a lot.  Thus we have to be careful.
 *
 * Many boundary conditions here.
 */
void vreplace(int l, int cnt, int newcnt)
{
	register int from, to, i;
	short savenote = 0;

#ifdef ADEBUG
	if (trace) {
		tfixnl(), fprintf(trace, "vreplace(%d, %d, %d)\n", l, cnt, newcnt);
		tvliny();
	}
#endif
	if (l >= vcnt)
		return;
	if (l < 0) {
		if (l + cnt < 0) {
			/*
			 * Nothing on the screen is relevant.
			 * Settle for redrawing from scratch (later).
			 */
			vcnt = 0;
			return;
		}
		/*
		 * Normalize l to top of screen; the add is
		 * really a subtract from cnt since l is negative.
		 */
		cnt += l;
		l = 0;

		/*
		 * Unseen lines were affect so notify (later).
		 */
		savenote++;
	}

	/*
	 * These shouldn't happen
	 * but would cause great havoc.
	 */
	if (cnt < 0)
		cnt = 0;
	if (newcnt < 0)
		newcnt = 0;

	/*
	 * Surely worthy of note if more than report
	 * lines were changed.
	 */
	if (cnt > value(REPORT) || newcnt > value(REPORT))
		savenote++;

	/*
	 * Same number of lines affeted as on screen, and we
	 * can insert and delete lines.  Thus we just type
	 * over them, since otherwise we will push them
	 * slowly off the screen, a clear lose.
	 */
	if (cnt == newcnt || vcnt - l == newcnt && insert_line && delete_line) {
		if (cnt > 1 && l + cnt > vcnt)
			savenote++;
		vdirty(l, newcnt);
	} else {
		/*
		 * Lines are going away, squish them out.
		 */
		if (cnt > 0) {
			/*
			 * If non-displayed lines went away,
			 * always notify.
			 */
			if (cnt > 1 && l + cnt > vcnt)
				savenote++;
			if (l + cnt >= vcnt)
				cnt = vcnt - l;
			else
				for (from = l + cnt, to = l; from <= vcnt; to++, from++)
					vlcopy(vlinfo[to], vlinfo[from]);
			vcnt -= cnt;
		}
		/*
		 * Open up space for new lines appearing.
		 * All new lines are piled in the same place,
		 * and will be unpiled by vredraw/vsync, which
		 * inserts lines in front as it unpiles.
		 */
		if (newcnt > 0) {
			/*
			 * Newlines are appearing which may not show,
			 * so notify (this is only approximately correct
			 * when long lines are present).
			 */
			if (newcnt > 1 && l + newcnt > vcnt + 1)
				savenote++;

			/*
			 * If there will be more lines than fit, then
			 * just throw way the rest of the stuff on the screen.
			 */
			if (l + newcnt > WBOT && insert_line && delete_line) {
				vcnt = l;
				goto skip;
			}
			from = vcnt, to = vcnt + newcnt;
			i = TUBELINES - to;
			if (i < 0)
				from += i, to += i;
			vcnt = to;
			for (; from >= l; from--, to--)
				vlcopy(vlinfo[to], vlinfo[from]);
			for (from = to + 1, to = l; to < l + newcnt && to <= WBOT + 1; to++) {
				LINE(to) = LINE(from);
				DEPTH(to) = 0;
				FLAGS(to) = VDIRT;
			}
		}
	}
skip:
	if (Pline == numbline && cnt != newcnt)
		/*
		 * When lines positions are shifted, the numbers
		 * will be wrong.
		 */
		vdirty(l, WECHO);
	if (!savenote)
		notecnt = 0;
#ifdef ADEBUG
	if (trace)
		tvliny();
#endif
}

/*
 * Start harcopy open.
 * Print an image of the line to the left of the cursor
 * under the full print of the line and position the cursor.
 * If we are in a scroll ^D within hardcopy open then all this
 * is suppressed.
 */
void sethard(void)
{

	if (state == VISUAL)
		return;
	rubble = 0;
	state = HARDOPEN;
	if (hold & HOLDROL)
		return;
	vup1();
	LINE(0) = WBOT;
	if (Pline == numbline)
		vgoto(WBOT, 0), ex_printf("%6d  ", lineDOT());
}

/*
 * Mark the lines starting at base for i lines
 * as dirty so that they will be checked for correct
 * display at next sync/redraw.
 */
void vdirty(register int base, register int i)
{
	register int l;
	
	for (l = base; l < vcnt; l++) {
		if (--i < 0)
			return;
		FLAGS(l) |= VDIRT;
	}
}
