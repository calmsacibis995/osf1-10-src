%{
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
static char rcsid[] = "@(#)$RCSfile$ $Revision$ (DEC) $Date$";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.13  com/cmd/cntl/cron/att1.y, cmdcntl, bos320, 9136320a 8/30/91 00:23:51
 */


#include <string.h>
#include <stdio.h>
#include <langinfo.h>
#include <locale.h>
#include <time.h>
#include <sys/dir.h>
#include "att1.h"

#include <ctype.h>

extern	int	gmtflag;
int timedon = 0;
int daydon = 0;
int yeartm = 0;
extern	int	mday[];
extern	struct	tm *tp, at, rt;

#include "cron_msg.h"
extern nl_catd catd;
#define	MSGSTR(Num,Str) catgets(catd,MS_CRON,Num,Str)
%}
%token	TIME
%token	NOW
%token	NOON
%token	N
%token	MIDNIGHT
%token	M
%token	MINUTE
%token	HOUR
%token	DAY
%token	WEEK
%token	MONTH
%token	YEAR
%token	UNIT
%token	SUFF
%token	AM
%token	A
%token	PM
%token	P
%token	ZULU
%token	NEXT
%token	NUMB
%token	COLON
%token	COMMA
%token	PLUS
%token	DAYN
%token	YEARN
%token	UTC
%token	UNKNOWN
%%

args
	: time date incr {
		if (at.tm_min >= 60 || at.tm_hour >= 24)
			atabort(MSGSTR(MS_BTIME,"bad time"));
		if (at.tm_year >= 100)
			at.tm_year -= 1900;
		if (at.tm_year < 70 || at.tm_year >= 138)
			/* need valid year 			*/
			/* The valid year is 1970 - 2037.	*/
			atabort(MSGSTR(MS_BYEAR,"bad year"));
		if (leap(at.tm_year))
			/* to check for leap year */
			mday[1]=29;
		if (at.tm_mon >= 12 || at.tm_mday > mday[at.tm_mon])
			/* to check for day */
			atabort(MSGSTR(MS_BDATE,"bad date"));
		return;
	}
	| time date incr UNKNOWN {
		yyerror();
	}
	;

time
	: hour opt_suff {
	timedon = 1;   
	checksuff:
		at.tm_hour = $1;
		switch ($2) {
		case P:
		case PM:
			if (at.tm_hour < 1 || at.tm_hour > 12)
				atabort(MSGSTR(MS_BHOUR,"bad hour"));
			at.tm_hour %= 12;
			at.tm_hour += 12;
			break;
		case A:
		case AM:
			if (at.tm_hour < 1 || at.tm_hour > 12)
				atabort(MSGSTR(MS_BHOUR,"bad hour"));
			at.tm_hour %= 12;
			break;
		case UTC:
		case ZULU:
			if (at.tm_hour == 24 && at.tm_min != 0)
				atabort(MSGSTR(MS_BTIME,"bad time"));
			at.tm_hour %= 24;
			gmtflag = 1;
		}
	}
	| hour COLON minute opt_suff {
		torder(&$1, &$3);
		timedon = 1;   
		at.tm_min = $3;
		$3 = $1;
		goto checksuff;
	}
	| hour minute opt_suff {
		torder(&$1, &$2);
		timedon = 1;   
		at.tm_min = $2;
		$2 = $1;
		goto checksuff;
	}
	| TIME {
		timedon = 1;   
		switch ($1) {
		case NOON:
			at.tm_hour = 12;
			break;
		case MIDNIGHT:
			at.tm_hour = 0;
			break;
		case NOW:
			at.tm_hour = tp->tm_hour;
			at.tm_min = tp->tm_min;
			at.tm_sec = tp->tm_sec;
			break;
		}
	}
	;

date
	: /*empty*/ {
		at.tm_mday = tp->tm_mday;
		at.tm_mon = tp->tm_mon;
		at.tm_year = tp->tm_year;
		if ((at.tm_hour < tp->tm_hour)
			|| ((at.tm_hour==tp->tm_hour)&&(at.tm_min<tp->tm_min)))
			rt.tm_mday++;
	}
	| dayn month {
		at.tm_mon = $2;
		at.tm_mday = $1;
		at.tm_year = tp->tm_year;
		if (at.tm_mon < tp->tm_mon)
			at.tm_year++;
	}
	| month dayn {
		at.tm_mon = $1;
		at.tm_mday = $2;
		at.tm_year = tp->tm_year;
		if (at.tm_mon < tp->tm_mon)
			at.tm_year++;
	}
	| dayn month COMMA year {
		at.tm_mon = $2;
		at.tm_mday = $1;
		at.tm_year = $4;
	}
	| month dayn COMMA year {
		at.tm_mon = $1;
		at.tm_mday = $2;
		at.tm_year = $4;
	}
	| DAY {
		at.tm_mon = tp->tm_mon;
		at.tm_mday = tp->tm_mday;
		at.tm_year = tp->tm_year;
		if ($1 < 7) {
			rt.tm_mday = $1 - tp->tm_wday;
			if (rt.tm_mday < 0)
				rt.tm_mday += 7;
		}
		else if ($1 == 8)
			rt.tm_mday += 1;
	}
	;

incr
	: /*empty*/
	| NEXT UNIT	{ addincr:
		switch ($2) {
		case MINUTE:
			rt.tm_min += $1;
			break;
		case HOUR:
			rt.tm_hour += $1;
			break;
		case DAY:
			rt.tm_mday += $1;
			break;
		case WEEK:
			rt.tm_mday += $1 * 7;
			break;
		case MONTH:
			rt.tm_mon += $1;
			break;
		case YEAR:
			rt.tm_year += $1;
			break;
		}
	}
	| PLUS opt_number UNIT { goto addincr; }
	;

dayn
	: DAYN {
			daydon = 1;
			$$ = $1; }
	;

year
	: number	{ $$ = $1; }
	| number number	{ $$ = 100 * $1 + $2; }
	;

hour
	: NUMB		{ $$ = $1; }
	;
minute
	: NUMB		{ 
			timedon = 1;
			$$ = $1; }
month
	: MONTH	        { 
			timedon = 2;  
			$$ = $1; 
			}
	;
opt_number
	: /* empty */	{ $$ = 1; }
	| number	{ $$ = $1; }
	;
number
	: NUMB		{ $$ = $1; }
	| number NUMB	{ $$ = 10 * $1 + $2; }
	;
opt_suff
	: /* empty */	{ $$ = 0; }
	| SUFF		{ $$ = $1; }
	;

%%
torder(h, m)
int *h, *m;
{
	char timeord[20];
	int i, tmp;
	char *s;

	if ((s = nl_langinfo(T_FMT)) != NULL) {
		strcpy(timeord, s);
		for (i=0; timeord[i] != '\0'; i++) {
			if (timeord[i] == 'h' || timeord[i] == 'H' ||
				timeord[i] == 'I' ||
				(timeord[i] == 's' && timeord[i+1] == 'H'))
				break;
			if (timeord[i] == 'm' || timeord[i] == 'M') {
				tmp = *m;
				*m = *h;
				*h = tmp;
				break;
			}
		}
	}
}


int dayfirst()
{
	char dateord[20];
	int i;
	char *s;

	if ((s = nl_langinfo(D_FMT)) != NULL) {
		strcpy(dateord, s);
		for (i=0; dateord[i] != '\0'; i++) {
			if (dateord[i] == 'm' || dateord[i] == 'B' ||
				dateord[i] == 'b' || dateord[i] == 'h' ||
				(dateord[i] == 'l' && dateord[i+1] == 'h') ||
				dateord[i] == 'M') {
				return(0);
			}
			if (dateord[i] == 'd') {
				return(1);
			}
		}
	}
	return(0);
}

#undef getc
#define	getc()		(*argp ? *argp++ : EOF)

char	*argp = "";

char timesep;

char mon[12][8] = {
			"jan",
		   	"feb",
			"mar",
			"apr",
			"may",
			"jun",
			"jul",
			"aug",
			"sep",
			"oct",
			"nov",
			"dec"
			};
char lgmon[12][26] = {
			"january",
			"february",
			"march",
			"april",
			"may",
			"june",
			"july",
			"august",
			"september",
			"october",
			"november",
			"december"
			};
char days[7][8] = {
			"sun",
			"mon",
			"tue",
			"wed",
			"thu",
			"fri",
			"sat"
			};
char lgdays[7][26]  =  {
			"sunday",
			"monday",
			"tuesday",
			"wednesday",
			"thursday",
			"friday",
			"saturday"
			};
char ampm[2][26]  = {
			"a.m.",
			"p.m."
			};
char rest[31][26] = {
			"am",
			"pm",
			"zulu",
			"now",
			"yesterday",
			"tomorrow",
			"noon",
			"midnight",
			"next",
			"weekdays",
			"weekend",
			"today",
			"a",
			"p",
			"n",
			"m",
			"minute",
			"minutes",
			"hour",
			"hours",
			"day",
			"days",
			"week",
			"weeks",
			"month",
			"months",
			"year",
			"years",
			"min",
			"mins",
			"utc"
			};

char nl_mon[12][8];
char nl_lgmon[12][26];
char nl_days[7][8];
char nl_lgdays[7][26];
char nl_ampm[2][26];

yylex()
{
	int c, j=0;
	char buf[26];
	int val;
	int num;
	

	while ((c = getc()) != EOF) {
		switch (c) {
		case '\t':
		case '\n':
		case ' ':
			break;
		case ':':
			if (timesep == ':') {
				yylval = 0;
				return(COLON);
			}
			break;
		case '.':
			if (timesep == '.') {
				yylval = 0;
				return(COLON);
			}
			break;
		case ',':
			yeartm++;
			yylval = 0;
			return(COMMA);
			break;
		case '+':
			yylval = 0;
			return(PLUS);
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (lookahd() == 2) {  /* a number */
				val = (c - '0') * 10 + (getc() - '0'); 
				num = 2;
			}
			else {
				val = c - '0';
				num = 1;
			}
			if (lookahd() == 1) {
				return(yylval=val,NUMB);
			}
			if (timedon == 2 && daydon != 1) {
				/*
				 * The order, month followed by day, should
				 * be passed in all locale environment.
				 */
				return(yylval = val,DAYN);
			}
			else {
				if (timedon == 1 && daydon != 1 && dayfirst()){
				  char *argpsav;
			
				  argpsav = argp;
				  if (yylex() == MONTH)  {
				    argp = argpsav;
				    yylval = val;
				    return(DAYN);
				  }
				  else {
				    argp = argpsav;
				    yylval = val;
				    return(NUMB);
				  }
				}
				return(yylval=val,NUMB);
			}
			break;
		default:
			buf[j++] = c;
			while (lookahd() == 1)
				buf[j++] = getc();
			buf[j] = '\0';
			j = 0;
			return(findToken(buf));
		}
	}
}


lookahd()
{
	switch (*argp) {
	case '\t':
	case '\n':
	case ' ':
	case ':':
	case '.':
	case ',':
	case '+':
		return(0);
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		return(2);
	default:
		return(1);
	}
}

#define EXACT_MATCH(i, inString, tokenString)	\
		(inString[i] == (char)0 && tokenString[i] == (char)0)
#define NO_MATCH	0
#define ONE_MATCH	1
#define PERFECT_MATCH	2
#define CHECK_EXACT_MATCH(match, token, tokenVal) if (match == PERFECT_MATCH){ yylval = tokenVal; return(token);}



/* 
 * findTokenInString() will search the input string for an initial
 * occurrence of a token (taken from the input token table) that is
 * longer then the token length passed in lenBestMatch.  It will
 * return upon finding a perfect match that is longer that
 * lenBestMatch, or after it search all known tokens.
 * Upon exit, it will set the values of the length of the best match and
 * index of the best match.
 *
 * Return Values:
 *   success: 	PERFECT_MATCH for exact match
 *		ONE_MATCH for match
 *   failure:	NO_MATCH for no match
 */
static int
findTokenInString(char *inString, 
		  char *tokenTab, int tabNumEl, int tabElSize,
		  int *lenBestMatch, int *indexBestMatch)
{
	int  i, len, foundOne;
	char *tokenp, *tokenInStr;

	tokenp = NULL;
	foundOne = NO_MATCH;
	len = 0;

	for (i = 0; i < tabNumEl; i++) {
		tokenp = &tokenTab[i*tabElSize];
		tokenInStr = strstr(inString, tokenp);
		if (tokenInStr == inString) {
			len = strlen(tokenp);
			if (len > *lenBestMatch) {
			        foundOne = ONE_MATCH;
				*lenBestMatch = len;
				*indexBestMatch = i;
				if (EXACT_MATCH(len, inString, tokenp)) {
					return(PERFECT_MATCH);
				}				
			}				
		}
	}

	return(foundOne);
}

/* 
 * findToken() takes an input string and finds longest token contained
 * by the string, based on matching the maximum initial segment of the 
 * string.  findToken will return upon finding a perfect match, or it 
 * search all known tokens to find the longest match.
 *
 * Return Values:
 *   success: 	findToken sets yylval to array index of token in 
 *		token array, sets pointer argp to the first char unused
 * 		in pattern in making the match and returns the token.
 */
findToken(char *inString)
{
	int  matchType, m1, m2, m3, m4;
	int  tokenBestMatch, lenBestMatch, indexBestMatch, len;
	char lcInString[30];
	char *tokenp;

	strtolower(lcInString, inString);
	tokenp = NULL;
	m1 = m2 = m3 = m4 = len = 0;
	lenBestMatch = 0;
	indexBestMatch = -1;
	tokenBestMatch = -1;

	/* Search through month tokens */
	if (   (m1 = findTokenInString(lcInString, &mon[0][0], 
			sizeof(mon)/sizeof(mon[0]), sizeof(mon[0]), 
			&lenBestMatch, &indexBestMatch)) == PERFECT_MATCH
	    || (m2 = findTokenInString(lcInString, &lgmon[0][0], 
			sizeof(lgmon)/sizeof(lgmon[0]), sizeof(lgmon[0]),
		        &lenBestMatch, &indexBestMatch)) == PERFECT_MATCH
	    || (m3 = findTokenInString(lcInString, &nl_lgmon[0][0], 
			sizeof(nl_lgmon)/sizeof(nl_lgmon[0]), sizeof(nl_lgmon[0]),
		        &lenBestMatch, &indexBestMatch)) == PERFECT_MATCH
	    || (m4 = findTokenInString(lcInString, &nl_mon[0][0],
			sizeof(nl_mon)/sizeof(nl_mon[0]), sizeof(nl_mon[0]),
		        &lenBestMatch, &indexBestMatch)) == PERFECT_MATCH ) {
		yylval = indexBestMatch;
		return(timedon == 2 ? NUMB : MONTH);
	}

	if (m1 || m2 || m3 || m4) {
		tokenBestMatch = (timedon == 2 ? NUMB : MONTH);
	}

	/* Search through ampm tokens */
	if (   (m1 = findTokenInString(lcInString, &ampm[0][0],
			sizeof(ampm)/sizeof(ampm[0]), sizeof(ampm[0]),
		        &lenBestMatch, &indexBestMatch)) == PERFECT_MATCH
	    || (m2 = findTokenInString(lcInString, &nl_ampm[0][0],
			sizeof(nl_ampm)/sizeof(nl_ampm[0]), sizeof(nl_ampm[0]),
		        &lenBestMatch, &indexBestMatch)) == PERFECT_MATCH) {
		yylval = (indexBestMatch ? PM : AM);
		return(SUFF);
	}

	if (m1 || m2 ) {
		indexBestMatch = (indexBestMatch ? PM : AM);
		tokenBestMatch = SUFF;

	}

	/* Search through the day tokens */
	if (   (m1 = findTokenInString(lcInString, &days[0][0],
			sizeof(days)/sizeof(days[0]), sizeof(days[0]),
			&lenBestMatch, &indexBestMatch)) == PERFECT_MATCH
	    || (m2 = findTokenInString(lcInString, &nl_days[0][0], 
			sizeof(nl_days)/sizeof(nl_days[0]), sizeof(nl_days[0]),
		        &lenBestMatch, &indexBestMatch)) == PERFECT_MATCH
	    || (m3 = findTokenInString(lcInString, &lgdays[0][0],
			sizeof(lgdays)/sizeof(lgdays[0]), sizeof(lgdays[0]),
		        &lenBestMatch, &indexBestMatch)) == PERFECT_MATCH
	    || (m4 = findTokenInString(lcInString, &nl_lgdays[0][0],
			sizeof(nl_lgdays)/sizeof(nl_lgdays[0]), 
			sizeof(nl_lgdays[0]),
		        &lenBestMatch, &indexBestMatch)) == PERFECT_MATCH) {
		yylval = indexBestMatch;
		return(DAY);
	}

	if (m1 || m2 || m3 || m4) {
		tokenBestMatch = DAY;
	}

	/* Search through the grab-bag tokens */
	matchType = findTokenInString(lcInString, &rest[0][0],
			sizeof(rest)/sizeof(rest[0]), sizeof(rest[0]),
		        &lenBestMatch, &indexBestMatch);
	if (matchType != NO_MATCH) {
		switch (indexBestMatch) {
		case 11:
			/*  "today"  */
			tokenBestMatch = DAY;
			indexBestMatch = 7;
			break;
		case 5:
			/*  "tomorrow"  */
			tokenBestMatch = DAY;
			indexBestMatch = 8;
			break;
		case 14:
		case 6:
			/* "n", "noon"  */
			tokenBestMatch = TIME;
			indexBestMatch = NOON;
			break;
		case 15:
		case 7:
			/*  "m", "midnight"  */
			tokenBestMatch = TIME;
			indexBestMatch = MIDNIGHT;
			break;
		case 3:
			/*  "now"    */
			tokenBestMatch = TIME;
			indexBestMatch = NOW;
			break;
		case 12:
		case 0:
			/* "a", "am"  */
			tokenBestMatch = SUFF;
			indexBestMatch = AM;
			break;
		case 13:
		case 1:
			/*  "p", "pm"  */
			tokenBestMatch = SUFF;
			indexBestMatch = PM;
			break;
		case 2:
		case 30:
			/*  "zulu", "utc"   */
			tokenBestMatch = SUFF;
			indexBestMatch = ZULU;
			break;
		case 8:
			/*  "next"   */
			tokenBestMatch = NEXT;
			indexBestMatch = 1;
			break;
		case 16:
		case 17:
		case 28:
		case 29:
			/*  "minutes", "minute", "min", "mins"	*/
			tokenBestMatch = UNIT;			/*FPM001*/
			indexBestMatch = MINUTE;
			break;
		case 18:
		case 19:
			/*  "hour", "hours"   */
			tokenBestMatch = UNIT;
			indexBestMatch = HOUR;
			break;
		case 20:
		case 21:
			/*  "day", "days"   */
			tokenBestMatch = UNIT;
			indexBestMatch = DAY;
			break;
		case 22:
		case 23:
			/*  "week", "weeks"   */
			tokenBestMatch = UNIT;
			indexBestMatch = WEEK;
			break;
		case 24:
		case 25:
			/*   "month", "months"   */
			tokenBestMatch = UNIT;
			indexBestMatch = MONTH;
			break;
		case 26:
		case 27:
			/*  "year", "years"   */
			tokenBestMatch = UNIT;
			indexBestMatch = YEAR;
			break;
		}
		CHECK_EXACT_MATCH(matchType, tokenBestMatch, indexBestMatch);
	}

	
	/* Error checking */
	if (lenBestMatch == 0 ) {
		fprintf(stderr, MSGSTR(MS_WORD,"unknown word, %s\n"), lcInString);
		exit(1);
	}

	/* 
	 * If we reach here, the inString was not an exact match for any
	 * token, but a substring of the inString was an exact match.  So,
	 * we should adjust argp to point to the char after the end of
	 * the matching substring, set yylval, and return.
	 */
	argp -= (strlen(inString) - lenBestMatch);
	yylval = indexBestMatch;
	return(tokenBestMatch);

}

getnls()
{
	char *s;
	int	i;
	static nl_item	abmon_item[12] = { ABMON_1, ABMON_2, ABMON_3, ABMON_4,
			ABMON_5, ABMON_6, ABMON_7, ABMON_8, ABMON_9, ABMON_10,
			ABMON_11, ABMON_12 };
	static nl_item	mon_item[12] = { MON_1, MON_2, MON_3, MON_4, MON_5,
			MON_6, MON_7, MON_8, MON_9, MON_10, MON_11, MON_12 };
	static nl_item	abday_item[7] = { ABDAY_1, ABDAY_2, ABDAY_3, ABDAY_4,
			ABDAY_5, ABDAY_6, ABDAY_7 };
	static nl_item	day_item[7] = { DAY_1, DAY_2, DAY_3, DAY_4, DAY_5,
			DAY_6, DAY_7 };
	static nl_item	ampm_item[2] = { AM_STR, PM_STR };

	for (i=0; i < 12; i++) {
		s = nl_langinfo(abmon_item[i]);
		if (*s != '\0')
			strtolower(nl_mon[i], s);
		else
			nl_mon[i][0] = '\0';
		s = nl_langinfo(mon_item[i]);
		if (*s != '\0')
			strtolower(nl_lgmon[i], s);
		else
			nl_lgmon[i][0] = '\0';
	}

	for (i=0; i < 7; i++) {
		s = nl_langinfo(abday_item[i]);
		if (*s != '\0')
			strtolower(nl_days[i], s);
		else
			nl_days[i][0] = '\0';
		s = nl_langinfo(day_item[i]);
		if (*s != '\0')
			strtolower(nl_lgdays[i], s);
		else
			nl_lgdays[i][0] = '\0';
	}

	for (i=0; i < 2; i++) {
		s = nl_langinfo(ampm_item[i]);
		if (*s != '\0')
			strtolower(nl_ampm[i], s);
		else
			nl_ampm[i][0] = '\0';
	}

        if ((s = nl_langinfo(T_FMT)) != NULL && strcmp(s,"")) {
                while (*s != '\0' && (isalnum(*s) || *s == '%' ))
                        s++;
                if (*s == '\0')
                        timesep = ':';
                else
                        timesep = *s;

	}
}

/*
 * NAME: strtolower()
 *
 * FUNCTION: Convert upper case to lower case.
 *
 * RETURNS: N/A
 */
strtolower(s1, s2)
char *s1, *s2;
{
	int	rec;
	wchar_t nlc, nlcl;

	while (*s2 != '\0') {
		if ((rec = mbtowc(&nlc, s2, strlen(s2))) == -1)
			break;
		s2 += rec;
		nlcl = towlower(nlc);
		if ((rec = wctomb(s1, nlcl)) == -1)
			break;
		s1 += rec;
	}
	if (rec == -1)
		strcpy(s1, s2);
	else
		*s1 = '\0';
}

