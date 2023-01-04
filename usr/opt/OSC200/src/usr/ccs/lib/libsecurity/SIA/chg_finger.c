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
static char *rcsid = "@(#)$RCSfile: chg_finger.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/04/01 20:20:03 $";
#endif

#include <sia.h>
#include <siad.h>
#include <sia_mech.h>

#include <sys/secdefines.h>
#include <sys/security.h>
#include <prot.h>

#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <strings.h>
#include <ctype.h>
#include <paths.h>
#include <limits.h>

#define BUFSIZE 256

struct default_values {
	char *name;
	char *office_num;
	char *office_phone;
	char *home_phone;
};

extern char *putGetLine();

static char *getFingerInfo();
static int finger_change(), get_defaults(), special_case(), illegal_input();

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

#include <sia.h>
#include <siad.h>

siad_chg_finger(collect,username,argc,argv)
int (*collect)();
const char *username;
int argc;
char *argv[];
{
        struct passwd *pwd;
	extern int (*_c2_collect)();

	_c2_collect = collect;
  
	set_auth_parameters(argc, argv);
	initprivs();
		    
	if (c2AuthorizePwdChange(username, &pwd) == SIADFAIL)
	  return(SIADFAIL);

	if (finger_change(pwd) == SIADFAIL)
	  return(SIADFAIL);

	return(SIADSUCCESS);
	
} /* siad_chg_shell */

static int
finger_change(pwd)
     struct passwd *pwd;
{
    char *pC;
    
    if ((pC = getFingerInfo(pwd)) == (char *)SIADFAIL)
      return(SIADFAIL);

    pwd->pw_gecos = pC;

    if (updatePasswordFile(pwd) == SIADFAIL)
      return(SIADFAIL);

    return(SIADSUCCESS);
    
} /* finger_change */

/*
 * Get name, room number, school phone, and home phone.
 */

static char *
getFingerInfo(pwd)
     struct passwd *pwd;
{
    char *pInput, buf[1024];
    static char answer[4*BUFSIZ];
    struct default_values defaults;

    answer[0] = '\0';

    if (get_defaults(pwd->pw_gecos, &defaults) == SIADFAIL)
      return(SIADFAIL);

    show_mesg("\n%s\n%s\n%s\n",
	    GETMSG(MS_SIA_PWD, DEF_VALUE, "Default values are printed inside of '[]'."),
	    GETMSG(MS_SIA_PWD, ACCEPT, "To accept the default, type <return>."),
	    GETMSG(MS_SIA_PWD, ENTRY1, "To have a blank entry, type the word 'none'."));
	
    /*
     * Get name.
     */

    sprintf(buf, GETMSG(MS_SIA_PWD, NAME, "\nName [%s]: "), defaults.name);

    do
      {
	pInput = putGetLine(buf, BUFSIZE);

	if (special_case(pInput, defaults.name)) 
	  break;

      } while (illegal_input(pInput));

    (void) strcpy(answer, pInput);

    /*
     * Get office location
     */

    sprintf(buf, GETMSG(MS_SIA_PWD, OFFICE, "Office Location [%s]: "),
	    defaults.office_num);
    do
      {
	pInput = putGetLine(buf, BUFSIZE);

	if (special_case(pInput, defaults.office_num))
	  break;

      } while (illegal_input(pInput));

    (void) strcat(strcat(answer, ","), pInput);

    /*
     * Get office phone number.
     */

    sprintf(buf, GETMSG(MS_SIA_PWD, PHONE1, "Office Phone [%s]: "),defaults.office_phone);

    do
      {
	pInput = putGetLine(buf, BUFSIZE);

	if (special_case(pInput, defaults.office_phone))
	  break;

      } while (illegal_input(pInput));

    (void) strcat(strcat(answer, ","), pInput);

    /*
     * Get home phone number
     */

    sprintf(buf, GETMSG(MS_SIA_PWD, PHONE2, "Home Phone [%s]: "), defaults.home_phone);

    do {
      pInput = putGetLine(buf, BUFSIZE);
	
      if (special_case(pInput, defaults.home_phone))
	break;
	
    } while (illegal_input(pInput));

    (void) strcat(strcat(answer, ","), pInput);

    if (strcmp(answer, pwd->pw_gecos) == 0)
      {
	show_mesg(GETMSG(MS_SIA_PWD, FINGERINFO, "Finger information unchanged."));
	return(SIADFAIL);
      }
    
    return(answer);
}

/*
 * get_defaults picks apart "str" and fills in a structure.
 * "str" contains up to 4 fields separated by commas.
 * Any field that is missing is set to blank.
 */

static int
get_defaults(str, answer)
     char *str;
     struct default_values *answer;
{
        answer->name = str;
	answer->office_num = "";
	answer->office_phone = "";
	answer->home_phone = "";
	str = index(answer->name, ',');
	
	if (str == 0) 
	  return(SIADSUCCESS);
	
	*str = '\0';
	answer->office_num = str + 1;
	str = index(answer->office_num, ',');
	
	if (str == 0) 
	  return(SIADSUCCESS);

	*str = '\0';
	answer->office_phone = str + 1;
	str = index(answer->office_phone, ',');

	if (str == 0) 
	  return(SIADSUCCESS);
	
	*str = '\0';
	answer->home_phone = str + 1;
	return(SIADSUCCESS);

} /* get_defaults */

/*
 *  special_case returns true when either the default is accepted
 *  (str = '\n'), or when 'none' is typed.  'none' is accepted in
 *  either upper or lower case (or any combination).  'str' is modified
 *  in these two cases.
 */

static int
special_case(str, default_str)
	char *str, *default_str;
{
	char buf[128];
	int i;
	
	/*
	 *  If the default is accepted, then change the old string do the 
	 *  default string.
	 */
	
	if (*str == '\n')
	  {
	    (void) strcpy(str, default_str);
	    return (1);
	  }
	
	/*
	 * if the input string == "none\n", case insensitive, return true
	 */
	
	for (i = 0; i < 5; i++)
	    buf[i] = (char)tolower((int)str[i]);

	if (strcmp(GETMSG(MS_SIA_PWD, NONE, "none\n"), buf) == 0)
	  {
	    *str = '\0';
	    return (1);
	  }

	return(0);
	
} /* special_case */

/*
 * This routine checks to see if input is valid for use as gecos info.
 * Since the passwd file treats ':', ',' and '\n' as special, they can't be
 * used in the gecos field.  If these characters are found we return '1'.
 */

static int
illegal_input(input_str)
	char *input_str;
{
	char *ptr, buf[256];
	int length;

	if (strpbrk(input_str, ",:")) {
	  show_error(GETMSG(MS_SIA_PWD, BADCHARS, "':' and ',' are not allowed."));
	  return(1);
	}

	/*
	 * Delete newline by shortening string by 1.
	 */

	input_str[strlen(input_str) - 1] = '\0';

	/*
	 * Don't allow control characters, etc in input string.
	 */

	for (ptr = input_str; *ptr; ptr++)
	  if (!isprint(*ptr)) {
	    show_error(GETMSG(MS_SIA_PWD, CTRCHARS, "Control characters are not allowed."));
	    return(1);
	  }

	return(0);
	
} /* illegal_input */

