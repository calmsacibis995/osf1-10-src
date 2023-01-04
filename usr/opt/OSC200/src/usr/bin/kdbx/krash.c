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
static char *rcsid = "@(#)$RCSfile: krash.c,v $ $Revision: 1.1.9.9 $ (DEC) $Date: 1994/01/24 22:17:08 $";
#endif
#ifdef __alpha
#define KERNEL
#include <arch/alpha/pmap.h>
#endif
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#include "krash.h"
#include "krash_p.h"
#undef KERNEL
#include <ldfcn.h>

/* *** BUGS -
   help - get rid of not registered messages
	- with /bin in KRASH_EXTPATH, lots of complain messages
   tests -
     bus_action kills dbx
   crash after extension crash
   > alias x "print 1;\\\
   print 2"
   > alias x
   x       print 1;\print 2
  socket | sh grep DGRAM  - hangs, but produces input when ^C'd
  sh cat /etc/disktab | sh grep :
  piping fileaction into awk hangs when awk exits
  nameistats - Unmatched ".
  ^C in middle of command line should clear command and reprompt
  > alias time 'print `ctime \`p time.tv_sec\``'
  > alias time
  time    print `ctime `p time.tv_sec``
  continued krash doesn't reprompt
  print "`sh cat /etc/disktab`" - loses newlines
  way to turn off paging
  p *allproc | grep utask - doesn't error - contention over dbx input
  p *allproc | sh grep utask - ^C, dbx output hasn't been eaten - nr
    No ^C, no prompt
  backgrounded krash exits
  expand_aliases handling of strings is broken.  strings aren't stored as
  strings, and the logic to handle them is wrong.
  source -x a file with help in it causes doubled lines
  *** FIXES
*/

/*
 *  Handy symbols: utsname cpup numcpus mastercpu paniccpu time
 *  All this:
#               repeat last command
#5              repeat command 5 (any number)
#h              show history list
!               escape to shell
?               print this list of available commands
b               dump buffer contents
blocks          disk blocks of an inode
buffer          buffer data
bufbusy         busy buffers
bufdirty                dirty buffers
bufclean                clean cached buffers
bufempty                empty buffers
bufhash         buffer cache hash lists
bufstats                buffer cache statistics
cache           buffers for a gnode
client          dump rpc client table  *** No symbol "chtable"
cmap            cmap entry dump - enter pfn *** dbx error
cmap -i         cmap entry dump - enter cmap index**can't find pte (0x1001caf0)
cmap -a         cmap entry dump - enter cmap address
cmap -h         cmap hash list dump - enter cmap index
cmap -b         cmap hash list dump - enter blkno
cpu             cpudata table *** No symbol "cpudata"
crcheck         check credential ref counts
credref         list references to a credential
dnlc            display the dnlc cache contents
ds              data address namelist search
dupreq          display the rpc duplicate request list
duphash         display the NFS dupreq hash list
export          display the current nfs export list
fsdata          filesystem data
g -             all gnodes
g -             list of gnode slots (all): g - 1 4 8
g -amod         gnodes with exact mode: g -amod 20622
g -fs           gnodes with file system:g -fs 2
g -gid          gnodes with gid: g -gid 10
g -gno          gnodes with g_number: g -gno 2
g -hmod         gnodes with type mode: g -hmod 7000
g -lmod         gnodes with permission mode: g -lmod 111
g -maj          gnodes with major device: g -maj 9
g -min          gnodes with minor device: g -min a
g -uid          gnodes with uid: g -uid 412
g               list of gnode slots (active only): g 1 5 7
gfree           free gnode slots
glock           locked gnode slots
gref            find refs on a gnode slot
gnode           active gnode table
gstats          gnode cache statistics
h               help on commands starting with letter: h g
h               help
history         get history
help            help
kmalloc         display kmalloc pool stats "bucket" doesn't exist
lock            display a smp lock trace
map             resource maps - type map for list *** No map names
mbuf            display mbufs on a socket
mntinfo         remote mount information
mscp            mscp disk & tape subsystem traversal
mscp -disk              mscp disk subsystem traversal
mscp -tape              mscp tape subsystem traversal - tmscp
mscp -config            mscp disk & tape subsystem configuration
mscp -connb             display mscp connection block
mscp -classb            display mscp class block
mscp -unitb             display mscp unit block
mscp -reqb              display mscp request block
mscp -dtable            display mscp disk unit table
mscp -ttable            display tmscp tape unit table
mscp -devunit           display unit block for major/minor
nm              name search
namei           display namei cache
ofile           u.u_ofile dump
od              dump symbol values
pcb             process control block
ports           display port control blocks
ports -ssp              display SSP port control blocks
ports -msi              display MSI port control blocks
ports -ci               display CI port control blocks
ports -gvp              display GVP port control blocks
ports -brief            display port blocks, in brief
ppte            ptes associated with proc slot
presto          presto status
proc -          process table long listing
proc -r         runnable processes only
proc            process table
proclock                show sleeplocks held by sleeping procs
ps              proc table summary with command lines
quit            exit
rnode           remote gnode fields
s               dump kernel stack for process
scs             System Communications Services (SCS)
scs -cb         SCS connection block structure
scs -cib                SCS connection info block structure
scs -pb         SCS path block structure
scs -pib                SCS path information block structure
scs -sb         SCS system block structure
scs -sib                SCS system information block structure
scsi            SCSI controller information
scsi -target           SCSI target information
scsi -devtab           SCSI devtab information
scsi -trans            SCSI transfer information
scsi -cmd              SCSI message/command data
scsi -bbr              SCSI Bad Block Replacement data
scsi -error            SCSI error information
scsi -sii              SCSI SII information
scsi -dct              SCSI DCT stats
scsi -spin             SCSI SPIN stats
scsi -all              all SCSIBUS informaton
socket          sockets from file table
sync            resync internal tables with /dev/mem
svcxprt         print rpc server xprt structure
svcreq          kernel rpc request
text            text table   *** No symbol "text"
trace -         kernel trace with variables
trace           kernel trace of process
ts              text address namelist search
tty -           print tty structure with clists
tty             print tty structure
u               user area
ufile           user open file table
user            user area
udpdata         kernel rpc udp input descriptor

 */

static Boolean do_alias(TokenArray *args, Context *context, IORec *io);
static Boolean do_context(TokenArray *args, Context *context, IORec *io);
static Boolean do_core(TokenArray *args, Context *context, IORec *io);
static Boolean do_coredata(TokenArray *args, Context *context, IORec *io);
static Boolean do_data_symbol(TokenArray *args, Context *context, IORec *io);
static Boolean do_dbx(TokenArray *args, Context *context, IORec *io);
static Boolean do_proc(TokenArray *args, Context *context, IORec *io);
static Boolean do_help(TokenArray *args, Context *context, IORec *io);
static Boolean do_print(TokenArray *args, Context *context, IORec *io);
static Boolean do_quit(TokenArray *args, Context *context, IORec *io);
static Boolean do_source(TokenArray *args, Context *context, IORec *io);
static Boolean do_suspend(TokenArray *args, Context *context, IORec *io);
static Boolean do_unalias(TokenArray *args, Context *context, IORec *io);

ComTable commands[] = {
  { "alias", do_alias, False, False,
"alias - Set or print aliases.                                              \n\
    alias - Print all aliases.                                              \n\
    alias x - Print alias for \"x\" if one exists.                          \n\
    alias x y - Alias \"x\" to \"y\".                                       \n\
    alias -val x - Print only the value of the alias of \"x\"." },
  { "context", do_context, False, False,
"context proc|user - Set context to user's aliases or extension's aliases.  \n\
    Used only by extensions." },
  { "core", do_core, False, False,
"core file - Switch to a different core file (not yet implemented)." },
  { "coredata", do_coredata, False, False,
"coredata start_address end_address - Dump in hex the contents of the core  \n\
    file starting at start_address and ending before end_address." },
  { "data_symbol", do_data_symbol, False, False,
"data_symbol [ address ... ]- Return the symbols corresponding to the       \n\
    addresses." },
  { "dbx", do_dbx, False, False,
"dbx command-string - Pass command-string to dbx." },
#define DBX_INDEX 5
  { "help", do_help, False, False,
"help [-long] [arg] - If arg is present, print help on commands, aliases and\n\
    extensions that begin with arg; the extension help will be the complete \n\
    help for each matching extension.  Otherwise print help on everything,  \n\
    with a one line summary of each extension.  The -long switch forces the \n\
    long version of extension help. " },
  { "proc", do_proc, True, True,
"proc [switches] args - Execute an extension and give it control of the     \n\
    kdbx session until it quits.                                           \n\
	proc [switches...] executable [args...] - Execute the named         \n\
            file and pass it args.                                          \n\
	proc [switches...] -pipe in_pipe out_pipe - Create                  \n\
            in_pipe and out_pipe as named pipes and read input from in_pipe \n\
             and write output to out_pipe.                                  \n\
    The -debug switch causes I/O to and from the extension to be printed    \n\
	on the screen.                                                      \n\
    The -redirect_output switch is used by extensions that execute other    \n\
        extensions if they want themselves, and not the user, to receive    \n\
        the output of those extensions.                                     \n\
    The -print_output switch causes the output of the process to be sent    \n\
        to the invoker of the process, without interpretation as kdbx       \n\
	commands.                                                           \n\
    The -tty switch causes kdbx to talk to the subprocess through a tty     \n\
        instead of pipes.  If -pipe is also present,                        \n\
        then this is ignored." },
  { "print", do_print, False, False,
"print string - Print string on the terminal.  If this command is used by   \n\
	an extension, it receives no output." },
  { "quit", do_quit, False, False,
"quit - Exit the current command loop.  If the current command loop is the  \n\
    top level loop that the user is using, kdbx exits.  Otherwise control   \n\
    is given to the next lowest loop." },
  { "source", do_source, False, False,
"source [-x] [files...] - Reads and interprets files as kdbx commands       \n\
    in the context of the current aliases.  If -x is present, then commands \n\
    are printed out as they are executed." },
  { "suspend", do_suspend,  False,  False, NULL },
  { "unalias", do_unalias, False, False,
"unalias name - Removes alias, if any, from name." }
};

static Boolean circular(TokenArray *new, AliasArray *old);
static char *read_command(FILE *fp);
static Boolean run_extension(Context *context, char *dir, char *name,
			     int namelen, Boolean *did_head, Boolean one_line,
			     IORec *io);
static Boolean run_extensions(Context *context, char *dir, char *arg,
			      Boolean long_help, ExtHelp *ext, IORec *io);
static Boolean command_help(Context *context, char *arg, Boolean long_help,
			    IORec *io);
static Boolean alias_help(Context *context, char *arg, Boolean long_help,
			  IORec *io);
static Boolean extension_help(Context *context, char *arg, Boolean long_help,
			      IORec *io, ExtHelp *ext);
static Context *new_context(void);
static Context *copy_context(Context *c);
static Boolean get_tty(char *name, FILE **fp);
static FILE **start_proc(TokenArray *args, int start, Boolean use_tty,
			 FILE **fp_ret, pid_t *pid_ret);
static FILE **start_pipe(char *in, char *out, FILE **fp);
static void readfile(char *file, Boolean xflag, Context *context,
		     Boolean quiet);
static void child_handler(int sig);
static void alarm_handler(int sig);
static void pipe_handler(int sig);
static void interrupt(int sig);
static void resignal(int sig);
static void cont_handler(int sig);
static void window(int sig);
static FILE *start_dbx(int argc, char **argv, Context *context);
static void expand_aliases1(TokenArray *command, int start, Context *context);
static Boolean expand_aliases(TokenArray *command, int start,
			      Context *context);
static void Usage(void);
static void execute(Context *context, char *command);
static Boolean execute_command(TokenArray *command, int start, int end,
			       Context *context, IORec *io_rec,
			       Boolean *user_input);
static Boolean parse(TokenArray *tokens, Context *context);
static Boolean do_normal_proc(TokenArray *args, Context *context,
			      Boolean debug, Boolean redirect, Boolean out,
			      Boolean use_tty, int start, IORec *io);
static Boolean do_pipeproc(TokenArray *args, Context *context, Boolean debug,
			   Boolean redirect, Boolean print_out, int start,
			   IORec *io);
static Boolean proc_cmd(TokenArray *args, Boolean *debug, Boolean *pipe,
			Boolean *redirect, Boolean *out, Boolean *use_tty,
			int *start);
static char *crunch(char *str);
static Boolean handle_io(Context *context, IOArray *io_array);
static void register_pid(pid_t pid, IOArray *io_array, int n);
static void delete_pid_index(IOArray *array, int n);
static void delete_pid(pid_t pid);
static TokenArray *ttoalias(TokenArray *tokens);
static void free_context(Context *c);
static void exit_status(pid_t pid, union wait *status);
static void resignal(int sig);
static void close_null_entries(IOArray *ios, Boolean force);
static Boolean io_isreadable(IORec *io, fd_set *mask);
static Boolean io_iswritable(IORec *io, fd_set *mask, Boolean check_mask);
static void setup_io(Context *context, IOArray *ios, IORec *io_rec,
		     Boolean pipe, Boolean user_input);
static char *stringify(TokenArray *tokens);
static TokenArray *eval_backquote(char *str, Context *context);
static void remove_pipes(void *arg);
static Boolean complain(pid_t pid);
static void dont_complain(pid_t pid);

Boolean dbx_dirty = False;
Boolean dbx_up = False;
Boolean terminal = False;
char *yylval = NULL;
struct sgttyb ttybsave;
struct sgttyb ttybnew;
Boolean prompting = False;
int dbx_pid;
jmp_buf current_jmp;
struct winsize win;

static Boolean intr = False;
static Boolean dbx_synced = False;
static Boolean intr_exit = True;
static Boolean in_loop = False;
static Boolean alarm_wentoff = False;
static Boolean prompt_after = False;
static char *image = NULL;
static char *corefile = NULL;
static Boolean kernel;
static pt_entry_t *ptes = NULL;

static jmp_buf alarm_jmp;

static char repaint = '\023';
static char *dbx_to_use = NULL;

static char **env;

static IORec user_out_rec;

static PidArray *pids = NULL;

static ProcArray *no_complain = NULL;

static fd_set select_rmask;
static struct timeval no_time = {0, 0};
static struct timeval *select_time = NULL;

static int interpret_level = 0;

static void register_pid(pid_t pid, IOArray *io_array, int n)
{
  PidRec p;

  assert(pids != NULL);
  p.pid = pid;
  p.array = io_array;
  p.index = n;
  PidArrayAppend(pids, &p);
}

static void delete_pid_index(IOArray *array, int n)
{
  PidRec *p;

  assert(pids != NULL);
  PidArrayIterPtr(pids, p){
    if(p->array == array){
      if(p->index == n){
#ifdef notdef
	if(kill(p->pid, SIGKILL) == -1){
	  if(errno != ESRCH) perror("killing process");
	}
#endif
	PidArrayDeleteElement(pids, p);
	p = PidArrayElement(pids, PidArrayIndex(pids, p) - 1);
      }
      else if(p->index > n) p->index--;
    }
  }
}

static void delete_pid(pid_t pid)
{
  PidRec *p;
  Boolean found;
  IORec *io;
  int i;

  assert(pids != NULL);
  found = False;
  PidArrayIterPtr(pids, p){
    if(p->pid == pid){
      if(p->array){
#ifdef notdef
	io = IOArrayElement(p->array, p->index);
	if(io->in_part.type != NoInput){
	  if(io->in_part.type == FileInput){
	    FD_CLR(fileno(io->in_part.u.file.fp), &select_rmask);
	    select_time = &no_time;
	  }
	  else fprintf(stderr, "delete_pid - type not file or packet\n");
	}
	(*io->out_part.proc)(io, NULL);
	set_null_out(io);
#endif
      }
      else {
	i = PidArrayIndex(pids, p);
	PidArrayDeleteElement(pids, p);
	p = PidArrayElement(pids, i - 1);
      }
      found = True;
    }
  }
  if(!found)
    fprintf(stderr, "delete_pid - pid %d not registered\n", pid);
}

static TokenArray *ttoalias(TokenArray *tokens)
{
  TokenArray *ret;
  Token *t, token;
  Boolean first;

  assert(tokens != NULL);
  ret = TokenArrayNew(NULL);
  first = True;
  TokenArrayIterPtr(tokens, t){
    if(first){
      first = False;
      continue;
    }
    token = *t;
    token.val = copy(token.val, -1);
    TokenArrayAppend(ret, &token);
  }
  return(ret);
}

static Boolean circular(TokenArray *new, AliasArray *old)
{
  char *cmd;
  Token *ptr;
  int len;
  Boolean found;
  AliasRec *a;

  assert((TokenArraySize(new) > 1) && (old != NULL));
  cmd = (TokenArrayElement(new, 0))->val;
  ptr = TokenArrayElement(new, 1);
  while(ptr){
    if(ptr->type == DOUBLE_STRING) break;
    if(!strcmp(ptr->val, cmd)) return(True);
    found = False;
    AliasArrayIterPtr(old, a){
      if(!strcmp(a->cmd, ptr->val)){
	found = True;
	break;
      }
    }
    if(found) ptr = TokenArrayFirstElement(a->args);
    else break;
  }
  return(False);
}

static Boolean do_alias(TokenArray *args, Context *context, IORec *io)
{
  AliasRec *a, alias;
  Boolean found, vflag;
  char *cmd, *s;
  int i;
  Token *t;

  assert((args != NULL) && (context != NULL) && (io != NULL));
  set_null_out(io);
  set_string_in(io, NULL);
  cmd = NULL;
  vflag = False;
  if(TokenArraySize(args) > 0){
    s = (TokenArrayElement(args, 0))->val;
    if(!strcmp(s, "-val")){
      if(TokenArraySize(args) != 2){
	append_string(io, "alias: -val is used with one other argument\n");
	return(True);
      }
      vflag = True;
      cmd = TokenArrayElement(args, 1)->val;
      TokenArrayDelete(args, 0);
    }
    else cmd = s;
  }
  if(TokenArraySize(args) < 2){
    AliasArrayIterPtr(context->current->aliases, a){
      if(((TokenArraySize(args) == 1) && !strcmp(cmd, a->cmd)) ||
	 (TokenArraySize(args) == 0)){
	if(!vflag){
	  append_string(io, a->cmd);
	  append_string(io, "\t");
	}
	TokenArrayIterPtr(a->args, t){
	  append_string(io, t->val);
	  append_string(io, " ");
	}
	append_string(io, "\n");
      }
    }
  }
  else if(circular(args, context->current->aliases)){
    append_string(io, "alias: Circular aliases are not allowed\n");
  }
  else {
    found = False;
    AliasArrayIterPtr(context->current->aliases, a){
      if(!strcmp(a->cmd, cmd)){
	found = True;
	break;
      }
    }
    if(found){
      TokenArrayIterPtr(a->args, t) if(t->val) free(t->val);
      TokenArrayDestroy(a->args);
      a->args = ttoalias(args);
    }
    else {
      alias.cmd = copy(cmd, -1);
      alias.args = ttoalias(args);
      AliasArrayAppend(context->current->aliases, &alias);
    }
  }
  return(True);
}

static Boolean do_core(TokenArray *args, Context *context, IORec *io)
{
  return(False);
}

static Boolean read_addr(int fd, unsigned long addr, unsigned long *val)
{
  if(lseek(fd, addr, SEEK_SET) == -1) return(False);
  if(read(fd, val, sizeof(long)) != sizeof(long)) return(False);
  return(True);
}

#ifdef __alpha
static Boolean addr_to_offset(unsigned long addr, unsigned long *ret, int fd,
			      Context *context)
{
  TokenArray *tokens;
  Token *t;
  pt_entry_t pte, *val;  
  char *str, *ptr;

  if(kernel){
    if(IS_SEG1_VA(addr)){
      if(ptes == NULL){
	tokens = eval_backquote("p kernel_pmap.level1_pt", context);
	str = TokenArrayElement(tokens, 0)->val;
	ptes = (pt_entry_t *) strtoul(str, &ptr, 0);
	if(ptr - str != strlen(str)) return(False);
	TokenArrayIterPtr(tokens, t) if(t->val) free(t->val);
	TokenArrayDestroy(tokens);
      }
      if(!addr_to_offset((unsigned long) (ptes + LEVEL1_PT_OFFSET(addr)),
			 (unsigned long *) &val, fd, context)) return(False);
      if(!read_addr(fd, (unsigned long) val, (unsigned long *) &pte))
	return(False);
      val = ((pt_entry_t *) PTETOPHYS(&pte)) + LEVEL2_PT_OFFSET(addr);
      if(!read_addr(fd, (unsigned long) val, (unsigned long *) &pte))
	return(False);
      val = ((pt_entry_t *) PTETOPHYS(&pte)) + LEVEL3_PT_OFFSET(addr);
      if(!read_addr(fd, (unsigned long) val, (unsigned long *) &pte))
	return(False);
      *ret = PTETOPHYS(&pte) + (addr & ((1 << PGSHIFT) - 1));      
      return(True);
    }
    else if(IS_KSEG_VA(addr)){
      *ret = KSEG_TO_PHYS(addr);
      return(True);
    }
    else {
      return(False);
    }
  }
  else {
    *ret = addr;
    return(True);
  }
}
#endif

static Boolean do_coredata(TokenArray *args, Context *context, IORec *io)
{
  TokenArray *tokens;
  Token *t;
  char *arg1, *arg2, *ptr, buf[64], command[32];
  unsigned long val1, val2, addr, buffer[(1<<PGSHIFT)/sizeof(long)];
  int n, fd, i, remain;
  Boolean ret;
  
  if(!corefile) return(False);
  if(TokenArraySize(args) != 2) return(False);
  set_null_out(io);
  set_string_in(io, NULL);
  arg1 = (TokenArrayElement(args, 0))->val;
  arg2 = (TokenArrayElement(args, 1))->val;
  val1 = strtoul(arg1, &ptr, 0);
  if(ptr - arg1 != strlen(arg1)) return(False);
  val2 = strtoul(arg2, &ptr, 0);
  if(ptr - arg2 != strlen(arg2)) return(False);
  if((fd = open(corefile, O_RDONLY)) == -1) return(False);
  remain = (val2 - val1 + sizeof(long) - 1);
  ret = True;
  if(!addr_to_offset(val1, &addr, fd, context)){
    sprintf(command, "0x%p/%dX", val1, remain/sizeof(long));
    tokens = eval_backquote(command, context);
    TokenArrayIterPtr(tokens, t){      
      if((t->type == ID) && (*(STRING_END(t->val) - 1)!= ':')){
	append_string(io, "0x");
	append_string(io, t->val);
	append_string(io, "\n");
      }
      free(t->val);
    }
    remain = 0;
    TokenArrayDestroy(tokens);
  }
  while(remain > 0){
    n = (1 << PGSHIFT) - (val1 & ((1 << PGSHIFT) - 1));
    if(n > remain) n = remain;
    if(!addr_to_offset(val1, &addr, fd, context) ||
       (lseek(fd, addr, SEEK_SET) == -1) ||
       (read(fd, buffer, n) != n)){
      ret = False;
      break;
    }
    if(ret){
      for(i=0;i<n/sizeof(long);i++){
	sprintf(buf, "0x%p\n", buffer[i]);
	append_string(io, buf);      
      }
    }
    remain -= n;
    val1 += n;
  }
  close(fd);
  return(ret);
}

static LDFILE *ldptr = NULL;

static char *lookup_address(char *file, long addr)
{
  SYMR sym;
  int i, n;

  if(ldptr == NULL){
    if((ldptr = ldopen(file, NULL)) == NULL) return(NULL);
  }
  if(PSYMTAB(ldptr) == 0) return(NULL);
  n = SYMHEADER(ldptr).isymMax + SYMHEADER(ldptr).iextMax;
  for(i=0;i<n;i++){
    if(ldtbread(ldptr, i, &sym) != SUCCESS) return(NULL);
    if(sym.value == addr) return(ldgetname(ldptr, &sym));
  }
  return(NULL);
}

static Boolean do_data_symbol(TokenArray *args, Context *context, IORec *io)
{
  Token *t;
  long val;
  char *ptr, *name;
  
  if(!image) return(False);
  set_null_out(io);
  set_string_in(io, NULL);
  TokenArrayIterPtr(args, t){
    val = strtoul(t->val, &ptr, 0);
    if(ptr - t->val == strlen(t->val)){
      if(kernel && IS_SEG1_VA(val)) append_string(io, "\n");
      else if((name = lookup_address(image, val)) != NULL)
	append_string(io, name);
    }
    append_string(io, "\n");
  }
  return(True);
}

static Boolean do_context(TokenArray *args, Context *context, IORec *io)
{
  set_null_out(io);
  set_string_in(io, NULL);
  if(TokenArraySize(args) == 0){
    if(context->current == context->user) append_string(io, "user\n");
    else append_string(io, "proc\n");
    return(True);
  }
  if((TokenArraySize(args) != 1) ||
     (strcmp((TokenArrayElement(args, 0))->val, "user") &&
      strcmp((TokenArrayElement(args, 0))->val, "proc"))) return(False);
  if(!strcmp((TokenArrayElement(args, 0))->val, "user"))
    context->current = context->user;
  else context->current = &context->vis;
  return(True);
}

char *copy(char *ptr, int len)
{
  char *ret;

  if(ptr == NULL) return(NULL);
  if(len == -1) len = strlen(ptr);
  NEW_TYPE(ret, len + 1, char, char *, "copy");
  strncpy(ret, ptr, len);
  ret[len] = '\0';
  return(ret);
}

static void clear_dirty(void *arg)
{
  dbx_dirty = False;
}

static Boolean do_dbx(TokenArray *args, Context *context, IORec *io)
{
  int size;
  Token *t;
  char *ptr, *str;

  if(TokenArraySize(args) == 0) return(False);
  size = 2;
  TokenArrayIterPtr(args, t){
    size += strlen(t->val) + 1;
  }
  NEW_TYPE(str, size, char, char *, "do_dbx");
  ptr = str;
  TokenArrayIterPtr(args, t){
    strcpy(ptr, t->val);
    strcat(ptr, " ");
    ptr = STRING_END(ptr);
  }
  strcat(ptr, "\n");
  set_null_out(&context->in_rec);
  set_string_in(&context->in_rec, str);
  set_text_out(io, context->dbx_fp, True);
  set_packet_in(io, context->dbx_fp, True);
  dbx_dirty = True;
  add_cleanup(clear_dirty, NULL);
  return(True);
}

static char *read_command(FILE *fp)
{
  char *ret, *ptr, buf[256];
  int olen, len;

  ret = NULL;
  while(1){
    errno = 0;
    if(fgets(buf, sizeof(buf)/sizeof(buf[0]), fp) == NULL){
      if(errno == EINTR){
	if(alarm_wentoff) alarm_wentoff = False;
	else continue;
      }
      if((errno != EINTR) && (errno != 0)) perror("read_command");
      break;
    }
    else {
      if(prompting) prompting = False;
      if(!ret){
	len = strlen(buf);
	NEW_TYPE(ret, len + 1, char, char *, "read_command");
	ptr = ret;
      }
      else {
	olen = len;
	len += strlen(buf);
	ret = (char *) realloc(ret, (len + 1) * sizeof(char));
	ptr = &ret[olen];
      }
      strcpy(ptr, buf);
      if(ret[len - 1] == '\n') {
	ret[len - 1] = '\0';
	break;
      }
    }
  }
  return(ret);
}

static Boolean run_extension(Context *context, char *dir, char *name,
			     int namelen, Boolean *did_head, Boolean one_line,
			     IORec *io)
{
  char *args[3], filename[MAXPATHLEN+1], *line, *last = NULL;
  int pid, in_fd[2], out_fd[2], len, mask;
  static int seconds = 2;
  FILE *in_fp;
  struct stat buf;
  Boolean ret;

  ret = False;
  strcpy(filename, dir);
  strcat(filename, "/");
  len = strlen(filename);
  strncat(filename, name, namelen);
  filename[len + namelen] = '\0';
  if((access(filename, X_OK) == 0) && (stat(filename, &buf) == 0) &&
     ((buf.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) != 0) &&
     (S_ISREG(buf.st_mode))){
    args[0] = filename;
    args[1] = "-help";
    args[2] = NULL;
    errno = 0;
    if((pipe(in_fd) == -1) || (pipe(out_fd) == -1)){
      perror("pipe");
      return(False);
    }
    errno = 0;
#ifdef notdef
    mask = sigblock(1 << SIGCHLD);
#endif
    if((pid = vfork()) != 0){
      register_pid(pid, NULL, 0);
      dont_complain(pid);
#ifdef notdef
      sigsetmask(mask);
#endif
      close(in_fd[1]);
      close(out_fd[0]);
      if((in_fp = fdopen(in_fd[0], "r")) == NULL){
	perror("fdopen");
	close(in_fd[0]);
	close(in_fd[1]);
	close(out_fd[0]);
	close(out_fd[1]);
	return(False);
      }
      do {
	if(setjmp(alarm_jmp) != 0) break;
	alarm(seconds);
	if((line = read_command(in_fp)) == NULL) break;
	alarm(0);
	if(last){
	  len = strlen(last);
	  if(last[len - 1] != '\\'){
	    free(last);
	    break;
	  }
	  last[len - 1] = '\0';
	  if(!*did_head){
	    append_string(io, "Extensions:\n");
	    append_string(io, "-----------\n");
	    *did_head = True;
	  }
	  append_string(io, last);
	  append_string(io, "\n");
	  ret = True;
	  if(one_line) break;
	  free(last);
	}
	last = line;
      } while(line != NULL);
      alarm(0);
      if(last) free(last);
      kill(pid, SIGKILL);
      fclose(in_fp);
      close(out_fd[1]);
    }
    else {
#ifdef notdef
      sigsetmask(mask);
#endif
      if((dup2(in_fd[1], 1) == -1) || (dup2(in_fd[1], 2) == -1) ||
	 (dup2(out_fd[0], 0) == -1)) exit(1);
      execvp(filename, args);
      exit(1);
    }
  }
  return(ret);
}

static Boolean run_extensions(Context *context, char *dir, char *arg,
			   Boolean long_help, ExtHelp *ext, IORec *io)
{
  struct dirent *dp;
  Boolean one_line, found, ret;
  char *s;
  int i;

  ret = False;
  if(*dir == '\0') dir = ".";
  if((arg == NULL) && (long_help == False)) one_line = True;
  else one_line = False;
  if(ext->dirp == NULL) ext->dirp = opendir(dir);
  if(ext->dirp != NULL){
    while((dp=readdir(ext->dirp)) != NULL){
      if((arg == NULL) || !strncmp(arg, dp->d_name, strlen(arg))){
	found = False;
	StringArrayIter(ext->exe_done, s, i){
	  if(!strncmp(dp->d_name, s, dp->d_namlen)){
	    found = True;
	    break;
	  }
	}
	if(!found){
	  if(run_extension(context, dir, dp->d_name, dp->d_namlen,
			   &ext->did_head, one_line, io)){
	    s = dp->d_name;
	    StringArrayAppend(ext->exe_done, &s);
	    ret = True;
	    break;
	  }
	}
      }
    }
  }
  else {
    closedir(ext->dirp);
    ext->dirp = NULL;
  }
  return(ret);
}

static Boolean command_help(Context *context, char *arg, Boolean long_help,
			 IORec *io)
{
  Boolean did_head, all;
  int i;

  did_head = False;
  if(arg == NULL) all = True;
  else all = False;
  for(i=0;i<sizeof(commands)/sizeof(commands[0]);i++){
    if(all || !strncmp(arg, commands[i].command, strlen(arg))){
      if(!did_head){
	append_string(io, "Commands:\n");
	append_string(io, "---------\n");
	did_head = True;
      }
      if(commands[i].help){
	append_string(io, commands[i].help);
	append_string(io, "\n");
      }
    }
  }
  return(did_head);
}

static Boolean alias_help(Context *context, char *arg, Boolean long_help,
		       IORec *io)
{
  Boolean did_head, all;
  AliasRec *a;
  Token *t;

  did_head = False;
  if(arg == NULL) all = True;
  else all = False;
  AliasArrayIterPtr(context->current->aliases, a){
    if(all || !strncmp(arg, a->cmd, strlen(arg))){
      if(!did_head){
	append_string(io, "Aliases:\n");
	append_string(io, "--------\n");
	did_head = True;
      }
      append_string(io, a->cmd);
      append_string(io, "\t");
      TokenArrayIterPtr(a->args, t){
	append_string(io, t->val);
	append_string(io, " ");
      }
      append_string(io, "\n");
    }
  }
  return(did_head);
}

static void new_exthelp(ExtHelp *ext)
{
  ext->done = False;
  ext->read_more = True;
  ext->exe_done = StringArrayNew(NULL);
  ext->ext_path = copy(getenv("KRASH_EXTPATH"), -1);
  ext->ext_path_save = ext->ext_path;
  ext->did_head = False;
  ext->dirp = NULL;
}

static void free_exthelp(ExtHelp *ext)
{
  StringArrayDestroy(ext->exe_done);
  free(ext->ext_path_save);
}

static Boolean extension_help(Context *context, char *arg, Boolean long_help,
			      IORec *io, ExtHelp *ext)
{  
  char *ptr, *dir, c;

  if(ext->ext_path && *ext->ext_path){
    for(ptr=ext->ext_path;*ptr && (*ptr != ':');ptr++) ;
    c = *ptr;
    *ptr = '\0';
    if(!run_extensions(context, ext->ext_path, arg, long_help, ext, io)){
      if(c == ':') ext->ext_path = ptr + 1;
      else ext->ext_path = ptr;
    }
  }
  else {
    if((dir = getenv("KRASH_LIBDIR")) == NULL) dir = "/var/kdbx";
    if(!run_extensions(context, dir, arg, long_help, ext, io))
      ext->done = True;
  }
  return(ext->did_head);
}

static Boolean help_cb(IORec *io, char *buf, int size)
{
  HelpRec *arg;
  Boolean ret;

  if(size == 0){
    set_null_in(io);
    return(False);
  }
  arg = (HelpRec *) io->in_part.u.arg;
  ret = True;
  switch(arg->where){
  case COMMAND_HELP:
    if(arg->first){
      if(command_help(arg->context, arg->subject, arg->long_help, &arg->io))
	arg->found_topic = True;
      arg->first = False;
    }
    if(read_string_block(&arg->io, buf, size) == False){
      arg->where = ALIAS_HELP;
      arg->first = True;
    }
    break;
  case ALIAS_HELP:
    if(arg->first){
      if(alias_help(arg->context, arg->subject, arg->long_help, &arg->io))
	arg->found_topic = True;
      arg->first = False;
    }
    if(read_string_block(&arg->io, buf, size) == False){
      arg->where = EXTENSION_HELP;
      arg->first = True;
    }
    break;
  case EXTENSION_HELP:
    if(arg->first) new_exthelp(&arg->ext);
    if(arg->ext.read_more){
      if(extension_help(arg->context, arg->subject, arg->long_help, &arg->io,
			&arg->ext))
	arg->found_topic = True;
      arg->ext.read_more = False;
    }
    arg->first = False;
    if(read_string_block(&arg->io, buf, size) == False){
      if(arg->ext.done == True){
	arg->where = NO_HELP;
	arg->first = True;
	free_exthelp(&arg->ext);
      }
      else arg->ext.read_more = True;
    }      
    break;
  case NO_HELP:
    if(arg->first){
      if(arg->found_topic == False){
	append_string(&arg->io, "There are no commands, aliases, or extensions beginning with \"");
	append_string(&arg->io, arg->subject);
	append_string(&arg->io, "\".\n");
      }
      arg->first = False;
    }
    ret = read_string_block(&arg->io, buf, size);
    break;
  default:
    fprintf(stderr, "help_cb - Bad type : %d\n", arg->where);
    break;
  }
  return(ret);
}

static Boolean do_help(TokenArray *args, Context *context, IORec *io)
{
  Boolean long_help;
  char *arg;
  HelpRec *rec;

  if(TokenArraySize(args) > 0){
    if(!strcmp((TokenArrayFirstElement(args))->val, "-long")){
      if(TokenArraySize(args) > 2) return(False);
      if(TokenArraySize(args) == 2) arg = (TokenArrayElement(args, 1))->val;
      else arg = NULL;
      long_help = True;
    }
    else {
      if(TokenArraySize(args) > 1) return(False);
      arg = (TokenArrayElement(args, 0))->val;
      long_help = True;
    }
  }
  else {
    long_help = False;
    arg = NULL;
  }
  NEW_TYPE(rec, 1, HelpRec, HelpRec *, "do_help");
  rec->found_topic = False;
  rec->where = COMMAND_HELP;
  rec->first = True;
  rec->context = context;
  rec->subject = copy(arg, -1);
  rec->long_help = long_help;
  set_null_out(io);
  set_cb_in(io, ImmediateInput, help_cb, NULL, False, rec);
  set_null_out(&rec->io);
  set_string_in(&rec->io, NULL);
  return(True);
}

static Context *new_context(void)
{
  Context *context;

  NEW_TYPE(context, 1, Context, Context *, "new_context");
  context->vis.aliases = AliasArrayNew(NULL);
  context->debug = False;
  context->debug_recurse = True;
  context->redirect_output = False;
  context->user = NULL;
  context->prompt_proc = NULL;
  context->current = &context->vis;
  context->parent = NULL;
  context->pid = -1;
  set_text_in(&context->in_rec, stdin, True, True);
  set_null_out(&context->in_rec);
  set_null_in(&context->out_rec);
  set_text_out(&context->out_rec, stdout, True);
  context->print_rec = user_out_rec;
  set_null_in(&context->print_rec);
  context->in_rec.context = context;
  context->in_rec.free_context = False;
  context->out_rec.context = context;
  context->out_rec.free_context = False;
  context->print_rec.context = context;
  context->print_rec.free_context = False;
  context->close_on_end = False;
  context->close_with_eof = False;
  context->level = 0;
  context->sub_level = 0;
  return(context);
}

static Context *copy_context(Context *c)
{
  Context *ret;
  AliasRec *a, alias;
  Token *t, token;
  TokenArray *args;

  NEW_TYPE(ret, 1, Context, Context *, "copy_context");
  *ret = *c;
  ret->vis.aliases = AliasArrayNew(NULL);
  if(c->current == &c->vis) ret->current = &ret->vis;
  else ret->current = ret->user;
  ret->out_rec.context = ret;
  ret->in_rec.context = ret;
  ret->print_rec.context = ret;
  ret->close_on_end = False;
  ret->level = 0;
  ret->sub_level = 0;
  return(ret);  
}

static void free_context(Context *c)
{
  AliasRec *a;
  Token *t;

  AliasArrayIterPtr(c->vis.aliases, a){
    free(a->cmd);
    TokenArrayIterPtr(a->args, t){
      if(t->val) free(t->val);
    }
    TokenArrayDestroy(a->args);
  }
  AliasArrayDestroy(c->vis.aliases);
  free(c);
}

#ifdef notdef
static Boolean get_pipes(in_fd, out_fd, in_fp, out_fp)
int *in_fd, *out_fd;
FILE **in_fp, **out_fp;
{
  if((pipe(in_fd) == -1) || (pipe(out_fd) == -1)){
    perror("pipe");
    return(False);
  }
  if(((*in_fp = fdopen(in_fd[0], "r")) == NULL) ||
     ((*out_fp = fdopen(out_fd[0], "w")) == NULL)){
    perror("fdopen");
    return(False);
  }
  return(True);
}
#endif

static Boolean get_tty(char *name, FILE **fp)
{
  char *c1, *c2;
  int fd;

  *fp = NULL;
  strcpy(name, "/dev/ptyxx");
  for(c1="pqrstuvwxyz";*c1;c1++){
    name[8] = *c1;
    for(c2="0123456789abcdef";*c2;c2++){
      name[9] = *c2;
      if((fd = open(name, O_RDWR)) != -1){
	*fp = fdopen(fd, "r+");
	return(True);
      }
    }
  }
  fprintf(stderr, "get_tty - Couldn't get a tty\n");
  return(False);
}

static FILE **start_proc(TokenArray *args, int start, Boolean use_tty,
			 FILE **fp_ret, pid_t *pid_ret)
{
  int in_fd[2], out_fd[2], fd, i, tty;
  char **argv, *str, name[MAXPATHLEN];
  FILE *fp;
  struct sgttyb buf;
  Token *t;

  if(start == TokenArraySize(args)) return(NULL);
  if(!use_tty){
    if((pipe(in_fd) == -1) || (pipe(out_fd) == -1)){
      perror("pipe");
      return(NULL);
    }
  }
  NEW_TYPE(argv, TokenArraySize(args) + 1 - start, char *, char **,
	   "start_proc");
  i = 0;
  TokenArrayIterPtr(args, t){
    if(i >= start){
      argv[i - start] = t->val;
    }
    i++;
  }
  argv[i - start] = NULL;
  if(use_tty){
    if((*pid_ret = forkpty(&fd, NULL, NULL, NULL)) == NULL) return(NULL);
  }
  else *pid_ret = vfork();
  if(*pid_ret != 0){
    if(use_tty){
      fp_ret[0] = fdopen(fd, "r+");
      fp_ret[1] = fp_ret[0];
    }
    else {
      close(in_fd[1]);
      close(out_fd[0]);
      fp_ret[0] = fdopen(in_fd[0], "r");
      fp_ret[1] = fdopen(out_fd[1], "w");
    }
  }
  else {
    if(use_tty){
      ioctl(0, TIOCGETP, &buf);
      buf.sg_flags &= ~ECHO;
      buf.sg_flags |= RAW;
      ioctl(0, TIOCSETP, &buf);
      ioctl(1, TIOCGETP, &buf);
      buf.sg_flags &= ~ECHO;
      buf.sg_flags |= RAW;
      ioctl(1, TIOCSETP, &buf);
    }
    else {
      close(in_fd[0]);
      close(out_fd[1]);
      if((dup2(in_fd[1], 1) == -1) || (dup2(out_fd[0], 0) == -1)){
	perror("dup2");
	exit(1);
      }
    }
    execvp(argv[0], argv);
    fprintf(stderr, "Couldn't exec %s:\n", argv[0]);
    perror("execvp");
    exit(1);
  }
  free(argv);
  return(fp_ret);
}

static Boolean proc_cmd(TokenArray *args, Boolean *debug, Boolean *pipe,
			Boolean *redirect, Boolean *out, Boolean *use_tty,
			int *start)
{
  Token *t;

  if(TokenArraySize(args) < 1) return(False);
  TokenArrayIterPtr(args, t){
    if(*t->val == '-'){
      if(!strcmp(t->val, "-debug")) *debug = True;
      else if(!strcmp(t->val, "-pipe")) *pipe = True;
      else if(!strcmp(t->val, "-redirect_output")) *redirect = True;
      else if(!strcmp(t->val, "-print_output")) *out = True;
      else if(!strcmp(t->val, "-tty")) *use_tty = True;
      else return(False);
    }
    else break;
  }
  if(start) *start = TokenArrayIndex(args, t);
  if(*pipe){
    if(TokenArraySize(args) - *start != 2) return(False);
    return(True);
  }
  else return(True);
}

static Boolean do_proc(TokenArray *args, Context *context, IORec *io)
{
  Boolean debug, pipe, redirect, out, use_tty, ret;
  int i;

  debug = False;
  pipe = False;
  redirect = False;
  out = False;
  use_tty = False;
  if(!proc_cmd(args, &debug, &pipe, &redirect, &out, &use_tty, &i))
    return(False);
  if(pipe) ret = do_pipeproc(args, context, debug, redirect, out, i, io);
  else ret = do_normal_proc(args, context, debug, redirect, out, use_tty, i,
			    io);
  context->close_on_end = True;
  prompt_after = True;
  return(ret);
}

static Boolean do_normal_proc(TokenArray *args, Context *context,
			      Boolean debug, Boolean redirect, Boolean out,
			      Boolean use_tty, int start, IORec *io)
{
  FILE *fp[2];
  pid_t pid;
  jmp_buf jmp;
  int val, mask;
  struct sgttyb buf;

  mask = sigblock(0);
  sigsetmask(mask & ~(1 << SIGCHLD));
  if(start_proc(args, start, use_tty, fp, &pid) != NULL){
    register_pid(pid, NULL, 0);
    sigsetmask(mask);   
    context->redirect_output = redirect;
    context->debug = debug;
    context->pid = pid;
    if(use_tty){
      set_text_out(io, fp[1], True);
      set_text_in(io, fp[0], False, True);
      context->in_rec.in_part.interpret = False;
      context->close_with_eof = True;
      ioctl(fileno(fp[0]), TIOCGETP, &buf);
      buf.sg_flags |= RAW;
      buf.sg_flags &= ~ECHO;
      ioctl(fileno(fp[0]), TIOCSETP, &buf);
    }
    else {
      sigsetmask(mask);   
      set_packet_out(io, fp[1], False);
      set_text_in(io, fp[0], True, False);
      set_packet_out(&context->out_rec, fp[1], False);
      set_null_in(&context->out_rec);
      set_null_out(&context->in_rec);
      set_text_in(&context->in_rec, fp[0], True, False);
    }
    return(True);
  }
  else return(False);
}

static FILE **start_pipe(char *in, char *out, FILE **fp)
{
  if((mknod(in, S_IFIFO | 0666, 0) == -1) ||
     (mknod(out, S_IFIFO | 0666, 0) == -1)){
    perror("mknod");
    return(NULL);
  }
  if(((fp[0] = fopen(in, "r+")) == NULL) ||
     ((fp[1] = fopen(out, "r+")) == NULL)){
    perror("fopen");
    return(NULL);
  }
  return(fp);
}

static void remove_pipes(void *arg)
{
  if((unlink(((PipeRec *) arg)->in) == -1) ||
     (unlink(((PipeRec *) arg)->out) == -1)){
    perror("Removing pipes");
  }
  free(((PipeRec *) arg)->in);
  free(((PipeRec *) arg)->out);
}

static Boolean do_pipeproc(TokenArray *args, Context *context, Boolean debug,
			   Boolean redirect, Boolean print_out, int start,
			   IORec *io)
{
  FILE *fp[2];
  jmp_buf jmp;
  int val;
  char *out, *in_pipe, *out_pipe;
  PipeRec *cleanup_arg;

  in_pipe = (TokenArrayElement(args, start))->val;
  out_pipe = (TokenArrayElement(args, start + 1))->val;
  if((start_pipe(in_pipe, out_pipe, fp)) != NULL){
    context->redirect_output = redirect;
    context->debug = debug;
    NEW_TYPE(cleanup_arg, 1, PipeRec, PipeRec *, "do_pipeproc");
    cleanup_arg->in = copy(in_pipe, -1);
    cleanup_arg->out = copy(out_pipe, -1);
    add_cleanup(remove_pipes, cleanup_arg);
    set_packet_out(io, fp[1], False);
    set_text_in(io, fp[0], True, False);
    set_packet_out(&context->out_rec, fp[1], False);
    set_null_in(&context->out_rec);
    set_null_out(&context->in_rec);
    set_text_in(&context->in_rec, fp[0], True, False);
    return(True);
  }
  else return(False);
}

static char *concat(str, new)
char *str, *new;
{
  return(NULL);
}

static Boolean do_print(TokenArray *args, Context *context, IORec *io)
{
  Token *t;

  set_null_out(io);
  set_string_in(io, NULL);
  io->output_to_user = True;
  TokenArrayIterPtr(args, t){
    if(context->redirect_output){
    }
    else {
      append_string(io, t->val);
      if(t != TokenArrayLastElement(args)) append_string(io, " ");
    }
  }
  if(*(STRING_END(TokenArrayLastElement(args)->val) - 1) != '\n')
    append_string(io, "\n");
  return(True);
}

static Boolean do_quit(TokenArray *args, Context *context, IORec *io)
{
  longjmp(context->jmp, 1);
}

static void readfile(char *file, Boolean xflag, Context *context,
		     Boolean quiet)
{
  FILE *fp, *save;
  jmp_buf jmp;
  int val;
  Boolean save_debug, save_debug_recurse;
  IORec io;
  IOArray *ios;
  void (*save_prompt)(void *);

  if((fp = fopen(file, "r")) != NULL){
    save_debug = context->debug;
    save_debug_recurse = context->debug_recurse;
    save_prompt = context->prompt_proc;
    context->debug = xflag;
    context->debug_recurse = False;
    context->prompt_proc = NULL;
    ios = IOArrayNew(NULL);
    io.context = context;
    io.free_context = False;
    io.output_to_user = False;
    set_null_out(&io);
    set_text_in(&io, fp, True, False);
    IOArrayAppend(ios, &io);
    set_text_out(&io, stdout, True);
    set_null_in(&io);
    IOArrayAppend(ios, &io);
    handle_io(context, ios);
    IOArrayDestroy(ios);
    context->debug = save_debug;
    context->debug_recurse = save_debug_recurse;
    context->prompt_proc = save_prompt;
  }
  else if(!quiet) perror("Reading file");
}

static Boolean do_source(TokenArray *args, Context *context, IORec *io)
{
  Token *t;
  Boolean xflag, first;

  if(!strcmp((TokenArrayFirstElement(args))->val, "-x")){
    xflag = True;
    first = True;
  }
  else {
    xflag = False;
    first = False;
  }
  TokenArrayIterPtr(args, t){
    if(first){
      first = False;
      continue;
    }
    readfile(t->val, xflag, context, False);
  }
  set_null_out(io);
  set_string_in(io, NULL);
  return(True);
}

static Boolean do_suspend(TokenArray *args, Context *context, IORec *io)
{
  kill(getpid(), SIGSTOP);
  set_null_out(io);
  set_string_in(io, NULL);
  return(True);
}

static Boolean do_unalias(TokenArray *args, Context *context, IORec *io)
{
  AliasRec *a;
  char *buf, *msg = "unalias - %s is not aliased\n";
  Boolean found;
  Token *t;

  set_string_in(io, NULL);
  set_null_out(io);
  TokenArrayIterPtr(args, t){
    found = False;
    AliasArrayIterPtr(context->current->aliases, a){
      if(!strcmp(t->val, a->cmd)){
	AliasArrayDeleteElement(context->current->aliases, a);
	found = True;
	break;
      }
    }
    if(!found){
      NEW_TYPE(buf, strlen(msg) + strlen(t->val) + 1, char, char *,
	       "do_unalias");
      sprintf(buf, msg, t->val);
      append_string(io, buf);
      free(buf);
      free(buf);
    }
  }
  return(True);
}

static char *signals[] = { "Signal 0", "SIGHUP", "SIGINT", "SIGQUIT",
			     "SIGILL", "SIGTRAP", "SIGABRT", "SIGEMT",
			     "SIGFPE", "SIGKILL", "SIGBUS", "SIGSEGV",
			     "SIGSYS", "SIGPIPE", "SIGALRM", "SIGTERM",
			     "SIGURG", "SIGSTOP", "SIGTSTP", "SIGCONT",
			     "SIGCHLD", "SIGTTIN", "SIGTTOU", "SIGIO",
			     "SIGXCPU", "SIGXFSZ", "SIGVTALRM", "SIGPROF",
			     "SIGWINCH", "SIGINFO", "SIGUSR1", "SIGUSR2" };

static void exit_status(pid_t pid, union wait *status)
{
  if(WIFEXITED(*status)){
    if(WEXITSTATUS(*status) != 0)
      fprintf(stderr, "pid %d exited with status %d\n", pid,
	      WEXITSTATUS(*status));
  }
  else if(WIFSIGNALED(*status))
    if((WTERMSIG(*status) < 0) ||
       (WTERMSIG(*status) > sizeof(signals)/sizeof(signals[0])))
      fprintf(stderr, "pid %d exited with signal %d\n", pid,
	      WTERMSIG(*status));
    else fprintf(stderr, "pid %d exited with signal %s\n", pid,
		 signals[WTERMSIG(*status)]);
}

static Boolean complain(pid_t pid)
{
  pid_t p;
  int i;

  ProcArrayIter(no_complain, p, i){
    if(p == pid) {
      ProcArrayDelete(no_complain, i);
      return(False);
    }
  }
  return(True);
}

static void dont_complain(pid_t pid)
{
  ProcArrayAppend(no_complain, &pid);
}

static void child_handler(int sig)
{
  int pid, i, p;
  union wait status;
  Boolean found;

  while((pid = wait3(&status, WNOHANG, NULL)) != 0){
    if(pid == -1){
      perror("wait");
      break;
    }
    if(pid == dbx_pid){
      exit_status(pid, &status);
      fprintf(stderr, "dbx (pid %d) died.  Exiting...\n", dbx_pid);
      if(WIFEXITED(status)) exit(WEXITSTATUS(status));
      else exit(1);
    }
    else {
      if(complain(pid)) exit_status(pid, &status);
      delete_pid(pid);
    }
  }
#ifdef notdef
  printf("leaving child_handler - pid = %d\n", pid);
#endif
}

static void alarm_handler(int sig)
{
  alarm_wentoff = True;
#ifdef notdef
  longjmp(alarm_jmp, 1);
#endif
}

static void pipe_handler(int sig)
{
#ifdef notdef
  fprintf(stderr, "Broken pipe\n");
#endif
}

static void interrupt(int sig)
{
  Boolean dojump;

  if(!intr_exit){
    if(intr) dojump = False;
    else dojump = True;
    (void)ioctl(0, TIOCSETP, (char *)&ttybsave);
    if(dbx_dirty && (dbx_pid > 0)) kill(dbx_pid, SIGINT);
    if(dojump && (interpret_level > 0)){
      intr = True;
      longjmp(current_jmp, 2);
    }
  }
  else {
    signal(SIGINT, SIG_DFL);
    kill(getpid(), SIGINT);
  }
}

static void window(int sig)
{
  ioctl(1, TIOCGWINSZ, (char *) &win);
  if(win.ws_row == 0) win.ws_row = 24;
  if(win.ws_col == 0) win.ws_col = 80;
}

static void cont_handler(int sig)
{
}

static void resignal(int sig)
{
  kill(dbx_pid, SIGINT);
}

static FILE *start_dbx(int argc, char **argv, Context *context)
{
  FILE *fp;
  int i, fd;
  char **new_argv, tty[11], *c1, *c2, *dbxpath;
  struct sgttyb buf;
  void (*handler)();
  IOArray *ios;
  IORec io;

  NEW_TYPE(new_argv, argc + 3, char *, char **, "start_dbx");
  new_argv[0] = "dbx";
  new_argv[1] = "-vd";
  for(i=0;i<argc;i++) new_argv[i+2] = argv[i];
  new_argv[i+2] = NULL;
  if(!strcmp(argv[0], "-k")){
    kernel = True;
    argv++;
    argc--;
  }
  else kernel = False;
  if(argc >= 1) image = argv[0];
  if(argc >= 2) corefile = argv[1];
  else if(kernel) corefile = "/dev/mem";
  if(dbx_to_use == NULL) dbxpath = "dbx";
  else dbxpath = dbx_to_use;
  if((dbx_pid = forkpty(&fd, NULL, NULL, NULL)) == 0){
    setpgrp(0, getpid());
    for(i=0;i<3;i++){
      ioctl(i, TIOCGETP, &buf);
      buf.sg_flags |= RAW;
      buf.sg_flags &= ~ECHO;
      ioctl(i, TIOCSETP, &buf);
    }
    execvp(dbxpath, new_argv, env);
    perror("execvp");
    exit(1);
  }
  else if(dbx_pid == -1) return(NULL);
  ioctl(fd, TIOCGETP, &buf);
  buf.sg_flags |= RAW;
  buf.sg_flags &= ~ECHO;
  buf.sg_flags &= ~CRMOD;
  ioctl(fd, TIOCSETP, &buf);
  free(new_argv);
  if((fp = fdopen(fd, "r+")) == NULL) return(NULL);
  ios = IOArrayNew(NULL);
  if(isatty(0)){
    IOArrayAppend(ios, &context->in_rec);
    set_text_out(&io, fp, True);
  }
  else set_null_out(&io);
  set_packet_in(&io, fp, True);
  io.context = context;
  io.output_to_user = False;
  io.free_context = False;
  context->dbx_fp = fp;
  IOArrayAppend(ios, &io);
  IOArrayAppend(ios, &context->out_rec);
  handle_io(context, ios);
  IOArrayDestroy(ios);
#ifdef notdef
  signal(SIGINT, handler);
  kill(dbx_pid, SIGINT);
  while(!dbx_synced){
    if(!dbx_ready(fp, sync_dbx, stdout, False, &rec)) return(NULL);
  }
#endif
  return(fp);
}

static char *token_val(Token *t)
{
  switch(t->type){
    case SEMICOLON: return(";");
    case PIPE: return("|");
    case DOUBLE_STRING:
    case SINGLE_STRING:
    case BACK_QUOTE:
    case WHITESPACE:
    case ID:
      return(t->val);
    case COMMENT:
      return("");
  default:
    fprintf(stderr, "token_val - bad type : %d\n", t->type);
    break;
  }
}

static void expand_aliases1(TokenArray *command, int start, Context *context)
{
  AliasRec *a;
  Token *t, *tok, token;
  int i;

  if((TokenArrayElement(command, start))->type == ID){
    AliasArrayIterPtr(context->current->aliases, a){
      tok = TokenArrayElement(command, start);
      if(!strcmp(token_val(tok), a->cmd)){
	if(tok->val) free(tok->val);
	TokenArrayDelete(command, start);
	i = start;
	TokenArrayIterPtr(a->args, t){
	  token.type = t->type;
	  if(t->val) token.val = copy(t->val, -1);
	  TokenArrayInsert(command, i, &token);
	  i++;
	}
	expand_aliases1(command, start, context);
	break;
      }
    }
  }
}

static Boolean expand_aliases(TokenArray *command, int start, Context *context)
{
  int old, new, len, i, pos;
  char *str;
  Token *t, token;
  Boolean again;

  if(TokenArraySize(command) > 0){
    while(1){
      old = TokenArraySize(command);
      expand_aliases1(command, start, context);
      new = TokenArraySize(command);
      again = False;
      if(TokenArraySize(command) > 0){
	for(i=0;i<new-old+1;i++){
	  if((TokenArrayElement(command, start + i)->type == DOUBLE_STRING) ||
	     (TokenArrayElement(command, start + i)->type == SINGLE_STRING) ||
	     (TokenArrayElement(command, start + i)->type == BACK_QUOTE))
	    again = True;
	}
	len = 0;
	for(i=0;i<new-old+1;i++){
	  t = TokenArrayElement(command, start + i);
	  len += strlen(token_val(t)) + 1;
	}
	NEW_TYPE(str, len, char, char *, "expand_aliases");
	*str = '\0';
	for(i=0;i<new-old+1;i++){
	  t = TokenArrayElement(command, start);
	  strcat(str, token_val(t));
	  if(i < new - old) strcat(str, " ");
	  TokenArrayDelete(command, start);
	}
	set_input(str);
	pos = start;
	while((i = yylex()) != 0){
	  if(i == ERROR) return(False);
	  else if(i != WHITESPACE){
	    if((pos == start) && (i == DOUBLE_STRING)) i = ID;
	    token.type = i;
	    token.val = yylval;
	    TokenArrayInsert(command, pos, &token);
	    pos++;
	  }
	}
	free(str);
      }
      if(!again) break;
    }
  }
  return(True);
}

static char *crunch(char *str)
{
  char *ptr;

  if(!str) return(NULL);
  for(ptr=str;*ptr;ptr++){
    if(*ptr == '\\'){
      strcpy(ptr, ptr + 1);
      switch(*ptr){
      case 'b':
	*ptr = '\b';
	break;
      case 'f':
	*ptr = '\f';
	break;
      case 'n':
	*ptr = '\n';
	break;
      case 'r':
	*ptr = '\r';
	break;
      case 't':
	*ptr = '\t';
	break;
      case 'v':
	*ptr = '\v';
	break;
      case '\n':
	strcpy(ptr, ptr + 1);
	break;
      default:
	break;
      }
    }
  }
  return(str);
}

static void do_cleanup(IOArray *ios, Context *context, void (*proc)(void *),
		       void *arg)
{
  Boolean found;
  IORec *io;

  found = False;
  IOArrayIterPtr(ios, io){
    if(io->context == context){
      found = True;
      break;
    }
  }
  if(!found) (*proc)(arg);
}

static void close_null_entries(IOArray *ios, Boolean force)
{
  Boolean check_nulls;
  IORec *io, *next_io;
  Context *context;

  do {
    check_nulls = False;
    IOArrayIterPtr(ios, io){
      if(io == IOArrayLastElement(ios)) break;
      next_io = IOArrayElement(ios, IOArrayIndex(ios, io) + 1);
      if((io->in_part.type == NoInput) &&
	 (next_io->out_part.type != NoOutput)){
	(*next_io->out_part.proc)(next_io, NULL);
	set_null_out(next_io);
	check_nulls = True;
      }
      else if((next_io->out_part.type == NoOutput) &&
	      (io->in_part.type != NoInput)){
	(*io->in_part.block_proc)(io, NULL, 0);
	set_null_in(io);
	check_nulls = True;
      }
    }
  } while(check_nulls);
  IOArrayIterPtr(ios, io){
    if(((io->in_part.type == NoInput) && (io->out_part.type == NoOutput)) ||
       force){
      if(io->free_context) free_context(io->context);
      else if(--io->context->sub_level == 0) io->context->level--;
      IOArrayDeleteElement(ios, io);
      delete_pid_index(ios, IOArrayIndex(ios, io));
      io = IOArrayElement(ios, (long) IOArrayIndex(ios, io) - 1);
    }
  }
  if(IOArraySize(ios) == 2){
    io = IOArrayElement(ios, 0);
    if((io->in_part.type == FileInput) && (io->in_part.u.file.fp == stdin) &&
       ((io->context->level > 1) || (io->context->parent != NULL))){
      if(io->free_context) free_context(io->context);
      else if(--io->context->sub_level == 0) io->context->level--;
      IOArrayDeleteElement(ios, io);
      delete_pid_index(ios, IOArrayIndex(ios, io));      
      io = IOArrayElement(ios, 0);
      if(io->free_context) free_context(io->context);
      else if(--io->context->sub_level == 0) io->context->level--;
      IOArrayDeleteElement(ios, io);
      delete_pid_index(ios, IOArrayIndex(ios, io));            
    }
  }
}

static Boolean io_isreadable(IORec *io, fd_set *mask)
{
  int i;

  switch(io->in_part.type){
  case FileInput:
    if(io->in_part.u.file.buffered || FD_ISSET(fileno(io->in_part.u.file.fp),
					       mask)) return(True);
#ifdef notdef
    ioctl(fileno(io->in_part.u.file.fp), FIONREAD, &i);
    if(i > 0) return(True);
#endif
    else return(False);
  case ImmediateInput:
    return(True);
  case NoInput:
    return(False);
  default:
    fprintf(stderr,
	    "io_isreadable - bad in_part type : %d\n", io->in_part.type);
    return(False);
  }
}

static Boolean io_iswritable(IORec *io, fd_set *mask, Boolean check_mask)

{
  switch(io->out_part.type){
  case FileOutput:
    if(FD_ISSET(fileno(io->out_part.u.file.fp), mask)) return(True);
    else return(False);
  case CallbackOutput:
    if(check_mask) return(False);
    else return(True);
  case NoOutput:
    return(False);
  default:
    fprintf(stderr,
	    "ioiswritable - bad out_part type : %d\n", io->out_part.type);
    return(False);
  }
}

static Boolean handle_io(Context *context, IOArray *io_array)
{
  fd_set rmask, wmask, no_write;
  IORec *io, *next_io, *io_ptr, new_io;
  IOArray *ios;
  int n, val, save_sublevel;
  unsigned int mask;
  char *last, buf[MAX_BUF+1], *command;
  struct timeval *time;
  Boolean ret, io_writable, return_val, do_prompt, copy_jmp, save_prompt_after;
  void (*save_prompt)(void *);
  jmp_buf save_jmp;

  return_val = True;
  if(prompting) prompting = False;
  FD_ZERO(&no_write);
  do_prompt = True;
  IOArrayIterPtr(io_array, io){
    if(io->context->sub_level == 0) io->context->level++;
    io->context->sub_level++;
  }
  do {
    if(context->prompt_proc && do_prompt) (*context->prompt_proc)(NULL);
    do_prompt = False;
    FD_ZERO(&rmask);
    select_time = NULL;
    IOArrayIterPtr(io_array, io){
      if(io == IOArrayLastElement(io_array)) break;
      next_io = IOArrayElement(io_array, IOArrayIndex(io_array, io) + 1);
      if(io->in_part.type == FileInput){
	if(!io_iswritable(next_io, &no_write, True)){
	  if(io->in_part.u.file.buffered) select_time = &no_time;
	  else {
	    FD_SET(fileno(io->in_part.u.file.fp), &rmask);
	  }
	}
      }
      else if(io->in_part.type == ImmediateInput){
	if(!io_iswritable(next_io, &no_write, True)) select_time = &no_time;
      }
    }
    bcopy(&rmask, &select_rmask, sizeof(rmask));
    mask = sigblock(0);
    sigsetmask(mask & ~(1 << SIGCHLD));
    if((n = select(NOFILE, &select_rmask, &no_write, NULL,
		   select_time)) == -1){
      sigsetmask(mask);
      if(errno != EINTR){
	if(errno != EBADF) perror("select failed");
#ifdef notdef
	close_null_entries(io_array, True);
#endif
	IOArrayIterPtr(io_array, io){
	  if(--io->context->sub_level == 0) io->context->level--;
	}
	return(True);
      }
      close_null_entries(io_array, False);
      continue;
    }
    sigsetmask(mask);
    IOArrayIterPtr(io_array, io){
      if(io == IOArrayLastElement(io_array)) break;
      next_io = IOArrayElement(io_array, IOArrayIndex(io_array, io) + 1);
      if(io_iswritable(next_io, &no_write, False) &&
	(next_io->out_part.type == FileOutput)){
	FD_CLR(fileno(next_io->out_part.u.file.fp), &no_write);
      } 
      if(io_isreadable(io, &select_rmask)){
	io_writable = True;
	if(next_io->out_part.type == FileOutput){
	  FD_ZERO(&wmask);
	  FD_SET(fileno(next_io->out_part.u.file.fp), &wmask);
#ifdef notdef
	  if((n = select(NOFILE, NULL, &wmask, NULL, &no_time)) <= 0){
#endif
	  if((n = select(NOFILE, NULL, &wmask, NULL, &no_time)) == 0){
	    io_writable = False;
	    FD_SET(fileno(next_io->out_part.u.file.fp), &no_write);
	  }
	  else if(n < 0){
	    io_writable = False;
	    if(errno != EBADF) perror("selecting on write");
	    else {
	      (*next_io->out_part.proc)(next_io, NULL);
	      set_null_out(next_io);
	      (*io->in_part.block_proc)(io, NULL, 0);
	      set_null_in(io);
	    }
	  }
	}
	if(io_writable){
	  if(io->in_part.interpret){
	    do_prompt = True;
	    ret = (*io->in_part.line_proc)(io, &command);
	    if(command && (*command != '\0')){	      
	      if((val = setjmp(io->context->jmp)) == 0){
		if(isatty(0)) intr_exit = False;
		bcopy(current_jmp, save_jmp, sizeof(jmp_buf));
		copy_jmp = True;
		interpret_level++;
		bcopy(io->context->jmp, current_jmp, sizeof(jmp_buf));
		if(context->debug){
		  (*user_out_rec.out_part.proc)(&user_out_rec, command);
#ifdef notdef
		  (*user_out_rec.out_part.proc)(&user_out_rec, "\n");
#endif
		}
		new_cleanup_level();
		save_prompt_after = prompt_after;
		prompt_after = False;
		save_sublevel = io->context->sub_level;
		io->context->sub_level = 0;
		execute(io->context, command);
		io->context->sub_level = save_sublevel;
		if(prompt_after){
		  next_io->context->level++;
		  (*next_io->context->out_rec.out_part.proc)(next_io, NULL);
		  next_io->context->level--;
		}
		free(command);
	      }
	      else if(val != 1){
                /* come to here when the user quits in either way */
		if(dbx_dirty){
		  ios = IOArrayNew(NULL);
		  new_io.context = context;
		  new_io.output_to_user = False;
		  new_io.free_context = False;
		  set_null_out(&new_io);
		  set_packet_in(&new_io, context->dbx_fp, True);
		  IOArrayAppend(ios, &new_io);
		  set_cb_out(&new_io, CallbackOutput, write_nothing, NULL);
		  set_null_in(&new_io);
		  IOArrayAppend(ios, &new_io);
		  save_prompt = context->prompt_proc;
		  context->prompt_proc = NULL;
		  handle_io(context, ios);
		  context->prompt_proc = save_prompt;
		  IOArrayDestroy(ios);
		  printf("\n");
		  dbx_dirty = False;
		}
		intr = False;
		ret = False;
		return_val = True;
		if(context->pid != -1) {
                    kill(context->pid, SIGKILL);
                    return(return_val);
		}
	      }
	      else {
		ret = False;
		return_val = False;
	      }
	    }
	    if(copy_jmp){
	      bcopy(save_jmp, current_jmp, sizeof(jmp_buf));
	      do_cleanups();
	      interpret_level--;
	      copy_jmp = False;
	      prompt_after = save_prompt_after;
	    }
	  }
	  else {
	    n = MIN(io->in_part.buf_size, next_io->out_part.buf_size);
	    ret = (*io->in_part.block_proc)(io, buf, n);
	    if(*buf != '\0'){
	      if(context->debug)
		(*user_out_rec.out_part.proc)(&user_out_rec, buf);
	      if(io->output_to_user){
		io_ptr = &io->context->print_rec;
		(*io_ptr->out_part.proc)(io_ptr, buf);
	      }
	      else if(!(*next_io->out_part.proc)(next_io, buf)){
		(*io->in_part.block_proc)(io, NULL, 0);
		set_null_in(io);
		(*next_io->out_part.proc)(next_io, NULL);
		set_null_out(next_io);
	      }
	    }
	    else if(isatty(0) && ret == True){
	      (*next_io->out_part.proc)(next_io, NULL);
	    }
	  }
	  if(ret == False){
	    (*io->out_part.proc)(io, NULL);
	    (*io->in_part.block_proc)(io, NULL, 0);
	    (*next_io->out_part.proc)(next_io, NULL);
	    set_null_out(io);
	    set_null_in(io);
	    set_null_out(next_io);
	    if(io->output_to_user){
	      next_io = &io->context->print_rec;
	      (*next_io->out_part.proc)(next_io, NULL);
	    }
	  }
	}
      }
    }
    close_null_entries(io_array, False);
  } while(IOArraySize(io_array) > 0);
  return(return_val);
}

static void setup_io(Context *context, IOArray *ios, IORec *io_rec,
		     Boolean pipe, Boolean user_input)
{
  IORec *io, new_io;
  Context *ncontext;
  int i, size;

  if(pipe){
    io = IOArrayLastElement(ios);
    if(!io->free_context){
      io->context = copy_context(io->context);
      io->free_context = True;
    }
    io->context->print_rec.out_part = io_rec->out_part;
  }
  IOArrayAppend(ios, io_rec);
  if(IOArrayFirstElement(ios)->out_part.type != NoOutput){
    IOArrayInsert(ios, 0, &context->in_rec);
    if(io_rec->context->pid != -1)
      register_pid(io_rec->context->pid, ios, IOArraySize(ios) - 1);
  }
  if(IOArrayLastElement(ios)->in_part.type != NoInput){
    IOArrayAppend(ios, &context->out_rec);
  }
  if(user_input){
    size = IOArraySize(ios);
    new_io.free_context = True;
    new_io.context = copy_context(context);
    new_io.output_to_user = io_rec->output_to_user;
    set_text_in(&new_io, stdin, False, isatty(0));
    set_null_out(&new_io);
    IOArrayAppend(ios, &new_io);
    for(i=1;i<size;i++){
      IOArrayAppend(ios, IOArrayElement(ios, i));
    }
    io = IOArrayElement(ios, size + 1);
    io->context = copy_context(io->context);
    io->free_context = True;   
  }
}

static char *stringify(TokenArray *tokens)
{
  int len;
  Token *t;
  char *ret, *ptr;

  len = 0;
  TokenArrayIterPtr(tokens, t){
    switch(t->type){
    case SEMICOLON:
    case PIPE:
      len++;
      break;
    case DOUBLE_STRING:
    case SINGLE_STRING:
    case BACK_QUOTE:
      len += strlen(t->val) + 2;
      break;
    case WHITESPACE:
    case ID:
      len += strlen(t->val);
      break;
    case COMMENT:
      break;
    default:
      fprintf(stderr, "stringify - Bad type : %d\n", t->type);
      break;
    }
  }
  NEW_TYPE(ret, len + 1, char, char *, "stringify");
  ptr = ret;
  TokenArrayIterPtr(tokens, t){
    switch(t->type){
    case SEMICOLON:
      strcpy(ptr, ";");
      break;
    case PIPE:
      strcpy(ptr, "|");
      break;
    case DOUBLE_STRING:
      strcpy(ptr, "\"");
      strcat(ptr, t->val);
      strcat(ptr, "\"");
      break;
    case SINGLE_STRING:
      strcpy(ptr, "'");
      strcat(ptr, t->val);
      strcat(ptr, "'");
      break;
    case BACK_QUOTE:
      strcpy(ptr, "`");
      strcat(ptr, t->val);
      strcat(ptr, "`");
      break;
    case WHITESPACE:
    case ID:
      strcpy(ptr, t->val);
      break;
    case COMMENT:
      break;
    default:
      fprintf(stderr, "stringify - Bad type : %d\n", t->type);
      break;
    }
    ptr = STRING_END(ptr);
  }
  return(ret);
}

static TokenArray *eval_backquote(char *str, Context *context)
{
  Context *ncontext;
  int i, token;
  TokenArray *ret;
  Token tok;
  char *result;
  IORec save_out, save_print;

  result = NULL;
  save_out = context->out_rec;
  save_print = context->print_rec;
#ifdef notdef
  set_cb_out(&context->out_rec, FileOutput, write_string, &result);
#endif
  set_cb_out(&context->out_rec, CallbackOutput, write_string, &result);
  context->print_rec = context->out_rec;
  execute(context, crunch(str));
  ret = TokenArrayNew(NULL);
  if(result){
    if(*(STRING_END(result) - 1) == '\n') *(STRING_END(result) - 1) = '\0';
    set_input(result);
    i = 0;
    while((token = yylex()) != 0){
      if(token == ERROR){
	TokenArrayDestroy(ret);
	return(NULL);
      }
      tok.type = token;
      tok.val = yylval;
      TokenArrayInsert(ret, i, &tok);
      i++;
    }
  }
  context->out_rec = save_out;
  context->print_rec = save_print;
  return(ret);
}

static void cleanup_parse(CleanParse *arg)
{  
  if(arg->context)
    arg->context->prompt_proc = arg->prompt_proc;
  if(arg->ios) IOArrayDestroy(arg->ios);
  free(arg);
}

static Boolean parse(TokenArray *tokens, Context *context)
{
  TokenArray *new, *quote;
  Token *t, tok, *ptr, *old;
  int token, i, j, start, current_index, cleanup;
  Boolean ret, expand, pipe, user_input;
  IORec io_rec, *io;
  IOArray *ios;
  void (*prompt_proc)(void *);
  Context *ncontext;
  CleanParse *arg;

  if(TokenArraySize(tokens) > 0){
    expand = True;
    io_rec.context = context;
    io_rec.output_to_user = False;
    i = 0;
    start = i;
    ios = IOArrayNew(NULL);
    pipe = False;
    TokenArrayIterPtr(tokens, t){
      switch(t->type){
      case SEMICOLON:
	if(!execute_command(tokens, start, i, context, &io_rec, &user_input))
	  return(False);
	if(!isatty(0) && (io_rec.out_part.type == FileInput) &&
	   (io_rec.out_part.u.file.fp == stdin)) set_null_out(&io_rec);
	setup_io(io_rec.context, ios, &io_rec, pipe, user_input);
	pipe = False;
	NEW_TYPE(arg, 1,  CleanParse, CleanParse *, "parse");
	if(!io_rec.free_context){
	  arg->context = io_rec.context;
	  arg->prompt_proc = io_rec.context->prompt_proc;
	}
	else arg->context = NULL;
	arg->ios = NULL;
	io_rec.context->prompt_proc = NULL;
	cleanup = add_cleanup((void (*)(void *)) cleanup_parse, arg);
	handle_io(io_rec.context, ios);
	delete_cleanup(cleanup, True);
	start = i + 1;
	IOArrayEmpty(ios);
	expand = True;
	break;
      case PIPE:
	if(!execute_command(tokens, start, i, context, &io_rec, &user_input))
	  return(False);
	IOArrayAppend(ios, &io_rec);
	if(IOArrayFirstElement(ios)->out_part.type != NoOutput){
	  IOArrayInsert(ios, 0, &io_rec.context->in_rec);
	}
	if(io_rec.context->pid != -1)
	  register_pid(io_rec.context->pid, ios, IOArraySize(ios) - 1);
	io_rec.output_to_user = False;
	start = i + 1;
	expand = True;
	pipe = True;
	break;
      case DOUBLE_STRING:
	set_input(t->val);
	new = TokenArrayNew(NULL);
	while((token = yylex()) != 0){
	  if(token == ERROR) return(False);
	  tok.type = token;
	  tok.val = yylval;
	  TokenArrayAppend(new, &tok);
	}
	TokenArrayIterPtr(new, ptr){
	  if(ptr->type == BACK_QUOTE){
	    if((quote = eval_backquote(ptr->val, context)) == NULL){
	      TokenArrayDestroy(new);
	      return(False);
	    }
	    current_index = TokenArrayIndex(new, ptr);
	    old = TokenArrayElement(new, current_index);
	    if(old->val) free(old->val);
	    TokenArrayDelete(new, current_index);
	    TokenArrayIterPtr(quote, ptr){
	      TokenArrayInsert(new, current_index, ptr);
	      current_index++;
	    }
	    ptr = TokenArrayElement(new, current_index);
	    TokenArrayDestroy(quote);
	  }
	}
	free(t->val);
	t->val = stringify(new);
	TokenArrayIterPtr(new, ptr){
	  if(ptr->val) free(ptr->val);
	}
	TokenArrayDestroy(new);
	expand = False;
	break;
      case SINGLE_STRING:
	expand = False;
	break;
      case BACK_QUOTE:
	if((new = eval_backquote(t->val, context)) == NULL) return(False);
	current_index = TokenArrayIndex(tokens, t);
	TokenArrayDelete(tokens, current_index);
	j = 0;
	TokenArrayIterPtr(new, ptr){
	  TokenArrayInsert(tokens, current_index + j, ptr);
	  j++;
	}
	t = TokenArrayElement(tokens, current_index - 1);
	TokenArrayDestroy(new);
	expand = False;
	continue;
      case WHITESPACE:
	break;
      case ID:
	if(expand){
	  current_index = TokenArrayIndex(tokens, t);
	  if(!expand_aliases(tokens, i, context)) return(False);
	  t = TokenArrayElement(tokens, current_index - 1);
	  expand = False;
	  continue;
	}
	break;
      case COMMENT:
	return(False);
      default:
	fprintf(stderr, "parse - bad token type : %d\n", t->type);
	break;
      }
      i++;
    }
    if(!execute_command(tokens, start, i, context, &io_rec, &user_input))
      return(False);
    if(!isatty(0) && (io_rec.out_part.type == FileInput) &&
       (io_rec.out_part.u.file.fp == stdin)) set_null_out(&io_rec);
    setup_io(io_rec.context, ios, &io_rec, pipe, user_input);
    NEW_TYPE(arg, 1,  CleanParse, CleanParse *, "parse");
    if(!io_rec.free_context){
      arg->context = io_rec.context;
      arg->prompt_proc = io_rec.context->prompt_proc;
    }
    else arg->context = NULL;
    arg->ios = ios;
    io_rec.context->prompt_proc = NULL;
    cleanup = add_cleanup((void (*)(void *)) cleanup_parse, arg);
    handle_io(io_rec.context, ios);
    delete_cleanup(cleanup, True);
  }
  return(True);
}

static void execute(Context *context, char *command)
{
  TokenArray *tokens;
  Token t, *ptr;
  int token;
  IORec *io;

  tokens = TokenArrayNew(NULL);
  set_input(command);
  while((token = yylex()) != 0){
    if(token == ERROR) break;
    t.type = token;
    t.val = yylval;
#ifdef notdef
    t.val = crunch(yylval);
#endif
    TokenArrayAppend(tokens, &t);
  }
  if(token != ERROR) parse(tokens, context);
  TokenArrayIterPtr(tokens, ptr){
    if(ptr->val) free(ptr->val);
  }
}

static Boolean execute_command(TokenArray *command, int start, int end,
			       Context *context, IORec *io,
			       Boolean *user_input)
{
  int i;
  Boolean (*ret)();
  Boolean (*save_proc)(IORec *, char *), val;
  TokenArray *tokens;
  Token token, *t;
  Context *ncontext;

  tokens = TokenArrayNew(NULL);
  for(i=start;i<end;i++){
    token = *TokenArrayElement(command, i);
    if(token.type != WHITESPACE){
      if(token.val) token.val = copy(token.val, -1);
      token.val = crunch(token.val);
      TokenArrayAppend(tokens, &token);
    }
  }
  val = False;
  if(TokenArraySize(tokens) > 0){
    ret = NULL;
    for(i=0;i<sizeof(commands)/sizeof(commands[0]);i++){
      if(!strcmp((TokenArrayFirstElement(tokens))->val, commands[i].command)){
	ret = commands[i].proc;
	break;
      }
    }
    if(ret == NULL){
      ret = do_dbx;
      i = DBX_INDEX;
    }
    else {
      if((TokenArrayFirstElement(tokens))->val)
	free((TokenArrayFirstElement(tokens))->val);
      TokenArrayDelete(tokens, 0);
    }
#ifdef notdef
    if(ret == do_quit) printf("quitting1\n");
#endif
    if(commands[i].copy_context){
      ncontext = copy_context(context);
      if(context->debug && context->debug_recurse) ncontext->debug = True;
      ncontext->parent = context;
      io->free_context = True;
    }
    else {
      ncontext = context;
      io->free_context = False;
    }
    if(user_input) *user_input = commands[i].user_input;
    io->context = ncontext;
    if(!(*ret)(tokens, ncontext, io) && commands[i].help){
      (*user_out_rec.out_part.proc)(&user_out_rec, commands[i].help);
      (*user_out_rec.out_part.proc)(&user_out_rec, "\n");
      val = False;
    }
    else val = True;
  }
  TokenArrayIterPtr(tokens, t){
    if(t->val) free(t->val);
  }
  TokenArrayDestroy(tokens);
  return(val);
}

static void Usage(void)
{
  fprintf(stderr, "kdbx [-dbx dbx-path] dbx-args\n");
  exit(1);
}

main(int argc, char **argv, char **envp)
{
  FILE *dbx_fp;
  Context *context;
  char initfile[MAXPATHLEN+1], *dir, *path, *newpath, *s, *colon, *extpath;
  int val, i;
  struct ltchars buf;
  IOArray *ios;
  IORec io;

  argc--;
  argv++;
  if(argc == 0) Usage();
  while(argc > 0){
    if(!strcmp(argv[0], "-dbx")){
      if(argc > 1){
	dbx_to_use = argv[1];
	argc -= 2;
	argv += 2;
      }
      else Usage();
    }
    else break;
  }
  env = envp;
  if(terminal = (isatty(0) && isatty(1))) {
    (void) ioctl(1, TIOCGWINSZ, (char *) &win); /* get winsize */
    if(win.ws_row == 0) win.ws_row = 24;
    if(win.ws_col == 0) win.ws_col = 80;
    (void)ioctl(0, TIOCGETP, (char *)&ttybsave);
    ttybnew = ttybsave;
  }
  signal(SIGPIPE, pipe_handler);
  signal(SIGINT, interrupt);
  signal(SIGWINCH, window);
  signal(SIGALRM, alarm_handler);
  signal(SIGCONT, cont_handler);
  sigblock(1 << SIGCHLD);
  pids = PidArrayNew(NULL);
  no_complain = ProcArrayNew(NULL);
  ioctl(0, TIOCGLTC, &buf);
  repaint = buf.t_rprntc;
  user_out_rec.context = NULL;
  set_text_out(&user_out_rec, stdout, True);
  set_null_in(&user_out_rec);
  context = new_context();
  set_null_out(&context->in_rec);
  set_text_in(&context->in_rec, stdin, False, True);
  set_text_out(&context->out_rec, stdout, True);
  set_null_in(&context->out_rec);
  context->out_rec.context = context;
  context->in_rec.context = context;
  context->user = &context->vis;
  if((dbx_fp = start_dbx(argc, argv, context)) == NULL){
    fprintf(stderr, "\nCouldn't start dbx\nExiting\n");
    exit(1);
  }
  signal(SIGCHLD, child_handler);
  set_text_in(&context->in_rec, stdin, True, True);
  context->dbx_fp = dbx_fp;
  if(isatty(0)) context->prompt_proc = prompt;
  pids = PidArrayNew(NULL);
  if((dir = getenv("KRASH_LIBDIR")) == NULL) dir = "/var/kdbx";
  if((extpath = getenv("KRASH_EXTPATH")) == NULL) extpath = "";
  if((*extpath != '\0') && (extpath[strlen(extpath) - 1] != ':')) colon = ":";
  else colon = "";
  if((path = getenv("PATH")) == NULL){
    NEW_TYPE(newpath, strlen("PATH=") + strlen(extpath) + strlen(colon) +
	     strlen(dir) + 1, char, char *, "main");
    sprintf(newpath, "PATH=%s%s%s", extpath, colon, dir);
  }
  else {
    NEW_TYPE(newpath, strlen("PATH=") + strlen(extpath) + strlen(colon) +
	     strlen(dir) + strlen(path) + 2, char, char *, "main");
    sprintf(newpath, "PATH=%s%s%s:%s", extpath, colon, dir, path);
  }
  putenv(newpath);
  strcpy(initfile, dir);
  strcat(initfile, "/system.kdbxrc");
  readfile(initfile, False, context, True);
  strcpy(initfile, dir);
  strcat(initfile, "/site.kdbxrc");
  readfile(initfile, False, context, True);
  if((dir = getenv("HOME")) != NULL){
    strcpy(initfile, dir);
    strcat(initfile, "/.kdbxrc");
    readfile(initfile, False, context, True);
  }
  getwd(initfile);
  if(!dir || (dir && strcmp(initfile, dir))){
    strcat(initfile, "/.kdbxrc");
    readfile(initfile, False, context, True);
  }
  ios = IOArrayNew(NULL);
  do {
    set_null_out(&io);
    set_text_in(&io, stdin, True, True);
    io.context = context;
    io.output_to_user = False;
    io.free_context = False;
    IOArrayAppend(ios, &io);
    set_text_out(&io, stdout, True);
    set_null_in(&io);
    IOArrayAppend(ios, &io);
    in_loop = True;
  } while(handle_io(context, ios));
  signal(SIGCHLD, SIG_DFL);
  if((dbx_pid != -1) && (kill(dbx_pid, SIGKILL) == -1) && (errno != ESRCH))
    fprintf(stderr, "Couldn't kill dbx (pid = %d)\n", dbx_pid);
  return(0);
}
