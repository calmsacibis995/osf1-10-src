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
static char *rcsid = "@(#)$RCSfile: screen.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/08/03 15:55:44 $";
#endif

#include <stdlib.h>
#include "xpg4demo.h"
#include "xpg4demo_msg.h"
#include "screen.h"

/*
 * Use `full screen mode' to display a record to be acted on by the user.
 * Interaction with the user is also done using full screen mode.
 * Use scrolling screen mode to display a listing of employee records.
 */
Boolean			FullScreenMode = FALSE;

/*
 * Define the separator between a data name and its value, assuming that 
 * the separator does not need to vary among locales. (Otherwise, the 
 * separator would need to be obtained from the message catalog.)
 */
static const char	*scrFieldSep = " :   ";

/*
 * (Pointer to) data structure for screen control.
 */
static Screen		*screen;

/*
 * Prototype data for the Screen data structure.
 */
static ScrProto scrProto[] = {
	{ DataField, S_SCR_BADGE, "Employee Badge Number", -1, BADGE_FORMAT },
	{ DataField, S_SCR_NAME, "First (Given) Name", -1, "%S" },
	{ DataField, S_SCR_SURNAME, "Surname", -1, "%S" },
	{ DataField, S_SCR_CC, "Cost Center", -1, "%S" },
	{ DataField, S_SCR_DOJ, "Date of Joining", S_SCR_DATE_FMT, "%x" },
	{ CommandField, S_SCR_MENU,
		  "1-Modify 2-Create 3-Delete 4-List 5-Exit ?", -1, "%s" },
	{ PromptField, 0, "", -1, "%s" },
	{ MessageField, 0, (char *) NULL, -1, (char *) NULL },
};

/*
 * The two arrays are used to swap first name and surname for certain
 * locales (needed in the Japanese locale, for example).
 */
static int scrFSMap[SCR_NFIELDS] = {
	SCR_BADGE,
	SCR_FIRSTNAME,
	SCR_SURNAME,
	SCR_CC,
	SCR_DOJ,
	SCR_MENU,
	SCR_PROMPT,
	SCR_MESSAGE,
};

static int scrSFMap[SCR_NFIELDS] = {
	SCR_BADGE,
	SCR_SURNAME,
	SCR_FIRSTNAME,
	SCR_CC,
	SCR_DOJ,
	SCR_MENU,
	SCR_PROMPT,
	SCR_MESSAGE,
};

static void scrInitTermcap(Screen *screen);

#ifdef CURSES
char	PC = '\0';
short	ospeed = 9600;

extern char *tgoto();
extern int tputs();
#else
char *tgoto();
#endif

/*
 * Initialize the termcap environment and the Screen data structure.
 * Zero is returned if all initialization is successful; otherwise, -1 is
 * returned.
 */
int
InitScreen()
{
	int i, line, field_sep_width;
	char *s;

	/*
	 * Allocate memory for the Screen data structure.
	 */
	if ((screen = (Screen *) calloc(sizeof(Screen), 1)) == (Screen *) NULL)
		NoMemory(GetErrorMsg(E_SCR_NOMEMORY, "screen structure"));

	/*
	 * Initialize termcap.
	 */
	scrInitTermcap(screen);

	/*
	 * Initialize Screen.
	 */
	screen->nfields = SCR_NFIELDS;

	/*
	 * Get a line offset for the top field.
	 */
	line = (screen->size.line - (2*SCR_NDATAFIELDS + 3*3))/2;
	if (line++ < 0) {
		/*
		 * Try narrow layout.
		 */
		line = (screen->size.line - (SCR_NDATAFIELDS + 2*3)) / 2;
		if (line++ < 0)
			return (-1);
		screen->narrow = TRUE;
	}
	screen->fldmap = SurnameFirst ? scrSFMap : scrFSMap;
	/*
	 * Determine line position of each field.
	 */
	for (i = 0; i <= SCR_FIELD_MAX; i++) {
		ScrField *field;
		ScrProto *proto;
		int j;

		field = &screen->field[i];
		proto = &scrProto[screen->fldmap[i]];
		field->field_type = proto->field_type;
		field->title_pos.line = field->output.line = line;
		if (proto->title_str)
			field->title_str = GetMessage(MSGString,
						      proto->msg_id,
						      proto->title_str);
		else
			field->title_str = proto->title_str;
		if (proto->out_msg_id == -1)
			field->out_format = proto->out_format;
		else
			field->out_format = GetMessage(MSGString,
						       proto->out_msg_id,
						       proto->out_format);
		field->title_pos.col = 1;
		if (field->field_type == DataField) {
			screen->max_field_width = MAX(MbsWidth(field->title_str),
					    screen->max_field_width);
			line += screen->narrow ? 1 : 2;
		} else
			line += screen->narrow ? 2 : 3;
	}
	++screen->max_field_width;
	field_sep_width = MbsWidth((char *) scrFieldSep);
	/*
	 * Set the positions for displaying employee data items.
	 */
	for (i = 0; i <= SCR_FIELD_MAX; i++) {
		ScrField *field;

		field = &screen->field[i];
		if (field->field_type == DataField)
			field->output.col = screen->max_field_width + field_sep_width;
		else
			field->output.col = MbsWidth(field->title_str) + field_sep_width;
	}
	return (0);
}

/*
 * Display the specified employee record. Always set to full screen
 * mode and set CurrentEmployee to be this record.
 */
void
ShowEmployee(Employee *emp)
{
	int  i;
	char buf[80];

	CL_SCREEN();
	FullScreenMode = TRUE;
	if (emp == (Employee *) NULL)
		return;

	for (i = 0; i <= SCR_FIELD_MAX; i++) {
		ScrField *field;

		field = &screen->field[i];
		if (field->field_type != DataField)
			continue;
		TGOTO(field->title_pos.col, field->title_pos.line);
		(void) fputs(field->title_str, stdout);
		TGOTO(screen->max_field_width, field->title_pos.line);
		(void) fputs(scrFieldSep, stdout);

		switch (screen->fldmap[i]) {
		case SCR_BADGE:
			printf(field->out_format, emp->badge_num);
			break;

		case SCR_FIRSTNAME:
			printf(field->out_format, emp->first_name);
			break;

		case SCR_SURNAME:
			printf(field->out_format, emp->surname);
			break;

		case SCR_CC:
			printf(field->out_format, emp->cost_center);
			break;

		case SCR_DOJ:
			strftime(buf, sizeof(buf), field->out_format,
				 &emp->date_of_join);
			(void) fputs(buf, stdout);
		}
	}
	FLUSH_SCREEN();
	CurrentEmployee = emp;
}

/*
 * Display the menu for command input.
 */
void
ShowMenu(void)
{
	ScrField *field;

	field = &screen->field[screen->fldmap[SCR_MENU]];
	if (FullScreenMode) {
		TGOTO(field->title_pos.col, field->title_pos.line);
		KILL_LINE();
		(void) fputs(field->title_str, stdout);
		TGOTO(field->output.col, field->output.line);
	} else
		printf("\n%s  ", field->title_str);
	FLUSH_SCREEN();
}

/*
 * Input badge number. Default badge number can be assigned using `emp'.
 * `fmt' is used for displaying the prompt message, if specified.
 */
BadgeNumber
InputBadgeNo(Employee *emp, const char *fmt)
{
	wchar_t buf[80];
	BadgeNumber badge = (BadgeNumber) -1;
	ScrField *field = &screen->field[screen->fldmap[SCR_BADGE]];
	char fmtbuf[256];

	/*
	 * It is necessary to copy the specified format because
	 * subsequent catgets() calls might corrupt the string.
	 */
	if (fmt)
		(void) strcpy(fmtbuf, fmt);

	for (;;) {
		if (emp) {
			badge = emp->badge_num;
			Ask(fmt? fmtbuf : GetMsg(I_SCR_PROMPT_BADGE_DEF, "%s [%ld]? "),
			    field->title_str, badge);
		} else
			Ask(fmt? fmtbuf : GetMsg(I_SCR_PROMPT_BADGE, "%s? "),
			    field->title_str);
		if (fgetws(buf, NUM_ELEMENTS(buf), stdin))
			if (buf[0] != L'\n')
				badge = (BadgeNumber) wcstol(buf, (wchar_t **) NULL, 10);
		if (badge >= BADGENUM_MIN && badge <= BADGENUM_MAX)
			break;

		Error(GetErrorMsg(E_SCR_INV_BADGE, "Invalid badge number"));
	}

	ErasePromptLine();
	EraseMessageLine();
	return (badge);
}

/*
 * Input first name. Default first name can be assigned using `emp'.
 */
void
InputFirstname(Employee *emp)
{
	ScrField *field = &screen->field[screen->fldmap[SCR_FIRSTNAME]];
	wchar_t buf[80];
	int len;

	for (;;) {
		if (emp->first_name[0])
			Ask(GetMsg(I_SCR_PROMPT_NAME_DEF, "%s [%S]? "),
			    field->title_str, emp->first_name);
		else
			Ask(GetMsg(I_SCR_PROMPT_NAME, "%s? "),
			    field->title_str);
		if (fgetws(buf, NUM_ELEMENTS(buf), stdin)) {
			if (buf[0] != L'\n') {
				len = wcslen(buf) - 1;
				buf[len] = '\0';
				if (len <= FIRSTNAME_MAX) {
					wcscpy(emp->first_name, buf);
					break;
				} else
					Error(GetErrorMsg(E_SCR_2LONG_1STNM,
							  "First name too long"));
			} else if (emp->first_name[0])
				break;
		}
	}
	EraseMessageLine();
}

/*
 * Input surname. Default surname can be assigned using `emp'.
 */
void
InputSurname(Employee *emp)
{
	ScrField *field = &screen->field[screen->fldmap[SCR_SURNAME]];
	wchar_t buf[80];
	int len;

	for (;;) {
		if (emp->surname[0])
			Ask(GetMsg(I_SCR_PROMPT_NAME_DEF, "%s [%S]? "),
			    field->title_str, emp->surname);
		else
			Ask(GetMsg(I_SCR_PROMPT_NAME, "%s? "),
			    field->title_str);
		if (fgetws(buf, NUM_ELEMENTS(buf), stdin)) {
			if (buf[0] != L'\n') {
				len = wcslen(buf) - 1;
				buf[len] = '\0';
				if (len <= SURNAME_MAX) {
					wcscpy(emp->surname, buf);
					break;
				} else
					Error(GetErrorMsg(E_SCR_2LONG_SURNM,
							  "Surname too long"));
			} else if (emp->surname[0])
				break;
		}
	}
	EraseMessageLine();
}

/*
 * Input cost center. Default cost center can be assigned using `emp'.
 */
void
InputCostCenter(Employee *emp)
{
	ScrField *field = &screen->field[screen->fldmap[SCR_CC]];
	wchar_t buf[80];
	int len;

	for (;;) {
		if (emp->cost_center[0])
			Ask(GetMsg(I_SCR_PROMPT_CC_DEF, "%s [%S]? "),
			    field->title_str, emp->cost_center);
		else
			Ask(GetMsg(I_SCR_PROMPT_CC, "%s? "),
			    field->title_str);
		if (fgetws(buf, NUM_ELEMENTS(buf), stdin)) {
			if (buf[0] != L'\n') {
				len = wcslen(buf) - 1;
				buf[len] = '\0';
				if (len == CC_LEN) {
					int i;

					for (i = 0; i < CC_LEN; i++) {
						if (!iswalnum(buf[i]))
							goto err;
						emp->cost_center[i] = towupper(buf[i]);
					}
					break;
				}
			err:
				emp->cost_center[0] = L'\0';
				Error(GetErrorMsg(E_SCR_INV_CC,
						  "Invalid Cost Center"));
			} else if (emp->cost_center[0])
				break;
		}
	}
	EraseMessageLine();
}

/*
 * Input date of joining. Default date can be assigned using `emp'. 
 * If available, use the strptime() function for date format 
 * conversion.
 */
void
InputDOJ(Employee *emp)
{
	ScrField *field = &screen->field[SCR_DOJ];
	char buf[80];
	int year, month, day;
	char *prompt = GetMessage(MSGString, S_SCR_DOJ_FMT,
				  "Date Of Joining (MM/DD/YY)");

	for (;;) {
		if (emp->date_of_join.tm_year) {
			strftime(buf, sizeof(buf), field->out_format, &emp->date_of_join);
			Ask(GetMsg(I_SCR_PROMPT_DOJ_DEF, "%s [%s]? "), prompt, buf);
		} else
			Ask(GetMsg(I_SCR_PROMPT_DOJ, "%s? "), prompt);
		if (gets(buf)) {
			if (buf[0]) {
				sscanf(buf, GetMsg(I_SCR_IN_DATE_FMT, "%2$d/%3$d/%1$d"),
				       &year, &month, &day);

				/*
				 * Perform simple check of date validity.
				 */
				if (year >= 1900 && year < 2000)
					year -= 1900;
				else if (!(year >= 0 && year < 100))
					goto invalid;
				if (month < 1 || month > 12)
					goto invalid;
				if (day < 1 || day > 31)
					goto invalid;
				emp->date_of_join.tm_year = year;
				emp->date_of_join.tm_mon = month - 1;
				emp->date_of_join.tm_mday = day;
				break;
			} else if (emp->date_of_join.tm_mday != 0)
				break;
		}
	invalid:
		Error(GetErrorMsg(E_SCR_INV_JOD, "Invalid date"));
	}
	free(prompt);
	EraseMessageLine();
}

void
ScrClear()
{
	CL_SCREEN();
	FLUSH_SCREEN();
}

void
GotoPromptLine(void)
{
	if (FullScreenMode) {
		TGOTO(screen->field[SCR_PROMPT].title_pos.col,
		      screen->field[SCR_PROMPT].title_pos.line);
		KILL_LINE();
		FLUSH_SCREEN();
	}
}

void
GotoMessageLine(void)
{
	if (FullScreenMode) {
		TGOTO(screen->field[SCR_MESSAGE].title_pos.col,
		      screen->field[SCR_MESSAGE].title_pos.line);
		KILL_LINE();
		FLUSH_SCREEN();
	}
}

#ifdef CURSES
static void
scrInitTermcap(Screen *screen)
{
	char termbuf[2048];
	char **areap;
	char *terminal_name;
	extern char *tgetstr();
	extern int tgetnum();
	int status;

	if ((terminal_name = getenv("TERM")) == (char *) NULL) {
		Warning(GetErrorMsg(E_SCR_UNKOWN_TERM,
				    "Unknown terminal type. Try \"ansi\""));
		terminal_name = "ansi";
	}
	if ((status = tgetent(termbuf, terminal_name)) < 0)
		Fatal(GetErrorMsg(E_SCR_TERMCAP_OPEN,
				  "Cannot open termcap/terminfo database"));
	if (status == 0)
		Fatal(GetErrorMsg(E_SCR_UNDEFTERM,
				  "Terminal type %s not defined"),
		      terminal_name);

	screen->termbuf = malloc(strlen(termbuf));
	if (screen->termbuf == (char *) NULL)
		NoMemory(GetErrorMsg(E_SCR_TERMNOMEM, "termcap buffer"));
	areap = &screen->termbuf;
	screen->size.col = tgetnum("co");
	screen->size.line = tgetnum("li");
	screen->cl = tgetstr("cl", areap);
	screen->cm = tgetstr("cm", areap);
	screen->ce = tgetstr("ce", areap);
}

static void
PutChar(int c)
{
	putchar(c);
}

#else
/*
 * If the curses library is not available, assume an ANSI terminal.
 */
static void
scrInitTermcap(Screen *screen)
{
	screen->size.col = 80;
	screen->size.line = 24;
	screen->cl = "\033[H\033[2J\033[?6l";
	screen->cm = "\033[%d;%dH";	/* Cursor Position (CUP) */
	screen->ce = "\033[K";
}

/*
 * Emulate tgoto() function of termcap routines.
 */
char *
tgoto(char *cm, int col, int line)
{
    static char buf[32];

    (void) sprintf(buf, cm, line-1, col-1);
    return (buf);
}

#endif /* CURSES */
