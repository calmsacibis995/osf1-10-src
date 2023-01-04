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
static char *rcsid = "@(#)$RCSfile: filename.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/03/16 19:02:16 $";
#endif

# include <dirent.h>
# include <sys/mount.h>
# include <sys/stat.h>
# include <stdio.h>
# include <sys/types.h>
# include "krash.h"

static char *help_string = 
"filename - translate a dev_t and ino_t to a filename \\\n\
    Usage: filename <dev_t> <ino_t> \\\n\
";

static char *mount_name(char *);
static char *partition_name(dev_t);
static char *path_name(char *, ino_t);

/*
** Mount fields needed to translate the partition name to a mount
** point name so that the partition's directory hierarchy can be
** traversed when searching for the specified inode.
*/
FieldRec mount_fields[] = {
  { ".m_next",			NUMBER, NULL, NULL },
  { ".m_stat.f_type",		NUMBER, NULL, NULL },
  { ".m_stat.f_mntonname",	STRING, NULL, NULL },
  { ".m_stat.f_mntfromname",	STRING, NULL, NULL }};
# define NUM_MOUNT_FIELDS (sizeof(mount_fields)/sizeof(mount_fields[0]))


/*
** Main program.
*/
main(argc, argv)
  int argc;
  char **argv;
{
  char *command;
  char buf[1100];
  char *device_name;
  long  device_number;
  long  inode_number;
  char *mount_point;

  /*
  ** Process a `-help' string.
  */
  check_args(argc, argv, help_string);

  /*
  ** If the number of arguments mismatch, then put out a usage
  ** message.
  */
  if (argc < 2 || argc > 3) {
    fprintf(stderr, "usage: %s <dev_t> [<ino_t>]\n", *argv);
    quit(1);
    }

  /*
  ** Advance over the command name, and store the dev and ino.
  */
  command = *argv;
  if (!to_number(*++argv, &device_number)) {
    fprintf(stderr, "%s: couldn't parse \"%s\% to a number\n",
		command, *argv);
    quit(1);
    }
  if (argc == 3) {
    if (!to_number(*++argv, &inode_number)) {
      fprintf(stderr, "%s: couldn't parse \"%s\" to a number\n",
		command, *argv);
      quit(1);
      }
    }
  else
    inode_number = 0;

  /*
  ** Display the base information.
  */
  sprintf(buf, "dev_t:        0x%08x (%d)",
		device_number,
		device_number);
  print(buf);
  sprintf(buf, "Major/Minor:  %d, %d",
		major(device_number),
		minor(device_number));
  print(buf);

  /*
  ** Translate the device number into a partition name using
  ** the definitions of the current `/dev'.
  */
  if (!(device_name = partition_name((dev_t) device_number))) {
    fprintf(stderr, "%s: couldn't translate dev_t 0x%08x to partition name in current /dev\n",
		command, device_number);
    quit(1);
    }

  /*
  ** Show the device name.
  */
  sprintf(buf, "Device name:  %s", device_name);
  print(buf);

  /*
  ** Check to see if it is mounted.
  */
  if (!(mount_point = mount_name(device_name)))
    sprintf(buf, "Mount point:  (None)  Device is not mounted");
  else
    sprintf(buf, "Mount point:  %s", mount_point);
  print(buf);

  /*
  ** If the inode number is non-zero, attempt to resolve its name.
  */
  if (inode_number) {
    /*
    ** Attempt to translate the name into a mount-point name.
    */
    char pathname[1025];

    if (!mount_point) {
      fprintf(stderr, "%s: device %s must be mounted to resolve ino_t\n",
		command, device_name);
      quit(1);
      }

    /*
    ** Resolve the directory.
    ** Copy the starting point to the work area, and begin traversing
    ** the directory hierarchy looking for the inode.  If not found,
    ** say so, otherwise show the resulting name.
    */
    strcpy(pathname, mount_point);
    if (!path_name(pathname, inode_number))
      fprintf(stderr, "%s: Unable to resolve ino_t %d\n",
		command, inode_number);
    else {
      sprintf(buf, "Pathname:     %s", pathname);
      print(buf);
      }
    }
  }


/*
** Translate a partition (device) name into a mount-point name.
*/
static char *mount_name(char *par_name)
{
  static char mnt_name[256];		/* Reasonable limit */
  long  addr;
  char *error;
  long  first_addr;
  DataStruct mount;

  /*
  ** Check the fields in our structure to see that they match
  ** the actual structure definition.
  */
  if (!check_fields("struct mount", mount_fields, NUM_MOUNT_FIELDS, NULL)) {
    field_errors(mount_fields, NUM_MOUNT_FIELDS);
    quit(1);
    }

  /*
  ** Get the root file system address to begin the search.
  */
  if (!read_sym_val("rootfs", NUMBER, &first_addr, &error)) {
    fprintf(stderr, "Couldn't read rootfs:\n");
    fprintf(stderr, "%s\n", error);
    quit(1);
    }

  /*
  ** Loop attempting to match up the name of the partition to
  ** a mount point.
  */
  for (addr = first_addr; addr; ) {
    /*
    ** Cast the address to a mount structure.
    */
    if (!cast((int) addr, "struct mount", &mount, &error)) {
      fprintf(stderr, "Couldn't cast to mount:\n");
      fprintf(stderr, "%s\n", error);
      quit(1);
      }

    /*
    ** Read the fields of the structure pointed to by `addr'.
    */
    if (!read_field_vals(mount, mount_fields, NUM_MOUNT_FIELDS)) {
      field_errors(mount_fields, NUM_MOUNT_FIELDS);
      quit(1);
      }
    
    /*
    ** If the names match, then return the name.
    */
    if (!strcmp(par_name, mount_fields[3].data)) {
      strcpy(mnt_name, mount_fields[2].data);
      return mnt_name;
      }

    /*
    ** Advance the pointer to the next structure in the mount list.
    */
    addr = (long) mount_fields[0].data;
    if (addr == first_addr)
      break;
    } /* for() */
  return NULL;
  }


/*
** Translate a device number (dev_t) into a partition name.
*/
static char *partition_name(dev_t dev)
{
  static char device_name[256+4+1];	/* `/dev/file-name-up-to-255-characters'<NUL> */
  DIR *dir;
  struct dirent *dirent;

  /*
  ** Find the device in the /tmp directory.
  */
  if (!(dir = opendir("/dev")))
    return NULL;

  /*
  ** Directory is accessed, read through each name in `/dev', and
  ** stat the file to obtain the (possible) major/minor numbers.
  ** When we have a block device which matches, return its name.
  */
  while (dirent = readdir(dir)) {
    /*
    ** Got some directory information.  Check for a block special
    ** with matching major and minor numbers.  If (when) found,
    ** return a pointer to the generated name.
    */
    struct stat local_stat;

    sprintf(device_name, "/dev/%s", dirent->d_name);
    if (!stat(device_name, &local_stat)) {
      if (S_ISBLK(local_stat.st_mode) &&
          major(local_stat.st_rdev) == major(dev) &&
          minor(local_stat.st_rdev) == minor(dev)) {
	closedir(dir);
	return device_name;
	}
      }
    }
  closedir(dir);
  return NULL;
  }


/*
** Build a pathname while searching for a specific inode.
*/
static char *path_name(char *candidate, ino_t inode)
{
  int candidate_size;
  struct stat stat_info;
  DIR *dir;
  struct dirent *dirent;

  candidate_size = strlen(candidate);

  /*
  ** Attempt to stat the file.
  */
  if (!stat(candidate, &stat_info)) {
    /*
    ** Issue a stat on the file to get information about it.
    */
    if (stat_info.st_ino == inode) {
      /*
      ** This is the file that we were looking for.
      ** Return its name to the caller.
      */
      return candidate;
      }

    /*
    ** This is NOT the inode that we want.
    ** If this file is NOT a directory, then return
    ** a NULL pointer to indicate that we can't go
    ** down any further.
    */
    if (!S_ISDIR(stat_info.st_mode))
      return NULL;

    /*
    ** This IS a directory.  Probe each member of the
    ** directory (don't check . and ..) recursively to
    ** find the file.
    */
    if (!(dir = opendir(candidate)))
      return NULL;

    /*
    ** Directory is accessed, read through each name in `/dev', and
    ** stat the file to obtain the (possible) major/minor numbers.
    ** When we have a block device which matches, return its name.
    */
    while (dirent = readdir(dir)) {
      char *actual_file;

      /*
      ** Got a directory entry.  Build a name for it and
      ** issue a recursive call to check it.  Note that
      ** `.' and `..' must be ignored.
      */
      if (!strcmp(dirent->d_name, ".") ||
          !strcmp(dirent->d_name, ".."))
        continue;
      candidate[candidate_size] = '/';
      strcpy(&candidate[candidate_size+1], dirent->d_name);

      /*
      ** Try this one.
      */
      if (actual_file = path_name(candidate, inode)) {
        /*
        ** Found the matching file ... return the full-blown name.
        */
        closedir(dir);
        return actual_file;
        }

      /*
      ** Actual file was not found in the specified (possible) dir.
      ** Do this again...
      */
      } /* while (dirent) */

    /*
    ** Not found in this possibility.
    ** Shrink the candidate's name down after closing the
    ** directory.
    */
    closedir(dir);
    candidate[candidate_size] = 0;
    } /* if stat() */
  return NULL;
  }
