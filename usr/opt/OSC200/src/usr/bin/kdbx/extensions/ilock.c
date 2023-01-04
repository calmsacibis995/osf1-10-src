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
static char *rcsid = "@(#)$RCSfile: ilock.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/11/17 14:54:25 $";
#endif

/* This kdbx extension prints out the lock status of inodes/cdnodes. */

#include <stdio.h>
#include <sys/time.h>
#include <sys/vnode.h>
#include "krash.h"

static char *help_string =
"ilock - print the inode/cdnode lock info                            \\\n\
   Usage : ilock [options]                                           \\\n\
     (no options) show lock status of all ACTIVE inodes/cdnodes\\\n\
                    (ACTIVE means: (usecount!=0 || holdcnt!=0) )\\\n\
     -all          show lock status of ALL inodes/cdnodes         \\\n\
     -vn <addr>    show lock status of the inode/cdnode contained in the given vnode \\\n\
     -in <inode#>  show lock status of the given inode \\\n\
";

/* please look at sys/vnode.h for meaning of these fields */
FieldRec vnode_fields[] = {
  { ".v_flag", NUMBER, NULL, NULL },
  { ".v_usecount", NUMBER, NULL, NULL },
  { ".v_holdcnt", NUMBER, NULL, NULL },
  { ".v_mount", NUMBER, NULL, NULL },
  { ".v_numoutput", NUMBER, NULL, NULL },
  { ".v_type", NUMBER, NULL, NULL },
  { ".v_tag", NUMBER, NULL, NULL },
  { ".v_un.vu_mountedhere", NUMBER, NULL, NULL },
  { ".v_data", NUMBER, NULL, NULL },
};

#define NUM_VNODE_FIELDS (sizeof(vnode_fields)/sizeof(vnode_fields[0]))

FieldRec inode_fields[] = {
  { ".i_io_lock.want_write", NUMBER, NULL, NULL },
  { ".i_io_lock.want_upgrade", NUMBER, NULL, NULL },
  { ".i_io_lock.waiting", NUMBER, NULL, NULL },
  { ".i_io_lock.can_sleep", NUMBER, NULL, NULL },
  { ".i_io_lock.read_count", NUMBER, NULL, NULL },
  { ".i_io_lock.recursion_depth", NUMBER, NULL, NULL },
  { ".i_io_lock.thread", NUMBER, NULL, NULL },
  { ".i_number", NUMBER, NULL, NULL },
  { ".i_fs", NUMBER, NULL, NULL },
};

#define NUM_INODE_FIELDS (sizeof(inode_fields)/sizeof(inode_fields[0]))

FieldRec cdnode_fields[] = {
  { ".cd_io_lock.want_write", NUMBER, NULL, NULL },
  { ".cd_io_lock.want_upgrade", NUMBER, NULL, NULL },
  { ".cd_io_lock.waiting", NUMBER, NULL, NULL },
  { ".cd_io_lock.can_sleep", NUMBER, NULL, NULL },
  { ".cd_io_lock.read_count", NUMBER, NULL, NULL },
  { ".cd_io_lock.recursion_depth", NUMBER, NULL, NULL },
  { ".cd_io_lock.thread", POINTER, NULL, NULL },
};

#define NUM_CDNODE_FIELDS (sizeof(cdnode_fields)/sizeof(cdnode_fields[0]))
#define offset 0xb0

static  char *vtype[9] = {"VNON", "VREG", "VDIR", "VBLK", "VCHR",
                      "VLNK", "VSOCK", "VFIFO", "VBAD"};
static char *vtagtype[10] = { "VT_NON", "VT_UFS", "VT_NFS", "VT_MFS", "VT_S5FS", 
       "VT_CDFS", "VT_DFS", "VT_EFS", "VT_PRFS", "VT_MSFS"};


main(int argc, char **argv)
{
  DataStruct inode, rnode, cdnode ;
  int i, c, index, struct_size ;
  long  nvnode, datasize, addr, addr1, cur_node ;
  char buf[256], typebuf[6], tagbuf[8], lock_info[60]="", *error ;
  /* It is VERY important that these flags are initialized to zero */
  int allflag=0, vnflag=0, inflag=0, inarg=0, found=0;
  long vnarg=0;
  int freeflag=0, fsflag=0, gidflag=0, uidflag=0;
  long fsarg=0, gid=0, uid=0;


  /*** parse command line arguments ***/

  check_args(argc, argv, help_string);
  for (i = 1; i < argc; i++)
      if (!strcmp(argv[i], "-all"))  allflag = 1;
      else if (!strcmp(argv[i], "-vn"))  {vnflag = 1; vnarg = strtoul(argv[++i],(char**)NULL,16);}
      else if (!strcmp(argv[i], "-in"))  {inflag = 1; inarg = atoi(argv[++i]);}
      else {
        fprintf(stderr, "%s: invalid option, `%s'\n", argv[0], argv[i]);
        quit(1);
      }

  /*** check fields ***/

  if(!check_fields("struct vnode", vnode_fields, NUM_VNODE_FIELDS, NULL)){
    field_errors(vnode_fields, NUM_VNODE_FIELDS);
    quit(1);
  }

  /*** read value of vnode, nvnode and vn_maxprivate ***/

  /* This gets the correct value of vnode (0x9021e000) and deposit it in addr */
  if(!read_sym_val("vnode", NUMBER, &addr, &error)){
    fprintf(stderr, "%s\n", error);
    quit(1);
  };

  /* get the number of elements in the vnode array */
  if(!read_sym_val("nvnode", NUMBER, &nvnode, &error)) {
    fprintf(stderr, "Couldn't read nvnode:\n");
    fprintf(stderr, "%s\n", error);
    quit(1);
  }

  /* get the size of the private data in each vnode */
  if(!read_sym_val("vn_maxprivate", NUMBER, &datasize, &error)){
    fprintf(stderr, "%s\n", error);
    quit(1);
  }

  /*** print output heading ***/

  sprintf(buf, " ADDR OF VNODE      WRITE UPGRADE WAITING SLEEP READ_CNT RECURSION INODE#");
  print(buf);
  print(       "==================  ===== ======= ======= ===== ======== ========= ======");
  
  /*** loop through the whole vnode table ***/

  for (i=0; i < nvnode; i++) {
  

    /* cast the address addr to a pointer to (struct vnode) & put it in cur_node */
    if(!cast(addr, "struct vnode", &cur_node, &error)){
      fprintf(stderr, "Couldn't cast to a addr:\n");
      fprintf(stderr, "%s\n", error);
      quit(1);
    }

    /* get the values of the current struct vnode into vnode_fields[] */
    if(!read_field_vals(cur_node, vnode_fields, NUM_VNODE_FIELDS)){
      field_errors(vnode_fields, NUM_VNODE_FIELDS);
      quit(1);
    }

    /* save the current vnode address and compute the next one */
    addr1 = addr;
    addr = addr + offset + datasize ; /* inc by sizeof(struct vnode) plus sizeof private data */

    if ((vnflag || inflag) && found) quit(0);
    if (vnflag && (vnarg != addr1)) continue;
    if ((!vnflag) || (!inflag))   /* if only one vnode, show it in any case */
    if ((!allflag) && ((vnode_fields[0].data == 0) && (vnode_fields[1].data == 0)))
       continue;  /* by default -- ACTIVE nodes only */

    /* convert v_type and v_tag to symbolic values */
    if (((int)vnode_fields[5].data  < 0) || ((int)vnode_fields[5].data > 8))
       strcpy(typebuf, " ? ");
    else {
      index = (int)(vnode_fields[5].data);
      strcpy(typebuf, vtype[index] );
    }
    if (((int)vnode_fields[6].data  < 0) || ((int)vnode_fields[6].data > 14))
       strcpy(tagbuf, " ? ");
    else {
      index = (int)(vnode_fields[6].data);
      strcpy(tagbuf, vtagtype[index] );
    }

    /* read into inode/cdnode */
      if (((int)vnode_fields[6].data  == VT_UFS)){

        /*** get inode info ***/
        if(!cast((long)(addr1+offset), "struct inode", &inode, &error)){
        fprintf(stderr, "Couldn't cast to inode:\n");
        fprintf(stderr, "%s\n", error);
        return(False);
        }

        if(!read_field_vals(inode, inode_fields, NUM_INODE_FIELDS)){
          field_errors(inode_fields, NUM_INODE_FIELDS);
          return(False);
        }
        
        if (inflag && (inarg != (int)inode_fields[7].data)) continue;
        sprintf(lock_info, "%6d %6d %6d %6d  %6d   %6d  %7d ",inode_fields[0].data,
          inode_fields[1].data, inode_fields[2].data, inode_fields[3].data, inode_fields[4].data,
          inode_fields[5].data, inode_fields[7].data);
        sprintf(buf, "0x%16lx %s", addr1, lock_info);
        print(buf);
        if (vnflag || inflag) found = 1;
      }  /* end of if(VT_UFS) */


      else if (((int)vnode_fields[6].data  == VT_CDFS) && (!inflag)){

        /*** get cdnode info ***/
        if(!cast((long)(addr1+offset), "struct cdnode", &cdnode, &error)){
        fprintf(stderr, "Couldn't cast to cdnode:\n");
        fprintf(stderr, "%s\n", error);
        return(False);
        }

        if(!read_field_vals(cdnode, cdnode_fields, NUM_CDNODE_FIELDS)){
          field_errors(cdnode_fields, NUM_CDNODE_FIELDS);
          return(False);
        }

        sprintf(lock_info, "%6d %6d %6d %6d  %6d   %6d ",cdnode_fields[0].data,cdnode_fields[1].data,
          cdnode_fields[2].data, cdnode_fields[3].data, cdnode_fields[4].data,
          cdnode_fields[5].data);
        sprintf(buf, "0x%16lx %s", addr1, lock_info);
        print(buf);
        if (vnflag || inflag) found = 1;
      }  /* end of if(VT_CDFS) */


#ifdef COUNT
    count++;
#endif
  } /* end of for loop */

#ifdef COUNT
  sprintf(buf,"count=%d", count);
  print(buf);
#endif
}
