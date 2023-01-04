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
static char *rcsid = "@(#)$RCSfile: ucred.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/11/17 14:54:53 $";
#endif

/* This kdbx's extension prints the ucred structures. */

#include <stdio.h>
#include "krash.h"

static char *help_string =
"ucred - print the ucred table                                     \\\n\
    Usage : ucred [options]                                        \\\n\
     (no options) list all ucred references by the proc/uthread/file/buf structures\\\n\
     -proc        list all ucred referenced by the proc structures \\\n\
     -file        list all ucred referenced by active file structures \\\n\
     -uthread     list all ucred referenced by the uthread structures \\\n\
     -buf         list all ucred referenced by the buf structures \\\n\
     -ref <addr>  list all data structures which reference a ucred structure\\\n\
     -check <addr>    check the reference count of a ucred structure \\\n\
     -checkall    check the reference count of ALL ucred structures \\\n\
";

FieldRec proc_fields[] = {
  { ".p_nxt", NUMBER, NULL, NULL },
  { ".p_rcred", NUMBER, NULL, NULL },
  { ".p_pid", NUMBER, NULL, NULL },
  { ".thread->u_address.uthread", NUMBER, NULL, NULL }, /* a ptr to uthread */
  { ".thread->u_address.uthread->uu_nd.ni_cred", NUMBER, NULL, NULL },

};

FieldRec thread_fields[] = {
  { ".u_address->uthread", NUMBER, NULL, NULL },  /* a ptr to uthread */
};

FieldRec uthread_fields[] = {
  { ".uu_nd.ni_cred", NUMBER, NULL, NULL }, /* a ptr to ucred */
};

FieldRec file_fields[] = {
  { ".f_cred", NUMBER, NULL, NULL },
  { ".f_count", NUMBER, NULL, NULL },
};

FieldRec ucred_fields[] = {
  { ".cr_ref", NUMBER, NULL, NULL },
  { ".cr_uid", NUMBER, NULL, NULL },
  { ".cr_gid", NUMBER, NULL, NULL },
  { ".cr_ruid", NUMBER, NULL, NULL },
};

FieldRec ucred2_fields[] = {
  { ".cr_ref", NUMBER, NULL, NULL },
  { ".cr_uid", NUMBER, NULL, NULL },
  { ".cr_gid", NUMBER, NULL, NULL },
  { ".cr_ruid", NUMBER, NULL, NULL },
};

FieldRec buf_fields[] = {
  { ".av_forw", NUMBER, NULL, NULL }, 
  { ".b_rcred", NUMBER, NULL, NULL },
  { ".b_wcred", NUMBER, NULL, NULL },
  { ".b_forw", NUMBER, NULL, NULL },
  { ".b_flags", NUMBER, NULL, NULL },
};

#define NUM_PROC_FIELDS (sizeof(proc_fields)/sizeof(proc_fields[0]))
#define NUM_UTHREAD_FIELDS (sizeof(uthread_fields)/sizeof(uthread_fields[0]))
#define NUM_FILE_FIELDS (sizeof(file_fields)/sizeof(file_fields[0]))
#define NUM_UCRED_FIELDS (sizeof(ucred_fields)/sizeof(ucred_fields[0]))
#define NUM_UCRED2_FIELDS (sizeof(ucred2_fields)/sizeof(ucred2_fields[0]))
#define NUM_BUF_FIELDS (sizeof(buf_fields)/sizeof(buf_fields[0]))

#define NOCRED (-1)
#define MAXUCRED 500
#define NPROC    500
#define NFILE    2048
#define NUTHREAD 500
#define NBUF     500
/* column #0: source addr, column #1: ucred addr, column #2: ref_count */
static long p_crtab[NPROC][3], f_crtab[NFILE][3], u_crtab[NUTHREAD][3], b_crtab[NBUF][3];
/* column #0: ucred addr, column #1: ref_count, column #2: found count */
static long crtab[MAXUCRED][3];

main(int argc, char **argv)
{
  DataStruct proc, uthread, file, ucred, ele, buf, ucred2 ;
  long procaddr, fileaddr, ucredaddr, nfile, bufaddr, bufaddr1, addrarg=0;
  char *error, bufout[256];
  int procflag=0, fileflag=0, bufflag=0, uthreadflag=0, allflag=0, refflag=0;
  int checkflag=0, check1flag=0, checkallflag=0;
  int p_count=0, f_count=0, u_count=0, b_count=0;
  int found = 0, cr_ref = 0, allcnt=0; 
  int i, j;

  /*** parse command line arguments ***/

  check_args(argc, argv, help_string);
  for (i = 1; i < argc; i++)
      if (!strcmp(argv[i], "-proc"))  procflag = 1;
      else if (!strcmp(argv[i], "-uthread")) uthreadflag = 1;
      else if (!strcmp(argv[i], "-file"))  fileflag = 1;
      else if (!strcmp(argv[i], "-buf"))  bufflag = 1;
      else if (!strcmp(argv[i], "-ref"))  {refflag=1; addrarg=strtoul(argv[++i],(char**)NULL,16);}
      else if (!strcmp(argv[i], "-check"))  {checkflag=1; check1flag=1; 
                                            addrarg=strtoul(argv[++i],(char**)NULL,16);}
      else if (!strcmp(argv[i], "-checkall"))  {checkflag=1; checkallflag = 1;}
      else {
        fprintf(stderr, "%s: invalid option, `%s'\n", argv[0], argv[i]);
        quit(1);
      }

   if (argc == 1) allflag=1;

   /*** print out heading ***/
   if(!check_fields("struct ucred", ucred_fields, NUM_UCRED_FIELDS, NULL)){
        field_errors(ucred_fields, NUM_UCRED_FIELDS);
        quit(1);
   }
   if (procflag||uthreadflag||fileflag||bufflag||allflag||refflag) {
      sprintf(bufout,"    ADDR OF UCRED      ADDR OF Ref      Ref Type  cr_ref  cr_uid  cr_gid  cr_ruid ");
      print(bufout);
      sprintf(bufout,"===================  ================== ========  ======  ======= ======  =======");
      print(bufout);
   }

  if (procflag || allflag || refflag || checkflag) {  /* procflag */
  
      /*
       * Get all the cred referenced in process table.
       */

      /*for (i=0; i<NPROC; i++) p_crtab[i][0]=p_crtab[i][1]=p_crtab[i][2]=0;*/
      if(!check_fields("struct proc", proc_fields, NUM_PROC_FIELDS, NULL)){
        field_errors(proc_fields, NUM_PROC_FIELDS);
        quit(1);
      }
      if(!read_sym_val("allproc", NUMBER, &procaddr, &error)){
        fprintf(stderr, "Couldn't read allproc:\n");
        fprintf(stderr, "%s\n", error);
        quit(1);
      }

      do {
        if(!cast(procaddr, "struct proc", &proc, &error)){
          fprintf(stderr, "Couldn't cast addr to a proc:\n");
          fprintf(stderr, "%s\n", error);
          quit(1);
        }
        if(!read_field_vals(proc, proc_fields, NUM_PROC_FIELDS)){
          field_errors(proc_fields, NUM_PROC_FIELDS);
          quit(1);
        }
        if(!cast((long)proc_fields[1].data, "struct ucred", &ucred, &error)){
          fprintf(stderr, "Couldn't cast addr to a ucred:\n");
          fprintf(stderr, "%s\n", error);
          quit(1);
        }
        if(!read_field_vals(ucred, ucred_fields, NUM_UCRED_FIELDS)){
          field_errors(ucred_fields, NUM_UCRED_FIELDS);
          quit(1);
        }
      
        p_crtab[p_count][0] = (long)procaddr;
        p_crtab[p_count][1] = (long)proc_fields[1].data;
        p_crtab[p_count][2] = (long)ucred_fields[0].data;
        p_count++;

        if (procflag||allflag||(refflag&&(addrarg==(long)proc_fields[1].data))) {
        sprintf(bufout,"0x%16lx   0x%16lx %8s %6d   %6d %6d  %6d ", proc_fields[1].data, procaddr,
          " proc ", ucred_fields[0].data, ucred_fields[1].data, ucred_fields[2].data,
           ucred_fields[3].data);
        print(bufout);
        }

        procaddr = (long) proc_fields[0].data;
      } while(procaddr != 0);

  } /* end of if (procflag) */

      /*
       * Get all the cred references by uthread table.
       */

  if (uthreadflag || allflag || refflag || checkflag) {  /* uthreadflag */
  
      /* for some reason it hangs if this is executed twice */
      if (uthreadflag && !allflag && !refflag)
      if(!check_fields("struct proc", proc_fields, NUM_PROC_FIELDS, NULL)){
        field_errors(proc_fields, NUM_PROC_FIELDS);
        quit(1);
      }

      if(!read_sym_val("allproc", NUMBER, &procaddr, &error)){
        fprintf(stderr, "Couldn't read allproc:\n");
        fprintf(stderr, "%s\n", error);
        quit(1);
      }

      do {
        if(!cast(procaddr, "struct proc", &proc, &error)){
          fprintf(stderr, "Couldn't cast addr to a proc:\n");
          fprintf(stderr, "%s\n", error);
          quit(1);
        }
        if(!read_field_vals(proc, proc_fields, NUM_PROC_FIELDS)){
          field_errors(proc_fields, NUM_PROC_FIELDS);
          quit(1);
        }

        if(!cast((long)proc_fields[4].data, "struct ucred", &ucred, &error)){
          fprintf(stderr, "Couldn't cast addr to a ucred:\n");
          fprintf(stderr, "%s\n", error);
          quit(1);
        }
        if(!read_field_vals(ucred, ucred_fields, NUM_UCRED_FIELDS)){
          field_errors(ucred_fields, NUM_UCRED_FIELDS);
          quit(1);
        }
      
        u_crtab[u_count][0] = (long)proc_fields[3].data;
        u_crtab[u_count][1] = (long)proc_fields[4].data;
        u_crtab[u_count][2] = (long)ucred_fields[0].data;
        u_count++;

        if (uthreadflag||allflag||(refflag&&(addrarg==(long)proc_fields[4].data))) {
        sprintf(bufout,"0x%16lx   0x%16lx %8s %6d   %6d %6d  %6d ", proc_fields[4].data, 
          proc_fields[3].data, "uthread", ucred_fields[0].data, 
          ucred_fields[1].data, ucred_fields[2].data, ucred_fields[3].data);
        print(bufout);
        }
        procaddr = (long) proc_fields[0].data;
      } while(procaddr != 0);

  } /* end of if (uthreadflag) */

      /*
       * Get all the cred references in file table.
       */

  if (fileflag || allflag || refflag || checkflag) {  /* fileflag */

      if(!check_fields("struct file", file_fields, NUM_FILE_FIELDS, NULL)){
        field_errors(file_fields, NUM_FILE_FIELDS);
        quit(1);
      }

      file = read_sym("file");
      if(!read_sym_val("nfile", NUMBER, &nfile, &error)){
        fprintf(stderr, "Couldn't read nfile:\n");
        fprintf(stderr, "%s\n", error);
        quit(1);
      }


      for(i=0; i<nfile; i++){

        if((ele = array_element(file, i, &error)) == NULL){
          fprintf(stderr, "Couldn't get array element\n");
          fprintf(stderr, "%s\n", error);
          quit(1);
        }
        if(!read_field_vals(ele, file_fields, NUM_FILE_FIELDS)){
          field_errors(file_fields, NUM_FILE_FIELDS);
          quit(1);
        }
      if ((long)file_fields[1].data != 0)
      { /* skip if the file node is not active */
        if(!cast((long)file_fields[0].data, "struct ucred", &ucred, &error)){
          fprintf(stderr, "Couldn't cast addr to a ucred:\n");
          fprintf(stderr, "%s\n", error);
          quit(1);
        }
        if(!read_field_vals(ucred, ucred_fields, NUM_UCRED_FIELDS)){
          field_errors(ucred_fields, NUM_UCRED_FIELDS);
          quit(1);
        }

        fileaddr = (long)struct_addr(ele);
      
        f_crtab[f_count][0] = (long)fileaddr;
        f_crtab[f_count][1] = (long)file_fields[0].data;
        f_crtab[f_count][2] = (long)ucred_fields[0].data;
        f_count++;

        if (fileflag||allflag||(refflag&&(addrarg==(long)file_fields[0].data))) {
        sprintf(bufout,"0x%16lx   0x%16lx %8s %6d   %6d %6d  %6d", file_fields[0].data, fileaddr,
          "file", ucred_fields[0].data, ucred_fields[1].data, ucred_fields[2].data,
           ucred_fields[3].data);
        print(bufout);
        }
      }

      };  /* end of for loop */ 

  } /* end of if (fileflag) */

      /*
       * Get all the cred references in buf table.
       */

  if (bufflag || allflag || refflag || checkflag) {

    if(!check_fields("struct buf", buf_fields, NUM_BUF_FIELDS, NULL)){
      field_errors(buf_fields, NUM_BUF_FIELDS);
      quit(1);
    }
    if(!read_sym_val("buf", NUMBER, &bufaddr, &error)){
      fprintf(stderr, "Couldn't read buf:\n");
      fprintf(stderr, "%s\n", error);
      quit(1);
    }

    bufaddr1 = bufaddr;

    do 
    {
      if(!cast(bufaddr, "struct buf", &buf, &error)){
        fprintf(stderr, "Couldn't cast nextaddr to a buf:\n");
        fprintf(stderr, "%s\n", error);
        quit(1);
      }
      if(!read_field_vals(buf, buf_fields, NUM_BUF_FIELDS)){
        field_errors(buf_fields, NUM_BUF_FIELDS);
        quit(1);
      }
      /* read into b_rcred */
      if (((long)buf_fields[1].data != NOCRED) && ((long)buf_fields[1].data != 0)) {
        if(!cast((long)buf_fields[1].data, "struct ucred", &ucred, &error)){
          fprintf(stderr, "Couldn't cast addr to a ucred:\n");
          fprintf(stderr, "%s\n", error);
          quit(1);
        }
        if(!read_field_vals(ucred, ucred_fields, NUM_UCRED_FIELDS)){
          field_errors(ucred_fields, NUM_UCRED_FIELDS);
          quit(1);
        }
      
        b_crtab[f_count][0] = (long)bufaddr;
        b_crtab[f_count][1] = (long)buf_fields[1].data;
        b_crtab[f_count][2] = (long)ucred_fields[0].data;
        b_count++;

        if (bufflag||allflag||(refflag&&(addrarg==(long)buf_fields[1].data))) {
        sprintf(bufout,"0x%16lx 0x%16lx %8s %6d %6d %6d %6d ", buf_fields[1].data, bufaddr," buf",
          ucred_fields[0].data, ucred_fields[1].data, ucred_fields[2].data, ucred_fields[3].data );
        print(bufout);
        }
      }
      /* read into b_wcred */
      if (((long)buf_fields[2].data != NOCRED) && ((long)buf_fields[2].data != 0)) {
        if(!cast((long)buf_fields[2].data, "struct ucred", &ucred2, &error)){
          fprintf(stderr, "Couldn't cast addr to a ucred2:\n");
          fprintf(stderr, "%s\n", error);
          quit(1);
        }
        if(!read_field_vals(ucred2, ucred2_fields, NUM_UCRED2_FIELDS)){
          field_errors(ucred2_fields, NUM_UCRED2_FIELDS);
          quit(1);
        }
      
        b_crtab[f_count][0] = (long)bufaddr;
        b_crtab[f_count][1] = (long)buf_fields[2].data;
        b_crtab[f_count][2] = (long)ucred_fields[0].data;
        b_count++;

        if (bufflag||allflag||(refflag&&(addrarg==(long)buf_fields[2].data))) {
        sprintf(bufout,"0x%16lx 0x%16lx %8s %6d %6d %6d %6d ", buf_fields[2].data, bufaddr, " buf",
          ucred2_fields[0].data, ucred2_fields[1].data, ucred2_fields[2].data, ucred2_fields[3].data );
        print(bufout);
        }
      }

      bufaddr = (long) buf_fields[0].data;
    } while(bufaddr1 != bufaddr);

  } /* enf of if (bufflag) */

      /*
       * check the reference count 
       */

  if (check1flag ) {

    print("   ADDR OF UCRED      cr_ref   Found");
    print("====================  ======  =======");
    for (i=0; i < p_count; i++)
      if (p_crtab[i][1] == addrarg) {
	found++;
	cr_ref = p_crtab[i][2];
      }

    for (i=0; i < u_count; i++)
      if (u_crtab[i][1] == addrarg) {
	found++;
	cr_ref = u_crtab[i][2];
      }

    for (i=0; i < f_count; i++)
      if (f_crtab[i][1] == addrarg) {
	found++;
	cr_ref = f_crtab[i][2];
      }
    sprintf(bufout, "0x%16lx  %6d %6d", addrarg, cr_ref, found);
    print(bufout);
  }  /* end of if (check1flag) */

  if (checkallflag ) {

    print("   ADDR OF UCRED      cr_ref   Found");
    print("====================  ======  =======");
    for (j=0; j < MAXUCRED; j++) crtab[j][0]=crtab[j][1]=crtab[j][2]=0;
    for (i=0; i < p_count; i++) {
      for (j=0; j < allcnt; j++)
      if (p_crtab[i][1] == crtab[j][0]) {
        crtab[j][2] = crtab[j][2] + 1;	/* found++ */
        goto cont1; 
      }
      /* add to the crtab if not there yet */
      crtab[allcnt][0] = p_crtab[i][1];
      crtab[allcnt][1] = p_crtab[i][2];
      crtab[allcnt][2] = 1;
      allcnt++;
    cont1: ;
    }

    for (i=0; i < u_count; i++) {
      for (j=0; j < allcnt; j++)
      if (u_crtab[i][1] == crtab[j][0]) {
        crtab[j][2] = crtab[j][2] + 1;	/* found++ */
        goto cont2; 
      }
      /* add to the crtab if not there yet */
      crtab[allcnt][0] = u_crtab[i][1];
      crtab[allcnt][1] = u_crtab[i][2];
      crtab[allcnt][2] = 1;
      allcnt++;
    cont2: ;
    }

    for (i=0; i < f_count; i++) {
      for (j=0; j < allcnt; j++)
      if (f_crtab[i][1] == crtab[j][0]) {
        crtab[j][2] = crtab[j][2] + 1;	/* found++ */
        goto cont3;
      }
      /* add to the crtab if not there yet */
      crtab[allcnt][0] = f_crtab[i][1];
      crtab[allcnt][1] = f_crtab[i][2];
      crtab[allcnt][2] = 1;
      allcnt++;
    cont3: ;
    }

    for (i=0; i < allcnt; i++) {
      if (crtab[i][1] == crtab[i][2])
        sprintf(bufout, "0x%16lx  %6d %6d", crtab[i][0], crtab[i][1], crtab[i][2]);
      else 
        sprintf(bufout, "0x%16lx  %6d %6d   *", crtab[i][0], crtab[i][1], crtab[i][2]);
      print(bufout);
    }
  }  /* end of if (checkallflag) */
}
