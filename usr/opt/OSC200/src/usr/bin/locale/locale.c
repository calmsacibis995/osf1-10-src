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
/*
 * OSF/1 1.2
 */
/*
 * HISTORY
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: locale.c,v $ $Revision: 1.1.5.6 $ (OSF) $Date: 1993/12/21 23:06:26 $";
#endif

/*
 * COMPONENT_NAME: (CMDLOC) Locale Database Commands
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.8  com/cmd/nls/locale.c, , bos320, 9134320 8/12/91 17:08:36
 */
#include <locale.h>
#include <sys/localedef.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include "pathnames.h"

extern char *optarg;
extern int optind;

/* 
*  prototypes for static functions
*/
void mark_keyword(char *key);
void usage(void);
void error(int, ...);
void list_locales();
void list_charmaps();
void dump_names();
int check_locale( const char*, const struct stat*);
int check_charmap( const char*, const struct stat*);
int list_dir(const char *, int (*)(const char *, const struct stat *));
int is_locale(const char *);

void (*pint)(void **, int, char *);
void (*pstr)(void **, int, char *);
void (*pnum)(void **, int, char *);
void (*pslst)(void **, int, char *, int);
void (*psvec)(void **, int, char *, int);
void (*prtcollate)(void **, int, char *, int);
void (*prtctype)(void **, int, char *, int);
void (*pstrnum)(void **, int, char *);

void prt_int(void **, int, char *);
void prt_str(void **, int, char *);
void prt_num(void **, int, char *);
void prt_slst(void **, int, char *, int);
void prt_svec(void **, int, char *, int);
void prt_collate(void **, int, char *, int);
void prt_ctype(void **, int, char *, int);
void prt_strnum(void **, int, char*);

void kprt_int(void **, int, char *);
void kprt_str(void **, int, char *);
void kprt_num(void **, int, char *);
void kprt_slst(void **, int, char *, int);
void kprt_svec(void **, int, char *, int);
void kprt_ctype(void **, int, char *, int);
void kprt_strnum(void **, int, char *);


#define SZ(t)    (sizeof(t) / sizeof(t[0]))

/*
*  These are arbitrary contants that are used for building the keyword
*  tables.  They simply name the index into the arrays at which the keyword
*  is located.
*/
#define CHARMAP                 0
#define LCCHARMAP               0
#define CODE_SET_NAME           1
#define MBCURMAX                2
#define MBCURMIN                3

#define LCCOLLATE               1

#define LCCTYPE                 2

#define LCMESSAGES	        3
#define YESEXPR		        0
#define NOEXPR		        1
#define YESSTR			2
#define NOSTR			3

#define LCMONETARY		4
#define INT_CURR_SYMBOL		0
#define CURRENCY_SYMBOL		1
#define MON_DECIMAL_POINT	2
#define MON_GROUPING		3
#define MON_THOUSANDS_SEP	4
#define POSITIVE_SIGN		5
#define NEGATIVE_SIGN		6
#define INT_FRAC_DIGITS		7
#define FRAC_DIGITS		8
#define P_CS_PRECEDES		9
#define P_SEP_BY_SPACE		10
#define N_CS_PRECEDES		11
#define N_SEP_BY_SPACE		12
#define P_SIGN_POSN		13
#define N_SIGN_POSN		14
#define DEBIT_SIGN		15
#define CREDIT_SIGN		16
#define LEFT_PARENTHESIS	17
#define RIGHT_PARENTHESIS	18

#define LCNUMERIC		5
#define DECIMAL_POINT		0
#define THOUSANDS_SEP		1
#define GROUPING		2

#define LCTIME		        6
#define ABDAY		        0
#define DAY		        1
#define ABMON		        2
#define MON		        3
#define D_T_FMT		        4
#define D_FMT			5
#define T_FMT		        6
#define AM_PM		        7
#define ERA		        8
#define ERA_D_FMT		        9
#define ERA_YEAR		        10
#define T_FMT_AMPM		        11
#define ERA_T_FMT			12
#define ERA_D_T_FMT			13
#define ALT_DIGITS			14

/*
*  An item list for each category.  The item list contains 
*    o a pointer to the function which prints the value of the variable, 
*    o the number of values the variable may assume,
*    o a boolean which indicates if the keyword for the item was on the 
*         command line,
*    o a string defining the keyword,
*    o the offset of the value in the category structure.
*/
typedef struct {
    void          (*(*prt))();
    int           n;
    unsigned char refd;
    char          *name;
    off_t         off;
} item_t;

/*
*  item list for the CHARMAP category 
*/
item_t chr_item_list[] = {
&pstr, 1, FALSE, "charmap",           offsetof(_LC_charmap_t, cm_csname),
&pstr, 1, FALSE, "code_set_name",     offsetof(_LC_charmap_t, cm_csname),
&pint, 1, FALSE, "mb_cur_max",        offsetof(_LC_charmap_t, cm_mb_cur_max),
&pint, 1, FALSE, "mb_cur_min",        offsetof(_LC_charmap_t, cm_mb_cur_min),
};

/*
*  item list for the LC_COLLATE category 
*/
item_t col_item_list[] = {
&prtcollate, 1, FALSE, "", 0,
};

/*
*  item list for the LC_CTYPE category 
*/
item_t ctp_item_list[] = {
&prtctype, 1, FALSE, "", 0,
};

/*
*  item list for the LC_MESSAGES category
*/
item_t msg_item_list[] = {
&pstr, 1, FALSE, "yesexpr",           offsetof(_LC_resp_t, yesexpr),
&pstr, 1, FALSE, "noexpr",            offsetof(_LC_resp_t, noexpr),
&pstr, 1, FALSE, "yesstr",            offsetof(_LC_resp_t, yesstr),
&pstr, 1, FALSE, "nostr",             offsetof(_LC_resp_t, nostr),
};

/*
*  item list for the LC_MONETARY category
*/
item_t mon_item_list[] = {
&pstr, 1,FALSE, "int_curr_symbol",  offsetof(_LC_monetary_t, int_curr_symbol),
&pstr, 1,FALSE, "currency_symbol",  offsetof(_LC_monetary_t, currency_symbol),
&pstr, 1,FALSE, "mon_decimal_point",offsetof(_LC_monetary_t, mon_decimal_point),
&pstrnum, 1,FALSE, "mon_grouping",     offsetof(_LC_monetary_t, mon_grouping),
&pstr, 1,FALSE, "mon_thousands_sep",offsetof(_LC_monetary_t, mon_thousands_sep),
&pstr, 1,FALSE, "positive_sign",    offsetof(_LC_monetary_t, positive_sign),
&pstr, 1,FALSE, "negative_sign",    offsetof(_LC_monetary_t, negative_sign),
&pnum, 1,FALSE, "int_frac_digits",  offsetof(_LC_monetary_t, int_frac_digits),
&pnum, 1,FALSE, "frac_digits",	    offsetof(_LC_monetary_t, frac_digits),
&pnum, 1,FALSE, "p_cs_precedes",    offsetof(_LC_monetary_t, p_cs_precedes),
&pnum, 1,FALSE, "p_sep_by_space",   offsetof(_LC_monetary_t, p_sep_by_space),
&pnum, 1,FALSE, "n_cs_precedes",    offsetof(_LC_monetary_t, n_cs_precedes),
&pnum, 1,FALSE, "n_sep_by_space",   offsetof(_LC_monetary_t, n_sep_by_space),
&pnum, 1,FALSE, "p_sign_posn",      offsetof(_LC_monetary_t, p_sign_posn),
&pnum, 1,FALSE, "n_sign_posn",      offsetof(_LC_monetary_t, n_sign_posn),
&pstr, 1,FALSE, "debit_sign",       offsetof(_LC_monetary_t, debit_sign),
&pstr, 1,FALSE, "credit_sign",      offsetof(_LC_monetary_t, credit_sign),
&pstr, 1,FALSE, "left_parenthesis", offsetof(_LC_monetary_t, left_parenthesis),
&pstr, 1,FALSE, "right_parenthesis",offsetof(_LC_monetary_t, right_parenthesis),
};

/*
*  item list for the LC_NUMERIC category
*/
item_t num_item_list[] = {
&pstr,     1, FALSE, "decimal_point", offsetof(_LC_numeric_t, decimal_point),
&pstr,     1, FALSE, "thousands_sep", offsetof(_LC_numeric_t, thousands_sep),
&pstrnum,  1, FALSE, "grouping",      offsetof(_LC_numeric_t, grouping),
};

/*
*  item list for the LC_TIME category
*/
item_t tim_item_list[] = {
&pslst, 7,  FALSE, "abday",	offsetof(_LC_time_t, abday),
&pslst, 7,  FALSE, "day",	offsetof(_LC_time_t, day),
&pslst, 12, FALSE, "abmon",	offsetof(_LC_time_t, abmon),
&pslst, 12, FALSE, "mon",	offsetof(_LC_time_t, mon),
&pstr,  1,  FALSE, "d_t_fmt",	offsetof(_LC_time_t, d_t_fmt),
&pstr,  1,  FALSE, "d_fmt",	offsetof(_LC_time_t, d_fmt),
&pstr,  1,  FALSE, "t_fmt",	offsetof(_LC_time_t, t_fmt),
&pslst, 2,  FALSE, "am_pm",	offsetof(_LC_time_t, am_pm),
&psvec, 1,  FALSE, "era",	offsetof(_LC_time_t, era),
&pstr,  1,  FALSE, "era_d_fmt",	offsetof(_LC_time_t, era_d_fmt),
&pstr,  1,  FALSE, "era_year",	offsetof(_LC_time_t, era_year),
&pstr,  1,  FALSE, "t_fmt_ampm",offsetof(_LC_time_t, t_fmt_ampm),
&pstr,  1,  FALSE, "era_t_fmt", offsetof(_LC_time_t, era_t_fmt),
&pstr,  1,  FALSE, "era_d_t_fmt", offsetof(_LC_time_t, era_d_t_fmt),
&pstr,  1,  FALSE, "alt_digits", offsetof(_LC_time_t, alt_digits),
};


/*
*  This is the category table.  There is one entry for each category which
*  contains 
*    o the category name, 
*    o two booleans which indicate if the category keyword was referenced
*         and if any item in the category was referenced, 
*    o a pointer to the category pointer for the locale,
*    o the number of locale items in the category
*    o a pointer to the item list for the category.
*/
struct category {
    char          *name;
    unsigned char key_refd;
    unsigned char cat_refd;
    void          **cat_ptr;
    unsigned char n_items;
    item_t        *items;
} cat_list[] = {
"CHARMAP",     FALSE, FALSE, (void **)&__lc_charmap, SZ(chr_item_list), chr_item_list,
"LC_COLLATE",  FALSE, FALSE, (void **)&__lc_charmap, SZ(col_item_list), col_item_list, 
"LC_CTYPE",    FALSE, FALSE, (void **)&__lc_charmap, SZ(ctp_item_list), ctp_item_list,
"LC_MESSAGES", FALSE, FALSE, (void **)&__lc_resp, SZ(msg_item_list), msg_item_list,
"LC_MONETARY", FALSE, FALSE, (void **)&__lc_monetary, SZ(mon_item_list), mon_item_list,
"LC_NUMERIC",  FALSE, FALSE, (void **)&__lc_numeric, SZ(num_item_list), num_item_list,
"LC_TIME",     FALSE, FALSE, (void **)&__lc_time, SZ(tim_item_list), tim_item_list,
};


/*
*  This is the keyword table which is searched for each keyword on the 
*  command line.  The keyword table contains the keyword itself, and a 
*  pointer to the 'referenced' fields of the category and item lists.
*
*  This list is sorted!
*/
struct {
    char *key;
    unsigned char *key_refd;
    unsigned char *cat_refd;
} keylist[]={
"CHARMAP",          
&cat_list[CHARMAP].key_refd,           &cat_list[CHARMAP].cat_refd,
"LC_COLLATE",          
&cat_list[LCCOLLATE].key_refd,         &cat_list[LCCOLLATE].cat_refd,
"LC_CTYPE",          
&cat_list[LCCTYPE].key_refd,           &cat_list[LCCTYPE].cat_refd,
"LC_MESSAGES",          
&cat_list[LCMESSAGES].key_refd,        &cat_list[LCMESSAGES].cat_refd,
"LC_MONETARY",	        
&cat_list[LCMONETARY].key_refd,        &cat_list[LCMONETARY].cat_refd,
"LC_NUMERIC",		
&cat_list[LCNUMERIC].key_refd,	       &cat_list[LCNUMERIC].cat_refd,
"LC_TIME",		
&cat_list[LCTIME].key_refd,            &cat_list[LCTIME].cat_refd,

"abday",		
&tim_item_list[ABDAY].refd,             &cat_list[LCTIME].cat_refd,
"abmon",		
&tim_item_list[ABMON].refd,             &cat_list[LCTIME].cat_refd,
"alt_digits",
&tim_item_list[ALT_DIGITS].refd,	&cat_list[LCTIME].cat_refd,
"am_pm",		
&tim_item_list[AM_PM].refd,             &cat_list[LCTIME].cat_refd,
"charmap",
&chr_item_list[LCCHARMAP].refd,         &cat_list[CHARMAP].cat_refd,
"code_set_name",
&chr_item_list[CODE_SET_NAME].refd,     &cat_list[CHARMAP].cat_refd,
"credit_sign",		
&mon_item_list[CREDIT_SIGN].refd,       &cat_list[LCMONETARY].cat_refd,
"currency_symbol",	
&mon_item_list[CURRENCY_SYMBOL].refd,   &cat_list[LCMONETARY].cat_refd,
"d_fmt",
&tim_item_list[D_FMT].refd,		&cat_list[LCTIME].cat_refd,
"d_t_fmt",		
&tim_item_list[D_T_FMT].refd,           &cat_list[LCTIME].cat_refd,
"day",		 	
&tim_item_list[DAY].refd,               &cat_list[LCTIME].cat_refd,
"debit_sign",		
&mon_item_list[DEBIT_SIGN].refd,        &cat_list[LCMONETARY].cat_refd,
"decimal_point",	
&num_item_list[DECIMAL_POINT].refd,     &cat_list[LCNUMERIC].cat_refd,
"era",		 	
&tim_item_list[ERA].refd,               &cat_list[LCTIME].cat_refd,
"era_d_fmt",		 	
&tim_item_list[ERA_D_FMT].refd,         &cat_list[LCTIME].cat_refd,
"era_d_t_fmt",
&tim_item_list[ERA_D_T_FMT].refd,	&cat_list[LCTIME].cat_refd,
"era_t_fmt",
&tim_item_list[ERA_T_FMT].refd,		&cat_list[LCTIME].cat_refd,
"era_year",		 	
&tim_item_list[ERA_YEAR].refd,          &cat_list[LCTIME].cat_refd,
"frac_digits",
&mon_item_list[FRAC_DIGITS].refd,	&cat_list[LCMONETARY].cat_refd,
"grouping",		
&num_item_list[GROUPING].refd,          &cat_list[LCNUMERIC].cat_refd,
"int_curr_symbol",	
&mon_item_list[INT_CURR_SYMBOL].refd,   &cat_list[LCMONETARY].cat_refd,
"int_frac_digits",	
&mon_item_list[INT_FRAC_DIGITS].refd,   &cat_list[LCMONETARY].cat_refd,
"left_parenthesis",	
&mon_item_list[LEFT_PARENTHESIS].refd,  &cat_list[LCMONETARY].cat_refd,
"mb_cur_max",
&chr_item_list[MBCURMAX].refd,          &cat_list[CHARMAP].cat_refd,
"mb_cur_min",
&chr_item_list[MBCURMIN].refd,          &cat_list[CHARMAP].cat_refd,
"mon",		
&tim_item_list[MON].refd,               &cat_list[LCTIME].cat_refd,
"mon_decimal_point",	
&mon_item_list[MON_DECIMAL_POINT].refd, &cat_list[LCMONETARY].cat_refd,
"mon_grouping",		
&mon_item_list[MON_GROUPING].refd,      &cat_list[LCMONETARY].cat_refd,
"mon_thousands_sep",	
&mon_item_list[MON_THOUSANDS_SEP].refd, &cat_list[LCMONETARY].cat_refd,
"n_cs_precedes",	
&mon_item_list[N_CS_PRECEDES].refd,     &cat_list[LCMONETARY].cat_refd,
"n_sep_by_space",	
&mon_item_list[N_SEP_BY_SPACE].refd,    &cat_list[LCMONETARY].cat_refd,
"n_sign_posn",		
&mon_item_list[N_SIGN_POSN].refd,       &cat_list[LCMONETARY].cat_refd,
"negative_sign",	
&mon_item_list[NEGATIVE_SIGN].refd,     &cat_list[LCMONETARY].cat_refd,
"noexpr",		
&msg_item_list[NOEXPR].refd,            &cat_list[LCMESSAGES].cat_refd,
"nostr",		
&msg_item_list[NOSTR].refd,             &cat_list[LCMESSAGES].cat_refd,
"p_cs_precedes",	
&mon_item_list[P_CS_PRECEDES].refd,     &cat_list[LCMONETARY].cat_refd,
"p_sep_by_space",	
&mon_item_list[P_SEP_BY_SPACE].refd,    &cat_list[LCMONETARY].cat_refd,
"p_sign_posn",		
&mon_item_list[P_SIGN_POSN].refd,       &cat_list[LCMONETARY].cat_refd,
"positive_sign",	
&mon_item_list[POSITIVE_SIGN].refd,     &cat_list[LCMONETARY].cat_refd,
"right_parenthesis",	
&mon_item_list[RIGHT_PARENTHESIS].refd, &cat_list[LCMONETARY].cat_refd,
"t_fmt",		
&tim_item_list[T_FMT].refd,             &cat_list[LCTIME].cat_refd,
"t_fmt_ampm",		
&tim_item_list[T_FMT_AMPM].refd,        &cat_list[LCTIME].cat_refd,
"thousands_sep",	
&num_item_list[THOUSANDS_SEP].refd,     &cat_list[LCNUMERIC].cat_refd,
"yesexpr",	 	
&msg_item_list[YESEXPR].refd,           &cat_list[LCMESSAGES].cat_refd,
"yesstr",	 	
&msg_item_list[YESSTR].refd,            &cat_list[LCMESSAGES].cat_refd,
};

static int err_count = 0;

void main(int argc, char *argv[])
{
    int show_cat = FALSE;
    int show_kwd = FALSE;
    int c;
    int i, j;

    char *locname;
    
    locname = "";
    if (argc==1) {
	dump_names();
	exit(0);
    }

    while((c=getopt(argc, argv, "amck")) != EOF) {
	switch (c) {
	  case 'a':
	    list_locales();
	    exit(0);
	    break;
	  case 'm':
	    list_charmaps();
	    exit(0);
	    break;
	  case 'c':
	    show_cat = TRUE;
	    break;
	  case 'k':
	    show_kwd = TRUE;
	    break;
	  default:
	    usage();
	}
    }

    if ((show_kwd || show_cat) && !argv[optind]) /* keyword is required */
	usage();

    for(i=optind; i < argc; i++)
	mark_keyword(argv[i]);

    setlocale(LC_ALL, locname);

    if (show_kwd) {
	pint = kprt_int;
	pstr = kprt_str;
	pnum = kprt_num;
	pslst = kprt_slst;
	psvec = kprt_svec;
	prtcollate = prt_collate;
	prtctype = kprt_ctype;
	pstrnum = kprt_strnum;
    } else {
	pint = prt_int;
	pstr = prt_str;
	pnum = prt_num;
	pslst = prt_slst;
	psvec = prt_svec;
	prtcollate = prt_collate;
	prtctype = prt_ctype;
	pstrnum = prt_strnum;
    }

    for (i=0; i<SZ(cat_list); i++) {
	if (cat_list[i].cat_refd) {
	    if (show_cat) 
		printf("%s\n", cat_list[i].name);
	    
	    /* category name was used as keyword - print entire category 
	     */
	    if (cat_list[i].key_refd) {
		for (j=0; j<cat_list[i].n_items; j++)
		    (*(*cat_list[i].items[j].prt))
			(cat_list[i].cat_ptr, 
			 cat_list[i].items[j].off, 
			 cat_list[i].items[j].name, 
			 cat_list[i].items[j].n);
	    } else if (cat_list[i].cat_refd) {
		/* category name was not used print only items specified 
		 */
		for (j=0; j<cat_list[i].n_items; j++)
		    if (cat_list[i].items[j].refd) 
			(*(*cat_list[i].items[j].prt))
			    (cat_list[i].cat_ptr, 
			     cat_list[i].items[j].off, 
			     cat_list[i].items[j].name,
			     cat_list[i].items[j].n);
	    }		
	}
    }

    if (err_count) 
	exit(1);
    else
	exit(0);
}	


/*
*  FUNCTION: dump_names
*
*  DESCRIPTION:
*  Print out effective locale names for the various categories.
*/
void dump_names(void)
{
    static const char *category_name[]={
	"LC_COLLATE", "LC_CTYPE", "LC_MONETARY", "LC_NUMERIC", 
	"LC_TIME",  "LC_MESSAGES"
	};
    
    int  i;
    char *s;
    char *lc_all;
    char *lang;

    lc_all = getenv("LC_ALL");
    if (lc_all == NULL) lc_all="";

    lang = getenv("LANG");
    if (lang == NULL) lang = "";

    s = setlocale(LC_ALL, "");

    if (s == NULL)
	s = setlocale(LC_ALL,NULL);

    /*
      LANG environment variable is always written out "as-is"
    */
    printf("LANG=%s\n", lang);

    for (i=0; i<=LC_MESSAGES; i++) {
	char *envvar;
	char *effval;

	envvar = getenv(category_name[i]);
	effval = strtok(s, " ");
	if (effval == NULL) 
	    effval = "";

	if (envvar == NULL)
	    printf("%s=\"%s\"\n", category_name[i], effval);
	else {

	    if (strcmp(envvar, effval) != 0)
		printf("%s=\"%s\"\n", category_name[i], effval);
	    else
		printf("%s=%s\n", category_name[i], effval);
	}

	s = NULL;
    }

    printf("LC_ALL=%s\n", lc_all);
}


/*
*  FUNCTION: list_dir
*
*  DESCRIPTION:
*  Opens a directory, calls the user-supplied function for each file
*  in the directory, and then closes the directory.  Returns 0 if
*  successful, returns -1 if any errors.
*/
int list_dir(const char *dirname, int (*fn)(const char *, const struct stat *))
{
    DIR *dirp;
    int dirnamelen;
    struct dirent *entry;
    char path[PATH_MAX + 1];
    struct stat statbuf;
    int status;

    dirp = opendir(dirname);
    if (dirp == NULL)
	return -1;

    /*
     * Is there room to append a '/' to the directory name?
     */
    dirnamelen = strlen(dirname);
    if ((dirnamelen + 1) >= sizeof path) {
	(void) closedir(dirp);
	return -1;
    }

    (void) strcpy(path, dirname);
    path[dirnamelen++] = '/';
    path[dirnamelen] = '\0';

    status = 0;
    while((entry = readdir(dirp)) != NULL) {
	if (strcmp(entry->d_name, ".")  == 0 ||
	    strcmp(entry->d_name, "..") == 0)
	    continue;

	if ((dirnamelen + strlen(entry->d_name)) >= sizeof path) {
	    status = -1;
	    continue;
	}

	(void) strcpy(path + dirnamelen, entry->d_name);

	if (stat(path, &statbuf)) {
	    status = -1;
	    continue;
	}

	(void) (*fn)(path, &statbuf);
    }

    (void) closedir(dirp);

    return status;
}


/*
*  FUNCTION: is_locale
*
*  DESCRIPTION:
*  Returns 1 if the file is a locale, returns 0 if not.
*
*  This is basically __lc_load() from libc with a few small changes.
*  It would have been simpler just to call "setlocale(LC_ALL, path)"
*  here and check the return value.  But that doesn't work when one
*  locale is a link to another.  In that case, setlocale() fails to
*  load the second locale.  That's due to a loader bug (QAR 17062).
*  We work around the bug by opening a locale just like setlocale(),
*  but then closing the locale before trying to open the next one.  
*/
int is_locale(const char *path)
{
    /* void * (*p)();  */
    _LC_object_t * (*p)();
    void *handle;
    _LC_object_t * q;
    int is = 0;

    /* load specified object */
    /* p = load(path, 0, ""); */
    handle = dlopen((char *)path, RTLD_LAZY);
    if (handle == NULL)
        return NULL;
    p = (_LC_object_t * (*)()) dlsym(handle, "instantiate");

    /*
      If load() succeeded,
         execute the method pointer which was returned.
         return the return value of the method.
      else
         return 0.
    */
    if (p != NULL) {

        /* invoke object instantiation method */
        q = (_LC_object_t *)(*p)(path);

        /* verify that what was returned was actually a locale */
        is = (q->magic == _LC_MAGIC) && (q->type_id == _LC_LOCALE);

	(void) dlclose(handle);
    }

    return is;
}


/*
*  FUNCTION: check_locale
*
*  DESCRIPTION:
*  Checks to see if the file is a locale, and if it is, prints the
*  name. 
*/
int check_locale(const char *name, const struct stat *s)
{
    if (S_ISREG(s->st_mode) && is_locale(name)) {
	const char *p = strrchr(name,'/');

	if (*p == '/')
	    p++; 			/*get rid of / */
	printf("%s\n", p);
    }

    return 0;
}


/*
*  FUNCTION: list_locales
* 
*  DESCRIPTION:
*  Locate and display all of the locally available locales on the system.
*  This is accomplished by searching the LOCPATH for locale objects.
*/
void list_locales(void)
{
    char *path;
    char *epath;
    char *locpath;

    locpath = getenv("LOCPATH");
    if (locpath == NULL || locpath[0] == '\0')
	locpath = DEF_LOCPATH;

    path = malloc(strlen(locpath)+1);
    if (path==NULL) exit(-1);
    strcpy(path, locpath);

    printf("POSIX\n");		/* Present by default */

    for (; path != NULL; path = epath) {
	epath = strchr(path, ':');
	if (epath != NULL) {
	    *epath = '\0';
	    epath++;
	}
	list_dir(path, check_locale);
    }
}


/*
*  FUNCTION: check_charmap
*
*  DESCRIPTION:
*  Checks to see if the file is a regular file or not.
*/
int check_charmap(const char *name, const struct stat *s)
{
    if (S_ISREG(s->st_mode)) {
	const char *p = strrchr(name,'/');

	if (*p == '/')
	    p++; 			/*get rid of / */
	printf("%s\n", p);
    }

    return 0;
}


void list_charmaps()
{
    char *path = DEF_CHARMAP;

    list_dir(path, check_charmap);
}


/* array of default messages and err ids */
#include "locale_msg.h"
char *err_fmt[]={
    "",
    "usage: locale [-amck] keyword ...\n",
    "locale: unrecognized keyword, '%s', in argument list.\n"
};


/*
*  FUNCTION: error
*
*  DESCRIPTION:
*  This function prints error messages given an error number 'err'.  
*  The routine accepts a variable number of arguments and all errors
*  are non-fatal, i.e. the routine returns regardless of the error
*  number.
*/
void error(int err, ...)
{
    nl_catd catd;
    va_list ap;

    catd = catopen(MF_LOCALE, NL_CAT_LOCALE);

    va_start(ap, err);

    vfprintf(stderr, 
	     catgets(catd, LOCALE, err, err_fmt[err]), 
	     ap);
    err_count++;
}


/*
*  FUNCTION: mark_keyword
*
*  DESCRIPTION: 
*  This routine searches the keyword table for 'key'.  If the keyword
*  is found, it marks the keyword as 'referenced', and the category in
*  which the keyword is located as 'referenced'.  
*
*  A binary search algorithm is used to locate the keyword
*/
void mark_keyword(char *key)
{
    int c;
    int middle;
    int top;
    int bottom;
    
    bottom = 0;
    top    = SZ(keylist) - 1;

    do {
	middle = ((top - bottom) / 2) + bottom;
	c=strcmp(key, keylist[middle].key);

	if (c == 0) {
	    *(keylist[middle].key_refd) = TRUE;
	    *(keylist[middle].cat_refd) = TRUE;
	    return;
	} else if (c < 0)
	    top = middle -1;
	else
	    bottom = middle +1;

    } while (top >= bottom);

    error(ERR_KEYWORD, key);
}


/*
*  FUNCTION: usage
*
*  DESCRIPTION:
*  Prints a usage statement for the command.
*/
void usage(void)
{
    nl_catd catd;

    catd = catopen(MF_LOCALE,NL_CAT_LOCALE);
    printf(catgets(catd, LOCALE, ERR_USAGE, err_fmt[ERR_USAGE]));
    exit(1);
}


/*
*  FUNCTION: print_string
*
*  DESCRIPTION:
*  Prints out a string as per the POSIX "escaped character" requirements
*/
void print_string(char *s)
{
    wchar_t pc;
    int     rc;

    while (*s != '\0') {
	rc = mbtowc(&pc, s, MB_CUR_MAX);
	if (rc < 0)
	    s++;
	else if (rc == 1) {
	    /* check for escape characters */
	    switch (*s) {
	      case '\\':
	      case ';':
	      case '"':
		putchar('\\');
		putchar(*s++);
		break;
	      default:
		if (iscntrl(*s)) 
		    printf("\\x%02x", *s++);
		else
		    putchar(*s++);
	    }

	} else 
	    for (; rc > 0; rc--)
		putchar(*s++);
    }
}


/*
*  FUNCTION: kprt_str
* 
*  DESCRIPTION: 
*  Prints the value of a string locale variable in the format specified
*  by POSIX when the -k flag is specified.  The address of this string 
*  is at offset p2 from *p1.
*
*  The format is "%s=\"%s\"\n", <keyword>, <value>
*/
void kprt_str(void **p1, int p2, char *name)
{
    char *s = *(char **)((char *)*p1+p2);

    printf("%s=\"", name);
    
    print_string(s);
    putchar('"');
    putchar('\n');
}


/*
*  FUNCTION: kprt_num
* 
*  DESCRIPTION: 
*  Prints the value of a numeric locale variable in the format specified
*  by POSIX when the -k flag is specified.  The address of this number is
*  at offset p2 from *p1.  The data being printed is a signed char.
*
*  The format is "%s=%d\n", <keyword>, <value>
*/
void kprt_num(void **p1, int p2, char *name)
{
    printf("%s=%d\n", name, *(signed char *)((char *)*p1+p2));
}


/*
*  FUNCTION: kprt_int
* 
*  DESCRIPTION: 
*  Prints the value of a numeric locale variable in the format specified
*  by POSIX when the -k flag is specified.  The address of this number is
*  at offset p2 from *p1.  The data type being printed is an integer.
*
*  The format is "%s=%d\n", <keyword>, <value>
*/
void kprt_int(void **p1, int p2, char *name)
{
    printf("%s=%d\n", name, *(int *)((char *)*p1+p2));
}


/*
*  FUNCTION: kprt_slst
* 
*  DESCRIPTION: 
*  Prints the list of strings for locale variables which take mulitple
*  string values, such as abday.  This format has not been specified by
*  POSIX, so this format is just a guess (probably wrong).
*
*  The format is "%s=\"%s\";...;\"%s\"\n", <keyword>, <value>, ..., <value>
*/
void kprt_slst(void **p1, int p2, char *name, int n)
{
    char **s;
    int  i;

    s = (char **)((char *)*p1+p2);
    printf("%s=", name);
    for (i=0; i<n; i++) {
	if (i != 0) 
	    putchar(';');

	putchar('"');
	print_string(s[i]);
	putchar('"');
    }
    putchar('\n');
}

/*
*  FUNCTION: kprt_svec
*
*  DESCRIPTION:
*  Just like the kprt_slst, but the vector is a NULL terminated array
*  with a pointer stored at the offset.
*/
void kprt_svec(void **p1, int p2, char *name, int n)
{
    char **s;

    s = * (char***)((char *)*p1+p2);
    printf("%s=", name);
    while(s && s[0]) {
	putchar('"');
	print_string(s[0]);
	putchar('"');
	s++;
	if (*s) putchar(';');
    }
    putchar('\n');
}

/*
*  FUNCTION: prt_str
* 
*  DESCRIPTION: 
*  This is the no-keyword version of the kprt_str function.
* 
*  The format is "%s\n", <value>
*/
void prt_str(void **p1, int p2, char *name)
{
    char *s = *(char **)((char *)*p1+p2);

    print_string(s);
    putchar('\n');
}


/*
*  FUNCTION: prt_int
* 
*  DESCRIPTION: 
*  Prints the value of a numeric locale variable in the format specified
*  by POSIX when the -k flag is specified.  The address of this number is
*  at offset p2 from *p1.  The data type being printed is an integer.
*
*  The format is "%s=%d\n", <keyword>, <value>
*/
void prt_int(void **p1, int p2, char *name)
{
    printf("%d\n", *(int *)((char *)*p1+p2));
}


/*
*  FUNCTION: prt_num
* 
*  DESCRIPTION: 
*  This is the no-keyword version of the kprt_num function.
* 
*  The format is "%d\n", <value>
*/
void prt_num(void **p1, int p2, char *name)
{
    
    printf("%d\n", *(signed char *)((char *)*p1+p2));
}


/*
*  FUNCTION: prt_slst
* 
*  DESCRIPTION: 
*  This is the no-keyword version of the kprt_slst function.
* 
*  The format is "\"%s\";...;\"%s\"\n", <value>, ..., <value>
*/
void prt_slst(void **p1, int p2, char *name, int n)
{
    char **s;
    int  i;

    s = (char **)((char *)*p1+p2);
    for (i=0; i<n; i++) {
	if (i != 0) 
	    putchar(';');

	putchar('"');
	print_string(s[i]);
	putchar('"');
    }
    putchar('\n');
}

void prt_svec(void **p1, int p2, char *name, int n)
{
    char **s = * (char ***)((char *)*p1+p2);

    while (s && *s) {
	putchar('"');
	print_string(*s);
	putchar('"');
	s++;
	if (*s)
	    putchar(';');
    }
    putchar('\n');
}
	
/*
*  FUNCTION: prt_collate
* 
*  DESCRIPTION:
*  Stub for the collate class which does nothing. 
*/
void prt_collate(void **p1, int p2, char *name, int n)
{
}


/*
*  FUNCTION: prt_ctype
* 
*  DESCRIPTION:
*  Prints out a list of the character classes defined for this locale.
*/
void prt_ctype(void **p1, int p2, char *name, int n)
{
    extern _LC_ctype_t *__lc_ctype;
    int i;   
    
    for (i=0; i < __lc_ctype->nclasses; i++)
	printf("%s\n", __lc_ctype->classnms[i].name);
}


/*
*  FUNCTION: kprt_ctype
* 
*  DESCRIPTION:
*  Prints out a list of the character classes defined for this locale.
*/
void kprt_ctype(void **p1, int p2, char *name, int n)
{
    extern _LC_ctype_t *__lc_ctype;
    int i;   
    
    for (i=0; i < __lc_ctype->nclasses; i++)
	printf("%s=0x%04X\n", 
	       __lc_ctype->classnms[i].name,
	       __lc_ctype->classnms[i].mask);
}

/*
*  FUNCTION: prt_strnum
* 
*  DESCRIPTION:
*  Prints out a semi-colon separated list of numbers for the grouping
*  keyword (LC_NUMERIC and LC_MONETARY). These numbers are stored as
*  a string. This string is NULL terminated. If the character CHAR_MAX is
*  reached before the string terminates, then there is no "repeat" (0)
*  then no "0" is printed at the end of the list
*/
void prt_strnum(void **p1,int p2,char *name)
{
    char *s = *(char **)((char *)*p1 + p2);
    int     i;

    i = 0;
    while ((*s != '\0')&& (*s != CHAR_MAX)) {
        if (i == 0)
	    i++;
	else
	    printf(";");
	printf("%d", *s++);
    }
    if (*s == '\0' && i != 0) 
	printf(";0");
    printf("\n");
}

/*
*  FUNCTION: kprt_strnum
* 
*  DESCRIPTION:
*  Prints out a semi-colon separated list of numbers for the grouping
*  keyword (LC_NUMERIC and LC_MONETARY). These numbers are stored as
*  a string. This string is NULL terminated. If the character CHAR_MAX is
*  reached before the string terminates, then there is no "repeat" (0)
*  then no "0" is printed at the end of the list
*/
void kprt_strnum(void **p1,int p2,char *name)
{
    char *s = *(char **)((char *)*p1 + p2);
    int     i;

    i = 0;
    printf("%s=",name);
    printf("\"");
    while ((*s != '\0') && (*s != CHAR_MAX)) {
	    if (i == 0) 
		i++;
	    else
		printf(";");
	    printf("%d", *s++);
    }
    if (*s == '\0' && i != 0)
	printf(";0");
    printf("\"\n");
}
