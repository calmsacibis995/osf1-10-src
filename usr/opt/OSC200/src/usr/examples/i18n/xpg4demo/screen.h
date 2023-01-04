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
/*
 * @(#)$RCSfile: screen.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/12/20 21:51:37 $
 */

/*
 * Number of fields, field indices, maximum field index
 */
#define	SCR_NFIELDS		8
#define SCR_NDATAFIELDS		5
#define	SCR_BADGE		0
#define	SCR_FIRSTNAME		1
#define	SCR_SURNAME		2
#define	SCR_CC			3
#define	SCR_DOJ			4
#define	SCR_MENU		5
#define SCR_PROMPT		6
#define	SCR_MESSAGE		7
#define	SCR_FIELD_MAX		SCR_MESSAGE

typedef enum {
	DataField, CommandField, PromptField, MessageField
} ScrFieldType;
typedef	short ScrAddress;		/* Assume ScrAddress < 32767 */
typedef struct _ScrPosition ScrPosition;
typedef struct _ScrPosition ScrSize;
typedef struct _ScrField ScrField;
typedef struct _ScreenRec Screen;
typedef struct _ScrProto ScrProto;

struct _ScrPosition {
	ScrAddress	col, line;
};

struct _ScrField {
	ScrFieldType	field_type;	/* Field type */
	ScrPosition	title_pos;	/* Screen position of title */
	char		*title_str;	/* Title string */
	char		*alt_title_str;	/* Alternate title string (optional) */
	ScrPosition	output;		/* Screen position of data output */
	char		*out_format;	/* Data output format */
};

struct _ScreenRec {
	ScrSize		size;
	int		*fldmap;
	Boolean		narrow;		/* True if narrow layout */
	char		*termbuf;	/* termcap buffer */
	char		*cl;		/* Clear screen */
	char		*cm;		/* Cursor motion */
	char		*ce;		/* Clear to end of line */
	int		max_field_width;/* Maximum field width */
	int		nfields;	/* Number of fields */
	ScrField	field[SCR_NFIELDS];
};

struct _ScrProto {
	ScrFieldType	field_type;	/* Field type */
	int		msg_id;		/* Message ID for title */
	char		*title_str;	/* Default title string */
	int		out_msg_id;	/* Message ID for output format;
					   -1 if not applicable */
	char		*out_format;	/* Default output format */
};

#define FLUSH_SCREEN()		fflush(stdout)
#ifdef CURSES
static void PutChar(int c);

#define TGOTO(col, line)	tputs(tgoto(screen->cm, col, line), 1, PutChar);
#define KILL_LINE()		tputs(screen->ce, 1, PutChar)
#define CL_SCREEN()		tputs(screen->cl, 1, PutChar)
#else
#define TGOTO(col, line)	fputs(tgoto(screen->cm, col, line),stdout);fflush(stdout)
#define KILL_LINE()		fputs(screen->ce,stdout);fflush(stdout)
#define CL_SCREEN()		fputs(screen->cl,stdout);fflush(stdout)
#endif
