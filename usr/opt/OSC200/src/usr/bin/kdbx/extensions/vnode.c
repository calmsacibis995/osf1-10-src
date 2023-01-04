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
static char *rcsid = "@(#)$RCSfile: vnode.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/11/17 14:55:01 $";
#endif

/* This kdbx extension prints out the vnode table. */
/* The vnode table is in fact an array with nvnode elements. */

#include <stdio.h>
#include <sys/time.h>
#include <sys/vnode.h>
#include <ufs/inode.h>
#include <ufs/dinode.h>
#include "krash.h"

static char *help_string =
"vnode - print the vnode table                                        \\\n\
   Usage : vnode [options]                                           \\\n\
     (no options)   show ACTIVE entries in the vnode table\\\n\
                    (ACTIVE means: (usecount!=0 || holdcnt!=0) )\\\n\
                    (ACTIVE is the default attribute for options\\\n\
                     other than -all and -free)\\\n\
     -free       show INACTIVE entries in the vnode table\\\n\
     -all        show ALL vnodes (both ACTIVE and INACTIVE)\\\n\
     -ufs        show UFS vnodes\\\n\
     -nfs        show NFS vnodes\\\n\
     -cdfs       show CDFS vnodes\\\n\
     -fs <addr>  show vnodes of a mounted file system\\\n\
                 (You can get <addr> by using the kdbx's mount command)\\\n\
     -u  <uid>   show vnode entries of a particular user\\\n\
     -g  <gid>   show vnode entries of a particular group\\\n\
     -v          show related inode/rnode/cdnode info (used with -ufs, -nfs, or -cdfs only)\\\n\
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

FieldRec inode_fields[] = {
  { ".i_fs", NUMBER, NULL, NULL }, 
  { ".i_number", NUMBER, NULL, NULL },  
  { ".i_din.di_mode", NUMBER, NULL, NULL }, 
  { ".i_din.di_uid", NUMBER, NULL, NULL }, 
  { ".i_din.di_gid", NUMBER, NULL, NULL }, 
  { ".i_din.di_qsize", NUMBER, NULL, NULL }, 
};

FieldRec rnode_fields[] = {
  { ".r_fh", STRUCTURE, NULL, NULL },
  { ".r_cred", NUMBER, NULL, NULL },
  { ".r_attr.va_fileid", NUMBER, NULL, NULL },
  { ".r_attr.va_mode", NUMBER, NULL, NULL },
  { ".r_attr.va_uid", NUMBER, NULL, NULL },
  { ".r_attr.va_gid", NUMBER, NULL, NULL },
  { ".r_attr.va_qsize", NUMBER, NULL, NULL },
};
/*  { ".r_attr", STRUCTURE, NULL, NULL },*/

FieldRec cdnode_fields[] = {
  { ".cd_flag", NUMBER, NULL, NULL },
  { ".cd_mode", NUMBER, NULL, NULL },
  { ".cd_uid", NUMBER, NULL, NULL },
  { ".cd_gid", NUMBER, NULL, NULL },
  { ".cd_cdin.dir_dat_len", NUMBER, NULL, NULL },
};

#define NUM_VNODE_FIELDS (sizeof(vnode_fields)/sizeof(vnode_fields[0]))
#define NUM_INODE_FIELDS (sizeof(inode_fields)/sizeof(inode_fields[0]))
#define NUM_RNODE_FIELDS (sizeof(rnode_fields)/sizeof(rnode_fields[0]))
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
  char buf[256], typebuf[6], tagbuf[8], irnode_info[60]="", *error ;
  /* It is VERY important that these flags are initialized to zero */
  int allflag=0, freeflag=0, fsflag=0, gidflag=0, uidflag=0;
  int mfsflag=0, ufsflag=0, nfsflag=0, vflag=0, guflag=0;
  int cdfsflag=0, count=0, short_address=1; /* set short_address=0 to get long addr */
  long fsarg=0, gid=0, uid=0;
  char short_addr1[12], short_addr2[12];

  /*** parse command line arguments ***/

  check_args(argc, argv, help_string);
  for (i = 1; i < argc; i++)
      /*   if (!strcmp(argv[i], "-help")) usage();*/
      if (!strcmp(argv[i], "-all"))  allflag = 1;
      else if (!strcmp(argv[i], "-free")) freeflag = 1;
      else if (!strcmp(argv[i], "-fs"))  {fsflag = 1; fsarg = strtoul(argv[++i],(char**)NULL,16);}
      else if (!strcmp(argv[i], "-g"))   {gidflag  = 1; gid = atol(argv[++i]); guflag=1;}
      else if (!strcmp(argv[i], "-u"))   {uidflag  = 1; uid = atol(argv[++i]); guflag=1;}
      else if (!strcmp(argv[i], "-v"))    vflag = 1; 
      else if (!strcmp(argv[i], "-ufs"))  ufsflag = 1; 
      else if (!strcmp(argv[i], "-nfs"))  nfsflag = 1; 
      else if (!strcmp(argv[i], "-cdfs"))  cdfsflag = 1; 
      else {
        fprintf(stderr, "%s: invalid option, `%s'\n", argv[0], argv[i]);
        quit(1);
      }
  if (vflag && !(ufsflag || nfsflag || cdfsflag)){
    sprintf(buf,"The -v option can be used only with the -ufs, -nfs or -cdfs option");
    print(buf);
    exit(0);
  }

  /*** check fields ***/

  if(!check_fields("struct vnode", vnode_fields, NUM_VNODE_FIELDS, NULL)){
    field_errors(vnode_fields, NUM_VNODE_FIELDS);
    quit(1);
  }
  if(!check_fields("struct inode", inode_fields, NUM_INODE_FIELDS, NULL)){
    field_errors(inode_fields, NUM_INODE_FIELDS);
    quit(1);
  }

  /*** read value of vnode, nvnode and vn_maxprivate ***/

  /* This gets the value of vnode and deposits it in addr */
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

  if (short_address)
    sprintf(buf, "ADDR_VNODE  V_TYPE  V_TAG USECNT HLDCNT  V_MOUNT    ");
  else 
    sprintf(buf, " ADDR OF VNODE     V_TYPE  V_TAG USECNT HLDCNT     V_MOUNT       ");
  if (ufsflag && vflag)  strcat(buf," INODE#  MODE   UID  GID  QSIZE");
  if (nfsflag && vflag)  strcat(buf," FILEID  MODE   UID  GID  QSIZE");
  if (cdfsflag && vflag) strcat(buf," CDFLAG  MODE   UID  GID  CDSIZE");
  print(buf);
  if (short_address)
    sprintf(buf, "=========== ====== ====== ====== ====== ===========");
  else 
    sprintf(buf, "================== ====== ====== ====== ====== ==================");
  if ((ufsflag || nfsflag || cdfsflag) && vflag) strcat(buf,"  ====== ======  ==== ==== ======");
  print(buf);
  
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

    /* filter #1 -- don't read any further unless really necessary */
    if ((!allflag && !freeflag) && ((vnode_fields[1].data == 0) && (vnode_fields[2].data == 0)))
       continue;  /* by default -- ACTIVE nodes only */
    if (freeflag && ((vnode_fields[1].data != 0) || (vnode_fields[2].data != 0)))
       continue;
    if (fsflag && ((long)vnode_fields[3].data != fsarg)) continue;
    if (ufsflag && (((int)vnode_fields[6].data != VT_UFS))) continue;
    if (nfsflag && (((int)vnode_fields[6].data != VT_NFS))) continue;
    if (cdfsflag && (((int)vnode_fields[6].data != VT_CDFS))) continue;

    /*** dive into inode or rnode only if "-ufs", "-nfs" or "-cdfs" is used ***/

    if (vflag || guflag) {

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

        /* filter #2 */
        if (uidflag && ((long)inode_fields[3].data != uid)) continue;
        if (gidflag && ((long)inode_fields[4].data != gid)) continue;

        if (vflag)
        sprintf(irnode_info,"%6d %6o %4d %4d %7d", inode_fields[1].data, inode_fields[2].data,
          inode_fields[3].data, inode_fields[4].data, inode_fields[5].data);
      }  /* end of if(VT_UFS) */

      else if (((int)vnode_fields[6].data  == VT_NFS)){

        /*** get rnode info ***/
        if(!cast((long)(addr1+offset), "struct rnode", &rnode, &error)){
        fprintf(stderr, "Couldn't cast to rnode:\n");
        fprintf(stderr, "%s\n", error);
        return(False);
        }
     
        if(!read_field_vals(rnode, rnode_fields, NUM_RNODE_FIELDS)){
          field_errors(rnode_fields, NUM_RNODE_FIELDS);
          return(False);
        }

        if (uidflag && ((long)rnode_fields[4].data != uid)) continue;
        if (gidflag && ((long)rnode_fields[5].data != gid)) continue;

        if (vflag)
        sprintf(irnode_info,"%6d %6o %4d %4d %7d", rnode_fields[2].data, rnode_fields[3].data,
          rnode_fields[4].data, rnode_fields[5].data, rnode_fields[6].data);
      }  /* end of if(VT_NFS) */

      else if (((int)vnode_fields[6].data  == VT_CDFS)){

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

        if (uidflag && ((long)cdnode_fields[2].data != uid)) continue;
        if (gidflag && ((long)cdnode_fields[3].data != gid)) continue;

        if (vflag)
        sprintf(irnode_info,"%6d %6o %4d %4d %7d", cdnode_fields[0].data, cdnode_fields[1].data,
          cdnode_fields[2].data, cdnode_fields[3].data, cdnode_fields[4].data);
      }  /* end of if(VT_CDFS) */

    } /* end of if(vflag) */

    /*** output section ***/

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

    /* print out the buffer */
    if (short_address)
    {
      format_addr((long)addr1, short_addr1);
      format_addr((long)vnode_fields[3].data, short_addr2);
      sprintf(buf, "%s %5s %7s %5d %6d  %s %s", short_addr1, typebuf, tagbuf, 
        vnode_fields[1].data, vnode_fields[2].data, short_addr2, irnode_info);
    }
    else
    {
      sprintf(buf, "0x%16lx %5s %7s %5d %6d  0x%16lx %s", addr1, typebuf, tagbuf, 
      vnode_fields[1].data, vnode_fields[2].data, vnode_fields[3].data, irnode_info);
    }
    print(buf);
    count++;
  } /* end of for loop */
  sprintf(buf, "total count = %d", count);
  print(buf);
}
