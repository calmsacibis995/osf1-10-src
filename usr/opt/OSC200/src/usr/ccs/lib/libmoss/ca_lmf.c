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
static char *rcsid = "@(#)$RCSfile: ca_lmf.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 15:59:11 $";
#endif
  /*
   * TITLE: CA_LMF.C
   *
   * Copyright (c) Digital Equipment Corporation, 1992
   * All Rights Reserved.  Unpublished rights reserved
   * under the copyright laws of the United States.
   *
   * The software contained on this media is proprietary
   * to and embodies the confidential technology of
   * Digital Equipment Corporation.  Possession, use,
   * duplication or dissemination of the software and
   * media is authorized only pursuant to a valid written
   * license from Digital Equipment Corporation.
   *
   * RESTRICTED RIGHTS LEGEND   Use, duplication, or
   * disclosure by the U.S. Government is subject to
   * restrictions as set forth in Subparagraph (c)(1)(ii)
   * of DFARS 252.227-7013, or in FAR 52.227-19, as
   * applicable.
   *
   * Abstract:
   *	POLYCENTER (tm) Common Agent (ULTRIX) license check rtn
   *		Until CA licenses arrive, Check for DECmcc licenses (tbs)
   *
   *	Routine interfaces:
   *		int ca_check_base_license()		-- returns 0 if success
   *		int ca_check_developers_license()	-- returns 0 if success
   *
   * Compilation
   *	if compiled with -DNO_PAK_REQ, then ca_check_ rtns always return success
   *
   * Revision History
   *	09-Aug-1992	Pete Burgess	Created
   *    11-Dec-1992	Pete Burgess	
   *		Since the base-license is now a subset of the dev-license,
   *		a check for base-license is satisfied by the presence of either license.
   *	21-Apr-1993	Pete Burgess
   *		Provide NO_PAK_REQ compilation switch for OSF/1 bundling
   */

/*
 *  Function Prototypes to satisfy c89 -std warnings.
 *
 *   - strncasecmp() is not defined in <string.h> or <strings.h>
 */

extern int strncasecmp() ;

/*
 *  ANSI C defines __mips and __ultrix. To be compatible with ANSI C,
 *  include <ansi_compat.h>. This file is included by some .h files but
 *  include it here just in case.
 */

#if !defined(__osf__) && !defined(sun) && !defined(sparc)
#include <ansi_compat.h>
#endif

#include <string.h>
#include <time.h>

#if defined(__ultrix) || defined(__osf__)
#include <lmf.h>
#endif

#include <stdio.h>

#if ( ( defined(__mips) && defined(__ultrix) ) || defined(__osf__) )

/* CA Developers Kit */
# define ED_PRODUCT		"COM-AGNT-DEV"
# define ED_PRODUCER 		"DEC"
# define ED_VERSION_MAJOR	1
# define ED_VERSION_MINOR	1
# define ED_RELEASE_DATE	"31-AUG-1993"

/* CA Base Kit*/
# define EC_PRODUCT		"COM-AGNT-BAS"
# define EC_PRODUCER 		"DEC"
# define EC_VERSION_MAJOR	1
# define EC_VERSION_MINOR	1
# define EC_RELEASE_DATE	"31-AUG-1993"

static ver_t ed_ver = {ED_VERSION_MAJOR, ED_VERSION_MINOR};
static ver_t ec_ver = {EC_VERSION_MAJOR, EC_VERSION_MINOR};

#endif


/*
 * Convert product release date 
 * from ASCII format to internal format
 */

static int convert_release_date (p_product_date_s, p_product_date)

char *p_product_date_s;
int *p_product_date;

{
  struct tm	time_structure;
  int		internal_product_date;
  int		day;
  int		year;
  char		month_string[4];
  int		month_int;
  int		convert_status;

  *p_product_date = 0;

  /* Initialize day, year, and month variable to null                          */  
  day = year = 0;

  /* Extract day, month, and year from the combined date string                */
  sscanf(p_product_date_s, "%d-%3s-%d",&day, month_string, &year);
  
  /* Map the 3 character month string to a number                              */
  
  convert_status = 0;
  if (strncasecmp(month_string, "JAN", 3)==0) month_int = 0;
  else if (strncasecmp(month_string, "FEB", 3)==0) month_int = 1;
  else if (strncasecmp(month_string, "MAR", 3)==0) month_int = 2;
  else if (strncasecmp(month_string, "APR", 3)==0) month_int = 3;
  else if (strncasecmp(month_string, "MAY", 3)==0) month_int = 4;
  else if (strncasecmp(month_string, "JUN", 3)==0) month_int = 5;
  else if (strncasecmp(month_string, "JUL", 3)==0) month_int = 6;
  else if (strncasecmp(month_string, "AUG", 3)==0) month_int = 7;
  else if (strncasecmp(month_string, "SEP", 3)==0) month_int = 8;
  else if (strncasecmp(month_string, "OCT", 3)==0) month_int = 9;
  else if (strncasecmp(month_string, "NOV", 3)==0) month_int = 10;
  else if (strncasecmp(month_string, "DEC", 3)==0) month_int = 11;
  else if ((day==0)&&(year==0)) month_int = 0;
  else convert_status = -1;
  
  if (convert_status != 0) {
    fprintf(stderr, "Internal failure in CA_LMF, converting month string  \n");
    return(0);
  }
  
/*
 * ca_check_base_license
 *
 * returns
 *	0 - success
 *	other - failure
 */
  /* Convert the date to seconds since 1-JAN-1970, GMT.                        */
  /* A GMT offset would have to be added to use local time.                    */
  
  memset (&time_structure, 0, sizeof(time_structure));
  time_structure.tm_year = year - 1900;
  time_structure.tm_mon = month_int;
  time_structure.tm_mday = day;
  
  internal_product_date = mktime(&time_structure);
  if (((int)internal_product_date == -1) && (day!=0) && (year!=0)) {
    fprintf(stderr, "\nInternal failure in CA_LMF, call to mktime\n");
    return(0);
  }
  
  *p_product_date = internal_product_date;
  return (1);
}


/*
 * ca_check_developer_license ()
 *
 * returns
 *	0 	- success
 *	other 	- failure
 */

ca_check_developer_license ()
{
  unsigned int lmf_status;
  int ed_prod_date;

#ifdef NO_PAK_REQ
  return (0);
#else  
  convert_release_date (ED_RELEASE_DATE, &ed_prod_date);

  lmf_status = lmf_probe_license( ED_PRODUCT, ED_PRODUCER, &ed_ver, ed_prod_date, 0);
  return (lmf_status);
#endif

}

/*
 * ca_check_base_license ()
 *	Check for either Base or Dev License
 * returns
 *	0 	- success
 *	other	- failure
 */
int ca_check_base_license ()
{
  unsigned int lmf_status;
  int ec_prod_date;

#ifdef NO_PAK_REQ
  return (0);
#else
  convert_release_date (EC_RELEASE_DATE, &ec_prod_date);
  lmf_status = lmf_probe_license (EC_PRODUCT, EC_PRODUCER, &ec_ver, ec_prod_date, 0);
  if (lmf_status) lmf_status = ca_check_developer_license();
  return (lmf_status);
#endif
}



