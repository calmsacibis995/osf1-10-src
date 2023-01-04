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
static char *rcsid = "@(#)$RCSfile: crcheck.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/11/05 17:50:12 $";
#endif
#include <stdio.h>
#include "krash.h"

/* buf.b_rcred
   buf.b_wcred
   file.f_cred
   proc.p_rcred
   nameidata.ni_cred

   ./bsd/SCCS/s.init_main.c
   ./bsd/SCCS/s.kern_exec.c
   ./bsd/SCCS/s.kern_prot.c
   ./bsd/kern_exec.c
   ./bsd/kern_prot.c
   ./nfs/SCCS/s.nfs_syscalls.c
   ./nfs/SCCS/s.nfs_vnops.c
   ./nfs/nfs_syscalls.c
   ./nfs/nfs_vnops.c
   ./vfs/SCCS/s.vfs_syscalls.c
   ./vfs/vfs_syscalls.c
*/

static char *help_string =
"crcheck - verify correctness of ucred reference counts                   \\\n\
    Usage : crcheck                                                       \\\n\
";

FieldRec file_fields[] = {
  { ".f_cred", NUMBER, NULL, NULL },
  { ".f_count", NUMBER, NULL, NULL }
};

#define NUM_FILE_FIELDS (sizeof(file_fields)/sizeof(file_fields[0]))

FieldRec cred_fields[] = {
  { ".cr_ref", NUMBER, NULL, NULL }
};

#define NUM_CRED_FIELDS (sizeof(cred_fields)/sizeof(cred_fields[0]))

FieldRec proc_fields[] = {
  { ".p_rcred", NUMBER, NULL, NULL },
  { ".p_nxt", NUMBER, NULL, NULL },
  { ".thread->u_address.uthread->uu_nd.ni_cred", NUMBER, NULL, NULL }
};

#define NUM_PROC_FIELDS (sizeof(proc_fields)/sizeof(proc_fields[0]))

#define NUMCREDS 1024

struct crinfo {
  unsigned addr;
  long ref;
};

struct crinfo crtab[NUMCREDS];

static int get_crinfo(addr, c)
unsigned addr;
struct crinfo **c;
{
  int i;

  for(i=0;i<NUMCREDS;i++){
    if(crtab[i].addr == 0) break;
    if(crtab[i].addr == addr){
      *c = &crtab[i];
      return(1);
    }
  }
  if (i == NUMCREDS) return(0);
  crtab[i].addr = addr;
  crtab[i].ref = 0;
  *c = &crtab[i];
  return(1);
}

static void file_creds()
{
  int i;
  long nfile;
  DataStruct fil, ele;
  char *error;
  struct crinfo *c;
  
  fil = read_sym("file");
  if(!read_sym_val("nfile", NUMBER, &nfile, &error)){
    fprintf(stderr, "Couldn't read nfile:\n");
    fprintf(stderr, "%s\n", error);
    quit(1);
  }
  for(i=0;i<nfile;i++){
    if((ele = array_element(fil, i, &error)) == NULL){
      fprintf(stderr, "Couldn't get array element\n");
      fprintf(stderr, "%s\n", error);
      quit(1);
    }
    if(!read_field_vals(ele, file_fields, NUM_FILE_FIELDS)){
      field_errors(file_fields, NUM_FILE_FIELDS);
      quit(1);
    }
    if(file_fields[1].data != 0){
      if(file_fields[0].data == 0)
	fprintf(stderr, "File entry %d has null cred\n", i);
      else if(get_crinfo(file_fields[0].data, &c)) c->ref++;
    }
  }
}

static void proc_creds(){
  DataStruct proc;
  long procaddr;
  char *error;
  struct crinfo *c;

  if(!read_sym_val("allproc", NUMBER, &procaddr, &error)){
    fprintf(stderr, "Couldn't read allproc:\n");
    fprintf(stderr, "%s\n", error);
    quit(1);
  }
  do {
    if(!cast(procaddr, "struct proc", &proc, &error)){
      fprintf(stderr, "Couldn't cast addr to a proc:\n");
      fprintf(stderr, "%s\n", error);
      return;
    }
    if(!read_field_vals(proc, proc_fields, NUM_PROC_FIELDS)){
      field_errors(proc_fields, NUM_PROC_FIELDS);
      return;
    }
    if(proc_fields[0].data == 0)
      fprintf(stderr, "Proc 0x%p has null cred\n", procaddr);
    else if(get_crinfo(proc_fields[0].data, &c)) c->ref++;
    if(proc_fields[2].data == 0)
      fprintf(stderr, "Proc 0x%p has null namei cred\n", procaddr);
    else if(get_crinfo(proc_fields[2].data, &c)) c->ref++;
    procaddr = (long) proc_fields[1].data;
  } while(procaddr != 0);
}

static void ni_creds(){
  
}

main(argc, argv)
int argc;
char **argv;
{
  int i;
  char *error;
  DataStruct cred;
  struct crinfo *c;

  check_args(argc, argv, help_string);
  if(!check_fields("struct file", file_fields, NUM_FILE_FIELDS, NULL)){
    field_errors(file_fields, NUM_FILE_FIELDS);
    quit(1);
  }
  if(!check_fields("struct proc", proc_fields, NUM_PROC_FIELDS, NULL)){
    field_errors(proc_fields, NUM_PROC_FIELDS);
    quit(1);
  }
  if(!check_fields("struct ucred", cred_fields, NUM_CRED_FIELDS, NULL)){
    field_errors(cred_fields, NUM_CRED_FIELDS);
    quit(1);
  }
  file_creds();
  proc_creds();
  ni_creds();
  for(i=0;i<NUMCREDS;i++){
    if(crtab[i].addr == 0) break;
    if(!cast(crtab[i].addr, "struct ucred", &cred, &error)){
      fprintf(stderr, "Couldn't cast address to a ucred:\n");
      fprintf(stderr, "%s\n", error);
      quit(1);
    }
    if(!read_field_vals(cred, cred_fields, NUM_CRED_FIELDS)){
      field_errors(cred_fields, NUM_CRED_FIELDS);
      quit(1);
    }
    if(crtab[i].ref != (int) cred_fields[0].data){
      fprintf(stderr, "Cred 0x%p has ref %d should be %d\n",
	     crtab[i].addr, cred_fields[0].data, crtab[i].ref);
    }
  }
}
