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
/* Input and output characters using the alpha simulator's console uart */
/* 001, 21-May-1990, Ken Lesniak */

#include <termios.h>
#undef CTRL

extern char *consolebase;

#define CONSOLE_BASE consolebase
#define TXDB 0x0118             /* Transmitter data buffer register */
#define TXCS 0x0110             /* Transmitter control/status register */
#define RXDB 0x0108             /* Receiver data buffer register */
#define RXCS 0x0100             /* Receiver control/status register */
#define PUTC(c) (base[TXDB / sizeof(int)] = c)
#define GETC() (base[RXDB / sizeof(int)])
#define CTRL(c) ('c' - 64)
#define FALSE 0
#define TRUE 1

extern struct termios _cur_termios;     /* Current terminal settings */

static int putcpos = 0;         /* Current output position */

void _putc(c)
        int             c;
    {
        register int    *base;

        base = (int *)CONSOLE_BASE;

       if (_cur_termios.c_oflag & OPOST) {
            if (_cur_termios.c_oflag & ONLCR && c == '\n') {
                putcpos = 0;
                PUTC('\r');
            };
	};

        switch (c) {
            case '\n':
                break;

            case '\r':
                putcpos = 0;
                break;

            case CTRL(H):
                if (putcpos > 0)
                    putcpos--;
                break;

            case '\t':
		putcpos = (putcpos + 8) & ~7;
		break;

/*	    case '\000': */	/* NULL */
	    case '\001':	/* CTRL(A) */
	    case '\002':	/* CTRL(B) */
	    case '\003':	/* CTRL(C) */
	    case '\004':	/* CTRL(D) */
	    case '\005':	/* CTRL(E) */
	    case '\006':	/* CTRL(F) */
	    case '\007':	/* CTRL(G) */
/*	    case '\010': */	/* CTRL(H) */
/*	    case '\011': */	/* \t */
/*	    case '\012': */	/* \n */
	    case '\013':	/* CTRL(K) */
	    case '\014':	/* CTRL(L) */
/*	    case '\015': */	/* \r */
	    case '\016':	/* CTRL(N) */
	    case '\017':	/* CTRL(O) */
	    case '\020':	/* CTRL(P) */
	    case '\021':	/* CTRL(Q) */
	    case '\022':	/* CTRL(R) */
	    case '\023':	/* CTRL(S) */
	    case '\024':	/* CTRL(T) */
	    case '\025':	/* CTRL(U) */
	    case '\026':	/* CTRL(V) */
	    case '\027':	/* CTRL(W) */
	    case '\030':	/* CTRL(X) */
	    case '\031':	/* CTRL(Y) */
	    case '\032':	/* CTRL(Z) */
/*	    case '\033': */	/* ESCape */
	    case '\034':	/* CTRL(\) */
	    case '\035':	/* CTRL(]) */
	    case '\036':	/* CTRL(^) */
	    case '\037':	/* CTRL(_) */
		if (_cur_termios.c_oflag & OPOST) {
		    PUTC('^');
		    c += 64;
		    putcpos++;
		};
		/* FALL INTO DEFAULT */

	    default:
		putcpos++;
		break;
	};
	PUTC(c);
    }

#define LINESZ 256
static char	line[LINESZ];
static int	linelen = 0;
static char	*linep;

static void getline()
    {
	register int	cursor;
	register int	*base;
	register char	c;
	int		pos[LINESZ];

	base = (int *)CONSOLE_BASE;
	cursor = 0;
	for (;;) {
	    pos[cursor] = putcpos;
	    c = GETC();

	    if (c == _cur_termios.c_cc[VERASE]) {
		if (cursor > 0) {
		    cursor--;
		    if (_cur_termios.c_lflag & ECHO) {
			while (putcpos != pos[cursor]) {
			    _putc(CTRL(H));
			    _putc(' ');
			    _putc(CTRL(H));
			};
		    };
		};
		continue;
	    } else if (c == _cur_termios.c_cc[VKILL]) {
		if (_cur_termios.c_lflag & ECHO) {
		    while (putcpos != pos[0]) {
			_putc(CTRL(H));
			_putc(' ');
			_putc(CTRL(H));
		    };
		};
		cursor = 0;
		continue;
	    } else if (_cur_termios.c_iflag & ICRNL && c == '\r')
		c = '\n';

	    if (cursor == LINESZ) {
		PUTC(CTRL(G));	/* really want <BEL>, not ^G */
		continue;
	    };

	    line[cursor++] = c;

	    if (c == _cur_termios.c_cc[VEOF])
		break;

	    if (_cur_termios.c_lflag & ECHO)
		_putc(c);

	    if (c == '\n')
		break;
	};

	linelen = cursor;
	linep = line;
    }

int _getc()
    {
	register int	*base;
	register int	c;

	if (_cur_termios.c_lflag & ICANON) {
	    if (linelen == 0)
		getline();
	    c = *linep++;
	    linelen--;

	    if (c == _cur_termios.c_cc[VEOF])
		c = -1;
	} else if (linelen > 0) {
	    c = *linep++;
	    linelen--;
	} else {
	    base = (int *)CONSOLE_BASE;
	    c = GETC();
	    if (_cur_termios.c_lflag & ECHO)
		_putc(c);
	};

	return c;
    }
