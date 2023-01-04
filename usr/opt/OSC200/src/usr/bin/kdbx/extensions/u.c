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
static char *rcsid = "@(#)$RCSfile: u.c,v $ $Revision: 1.1.10.4 $ (DEC) $Date: 1993/08/23 21:32:38 $";
#endif
#include <stdio.h>
#include <sys/user.h>
#include "krash.h"

static char *help_string =
"u - print a u structure                                                  \\\n\
    Usage : u [proc-addr]                                                 \\\n\
    If proc-addr is present, the u area associated with it is printed.    \\\n\
    Otherwise the u area is taken from active_threads[0]->u_address.utask \\\n\
";

#ifndef NOFILE_IN_U
#define NOFILE_IN_U 64
#endif

FieldRec utask_fields[] = {
  { ".uu_procp", NUMBER, NULL, NULL },
  { ".uu_comm", STRING, NULL, NULL },
  { ".uu_argp", POINTER, NULL, NULL },
  { ".uu_arg_size", NUMBER, NULL, NULL },
  { ".uu_file_state.uf_lastfile", NUMBER, NULL, NULL },
  { ".uu_file_state.uf_ofile", ARRAY, NULL, NULL },
  { ".uu_file_state.uf_pofile", ARRAY, NULL, NULL },
  { ".uu_file_state.uf_ofile_of", NUMBER, NULL, NULL },
  { ".uu_file_state.uf_pofile_of", NUMBER, NULL, NULL },
  { ".uu_file_state.uf_of_count", NUMBER, NULL, NULL },
  { ".uu_tsize", NUMBER, NULL, NULL },
  { ".uu_dsize", NUMBER, NULL, NULL },
  { ".uu_ssize", NUMBER, NULL, NULL },
  { ".uu_outime", NUMBER, NULL, NULL },
  { ".uu_signal", ARRAY, NULL, NULL },
  { ".uu_sigmask", ARRAY, NULL, NULL },
  { ".uu_sigonstack", NUMBER, NULL, NULL },
  { ".uu_oldmask", NUMBER, NULL, NULL },
  { ".uu_sigstack.ss_sp", NUMBER, NULL, NULL },
  { ".uu_sigstack.ss_flags", NUMBER, NULL, NULL },
  { ".uu_utnd.utnd_cdir", NUMBER, NULL, NULL },
  { ".uu_utnd.utnd_rdir", NUMBER, NULL, NULL },
  { ".uu_cmask", NUMBER, NULL, NULL },
  { ".uu_timer", ARRAY, NULL, NULL },
  { ".uu_timer[0].it_interval.tv_sec", NUMBER, NULL, NULL },
  { ".uu_timer[0].it_interval.tv_usec", NUMBER, NULL, NULL },
  { ".uu_timer[0].it_value.tv_sec", NUMBER, NULL, NULL },
  { ".uu_timer[0].it_value.tv_usec", NUMBER, NULL, NULL },
  { ".uu_start.tv_sec", NUMBER, NULL, NULL },
  { ".uu_start.tv_usec", NUMBER, NULL, NULL },
  { ".uu_acflag.fi_flag", STRUCTURE, NULL, NULL },
  { ".uu_sigstack.ss_size", NUMBER, NULL, NULL },
};

#ifdef notdef
char *utask_hints[] = { "struct proc", NULL, NULL, NULL, NULL, "struct file",
			  NULL, "struct file", NULL, NULL, NULL, NULL, NULL,
			  NULL, NULL, NULL, NULL, NULL, NULL, NULL,
			  "struct vnode", "struct vnode", NULL, NULL, NULL,
			  NULL, NULL, NULL, NULL, NULL, NULL, NULL };
#endif

char *utask_how_hints = { ".whatis_struct" };

static KHints utask_hints = { &utask_how_hints, NULL };

#define NUM_UTASK_FIELDS (sizeof(utask_fields)/sizeof(utask_fields[0]))

FieldRec uthread_fields[] = {
  { ".uu_ar0", NUMBER, NULL, NULL }
#ifdef notdef
  { ".", NUMBER, NULL, NULL },
  { ".", NUMBER, NULL, NULL },
  { ".", NUMBER, NULL, NULL },
  { ".", NUMBER, NULL, NULL },
  { ".", NUMBER, NULL, NULL },
  { ".", NUMBER, NULL, NULL },
  { ".", NUMBER, NULL, NULL },
  { ".", NUMBER, NULL, NULL },
  { ".", NUMBER, NULL, NULL },
  { ".", NUMBER, NULL, NULL },
  { ".", NUMBER, NULL, NULL },
#endif
};

char *uthread_hints[] = { "" };

#define NUM_UTHREAD_FIELDS (sizeof(uthread_fields)/sizeof(uthread_fields[0]))

FieldRec proc_fields[] = {
  { ".thread->u_address.uthread", NUMBER, NULL, NULL },
  { ".utask", NUMBER, NULL, NULL }
};

#define NUM_PROC_FIELDS (sizeof(proc_fields)/sizeof(proc_fields[0]))

static void Usage(){
  fprintf(stderr, "Usage : u [proc addr]\n");
  quit(1);
}

main(argc, argv)
int argc;
char **argv;
{
  char buf[256], *slot, *ptr, *error, *args, *end, c, fflg;
  int i, n;
  long procslot, utaskaddr, uthreadaddr, procaddr, fp, val1, val2, start_addr;
  DataStruct utask, uthread, proc, ofile, pofile;

  check_args(argc, argv, help_string);
  if(argc == 1){
    if(!read_sym_val("active_threads[0]->u_address.utask", NUMBER, &utaskaddr,
		     &error)){
      fprintf(stderr, "Couldn't read active_threads[0]->u_address.utask:\n");
      fprintf(stderr, "%s:\n", error);
      quit(1);
    }
    if(!read_sym_val("active_threads[0]->u_address.uthread", NUMBER,
		     &uthreadaddr, &error)){
      fprintf(stderr, "Couldn't read active_threads[0]->u_address.uthread:\n");
      fprintf(stderr, "%s:\n", error);
      quit(1);
    }
  }
  else if(argc == 2){ 
    procslot = to_address(argv[1], &error);
    if((procslot == 0) && error){
      fprintf(stderr, "%s\n", error);
      quit(1);
    }
    if(!read_sym_val("allproc", NUMBER, &procaddr, &error)){
      fprintf(stderr, "Couldn't read allproc:\n");
      fprintf(stderr, "%s\n", error);
      quit(1);
    }
    if(!check_fields("struct proc", proc_fields, NUM_PROC_FIELDS, NULL)){
      field_errors(proc_fields, NUM_PROC_FIELDS);      
      quit(1);
    }
    if(!cast(procslot, "struct proc", &proc, &error)){
      fprintf(stderr, "Couldn't cast procslot to a proc:\n");
      fprintf(stderr, "%s\n", error);
      quit(1);
    }
    if(!read_field_vals(proc, proc_fields, NUM_PROC_FIELDS)){
      field_errors(proc_fields, NUM_PROC_FIELDS);
      quit(1);
    }
    uthreadaddr = (DataStruct) proc_fields[0].data;
    utaskaddr = (DataStruct) proc_fields[1].data;
  }
  else Usage();
  if(!check_fields("struct utask", utask_fields, NUM_UTASK_FIELDS, NULL)){
    field_errors(utask_fields, NUM_UTASK_FIELDS);
    quit(1);
  }
  if(!check_fields("struct uthread", uthread_fields, NUM_UTHREAD_FIELDS,
		   NULL)){
    field_errors(uthread_fields, NUM_UTHREAD_FIELDS);
    quit(1);
  }
  if(!cast(utaskaddr, "struct utask", &utask, &error) ||
     !cast(uthreadaddr, "struct uthread", &uthread, &error)){
    fprintf(stderr, "Couldn't cast utaskaddr or uthreadaddr:\n");
    fprintf(stderr, "%s\n", error);
    quit(1);
  }
  if(!read_field_vals(utask, utask_fields, NUM_UTASK_FIELDS)){
    field_errors(utask_fields, NUM_UTASK_FIELDS);
    quit(1);
  }
  if(!read_field_vals(uthread, uthread_fields, NUM_UTHREAD_FIELDS)){
    field_errors(uthread_fields, NUM_UTHREAD_FIELDS);
    quit(1);
  }
  sprintf(buf, "procp\t0x%08x", utask_fields[0].data);
  print(buf);
  sprintf(buf, "ar0\t0x%08x", uthread_fields[0].data);
  print(buf);
  sprintf(buf, "comm\t %s", utask_fields[1].data);
  print(buf);
#ifdef notdef
  NEW_TYPE(args, strlen("args\t") + (int) utask_fields[3].data + 1, char,
	   char *, "main");
  strcpy(args, "args\t");
  end = &args[strlen("args\t")];
  for(i=0;i<(int) utask_fields[3].data;i++){
    if(!array_element_val((DataStruct) utask_fields[2].data, i, &val1,
			  &error)){
      fprintf(stderr, "Couldn't read %d'th character of args:\n", i);
      fprintf(stderr, "%s\n", error);
      quit(1);
    }
    c = val1;
    if(c == '\0') *end = ' ';
    else *end = c;
    end++;
  }
  *end = '\0';
  print(args);
  free(args);

#endif
  sprintf(buf,"args\t%s",(char *)utask_fields[2].data);
  print(buf);
  sprintf(buf, "u_ofile: (u_lastfile: %d)", utask_fields[4].data);
  printf(buf);
  if((long) utask_fields[4].data != 0){
    sprintf(buf, "u_ofile_of: 0x%8x u_pofile_of: 0x%8x",
	    struct_addr((DataStruct) utask_fields[5].data),
	    struct_addr((DataStruct) utask_fields[6].data));
    print(buf);
  }

  if((n = array_size((DataStruct) utask_fields[5].data, &error)) == -1){
    fprintf(stderr, "Couldn't read array size:\n", i);
    fprintf(stderr, "%s: %s\n", utask_fields[5].name, error);
    quit(1);    
  }

  for(i=0;i<=(int)utask_fields[4].data;i++){
    if(i>=NOFILE_IN_U){
      start_addr = (long) ((long *)utask_fields[7].data + i-NOFILE_IN_U);
      if(!read_memory(start_addr , sizeof(long *), &val1, &error) ||
         !read_memory((long)utask_fields[8].data , sizeof(long *), &val2, &error)){
        fprintf(stderr, "Couldn't read_memory\n");
        fprintf(stderr, "%s\n", error);
        quit(1);
      }
    }
    else {
      ofile = (DataStruct) utask_fields[5].data;
      pofile = (DataStruct) utask_fields[6].data;      
    }
    if (i < NOFILE_IN_U)
    if(!array_element_val(ofile, i, &val1, &error) ||
       !array_element_val(pofile, i, &val2, &error)){
      fprintf(stderr, "Couldn't read %d'th element of ofile or pofile:\n", i);
      fprintf(stderr, "%s\n", error);
      quit(1);
    }
    fp = val1;
    fflg = val2;
    if(fp == 0) continue;
    sprintf(buf, " %d 0x%p  %s%s", i, fp,
	    (int) fflg & UF_EXCLOSE ? "Auto-close " : "",
	    (int) fflg & UF_MAPPED ? "Mapped " : "");
    print(buf);
  }
  sprintf(buf, "sizes\t%d %d %d (clicks)", utask_fields[10].data,
	  utask_fields[11].data, utask_fields[12].data);
  print(buf);
  sprintf(buf, "u_outime\t%d", utask_fields[13].data);
  print(buf);
  print("sigs");
  buf[0] = '\0';
  for(i=0;i<NSIG;i++){
    if(i % 8 == 0) strcat(buf, "\t");
    if(!array_element_val((DataStruct) utask_fields[14].data, i, &val1,
			  &error)){
      fprintf(stderr, "Couldn't read %d'th element of signal:\n", i);
      fprintf(stderr, "%s\n", error);
      quit(1);
    }
    sprintf(&buf[strlen(buf)], "%5.1x ", n);
    if (i % 8 == 7){
      print(buf);
      buf[0] = '\0';
    }
  }
  if(NSIG % 8 != 0) print("");
  print("sigmask");
  for(i=0;i<NSIG;i++){
    if(i % 8 == 0) strcat(buf, "\t");
    if(!array_element_val((DataStruct) utask_fields[15].data, i, &val1,
			  &error)){
      fprintf(stderr, "Couldn't read %d'th element of signal:\n", i);
      fprintf(stderr, "%s\n", error);
      quit(1);
    }
    n = val1;
    sprintf(&buf[strlen(buf)], "%5.1x ", n);
    if (i % 8 == 7){
      print(buf);
      buf[0] = '\0';
    }
  }
  if(NSIG % 8 != 0) print("");
  sprintf(buf, "sigonstack\t%9.1x", (long)utask_fields[16].data & SS_ONSTACK);
  print(buf);
  sprintf(buf, "oldmask   \t%9.1x", utask_fields[17].data);
  print(buf);
  sprintf(buf, "sigstack  \t%9.1x\t%9.1x\t%9.1x", utask_fields[18].data,
	  utask_fields[31].data, utask_fields[19].data);
  print(buf);
  sprintf(buf, "cdir rdir\t%9.1x %9.1x", utask_fields[20].data,
	  utask_fields[21].data);
  print(buf);
  sprintf(buf, "cmask    \t0%o", utask_fields[22].data);
  print("timers");
  if((n = array_size((DataStruct) utask_fields[23].data, &error)) == -1){
    fprintf(stderr, "Couldn't read array size:\n", i);
    fprintf(stderr, "%s: %s\n", utask_fields[23].name, error);
    quit(1);    
  }
  for(i=0;i<n;i++){
    sprintf(buf, "\t%12d %12d %12d %12d", utask_fields[23].data,
	    utask_fields[24].data, utask_fields[25].data,
	    utask_fields[26].data);
  }
  sprintf(buf, "start    \t%12d %12d", utask_fields[27].data,
	  utask_fields[28].data);
  print(buf);
  sprintf(buf, "acflag   \t%d", utask_fields[29].data);
  print(buf);
#ifdef notdef
  print("limits");
  for(i=0;i<RLIM_NLIMITS;i++) {
    sprintf(buf, "%d ",U.u_rlimit[i]);
  }
        printf("\n");
        printf("quota    \t%9.1x\n",U.u_quota);
        printf("quotaflag\t%9.1x\n",U.u_qflags);
        printf("smem     \t%9.1x %9.1x\n",
                U.u_smsize,U.u_lock);
        printf("prof     \t%9.1x %9.1x %9.1x %9.1x\n",
                U.u_prof.pr_base, U.u_prof.pr_size,
                U.u_prof.pr_off, U.u_prof.pr_scale);
        printf("u_nache  \toff %d ino %d dev %d tim %d\n",
                U.u_ncache.nc_prevoffset,U.u_ncache.nc_inumber,
                U.u_ncache.nc_dev,U.u_ncache.nc_time);
        printf("nameidata\n");
        printf("\tnameiop, error, endoff\t%8x %8d %8d\n",
                nd->ni_nameiop,nd->ni_error,nd->ni_endoff);
        printf("\t   base, count, offset\t%8x %8d %8d\n",
                nd->ni_base,nd->ni_count,nd->ni_offset);
        printf("\tdent ino %d name %.14s dirp %p\n",
                nd->ni_dent.d_ino,nd->ni_dent.d_name, nd->ni_dirp);
        printf("\tsegflg\t%8d\n", nd->ni_segflg);
        printf("u_stack  \t%9.1x\n",&U.u_stack[0]);
#endif
  quit(0);
}
