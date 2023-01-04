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
static char *rcsid = "@(#)$RCSfile: pcb.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/24 15:30:11 $";
#endif

#include <stdio.h>
#include "krash.h"

static char *help_string =
"pcb - print the pcb for a given thread                              \\\n\
    Usage : pcb thread_address                                           \\\n\
";

FieldRec fields[] = {
  { ".pcb", NUMBER, NULL, NULL },
  { ".pcb->pcb_ksp", NUMBER, NULL, NULL },
  { ".pcb->pcb_usp", NUMBER, NULL, NULL },
  { ".pcb->pcb_ptbr", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[0]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[1]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[2]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[3]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[4]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[5]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[6]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[7]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[8]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[9]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[10]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[11]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[12]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[13]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[14]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[15]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[16]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[17]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[18]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[19]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[20]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[21]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[22]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[23]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[24]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[25]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[26]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[27]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[28]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[29]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[30]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[31]", NUMBER, NULL, NULL },
  { ".pcb->pcb_regs[32]", NUMBER, NULL, NULL },
  { ".pcb->pcb_paddr", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[0]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[1]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[2]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[3]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[4]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[5]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[6]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[7]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[8]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[9]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[10]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[11]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[12]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[13]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[14]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[15]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[16]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[17]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[18]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[19]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[20]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[21]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[22]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[23]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[24]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[25]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[26]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[27]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[28]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[29]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[30]", NUMBER, NULL, NULL },
  { ".pcb->pcb_fpregs[31]", NUMBER, NULL, NULL }
};

#define NUM_FIELDS (sizeof(fields)/sizeof(fields[0]))

Usage() {
  fprintf(stderr, "Usage : pcb thread_address\n");
  quit(1);
}

main(int argc, char **argv)
{
  DataStruct thread;
  char buf[256], *error, ksp[12], addr[12];
  long threadaddr;
  unsigned long reg[32], fpreg[32];	
  int i, j, k, num, fp;

  if(argc !=2) Usage();
  check_args(argc, argv, help_string);
  if(!check_fields("struct thread", fields, NUM_FIELDS, NULL)){
    field_errors(fields, NUM_FIELDS);
    quit(1);
  }  
    threadaddr = strtoul(argv[1], (char **)NULL, 16);
  sprintf(buf, "Addr pcb        ksp             usp             pc                     ps");
  print(buf);
    if(!cast(threadaddr, "struct thread", &thread, &error)){
      fprintf(stderr, "Couldn't cast to a thread:\n");
      fprintf(stderr, "%s:\n", error);
    }
    if(!read_field_vals(thread, fields, NUM_FIELDS)){
      field_errors(fields, NUM_FIELDS);
      return(False);
    }
    sprintf(buf, "%-15s %-15s 0x%-13lx 0x%-20lx 0x%x", 
           format_addr((unsigned long) fields[0].data, addr),
           format_addr((unsigned long) fields[1].data, ksp),
           fields[2].data,
           fields[32].data,
           fields[31].data);
    print(buf);
    
    sprintf(buf, "sp                     ptbr       pcb_physaddr");
    print(buf);

    sprintf(buf, "0x%-20lx 0x%-8x 0x%-x",
           fields[30].data,
           fields[3].data,
           fields[37].data);
    print(buf);
    for(i=0; i<16; i++){
       reg[i] = (unsigned long) fields[4+i].data;
    }
    for(j=0; j<3; j++){
       reg[16+j] = (unsigned long) fields[34+j].data;
    }
    for(k=0; k<10; k++){
       reg[19+k] = (unsigned long) fields[20+k].data;
    }
    reg[29] = (unsigned long) fields[33].data;
    print(" "); 

    for(num=0; num<30; num++){
      if(reg[num] != 0){
        sprintf(buf, "r" "%-4d" "0x%lx",  num, reg[num]);
        print(buf);
      }
    }
    print(" "); 
    
    for(fp=0; fp<32; fp++){
      if((unsigned long) fields[38+fp].data != 0){
        sprintf(buf, "f" "%-4d" "0x%lx", fp, (unsigned long) fields[38+fp].data);
        print(buf);
      }
    }
}
