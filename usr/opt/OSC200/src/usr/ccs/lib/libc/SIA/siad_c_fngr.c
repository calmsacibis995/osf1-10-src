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
static char *rcsid = "@(#)$RCSfile: siad_c_fngr.c,v $ $Revision: 1.1.12.4 $ (DEC) $Date: 1993/08/04 21:22:03 $";
#endif
/*****************************************************************************
*
*	int	siad_chg_finger((*sia_collect),username)
*
* Description: The purpose of this routine is to perform a change finger
* function.
*
* Returns: 
*		SAIDSUCCESS => CONTINUE
*
*		SIADFAIL => DO NOT CONTINUE 
******************************************************************************/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_SHARED_LIBRARIES) || (!defined(_SHARED_LIBRARIES) && !defined(_THREAD_SAFE))
#pragma weak siad_chg_finger = __siad_chg_finger
#endif
#endif
#include "siad.h"
#include "siad_bsd.h"
extern struct passwd siad_bsd_passwd;
extern char siad_bsd_getpwbuf[];

#include <sys/secdefines.h>
#include <stdio.h>
#include <ndbm.h>
#include <ctype.h>
#include <paths.h>
#include <limits.h>

#ifdef NLS
#include <locale.h>
#endif

#undef	MSGSTR
#define	MSGSTR(n,s)	GETMSGSTR(MS_BSDPASSWD,(n),(s))

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif


/*
 * This should be the first thing returned from a getloginshells()
 * but too many programs know that it is /sbin/sh.
 *
 * Note:  The default shell as determined by <paths.h>, and as used in
 *        login(1) is /bin/sh, not /sbin/sh.  /sbin/sh might not be in
 *        /etc/shells.  -dlong 91/10/8
 */

#define	DEFSHELL	_PATH_BSHELL

#define	PASSWD		"/etc/passwd"
#define	PTEMP		"/etc/ptmp"
#define	EOS		'\0';
#define MAX_PWD_LENGTH  16
#define progname	"passwd"

siad_chg_finger(collect,username,argc,argv)
int (*collect)();
char *username;
int     argc;
char    *argv[];
{
	struct passwd   *pwd;
	extern	struct passwd   *getpwnam();
        FILE    *tf;
        FILE    *passfp;
        DBM     *dp;
        uid_t   uid, getuid();
        uid_t   euid, geteuid();
        int     i, acctlen, ch, fd, dochfn, dochsh;
        char    *p, *str, *cp, *umsg,
                *getfingerinfo(), *getloginshell(), *getnewpasswd();
#ifdef NLS
        (void) setlocale( LC_ALL, "" );
#endif
	/****** Check if authorized calling program *******/
	if((euid=geteuid()) != 0)
		return(SIADFAIL);
	/****** Check if user is registered with proper uid *****/
	uid=getuid();
	pwd=NULL;
	if(username == NULL)
		username=getlogin();
	if(bsd_siad_getpwnam(username,&siad_bsd_passwd, &siad_bsd_getpwbuf,SIABUFSIZ) == SIADSUCCESS)
		pwd = &siad_bsd_passwd;
	if(pwd == NULL) /* should be siad_getpwnam call */
		{
		sia_warning(collect,MSGSTR(UNKUID, "%s: %u: unknown user uid.\n"), progname, uid);
                return(SIADFAIL);
		}
	if((uid != 0) && (uid != pwd->pw_uid))
		{
		sia_warning(collect,MSGSTR(PERMDEN1, "Permission denied.\n"));
		return(SIADFAIL);
		}
	/*printf(MSGSTR(CHANG, "Changing %s for %s.\n"));*/
	/********************************************/
	/* collecting and checking the new password */
	/********************************************/
	if((cp = getfingerinfo((collect),pwd, uid)) == NULL)
		return(SIADFAIL);
	if(bsd_chg_it((collect),pwd,uid,cp,CHGFINGER) == SIADFAIL)
		return(SIADFAIL);
	else	return(SIADSUCCESS);
}

#define FINGBUFSIZ 100

static struct default_values {
	char *name;
	char *office_num;
	char *office_phone;
	char *home_phone;
};

/*
 * Get name, room number, school phone, and home phone.
 */
static char *
getfingerinfo(collect,pwd,u)
	int (*collect)();
	struct passwd *pwd;
	uid_t u;
{
	char in_str[FINGBUFSIZ+1];
	struct default_values *defaults, *get_defaults();
	static char answer[4*(FINGBUFSIZ+1)];
	static char pmptstr[FINGBUFSIZ+1];
	prompt_t askforpass; /* 0 prompt for collecting shell */
        int             timeout=0;
        int             rendition=SIAONELINER;
        unsigned char   *title=NULL;
        int             num_prompts=1;
        askforpass.result= (uchar *)&in_str;
        askforpass.max_result_length=FINGBUFSIZ;
        askforpass.min_result_length=0;
        askforpass.control_flags = SIAPRINTABLE;
	askforpass.prompt = (uchar *)&pmptstr;
	answer[0] = '\0';
	defaults = get_defaults((collect),pwd->pw_gecos);
	sia_warning((collect), MSGSTR(DEF_VALUE, "Default values are printed inside of '[]'."));
	sia_warning((collect), MSGSTR(ACCEPT, "To accept the default, type <return>."));
	sia_warning((collect), MSGSTR(ENTRY1, "To have a blank entry, type the word 'none'."));
	/*
	 * Get name.
	 */
	do {
		sprintf(askforpass.prompt, MSGSTR(NAME, "\nName [%s]: "), defaults->name);
		if((*collect)(timeout,rendition,title,num_prompts,&askforpass) != SIACOLSUCCESS)
                        {
                        return(NULL);
                        }
		if (special_case(in_str, defaults->name)) 
			break;
	} while (illegal_input((collect),in_str));
	(void) strcpy(answer, in_str);
	/*
	 * Get room number.
	 */
	do {
		sprintf(askforpass.prompt, MSGSTR(ROOM, "Room number (Exs: 597E or 197C) [%s]: "), defaults->office_num);
		 if((*collect)(timeout,rendition,title,num_prompts,&askforpass) != SIACOLSUCCESS)
			{
                        return(NULL);
                        }
		if (special_case(in_str, defaults->office_num))
			break;
	} while (illegal_input((collect),in_str) || illegal_building(in_str));
	(void) strcat(strcat(answer, ","), in_str);
	/*
	 * Get office phone number.
	 * Remove hyphens.
	 */
	do {
		sprintf(askforpass.prompt, MSGSTR(PHONE1, "Office Phone (Ex: 6426000) [%s]: "), defaults->office_phone);
                 if((*collect)(timeout,rendition,title,num_prompts,&askforpass) != SIACOLSUCCESS)
                        {
                        return(NULL);
                        }
		if (special_case(in_str, defaults->office_phone))
			break;
		remove_hyphens(in_str);
	} while (illegal_input((collect),in_str) || not_all_digits((collect),in_str));
	(void) strcat(strcat(answer, ","), in_str);
	/*
	 * Get home phone number.
	 * Remove hyphens if present.
	 */
	do {
		sprintf(askforpass.prompt, MSGSTR(PHONE2, "Home Phone (Ex: 9875432) [%s]: "), defaults->home_phone);
                 if((*collect)(timeout,rendition,title,num_prompts,&askforpass) != SIACOLSUCCESS)
                        {
                        return(NULL);
                        }
		if (special_case(in_str, defaults->home_phone))
			break;
		remove_hyphens(in_str);
	} while (illegal_input((collect),in_str) || not_all_digits((collect),in_str));
	(void) strcat(strcat(answer, ","), in_str);
	if (strcmp(answer, pwd->pw_gecos) == 0) {
		sia_warning((collect),MSGSTR(FINGERINFO, "Finger information unchanged."));
		exit(1);
	}
	return (answer);
}

/*
 * Prints an error message if a ':', ',' or a newline is found in the string.
 * A message is also printed if the input string is too long.  The password
 * file uses :'s as separators, and are not allowed in the "gcos" field;
 * commas are used as separators in the gcos field, so are disallowed.
 * Newlines serve as delimiters between users in the password file, and so,
 * those too, are checked for.  (I don't think that it is possible to
 * type them in, but better safe than sorry)
 *
 * Returns '1' if a colon, comma or newline is found or the input line is
 * too long.
 */
static
illegal_input(collect,input_str)
	int (*collect)();
	char *input_str;
{
	char *ptr;
	int error_flag = 0;
	int length = strlen(input_str);

	if (strpbrk(input_str, ",:")) {
		sia_warning((collect), MSGSTR(BADCHARS, "':' and ',' are not allowed."));
		error_flag = 1;
	}
	/*
	 * Don't allow control characters, etc in input string.
	 */
	for (ptr = input_str; *ptr; ptr++)
		if (!isprint(*ptr)) {
			sia_warning((collect),MSGSTR(CTRCHARS, "Control characters are not allowed."));
			error_flag = 1;
			break;
		}
	return (error_flag);
}

/*
 * Removes '-'s from the input string.
 */
static
remove_hyphens(str)
	char *str;
{
	char *hyphen;

	while ((hyphen = index(str, '-')) != NULL)
		(void) strcpy(hyphen, hyphen+1);
}

/*
 *  Checks to see if 'str' contains only digits (0-9).  If not, then
 *  an error message is printed and '1' is returned.
 */
static
not_all_digits(collect,str)
	int (*collect)();
	register char *str;
{
	if(!strcmp(str,MSGSTR(NONE, "none")))
		return(0);

	for (; *str; ++str)
		if (!isdigit(*str)) {
			sia_warning((collect), MSGSTR(PHONEMSG, "Phone numbers may only contain digits."));
			return(1);
		}
	return(0);
}

/*
 * Deal with Berkeley buildings.  Abbreviating Cory to C and Evans to E.
 * Correction changes "str".
 *
 * Returns 1 if incorrect room format.
 * 
 * Note: this function assumes that the newline has been removed from str.
 */
static
illegal_building(str)
	register char *str;
{
	int length = strlen(str);
	register char *ptr;

	/*
	 * If the string is [Ee]vans or [Cc]ory or ends in
	 * [ \t0-9][Ee]vans or [ \t0-9M][Cc]ory, then contract the name
	 * into 'E' or 'C', as the case may be, and delete leading blanks.
	 */
	if (length >= 5 && strcmp(ptr = str + length - 4, "vans") == 0 &&
	    (*--ptr == 'e' || *ptr == 'E') &&
	    (--ptr < str || isspace(*ptr) || isdigit(*ptr))) {
		for (; ptr > str && isspace(*ptr); ptr--)
			;
		ptr++;
		*ptr++ = 'E';
		*ptr = '\0';
	} else
	if (length >= 4 && strcmp(ptr = str + length - 3, "ory") == 0 &&
	    (*--ptr == 'c' || *ptr == 'C') &&
	    (--ptr < str || *ptr == 'M' || isspace(*ptr) || isdigit(*ptr))) {
		for (; ptr > str && isspace(*ptr); ptr--)
			;
		ptr++;
		*ptr++ = 'C';
		*ptr = '\0';
	}
	return (0);
}

/*
 * get_defaults picks apart "str" and returns a structure points.
 * "str" contains up to 4 fields separated by commas.
 * Any field that is missing is set to blank.
 */
static struct default_values *
get_defaults(collect,str)
	int (*collect)();
	char *str;
{
	struct default_values *answer;

	answer = (struct default_values *)
		malloc((unsigned)sizeof(struct default_values));
	if (answer == (struct default_values *) NULL) {
		sia_warning((collect), MSGSTR(ALLOCATE, "\nUnable to allocate storage in get_defaults!\n"), stderr);
		exit(1);
	}
	/*
	 * Values if no corresponding string in "str".
	 */
	answer->name = str;
	answer->office_num = "";
	answer->office_phone = "";
	answer->home_phone = "";
	str = index(answer->name, ',');
	if (str == 0) 
		return (answer);
	*str = '\0';
	answer->office_num = str + 1;
	str = index(answer->office_num, ',');
	if (str == 0) 
		return (answer);
	*str = '\0';
	answer->office_phone = str + 1;
	str = index(answer->office_phone, ',');
	if (str == 0) 
		return (answer);
	*str = '\0';
	answer->home_phone = str + 1;
	return (answer);
}

/*
 *  special_case returns true when either the default is accepted
 *  (str = '\n'), or when 'none' is typed.  'none' is accepted in
 *  either upper or lower case (or any combination).  'str' is modified
 *  in these two cases.
 */
static
special_case(str,default_str)
	char *str, *default_str;
{
	char *ptr, *wordptr;

	/*
	 *  If the default is accepted, then change the old string do the 
	 *  default string.
	 */
	if ((*str == '\n') || (*str == '\0')) {
		(void) strcpy(str, default_str);
		return (1);
	}
	/*
	 *  Check to see if str is 'none'.  (It is questionable if case
	 *  insensitivity is worth the hair).
	 */
	wordptr = MSGSTR(NONE, "none\n") - 1;
	for (ptr = str; *ptr != '\0'; ++ptr) {
		++wordptr;
		if (*wordptr == '\0')	/* then words are different sizes */
			return (0);
		if (*ptr == *wordptr)
			continue;
		if (isupper(*ptr) && (tolower(*ptr) == *wordptr))
			continue;
		/*
		 * At this point we have a mismatch, so we return
		 */
		return (0);
	}
	/*
	 * Make sure that words are the same length.
	 */
	if (*(wordptr+1) != '\0')
		return (0);
	/*
	 * Change 'str' to be the null string
	 */
	*str = '\0';
	return (1);
}
