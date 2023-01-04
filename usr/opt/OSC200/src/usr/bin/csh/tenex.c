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
static char rcsid[] = "@(#)$RCSfile: tenex.c,v $ $Revision: 1.1.9.4 $ (DEC) $Date: 1993/12/21 21:48:55 $";
#endif
/*
 * HISTORY
 */
/*
 * This is a command line editor for csh.
 *
 * The editor was written by Duane Williams with assistance
 * from Doug Philips, who also wrote the interface to the history
 * list and the code to save and restore keyboard macros in files.
 * The tenex style filename recognition was written by Ken Greer.
 *
 * Copyright (C) 1983,1984 by Duane Williams
 * 
 *
 * NOTE:  For OSF/1 this package does not currently support i18n.
 *	  This is technically not supported code.
 * 	  A man page for this functionality can be found in
 * 	  the nosupport directories. (/usr/local/nosupport/man)
 *
 */

#ifdef CMDEDIT
#include <sgtty.h>
#include <sys/file.h>
#include <string.h>
#include <unistd.h>
#include "sh.h"
#undef TRUE
#undef FALSE

#define CTRL_AT '\000'
#define CTRL_A '\001'
#define CTRL_B '\002'
#define CTRL_C '\003'
#define CTRL_D '\004'
#define CTRL_E '\005'
#define CTRL_F '\006'
#define CTRL_G '\007'
#define CTRL_H '\010'
#define CTRL_K '\013'
#define CTRL_L '\014'
#define CTRL_N '\016'
#define CTRL_P '\020'
#define CTRL_Q '\021'
#define CTRL_R '\022'
#define CTRL_S '\023'
#define CTRL_T '\024'
#define CTRL_U '\025'
#define CTRL_V '\026'
#define CTRL_W '\027'
#define CTRL_X '\030'
#define CTRL_Y '\031'
#define ESCAPE '\033'
#define LINEFEED '\n'
#define RETURN   '\r'
#define RUBOUT '\177'
#define SPACE	  ' '
#define TAB    '\t'

#define BINDINGS  256
#define MAXSPACES 128
#define MAXINT	  0x7FFF

#define DEF	       Editor
#define READ(buf,n)    read(SHIN,buf,n)
#define ISDIGIT(c)     ('0' <= (c) && (c) <= '9')

#define SETBINDING(keymap,function,key)	 keymap[key].Routine = function
#define REGBIND(function,key) SETBINDING(RegBindings,function,key)
#define ESCBIND(function,key) SETBINDING(EscBindings,function,key)
#define CNXBIND(function,key) SETBINDING(CnxBindings,function,key)
#define ANSIBIND(function,key) SETBINDING(AnsiBindings,function,key)

#define loop for(;;)

typedef enum { FALSE, TRUE } boolean;
typedef enum { EMACS,VI,ESC,CNX,RET,QUO,REP,INS,OVERFLOW,DUMB,XEOF,ANSI} code;
typedef enum { BACK, FORW } direction;

typedef struct {
    uchar_t * Head;
    uchar_t * Tail;
} String;

typedef code (*CodeFunction)();

typedef struct {
    CodeFunction Routine;
    uchar_t	 MacroName;
} KeyBinding;


extern short ospeed;

/*
 * Prototype static functions.
 */

static int			ShoutItOut(uchar_t c);
static int			PrintingLength (uchar_t *string, int);
static void			WriteToScreen (uchar_t *, int);
static void			Backup (uchar_t *, int);
static void			BlotOut (uchar_t *, int);
static void			SaveTtyState(void);
static void			ReadTermcap(void);
static void			EnterCbreakMode(boolean);
static void			ExitCbreakMode(void);
static int			GetChar(uchar_t *);
static code			SetCtrlX(void);
static code			SetEscape(void);
static code			SetAnsi(void);
static code			EnterViInsert(void);
static code			EnterViAppend(void);
static code			ExitViInsert(void);
static code			Return (void);
static code			EndOfFile(void);
static code			Repetition(void);
static code			InsertChar(void);
static code			InsertLiteralChar(void);
static int			DoInsertChar(void);
static int			InsertString(uchar_t *, uchar_t *);
static code			DefaultBinding (void);
static code			Ignore (void);
static code			Tab (void);
static code			Redisplay (void);
static void			Crlf (void);
static void			NewLine (void);
static code			ClearScreen (void);
static void			Beep (void);
static code			TransposeChars (void);
static code			GnuTransposeChars (void);
static boolean			IsSpaceChar (uchar_t *);
static uchar_t *		NextWordPos (uchar_t *);
static uchar_t *		PrevWordPos (uchar_t *);
static void			DeleteNChars (int, direction);
static code			DeleteWord (void);
static code			EraseWord (void);
static code			DeleteCurrentChar (void);
static code			DeletePreviousChar (void);
static code			DeleteCurrentCharOrEof (void);
static void			CopyNChars (uchar_t *, uchar_t *, int);
static code			SetMark (void);
static code			KillRegion (void);
static code			KillToEOL (void);
static code			EraseLine (void);
static code			YankKillBuffer (void);
static code			WipeLine (void);
static void			MoveNChars (int, direction);
static code			ForwardChar (void);
static code			BeginningOfLine (void);
static code			EndOfLine (void);
static code			ForwardWord (void);
static code			BackwardWord (void);
static code			BackSpace (void);
static void			InitBuffers(void);
static code			FilenameExpansion (void);
static code			FilenameList (void);
static code			FilenameListOrEof (void);
static boolean			PushMacro (String);
static void			PopMacro (void);
static code			ExecuteMacro (void);
static code			ExecuteNamedMacro (void);
static code			ExecuteUnNamedMacro (void);
static code			StartRemembering (void);
static code			StopRemembering (void);
static code			DefineNamedMacro (void);
static int			WriteMacroFile(uchar_t *);
static int			ReadMacroFile(uchar_t *);
static code			SaveMacroFile(void);
static code			LoadMacroFile(void);
static uchar_t *		string_index (String, uchar_t);
static uchar_t *		string_rindex (String, uchar_t); 
static boolean			Match(uchar_t *,uchar_t *,uchar_t *,uchar_t *);
static uchar_t *		Index(String, String);
static uchar_t *			ReverseIndex(String, String);
static code			IncrementalSearchForward (void);
static code			IncrementalSearchReverse (void);
static code			SearchForward(void);
static code			SearchReverse(void);
static int			InsertCurrentHistEntry(void);
static code			PreviousHistEntry(void);
static code			NextHistEntry(void);
static code			ChangeFollowingObject(void);
static code			DeleteFollowingObject(void);
static code			AppendToEOL(void);
static code			InsertAtBOL(void);
static code			ChangeChar(void);
static code			ChangeToEOL(void);
static code			ChangeWholeLine(void);
static code			ReplaceChar(void);
static code			ViYankKillBuffer(void);
static void			InitMacros(void);
static void			InitBindings(uchar_t *);
static void			InitEmacsBindings(void);
static void			InitViBindings(void);
static void			InitDumbBindings(void);
static void			InitDefaultBindings(void);
static void			InitAnsiBindings(void);
static void			SetUserBindings(void);
static int			keytran(uchar_t *, KeyBinding **, uchar_t *);


/*
 *---------------------------------------------------------------------------
 * Global variables, but not seen outside this package.
 *---------------------------------------------------------------------------
 */
static KeyBinding RegBindings [BINDINGS];
static KeyBinding EscBindings [BINDINGS];
static KeyBinding CnxBindings [BINDINGS];
static KeyBinding AnsiBindings [BINDINGS];
static KeyBinding * Bindings;		/* current keymap */
static KeyBinding * OldBindings;

static uchar_t CurrentChar;
static String KeyboardMacro [BINDINGS]; /* named keyboard macros */
static String UnNamedMacro;		/* unnamed keyboard macro */
static uchar_t * MacroBuffer = (uchar_t *) NULL;
static uchar_t * SpaceBuffer = (uchar_t *) NULL;
static uchar_t * BackSpaceBuffer = (uchar_t *) NULL;
static uchar_t * KillBuffer = (uchar_t *) NULL;
static uchar_t * KillTail;
static uchar_t * InputBuffer;		/* beginning of input buffer */
static uchar_t * InputPos;		        /* current position (cursor) */
static uchar_t * InputTail;		/* after last char of input */
static uchar_t * Mark;		        /* remember position in input */
static uchar_t * MacroStart = 0;		/* executing macro pointer */
static uchar_t * MacroTail;		/* after last char of macro */
static uchar_t * MacroFinal;
static boolean MacroDefined = FALSE;
static boolean DefiningMacro = FALSE;
static String * MacroStack = 0;	        /* macros can call macros */
static int MacroStackDepth = 0;	        /* current PushMacro position */
static int MacroStackLimit = 0;	        /* size of the macro stack */
static uchar_t SpaceChar [MAXSPACES];      /* chars to treat as spaces */
static int InputBufferSize;
static int RepetitionCount;	        /* repeat factor for functions */
static boolean AudibleBell = TRUE;      /* whether to use audible bell */
static boolean VisibleBell = FALSE;	/* whether to use visible bell */
static boolean ShowControlChars = TRUE; /* reset from local mode word */
static boolean ControlCharsInInput = FALSE;
static code Editor;			/* set from editmode variable */
static boolean Initialized = FALSE;
static boolean MagicChar = FALSE;       /* is there a pushed back char? */
static uchar_t MagicPushBack;		/* single push back for repeat */
static struct Hist * curHistPos;	/* current position in history */
static struct sgttyb TtySgttyb;
static struct tchars TtyTchars, TenexTchars;
static struct ltchars TtyLocalChars, TenexLchars;
static int TtyLocalModeWord;
static uchar_t ReprintChar, WordErase, LiteralNext;
static uchar_t EraseChar, KillChar;

/* Termcap Storage */
static boolean TermKnown;
static uchar_t areabuf [256];		/* storage for tgetstr() */
static uchar_t * BackSpaceSequence;	/* pointers into areabuf */
static uchar_t * ClearScreenSequence;
static uchar_t * DeleteCharacterSequence;
static uchar_t * EndDeleteSequence;
static uchar_t * EndInsertSequence;
static uchar_t * InsertPrefixSequence;
static uchar_t * InsertSuffixSequence;
static uchar_t * StartDeleteSequence;
static uchar_t * StartInsertSequence;
static uchar_t * VisibleBellSequence;
static int LengthOfBackSpaceSequence;
static int LengthOfClearScreenSequence;
static int LengthOfDeleteCharacterSequence;
static int LengthOfEndDeleteSequence;
static int LengthOfEndInsertSequence;
static int LengthOfInsertPrefixSequence;
static int LengthOfInsertSuffixSequence;
static int LengthOfStartDeleteSequence;
static int LengthOfStartInsertSequence;
static int LengthOfVisibleBellSequence;
static int LengthOfScreen;

CodeFunction FunctionPointers[];
uchar_t * FunctionNames [];

static int
ShoutItOut(uchar_t c)
{
    write( SHOUT, &c, 1);
    return(0);
}

/*---------------------------------------------------------------------------
 * The following three functions control output to the tty and ensure the
 * proper display of control chars, if requested.
 *---------------------------------------------------------------------------
 */
static int
PrintingLength (register uchar_t *string, register int length)
{
    register uchar_t * end = string + length;
    
    if (! ControlCharsInInput) {
        return(length);
    }
    
    if (ShowControlChars) {
	for (; string < end; string++) {
	    if (*string < SPACE) length++;
	}
    }
    else {
        for (; string < end; string++) {
	    if (*string < SPACE) length--;
	}
    }
    
    return(length);
}

/*
 * This function writes 'length' chars to the tty 
 * from a buffer starting at 'string'.
 */
static void
WriteToScreen (register uchar_t *string, register int length)
{
    static uchar_t buf[2] = {'^',0};
    
    if (ControlCharsInInput) {
	register uchar_t * p, * end;
	
	end = string + length;

	while (string < end) {
	    for (p = string; p < end && *p >= SPACE; p++);   /* 001  */
	    write (SHOUT, string, p - string);
	    
	    if (ShowControlChars && p < end) {
		buf[1] = (*p + 0100); /* print RUBOUT as ^? */  /* 001  */
		write (SHOUT, buf, 2);
	    }
	    
	    string = p + 1;
	}
    }
    else {
        write (SHOUT, string, length);
    }
}

/* This function backs up over 'length' chars on the tty determining the
   proper printing length from examination of a buffer starting at
   'string' */
static void
Backup (register uchar_t *string, register int length)
{
    if (ControlCharsInInput) {
	length = PrintingLength (string, length);
    }

    /* Most, if not all, of our terminals backspace with ^H; so we
       optimize backspacing by using the backspace buffer.  I hope the
       else clause is never executed. */

    if (LengthOfBackSpaceSequence == 1) {
        write (SHOUT, BackSpaceBuffer, length);
    }
    else {
	register int i;
	
        for (i = 0; i < length; i++) {
/*	    write( SHOUT, BackSpaceSequence, LengthOfBackSpaceSequence);*/
	    tputs( BackSpaceSequence, LengthOfScreen, ShoutItOut);
	}
    }
}

static void
BlotOut (register uchar_t *string, register int length)
{
    if (ControlCharsInInput) {
        length = PrintingLength (string, length);
	write (SHOUT, SpaceBuffer, length);
    }
    else {
        write (SHOUT, SpaceBuffer, length);
    }
}

/*---------------------------------------------------------------------------
 * The tenex routine runs with the user's terminal in cbreak mode.  Since
 * output processing will still be done by the tty driver in this mode, we
 * have to disable it explicitly; otherwise the affected keys can't be
 * rebound in the editor.
 *---------------------------------------------------------------------------
 */

int
tenex_initialized(void)
{
    return(Initialized == TRUE);
}

void
dosetupterm (void)
{
    Initialized = FALSE;
}

static void
SaveTtyState(void)
{
    static boolean inited = FALSE;

    if (inited) return;			   /* only do this routine once */
    
    IOCTL(SHIN, TIOCGETC, &TtyTchars, "22"); /* save current tchars */

    TenexTchars = TtyTchars;

    /* Leave t_intrc alone.  Interrupts are for real. */
    TenexTchars.t_quitc  = -1;
    TenexTchars.t_startc = -1;
    TenexTchars.t_stopc  = -1;

    IOCTL(SHIN, TIOCGLTC, &TtyLocalChars, "23");/* save ltchars */

    TenexLchars = TtyLocalChars;
    ReprintChar = TtyLocalChars.t_rprntc;
    WordErase   = TtyLocalChars.t_werasc;
    LiteralNext = TtyLocalChars.t_lnextc;
    
    TenexLchars.t_suspc  = -1;
    TenexLchars.t_dsuspc = -1;
    TenexLchars.t_rprntc = -1;
    TenexLchars.t_flushc = -1;
    TenexLchars.t_werasc = -1;
    TenexLchars.t_lnextc = -1;

    IOCTL(SHIN, TIOCLGET, &TtyLocalModeWord, "24");
    ShowControlChars = (TtyLocalModeWord & LCTLECH) ? TRUE : FALSE;

    IOCTL(SHIN, TIOCGETP, &TtySgttyb, "25");
    EraseChar = TtySgttyb.sg_erase;
    KillChar  = TtySgttyb.sg_kill;

    inited = TRUE;
}

static void
ReadTermcap(void)
{
    register struct varent * v;
    register uchar_t ** vp;
    register int i;
    static boolean Disabled = FALSE;
    
    TermKnown = FALSE;

    for (i = 0; i < sizeof(areabuf); i++) {
        areabuf[i] = '\0';
    }

    LengthOfBackSpaceSequence = 0;
    LengthOfClearScreenSequence = 0;
    LengthOfDeleteCharacterSequence = 0;
    LengthOfEndDeleteSequence = 0;
    LengthOfEndInsertSequence = 0;
    LengthOfInsertPrefixSequence = 0;
    LengthOfInsertSuffixSequence = 0;
    LengthOfStartDeleteSequence = 0;
    LengthOfStartInsertSequence = 0;
    LengthOfVisibleBellSequence = 0;

    /* Get termcap description of tty capabilities that we are interested
       in. */
    if ((v = adrof((uchar_t *)"term")) && (vp = (uchar_t **)v -> vec)) {
        uchar_t termbuf [1024];	    /* size required by tgetent() */
	uchar_t * area = areabuf;
	
	if (tgetent(termbuf,*vp) != 1) {    /* 1 == success */
	    csh_printf("\nWarning: no termcap entry for %s.  Editing disabled.\n",
		    *vp);
	    printprompt();
	    /* We are totally screwed if we don't know how to
	       backspace! */
	    BackSpaceSequence = area;
	    *area++ = CTRL_H;
	    LengthOfBackSpaceSequence = 1;
	    goto EndReadTermcap;
	}
	else {
	    TermKnown = TRUE;
	}
		
	BackSpaceSequence = area;
	if (tgetflag("bs")) {		/* backspace with ^H */
	    *area++ = CTRL_H;
	    LengthOfBackSpaceSequence = 1;
	}
	else {
	    tgetstr("bc",&area);	/* alternate backspace */
	    LengthOfBackSpaceSequence = strlen(BackSpaceSequence);
	    if (LengthOfBackSpaceSequence <= 0) {
		csh_printf("\nWarning: incomplete termcap entry.  Editing disabled.\n");
		printprompt();
		/* We are totally screwed if we don't know how to
		   backspace! */
	        TermKnown = FALSE;
		*area++ = CTRL_H;	/* use best guess */
		LengthOfBackSpaceSequence = 1;
	    }
	}
	
	ClearScreenSequence = area;
	tgetstr("cl",&area);		/* clear screen sequence */
	LengthOfClearScreenSequence = strlen(ClearScreenSequence);
	/* It doesn't matter if the clear screen sequence is null. */
	
	/* See how the user wants errors signaled.  The default action is
	   an audible bell.  The user can also choose to have either no
	   signal, a visible bell, or both a visible and an audible bell. */

	if (adrof((uchar_t *)"nobeep") || TermKnown == FALSE) {
	    AudibleBell = VisibleBell = FALSE;
	}
	else {
	    AudibleBell = TRUE;
	    if (VisibleBell = (adrof((uchar_t *)"visiblebell") ? TRUE : FALSE)) {
		VisibleBellSequence = area;
		tgetstr("vb",&area);	/* get visible bell sequence */
		LengthOfVisibleBellSequence = strlen(VisibleBellSequence);
		if (LengthOfVisibleBellSequence == 0) {
		    VisibleBell = FALSE;	/* tty can't do visible bell */
		}
		else {
		    AudibleBell = (adrof((uchar_t *)"audiblebell") ? TRUE : FALSE);
		}
	    }
	}
	
	StartInsertSequence = area;
	tgetstr("im",&area);		/* enter insert mode */
	LengthOfStartInsertSequence = strlen(StartInsertSequence);

	EndInsertSequence = area;
	tgetstr("ei",&area);		/* exit insert mode */
	LengthOfEndInsertSequence = strlen(EndInsertSequence);
	
	InsertPrefixSequence = area;
	tgetstr("ic",&area);		/* send before char insert */
	LengthOfInsertPrefixSequence = strlen(InsertPrefixSequence);
	
	InsertSuffixSequence = area;
	tgetstr("ip",&area);		/* send after char insert */
	LengthOfInsertSuffixSequence = strlen(InsertSuffixSequence);
	
	StartDeleteSequence = area;
	tgetstr("dm",&area);		/* enter delete mode */
	LengthOfStartDeleteSequence = strlen(StartDeleteSequence);
	
	EndDeleteSequence = area;
	tgetstr("ed",&area);		/* exit delete mode */
	LengthOfEndDeleteSequence = strlen(EndDeleteSequence);
	
	DeleteCharacterSequence = area;
	tgetstr("dc",&area);		/* delete one char in del mode */
	LengthOfDeleteCharacterSequence = strlen(DeleteCharacterSequence);

	if ((LengthOfScreen = tgetnum("li")) == -1)
	    LengthOfScreen = 1;
           
    }
    else {
        csh_printf("\nWarning: terminal type unknown.  Editing disabled.\n");
	printprompt();
	/* We are totally screwed if we don't know how to
	   backspace! */
	BackSpaceSequence = areabuf;
	*areabuf = CTRL_H;	/* use best guess */
	LengthOfBackSpaceSequence = 1;
    }

EndReadTermcap:;

    if (Disabled && TermKnown) {
        csh_printf("\nEditing enabled.\n");
	printprompt();
    }
    Disabled = TermKnown ? FALSE : TRUE;
}

static void
EnterCbreakMode(boolean intrbefore)
{
    struct tchars tmptty;

    /* Only save these if we weren't interrupted last time and thus
	    were able to restore the previous set */
    if (! intrbefore) {
	IOCTL(SHIN, TIOCGETC, &tmptty, "26");
	/* Interrupt can hit us in here, everything else can't */
	TenexTchars.t_intrc = TtyTchars.t_intrc = tmptty.t_intrc;
	TtyTchars.t_quitc = tmptty.t_quitc;
	TtyTchars.t_startc = tmptty.t_startc;
	TtyTchars.t_stopc = tmptty.t_stopc;
	TtyTchars.t_eofc = tmptty.t_eofc;
	TtyTchars.t_brkc = tmptty.t_brkc;

	/* save ltchars */
	IOCTL(SHIN, TIOCGLTC, &TtyLocalChars, "27");
    }
    IOCTL(SHIN, TIOCSETC, &TenexTchars, "28");
    IOCTL(SHIN, TIOCSLTC, &TenexLchars, "29");
    IOCTL(SHIN, TIOCGETP, &TtySgttyb, "30");
    ospeed = TtySgttyb.sg_ospeed;
    TtySgttyb.sg_flags |= (CBREAK | CRMOD);
    TtySgttyb.sg_flags &= ~(ECHO | RAW);
    IOCTL(SHIN, TIOCSETN, &TtySgttyb, "31");
}

static void
ExitCbreakMode(void)
{

    IOCTL(SHIN, TIOCSETC, &TtyTchars, "32");
    IOCTL(SHIN, TIOCSLTC, &TtyLocalChars, "33");

    TtySgttyb.sg_flags |= ECHO;
    TtySgttyb.sg_flags &= ~CBREAK;
    IOCTL(SHIN, TIOCSETN, &TtySgttyb, "34");
}

/*---------------------------------------------------------------------------
 * GetChar -- Except for pending input typed after a \n, this function
 * does all the input for the tenex package
 *---------------------------------------------------------------------------
 */
static int
GetChar (uchar_t *buf)
{
beginGetChar:
    if (ShellTypeAheadToTenex) {    /* from history expansion */
	InsertString( (uchar_t *)ShellTypeAheadToTenex, 
		(uchar_t *)(ShellTypeAheadToTenex + 
		strlen((char *)ShellTypeAheadToTenex)));
	ShellTypeAheadToTenex = 0;
    }
    if (MagicChar) {		    /* pushed back from repeat function */
        *buf = MagicPushBack;
	MagicChar = FALSE;
    }
    else if (MacroStart) {	    /* reading from user defined macro */
	if (MacroStart < MacroTail) {
	    *buf = *MacroStart++;
	}
	else {
	    PopMacro();
	    goto beginGetChar;
	}
    }
    else {			    /* interactive */
	int count;
	
	if ((count = READ (buf, 1))  <= 0 ) {
	    if (count == 0) {
	        SaveMacros();
		return 0;
	    }
	    else {
	        return 0;
	    }
	}
    }

    /* 001  */

    if (DefiningMacro && !MacroStart) {	  	/* exec macros called
						   while defining a macro,
						   but don't copy them */
	if (UnNamedMacro.Tail >= MacroFinal) {  /* MacroFinal is ptr to
						   end of unnamed macro
						   buffer */
	    MacroDefined = DefiningMacro = FALSE;
	    Beep();
	}
	*UnNamedMacro.Tail++ = *buf;
    }

    return 1;
}

/*---------------------------------------------------------------------------
 * Tenex -- this is the main routine
 *---------------------------------------------------------------------------
 */
int
ed_tenex (uchar_t *inputline, int inputlinesize)
{
    static boolean interrupted = FALSE;	   /* macros can be interrupted */
    register code chcode;		   /* code of last char read */
    boolean oi = interrupted;		   /* need to save this to pass to
					      EnterCbreakMode */
    
 /* initialize */

    InputBufferSize = inputlinesize;
    Mark = InputPos = InputTail = InputBuffer = inputline;
    RepetitionCount = 1;
    Bindings = OldBindings = RegBindings;
    curHistPos = 0;
    ControlCharsInInput = FALSE;

    if (! Initialized) {
	struct varent * v = adrof((uchar_t *)"editmode");
	uchar_t * ed = 0;
	uchar_t ** vp;

	if (v && (vp = (uchar_t **)(v -> vec))) ed = *vp;
		
	SaveTtyState();
	ReadTermcap();
	Initialized = TRUE;
	dobindings( ed);
        InitBuffers();
    }
    
    if (interrupted) {
	MacroStart = 0, MacroStackDepth = 0;
    }
    else {
        interrupted = TRUE;
    }
    
    EnterCbreakMode( oi);

    loop {
	if (GetChar (&CurrentChar) == 0) {
	    chcode = XEOF;
	    goto endloop;
	}
	
	switch ( chcode = (*Bindings[CurrentChar].Routine)() ) {
	    case RET:
	        goto endloop;
	    case CNX:
	    case ESC:
 	    case ANSI:
	        break;
	    case OVERFLOW:
		ExitCbreakMode();
		return inputlinesize;
	    case DUMB:
	    case INS:
		RepetitionCount = 1;
		break;
	    case REP:
		Bindings = RegBindings;
	        break;
	    case EMACS:
		Bindings = RegBindings;
		RepetitionCount = 1;
		break;
	    case VI:
		Bindings = OldBindings;
		RepetitionCount = 1;
	        break;
	    case XEOF:
	        goto endloop;
	    default:
	        break;
	}
    }

endloop:;
    ExitCbreakMode();
    if (chcode != XEOF && InputTail - InputBuffer)
	NewLine();
    interrupted = FALSE;
    return (InputTail - InputBuffer);
}

/*---------------------------------------------------------------------------
 * The following functions simply switch to one of the alternate keymaps.
 *---------------------------------------------------------------------------
 */
static code
SetCtrlX (void)
{
    Bindings = CnxBindings;
    return CNX;
}

static code
SetEscape (void)
{
    OldBindings = Bindings;
    Bindings = EscBindings;
    return ESC;
}

static code
SetAnsi (void)
{
    Bindings = AnsiBindings;
    return ANSI;
}


static code
EnterViInsert (void)
{
    if (Editor == VI) {
	Bindings = OldBindings = RegBindings;
	return INS;
    }
    return DEF;
}

static code
EnterViAppend (void)
{
    if (Editor == VI) {
	if (InputPos < InputTail)
	    MoveNChars(1, FORW);
	Bindings = OldBindings = RegBindings;
	return INS;
    }
    return DEF;
}

static code
ExitViInsert (void)
{
    if (Editor == VI) {
	Bindings = OldBindings = CnxBindings;
	return INS;
    }
    return DEF;
}

static code
Return (void)
{
    EndOfLine();
    *InputTail++ = '\n';
    return RET;
}

static code
EndOfFile (void)
{
    return XEOF;
}

/*---------------------------------------------------------------------------
 * Reads an emacs like repetition factor or sets the default factor 
 * (4 * the previous repetition factor)
 *---------------------------------------------------------------------------
 */
static code
Repetition (void)
{
    long n;
    uchar_t buf;

    if (Editor == VI) {
        buf = CurrentChar;
    }
    else {
	GetChar (&buf);
    
	if (! ISDIGIT(buf)) {
	    MagicPushBack = buf;	  /* push back char */
	    MagicChar = TRUE;
	    n = RepetitionCount * 4;
	    if (n > MAXINT) {
	        Beep();
		return DEF;
	    }
	    RepetitionCount = n;
	    return REP;
	}
    }

    n = buf - '0';

    loop {
	GetChar (&buf);
	
	if (ISDIGIT(buf)) {
	    n = n * 10 + (buf - '0');
	    if (n > MAXINT) {
	        Beep();
		return DEF;
	    }
	}
	else {
	    MagicPushBack = buf;	  /* push back char */
	    MagicChar = TRUE;
	    break;
	}
    }

    RepetitionCount = n;
    return REP;
}

/*---------------------------------------------------------------------------
 * InsertChar -- put a char into the input buffer and echo it on the
 * user's terminal.
 *
 * This function is complicated by the possibilities for overflow of the
 * input buffer.  No matter where the cursor is on the line, if doing the
 * full insertion would overflow the buffer, then we just scream at the
 * user and return.  If the cursor is at the end of the line and inserting
 * the chars will exactly fill the buffer, then we insert them and tell
 * the shell to give us a new buffer.  This can't be done if the cursor
 * isn't at the end of the line since it would make the position of the
 * cursor incorrect.  Most of the time there is plenty of room; so we just
 * insert the chars and return to the user for more input.
 *
 * Note: the buffer is returned to the shell when it fills, not when it
 * overflows; so the Return function can put a \n in the buffer without
 * worrying about overflow.
 *---------------------------------------------------------------------------
 */
static code
InsertChar (void)
{
    register int num, repetition = RepetitionCount;
    register uchar_t * c, * d;
    register uchar_t * oldPos = InputPos;
    register uchar_t * final = InputBuffer + InputBufferSize;
    
    if (CurrentChar < SPACE) {
        ControlCharsInInput = TRUE;
    }
    
    /* In middle of line. */
    if (InputTail > InputPos) {

	/* Cannot allow input to either fill or overflow buffer! */
	if ( repetition >= final - InputTail ) {
	    Beep();
	    return DEF;
	}
    
	/* Shift tail of line to make room for insertion. */
	c = InputTail - 1;
	d = c + repetition;
	
	for (; c >= InputPos; c--, d--) {
	    *d = *c;
	}

	/* Put chars in input buffer. */
	InputTail += repetition;
	while (repetition-- > 0) {
	    *InputPos = CurrentChar;
	    InputPos++;
	}
	WriteToScreen (oldPos, InputTail - oldPos);
	Backup (InputPos, InputTail - InputPos);
    }
    else {
	/* Cannot allow input to suddenly overflow input buffer! */
	if ( repetition > final - InputTail ) {
	    Beep();
	    return DEF;
	}
    
	InputTail += repetition;
	while (repetition-- > 0) {
	    *InputPos = CurrentChar;
	    InputPos++;
	} 
	WriteToScreen (oldPos, InputTail - oldPos);
    }

    if (InputTail == final) {
        return OVERFLOW;
    }
    
    return INS;
}

static code
InsertLiteralChar (void)
{
    GetChar (&CurrentChar);
    return InsertChar();
}

static int
DoInsertChar (void)
{
    if (InputTail + RepetitionCount >= InputBuffer + InputBufferSize) {
        return(0);
    }
    InsertChar ();
    return(1);
}

static int
InsertString (uchar_t *head, register uchar_t *tail)
{
    register uchar_t * start;
    register int    times;
    register int    count;
    
    times = count = RepetitionCount;
    
    if (head < tail) {
	/* Must not allow input buffer to overflow on InsertChar when
	   InsertChar won't have a chance to return a full buffer to the
	   shell! */
	if ((InputTail + (tail - head)) >= (InputBuffer + InputBufferSize)) {
	    return 0;
	}
	
	RepetitionCount = 1;

	for (; times--;) {
	    for (start = head; start < tail; start++) {
	        CurrentChar = *start;                           /* 001 */
		InsertChar ();
	    }
	}

	RepetitionCount = count;
    }

    return 1;
}

static code
DefaultBinding (void)
{
    Beep();
    return DEF;
}

static code
Ignore (void)
{
    return DEF;
}

#define TAB_LENGTH 8

static code
Tab (void)
{
    register int n, repetition = RepetitionCount;
    register uchar_t * newPos;
    
    n = (InputPos - InputBuffer) / TAB_LENGTH;
    newPos = InputBuffer + (n + 1) * TAB_LENGTH;
    if (repetition > 1) 
	newPos += (repetition - 1) * TAB_LENGTH;

    CurrentChar = ' ';
    RepetitionCount = newPos - InputPos;
    if (! DoInsertChar ()) Beep();
    return DEF;
}

/*---------------------------------------------------------------------------
 * Redisplay -- redraw the current command line on the user's terminal
 *---------------------------------------------------------------------------
 */
static code
Redisplay (void)
{
    Crlf();
    WriteToScreen (InputBuffer, InputTail - InputBuffer);
    Backup (InputPos, InputTail - InputPos);
    
    return DEF;
}

static void
Crlf (void)
{
    static uchar_t * crlf = (uchar_t *)"\r\n";
    write (SHOUT, crlf, 2);
}

static void
NewLine (void)
{
    uchar_t nl = LINEFEED;
    write (SHOUT, &nl, 1);
}

static code
ClearScreen (void)
{
    if (LengthOfClearScreenSequence) {
/*	write (SHOUT, ClearScreenSequence, LengthOfClearScreenSequence); */
	tputs ( ClearScreenSequence, LengthOfScreen, ShoutItOut);
	printprompt();
	WriteToScreen (InputBuffer, InputTail - InputBuffer);
	Backup (InputPos, InputTail - InputPos);
    }
    else {
        Beep();
    }
    return DEF;
}

static void
Beep (void)
{
    if (VisibleBell) {
/*	write (SHOUT, VisibleBellSequence, LengthOfVisibleBellSequence); */
	tputs ( VisibleBellSequence, LengthOfScreen, ShoutItOut);
    }
    if (AudibleBell) {
	char beep = CTRL_G;
	write (SHOUT, &beep, 1);
    }
}

/*---------------------------------------------------------------------------
 * TransposeChars -- swap the two chars preceding the cursor
 *---------------------------------------------------------------------------
 */
static code
TransposeChars (void)
{
    uchar_t temp;
    
    if (InputPos - InputBuffer >= 2) {
	temp = *(InputPos - 2);
        *(InputPos - 2) = *(InputPos - 1);
	*(InputPos - 1) = temp;
	Backup (InputPos - 2, 2);
	WriteToScreen (InputPos - 2, 2);
    }
    else {
        Beep();
    }

    return DEF;
}

/*---------------------------------------------------------------------------
 * GnuTransposeChars -- swap the two chars on either side of cursor
 *                      Semantics similar to Stallman's Emacs
 *---------------------------------------------------------------------------
 */

static code
GnuTransposeChars (void)
{
    uchar_t temp;
    int disp;
    
    if (InputPos > InputBuffer) {

        if (InputTail == InputPos) disp = 2; /* at end of line */
	else disp = 1;

	temp = *(InputPos - disp);
	*(InputPos - disp) = *(InputPos - (disp - 1));
	*(InputPos - (disp - 1)) = temp;
	Backup (InputPos - disp, disp);
	WriteToScreen (InputPos - disp, disp);
	MoveNChars(2 - disp, FORW);
    }
    else {
        Beep();
    }

    return DEF;
}


static boolean
IsSpaceChar (uchar_t *pos)
{
    return (SpaceChar[*pos] ? TRUE : FALSE);
}

/*---------------------------------------------------------------------------
 * NextWordPos -- returns a pointer to the end of the word following the
 * cursor.  If the cursor is in the middle of a word a pointer to its end
 * is returned.
 *---------------------------------------------------------------------------
 */
static uchar_t *
NextWordPos (uchar_t *cur)
{
    register uchar_t * temp = cur;

    /* skip over spaces */
    for (;temp < InputTail && IsSpaceChar(temp); temp++);
    
    /* don't go past end of last word */
    if (temp == InputTail) {
        return cur;
    }
    
    /* skip over next word */
    for (; temp < InputTail && !IsSpaceChar (temp); temp++);

    return temp;
}

/*---------------------------------------------------------------------------
 * PrevWordPos -- returns a pointer to the beginning of the previous word.
 * If the cursor is in the middle of a word a pointer to the beginning of
 * it will be returned.
 *---------------------------------------------------------------------------
 */
static uchar_t *
PrevWordPos (uchar_t *cur)
{
    register uchar_t * temp = cur;
    
    /* skip over preceding spaces */
    for (;temp > InputBuffer && IsSpaceChar(temp-1); temp--);

    /* don't go past beginning of first word */
    if (temp == InputBuffer) {
        return(cur);
    }
    
    /* skip over preceding word */
    for (; temp > InputBuffer && !IsSpaceChar(temp-1); temp--);

    return(temp);
}

/*---------------------------------------------------------------------------
 * DeleteNChars -- deletes a specified number of chars from the input
 * buffer either backwards or forwards from the current position.
 *---------------------------------------------------------------------------
 */
static void
DeleteNChars (int num, direction dir)
{
    register uchar_t * newcurrent, * temp;
    
    if (dir == FORW) {
	newcurrent = InputPos + num;

        WriteToScreen (newcurrent, InputTail - newcurrent);
	BlotOut (InputPos, num);
	Backup (InputPos, InputTail - InputPos);
	
	for (temp = InputPos; newcurrent < InputTail; temp++, newcurrent++) {
	    *temp = *newcurrent;
	}
	InputTail -= num;
    }
    else if (dir == BACK) {
        newcurrent = InputPos - num;
	
	Backup (newcurrent, num);
	WriteToScreen (InputPos, InputTail - InputPos);
	BlotOut (newcurrent, num);
	Backup (newcurrent, InputTail - newcurrent);
	
	for (temp = newcurrent; InputPos < InputTail; temp++, InputPos++) {
	    *temp = *InputPos;
	}
	InputTail -= num;
	InputPos = newcurrent;
    }
}

/*---------------------------------------------------------------------------
 * DeleteWord -- delete from the cursor to the end of the following word
 *---------------------------------------------------------------------------
 */
static code
DeleteWord (void)
{
    register uchar_t  *temp;
    register int    num;

    temp = InputPos;

    do {
	temp = NextWordPos (temp);
    } while (--RepetitionCount > 0);

    if (num = (temp - InputPos)) {
	if (Editor == VI) {
	    CopyNChars (KillBuffer, InputPos, num);
	    KillTail = KillBuffer + num;
	}
	DeleteNChars (num, FORW);
    }
    else {
	Beep ();
    }

    return DEF;
}

/*---------------------------------------------------------------------------
 * EraseWord -- delete from the cursor to the beginning of the previous
 * word
 *---------------------------------------------------------------------------
 */
static code
EraseWord (void)
{
    register uchar_t  *temp;
    register int    num;

    temp = InputPos;

    do {
	temp = PrevWordPos (temp);
    } while (--RepetitionCount > 0);

    if (num = (InputPos - temp)) {
	if (Editor == VI) {
	    CopyNChars (KillBuffer, temp, num);
	    KillTail = KillBuffer + num;
	}
	DeleteNChars (num, BACK);
    }
    else {
	Beep ();
    }

    return DEF;
}

/*---------------------------------------------------------------------------
 * The following two functions delete a single char either following or
 * preceding the current position.
 *---------------------------------------------------------------------------
 */
static code
DeleteCurrentChar (void)
{
    register int num;
    
    num = MIN(InputTail - InputPos, RepetitionCount);
    if (num) {
	if (num < RepetitionCount) {
	    Beep();
	}
	if (Editor == VI) {
	    CopyNChars (KillBuffer, InputPos, num);
	    KillTail = KillBuffer + num;
	}
	DeleteNChars (num, FORW);
    }
    else {
        Beep();
    }

    return DEF;
}

static code
DeletePreviousChar (void)
{
    register int num;
    
    num = MIN(InputPos - InputBuffer, RepetitionCount);
    if (num) {
	if (num < RepetitionCount) {
	    Beep();
	}
	if (Editor == VI) {
	    CopyNChars (KillBuffer, InputPos - num, num);
	    KillTail = KillBuffer + num;
	}
	DeleteNChars (num, BACK);
    }
    else {
        Beep();
    }

    return DEF;
}

/*
 * End of file if the line is empty, else delete next char
 */
static code 
DeleteCurrentCharOrEof (void)
{
    if (CurrentChar == TtyTchars.t_eofc && InputTail == InputBuffer) {
	return EndOfFile();
    }
    else {
	return DeleteCurrentChar();
    }
}

static void
CopyNChars (uchar_t *dest, uchar_t *src, register int num)
{
    register int i;
    
    for (i = 0; i < num; i++) {
        *dest++ = *src++;
    }
}

/*---------------------------------------------------------------------------
 * Kill buffer code
 *---------------------------------------------------------------------------
 */
static code
SetMark (void)
{
    Mark = InputPos;
    return DEF;
}

static code
KillRegion (void)
{
    register int len;
    register uchar_t *pos;
    
    if (! Mark) {
        Beep();
    }
    else if (InputPos > Mark) {
	len = InputPos - Mark;
	CopyNChars (KillBuffer, Mark, len);
        DeleteNChars (len, BACK);
	KillTail = KillBuffer + len;
    }
    else if (InputPos < Mark) {
	len = Mark - InputPos;
	CopyNChars (KillBuffer, InputPos, len);
        DeleteNChars (len, FORW);
	Mark = InputPos;
	KillTail = KillBuffer + len;
    }
    else {
        Beep();
    }
    
    return DEF;
}

static code
KillToEOL (void)
{
    register int temp;
    
    if (temp = InputTail - InputPos) {
	CopyNChars (KillBuffer, InputPos, temp);
	DeleteNChars (temp, FORW);
    	KillTail = KillBuffer + temp;
    }

    return DEF;
}

static code
EraseLine (void)
{
    BeginningOfLine();
    return KillToEOL();
}

static code
YankKillBuffer (void)
{
    if (KillBuffer >= KillTail || ! InsertString( KillBuffer, KillTail)) {
        Beep();
    }
    return DEF;
}

static code
WipeLine (void)
{
    register int temp;

    BeginningOfLine();
    if (temp = InputTail - InputPos) {
	DeleteNChars (temp, FORW);
    }
    return DEF;
}

/*---------------------------------------------------------------------------
 * The following functions just move the cursor around in the input buffer.
 *---------------------------------------------------------------------------
 */
static void
MoveNChars (int num, direction dir)
{
    if (dir == FORW) {
        WriteToScreen (InputPos, num);
	InputPos += num;	
    }
    else if (dir == BACK) {
	InputPos -= num;
        Backup (InputPos, num);
    }
}

static code
ForwardChar (void)
{
    register int num;

    num = MIN(InputTail - InputPos, RepetitionCount);
        
    if (num) {
	MoveNChars (num, FORW);
    }
    else {
        Beep();
    }

    return DEF;
}

static code
BeginningOfLine (void)
{
    register int num;
    
    if (num = (InputPos - InputBuffer)) {
	MoveNChars (num, BACK);
    }

    return DEF;
}

static code
EndOfLine (void)
{
    register int num;
    
    if (num = (InputTail - InputPos)) {
	MoveNChars (InputTail - InputPos, FORW);
    }

    return DEF;
}

static code
ForwardWord (void)
{
    register uchar_t  *temp, *prev;
    register int    num;

    temp = InputPos;

    do {			/* locate new cursor position */
	prev = temp;
	temp = NextWordPos (prev);
	if (temp == prev) {
	    break;
	}
    } while (--RepetitionCount > 0);

    if (num = (temp - InputPos)) {
	MoveNChars (num, FORW);
    }

    return DEF;
}

static code
BackwardWord (void)
{
    register uchar_t  *temp, *prev;
    register int    num;

    temp = InputPos;

    do {			/* locate new cursor position */
	prev = temp;
	temp = PrevWordPos (prev);
	if (temp == prev) {
	    break;
	}
    } while (--RepetitionCount > 0);

    if (num = (InputPos - temp)) {
	MoveNChars (num, BACK);
    }

    return DEF;
}

static code
BackSpace (void)
{
    register int num;
        
    num = MIN(InputPos - InputBuffer, RepetitionCount);

    if (num) {
	MoveNChars (num, BACK);
    }
    else {
        Beep();
    }

    return DEF;
}

static void
InitBuffers(void)
{
    register struct varent *v = adrof((uchar_t *)"breakchars");
    register unsigned char **vp, *p;
    register int    i;

    for (i = 0; i < MAXSPACES; i++) {
	SpaceChar[i] = 0;
    }

    if (v == 0 || (vp = v -> vec) == 0) {
	SpaceChar[SPACE] = 1;
	SpaceChar['\t'] = 1;
	SpaceChar['/'] = 1;
	SpaceChar['\\'] = 1;
	SpaceChar['('] = 1;
	SpaceChar[')'] = 1;
	SpaceChar['['] = 1;
	SpaceChar[']'] = 1;
	SpaceChar['{'] = 1;
	SpaceChar['}'] = 1;
	SpaceChar['.'] = 1;
	SpaceChar[','] = 1;
	SpaceChar[';'] = 1;
	SpaceChar['>'] = 1;
	SpaceChar['<'] = 1;
	SpaceChar['!'] = 1;
	SpaceChar['^'] = 1;
	SpaceChar['&'] = 1;
	SpaceChar['|'] = 1;
    }
    else {
	for (p = *vp; *p; p++) {
	    SpaceChar[*p] = 1;
	}
    }

    if (MacroBuffer == (uchar_t *) NULL)
    	MacroBuffer = (uchar_t *)malloc(BUFSIZ + 2);
    MacroFinal  = MacroBuffer + BUFSIZ + 2;

    if (KillBuffer == (uchar_t *) NULL)
    	KillBuffer = (uchar_t *)malloc(BUFSIZ);
    KillTail   = KillBuffer;

    if (SpaceBuffer == (uchar_t *) NULL)
    	SpaceBuffer = (uchar_t *)malloc(BUFSIZ);
    for (i = 0; i < BUFSIZ; i++) 
	*(SpaceBuffer + i) = SPACE;
    
    if (LengthOfBackSpaceSequence == 1) {
    	if (BackSpaceBuffer == (uchar_t *) NULL)
		BackSpaceBuffer = (uchar_t *)malloc(BUFSIZ);
	for (i = 0; i < BUFSIZ; i++) 
	    *(BackSpaceBuffer + i) = *BackSpaceSequence;
    }
}

/*---------------------------------------------------------------------------
 * Tenex like filename expansion and listing of files matching a prefix
 *---------------------------------------------------------------------------
 */
static code
FilenameExpansion (void)
{
    extern char * index();
    static uchar_t *delims = (uchar_t *)" '\"\t;&<>()|^%";
    register uchar_t  *str_end, *word_start;
    register int    space_left, numitems, not_eol;

    *InputTail = '\0';
    str_end = InputTail;
 /* 
  * Find LAST occurence of a delimiter in the inputline.
  * The word start is one uchar_tacter past it.
  */
    for (word_start = str_end; word_start > InputBuffer; --word_start)
	if (index (delims, word_start[-1]))
	    break;

    space_left = InputBufferSize - (word_start - InputBuffer) - 1;
    numitems = tenex_search ((uchar_t *)word_start, RECOGNIZE, space_left);
    InputTail += strlen(word_start) - (str_end - word_start);
    WriteToScreen (InputPos, InputTail - InputPos);
    InputPos = InputTail;

    if (numitems != 1) {
        Beep();
    }

    return DEF;
}

static code
FilenameList (void)
{
    static uchar_t *delims = (uchar_t *)" '\"\t;&<>()|^%";
    register uchar_t  *str_end, *word_start, last_char, should_retype;
    register int    space_left, numitems;

    *InputTail = '\0';
    str_end = InputTail;
 /* 
  * Find LAST occurence of a delimiter in the inputline.
  * The word start is one character past it.
  */
    for (word_start = str_end; word_start > InputBuffer; --word_start)
	if (index (delims, word_start[-1]))
	    break;

    space_left = InputBufferSize - (word_start - InputBuffer) - 1;

    Crlf();
    numitems = tenex_search ((uchar_t *)word_start, LIST, space_left);
    printprompt();
    WriteToScreen (InputBuffer, InputTail - InputBuffer);
    Backup (InputPos, InputTail - InputPos);

    return DEF;
}

static code
FilenameListOrEof (void)
{
    if (CurrentChar == TtyTchars.t_eofc && InputTail == InputBuffer) {
	return EndOfFile();
    }
    else {
	return FilenameList();
    }
}

/*---------------------------------------------------------------------------
 * Code for Keyboard Macros
 *---------------------------------------------------------------------------
 */
#define MACRO_STACK_INCREMENT 10
#define MACRO_ABSURDITY 128

static boolean
PushMacro (String mac)
{
    register int i;
    
    if (MacroStackDepth >= MacroStackLimit) {
	MacroStackLimit += MACRO_STACK_INCREMENT;
	if (MacroStack) {
	    MacroStack = (String *) realloc( MacroStack,
		    MacroStackLimit * sizeof( String));
	}
	else {
	    MacroStack = (String *) malloc( MacroStackLimit * sizeof( String));
	}
	if (! MacroStack) {
	    csh_printf("\nout of memory\n");
	    if (MacroStackLimit > MACRO_STACK_INCREMENT) {
	        MacroStackLimit = MACRO_STACK_INCREMENT;
		MacroStack = (String *) malloc( MacroStackLimit * 
			sizeof( String));
		if (! MacroStack) {
		    MacroStackLimit = 0;
		}
	    }
	    MacroStart = 0, MacroStackDepth = 0;
	    return FALSE;
	}
    }
    if (MacroStart) {
	MacroStack[MacroStackDepth].Head = MacroStart;
	MacroStack[MacroStackDepth++].Tail = MacroTail;
    }

    MacroStart = mac.Head;
    MacroTail = mac.Tail;

    /* Don't start a macro that is already on the stack. */
    for (i = 0; i < MacroStackDepth; i++) {
        if (MacroStart <= MacroStack[i].Head && 
		MacroStack[i].Head < MacroTail) {
	    MacroStart = 0, MacroStackDepth = 0;
	    MacroStackLimit = MACRO_STACK_INCREMENT;
	    free( MacroStack);
	    MacroStack = (String *) malloc( MacroStackLimit * sizeof( String));
	    if (! MacroStack) {
	        csh_printf("\nout of memory\n");
		MacroStackLimit = 0;
	    }
	    return FALSE;
	}
    }

    return TRUE;
}

static void
PopMacro (void)
{
    if (--MacroStackDepth < 0) {
	MacroStart = 0, MacroStackDepth = 0;
    }
    else {
        MacroStart = MacroStack[MacroStackDepth].Head;
	MacroTail = MacroStack[MacroStackDepth].Tail;
    }
}

static code
ExecuteMacro (void)
{
    uchar_t buf = Bindings[CurrentChar].MacroName;
    
    if (! KeyboardMacro[buf].Head || ! PushMacro( KeyboardMacro[buf])) {
	Beep();
    }

    return DEF;
}

static code
ExecuteNamedMacro (void)
{
    uchar_t buf;
    
    GetChar (&buf);
    
    if (! KeyboardMacro[buf].Head || ! PushMacro( KeyboardMacro[buf])) {
	Beep();
    }

    return DEF;
}

static code
ExecuteUnNamedMacro (void)
{
    if (! MacroDefined || ! PushMacro( UnNamedMacro)) {
	Beep();
    }
    
    return DEF;
}

static code
StartRemembering (void)
{
    if (DefiningMacro) {
	MacroDefined = DefiningMacro = FALSE;
        Beep();
    }
    else {
	DefiningMacro = TRUE;
	MacroDefined = FALSE;
    }
    UnNamedMacro.Head = MacroBuffer;
    UnNamedMacro.Tail = MacroBuffer;
    return DEF;
}

static code
StopRemembering (void)
{
    if (DefiningMacro) {
	/* remove macro closing chars from tail of macro */
	UnNamedMacro.Tail--;
	if (Bindings != RegBindings) UnNamedMacro.Tail--;

	/* ignore null macros; otherwise DefineNamedMacro will fail with
	   malloc(0) */
        if (UnNamedMacro.Tail - UnNamedMacro.Head > 0) {
	    MacroDefined = TRUE;
	}
	DefiningMacro = FALSE;
    }
    else {
        Beep();
    }

    return DEF;
}

static code
DefineNamedMacro (void)
{
    register int len;
    uchar_t buf;
    
    GetChar (&buf);
    
    if (KeyboardMacro[buf].Head) {
	free(KeyboardMacro[buf].Head);
    }

    if (DefiningMacro || !MacroDefined) {
	KeyboardMacro[buf].Head = 0;
        Beep();
    }
    else {
	if (!(KeyboardMacro[buf].Head = 
	    (uchar_t *)malloc (len = UnNamedMacro.Tail - UnNamedMacro.Head))) {
	    Beep();
	}
	else {
	    CopyNChars (KeyboardMacro[buf].Head, UnNamedMacro.Head, len);
	    KeyboardMacro[buf].Tail = KeyboardMacro[buf].Head + len;
	}
    }
    
    return DEF;
}

static int
WriteMacroFile(uchar_t *fname)
{
  int fd;

  fd = open( fname, (O_WRONLY|O_CREAT|O_TRUNC), (S_IRUSR|S_IWUSR));
  if (fd != -1) {
    unsigned char curmacro;
    unsigned char blen[4];	/* sizeof(len) */
    uchar_t *iter;
    unsigned int len;

    for (curmacro = 0; curmacro < BINDINGS; curmacro++) {
      if (iter = KeyboardMacro[ curmacro].Head) {
	len = (KeyboardMacro[ curmacro].Tail-iter);
	blen[0] = len & 0xff;
	blen[1] = (len >> 8) & 0xff;
	blen[2] = (len >> 16) & 0xff;
	blen[3] = (len >> 24) & 0xff;
	write( fd, (char *)&curmacro, 1);
	write( fd, (char *)blen, 4);
	write( fd, iter, len);
	write( fd, "\n", 1);	 /* just for readability */
      }
    }
    close(fd);
  }
  return fd;
}

static int
ReadMacroFile(uchar_t *fname)
{
  int fd = open( fname, (int)O_RDONLY);
  if (fd != -1) {
    unsigned int size = lseek( fd, 0, SEEK_END); /*Seek to end/get size*/
    lseek( fd, 0, SEEK_SET);			 /*Back to beginning to read*/

    if (size) {
      uchar_t *filbuf, *end, *curpos;
      unsigned char curmacro;		/* the macro we are defining */
      unsigned int macrosize;		/* and its size */

      filbuf = (uchar_t *)malloc( size);
      if (!filbuf) {			/* malloc failure */
	close( fd);
	return -1;
      }
      end = filbuf + size;
      curpos = filbuf;

      read( fd, filbuf, size);		/* vread is known not to work here! */

      while (curpos < end) {
	curmacro = *curpos++;		/* extract the macro name */
	macrosize = *curpos++;
	macrosize |= *curpos++ << 8;
	macrosize |= *curpos++ << 16;
	macrosize |= *curpos++ << 24;

	if ((macrosize<= 0) || (macrosize > 1024)) break;

#define CurMac KeyboardMacro[ curmacro]
	if (CurMac.Head) free( CurMac.Head);  /* reclaim existing macro */

	if (!(CurMac.Tail = CurMac.Head = (uchar_t *)malloc( macrosize))) break;

	if (curpos + macrosize >= end) break; /* not past eof we don't */

	while (macrosize-- > 0) *(CurMac.Tail++) = *curpos++;
	/* Copy macro, this leaves Tail in the 'right' place */
#undef CurMac

	while ( (curpos < end) && (*curpos++ != '\n') );
	/* Scan for trailing newline... */
      }
      free( filbuf);
    }
    close( fd);
  }
  return fd;
}

static code
SaveMacroFile(void)
{
  struct stat statbuf;

  *InputTail = '\0';
  if (stat( InputBuffer, &statbuf) == 0) { /* Don't overwrite existing files */
    uchar_t * errmsg = (uchar_t *)" [ File already exists ]";
    InsertString( errmsg, errmsg+strlen(errmsg));
    Beep();
    return DEF;
  }
  if (WriteMacroFile( InputBuffer) == -1)
    Beep();
  else {
    BeginningOfLine();
    KillToEOL();
  }
  return DEF;
}

static code
LoadMacroFile(void)
{
  *InputTail = '\0';
  if (ReadMacroFile( InputBuffer) == -1)
    Beep();
  else {
    BeginningOfLine();
    KillToEOL();
  }
  return DEF;
}


/*---------------------------------------------------------------------------
 * Incremental Search
 *---------------------------------------------------------------------------
 */

/* string_index returns a pointer to the first instance of 'chr' in 'str',
    or 0 if none is found */
static uchar_t *
string_index (String str, uchar_t chr)
{
    register uchar_t *p;
    
    for (p = str.Head; p < str.Tail; p++) {
        if (*p == chr) return p;
    }
    return 0;
}

/* string_rindex returns a pointer to the last instance of 'chr' in 'str',
   or 0 if none is found */
static uchar_t *
string_rindex (String str, uchar_t chr)
{
    register uchar_t *p;
    
    for (p = str.Tail - 1; p >= str.Head; p--) {
        if (*p == chr) return p;
    }
    return 0;
}
/* returns true iff the string defined by (s1,e1) is identical to the
   string defined by (s2,e2).  The 's' pointer points to the first char of
   the string, while the 'e' pointer points to the char after the last
   char of the string. */
static boolean
Match(register uchar_t *s1,register uchar_t *e1,register uchar_t *s2,register uchar_t *e2)
{
    if ((e1 - s1) != (e2 - s2)) return FALSE;

    for (; s1 < e1 && *s1 == *s2; s1++, s2++);

    if (s1 < e1) return FALSE;
    else return TRUE;    
}

/* returns a pointer to the first char of the first instance of 'search'
   in 'input' */
static uchar_t *
Index (String input, String search)
{
    int searchlength;
    uchar_t lastchar;
    uchar_t * lastpos, * firstpos;
    String tempstring;
    
    searchlength = search.Tail - search.Head;
    if (searchlength == 0) return input.Head;
    lastchar = search.Tail[-1];

    tempstring.Tail = input.Tail;

    loop {    
	tempstring.Head = input.Head + searchlength - 1;
	if (tempstring.Head >= tempstring.Tail) return 0;

	if ((lastpos = string_index(tempstring,lastchar)) == 0) return 0;
        
	firstpos = lastpos - searchlength + 1;
    
	if (Match(firstpos,lastpos,search.Head,search.Tail-1)) 
	    return firstpos;

	input.Head = firstpos + 1;
    }
}

/* returns a pointer to the first char of the last instance 'search' in
   'input' */
static uchar_t *
ReverseIndex (String input, String search)
{
    int searchlength;
    uchar_t lastchar;
    uchar_t *lastpos, *firstpos;
    
    searchlength = search.Tail - search.Head;
    if (searchlength == 0) return input.Head;
    lastchar = search.Tail[-1];

    loop {
	if ((lastpos = string_rindex(input,lastchar)) == 0) return 0;
	
	firstpos = lastpos - searchlength + 1;

	if (Match(firstpos,lastpos,search.Head,search.Tail-1))
	    return firstpos;
	
	input.Tail = lastpos;
    }
}

static code
IncrementalSearchForward (void)
{
    uchar_t searchbuffer [256];
    String searchstring;
    String inputstring, oldinputstring;
    int movement;
    uchar_t *ptr;

    inputstring.Head = InputPos;
    inputstring.Tail = InputTail;
    oldinputstring = inputstring;
    searchstring.Head = searchstring.Tail = searchbuffer;

    GetChar (searchstring.Tail);
    
    while (*(searchstring.Tail) != ESCAPE) {
	if (*(searchstring.Tail) != CTRL_S) {
	    searchstring.Tail++;
	}
	else {
	    oldinputstring = inputstring;
	    inputstring.Head = (uchar_t *)
		MIN(
		  (int)(InputPos - (searchstring.Tail - searchstring.Head) + 1),
		  (int)(InputTail));
	}
			
	if (ptr = Index (inputstring, searchstring)) {
	    inputstring.Head = ptr;
	    movement = (ptr + (searchstring.Tail - searchstring.Head)) 
			- InputPos;
	    MoveNChars (movement, FORW);
	}
	else {
	    inputstring = oldinputstring;
	    searchstring.Tail--;
	    Beep();
	}

	if (searchstring.Tail > searchbuffer + 255) {
	    Beep();
	    return DEF;
	}
	
        GetChar (searchstring.Tail);
    }
    
    return DEF;
}

static code
IncrementalSearchReverse (void)
{
    uchar_t searchbuffer [256];
    String searchstring;
    String inputstring, oldinputstring;
    int movement;
    uchar_t *ptr;

    inputstring.Head = InputBuffer;
    inputstring.Tail = InputPos;
    oldinputstring = inputstring;
    searchstring.Head = searchstring.Tail = searchbuffer;

    GetChar (searchstring.Tail);
    
    while (*(searchstring.Tail) != ESCAPE) {
	if (*(searchstring.Tail) != CTRL_R) {
	    searchstring.Tail++;
	}
	else {
	    oldinputstring = inputstring;
	    inputstring.Tail = (uchar_t *)
		MAX(
		  (int)(InputPos + (searchstring.Tail - searchstring.Head) - 1),
		  (int)(InputBuffer));
	}
		
	if (ptr = ReverseIndex (inputstring, searchstring)) {
	    movement = InputPos - ptr;
	    MoveNChars (movement, BACK);
	}
	else {
	    inputstring = oldinputstring;
	    searchstring.Tail--;
	    Beep();
	}

	if (searchstring.Tail > searchbuffer + 255) {
	    Beep();
	    return DEF;
	}
	
        GetChar (searchstring.Tail);
    }
    
    return DEF;
}

static code
SearchForward(void)
{
    uchar_t lookfor, * found;
    
    *InputTail = '\0';
    GetChar( &lookfor);
    
    if (found = (uchar_t *)index( InputPos, lookfor)) {
        MoveNChars( found - InputPos + 1, FORW);
    }
    else {
        Beep();
    }
    return DEF;
}

static code
SearchReverse(void)
{
    extern char * rindex();
    uchar_t lookfor, save, * found;
    
    GetChar( &lookfor);

    save = *InputPos;
    *InputPos = '\0';
    
    if (found = (uchar_t *)rindex( InputBuffer, lookfor)) {
	*InputPos = save;
        MoveNChars( InputPos - found, BACK);
    }
    else {
        *InputPos = save;
	Beep();
    }
    return DEF;
}

/*---------------------------------------------------------------------------
 * Doug's code for accessing the history list
 *---------------------------------------------------------------------------
 */
static int
InsertCurrentHistEntry(void)
{
  register struct wordent *stop = &(curHistPos->Hlex);
  register struct wordent *wordl = stop->next;
  int oldrep = RepetitionCount;
  RepetitionCount = 1;
  MoveNChars( InputPos-InputBuffer, BACK);
  DeleteNChars( InputTail-InputBuffer, FORW);
  loop {
    register uchar_t *temp = (uchar_t *)wordl->word;

    if (! InsertString( temp, (temp)+strlen(temp))) return 0;
    wordl = wordl->next;
    if (wordl->next == stop) break;  /* Last word is a newline string */
    CurrentChar = ' '; 
    if (! DoInsertChar()) return 0;
  }
  RepetitionCount = oldrep;
  return 1;
}

static code
PreviousHistEntry(void)
{
  register int repl;
  for( repl = 0; repl < RepetitionCount; repl++ ) {
    if (curHistPos) {
      if ((curHistPos = curHistPos->Hnext) == 0)
        curHistPos = Histlist.Hnext;
    }
    else {
      curHistPos = Histlist.Hnext;
    }
  }
  if (! curHistPos || ! InsertCurrentHistEntry())
    Beep();
  return DEF;
}

static code
NextHistEntry(void)
{
  register struct Hist *target;
  register int repl;

  for( repl = 0; repl < RepetitionCount; repl++) {
    target = curHistPos;  /* save where we are first, then */
    curHistPos = Histlist.Hnext;  /* always start at top.  Its no slower. */
    if (Histlist.Hnext != 0) {
      if (target == Histlist.Hnext) target = 0;
      while (curHistPos->Hnext != target)
        if ((curHistPos = curHistPos->Hnext) == 0) curHistPos = Histlist.Hnext;
    }
  }
  if (! curHistPos || ! InsertCurrentHistEntry())
      Beep();
  return DEF;
}

/*---------------------------------------------------------------------------
 * Special routines for VI mode
 *---------------------------------------------------------------------------
 */
static code
ChangeFollowingObject (void)
{
    uchar_t buf;
    
    GetChar (&buf);
    
    switch (buf) {
	case SPACE:
	    DeleteCurrentChar();
	    break;
	case 'b':
	    EraseWord();
	    break;
	case 'd':
	    BeginningOfLine();
	    KillToEOL();
	    break;
	case 'w':
	    DeleteWord();
	    break;
	default:
	    Beep();
	    return DEF;
    }
    
    return EnterViInsert();
}

static code
DeleteFollowingObject (void)
{
    uchar_t buf;
    code result;
    
    GetChar (&buf);
    
    switch (buf) {
	case SPACE:
	    result = DeleteCurrentChar();
	    break;
	case 'b':
	    result = EraseWord();
	    break;
	case 'd':
	    BeginningOfLine();
	    result = KillToEOL();
	    break;
	case 'w':
	    result = DeleteWord();
	    break;
	default:
	    Beep();
	    result = DEF;
	    break;
    }
    
    return result;
}

static code
AppendToEOL (void)
{
    EndOfLine ();
    return EnterViInsert();
}

static code
InsertAtBOL (void)
{
    BeginningOfLine ();
    return EnterViInsert();
}

static code
ChangeChar (void)
{
    DeleteCurrentChar();
    return EnterViInsert();
}

static code
ChangeToEOL (void)
{
    KillToEOL ();
    return EnterViInsert();
}

static code
ChangeWholeLine (void)
{
    BeginningOfLine();
    KillToEOL();
    return EnterViInsert();
}

static code
ReplaceChar (void)
{
    DeleteCurrentChar();
    GetChar (&CurrentChar);
    if (! DoInsertChar()) Beep();
    return DEF;
}

static code
ViYankKillBuffer (void)
{
    if (InputPos < InputTail) ForwardChar ();
    YankKillBuffer ();
    return DEF;
}

/*---------------------------------------------------------------------------
 * InitMacros -- read macro files during initialization
 *---------------------------------------------------------------------------
 */
static void
InitMacros (void)
{
    register struct varent * v = adrof ((uchar_t *)"macrofiles");
    register uchar_t ** vp;
    register uchar_t * name;

    if (v == 0 || (vp = (uchar_t **)v -> vec) == 0) {    /* no macro file */
        return;
    }
    
    loop {
        if ( (name = *vp++) == 0 ) return;
	ReadMacroFile (name);
    }
}

/* so it can be called from goodbye() */
void
SaveMacros (void)
{
    register struct varent * v = adrof ((uchar_t *)"savemacros");
    register uchar_t ** vp;
    register uchar_t * name;

    if (Editor == DUMB || v == 0 || (vp = (uchar_t **)v -> vec) == 0) {
        return;
    }
    
    if ( (name = *vp) == 0 ) return;
    WriteMacroFile (name);
}

/*---------------------------------------------------------------------------
 * Sets the bindings for keymaps.
 *---------------------------------------------------------------------------
 */

/* dobindings is called from doset (sh.set.c) and during tenex
   initialization */
int
dobindings (uchar_t *val)
{
    if (! Initialized) return;	  /* only works when tenex is running */
    
    InitBindings( val);
    if (Editor != DUMB) {
	InitMacros();
	SetUserBindings();
	/* Add parameter for csh.login execution BJB 002 */
	if (adrof((uchar_t *)"home"))
		srccat(value((uchar_t *)"home"), (uchar_t *)"/.bindings", 1);
    }
}

static void
InitBindings (uchar_t *ed)
{

    if (ed == 0 || TermKnown == FALSE) {
        Editor = DUMB;
    }
    else {
	folddown (ed, ed);
        if (EQ(ed,"vi") || EQ(ed,"ex")) {
	    Editor = VI;
	}
	else if (EQ(ed,"emacs")) {
	    Editor = EMACS;
	}
	else
	    Editor = DUMB;
    }

    InitDefaultBindings();

    switch (Editor) {
	case EMACS:
	    InitAnsiBindings();
	    InitEmacsBindings();
	    break;
	case VI:
	    InitViBindings();
	    break;
	default:
	    InitDumbBindings();
	    break;
    }
}    

static void
InitEmacsBindings(void)
{
    CNXBIND (ExecuteUnNamedMacro, 'e');
    CNXBIND (SaveMacroFile, CTRL_S);
    CNXBIND (StartRemembering, '(');
    CNXBIND (StopRemembering, ')');
    CNXBIND (LoadMacroFile, CTRL_R);

    ESCBIND (BackwardWord, 'b');
    ESCBIND (DefineNamedMacro, 'n');
    ESCBIND (DeleteWord, 'd');
    ESCBIND (EraseWord, 'h');
    ESCBIND (FilenameExpansion, ESCAPE);
    ESCBIND (FilenameList, 'l');
    ESCBIND (ForwardWord, 'f');
    ESCBIND (ExecuteNamedMacro, 'e');
    ESCBIND (ExecuteNamedMacro, 'x');

    REGBIND (BackSpace, CTRL_B);
    REGBIND (BeginningOfLine, CTRL_A);
    REGBIND (ClearScreen, CTRL_L);
    REGBIND (DeleteCurrentChar, CTRL_D);
    REGBIND (DeletePreviousChar, CTRL_H);
    REGBIND (DeletePreviousChar, RUBOUT);
    REGBIND (EndOfLine, CTRL_E);
    REGBIND (ForwardChar, CTRL_F);
    REGBIND (InsertLiteralChar, CTRL_Q);
    REGBIND (KillToEOL, CTRL_K);
    REGBIND (KillRegion, CTRL_W);
    REGBIND (NextHistEntry, CTRL_N);
    REGBIND (PreviousHistEntry, CTRL_P);
    REGBIND (Redisplay, CTRL_R);
    REGBIND (Repetition, CTRL_U);
    REGBIND (Return, LINEFEED);
    REGBIND (Return, RETURN);
    REGBIND (SetCtrlX, CTRL_X);
    REGBIND (SetEscape, ESCAPE);
    REGBIND (SetMark, CTRL_AT);
    REGBIND (Tab, TAB);
    REGBIND (TransposeChars, CTRL_T);
    REGBIND (YankKillBuffer, CTRL_Y);
}

static void
InitViBindings(void)
{
    CNXBIND (AppendToEOL, 'A');
    CNXBIND (BackSpace, CTRL_H);
    CNXBIND (BackSpace, 'h');
    CNXBIND (BackwardWord, 'B');
    CNXBIND (BackwardWord, 'b');
    CNXBIND (BeginningOfLine, '0');
    CNXBIND (BeginningOfLine, '^');
    CNXBIND (ChangeChar, 's');
    CNXBIND (ChangeFollowingObject, 'c');
    CNXBIND (ChangeToEOL, 'C');
    CNXBIND (ChangeWholeLine, 'S');
    CNXBIND (DeleteCurrentChar, 'x');
    CNXBIND (DeleteFollowingObject, 'd');
    CNXBIND (DeletePreviousChar, 'X');
    CNXBIND (EndOfLine, '$');
    CNXBIND (FilenameExpansion, ESCAPE);
    CNXBIND (FilenameListOrEof, CTRL_D);
    CNXBIND (ForwardChar, 'l');
    CNXBIND (ForwardChar, SPACE);
    CNXBIND (ForwardWord, 'w');
    CNXBIND (ForwardWord, 'W');
    CNXBIND (ForwardWord, 'e');
    CNXBIND (InsertAtBOL, 'I');
    CNXBIND (KillToEOL, 'D');
    CNXBIND (ExecuteNamedMacro, '@');
    CNXBIND (NextHistEntry, '+');
    CNXBIND (NextHistEntry, 'j');
    CNXBIND (NextHistEntry, CTRL_N);
    CNXBIND (PreviousHistEntry, '-');
    CNXBIND (PreviousHistEntry, 'k');
    CNXBIND (PreviousHistEntry, CTRL_P);
    CNXBIND (Redisplay, CTRL_L);
    CNXBIND (Redisplay, CTRL_R);
    CNXBIND (Redisplay, 'z');
    CNXBIND (Repetition, '1');
    CNXBIND (Repetition, '2');
    CNXBIND (Repetition, '3');
    CNXBIND (Repetition, '4');
    CNXBIND (Repetition, '5');
    CNXBIND (Repetition, '6');
    CNXBIND (Repetition, '7');
    CNXBIND (Repetition, '8');
    CNXBIND (Repetition, '9');
    CNXBIND (ReplaceChar, 'r');
    CNXBIND (Return, LINEFEED);
    CNXBIND (Return, RETURN);
    CNXBIND (IncrementalSearchForward, '/');
    CNXBIND (IncrementalSearchReverse, '?');
    CNXBIND (SearchForward, 'f');
    CNXBIND (SearchReverse, 'F');
    CNXBIND (SetMark, 'm');
    CNXBIND (EnterViAppend, 'a');
    CNXBIND (EnterViInsert, 'i');
    CNXBIND (ViYankKillBuffer, 'p');
    CNXBIND (ViYankKillBuffer, 'P');
    
    REGBIND (DeletePreviousChar, CTRL_H);
    REGBIND (DeletePreviousChar, EraseChar);
    REGBIND (EraseWord, CTRL_W);
    REGBIND (ExitViInsert, ESCAPE);
    REGBIND (FilenameListOrEof, CTRL_D);
    REGBIND (InsertLiteralChar, CTRL_Q);
    REGBIND (InsertLiteralChar, CTRL_V);
    REGBIND (KillRegion, CTRL_U);
    REGBIND (NextHistEntry, CTRL_N);
    REGBIND (PreviousHistEntry, CTRL_P);
    REGBIND (Redisplay, CTRL_L);
    REGBIND (Redisplay, CTRL_R);
    REGBIND (Return, LINEFEED);
    REGBIND (Return, RETURN);
    REGBIND (Tab, TAB);
}    

static void
InitDumbBindings(void)
{
    register int i;
    
    for (i = 0; i < BINDINGS; i++)
	REGBIND (InsertChar, i);
    REGBIND (DeletePreviousChar, EraseChar);
    REGBIND (FilenameExpansion, ESCAPE);
    REGBIND (FilenameListOrEof, CTRL_D);
    REGBIND (Redisplay, ReprintChar);
    REGBIND (KillRegion, KillChar);
    REGBIND (EraseWord, WordErase);
    REGBIND (InsertLiteralChar, LiteralNext);
    REGBIND (Return, LINEFEED);
    REGBIND (Return, RETURN);
}

static void
InitAnsiBindings(void)
{
    ESCBIND (SetAnsi, '[');
    ESCBIND (SetAnsi, 'O');
    ANSIBIND (PreviousHistEntry, 'A');
    ANSIBIND (NextHistEntry, 'B');
    ANSIBIND (ForwardChar, 'C');
    ANSIBIND (BackSpace, 'D');
}

static void
InitDefaultBindings(void)
{
    register int i;
    
    for (i = 0; i < BINDINGS; i++) {
	REGBIND (DefaultBinding, i);
	ESCBIND (DefaultBinding, i);
	CNXBIND (DefaultBinding, i);
 	ANSIBIND(DefaultBinding, i);
    }

    for (i = SPACE; i < BINDINGS; i++) {   /* 001 */
	REGBIND (InsertChar, i);
    }
}

/*---------------------------------------------------------------------------
 * dobind -- a new cshell command to allow the user to interactively bind
 * internal functions and macros to keys.  Its arguments should be a list
 * of function name and key name pairs.
 *---------------------------------------------------------------------------
 */
void
dobind (register uchar_t **arglist)
{
    register uchar_t ** vp = arglist;
    register uchar_t * proc, * keyname;
    register int index, i;
    KeyBinding * keymap;

    uchar_t key;

    if (++vp == 0) {    /* no bindings */
        return;
    }
    
    loop {
        if ( ((proc = *vp++) == 0) || ((keyname = *vp++) == 0) ) return;

	strip((uchar_t *)keyname);	/* Remove quotes from string */
	
	if (proc[1] == 0 && KeyboardMacro[ proc[0] ].Head) {
	    index = stablk((uchar_t *) "executemacro", (uchar_t **)FunctionNames);
	}
	else {
	    folddown (proc, proc);
	    if ((index = stablk (proc, (uchar_t **)FunctionNames)) < 0 ) {
		csh_printf("bind: unknown function '%s'\n", proc);
		return;
	    }
	}
	if (keytran (keyname, &keymap, &key) == 0) {
	    csh_printf("bind: illegal key binding for %s\n", proc);
	    return;
	}
	keymap[key].Routine = FunctionPointers [index];
	keymap[key].MacroName = proc[0];
    }
}

/* The following routine sets key bindings from user instructions in the
   shell variable "bindings".  The value of this variable should be a list
   of procedure/key pairs:  (proc1 key1 proc2 key2 ...).  The legal
   procedure names are defined in the FunctionNames table. */

static void
SetUserBindings (void)
{
    register struct varent * v = adrof((uchar_t *)"bindings");
    register uchar_t ** vp;
    register uchar_t * proc, * keyname;
    register int index;
    KeyBinding * keymap;
    uchar_t key;

    if (v == 0 || (vp = (uchar_t **)v -> vec) == 0) {    /* no user bindings */
        return;
    }
    
    loop {
        if ( ((proc = *vp++) == 0) || ((keyname = *vp++) == 0) ) return;
	if (proc[1] == 0 && KeyboardMacro[ proc[0] ].Head) {
	    index = stablk((uchar_t *) "executemacro", FunctionNames);
	}
	else {
	    folddown (proc, proc);
	    if ((index = stablk (proc, FunctionNames)) < 0 ) {
		csh_printf("bindings: unknown function '%s'\n", proc);
		return;
	    }
	}
	if (keytran (keyname, &keymap, &key) == 0) {
	    csh_printf("bindings: illegal key binding for %s\n", proc);
	    return;
	}
	keymap[key].Routine = FunctionPointers [index];
	keymap[key].MacroName = proc[0];
    }
}

static int
keytran (uchar_t *keyname, KeyBinding **keymap, uchar_t *key)
{
    if (keyname[0] != '\\') {
        *keymap = RegBindings;
    }
    else if (keyname[1] == '\\') {
        *keymap = RegBindings;
	keyname += 1;
    }
    else if (keyname[1] == 'e') {
	if ( (Editor == EMACS) &&
	     ( (keyname[2] == '[') || (keyname[2] == 'O') ) ) {
	    *keymap = AnsiBindings;
	    keyname += 3;
	} else {
            *keymap = EscBindings;
	    keyname += 2;
	}
    }
    else if (keyname[1] == '^') {
        if (keyname[2] == 'X' || keyname[2] == 'x') {
	    *keymap = CnxBindings;
	    keyname += 3;
	}
	else {
	    *keymap = RegBindings;
	}
    }                               /*  001  */
    else {                          /* Default to regular bindings, */
         *keymap = RegBindings;     /*   so '\\' is not necessary.  */
    }
                               /* 001  Add b,f,and v cases */

    if (keyname[0] != '\\')         *key = keyname[0];	/* regular char */
    else if (keyname[1] == '\\')    *key = '\\';	/* literal \ */
    else if (keyname[1] == 'b')     *key = '\b';        /* backspace */
    else if (keyname[1] == 'e')	    *key = 033;	        /* escape */
    else if (keyname[1] == 'f')     *key = '\f';        /* formfeed */
    else if (keyname[1] == 'n')	    *key = '\n';	/* newline */
    else if (keyname[1] == 'r')	    *key = '\r';	/* return */
    else if (keyname[1] == 't')	    *key = '\t';	/* tab */
    else if (keyname[1] == 'v')     *key = '\v';        /* vertical tab */
    else if (keyname[1] == '^') {
	if (keyname[2] == '?')	    *key = 0177;	/* rubout */
	else if (keyname[2] == '[') *key = 033;		/* escape */
	else {
	    if (keyname[2] > 0140 && keyname[2] < 0173) /* fold up */
		keyname[2] -= 040;
	    if (keyname[2] < 077 || keyname[2] > 0137) return 0;
	    *key = (keyname[2] + 0100); /* map to control */  /* 001 */
	}
    } else if (keyname[1] >= '0' && keyname[1] <= '7') { /* octal */  /* 001 */
      register int cnt, val, ch;
      register uchar_t *cp = &keyname[1];

      for (cnt = 0, val = 0; cnt < 3; cnt++) {
	ch = *cp++ & 0377;
	if (ch < '0' || ch > '7') break;
	val = (val << 3) | (ch - '0');
      }
      if (val < BINDINGS)
	*key = (uchar_t) val;
      
      else {
        return 0;
      }
    }

    return 1;
}
    
uchar_t * FunctionNames [] = {
    (uchar_t *)"backspace",
    (uchar_t *)"backwardword",
    (uchar_t *)"beginningofline",
    (uchar_t *)"clearscreen",
    (uchar_t *)"defaultbinding",
    (uchar_t *)"definenamedmacro",
    (uchar_t *)"deletecurrentchar",
    (uchar_t *)"deletecurrentcharoreof",
    (uchar_t *)"deletepreviouschar",
    (uchar_t *)"deleteword",
    (uchar_t *)"endoffile",
    (uchar_t *)"endofline",
    (uchar_t *)"eraseline",
    (uchar_t *)"eraseword",
    (uchar_t *)"executemacro",
    (uchar_t *)"executenamedmacro",
    (uchar_t *)"executeunnamedmacro",
    (uchar_t *)"filenameexpansion",
    (uchar_t *)"filenamelist",
    (uchar_t *)"filenamelistoreof",
    (uchar_t *)"forwardchar",
    (uchar_t *)"forwardword",
    (uchar_t *)"gnutransposechars",
    (uchar_t *)"ignore",
    (uchar_t *)"incrementalsearchforward",
    (uchar_t *)"incrementalsearchreverse",
    (uchar_t *)"insertchar",
    (uchar_t *)"insertliteralchar",
    (uchar_t *)"killregion",
    (uchar_t *)"killtoeol",
    (uchar_t *)"loadmacrofile",
    (uchar_t *)"nexthistentry",
    (uchar_t *)"previoushistentry",
    (uchar_t *)"redisplay",
    (uchar_t *)"repetition",
    (uchar_t *)"return",
    (uchar_t *)"savemacrofile",
    (uchar_t *)"searchforward",
    (uchar_t *)"searchreverse",
    (uchar_t *)"setmark",
    (uchar_t *)"startremembering",
    (uchar_t *)"stopremembering",
    (uchar_t *)"tab",
    (uchar_t *)"transposechars",
    (uchar_t *)"wipeline",
    (uchar_t *)"yankkillbuffer",
    0    
};

CodeFunction FunctionPointers [] = {
    BackSpace,
    BackwardWord,
    BeginningOfLine,
    ClearScreen,
    DefaultBinding,
    DefineNamedMacro,
    DeleteCurrentChar,
    DeleteCurrentCharOrEof,
    DeletePreviousChar,
    DeleteWord,
    EndOfFile,
    EndOfLine,
    EraseLine,
    EraseWord,
    ExecuteMacro,
    ExecuteNamedMacro,
    ExecuteUnNamedMacro,
    FilenameExpansion,
    FilenameList,
    FilenameListOrEof,
    ForwardChar,
    ForwardWord,
    GnuTransposeChars,
    Ignore,
    IncrementalSearchForward,
    IncrementalSearchReverse,
    InsertChar,
    InsertLiteralChar,
    KillRegion,
    KillToEOL,
    LoadMacroFile,
    NextHistEntry,
    PreviousHistEntry,
    Redisplay,
    Repetition,
    Return,
    SaveMacroFile,
    SearchForward,
    SearchReverse,
    SetMark,
    StartRemembering,
    StopRemembering,
    Tab,
    TransposeChars,
    WipeLine,
    YankKillBuffer
};

/*  folddown  --  perform case folding
 *
 *  Usage:  p = folddown (out,in);
 *	uchar_t *p,*in,*out;
 *
 *  Folddown performs case-folding, moving string "in" to
 *  "out" and folding uuper to lower case en route.
 *  The same string may be specified as both "in" and "out".
 *  The address of "out" is returned for convenience.
 */

typedef enum {FOLDUP, FOLDDOWN} FOLDMODE;

uchar_t *
folddown (uchar_t *out, uchar_t *in)
{
	register uchar_t *i,*o;
	register uchar_t lower;
	uchar_t upper;
	int delta;

	lower = 'A';
	upper = 'Z';
	delta = 'a' - 'A';

	i = in;
	o = out;
	do {
		if (*i >= lower && *i <= upper)		*o++ = *i++ + delta;
		else					*o++ = *i++;
	} 
	while (*i);
	*o = '\0';
	return (out);
}

/*  stablk  --  string table lookup
 *
 *  Usage:  i = stablk (arg,table);
 *
 *	int i;
 *	uchar_t *arg,**table;
 *
 *  Stablk looks for a string in "table" which matches
 *  "arg".  Table is declared like this:
 *    uchar_t *table[] = {"string1","string2",...,0};
 *
 */


int
stablk (uchar_t *arg, uchar_t **table)
{
	register int i;
	int exactmatch=0;

	for (i=0; table[i] != NULL && exactmatch == 0; i++) {
		if ((table[i][0] == arg[0]) && (strcmp(table[i],arg) == 0))  
			exactmatch = 1;
	}
	if (exactmatch)  
		return (i-1);		/* i-1th entry is exact match */
	else
		return (-1);
}
#endif	/* CMEDIT */
