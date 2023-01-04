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
static char *rcsid = "@(#)$RCSfile: input.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/19 09:38:22 $";
#endif
#include "krash.h"

main(){
  Status status;
  char *resp;

  while(1){
    print("Give me some input");
    if((resp = read_response(&status)) == NULL){
      if((status.type == Local) && (status.u.comm == READ_EOF)) break;
      print("read_response failed");
      quit(1);
    }
    if(resp[strlen(resp)-1] == '\n') resp[strlen(resp)-1] = '\0';
    dbx(resp, False);
    free(resp);
  }
}
