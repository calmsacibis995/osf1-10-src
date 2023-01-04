/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1990
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
#define KD_READY_0

#define SCREEN ((char *) 0xc00b8000)
#define LINES	24
#define COLS	80
#define SIZE	(LINES*COLS*2)
#define SCREEN_END (SCREEN+SIZE)

/*
 * Color attribute byte.  0-7 are normal intensity colors.
 * 8-15 are high intensity colors.  For example, (INTENSE + RED)
 * or (INTENSE | RED) will give you the 'Light Red' color.
 */
#define BLACK                           0
#define BLUE                            1
#define GREEN                           2
#define CYAN                            3
#define RED                             4
#define MAGENTA                         5
#define YELLOW                          6
#define WHITE                           7
#define INTENSE                         8

#define NORMAL 0x07

int screen_col, screen_line;
int screen_lines = LINES;

char *lines[LINES+1] = { 0 };

int _noputchar;
int screen_delay = 10000;

#ifdef KD_READY_0
extern int	cnputc();
int	outputing_debugger_text;
#endif KD_READY_0

screen_putchar(c)
{
#ifdef KD_READY_0
	outputing_debugger_text = 1;		/* TRUE */
	cnputc(c);
	outputing_debugger_text = 0;		/* FALSE */
#else
	int i;

	if (_noputchar)
		return;
	if (lines[screen_line] == 0)
		init_screen();
	switch(c)
	{
	case '\t':	/* output some blanks for tabs */
		do
		{
			lines[screen_line][screen_col++] = ' ';
			lines[screen_line][screen_col++] = NORMAL;
		} while (screen_col & 0xf);
		break;
	default:
		lines[screen_line][screen_col++] = c;
		lines[screen_line][screen_col++] = NORMAL;
		if (screen_col < COLS*2)
			break;
		/* fall thru */
	case '\n':
		while (screen_col < COLS*2)
			{
			lines[screen_line][screen_col++] = ' ';
			lines[screen_line][screen_col++] = NORMAL;
			}
		screen_col = 0;
		if (++screen_line >= LINES)
			screen_line = 0;
		for (i=0; i<screen_delay; ++i )
			;		/* delay for a while so it can be read */
		break;

	}
#endif KD_READY_0
}

init_screen()
{
	unsigned char *screen = SCREEN;
	int i;

	if (screen[0] == 0xff && screen[1] == 0xff)
		screen -= 0x8000;	/* use mono screen! */
	for (i=0; i<=LINES; ++i, screen += COLS*2)
		lines[i] = screen;
	clear_screen();
}

clear_screen()
{
/*	memset(SCREEN, 0, SCREEN_END - SCREEN);	/* */
}
