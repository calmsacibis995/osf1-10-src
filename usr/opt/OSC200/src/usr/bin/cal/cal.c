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
static char rcsid[] = "@(#)$RCSfile: cal.c,v $ $Revision: 4.2.5.4 $ (DEC) $Date: 1993/10/11 16:00:22 $";
#endif
/*
 * OSF/1 1.1
 */
/*
 * HISTORY
 */
/*
 * COMPONENT_NAME: (CMDMISC) miscellaneous commands
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * cal.c 1.15  com/cmd/misc/cal.c, cmdmisc, bos320, 9125320 5/22/91 22:02:46
 */

#define MOWIDTH		30		/* dimensions of printed calendars */ 
#define MODEPTH		6
#define TTLWIDTH	26
#define LNWIDTH		80		/*it is modifed to ensure enough size*/
#define STRSIZE		LNWIDTH * MODEPTH
#define COLUMN_WIDTH	4
#define MAXYR		9999
void cal(), pstr();
int max_week_len();

char	*dayw[] = {
	" Su ",
	" Mo ",
	" Tu ",
	" We ",
	" Th ",
	" Fr ",
	" Sa "
};

char    *lmon[12]= {
	"January", "February", "March", "April",
	"May", "June", "July", "August",
	"September", "October", "November", "December"
};

char	mon[13] = {               /* month ending days */
	0,
	31, 29, 31, 30,
	31, 30, 31, 31,
	30, 31, 30, 31,
};

#include <locale.h> 
#include "cal_msg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <langinfo.h>
char *center();
char *nlfile;

char *days[] = {
	"Sun","Mon","Tue","Wed",
	"Thu","Fri","Sat"
};


#define MSGSTR(Num, Str) catgets(catd, MS_CAL, Num, Str)
nl_catd catd;


char	string[STRSIZE];  /* days of the calendar(s) */
/* it is modified to ensure the size of mhdr is enough */
char    mhdr[41];  /* month title for single month calendar */

/*
 * NAME: main
 *                                                                    
 * FUNCTION: The cal command does one of four things when invoked:
 *	     - prints usage statement if argv[1] == "-?"
 *	     - prints current month of current year by default
 *	     - prints a full year calendar if called with one argument
 *	     - prints a single month calendar if called with two 
 *	       arguments
 *
 * RETURN VALUE DESCRIPTION:  0  successful completion of any of the above 
 *			      1  bad input detected 
 */  
main(argc, argv)
int argc;
char *argv[];
{
	register int y, i, j;
	int ttlwidth,mowidth,lnwidth,max,new;	/* added variables in modification */
	int m;
	time_t today;
	struct tm *ts;
	
        (void)setlocale(LC_ALL, ""); 
        catd = catopen(MF_CAL, NL_CAT_LOCALE);
   
	initdaysmonths();

/* modified code */
	ttlwidth = TTLWIDTH;
	max = max_week_len();
	mowidth = MOWIDTH;
	if (max > 3) {
		ttlwidth += (max - 3)*7;
		mowidth = ((max * 7) + 9) ;
		}
	lnwidth = 2 * mowidth;
	new = 2;
	if (ttlwidth >40){
	   	lnwidth = mowidth; 
		new = 1;
	}

/*	
 *	print out complete year
 *	(CASE 1)
 */

	if(argc > 3) {
	      fprintf(stderr,MSGSTR(USAGE, "usage: cal [[month] year]\n"));
	      exit(1);
	}
	if(argc == 2) {
	   if ( argv[1][0] == '-' ) {
		   if ( argv[1][1] == '?' ) {
		      /* This is a POSIX requirement */
		      printf(MSGSTR(USAGE, "usage: cal [[month] year]\n"));
		      exit(0);
		   } else {
		      printf(MSGSTR(USAGE, "usage: cal [[month] year]\n"));
		      exit(1);
		   }
	   }
	   if ((y = number(argv[1])) == 0) {
	      printf(MSGSTR(USAGE, "usage: cal [[month] year]\n"));
	      exit(1);
	   }
	   if(y>MAXYR) {
			fprintf(stderr, MSGSTR(BADYEARARG, "Bad argument\n"));
			exit(1);
	   }
	   printf("\n\n\n");
           printf("			%u\n", y);
           printf("\n");

	/* The coding is modified */

	   for(i=0; i<12; i+=new) {
		for(j=0; j<STRSIZE; j++)
			string[j] = '\0';

		printf("%s   ", center(lmon[i],ttlwidth));
		if (new == 2) 
			printf("%s\n", center(lmon[i+1],ttlwidth));
	 	else printf("\n");
		prn_days_hdr();
		if (new == 2) prn_days_hdr();
		printf("\n");

		cal(i+1, y, string, lnwidth);
		if (new == 2) 
			cal(i+2, y, string+mowidth-1, lnwidth);

		/* print out the rows and columns of month days */
		for(j=0; j<MODEPTH*lnwidth; j+=lnwidth)
			pstr(string+j, lnwidth);
	   }
           printf("\n\n\n");
	   exit(0);
	}
/*	   
 *	   print out just month
 *	   (CASE 2)
 */
	   if (argc ==1 ){
	      time(&today);
	      ts = localtime(&today);
	      m = ts->tm_mon+1;
	      y = 1900 + ts->tm_year;
	   }
	   else{
	      m = number(argv[1]);
	      y = number(argv[2]);
	   }
	   if(m<1 || m>12)
		{
			fprintf(stderr, MSGSTR(BADMONARG, "Bad argument\n"));
			exit(1);
		}
	   if(y<1 || y>MAXYR)
		{
			fprintf(stderr, MSGSTR(BADYEARARG, "Bad argument\n"));
			exit(1);
		}
	   sprintf(mhdr, "%s %u",lmon[m-1], y);
	   printf("%s\n", center(mhdr, ttlwidth));
	   prn_days_hdr();
	   printf("\n");

	   cal(m, y, string, mowidth);
	   for(i=0; i<MODEPTH*mowidth; i+=mowidth)
		pstr(string+i, mowidth);
	   exit(0);
}  


/*
 * NAME: number
 *                                                                    
 * FUNCTION: converts a string of numbers to its numeric value
 *
 * RETURN VALUE DESCRIPTION:  the numeric value of the string if its characters
 *			      are 0 - 9, zero otherwise
 */  
int number(str)
char *str;
{
	register int n, c;
	register char *s;

	n = 0;
	s = str;
	while(c = *s++) {
		if(c<'0' || c>'9')
			return(0);
		n = n*10 + c-'0';
	}
	return(n);
}


/*
 * NAME: pstr
 *                                                                    
 * FUNCTION:  prints the given string to stdout, after replacing nulls with
 *            blanks
 *                                                                    
 * RETURN VALUE DESCRIPTION: NONE
 *
 */  
void  pstr(str, n)           /* void * declaration causes clash with str */
char *str;
int n;
{
	register int i;
	register char *s;

	s = str;
	i = n;
	while(i--)
		if(*s++ == '\0')
			s[-1] = ' ';         /* replace the null */
	i = n+1;
	while(i--)
		if(*--s != ' ')
			break;
	s[1] = '\0';
	printf("%s\n", str);
}




/*
 * NAME: cal
 *                                                                    
 * FUNCTION: fills an array with the days of the given month 
 *                                                                    
 * RETURN VALUE DESCRIPTION: NONE 
 *			     
 */  
void cal(m, y, p, w)        
int m,y,w;      /* month, year, offset to the next week of current month */
char *p;	/* start addr of the array to be loaded with calendar days */ 
{
	int j,column_width,max,offset;	/* offset is to make the day    */
					/* at the center 		*/
	register int d, i; /* day of the week, counter */
	register char *s;  /* current pointer position in the array of days */

	s = p;
	d = jan1(y);
	mon[2] = 29;
	mon[9] = 30;
	max = max_week_len();

	switch((jan1(y+1)+7-d)%7) {

	/*
	 *	non-leap year
	 */
	case 1:
		mon[2] = 28;
		break;

	/*
	 *	1752
	 */
	default:
		mon[9] = 19;
		break;

	/*
	 *	leap year
	 */
	case 2:
		;
	}
	for(i=1; i<m; i++) {
		d += mon[i];    
		}
	d %= 7;		

	column_width = COLUMN_WIDTH;
	if (max > 3) column_width = max +1;
	s += column_width*d;
	if (max > 3) {
		offset=(max -2)/2 ;
		s+= offset;
		p+= offset;
	}
	for(i=1; i<=mon[m]; i++) {
		if(i==3 && mon[m]==19) {
			i += 11;
			mon[m] += 11;
		}
		if(i > 9)
			*s = i/10+'0';   /* load the 10's digit */
		s++;

		*s++ = i%10+'0';         /* load the  1's digit */
		s++; s++;

		if (max > 3) for(j=max-3;j!=0;j--)  s++;

		if(++d == 7) {
			d = 0;
			s = p+w;   /* advance to load the next week */  
			p = s;
		}
	}
}


/*
 * NAME: jan1
 *                                                                    
 * FUNCTION: returns the day of the week of Jan 1 of the given year
 *                                                                    
 * RETURN VALUE DESCRIPTION:  0 through 6 correspond to Sunday through Saturday 
 *			     
 */  

int jan1(yr)
int yr;
{
	register int y, d;

/*
 *	normal gregorian calendar has
 *	one extra day per four years
 */

	y = yr;
	d = 4+y+(y+3)/4;

/*
 *	julian calendar is the
 *	regular gregorian
 *	less three days per 400
 */

	if(y > 1800) {
		d -= (y-1701)/100;
		d += (y-1601)/400;
	}

/*
 *	great calendar changeover instant
 */

	if(y > 1752)
		d += 3;

	return(d%7);
}

/*
 * NAME: center
 *                                                                    
 * FUNCTION: centers string s in a field of length len
 *                                                                    
 * RETURN VALUE DESCRIPTION: NONE 
 *			     
 */  
char *center(s, len)
   char *s; int len; 
{
	int i, slen;
	static char buf[4 * MOWIDTH + 1];

	slen = mbslen(s);
	if (slen > len)
		mbsncpy(buf, s, len);
	else {
		for(i=0;i<((len-slen)/2)+((len-slen)%2);i++) buf[i]=' ';
		strcpy(&buf[i], s);
		for(i=strlen(buf);i<len;i++) buf[i]=' ';
	}
	buf[strlen(buf)]='\0';
	return(buf);
}



/*
 * NAME: initdaysmonths
 * FUNCTION: initialize the arrays of days and months.
 */

nl_item dayitems[7] = {
	DAY_1, DAY_2, DAY_3, DAY_4,
	DAY_5, DAY_6, DAY_7
};

nl_item abdayitems[7] = {
	ABDAY_1, ABDAY_2, ABDAY_3, ABDAY_4,
	ABDAY_5, ABDAY_6, ABDAY_7
};

nl_item monthitems[12] = {
	MON_1, MON_2, MON_3, MON_4, MON_5, MON_6,
	MON_7, MON_8, MON_9, MON_10, MON_11, MON_12
};

initdaysmonths()
{
	char *ptr, *cp;
	int i;

	for(i = 0; i < 7; i++) {
		ptr = nl_langinfo(dayitems[i]);
		if (ptr == NULL) continue     ;
		if((days[i] = malloc(strlen(ptr) + 1)) != NULL) {
			strcpy(days[i], ptr);
		}
	}

	for(i = 0; i < 7; i++) {
		ptr = nl_langinfo(abdayitems[i]);
		if (ptr == NULL) continue	;
		if((dayw[i] = malloc(strlen(ptr) + 1)) != NULL) {
			strcpy(dayw[i], ptr);
		}
	}

	for(i = 0; i < 12; i++) {
		ptr = nl_langinfo(monthitems[i]);
		if (ptr == NULL) continue	;
		if((lmon[i] = malloc(strlen(ptr) + 1)) != NULL) {
			strcpy(lmon[i], ptr);
		}
	}

}

/*
 * NAME: prn_days_hdr
 * FUNCTION: print the days header above the calendar
 */

prn_days_hdr()
{
	int i,max;

	for(i = 0; i < 7; i++) {
		printf("%-4s", dayw[i]);
		if ((max=max_week_len()) > 3)
			printf(" ");
	}
	printf(" ");
}

int max_week_len()
{
	int i,max;
	max = strlen(dayw[0]);
        for(i = 1; i < 7; i++) {
		if (strlen(dayw[i]) > max) 
			max = strlen(dayw[i]);
        }
	return max;
}
