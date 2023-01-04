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
static char *rcsid = "@(#)$RCSfile: ofile.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/11/17 14:54:49 $";
#endif

/* This kdbx's extension prints the open files' structures. */

#include <stdio.h>
#include <sys/file.h>
#include <sys/user.h>
#include <sys/vnode.h>
#include <krash.h>

static char *help_string =
"ofile - print the open file table                                     \\\n\
    Usage : ofile [options]                                        \\\n\
     (no options) list all files opened by each process in the system\\\n\
     -proc <addr> list all files opened by a process \\\n\
     -pid  <pid>  list all files opened by a process \\\n\
     -v           verbose mode \\\n\
";

FieldRec proc_fields[] = {
  { ".p_nxt", NUMBER, NULL, NULL },
  { ".p_pid", NUMBER, NULL, NULL },
  { ".utask", NUMBER, NULL, NULL },
  { ".thread->u_address.uthread", NUMBER, NULL, NULL }, /* a ptr to uthread */
  { ".thread->u_address.uthread->uu_nd.ni_cred", NUMBER, NULL, NULL },

};

FieldRec utask_fields[] = {
  { ".uu_file_state.uf_ofile", ARRAY, NULL, NULL },
  { ".uu_file_state.uf_pofile", ARRAY, NULL, NULL },
  { ".uu_file_state.uf_lastfile", NUMBER, NULL, NULL },
  { ".uu_file_state.uf_of_count", NUMBER, NULL, NULL },
  { ".uu_file_state.uf_ofile_of", NUMBER, NULL, NULL },
  { ".uu_file_state.uf_pofile_of", NUMBER, NULL, NULL },
};

FieldRec file_fields[] = {
  { ".f_flag", NUMBER, NULL, NULL },
  { ".f_count", NUMBER, NULL, NULL },
  { ".f_type", NUMBER, NULL, NULL },
  { ".f_data", NUMBER, NULL, NULL },
  { ".f_u.fu_offset", NUMBER, NULL, NULL },
};

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


FieldRec thread_fields[] = {
  { ".u_address->uthread", NUMBER, NULL, NULL },  /* a ptr to uthread */
};

FieldRec uthread_fields[] = {
  { ".uu_nd.ni_cred", NUMBER, NULL, NULL }, /* a ptr to ucred */
};

#define NUM_PROC_FIELDS (sizeof(proc_fields)/sizeof(proc_fields[0]))
#define NUM_FILE_FIELDS (sizeof(file_fields)/sizeof(file_fields[0]))
#define NUM_UTASK_FIELDS (sizeof(utask_fields)/sizeof(utask_fields[0]))


main(int argc, char **argv)
{
  DataStruct proc, utask, file, ele, ofile, pofile;
  long procaddr, fileaddr, nfile, fp, addrarg=0, pidarg=0;
  long val1, val2, fptr, start_addr;
  char *error, bufout[256], fflg;
  int slot, i, j, n, pid, index ;
  int procflag=0, pidflag=0, vflag=0;

  DataStruct inode, rnode, cdnode ;
  long  nvnode, datasize, addr, addr1, cur_node ;
  char  typebuf[6], tagbuf[8], irnode_info[60]="", vnode_info[60]="" ;
  char vnode_saddr[12], mount_saddr[12], file_saddr[12];

  /*** parse command line arguments ***/

  check_args(argc, argv, help_string);
  for (i = 1; i < argc; i++)
      if (!strcmp(argv[i], "-proc")) {procflag=1;
                       addrarg=strtoul(argv[++i],(char**)NULL,16);}
      else if (!strcmp(argv[i], "-pid"))  {pidflag=1;
                       pidarg=atoi(argv[++i]);}
      else if (!strcmp(argv[i], "-v"))    {vflag=1;}
      else {
        fprintf(stderr, "%s: invalid option, `%s'\n", argv[0], argv[i]);
        quit(1);
      }

  if(!check_fields("struct file", file_fields, NUM_FILE_FIELDS, NULL)){
      field_errors(file_fields, NUM_FILE_FIELDS);
      quit(1);
  }
  
  
      /*
       * Get the pointer to the open file table for all processes
       * Let's start with allproc.
       */

      if(!check_fields("struct proc", proc_fields, NUM_PROC_FIELDS, NULL)){
        field_errors(proc_fields, NUM_PROC_FIELDS);
        quit(1);
      }
      if(!read_sym_val("allproc", NUMBER, &procaddr, &error)){
        fprintf(stderr, "Couldn't read allproc:\n");
        fprintf(stderr, "%s\n", error);
        quit(1);
      }

      /* will loop through all processes */

      do {

        /* read a proc structure */
        if(!cast(procaddr, "struct proc", &proc, &error)){
          fprintf(stderr, "Couldn't cast addr to a proc:\n");
          fprintf(stderr, "%s\n", error);
          quit(1);
        }
        if(!read_field_vals(proc, proc_fields, NUM_PROC_FIELDS)){
          field_errors(proc_fields, NUM_PROC_FIELDS);
          quit(1);
        }
        if (procflag && ((long)procaddr != addrarg)) goto cont;
        if (pidflag && ((long)proc_fields[1].data != pidarg)) goto cont;

        /* print out the proc info */
        print(" ");
        sprintf(bufout,"Proc=0x%16lx   pid=%5d ", procaddr, proc_fields[1].data);
        print(bufout);

                                                                                
        /* read a utask structure */
        if ((long)proc_fields[2].data) { /* skip process #0 */
          if(!cast((long)proc_fields[2].data, "struct utask", &utask, &error)){
            fprintf(stderr, "Couldn't cast addr to a utask:\n");
            fprintf(stderr, "%s\n", error);
            quit(1);
          }
          if(!read_field_vals(utask, utask_fields, NUM_UTASK_FIELDS)){
            field_errors(utask_fields, NUM_UTASK_FIELDS);
            quit(1);
          }

          print(" ");
          if (vflag) {
            print(" ADDR_FILE   f_cnt  ADDR_VNODE V_TYPE  V_TAG   USECNT HLDCNT  V_MOUNT    INO#    QSIZE");
            print("=========== ====== =========== ====== ======= ======= ====== =========== ======  ======");
          }

          /*
           * Loop through the open file array, utask_fields[2].data points to
           * or beyond the last open file.
           */
          for (i=0; i<= (int)utask_fields[2].data; i++)
          {
            if (i < NOFILE_IN_U) {  /* no overflow */
              if(!array_element_val((long)utask_fields[0].data , i, &fptr, &error)){
                fprintf(stderr, "Couldn't get array element\n");
                fprintf(stderr, "%s\n", error);
                quit(1);
              }
            }
            else {
              start_addr = (long) ((long *)utask_fields[4].data + i-NOFILE_IN_U);
              if(!read_memory(start_addr , sizeof(long *), &fptr, &error)){
                fprintf(stderr, "Couldn't read_memory\n");
                fprintf(stderr, "%s\n", error);
                quit(1);
              }
            }

            if (fptr) { /* print/process only non-NULL entries */
              if (vflag) {  /* read into file structure */
                strcpy(vnode_info, ""); strcpy(irnode_info,"");
                if(!cast(fptr, "struct file", &file, &error)){
                  fprintf(stderr, "Couldn't cast addr to a file:\n");
                  fprintf(stderr, "%s\n", error);
                  quit(1);
                }
                if(!read_field_vals(file, file_fields, NUM_FILE_FIELDS)){
                  field_errors(file_fields, NUM_FILE_FIELDS);
                  quit(1);
                }

                /**** dive in ****/

                /* read into vnode */
                if(!cast((long)file_fields[3].data, "struct vnode", &cur_node, &error)){
                  fprintf(stderr, "Couldn't cast to a vnode:\n");
                  fprintf(stderr, "%s\n", error);
                  quit(1);
                }

                /* get the values of the current struct vnode into vnode_fields[] */
                if(!read_field_vals(cur_node, vnode_fields, NUM_VNODE_FIELDS)){
                  field_errors(vnode_fields, NUM_VNODE_FIELDS);
                  quit(1);
                }
                
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

                format_addr((long)vnode_fields[3].data, mount_saddr);
                sprintf(vnode_info, " %5s %7s %5d %6d   %s", typebuf, tagbuf,
                  vnode_fields[1].data, vnode_fields[2].data, mount_saddr); 

                /* save the current vnode address and compute the next one */
                addr1 = (long)file_fields[3].data;
                /* inc by sizeof(struct vnode) plus sizeof private data */
                addr = addr + offset + datasize ; 

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
                  sprintf(irnode_info,"%6d %7d", inode_fields[1].data, 
                  inode_fields[5].data);
                }

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
                  sprintf(irnode_info,"%6d %7d", rnode_fields[2].data, 
                    rnode_fields[6].data);
                }

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
                  sprintf(irnode_info,"%6d %7d", cdnode_fields[0].data, 
                    cdnode_fields[4].data);
                }

                /**** end of dive in ****/

                format_addr((long)fptr, file_saddr);
                format_addr((long)file_fields[3].data, vnode_saddr);
                sprintf(bufout,"%s %4d   %s %s %s", file_saddr, file_fields[1].data,
                  vnode_saddr, vnode_info, irnode_info );
                print(bufout);
              }
              else /* !vflag */
              {
                sprintf(bufout, "\tofile[%2d]=0x%p", i, fptr);
                print(bufout);
              }
            }
          } /* end of for */
        } /* if not process #0 */

cont:
	slot++;
        procaddr = (long) proc_fields[0].data;
      } while(procaddr != 0);


} /* end of program */
