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
static char	*sccsid = "@(#)$RCSfile: lvsync.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/01/22 15:57:03 $";
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
 *   lvsync:
 *   Synchronizes logical volume mirrors that are stale in one or
 *   more logical volume.
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

#define USAGE	"Usage: lvsync  \
 LogicalVolumePath... \n"

/* Options which require an argument for their value */
#define OPT_WITH_VAL_NUM	0
#define OPTIONS_WITH_VALUE	""

/* Options which DO NOT require an argument; i.e., boolean flags */
#define OPT_WITHOUT_VAL_NUM	0
#define OPTIONS_WITHOUT_VALUE	""

/* Requested args (mandatory) */
#define REQ_ARGS_NUM		1
char **LogicalVolumePath;	/* maybe a list => array of pointers */
int LogicalVolumePath_cnt;	/* number of items in the above array */

/* There are no extra args (optional) */



main(int argc, char **argv)
{
   register int i;
   char **lv_names;
   int lv_cnt;
   char *new_vg_path;
   char vg_path[PATH_MAX + 1];
   char *clean_path;
   char *lv_path;
   int vg_fd = -1;
   int error = 0;


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

   /* Store an emtpy string into vg_path */
   vg_path[0] = '\0';

   /* 
    * Because of consistency reasons, the interupts are disabled 
    * from here on.
    */
   disable_intr();

   /* Loop through the list of LV's supplied */
   for (i = 0; i < LogicalVolumePath_cnt; i++) {

      lv_path = LogicalVolumePath[i];

      /* Get path name of VG owning this LV */
      if (lvtovg(lv_path, &new_vg_path) != OK) {
         print_prgname();
         fprintf(stderr, MSG_NO_VGFORLV, lv_path);
	 error = 1;
         continue;
      }

      /*
       *   LV's might belong to all different VG's; so we keep track of
       *   which VG was most recently used.
       */

      if (!eq_string(vg_path, new_vg_path)) {

	 /* Was any VG already opened? If so, close it */
	 if (vg_fd != -1)
	    close(vg_fd);

         /* A library function takes care of generating the clean path */
         if ((clean_path = check_and_openvg(new_vg_path, &vg_fd)) == NULL)
            continue;
   
         /* Get the names and minor numbers of the LV's that belong to the VG */
         if (lvminors_and_names(vg_fd, clean_path, &lv_names, &lv_cnt) != OK) {
            print_prgname();
            fprintf(stderr, MSG_NO_LVNAMES, new_vg_path);
	    error = 1;
            continue;
         }

	 /* Save most recently accessed VG */
	 strcpy(vg_path, new_vg_path);
      }

      if (resync_lv(vg_fd, lvpathtolvminor(lv_path)) != OK) {
	 print_prgname();
	 fprintf(stderr, MSG_CANT_SYNC_LV, lv_path);
	 error = 1;
      }
      else
	 printf(MSG_RESYNCED_LV, lv_path);
   }

   /* Clean exit */
   return(error);
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
   LogicalVolumePath = left_arg(&LogicalVolumePath_cnt);

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
