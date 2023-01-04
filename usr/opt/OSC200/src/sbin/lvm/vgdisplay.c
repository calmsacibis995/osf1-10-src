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
static char	*sccsid = "@(#)$RCSfile: vgdisplay.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/01/08 15:42:30 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */

/*
 * OSF/1 Release 1.0
 */

/*
 *   vgdisplay:
 *   Displays information about volume groups.
 */

/* Each file containing a main() has some privilege: see "lvmcmds.h" */
#define LVM_CMD_MAIN_FILE
#include "lvmcmds.h"

/*
 *   Here are all the declarations that are specific to this command,
 *   that is, file inclusions, definitions, variables, types, etc.
 */

/* Local functions */
static int check_usage_semantics();
static int set_defaults();

/*
 *   Usage message: badly formatted because automatically
 *   generated. A local library routine will print it
 *   in a better fashion.
 */

#define USAGE	"Usage: vgdisplay  [-v] [\
 VolumeGroupName... ]\n"

/* Options which require an argument for their value */
#define OPT_WITH_VAL_NUM	0
#define OPTIONS_WITH_VALUE	""

/* Options which DO NOT require an argument; i.e., boolean flags */
#define OPT_WITHOUT_VAL_NUM	1
#define OPTIONS_WITHOUT_VALUE	"v"
char vflag;

/* Requested args (mandatory) */
#define REQ_ARGS_NUM		0

/* Extra args (optional) */
char **VolumeGroupName;	/* maybe a list => array of pointers */
int VolumeGroupName_cnt;	/* number of items in the above array */



main(int argc, char **argv)
{
   register char *vg_path;
   register int i;


   /* Initialize the i18n (internationalization) support */
   msg_init();

   /* Set defaults specific to this command */
   set_defaults();

   /* See if the user typed a proper request */
   if (check_usage(argc, argv) != OK) {
      print_usage(USAGE);
      print_arg_error();
      exit(1);
   }

   init_debug();

   /*
    *   If on the command line no VG was specified, we have to dump all
    *   the VG's belonging to this system. Therefore, we extract them from
    *   the lvmtab file.
    */

   if (VolumeGroupName_cnt == 0) {
      if (lvmtab_read() != OK ||
	  lvmtab_getvgnames(&VolumeGroupName, &VolumeGroupName_cnt) != OK) {

	 /* Couldn't access the lvmtab file */
	 print_prgname();
	 fprintf(stderr, MSG_NO_VGNAMES, LVMTABPATH);
	 exit(FATAL_ERROR);
      }
   }

   /* Just loop on the volume group names, and call the library routine */
   dump_vg_title();
   for (i = 0; i < VolumeGroupName_cnt; i++) {

      vg_path = VolumeGroupName[i];
      if (dump_vg(vg_path, vflag? VERBOSE : TERSE) != OK) {
	 print_prgname();
	 fprintf(stderr, MSG_CANT_DUMP_VG, vg_path);
	 return(FATAL_ERROR);
      }
   }


   /* Clean exit */
   return(0);
}



int
check_usage(int argc, char **argv)
{
   /* Call the general-purpose routine to check usage syntax */
   if (parse_args(&argc, &argv, OPTIONS_WITHOUT_VALUE, 
         OPTIONS_WITH_VALUE, REQ_ARGS_NUM) != OK)
      return(NOT_OK);

   /* Check whether usage syntax is correct */
   if (check_usage_syntax() != OK)
      return(NOT_OK);

   /* If we get to this point, usage syntax is correct */
   if (check_usage_semantics() != OK)
      return(NOT_OK);

   return(OK);
}



int
check_usage_syntax()
{
   register int i;

   /* See which options without value have been used */
   vflag = used_opt('v');


   /* Set references to extra arguments */
   VolumeGroupName = left_arg(&VolumeGroupName_cnt);

   return(OK);
}



/*
 *   Here are all the subroutines that are specific to this command,
 *   that is, routines that don't fit into the LVM cmd's library
 */

static int
check_usage_semantics()
{

   /* Nothing to complain about */
   return(OK);
}



static int
set_defaults()
{
}
