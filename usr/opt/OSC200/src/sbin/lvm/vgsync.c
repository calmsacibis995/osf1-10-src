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
static char	*sccsid = "@(#)$RCSfile: vgsync.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:49:35 $";
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
 *   vgsync:
 *   Synchronizes logical volume mirrors that are stale in one or more
 *   volume groups.
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
static int resync_vg(char *vg_path);

/*
 *   Usage message: badly formatted because automatically
 *   generated. A local library routine will print it
 *   in a better fashion.
 */

#define USAGE	"Usage: vgsync \
 VolumeGroupName... \n"

/* Options which require an argument for their value */
#define OPT_WITH_VAL_NUM	0
#define OPTIONS_WITH_VALUE	""

/* Options which DO NOT require an argument; i.e., boolean flags */
#define OPT_WITHOUT_VAL_NUM	0
#define OPTIONS_WITHOUT_VALUE	""

/* Requested args (mandatory) */
#define REQ_ARGS_NUM		1
char **VolumeGroupName;	/* maybe a list => array of pointers */
int VolumeGroupName_cnt;	/* number of items in the above array */

/* There are no extra args (optional) */



main(int argc, char **argv)
{
   char *vg_path;
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
    * Because of consistency reasons, the interupts are disabled 
    * from here on.
    */
   disable_intr();

   /* Loop through the list of VG's supplied */
   for (i = 0; i < VolumeGroupName_cnt; i++) {

      vg_path = VolumeGroupName[i];
      if (resync_vg(vg_path) != OK) {
	 print_prgname();
	 fprintf(stderr, MSG_CANT_SYNC_VG, vg_path);

	 /* No reason to exit; just continue */
      }
      else
	 printf(MSG_RESYNCED_VG, vg_path);
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

   /* Set references to mandatory arguments */
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



static int
resync_vg(char *vg_path)
{
   register int i;
   char *clean_path;
   int vg_fd;
   char **lv_names;
   char **lv_pathp;
   int lv_cnt;

   debug(dbg_entry("resync_vg"));

   /* Another library function takes care of generating the clean path */
   if ((clean_path = check_and_openvg(vg_path, &vg_fd)) == NULL) {
      debug(dbg_exit());
      return(NOT_OK);
   }
   
   /* Loop through the LV's that belong to the VG; get the names, first */
   if (lvminors_and_names(vg_fd, clean_path, &lv_names, &lv_cnt) != OK) {
      print_prgname();
      fprintf(stderr, MSG_NO_LVNAMES, vg_path);
      debug(dbg_exit());
      return(NOT_OK);
   }

   for (i = 0, lv_pathp = lv_names; i < lv_cnt; i++, lv_pathp++) {
      if (resync_lv(vg_fd, lvpathtolvminor(*lv_pathp)) != OK) {
	 print_prgname();
	 fprintf(stderr, MSG_CANT_SYNC_LV, *lv_pathp);
	 debug(dbg_exit());
	 return(NOT_OK);
      }
      else 
	 printf(MSG_RESYNCED_LV, *lv_pathp);
   }

   debug(dbg_exit());
   return(OK);
}
