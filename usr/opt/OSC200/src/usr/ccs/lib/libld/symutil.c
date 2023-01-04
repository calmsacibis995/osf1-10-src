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
static char	*sccsid = "@(#)$RCSfile: symutil.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:05:42 $";
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
 * Exported routines:
 *
 *
 * export_foreach (char *filename, void (*func)(char *))
 *
 * This routine takes the filename of a file containing either a Mach-O
 * (OSF/Rose) object, or a library (archive) of Mach-O objects. It then
 * calls the function specified by the second argument once for each 
 * external symbol in the object, passing that function the symbol name
 * as an argument.
 *
 * Someday, this routine should be made more general, by allowing 
 * more flexibility in specifying imported symbols and stab symbols as
 * well. Furthermore, the routine should probably be passed a pointer
 * to a symbol_info_t, rather than a pointer to the the name.
 *
*/

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <ar.h>
#include <ldfcn.h>
#include <string.h>
#include <stdlib.h>
#include <mach_o_format.h>
#include <mach_o_header.h>

typedef void (*functype)(char *); /* function passed in to iterator */

static int read_symbols_and_strings(FILE *file, int  *symcount,
				    symbol_info_t **symbols, char **strings);

static int export_foreach_file(FILE  *file, functype func);

int
export_foreach (char *filename, functype func)
{
  FILE *file;
  int  retval, count;
  struct arbuf_t arbuf;
  struct ar_hdr  ariobuf;
  
  file = fopen (filename, "r");
  if (file == NULL) return (-1);

  if (ARisarchive(file))
    {
      /* For each member of the archive */
      if (fseek(file, SARMAG, SEEK_SET) == -1)  /* skip archive header */
	return (-1);
      while (!feof(file))
	{
	  off_t	pos;
	  char	*p;
	  /* Read in the member header */
	  
	  count = fread(&ariobuf, sizeof(ariobuf), (size_t) 1, file);
	  
	  if (count == 0)
	    {
	      fclose(file);
	      return (0);
	    }
	  else if (count != 1)
	    return(-1);

	  (void)strncpy(arbuf.ar_name, ariobuf.ar_name, sizeof(arbuf.ar_name));
	  p = strchr(arbuf.ar_name, '/');
	  if(p)
	    *p = '\0';			/* Mark end of member */
	  else if (p = strchr(arbuf.ar_name, ' '))
	    *p = '\0';

	  arbuf.ar_date = atol(ariobuf.ar_date);
	  arbuf.ar_uid  = atol(ariobuf.ar_uid);
	  arbuf.ar_gid  = atol(ariobuf.ar_gid);
	  arbuf.ar_mode = strtol(ariobuf.ar_mode,(char **)NULL,8);
	  arbuf.ar_size = atol(ariobuf.ar_size);

	  /* Member size is rounded up to word */ 
	  arbuf.ar_size++; arbuf.ar_size &= ~1;
	  
	  /* One more sanity-check on the archive header! */
	  if (strncmp(ariobuf.ar_fmag, ARFMAG, sizeof(ariobuf.ar_fmag)))
	    return (-1);
	  
	  pos = ftell(file) + arbuf.ar_size;

	  /*
	   * Check for Archive index if present and jump around it!
	   */
	  if (!strncmp( arbuf.ar_name,  "__.SYMDEF", strlen("__.SYMDEF")))
	    goto next;
      
	  /* Call specified routine and check for errors */
	  
	  if (retval = export_foreach_file (file, func))
	    return(retval);

	next:	
	  /* Go to next member in archive */
	  if (fseek(file, pos, SEEK_SET) != 0)
	    return(-1);
	} /* while */
      fclose(file);
      return(0);
    }
  else
    {
      /*
       * Simple case - not an archive
       */
      if (fseek(file, 0, SEEK_SET) != 0)
	return (-1);
      retval = export_foreach_file(file, func);
      fclose(file);
      return (retval);
    }
}

int
export_foreach_file(FILE *file, functype func)
{
  symbol_info_t *symbols, *sp;
  int index, symcount = 0;
  char *strings;
  off_t pos;

  pos = ftell(file); /* remember position in file */

  if (read_symbols_and_strings(file, &symcount, &symbols, &strings) != 0)
    return (-1);
  for (sp = symbols, index = 0;
       index < symcount;
       sp++, index++)
    if (sp->si_flags & SI_EXPORT_F)
      func(sp->si_name.symbol_name + strings); /* call function */
  free(strings); /* Done with strings */
  free(symbols); /* Ditto for symbols */

  fseek(file, pos, SEEK_SET); /* restore file pointer */
  return (0);
}


int
read_symbols_and_strings(FILE *file,                /* file */
			 int  *symcount,            /* OUT - # of symbols */
			 symbol_info_t **symbols,   /* OUT - symbol array */
			 char **strings)            /* OUT - strings */
{
  mo_header_t   mo_header;
  ldc_header_t *load_commands, *h;
  ldc_header_t **ldc_map;
  symbols_command_t *s;
  char *cp;
  int i, nread, ret;
  int stringsize = 0;         /* total size (in bytes) of string table(s) */
  int symsize    = 0;         /* total size (in bytes) of symbol section(s) */
  char *endstrings;           /* pointer to end of strings */
  symbol_info_t *endsymbols;  /* ditto for symbols */
  char *comical_header;
  off_t object_start;

  object_start = ftell(file);        /* get start of "real" object */

  comical_header = malloc(MO_SIZEOF_RAW_HDR);
  
  nread = fread (comical_header, 1, MO_SIZEOF_RAW_HDR, file);

  if (nread != MO_SIZEOF_RAW_HDR) return (-1);


  /* make the canonical header readable */
  ret = decode_mach_o_hdr (comical_header,
			   MO_SIZEOF_RAW_HDR ,
			   MOH_HEADER_VERSION, 
			   &mo_header);
  if (ret != 0) return (-1);

  if (fseek(file,
	    mo_header.moh_first_cmd_off + object_start,
	    SEEK_SET) == -1)
    return (-1);

  load_commands = (ldc_header_t *) malloc(mo_header.moh_sizeofcmds);

  if (fread((char *)load_commands, 1, mo_header.moh_sizeofcmds, file)!= 
      mo_header.moh_sizeofcmds) return (-1);

  ldc_map = (ldc_header_t **)malloc(mo_header.moh_n_load_cmds*sizeof(*ldc_map));
  cp = (char *)load_commands;
  for (i = 0; i < mo_header.moh_n_load_cmds; i++) {
    h = (ldc_header_t *)cp;
    ldc_map[i] = h;
    cp += h->ldci_cmd_size;
  }

  /*
   * Loop through all the load commands. We only care about
   * two load commands - the strings and the symbols. 
   * Loop twice, figuring out the size of the  symbols
   * and strings during the first loop, and reading them
   * on the second loop. This scheme allows multiple string
   * or symbol sections, which
   * may someday be present in the file.
   */

  for (i = 0; i < mo_header.moh_n_load_cmds; i++) {
    h = ldc_map[i];
    switch (h->ldci_cmd_type) {
    case LDC_STRINGS:
      stringsize += h->ldci_section_len;
      break;
    case LDC_SYMBOLS:
      s = (symbols_command_t *) h;
      switch (s->symc_kind) {
      case SYMC_DEFINED_SYMBOLS:
	symsize  += h->ldci_section_len;
	*symcount += s->symc_nentries;
 	break;
      }
    }
  }

  /*
   * Allocate room for the symbols and strings, and read them 
   * during a second pass. This allows multiple string and symbol 
   * sections to coexist.
   */

  *symbols = endsymbols = (symbol_info_t *) malloc(symsize);
  *strings = endstrings = malloc(stringsize);

  for (i = 0; i < mo_header.moh_n_load_cmds; i++) {
    h = ldc_map[i];
    switch (h->ldci_cmd_type) {
    case LDC_STRINGS:
      if (fseek(file, h->ldci_section_off + object_start, SEEK_SET) != 0)
	return (-1);
      fread(endstrings, 1, h->ldci_section_len, file);
      endstrings += h->ldci_section_len;
      break;
    case LDC_SYMBOLS:
      s = (symbols_command_t *) h;
      switch (s->symc_kind) {
      case SYMC_DEFINED_SYMBOLS:
	if (fseek(file, h->ldci_section_off + object_start, SEEK_SET) != 0)
	  return (-1);
	fread(endsymbols, 1, h->ldci_section_len, file);
	endsymbols += s->symc_nentries;
	break;
      }
    }
  }
  return (0);
}


